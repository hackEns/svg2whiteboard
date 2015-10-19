set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake) # Arduino Toolchain

set(ARDUINO_DEFAULT_BOARD mega) # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT /dev/ttyACM0) # Default Port, when not specified
cmake_minimum_required(VERSION 2.8)
#====================================================================#
#  Setup Project                                                     #
#====================================================================#
project(Svg2Whiteboard C CXX)

print_board_list()
print_programmer_list()

#====================================================================#
# Original blink sketch (from Arduino SDK examples)
#====================================================================#
# Anyone wants to dig into arduino-cmake to find a clean way to do that?
add_definitions(-DARDUINO_ARCH_AVR)

# Some magic, cf http://forum.pololu.com/viewtopic.php?f=3&t=3898
add_definitions(-fno-use-cxa-atexit)

generate_arduino_library(servo_lib
    SRCS /usr/share/arduino/libraries/Servo/src/avr/Servo.cpp
    HDRS /usr/share/arduino/libraries/Servo/src/Servo.h
    BOARD mega)

include_directories(/usr/share/arduino/libraries/Servo/src/)

generate_arduino_firmware(whiteboard_clock
    SKETCH "whiteboard_clock.ino"
    LIBS servo_lib mega_CORE
    PORT  /dev/ttyACM0
    SERIAL picocom @SERIAL_PORT@
    BOARD mega)

