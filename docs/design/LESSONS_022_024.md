# Lessons 022--024 design: observable greenhouse trainer

Status: executable design; hardware acceptance deferred.

This slice teaches moisture measurement, bounded load intent, and scheduled
control without switching a real pump. The released reference circuit is E1:
a USB-powered Mega 2560, a 10 kΩ potentiometer that simulates a moisture
probe, resistor-limited LEDs, buttons, and optional display/storage components
introduced by earlier lessons. It contains no relay, motor, mains connection,
external load supply, water, or unattended actuator.

An actual low-voltage pump is not a lesson deliverable. The software boundary
ends at an inert `PumpOutput` capability. The only released Mega adapter is
`IndicatorPump`, which turns a resistor-limited LED on and off. A later rated
driver adapter requires a separate part-specific electrical design, load-power
disconnect, current and stall measurements, flyback or module-protection
review, and physical acceptance. It must not be inferred from this lesson.

## Dependency narrative

```text
022 owned buses + RTC/storage  -> durable evidence boundary
022 AnalogInput + calibration -> MoistureSensor -> MoistureSample
023 constrained inert loads   -> PumpOutput + WateringController
024 scheduler                 -> GreenhouseController -> view + records
```

The canonical project loop reads in the same vocabulary:

```cpp
const TimePoint now = TimePoint (millis ());

greenhouse.observe (now);
greenhouse.decide  (now, readGreenhouseInput ());
greenhouse.actuate (now);
greenhouse.present (now);
greenhouse.record  (now);
healthPattern.update (now, greenhouse.snapshot ().mode);
```

`observe()` samples. `decide()` changes only deterministic state and intent.
`actuate()` is the only operation that may change the pump indicator.
`present()` displays an already-decided view. `record()` publishes the same
snapshot. No stage reads a global clock, blocks, or hides another stage.

## Lesson 022: owned buses, records, and moisture evidence

### Learning outcome

Own the Mega communication buses, timestamp and store a bounded record, measure
one named analog test point, calibrate its dry and wet endpoints, and publish a
moisture sample whose validity is separate from its numeric value.
The learner must distinguish:

- no accepted sample;
- a valid dry-to-wet permille value;
- raw input at a configured fault rail;
- calibration with no usable span;
- a once-valid sample that has become stale.

The potentiometer is the reference source because it is repeatable, dry, and
safe. It represents a probe output without claiming to reproduce soil physics.
A physical soil probe is deferred entirely. Adding one requires a separately
reviewed module, voltage, output range, source impedance, supply/common-ground
design, corrosion policy, and bench card; it cannot change the public sample
model or deterministic tests.

### Canonical bus and storage boundary

Lesson 022 preserves the canonical bus, RTC, and storage runway. Moisture is
the observable record payload; it does not replace that infrastructure.

```cpp
struct I2cDriver
{
    virtual ~I2cDriver () noexcept;

    virtual Status configure () noexcept = 0;
    virtual void   disable   () noexcept = 0;
    virtual Status transfer  (uint8_t        address,
                              const uint8_t* writeData,
                              uint8_t        writeSize,
                              uint8_t*       readData,
                              uint8_t        readSize) noexcept = 0;
};

struct I2cBus
{
    I2cBus  (ResourceRegistry& resources,
             I2cDriver&        driver) noexcept;
    ~I2cBus () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status transfer   (uint8_t        address,
                       const uint8_t* writeData,
                       uint8_t        writeSize,
                       uint8_t*       readData,
                       uint8_t        readSize) noexcept;
};

struct SpiSettings
{
    uint32_t clockHz;
    uint8_t  mode;
    bool     mostSignificantBitFirst;
};

struct SpiDriver
{
    virtual ~SpiDriver () noexcept;

    virtual Status configure () noexcept = 0;
    virtual void   disable   () noexcept = 0;
    virtual Status transfer  (const SpiSettings& settings,
                              PinId             chipSelect,
                              const uint8_t*    writeData,
                              uint8_t*          readData,
                              uint16_t          size) noexcept = 0;
};

struct SpiBus
{
    SpiBus  (ResourceRegistry& resources,
             SpiDriver&        driver) noexcept;
    ~SpiBus () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status transfer   (PinId              chipSelect,
                       const SpiSettings& settings,
                       const uint8_t*     writeData,
                       uint8_t*           readData,
                       uint16_t           size) noexcept;
};
```

