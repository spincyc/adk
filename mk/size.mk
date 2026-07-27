FIRMWARE_SIZE_REPORT   ?= $(BUILD_DIR)/size/avr.tsv
FIRMWARE_SIZE_BASELINE ?= docs/size_baseline.tsv

.PHONY: size size-check size-update

size: arduino
	mkdir -p "$(dir $(FIRMWARE_SIZE_REPORT))"
	python scripts/check_firmware_sizes.py measure \
		--build-root "$(BUILD_DIR)/arduino" \
		--report "$(FIRMWARE_SIZE_REPORT)" \
		--fqbn "$(BOARD_FQBN)" \
		--examples $(EXAMPLES)

size-check: size
	python scripts/check_firmware_sizes.py check \
		--report "$(FIRMWARE_SIZE_REPORT)" \
		--baseline "$(FIRMWARE_SIZE_BASELINE)" \
		--examples $(EXAMPLES)

size-update: size
	python scripts/check_firmware_sizes.py update \
		--report "$(FIRMWARE_SIZE_REPORT)" \
		--baseline "$(FIRMWARE_SIZE_BASELINE)" \
		--examples $(EXAMPLES)
	@echo "Review and justify the explicit baseline changes before committing."
