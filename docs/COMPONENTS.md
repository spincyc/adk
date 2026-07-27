# Component catalog

This is the planning map for first-class ADK interfaces. It complements
`CURRICULUM.md`: that file defines teaching order; this file defines ownership,
composition, resources, and test seams. Names are targets until their public
headers land. Legacy interfaces are not dependencies.

## Design rule

```text
board profile
    -> resource registry
        -> endpoint
            -> component
                -> behavior
                    -> project
```

Prefer composition. A `Button` has a `DigitalInput`; it is not a specialized
input pin. A `RgbLed` has three `PwmOutput` objects. Runtime inheritance is
reserved for a small substitutable seam whose flash and RAM cost has been
measured. Constructors are inert. `initialize()` claims all resources or none;
`shutdown()` is idempotent, and destruction releases every claim.

## Resource vocabulary

| Resource | Ownership | Examples | Conflict evidence |
|---|---|---|---|
| `Pin` | Exclusive while configured | Digital, analog, PWM, chip select | Competing claim fails without changing either pin |
| `Timer` | Exclusive mode; channels may be leased when compatible | Tone, PWM, servo, scheduler capture | Incompatible frequency or mode is rejected |
| `Interrupt` | Exclusive line | Encoder, pulse capture, wake input | Duplicate attachment is rejected |
| `I2cBus` | One bus owner; unique address leases | LCD backpack, RTC, environmental sensor | Duplicate address or absent device is reported |
| `SpiBus` | One bus owner; exclusive chip-select leases | SD card, display, radio receiver | Transaction restores mode and chip select |
| `SerialPort` | One configured owner | Console, GPS, serial module | Baud/configuration conflict is reported |
| `Adc` | Shared controller with explicit reference policy | Potentiometer, light, temperature | Reference conflict is reported |
| `PowerDomain` | Explicit external-supply boundary | Servo, motor, relay | Logic can become inert without assuming load power is gone |
| `StorageRegion` | Exclusive range or append stream | EEPROM settings, SD log | Bounds, full media, and interrupted write are injectable |
| `ProtocolAddress` | Unique within its owning bus | I2C address, radio node identity | Collision is observable before device initialization |

The registry identifies capability separately from ownership. A valid pin may
still lack PWM, interrupt, or analog-input capability.

## Target hierarchy

### Foundation

| Interface | Owns or borrows | Contract | Test seam | Lesson |
|---|---|---|---|---:|
| `Status` / `Result<T>` | Nothing | Explicit, non-throwing success or failure | Table of every status and rollback path | 001 |
| `TimePoint`, `Duration` | Nothing | Wrap-safe, explicit units | Boundary clock and recorded timestamps | 001 |
| `Clock` | Nothing | Supplies time; never advances implicitly | `ManualClock` | 001 |
| `BoardProfile` | Immutable capability data | Validates pins, buses, timers, and limits | Tiny synthetic board profiles | 001 |
| `ResourceRegistry` | Claim bitmap/state | Atomic acquire, release, and conflict reporting | Empty, full, duplicate, move/destruction tests | 001 |
| `ResourceClaim` | One active lease | Move-only RAII release | Scope exit and stack unwinding | 001 |
| `UpdateGroup` | Borrowed updatable objects | Stable deterministic order | Recorded call order and mutation attempts | 003 |

### Endpoints

Endpoints own electrical or peripheral configuration. Components translate
that configuration into domain meaning.

