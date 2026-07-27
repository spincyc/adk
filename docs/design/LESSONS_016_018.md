# Lessons 016–018 executable design

Status: interface proposal with a host-verified keypad interpreter. Hardware
adapters, examples, PDFs, and bench acceptance remain deferred. These lessons
do not secure property and must be labelled an inert operator-input trainer.

## Progression

| Lesson | New interface | Learner evidence |
|---:|---|---|
| 016 | `MatrixKeypad` adapter and deterministic `Keypad` interpreter | Key label on the display; row test points show one bounded scan pulse |
| 017 | `BoundedServo` and versioned `ServoCalibration` | Pointer moves only between two marked stops; TP-S shows bounded pulses |
| 018 | `AccessTrainer` composition | Display, RGB state, and soft latch agree with a replayable audit record |

The implementation order is keypad interpreter, matrix electrical adapter,
servo pulse endpoint, bounded servo, access state machine, then the project
composition. No project type reaches around a component to manipulate pins,
timers, storage, or clocks.

## Lesson 016 — keypad and operator events

The electrical adapter owns four row outputs and four column inputs. It drives
at most one row low, leaves inactive rows high impedance, and samples pull-up
columns. A complete scan produces a 16-bit mask; the lesson uses twelve keys
and rejects the unused four positions. Shutdown returns every row and column to
high impedance.

The supported interpreter is `Keypad` in `src/keypad.h`. Its input is an
explicit timestamped `KeypadSample`, so host replay and a future matrix adapter
share one state machine. It reports:

- exactly one debounced key as `Pressed`;
- no key as `Released`;
- two or more keys as `InvalidChord`;
- an electrical scan failure as `Fault`;
- stable press and release events until the next `update()`.

An invalid chord disarms input until every key is released. This prevents a
three-key matrix ghost from becoming a credential digit as fingers lift.
The interpreter allocates nothing, invokes no callbacks, and accepts timer
wraparound. Tests cover bounce, held keys, chord-to-single transitions, fault,
release gating, lifecycle, and wraparound.

The canonical example should read:

```text
loop:
    observeKeypad()
    decideOperatorEvent()
    showOperatorEvidence()
```

Do not echo a complete entered code. The seven-segment or LCD shows the current
key during the lesson, then changes to a count of accepted keys in lesson 018.
The RGB indicator distinguishes ready, accepted event, invalid chord, and scan
fault. TP-R0 is the non-Serial acquisition signal; with a logic probe it shows
the row scan only after all seven pin claims succeed. All row pins becoming
high impedance is separate safe-state evidence.

## Lesson 017 — bounded servo

Use a small documented 5 V hobby servo with a separate current-limited supply,
common ground, a stable fixture, and an independent load-power switch. The
Mega pin carries signal only. This is energy class E2 and remains draft until a
person records stall current, supply limit, fixture, and bench observations.

Proposed declarations:

```cpp
struct ServoCalibration
{
    uint16_t closedPulseMicroseconds;
    uint16_t openPulseMicroseconds;
    uint16_t minimumPulseMicroseconds;
    uint16_t maximumPulseMicroseconds;
    uint16_t version;
    uint16_t checksum;
};

struct ServoPosition
{
    uint16_t permille;
};

struct BoundedServo
{
    Status initialize ();
    void   shutdown   ();
    Status command    (ServoPosition position);
    Status update     (TimePoint now);

    bool              initialized ();
    ServoSnapshot     snapshot    ();
};
```

`initialize()` validates version, checksum, monotonic endpoints, absolute pulse
bounds, pin capability, and exclusive timer ownership before producing a
pulse. A bad record returns `InvalidArgument` and produces no pulse. A command
is clamped only to the validated closed/open envelope; configuration outside
the absolute safe range is rejected rather than repaired.

The first exercise uses an unattached paper pointer. The learner marks closed
and open stops on a card and commands only those endpoints. TP-S at the signal
pin is the primary electrical evidence; the pointer is the physical evidence.
A status LED proves initialization before load power is applied. Physical power
removal, not `shutdown()`, is the stop method.

