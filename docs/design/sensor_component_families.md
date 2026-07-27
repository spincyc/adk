# Sensor component families

Status: design input, not a supported API.

This plan groups the Mega 2560 Most Complete kit and common 37-module kits by
electrical contract. Product names are evidence about a board in the box, not
good interface boundaries. Sellers change module revisions, comparator
polarity, resistor values, and even sensor ICs without changing the familiar
module name. A lesson must identify the actual board and IC before selecting a
configuration.

The inventory basis is the current Elegoo kit descriptions: the Most Complete
kit advertises more than sixty kinds of parts and specifically names the
LCD1602, GY-521, RFID, sensors, motors, and discrete parts; the separate
37-module kit is a changing collection rather than a stable bill of materials.
See the [Most Complete kit][most-complete], [37-module kit][sensor-kit], and
[Elegoo kit wiki][kit-wiki]. These links establish scope, not electrical
ratings. IC datasheets and a measured board inspection remain authoritative.

## Design rules

- Components contain endpoints; endpoints contain resource claims.
- Prefer composition. Inheritance is reserved for platform or transport seams
  that require a substitutable host fake.
- Construction is inert. Hardware owners use `initialize()`, `shutdown()`, and
  `initialized()` with transactional acquisition and idempotent cleanup.
- Pure transforms do not pretend to own hardware. They accept explicit values,
  expose reset where stateful, and allocate no memory.
- Time enters as `TimePoint`; samples and transactions never read a hidden
  clock.
- Measurements use fixed-width raw or fixed-point values. Conversion units are
  named in the type or accessor; firmware does not require floating point.
- Every observation distinguishes transport status, sample validity, and the
  value. Zero is never a generic failure sentinel.
- Every circuit reserves a non-Serial observation: its primary effect, a
  diagnostic LED, or a named electrical test point.

## Proposed hierarchy

```text
Platform
    DigitalDriver
    AnalogDriver
    I2cDriver
    SpiDriver
    PulseDriver

Resource
    ResourceRegistry
    PinClaim / SharedResourceClaim
    I2cBus -> I2cDevice
    SpiBus -> SpiDevice
    OneWireBus -> OneWireDevice

Endpoint
    DigitalInput / DigitalOutput / PwmOutput / AnalogInput
    PulseInput / EdgeCounter
    I2cRegisterDevice / SpiRegisterDevice

Policy (hardware independent)
    LinearCalibration / MovingAverage / Deadband
    Threshold / Hysteresis / RangeValidation
    SampleSchedule / Freshness
    TransferFunction / Checksum

Component
    AnalogSensor / ThresholdSensor / PulseRangeSensor
    EnvironmentalSensor / MotionSensor / ContactSensor
    Display, storage, and actuator families

Project
    NightLight / EnvironmentalStation / OperatorPanel / Greenhouse
```

The tree describes dependency, not C++ inheritance. For example,
`ThresholdSensor` contains an `AnalogInput` and threshold policy; it does not
derive from `AnalogInput`.

## Common observation vocabulary

Use one small envelope across sensor families:

```cpp
enum struct SampleValidity : uint8_t
{
    Valid,
    BelowRange,
    AboveRange,
    Disconnected,
    Saturated,
    Stale,
    ChecksumFailure
};

template<typename Value>
struct Observation
{
    Status         status;
    SampleValidity validity;
    TimePoint      observedAt;
    Value          value;
};
```

This is a design sketch. A template would be header-defined, so measure its
flash cost before adoption. If repeated instantiations grow firmware, use
concrete envelopes such as `ScalarObservation` and `EnvironmentObservation`.
`Status` reports whether the operation ran; validity reports what the resulting
sample means.

## Reusable endpoint families

### Analog scalar

`AnalogInput` supplies a raw ADC reading. `LinearCalibration`, `MovingAverage`,
and `Deadband` transform explicit readings. Add these policies only when a
lesson needs them:

- `RangeValidation`: open, short, saturated, and plausible range;
- `Hysteresis`: separate rising and falling thresholds;
- `PiecewiseCalibration`: fixed-capacity calibration points for nonlinear
  sensors;
- `VoltageDivider`: fixed-point resistance from ADC ratio and known resistor;
- named transfer functions for thermistors and photoresistors.

