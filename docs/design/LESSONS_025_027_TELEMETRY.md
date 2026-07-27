# Lessons 025--027 executable design

This handoff defines the receive-only telemetry slice. It deliberately separates
electrical capture, protocol interpretation, record encoding, and operator
presentation. No interface in this slice transmits, replays, clones, brute
forces, jams, or maps an observed command to access control.

Lessons 025--027 are E1 when using wired or infrared kit hardware and E0 when
using prerecorded radio captures. Radio work remains passive, lawful, and
receive-only. The telemetry console is educational and must not monitor or
dispatch a safety-critical condition.

## Teaching progression

| Lesson | New first-class material | Learner's evidence chain |
|---:|---|---|
| 025 | bounded pulse capture, infrared frame decoding, record encoding | observed light -> pulse train -> frame validity -> stable record -> cue LED |
| 026 | receive-only sample records, packet integrity, freshness | synthetic/received samples -> packet checks -> accepted observation -> age display |
| 027 | deterministic multi-source telemetry console | source health -> normalized reading -> console decision -> display/alarm/log evidence |

The code and lessons use four distinct words:

- **capture** is timestamped physical evidence;
- **frame** is a bounded candidate protocol unit;
- **packet** is an ADK-owned telemetry envelope;
- **record** is the stable text or binary representation written to storage.

These terms must not be interchanged.

## Shared bounded data types

Do not use `String`, heap storage, stream insertion, locale-sensitive
formatting, or unbounded parsing.

```cpp
struct ByteSpan
{
    const uint8_t* data;
    uint16_t       size;
};

struct MutableByteSpan
{
    uint8_t* data;
    uint16_t capacity;
};

struct TextSpan
{
    const char* data;
    uint16_t    size;
};

struct MutableTextSpan
{
    char*    data;
    uint16_t capacity;
};
```

Every encoder returns both status and bytes written. Every decoder accepts an
explicit length, rejects trailing data when its contract requires an exact
record, and leaves output unchanged on failure.

## Lesson 025 — evidence before command meaning

### Capture boundary

The infrared receiver module produces a demodulated digital pulse stream. A
generic capture endpoint owns the pin and interrupt resource; the decoder
consumes immutable pulse records. `PulseCapture` may share the lower-level edge
capture introduced earlier, but it adds no protocol knowledge.

```cpp
enum struct PulseLevel : uint8_t
{
    Mark,
    Space
};

struct Pulse
{
    PulseLevel          level;
    MicrosecondDuration duration;
};

enum struct CaptureState : uint8_t
{
    Idle,
    Capturing,
    Complete,
    Overflow,
    TimingFault
};

struct PulseCaptureConfig
{
    MicrosecondDuration frameGap;
    MicrosecondDuration minimumPulse;
    MicrosecondDuration maximumPulse;
};

struct PulseFrame
{
    const Pulse* data;
    uint8_t      size;
    uint32_t     sequence;
    CaptureState state;
};

struct PulseCapture
{
    static constexpr uint8_t capacity = 100;

    PulseCapture  (ResourceRegistry&        resources,
                   PinId                    pin,
                   const PulseCaptureConfig& config) noexcept;
    ~PulseCapture () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (MicrosecondTimePoint now) noexcept;

    PulseFrame frame       () const noexcept;
    Status     acknowledge (uint32_t sequence) noexcept;
};
```

Construction is inert. Initialization validates timing before claiming pin and
interrupt. Capture begins only after an idle gap. A complete snapshot remains
stable until acknowledged with the matching sequence. New edges while a frame
is awaiting acknowledgement set an explicit overrun status; they do not mutate
the published frame. The ISR records bounded edge timestamps only.

The public decoder interface is independent of Arduino and of the receiver:

```cpp
enum struct InfraredProtocol : uint8_t
{
    Unknown,
    Nec
};

enum struct FrameValidity : uint8_t
{
    Valid,
    Repeat,
    UnknownProtocol,
    TimingInvalid,
    IntegrityInvalid,
    Truncated,
    Overflow
};

struct InfraredFrame
{
    InfraredProtocol protocol;
    FrameValidity    validity;
    uint32_t         address;
    uint32_t         command;
    uint32_t         captureSequence;
};

struct InfraredDecoder
{
    Status decode (const PulseFrame& capture,
                   InfraredFrame&    output) const noexcept;
};
```

