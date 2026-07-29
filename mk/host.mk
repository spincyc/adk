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
	$(BUILD_DIR)/host/test_access_trainer \
	$(BUILD_DIR)/host/test_acoustic_envelope \
	$(BUILD_DIR)/host/test_analog_input \
	$(BUILD_DIR)/host/test_analog_joystick \
	$(BUILD_DIR)/host/test_balance_table_instrument_lifecycle \
	$(BUILD_DIR)/host/test_balance_table_instrument_evidence \
	$(BUILD_DIR)/host/test_balance_table_instrument_boundaries \
	$(BUILD_DIR)/host/test_bounded_homing_policy \
	$(BUILD_DIR)/host/test_bounded_stepper_sequence \
	$(BUILD_DIR)/host/test_bus \
	$(BUILD_DIR)/host/test_calibration_console \
	$(BUILD_DIR)/host/test_captured_ir_evidence \
	$(BUILD_DIR)/host/test_clue_constraint_model_evaluation \
	$(BUILD_DIR)/host/test_clue_constraint_model_prepared \
	$(BUILD_DIR)/host/test_io \
	$(BUILD_DIR)/host/test_cue_audit \
	$(BUILD_DIR)/host/test_fault_aware_operator_panel_lifecycle \
	$(BUILD_DIR)/host/test_fault_aware_operator_panel_interaction \
	$(BUILD_DIR)/host/test_fault_aware_operator_panel_recovery \
	$(BUILD_DIR)/host/test_fault_aware_operator_panel_project \
	$(BUILD_DIR)/host/test_inert_load_interlock \
	$(BUILD_DIR)/host/test_inert_load_panel \
	$(BUILD_DIR)/host/test_inert_channel_assessor \
	$(BUILD_DIR)/host/test_inert_cue_scheduler \
	$(BUILD_DIR)/host/test_inert_escape_console_configuration \
	$(BUILD_DIR)/host/test_inert_escape_console_admission \
	$(BUILD_DIR)/host/test_inert_escape_console_restart \
	$(BUILD_DIR)/host/test_inert_escape_console_replay \
	$(BUILD_DIR)/host/test_inert_escape_console_audit \
	$(BUILD_DIR)/host/test_inert_show_simulator \
	$(BUILD_DIR)/host/test_inertial_observation \
	$(BUILD_DIR)/host/test_inert_ir_translator \
	$(BUILD_DIR)/host/test_inert_ir_translator_integrity \
	$(BUILD_DIR)/host/test_inert_parts_carousel_operation \
	$(BUILD_DIR)/host/test_inert_parts_carousel_audit \
	$(BUILD_DIR)/host/test_interaction_intent_policy \
	$(BUILD_DIR)/host/test_kinetic_sculpture \
	$(BUILD_DIR)/host/test_known_ir_emission_policy \
	$(BUILD_DIR)/host/test_local_identity_registry \
	$(BUILD_DIR)/host/test_orientation_presentation \
	$(BUILD_DIR)/host/test_infrared_decoder \
	$(BUILD_DIR)/host/test_infrared_record \
	$(BUILD_DIR)/host/test_keypad \
	$(BUILD_DIR)/host/test_lesson039_adapter_policy \
	$(BUILD_DIR)/host/test_lesson039_frame_cue_gate \
	$(BUILD_DIR)/host/test_lesson039_hardware_acquisition_failure \
	$(BUILD_DIR)/host/test_lesson039_replay_adapter \
	$(BUILD_DIR)/host/test_matrix_keypad \
	$(BUILD_DIR)/host/test_magnetic_observation \
	$(BUILD_DIR)/host/test_mega_avr_bus_io \
	$(BUILD_DIR)/host/test_mega_bus_driver \
	$(BUILD_DIR)/host/test_mono_led \
	$(BUILD_DIR)/host/test_moisture_sensor \
	$(BUILD_DIR)/host/test_motor_intent \
	$(BUILD_DIR)/host/test_museum_case_monitor \
	$(BUILD_DIR)/host/test_multiplexed_digit_policy \
	$(BUILD_DIR)/host/test_max7219_presentation_policy \
	$(BUILD_DIR)/host/test_button \
	$(BUILD_DIR)/host/test_character_display \
	$(BUILD_DIR)/host/test_climate_sensor \
	$(BUILD_DIR)/host/test_contact_dynamics \
	$(BUILD_DIR)/host/test_course_marshal \
	$(BUILD_DIR)/host/test_dht11_sensor \
	$(BUILD_DIR)/host/test_dual_display_timing_desk \
	$(BUILD_DIR)/host/test_environmental_station \
	$(BUILD_DIR)/host/test_greenhouse_controller \
	$(BUILD_DIR)/host/test_greenhouse_health_pattern \
	$(BUILD_DIR)/host/test_night_light \
	$(BUILD_DIR)/host/test_observation_tracker \
	$(BUILD_DIR)/host/test_one_wire_transaction_policy \
	$(BUILD_DIR)/host/test_one_wire_transaction_policy_search \
	$(BUILD_DIR)/host/test_one_wire_transaction_policy_timing \
	$(BUILD_DIR)/host/test_one_wire_transaction_policy_interrupt \
	$(BUILD_DIR)/host/test_qualified_18b20_probe_set_identity \
	$(BUILD_DIR)/host/test_qualified_18b20_probe_set_conversion \
	$(BUILD_DIR)/host/test_qualified_18b20_probe_set_decode \
	$(BUILD_DIR)/host/test_qualified_18b20_probe_set_state \
	$(BUILD_DIR)/host/test_optical_observation \
	$(BUILD_DIR)/host/test_packet_receiver \
	$(BUILD_DIR)/host/test_passage_ledger \
	$(BUILD_DIR)/host/test_magnetic_passage_logger \
	$(BUILD_DIR)/host/test_passage_qualifier \
	$(BUILD_DIR)/host/test_percussion_sequencer \
	$(BUILD_DIR)/host/test_presence_model \
	$(BUILD_DIR)/host/test_reaction_timer \
	$(BUILD_DIR)/host/test_record_sink \
	$(BUILD_DIR)/host/test_resistive_probe_observation \
	$(BUILD_DIR)/host/test_thermal_radiant_observation \
	$(BUILD_DIR)/host/test_pwm_output \
	$(BUILD_DIR)/host/test_quadrature_encoder \
	$(BUILD_DIR)/host/test_rgb_led \
	$(BUILD_DIR)/host/test_rover_controller \
	$(BUILD_DIR)/host/test_rtc_storage \
	$(BUILD_DIR)/host/test_piezo_sounder \
	$(BUILD_DIR)/host/test_pulse_capture \
	$(BUILD_DIR)/host/test_sampled_signal \
	$(BUILD_DIR)/host/test_servo_calibration \
	$(BUILD_DIR)/host/test_servo_output \
	$(BUILD_DIR)/host/test_seven_segment_display \
	$(BUILD_DIR)/host/test_shift_register \
	$(BUILD_DIR)/host/test_simon \
	$(BUILD_DIR)/host/test_threshold_input \
	$(BUILD_DIR)/host/test_traffic_junction \
	$(BUILD_DIR)/host/test_telemetry_packet \
	$(BUILD_DIR)/host/test_telemetry_evidence \
	$(BUILD_DIR)/host/test_telemetry_console \
	$(BUILD_DIR)/host/test_telemetry_console_project \
	$(BUILD_DIR)/host/test_telemetry_record \
	$(BUILD_DIR)/host/test_ultrasonic_ranger \
	$(BUILD_DIR)/host/test_watering_controller

