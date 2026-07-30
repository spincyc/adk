# Supported API

> **Experimental:** these are ADK's first-class interfaces, but they are not
> release-stable until their host tests, Mega 2560 examples, and hardware
> acceptance checks pass. Expect source changes before version 1.0.

Use these interfaces for new work. The imported API is isolated under
[`legacy/`](legacy/index.md) and receives compatibility fixes only.

## Include and ownership model

Include only the headers a translation unit uses. A `Runtime` owns the resource
registry shared by endpoints:

```cpp
#include <digital_output.h>
#include <runtime.h>

adk::Runtime       runtime;
adk::DigitalOutput debugLed (runtime.resources (), LED_BUILTIN);
```

Endpoints and components are exclusive, non-copyable, and non-movable. Keep the
runtime alive longer than every object that uses it. ADK uses static storage
only; initialization does not allocate memory.

## Status

Operations report a complete `adk::Status` value, never exceptions:

| `StatusCode` from `error()` | Meaning |
|---|---|
| `Ok` | Operation completed |
| `InvalidArgument` | Configuration is internally invalid |
| `InvalidConfiguration` | A complete component configuration is invalid |
| `InvalidPin` | Pin is outside the supported board profile |
| `Unsupported` | Hardware cannot provide the requested capability |
| `ResourceBusy` | Another live object owns the resource |
| `NotInitialized` | An operation requires successful initialization |
| `CapacityExceeded` | Fixed-capacity storage is full |
| `HardwareFailure` | The hardware operation failed |
| `Timeout` | A bounded operation or confirmation window expired |
| `InternalInvariant` | A state-machine invariant was not preserved |

Use `status.ok()` for normal control flow. `error()` exposes the
`StatusCode` when diagnostics or a specific recovery path needs it.
`transient()` marks a potentially recoverable cause; it is not an instruction
to retry. Pass the complete `Status` object to logging and policy code so it can
also use `statusName(status)`.

`Result<T>` couples a complete status with a value. Check `ok()` before
`value()`. Use `status()` to pass the complete failure onward, `error()` to
inspect its code, and `transient()` to classify its cause.

## Lifecycle

Every owning endpoint or component follows the same contract:

```cpp
adk::Status status = component.initialize ();

if (!status.ok ())
{
    // Report the fault without using the component.
}

component.shutdown ();
```

- Construction is inert.
- `initialize()` acquires every resource or rolls back the attempt.
- Repeated `shutdown()` is safe.
- Destruction calls `shutdown()`.
- Cleanup is `noexcept`; ADK does not throw internally and remains safe during
  stack unwinding in an exception-enabled program.
- Use the object only while `initialized()` is true.

There is no global dispatcher. The application owns objects and supplies time
explicitly to stateful components.

## Digital output

`DigitalOutput` is the first diagnostic endpoint. Initialization claims its pin,
sets the output latch to the configured initial level, and only then enables
output mode. Shutdown returns the pin to high impedance before releasing it.

```cpp
adk::Runtime       runtime;
adk::DigitalOutput probe (runtime.resources (),
                          LED_BUILTIN,
                          adk::Level::Low);

void setup ()
{
    const adk::Status status = probe.initialize ();

    if (status.ok ())
    {
        probe.write (adk::Level::High);
    }
}

void loop ()
{
}
```

`write()` returns `NotInitialized` before successful initialization.
`pin()` and `level()` expose configuration and the last accepted level for
diagnostics. A generic endpoint does not know whether a circuit is active-high
or active-low; semantic components must define their own inactive state.

## Digital input

`DigitalInput` owns one input pin. `Pull::Up` selects the Mega 2560 internal
pull-up; `Pull::None` requires an external circuit that never leaves the input
floating.

```cpp
adk::DigitalInput input (runtime.resources (), 7, adk::Pull::Up);

if (input.initialize ().ok ())
{
    input.update ();
    const adk::Level stableSnapshot = input.read ();
    const adk::Level immediateLevel = input.sample ();
}
```

`update()` refreshes the cached value returned by `read()`. `sample()` performs
an immediate hardware read and is intended for diagnosis. Neither electrical
level is a semantic button state.

## Button

`Button` composes a `DigitalInput`. The default circuit uses an internal pull-up
and a switch to ground, so `Low` means pressed.

```cpp
adk::ButtonConfig config {
    7,
    adk::Pull::Up,
    adk::Level::Low,
    adk::Duration (20)
};

adk::Button button (runtime.resources (), config);
bool        buttonReady = false;

void setup ()
{
    buttonReady = button.initialize ().ok ();
}

void loop ()
{
    if (!buttonReady)
    {
        return;
    }

    button.update (adk::TimePoint (millis ()));

    if (button.pressEvent ())
    {
        probe.write (adk::Level::High);
    }

    if (button.releaseEvent ())
    {
        probe.write (adk::Level::Low);
    }
}
```

Call `initialize()` successfully before `update()`. `rawPressed()` supports
wiring and bounce diagnosis. `pressed()` is debounced. `pressEvent()` and
`releaseEvent()` are non-consuming snapshots for the current update; the next
update replaces them. A press must be released before another is accepted.
Composition logic, not scan order, decides how simultaneous buttons behave.

`TimePoint` and `Duration` use unsigned 32-bit milliseconds. Elapsed-time
calculation remains correct across timer wrap for intervals shorter than half
the counter range.

## Bounded servo

`BoundedServo` is a hardware-neutral calibrated intent model.
`ServoConfigurationRecord` encodes a fixed, versioned configuration value; it
does not provide EEPROM storage or torn-write recovery. `ServoOutput` owns the
Mega 2560 D44/OC5C endpoint and Timer5.

An `ExternalPowerDomainGate` expresses whether software may issue a pulse. It
does not detect voltage, prove isolation, or switch the servo supply. Keep the
servo-positive lead physically open while verifying the initial safe waveform
at TP-S, then admit load power only under the lesson 017 E2 bench procedure.

[Lesson 017](lessons/017.md) gives the complete lifecycle and evidence order.

## PWM output and RGB LED