The bus owners exclusively claim `ResourceKind::I2cBus` or
`ResourceKind::SpiBus`. Their Mega adapters also claim the fixed bus pins.
An I2C transfer accepts only 7-bit non-reserved addresses and bounded buffers.
A SPI transfer owns and validates its chip-select pin, asserts it only for the
transaction, restores it inactive even after failure, and applies settings on
every transaction. No global `Wire` or `SPI` owner is exposed.

The first implementation deliberately serializes transactions; it does not
retain a callback, queue, or caller buffer after return. Fixed host drivers
record exact configuration, address, settings, bytes, chip-select edges, and
injected failure offsets. Tests cover claim conflicts, every address/mode
boundary, zero and maximum transfer lengths, null-buffer rules, partial
failure, rollback, bus restoration, rollover-independent replay, shutdown,
and destruction.

The RTC adapter returns a value plus explicit oscillator/power-loss state:

```cpp
enum struct ClockState : uint8_t
{
    Valid,
    NotSet,
    OscillatorStopped,
    TransportFault
};

struct ClockReading
{
    uint32_t   unixSeconds;
    ClockState state;
};

struct Rtc
{
    virtual ~Rtc () noexcept;

    virtual Status               initialize () noexcept = 0;
    virtual void                 shutdown   () noexcept = 0;
    virtual Result<ClockReading> read       () noexcept = 0;
};
```

Civil-time formatting is outside this interface. The project records the raw
seconds and state. A specific RTC adapter lands only after selecting its module
and primary datasheet.

Storage is bounded and synchronous:

```cpp
struct Storage
{
    virtual ~Storage () noexcept;

    virtual Status initialize () noexcept = 0;
    virtual void   shutdown   () noexcept = 0;
    virtual Status append     (const uint8_t* data,
                               uint16_t       size) noexcept = 0;
    virtual Status sync       () noexcept = 0;
};
```

`append()` reports failure without claiming durability. `sync()` is the
durability boundary. A fixed in-memory storage fake injects failure at each
byte and sync. The SD adapter owns one `SpiBus` relationship and chip-select;
it never exposes a filesystem global. Capacity exhaustion, partial append,
failed sync, restart, and exact record recovery are required tests.

TP-SDA, TP-SCL, TP-MOSI, TP-MISO, TP-SCK, and TP-CS provide non-Serial bus
evidence. An activity LED shows a completed record sync; a distinct red pattern
shows transport or storage failure. Logic activity proves a transaction, not
correct time or durable media.

### Public model

```cpp
enum struct MoistureSampleState : uint8_t
{
    Unavailable,
    Valid,
    InputBelowRange,
    InputAboveRange,
    Stale
};

struct MoistureCalibration
{
    AnalogInput::Reading dryReading;
    AnalogInput::Reading wetReading;
    AnalogInput::Reading faultMargin;
};

struct MoistureSample
{
    uint16_t            moisturePermille;
    AnalogInput::Reading rawReading;
    TimePoint           observedAt;
    MoistureSampleState state;
};

struct MoistureSensor
{
    MoistureSensor  (AnalogInput&               input,
                     const MoistureCalibration& calibration) noexcept;
    ~MoistureSensor () noexcept;

    MoistureSensor& operator= (const MoistureSensor&) = delete;
    MoistureSensor  (const MoistureSensor&)           = delete;
    MoistureSensor& operator= (MoistureSensor&&)      = delete;
    MoistureSensor  (MoistureSensor&&)                = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (TimePoint now) noexcept;

    MoistureSample sample     (TimePoint now,
                               Duration  staleAfter) const noexcept;
    bool           initialized () const noexcept;
};
```

`MoistureSensor` borrows an `AnalogInput`; the input must outlive it. It does
not claim the same pin again. Initialization validates:

- both calibration values are at most `AnalogInput::maximumReading`;
- `dryReading != wetReading`;
- `faultMargin <= 64`;
- the dry-to-wet span is greater than `2 * faultMargin`.

Dry and wet endpoints may ascend or descend. Mapping is integer-only, rounded
to the nearest permille, and clamped to 0--1000 after validity checks:

```text
moisture = round((raw - dry) * 1000 / (wet - dry))
```

The implementation uses a signed 32-bit numerator and denominator. It must
not rely on unsigned wrap or floating point.