HOST_HEADERS := $(shell find src tests/fake_arduino -type f -name '*.h' | sort)

INERT_SHOW_TRACE ?= tests/fixtures/inert_show_happy.trace
INERT_SHOW_REPLAY_GOLDEN ?= tests/fixtures/inert_show_happy.golden
INERT_SHOW_AUDIT_GOLDEN ?= tests/fixtures/inert_show_happy_audit.golden

.PHONY: host-test host-test-exceptions inert-show-test inert-show-replay \
	inert-show-audit inert-show-acceptance
host-test: $(HOST_TESTS)
	@for test in $(HOST_TESTS); do "$$test"; done

inert-show-test: $(BUILD_DIR)/host/test_inert_show_simulator
	$(BUILD_DIR)/host/test_inert_show_simulator

inert-show-replay: $(BUILD_DIR)/host/inert_show_trace_runner
	$(BUILD_DIR)/host/inert_show_trace_runner "$(if $(TRACE),$(TRACE),$(INERT_SHOW_TRACE))"

inert-show-audit: $(BUILD_DIR)/host/inert_show_trace_runner
	$(BUILD_DIR)/host/inert_show_trace_runner \
		"$(if $(TRACE),$(TRACE),$(INERT_SHOW_TRACE))" --audit

inert-show-acceptance: $(BUILD_DIR)/host/inert_show_trace_runner
	@$(BUILD_DIR)/host/inert_show_trace_runner "$(INERT_SHOW_TRACE)" | \
		cmp - "$(INERT_SHOW_REPLAY_GOLDEN)"
	@$(BUILD_DIR)/host/inert_show_trace_runner "$(INERT_SHOW_TRACE)" --audit | \
		cmp - "$(INERT_SHOW_AUDIT_GOLDEN)"

host-test-exceptions: | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_core_exceptions"
	$(BUILD_DIR)/host/test_core_exceptions
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) src/cue_audit.cpp tests/test_cue_audit.cpp \
		$(HOST_LDFLAGS) -o "$(BUILD_DIR)/host/test_cue_audit_exceptions"
	$(BUILD_DIR)/host/test_cue_audit_exceptions
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) src/cue_audit.cpp src/inert_cue_scheduler.cpp \
		tests/test_inert_cue_scheduler.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_inert_cue_scheduler_exceptions"
	$(BUILD_DIR)/host/test_inert_cue_scheduler_exceptions
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) src/inert_channel_assessor.cpp src/cue_audit.cpp \
		src/inert_cue_scheduler.cpp src/inert_show_simulator.cpp \
		tests/test_inert_show_simulator.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_inert_show_simulator_exceptions"
	$(BUILD_DIR)/host/test_inert_show_simulator_exceptions
	$(CXX) $(HOST_CPPFLAGS) $(filter-out -fno-exceptions,$(HOST_CXXFLAGS)) \
		$(HOST_CORE_SOURCES) src/percussion_sequencer.cpp \
		tests/test_percussion_sequencer.cpp $(HOST_LDFLAGS) \
		-o "$(BUILD_DIR)/host/test_percussion_sequencer_exceptions"
	$(BUILD_DIR)/host/test_percussion_sequencer_exceptions

