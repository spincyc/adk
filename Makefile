# ADK: the library, its host tests, the examples, the pins check, the website
# and the board package. Everything this Makefile builds goes under
# $(BUILD_DIR), which git ignores; make help lists the targets.

# Settings ---------------------------------------------------------------------

BUILD_DIR    ?= build
FQBN         ?= arduino:avr:mega
PORT         ?= /dev/ttyACM0
CXX          ?= c++
PYTHON       ?= python3
CHROMIUM     ?= chromium
PDF_WORKERS  ?= 2
PDF_MAX_MB   ?= 8
SITE_MAX_MB  ?= 500
ARDUINO_CORE ?= arduino:avr@1.8.8

# The C++23 avr-gcc ------------------------------------------------------------
#
# The Arduino core's own compiler (GCC 7) stops at C++17, so the examples
# build with the avr-gcc ADK Boards installs: boards/toolchain.json lists its
# download and SHA-256 for each computer. make toolchain fetches it into
# $(BUILD_DIR) on Linux and macOS, x86-64 or arm64; anywhere else, install
# avr-gcc 16 yourself and set TOOLCHAIN to its folder.

HOST             := $(shell uname -s)-$(shell uname -m)
TOOLCHAIN_HOSTS  := Linux-x86_64:x86_64-linux-gnu      \
                    Linux-aarch64:aarch64-linux-gnu    \
                    Darwin-x86_64:x86_64-apple-darwin  \
                    Darwin-arm64:arm64-apple-darwin
TOOLCHAIN_HOST   := $(patsubst $(HOST):%,%,$(filter $(HOST):%,$(TOOLCHAIN_HOSTS)))
TOOLCHAIN_ENTRY  := import json, sys;                                             \
                    print (*next ((system["url"],                                 \
                                   system["checksum"].removeprefix ("SHA-256:"))  \
                                  for system in json.load (sys.stdin)["systems"]  \
                                  if system["host"] == sys.argv[1]))
AVR_GCC_DOWNLOAD := $(if $(TOOLCHAIN_HOST),$(shell $(PYTHON) -c '$(TOOLCHAIN_ENTRY)'  \
                        $(TOOLCHAIN_HOST) < boards/toolchain.json))
AVR_GCC_URL      := $(word 1,$(AVR_GCC_DOWNLOAD))
AVR_GCC_SHA256   := $(word 2,$(AVR_GCC_DOWNLOAD))
AVR_GCC          := $(basename $(basename $(notdir $(AVR_GCC_URL))))
TOOLCHAIN        ?= $(BUILD_DIR)/toolchain/$(AVR_GCC)
SHA256SUM        := $(if $(shell command -v sha256sum),sha256sum,shasum -a 256)
AVR_PROPERTIES   := --build-property "compiler.path=$(abspath $(TOOLCHAIN))/bin/"  \
                    --build-property "compiler.cpp.extra_flags=-std=gnu++23"

# Sources and examples ---------------------------------------------------------
#
# One sketch per lesson, examples/lessons/001-blink/001-blink.ino, or one per
# board in a two-board lesson, examples/lessons/044-remote-dial/Dial/Dial.ino.
# Each example builds in a folder of its own, build/arduino/<example>.

LIBRARY_SOURCES  := $(wildcard src/adk/*.cpp)
LIBRARY_FILES    := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp)  \
                    library.properties
TEST_SOURCES     := $(wildcard tests/*.cpp tests/arduino/*.cpp)
EXAMPLE_TESTS     := $(wildcard tests/examples/*.cpp)
SKETCHES         := $(wildcard examples/lessons/*/*.ino examples/lessons/*/*/*.ino)
EXAMPLES         := $(patsubst examples/%/,%,$(sort $(dir $(SKETCHES))))
STYLED           := $(wildcard src/*.h src/adk/*.h src/adk/*.cpp)  \
                    $(wildcard tests/*.h tests/*.cpp)              \
                    $(wildcard tests/arduino/* tests/probe/*.cpp)  \
                    $(wildcard tests/examples/*.h)                 \
                    $(wildcard tests/avr/*/*.ino)                  \
                    $(EXAMPLE_TESTS)                               \
                    $(SKETCHES)                                    \
                    Makefile

