# Lessons 013--015 design: environmental station

Status: executable design; hardware acceptance deferred.

This slice turns an environmental measurement into evidence without coupling
sensor transport, presentation, or recording. The reference circuit is E1:
USB-powered Mega 2560, one DHT11-compatible 5 V module, one HD44780-compatible
16x2 LCD wired in four-bit parallel mode, and one resistor-limited RGB status
LED. The first host adapter is `Dht11Sensor`, matching the common Mega kit
module. Exact module and display part numbers remain physical-acceptance
blockers until their primary datasheets are recorded; they do not block host
lessons.

## Dependency narrative

```text
013 sensor transport -> validated ClimateSample
014 ClimateSample     -> LCD view + StableRecord
015 scheduler         -> sensor, display, record sink, health indicator
```

The software uses the same nouns. A loop reads as:

```cpp
const TimePoint now = TimePoint (millis ());

station.observe (now);
station.decide  (now);
station.present (now);
```

Transport code never writes the display. Formatting never reads hardware.
The station never interprets a failed transport as a physical measurement.

## Lesson 013: temperature and humidity sensing

### Learning outcome

Acquire a sensor transaction, validate its frame and physical ranges, then
publish one immutable sample. A learner must be able to distinguish:

- no completed transaction;
- transport timeout;
- checksum failure;
- a decoded but invalid measurement;
- a valid fresh measurement;
- a formerly valid measurement that has become stale.

### Public model

Use fixed-point units. Floating point is unnecessary and makes record output
larger and less reproducible.

```cpp
enum struct ClimateSampleState : uint8_t
{
    Unavailable,
    Valid,
    TransportTimeout,
    ChecksumFailure,
    TemperatureOutOfRange,
    HumidityOutOfRange,
    Stale
};

struct ClimateSample
{
    int16_t            temperatureCentiCelsius;
    uint16_t           humidityPermille;
    TimePoint          observedAt;
    ClimateSampleState state;
};

struct ClimateSensor
{
    virtual ~ClimateSensor () noexcept;

    virtual Status        initialize ()                         noexcept = 0;
    virtual void          shutdown   ()                         noexcept = 0;
    virtual Status        update     (TimePoint now)             noexcept = 0;
    virtual ClimateSample sample     (TimePoint now,
                                      Duration staleAfter) const noexcept = 0;
};
```

`ClimateSensor` is a deliberately narrow substitutable capability: the project
must run unchanged against a recorded source, a host fake, and the physical
adapter. It owns no policy beyond one sensor transaction and freshness. If the
virtual call cost fails the measured Mega budget, replace this boundary with a
template in the project coordinator; do not duplicate station logic.

The physical adapter is named after the selected protocol rather than claiming
all sensors behave alike: `Dht11Sensor`. Construction is
inert. `initialize()` claims its data pin and verifies capability without
claiming that a responsive sensor exists. `update(now)` respects the
datasheet's minimum sample interval. An early call leaves the current snapshot
unchanged and returns `Status::Ok`.

Transport and value validity are separate:

| Evidence | `Status` | sample state |
|---|---|---|
| Frame decoded and in range | `Ok` | `Valid` |
| No response before protocol deadline | `HardwareFailure` | `TransportTimeout` |
| Frame timing completed, checksum differs | `HardwareFailure` | `ChecksumFailure` |
| Decoded temperature outside selected part range | `InvalidArgument` | `TemperatureOutOfRange` |
| Decoded humidity above 1000 permille | `InvalidArgument` | `HumidityOutOfRange` |
| Last valid observation exceeds caller policy | `Ok` | `Stale` |

Never replace a last-known-good value silently. A fault snapshot may retain its
numeric fields for diagnosis, but its state controls every consumer.

### Deterministic host source

`RecordedClimateSensor` stores a non-owning pointer and count to a caller-owned,
fixed trace:

```cpp
struct RecordedClimateFrame
{
    TimePoint          availableAt;
    ClimateSample      sample;
    Status             updateStatus;
};
```

It consumes all frames whose `availableAt` is not later than supplied `now`.
Equal inputs yield equal snapshots and status traces. No source consults a
global clock.

### Example and physical evidence

Reference wiring reserves D22 for sensor data and a named TP13 at the data pin.
The exact pull-up value comes from the selected sensor/module datasheet. A
resistor-limited status LED uses:

- slow blue pulse: resource acquired, waiting for first frame;
- steady green: current valid frame;
- two red pulses: transport timeout;
- three red pulses: checksum or decoded-value failure;
- amber: last valid frame is stale.

