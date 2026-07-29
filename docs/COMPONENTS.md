# Component catalog

This is the planning map for first-class ADK interfaces. It complements
`CURRICULUM.md`: that file defines teaching order; this file defines ownership,
composition, resources, and test seams. The lesson 042 slice is implemented,
host verified, and experimental. A catalog name is still a target unless a
matching public header has landed; in particular, lesson 009 models its
photoresistor through `AnalogInput` and does not yet publish a
`Photoresistor` adapter. Mega 2560 bench acceptance is still open. Legacy
interfaces are not dependencies.

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

`ResourceRegistry` has a fixed 17-byte payload: 11 bytes track exclusive
resources and six counters track shared Mega timer claims. It does not
allocate.

### Endpoints

Endpoints own electrical or peripheral configuration. Components translate
that configuration into domain meaning.

| Interface | Resources | Key policy | Test seam | Lesson |
|---|---|---|---|---:|
| `DigitalOutput` | `Pin` | Named active/inactive levels; high-impedance generic shutdown | `DigitalIoBackend` write/mode trace | 001 |
| `DigitalInput` | `Pin` | Floating or pull-up input; exposes electrical level | Scripted sample backend | 002 |
| `PwmOutput` | `Pin`, shared compatible `Timer` | Bounded duty cycle at board default frequency | PWM configuration trace | 004 |
| `AnalogInput` | `Pin` | Explicit raw sample; no hidden calibration or filtering | Supplied ADC sequence | 007 |
| `PulseInput` | `Pin`, optional `Interrupt`/`Timer` | Timeout differs from zero-width pulse | Timestamped edge trace | 019 |
| `ToneOutput` | `Pin`, `Timer` | Frequency/duration without blocking | Timer and pin event trace | 005 |
| `ServoOutput` | `Pin`, `Timer`, `PowerDomain` | Bounded pulse width; inactive on shutdown | Pulse trace and external-power fake | 017 |
| `SerialBus` | `SerialPort` | Explicit framing, baud, and timeout | Byte-stream fake | 025 |
| `I2cBus` / `I2cDevice` | `I2cBus`, address lease | One transaction owner; bounded transfers | Register-device fake and NACK injection | 022 |
| `SpiBus` / `SpiDevice` | `SpiBus`, chip-select `Pin` | Per-device mode/clock restored each transaction | Transfer trace and bus-state assertions | 022 |
| `Storage` | `StorageRegion` | Bounded reads; explicit sync and partial-write result | In-memory media with failure offsets | 022 |

### Circuit components

| Component | Composes | Semantic contract | First project |
|---|---|---|---:|
| `MonoLed` | `DigitalOutput` | `on()`, `off()`, inactive destruction | 003 |
| `Button` | `DigitalInput`, clocked debounce state | Raw/stable state; one-cycle press/release/hold snapshots | 003 |
| `ButtonGroup` | Borrowed `Button` objects | Chords and simultaneous-input policy; no event consumption | 003 |
| `RgbLed` | Three `PwmOutput` objects | Color and brightness within current budget | 006 |
| `PiezoSounder` | Pin and timer claims | Named timed cues; silence is safe state | 006 |
| `LinearCalibration` | Raw sample and two calibration points | Bounded integer mapping with explicit invalid configuration | 009 |
| `MovingAverage` / `Deadband` | Supplied scalar samples | Fixed-capacity deterministic smoothing; raw value remains available | 009 |
| `Photoresistor` | `AnalogInput`, divider model | Relative illumination; saturation is distinct | 009 |
| `Thermistor` | `AnalogInput`, calibration model | Temperature result or open/short fault | Later sensor work |
| `ShiftRegister` | Data/clock/latch `DigitalOutput` objects | Atomic presented value | 012 |
| `SevenSegmentDisplay` | `ShiftRegister` or direct outputs | Glyph model separated from scanning | 012 |
| `CharacterDisplay` | Parallel endpoints or `I2cDevice` | Presentation buffer separated from application state | 015 |
| `ClimateSensor` | Borrowed transport | Timestamped fixed-point sample with explicit validity | 015 |
| `Dht11Sensor` | One bidirectional digital pin | Explicit stabilization and cadence; high-impedance shutdown | 015 |
| `Rtc` | Borrowed clock adapter | Reading and oscillator state remain distinct from transport status | 024 |
| `FixedStorage` | `FixedStorageMedium`, `Storage` | Staged and durable prefixes make append/sync/restart explicit | 024 |
| `Keypad` | Row outputs, column inputs | Debounced key/chord snapshots | 018 |
| `MatrixKeypad` | Four row outputs and three column inputs | One complete release-gated key observation per scan | 018 |
| `AnalogJoystick` | Two `AnalogInput` objects and `Button` | Calibrated axes, dead zone, and distinct selection events | 033 |
| `QuadratureEncoder` | Two `DigitalInput` objects | Observed Gray-code edges, invalid-transition evidence, and saturating position | 033 |
| `Servo` | `ServoOutput` | Calibrated bounded position, never implied load safety | 018 |
| `UltrasonicRanger` | Trigger output, `PulseInput` | Distance, timeout, and out-of-range are distinct | 021 |
| `PirSensor` | `DigitalInput` | Warm-up and motion state are explicit | 021 |
| `MotorIntent` | Supplied motion requests and time | Bounded duty, reversal dead time, and stop dominance | 021 |
| `Relay` | `DigitalOutput`, `PowerDomain` | Inert load only in lessons; inactive default | 024 |
| `IndicatorPump` / `InertLoadPanel` | Three borrowed output channels | Transactional all-off exclusion; resistor-limited LEDs only | 024 |
| `WateringController` | Moisture samples, `PumpOutput`, supplied time | Hysteresis, lockout, stale/fault stop, deterministic recovery | 024 |
| `InfraredReceiver` | `PulseInput` or serial endpoint | Raw timing evidence separated from decoder | 027 |
| `CapturedIrEvidence` | Borrowed decoder and caller-owned pulse storage | Copies attributable receive evidence while disposition, strength, provenance, and pulse words remain distinct | 054 |
| `KnownIrEmissionPolicy` | Supplied time and immutable firmware catalog | Closed symbolic catalog produces only copied transactional envelope intent with cancellation and terminal attribution | 054 |
| `RadioObserver` | Receive-only bus/serial device | Passive timestamped observations; no transmit API | 027 |
| `ContinuityModel` | Injected samples, no energetic load | Open/closed/short/stale simulation | 030 |
| `CueAuditBuffer` / `CueAuditEncoder` | Caller-owned fixed storage | Append-only decisions and byte-stable records | 029 |
| `InertCueScheduler` | Supplied time and operator values | Confirmation-gated inert visual intervals | 029 |
| `InertShowSimulator` | Borrowed assessor, scheduler, and audit | Complete canonical observation frames gate mapped inert cues | 030 |
| `ReflectiveObservationPolicy`, `BeamObservationPolicy` | Copied optical samples | Source-specific qualification retains provenance, calibration revision, transitions, and faults | 042 |
| `PirObservationPolicy`, `PresenceModel` | Copied PIR, beam, reflective, and range evidence | Eligibility, freshness, validity, and disagreement remain explicit | 042 |
| `CourseStartPolicy`, `CourseMarshal`, `CourseMarshalPresenter` | Copied start, presence, checkpoint, and time values plus caller-owned storage | Only an explicit button with PIR eligibility starts a fixed-capacity replayable run | 042 |