`faultMargin` permits small measurement variation beyond each calibrated
endpoint while treating larger excursions as suspected wiring or source
faults. Comparisons use signed intermediates, so an endpoint close to 0 or
1023 cannot wrap. For an ascending calibration, a reading below
`dryReading - faultMargin` is `InputBelowRange`; a reading above
`wetReading + faultMargin` is `InputAboveRange`. Descending calibration reverses
those comparisons while preserving the state names as electrical range
direction, not semantic dryness. A fault retains `rawReading`, sets
`moisturePermille` to zero, and is never consumed as a valid measurement.

`update(now)` calls `AnalogInput::update()`, captures one reading and timestamp,
then derives the immutable sample. The current `AnalogInput::update()` cannot
report a platform read failure; the moisture layer does not invent one.
`sample(now, staleAfter)` returns `Stale`
when the last valid sample age is strictly greater than `staleAfter`.
Freshness does not mutate stored state. Repeating `sample()` is idempotent.
A newer range fault always remains the returned state; an older valid sample
cannot replace it with `Stale`.

### Host fake and exact cases

The existing fake analog input supplies readings. Required tests include:

- ascending calibration `200 -> 0`, `600 -> 500`, `1000 -> 1000`;
- descending calibration `900 -> 0`, `500 -> 500`, `100 -> 1000`;
- signed arithmetic extremes at 0 and 1023 for both endpoint directions;
- nearest-integer rounding immediately above and below a half step;
- both valid endpoints and one reading immediately inside each boundary;
- below-range and above-range faults;
- invalid pin, busy pin, repeated initialization, and pre-initialized input;
- equal endpoints, insufficient span, excessive guard, and out-of-range values;
- age equal to `staleAfter`, age one millisecond beyond it, and rollover;
- recovery from a fault with no stale valid value leaking through;
- repeated update at the same timestamp and byte-identical sample replay;
- shutdown, destruction, reinitialization, and input resource reuse.

A golden trace uses:

| `now` | raw | expected state | expected permille |
|---:|---:|---|---:|
| 0 | 200 | `Valid` | 0 |
| 100 | 360 | `Valid` | 200 |
| 200 | 600 | `Valid` | 500 |
| 300 | 840 | `Valid` | 800 |
| 400 | 1000 | `Valid` | 1000 |
| 500 | 1023 | `InputAboveRange` | 0 |
| 600 | 680 | `Valid` | 600 |

The fixture uses `dryReading = 200`, `wetReading = 1000`, and
`faultMargin = 16`.

### Circuit and observation

Reference wiring:

| Connection | Purpose |
|---|---|
| Potentiometer outer pin to 5 V | simulated sensor supply |
| Potentiometer other outer pin to GND | reference return |
| Potentiometer wiper to A1 / TP22 | measurable simulated moisture voltage |
| D36 through 220 Ω to green LED and GND | valid sample indicator |
| D37 through 220 Ω to red LED and GND | invalid or stale indicator |

Before turning the knob, predict the direction of TP22 voltage and moisture
permille. Measure TP22 relative to the named GND point, observe the status
LED, then compare the raw and calibrated values through an optional stable
record. The LED proves validity state; TP22 proves the electrical input. Neither
proves a real soil-water relationship.

Resource-acquisition evidence is a green-red-green startup sweep. Safe-state
evidence is a separate measurement: after `shutdown()`, D36 and D37 are high
impedance and no LED is lit. Physical measurements remain open until recorded.

### Lesson package

The HTML page carries the exact model, mapping, validity table, source and test
links, and CLI commands. The PDF carries:

- a pencil orientation drawing and authoritative connection table;
- an ADC voltage-versus-reading worksheet;
- ascending and descending calibration graphs;
- prediction, observation, and interpretation rows;
- a stale/fault diagnosis tree;
- deterministic trace exercises;
- a bench card that labels every physical result open.

## Lesson 023: bounded watering intent and inert output

### Learning outcome

Turn valid moisture samples into bounded watering intent, separate policy from
physical output, and prove that faults, time limits, shutdown, and destruction
select `Off`. The reference “pump” is an LED.

### Output capability

