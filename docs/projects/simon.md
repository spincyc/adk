# Lesson 006: deterministic Simon

Simon is the second integration checkpoint. It combines the supported input,
output, PWM, RGB, and sound layers around a hardware-neutral game engine.

## Status

| Layer | Evidence | Status |
|---|---|---|
| `Simon`, fixed source, seeded source | Deterministic host tests and golden vectors | Host verified |
| Component library | Compiles for `arduino:avr:mega` | Mega compiled |
| Four-button, light, and sound adapter | Canonical example and exact lesson circuit | Hardware experimental |
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
| Acquisition indicator | D13 | Built-in LED; 250 ms pulse after all resources initialize |
| Four buttons | D22–D25 | Internal pull-up; switch to GND |
| Four cue LEDs | D30–D33 | One 330 Ω branch per LED |
| RGB status | D5–D7 | Common cathode; one 330 Ω resistor per die |
| Passive piezo | D11 | 100 Ω series resistor; no speaker |

The D13 acquisition indicator is not game behavior evidence. Its 250 ms pulse
is followed by a 750 ms interval in which D13 and all game outputs remain dark;
button sampling and game behavior begin only after that separator. Seven
independent external LED resistors are required. Confirm the RGB package pinout
from its datasheet. Remove every power source before changing wiring.

The adapter has an input-independent 120-second run deadline measured from
the same successful-acquisition instant that begins the D13 pulse. Thus game
behavior starts 1 second into that 120-second interval. At the deadline it
stops the tone and commands every light inactive, holds that observable
inactive state for 250 ms, then releases D5–D7, D11, D13, D22–D25, and D30–D33
to high impedance in reverse acquisition order and halts. Upload or reset is
required to restart. A near-simultaneous button press is only a physical chord
trial; it never controls shutdown and does not establish exact simultaneity,
which remains host-trace evidence.

The output adapter maps neutral cues to position and tone. RGB color is
supplementary; required state must also be available through position, pattern,
text, or distinct sound.

## Hardware acceptance

Do not mark the project hardware verified until a named person records:

1. commit, operator, independent reviewer, board and retained specimen IDs,
   primary sources, tool versions, supply/current limit, and instruments;
2. unpowered continuity, polarity, pinout, and measured resistor checks;
3. the D13 250 ms pulse and 750 ms dark separator, independently of every
   game-behavior output;
4. each raw and debounced button state;
5. each cue LED, RGB channel, and bounded tone;
6. the published fixed replay and seeded replay; build the fixed artifact with
   `--build-property compiler.cpp.extra_flags=-DADK_LESSON006_FIXED_REPLAY=1`,
   record that define, and use the canonical adapter's bounded `One`, `Three`
   source and matching published timing configuration;
7. mismatch, timeout, held input, reset, and a near-simultaneous chord trial;
8. flash, static RAM, measured rail, resistor drops, per-channel current,
   board-revision-specific D13 load, worst commanded current, and seven-die
   external fault bound; and
9. the 120-second deadline, 250 ms inactive interval, and subsequent
   high-impedance outputs with the game arranged in several active phases at
   expiry, followed by physical power removal.

Prove high impedance physically rather than inferring it from a dark LED. With
USB removed, disconnect the load branch and install measured 10 kΩ resistors
as 5 V--10 kΩ--TP--10 kΩ--GND on one named pin at a time. Calculate driven and
released midpoint ranges from measured rail, tolerance, and DMM input
resistance. Record D6 explicitly, plus every other endpoint. Remove USB before
moving the fixture.

Stop and remove USB for heat, odor, resets, painful or distorted sound,
unexpected brightness, current above the reviewed aggregate bound, a specimen
identity/pinout mismatch, or any result that disagrees with the predicted safe
state. The acceptance remains physically pending until the complete signed
card is published.

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
