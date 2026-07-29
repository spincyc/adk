.PHONY: ir-resource-check

ir-resource-check: arduino-check
	python3 scripts/check_ir_resource_probe.py \
		--arduino-cli "$(ARDUINO_CLI)" \
		--fqbn "$(BOARD_FQBN)"
