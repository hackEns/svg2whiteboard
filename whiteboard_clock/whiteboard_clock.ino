#include <Servo.h>

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define SERVO_PIN          11

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24

#define SDPOWER            -1
#define SDSS               53
#define LED_PIN            13

#define FAN_PIN            9

#define PS_ON_PIN          12
#define KILL_PIN           -1

#define HEATER_0_PIN       10
#define HEATER_1_PIN       8
#define TEMP_0_PIN          13   // ANALOG NUMBERING
#define TEMP_1_PIN          14   // ANALOG NUMBERING

#define STEPS_PER_TURN 6400

#define X_MACHINE 0
#define Y_MACHINE 1
#define Z_MACHINE 2
#define E_MACHINE 3

#define L1 Z_MACHINE
#define L2 E_MACHINE

char step_pin[] = {X_STEP_PIN, Y_STEP_PIN, Z_STEP_PIN, E_STEP_PIN};
char dir_pin[] = {X_DIR_PIN, Y_DIR_PIN, Z_DIR_PIN, E_DIR_PIN};
char enable_pin[] = {X_ENABLE_PIN, Y_ENABLE_PIN, Z_ENABLE_PIN, E_ENABLE_PIN};
float pos[] = {0.,0.,0.,0.};
float frac[] = {0.,0.,0.,0.};

#define stepsPerCm 682.0f
float spdcmps = 60.0f;

// Serial management

// memory to store a list of points
#define MAX_COMMANDS 10
float stacked_points_x[MAX_COMMANDS];
float stacked_points_y[MAX_COMMANDS];
bool stacked_points_drawn[MAX_COMMANDS];

// how far the stacked_points_* array should be browsed (the last points could not be relevant)
int commands_saved = 0;

#define START_LINE 0
#define READING_X 1
#define READING_Y 2
#define READING_IS_DRAWN 3
#define WAITING_EOL 4
#define SERIAL_ERROR 5

int serial_state = START_LINE;
String waiting_data;
float waiting_x = 0.;
float waiting_y = 0.;
boolean waiting_drawn = true;




// Advance axis /axis/ to increase its position by /dxc/ (positive, given in centimeters).
// /dxc/ can be fractionnal. To be safe, always give distance < 1/stepsPerCm, since the steppers
// have no fixed speed.
void advance(float dxc, char axis) {
  if (dxc==0) return;
  dxc=dxc+frac[axis];
  int dx = dxc*stepsPerCm;
  if (dx>1) {
    Serial.print("/!\\ dxc = ");
    Serial.print(dxc);
    Serial.println(">= 2/stepsPerCm");
  }
  float rem = dxc*stepsPerCm - dx;
  frac[axis]=rem/stepsPerCm;
  for (int i=0; i<dx; i++) {
    digitalWrite(step_pin[axis],1);
    delayMicroseconds(1);
    digitalWrite(step_pin[axis],0);
 }
}

// Moves cursor by (dl1c,dl2c) (given in cm), the fastest axis goes at speed spd (in cm/s)
// dxc and dyc should not both be 0
void coordinated(float dxc, float dyc, float spd) {
  if (dxc<0) {
    dxc=-dxc;
    digitalWrite(dir_pin[L1],1);
  }
  else
    digitalWrite(dir_pin[L1],0);
  if (dyc<0) {
    dyc=-dyc;
    digitalWrite(dir_pin[L2],1);
  }
  else
    digitalWrite(dir_pin[L2],0);
  
  float acc = 0;
  if (dxc>dyc) {
    // x goes faster
    while (acc < dxc) {
      float adv;
      if (acc + 1.0f/stepsPerCm <= dxc)
        adv = 1/stepsPerCm;
      else
        adv = dxc - acc;
      advance(adv, L1);
      advance(dyc/dxc * adv, L2);
      acc += adv;
   //   Serial.println(1000000/(spd*stepsPerCm));
      delayMicroseconds(1000000/(spd*stepsPerCm));
    }
  }
  else {
    // y goes faster
    while (acc < dyc) {
      float adv;
      if (acc + 1/stepsPerCm <= dyc)
        adv = 1/stepsPerCm;
      else
        adv = dyc - acc;
      advance(dxc/dyc * adv, L1);
      advance(adv, L2);
      acc += adv;
      delayMicroseconds(1000000/(spd*stepsPerCm));
    }
  }
}

// Distance between spools
const float e1 = 50.0f;

// Convert (x,y) to (l1,l2)
float l1(float x, float y) {
  return sqrt(y*y+(e1/2+x)*(e1/2+x));
}

float l2(float x, float y) {
  return sqrt(y*y+(e1/2-x)*(e1/2-x));
}

// Current (l1,l2)
float cl1=12, cl2=12;

void mv(float l1, float l2) {
  Serial.print("We are at ");
  Serial.print(cl1);
  Serial.print(", ");
  Serial.print(cl2);
  Serial.print(" and are going to ");
  Serial.print(l1);
  Serial.print(", ");
  Serial.println(l2);
  
  float dl1=l1-cl1;
  float dl2=l2-cl2;
  coordinated(dl1,dl2,spdcmps);
  cl1=l1;
  cl2=l2;
}

