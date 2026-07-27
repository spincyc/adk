.PHONY: arduino arduino-check $(addprefix arduino-,$(EXAMPLES))
arduino: $(addprefix arduino-,$(EXAMPLES))

arduino-check:
	@command -v "$(ARDUINO_CLI)" >/dev/null || \
		{ echo "arduino-cli is required for firmware builds."; exit 1; }

$(addprefix arduino-,$(EXAMPLES)): arduino-%: arduino-check | $(BUILD_MARKER)
	$(ARDUINO_CLI) compile \
		--fqbn "$(BOARD_FQBN)" \
		--library . \
		--build-path "$(BUILD_DIR)/arduino/$*" \
		"examples/$*"

.PHONY: upload
upload: arduino-check | $(BUILD_MARKER)
	@test -n "$(EXAMPLE)" || \
		{ echo "Set EXAMPLE=Lesson001DigitalOutput."; exit 1; }
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	$(ARDUINO_CLI) compile --upload \
		--fqbn "$(BOARD_FQBN)" \
		--port "$(PORT)" \
		--library . \
		--build-path "$(BUILD_DIR)/arduino/$(EXAMPLE)" \
		"examples/$(EXAMPLE)"