| Interface | Resources | Key policy | Test seam | Lesson |
|---|---|---|---|---:|
| `DigitalOutput` | `Pin` | Named active/inactive levels; high-impedance generic shutdown | `DigitalIoBackend` write/mode trace | 002 |
| `DigitalInput` | `Pin` | Floating, pull-up, or supported pull-down; exposes electrical level | Scripted sample backend | 004 |
| `PwmOutput` | `Pin`, compatible `Timer` channel | Bounded duty cycle and explicit frequency | PWM configuration trace | 007 |
| `AnalogInput` | `Pin`, borrowed `Adc` | Raw sample and explicit reference; no hidden filtering | Supplied sample sequence | 010 |
| `PulseInput` | `Pin`, optional `Interrupt`/`Timer` | Timeout differs from zero-width pulse | Timestamped edge trace | 022 |
| `ToneOutput` | `Pin`, `Timer` | Frequency/duration without blocking | Timer and pin event trace | 008 |
| `ServoOutput` | `Pin`, `Timer`, `PowerDomain` | Bounded pulse width; inactive on shutdown | Pulse trace and external-power fake | 020 |
| `SerialBus` | `SerialPort` | Explicit framing, baud, and timeout | Byte-stream fake | 025 |
| `I2cBus` / `I2cDevice` | `I2cBus`, address lease | One transaction owner; bounded transfers | Register-device fake and NACK injection | 016 |
| `SpiBus` / `SpiDevice` | `SpiBus`, chip-select `Pin` | Per-device mode/clock restored each transaction | Transfer trace and bus-state assertions | 017 |
| `Storage` | `StorageRegion` | Bounded reads; explicit sync and partial-write result | In-memory media with failure offsets | 017 |

### Circuit components

| Component | Composes | Semantic contract | First project |
|---|---|---|---:|
| `MonoLed` | `DigitalOutput` | `on()`, `off()`, inactive destruction | 003 |
| `Button` | `DigitalInput`, clocked debounce state | Raw/stable state; one-cycle press/release/hold snapshots | 006 |
| `ButtonGroup` | Borrowed `Button` objects | Chords and simultaneous-input policy; no event consumption | 006 |
| `RgbLed` | Three `PwmOutput` objects | Color and brightness within current budget | 009 |
| `PiezoSounder` | `ToneOutput` | Named cues; silence is safe state | 009 |
| `Potentiometer` | `AnalogInput`, calibration | Position with explicit range and uncertainty | 012 |
| `Photoresistor` | `AnalogInput`, divider model | Relative illumination; saturation is distinct | 012 |
| `Thermistor` | `AnalogInput`, calibration model | Temperature result or open/short fault | 012 |
| `ShiftRegister` | Data/clock/latch `DigitalOutput` objects | Atomic presented value | 015 |
| `SevenSegmentDisplay` | `ShiftRegister` or direct outputs | Glyph model separated from scanning | 015 |
| `CharacterDisplay` | Parallel endpoints or `I2cDevice` | Presentation buffer separated from application state | 015 |
| `Rtc` | `I2cDevice` | Validated civil time plus oscillator/power-loss status | 018 |
| `SdCard` | `SpiDevice`, `Storage` | Append/sync outcomes preserve audit state | 018 |
| `Keypad` | Row outputs, column inputs | Debounced key/chord snapshots | 021 |
| `Joystick` | Two `AnalogInput`, optional `Button` | Calibrated axes and dead zone | 021 |
| `RotaryEncoder` | Two inputs, optional interrupt claims | Direction/steps from supplied edge timestamps | 021 |
| `Servo` | `ServoOutput` | Calibrated bounded position, never implied load safety | 021 |
| `UltrasonicRanger` | Trigger output, `PulseInput` | Distance, timeout, and out-of-range are distinct | 024 |
| `PirSensor` | `DigitalInput` | Warm-up and motion state are explicit | 024 |
| `MotorDriver` | Direction outputs, PWM enable, `PowerDomain` | Coast, brake, direction, and interlock are named | 024 |
| `Relay` | `DigitalOutput`, `PowerDomain` | Inert load only in lessons; inactive default | 024 |
| `InfraredReceiver` | `PulseInput` or serial endpoint | Raw timing evidence separated from decoder | 027 |
| `RadioObserver` | Receive-only bus/serial device | Passive timestamped observations; no transmit API | 027 |
| `ContinuityModel` | Injected samples, no energetic load | Open/closed/short/stale simulation | 030 |

### Kit module adapters

Common kit boards should be thin adapters over the contracts above, not new
architectural layers.

