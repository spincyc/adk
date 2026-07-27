# Lesson 016 delivery — 4×3 matrix keypad

Status: canonical delivery specification. The 4×3 adapter and deterministic
interpreter exist experimentally; the lesson remains **host verified; hardware
acceptance open** until every gate below is recorded. This specification
supersedes any lesson-016 proposal for a four-column or 4×4 circuit.

## Learning boundary

Lesson 016 teaches one complete operator-input path:

```text
4×3 membrane matrix
    -> MatrixKeypad electrical scan
        -> KeypadSample
            -> Keypad debounce and release gating
                -> KeypadSnapshot
                    -> visible operator evidence
```

`MatrixKeypad` owns the seven Mega pins and composes `Keypad`. `Keypad` owns no
hardware: it interprets an explicit timestamp and 12-bit sample. The adapter
does not duplicate debounce, key mapping, chord policy, or event state.

This is an E1 lesson. It uses only the USB-powered Mega 2560, a passive 4×3
membrane keypad, and the resistor already fitted to the board's D13 LED. It is
operator-input instruction, not an access-control or security interface.

## Canonical public composition

The lesson uses the existing declarations in `keypad.h` and
`matrix_keypad.h`:

```cpp
const adk::KeypadConfig debounce (adk::Duration (20));

const adk::MatrixKeypadPins keypadPins =
{
    22, 23, 24, 25,
    26, 27, 28
};

adk::Runtime       runtime;
adk::MatrixKeypad  keypad (runtime.resources (), keypadPins, debounce);
adk::MonoLed       evidence (runtime.resources (), LED_BUILTIN);
```

The learner-facing contract is:

| Operation | Meaning |
|---|---|
| `initialize()` | Validate seven unique Mega pins, initialize the interpreter, acquire all seven claims transactionally, then configure the scan circuit |
| `update(now)` | Perform one bounded four-row scan and advance the interpreter with caller-supplied time |
| `snapshot()` | Return the stable key, state, status, raw mask, and press/release events for this update |
| `shutdown()` | Return all seven keypad pins to input/high-impedance state, release every claim, and reset the interpreter |
| destructor | Call `shutdown()` without throwing |

Construction is inert. Repeated initialization succeeds without reconfiguring
pins. Repeated shutdown has no hardware effect. A failed pin validation, claim,
or interpreter initialization produces no scan and retains no claim.

`KeypadSnapshot` distinguishes:

- `Released`: no stable key;
- `Pressed`: exactly one stable mapped key;
- `InvalidChord`: two or more stable positions;
- `Fault`: the supplied scan is explicitly invalid;
- `pressEvent`: one accepted press after a complete released state;
- `releaseEvent`: return to the released state after a press or rejected chord.

An invalid chord disarms presses until every key is released. A chord that
collapses to one key does not become a new digit. Events are observations, not
callbacks, and remain true only for the snapshot following the update that
accepted the transition.

## Exact circuit

Use a membrane keypad whose seven-tail pinout has been positively identified
from its manufacturer documentation or with an unpowered continuity check.
Printed connector order is not assumed to match row/column order.

| Logical signal | Mega pin | Direction during scan | Keypad connection |
|---|---:|---|---|
| `Row0` / TP-R0 | D22 | Output low only while row 0 is sampled; input otherwise | Row containing 1, 2, 3 |
| `Row1` / TP-R1 | D23 | Output low only while row 1 is sampled; input otherwise | Row containing 4, 5, 6 |
| `Row2` / TP-R2 | D24 | Output low only while row 2 is sampled; input otherwise | Row containing 7, 8, 9 |
| `Row3` / TP-R3 | D25 | Output low only while row 3 is sampled; input otherwise | Row containing `*`, 0, `#` |
| `Column0` | D26 | Input with internal pull-up | Column containing 1, 4, 7, `*` |
| `Column1` | D27 | Input with internal pull-up | Column containing 2, 5, 8, 0 |
| `Column2` | D28 | Input with internal pull-up | Column containing 3, 6, 9, `#` |
| `OperatorEvidence` | D13 | Board LED output | No external connection |

The authoritative schematic shows four normally open switch rows crossing
three pulled-up columns. One closed key connects its selected column to the
single row driven low. Inactive rows are inputs, not driven high. No keypad
tail conductor connects to 5 V or GND directly.