```cpp
enum struct PumpState : uint8_t
{
    Off,
    On
};

struct PumpOutput
{
    virtual ~PumpOutput () noexcept;

    virtual Status    initialize ()                    noexcept = 0;
    virtual void      shutdown   ()                    noexcept = 0;
    virtual Status    setState   (PumpState state)     noexcept = 0;
    virtual PumpState state      () const              noexcept = 0;
    virtual bool      initialized () const             noexcept = 0;
};

struct IndicatorPump final : PumpOutput
{
    IndicatorPump  (ResourceRegistry& resources,
                    PinId             indicatorPin) noexcept;
    ~IndicatorPump () noexcept override;

    IndicatorPump& operator= (const IndicatorPump&) = delete;
    IndicatorPump  (const IndicatorPump&)           = delete;
    IndicatorPump& operator= (IndicatorPump&&)      = delete;
    IndicatorPump  (IndicatorPump&&)                = delete;

    Status    initialize () noexcept override;
    void      shutdown   () noexcept override;
    Status    setState   (PumpState state) noexcept override;
    PumpState state      () const noexcept override;
    bool      initialized () const noexcept override;
};
```

`IndicatorPump` owns one active-high `DigitalOutput`. Construction is inert.
Initialization writes low before the pin becomes an output. `setState(On)`
lights one resistor-limited LED. `shutdown()` writes low, returns the pin to
high impedance, and releases it. Repeated shutdown and destruction are safe.

No supported adapter in this slice accepts a relay, transistor, motor, pump,
external supply, or mains voltage. `PumpOutput` describes controller intent,
not electrical suitability or functional safety.

### Constrained simulated loads

Lesson 023 also preserves the canonical fan/pump/heater simulation. Three
resistor-limited LEDs compose behind one policy boundary:

```cpp
enum struct SimulatedLoad : uint8_t
{
    None,
    Fan,
    Pump,
    Heater
};

struct SimulatedLoadSnapshot
{
    SimulatedLoad active;
    Status        lastStatus;
};

struct InertLoadPanel
{
    InertLoadPanel  (ResourceRegistry& resources,
                     PinId             fanIndicator,
                     PinId             pumpIndicator,
                     PinId             heaterIndicator) noexcept;
    ~InertLoadPanel () noexcept;

    InertLoadPanel& operator= (const InertLoadPanel&) = delete;
    InertLoadPanel  (const InertLoadPanel&)           = delete;
    InertLoadPanel& operator= (InertLoadPanel&&)      = delete;
    InertLoadPanel  (InertLoadPanel&&)                = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status select     (SimulatedLoad load) noexcept;

    SimulatedLoadSnapshot snapshot     () const noexcept;
    bool                  initialized () const noexcept;
};
```

At most one indicator may be active. `select()` first turns every currently
active channel off, then turns on the selected channel. If either operation
fails, it makes one best-effort all-off pass and reports `HardwareFailure`;
the snapshot becomes `None`. Initialization acquires all three pins
transactionally and establishes all-off. Shutdown performs all-off, returns
every pin to high impedance in reverse acquisition order, and releases them.

`IndicatorPump` is the focused lesson adapter and `InertLoadPanel` is the
project composition. The greenhouse example uses a small `PumpOutput` adapter
that borrows the panel and translates `On` to `select(Pump)` and `Off` to
`select(None)`. Its lifetime cannot exceed the panel. Fan and heater states
remain manual fault/exclusion exercises in lesson 023; project 024 never turns
them on automatically.

Tests exhaust all transitions among `None`, `Fan`, `Pump`, and `Heater`;
initialization failure at each pin; failure while turning an old channel off;
failure while turning a new channel on; exact rollback; no duplicate write for
an unchanged selection; repeated shutdown; destruction; and proof that no
trace contains two active indicators. D38, D40, and D41 are the pump, fan, and
heater indicators. Each has its own 220 Ω resistor.

### Deterministic watering controller

