HOST_CORE_SOURCES := \
	src/resource.cpp \
	src/runtime.cpp \
	src/status.cpp \
	src/time.cpp

HOST_IO_SOURCES := \
	$(HOST_CORE_SOURCES) \
	src/digital_input.cpp \
	src/digital_output.cpp \
	tests/fake_arduino/Arduino.cpp

HOST_TESTS := \
	$(BUILD_DIR)/host/test_core \
	$(BUILD_DIR)/host/test_io \
	$(BUILD_DIR)/host/test_mono_led \
	$(BUILD_DIR)/host/test_button

HOST_HEADERS := $(shell find src tests/fake_arduino -type f -name '*.h' | sort)

.PHONY: host-test host-test-exceptions
host-test: $(HOST_TESTS)
	@for test in $(HOST_TESTS); do "$$test"; done

host-test-exceptions: | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_core_exceptions"
	$(BUILD_DIR)/host/test_core_exceptions

$(BUILD_DIR)/host/test_core: $(HOST_CORE_SOURCES) tests/test_core.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_io: $(HOST_IO_SOURCES) tests/test_io.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) tests/test_io.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_mono_led: $(HOST_IO_SOURCES) src/mono_led.cpp \
		tests/test_mono_led.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/mono_led.cpp tests/test_mono_led.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_button: $(HOST_IO_SOURCES) src/button.cpp \
		tests/test_button.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/button.cpp tests/test_button.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host: | $(BUILD_MARKER)
	mkdir -p "$@"
