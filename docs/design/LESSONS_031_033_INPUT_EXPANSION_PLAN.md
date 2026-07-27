# Lessons 031–033 input expansion plan

Status: proposed delivery boundary, 2026-07-27. This plan extends the completed
001–030 spine. It does not renumber or replace any existing lesson.

## Decision

The first expansion block teaches two missing, high-reuse input mechanisms:

| Lesson | Kind | Subject | Required outcome |
|---:|---|---|---|
| 031 | Component | Two-axis analog joystick | Calibrated axes, center dead zone, and switch events remain distinct from raw observations |
| 032 | Component | Quadrature rotary encoder | Every valid phase transition produces a deterministic signed count; invalid transitions remain observable |
| 033 | Project-bearing | Calibration console | Joystick selection, encoder trimming, explicit commit/cancel, and visible preview replay exactly |

The tilt-ball switch does not receive a first-class adapter in this block. Its
electrical behavior is already represented by `DigitalInput`; a semantic
`TiltSwitch` adds little reuse compared with joystick calibration or quadrature
decoding. It can be a short exercise in 031 without entering the supported API.

## Vendor-name boundary

Elegoo's revisioned tutorial calls the parts “Joystick module” and “Rotary
Encoder Module.” ADK must not claim that an unmarked board is a KY-023 or
KY-040. Those catalog names are common vendor aliases, not identities established
by the official kit manifest.

The lesson accepts only these observed signal roles:

| Tutorial/module label | ADK role | Required inspection |
|---|---|---|
| `VRx`, `X`, or `HOR` | joystick X analog signal | Identify with the board unpowered; verify 0–5 V before connection |
| `VRy`, `Y`, or `VER` | joystick Y analog signal | Identify with the board unpowered; verify 0–5 V before connection |
| `SW`, `KEY`, or `SEL` | joystick push switch | Determine whether it closes to ground; default lesson assumes active-low |
| `CLK`, `A`, or `S1` | encoder phase A | Confirm it is a logic output/contact, not supply |
| `DT`, `B`, or `S2` | encoder phase B | Confirm it is a logic output/contact, not supply |
| encoder `SW` or `KEY` | separate push switch | Use an ordinary `Button`; it is not part of `QuadratureEncoder` |
| `+`, `VCC`, or `5V` | module supply | Confirm actual module marking and rating |
| `-`, `GND`, or `0V` | common ground | Continuity-check against the ground terminal |

Pin order is never inferred from board color, internet photographs, or an alias.
Unknown markings stop the hardware procedure. Axis direction and encoder
direction are configuration, not universal properties of these modules.

## Lesson 031: analog joystick

### Dependency boundary

Requires `AnalogInput`, `Button`, `LinearCalibration`, `Deadband`,
`ResourceRegistry`, and explicit `TimePoint`. It adds no ADC-reference control
and does not call `analogReference()`. The shared ADC/reference acquisition
policy remains a future platform owner.

### Public interface

Files: `src/analog_joystick.h` and `src/analog_joystick.cpp`.

```cpp
namespace adk {

    struct JoystickAxisConfig
    {
        JoystickAxisConfig (PinId    pin,
                            uint16_t center,
                            uint16_t observedMinimum,
                            uint16_t observedMaximum,
                            uint16_t deadZone,
                            bool     inverted = false) noexcept;

        PinId    pin;
        uint16_t center;
        uint16_t observedMinimum;
        uint16_t observedMaximum;
        uint16_t deadZone;
        bool     inverted;
    };

    struct AnalogJoystickConfig
    {
        AnalogJoystickConfig (const JoystickAxisConfig& xAxis,
                              const JoystickAxisConfig& yAxis,
                              const ButtonConfig&       selectButton) noexcept;

        JoystickAxisConfig xAxis;
        JoystickAxisConfig yAxis;
        ButtonConfig       selectButton;
    };

    struct JoystickAxisSnapshot
    {
        uint16_t raw;
        int16_t  position;
        bool     centered;
        bool     saturated;
    };

    struct AnalogJoystickSnapshot
    {
        JoystickAxisSnapshot x;
        JoystickAxisSnapshot y;
        bool                 rawSelected;
        bool                 selected;
        bool                 selectEvent;
        bool                 releaseEvent;
        Status               status;
    };

    struct AnalogJoystick
    {
        static constexpr int16_t minimumPosition = -1000;
        static constexpr int16_t maximumPosition =  1000;

        AnalogJoystick  (ResourceRegistry&           resources,
                         const AnalogJoystickConfig& config) noexcept;
        ~AnalogJoystick () noexcept;

        AnalogJoystick            (const AnalogJoystick&) = delete;
        AnalogJoystick& operator= (const AnalogJoystick&) = delete;
        AnalogJoystick            (AnalogJoystick&&)      = delete;
        AnalogJoystick& operator= (AnalogJoystick&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now) noexcept;

        bool                   initialized () const noexcept;
        AnalogJoystickSnapshot snapshot    () const noexcept;

        const AnalogInput& xInput      () const noexcept;
        const AnalogInput& yInput      () const noexcept;
        const Button&      selectButton () const noexcept;
    };
}
```

