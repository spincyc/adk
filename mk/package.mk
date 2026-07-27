PACKAGE_REF ?= HEAD

.PHONY: package-smoke
package-smoke: arduino-check
	ARDUINO_CLI="$(ARDUINO_CLI)" \
	BOARD_FQBN="$(BOARD_FQBN)" \
	PACKAGE_REF="$(PACKAGE_REF)" \
	sh scripts/package_smoke.sh

