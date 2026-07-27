SIZE                 ?= size
ARDUINO_LINT         ?= arduino-lint
ARDUINO_LINT_VERSION ?= 1.3.0
ARDUINO_LINT_RELEASE_MODE ?= submit
QUALITY_HOST_MAX     ?= 32768
QUALITY_ARCH_PACKAGES = base-devel clang

.PHONY: quality quality-fast quality-lint quality-test quality-size \
	quality-tools quality-packages host-size-check firmware-size-check \
	host-test-sanitize serial-log-test deployed-site-test arduino-lint \
	arduino-lint-submit arduino-lint-update arduino-lint-release \
	release-metadata-check release-check

quality: quality-fast firmware-size-check package-smoke native-package-smoke \
	lessons-check site-check

quality-fast: quality-tools quality-lint quality-test quality-size

quality-lint: style-check headers-check

quality-test: host-test host-test-exceptions host-test-sanitize \
	serial-log-test deployed-site-test usb-matrix-check usb-mesh-check hdmi-mesh-check \
	route-profile-check

serial-log-test:
	python -m unittest tests/test_serial_log.py

deployed-site-test:
	python -m unittest tests/test_deployed_site.py

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
		echo "arduino-lint is required for a release and is not in Arch official repositories." >&2; \
		exit 2; \
	}
	@"$(ARDUINO_LINT)" --version | grep -F "$(ARDUINO_LINT_VERSION)" >/dev/null || { \
		echo "arduino-lint $(ARDUINO_LINT_VERSION) is required." >&2; \
		exit 2; \
	}
	$(ARDUINO_LINT) --compliance strict --project-type library --recursive .

arduino-lint-submit: arduino-lint
	$(ARDUINO_LINT) --compliance strict --library-manager submit \
		--project-type library --recursive .

arduino-lint-update: arduino-lint
	$(ARDUINO_LINT) --compliance strict --library-manager update \
		--project-type library --recursive .

arduino-lint-release:
	@case "$(ARDUINO_LINT_RELEASE_MODE)" in \
		submit) $(MAKE) --no-print-directory arduino-lint-submit ;; \
		update) $(MAKE) --no-print-directory arduino-lint-update ;; \
		*) echo "ARDUINO_LINT_RELEASE_MODE must be submit or update." >&2; exit 2 ;; \
	esac

release-metadata-check:
	python3 scripts/check_release.py --ref "$(PACKAGE_REF)"

release-check: release-metadata-check
	$(MAKE) --no-print-directory clean
	$(MAKE) --no-print-directory quality PACKAGE_REF="$(PACKAGE_REF)"
	$(MAKE) --no-print-directory arduino-lint-release
	python3 scripts/check_release.py --ref "$(PACKAGE_REF)"