```cpp
struct WateringConfig
{
    uint16_t startBelowPermille;
    uint16_t stopAtPermille;
    Duration maximumOnTime;
    Duration minimumOffTime;
};

enum struct WateringState : uint8_t
{
    Starting,
    Idle,
    Watering,
    LockedOut,
    SensorFault,
    OutputFault
};

enum struct WateringReason : uint8_t
{
    None,
    DryThreshold,
    WetThreshold,
    MaximumOnTime,
    MinimumOffTime,
    OperatorInhibit,
    InvalidSample,
    OutputFailure,
    Shutdown
};

struct WateringSnapshot
{
    WateringState  state;
    WateringReason reason;
    PumpState      requestedPump;
    TimePoint      stateSince;
};

struct WateringController
{
    WateringController  (const WateringConfig& config,
                         PumpOutput&            pump) noexcept;
    ~WateringController () noexcept;

    WateringController& operator= (const WateringController&) = delete;
    WateringController  (const WateringController&)           = delete;
    WateringController& operator= (WateringController&&)      = delete;
    WateringController  (WateringController&&)                = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status decide     (TimePoint             now,
                       const MoistureSample& sample,
                       bool                  wateringAllowed) noexcept;
    Status actuate    () noexcept;

    WateringSnapshot snapshot     () const noexcept;
    bool             initialized () const noexcept;
};
```

Valid configuration requires:

- `startBelowPermille < stopAtPermille`;
- `stopAtPermille <= 1000`;
- nonzero `maximumOnTime`;
- nonzero `minimumOffTime`;
- both durations no greater than `0x7fffffff` milliseconds.

`decide()` applies these rules in order without touching the output:

1. a latched `OutputFault` preserves `Off`;
2. an invalid or stale sample requests `Off` and enters `SensorFault`;
3. `wateringAllowed == false` requests `Off`, enters `Idle`, and records
   `OperatorInhibit`;
4. while watering, elapsed time at or beyond `maximumOnTime` requests `Off`
   and enters `LockedOut`;
5. while watering, moisture at or above `stopAtPermille` requests `Off` and
   enters `Idle`;
6. while off, moisture below `startBelowPermille` may request `On` only after
   `minimumOffTime`;
7. values between thresholds preserve the safe current state.

Initialization sets `Off` but does not invent a timestamp. The first
`decide()` anchors the initial off interval and therefore cannot request `On`.
Shutdown attempts `Off` before shutting down the output. A caller cannot clear
`LockedOut` merely by supplying another dry sample: it returns to `Idle` only
after `minimumOffTime`, then a later `decide()` may start watering.
Reinitialization is the explicit recovery from `OutputFault`. `actuate()`
applies the decided
state once; an output failure changes the snapshot to `OutputFault`, requests
`Off`, and makes one best-effort `Off` command.

### Required deterministic tests

Use `RecordingPumpOutput`, a fixed host fake with injected initialization and
command failures plus an exact timestamped command trace.

Test:

- invalid configurations and no output call before initialization;
- initialization success, failure, rollback, repetition, and destruction;
- exact dry threshold and one permille below it;
- exact wet threshold and one permille below it;
- minimum-off boundary and one millisecond before it;
- maximum-on boundary and one millisecond before it;
- invalid and stale samples from every active state;
- timestamp rollover in both timers;
- delayed calls that exceed both a moisture transition and timeout, with
  timeout/fault precedence preserved;
- output failure on `On`, output failure on `Off`, and recovery policy;
- no duplicate hardware command when requested state is unchanged;
- shutdown from every state and exact final `Off` trace;
- two runs of the same sample/time trace with identical snapshots and commands.

Golden trace for thresholds 350/600, maximum-on 5000 ms, minimum-off 2000 ms:

| `now` | moisture/state | controller | pump | reason |
|---:|---|---|---|---|
| 0 | 500 `Valid` | `Idle` | `Off` | `None` |
| 1999 | 300 `Valid` | `Idle` | `Off` | `MinimumOffTime` |
| 2000 | 300 `Valid` | `Watering` | `On` | `DryThreshold` |
| 4000 | 500 `Valid` | `Watering` | `On` | `DryThreshold` |
| 4500 | 600 `Valid` | `Idle` | `Off` | `WetThreshold` |
| 6500 | 300 `Valid` | `Watering` | `On` | `DryThreshold` |
| 11500 | 300 `Valid` | `LockedOut` | `Off` | `MaximumOnTime` |
| 13500 | 300 `Valid` | `Idle` | `Off` | `MinimumOffTime` |
| 13501 | 300 `Valid` | `Watering` | `On` | `DryThreshold` |
| 14000 | 0 `Stale` | `SensorFault` | `Off` | `InvalidSample` |

### Circuit and observation

Reference wiring uses D38 through 220 Ω to a blue “simulated pump” LED and
GND. D39 through 220 Ω drives an amber lockout indicator. The moisture-valid
green/red pair from lesson 022 remains present.