`PwmOutput` has the same inert construction and RAII cleanup contract as
`DigitalOutput`, but accepts an 8-bit duty value. It rejects valid non-PWM Mega
pins with `Unsupported`. Default-frequency channels share their timer lease;
frequency-changing components must claim a timer exclusively.

```cpp
adk::RgbLed led (runtime.resources (),
                 {6, 220},
                 {5, 220},
                 {3, 220});

if (led.initialize ().ok ())
{
    led.set (adk::Rgb (255, 64, 0));
}
```

`RgbLed` composes three PWM endpoints transactionally. The current component is
for a common-cathode LED: zero is inactive. Each `RgbLedChannel` records its pin
and series-resistor value.

## Piezo sounder

`PiezoSounder` owns one pin and Timer2. `play()` replaces any current tone and
stores the caller-provided start time; `update()` stops it at the deterministic
deadline.

```cpp
sounder.play
(
    440,
    adk::Duration  (250),
    adk::TimePoint (millis ())
);
```

Supported frequencies are 31–20,000 Hz and durations are 1–60,000 ms. The
component does not queue notes or call `delay()`. Direct Arduino `tone()` or
timer-library calls bypass ADK ownership and must not be mixed with it.

## Analog input

`AnalogInput` owns one Mega analog-capable pin. `initialize()` validates board
capability before claiming the pin, configures it as an input, and records an
initial reading. `update()` refreshes the cached value returned by `read()`;
`sample()` performs an immediate ADC read for diagnosis.

```cpp
adk::AnalogInput potentiometer (runtime.resources (), 54); // Mega A0

if (potentiometer.initialize ().ok ())
{
    potentiometer.update ();
    const adk::AnalogInput::Reading position = potentiometer.read ();
}
```

Readings are clamped to the Mega's 10-bit range, 0–1023. They are dimensionless
ADC counts, not volts or a semantic sensor value. The circuit and reference
voltage determine their meaning. Lesson 007 names TP1 between the potentiometer
wiper and A0 so the electrical input can be measured independently of software.

## Sampled signal processing

Sample processing is hardware-neutral and advances only when the application
supplies a value:

- `LinearCalibration` maps one observed range into another, with optional
  clamping and support for descending output.
- `MovingAverage` keeps a fixed window of at most 32 samples and exposes its
  warm-up count.
- `Deadband` holds its last value until the supplied sample differs by the
  configured width.

```cpp
const adk::LinearCalibrationConfig calibrationConfig = {
    100, 900, 0, 1000, true};

adk::LinearCalibration calibration (calibrationConfig);
adk::MovingAverage     average     (8);
adk::Deadband          deadband    (10);

const adk::Result<uint16_t> mapped = calibration.map (rawSample);

if (!mapped.ok ())
{
    showSensorFault (mapped.status ());
    return;
}

const adk::Result<uint16_t> smooth = average.addSample (mapped.value ());

if (!smooth.ok ())
{
    showSensorFault (smooth.status ());
    return;
}

const uint16_t stable = deadband.addSample (smooth.value ());
```

Check each `Result` before using `value()`. `reset()` clears accumulated filter
state so the same sample sequence can be replayed from the same initial state.
These types allocate no heap memory, read no pins, and consult no hidden clock.

## Adaptive night light

`NightLight` is a hardware-neutral decision engine. The application calibrates
and filters its sensor observation, passes one normalized `NightLightInput`,
then maps the resulting snapshot to a PWM lamp and RGB diagnostic output.

The default configuration turns on strictly below 350 permille and turns off
strictly above 450 permille. This hysteresis prevents chatter inside the band.
While active, darker observations map to greater duty within the configured
24–192 default range.

`LightSampleState` distinguishes `Valid`, `BelowRange`, `AboveRange`, and
`Stale`. A sensor fault enters `NightLightMode::Fault`, reports
`NightLightDiagnostic::SensorFault`, and requests duty zero. A normalized input
above 1000 is invalid and also requests duty zero.

The snapshot exposes mode, sample state, diagnostic state, status, normalized
light, requested duty, and `lampOn`. The engine owns no hardware; the adapter
keeps the PWM lamp off if initialization or actuation fails.

## Shift register and seven-segment display

`ShiftRegisterOutput` composes three `DigitalOutput` endpoints for the data,
clock, and latch lines of a serial-in, parallel-out register. Initialization
claims all three pins transactionally and presents the configured inactive byte.
`show()` shifts the most-significant bit first and latches the complete byte;
callers never observe a deliberately half-latched value.

```cpp
const adk::ShiftRegisterPins displayPins = {22, 23, 24};

adk::ShiftRegisterOutput registerOutput
(
    runtime.resources (),
    displayPins
);

if (registerOutput.initialize ().ok ())
{
    registerOutput.show (0x3FU);
}
```

`clear()` presents zero. Shutdown first presents `inactiveValue()`, then returns
the three pins to high impedance in reverse acquisition order.

`SevenSegmentDisplay` gives those bits circuit meaning. It supports
common-cathode and common-anode displays, hexadecimal digits, dash, blank, and
an optional decimal point:

```cpp
adk::SevenSegmentDisplay display
(
    runtime.resources (),
    displayPins,
    adk::SevenSegmentPolarity::CommonCathode
);

display.initialize ();
display.show (adk::SevenSegmentGlyph::A);
display.blank ();
```

Every segment, including the decimal point, still requires its own resistor.
`encodedValue()` exposes the byte requested from the shift register so TP-data,
TP-clock, TP-latch, and the visible glyph can be interpreted as separate links
in the evidence chain. The three-wire interface does not own `/OE`; it cannot
promise that a physical register remains blank while the Mega itself powers up.

## Traffic junction engine

`TrafficJunction` is a hardware-neutral, nonblocking state machine. The
application observes a complete request and circuit-health snapshot, supplies
the current time, then actuates the returned complete signal pattern:

```cpp
const adk::TrafficInput observation
(
    pedestrianButton.pressEvent (),
    circuitHealthy
);

traffic.update (now, observation);

const adk::TrafficSnapshot decision = traffic.snapshot ();
```

`TrafficConfig` makes startup all-red, vehicle all-red, green, yellow,
pedestrian walk, and clearance durations explicit. The snapshot reports phase,
status, complete signals, phase start, next deadline, request state, transition
events, and transition count.

