CXX       ?= c++
BUILD_DIR ?= build

SOURCES := \
	src/color.cpp \
	src/defines.cpp \
	src/led.cpp \
	src/object.cpp \
	src/pin.cpp \
	tests/test_adk.cpp

HEADERS := $(shell find src ../tests/fake_arduino -type f -name '*.h' | sort)
TEST    := $(BUILD_DIR)/test_adk

CPPFLAGS += -Isrc -I../tests/fake_arduino
CXXFLAGS += -std=c++17 -Os -fno-exceptions -fno-rtti
CXXFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Werror

.DEFAULT_GOAL := check

.PHONY: check clean
check: $(TEST)
	$(TEST)

$(TEST): $(SOURCES) $(HEADERS)
	mkdir -p "$(BUILD_DIR)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o "$@"

clean:
	rm -rf -- "$(BUILD_DIR)"
