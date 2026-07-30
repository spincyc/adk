# Components

Status meanings:

- **Host verified** — strict deterministic tests pass.
- **Host verified; bench open** — deterministic tests and the canonical Mega
  adapter pass their non-hardware gates; physical acceptance remains open.
- **Hardware experimental** — Mega build passes; bench card remains open.
- **Queued** — contract or curriculum placement only.

| Layer | Type | Status | Owns |
|---|---|---|---|
| Core | `Runtime`, `ResourceClaim`, `TimePoint`, `Status` | Host verified | Fixed registry and explicit time |
| Endpoint | `DigitalOutput` | Hardware experimental | One output pin |
| Endpoint | `DigitalInput` | Hardware experimental | One input pin |
| Component | `MonoLed` | Hardware experimental | One `DigitalOutput` |
| Component | `Button` | Hardware experimental | One `DigitalInput` |
| Behavior | `ReactionTimer` | Hardware experimental | No hardware; observes Button and time |
| Endpoint | `PwmOutput` | Hardware experimental | One PWM pin; shared default timer |
| Component | `RgbLed` | Hardware experimental | Three `PwmOutput` endpoints |
| Component | `PiezoSounder` | Hardware experimental | One pin and Timer2 |
| Behavior | `Simon` | Host verified | No hardware; observes input snapshots and time |
| Endpoint | `AnalogInput` | Hardware experimental | One analog-capable pin |
| Value behavior | `LinearCalibration`, `MovingAverage`, `Deadband` | Host verified | Fixed-size sample state |
| Behavior | `NightLight` | Host verified | No hardware; observes normalized samples |
| Component | `ShiftRegisterOutput` | Hardware experimental | Three digital outputs |
| Component | `SevenSegmentDisplay` | Hardware experimental | One shift-register output |
| Behavior | `TrafficJunction` | Host verified | No hardware; observes request, health, and time |
| Contract | `ClimateSensor`, `ClimateSample` | Host verified | No hardware; validated timestamped climate observations |
| Component | `Dht11Sensor` | Hardware experimental | One bidirectional data pin |
| Component | `CharacterDisplay` | Hardware experimental | Six digital output endpoints |
| Behavior | `EnvironmentalStation` | Host verified | No hardware; observes climate, controls, and time |
| Component | `Keypad`, `MatrixKeypad` | Hardware experimental | Four row outputs and three column inputs |
| Behavior | `BoundedServo` | Host verified | No hardware; maps bounded calibrated intent |
| Value codec | `ServoConfigurationRecord` | Host verified | Fixed versioned record; no storage transport |
| Endpoint | `ServoOutput` | Hardware experimental | D44/OC5C, Timer5, and a power-admission gate |
| Behavior | `AccessTrainer` | Host verified | No hardware; observes key, health, and time snapshots |
| Endpoint | `PulseInput` | Hardware experimental | One pulse-observation pin |
| Component | `UltrasonicRanger` | Hardware experimental | Trigger output plus pulse input |
| Behavior | `MotorIntent` | Host verified | No hardware; observes requests, faults, and time |
| Behavior | `RoverController` | Host verified | No hardware; observes range, route, and time |
| Endpoint | `I2cBus`, `I2cDevice` | Hardware experimental | Mega TWI pins and one fixed address |
| Endpoint | `SpiBus`, `SpiDevice` | Hardware experimental | Mega SPI pins and one chip-select |
| Component | `MoistureSensor` | Hardware experimental | One analog input and explicit calibration |
| Contract | `Rtc`, `Storage` | Host verified | Borrowed clock/media adapters |
| Component | `FixedStorage` | Host verified | Fixed staged and durable record prefixes |
| Component | `IndicatorPump`, `InertLoadPanel` | Hardware experimental | Three borrowed resistor-LED channels |
| Behavior | `WateringController` | Host verified | Moisture samples, output intent, and supplied time |
| Behavior | `GreenhouseController` | Host verified | Borrowed sensor, output, display, record sink, and supplied time |
| Component | `GreenhouseHealthPattern` | Hardware experimental | One RGB evidence LED |
| Adapter | `StorageRecordSink` | Host verified | Borrowed storage with append/sync durability |
| Endpoint | `PulseCapture`, `MegaPulseCaptureIo` | Hardware experimental | One interrupt-capable pin and interrupt line |
| Behavior | `InfraredDecoder` | Host verified | Immutable classic NEC pulse evidence |
| Value codec | `InfraredRecord` | Host verified | Fixed receive-only observation record |
| Value codec | `TelemetryPacketCodec` | Host verified | Exact 19-byte ADK-owned receive-only packet |
| Behavior | `ObservationTracker` | Host verified | Local receipt time, sequence, quality, and freshness |
| Adapter | `PacketReceiver` | Host verified | Fixed-capacity caller-supplied packet queue |
| Behavior | `TelemetryEvidenceModel` | Host verified | Visible packet and freshness evidence |
| Value codec | `CueAuditBuffer`, `CueAuditEncoder` | Host verified | Caller-owned bounded records and stable text |
| Behavior | `InertCueScheduler` | Host verified | Supplied time, confirmation, holds, and inert snapshots |
| Behavior | `InertShowSimulator` | Host verified | Borrowed assessor, scheduler, audit, complete observation frames, and supplied time |
| Input expansion | `AnalogJoystick`, `QuadratureEncoder`, `CalibrationConsole` | Host verified; bench open | Lessons 031–033 |
| Magnetic observation | `LinearHall`, `MagneticContact` | Host verified; exact specimen and bench open | Lesson 034 |
| Passage policy | `PassageQualifier` | Host verified; exact specimen and bench open | Lesson 035 |
| Durable passage project | `TwoSlotPassageLedger`, `MagneticPassageLogger` | Host verified; exact specimen and bench open | Lesson 036 |
| Contact policy | `ContactDynamics` | Host verified; incoming conformance and bench open | [Lesson 037](lessons/037.md) |
| Acoustic policy | `AcousticEnvelope` | Host verified; incoming conformance and bench open | [Lesson 038](lessons/038.md) |
| Percussion project | `PercussionSequencer` | Host verified; incoming conformance and bench open | [Lesson 039](lessons/039.md) |
| Optical observation policy | `ReflectiveObservationPolicy`, `BeamObservationPolicy` | Host verified; powered adapter and bench open | [Lesson 040](lessons/040.md) |
| Presence composition | `PresenceModel` | Host verified; powered adapter and bench open | [Lesson 041](lessons/041.md) |
| Course-marshal project | `CourseMarshal` | Host verified; powered adapter and bench open | [Lesson 042](lessons/042.md) |
| Inertial observation policy | `InertialObservationPolicy` | Host verified; E0 replay only | [Lesson 043](lessons/043.md) |
| Orientation presentation | `OrientationPolicy`, `BalancePresentationPolicy` | Host verified; E0 replay only | [Lesson 044](lessons/044.md) |
| Balance-table project | `BalanceInstrument` | Host verified; E0 replay only | [Lesson 045](lessons/045.md) |
| Interaction intent policy | `InteractionIntentPolicy` | Host verified; E0 replay only | [Lesson 046](lessons/046.md) |
| Bounded logical step sequencing | `BoundedStepperSequence` | Host verified; E0 replay only | [Lesson 047](lessons/047.md) |
| Kinetic-light project | `KineticLightSculpture` | Host verified; E0 replay only | [Lesson 048](lessons/048.md) |
| Local identity registry | `LocalIdentityRegistry` | Host verified; E0 replay only | [Lesson 049](lessons/049.md) |
| Bounded logical homing | `BoundedHomingPolicy` | Host verified; E0 replay only | [Lesson 050](lessons/050.md) |
| Inert parts-carousel project | `InertPartsCarousel` | Host verified; E0 replay only | [Lesson 051](lessons/051.md) |
| Captured infrared evidence | `CapturedIrEvidence` | Host verified; E0 replay only | [Lesson 052](lessons/052.md) |
| Known infrared emission policy | `KnownIrEmissionPolicy` | Host verified; E0 replay only | [Lesson 053](lessons/053.md) |
| Inert IR translator project | `InertIrTranslator` | Host verified; E0 replay only | [Lesson 054](lessons/054.md) |
| Clue constraint policy | `ClueConstraintModel` | Host verified; E0 replay only | [Lesson 055](lessons/055.md) |
| Fault-aware operator panel | `FaultAwareOperatorPanel` | Host verified; E0 replay only | [Lesson 056](lessons/056.md) |
| Inert escape-console project | `InertEscapeConsole` | Host verified; E0 replay only | [Lesson 057](lessons/057.md) |
| Multiplexed digit policy | `MultiplexedDigitPolicy` | Host verified; E0 intent only | [Lesson 058](lessons/058.md) |
| MAX7219 presentation policy | `Max7219PresentationPolicy` | Host verified; E0 intent only | [Lesson 059](lessons/059.md) |
| Dual-display timing desk | `DualDisplayTimingDesk` | Host verified; E0 intent only | [Lesson 060](lessons/060.md) |
| Resistive-probe observation policy | `ResistiveProbeObservationPolicy` | Host verified; E0 copied evidence only; E1a open | [Lesson 061](lessons/061.md) |
| Thermal/radiant observation policy | `ThermalRadiantObservationPolicy` | Host verified; E0 copied evidence only; E1b open | [Lesson 062](lessons/062.md) |
| Inert museum-case monitor | `MuseumCaseMonitor` | Host verified; E0 inert intent only; E1c/E1d/E2 open | [Lesson 063](lessons/063.md) |
| Bounded copied 1-Wire transactions | `OneWireTransactionPolicy` | Host verified; E0 semantic line intent only; E1a/E1b open | [Lesson 064](lessons/064.md) |
| Qualified four-probe 18B20 set | `Qualified18B20ProbeSetPolicy` | Host verified; E0 copied evidence only; E1a–E1c open | [Lesson 065](lessons/065.md) |
| Thermal-gradient mapper | `ThermalGradientMapper` | Host verified; E0 copied evidence and volatile intent only; E1a–E1d open | [Lesson 066](lessons/066.md) |
| Normalized inertial record | `InertialRecordNormalizer`, `InertialRecordCodec` | Host verified; E0 copied evidence only; E1a–E1c open | [Lesson 067](lessons/067.md) |
| Configured inertial-record qualification | `InertialRecordQualificationPolicy` | Host verified; E0 copied evidence only; E1a–E1c open | [Lesson 068](lessons/068.md) |
| Qualified motion recorder | `QualifiedMotionRecorder`, `MotionRecordCodec` | Host verified; E0 volatile copied evidence and intent only; exact resource review complete; E1a–E1c open | [Lesson 069](lessons/069.md) |
| Planned characterization arc | Declared descriptors, copied sweeps, and characterization-result intent | Queued; exact fixtures E1-open | Lessons 070–072 |
| Reserved arcs | Authorized-family replacements pending | Re-scope required | Lessons 073–078 |
| Planned qualification arc | Bounded load driver and indicator semantics | Queued | Lessons 079–081 |