$(BUILD_DIR)/host/test_core: $(HOST_CORE_SOURCES) tests/test_core.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) tests/test_core.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_clue_constraint_model_evaluation: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/pulse_input.cpp \
		tests/test_clue_constraint_model.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/pulse_input.cpp \
		-DADK_CLUE_CONSTRAINT_TEST_PART=1 \
		tests/test_clue_constraint_model.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_clue_constraint_model_prepared: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/pulse_input.cpp \
		tests/test_clue_constraint_model.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/pulse_input.cpp \
		-DADK_CLUE_CONSTRAINT_TEST_PART=2 \
		tests/test_clue_constraint_model.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_fault_aware_operator_panel_lifecycle: $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		tests/test_fault_aware_operator_panel.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		-DADK_FAULT_AWARE_PANEL_TEST_PART=1 \
		tests/test_fault_aware_operator_panel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_fault_aware_operator_panel_interaction: $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		tests/test_fault_aware_operator_panel.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		-DADK_FAULT_AWARE_PANEL_TEST_PART=2 \
		tests/test_fault_aware_operator_panel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_fault_aware_operator_panel_recovery: $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		tests/test_fault_aware_operator_panel.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		-DADK_FAULT_AWARE_PANEL_TEST_PART=3 \
		tests/test_fault_aware_operator_panel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_fault_aware_operator_panel_project: $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		tests/test_fault_aware_operator_panel.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/fault_aware_operator_panel.cpp src/pulse_input.cpp \
		-DADK_FAULT_AWARE_PANEL_TEST_PART=4 \
		tests/test_fault_aware_operator_panel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_escape_console_configuration: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		tests/test_inert_escape_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		-DADK_ESCAPE_CONSOLE_TEST_PART=1 \
		tests/test_inert_escape_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_escape_console_admission: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		tests/test_inert_escape_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		-DADK_ESCAPE_CONSOLE_TEST_PART=2 \
		tests/test_inert_escape_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_escape_console_restart: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		tests/test_inert_escape_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		-DADK_ESCAPE_CONSOLE_TEST_PART=3 \
		tests/test_inert_escape_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_escape_console_replay: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		tests/test_inert_escape_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		-DADK_ESCAPE_CONSOLE_TEST_PART=4 \
		tests/test_inert_escape_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_escape_console_audit: $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		tests/test_inert_escape_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/clue_constraint_model.cpp src/fault_aware_operator_panel.cpp \
		src/inert_escape_console.cpp src/pulse_input.cpp \
		-DADK_ESCAPE_CONSOLE_TEST_PART=5 \
		tests/test_inert_escape_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_captured_ir_evidence: $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_captured_ir_evidence.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_captured_ir_evidence.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_known_ir_emission_policy: $(HOST_CORE_SOURCES) \
		src/known_ir_emission_policy.cpp src/pulse_input.cpp \
		tests/test_known_ir_emission_policy.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/known_ir_emission_policy.cpp src/pulse_input.cpp \
		tests/test_known_ir_emission_policy.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_ir_translator: $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/inert_ir_translator.cpp src/known_ir_emission_policy.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_inert_ir_translator.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/inert_ir_translator.cpp src/known_ir_emission_policy.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		-DADK_IR_TRANSLATOR_TEST_PART=1 tests/test_inert_ir_translator.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_ir_translator_integrity: $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/inert_ir_translator.cpp src/known_ir_emission_policy.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_inert_ir_translator.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/board.cpp src/captured_ir_evidence.cpp src/infrared_decoder.cpp \
		src/inert_ir_translator.cpp src/known_ir_emission_policy.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		-DADK_IR_TRANSLATOR_TEST_PART=2 tests/test_inert_ir_translator.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_cue_audit: $(HOST_CORE_SOURCES) src/cue_audit.cpp \
		tests/test_cue_audit.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/cue_audit.cpp tests/test_cue_audit.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_cue_scheduler: $(HOST_CORE_SOURCES) \
		src/cue_audit.cpp src/inert_cue_scheduler.cpp \
		tests/test_inert_cue_scheduler.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/cue_audit.cpp src/inert_cue_scheduler.cpp \
		tests/test_inert_cue_scheduler.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_show_simulator: $(HOST_CORE_SOURCES) \
		src/inert_channel_assessor.cpp src/cue_audit.cpp \
		src/inert_cue_scheduler.cpp src/inert_show_simulator.cpp \
		tests/test_inert_show_simulator.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/inert_channel_assessor.cpp src/cue_audit.cpp \
		src/inert_cue_scheduler.cpp src/inert_show_simulator.cpp \
		tests/test_inert_show_simulator.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inertial_observation: $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp tests/test_inertial_observation.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp tests/test_inertial_observation.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_orientation_presentation: $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		tests/test_orientation_presentation.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		tests/test_orientation_presentation.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_balance_table_instrument_lifecycle: $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		-DADK_BALANCE_TABLE_TEST_PART=1 $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_balance_table_instrument_evidence: $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		-DADK_BALANCE_TABLE_TEST_PART=2 $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_balance_table_instrument_boundaries: $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		-DADK_BALANCE_TABLE_TEST_PART=3 $(HOST_CORE_SOURCES) \
		src/inertial_observation.cpp src/orientation_presentation.cpp \
		src/balance_table_instrument.cpp tests/test_balance_table_instrument.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_bounded_homing_policy: $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp \
		tests/test_bounded_homing_policy.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp \
		tests/test_bounded_homing_policy.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_bounded_stepper_sequence: $(HOST_CORE_SOURCES) \
		src/bounded_stepper_sequence.cpp \
		tests/test_bounded_stepper_sequence.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/bounded_stepper_sequence.cpp \
		tests/test_bounded_stepper_sequence.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_interaction_intent_policy: $(HOST_CORE_SOURCES) \
		src/contact_dynamics.cpp src/interaction_intent_policy.cpp \
		tests/test_interaction_intent_policy.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/contact_dynamics.cpp src/interaction_intent_policy.cpp \
		tests/test_interaction_intent_policy.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_kinetic_sculpture: $(HOST_CORE_SOURCES) \
		src/contact_dynamics.cpp src/interaction_intent_policy.cpp \
		src/bounded_stepper_sequence.cpp src/kinetic_sculpture.cpp \
		tests/test_kinetic_sculpture.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/contact_dynamics.cpp src/interaction_intent_policy.cpp \
		src/bounded_stepper_sequence.cpp src/kinetic_sculpture.cpp \
		tests/test_kinetic_sculpture.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_local_identity_registry: $(HOST_CORE_SOURCES) \
		src/local_identity_registry.cpp \
		tests/test_local_identity_registry.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/local_identity_registry.cpp \
		tests/test_local_identity_registry.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_parts_carousel_operation: $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp src/bounded_stepper_sequence.cpp \
		src/local_identity_registry.cpp src/inert_parts_carousel.cpp \
		tests/test_inert_parts_carousel.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		-DADK_INERT_PARTS_CAROUSEL_TEST_PART=1 $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp src/bounded_stepper_sequence.cpp \
		src/local_identity_registry.cpp src/inert_parts_carousel.cpp \
		tests/test_inert_parts_carousel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_parts_carousel_audit: $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp src/bounded_stepper_sequence.cpp \
		src/local_identity_registry.cpp src/inert_parts_carousel.cpp \
		tests/test_inert_parts_carousel.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		-DADK_INERT_PARTS_CAROUSEL_TEST_PART=2 $(HOST_CORE_SOURCES) \
		src/bounded_homing_policy.cpp src/bounded_stepper_sequence.cpp \
		src/local_identity_registry.cpp src/inert_parts_carousel.cpp \
		tests/test_inert_parts_carousel.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/inert_show_trace_runner: $(HOST_CORE_SOURCES) \
		src/inert_channel_assessor.cpp src/cue_audit.cpp \
		src/inert_cue_scheduler.cpp src/inert_show_simulator.cpp \
		tests/inert_show_trace_runner.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/inert_channel_assessor.cpp src/cue_audit.cpp \
		src/inert_cue_scheduler.cpp src/inert_show_simulator.cpp \
		tests/inert_show_trace_runner.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_bus: $(HOST_CORE_SOURCES) \
		src/i2c_bus.cpp src/spi_bus.cpp tests/test_bus.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/i2c_bus.cpp src/spi_bus.cpp \
		tests/test_bus.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_calibration_console: $(HOST_CORE_SOURCES) \
		src/calibration_console.cpp tests/test_calibration_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/calibration_console.cpp \
		tests/test_calibration_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_course_marshal: $(HOST_CORE_SOURCES) \
		src/course_marshal.cpp src/optical_observation.cpp \
		src/presence_model.cpp src/pulse_input.cpp \
		tests/test_course_marshal.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/course_marshal.cpp \
		src/optical_observation.cpp src/presence_model.cpp \
		src/pulse_input.cpp tests/test_course_marshal.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_access_trainer: $(HOST_CORE_SOURCES) \
		src/access_trainer.cpp src/keypad.cpp tests/test_access_trainer.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/access_trainer.cpp src/keypad.cpp \
		tests/test_access_trainer.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_acoustic_envelope: $(HOST_CORE_SOURCES) \
		src/acoustic_envelope.cpp tests/test_acoustic_envelope.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/acoustic_envelope.cpp \
		tests/test_acoustic_envelope.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_one_wire_transaction_policy: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_one_wire_transaction_policy_search: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_search.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_search.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_one_wire_transaction_policy_timing: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_timing.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		-DADK_TESTING src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_timing.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_one_wire_transaction_policy_interrupt: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_interrupt.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		-DADK_TESTING src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		tests/test_one_wire_transaction_policy_interrupt.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_qualified_18b20_probe_set_identity: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_identity.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_identity.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_qualified_18b20_probe_set_conversion: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_conversion.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_conversion.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_qualified_18b20_probe_set_decode: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_decode.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_decode.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_qualified_18b20_probe_set_state: $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_state.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) $(HOST_CORE_SOURCES) \
		src/one_wire_transaction_policy.cpp src/pulse_input.cpp \
		src/qualified_18b20_probe_set_policy.cpp \
		tests/test_qualified_18b20_probe_set_state.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_optical_observation: $(HOST_CORE_SOURCES) \
		src/optical_observation.cpp tests/test_optical_observation.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/optical_observation.cpp \
		tests/test_optical_observation.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_presence_model: $(HOST_CORE_SOURCES) \
		src/optical_observation.cpp src/presence_model.cpp src/pulse_input.cpp \
		tests/test_presence_model.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/optical_observation.cpp \
		src/presence_model.cpp src/pulse_input.cpp \
		tests/test_presence_model.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_analog_input: $(HOST_CORE_SOURCES) \
		src/analog_input.cpp src/board.cpp src/digital_output.cpp \
		tests/fake_arduino/Arduino.cpp \
		tests/test_analog_input.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/analog_input.cpp src/board.cpp \
		src/digital_output.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_analog_input.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_analog_joystick: $(HOST_IO_SOURCES) \
		src/analog_input.cpp src/analog_joystick.cpp src/board.cpp \
		src/button.cpp tests/test_analog_joystick.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/analog_input.cpp src/analog_joystick.cpp \
		src/board.cpp src/button.cpp tests/test_analog_joystick.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_io: $(HOST_IO_SOURCES) tests/test_io.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) tests/test_io.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_quadrature_encoder: $(HOST_IO_SOURCES) src/board.cpp \
		src/quadrature_encoder.cpp tests/test_quadrature_encoder.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/board.cpp src/quadrature_encoder.cpp \
		tests/test_quadrature_encoder.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_passage_qualifier: $(HOST_CORE_SOURCES) \
		src/passage_qualifier.cpp tests/test_passage_qualifier.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/passage_qualifier.cpp \
		tests/test_passage_qualifier.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_passage_ledger: $(HOST_CORE_SOURCES) \
		src/passage_ledger.cpp tests/test_passage_ledger.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/passage_ledger.cpp \
		tests/test_passage_ledger.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_percussion_sequencer: $(HOST_CORE_SOURCES) \
		src/percussion_sequencer.cpp tests/test_percussion_sequencer.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/percussion_sequencer.cpp \
		tests/test_percussion_sequencer.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_lesson039_adapter_policy: \
		examples/Lesson039PercussionSequencer/Lesson039AdapterPolicy.h \
		examples/Lesson039PercussionSequencer/tests/test_adapter_policy.cpp \
		| $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		examples/Lesson039PercussionSequencer/tests/test_adapter_policy.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_lesson039_frame_cue_gate: \
		examples/Lesson039PercussionSequencer/FrameCueGate.h \
		examples/Lesson039PercussionSequencer/tests/test_frame_cue_gate.cpp \
		| $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		examples/Lesson039PercussionSequencer/tests/test_frame_cue_gate.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_lesson039_hardware_acquisition_failure: $(HOST_IO_SOURCES) \
		src/acoustic_envelope.cpp src/analog_input.cpp src/board.cpp src/button.cpp \
		src/contact_dynamics.cpp src/mono_led.cpp src/percussion_sequencer.cpp \
		src/piezo_sounder.cpp src/seven_segment_display.cpp \
		src/shift_register.cpp \
		examples/Lesson039PercussionSequencer/FrameCueGate.h \
		examples/Lesson039PercussionSequencer/Lesson039AdapterPolicy.h \
		examples/Lesson039PercussionSequencer/Lesson039PercussionSequencer.ino \
		examples/Lesson039PercussionSequencer/tests/test_hardware_acquisition_failure.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/acoustic_envelope.cpp src/analog_input.cpp \
		src/board.cpp src/button.cpp src/contact_dynamics.cpp \
		src/mono_led.cpp src/percussion_sequencer.cpp src/piezo_sounder.cpp \
		src/seven_segment_display.cpp src/shift_register.cpp \
		examples/Lesson039PercussionSequencer/tests/test_hardware_acquisition_failure.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_lesson039_replay_adapter: $(HOST_IO_SOURCES) \
		src/acoustic_envelope.cpp src/analog_input.cpp src/board.cpp src/button.cpp \
		src/contact_dynamics.cpp src/mono_led.cpp src/percussion_sequencer.cpp \
		src/piezo_sounder.cpp src/seven_segment_display.cpp \
		src/shift_register.cpp \
		examples/Lesson039PercussionSequencer/FrameCueGate.h \
		examples/Lesson039PercussionSequencer/Lesson039AdapterPolicy.h \
		examples/Lesson039PercussionSequencer/Lesson039PercussionSequencer.ino \
		examples/Lesson039PercussionSequencer/tests/test_replay_adapter.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/acoustic_envelope.cpp src/analog_input.cpp \
		src/board.cpp src/button.cpp src/contact_dynamics.cpp \
		src/mono_led.cpp src/percussion_sequencer.cpp src/piezo_sounder.cpp \
		src/seven_segment_display.cpp src/shift_register.cpp \
		examples/Lesson039PercussionSequencer/tests/test_replay_adapter.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_magnetic_passage_logger: $(HOST_IO_SOURCES) \
		src/board.cpp src/magnetic_passage_logger.cpp src/passage_ledger.cpp \
		src/rtc.cpp src/seven_segment_display.cpp src/shift_register.cpp \
		tests/test_magnetic_passage_logger.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/board.cpp src/magnetic_passage_logger.cpp \
		src/passage_ledger.cpp src/rtc.cpp src/seven_segment_display.cpp \
		src/shift_register.cpp tests/test_magnetic_passage_logger.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_magnetic_observation: $(HOST_IO_SOURCES) \
		src/analog_input.cpp src/board.cpp src/magnetic_observation.cpp \
		tests/test_magnetic_observation.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/analog_input.cpp src/board.cpp \
		src/magnetic_observation.cpp tests/test_magnetic_observation.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_load_interlock: $(HOST_CORE_SOURCES) \
		src/inert_load_interlock.cpp src/power_domain.cpp \
		tests/test_inert_load_interlock.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/inert_load_interlock.cpp \
		src/power_domain.cpp tests/test_inert_load_interlock.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_load_panel: $(HOST_IO_SOURCES) \
		src/inert_load_panel.cpp src/pump_output.cpp \
		tests/test_inert_load_panel.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/inert_load_panel.cpp \
		src/pump_output.cpp tests/test_inert_load_panel.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_inert_channel_assessor: $(HOST_CORE_SOURCES) \
		src/inert_channel_assessor.cpp \
		tests/test_inert_channel_assessor.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/inert_channel_assessor.cpp \
		tests/test_inert_channel_assessor.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_infrared_decoder: $(HOST_CORE_SOURCES) \
		src/board.cpp src/infrared_decoder.cpp src/pulse_capture.cpp \
		src/pulse_input.cpp tests/test_infrared_decoder.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/infrared_decoder.cpp \
		src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_infrared_decoder.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_infrared_record: $(HOST_CORE_SOURCES) \
		src/infrared_record.cpp tests/test_infrared_record.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/infrared_record.cpp \
		tests/test_infrared_record.cpp $(HOST_LDFLAGS) -o "$@"

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