This family covers potentiometers, photoresistors, thermistors, water-level
boards, analog Hall sensors, analog sound-envelope boards, joystick axes, flame
boards, and many gas boards. It must not imply that unlike sensors share units
or safety claims.

### Digital state and threshold

`DigitalInput` plus existing debounce and edge behavior covers buttons, tilt
balls, reed switches, knock switches, digital Hall modules, line trackers,
light barriers, and comparator outputs. Add a `Hysteresis` policy only for a
threshold derived in software; a module's LM393 comparator already has its own
physical threshold and may have undocumented polarity.

Avoid wrappers such as `FlameDetected`, `SoundDetected`, and `ObstacleDetected`
when they merely rename one bit. Use a configured `ThresholdInput` whose
contract names active level, pull policy, expected source, and test point.
Semantic project state belongs above it.

Modules exposing both `AO` and `DO` compose an `AnalogInput` and a
`DigitalInput`. A future `ComparatorObservation` can report both values
together so a learner can compare the trimmer threshold with the raw signal.
It must acquire both pins transactionally.

### Contact, motion, and human input

Buttons, joystick switches, reed switches, tilt switches, rotary encoders,
keypads, and touch boards share digital endpoints but not identical behavior.
Build from three policies:

- stable-level debounce;
- edge and release gating;
- fixed-capacity chord or quadrature decoding.

`RotaryEncoder` composes two `DigitalInput` endpoints and a quadrature decoder.
`Joystick` composes two `AnalogInput` endpoints and one `Button`. A membrane
keypad owns row outputs and column inputs as a matrix; it does not masquerade as
several independent buttons.

Do not publish a mercury-switch lesson. If a legacy kit contains one, identify
and isolate it; use the enclosed ball-tilt module instead.

### Pulse duration, frequency, and count

Introduce a bounded `PulseInput` after nonblocking scheduling. It owns a
capture-capable pin and reports high/low duration, timeout, and overflow
separately. A `PulseRangeSensor` then composes one trigger output, one pulse
input, a fixed-point sound-speed policy, minimum retrigger interval, and stale
sample policy. This supports HC-SR04-style ultrasonic ranging without putting
blocking `pulseIn()` inside a component.

An `EdgeCounter` supports encoder, tachometer, Hall, and frequency experiments.
Interrupt-backed production and explicit-edge host implementations must yield
the same snapshots. Timer and interrupt claims are explicit.

### Sampled single-wire protocols

Do not conflate these buses:

- Dallas/Maxim 1-Wire supports devices such as DS18B20 and needs a
  `OneWireBus`, ROM identity, conversion timing, and CRC policy.
- DHT11/DHT22 use a different timing protocol and need a bounded
  `DhtTransport`, checksum validation, and minimum sample interval.

Both expose nonblocking request/poll/read phases and explicit stale data. They
may share pulse-capture mechanics, scheduling, checksum helpers, temperature
value types, and observation envelopes; they must not share a false protocol
base type.

### I2C register devices

Build one owning `I2cBus` and non-owning `I2cDevice` relationships before
component drivers. Transactions specify address, bounded buffer, deadline, and
result count. A register helper handles byte order without heap allocation.

This seam supports:

- GY-521 / MPU-6050 motion samples;
- DS1307 or similar RTC modules;
- I2C backpack LCDs;
- common pressure, light, and environmental sensors added later.

Device components own configuration and conversion policy, not the bus.
`MotionSensor` returns raw axes first; scaling, offsets, orientation, and
filtering remain explicit policies. `RealTimeClock` distinguishes bus failure,
invalid BCD, oscillator-stop state, and calendar validity.

### SPI register and framed devices

`SpiBus` owns controller configuration. Each `SpiDevice` owns chip select and
restores bus mode and clock after a bounded transaction. This supports the
MFRC522 RFID board, SD storage, displays, and later ADCs.

Do not hide RFID policy in the transport. Frame exchange, CRC, card identity,
and application authorization are separate layers. The access-control lesson
remains an inert trainer and makes no security claim.

## Kit-module mapping

