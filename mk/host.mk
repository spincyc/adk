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

HOST_PWM_SOURCES := \
	$(HOST_IO_SOURCES) \
	src/board.cpp \
	src/pwm_output.cpp

HOST_RGB_SOURCES := \
	$(HOST_PWM_SOURCES) \
	src/rgb_led.cpp

HOST_PIEZO_SOURCES := \
	$(HOST_IO_SOURCES) \
	src/piezo_sounder.cpp

HOST_TESTS := \
	$(BUILD_DIR)/host/test_core \
	$(BUILD_DIR)/host/test_analog_input \
	$(BUILD_DIR)/host/test_io \
	$(BUILD_DIR)/host/test_keypad \
	$(BUILD_DIR)/host/test_matrix_keypad \
	$(BUILD_DIR)/host/test_mono_led \
	$(BUILD_DIR)/host/test_button \
	$(BUILD_DIR)/host/test_character_display \
	$(BUILD_DIR)/host/test_climate_sensor \
	$(BUILD_DIR)/host/test_dht11_sensor \
	$(BUILD_DIR)/host/test_night_light \
	$(BUILD_DIR)/host/test_reaction_timer \
	$(BUILD_DIR)/host/test_pwm_output \
	$(BUILD_DIR)/host/test_rgb_led \
	$(BUILD_DIR)/host/test_piezo_sounder \
	$(BUILD_DIR)/host/test_sampled_signal \
	$(BUILD_DIR)/host/test_seven_segment_display \
	$(BUILD_DIR)/host/test_shift_register \
	$(BUILD_DIR)/host/test_simon \
	$(BUILD_DIR)/host/test_traffic_junction

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

$(BUILD_DIR)/host/test_analog_input: $(HOST_CORE_SOURCES) \
		src/analog_input.cpp src/board.cpp src/digital_output.cpp \
		tests/fake_arduino/Arduino.cpp \
		tests/test_analog_input.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/analog_input.cpp src/board.cpp \
		src/digital_output.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_analog_input.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_io: $(HOST_IO_SOURCES) tests/test_io.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) tests/test_io.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_keypad: $(HOST_CORE_SOURCES) src/keypad.cpp \
		tests/test_keypad.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/keypad.cpp tests/test_keypad.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_matrix_keypad: $(HOST_IO_SOURCES) src/keypad.cpp \
		src/matrix_keypad.cpp tests/test_matrix_keypad.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/keypad.cpp src/matrix_keypad.cpp \
		tests/test_matrix_keypad.cpp $(HOST_LDFLAGS) -o "$@"

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

$(BUILD_DIR)/host/test_character_display: $(HOST_CORE_SOURCES) \
		src/board.cpp src/character_display.cpp src/climate_sensor.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_character_display.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/character_display.cpp \
		src/climate_sensor.cpp tests/fake_arduino/Arduino.cpp \
		tests/test_character_display.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_climate_sensor: $(HOST_CORE_SOURCES) \
		src/climate_sensor.cpp tests/test_climate_sensor.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/climate_sensor.cpp \
		tests/test_climate_sensor.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_dht11_sensor: $(HOST_CORE_SOURCES) \
		src/board.cpp src/climate_sensor.cpp src/dht11_sensor.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_dht11_sensor.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/climate_sensor.cpp \
		src/dht11_sensor.cpp tests/fake_arduino/Arduino.cpp \
		tests/test_dht11_sensor.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_night_light: $(HOST_CORE_SOURCES) \
		src/night_light.cpp tests/test_night_light.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/night_light.cpp \
		tests/test_night_light.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_reaction_timer: $(HOST_IO_SOURCES) src/button.cpp \
		src/reaction_timer.cpp tests/test_reaction_timer.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/button.cpp src/reaction_timer.cpp \
		tests/test_reaction_timer.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_pwm_output: $(HOST_PWM_SOURCES) \
		tests/test_pwm_output.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_PWM_SOURCES) tests/test_pwm_output.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_rgb_led: $(HOST_RGB_SOURCES) \
		tests/test_rgb_led.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_RGB_SOURCES) tests/test_rgb_led.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_piezo_sounder: $(HOST_PIEZO_SOURCES) \
		tests/test_piezo_sounder.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_PIEZO_SOURCES) tests/test_piezo_sounder.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_sampled_signal: $(HOST_CORE_SOURCES) \
		src/sampled_signal.cpp tests/test_sampled_signal.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/sampled_signal.cpp \
		tests/test_sampled_signal.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_seven_segment_display: $(HOST_IO_SOURCES) \
		src/shift_register.cpp src/seven_segment_display.cpp \
		tests/test_seven_segment_display.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/shift_register.cpp \
		src/seven_segment_display.cpp tests/test_seven_segment_display.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_shift_register: $(HOST_IO_SOURCES) \
		src/shift_register.cpp tests/test_shift_register.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/shift_register.cpp \
		tests/test_shift_register.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_simon: $(HOST_CORE_SOURCES) src/simon.cpp \
		tests/test_simon.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/simon.cpp tests/test_simon.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_traffic_junction: $(HOST_CORE_SOURCES) \
		src/traffic_junction.cpp tests/test_traffic_junction.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/traffic_junction.cpp \
		tests/test_traffic_junction.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host: | $(BUILD_MARKER)
	mkdir -p "$@"