Composition is preferred: a Button has an input; it is not a specialized pin.
Behavior engines expose output intent rather than hiding hardware callbacks.
`Simon` therefore owns neither buttons nor cue devices: an adapter translates
one complete button observation into `SimonInput`, then maps `SimonSnapshot`
onto LEDs, RGB feedback, and sound.

`NightLight` follows the same boundary. Its Mega adapter owns the sensor, lamp,
and diagnostic LED, while the engine only accepts a complete observation and
returns output intent.

The same composition continues through lessons 010–021. A
`SevenSegmentDisplay` owns its serialized output; `TrafficJunction` instead
owns no pins and returns a complete legal signal pattern for its Mega adapter.
`Dht11Sensor` owns one bidirectional data pin while presenting validated,
timestamped climate samples through the transport-neutral `ClimateSensor`
contract. `BoundedServo` produces calibrated pulse intent while `ServoOutput`
owns the Timer5 endpoint; the external-power gate admits commands but is not
evidence that load power is present or disconnected.

`AccessTrainer` keeps the lesson 018 circuit inert: it returns visible
soft-latch and audit intent without driving a servo or storing credentials.
`UltrasonicRanger` separates pulse timing from distance validity.
`MotorIntent` and `RoverController` remain hardware-neutral policy engines;
the lesson 021 Mega stage presents requested and applied motion on LEDs, not
motors.