The LED proves the software state, while TP13 proves the electrical exchange.
A logic analyzer at TP13 should show a request followed by a sensor response;
it does not prove decoded values are physically accurate. The safe-state test
separately confirms D22 becomes high impedance after `shutdown()`.

Host cases: invalid pin, busy pin, repeated initialize, every frame state,
minimum interval boundary, timestamp rollover, stale boundary, recovery after
fault, repeated shutdown, destruction, and byte-identical replay.

## Lesson 014: character LCD and stable records

### Learning outcome

Render an already-decided view and encode the same state as a locale-independent
record. Presentation owns neither sensing nor freshness policy.

### Display boundary

The display owns six `DigitalOutput` endpoints: register select, enable, and
four data lines. Read/write remains tied low, so the lesson never drives the
display bus toward the Mega. The later bus lesson may introduce an I2C
backpack; lesson 014 must not quietly introduce the `I2cBus` planned for lesson
022. Proposed interfaces:

```cpp
struct CharacterDisplay
{
    virtual ~CharacterDisplay () noexcept;

    virtual Status initialize ()                              noexcept = 0;
    virtual void   shutdown   ()                              noexcept = 0;
    virtual Status present    (const CharacterView& view)     noexcept = 0;
};

struct CharacterView
{
    static const uint8_t columns = 16;
    static const uint8_t rows    = 2;

    char line[rows][columns + 1];
};
```

`CharacterView` is complete: every cell is a printable ASCII character and
each row is NUL terminated. `present()` compares with the previous view and
writes only changed runs. It never clears a working display between frames, so
normal updates do not flicker. Failed writes leave the cached view invalid;
the next call redraws the whole screen.

### Stable record grammar

`StableRecord` is a fixed-capacity value, not a stream side effect:

```cpp
struct StableRecord
{
    static const uint8_t capacity = 96;

    char    text[capacity];
    uint8_t length;
};

struct ClimateRecord
{
    uint32_t           sequence;
    TimePoint          observedAt;
    ClimateSampleState state;
    int16_t            temperatureCentiCelsius;
    uint16_t           humidityPermille;
};

Result<StableRecord> formatClimateRecord
    (const ClimateRecord& record) noexcept;
```

Version 1 wire grammar is UTF-8 ASCII with one LF terminator:

```text
adk-climate,1,<sequence>,<milliseconds>,<state>,<signed-centi-C>,<permille>\n
```

Example:

```text
adk-climate,1,42,12500,valid,2317,487
```

Fields never depend on locale. Decimal points, degree symbols, labels, spaces,
and localized status prose belong only in views. Invalid samples retain their
state and use their captured raw decoded values; consumers must not treat those
values as measurements. The formatter writes integers directly into the fixed
buffer and returns `CapacityExceeded` rather than truncating.

`RecordSink` is the substitutable output capability:

```cpp
struct RecordSink
{
    virtual ~RecordSink () noexcept;

    virtual Status initialize ()                         noexcept = 0;
    virtual void   shutdown   ()                         noexcept = 0;
    virtual Status append     (const StableRecord& record) noexcept = 0;
};
```

Serial is one optional sink. A fixed host capture and, later, an SD sink use
the same records. The LCD is not a record sink.

### Display view

For a valid sample:

```text
T +23.17 C
RH  48.7 %  OK
```

For faults, line one shows `SENSOR` and line two contains a stable short token
such as `TIMEOUT`, `CHECKSUM`, `RANGE`, or `STALE`. Text is never the sole
physical observation: the lesson retains the RGB health pattern from 013.

Host cases: every signed temperature boundary, zero and 1000 permille, maximum
sequence/time values, every state token, capacity proof, row padding, changed
run calculation, full redraw after injected bus failure, deterministic bytes,
and absence of locale-sensitive library calls.

## Lesson 015 project: environmental station

### Learning outcome

Schedule acquisition, presentation, records, and health signaling without
blocking or conflating failure domains. A replay trace must reproduce every
view, record byte, and indicator state.

### Coordinator model

```cpp
struct EnvironmentalStationConfig
{
    Duration sampleInterval;
    Duration displayInterval;
    Duration recordInterval;
    Duration staleAfter;
};

enum struct StationHealth : uint8_t
{
    Starting,
    Healthy,
    SensorFault,
    DisplayFault,
    RecordFault,
    MultipleFaults
};

struct EnvironmentalStationSnapshot
{
    ClimateSample sample;
    StationHealth health;
    Status        sensorStatus;
    Status        displayStatus;
    Status        recordStatus;
    uint32_t      recordSequence;
};

struct EnvironmentalStation
{
    EnvironmentalStation (const EnvironmentalStationConfig& config,
                          ClimateSensor&                    sensor,
                          CharacterDisplay&                 display,
                          RecordSink&                       records) noexcept;
    ~EnvironmentalStation () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status observe    (TimePoint now) noexcept;
    Status decide     (TimePoint now) noexcept;
    Status present    (TimePoint now) noexcept;

    EnvironmentalStationSnapshot snapshot () const noexcept;
};
```

