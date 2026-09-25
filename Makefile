# Everything this Makefile builds goes under $(BUILD_DIR), which git ignores.

BUILD_DIR ?= build
FQBN      ?= arduino:avr:mega
PORT      ?= /dev/ttyACM0
CXX       ?= c++
PYTHON    ?= python3
CHROMIUM  ?= chromium

# ADK is C++23. The Arduino core's own compiler (GCC 7) stops at C++17, so
# examples build with a newer avr-gcc, fetched once into $(BUILD_DIR) and
# checked against its published SHA-256.
AVR_GCC         := avr-gcc-16.1.0-x64-linux
AVR_GCC_URL     := https://github.com/ZakKemble/avr-gcc-build/releases/download/v16.1.0-1/$(AVR_GCC).tar.bz2
AVR_GCC_SHA256  := 8621ecc6514df50202b58b23b0f8f72f0e535ec35ee40426194e9a15c57692f6
TOOLCHAIN       := $(BUILD_DIR)/toolchain/$(AVR_GCC)
AVR_PROPERTIES  := --build-property "compiler.path=$(abspath $(TOOLCHAIN))/bin/" \
                   --build-property "compiler.cpp.extra_flags=-std=gnu++23"

LIBRARY_SOURCES := $(wildcard src/adk/*.cpp)
TEST_SOURCES    := $(wildcard tests/*.cpp tests/arduino/*.cpp)
EXAMPLES        := $(patsubst examples/%/,%,$(sort $(dir $(wildcard examples/*/*.ino))))

HOST_DIR   := $(BUILD_DIR)/host
HOST_FLAGS := -std=c++23 -g -Os -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror \
              -fno-exceptions -fno-rtti -Isrc -Itests/arduino -Itests
HOST_OBJECTS := $(patsubst %.cpp,$(HOST_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))

SANITIZE_DIR     := $(BUILD_DIR)/sanitize
SANITIZE_FLAGS   := $(HOST_FLAGS) -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
SANITIZE_OBJECTS := $(patsubst %.cpp,$(SANITIZE_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))

ARDUINO_DIR   := $(BUILD_DIR)/arduino
ARDUINO_CACHE := $(BUILD_DIR)/arduino-cache
ARDUINO_LOGS  := $(patsubst %,$(ARDUINO_DIR)/%.log,$(EXAMPLES))
PIN_CHECKS    := $(patsubst %,$(ARDUINO_DIR)/%/pins.ok,$(EXAMPLES))

# A sketch's setup () runs on the host against the fake core, to list the
# pins it claims; warnings are the AVR build's business.
PROBE_FLAGS   := -std=gnu++23 -w -fno-exceptions -fno-rtti -Isrc -Itests/arduino -Itests
PROBE_OBJECTS := $(filter $(HOST_DIR)/obj/src/% $(HOST_DIR)/obj/tests/arduino/% \
                          $(HOST_DIR)/obj/tests/fake_%,$(HOST_OBJECTS))
LIBRARY_FILES := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp) library.properties

VENV := $(BUILD_DIR)/venv
export PYTHONPYCACHEPREFIX := $(abspath $(BUILD_DIR))/pycache

# The pinned MkDocs 1.6 is what the site is built with; skip Material's
# notice about the incompatible MkDocs 2.0.
export NO_MKDOCS_2_WARNING := 1

.DEFAULT_GOAL := test
.SECONDEXPANSION:
.PHONY: all check test sanitize toolchain examples pins size site pdf serve style upload monitor \
        clean help

all: check

## check         everything CI runs: tests, examples, sizes, the site and its PDFs
check: style test sanitize examples pins size pdf

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

## toolchain     fetch the C++23 avr-gcc the examples build with
toolchain: $(TOOLCHAIN)/bin/avr-g++

$(TOOLCHAIN)/bin/avr-g++:
	@mkdir -p $(BUILD_DIR)/toolchain
	curl --fail --location --silent --show-error --output $(TOOLCHAIN).tar.bz2 $(AVR_GCC_URL)
	echo "$(AVR_GCC_SHA256)  $(TOOLCHAIN).tar.bz2" | sha256sum --check --quiet
	tar -xjf $(TOOLCHAIN).tar.bz2 -C $(BUILD_DIR)/toolchain
	@rm $(TOOLCHAIN).tar.bz2
	@touch $@

## examples      compile every example for the Mega 2560
examples: $(ARDUINO_LOGS)

$(ARDUINO_DIR)/%.log: examples/$$*/$$*.ino $(LIBRARY_FILES) | $(TOOLCHAIN)/bin/avr-g++
	@mkdir -p $(ARDUINO_DIR)/$* $(ARDUINO_CACHE)
	@echo "  AVR  examples/$*"
	@ARDUINO_BUILD_CACHE_PATH=$(abspath $(ARDUINO_CACHE)) \
	    arduino-cli compile --fqbn $(FQBN) --library . --warnings all $(AVR_PROPERTIES) \
	    --build-path $(ARDUINO_DIR)/$* examples/$* > $@.tmp 2>&1 \
	    || (cat $@.tmp; rm -f $@.tmp; exit 1)
	@if grep -A3 -E '(src/adk|examples)/.*warning' $@.tmp; then rm -f $@.tmp; exit 1; fi
	@mv $@.tmp $@

## pins          check each example claims exactly the pins its lesson's circuit wires
pins: $(PIN_CHECKS)

$(ARDUINO_DIR)/%/pins.ok: $(ARDUINO_DIR)/%.log $(PROBE_OBJECTS) tests/probe/pins.cpp tests/pins.py \
                          $$(wildcard docs/lessons/$$(shell echo $$* | cut -c7-8)-*/circuit.py) \
                          $(wildcard docs/_theme/*.py)
	@echo "  PINS examples/$*"
	@$(CXX) $(PROBE_FLAGS) $(ARDUINO_DIR)/$*/sketch/$*.ino.cpp tests/probe/pins.cpp \
	    $(PROBE_OBJECTS) -o $(ARDUINO_DIR)/$*/probe
	@$(ARDUINO_DIR)/$*/probe > $(ARDUINO_DIR)/$*/pins.txt
	@$(PYTHON) tests/pins.py $* $(ARDUINO_DIR)/$*/pins.txt
	@touch $@

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

## pdf           print every lesson page to build/site/pdf
pdf: site
	@mkdir -p $(BUILD_DIR)/site/pdf
	@for page in $(BUILD_DIR)/site/lessons/*/index.html; do \
	    lesson=$$(basename $$(dirname $$page)); \
	    echo "  PDF  $$lesson"; \
	    $(CHROMIUM) --headless=new --no-sandbox --disable-gpu --no-pdf-header-footer \
	        --virtual-time-budget=10000 --run-all-compositor-stages-before-draw \
	        --print-to-pdf=$$PWD/$(BUILD_DIR)/site/pdf/$$lesson.pdf \
	        file://$$PWD/$$page 2>/dev/null || exit 1; \
	done

## serve         preview the website at http://127.0.0.1:8000
serve: $(VENV)/.installed
	$(VENV)/bin/mkdocs serve

$(VENV)/.installed: docs/requirements.txt
	$(PYTHON) -m venv $(VENV)
	$(VENV)/bin/pip install --quiet --disable-pip-version-check -r $<
	@touch $@

STYLED := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp tests/*.h tests/*.cpp tests/arduino/* \
                    tests/probe/*.cpp) \
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