Lesson 022 owns I2C addresses and SPI chip-selects for each device. Transactions
are bounded and restore controller state. `MoistureSensor` keeps calibration
and sample validity explicit; `Rtc` and `FixedStorage` model clock state,
append, sync, failure, and restart without claiming a physical RTC or SD-card
adapter.

Lessons 023--029 continue with inert load policy, greenhouse composition,
receive-only infrared and telemetry evidence, a deterministic telemetry
console, inert-channel assessment, and deterministic cue scheduling with a
bounded replayable audit. Lesson 030 composes the assessor and scheduler with
an explicit plan-position-to-channel map; incomplete evidence cannot authorize
a transition, and contradictory evidence faults the composition.

Lessons 037--039 keep sensing and sequencing policy separate from their
adapters. `ContactDynamics` qualifies timestamped contact samples without
owning an input pin. `AcousticEnvelope` qualifies timestamped envelope and
optional threshold observations without owning the microphone board.
`PercussionSequencer` consumes complete contact and acoustic observations and
returns deterministic pattern and cue intent; it owns no pins or sounder.
Their documented external reference fixtures make the Mega circuits
reproducible, but incoming conformance and every E1 bench record remain open.

Lessons 040--042 keep source qualification, presence composition, and run
policy independent of pins and specimens. `ReflectiveObservationPolicy` and
`BeamObservationPolicy` retain source identity, calibration revision,
transitions, and faults.
`PresenceModel` combines copied PIR, beam, and range evidence without allowing
PIR motion to authorize a run. `CourseMarshal` starts only from a debounced
explicit button action and produces fixed-capacity, replayable checkpoint and
finish evidence. Powered adapters, exact wiring, and every E1 bench record
remain open.

