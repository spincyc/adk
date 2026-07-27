include mk/config.mk
include mk/bootstrap.mk
include mk/host.mk
include mk/arduino.mk
include mk/hardware.mk
include mk/package.mk
include mk/size.mk
include mk/docs.mk
include mk/site.mk
include mk/style.mk
include mk/headers.mk
include mk/legacy.mk
include mk/quality.mk
include mk/usb_matrix.mk
include mk/usb_mesh.mk
include mk/hdmi_mesh.mk

.DEFAULT_GOAL := check

.PHONY: check clean help
check: host-test style-check headers-check

help:
	@awk '/^## [^ ]+  +/ { sub(/^## /, ""); print }' docs/CLI.md

clean:
	@if test -f "$(BUILD_MARKER)"; then \
		rm -rf -- "$(BUILD_DIR)"; \
	else \
		echo "Nothing to clean: $(BUILD_DIR) is not an ADK build directory."; \
	fi
