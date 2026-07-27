HDMI_MESH_BUILD     := $(BUILD_DIR)/hdmi-mesh
ROUTE_PROFILE_BUILD := $(BUILD_DIR)/route-profiles

HDMI_MESH_SOURCES := \
	research/hdmi_mesh/hdmi_mesh_controller.cpp \
	research/hdmi_mesh/test_hdmi_mesh_controller.cpp

HDMI_MESH_HEADERS := \
	research/hdmi_mesh/hdmi_mesh_controller.h

ROUTE_PROFILE_SOURCES := \
	research/route_profiles/route_profile.cpp \
	research/route_profiles/test_route_profile.cpp

ROUTE_PROFILE_HEADERS := \
	research/route_profiles/route_profile.h

HDMI_MESH_TEST     := $(HDMI_MESH_BUILD)/test-hdmi-mesh-controller
ROUTE_PROFILE_TEST := $(ROUTE_PROFILE_BUILD)/test-route-profile

.PHONY: hdmi-mesh-check hdmi-mesh-test route-profile-check route-profile-test

hdmi-mesh-check: hdmi-mesh-test

hdmi-mesh-test: $(HDMI_MESH_TEST)
	$(HDMI_MESH_TEST)

route-profile-check: route-profile-test

route-profile-test: $(ROUTE_PROFILE_TEST)
	$(ROUTE_PROFILE_TEST)

$(HDMI_MESH_TEST): $(HDMI_MESH_SOURCES) $(HDMI_MESH_HEADERS) | $(HDMI_MESH_BUILD)
	$(CXX) -Iresearch/hdmi_mesh $(HOST_CXXFLAGS) \
		$(HDMI_MESH_SOURCES) $(HOST_LDFLAGS) -o "$@"

$(ROUTE_PROFILE_TEST): \
		$(ROUTE_PROFILE_SOURCES) $(ROUTE_PROFILE_HEADERS) | $(ROUTE_PROFILE_BUILD)
	$(CXX) -Iresearch/route_profiles $(HOST_CXXFLAGS) \
		$(ROUTE_PROFILE_SOURCES) $(HOST_LDFLAGS) -o "$@"

$(HDMI_MESH_BUILD) $(ROUTE_PROFILE_BUILD): | $(BUILD_MARKER)
	mkdir -p "$@"
