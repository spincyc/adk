# Curriculum pacing audit

> Historical audit: the recommendations through lesson 030 remain useful.
> Its former post-030 sequence is superseded by
> [`CURRICULUM.md`](../CURRICULUM.md) and
> [`WORK_QUEUE.md`](../WORK_QUEUE.md), which are the canonical numbered
> curriculum and implementation queue.

## Purpose

This note reviews the canonical lessons 001–030 as a learning sequence rather
than as an implementation queue. It preserves every existing lesson number and
project. It does not redefine support status. Its recommendations narrow the
teaching job of overloaded lessons, move secondary material into prerequisite
references or later lessons, and append new Elegoo-kit work after lesson 030.

The cadence remains:

```text
component -> component -> project
```

Every lesson has one substantive teaching concept. A project may exercise many
known components, but its new concept is composition: one state model, one
integration risk, and one end-to-end body of evidence. Projects do not quietly
introduce hardware abstractions.

## Pacing rules

1. A component lesson introduces one public abstraction or one reusable
   behavior. A semantic wrapper may accompany its endpoint when separating
   them would leave either lesson electrically meaningless.
2. Resource claims, status handling, lifetime, deterministic time, safe
   shutdown, and circuit diagnostics are recurring methods, not five extra
   lesson subjects.
3. A project lesson introduces no new endpoint, device driver, bus, storage
   format, or calibration algorithm.
4. Each lesson ends in one observable claim. Worksheets and acceptance checks
   support that claim; a checklist alone is never the lesson.
5. A new physical variable is first studied with a controllable source, then
   with an environmental source. A new actuator is first exercised unloaded or
   with an inert indicator.
6. Optional instruments strengthen evidence but do not become undeclared
   prerequisites.
7. Lessons keep their current numbers. If narrowing creates useful overflow,
   preserve it as an extension or teach it in an appended lesson after 030.

## Audit of lessons 001–030