| Kit part | Adapter composition | Placement |
|---|---|---:|
| Active buzzer | `DigitalOutput` + `ActiveBuzzer` | 008 |
| Passive buzzer | `ToneOutput` + `PiezoSounder` | 008 |
| 74HC595 | `ShiftRegister` | 013 |
| 4-digit seven-segment | `ShiftRegister`/multiplex outputs + display model | 013 |
| 8x8 LED matrix | Shift-register or SPI transport + matrix model | 014--015 |
| HD44780 / I2C LCD | Parallel endpoints or `I2cDevice` + display model | 014 |
| DHT11/DHT22 | Timed digital endpoint + validated sample | 011 or extension |
| TMP36 / LM35 | `AnalogInput` + linear calibration | 011 |
| LDR module | `AnalogInput`, optional threshold `DigitalInput` | 011 |
| Soil moisture / water level | `AnalogInput` + corrosion-aware sampling policy | 011/018 |
| Sound sensor | Analog envelope and optional threshold input | 011 |
| HC-SR04 | `UltrasonicRanger` | 022 |
| PIR module | `PirSensor` | 022 |
| MPU6050 | `I2cDevice` + inertial sample model | 016/018 extension |
| DS1307/DS3231 | `Rtc` | 017--018 |
| MicroSD module | `SpiDevice` + `SdCard` | 017--018 |
| Membrane keypad | `Keypad` | 019 |
| Analog joystick | `Joystick` | 019 |
| KY-040 encoder | `RotaryEncoder` | 019 |
| SG90 servo | `ServoOutput` + `Servo` + external supply boundary | 020--021 |
| 28BYJ-48 + ULN2003 | Four outputs + bounded stepper sequencer | 023--024 extension |
| L293D/L298N driver | `MotorDriver` + external supply boundary | 023--024 |
| DC motor/fan | Load behind `MotorDriver`; never direct from a pin | 023--024 |
| Relay module | `Relay` + inert test load and isolation review | 023--024 |
| IR receiver/remote | `InfraredReceiver` + decoder | 025--027 |
| RFID module | SPI/UART device + identity record | 025/027 extension |
| Receive-capable RF module | `RadioObserver`, receive-only | 026--027 |

Adapters outside the 30-lesson spine are extension lessons. They must reuse
the nearest endpoint and join a scheduled project before becoming supported.

## Behavior composition

Behaviors borrow components and contain application state. They do not claim
pins behind a component's back.

| Behavior | Inputs | Outputs | Deterministic seam | Project |
|---|---|---|---|---:|
| `Blink` / heartbeat | `TimePoint`, health state | `MonoLed` intent | Supplied clock and output trace | 003 |
| Reaction state machine | Button snapshots, seed/config | LED and cue intents | Fixed delay source and input trace | 006 |
| Simon engine | Button/cue IDs, seed/config, time | Cue/phase model | Versioned PRNG, manual clock, replay trace | 009 |
| Threshold controller | Calibrated samples, hysteresis config | Color/brightness intent | Sample trace | 012 |
| Instrument model | Controls and channel samples | Presentation model | Input/output snapshots | 015 |
| Logger | Sample records and timestamps | Append/sync requests | Memory storage and failure offsets | 018 |
| Lock model | Key events, timeout, attempt policy | Latch/display/log intents | Manual clock and replay trace | 021 |
| Rover controller | Range/motion samples, operator mode | Bounded drive intent | World trace and emergency-stop injection | 024 |
| Telemetry console | Timestamped local/remote samples | Display/alarm/log intents | Recorded packet and fault trace | 027 |
| Cue simulator | Confirmations, continuity simulation, clock | Inert cue/audit intent | Full replay, fault matrix, redundant-state model | 030 |

## Mega 2560 profile

The first board profile targets the Arduino Mega 2560 Rev3:

