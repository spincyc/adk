# Component-family architecture

This is the design authority for first-class ADK components beyond GPIO. It
maps the Elegoo Mega 2560 Most Complete kit and common 37-module sensor kits
without promising that every vendor revision contains the same parts.

The rule is simple: inherit only to substitute a hardware driver; compose to
build circuit meaning. Public components are concrete `struct`s. They own
endpoints and claims, accept time and samples explicitly, and never allocate,
throw, or rely on global registration.

## Common vocabulary

| Kind | Responsibility | Ownership |
|---|---|---|
| Driver seam | Replaceable board or protocol operation | Non-owning reference |
| Claim | Pin, timer, interrupt, bus, address, or port reservation | Exclusive or explicit shared token |
| Endpoint | Electrical behavior and safe release | Owns every required claim |
| Device | Protocol and conversion for one part | Owns endpoints; borrows a bus |
| Component | Circuit meaning and deterministic state | Owns devices/endpoints |
| Project | Coordinates components | Owns components and injects time |

Drivers may use a small non-owning function table when host substitution is
required. Do not use a virtual lifecycle hierarchy. `DigitalOutput`,
`PwmOutput`, `ServoOutput`, and `ToneOutput` are not subclasses of one another:
their capabilities, timer claims, and shutdown contracts differ.

Each owner follows:

```cpp
Status initialize () noexcept;
void   shutdown   () noexcept;
bool   initialized () const noexcept;
```

Initialization validates configuration and capabilities, acquires all claims,
then touches hardware. Failure releases claims in reverse order and produces no
externally active output. Shutdown first selects the documented inactive state,
then makes signal pins high impedance and releases claims. It is idempotent and
is called by the destructor.

## Resource model

Extend `ResourceKind` only for resources that can conflict independently:

- `Pin`: GPIO identity, including analog-number aliases.
- `Timer`: PWM, tone, servo, pulse measurement, and scheduling conflicts.
- `Interrupt`: external and pin-change interrupt channels.
- `I2cBus`, `SpiBus`, `SerialPort`: one bus owner per peripheral.
- `I2cAddress`: unique address within a borrowed bus.
- `ChipSelect`: an output pin owned by one SPI device.
- `PowerDomain`: switched or externally supplied rail with a current budget.
- `Adc`: conversion engine when asynchronous sampling is introduced.

Bus owners hold peripheral claims. Devices borrow a bus through a non-null
reference and own only their address or chip-select claim. They cannot outlive
the bus. Transactions are synchronous at first and expose timeouts; later
nonblocking drivers must use explicit `update(TimePoint)`.

Timer claims need modes:

- exclusive for tone generation and timer reconfiguration;
- compatible shared channels for PWM pins on the same unchanged timer;
- multiplexed ownership for a servo bank that schedules several servo signals.

No component silently reconfigures a timer already used by another mode.

## Power domains and safe states

Pin ownership does not imply power ownership. Every actuator with meaningful
load accepts a `PowerDomain&` or an explicit `ExternallyPowered` token.

```text
PowerDomain
    voltage range
    continuous and peak current budgets
    common-ground requirement
    enable endpoint, if switched
    fault and brownout observations
```

Initialization rejects an incompatible declared domain before enabling a load.
Software current budgets are evidence and conflict detection, not electrical
protection. Motors, servos, steppers, relays, solenoids, pumps, and LED strips
normally require an external supply, a common ground, and a suitable driver.

Safe release is component-specific:

| Family | Inactive before release |
|---|---|
| LED/display | blank or documented dark state |
| Buzzer/tone | waveform stopped; pin inactive |
| Relay | coil de-energized |
| DC motor | coast by default; optional explicit brake |
| Stepper | coils de-energized |
| Servo | pulse detached; position is not guaranteed |
| Bus device | transaction ended; chip select inactive |
| Radio transmitter | transmit disabled |

Never describe destruction as removing external power or stored mechanical
energy.

## Output and actuator families

### Light

`MonoLed` composes `DigitalOutput`. `PwmLed` composes `PwmOutput`. `RgbLed`
contains three `PwmOutput`s and a polarity/configuration value; it is not three
`PwmLed`s because initialization must claim all channels transactionally.
`LedBar` contains a fixed span of outputs or a `ShiftRegisterOutput`.

Addressable strips receive a dedicated `PixelBus` endpoint because their
timing, memory, current, and interrupt effects are unlike PWM.

```cpp
struct PwmLed;
struct RgbLed;
struct LedBar;
struct PixelBus;
struct PixelStrip;
```

Test seams record physical writes. Tests cover polarity, color-channel order,
clamping, all-off rollback, timer collisions, deterministic animation frames,
and current-budget rejection.

### Sound

`ActiveBuzzer` composes `DigitalOutput`; the module makes its own tone.
`ToneOutput` owns a pin and exclusive timer claim. `PassiveBuzzer` composes
`ToneOutput` and adds note semantics. `MelodyPlayer` owns no hardware; it
coordinates a borrowed `PassiveBuzzer` from explicit timestamps.

