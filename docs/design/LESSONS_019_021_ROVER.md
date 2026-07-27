# Lessons 019--021 executable design

This is the implementation handoff for the distance, motor, and tabletop-rover
slice. It fixes the teaching boundary and proposed interfaces before code is
added. Names may change during review, but the ownership and safety contracts
must not weaken.

All three lessons remain **host verified** until a person completes the Mega
2560 bench cards. Lesson 019 is E1. Lessons 020 and 021 are E2.

## Teaching progression

| Lesson | New first-class material | Learner's evidence chain |
|---:|---|---|
| 019 | microsecond time, edge capture, ultrasonic ranging | trigger pulse -> echo width -> validity -> distance -> range LED |
| 020 | bounded motor command, reversal dead time, encoder observation | requested motion -> driver state -> enable waveform -> encoder edges |
| 021 | deterministic rover supervisor and scripted route | range and wheel evidence -> decision -> bounded motion -> physical result |

Lesson 019 introduces no motor. Lesson 020 starts with LEDs in place of the
driver, then an unloaded motor with its wheel raised. Lesson 021 first runs the
same route against the host plant, then with both wheels raised, and only then
inside a bounded tabletop or floor enclosure.

## Required dependency boundary

Millisecond `TimePoint` is not precise enough to measure an ultrasonic echo.
Add these core values before the sensor:

```cpp
struct MicrosecondDuration
{
    using Raw = uint32_t;

    explicit MicrosecondDuration (Raw microseconds = 0) noexcept;

    Raw microseconds () const noexcept;
};

struct MicrosecondTimePoint
{
    using Raw = uint32_t;

    explicit MicrosecondTimePoint (Raw microseconds = 0) noexcept;

    Raw                 microseconds () const noexcept;
    MicrosecondDuration elapsedSince (MicrosecondTimePoint earlier) const noexcept;
};
```

Elapsed subtraction uses unsigned wraparound. Durations accepted by this slice
must remain below half the 32-bit range. Do not silently convert between
millisecond and microsecond values.

An interrupt-backed endpoint belongs below the distance and encoder components:

```cpp
enum struct Edge : uint8_t
{
    Rising,
    Falling
};

struct EdgeEvent
{
    Edge                 edge;
    MicrosecondTimePoint observedAt;
};

struct EdgeCapture
{
    EdgeCapture  (ResourceRegistry& resources,
                  PinId             pin) noexcept;
    ~EdgeCapture () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;

    Status update (MicrosecondTimePoint now) noexcept;

    bool      event       () const noexcept;
    EdgeEvent latestEvent () const noexcept;
    bool      overflowed  () const noexcept;
    bool      initialized () const noexcept;
};
```

`EdgeCapture` claims both the pin and its external-interrupt resource. It stores
events in a fixed-capacity ring. The platform ISR records only the edge and
timestamp; it performs no callback, allocation, conversion, or component
update. `update()` transfers a stable snapshot into ordinary code. Overflow is
sticky and explicit. On Mega 2560, examples use only documented external
interrupt pins.

## Lesson 019 — explicit distance validity

### Component boundary

The supported component is `UltrasonicRanger`, initially documented for the
owned HC-SR04-style module in the common Arduino kit. The deterministic engine
does not know centimeters, temperature compensation, or Arduino calls; it
turns timestamped echo observations into a pulse result. A thin component owns
the trigger output and echo capture and applies the configured conversion.

```cpp
enum struct RangeState : uint8_t
{
    Idle,
    WaitingForRise,
    WaitingForFall,
    Ready,
    NoEcho,
    TooNear,
    TooFar,
    CaptureFault
};

struct RangeConfig
{
    MicrosecondDuration triggerWidth;
    MicrosecondDuration riseTimeout;
    MicrosecondDuration echoTimeout;
    uint16_t            minimumMillimeters;
    uint16_t            maximumMillimeters;
    uint32_t            soundSpeedMillimetersPerSecond;
};

struct RangeReading
{
    RangeState          state;
    Status              status;
    uint16_t            millimeters;
    MicrosecondDuration echoWidth;
    uint32_t            sequence;
};

struct UltrasonicRanger
{
    UltrasonicRanger  (ResourceRegistry&  resources,
                       PinId              triggerPin,
                       PinId              echoPin,
                       const RangeConfig& config) noexcept;
    ~UltrasonicRanger () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;

    Status request (MicrosecondTimePoint now) noexcept;
    Status update  (MicrosecondTimePoint now) noexcept;

    RangeReading reading     () const noexcept;
    bool         busy        () const noexcept;
    bool         initialized () const noexcept;
};
```

Construction is inert. `initialize()` validates configuration and capabilities,
claims all resources transactionally, drives trigger low, and begins in
`Idle`. `request()` is accepted only while idle or after a completed reading.
It emits one bounded trigger pulse through a nonblocking update sequence.
`update()` consumes captured edges and changes state. A result snapshot remains
stable until the next accepted request.

