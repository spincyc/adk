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

Operations report `adk::Status`, never exceptions:

| Status | Meaning |
|---|---|
| `Ok` | Operation completed |
| `InvalidArgument` | Configuration is internally invalid |
| `InvalidPin` | Pin is outside the supported board profile |
| `Unsupported` | Hardware cannot provide the requested capability |
| `ResourceBusy` | Another live object owns the resource |
| `NotInitialized` | An operation requires successful initialization |
| `CapacityExceeded` | Fixed-capacity storage is full |
| `HardwareFailure` | The hardware operation failed |

`statusName(status)` returns a short diagnostic name. `Result<T>` couples a
status with a value; check `ok()` before using `value()`.

## Lifecycle

Every owning endpoint or component follows the same contract:

```cpp
adk::Status status = component.initialize ();

if (status != adk::Status::Ok)
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

    if (status == adk::Status::Ok)
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

if (input.initialize () == adk::Status::Ok)
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
    buttonReady = button.initialize () == adk::Status::Ok;
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

if (led.initialize () == adk::Status::Ok)
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

if (potentiometer.initialize () == adk::Status::Ok)
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
    showSensorFault ();
    return;
}

const adk::Result<uint16_t> smooth = average.addSample (mapped.value ());

if (!smooth.ok ())
{
    showSensorFault ();
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