float y0 = 30;
float currentX=0, currentY=y0;

void gotoXY(float tx, float ty) {
  Serial.print("Goto X="); Serial.print(tx); Serial.print(", ");
  Serial.println(ty);
  mv(l1(tx,ty),l2(tx,ty));
  currentX = tx;
  currentY = ty;
}

void interpretCommand() {
  Serial.print("Enter actual l1 and l2. Expecting ");
  Serial.println(cl1);
  while (Serial.available()==0);
  Serial.setTimeout(5);
  float l1 = Serial.parseFloat();
  float l2 = Serial.parseFloat();
  float rl1 = cl1-l1;
  float rl2 = cl2-l2;
  Serial.print("rewind l1 = ");
  Serial.print(rl1);
  Serial.print(", rewind l2 = ");
  Serial.print(rl2);
  Serial.println();
  coordinated(rl1,rl2,spdcmps);
}

float xmin = -44.4175;
float xmax = 15.5938;
float ymin = -11.9128;
float ymax = 26.7276;


float f = 4.0f;//(xmax-xmin)/15;

void moveTo(float x, float y) {
  gotoXY(x/f,y/f+y0);
}
void lineTo(float x, float y) {
  gotoXY(x/f,y/f+y0);
}
void lineToRel(float x, float y) {
  gotoXY(currentX+x/f, currentY+y/f);
}
#include "commands.c"


float data[] = {0,0, 2,-2, -2,-2, -2,0, 2,0, 2,2, -2,2};
int datalen = 7;

Servo servo;

void setup() {

  pinMode(FAN_PIN,OUTPUT);
  digitalWrite(FAN_PIN, 1);
  Serial.begin(115200);
  waiting_data.reserve(200); // FIXME: check for overflows
  for (int i=0; i<4; i++) {
    pinMode(step_pin[i],OUTPUT);
    pinMode(dir_pin[i],OUTPUT);
    pinMode(enable_pin[i],OUTPUT);
    digitalWrite(step_pin[i],0);
    digitalWrite(dir_pin[i],0);
    digitalWrite(enable_pin[i],0);  //0 for enable
  }
  cl1 = 75;
  cl2 = 75;

  servo.attach(SERVO_PIN);
}

bool first_time = true;
int current_command = 0;

void erase_saved_commands () {
	current_command = 0;
	commands_saved = 0;
	Serial.println("REQUEST_DATA");
}

void loop() {
	if(first_time) {
		servo.write(43);
		servo.write(55);
		/*delay(2000);
		  servo.write(55);
		  delay(2000);*/

		delay(1000);
		//interpretCommand();

		gotoXY(0,70);
		moveTo(0,0);
		servo.write(43);
		first_time = false;
	}

	if (current_command == commands_saved && current_command > 0) {
		erase_saved_commands ();
	}
	else if(current_command < commands_saved) {
		if (stacked_points_drawn[current_command]) {
			servo.write(43);
		}
		else {
			servo.write(55);
		}
		lineToRel (stacked_points_x[current_command], stacked_points_y[current_command]);
		current_command++;
	}

	delay(100);
}

void serialEvent() {
	/* small automaton */
	/* 0: start of a line
	1: reading X
	2: reading Y
	3: reading is_drawn
	4: error*/
	while (Serial.available()) {
		Serial.println("received_serial");
		if(commands_saved == MAX_COMMANDS) {
			Serial.println("STACK_FULL");
			break;
		}
		// get the new byte:
		char inChar = (char)Serial.read();
		Serial.println("received_serial" + inChar);
		if (inChar == '\n') {
			if (serial_state != WAITING_EOL) {
				Serial.println("ERROR");
			}
			else {
				stacked_points_x[commands_saved] = waiting_x;
				stacked_points_y[commands_saved] = waiting_y;
				stacked_points_drawn[commands_saved] = waiting_drawn;
				commands_saved ++;
			}
			serial_state = START_LINE;
		} else if (serial_state == START_LINE && inChar == 'P') {
			serial_state = READING_X;
			waiting_data = "";
		}
		else if (serial_state == READING_X && inChar == ';') {
			serial_state = READING_Y;
			waiting_x = waiting_data.toFloat ();
			waiting_data = "";
		}
		else if (serial_state == READING_Y && inChar == ';') {
			serial_state = READING_IS_DRAWN;
			waiting_y = waiting_data.toFloat ();
			waiting_data = "";
		}
		else if (serial_state == READING_IS_DRAWN) {
			if(inChar == '0') {
				waiting_drawn = false;
				serial_state = WAITING_EOL;
			} else if(inChar == '1') {
				waiting_drawn = true;
				serial_state = WAITING_EOL;
			}
			else {
				serial_state = SERIAL_ERROR;
				Serial.println("ERROR");
			}
		}
		else if (serial_state == READING_X || serial_state == READING_Y) {
			waiting_data += inChar;
		}
	}
}