$(BUILD_DIR)/host/test_mega_bus_driver: $(HOST_CORE_SOURCES) \
		src/i2c_bus.cpp src/mega_bus_driver.cpp src/spi_bus.cpp \
		tests/test_mega_bus_driver.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/i2c_bus.cpp src/mega_bus_driver.cpp \
		src/spi_bus.cpp tests/test_mega_bus_driver.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_mega_avr_bus_io: $(HOST_CORE_SOURCES) \
		src/i2c_bus.cpp src/mega_avr_bus_io.cpp src/mega_bus_driver.cpp \
		src/spi_bus.cpp tests/test_mega_avr_bus_io.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/i2c_bus.cpp src/mega_avr_bus_io.cpp \
		src/mega_bus_driver.cpp src/spi_bus.cpp \
		tests/test_mega_avr_bus_io.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_mono_led: $(HOST_IO_SOURCES) src/mono_led.cpp \
		tests/test_mono_led.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/mono_led.cpp tests/test_mono_led.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_moisture_sensor: $(HOST_CORE_SOURCES) \
		src/analog_input.cpp src/board.cpp src/digital_output.cpp \
		src/moisture_sensor.cpp tests/fake_arduino/Arduino.cpp \
		tests/test_moisture_sensor.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/analog_input.cpp src/board.cpp \
		src/digital_output.cpp src/moisture_sensor.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_moisture_sensor.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_motor_intent: $(HOST_CORE_SOURCES) \
		src/motor_intent.cpp src/power_domain.cpp \
		tests/test_motor_intent.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/motor_intent.cpp src/power_domain.cpp \
		tests/test_motor_intent.cpp $(HOST_LDFLAGS) -o "$@"

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