Version one recognizes one published, owned, harmless kit-remote format only.
The decoder reports unknown evidence without guessing. It validates leader,
bit timing, exact length, inverse/check fields, and repeat form. Tolerances are
configuration constants justified by the protocol source and measured receiver
behavior. The first API exposes numeric fields, not a semantic action such as
`unlock`, `launch`, or `open`.

### Stable capture record

`InfraredRecordEncoder` writes a canonical one-line record into caller storage:

```text
IR1,<sequence>,<protocol>,<validity>,<address-hex>,<command-hex>\n
```

Hex width and case, decimal rules, field order, newline, and overflow behavior
are fixed. Unknown or invalid frames retain evidence state but do not invent
address or command values. The encoder is independently golden-vector tested.

### Narrative example and observability

The supported hardware example uses an owned kit infrared remote pointed at an
ordinary receiver module. The exercise may also replay a compiled-in synthetic
pulse fixture through the decoder without any emitter.

```cpp
void loop ()
{
    observeInfrared ();
    interpretFrameEvidence ();
    recordFrameEvidence ();
    showFrameEvidence ();
}
```

Blue means waiting, green means valid owned-protocol frame, amber means unknown
protocol, and red means timing/integrity/overflow fault. Distinct blink cadence
keeps states distinguishable without color. TP1 is the receiver data output;
the learner compares its pulse train with the frame LED and canonical record.
Serial can show the record but is not the sole evidence. No example emits IR.

### Deterministic tests

- invalid capture timing, invalid/non-interrupt pin, duplicate claim, and every
  initialization rollback;
- empty, minimum, maximum, and over-capacity captures;
- exact gap threshold, pulse limits, timestamp wrap, queue overrun, stale
  acknowledgement, and repeated acknowledgement;
- NEC zero, one, mixed, repeat, exact-boundary timing, inverted-byte failure,
  short, long, and unknown frames;
- noisy prefix/suffix cannot be accepted as a valid embedded frame;
- decoder leaves output unchanged on failure where specified;
- canonical record golden bytes, smallest buffer, one-byte-short buffer, and
  no partial record on overflow;
- repeat initialization, shutdown while capturing, destruction, and claim
  reuse;
- identical pulse traces produce byte-identical frames and records.

## Lesson 026 — receive-only observations and packet integrity

### Safety boundary

The executable lesson uses generated fixtures, saved captures, or a wired
logic-level receiver module whose exact frequency, voltage, lawful use, and
ownership have been reviewed. It adds no RF transmitter, waveform generator,
remote-command mapping, antenna amplifier, rolling-code analysis, key recovery,
or raw replay export.

Raw captures may contain information belonging to other people. The lesson
records only owned lab fixtures. Logs use opaque source identifiers chosen by
the learner, not device serial numbers, addresses, locations, credentials, or
payloads that are unnecessary for the experiment.

### ADK telemetry packet

Packetization is an ADK-owned lab format, not an observed radio protocol. It
lets prerecorded fixtures and later wired sources share deterministic tests.

```cpp
enum struct TelemetryKind : uint8_t
{
    Temperature,
    RelativeHumidity,
    Distance,
    Contact,
    Counter
};

enum struct SampleQuality : uint8_t
{
    Valid,
    SensorFault,
    OutOfRange,
    StaleAtSource
};

struct TelemetrySample
{
    uint16_t      sourceId;
    uint16_t      sequence;
    uint32_t      observedMilliseconds;
    TelemetryKind kind;
    SampleQuality quality;
    int32_t       value;
    int8_t        decimalExponent;
};

enum struct PacketValidity : uint8_t
{
    Valid,
    BadVersion,
    BadLength,
    BadType,
    BadQuality,
    BadIntegrity,
    TrailingData
};

struct TelemetryPacketCodec
{
    static constexpr uint8_t version = 1;
    static constexpr uint8_t size    = 19;

    Result<uint16_t> encode (const TelemetrySample& sample,
                             MutableByteSpan         output) const noexcept;
    PacketValidity   decode (ByteSpan                packet,
                             TelemetrySample&        output) const noexcept;
};
```

