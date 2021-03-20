BOARD_TAG = nano
BOARD_SUB = atmega328old
ARDMK_VENDOR = archlinux-arduino
ARDUINO_PORT = /dev/ttyUSB0
# NOTE: The MCUFRIEND_kbv library needs hacking:
# Edit utility/mcufriend_shield.h:  #define USE_SPECIAL
# Edit utility/mcufriend_special.h: #define USE_OPENSMART_SHIELD_PINOUT
# Edit MCUFRIEND_kbv.cpp:           #define SUPPORT_8352B
ARDUINO_LIBS = SPI Adafruit_GFX_Library MCUFRIEND_kbv
#ARDUINO_DIR = /usr/share/arduino
USER_LIB_PATH = $(HOME)/Arduino/libraries
include /usr/share/arduino/Arduino.mk
CXXFLAGS_STD=-std=c++17
