.PHONY: escape-console-resource-check ir-resource-check

ir-resource-check: arduino-check
	python3 scripts/check_ir_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)"

escape-console-resource-check: arduino-check
	python3 scripts/check_escape_console_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-complete