$(BUILD_DIR)/host/test_contact_dynamics: $(HOST_CORE_SOURCES) \
		src/contact_dynamics.cpp tests/test_contact_dynamics.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/contact_dynamics.cpp \
		tests/test_contact_dynamics.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_dht11_sensor: $(HOST_CORE_SOURCES) \
		src/board.cpp src/climate_sensor.cpp src/dht11_sensor.cpp \
		tests/fake_arduino/Arduino.cpp tests/test_dht11_sensor.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/climate_sensor.cpp \
		src/dht11_sensor.cpp tests/fake_arduino/Arduino.cpp \
		tests/test_dht11_sensor.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_environmental_station: $(HOST_CORE_SOURCES) \
		src/climate_sensor.cpp src/environmental_station.cpp \
		tests/test_environmental_station.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/climate_sensor.cpp \
		src/environmental_station.cpp tests/test_environmental_station.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_greenhouse_controller: $(HOST_IO_SOURCES) \
		src/analog_input.cpp src/board.cpp src/character_display.cpp \
		src/climate_sensor.cpp src/greenhouse_controller.cpp src/moisture_sensor.cpp \
		src/pump_output.cpp src/record_sink.cpp src/watering_controller.cpp \
		tests/test_greenhouse_controller.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/analog_input.cpp src/board.cpp \
		src/character_display.cpp src/climate_sensor.cpp \
		src/greenhouse_controller.cpp \
		src/moisture_sensor.cpp src/pump_output.cpp src/record_sink.cpp \
		src/watering_controller.cpp tests/test_greenhouse_controller.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_greenhouse_health_pattern: $(HOST_RGB_SOURCES) \
		src/analog_input.cpp src/character_display.cpp src/climate_sensor.cpp \
		src/greenhouse_controller.cpp src/greenhouse_health_pattern.cpp \
		src/moisture_sensor.cpp src/pump_output.cpp src/record_sink.cpp \
		src/watering_controller.cpp \
		tests/test_greenhouse_health_pattern.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_RGB_SOURCES) src/analog_input.cpp src/character_display.cpp \
		src/climate_sensor.cpp src/greenhouse_controller.cpp \
		src/greenhouse_health_pattern.cpp src/moisture_sensor.cpp \
		src/pump_output.cpp src/record_sink.cpp src/watering_controller.cpp \
		tests/test_greenhouse_health_pattern.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_night_light: $(HOST_CORE_SOURCES) \
		src/night_light.cpp tests/test_night_light.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/night_light.cpp \
		tests/test_night_light.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_observation_tracker: $(HOST_CORE_SOURCES) \
		src/observation_tracker.cpp src/telemetry_packet.cpp \
		tests/test_observation_tracker.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/observation_tracker.cpp \
		src/telemetry_packet.cpp tests/test_observation_tracker.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_packet_receiver: $(HOST_CORE_SOURCES) \
		src/packet_receiver.cpp tests/test_packet_receiver.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/packet_receiver.cpp \
		tests/test_packet_receiver.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_reaction_timer: $(HOST_IO_SOURCES) src/button.cpp \
		src/reaction_timer.cpp tests/test_reaction_timer.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/button.cpp src/reaction_timer.cpp \
		tests/test_reaction_timer.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_record_sink: $(HOST_CORE_SOURCES) \
		src/fixed_storage.cpp src/record_sink.cpp src/storage.cpp \
		tests/test_record_sink.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/fixed_storage.cpp src/record_sink.cpp \
		src/storage.cpp tests/test_record_sink.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_pwm_output: $(HOST_PWM_SOURCES) \
		tests/test_pwm_output.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_PWM_SOURCES) tests/test_pwm_output.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_rgb_led: $(HOST_RGB_SOURCES) \
		tests/test_rgb_led.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_RGB_SOURCES) tests/test_rgb_led.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_rover_controller: $(HOST_CORE_SOURCES) \
		src/motor_intent.cpp src/power_domain.cpp src/pulse_input.cpp \
		src/rover_controller.cpp src/ultrasonic_ranger.cpp \
		tests/test_rover_controller.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/motor_intent.cpp src/power_domain.cpp \
		src/pulse_input.cpp src/rover_controller.cpp \
		src/ultrasonic_ranger.cpp tests/test_rover_controller.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_rtc_storage: $(HOST_CORE_SOURCES) \
		src/fixed_storage.cpp src/rtc.cpp src/storage.cpp \
		tests/test_rtc_storage.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/fixed_storage.cpp src/rtc.cpp \
		src/storage.cpp tests/test_rtc_storage.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_piezo_sounder: $(HOST_PIEZO_SOURCES) \
		tests/test_piezo_sounder.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_PIEZO_SOURCES) tests/test_piezo_sounder.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_pulse_capture: $(HOST_CORE_SOURCES) \
		src/board.cpp src/pulse_capture.cpp src/pulse_input.cpp \
		tests/test_pulse_capture.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/pulse_capture.cpp \
		src/pulse_input.cpp tests/test_pulse_capture.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_sampled_signal: $(HOST_CORE_SOURCES) \
		src/sampled_signal.cpp tests/test_sampled_signal.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/sampled_signal.cpp \
		tests/test_sampled_signal.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_servo_calibration: $(HOST_CORE_SOURCES) \
		src/servo_calibration.cpp src/servo_configuration.cpp \
		tests/test_servo_calibration.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/servo_calibration.cpp \
		src/servo_configuration.cpp tests/test_servo_calibration.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_servo_output: $(HOST_CORE_SOURCES) \
		src/board.cpp src/power_domain.cpp src/servo_output.cpp \
		tests/test_servo_output.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/board.cpp src/power_domain.cpp \
		src/servo_output.cpp \
		tests/test_servo_output.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_seven_segment_display: $(HOST_IO_SOURCES) \
		src/shift_register.cpp src/seven_segment_display.cpp \
		tests/test_seven_segment_display.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/shift_register.cpp \
		src/seven_segment_display.cpp tests/test_seven_segment_display.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_multiplexed_digit_policy: $(HOST_CORE_SOURCES) \
		src/multiplexed_digit_policy.cpp \
		tests/test_multiplexed_digit_policy.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/multiplexed_digit_policy.cpp \
		tests/test_multiplexed_digit_policy.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_max7219_presentation_policy: $(HOST_CORE_SOURCES) \
		src/max7219_presentation_policy.cpp \
		tests/test_max7219_presentation_policy.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/max7219_presentation_policy.cpp \
		tests/test_max7219_presentation_policy.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_dual_display_timing_desk: $(HOST_CORE_SOURCES) \
		src/multiplexed_digit_policy.cpp \
		src/max7219_presentation_policy.cpp \
		src/dual_display_timing_desk.cpp \
		tests/test_dual_display_timing_desk.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/multiplexed_digit_policy.cpp \
		src/max7219_presentation_policy.cpp \
		src/dual_display_timing_desk.cpp \
		tests/test_dual_display_timing_desk.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_resistive_probe_observation: $(HOST_CORE_SOURCES) \
		src/resistive_probe_observation.cpp \
		tests/test_resistive_probe_observation.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/resistive_probe_observation.cpp \
		tests/test_resistive_probe_observation.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_thermal_radiant_observation: $(HOST_CORE_SOURCES) \
		src/thermal_radiant_observation.cpp \
		tests/test_thermal_radiant_observation.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/thermal_radiant_observation.cpp \
		tests/test_thermal_radiant_observation.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_museum_case_monitor: $(HOST_CORE_SOURCES) \
		src/magnetic_observation.cpp src/resistive_probe_observation.cpp \
		src/thermal_radiant_observation.cpp src/museum_case_monitor.cpp \
		tests/test_museum_case_monitor.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/magnetic_observation.cpp \
		src/resistive_probe_observation.cpp src/thermal_radiant_observation.cpp \
		src/museum_case_monitor.cpp tests/test_museum_case_monitor.cpp \
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

