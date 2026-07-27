HOST_SOURCES := \
	src/color.cpp \
	src/defines.cpp \
	src/led.cpp \
	src/object.cpp \
	src/pin.cpp \
	tests/test_adk.cpp

HOST_HEADERS := $(shell find src tests/fake_arduino -type f -name '*.h' | sort)
HOST_TEST := $(BUILD_DIR)/host/test_adk

.PHONY: host-test
host-test: $(HOST_TEST)
	$(HOST_TEST)

$(HOST_TEST): $(HOST_SOURCES) $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_SOURCES) $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host: | $(BUILD_MARKER)
	mkdir -p "$@"
