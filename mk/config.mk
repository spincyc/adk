ARDUINO_CLI ?= arduino-cli
CXX         ?= c++
PDFLATEX    ?= pdflatex

BUILD_DIR   ?= build
BUILD_MARKER := $(BUILD_DIR)/.adk-build
BOARD_FQBN  ?= arduino:avr:mega
ARDUINO_AVR_CORE ?= arduino:avr@1.8.8
LESSONS     := 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031 032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047 048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063 064 065 066 067 068 069 070 071 072
EXAMPLES    := \
	Lesson001DigitalOutput \
	Lesson002DigitalInput \
	Lesson003ReactionTimer \
	Lesson004PwmRgb \
	Lesson005PiezoSounder \
	Lesson006Simon \
	Lesson007AnalogInput \
	Lesson008SampledSignal \
	Lesson009AdaptiveNightLight \
	Lesson010ShiftRegisterDisplay \
	Lesson011TimedTraffic \
	Lesson012TrafficJunction \
	Lesson013Dht11Climate \
	Lesson014CharacterDisplay \
	Lesson015EnvironmentalStation \
	Lesson016MatrixKeypad \
	Lesson017BoundedServo \
	Lesson018AccessTrainer \
	Lesson019UltrasonicRange \
	Lesson020MotorIntent \
	Lesson021BenchRover \
	Lesson022OwnedBuses \
	Lesson023InertLoadInterlock \
	Lesson024GreenhouseTrainer \
	Lesson025InfraredEvidence \
	Lesson026TelemetryPacket \
	Lesson027TelemetryConsole \
	Lesson028InertChannelAssessment \
	Lesson029InertCueSchedule \
	Lesson030InertShowSimulator \
	Lesson031AnalogJoystick \
	Lesson032QuadratureEncoder \
	Lesson033CalibrationConsole \
	Lesson034MagneticObservation \
	Lesson035PassageQualifier \
	Lesson036MagneticPassageLogger \
	Lesson037ContactDynamics \
	Lesson038AcousticEnvelope \
	Lesson039PercussionSequencer \
	Lesson040OpticalObservation \
	Lesson041PresenceModel \
	Lesson042CourseMarshal \
	Lesson043InertialObservation \
	Lesson044OrientationPresentation \
	Lesson045BalanceTableInstrument \
	Lesson046InteractionIntent \
	Lesson047BoundedStepperSequence \
	Lesson048KineticLightSculpture \
	Lesson049LocalIdentityRegistry \
	Lesson050BoundedHomingPolicy \
	Lesson051InertPartsCarousel \
	Lesson052CapturedIrEvidence \
	Lesson053KnownIrEmission \
	Lesson054IrTranslator \
	Lesson055ClueConstraintModel \
	Lesson056FaultAwareOperatorPanel \
	Lesson057InertEscapeConsole \
	Lesson058MultiplexedDigits \
	Lesson059Max7219Presentation \
	Lesson060DualDisplayTimingDesk \
	Lesson061ResistiveProbeObservation \
	Lesson062ThermalRadiantObservation \
	Lesson063MuseumCaseMonitor \
	Lesson064OwnedSingleWireTransactions \
	Lesson065Qualified18B20ProbeSet \
	Lesson066ThermalGradientMapper \
	Lesson067InertialRecordNormalization \
	Lesson068InertialRecordQualification \
	Lesson069InterchangeableMotionRecorder \
	Lesson070ThresholdDescriptor \
	Lesson071Characterization \
	Lesson072ModuleCharacterizationBench
PORT        ?=
BAUD        ?= 115200
SERIAL_LOG  ?= $(BUILD_DIR)/serial/monitor.log

HOST_CPPFLAGS += -Isrc -Itests/fake_arduino
HOST_CXXFLAGS += -std=c++17 -Os -flto
HOST_CXXFLAGS += -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections
HOST_CXXFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Werror
HOST_LDFLAGS  += -flto -Wl,--gc-sections

$(BUILD_MARKER):
	mkdir -p "$(BUILD_DIR)"
	touch "$@"