The PDF pencil drawing may show the keypad above the Mega and the seven-wire
tail fanning toward D22–D28. It must label orientation as illustrative and
place this pin table beside it; the drawing never substitutes for the
schematic or continuity check.

The lesson does not add isolation diodes. Some multi-key combinations in a
diode-less matrix can be electrically ambiguous or create phantom positions.
Every multi-bit result is therefore rejected as `InvalidChord`; lesson 016
does not claim arbitrary simultaneous-key recognition or anti-ghosting.

## Narrative Mega example

The canonical sketch is
`examples/Lesson016MatrixKeypad/Lesson016MatrixKeypad.ino`. Its high-level
flow appears before mechanics and uses the same verbs as the lesson:

```cpp
void setup ()
{
    running = initializeOperatorPanel ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    const adk::KeypadSnapshot observation = observeKeypad       (now);
    const OperatorDecision    decision    = decideOperatorEvent (observation);

    if (!showOperatorEvidence (now, decision))
    {
        stopSafely ();
    }
}
```

`initializeOperatorPanel()` acquires the keypad first and evidence LED second,
then shows a bounded ready pulse. If either acquisition fails, it shuts down
already acquired objects in reverse order. `observeKeypad()` calls
`keypad.update(now)` exactly once and returns one snapshot.
`decideOperatorEvent()` converts snapshots to `Ready`, `AcceptedKey`,
`InvalidChord`, or `ScanFault`; it does not manipulate pins.
`showOperatorEvidence()` advances a nonblocking D13 pattern. `stopSafely()`
shuts down the LED and keypad in reverse order and stops further scanning.

No `delay()`, heap allocation, callback, hidden clock, credential buffer, or
Serial dependency is permitted. Serial may optionally print the same snapshot
for comparison, but removing the monitor must not change the experiment.

## Non-Serial evidence

The example provides two independent evidence paths.

### Resource acquisition and scan evidence

TP-R0 is D22, measured relative to Mega GND with a logic probe or oscilloscope.

1. **Predict:** before successful initialization, TP-R0 remains an input and no
   low scan pulse appears. After every claim succeeds, each `update()` produces
   one bounded low selection interval at TP-R0.
2. **Observe:** attach the probe with USB power removed, inspect the circuit,
   restore USB power, and observe TP-R0 while the sketch updates.
3. **Interpret:** a repeated bounded pulse proves that scanning began after
   acquisition; its absence does not alone identify which claim failed.

### Operator and fault evidence

D13 reports the interpreted result without changing keypad timing:

| State/event | D13 pattern |
|---|---|
| Ready/released | one short pulse after initialization, then off |
| Accepted digit `0`–`9` | one group containing one through ten short pulses |
| Accepted `*` | one long pulse |
| Accepted `#` | two long pulses |
| Invalid chord | repeating paired short pulses until full release |
| Scan/update fault | repeating three-short fault pattern |

Patterns are scheduled from `TimePoint`; they do not block scanning. A new
accepted event replaces only a completed ready/accepted pattern. Fault and
invalid-chord indications dominate accepted-key indications.

### Safe-state evidence

Safe state is separate from the ready light and TP-R0 activity:

1. **Predict:** after `shutdown()` all D22–D28 pins are inputs, row pulses stop,
   column pull-ups are removed, and D13 is returned through `MonoLed` shutdown.
2. **Observe:** invoke the documented shutdown exercise, verify no TP-R0 pulse,
   then inspect pin modes with the host fake; the physical bench card records
   the measured TP-R0 behavior rather than inferring it from D13.
3. **Interpret:** no row activity supports the safe-state claim. An extinguished
   D13 alone does not prove that keypad resources were released.

## Deterministic host verification

`test_keypad` proves the hardware-independent interpreter:

- all 12 positions map in row-major order to 1–9, `*`, 0, `#`;
- bounce immediately before, at, and after the debounce boundary;
- held keys emit one press only;
- a press must be followed by release before another press is armed;
- every two-key/chord case is rejected until full release;
- invalid samples become `Fault` without fabricating a key;
- repeated timestamps, backward time, wraparound, and the valid half-range;
- invalid masks and invalid debounce configuration;
- shutdown, reinitialize, and byte-equivalent replay of the same trace.

`test_matrix_keypad` proves adapter ownership and scan order:

- construction causes no pin operation;
- seven unique supported pins are required;
- busy claim at each acquisition position rolls back every earlier claim;
- interpreter failure occurs before pin claims or scan operations;
- one and every individual matrix position produces the expected mask;
- row order is 0 through 3 and column order is 0 through 2;
- exactly one row is output-low at a time and returns to input after sampling;
- columns remain `INPUT_PULLUP` only while initialized;
- no-key, single-key, and multi-key scans feed the same interpreter contract;
- injected resource failures roll back initialization without a scan;
- an explicit scan-validity seam feeds electrical/driver failure to
  `KeypadSample.valid`; raw Arduino reads cannot manufacture this evidence;
- repeated initialization, repeated shutdown, destruction while initialized,
  and subsequent reuse of all seven pins;
- scan and interpreter traces replay identically.

Header-alone compilation and copy/move trait checks are required.
`MatrixKeypad` remains non-copyable and non-movable because it owns claims.

## HTML, PDF, build, and size deliverables

The HTML reference at `site/pages/lessons/016.md` contains the concise API and
lifecycle contract, exact wiring table, status/pattern table, copyable CLI
commands, source/example/test links, errata, and the explicit open bench card.
It links the printable PDF and says how the two formats complement each other.

The printable source at `lessons/016-matrix-keypad.tex` builds
`doc/lessons/016-matrix-keypad.pdf`. The PDF is black and white and adds:

- authoritative schematic plus adjacent connection list;
- pencil-style orientation drawing with contextual alternative text;
- row-scan timing diagram;
- debounce and release-gating state diagram;
- predict–observe–interpret worksheets for TP-R0, accepted keys, invalid
  chords, scan fault, shutdown, and reset;
- a twelve-key mapping worksheet and replay trace;
- acquisition-versus-safe-state troubleshooting tree;
- blank measurements, tool versions, Mega revision, supply, reviewer, date,
  deviations, and hardware acceptance result.

Text and line style, not color, distinguish every state. Code remains selectable
text. Required metadata, embedded fonts, extraction, link, grayscale, 200%
zoom, and deterministic-build checks follow `docs/PDF_POLICY.md`; no tagged-PDF
or accessibility conformance claim is made without the recorded review.

Integration provides these CLI gates:

```sh
make host-test-keypad
make host-test-matrix-keypad
make arduino-Lesson016MatrixKeypad
make firmware-size EXAMPLE=Lesson016MatrixKeypad
make lessons
make lessons-check
make site
make site-check
make package-smoke
```

The example receives an explicit flash and static-RAM budget only after the
first measured Mega 2560 compile. The acceptance record stores the measured
bytes, percentages of board capacity, Arduino CLI version, AVR core version,
and revision. A guessed size is not a baseline.

Promotion requires `make check`, sanitizer verification, Arduino compilation,
size enforcement, lesson/PDF checks, strict site checks, library lint, and
archive installation. Physical checks remain visibly open until performed;
passing non-hardware gates yields only **host verified; hardware acceptance
open**.

Before promotion, integration must close these known experimental gaps:

- route matrix I/O through the repository platform seam, or add an equally
  narrow injectable scanner, so a driver/read failure can reach
  `KeypadSample.valid` rather than the adapter always supplying `true`;
- prove all twelve positions individually; a fake that holds one column low
  for all four row selections proves only an invalid chord;
- add the nonblocking D13 evidence presenter without placing pattern timing in
  `Keypad` or `MatrixKeypad`;
- confirm the keypad tail order from the selected kit part and cite its primary
  documentation before publishing a physical orientation drawing.

## Deferred 4×4 mapped variant

A 4×4 keypad is a later mapped adapter variant, not lesson 016 and not an
implicit widening of the 12-key API. It requires:

- an eighth claimed pin and transactional rollback across all eight claims;
- a 16-position raw mask;
- an explicit immutable 16-entry `KeypadMap` supplied by the caller;
- four additional semantic keys selected by that map rather than hard-coded
  A/B/C/D behavior;
- separate scan, mapping, chord, ghosting, size, example, and bench evidence.

The variant may reuse a generalized matrix scanner and the debounce/release
state machine only after measurement shows that doing so keeps the 4×3 header
and firmware tight. It must not change lesson 016 pin assignments, key order,
examples, PDFs, or acceptance evidence. Until that boundary lands, the
first-class supported shape is exactly four rows by three columns.