Predict whether D38 may turn on before changing TP22. The four LEDs distinguish
validity, watering intent, and timeout lockout without Serial. TP38 measured
relative to GND proves the commanded electrical level. The LED does not prove
that a physical pump could be driven, and the lesson makes no such inference.

Safe-state evidence separately observes D38 low immediately before it becomes
high impedance at shutdown. Resource evidence uses the startup sweep; normal
`Off` is not accepted as proof that resource acquisition succeeded.

The PDF includes a state graph, exact timeline worksheets, precedence table,
failure-injection exercise, pencil layout, and a conspicuous “LED only—no
relay or pump” boundary on every wiring page.

## Lesson 024 project: observable greenhouse trainer

### Learning outcome

Compose moisture sampling, bounded watering intent, scheduled observation,
stable records, display presentation, operator inhibition, and diagnostic
output. A saved trace must reproduce every controller snapshot, view, record,
and output command.

The trainer is not an unattended irrigation product. It controls only the
`IndicatorPump` LED in the released example.

### Coordinator model

```cpp
struct GreenhouseConfig
{
    Duration sampleInterval;
    Duration displayInterval;
    Duration recordInterval;
    Duration staleAfter;
};

enum struct GreenhouseMode : uint8_t
{
    Starting,
    Monitoring,
    Watering,
    Inhibited,
    SensorFault,
    OutputFault,
    DisplayFault,
    RecordFault,
    MultipleFaults
};

struct GreenhouseInput
{
    bool wateringAllowed;
};

struct GreenhouseSnapshot
{
    MoistureSample   moisture;
    WateringSnapshot watering;
    GreenhouseMode   mode;
    Status           sensorStatus;
    Status           outputStatus;
    Status           displayStatus;
    Status           recordStatus;
    uint32_t         recordSequence;
};

struct GreenhouseController
{
    GreenhouseController  (const GreenhouseConfig& config,
                           MoistureSensor&         moisture,
                           WateringController&     watering,
                           CharacterDisplay&       display,
                           RecordSink&             records) noexcept;
    ~GreenhouseController () noexcept;

    GreenhouseController& operator= (const GreenhouseController&) = delete;
    GreenhouseController  (const GreenhouseController&)           = delete;
    GreenhouseController& operator= (GreenhouseController&&)      = delete;
    GreenhouseController  (GreenhouseController&&)                = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status observe    (TimePoint now) noexcept;
    Status decide     (TimePoint              now,
                       const GreenhouseInput& input) noexcept;
    Status actuate    (TimePoint now) noexcept;
    Status present    (TimePoint now) noexcept;
    Status record     (TimePoint now) noexcept;

    GreenhouseSnapshot snapshot     () const noexcept;
    bool               initialized () const noexcept;
};
```

The earlier `CharacterDisplay` and `RecordSink` capabilities are reused. The
coordinator does not own the RGB diagnostic component; the narrative example
maps `GreenhouseMode` to an external status pattern.

Initialization order is moisture, watering, display, records. Failure rolls
back in reverse. Shutdown sets watering intent off, then shuts down records,
display, watering, and moisture. A display or record fault never changes a
valid moisture sample. A sensor fault and operator inhibition always suppress
watering. An output failure dominates presentation and remains latched until
reinitialization.

Dependencies are caller-allocated but dedicated to this coordinator while it
is initialized. The coordinator owns their initialize/shutdown calls; the
caller owns only storage lifetime and must make every object outlive the
coordinator. A dependency that was initialized earlier still receives its
idempotent initialize call and is shut down by the coordinator. Tests lock this
ownership rule so partial external lifecycle management cannot become implicit.

`observe(now)` takes at most one due sample. `decide(now, input)` applies
freshness, inhibition, and watering policy without writing hardware.
`actuate(now)` applies the already-decided pump state once. `present(now)`
writes the due display view. `record(now)` appends the due stable record. They
perform no sensing or policy change. Repeated stage calls at one timestamp have
no effect after success. A failed `record(now)` is the one exception: it keeps
one pending record and may retry at the same or a later timestamp. New decisions
cannot replace that pending record. Missed periods coalesce instead of
bursting; there is no record queue beyond the one retry slot.

Configuration requires nonzero intervals, every interval and `staleAfter` no
greater than `0x7fffffff` milliseconds, and
`staleAfter >= sampleInterval`. The first `observe()` after initialization
anchors every schedule and performs the first sample immediately. The first
display and record become due after their full intervals. A delayed call
advances each due epoch to `now` and performs one operation.

