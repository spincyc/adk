SIZE                 ?= size
ARDUINO_LINT         ?= arduino-lint
QUALITY_HOST_MAX     ?= 32768
QUALITY_ARCH_PACKAGES = base-devel clang

.PHONY: quality quality-fast quality-lint quality-test quality-size \
	quality-tools quality-packages host-size-check firmware-size-check \
	host-test-sanitize arduino-lint

quality: quality-fast firmware-size-check package-smoke lessons-check site-check

quality-fast: quality-tools quality-lint quality-test quality-size

quality-lint: style-check

quality-test: host-test host-test-exceptions host-test-sanitize \
	usb-matrix-check usb-mesh-check hdmi-mesh-check route-profile-check

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

host-test-sanitize:
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
	$(MAKE) --no-print-directory host-test \
		BUILD_DIR="$(BUILD_DIR)/sanitize" \
		HOST_CXXFLAGS="$(HOST_CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer" \
		HOST_LDFLAGS="$(HOST_LDFLAGS) -fsanitize=address,undefined"

firmware-size-check: size-check
	@echo "Firmware satisfies the recorded $(BOARD_FQBN) budgets."

arduino-lint:
	@command -v "$(ARDUINO_LINT)" >/dev/null || { \
		echo "arduino-lint is optional and is not in Arch official repositories." >&2; \
		exit 2; \
	}
	$(ARDUINO_LINT) --compliance strict .