| Module or part | Endpoint composition | Reusable policy or protocol |
|---|---|---|
| Potentiometer, photoresistor | `AnalogInput` | calibration, range, average, deadband |
| Thermistor | `AnalogInput` | voltage divider, nonlinear transfer, range |
| Water-level board | `AnalogInput` | calibration, dry/wet range, corrosion warning |
| Analog sound board | `AnalogInput` | envelope/window statistics; no sound-level claim |
| Flame or gas board | analog/digital input | comparator observation; no safety claim |
| Hall, reed, tilt, knock | `DigitalInput` | polarity, debounce, edge, stuck fault |
| Line tracker, obstacle board | `DigitalInput` | comparator polarity and test point |
| Joystick | two analog inputs plus `Button` | center calibration, dead zone |
| Rotary encoder | two digital inputs plus optional button | quadrature, debounce |
| Keypad | row outputs plus column inputs | matrix scan, chord/ghosting policy |
| HC-SR04 | digital trigger plus `PulseInput` | cadence, timeout, sound-speed conversion |
| DHT11 | `DhtTransport` | checksum, cadence, stale sample |
| DS18B20 | `OneWireDevice` | ROM, CRC, conversion timing |
| GY-521 / MPU-6050 | `I2cDevice` | register codec, scale, calibration |
| RTC | `I2cDevice` | BCD codec, calendar validity, oscillator state |
| LCD1602 backpack | `I2cDevice` | presentation buffer and bounded refresh |
| MFRC522 | `SpiDevice` | framed protocol, CRC, identity policy |
| IR receiver | pulse/edge capture | bounded timing record, known-protocol decoder |

Active buzzers, lasers, relays, servos, motors, LEDs, displays, and shift
registers are output families rather than environmental sensors. They reuse the
same resource, transport, scheduling, and observability layers. Relay exercises
remain low-voltage inert simulations; lasers require an eye-safety review and
are not a default diagnostic output.

## Interface order

Land boundaries in this order so no component invents its own transport:

1. Complete `AnalogInput` and sampled-signal policies.
2. Add `RangeValidation`, `Hysteresis`, and a concrete scalar observation.
3. Build lesson 009 Night Light from those pieces.
4. Add explicit scheduling and freshness.
5. Add pulse capture and edge counting.
6. Add `I2cBus`, device relationship, transaction fake, and register codec.
7. Add one simple I2C component, then GY-521 and RTC.
8. Add independent DHT and 1-Wire transports over shared timing mechanics.
9. Add `SpiBus` and a simple framed fake before SD or RFID.
10. Add matrix input and quadrature components.

Each boundary needs the normal source, deterministic tests, Mega example, HTML,
PDF, size, and deferred bench record. Do not create all kit-specific names
upfront. Promote a semantic component only when it adds behavior, validation,
or lifecycle beyond endpoint-plus-policy composition.

## Testing matrix

Every sensor family tests:

- inert construction, repeated initialization, rollback, shutdown, and
  destruction while active;
- minimum, maximum, just-outside, saturation, disconnection, and implausible
  samples;
- identical output from repeated timestamp/sample traces;
- timestamp rollover, timeout, stale transition, recovery, and reset;
- fixed-buffer exhaustion, short reads, extra bytes, checksum failure, and
  transport failure where applicable;
- resource conflicts before hardware operations;
- acquisition evidence separately from safe-state evidence.

For observability, analog lessons name the ADC node as a voltage test point and
pair it with a bounded visible output. Digital comparator lessons expose raw
analog and comparator LED/output together. Protocol lessons use a health LED or
display state plus named SDA/SCL, pulse, or chip-select test points. Serial logs
remain optional corroboration.

## Decisions to defer

- Do not standardize a sensor IC from a seller's generic module name.
- Do not choose floating-point engineering units before AVR size measurements.
- Do not choose templates for every observation type before measuring code
  duplication.
- Do not promise interrupt-driven capture until timer and interrupt ownership
  compose with PWM and tone.
- Do not publish gas, flame, water, motion, or access modules as safety devices.
- Do not implement infrared or radio replay from unknown captures.

[most-complete]: https://us.elegoo.com/products/elegoo-mega-2560-the-most-complete-starter-kit
[sensor-kit]: https://www.elegoo.com/products/elegoo-37-in-1-sensor-kit
[kit-wiki]: https://wiki.elegoo.com/oshw-getting-started-%26-kits
