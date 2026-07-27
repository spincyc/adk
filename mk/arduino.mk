.PHONY: arduino arduino-check boards monitor monitor-describe serial-log \
	$(addprefix arduino-,$(EXAMPLES)) $(addprefix upload-,$(EXAMPLES))
arduino: $(addprefix arduino-,$(EXAMPLES))

arduino-check:
	@command -v "$(ARDUINO_CLI)" >/dev/null || \
		{ echo "arduino-cli is required for firmware builds."; exit 1; }

boards: arduino-check
	$(ARDUINO_CLI) board list

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
	@test -d "examples/$(EXAMPLE)" || \
		{ echo "Unknown example: $(EXAMPLE)"; exit 1; }
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	$(ARDUINO_CLI) compile --upload \
		--fqbn "$(BOARD_FQBN)" \
		--port "$(PORT)" \
		--library . \
		--build-path "$(BUILD_DIR)/arduino/$(EXAMPLE)" \
		"examples/$(EXAMPLE)"

$(addprefix upload-,$(EXAMPLES)): upload-%: arduino-check | $(BUILD_MARKER)
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	$(ARDUINO_CLI) compile --upload \
		--fqbn "$(BOARD_FQBN)" \
		--port "$(PORT)" \
		--library . \
		--build-path "$(BUILD_DIR)/arduino/$*" \
		"examples/$*"

monitor: arduino-check
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	$(ARDUINO_CLI) monitor \
		--port "$(PORT)" \
		--fqbn "$(BOARD_FQBN)" \
		--config "baudrate=$(BAUD)" \
		--timestamp

monitor-describe: arduino-check
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	$(ARDUINO_CLI) monitor \
		--port "$(PORT)" \
		--fqbn "$(BOARD_FQBN)" \
		--describe

serial-log: arduino-check | $(BUILD_MARKER)
	@test -n "$(PORT)" || { echo "Set PORT=/dev/ttyACM0."; exit 1; }
	mkdir -p "$(dir $(SERIAL_LOG))"
	$(ARDUINO_CLI) monitor \
		--port "$(PORT)" \
		--fqbn "$(BOARD_FQBN)" \
		--config "baudrate=$(BAUD)" \
		--timestamp | tee "$(SERIAL_LOG)"