| Capability | Profile fact |
|---|---|
| MCU and logic | ATmega2560, 5 V logic, 16 MHz |
| Memory | 256 KiB flash, 8 KiB SRAM, 4 KiB EEPROM |
| Digital I/O | 54 pins; board documentation identifies 15 PWM outputs |
| Analog input | 16 channels, nominal 10-bit ADC |
| Hardware serial | Four UARTs |
| External interrupts | Pins 2, 3, 18, 19, 20, and 21 |
| I2C/TWI | Pins 20 (SDA) and 21 (SCL), also dedicated header aliases |
| SPI | Pins 50--53 and ICSP header |
| Per-pin current | Design to the board's documented 20 mA operating guidance; never treat absolute maximum ratings as a target |

Pin aliases refer to the same physical resource and must collide in the
registry. Board data must encode PWM channels and timer coupling; `tone()`,
servo control, and PWM can conflict even when their signal pins differ.
Hardware serial, USB upload, interrupt use, I2C aliases, and SPI chip selects
must remain visible to the learner rather than hidden as “free” pins.

Use the official board pinout and the ATmega2560 datasheet as the authority for
electrical and timer details:

- [Arduino Mega 2560 Rev3](https://docs.arduino.cc/hardware/mega-2560/)
- [Mega 2560 pinout PDF](https://docs.arduino.cc/resources/pinouts/A000067-full-pinout.pdf)
- [ATmega640/1280/1281/2560/2561 datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf)

## Verification matrix

Each interface receives only the seams it needs; production APIs do not grow
solely to satisfy tests.

| Boundary | Host evidence | Mega evidence |
|---|---|---|
| Resource ownership | Conflict, exhaustion, rollback, scope exit, stack unwinding | Two components deliberately request one pin/timer |
| Electrical endpoint | Exact mode/write/sample trace and safe shutdown | Meter/LED/logic-analyzer acceptance card |
| Time behavior | Manual clock, wraparound, early/late/equal timestamps | Recorded timing tolerance |
| Bus device | Register/byte fake, NACK/timeout/partial-transfer injection | Known-device identity and transaction trace |
| Storage | In-memory media, full/partial/interrupted write | Power-cycle-safe exercise on disposable media |
| Human input | Saved bounce/chord/hold traces | Repeatable press protocol with raw diagnostics |
| Sensor | Boundary samples, calibration, open/short/stale faults | Reference measurement and uncertainty table |
| Actuator intent | Command trace, limits, interlock, emergency stop | Inert load first; external power and isolation inspected |
| Project | End-to-end replay, fault matrix, state coverage | Staged build checks and final acceptance record |

All host suites build with exceptions and RTTI disabled, but destruction is
also tested during unwinding from an exception-enabled harness. Randomized
tests print their seed and algorithm version. No test reads wall-clock time.

## Lesson and project index

Projects remain every third lesson so each pair of component lessons is
immediately exercised:

```text
001 foundation
002 output + LED        -> 003 heartbeat/output exerciser
004 input
005 button              -> 006 reaction timer
007 PWM + RGB
008 sound               -> 009 Simon
010 analog + calibration
011 sampled sensors     -> 012 adaptive night-light
013 shift/display
014 character display   -> 015 bench instrument
016 I2C
017 SPI + storage       -> 018 environmental logger
019 rich input
020 servo boundary      -> 021 lock simulator
022 proximity
023 motor/relay intent  -> 024 tabletop rover
025 infrared receive
026 passive radio       -> 027 telemetry console
028 fault simulation
029 cue scheduling      -> 030 inert show-cue simulator
```

Digital output stays first because it supplies visible diagnostics for every
later circuit. A project must use only interfaces introduced earlier. Each
component names at least one later project that proves reuse, and every project
publishes a dependency map showing that composition.

## Addition checklist

Before adding a component:

1. Name its physical safe state and external-power boundary.
2. Select existing endpoints and resources; justify any new resource kind.
3. State ownership, units, ranges, failure states, and deterministic inputs.
4. Identify its host fake, failure injection, and Mega acceptance evidence.
5. Assign a component lesson and a later project.
6. Measure flash, SRAM, and timer/bus impact.
7. Link primary datasheets and board documentation.
8. Update this catalog, the curriculum, site navigation, and deferred ledger.

If those answers do not fit in a terse public header, the abstraction is not
ready.