The wire format fixes magic, version, byte order, signed representation, exact
length, and CRC-16 parameters in the lesson appendix. It contains no pointer,
padding, native `enum`, native `bool`, or compiler-dependent struct image.
Integrity detects accidental corruption; the lesson explicitly says CRC is not
authentication, confidentiality, provenance, or replay protection.

### Sequence and freshness tracker

```cpp
enum struct SequenceState : uint8_t
{
    First,
    InOrder,
    Duplicate,
    Gap,
    Reordered
};

enum struct Freshness : uint8_t
{
    Fresh,
    Aging,
    Stale
};

struct ObservationState
{
    TelemetrySample sample;
    SequenceState   sequenceState;
    Freshness       freshness;
    Duration        age;
    Status          status;
};

struct ObservationTrackerConfig
{
    Duration agingAfter;
    Duration staleAfter;
};

struct ObservationTracker
{
    ObservationTracker (uint16_t                        sourceId,
                        const ObservationTrackerConfig& config) noexcept;

    Status initialize () noexcept;
    Status accept     (const TelemetrySample& sample,
                       TimePoint              receivedAt) noexcept;
    Status update     (TimePoint now) noexcept;

    ObservationState state () const noexcept;
};
```

Age is based on local receipt time, not a remote timestamp. Remote timestamps
remain evidence and are never trusted to extend freshness. Sequence comparison
uses a documented half-range rule. Duplicate and reordered samples do not
replace the last accepted value. A forward gap may update the value while
remaining explicitly visible. Source mismatch is invalid input.

### Receive adapter

`PacketReceiver` is a semantic interface implemented by test fixtures, a
recorded-file CLI tool, or a reviewed receive-only hardware adapter:

```cpp
struct PacketObservation
{
    ByteSpan  packet;
    TimePoint receivedAt;
    uint32_t  captureSequence;
};

struct PacketReceiver
{
    virtual ~PacketReceiver () noexcept;

    virtual Status update      (TimePoint now) noexcept = 0;
    virtual bool   observation () const noexcept         = 0;
    virtual PacketObservation latest () const noexcept   = 0;
    virtual Status acknowledge (uint32_t captureSequence) noexcept = 0;
};
```

Use runtime polymorphism only if measured flash/RAM accepts it and `-fno-rtti`
still passes. Otherwise use explicit composition or a small tagged adapter.
No callback may outlive the receiver.

### Narrative example and observability

The canonical Mega example decodes compiled-in packet fixtures on a fixed
schedule; an optional reviewed receive-only adapter can be a separate example.
An RGB status shows accepted/fresh, gap/aging, stale, and corrupt states.
A seven-segment or LCD view alternates source ID, value, and age category.

```cpp
void loop ()
{
    observePacket ();
    verifyPacket ();
    updateFreshness ();
    showObservationEvidence ();
}
```

TP1 is available only in the separate physical receiver example and names the
module data output. The fixture example needs no RF hardware and offers the
same packet/state evidence. Serial may write canonical records but never
controls receiver tuning or replay.

### Deterministic tests

- golden packet byte layout for positive, negative, zero, and scale extremes;
- every malformed field, all one-bit corruption positions, truncation,
  extension, unaligned input storage, and output-buffer overflow;
- decode failure leaves caller output unchanged;
- first, in-order, duplicate, gap, reorder, wrap, and half-range ambiguity;
- local age exactly before, at, and after aging/stale thresholds;
- remote clock reversal or future time does not alter freshness;
- invalid quality remains distinct from transport corruption and staleness;
- acknowledgement, overrun, receiver fault, shutdown, and restart traces;
- identical input packets and receipt times produce identical state records.

## Lesson 027 — multi-source telemetry console

### Normalized console model

The console engine owns no receiver, display, storage, or clock. It accepts a
complete fixed-size observation set and emits display/log/alarm intent.