The Lessons 040--042 contracts are published in
[`optical_observation.h`](https://github.com/spincyc/adk/blob/main/src/optical_observation.h),
[`presence_model.h`](https://github.com/spincyc/adk/blob/main/src/presence_model.h),
and
[`course_marshal.h`](https://github.com/spincyc/adk/blob/main/src/course_marshal.h).
Their deterministic seams are exercised by the matching
[optical](https://github.com/spincyc/adk/blob/main/tests/test_optical_observation.cpp),
[presence](https://github.com/spincyc/adk/blob/main/tests/test_presence_model.cpp),
and
[course](https://github.com/spincyc/adk/blob/main/tests/test_course_marshal.cpp)
tests and canonical `Lesson040OpticalObservation`,
`Lesson041PresenceModel`, and `Lesson042CourseMarshal` Mega replays. These are
E0 hardware-neutral policies: no powered optical or PIR adapter, exact wiring,
formal electrical schematic, or E1 acceptance is claimed.

### Kit module adapters

Common kit boards should be thin adapters over the contracts above, not new
architectural layers.

| Kit part | Adapter composition | Placement |
|---|---|---:|
| Active buzzer | Future exact-specimen `DigitalOutput` semantic adapter | Deferred; not Lesson 005 support |
| Passive buzzer | Timer and pin claims + `PiezoSounder` | 005 |
| 74HC595 | `ShiftRegister` | 010 |
| 4-digit seven-segment | Lesson 010 glyph/shift-register reuse; published multiplex policy and future exact endpoint | 058 and 060 host verified; E1 fixture open |
| 8x8 LED matrix | E0 MAX7219 presentation policy; future exact SPI endpoint | 059 and 060 host verified; E1 fixture open |
| HD44780 / I2C LCD | Parallel endpoints or `I2cDevice` + display model | 014 |
| DHT11/DHT22 | Timed digital endpoint + validated sample | 013 |
| TMP36 / LM35 | `AnalogInput` + linear calibration | Non-kit extension only; not an authorized Elegoo family |
| LDR module | `AnalogInput`, optional threshold `DigitalInput` | 008 |
| Water Level | E0 copied resistive-probe observation and corrosion-duty evidence; future exact endpoint | 061 host verified; E1a exact board/bench open |
| Digital Temperature | E0 copied categorical evidence; adapter pending exact specimen identity and not assumed to be 18B20 | 062--063 host verified; E1b unidentified |
| 18B20 Temp | Exact-specimen single-wire transport + qualified thermal value | 064--066 queued |
| Metal Touch | `InteractionIntentPolicy` accepts copied E0 tactile/directional evidence; exact powered input adapters remain gated | 046--048 host verified; E1 adapter open |
| Sound sensor | Relative ADC envelope and optional qualified threshold input | 038 published against an external reference; Elegoo substitution open |
| HC-SR04 | `UltrasonicRanger` | 019 |
| PIR module | `PirSensor` | 019 |
| MPU6050 / QMI8658 | `InertialObservationPolicy` accepts synthetic E0 fixtures; one future exact adapter remains gated behind the revision-neutral copied sample | 043--045 host verified; normalized records 067--069 |
| DS1307/DS3231 | `Rtc` | 022--024 |
| MicroSD module | `SpiDevice` + `SdCard` | 022--024 |
| Membrane keypad | `Keypad` | 016 |
| Analog joystick | `AnalogJoystick` | 031 |
| Rotary encoder | `QuadratureEncoder` | 032 |
| SG90 servo | `ServoOutput` + `Servo` + external supply boundary | 017--018 |
| 28BYJ-48 + ULN2003 | `BoundedStepperSequence` publishes logical coil intent without owning outputs | 046--048 host verified E0; exact energized system remains E2-open |
| L293D/L298N driver | `MotorDriver` + external supply boundary | 020--021 |
| DC motor/fan | Load behind `MotorDriver`; never direct from a pin | 020--021 |
| Relay module | `Relay` + inert test load and isolation review | 023--024 |
| IR receiver/remote | `InfraredReceiver` + decoder | 025--027 |
| RFID module | `LocalIdentityRegistry` (049), `BoundedHomingPolicy` (050), and `InertPartsCarousel` (051) are host-verified zero-resource E0 surfaces; exact RFID, input, and storage adapters remain E1-gated, and powered motion remains E2-gated | 049--051 host verified; E1/E2 open |
| Listed IR-emission module | `KnownIrEmissionPolicy` publishes inert known-code intent; an exact-specimen output adapter remains gated | 052--054 host verified E0; exact identity and E1 fixture open |
| Receive-capable RF module | Inventory-gated future adapter | Deferred |

Adapters outside the canonical numbered curriculum are extension lessons. They
must reuse the nearest endpoint and join a scheduled project before becoming
supported.

## Behavior composition

Behaviors borrow components and contain application state. They do not claim
pins behind a component's back.

| Behavior | Inputs | Outputs | Deterministic seam | Project |
|---|---|---|---|---:|
| `Blink` / heartbeat | `TimePoint`, health state | `MonoLed` intent | Supplied clock and output trace | 003 |
| Reaction state machine | Button snapshots, seed/config | LED and cue intents | Fixed delay source and input trace | 003 |
| Simon engine | Button/cue IDs, seed/config, time | Cue/phase model | Versioned PRNG, manual clock, replay trace | 006 |
| `NightLight` | Calibrated light samples and validity | Bounded duty, hysteresis, diagnostic intent | Sample trace | 009 |
| Instrument model | Controls and channel samples | Presentation model | Input/output snapshots | 015 |
| Logger | Sample records and timestamps | Append/sync requests | Memory storage and failure offsets | 018 |
| Lock model | Key events, timeout, attempt policy | Latch/display/log intents | Manual clock and replay trace | 021 |
| Rover controller | Range/motion samples, operator mode | Bounded drive intent | World trace and emergency-stop injection | 024 |
| Telemetry console | Configured packet observations | Health/signal/log intents | Recorded packet and fault trace | 027 |
| Cue simulator | Confirmations, continuity simulation, clock | Inert cue/audit intent | Full replay, fault matrix, redundant-state model | 030 |
| `CalibrationConsole` | Joystick/encoder/button snapshots and time | Preview, committed values, acknowledgement, and fault state | Supplied input/time replay | 033 |
| `InertIrTranslator` | Copied receive evidence, immutable local catalog, supplied time, and caller-owned pulse storage | Fixed different-symbol allowlist translation with cancellation, self-echo suppression, and attributable round-trip results | Exact synthetic fixtures, field-mutation matrix, timing boundaries, and byte-identical replay | 054 |

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
001 output + LED
002 input               -> 003 button + reaction timer
004 PWM + RGB
005 sound               -> 006 Simon
007 analog + calibration
008 sampled sensors     -> 009 adaptive night-light
010 shift/display
011 timed junction      -> 012 traffic junction
013 environmental sensor
014 display + records   -> 015 environmental station
016 keypad
017 servo + storage     -> 018 access-control trainer
019 proximity
020 motor + encoder     -> 021 bench rover
022 buses + storage
023 relay simulation    -> 024 greenhouse controller
025 infrared receive
026 telemetry evidence  -> 027 telemetry console
028 fault simulation
029 cue scheduling      -> 030 inert show-cue simulator
```

Digital output stays first because it supplies visible diagnostics for every
later circuit. A project must use only interfaces introduced earlier. Each
component names at least one later project that proves reuse, and every project
publishes a dependency map showing that composition.

The analog slice deliberately separates evidence:

```text
AnalogInput -> raw ADC count -> calibration -> filtering -> NightLight intent
```

The endpoint never silently filters. The behavior never reads Arduino globals.
An example may present both raw and processed values over optional Serial, but
the named analog test point and PWM lamp remain the required circuit-native
proof.

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