Stage return values describe the operation attempted by that call. Snapshot
statuses retain the most recent subsystem result. Mode precedence is:

```text
OutputFault
    -> MultipleFaults when any other current fault also exists
    -> SensorFault
    -> DisplayFault
    -> RecordFault
    -> Inhibited
    -> Watering
    -> Monitoring
```

Sensor, display, and record faults recover after their next successful
operation. `OutputFault` is latched until shutdown and successful
reinitialization. A failed best-effort `Off` remains `OutputFault`; no later
sample can request `On`.

### Stable record

Version 1 is locale-independent ASCII with LF. Tokens are at most eight bytes:

```text
adk-gh,1,<sequence>,<milliseconds>,<sample>,<raw>,<permille>,<water>,<reason>,<mode>\n
```

Example:

```text
adk-gh,1,17,42000,valid,472,340,on,dry,watering
```

Use the existing 96-byte `StableRecord`. The worst case is 77 bytes including
LF: two 10-digit integers, a four-digit raw reading, a four-digit permille
field, delimiters, and three eight-byte tokens. A
compile-time constant and host assertion preserve that proof. Sequence
increments only after successful append. The pending record stores its original
decision timestamp and bytes; a retry is byte-identical even after later
observations. Tokens are lowercase ASCII and versioned. Invalid samples retain
raw evidence but never represent their permille field as valid moisture;
encode that field as `-`.

### Display and circuit-native health

Normal 16x2 views:

```text
MOIST  34.0 %
PUMP ON  00:04
```

```text
MOIST  72.5 %
PUMP OFF  READY
```

Fault views use stable tokens: `SENSOR`, `OUTPUT`, `DISPLAY`, `RECORD`, and
`MULTI`. Because a failed display cannot prove its own failure, RGB patterns
remain authoritative:

| Mode | RGB evidence |
|---|---|
| `Starting` | slow blue pulse |
| `Monitoring` | green heartbeat |
| `Watering` | cyan heartbeat while D38 is on |
| `Inhibited` | steady amber |
| `SensorFault` | two red pulses |
| `OutputFault` | steady red |
| `DisplayFault` | three amber pulses |
| `RecordFault` | four violet pulses |
| `MultipleFaults` | alternating red and violet |

The example owns deterministic diagnostic behavior:

```cpp
struct GreenhouseHealthPattern
{
    explicit GreenhouseHealthPattern (RgbLed& led) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (TimePoint      now,
                       GreenhouseMode mode) noexcept;
};
```

Pattern periods and phase origins are constants published in the lesson.
Changing mode restarts its pattern at phase zero. Repeating an update at one
timestamp produces no additional write. Host tests cover every phase boundary,
rollover, mode change, LED failure, shutdown, and byte-identical RGB traces.

The project retains TP22 for analog evidence and TP38 for commanded output.
If the earlier I2C display and SPI storage are composed, TP-SDA, TP-SCL,
TP-MOSI, TP-MISO, TP-SCK, and each chip-select are named in the schematic.
Serial capture is optional supporting evidence.

### Deterministic project tests

Required fixtures cover:

- every invalid `GreenhouseConfig`, first-due epochs, and half-range limits;
- initialization failure at every dependency and exact reverse rollback;
- already-initialized borrowed dependencies and their documented ownership;
- startup to first valid sample;
- dry start before and at minimum-off time;
- wet stop, hysteresis, maximum-on lockout, and later recovery;
- operator inhibition before start and while watering;
- every moisture state, staleness boundary, and sensor recovery;
- output failure on both transitions and latched recovery;
- isolated display and record failures while sampling and safe output continue;
- simultaneous failures and deterministic precedence;
- exact schedule boundaries, mutually prime intervals, delayed loop calls,
  coalescing, timestamp rollover, and repeated stage calls;
- failed record append followed by byte-identical retry;
- pending-record capacity, same-timestamp retry, and no replacement by a newer
  snapshot;
- stable view cells and exact record grammar;
- exact stage return and retained-status precedence for simultaneous failures;
- every health-pattern phase boundary and rollover;
- shutdown from every mode, final `Off`, high-impedance output, destruction,
  and reinitialization;
- two complete replay runs with identical snapshots, commands, views, records,
  and diagnostic-pattern intents.