```cpp
enum struct ConsoleHealth : uint8_t
{
    Starting,
    Healthy,
    Degraded,
    Fault,
    Stopped
};

enum struct ConsoleSignal : uint8_t
{
    None,
    Notice,
    Attention
};

struct ConsoleSource
{
    uint16_t        sourceId;
    TelemetryKind   kind;
    SampleQuality   quality;
    SequenceState   sequenceState;
    Freshness       freshness;
    int32_t         value;
    int8_t          decimalExponent;
};

struct ConsoleInput
{
    const ConsoleSource* sources;
    uint8_t              sourceCount;
    bool                 nextPressed;
    bool                 acknowledgePressed;
    TimePoint            observedAt;
};

struct ConsoleOutput
{
    ConsoleHealth health;
    ConsoleSignal signal;
    uint8_t       selectedSource;
    bool          writeRecord;
    Status        status;
};

struct TelemetryConsole
{
    static constexpr uint8_t sourceCapacity = 8;

    explicit TelemetryConsole (uint8_t expectedSources) noexcept;

    Status initialize () noexcept;
    Status update     (const ConsoleInput& input) noexcept;
    void   shutdown   () noexcept;

    ConsoleOutput output () const noexcept;
};
```

The input set is sorted by configured source slot, not packet arrival order.
Missing slots are explicit stale observations. `Healthy` requires every
expected source to be valid and fresh. `Degraded` covers an aging source or a
sequence gap. `Fault` covers stale, invalid-quality, corrupt/missing, duplicate
identity, or impossible configuration. Acknowledgement silences an audible
attention pattern but never changes source health or erases evidence.

This is an educational annunciator, not a burglar, fire, medical, industrial,
environmental-safety, or emergency system. It does not contact people or
services and makes no dispatch decision.

### Logging contract

The project reuses the owned bus/storage interfaces from lessons 022--024.
Presentation and storage failures cannot rewrite observation state. A bounded
`TelemetryRecordEncoder` writes one canonical CSV line:

```text
TEL1,<local-ms>,<source>,<kind>,<quality>,<sequence-state>,
<freshness>,<value>,<exponent>,<console-health>\n
```

The actual record has no whitespace or line break between fields. Fields are
ASCII with fixed spelling, decimal grammar, and `\n`. No user-controlled text
enters the record, so formula injection and delimiter escaping are absent by
construction. Storage failure sets a separate log diagnostic and never
converts a stale source to healthy.

The logger applies a deterministic policy: write on accepted sample, health
transition, operator acknowledgement, or periodic heartbeat. Simultaneous
reasons produce one record with a fixed priority field. A failed write remains
visible and is retried only according to a bounded configured policy; no
unbounded queue grows in SRAM.

### Narrative example

Objects appear in dependency order: runtime, buttons, display, RGB/piezo
diagnostics, optional storage, fixed packet sources, trackers, console. The
supported first sketch uses two wired/synthetic sources plus one prerecorded
receive fixture. It does not require an RF adapter to complete the lesson.

```cpp
void loop ()
{
    observeTelemetrySources ();
    decideConsoleState ();
    presentConsoleState ();
    recordConsoleEvidence ();
}
```

The display alternates value and freshness without hiding raw quality.
RGB/cadence states are blue starting, green healthy, amber degraded, red fault,
and unlit stopped. Piezo is bounded to a brief attention cue and can be
acknowledged; the red fault evidence remains. Storage activity has its own LED,
so source health and record success cannot be confused. Optional Serial emits
the exact same canonical record bytes stored to the card.

### Project tests

- zero, maximum, and over-capacity source configurations;
- source ordering independent of packet arrival ordering;
- startup with no samples, one-by-one recovery, all healthy, aging, stale,
  invalid quality, gap, corrupt input, duplicate ID, and missing slot;
- exact health precedence when multiple faults coexist;
- selection next-edge, wrap, held button, invalid chord, acknowledgement,
  attention silence, and new-fault reannouncement;
- source age and heartbeat boundaries across timestamp wrap;
- record triggers, simultaneous-trigger priority, exact golden CSV, smallest
  buffer, one-byte-short buffer, storage failure, bounded retry, and restart;
- display failure and storage failure never alter motor/actuator state because
  this project has no actuator;