A request is release-gated by the `Button` before it reaches the engine and is
then retained until its legal pedestrian phase. `TrafficPhase::Fault` has no
deadline and presents all red with pedestrian stop. A false
`TrafficInput::circuitHealthy` enters that state immediately; the engine never
attempts hardware recovery or hides the fault behind a callback.

The engine owns no endpoints and cannot prove that a commanded LED is
electrically active. The Mega adapter separately indicates successful resource
acquisition on D13 and presents the complete vehicle and pedestrian pattern on
resistor-limited LEDs.

## Climate sensor and DHT11 adapter

`ClimateSensor` is the transport-neutral contract for timestamped, fixed-point
temperature and relative-humidity samples. Consumers inspect
`ClimateSample::state` before using either numeric field; unavailable,
transport-timeout, checksum, range, stale, and invalid-timing states remain
distinct.

`Dht11Sensor` implements that contract while exclusively owning one
bidirectional Mega pin. Construction is inert. The first `update(now)` anchors
the stabilization interval, later updates enforce the one-second acquisition
cadence, and `shutdown()` returns the data line to high impedance.

```cpp
adk::Dht11Sensor sensor (runtime.resources (), 22);

if (sensor.initialize ().ok ())
{
    const adk::TimePoint now (millis ());

    sensor.update (now);

    const adk::ClimateSample observation =
        sensor.sample (now, adk::Duration (5000));
}
```

Tests inject the transport to replay pulse widths and faults without hardware.
The RGB health pattern and the D22 data test point provide separate
software-state and electrical-activity evidence.

## Character display and environmental station

`CharacterDisplay` owns six parallel HD44780 signal endpoints and stages a
fixed 16×2 presentation before committing it to the display. Initialization
and writes roll back or enter the documented inert state on endpoint failure.

`EnvironmentalStation` is hardware-neutral. It accepts complete climate
observations, controls, and supplied time, then returns stable display and
record intent with explicit sample age, extrema, and health. It does not read a
sensor, display, Serial stream, or hidden clock.

## Matrix keypad and access trainer

`Keypad` interprets release-gated key observations. `MatrixKeypad` owns four
row outputs and three column inputs and presents one complete scan result; scan
order never becomes application policy.

`AccessTrainer` consumes key, component-health, and supplied-time snapshots. It
returns display, audit, and inert soft-latch intent. It does not store a
credential, drive a servo, or claim to provide physical security. Lesson 018's
Mega circuit presents policy state on the LCD and a resistor-limited LED.

## Ultrasonic range, motor intent, and rover supervision

`PulseInput` records an explicitly supplied echo duration or timeout.
`UltrasonicRanger` converts that observation into a sample that keeps valid,
timeout, too-near, too-far, and hardware-fault outcomes distinct.

`MotorIntent` is a hardware-neutral policy engine for bounded duty, reversal
dead time, and stop-dominant faults. `RoverController` composes fresh range
samples, route commands, and supplied time into requested and applied motion
intent. The lesson 021 E1 circuit uses LEDs only; it does not drive motors.

## Simon engine

`Simon` is a hardware-neutral deterministic state machine. The application
samples all four buttons first, supplies one complete `SimonInput`, then maps
the returned `SimonSnapshot` to output components.

`FixedCueSource` supports exact test vectors.
`XorShift32CueSource` provides the versioned `XorShift32V1` sequence. The same
configuration, algorithm version, seed, timestamps, and input snapshots produce
the same states and outcomes.

The engine uses fixed-capacity storage and owns no hardware. Its cue source must
outlive it. Simultaneous presses are invalid independent of button scan order,
and a correct press is not complete until release.

## Inert cue scheduler and audit

`InertCueScheduler` copies a fixed plan and consumes only caller-supplied time
and `CueOperatorInput` values. The three-state `CueEvidenceGate` overload keeps
composition policy outside the scheduler: Permit allows progress, Hold keeps
the schedule inert without reporting an error, and Fault enters the scheduler's
terminal fault phase with the supplied non-OK status. The two-argument
`update()` overload is equivalent to Permit. Review is a level; Run, Confirm,
Skip, and Cancel are edges. Cancel dominates, releasing Review enters Held,
and no cue becomes Active without explicit confirmation. A delayed update
finishes at most the current cue and leaves its successor Waiting so that the
successor's evidence is checked on the next complete input frame.

`CueSchedulerSnapshot::phase` is authoritative. `hasCue` names a current or
next cue in Waiting, Confirmation, and Active; applications present a cue only
for Active. Cue IDs are opaque labels and never hardware addresses. `cueCount()`
and the bounds-checked `cue(index)` expose the copied plan for composition-time
validation; they do not transfer ownership or mutate schedule state.

`CueAuditBuffer` uses bounded caller-owned append storage. It never overwrites
records. `CueAuditEncoder` atomically emits the versioned `adk-cue,1` text
grammar into caller storage without allocation. The scheduler owns no pins,
clock, stream, callback, or generic output adapter. See
[Lesson 029](lessons/029.md) for the exact lifecycle, timing, circuit, and open
bench procedure.

## Inert show simulator

`InertShowSimulator` composes one `InertChannelAssessor`, one
`InertCueScheduler`, and their caller-owned `CueAuditBuffer`. Its copied
`InertCueChannelMap` maps plan positions to assessment channels explicitly;
cue IDs remain labels and are never interpreted as channel numbers.

Every `update()` supplies exactly eight uniquely identified observations from
one timestamp plus one complete operator snapshot. Input order is irrelevant:
the simulator canonicalizes observations by channel before assessment,
scheduling, and trace hashing. Open, short, stale, or unavailable evidence
holds the selected cue inert. Contradictory evidence faults the composition.
Cancel has precedence and can still move the scheduler to its safe terminal
state when an otherwise malformed new frame is supplied.

The simulator initializes the assessor before the scheduler, rolls back a
partial start, and shuts them down in reverse order. It owns no hardware and
does not own the borrowed components. The audit remains readable after
shutdown. `InertShowSnapshot` exposes the composition state and fault,
authoritative nested scheduler snapshot, selected-channel assessment when one
is relevant, audit count, status, and deterministic `traceDigest`. Replaying
the same valid timestamped frames from the same initial state produces the
same digest and audit records.