| Lesson | One teaching concept | Pacing decision |
|---:|---|---|
| 001 | Owning one safe digital output | Keep `DigitalOutput` and `MonoLed` together as one visible ownership story. Present status, claims, time, and lifecycle only as the minimum operating method; do not attempt to teach the whole runtime architecture here. |
| 002 | Observing a raw digital input | Keep pull-up wiring and visible bounce. Do not teach debounce or semantic button events yet. |
| 003 | Composing a deterministic reaction timer | Preserve the Reaction Timer. `Button` debounce is currently new project-level machinery; move its full contract and tests into lesson 002 as a final semantic wrapper, while lesson 003 only applies already-defined events. |
| 004 | Controlling PWM duty through an RGB LED | Keep `PwmOutput` and `RgbLed` together around one claim: bounded duty changes channel output. Timer conflicts are a diagnostic case, not a second conceptual unit. |
| 005 | Scheduling a nonblocking tone | Keep `PiezoSounder`. Teach timer leasing only through the observable coexistence/conflict of tone and PWM. |
| 006 | Composing deterministic Simon | Preserve Simon. Seeded sequence generation, cue identity, timing, buttons, light, and sound must all be established APIs before the project; the new lesson concept is replayable game-state composition. |
| 007 | Sampling a controlled analog voltage | Narrow to `AnalogInput` plus potentiometer. Mapping may be a transparent example, but calibration policy belongs in 008. |
| 008 | Stabilizing sampled data | Make calibration and filtering one pipeline whose single claim is stable, bounded output from supplied samples. Teach one filter, preferably a fixed-window mean; deadband is an extension rather than another required algorithm. |
| 009 | Composing an adaptive night light | Preserve the project. The photoresistor divider is the project's environmental substitution for the controlled source, not a new general sensor abstraction. Hysteresis must already be a small reusable behavior from 008 or be presented as the project's sole composition policy. |
| 010 | Serializing output to a seven-segment display | Keep the 74HC595 and one digit together. Bit transport and glyph presentation are two layers of the same observable serialized-output claim; avoid adding display multiplexing. |
| 011 | Advancing a nonblocking traffic state machine | Keep pedestrian requests as input events to one state model. Do not add junction wiring or shift-register instruction here. |
| 012 | Composing a fail-safe traffic junction | Preserve the project. Its new concept is enforcement of the no-conflicting-greens invariant with all-red failure behavior. |
| 013 | Validating an environmental sample | Keep the DHT11 adapter focused on the distinction among valid, invalid, stale, and transport-failed samples. Scheduling is an existing clock application. |
| 014 | Presenting stable state on a character LCD | Narrow to LCD presentation. Locale-independent record formatting can be demonstrated as the presentation boundary, but durable logging does not belong here. |
| 015 | Composing an environmental station | Preserve the project. Its new concept is freshness-aware presentation: acquisition, health, age, and display describe one stable snapshot. |
| 016 | Scanning a matrix keypad | Keep the 4×3 keypad and explicit invalid-chord behavior. Access policy and persistence remain later concerns. |
| 017 | Bounding servo position | Narrow to servo command, external-power boundary, and safe inactive behavior. Persistent configuration is not required to prove bounded motion and should become an extension or later appended lesson. |
| 018 | Composing an inert access-control trainer | Preserve the project. Use compile-time or supplied test credentials so storage is not introduced here. The new concept is deterministic lockout and operator-state composition. |
| 019 | Distinguishing valid range from no echo | Keep the ultrasonic sensor focused on timeout, range validity, and units. Filtering is reused from 008 rather than retaught. |
| 020 | Driving a motor through a safe command boundary | Narrow to direction, enable, reversal dead time, and stop dominance. Encoder measurement is a separate subsystem taught in 032 and is not required by the canonical rover. |
| 021 | Composing a tabletop bench rover | Preserve the project with scripted motion and range-triggered stop. Do not require closed-loop encoder control in the canonical acceptance test; wheels-raised and inert-indicator stages come first. |
| 022 | Owning and recording one durable transaction | This is the most overloaded lesson: `I2cBus`, `SpiBus`, RTC, and SD cannot each receive honest treatment. Keep the existing content, but define the lesson's required claim as atomic timestamped record creation through already-supplied bus adapters. Move bus-electrical instruction and driver construction to append-only lessons. |
| 023 | Enforcing mutually exclusive load intent | Keep relays inert and represented by lamps. The lesson teaches the interlock, not relay contact wiring or mains control. |
| 024 | Composing a reproducible greenhouse trainer | Preserve the project. Its new concept is deterministic decisions from schedules, samples, faults, and constrained inert loads. |
| 025 | Separating captured IR evidence from command meaning | Keep receive and decode as one evidence boundary. Teach one owned remote protocol; protocol breadth is an extension. |
| 026 | Recording passive radio observations | Keep receive-only observation, timestamps, age, and integrity. Spectrum-analyzer operation and protocol reverse engineering are outside the required lesson. |
| 027 | Composing a multi-source telemetry console | Preserve the project. Its new concept is freshness and provenance across sources sharing one scheduler. |
| 028 | Injecting channel faults | Narrow to the continuity model and explicit open, short, stale, and contradictory states. Redundant operator-state policy belongs in 029. |
| 029 | Scheduling inert cues with confirmation and audit | Keep cue scheduling as the primary concept. Confirmation, stop dominance, and the audit record are inseparable evidence for the same state transition, but UI construction is reused rather than introduced. |
| 030 | Composing the inert show-cue simulator | Preserve the capstone. Its one new concept is reviewed end-to-end safety composition and exact replay; it remains physically incapable of firing or controlling pyrotechnics. |

## Dependency corrections without renumbering

The existing main spine remains valid, but four boundaries need explicit
correction:

```text
001 DigitalOutput
  -> 002 DigitalInput + completed Button contract
      -> 003 Reaction Timer

007 AnalogInput
  -> 008 calibration/filter behavior
      -> 009 Night Light

016 Keypad
  -> 017 bounded Servo (volatile supplied configuration)
      -> 018 Access Trainer

019 Range
  -> 020 safe MotorDriver
      -> 021 Bench Rover
```

Lesson 022 should consume library-supplied bus adapters rather than pretending
to teach both I2C and SPI from first principles. This preserves the greenhouse,
telemetry, and cue dependencies without renumbering. Lesson 032 now gives
quadrature input its own instructional space; motor-mounted feedback remains a
later specialization.

## Recommended time envelope

Component lessons should fit one 75-minute session:

- 10 minutes: predict and identify the claim;
- 15 minutes: powerless build and inspection;
- 20 minutes: controlled experiment;
- 15 minutes: fault injection and diagnosis;
- 10 minutes: evidence and explanation;
- 5 minutes: safe shutdown and next dependency.

Projects should fit two 75-minute sessions. Session one assembles and proves
subsystems; session two runs deterministic scenarios, faults, and the final
acceptance record. A project that needs a third session is carrying undeclared
component instruction and should be narrowed.

## Historical append-only recommendation

This section records the pacing audit's original recommendation. It is not the
numbering authority. The input-first 031--033 block and the canonical 034--060
sequence are now recorded in the [work queue](../WORK_QUEUE.md) and
[component-project cadence](../projects/component_project_cadence.md).

The common Mega starter-kit inventory contains useful components not given
enough instructional space in 001–030: joystick, rotary encoder, stepper motor,
8×8 matrix, four-digit display, PIR, tilt and sound switches, thermistor,
water-level probe, RFID, and dedicated bus experiments. Exact kit contents
vary, so every lesson must publish substitutions and require the actual module
datasheet.

