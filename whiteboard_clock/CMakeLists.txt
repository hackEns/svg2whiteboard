# This configuration file outlines some of the ways Arduino CMake
# can be used. For a detailed explenation of all the options please
# reade README.rst.

set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake) # Arduino Toolchain

set(ARDUINO_DEFAULT_BOARD mega) # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT /dev/ttyACM1) # Default Port, when not specified
cmake_minimum_required(VERSION 2.8)
#====================================================================#
#  Setup Project                                                     #
#====================================================================#
project(Svg2Whiteboard C CXX)

print_board_list()
print_programmer_list()
#====================================================================#
# master_writer example from Wire library
#====================================================================#
#generate_arduino_example(wire_example
#                         LIBRARY Wire
#                         EXAMPLE master_writer)

# Alternative: by vairables
#set(wire_example_LIBRARY Wire)
#set(wire_example_EXAMPLE master_writer)
#
#generate_arduino_example(wire_example)



#====================================================================#
# Original blink sketch (from Arduino SDK examples)
#====================================================================#
# Anyone wants to dig into arduino-cmake to find a clean way to do that?
add_definitions(-DARDUINO_ARCH_AVR)

generate_arduino_library(servo_lib
    SRCS /usr/share/arduino/libraries/Servo/src/avr/Servo.cpp
    HDRS /usr/share/arduino/libraries/Servo/src/Servo.h
    BOARD mega)

include_directories(/usr/share/arduino/libraries/Servo/src/)

generate_arduino_firmware(whiteboard_clock
    SKETCH "whiteboard_clock.ino"
    LIBS servo_lib
    PORT  /dev/ttyACM1
    SERIAL picocom @SERIAL_PORT@
    BOARD mega)

# Alternative: by variables
#set(blink_original_SKETCH "${BLINK_SKETCH}")
#set(blink_original_PORT  /dev/ttyACM)
#set(blink_original_SERIAL picocom @SERIAL_PORT@)
#set(blink_original_BOARD uno)
#
#generate_arduino_firmware(blink_original)




#====================================================================#
# Bundled blink sketch example
#====================================================================#
#generate_arduino_firmware(blink_bundled
#    SKETCH Blink
#    PROGRAMMER usbtinyisp
#    NO_AUTOLIBS)

# Alternative: by variables
#set(blink_bundled_SKETCH Blink)
#set(blink_bundled_PROGRAMMER usbtinyisp)
#set(blink_bundled_NO_AUTOLIBS true)
#
#generate_arduino_firmware(blink_bundled)




#====================================================================#
# Advanced static library exmaple
#====================================================================#
#generate_arduino_library(blink_lib
#    SRCS blink_lib.cpp
#    HDRS blink_lib.h
#    BOARD uno)

# Alternative: by variables
#set(blink_lib_SRCS blink_lib.cpp)
#set(blink_lib_HDRS blink_lib.h)
#set(blink_lib_BOARD uno)
#
#generate_arduino_library(blink_lib)




#====================================================================#
# Advanced firwmare example
#====================================================================#
#generate_arduino_firmware(blink
#    SRCS blink.cpp
#    LIBS blink_lib
#    BOARD uno)

# Alternative: by variables
#set(blink_SRCS blink.cpp)
#set(blink_LIBS blink_lib)
#set(blink_BOARD uno)
#
#generate_arduino_firmware(blink)