```cpp
struct ToneOutput;
struct ActiveBuzzer;
struct PassiveBuzzer;
struct MelodyPlayer;
```

The driver seam records start/stop frequency operations. Tests cover zero and
limit frequencies, duration boundaries, timer conflicts, rollover, rests,
restart, and silence during every failure and shutdown path.

### Relay and switched load

`RelayOutput` composes `DigitalOutput`, polarity, and a `PowerDomain&`. It
defaults off and never presents a generic `toggle()` operation. A
`TimedRelay` coordinates a borrowed relay using explicit time.

Relay lessons use only low-energy loads. Mains, ignition, pyrotechnic,
life-safety, and unattended heater control are outside ADK.

Tests cover active-low boards, boot/shutdown glitches, maximum-on timeout,
brownout/fault input, and de-energized rollback.

### Servo

`ServoBank` owns the timer and platform servo driver. `ServoOutput` owns one
signal pin and borrows its bank. `PositionServo` and `ContinuousServo` compose
that endpoint with distinct units and limits; neither inherits from the other.

```cpp
struct ServoBank;
struct ServoOutput;
struct PositionServo;
struct ContinuousServo;
```

The power domain is separate from the signal pin. Tests use a pulse-recorder
seam and cover attach rollback, pulse bounds, slew scheduling, detach,
shared-bank capacity, and conflicts with PWM/tone. Arduino documents both the
timer sharing and the need for an external servo supply for larger loads:
[Servo library](https://docs.arduino.cc/libraries/servo/).

### DC motor and fan

`HBridge` owns two direction outputs, one optional PWM enable, and a power
domain. `DcMotor` composes an `HBridge` and expresses signed speed, coast, and
brake. `Fan` composes a unidirectional switch or PWM endpoint and must not imply
reversal.

```cpp
struct HBridge;
struct DcMotor;
struct Fan;
```

Tests cover shoot-through prevention, direction reversal through a mandatory
neutral interval, brake/coast distinction, timer conflict, injected driver
fault, and power-budget rejection.

### Stepper

`StepperDriver` is an endpoint for either four coil outputs or step/direction/
enable pins. `StepperMotor` adds steps-per-revolution and travel state.
`MotionPlan` is a deterministic coordinator with explicit time and fixed
storage.

Tests cover phase sequences in both directions, limit handling, acceleration
boundaries, rollover, emergency disable, and coil-off teardown. The small kit
stepper still receives an external driver and declared supply domain.

## Display families

Display interfaces expose meaning, not framebuffer inheritance:

```cpp
struct CharacterDisplay;
struct SegmentDisplay;
struct MatrixDisplay;
struct DisplayBuffer;
```

- `Hd44780Display` composes either parallel digital endpoints or an I2C
  backpack device.
- `SevenSegmentDisplay` composes direct outputs or a shift register.
- `FourDigitDisplay` owns digit-select outputs and a timer/scheduling policy.
- `Max7219Display` borrows `SpiBus` and owns chip select.
- `OledDisplay` borrows I2C or SPI and owns a fixed-size page buffer only when
  its configured mode requires one.

Do not introduce a universal `Display` base. Character cells, segments, and
pixels are not safely substitutable. Small formatting algorithms may operate
on narrow writer concepts through templates after their flash cost is measured.

The test seam records commands and data bytes. Golden tests cover initialization
sequence, clipping, character encoding, orientation, dirty-region writes,
address conflicts, bus faults, and blank-before-release.

## Communication and storage

### Buses

```cpp
struct I2cBus;
struct SpiBus;
struct SerialPort;
struct OneWireBus;
```

Each bus owns platform configuration and a resource claim. Configuration is an
immutable value. Operations return `Status`/`Result`, take bounded caller-owned
buffers, and include an explicit timeout or deadline. No operation allocates.

I2C devices own an address claim; SPI devices own chip select and immutable
mode/clock requirements while each transaction applies its settings. Arduino's
Wire implementation has a small bounded buffer and timeouts are not universally
enabled by default, so ADK must validate transfer size and configure a bounded
failure path: [Wire reference](https://docs.arduino.cc/language-reference/).

The host seam scripts responses by bus/address/register and records every
transaction. Tests cover short transfer, NACK, timeout, arbitration/platform
failure, wrong device identity, chip-select cleanup, and deterministic retry
policy.

### Shift registers and expanders

`ShiftRegisterOutput` owns clock, data, and latch endpoints; it provides a
fixed-width output bank. `ShiftRegisterInput` similarly owns clock/load/data.
I2C GPIO expanders borrow `I2cBus`. Higher components borrow explicit channels
from these banks rather than pretending expanded pins are native `PinId`s.

Tests assert bit order, latch atomicity, initial image, bounds, and safe output
image before release.

### Infrared and radio

`IrReceiver` owns an interrupt/timer capture endpoint. `IrTransmitter` owns a
carrier timer and output. `RadioReceiver` and `RadioTransmitter` wrap a
documented module-specific bus driver; there is no generic “RF frequency
cloner.”

Receive traces may be analyzed and replayed in simulation. Transmission lessons
require a known lawful protocol, permitted band, inert receiver, and explicit
transmit enable. Unknown launcher/ignition control remains excluded.

### Storage and time

`EepromStore`, `SdCard`, and `RealTimeClock` are devices, not global services.
They borrow their bus. A fixed-record `LogWriter` composes storage and a
borrowed clock. Host tests inject torn writes, full capacity, CRC mismatch,
rollover, and replayed timestamps.

## Sensor and input families

Sensor modules fit a small number of electrical shapes. Reuse endpoints without
erasing sensor units:

| Shape | Endpoint | Example devices |
|---|---|---|
| Digital state | `DigitalInput` | tilt, reed, knock comparator, flame comparator, line tracker |
| Analog scalar | `AnalogInput` | potentiometer, photoresistor, microphone envelope, thermistor, joystick axes |
| Timed pulse | `PulseInput` | ultrasonic echo, speed/encoder pulse, pulse sensor |
| Resistive timing | dedicated timed endpoint | capacitive touch and some moisture boards |
| I2C device | borrowed `I2cBus` | MPU-6050/GY-521, RTC, pressure sensors |
| One-wire device | borrowed `OneWireBus` | DS18B20 |
| Encoded digital protocol | dedicated driver | DHT11/DHT22 |

`Sensor<T>` should not be a runtime base. Concrete sensors return unit-bearing
values such as `Temperature`, `RelativeHumidity`, `Distance`, `Acceleration`,
or `IlluminanceEstimate`. Reusable filters are value-type composition:

```cpp
template<typename Sample, std::size_t Count>
struct MovingAverage;

struct Threshold;
struct Hysteresis;
struct Calibration;
```

Template use is confined to algorithms where it removes runtime overhead and
does not inflate every driver. Calibrations are immutable and validated.
Events are stable snapshots until the next explicit update.

Common 37-module sets include LEDs, reed and tilt switches, Hall sensors,
temperature sensors, light/flame sensors, microphones, buzzers, encoders,
joysticks, and relay modules; use the vendor list as inventory, not as an API
taxonomy: [Keyestudio 37-in-1 list](https://wiki.keyestudio.com/900852_Keyes_37_in_1_Sensor_Kit_for_Arduino).

## Driver seams

Production and host tests execute the same component state machines. Limit
substitution to these narrow seams:

- GPIO/ADC/PWM/timer driver function tables;
- I2C, SPI, serial, one-wire transaction drivers;
- servo/tone/pulse engine drivers;
- explicit time supplied to `update(TimePoint)`;
- declared power-domain observations and injected faults.

Each fake has fixed storage in embedded-compatible tests where practical. It
records timestamped operations and accepts scripted results. Do not mock a
component that can be tested by composing real state logic with fake endpoints.

Required tests for every family:

1. trait, configuration, and capability boundaries;
2. initialization success, idempotence, and every injected failure point;
3. claim collision, rollback, capacity, release, and reuse;
4. command/value endpoints and unit limits;
5. exact deterministic trace replay;
6. timeout and timestamp rollover;
7. shutdown from every active state and destruction during caller unwinding;
8. power loss/fault observation without unsafe reactivation;
9. Mega 2560 compile, size record, and later physical acceptance.

## Recommended implementation sequence

The curriculum remains two component lessons plus one integrating project.
Within that cadence, land dependencies in this order:

1. `AnalogInput`, filtering values, potentiometer, photoresistor.
2. `PwmLed` and transactional `RgbLed`.
3. Project: automatic color night-light.
4. `ToneOutput`, active/passive buzzer.
5. `ShiftRegisterOutput`, seven-segment display.
6. Project: timed quiz console.
7. `I2cBus`, MPU-6050, character-display backpack.
8. `SpiBus`, MAX7219/SD device foundations.
9. Project: environmental instrument panel.
10. `ServoBank`/servo, ultrasonic pulse input.
11. `HBridge`/DC motor, encoder/line sensors.
12. Project: deterministic bench rover.
13. Stepper, RTC/storage, keypad, relay, IR receive.
14. Project blocks for greenhouse, data logger, access-panel simulator, and
    inert show-cue simulator.

Every commit includes the interface, out-of-line implementation, host tests,
Mega example, terse HTML contract, rich complementary PDF, and status update.
Hardware-dependent claims stay “host verified” until a recorded bench
acceptance exists.

## Inventory and authority notes

Elegoo describes the current Mega kit as more than 200 pieces with more than 35
lessons, including LCD1602 and GY-521 modules:
[Elegoo kit page](https://us.elegoo.com/products/elegoo-mega-2560-the-most-complete-starter-kit).
Kit revisions and generic 37-module bundles vary. Agents must record the exact
module marking, voltage, polarity, driver IC, and schematic before implementing
an adapter. The electrical datasheet outranks tutorial code.

This document chooses architecture and sequencing. `CURRICULUM.md` owns lesson
numbers, `SAFETY_MODEL.md` owns hazard limits, `TESTING.md` owns evidence, and
`DEVELOPMENT.md` owns acceptance gates.
