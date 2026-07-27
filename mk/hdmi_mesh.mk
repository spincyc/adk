HDMI_MESH_BUILD     := $(BUILD_DIR)/hdmi-mesh
ROUTE_PROFILE_BUILD := $(BUILD_DIR)/route-profiles
HDMI_MESH_PYTHON    ?= python

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
HDMI_MESH_OBSERVER := scripts/hdmi_mesh_observe.py
HDMI_MESH_OBSERVER_TEST := scripts/test_hdmi_mesh_observe.py

.PHONY: hdmi-mesh-check hdmi-mesh-test hdmi-mesh-observer-test \
	hdmi-mesh-routes hdmi-mesh-route hdmi-mesh-trace hdmi-mesh-crc \
	hdmi-mesh-latency route-profile-check route-profile-test

check: hdmi-mesh-check route-profile-check

hdmi-mesh-check: hdmi-mesh-test hdmi-mesh-observer-test

hdmi-mesh-test: $(HDMI_MESH_TEST)
	$(HDMI_MESH_TEST)

hdmi-mesh-observer-test: $(HDMI_MESH_OBSERVER) $(HDMI_MESH_OBSERVER_TEST)
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER_TEST)

hdmi-mesh-routes:
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER) routes

hdmi-mesh-route:
	@test -n "$(HDMI_SOURCE)" || { echo "HDMI_SOURCE is required." >&2; exit 2; }
	@test -n "$(HDMI_DESTINATION)" || { \
		echo "HDMI_DESTINATION is required." >&2; exit 2; \
	}
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER) route \
		--source "$(HDMI_SOURCE)" --destination "$(HDMI_DESTINATION)"

hdmi-mesh-trace:
	@test -n "$(HDMI_ROUTE)" || { echo "HDMI_ROUTE is required." >&2; exit 2; }
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER) trace --route "$(HDMI_ROUTE)"

hdmi-mesh-crc:
	@test -n "$(HDMI_ROUTE)" || { echo "HDMI_ROUTE is required." >&2; exit 2; }
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER) crc --route "$(HDMI_ROUTE)"

hdmi-mesh-latency:
	@test -n "$(HDMI_ROUTE)" || { echo "HDMI_ROUTE is required." >&2; exit 2; }
	$(HDMI_MESH_PYTHON) $(HDMI_MESH_OBSERVER) latency --route "$(HDMI_ROUTE)"

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
