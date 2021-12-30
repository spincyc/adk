
ARDMK_DIR       := $(HOME)/git/Arduino-Makefile
ARDUINO_DIR     := $(HOME)/Downloads/arduino-1.8.19

USER_LIB_PATH   := $(HOME)/git
ARDUINO_LIBS    := adk

BOARD_TAG       := mega
BOARD_SUB       := atmega2560
ARDUINO_PORT    := /dev/ttyACM0

CFLAGS_STD      := -std=gnu11
CXXFLAGS_STD    := -std=c++17

DIAGNOSTICS_COLOR_WHEN := never

include $(ARDMK_DIR)/Arduino.mk