Property checks over generated bounded traces assert:

```text
invalid moisture              -> requested pump is Off
wateringAllowed == false      -> requested pump is Off
watering duration >= maximum  -> requested pump is Off
output fault                  -> requested pump is Off
shutdown complete             -> requested pump is Off
```

### Canonical Mega example outline

Objects appear in dependency order:

```cpp
Runtime                   runtime;
I2cBus                    displayBus (...);
SpiBus                    storageBus (...);
AnalogInput               moistureInput (...);
Button                    wateringInhibit (...);
MoistureSensor            moistureSensor (...);
InertLoadPanel            loadPanel (...);
PanelPumpOutput           pumpOutput (...);
WateringController        watering (...);
Hd44780Display            display (...);
SdRecordSink              records (...);
GreenhouseController      greenhouse (...);
RgbLed                    healthLed (...);
GreenhouseHealthPattern   healthPattern (...);
```

The exact bus/storage objects depend on their landed lesson-022 interfaces.
If storage is not ready at the project boundary, a fixed-capacity RAM record
sink is the canonical first adapter; Serial remains optional and may not be
the only record evidence.

`setup()` reads as acquire, configure, start. `loop()` reads as observe,
decide, actuate, present, record. High-level functions precede pin constants
and adapter mechanics. Code, display text, record tokens, state graph, and
lesson prose use the same nouns.

### Staged project build

1. Replay moisture samples into the controller with fake output/display/sink.
2. Add the A1 potentiometer, validity LEDs, and TP22 measurement.
3. Add `IndicatorPump`; prove minimum-off and maximum-on timing at TP38.
4. Add the inhibit button and prove it dominates a watering request.
5. Add the character display and inject display failure without changing pump
   policy.
6. Add fixed records and verify retry bytes; optionally capture over Serial.
7. Run every synthetic fault and predict the RGB pattern before observation.
8. Record startup, monitoring, watering, inhibition, each fault, recovery,
   reset, shutdown, and power-removed states.

No stage adds a relay, motor, pump, water, mains connection, or unattended
operation.

### Lesson package

HTML supplies exact APIs, state and precedence tables, stable-record grammar,
source/tests, size evidence, and CLI commands. The PDF supplies:

- authoritative schematic plus pencil orientation drawing;
- ownership map and observe-decide-actuate-record timing diagram;
- moisture calibration and schedule worksheets;
- state graph, fault matrix, and troubleshooting tree;
- exact replay trace and record exercises;
- prediction-observation-interpretation tables;
- explicit resource-acquisition and safe-state checks;
- staged acceptance card with every unperformed physical field left open.

The package status remains **host verified; hardware acceptance open** until a
Mega 2560 bench record names the board, supply, potentiometer, resistor values,
instruments, TP22/TP38 measurements, LED currents, bus observations, shutdown
state, deviations, reviewer, and date.

## Safety boundary

- The released circuit is USB-powered E1 and uses only resistor-limited LEDs
  as load indicators.
- No lesson wiring includes a relay, transistor, MOSFET, motor, solenoid,
  pump, external load supply, mains voltage, water, or energetic load.
- `PumpOutput` is a software substitutability boundary. It is not evidence that
  arbitrary hardware is safe, compatible, isolated, or adequately rated.
- A later physical actuator requires a named module and pump, primary
  datasheets, separate low-voltage supply, common-reference or isolation
  design, overcurrent protection, stored-energy handling, dry-run policy,
  leak containment, independent load-power removal, and supervised bench
  acceptance.
- No ADK greenhouse lesson controls an unattended load or makes agricultural,
  environmental, or life-safety claims.
- Serial can explain a fault but cannot be the only indication of validity,
  watering intent, inhibition, timeout, or output failure.

## Implementation boundaries

Land in dependency order:

1. `sensor: add moisture sample model`
2. `sensor: add calibrated moisture input`
3. `output: add inert pump indicator`
4. `control: add bounded watering policy`
5. `project: add greenhouse trainer`
6. `lessons: teach observable greenhouse control`
7. `site: publish greenhouse trainer`

The sample model lands independently of Arduino hardware. The inert adapter
lands independently of the controller. The controller never includes a relay
or pump driver. Integration files (`Adk.h`, Make fragments, navigation, shared
indexes, size baselines, and release metadata) belong to the coordinator.
