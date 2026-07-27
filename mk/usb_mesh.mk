USB_MESH_BUILD := $(BUILD_DIR)/usb-mesh

USB_MESH_CONTROLLER_SOURCES := \
	research/usb_mesh/mesh_controller.cpp \
	research/usb_mesh/test_mesh_controller.cpp

USB_MESH_CONTROLLER_HEADERS := \
	research/usb_mesh/mesh_controller.h

USB_MESH_ADAPTER_SOURCES := \
	research/usb_mesh_adapter/usb_action_adapter.cpp \
	research/usb_mesh_adapter/test_usb_action_adapter.cpp

USB_MESH_ADAPTER_HEADERS := \
	research/usb_mesh_adapter/usb_action_adapter.h

USB_MESH_TESTS := \
	$(USB_MESH_BUILD)/test-mesh-controller \
	$(USB_MESH_BUILD)/test-usb-action-adapter

.PHONY: usb-mesh-check usb-mesh-test

check: usb-mesh-check

usb-mesh-check: usb-mesh-test

usb-mesh-test: $(USB_MESH_TESTS)
	@for test in $(USB_MESH_TESTS); do "$$test"; done

$(USB_MESH_BUILD)/test-mesh-controller: \
		$(USB_MESH_CONTROLLER_SOURCES) \
		$(USB_MESH_CONTROLLER_HEADERS) | $(USB_MESH_BUILD)
	$(CXX) -Iresearch/usb_mesh $(HOST_CXXFLAGS) \
		$(USB_MESH_CONTROLLER_SOURCES) $(HOST_LDFLAGS) -o "$@"

$(USB_MESH_BUILD)/test-usb-action-adapter: \
		$(USB_MESH_ADAPTER_SOURCES) \
		$(USB_MESH_ADAPTER_HEADERS) | $(USB_MESH_BUILD)
	$(CXX) -Iresearch/usb_mesh_adapter $(HOST_CXXFLAGS) \
		$(USB_MESH_ADAPTER_SOURCES) $(HOST_LDFLAGS) -o "$@"

$(USB_MESH_BUILD): | $(BUILD_MARKER)
	mkdir -p "$@"
