USB_MATRIX       := python scripts/usb_matrix.py
USB_MATRIX_LEDGER ?= $(BUILD_DIR)/usb-matrix/leases.json
USB_DEVICE_NODE  ?=
USB_BUS_ID       ?=
USB_HOST_NODE    ?=
USB_PORT         ?=

.PHONY: usb-matrix-setup usb-matrix-check usb-matrix-test
.PHONY: usb-matrix-discover usb-matrix-status usb-matrix-doctor
.PHONY: usb-matrix-dry-run usb-matrix-log usb-local usb-remote usb-ports
.PHONY: usb-export-plan usb-export usb-assign-plan usb-assign
.PHONY: usb-release-plan usb-release
.PHONY: usb-import-plan usb-import usb-route-plan usb-route

usb-matrix-setup: | $(BUILD_MARKER)
	mkdir -p "$(BUILD_DIR)/usb-matrix"
	@echo "Phase-one state directory: $(BUILD_DIR)/usb-matrix"
	@echo "Install the stock Arch command separately if needed: pacman -S usbip"

usb-matrix-check: usb-matrix-test

usb-matrix-test:
	python -m unittest tests/test_usb_matrix.py tests/test_usb_matrix_security.py

usb-matrix-discover: usb-local
	@if test -n "$(USB_DEVICE_NODE)"; then \
		$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" \
			discover-remote "$(USB_DEVICE_NODE)"; \
	else \
		echo "Set USB_DEVICE_NODE to include remote discovery."; \
	fi

usb-matrix-status:
	@echo "Experimental status; this is not the authoritative fenced controller."
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" status

usb-matrix-doctor:
	@command -v python >/dev/null || { echo "Missing python."; exit 1; }
	@command -v usbip >/dev/null || { \
		echo "Missing usbip; install the stock Arch package: pacman -S usbip"; \
		exit 1; \
	}
	@echo "Python and usbip are available; no kernel or device mutation was attempted."

usb-matrix-dry-run: usb-assign-plan

usb-matrix-log:
	@echo "Phase one has no authoritative audit journal."
	@echo "The temporary ledger snapshot follows:"
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" status

usb-local:
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" discover-local

usb-remote:
	@test -n "$(USB_DEVICE_NODE)" || { echo "Set USB_DEVICE_NODE."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" discover-remote "$(USB_DEVICE_NODE)"

usb-ports:
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" ports

usb-export-plan:
	@test -n "$(USB_BUS_ID)" || { echo "Set USB_BUS_ID."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" export "$(USB_BUS_ID)"

usb-export:
	@test -n "$(USB_BUS_ID)" || { echo "Set USB_BUS_ID."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" export "$(USB_BUS_ID)" --execute

usb-assign-plan:
	@test -n "$(USB_DEVICE_NODE)" || { echo "Set USB_DEVICE_NODE."; exit 1; }
	@test -n "$(USB_BUS_ID)" || { echo "Set USB_BUS_ID."; exit 1; }
	@test -n "$(USB_HOST_NODE)" || { echo "Set USB_HOST_NODE."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" assign \
		"$(USB_DEVICE_NODE)" "$(USB_BUS_ID)" "$(USB_HOST_NODE)"

usb-assign:
	@test -n "$(USB_DEVICE_NODE)" || { echo "Set USB_DEVICE_NODE."; exit 1; }
	@test -n "$(USB_BUS_ID)" || { echo "Set USB_BUS_ID."; exit 1; }
	@test -n "$(USB_HOST_NODE)" || { echo "Set USB_HOST_NODE."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" assign \
		"$(USB_DEVICE_NODE)" "$(USB_BUS_ID)" "$(USB_HOST_NODE)" --execute

usb-import-plan: usb-assign-plan

usb-import: usb-assign

usb-route-plan: usb-assign-plan

usb-route: usb-assign

usb-release-plan:
	@test -n "$(USB_HOST_NODE)" || { echo "Set USB_HOST_NODE."; exit 1; }
	@test -n "$(USB_PORT)" || { echo "Set USB_PORT."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" release \
		"$(USB_HOST_NODE)" --port "$(USB_PORT)"

usb-release:
	@test -n "$(USB_HOST_NODE)" || { echo "Set USB_HOST_NODE."; exit 1; }
	@test -n "$(USB_PORT)" || { echo "Set USB_PORT."; exit 1; }
	$(USB_MATRIX) --ledger "$(USB_MATRIX_LEDGER)" release \
		"$(USB_HOST_NODE)" --port "$(USB_PORT)" --execute
