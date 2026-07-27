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