`position` is a fixed, dimensionless range. Values below `center - deadZone`
map linearly from `observedMinimum` to `-1000`; values above
`center + deadZone` map to `+1000`. The center interval maps to zero. Mapping
uses 32-bit intermediates, clamps before narrowing, and applies `inverted`
last. `saturated` means the raw value is at or beyond a configured observed
endpoint; it is evidence, not a hardware-failure claim.

Construction is inert. Initialization validates all configuration and board
capabilities before claims, then acquires X, Y, and select pins transactionally.
Failure releases earlier claims in reverse order and performs no later hardware
operation. Repeated initialization is inert. Shutdown releases select, Y, then
X and preserves the last snapshot with `Status::NotInitialized`.

`update(now)` samples each input exactly once in X, Y, select order and delegates
switch debounce to the existing explicit-time `Button`. Snapshot events remain
stable until the next update.

### Deterministic tests

`tests/test_analog_joystick.cpp` must cover:

- invalid, duplicate, unsupported, and busy pins before hardware access;
- centers outside observed bounds, overlapping dead zones, and zero spans;
- every partial-acquisition failure and reverse rollback;
- initialization, repeated initialization, shutdown, repeated shutdown,
  destruction, and claim reuse;
- exact X/Y/select sample order and one read per endpoint per update;
- raw minimum, center-minus-one, both dead-zone edges, center-plus-one, and raw
  maximum;
- asymmetric calibration, both inversion choices, clamp behavior, and 32-bit
  arithmetic boundaries;
- debounce, press/release events, bounce, stuck select, and timestamp rollover;
- raw values retained beside interpreted values;
- identical input/time traces producing byte-identical snapshots and hardware
  traces;
- non-copyable and non-movable traits.

### Circuit and observability

Reference wiring: `VRx -> A0/D54`, `VRy -> A1/D55`, `SW -> D22` with
`Pull::Up`, plus inspected 5 V and ground. Two named test points, `TP-X` and
`TP-Y`, expose the wiper voltages. An RGB LED previews X as red/green direction
and Y as brightness; its distinct blue ready pulse proves successful resource
acquisition. The learner separately measures that shutdown returns all three
input pins to `INPUT`. Serial may print raw counts but cannot replace the two
voltage measurements or RGB preview.

## Lesson 032: quadrature rotary encoder

### Dependency boundary

Requires `DigitalInput`, `ResourceRegistry`, and `Status`. The decoder owns only
phase A and B. The module's push switch remains a separate `Button`, preserving
composition and allowing encoders without switches.

### Public interface

Files: `src/quadrature_encoder.h` and `src/quadrature_encoder.cpp`.

```cpp
namespace adk {

    enum struct Rotation : int8_t
    {
        CounterClockwise = -1,
        None             =  0,
        Clockwise        =  1
    };

    struct QuadratureEncoderConfig
    {
        QuadratureEncoderConfig (PinId phaseA,
                                 PinId phaseB,
                                 Pull  pull      = Pull::Up,
                                 bool  reversed  = false) noexcept;

        PinId phaseA;
        PinId phaseB;
        Pull  pull;
        bool  reversed;
    };

    struct QuadratureEncoderSnapshot
    {
        int32_t  position;
        int8_t   delta;
        Rotation rotation;
        uint8_t  phaseMask;
        uint16_t invalidTransitions;
        bool     positionSaturated;
        Status   status;
    };

    struct QuadratureEncoder
    {
        QuadratureEncoder  (ResourceRegistry&             resources,
                            const QuadratureEncoderConfig& config) noexcept;
        ~QuadratureEncoder () noexcept;

        QuadratureEncoder            (const QuadratureEncoder&) = delete;
        QuadratureEncoder& operator= (const QuadratureEncoder&) = delete;
        QuadratureEncoder            (QuadratureEncoder&&)      = delete;
        QuadratureEncoder& operator= (QuadratureEncoder&&)      = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        Status update      () noexcept;
        void   resetPosition (int32_t position = 0) noexcept;

        bool                      initialized () const noexcept;
        QuadratureEncoderSnapshot snapshot    () const noexcept;

        const DigitalInput& phaseAInput () const noexcept;
        const DigitalInput& phaseBInput () const noexcept;
    };
}
```

