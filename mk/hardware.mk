HARDWARE_RECORD ?= $(BUILD_DIR)/hardware/lesson$(LESSON).md

.PHONY: hardware-card hardware-note analog-note hardware-card-check

hardware-card: | $(BUILD_MARKER)
	@test -n "$(LESSON)" || { echo "Set LESSON=007."; exit 1; }
	@test ! -e "$(HARDWARE_RECORD)" || { \
		echo "Record already exists: $(HARDWARE_RECORD)"; \
		exit 1; \
	}
	mkdir -p "$(dir $(HARDWARE_RECORD))"
	sed "s/@LESSON@/$(LESSON)/g" \
		docs/templates/hardware-acceptance.md > "$(HARDWARE_RECORD)"
	@echo "Created $(HARDWARE_RECORD)."

hardware-note:
	@test -n "$(LESSON)" || { echo "Set LESSON=007."; exit 1; }
	@test -f "$(HARDWARE_RECORD)" || { \
		echo "Create $(HARDWARE_RECORD) with make hardware-card LESSON=$(LESSON)."; \
		exit 1; \
	}
	@test -n "$(SIGNAL)" || { echo "Set SIGNAL to a pin or named test point."; exit 1; }
	@test -n "$(PREDICTION)" || { echo "Set PREDICTION."; exit 1; }
	@test -n "$(OBSERVATION)" || { echo "Set OBSERVATION."; exit 1; }
	@test -n "$(INTERPRETATION)" || { echo "Set INTERPRETATION."; exit 1; }
	printf '\n### Circuit observation\n\n- Signal or test point: %s\n- Prediction: %s\n- Observation: %s\n- Interpretation: %s\n' \
		"$(SIGNAL)" "$(PREDICTION)" "$(OBSERVATION)" "$(INTERPRETATION)" \
		>> "$(HARDWARE_RECORD)"

analog-note:
	@test -n "$(LESSON)" || { echo "Set LESSON=007."; exit 1; }
	@test -f "$(HARDWARE_RECORD)" || { \
		echo "Create $(HARDWARE_RECORD) with make hardware-card LESSON=$(LESSON)."; \
		exit 1; \
	}
	@test -n "$(TEST_POINT)" || { echo "Set TEST_POINT, for example A0-GND."; exit 1; }
	@test -n "$(EXPECTED_V)" || { echo "Set EXPECTED_V."; exit 1; }
	@test -n "$(MEASURED_V)" || { echo "Set MEASURED_V."; exit 1; }
	@test -n "$(RAW_SAMPLE)" || { echo "Set RAW_SAMPLE."; exit 1; }
	@test -n "$(OUTPUT)" || { echo "Set OUTPUT to the observed PWM or light state."; exit 1; }
	printf '\n### Analog observation\n\n- Test point: %s\n- Expected voltage: %s V\n- Measured voltage: %s V\n- Raw sample: %s\n- PWM or visible output: %s\n' \
		"$(TEST_POINT)" "$(EXPECTED_V)" "$(MEASURED_V)" \
		"$(RAW_SAMPLE)" "$(OUTPUT)" >> "$(HARDWARE_RECORD)"

hardware-card-check:
	@test -n "$(LESSON)" || { echo "Set LESSON=007."; exit 1; }
	@test -s "$(HARDWARE_RECORD)" || { echo "Missing record: $(HARDWARE_RECORD)"; exit 1; }
	@grep -q '^## Resource acquisition evidence' "$(HARDWARE_RECORD)"
	@grep -q '^## Safe-state evidence' "$(HARDWARE_RECORD)"
	@grep -q '^## Analog observations' "$(HARDWARE_RECORD)"
	@echo "Record structure is present; a named human must review the observations."
