.PHONY: display-timing-resource-check escape-console-resource-check \
	ir-resource-check museum-case-resource-check \
	module-characterization-resource-check motion-recorder-resource-check \
	thermal-gradient-resource-check

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
		--require-through 063

thermal-gradient-resource-check: arduino-check
	python3 scripts/check_thermal_gradient_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-through 066

motion-recorder-resource-check: arduino-check
	python3 scripts/check_motion_recorder_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-through 069

module-characterization-resource-check: arduino-check
	python3 scripts/check_module_characterization_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)" \
		--require-through 072
