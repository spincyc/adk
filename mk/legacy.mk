LEGACY_DIR      ?= legacy
LEGACY_MAKEFILE := $(LEGACY_DIR)/Makefile

.PHONY: legacy legacy-check legacy-clean legacy-help

legacy: legacy-check

legacy-check:
	@test -f "$(LEGACY_MAKEFILE)" || { \
		echo "Legacy build unavailable: $(LEGACY_MAKEFILE) is missing." >&2; \
		exit 2; \
	}
	+$(MAKE) --no-print-directory -C "$(LEGACY_DIR)" check \
		ARDUINO_CLI="$(ARDUINO_CLI)" \
		BOARD_FQBN="$(BOARD_FQBN)"

legacy-clean:
	@if test -f "$(LEGACY_MAKEFILE)"; then \
		$(MAKE) --no-print-directory -C "$(LEGACY_DIR)" clean; \
	else \
		echo "Nothing to clean: $(LEGACY_MAKEFILE) is missing."; \
	fi

legacy-help:
	@echo "Legacy code is historical and unsupported."
	@echo "Run 'make legacy' explicitly; normal gates exclude it."
