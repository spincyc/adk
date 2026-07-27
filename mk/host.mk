HOST_CORE_SOURCES := \
	src/resource.cpp \
	src/runtime.cpp \
	src/status.cpp \
	src/time.cpp

HOST_HEADERS := $(shell find src tests/fake_arduino -type f -name '*.h' | sort)
HOST_TEST    := $(BUILD_DIR)/host/test_core

.PHONY: host-test host-test-exceptions
host-test: $(HOST_TEST)
	$(HOST_TEST)

host-test-exceptions: | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_core_exceptions"
	$(BUILD_DIR)/host/test_core_exceptions

$(HOST_TEST): $(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_HEADERS) \
		| $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host: | $(BUILD_MARKER)
	mkdir -p "$@"