`NoEcho` means no rising edge before `riseTimeout`. A rise followed by no fall
means `CaptureFault`, not a very distant object. A complete pulse converting
below or above configured bounds becomes `TooNear` or `TooFar`; it is never
clamped into a valid distance. Conversion uses fixed-point integer arithmetic,
rounds once, and detects overflow. The published default records the assumed
sound speed; it does not claim temperature-corrected accuracy.

Shutdown first drives trigger low, disables capture, then releases claims.
Repeated shutdown is inert. No edge that arrives after shutdown changes the
snapshot.

### Narrative example

Objects appear as runtime, ranger, three range LEDs, and a nonblocking sample
scheduler. `setup()` acquires the LEDs, proves acquisition with a brief
left-to-right light walk, initializes the ranger, then shows ready. `loop()`
reads:

```cpp
void loop ()
{
    observeRange ();
    chooseRangeSignal ();
    showRangeSignal ();
}
```

Green means a valid in-range result, amber means a complete but out-of-range
echo, and red means timeout or capture fault. TP1 is the trigger pin and TP2 is
the echo pin. The learner predicts the trigger pulse, observes TP1/TP2 with a
logic analyzer, and compares echo width with the LED category. Serial may print
the raw width and converted distance but is not required.

### Deterministic tests

- all configuration limits, invalid pin, non-interrupt echo pin, and duplicate
  claims fail before a hardware write;
- initialization rollback at each claim and platform-failure point;
- exact trigger-low, trigger-high, trigger-low trace and repeat interval;
- rising and falling edge at every timeout boundary;
- no rise, no fall, reversed edge, duplicate edge, ring overflow, and stale
  event;
- minimum, maximum, just-outside, conversion rounding, and arithmetic limit;
- request while busy, repeated request after completion, and stable snapshot;
- timestamp wraparound for trigger, rise, and fall;
- repeated initialization, repeated shutdown, destruction while waiting, and
  resource reuse after destruction;
- the same event trace twice produces byte-identical readings.

## Lesson 020 — bounded motor and encoder evidence

### Motor contract

The first implementation targets one low-voltage brushed DC channel behind a
documented H-bridge module. The public component owns two direction outputs and
one PWM enable. It never drives a motor directly.

```cpp
enum struct MotorDirection : uint8_t
{
    Stopped,
    Forward,
    Reverse
};

enum struct MotorPhase : uint8_t
{
    Inactive,
    Running,
    WaitingForDeadTime,
    Fault
};

struct MotorConfig
{
    MicrosecondDuration reversalDeadTime;
    uint8_t             maximumDuty;
};

struct MotorCommand
{
    MotorDirection direction;
    uint8_t        duty;
};

struct MotorSnapshot
{
    MotorCommand requested;
    MotorCommand applied;
    MotorPhase   phase;
    Status       status;
};

struct MotorDriver
{
    MotorDriver  (ResourceRegistry&  resources,
                  PinId              forwardPin,
                  PinId              reversePin,
                  PinId              enablePin,
                  const MotorConfig& config) noexcept;
    ~MotorDriver () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;

    Status command (const MotorCommand& command,
                    MicrosecondTimePoint now) noexcept;
    Status update  (MicrosecondTimePoint now) noexcept;
    Status stop    () noexcept;

    MotorSnapshot snapshot    () const noexcept;
    bool          initialized () const noexcept;
};
```

`Stopped` requires duty zero. A moving command requires duty from one through
`maximumDuty`. A reversal immediately writes duty zero and both direction
inputs inactive. Only after `reversalDeadTime` may it select the opposite
direction and restore bounded duty. A new stop during dead time cancels the
pending command. A same-direction speed change needs no dead time. Any endpoint
failure attempts the inactive state and leaves a sticky `Fault` snapshot until
shutdown and reinitialization.

The semantic inactive state is PWM duty zero plus both direction inputs low.
Shutdown applies it before making pins high impedance. The component exposes no
brake mode in its first interface because H-bridge brake truth tables differ.

### Encoder contract

The first wheel sensor counts one edge per marked interval; it observes motion
but does not infer direction. Direction remains the commanded direction.

```cpp
enum struct EncoderState : uint8_t
{
    Ready,
    Overflow,
    CaptureFault
};

struct EncoderReading
{
    EncoderState state;
    Status       status;
    uint32_t     totalEdges;
    uint16_t     edgesSinceUpdate;
};

struct WheelEncoder
{
    WheelEncoder  (ResourceRegistry& resources,
                   PinId             pin,
                   Edge              countedEdge) noexcept;
    ~WheelEncoder () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (MicrosecondTimePoint now) noexcept;

    EncoderReading reading     () const noexcept;
    bool           initialized () const noexcept;
};
```