# Host tests -------------------------------------------------------------------

HOST_DIR         := $(BUILD_DIR)/host
HOST_FLAGS       := -std=c++23       \
                    -g               \
                    -Os              \
                    -Wall            \
                    -Wextra          \
                    -Wpedantic       \
                    -Wconversion     \
                    -Wshadow         \
                    -Werror          \
                    -fno-exceptions  \
                    -fno-rtti        \
                    -Isrc            \
                    -Itests/arduino  \
                    -Itests
HOST_OBJECTS     := $(patsubst %.cpp,$(HOST_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))
HOST_EXAMPLES    := $(patsubst tests/examples/%.cpp,$(HOST_DIR)/examples/%,$(EXAMPLE_TESTS))

# UBSan only reports and carries on unless it is told not to recover.
SANITIZE_DIR     := $(BUILD_DIR)/sanitize
SANITIZE_FLAGS   := $(HOST_FLAGS)                 \
                    -O1                           \
                    -fsanitize=address,undefined  \
                    -fno-sanitize-recover=all     \
                    -fno-omit-frame-pointer
SANITIZE_OBJECTS := $(patsubst %.cpp,$(SANITIZE_DIR)/obj/%.o,$(LIBRARY_SOURCES) $(TEST_SOURCES))
SANITIZE_CASES   := $(patsubst tests/examples/%.cpp,$(SANITIZE_DIR)/examples/%,$(EXAMPLE_TESTS))

# The Mega's builds, the pins check and the smoke test -------------------------
#
# A warning from the library or an example fails its build; arduino-cli
# names each file by its full path. The pins check runs a sketch's setup ()
# on the host against the fake core, to list the pins it claims, and the
# smoke test runs its setup () and loop () there under the sanitizers. Both
# take the sketch as Arduino preprocessed it for the Mega, so their
# warnings are the AVR build's business.

ARDUINO_DIR      := $(BUILD_DIR)/arduino
ARDUINO_CACHE    := $(BUILD_DIR)/arduino-cache
ARDUINO_LOGS     := $(patsubst %,$(ARDUINO_DIR)/%.log,$(EXAMPLES))
PIN_CHECKS       := $(patsubst %,$(ARDUINO_DIR)/%/pins.ok,$(EXAMPLES))
SMOKE_CHECKS     := $(patsubst %,$(SANITIZE_DIR)/smoke/%.ok,$(EXAMPLES))
WARNINGS         := '^$(CURDIR)/(src|examples)/[^:]+:[0-9]+:[0-9]+: warning'
PROBE_FLAGS      := -std=gnu++23     \
                    -w               \
                    -fno-exceptions  \
                    -fno-rtti        \
                    -Isrc            \
                    -Itests/arduino  \
                    -Itests
PROBE_OBJECTS    := $(filter $(HOST_DIR)/obj/src/%            \
                             $(HOST_DIR)/obj/tests/arduino/%  \
                             $(HOST_DIR)/obj/tests/fake_%,    \
                             $(HOST_OBJECTS))
SANITIZE_PROBES  := $(patsubst $(HOST_DIR)/%,$(SANITIZE_DIR)/%,$(PROBE_OBJECTS))
SANITIZE_PROBE_FLAGS := $(PROBE_FLAGS)                  \
                        -g                              \
                        -O1                             \
                        -fsanitize=address,undefined    \
                        -fno-sanitize-recover=all       \
                        -fno-omit-frame-pointer

# The AVR's own widths ---------------------------------------------------------
#
# The host's int is 32 bits, but the Mega's is 16, its long 32 and its double
# no wider than float. tests/avr/widths.cpp runs a few pure-logic tests on the
# AVR itself, built as an example is, in the simulator that comes with
# avr-gdb. The sketch stops at a break instruction, or after 10^8 cycles if
# it never gets there, and avr-gdb reads its report back from its RAM.

AVR_TEST_DIR     := $(BUILD_DIR)/avr-test
AVR_SIMULATOR    := $(TOOLCHAIN)/bin/avr-gdb -q -batch -nx                \
                    -ex 'target sim --watch-cycles-breakpoint 100000000'  \
                    -ex load                                              \
                    -ex run                                               \
                    -ex 'printf "%s\n", adkReport'