$(BUILD_DIR)/host/test_threshold_input: src/threshold_input.cpp \
		tests/test_threshold_input.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		src/threshold_input.cpp tests/test_threshold_input.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_traffic_junction: $(HOST_CORE_SOURCES) \
		src/traffic_junction.cpp tests/test_traffic_junction.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/traffic_junction.cpp \
		tests/test_traffic_junction.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_telemetry_packet: $(HOST_CORE_SOURCES) \
		src/telemetry_packet.cpp tests/test_telemetry_packet.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/telemetry_packet.cpp \
		tests/test_telemetry_packet.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_telemetry_evidence: $(HOST_CORE_SOURCES) \
		src/observation_tracker.cpp src/telemetry_evidence.cpp \
		src/telemetry_packet.cpp tests/test_telemetry_evidence.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/observation_tracker.cpp \
		src/telemetry_evidence.cpp src/telemetry_packet.cpp \
		tests/test_telemetry_evidence.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_telemetry_console: $(HOST_CORE_SOURCES) \
		src/telemetry_console.cpp tests/test_telemetry_console.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/telemetry_console.cpp \
		tests/test_telemetry_console.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_telemetry_record: $(HOST_CORE_SOURCES) \
		src/telemetry_console.cpp src/telemetry_record.cpp \
		tests/test_telemetry_record.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/telemetry_console.cpp \
		src/telemetry_record.cpp tests/test_telemetry_record.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_telemetry_console_project: $(HOST_CORE_SOURCES) \
		src/record_sink.cpp src/telemetry_console.cpp \
		src/telemetry_console_project.cpp src/telemetry_record.cpp \
		tests/test_telemetry_console_project.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/record_sink.cpp \
		src/telemetry_console.cpp src/telemetry_console_project.cpp \
		src/telemetry_record.cpp tests/test_telemetry_console_project.cpp \
		$(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_ultrasonic_ranger: $(HOST_CORE_SOURCES) \
		src/pulse_input.cpp src/ultrasonic_ranger.cpp \
		tests/test_ultrasonic_ranger.cpp $(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_CORE_SOURCES) src/pulse_input.cpp src/ultrasonic_ranger.cpp \
		tests/test_ultrasonic_ranger.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host/test_watering_controller: $(HOST_IO_SOURCES) \
		src/analog_input.cpp src/board.cpp src/moisture_sensor.cpp \
		src/pump_output.cpp \
		src/watering_controller.cpp tests/test_watering_controller.cpp \
		$(HOST_HEADERS) | $(BUILD_DIR)/host
	$(CXX) $(HOST_CPPFLAGS) $(HOST_CXXFLAGS) \
		$(HOST_IO_SOURCES) src/analog_input.cpp src/board.cpp \
		src/moisture_sensor.cpp \
		src/pump_output.cpp src/watering_controller.cpp \
		tests/test_watering_controller.cpp $(HOST_LDFLAGS) -o "$@"

$(BUILD_DIR)/host: | $(BUILD_MARKER)
	mkdir -p "$@"