Counts are monotonic modulo 32 bits, and delta computation is explicit. Capture
overflow is not silently treated as wheel speed. A later quadrature component
may add observed direction without changing `MotorDriver`.

### Narrative example and observation

The example first replaces the H-bridge inputs with resistor-limited LEDs:
forward LED, reverse LED, and PWM-enable LED. The learner observes the
dead-time dark interval before any motor power is present. The E2 continuation
uses one rated module and unloaded motor.

`loop()` reads:

```cpp
void loop ()
{
    observeOperatorRequest ();
    chooseMotorCommand ();
    applyMotorCommand ();
    showMotionEvidence ();
}
```

TP1 and TP2 are the direction inputs; TP3 is enable. A reflective mark on the
raised wheel makes encoder motion visible beside a diagnostic LED that toggles
on each stable count. The acquisition light walk and the inactive-state
measurement are separate observations. Optional Serial lists requested state,
applied state, and edge delta.

### Deterministic tests

- capability, timer conflict, invalid duty, and configuration validation;
- three-resource transactional initialization and every rollback point;
- stop, forward, reverse, same-direction duty changes, and duty bounds;
- both reversal directions, exact dead-time boundary, time jump, and wrap;
- stop or replacement command during dead time;
- injected failure on each hardware write forces the best attainable inactive
  state and preserves the first failure;
- repeated lifecycle operations, active destruction, and claim reuse;
- encoder exact count, burst, no motion, counter wrap, queue overflow, capture
  failure, shutdown, and restart;
- paired command/count traces expose commanded motion with zero encoder edges.

### E2 hardware boundary

The lesson names one supported driver variant only after its primary datasheet
is selected. Its motor voltage, continuous and peak current, logic thresholds,
flyback strategy, thermal behavior, and truth table must be cited. Record motor
stall current at the chosen supply or use a current-limited source below the
driver and wiring ratings. Logic and motor supplies are drawn separately, with
the required common reference or isolation.

A physical switch removes motor-supply power. It is not wired as a mere Mega
input and is not called a software emergency stop. USB may remain connected
only when the documented module prevents back-powering. Uncertainty blocks the
bench continuation; the LED simulation still completes the software lesson.

## Lesson 021 — deterministic tabletop rover

### Hardware-neutral supervisor

The project behavior accepts observations and emits intent. It owns no GPIO,
clock, sensor, or driver. The Mega sketch adapts the intent to two
`MotorDriver` objects.

```cpp
enum struct RoverMode : uint8_t
{
    Inactive,
    Ready,
    Driving,
    Turning,
    ObstacleHold,
    MotionFault,
    SensorFault,
    RouteComplete,
    Stopped
};

enum struct RouteAction : uint8_t
{
    Drive,
    TurnLeft,
    TurnRight,
    Pause,
    Finish
};

struct RouteStep
{
    RouteAction action;
    uint16_t    targetEdges;
    Duration    maximumDuration;
    uint8_t     duty;
};

struct RoverConfig
{
    uint16_t minimumClearanceMillimeters;
    uint16_t resumeClearanceMillimeters;
    Duration rangeMaximumAge;
    Duration motionStartTimeout;
    Duration wheelMismatchTimeout;
    uint16_t maximumWheelEdgeDifference;
};

struct RoverInput
{
    RangeReading range;
    uint32_t     leftEdges;
    uint32_t     rightEdges;
    bool         startPressed;
    bool         stopRequested;
    TimePoint    observedAt;
};

struct RoverOutput
{
    MotorCommand leftMotor;
    MotorCommand rightMotor;
    RoverMode    mode;
    uint8_t      routeIndex;
    Status       status;
};

struct Rover
{
    Rover (const RoverConfig& config,
           const RouteStep*   route,
           uint8_t            routeLength) noexcept;

    Status initialize () noexcept;
    Status update     (const RoverInput& input) noexcept;
    void   shutdown   () noexcept;

    RoverOutput output () const noexcept;
};
```

The route is caller-owned static storage with a fixed maximum length checked at
initialization. No pointer is retained unless its lifetime contract is explicit
and documented; copying a small bounded route into component storage is
preferred after SRAM measurement.

Every update computes output only from configuration, prior state, current
input, and supplied time. `stopRequested`, stale or invalid range, encoder
capture fault, failure to see initial motion, excessive wheel disagreement,
route timeout, and internal invalid state all request both motors stopped.
Obstacle hold uses hysteresis: stop at or below minimum clearance and resume
only at or above the larger resume clearance with a fresh valid reading.

The software stop button is named `stopRequested`; it is useful evidence but is
not credited as the physical motor-power disconnect. Restart after a fault
requires stop release, a fresh valid range, stationary evidence, and a new
start edge. No automatic restart follows reset or sensor recovery.