The three calls intentionally mirror the example. `observe()` performs only
due sensor work and captures its result. `decide()` derives freshness, view,
record, and health state without hardware writes. `present()` performs due
display and record writes, then reports the aggregate health to the caller for
RGB actuation. A second call at the same timestamp has no effect.

Initialization order is sensor, display, records. Failure rolls back in reverse
order. Shutdown is records, display, sensor. A display or record failure does
not erase a valid sensor sample. A sensor fault is recorded and displayed. A
record failure does not freeze the display. Sequence increments only after a
successful append, so retries preserve identical bytes.

Scheduling uses `now.elapsedSince(lastAttempt) >= interval`, which remains
correct across the 32-bit timestamp rollover. Missed periods coalesce into one
attempt; the station does not burst old work. Valid configuration requires
nonzero intervals and `staleAfter >= sampleInterval`.

### Health observation

The RGB LED is an output owned by the example, not hidden inside the station:

| Health | Circuit-native observation |
|---|---|
| `Starting` | slow blue pulse |
| `Healthy` | green heartbeat after each accepted sample |
| `SensorFault` | two red pulses |
| `DisplayFault` | three amber pulses |
| `RecordFault` | four violet pulses |
| `MultipleFaults` | alternating red and violet |

The LCD shows the latest decided state and is therefore not allowed to prove
its own health. The RGB channel proves coordinator state. TP13 at sensor data,
TP14 at LCD enable, and TP15 at LCD D4 distinguish sensor exchange from display
traffic. An optional Serial sink supplies exact records but is never required
to interpret station health.

### Project build stages

1. Replay recorded frames with no hardware and compare exact snapshots.
2. Run the sensor plus RGB indicator; leave display and records as host fakes.
3. Add LCD presentation; inject an address/write failure and retain sensing.
4. Add the stable Serial record sink and capture with `make serial-log`.
5. Disconnect one subsystem at a time and predict the health pattern first.
6. Record startup, healthy, each isolated fault, combined fault, recovery,
   reset, shutdown, and power-removed observations.

### Required deterministic traces

- startup to first valid frame;
- regular samples with display and record periods that are not multiples;
- transport timeout, checksum failure, invalid values, stale data, and recovery;
- display-only and sink-only failures with continued sensing;
- simultaneous failures and ordered recovery;
- exact interval boundaries and timestamp rollover;
- repeated calls at one timestamp;
- delayed loop with missed periods and no catch-up burst;
- record append failure followed by byte-identical retry;
- initialization failure at each dependency with reverse rollback;
- shutdown from every health state and reinitialization.

### Canonical Mega example outline

Objects appear as platform, resource owners, endpoints, components, behavior:

```cpp
Runtime                    runtime;
I2cBus                     displayBus (...);
Dht11Sensor                climateSensor (...);
Hd44780Display             climateDisplay (...);
SerialRecordSink           climateRecords (...);
EnvironmentalStation       station (...);
RgbLed                     healthLed (...);
StationHealthPattern       healthPattern (...);
```

`setup()` initializes the station and status output, then selects `Starting`.
`loop()` obtains `now`, calls `observe`, `decide`, and `present`, then updates
the health pattern. Low-level pin and address constants follow the narrative
functions. No `delay()` appears.

### Lesson package

The HTML reference carries exact API contracts, record grammar, state-token
table, source/test links, and CLI capture commands. The PDF carries:

- pencil orientation drawing and authoritative wiring table;
- timing diagram for the three schedules;
- fault-isolation tree using RGB, TP13, TP14, and TP15;
- prediction and observation worksheets;
- exact expected stable-record lines;
- staged acceptance card and claim-evidence-reasoning exercise.

Each lesson remains “host verified” until the named Mega, sensor module, LCD,
pull-up, display voltage compatibility, pin assignments, currents, and
observed waveforms are recorded on a bench card.

## Commit boundaries

Land the slice in dependency order:

1. `sensor: model climate measurements`
2. `sensor: add selected transport adapter`
3. `display: add character views`
4. `record: add stable climate format`
5. `project: add environmental station`
6. `lessons: teach environmental evidence`
7. `site: publish environmental station`

The sensor protocol adapter must not share a commit with its domain model.
Formatting and display must remain independently host-testable. Integration
files (`Adk.h`, make fragments, site navigation, shared indexes, release
metadata) belong to the named integrator.