## Contact, acoustic, and percussion policy

`ContactDynamics` is a hardware-neutral policy for timestamped dry-contact
samples. It qualifies stable transitions, refractory timing, release, stuck
state, and timing faults without owning a `DigitalInput`. The Lesson 037 Mega
adapter supplies active-low observations from an external 10 kΩ pull-up and a
documented C&K/Littelfuse reference contact.

`AcousticEnvelope` accepts one complete timestamped analog-envelope
observation with an optional digital-threshold level. It performs startup
calibration, baseline tracking, event qualification, clipping detection, and
fault reporting without owning an `AnalogInput` or acoustic module. The
Lesson 038 adapter uses the documented SparkFun Sound Detector reference,
leaves `AUDIO` disconnected, and treats its `ENVELOPE` and active-high `GATE`
outputs as separate observations.

`PercussionSequencer` composes four contact observations, one acoustic
observation, tempo intent, and supplied time. Its fixed-capacity pattern,
record/play state, fault policy, and snapshots are deterministic; the engine
owns no pins, clock, LED, or sounder. The Lesson 039 adapter maps returned
intent onto resistor-limited visible evidence and a passive sounder.

[Lessons 037](lessons/037.md), [038](lessons/038.md), and
[039](lessons/039.md) provide the exact lifecycle, reference circuits, and
open acceptance procedures. Their non-hardware gates are host verified. That
host and Mega evidence does not constitute incoming-fixture conformance,
physical bench acceptance, or qualification of an optional kit substitution.

## Optical, presence, and course-marshal policy

`ReflectiveObservationPolicy` and `BeamObservationPolicy` consume copied
samples and own no endpoint or clock. Their snapshots retain source identity,
calibration revision, raw evidence, qualified state, stable duration,
transition events, quality, and status. Reflective policy makes range,
calibration, polarity, activation, and release explicit; beam policy makes
interruption polarity and dwell explicit.

`PirObservationPolicy` preserves warm-up, motion and clear qualification,
stuck-motion, source identity, timing, and fault state. `PresenceModel`
combines copied PIR, beam, reflective finish-guard, and timed range evidence.
It exposes per-source availability, age, validity, staleness, approach state,
passage, and disagreement. It does not sample hardware, hide a clock, or treat
PIR motion as authorization.

`CourseStartPolicy` accepts only the configured debounced button source and
requires valid PIR eligibility. `CourseMarshal` then consumes copied start,
presence, checkpoint, and supplied-time values. It retains a fixed maximum of
four ordered checkpoints in caller-owned storage, rejects skipped, reversed,
duplicate, simultaneous, premature-finish, timed-out, and faulty runs, and
preserves a replay identity for each accepted or rejected record.
`CourseMarshalPresenter` maps snapshots to display and visible evidence intent
without owning pins, LEDs, sounders, or displays.

[Lessons 040](lessons/040.md), [041](lessons/041.md), and
[042](lessons/042.md) provide the lifecycle, deterministic replay, and open
acceptance procedures. Their non-hardware gates are host verified. Powered
adapters, exact specimen qualification, wiring, electrical schematics, and E1
bench acceptance remain open.