### Narrative Mega adapter

Objects appear in this order: runtime, start/stop buttons, range sensor, wheel
encoders, motor drivers, RGB status, static route, rover supervisor. Setup
acquires passive inputs and diagnostics first, proves them, initializes motor
drivers last, commands stop, and waits in ready state.

```cpp
void loop ()
{
    observeRover ();
    decideRoverMotion ();
    actuateRover ();
    showRoverEvidence ();
}
```

The adapter always applies stop commands before showing a fault. It never
allows Serial input to command motion. A compile-time host route and the Mega
route use identical `RouteStep` records.

RGB evidence is blue ready, green driving, violet turning, amber obstacle hold,
red fault, and white route complete; blink cadence distinguishes states in
grayscale. Wheel markers and encoder LEDs verify physical motion. TP-L and TP-R
are the two enable pins: both must measure inactive during startup, hold,
fault, shutdown, and after reset. The acquisition light walk proves object
initialization separately.

### Host plant

Add a small integer-only `DifferentialDrivePlant` under `tests/`, not `src/`.
It accepts left/right motor commands and elapsed time and produces edge deltas
and obstacle range. Parameters include edges per second at full duty, per-wheel
gain, start delay, stopping delay, and an obstacle script. It models only enough
behavior to test the supervisor and makes no physical-accuracy claim.

Golden replay records:

```text
time, range-state, range-mm, left-total, right-total,
start, stop-request, rover-mode, left-command, right-command, route-index
```

The same CSV fixture drives the host test and may be logged by the Mega for
comparison. Serial is supporting evidence, not control or sole diagnosis.

### Project tests

- invalid configuration, empty route, oversized route, invalid step, and duty
  over the configured motor bound;
- acquire-ready-start sequence and no motion before a start edge;
- every route action and transition, exact edge target, exact timeout, large
  update jump, and timestamp wraparound;
- obstacle just outside, at, and inside each hysteresis threshold;
- no echo, stale reading, capture fault, too-near, and too-far policy;
- one or both encoders stalled, delayed first edge, excessive mismatch, burst,
  overflow, and movement reported while stopped;
- stop request from every active state and start/stop chord;
- motor-adapter failure on left first and right first, proving both stop
  commands are attempted;
- reset and reinitialize never resume a route automatically;
- shutdown from every mode emits stopped intent;
- two identical plant runs produce byte-identical golden traces.

## CLI and publication targets

The slice is incomplete until these ordinary repository targets include it:

```text
make host-test
make host-test-sanitize
make header-check
make arduino-Lesson019UltrasonicRange
make arduino-Lesson020MotorDriver
make arduino-Lesson021BenchRover
make size-check
make lessons
make lessons-check
make site
make site-check
make package-smoke
make quality
```

Hardware-only helpers should remain CLI-driven:

```text
make upload EXAMPLE=Lesson019UltrasonicRange PORT=/dev/ttyACM0
make serial-log PORT=/dev/ttyACM0 SERIAL_LOG=build/lesson019.csv
make monitor PORT=/dev/ttyACM0 BAUD=115200
```

The PDFs contain prediction tables, pencil orientation drawings, authoritative
schematics, TP waveforms, driver-rating worksheet, current and stall-current
record, physical power-removal check, wheels-raised sequence, bounded-area
sequence, failure injection, and unsigned hardware acceptance cards. HTML
contains the searchable API, state tables, complete canonical sketch links,
test traces, corrections, datasheet links, and explicit host-only status.

## Coherent implementation commits

Land only after prior curriculum dependencies exist:

1. `time: add microsecond values`
2. `input: add deterministic edge capture`
3. `range: add explicit ultrasonic readings`
4. `lesson: add observable distance sensing`
5. `motor: add bounded driver channel`
6. `encoder: add wheel motion evidence`
7. `lesson: add safe motor observation`
8. `project: add deterministic bench rover`
9. `lessons: publish rover project`

Shared indexes, `Adk.h`, Make fragments, site navigation, size baselines, and
status tables belong to the integrator at the appropriate boundary.

## Deferred physical decisions

The code may progress without bench testing, but publication must label these
items open:

- exact owned ultrasonic module and its minimum cycle time;
- exact owned motor, encoder geometry, driver module, and primary datasheets;
- measured motor stall current and selected current limit;
- driver logic compatibility, flyback implementation, thermal limit, and
  back-power behavior;
- physical disconnect, fuse, wire, connector, restraint, guard, and bounded
  test area;
- measured trigger/echo timing, PWM enable, dead time, encoder integrity,
  stopping distance, loss-of-control stop, and reset behavior.

No lesson claims obstacle avoidance as a protective function. Range sensing,
software stop, RAII shutdown, and deterministic tests do not replace the
physical motor-power disconnect or supervision.