The unit is one valid Gray-code edge, not a vendor-dependent “click.” Valid
clockwise phase masks are `00 -> 01 -> 11 -> 10 -> 00`; reverse traversal is
counterclockwise. `reversed` changes the reported sign. No-skips and bounce
reversals are decoded literally. A two-bit jump is invalid: position and delta
remain unchanged, `invalidTransitions` increments with saturation at 65535,
and the new phase becomes the comparison baseline so the decoder can recover.

`delta` reports only the current update: `-1`, `0`, or `1`. Position saturates
at `INT32_MIN`/`INT32_MAX`; it never wraps. Initialization reads A then B once
to establish a baseline and emits no movement. `resetPosition()` changes only
the accumulated position and clears `positionSaturated`; it does not touch
hardware or the phase baseline.

Initialization validates distinct Mega digital pins before claims, acquires A
then B transactionally, configures the selected pull, and rolls A back if B
fails. Shutdown releases B then A and retains count evidence while reporting
`NotInitialized`.

### Deterministic tests

`tests/test_quadrature_encoder.cpp` must cover:

- all four starting phases and both complete traversal directions;
- `reversed`, partial turns, direction reversal, repeated states, contact
  bounce, and recovery after every invalid two-bit jump;
- exact A-then-B read order and one read per phase per update;
- invalid-transition counter saturation;
- position at `INT32_MIN` and `INT32_MAX`, saturation, and reset;
- invalid/duplicate/busy pins, each acquisition failure, rollback, and reuse;
- all pull configurations supported by current `DigitalInput`;
- initialization baseline, idempotent lifecycle, destruction, and safe input
  modes;
- deterministic trace replay and ownership traits.

### Circuit and observability

Reference wiring: inspected `CLK/A -> D24`, `DT/B -> D25`, module switch
`SW -> D26`, and common ground. The lesson uses four LEDs as a binary position
nibble. A separate RGB LED shows green for a valid edge, amber for idle, and
red for an invalid transition. Named `TP-A` and `TP-B` allow a meter or logic
analyzer to verify the phase sequence. The display proves decoder behavior;
the test points distinguish wiring/contact behavior from software decisions.

## Lesson 033 project: calibration console

### Purpose and prerequisites

Lesson 033 is an E1, multi-component project. It requires acceptance of all
earlier lessons, specifically:

- 001–005 for RAII, LEDs, PWM/RGB, and sound;
- 007–008 for raw/calibrated analog evidence;
- 010 and 014 for seven-segment/character presentation;
- 016 for operator-input event vocabulary;
- 031 for joystick axes and select events;
- 032 for encoder edges and the separate encoder button.

It edits two bounded calibration values without storage or actuator control.
The joystick selects a field and supplies a coarse preview; the encoder applies
single-unit trim; joystick select commits; encoder-button press cancels.

### Hardware-neutral project interface

Files: `src/calibration_console.h` and `src/calibration_console.cpp`.

```cpp
namespace adk {

    enum struct CalibrationField : uint8_t
    {
        Minimum,
        Maximum
    };

    enum struct CalibrationConsoleState : uint8_t
    {
        Selecting,
        Editing,
        Committed,
        Cancelled,
        Fault
    };

    struct CalibrationConsoleConfig
    {
        CalibrationConsoleConfig (uint16_t lowerLimit,
                                  uint16_t upperLimit,
                                  uint16_t minimumSeparation,
                                  Duration acknowledgement) noexcept;

        uint16_t lowerLimit;
        uint16_t upperLimit;
        uint16_t minimumSeparation;
        Duration acknowledgement;
    };

    struct CalibrationConsoleInput
    {
        CalibrationConsoleInput () noexcept;

        int16_t  joystickX;
        int16_t  joystickY;
        bool     selectEvent;
        int8_t   encoderDelta;
        bool     cancelEvent;
        bool     inputValid;
    };

    struct CalibrationConsoleSnapshot
    {
        CalibrationConsoleState state;
        CalibrationField        field;
        uint16_t                committedMinimum;
        uint16_t                committedMaximum;
        uint16_t                previewMinimum;
        uint16_t                previewMaximum;
        bool                    changed;
        Status                  status;
    };

    struct CalibrationConsole
    {
        explicit CalibrationConsole (
            const CalibrationConsoleConfig& config) noexcept;

        Status initialize (uint16_t initialMinimum,
                           uint16_t initialMaximum) noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint                      now,
                           const CalibrationConsoleInput& input) noexcept;

        bool                       initialized () const noexcept;
        CalibrationConsoleSnapshot snapshot    () const noexcept;
    };
}
```

