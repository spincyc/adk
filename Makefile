# Everything this Makefile builds goes under $(BUILD_DIR), which git ignores.

BUILD_DIR ?= build
FQBN      ?= arduino:avr:mega
PORT      ?= /dev/ttyACM0
CXX       ?= c++
PYTHON    ?= python3

LIBRARY_SOURCES := $(wildcard src/adk/*.cpp)
TEST_SOURCES    := $(wildcard tests/*.cpp tests/arduino/*.cpp)
EXAMPLES        := $(patsubst examples/%/,%,$(sort $(dir $(wildcard examples/*/*.ino))))

HOST_DIR   := $(BUILD_DIR)/host
HOST_FLAGS := -std=c++11 -g -Os -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror \
              -fno-exceptions -fno-rtti -Isrc -Itests/arduino -Itests
HOST_OBJECTS := $(patsubst %.cpp,$(HOST_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))

SANITIZE_DIR     := $(BUILD_DIR)/sanitize
SANITIZE_FLAGS   := $(HOST_FLAGS) -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
SANITIZE_OBJECTS := $(patsubst %.cpp,$(SANITIZE_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))

ARDUINO_DIR   := $(BUILD_DIR)/arduino
ARDUINO_CACHE := $(BUILD_DIR)/arduino-cache
ARDUINO_LOGS  := $(patsubst %,$(ARDUINO_DIR)/%.log,$(EXAMPLES))
LIBRARY_FILES := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp) library.properties

VENV := $(BUILD_DIR)/venv
export PYTHONPYCACHEPREFIX := $(abspath $(BUILD_DIR))/pycache

.DEFAULT_GOAL := test
.SECONDEXPANSION:
.PHONY: all check test sanitize examples size site serve style upload monitor clean help

all: check

## check         everything CI runs: tests, examples, sizes, and the site
check: style test sanitize examples size site

## test          build and run the host tests (TEST=name runs matching cases)
test: $(HOST_DIR)/tests
	$(HOST_DIR)/tests $(TEST)

$(HOST_DIR)/tests: $(HOST_OBJECTS)
	@$(CXX) $(HOST_FLAGS) $^ -o $@

$(HOST_DIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "  CXX  $<"
	@$(CXX) $(HOST_FLAGS) -MMD -MP -c $< -o $@

## sanitize      run the host tests under AddressSanitizer and UBSan
sanitize: $(SANITIZE_DIR)/tests
	$(SANITIZE_DIR)/tests $(TEST)

$(SANITIZE_DIR)/tests: $(SANITIZE_OBJECTS)
	@$(CXX) $(SANITIZE_FLAGS) $^ -o $@

$(SANITIZE_DIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "  CXX  $< (sanitized)"
	@$(CXX) $(SANITIZE_FLAGS) -MMD -MP -c $< -o $@

-include $(HOST_OBJECTS:.o=.d) $(SANITIZE_OBJECTS:.o=.d)

## examples      compile every example for the Mega 2560
examples: $(ARDUINO_LOGS)

$(ARDUINO_DIR)/%.log: examples/$$*/$$*.ino $(LIBRARY_FILES)
	@mkdir -p $(ARDUINO_DIR)/$* $(ARDUINO_CACHE)
	@echo "  AVR  examples/$*"
	@ARDUINO_BUILD_CACHE_PATH=$(abspath $(ARDUINO_CACHE)) \
	    arduino-cli compile --fqbn $(FQBN) --library . --warnings all \
	    --build-path $(ARDUINO_DIR)/$* examples/$* > $@.tmp 2>&1 \
	    || (cat $@.tmp; rm -f $@.tmp; exit 1)
	@if grep -A3 -E '(src/adk|examples)/.*warning' $@.tmp; then rm -f $@.tmp; exit 1; fi
	@mv $@.tmp $@

## size          print the flash and RAM each example uses
size: $(ARDUINO_LOGS)
	@printf '%-36s %8s %6s\n' Example Flash RAM
	@for example in $(EXAMPLES); do \
	    log=$(ARDUINO_DIR)/$$example.log; \
	    flash=$$(sed -n 's/^Sketch uses \([0-9]*\) bytes.*/\1/p' $$log); \
	    ram=$$(sed -n 's/^Global variables use \([0-9]*\) bytes.*/\1/p' $$log); \
	    printf '%-36s %8s %6s\n' $$example $$flash $$ram; \
	done

## site          build the website into build/site
site: $(VENV)/.installed
	$(VENV)/bin/mkdocs build --strict --site-dir $(abspath $(BUILD_DIR))/site

## serve         preview the website at http://127.0.0.1:8000
serve: $(VENV)/.installed
	$(VENV)/bin/mkdocs serve

$(VENV)/.installed: docs/requirements.txt
	$(PYTHON) -m venv $(VENV)
	$(VENV)/bin/pip install --quiet --disable-pip-version-check -r $<
	@touch $@

STYLED := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp tests/*.h tests/*.cpp tests/arduino/*) \
          $(wildcard examples/*/*.ino)

## style         check the mechanical rules of docs/STYLE.md
style:
	@$(PYTHON) tests/style.py $(STYLED)

## upload        upload one example: make upload EXAMPLE=Lesson01Blink PORT=/dev/ttyACM0
upload: $(ARDUINO_DIR)/$(EXAMPLE).log
	arduino-cli upload --fqbn $(FQBN) --port $(PORT) --input-dir $(ARDUINO_DIR)/$(EXAMPLE)

## monitor       open the serial monitor at 9600 baud
monitor:
	arduino-cli monitor --port $(PORT) --config baudrate=9600

## clean         remove everything this Makefile built
clean:
	rm -rf $(BUILD_DIR)

## help          list these targets
help:
	@sed -n 's/^## //p' $(MAKEFILE_LIST)
