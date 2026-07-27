include mk/config.mk
include mk/bootstrap.mk
include mk/host.mk
include mk/arduino.mk
include mk/package.mk
include mk/docs.mk
include mk/site.mk
include mk/style.mk
include mk/legacy.mk
include mk/quality.mk

.DEFAULT_GOAL := check

.PHONY: check clean help
check: host-test style-check

help:
	@awk '/^## [^ ]+  +/ { sub(/^## /, ""); print }' docs/CLI.md

clean:
	@if test -f "$(BUILD_MARKER)"; then \
		rm -rf -- "$(BUILD_DIR)"; \
	else \
		echo "Nothing to clean: $(BUILD_DIR) is not an ADK build directory."; \
	fi