# Each lesson's own targets ----------------------------------------------------
#
# Every lesson's folder has the same name in docs/lessons and in
# examples/lessons, and targets of that name: make 001-blink compiles its
# sketch, and make upload-001-blink sends it to the Mega on PORT. In a
# two-board lesson make 044-remote-dial compiles both sketches, and each
# board's sketch has a pair of its own, named by the sketch in lower case:
# make 044-remote-dial-servo and make upload-044-remote-dial-servo. Each
# target is name:example in LESSON_TARGETS, which make lessons lists.

LESSONS          := $(patsubst docs/lessons/%/,%,$(wildcard docs/lessons/[0-9][0-9][0-9]-*/))
board_of          = $(word 3,$(subst /, ,$1))
BOARD_SKETCHES   := $(foreach example,$(EXAMPLES),$(if $(call board_of,$(example)),$(example)))
BOARD_NAMES      := $(join $(addsuffix :,$(BOARD_SKETCHES)),  \
                           $(shell echo $(notdir $(BOARD_SKETCHES)) | tr A-Z a-z))
sketches_of       = $(filter lessons/$1 lessons/$1/%,$(EXAMPLES))
two_boards        = $(filter lessons/$1/%,$(EXAMPLES))
board_name        = $(patsubst $1:%,%,$(filter $1:%,$(BOARD_NAMES)))
target_of         = $1$(if $(call board_of,$2),-$(call board_name,$2))
circuit_of        = $(wildcard docs/lessons/$(word 2,$(subst /, ,$1))/circuit.py)
lesson_lines      = $(if $(call two_boards,$1),$1:lessons/$1/*)  \
                    $(filter $1:% $1-%,$(LESSON_TARGETS))
LESSON_TARGETS   := $(foreach lesson,$(LESSONS),                         \
                        $(foreach sketch,$(call sketches_of,$(lesson)),  \
                            $(call target_of,$(lesson),$(sketch)):$(sketch)))
LESSON_LINES     := $(foreach lesson,$(LESSONS),$(call lesson_lines,$(lesson)))

# ADK Boards -------------------------------------------------------------------
#
# The Arduino IDE package the site publishes is installed into arduino-cli
# directories of its own, never the user's ~/.arduino15.

BOARDS_DIR       := $(BUILD_DIR)/boards
BOARDS_HOME      := $(abspath $(BOARDS_DIR))
BOARDS_INDEX     := file://$(abspath $(BUILD_DIR))/site/package_adk_index.json
BOARDS_EXAMPLES  := lessons/001-blink  \
                    lessons/006-simon
BOARDS_CLI       := ARDUINO_CONFIG_FILE=$(BOARDS_HOME)/arduino-cli.yaml     \
                    ARDUINO_DIRECTORIES_DATA=$(BOARDS_HOME)/data            \
                    ARDUINO_DIRECTORIES_DOWNLOADS=$(BOARDS_HOME)/downloads  \
                    ARDUINO_DIRECTORIES_USER=$(BOARDS_HOME)/user            \
                    ARDUINO_BUILD_CACHE_PATH=$(BOARDS_HOME)/cache           \
                    ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS=$(BOARDS_INDEX)   \
                    arduino-cli

# The website ------------------------------------------------------------------
#
# Python keeps its caches, and the drawing engine the wires it has routed and
# the drawings it has made, in the build directory, so a rebuild only draws
# what has changed. The pinned MkDocs 1.6 builds the site; skip Material's
# notice about MkDocs 2.0.

VENV                       := $(BUILD_DIR)/venv
export PYTHONPYCACHEPREFIX := $(abspath $(BUILD_DIR))/pycache
export ADK_DRAWINGS        := $(abspath $(BUILD_DIR))/drawings
export NO_MKDOCS_2_WARNING := 1

# What the build needs ---------------------------------------------------------
#
# make deps installs the system's packages, then the Arduino core the examples
# build against, the C++23 avr-gcc and the website's Python, each only if it
# is missing. Each system has a list of packages; so far make deps installs
# them only on Arch Linux (and its kin), and elsewhere says what to install.

SYSTEM           := $(shell if [ -r /etc/os-release ]; then . /etc/os-release;  \
                        echo $$ID $$ID_LIKE; else uname -s; fi)
DEPS_SYSTEM      := $(firstword $(filter arch debian fedora Darwin,$(SYSTEM)) other)
DEPS_arch        := base-devel   \
                    python       \
                    chromium     \
                    arduino-cli  \
                    curl         \
                    tar          \
                    bzip2        \
                    git
DEPS_debian      := build-essential  \
                    python3-venv     \
                    chromium         \
                    curl             \
                    bzip2            \
                    git
DEPS_fedora      := gcc-c++   \
                    make      \
                    python3   \
                    chromium  \
                    curl      \
                    bzip2     \
                    git
DEPS_Darwin      := python       \
                    arduino-cli  \
                    chromium
DEPS_other       := a C++ compiler, make, Python 3, Chromium, arduino-cli, curl, bzip2 and git
# Debian and Fedora have no package of arduino-cli; its own installer does.
CLI_HINT         := and arduino-cli, as its installation guide explains:  \
                    https://arduino.github.io/arduino-cli/latest/installation/

# Targets ----------------------------------------------------------------------

.DEFAULT_GOAL := test
.SECONDEXPANSION:
.PHONY: all          \
        check        \
        deps         \
        deps-arch    \
        deps-debian  \
        deps-fedora  \
        deps-Darwin  \
        deps-other   \
        test         \
        sanitize     \
        smoke        \
        avr-test     \
        toolchain    \
        examples     \
        lessons      \
        pins         \
        size         \
        site         \
        pdf          \
        boards       \
        serve        \
        style        \
        upload       \
        monitor      \
        clean        \
        help

all: check

## check           what CI runs on Linux: tests, examples, pins, sizes, the site, PDFs and boards
check: style     \
       test      \
       sanitize  \
       avr-test  \
       examples  \
       smoke     \
       pins      \
       size      \
       pdf       \
       boards

## deps            install what the build needs (on Arch Linux, its packages with sudo pacman)
deps: deps-$(DEPS_SYSTEM)
	@arduino-cli core list | grep -q '^$(subst @, *,$(ARDUINO_CORE)) '  \
	    || { arduino-cli core update-index && arduino-cli core install $(ARDUINO_CORE); }
	@$(MAKE) --no-print-directory toolchain $(VENV)/.installed

# pacman -T names the packages that are missing, if any.
deps-arch:
	@missing="$$(pacman -T $(DEPS_arch))";  \
	    if [ -n "$$missing" ]; then sudo pacman -S --needed $$missing; fi

deps-debian deps-fedora deps-Darwin deps-other:
	@echo "make deps doesn't install packages here yet. Install:"
	@echo "    $(DEPS_$(DEPS_SYSTEM))"
	$(if $(filter debian fedora,$(DEPS_SYSTEM)),@echo "$(CLI_HINT)")
	@echo "then run:"
	@echo "    arduino-cli core update-index"
	@echo "    arduino-cli core install $(ARDUINO_CORE)"
	@echo "    make toolchain $(VENV)/.installed"
	@exit 1

## test            build and run the host tests (TEST=name runs matching cases)
test: $(HOST_DIR)/tests $(HOST_EXAMPLES)
	$(HOST_DIR)/tests $(TEST)
	@set -e; for example in $(HOST_EXAMPLES); do "$$example"; done

$(HOST_DIR)/tests: $(HOST_OBJECTS)
	@$(CXX) $(HOST_FLAGS) $^ -o $@

$(HOST_DIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "  CXX  $<"
	@$(CXX) $(HOST_FLAGS) -MMD -MP -c $< -o $@

# Each regression includes a real sketch, with its own setup and loop and
# its own main. It shares the library and fake core, never the test runner.
$(HOST_DIR)/examples/%: tests/examples/%.cpp $(PROBE_OBJECTS)
	@mkdir -p $(@D)
	@echo "  TEST $<"
	@$(CXX) $(PROBE_FLAGS) -MMD -MP -MF $@.d $< $(PROBE_OBJECTS) -o $@

## sanitize        run the host tests under AddressSanitizer and UBSan
sanitize: $(SANITIZE_DIR)/tests $(SANITIZE_CASES)
	$(SANITIZE_DIR)/tests $(TEST)
	@set -e; for example in $(SANITIZE_CASES); do "$$example"; done

$(SANITIZE_DIR)/tests: $(SANITIZE_OBJECTS)
	@$(CXX) $(SANITIZE_FLAGS) $^ -o $@

$(SANITIZE_DIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "  CXX  $< (sanitized)"
	@$(CXX) $(SANITIZE_FLAGS) -MMD -MP -c $< -o $@

$(SANITIZE_DIR)/examples/%: tests/examples/%.cpp $(SANITIZE_PROBES)
	@mkdir -p $(@D)
	@echo "  TEST $< (sanitized)"
	@$(CXX) $(SANITIZE_PROBE_FLAGS) -MMD -MP -MF $@.d $< $(SANITIZE_PROBES) -o $@

## smoke           run every example's setup and loop on the host, under ASan and UBSan
#
# A few hundred passes of each loop () (tests/probe/smoke.cpp): a crash,
# undefined behavior or a hang fails. It follows each example's AVR build,
# so unlike sanitize it needs the toolchain.
smoke: $(SMOKE_CHECKS)

$(SANITIZE_DIR)/smoke/%.ok: $(ARDUINO_DIR)/%.log   \
                            $(SANITIZE_PROBES)     \
                            tests/probe/smoke.cpp
	@mkdir -p $(@D)
	@echo "  RUN  examples/$* (sanitized)"
	@$(CXX) $(SANITIZE_PROBE_FLAGS)                    \
	    $(ARDUINO_DIR)/$*/sketch/$(notdir $*).ino.cpp  \
	    tests/probe/smoke.cpp                          \
	    $(SANITIZE_PROBES)                             \
	    -o $(basename $@)
	@$(basename $@)
	@touch $@

-include $(HOST_OBJECTS:.o=.d) $(SANITIZE_OBJECTS:.o=.d)
-include $(HOST_EXAMPLES:%=%.d) $(SANITIZE_CASES:%=%.d)

## toolchain       fetch the C++23 avr-gcc the examples build with
toolchain: $(TOOLCHAIN)/bin/avr-g++

$(TOOLCHAIN)/bin/avr-g++:
	$(if $(AVR_GCC_URL),,$(error make toolchain fetches avr-gcc for Linux and macOS on  \
	    x86-64 or arm64, not $(HOST): install avr-gcc 16 and set TOOLCHAIN to its folder))
	@mkdir -p $(BUILD_DIR)/toolchain
	curl --fail --location --silent --show-error --output $(TOOLCHAIN).tar.bz2 $(AVR_GCC_URL)
	@sum=$$($(SHA256SUM) $(TOOLCHAIN).tar.bz2); sum=$${sum%% *};  \
	    [ "$$sum" = "$(AVR_GCC_SHA256)" ]                         \
	    || { echo "$(TOOLCHAIN).tar.bz2: SHA-256 $$sum, not $(AVR_GCC_SHA256)"; exit 1; }
	tar -xjf $(TOOLCHAIN).tar.bz2 -C $(BUILD_DIR)/toolchain
	@rm $(TOOLCHAIN).tar.bz2
	@touch $@

## examples        compile every example for the Mega 2560
examples: $(ARDUINO_LOGS)

# The toolchain is a prerequisite, so a new avr-gcc rebuilds every example.
# A failed build leaves no objects behind: arduino-cli would reuse them next
# time, print no warning, and pass.
$(ARDUINO_DIR)/%.log: examples/$$*/$$(notdir $$*).ino  \
                      $(LIBRARY_FILES)                 \
                      $(TOOLCHAIN)/bin/avr-g++
	@mkdir -p $(ARDUINO_DIR)/$* $(ARDUINO_CACHE)
	@echo "  AVR  examples/$*"
	@ARDUINO_BUILD_CACHE_PATH=$(abspath $(ARDUINO_CACHE))  \
	    arduino-cli compile                                \
	        --fqbn $(FQBN)                                 \
	        --library .                                    \
	        --warnings all                                 \
	        $(AVR_PROPERTIES)                              \
	        --build-path $(ARDUINO_DIR)/$*                 \
	        examples/$* > $@.tmp 2>&1                      \
	    || (cat $@.tmp; rm -rf $@.tmp $(ARDUINO_DIR)/$*; exit 1)
	@if grep -A3 -E $(WARNINGS) $@.tmp; then rm -rf $@.tmp $(ARDUINO_DIR)/$*; exit 1; fi
	@mv $@.tmp $@

## avr-test        run a few tests on a simulated Mega, where int is 16 bits
avr-test: $(AVR_TEST_DIR)/widths.ok

$(AVR_TEST_DIR)/widths.ok: tests/avr/widths.cpp  \
                           $(LIBRARY_FILES)      \
                           $(TOOLCHAIN)/bin/avr-g++
	@mkdir -p $(AVR_TEST_DIR)/sketch/widths $(ARDUINO_CACHE)
	@cp tests/avr/widths.cpp $(AVR_TEST_DIR)/sketch/widths/widths.ino
	@echo "  AVR  tests/avr/widths.cpp"
	@ARDUINO_BUILD_CACHE_PATH=$(abspath $(ARDUINO_CACHE))           \
	    arduino-cli compile                                         \
	        --fqbn $(FQBN)                                          \
	        --library .                                             \
	        --warnings all                                          \
	        $(AVR_PROPERTIES)                                       \
	        --build-path $(AVR_TEST_DIR)/widths                     \
	        $(AVR_TEST_DIR)/sketch/widths                           \
	        > $(AVR_TEST_DIR)/widths.log 2>&1                       \
	    || (cat $(AVR_TEST_DIR)/widths.log; exit 1)
	@echo "  SIM  tests/avr/widths.cpp"
	@$(AVR_SIMULATOR) $(AVR_TEST_DIR)/widths/widths.ino.elf > $(AVR_TEST_DIR)/widths.out 2>&1  \
	    || true
	@grep '^avr: ' $(AVR_TEST_DIR)/widths.out || cat $(AVR_TEST_DIR)/widths.out
	@grep -q '^avr: [0-9]* checks, 0 failures$$' $(AVR_TEST_DIR)/widths.out
	@touch $@

## NNN-name        compile one lesson's sketches, as in make 001-blink
## upload-NNN-name compile a lesson's sketch and upload it to the Mega on PORT
## lessons         list every lesson's targets and the examples each builds
lessons:
	@$(foreach line,$(LESSON_LINES),printf '%-32s examples/%s\n' $(subst :, ,$(line));)

# make <name> compiles one example, and make upload-<name> uploads it; $1
# is the name and the example, as in "001-blink lessons/001-blink".
define example_targets
.PHONY: $(word 1,$1) upload-$(word 1,$1)
$(word 1,$1): $(ARDUINO_DIR)/$(word 2,$1).log
upload-$(word 1,$1): $(ARDUINO_DIR)/$(word 2,$1).log
	arduino-cli upload --fqbn $$(FQBN) --port $$(PORT) --input-dir $(ARDUINO_DIR)/$(word 2,$1)
endef

# A two-board lesson's own target compiles both its boards' sketches.
define lesson_target
$(if $(call two_boards,$1),.PHONY: $1
$1: $(patsubst %,$(ARDUINO_DIR)/%.log,$(call sketches_of,$1)))
endef

$(foreach target,$(LESSON_TARGETS),$(eval $(call example_targets,$(subst :, ,$(target)))))
$(foreach lesson,$(LESSONS),$(eval $(call lesson_target,$(lesson))))

## pins            test the circuit model, then hold each example to its lesson's circuit
pins: $(BUILD_DIR)/circuits.ok $(PIN_CHECKS)

$(BUILD_DIR)/circuits.ok: tests/circuits.py $(wildcard docs/_theme/*.py)
	@mkdir -p $(@D)
	@echo "  PY   tests/circuits.py"
	@$(PYTHON) tests/circuits.py
	@touch $@

$(ARDUINO_DIR)/%/pins.ok: $(ARDUINO_DIR)/%.log     \
                          $(PROBE_OBJECTS)         \
                          tests/probe/pins.cpp     \
                          tests/pins.py            \
                          $$(call circuit_of,$$*)  \
                          $(wildcard docs/_theme/*.py)
	@echo "  PINS examples/$*"
	@$(CXX) $(PROBE_FLAGS)                             \
	    $(ARDUINO_DIR)/$*/sketch/$(notdir $*).ino.cpp  \
	    tests/probe/pins.cpp                           \
	    $(PROBE_OBJECTS)                               \
	    -o $(ARDUINO_DIR)/$*/probe
	@$(ARDUINO_DIR)/$*/probe > $(ARDUINO_DIR)/$*/pins.txt
	@$(PYTHON) tests/pins.py $* $(ARDUINO_DIR)/$*/pins.txt
	@touch $@

## size            print the flash and RAM each example uses
size: $(ARDUINO_LOGS)
	@printf '%-36s %8s %6s\n' Example Flash RAM
	@for example in $(EXAMPLES); do                                              \
	    log=$(ARDUINO_DIR)/$$example.log;                                        \
	    flash=$$(sed -n 's/^Sketch uses \([0-9]*\) bytes.*/\1/p' $$log);         \
	    ram=$$(sed -n 's/^Global variables use \([0-9]*\) bytes.*/\1/p' $$log);  \
	    printf '%-36s %8s %6s\n' $$example $$flash $$ram;                        \
	done

## site            build the website into build/site, and check its links
site: $(VENV)/.installed $(BUILD_DIR)/steps.ok
	$(VENV)/bin/mkdocs build --strict --site-dir $(abspath $(BUILD_DIR))/site
	@$(PYTHON) tests/site_links.py $(BUILD_DIR)/site

$(BUILD_DIR)/steps.ok: tests/build_steps.py tests/navigation_ids.py  \
                     $(wildcard docs/_theme/*.py)                    \
                     $(wildcard docs/lessons/*/index.md) $(VENV)/.installed
	$(VENV)/bin/python tests/build_steps.py
	$(VENV)/bin/python tests/navigation_ids.py
	@touch $@

## pdf             print every lesson page to build/site/pdf
#
# Each PDF must be under PDF_MAX_MB and the site under SITE_MAX_MB; GitHub
# Pages publishes at most 1 GB. With poppler's pdftoppm and pdftotext, this
# also lists pages less than half full, for someone to look at.
pdf: site $(BUILD_DIR)/view.ok $(BUILD_DIR)/print.ok
	@$(VENV)/bin/python docs/_theme/print_pdfs.py   \
	    --site-dir "$(abspath $(BUILD_DIR))/site"   \
	    --chromium "$(CHROMIUM)"                    \
	    --workers "$(PDF_WORKERS)"                  \
	    --max-pdf-mb "$(PDF_MAX_MB)"                \
	    --max-site-mb "$(SITE_MAX_MB)"
	@if command -v pdftoppm > /dev/null && command -v pdftotext > /dev/null; then  \
	    $(PYTHON) tests/pdf_pages.py $(BUILD_DIR)/site/pdf;                        \
	fi

$(BUILD_DIR)/view.ok: tests/build_view.html docs/assets/steps.js docs/assets/adk.css
	@mkdir -p $(BUILD_DIR)
	@echo "  VIEW tests/build_view.html"
	@$(CHROMIUM) --headless=new --no-sandbox --disable-gpu                 \
	    --window-size=1280,900                                             \
	    --user-data-dir=$(abspath $(BUILD_DIR))/chromium-view-test         \
	    --dump-dom file://$(abspath tests/build_view.html)                 \
	    > $(BUILD_DIR)/view-test.html 2> $(BUILD_DIR)/view-test.log
	@grep -q 'data-test-result="pass"' $(BUILD_DIR)/view-test.html || \
	    { cat $(BUILD_DIR)/view-test.html; exit 1; }
	@touch $@

$(BUILD_DIR)/print.ok: tests/print_view.html docs/assets/print.js docs/assets/adk.css
	@mkdir -p $(BUILD_DIR)
	@echo "  VIEW tests/print_view.html"
	@$(CHROMIUM) --headless=new --no-sandbox --disable-gpu                 \
	    --user-data-dir=$(abspath $(BUILD_DIR))/chromium-print-test        \
	    --dump-dom file://$(abspath tests/print_view.html)                 \
	    > $(BUILD_DIR)/print-test.html 2> $(BUILD_DIR)/print-test.log
	@grep -q 'data-test-result="pass"' $(BUILD_DIR)/print-test.html || \
	    { cat $(BUILD_DIR)/print-test.html; exit 1; }
	@touch $@

## boards          install the site's ADK Boards package and compile lessons with it
#
# Boards Manager takes an archive already in its downloads folder when its
# checksum and size match the index, so this installs the platform the site
# just built while the index keeps its published URLs. Adk.h refuses any
# compiler before C++23, so a lesson that compiles was built by ADK's
# avr-gcc; and a rehearsed bootloader burn must match the stock Mega's.
boards: site
	@rm -rf $(BOARDS_DIR)/data $(BOARDS_DIR)/sketches
	@mkdir -p $(BOARDS_DIR)/downloads/packages $(BOARDS_DIR)/sketches
	@cp $(BUILD_DIR)/site/adk-avr-*.tar.bz2 $(BOARDS_DIR)/downloads/packages/
	@echo "  CLI  core install $(ARDUINO_CORE) adk:avr"
	@$(BOARDS_CLI) core update-index
	@$(BOARDS_CLI) core install $(ARDUINO_CORE) adk:avr
	@for example in $(BOARDS_EXAMPLES); do                  \
	    echo "  AVR  examples/$$example (adk:avr:mega)";    \
	    log=$(BOARDS_DIR)/sketches/$$example.log;           \
	    mkdir -p $(BOARDS_DIR)/sketches/$$example;          \
	    $(BOARDS_CLI) compile                               \
	        --fqbn adk:avr:mega                             \
	        --library .                                     \
	        --warnings all                                  \
	        --build-path $(BOARDS_DIR)/sketches/$$example   \
	        examples/$$example > $$log 2>&1                 \
	        || { cat $$log; exit 1; };                      \
	    if grep -A3 -E $(WARNINGS) $$log; then exit 1; fi;  \
	done
	@echo "  CLI  burn-bootloader --dry-run, stock Mega and ADK Mega"
	@for vendor in arduino adk; do                             \
	    $(BOARDS_CLI) burn-bootloader                          \
	        --dry-run                                          \
	        --verbose                                          \
	        --fqbn $$vendor:avr:mega                           \
	        --programmer avrispmkii                            \
	        > $(BOARDS_DIR)/sketches/$$vendor.burn || exit 1;  \
	done
	@grep -q stk500boot_v2_mega2560.hex $(BOARDS_DIR)/sketches/adk.burn
	@diff $(BOARDS_DIR)/sketches/arduino.burn $(BOARDS_DIR)/sketches/adk.burn

## serve           preview the website at http://127.0.0.1:8000
serve: $(VENV)/.installed
	$(VENV)/bin/mkdocs serve

$(VENV)/.installed: docs/requirements.txt
	$(PYTHON) -m venv --clear $(VENV)
	$(VENV)/bin/pip install --quiet --disable-pip-version-check --require-hashes -r $<
	@touch $@

## style           check the mechanical rules of docs/STYLE.md
style:
	@$(PYTHON) tests/style.py $(STYLED)

## upload          upload an example by its path: make upload EXAMPLE=lessons/001-blink
upload: $(ARDUINO_DIR)/$(EXAMPLE).log
	arduino-cli upload --fqbn $(FQBN) --port $(PORT) --input-dir $(ARDUINO_DIR)/$(EXAMPLE)

## monitor         open the serial monitor at 9600 baud
monitor:
	arduino-cli monitor --port $(PORT) --config baudrate=9600

## clean           remove everything this Makefile built
clean:
	rm -rf $(BUILD_DIR)

## help            list these targets
help:
	@sed -n 's/^## //p' $(MAKEFILE_LIST)