The first six appended blocks preserve the every-third-lesson project cadence:

| Lesson | Kind | One substantive concept | Observable outcome |
|---:|---|---|---|
| 031 | Component | Two-axis joystick as calibrated analog input plus a digital press | A supplied trace and physical positions map to neutral directional intent with dead zone |
| 032 | Component | Rotary encoder transition decoding | Recorded quadrature traces produce direction and bounded count without scan-order ambiguity |
| 033 | Project | Calibration console | Joystick selection, encoder trimming, commit, cancel, and visible preview replay deterministically |
| 034 | Component | Stepper phase sequencing through the rated driver module | A bounded sequence advances known steps and always de-energizes safely |
| 035 | Component | Position reference and homing with a low-energy switch | Approach, contact, backoff, timeout, and fault states are separately observable |
| 036 | Project | Tabletop pointer | Homing plus scripted step motion reaches repeatable paper targets without closed-loop claims |
| 037 | Component | Multiplexing an 8×8 LED matrix | One framebuffer becomes a stable scanned image without blocking other scheduled work |
| 038 | Component | Presenting a multi-digit numeric value | A bounded number maps to four-digit glyphs, leading-zero policy, and overflow indication |
| 039 | Project | Instrument panel | Existing sensors drive matrix trends and numeric readouts from one immutable snapshot |
| 040 | Component | PIR motion as a timed digital observation | Warm-up, active interval, retrigger, and no-motion states remain distinct |
| 041 | Component | Contact/vibration/sound modules as qualified digital events | Raw thresholds and module limitations are visible rather than mislabeled as measured physical quantities |
| 042 | Project | Room-event recorder | Timestamped motion and event inputs produce a deterministic, privacy-preserving local log and indicator |
| 043 | Component | Thermistor divider and nonlinear conversion | Named test-point voltage maps to temperature with stated model and uncertainty |
| 044 | Component | Water-level probe as a corrosion-prone qualitative sensor | Power-gated samples classify dry/wet bands while limitations and cleanup are explicit |
| 045 | Project | Plant-care observer | Temperature, light, and moisture observations produce advice and records, never automatic watering |
| 046 | Component | I2C transaction ownership | Address selection, transaction result, and bus recovery are deterministic against a fake and one kit device |
| 047 | Component | SPI transaction ownership | Mode, clock, chip-select lifetime, and restoration are observable with loopback or an owned device |
| 048 | Project | Multi-bus data logger | RTC-like time and disposable SD storage compose through independent leases and recover from interrupted records |
| 049 | Component | RFID card observation with an owned RC522 | UID bytes and read failures become local events without treating identity as authentication |
| 050 | Component | Authorization policy over supplied identifiers | Allow, deny, expiry, and duplicate presentation are pure deterministic decisions |
| 051 | Project | Inert attendance/demo kiosk | RFID events, LCD prompts, indicators, and local audit records compose without controlling a real lock |

Lessons 052–054 should be reserved for the encoder feedback deferred from 020:

| Lesson | Kind | One substantive concept | Observable outcome |
|---:|---|---|---|
| 052 | Component | Counting motor encoder edges | Synthetic and wheels-raised traces produce bounded position and velocity estimates |
| 053 | Component | Closed-loop speed intent | A fixed-step controller responds deterministically to supplied targets and measurements with saturation visible |
| 054 | Project | Constant-speed bench drive | The raised-wheel rig demonstrates target tracking, stop dominance, and replayable controller evidence |

This appended order deliberately avoids weak “tour the sensor” lessons. Tilt,
sound, and water modules are taught through the limits of what they can
actually establish. No module earns a lesson merely because it is present in
the kit.

## Content that should remain extensions

The following are useful exercises but not full lessons unless they acquire a
distinct abstraction, experiment, and downstream project:

- changing LED colors or blink rates;
- printing the same value to Serial;
- trying another keypad layout;
- adding more glyphs to a display;
- substituting a second DHT-like sensor;
- changing Simon difficulty constants;
- reading a threshold module without characterizing its threshold;
- assembling a checklist without a new observable claim.

These belong under “extend,” where they reinforce an interface without
inflating lesson count.

## Acceptance test for the sequence

Before promoting a proposed lesson, answer all of these with concrete nouns:

1. What single concept does the learner explain afterward?
2. What circuit-native observation supports that explanation?
3. Which already-taught components does it consume?
4. Which later project needs it?
5. What fault can the learner inject safely?
6. What has been deliberately excluded?

If the answers name two unrelated abstractions, split or narrow the lesson. If
the only outcome is “all boxes checked,” reject it as filler.
