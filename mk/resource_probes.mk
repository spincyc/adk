.PHONY: display-timing-resource-check escape-console-resource-check ir-resource-check museum-case-resource-check

ir-resource-check: arduino-check
	python3 scripts/check_ir_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)"

escape-console-resource-check: arduino-check
	python3 scripts/check_escape_console_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-complete

display-timing-resource-check: arduino-check
	python3 scripts/check_display_timing_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-through 060

museum-case-resource-check: arduino-check
	python3 scripts/check_museum_case_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-through 062