Host tests must cover every invalid calibration field, claim rollback, timer
conflict, pulse endpoints, repeated commands, update jitter bounds, timestamp
wrap, injected driver failure, repeated shutdown, destruction while active,
and reuse of the pin and timer after destruction. A recording pulse driver
keeps the state machine independent of Arduino's global Servo library.

## Lesson 018 — inert access-control trainer

This project is a tabletop state-machine trainer. Its latch is a foam flag or
paper pointer. It is not a lock, door controller, alarm, authentication
component, or security recommendation.

Proposed deterministic input:

```cpp
struct AccessInput
{
    KeypadSnapshot keypad;
    TimePoint      now;
};
```

Proposed public snapshot:

```cpp
enum struct AccessState : uint8_t
{
    Ready,
    Entering,
    Granted,
    Denied,
    LockedOut,
    Fault
};

struct AccessSnapshot
{
    AccessState state;
    Status      status;
    uint8_t     enteredCount;
    uint8_t     failedAttempts;
    bool        softLatchOpen;
    bool        clearEntry;
    bool        appendAuditRecord;
};
```

The credential fixture is a four-key teaching sequence supplied explicitly to
the constructor. It is not persisted as plaintext by a storage component.
Digits append; `Star` clears; `Hash` submits. A wrong-length or wrong-value
submission has identical public timing and display behavior. Three denied
submissions enter a timed lockout. Time rollback, keypad fault, corrupt
configuration, display failure, or actuator failure enters `Fault`, requests a
closed soft latch, and requires an explicit reset after the fault clears.

The coordinator never commands pulse widths. It publishes
`softLatchOpen`; the example translates that domain intent to the two validated
`BoundedServo` endpoints. RGB patterns are ready=blue, entering=white,
granted=green, denied=amber, locked-out=slow amber pulse, fault=red. The display
shows prompts and counts, never the credential. A piezo cue is optional and
cannot be the sole evidence.

Required replay traces include:

1. correct sequence, submit, bounded grant interval, automatic close;
2. wrong sequence, clear, retry, and success;
3. three failures, exact lockout boundary, one tick before and after;
4. invalid chord and stuck key without appended digits;
5. timestamp wrap during grant and lockout;
6. keypad, display, audit, and actuator failures from every active state;
7. reset and shutdown from every state;
8. the same trace twice with byte-identical snapshots and audit intents.

Audit output is an intent handed to a later record sink, not hidden I/O. Records
contain sequence number, event kind, result, and monotonic elapsed time; they
contain no entered digits. Storage failure is visible and closes the soft latch.

## Lesson package outlines

Each HTML page carries the exact API, resource table, status patterns, source
links, CLI commands, host traces, and deferred bench card. Each PDF adds:

- a pencil orientation drawing plus an authoritative pin table;
- predict–observe–interpret worksheets;
- state and timing diagrams;
- fault trees separating acquisition, safe state, and physical effect;
- blank measurement and bench-acceptance records;
- exercises that replay traces before changing hardware.

Suggested Mega allocation is keypad rows D22–D25, columns D26–D29, RGB D5–D7,
display on the lesson-010 shift-register pins, servo signal D44 with its actual
timer documented by the chosen pulse endpoint, and a dedicated status LED on
D13. Integration must run the board capability and timer-conflict check before
these pins become canonical.

All workflows remain CLI-driven:

```sh
make host-test-keypad
make host-test-access-trainer
make arduino-Lesson016MatrixKeypad
make arduino-Lesson017BoundedServo
make arduino-Lesson018AccessTrainer
make upload EXAMPLE=Lesson018AccessTrainer PORT=/dev/ttyACM0
make monitor PORT=/dev/ttyACM0
make lessons
make site
```

These target names are requirements for integration, not claims that the
unintegrated targets already exist.