The engine is hardware-neutral and owns no endpoints. The example is the
adapter: it observes `AnalogJoystick`, `QuadratureEncoder`, and cancel `Button`;
decides through `CalibrationConsole`; then actuates LCD, RGB, four LEDs, and
optional piezo. Production and host tests use the same engine.

In `Selecting`, X less than `-500` chooses `Minimum` and greater than `500`
chooses `Maximum`; the center band preserves the field. A select event copies
committed values into preview and enters `Editing`. In `Editing`, moving Y
outside `-500..500` maps it to a coarse candidate across the configured legal
range; returning Y to the center band preserves that candidate. The encoder
delta then trims the current candidate by one. Minimum may never exceed
`maximum - minimumSeparation`; maximum may never fall below
`minimum + minimumSeparation`. Select commits atomically. Cancel restores both
preview values. Commit/cancel acknowledgement lasts the configured duration,
then returns to `Selecting`.

`inputValid == false`, backward non-wrapping time, or impossible configuration
enters `Fault`; no value commits from fault. Shutdown clears transient events
but preserves the last committed pair as inspectable evidence.

### Project tests

`tests/test_calibration_console.cpp` must cover:

- every state and transition, including acknowledgement boundaries and
  timestamp rollover;
- both field selections, center-band stability, coarse endpoints, every clamp,
  and exact encoder trim;
- atomic commit, cancel rollback, repeated select/cancel events, simultaneous
  commit/cancel precedence (`cancel` wins), and unchanged commits;
- invalid starting ranges, minimum separation, and configuration overflow;
- invalid input from either hardware component, recovery only through explicit
  reinitialization, and shutdown from every state;
- identical seed-free input/time recordings producing byte-identical snapshots;
- adapter-level host replay that drives fake joystick/encoder traces and
  verifies LCD/RGB/LED presentation beside engine snapshots.

### Narrative example and circuit evidence

`examples/Lesson033CalibrationConsole/Lesson033CalibrationConsole.ino` follows:

```text
setup: acquire inputs -> acquire indicators -> initialize console -> show ready
loop:  observe controls -> decide calibration -> present preview
```

High-level helpers are `observeControls()`, `decideCalibration()`, and
`presentPreview()`. Mechanism helpers follow these in the file. The vocabulary
matches the HTML and PDF.

The LCD shows field, committed value, and preview value. RGB communicates state:
blue selecting, amber editing, green committed, violet cancelled, red fault.
Four LEDs show the high nibble of the preview so correctness remains observable
if the LCD adapter fails. `TP-X`, `TP-Y`, `TP-A`, and `TP-B` preserve the input
evidence chain. A short piezo cue is supplemental; Serial is optional.

The project acceptance record proves these separately:

1. every owner acquired its pins;
2. input test points match the raw snapshot;
3. interpreted joystick/encoder state matches LEDs;
4. preview and committed values differ until explicit commit;
5. shutdown returns inputs to high impedance and outputs to documented inactive
   states.

## Delivery and commit order

Each boundary is independently buildable:

1. `input: add calibrated analog joystick`
   - interface, implementation, fake-driven tests, umbrella header, host target;
2. `lessons: teach two-axis joystick`
   - example 031, HTML, TeX/PDF, pencil diagram, CLI targets, size evidence;
3. `input: add quadrature encoder`
   - interface, implementation, fake-driven tests, umbrella header, host target;
4. `lessons: teach quadrature input`
   - example 032, HTML, TeX/PDF, timing drawing, CLI targets, size evidence;
5. `project: add calibration console`
   - hardware-neutral engine and exhaustive tests;
6. `lessons: build calibration console`
   - narrative example 033, adapter replay, HTML, TeX/PDF, diagrams, size;
7. `docs: publish input expansion block`
   - curriculum/status/component tables, kit-coverage links, roadmap, site nav;
8. `release: verify input expansion`
   - archive consumer, strict lint, Mega compilation, PDFs, site, size budgets,
     hardware-deferred ledger.

Every boundary runs `make style`, the focused host target, ordinary and
exception-enabled host gates, header-alone compilation, and `git diff --check`.
Lesson boundaries additionally run their named Mega compile, PDF, site, and
size targets. The final gate runs `make quality`, installed-archive smoke tests,
and deterministic PDF checks.

## Explicit deferrals

- No guessed KY-023/KY-040 compatibility claim.
- No interrupt-driven decoder until polling evidence establishes a measured
  need and an interrupt resource interface exists.
- No EEPROM persistence in 033; storage ownership remains a separate lesson.
- No ADC reference changes or hidden filtering in `AnalogJoystick`.
- No acceleration curve for the encoder; edge count remains the stable unit.
- No motor, servo, relay, access-control, or safety-control output.
- No physical-verification claim without a completed Mega 2560 bench card.