Lessons 043--045 keep copied inertial values, gravity-relative orientation,
and balance-table composition independent of sensors and output hardware.
`InertialObservationPolicy` preserves source, range, revision, readiness,
sequence, age, saturation, and producer status. `OrientationPolicy` maps an
explicit right-handed board frame into fixed-point pitch and roll, while
`BalancePresentationPolicy` returns bounded light and tone intent.
`BalanceInstrument` atomically composes copied inertial, joystick, and button
records; only a qualified button event can freeze evidence. These E0 policies
own no bus, pin, clock, sensor, actuator, or presentation endpoint. Their Mega
sketches are compile-only memory replays, not powered or physical evidence.

Lessons 046--048 keep copied interaction evidence, logical step intent, and
kinetic-light composition independent of live inputs and energized motion.
`InteractionIntentPolicy` transactionally combines copied contact and
directional evidence. `BoundedStepperSequence` advances bounded logical
position and coil intent from explicit commands and supplied time.
`KineticLightSculpture` atomically composes both policies with an independent
stop path and semantic light intent. These E0 policies own no GPIO, clock,
timer, interrupt, motor driver, coil, supply, or physical position. Exact
tactile and directional adapters remain E1-gated; exact stepper, ULN2003,
power, restraint, and moving-sculpture acceptance remain E2-gated.

Lessons 049--051 keep copied identity, key, home, stop, and presentation
evidence separate from endpoints and media. `LocalIdentityRegistry` validates
fixed caller-owned record images and exposes explicit external-commit
reconciliation. `BoundedHomingPolicy` publishes at most one bounded semantic
step request per accepted frame and never treats logical position as measured
position. `InertPartsCarousel` composes both policies with a private logical
step sequencer, closed/open gate intent, and paired audit records. E0 owns no
RFID, keypad, switch, display, storage transport, driver, motor, servo, or
supply. Exact inert endpoints and media remain E1-gated; energized motion and
gate actuation remain E2-gated.

- [Exact API](api-supported.md)
- [Full component catalog](docs/COMPONENTS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Deterministic testing](docs/TESTING.md)