- shutdown from every health state produces stopped/unlit/silent output intent;
- two replays of mixed wired and recorded fixtures are byte-identical.

## Security and privacy review

This slice teaches evidence integrity, not adversarial protocol security.

- CRC detects random corruption; it does not authenticate a sender.
- Source IDs organize owned fixtures; they do not prove identity.
- Sequence values expose duplicates and gaps; they do not prevent replay.
- Local freshness bounds data age; it does not prove a remote clock or sensor.
- An RF receiver can capture unrelated traffic; test only controlled fixtures.
- Logs minimize identifiers and payload, have a documented retention location,
  and are not published with raw third-party captures.
- Unknown frames remain unknown and have no semantic command mapping.
- No decoded value authorizes motion, access, ignition, transmission, or a
  safety-critical action.

An authenticated telemetry extension would require a separate threat model,
key lifecycle, nonce/replay design, cryptographic implementation review, and a
more capable platform. It must not be implied by this lesson.

## CLI and fixture workflow

All supported work is repository-driven:

```text
make host-test
make host-test-sanitize
make telemetry-fixtures-check
make telemetry-golden-update APPROVE=1
make arduino-Lesson025InfraredEvidence
make arduino-Lesson026ReceiveOnlyTelemetry
make arduino-Lesson027TelemetryConsole
make size-check
make lessons
make lessons-check
make site
make site-check
make package-smoke
make quality
```

`telemetry-golden-update` writes only derived ADK packet/record fixtures. It
must reject raw unknown RF captures and requires explicit approval to replace
goldens. Ordinary checks never rewrite fixtures.

Hardware helpers remain CLI-driven:

```text
make upload EXAMPLE=Lesson025InfraredEvidence PORT=/dev/ttyACM0
make serial-log PORT=/dev/ttyACM0 SERIAL_LOG=build/lesson025.csv
make monitor PORT=/dev/ttyACM0 BAUD=115200
```

There is intentionally no transmit, replay, clone, brute-force, tune-and-scan,
or protocol-export Make target.

## PDF and HTML division

Lesson PDFs provide pencil orientation diagrams, authoritative receiver wiring,
pulse worksheets, packet-layout worksheets, predict/observe/interpret tables,
privacy review, fault injection, golden-record comparison, and unsigned bench
cards. Timing drawings are accompanied by textual mark/space tables.

HTML provides searchable APIs, byte-layout tables, state precedence, canonical
example downloads, generated golden fixtures, test links, primary protocol and
module references, corrections, and explicit host/fixture verification status.
Neither format may label prerecorded RF evidence as a live hardware result.

## Coherent implementation commits

Land after bus, storage, scheduler, display, and operator-input prerequisites:

1. `capture: add bounded pulse evidence`
2. `infrared: decode owned protocol frames`
3. `records: add canonical infrared encoding`
4. `lesson: add infrared evidence`
5. `telemetry: add canonical packet codec`
6. `telemetry: track sequence and freshness`
7. `lesson: add receive-only observations`
8. `project: add deterministic telemetry console`
9. `records: add bounded telemetry logging`
10. `lessons: publish telemetry console`

The integrator owns shared headers, Make fragments, indexes, navigation, size
baselines, generated PDFs, and status tables.

## Deferred physical decisions

Implementation may continue with synthetic fixtures while these remain open:

- exact owned infrared receiver and harmless remote, supply, demodulation
  polarity, and primary protocol reference;
- exact lawful receive-only radio module, band, location, antenna, voltage
  compatibility, and primary datasheet;
- whether a hardware radio adapter is useful after the fixture lesson already
  meets the learning objective;
- measured capture jitter, ISR capacity, overflow behavior, and SRAM budget;
- log-media capacity, removal behavior, corrupt-media recovery, and retention;
- physical Mega acceptance for startup, fault, shutdown, power removal,
  receiver disconnect, storage failure, and timestamp wrap evidence.

Unresolved legality, privacy, frequency, module identity, voltage, or protocol
ownership blocks the live-radio adapter, not the synthetic lesson. ADK remains
receive-only and provides no unknown-protocol replay path.