- Optical policy: [source](https://github.com/spincyc/adk/blob/main/src/optical_observation.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_optical_observation.cpp),
  [Mega replay](downloads/sketches/Lesson040OpticalObservation.ino), and
  [Lesson 040](lessons/040.md)
- Presence policy: [source](https://github.com/spincyc/adk/blob/main/src/presence_model.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_presence_model.cpp),
  [Mega replay](downloads/sketches/Lesson041PresenceModel.ino), and
  [Lesson 041](lessons/041.md)
- Course policy: [source](https://github.com/spincyc/adk/blob/main/src/course_marshal.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_course_marshal.cpp),
  [Mega replay](downloads/sketches/Lesson042CourseMarshal.ino), and
  [Lesson 042](lessons/042.md)

The canonical Mega 2560 builds measure 5,112 bytes flash / 300 bytes static
RAM for Lesson 040, 8,818 / 226 for Lesson 041, and 15,596 / 901 for Lesson
042. These compile measurements are software evidence, not powered or physical
acceptance.

## Copied inertial, orientation, and balance-table policy

`InertialObservationPolicy` accepts caller-supplied six-axis fixed-point
values and preserves their provenance, revisions, ranges, timestamp, sequence,
readiness, saturation, age, and complete producer status. It classifies copied
evidence as current, stale, saturated, or invalid without owning a sensor, bus,
endpoint, callback, or clock.

`OrientationPolicy` validates that complete observation, applies one of the 24
right-handed `BoardFrame` mappings, and returns bounded fixed-point pitch and
roll. `BalancePresentationPolicy` maps the estimate and caller-supplied
sensitivity to light and tone intent; neither policy actuates hardware.

`BalanceInstrument` atomically composes copied inertial, joystick, button,
time, and sequence evidence. A qualified button press is the sole freeze
authority. The project retains separate live and frozen evidence, latches
producer and skew faults, and requires an acknowledged recovery sequence
before returning to live presentation.

[Lessons 043](lessons/043.md), [044](lessons/044.md), and
[045](lessons/045.md) provide the deterministic replay and open acceptance
procedures. Their host gates and compile-only Mega examples are verified at
E0. They do not provide MPU6050 or QMI8658 adapters, I2C transactions, wiring,
powered presentation, or physical measurements.

- Inertial policy: [source](https://github.com/spincyc/adk/blob/main/src/inertial_observation.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_inertial_observation.cpp),
  [Mega replay](downloads/sketches/Lesson043InertialObservation.ino), and
  [Lesson 043](lessons/043.md)
- Orientation policy: [source](https://github.com/spincyc/adk/blob/main/src/orientation_presentation.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_orientation_presentation.cpp),
  [Mega replay](downloads/sketches/Lesson044OrientationPresentation.ino), and
  [Lesson 044](lessons/044.md)
- Balance instrument: [source](https://github.com/spincyc/adk/blob/main/src/balance_table_instrument.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_balance_table_instrument.cpp),
  [Mega replay](downloads/sketches/Lesson045BalanceTableInstrument.ino), and
  [Lesson 045](lessons/045.md)

The canonical Mega 2560 builds measure 6,682 bytes flash / 949 bytes static
RAM for Lesson 043, 13,740 / 748 for Lesson 044, and 26,398 / 1,898 for Lesson
045. These compile measurements are software evidence, not powered or physical
acceptance.

## Copied interaction, bounded logical motion, and kinetic-light policy

`InteractionIntentPolicy` transactionally combines caller-supplied
`ContactSample` and `DirectionalEvidence` values. Its
`preview()` / `canCommit()` / `commit()` seam preserves source identity,
configuration revision, time, sequence, age, saturation, status, directional
hysteresis, and qualified touch events without owning an input endpoint.

`BoundedStepperSequence` accepts explicit `StepperCommand` values and supplied
time. The same transactional seam advances a bounded logical position and
four-bit coil intent while enforcing command age, interval, position, cancel,
replacement, and stop rules. Logical position and coil intent are replayable
policy outputs; they are not measured shaft position or energized coils.

`KineticLightSculpture` owns both policies by value and accepts complete
`SculptureInput` frames. It binds qualified interaction authorization to one
bounded motif, keeps an independent stop path dominant, and publishes
`SculptureSnapshot` with semantic light intent. Malformed frames reject
atomically; admitted producer or motion faults latch an all-off fault state.

[Lessons 046](lessons/046.md), [047](lessons/047.md), and
[048](lessons/048.md) provide deterministic host replay and compile-only Mega
examples at E0. Exact tactile and directional adapters, powered GPIO, and
indicator acceptance remain E1-gated. The exact stepper and driver, coil
power, restraint, independent power removal, moving mechanism, and physical
acceptance remain E2-gated.

- Interaction policy: [source](https://github.com/spincyc/adk/blob/main/src/interaction_intent_policy.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_interaction_intent_policy.cpp),
  [Mega replay](downloads/sketches/Lesson046InteractionIntent.ino), and
  [Lesson 046](lessons/046.md)
- Logical step policy: [source](https://github.com/spincyc/adk/blob/main/src/bounded_stepper_sequence.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_bounded_stepper_sequence.cpp),
  [Mega replay](downloads/sketches/Lesson047BoundedStepperSequence.ino), and
  [Lesson 047](lessons/047.md)
- Kinetic-light project: [source](https://github.com/spincyc/adk/blob/main/src/kinetic_sculpture.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_kinetic_sculpture.cpp),
  [Mega replay](downloads/sketches/Lesson048KineticLightSculpture.ino), and
  [Lesson 048](lessons/048.md)

The canonical Mega 2560 builds measure 6,956 bytes flash / 733 bytes static
RAM for Lesson 046, 8,068 / 1,053 for Lesson 047, and 22,216 / 1,470 for Lesson
048. These compile measurements are software evidence, not powered or physical
acceptance.

## Local identity, bounded homing, and inert carousel policy

`LocalIdentityRegistry` admits copied `IdentityEvidence`, matches fixed local
bindings, bounds failed attempts and lockout, and exports a caller-owned
candidate image. Enrollment becomes current only after synchronized,
reread-validated, byte-matching external-commit evidence; that protocol is not
a claim about any physical medium.

`BoundedHomingPolicy` consumes copied home and independent-stop evidence with
explicit commands and time. It releases an initially active home indication,
requires a qualified acquisition edge, and emits at most one signed semantic
step request per accepted frame. Its session-local logical coordinate is not
measured shaft position.

`InertPartsCarousel` coordinates a borrowed identity registry and homing policy
with one private logical step sequencer. Exact confirmation and an acknowledged
start-audit image gate homing, bounded positioning, semantic gate intent, and
paired terminal reconciliation. Independent stop clears coil and gate intent;
the project owns no endpoint, media transport, actuator, supply, or mechanism.

[Lessons 049](lessons/049.md), [050](lessons/050.md), and
[051](lessons/051.md) provide deterministic host replay and compile-only Mega
examples at E0. Exact RFID, keypad, home, stop, display, indicator, and media
adapters remain E1-gated. Exact motor, driver, servo, actuator power,
restraint, independent power removal, and physical acceptance remain E2-gated.

- Local identity registry: [source](https://github.com/spincyc/adk/blob/main/src/local_identity_registry.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_local_identity_registry.cpp),
  [Mega replay](downloads/sketches/Lesson049LocalIdentityRegistry.ino), and
  [Lesson 049](lessons/049.md)
- Bounded homing policy: [source](https://github.com/spincyc/adk/blob/main/src/bounded_homing_policy.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_bounded_homing_policy.cpp),
  [Mega replay](downloads/sketches/Lesson050BoundedHomingPolicy.ino), and
  [Lesson 050](lessons/050.md)
- Inert parts carousel: [source](https://github.com/spincyc/adk/blob/main/src/inert_parts_carousel.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_inert_parts_carousel.cpp),
  [Mega replay](downloads/sketches/Lesson051InertPartsCarousel.ino), and
  [Lesson 051](lessons/051.md)

The canonical Mega 2560 builds measure 7,026 bytes flash / 1,113 bytes static
RAM for Lesson 049, 8,272 / 574 for Lesson 050, and 26,014 / 1,933 for Lesson
051. These compile measurements are software evidence, not powered, durable,
or physical acceptance.

## Copied IR evidence, known emission intent, and inert translation

`CapturedIrEvidence` synchronously copies one published Lesson 025 pulse frame
into caller-owned storage and preserves its source, configuration, sequence,
observation time, shape, and integrity classification. Known commands,
repeats, unknown shapes, malformed timing, overflow, and source faults remain
receive evidence; none grants transmission authority.

`KnownIrEmissionPolicy` accepts only an immutable firmware-authored
`LocalIrCodeId`. Its transactional preview publishes bounded
`CarrierOn`/`CarrierOff` envelope intent against explicit time, catalog
revision, and catalog digest. It accepts no pulse frame, decoded raw command,
duration array, arbitrary bytes, or learned entry.

`InertIrTranslator` composes those policies through one fixed allowlist. Only
valid attributable receive evidence may map to a different local catalog
symbol. Repeat, unknown, malformed, self-echo, stale, and arbitrary captured
evidence cannot reach the emission policy.

[Lessons 052](lessons/052.md), [053](lessons/053.md), and
[054](lessons/054.md) publish a host-verified E0 boundary with deterministic
host replay and compile-only Mega examples. This is not powered or optical
support. The arc owns no receiver, emitter,
pin, timer, interrupt, carrier endpoint, optical power path, or controlled
device. Exact fixtures, resource allocation, authoritative schematic,
acquisition and safe-state evidence, and physical acceptance remain E1-gated.

- Captured IR evidence: [source](https://github.com/spincyc/adk/blob/main/src/captured_ir_evidence.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_captured_ir_evidence.cpp),
  [Mega replay](downloads/sketches/Lesson052CapturedIrEvidence.ino), and
  [Lesson 052](lessons/052.md)
- Known IR emission policy: [source](https://github.com/spincyc/adk/blob/main/src/known_ir_emission_policy.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_known_ir_emission_policy.cpp),
  [Mega replay](downloads/sketches/Lesson053KnownIrEmission.ino), and
  [Lesson 053](lessons/053.md)
- Inert IR translator: [source](https://github.com/spincyc/adk/blob/main/src/inert_ir_translator.h),
  [deterministic tests](https://github.com/spincyc/adk/blob/main/tests/test_inert_ir_translator.cpp),
  [Mega replay](downloads/sketches/Lesson054IrTranslator.ino), and
  [Lesson 054](lessons/054.md)

Fresh Mega 2560 measurements for this published E0 boundary are 5,530
bytes flash / 1,096 bytes static SRAM for Lesson 052, 4,854 / 276 for Lesson
053, and 16,162 / 1,343 for Lesson 054. The maximum E0 composition measures
21,864 bytes flash and 3,531 bytes static SRAM. Its conservative compiler
call-chain stack estimate is approximately 888 bytes, leaving 3,773 bytes of
Mega SRAM after static storage and that estimate. The stack result misses the
800-byte target but passes the 1,024-byte hard limit; it is compiler evidence,
not a runtime high-water measurement.

On the AVR ABI, `CapturedIrEvidence` is 48 bytes,
`KnownIrEmissionPolicy` is 74 bytes, and `InertIrTranslator` is 406 bytes.
Lesson 054 deliberately keeps its required 100-word copied-pulse buffer in
caller-owned storage: the buffer is exactly 400 bytes, so the translator plus
that buffer is 806 bytes without embedding or duplicating the buffer inside
the coordinator. These are software resource measurements, not
powered-operation, optical-output, or bench evidence.

## Bounded copied 1-Wire transactions

`OneWireTransactionPolicy` is the host-verified Lesson 064 E0 policy for a
closed set of typed 1-Wire operations. It expands one request into bounded
semantic `DriveLow`, `Release`, and `Sample` intents, then accepts only fully
correlated copied receipts. It is not a hardware bus or GPIO adapter and owns
no pin, pull-up, timer, interrupt, clock, supply, callback, or heap allocation.

The admitted operations are reset/presence, one bounded Search ROM pass,
single-drop Read ROM, and addressed power-supply, conversion-start,
single-status-read, and scratchpad-read transactions. Arbitrary opcodes,
captured pulse scripts, unknown-protocol replay, unbounded polling, EEPROM
copy, and parasite-power operation are outside the published contract.

Initialization, reset, cancellation, timeout, producer failure, and shutdown
all preserve explicit release-request and release-confirmation state. The
policy cannot claim a physical idle voltage: `Release` is semantic intent, and
a copied cleanup receipt is only producer evidence. A future E1 endpoint must
separately prove open-drain-safe ownership, pull-up behavior, line voltage,
slot timing, rollback, and shutdown.

- One-wire policy:
  [source](https://github.com/spincyc/adk/blob/main/src/one_wire_transaction_policy.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/one_wire_transaction_policy.cpp),
  [core tests](https://github.com/spincyc/adk/blob/main/tests/test_one_wire_transaction_policy.cpp),
  [timing tests](https://github.com/spincyc/adk/blob/main/tests/test_one_wire_transaction_policy_timing.cpp),
  [search tests](https://github.com/spincyc/adk/blob/main/tests/test_one_wire_transaction_policy_search.cpp),
  [interrupt-pressure tests](https://github.com/spincyc/adk/blob/main/tests/test_one_wire_transaction_policy_interrupt.cpp),
  [Mega replay](downloads/sketches/Lesson064OwnedSingleWireTransactions.ino),
  and [Lesson 064](lessons/064.md)

The canonical Mega replay and the isolated no-LTO/resource probes are software
evidence only. Exact externally powered and parasite-powered endpoints remain
separate E1a and E1b qualification campaigns.

## Qualified 18B20 probe sets and thermal-gradient mapping

`Qualified18B20ProbeSetPolicy` is the Lesson 065 E0 policy for four configured
family-`0x28` identities. It consumes complete copied Lesson 064 transaction
evidence and preserves correlated discovery, conversion, scratchpad CRC,
resolution, range, step, freshness, disappearance, and replay state. It owns
no probe, bus, endpoint, clock, or powered fixture.

`ThermalGradientMapper` is the Lesson 066 E0 composition. It structurally
validates a complete caller-supplied probe-set image, projects two to four
selected identities into configured spatial order, widens adjacent
temperature intervals before classifying direction, and publishes explicit
probe, interval, and overall fault cells. Supplied controls advance bounded
page intent and may emit one volatile record-intent image. Structural
validation is not source authentication; the mapper neither drives a display
nor opens, writes, acknowledges, or recovers storage.

- Probe-set policy:
  [source](https://github.com/spincyc/adk/blob/main/src/qualified_18b20_probe_set_policy.h),
  [Mega replay](downloads/sketches/Lesson065Qualified18B20ProbeSet.ino), and
  [Lesson 065](lessons/065.md)
- Thermal mapper:
  [source](https://github.com/spincyc/adk/blob/main/src/thermal_gradient_mapper.h),
  [configuration tests](https://github.com/spincyc/adk/blob/main/tests/test_thermal_gradient_mapper_config.cpp),
  [gradient tests](https://github.com/spincyc/adk/blob/main/tests/test_thermal_gradient_mapper_gradient.cpp),
  [control tests](https://github.com/spincyc/adk/blob/main/tests/test_thermal_gradient_mapper_control.cpp),
  [record tests](https://github.com/spincyc/adk/blob/main/tests/test_thermal_gradient_mapper_record.cpp),
  [Mega replay](downloads/sketches/Lesson066ThermalGradientMapper.ino), and
  [Lesson 066](lessons/066.md)

The Lesson 066 canonical replay measures 16,662 bytes flash and 2,210 bytes
static SRAM. Exact no-LTO evidence measures 18,822/2,210 bytes, 855 bytes of
conservative synchronous stack, and a 448-byte mapper. These are software
resource measurements. Exact probes, electrical timing, thermal accuracy,
presentation, persistence, authentication, and E1a--E1d acceptance remain
open.

## Normalized inertial records

`InertialRecordNormalizer` is the Lesson 067 E0 component for one complete
copied `InertialSample`. It preserves source attribution, configured revisions,
declared ranges, observation time, sequence, readiness, saturation, producer
status, and all six source-frame values. It derives an explicit recorded,
not-ready, or source-fault state without acquiring, calibrating, rotating,
qualifying, comparing, or retaining a physical sensor.

`InertialRecordCodec` gives the value one canonical 64-byte image with explicit
field order, little-endian integers, reserved-zero fields, and CRC-16
integrity. Encoding and decoding stage complete candidates and leave caller
output unchanged on rejection. The checksum detects accidental corruption; it
does not authenticate a source or establish durable storage.

- Inertial record:
  [source](https://github.com/spincyc/adk/blob/main/src/inertial_record.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/inertial_record.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/test_inertial_record.cpp),
  [Mega replay](downloads/sketches/Lesson067InertialRecordNormalization.ino),
  and [Lesson 067](lessons/067.md)

This is copied E0 software evidence only. Exact MPU6050 and QMI8658 identities,
powered adapters, electrical behavior, mounting, calibration, source
presentation, persistence, and bench acceptance remain open
E1a--E1c work.

## Configured inertial-record qualification

`InertialRecordQualificationPolicy` is the Lesson 068 E0 component for one
explicitly configured stream of Lesson 067 records. It validates the complete
source and revision domain, applies one `SourceAxisMapping`, and accepts a
bounded stationary window only when every copied record passes readiness,
status, saturation, supplied-time, sequence, acceleration, and angular-rate
checks.

An attempt moves from `Idle` to `Collecting`, then terminal `Qualified` or
`Rejected`. The copied `InertialQualificationEvidence` binds the attempt and
lifecycle generations, mapping, record range, extrema, widened sums, means,
maximum age and gap, terminal source-frame record, mapped terminal record,
reason, and status. A byte-identical duplicate is idempotent; a changed
duplicate, gap, regression, source mismatch, or unhealthy record terminalizes
the attempt. Reset is required before another attempt.

- Inertial record qualification:
  [source](https://github.com/spincyc/adk/blob/main/src/inertial_record_qualification.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/inertial_record_qualification.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/test_inertial_record_qualification.cpp),
  [Mega replay](downloads/sketches/Lesson068InertialRecordQualification.ino),
  and [Lesson 068](lessons/068.md)

This policy qualifies copied synthetic record evidence only. Configuration
with physical-family source tags is unsupported at E0. No result authenticates
an MPU6050 or QMI8658, proves a mounting or calibration, operates a bus, or
establishes bench acceptance. The canonical Mega replay measures 14,908 bytes
of flash and 766 bytes of static SRAM.

## Qualified motion recorder

`QualifiedMotionRecorder` is the Lesson 069 E0 project for one explicitly
configured source per session. It admits one terminal Lesson 068
qualification envelope, correlates subsequent Lesson 067 records and copied
controls, advances a fixed six-step hand-motion script, and publishes
fault-dominant `MotionPresentationIntent`.

The caller supplies the complete `MotionRecordImage` array synchronously to
each update. The recorder validates its exact capacity, stages one canonical
128-byte image, and appends atomically; it retains no caller pointer and never
wraps or silently replaces an older cell. `MotionRecordCodec` validates
framing, integrity, and semantic fields without treating C++ object layout as
a file format. Export is only a volatile request/acknowledgement handshake.

- Qualified motion recorder:
  [source](https://github.com/spincyc/adk/blob/main/src/qualified_motion_recorder.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/qualified_motion_recorder.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/test_qualified_motion_recorder.cpp),
  [Mega replay](downloads/sketches/Lesson069InterchangeableMotionRecorder.ino),
  and [Lesson 069](lessons/069.md)

The honest canonical Mega composition measures 39,428 bytes flash and 2,347
bytes static SRAM. Exact no-LTO evidence measures 35,144 bytes flash, 2,347
bytes static SRAM, 861 bytes stack, a 509-byte object, a 128-byte record image,
and 4,856 bytes residual SRAM. Exact flash, static SRAM, and stack miss their
targets but pass independently reviewed fingerprint-bound hard gates. This E0
result owns no sensor, control endpoint, display, RTC, media, filesystem, or
durable storage.

## Descriptor-driven threshold modules

Lesson 070 introduces stateless copied-value contracts for low-voltage
threshold modules. `ModuleThresholdDescriptor` declares one fixture's
identity, channel topology, comparator output and polarity, pull requirement,
electrical ranges, raw domain, threshold-control direction, and independently
known or unknown warm-up and settling durations.

`ModuleThresholdFrame` binds copied analog and comparator evidence to the full
descriptor identity, specimen and electrical-evidence revisions, source
configuration, sequence, and supplied observation time. Validation preserves
absent, current, stale, and producer-fault channel states without repairing
unknown declarations or inventing physical conclusions.

- Threshold-module descriptor:
  [source](https://github.com/spincyc/adk/blob/main/src/module_threshold_descriptor.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/module_threshold_descriptor.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/module_threshold_descriptor_test.cpp),
  [Mega replay](downloads/sketches/Lesson070ThresholdDescriptor.ino),
  and [Lesson 070](lessons/070.md)

The canonical compile-only Mega replay measures 4,564 bytes of flash and 684
bytes of static SRAM. This is E0 software evidence only: descriptor validity
and declaration completeness do not prove that a physical specimen matches
the declarations, authorize power, or qualify acquisition.

## Threshold characterization

`ModuleCharacterizationPolicy` is the Lesson 071 streaming E0 policy. It
accepts 2--16 copied points in each of three explicit legs: ascending,
descending, and verification. The two learning legs freeze adjacent transition
brackets; the verification leg reports only a conservative
`Consistent`, `Ambiguous`, or `Disagrees` relation against the guaranteed and
ambiguity intervals.

`ModuleCharacterizationEvidence` retains the complete descriptor, run and leg
identity, counts, both transition brackets, three intervals, terminal reason,
and compact first, last, and offending witnesses. Supplied time, sequence,
direction, warm-up, settling, producer status, correlation, and lifecycle
errors remain deterministic and attributable. A rail result is permitted only
for the endpoint-only no-transition exception and cannot manufacture an
interval or advance to the next leg.

- Threshold characterization:
  [source](https://github.com/spincyc/adk/blob/main/src/module_characterization.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/module_characterization.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/module_characterization_test.cpp),
  [Mega replay](downloads/sketches/Lesson071Characterization.ino),
  and [Lesson 071](lessons/071.md)

The canonical Mega replay measures 10,200 bytes flash and 1,160 bytes static
SRAM. The independently reviewed exact no-LTO boundary measures 11,562 bytes
flash, 339 bytes synchronous stack, a 498-byte policy, a 375-byte evidence
value, and a 57-byte caller-local point. This remains copied E0 evidence: it
does not identify, energize, sweep, or qualify a physical module.

## Inert module-characterization bench

`InertModuleCharacterizationBench` is the Lesson 072 E0 project. It admits one
atomic terminal `ModuleCharacterizationEnvelope`, advances the fixed
`InspectDeclaration`, `ReviewAscending`, `ReviewDescending`,
`ReviewVerification`, and `PrepareRecord` script, and exposes
fault-dominant `ModuleBenchPresentationIntent`.

`prepareRecord()` is a separate final-step action. It uses
`ModuleCharacterizationRecordCodec` to stage and atomically copy one canonical
192-byte `ADMC` image into caller memory. Decode distinguishes length,
framing, integrity, and semantic failures and leaves output unchanged on
failure. The record retains compact declarations and review evidence plus
domain-separated descriptor, evidence, and witness digests; those digests
detect correlation failures but do not prove source identity.

- Inert module-characterization bench:
  [source](https://github.com/spincyc/adk/blob/main/src/inert_module_characterization_bench.h),
  [implementation](https://github.com/spincyc/adk/blob/main/src/inert_module_characterization_bench.cpp),
  [host tests](https://github.com/spincyc/adk/blob/main/tests/inert_module_characterization_bench_test.cpp),
  [Mega replay](downloads/sketches/Lesson072ModuleCharacterizationBench.ino),
  and [Lesson 072](lessons/072.md)

The canonical Mega composition measures 24,860 bytes flash and 1,998 bytes
static SRAM. Exact no-LTO evidence measures 27,354 bytes flash, 2,002 bytes
static SRAM, 740 bytes synchronous stack, a 436-byte bench, exactly 192 bytes
per record image, 384 bytes for both simultaneously live images, and 5,322
bytes residual SRAM. Flash exceeds its 24 KiB target but passes the
independently reviewed 32 KiB hard limit; all other targets pass. This remains
volatile copied evidence: no module, acquisition endpoint, clock, display,
storage transport, or powered fixture is owned or qualified.

## Error and electrical safety

- Treat `ResourceBusy` as a wiring or ownership error; do not steal a pin.
- Remove power before changing wiring.
- Do not drive an externally driven signal as an output.
- Respect Mega 2560 voltage and current limits; use suitable resistors and
  driver circuitry.
- RAII restores ownership and pin mode, but it cannot make unsafe external
  circuitry safe.

See [Safety](safety.md), [Determinism](determinism.md), the
[architecture](docs/ARCHITECTURE.md), and the
[development contract](docs/DEVELOPMENT.md) for the full rules.

## Verification status

An interface becomes release-supported only when its header and out-of-line
implementation, deterministic host tests, Mega 2560 build, hardware acceptance
record, HTML reference, and lesson PDF all pass together. The
[component index](components.md) records that evidence. Until then, this page
describes the experimental first-class API rather than a stable ABI.

## USB and HDMI research boundaries

The transparent USB and HDMI mesh is research, not part of the Arduino library
ABI and not a supported physical product. Its ordinary Linux controller plans,
authorizes, fences, and audits routes; it carries neither USB transactions nor
HDMI media.

The USB product target connects one unmodified Windows or Linux computer to a
computer attachment unit (`Cau`), crosses the shared switched Ethernet fabric,
then reaches one peripheral attachment unit (`Pau`) and the exact physical
topology rooted at one of its four independently powered USB 3 Type-A ports.
The `Cau` consumes one computer USB port and does not invent a hub. A
user-supplied hub, when present at the `Pau`, and all its descendants move as
one atomic topology. See the
[transparent USB product contract](docs/research/USB_TRANSPARENT_PRODUCT.md)
and [mesh roadmap](projects/mesh-roadmap.md).

Linux USB/IP and its temporary single-process ledger are prototype measurement
tools only. They cannot establish physical transparency, Windows compatibility,
USB 3 timing, durable ownership, or product conformance. They are neither the
product controller nor its authoritative route state. The
[USB/IP procedure](projects/usb3-matrix.md) preserves that limited boundary.

The HDMI mesh terminates the source-side HDMI link, transports interpreted
video, audio, timing, and metadata over the same Ethernet fabric, then
reconstructs a fresh sink-side HDMI link. Named profiles make bandwidth and
latency tradeoffs explicit; a pinned profile never silently degrades. See the
[HDMI mesh architecture](docs/research/HDMI_MESH_ARCHITECTURE.md).

Executable research models and deterministic host tests exercise routing,
fencing, profile selection, and failure policy. No physical transparent USB
attachment, HDMI payload, PoE power, throughput, latency, recovery, or
interoperability result has yet been recorded.
