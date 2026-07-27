SIZE                 ?= size
ARDUINO_LINT         ?= arduino-lint
QUALITY_HOST_MAX     ?= 32768
QUALITY_ARCH_PACKAGES = base-devel clang

.PHONY: quality quality-fast quality-lint quality-test quality-size \
	quality-tools quality-packages host-size-check firmware-size-check \
	arduino-lint

quality: quality-fast firmware-size-check lessons-check site-check

quality-fast: quality-tools quality-lint quality-test quality-size

quality-lint: style-check

quality-test: host-test host-test-exceptions

quality-size: host-size-check

quality-tools:
	@command -v "$(CXX)" >/dev/null || { echo "$(CXX) is required." >&2; exit 1; }
	@command -v "$(SIZE)" >/dev/null || { echo "$(SIZE) is required." >&2; exit 1; }

quality-packages:
	sudo pacman -Syu --needed $(QUALITY_ARCH_PACKAGES)

host-size-check: host-test
	@for image in $(HOST_TESTS); do \
		used="$$("$(SIZE)" "$$image" | awk 'NR == 2 { print $$1 + $$2 }')"; \
		test -n "$$used" || { echo "Unable to measure $$image." >&2; exit 1; }; \
		test "$$used" -le "$(QUALITY_HOST_MAX)" || { \
			echo "$$image: $$used exceeds $(QUALITY_HOST_MAX)-byte budget." >&2; \
			exit 1; \
		}; \
		echo "$$image: $$used / $(QUALITY_HOST_MAX) bytes."; \
	done

firmware-size-check: arduino
	@echo "Firmware fits the $(BOARD_FQBN) board limits."

arduino-lint:
	@command -v "$(ARDUINO_LINT)" >/dev/null || { \
		echo "arduino-lint is optional and is not in Arch official repositories." >&2; \
		exit 2; \
	}
	$(ARDUINO_LINT) --compliance strict .
