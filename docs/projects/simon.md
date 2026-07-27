# Lesson 006: deterministic Simon

Simon is the second integration checkpoint. It combines the supported input,
output, PWM, RGB, and sound layers around a hardware-neutral game engine.

## Status

| Layer | Evidence | Status |
|---|---|---|
| `Simon`, fixed source, seeded source | Deterministic host tests and golden vectors | Host verified |
| Component library | Compiles for `arduino:avr:mega` | Mega compiled |
| Four-button, light, and sound adapter | Exact lesson circuit; canonical example pending | Hardware experimental |
| Physical circuit | Acceptance card not recorded | Not hardware verified |

Compilation and simulation do not replace the physical acceptance card.
Lesson 006 is energy class **E1**: low-voltage Mega GPIO, current-limited LEDs,
buttons, and one passive piezo only.

## Composition

```text
4 Button objects ---- complete SimonInput ----> Simon
                                                   |
                                            SimonSnapshot
                                                   |
                     +-----------------------------+------------------+
                     |                             |                  |
                4 MonoLed                    RgbLed/PwmOutput     PiezoSounder
```

`Simon` owns no pin, button, LED, sounder, or clock. The application:

1. obtains one `TimePoint`;
2. updates every button once;
3. constructs one complete `SimonInput`;
4. updates the engine once; and
5. maps one `SimonSnapshot` to outputs.

This makes button scan order irrelevant and gives every observer one coherent
frame.

## Current interface

`CueId` has four neutral values: `One`, `Two`, `Three`, and `Four`. Position,
color, pin, and tone belong to the hardware adapter.

`SimonInput` contains:

- `activeMask`;
- `pressedMask`;
- `releasedMask`; and
- `startEvent`.

Bits 0–3 map to the four cues. Higher bits are invalid. A press with anything
other than one active cue fails as `InvalidInput`; scan order never chooses a
winner. A correct press is incomplete until every active bit is released.

`SimonSnapshot` publishes phase, outcome, status, cue presence and values,
deadline presence and value, LED mask, sequence length, playback and player
indices, and whether input was accepted.

## Sequence-source hierarchy

`CueSource` is a deliberately bounded runtime-polymorphic seam. `Simon` holds a
non-owning pointer, so the source must outlive the engine. The hierarchy owns no
hardware and uses no allocation or RTTI.

- `FixedCueSource` supplies exact fixtures.
- `XorShift32CueSource` supplies the versioned `XorShift32V1` sequence.

The virtual seam is retained because these sources are genuinely
substitutable. Its vtable, thunk, flash, SRAM, and object costs are measured
inside the Simon size budget. New virtual hierarchies require separate review.

For `XorShift32V1`, each draw applies 32-bit xorshift operations and maps the
low two state bits to a cue. Seed zero is normalized to `0x6d2b79f5`. An
algorithm version never changes after publication; a new algorithm requires a
new enum value and golden vectors.

## State and timing

The implemented phases are:

```text
Idle
  -> PlaybackOn <-> PlaybackGap
  -> AwaitPress -> AwaitRelease
  -> RoundSuccess -> PlaybackOn
  -> GameSuccess | GameFailure
```

Time advances only through `update(TimePoint now, const SimonInput&)`.
Configuration is copied at construction and validates cue-on, gap, input
timeout, result duration, growth, and fixed-capacity bounds.

At an input deadline, timeout is evaluated before a press from the same
timestamp. Elapsed-time arithmetic is wrap-safe for valid intervals shorter
than half the 32-bit range. Backward or ambiguous movement is rejected.

## Required replay evidence

A reproducible run records:

- library revision;
- configuration values;
- algorithm version and normalized seed, or fixed source contents;
- every timestamp and complete input mask; and
- every resulting snapshot field.

Tests cover:

- all configuration boundaries;
- fixed and seeded golden vectors;
- cue-on, gap, input, and result deadlines;
- correct rounds and growth;
- mismatch, timeout, chords, and invalid bits;
- release-before-next-press;
- fixed-source exhaustion and invalid source output;
- capacity 32 and rejection above capacity;
- repeated timestamps and counter rollover;
- restart after each terminal result; and
- identical snapshots from two identical replays.

The complete test suite is the executable behavioral authority. The printable
lesson presents the principal golden trace as a prediction and evidence
worksheet.

## Hardware mapping

The lesson circuit uses:

| Function | Mega pins | Electrical rule |
|---|---|---|
| Four buttons | D22–D25 | Internal pull-up; switch to GND |
| Four cue LEDs | D30–D33 | One 330 Ω branch per LED |
| RGB status | D5–D7 | Common cathode; one 330 Ω resistor per die |
| Passive piezo | D11 | 100 Ω series resistor; no speaker |

Seven independent LED resistors are required. Confirm the RGB package pinout
from its datasheet. Remove every power source before changing wiring.

The output adapter maps neutral cues to position and tone. RGB color is
supplementary; required state must also be available through position, pattern,
text, or distinct sound.

## Hardware acceptance

Do not mark the project hardware verified until a named person records:

1. commit, board ID, tool versions, supply, and instruments;
2. unpowered continuity and resistor checks;
3. each raw and debounced button state;
4. each cue LED, RGB channel, and bounded tone;
5. the published fixed replay and seeded replay;
6. mismatch, timeout, held input, and simultaneous input;
7. flash, static RAM, and worst commanded LED current; and
8. inactive, high-impedance outputs after shutdown in several active phases.

Stop for heat, odor, resets, painful sound, unexpected brightness, or a result
that disagrees with the predicted safe state.

## Companion material

- [Lesson 006 PDF and downloads](../../lessons/006.md)
- [Current API](../../api-supported.md)
- [Determinism](../../determinism.md)
- [Safety model](../SAFETY_MODEL.md)
- [Size budgets](../SIZE_BUDGETS.md)
- [Curriculum](../CURRICULUM.md)

HTML is the concise, searchable contract and errata surface. The PDF supplies
the exact bench schematic, current worksheet, state diagram, golden replay,
fault matrix, troubleshooting table, and signed acceptance record.
