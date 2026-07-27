ARDUINO_CLI ?= arduino-cli
CXX         ?= c++
PDFLATEX    ?= pdflatex

BUILD_DIR   ?= build
BUILD_MARKER := $(BUILD_DIR)/.adk-build
BOARD_FQBN  ?= arduino:avr:mega
LESSONS     := 001 002 003 004 005 006 007 008 009 010 011 012 013 014
EXAMPLES    := \
	Lesson001DigitalOutput \
	Lesson002DigitalInput \
	Lesson003ReactionTimer \
	Lesson004PwmRgb \
	Lesson005PiezoSounder \
	Lesson006Simon \
	Lesson007AnalogInput \
	Lesson008SampledSignal \
	Lesson009AdaptiveNightLight \
	Lesson010ShiftRegisterDisplay \
	Lesson011TimedTraffic \
	Lesson012TrafficJunction \
	Lesson013Dht11Climate \
	Lesson014CharacterDisplay
PORT        ?=
BAUD        ?= 115200
SERIAL_LOG  ?= $(BUILD_DIR)/serial/monitor.log

HOST_CPPFLAGS += -Isrc -Itests/fake_arduino
HOST_CXXFLAGS += -std=c++17 -Os -flto
HOST_CXXFLAGS += -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections
HOST_CXXFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Werror
HOST_LDFLAGS  += -flto -Wl,--gc-sections

$(BUILD_MARKER):
	mkdir -p "$(BUILD_DIR)"
	touch "$@"
