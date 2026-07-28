# Physical acceptance campaign plan

Status: software preparation audited 2026-07-27; no physical observation or
acceptance is claimed.

## Campaign boundary

Lessons 001--030 remain host verified and bench open. Execute them in numeric
order. Do not create a per-lesson physical child task until all of these are
named and available:

- operator and independent reviewer;
- exact Mega 2560 board and revision;
- exact retained specimens, markings, pinout, and primary electrical sources;
- supply, voltage, current limit, and physical power-removal method;
- measured resistors and relevant polarity, forward-voltage, and current data;
- meter and any required oscilloscope or logic analyzer, including probe
  reference and high-impedance test method;
- sketch commit, upload command, Arduino CLI, AVR core, and port; and
- lesson-specific stop conditions and a reproducible operator-reachable
  shutdown fixture.

Create children only at that boundary, one lesson at a time. Each signed card
must separately record resource acquisition or rollback, primary behavior,
non-Serial evidence, reset, controlled shutdown, high-impedance or other safe
state, physical power removal, deviations, uncertainty, reviewer, and result.
Every measurement uses prediction, actual observation with location and
timing, and interpretation. Host traces, compilation, dark LEDs, and blank
worksheets are not bench evidence.

## Preparation audit

| Lessons | Readiness | Required preparation before child creation |
|---:|---|---|
| 001--002 | Software prepared; bench resources gated | E1 declarations, finite/reachable shutdown, complete P/O/I records, and physical weak-bias methods are published. Before child creation, identify the exact board/switch, supply, meter/fixture, operator, and reviewer; calculate the board-specific D13 and D22 voltage ranges. |
| 003--004 | Software prepared; bench resources gated | D8 is the canonical Lesson 003 cue and D13 is acquisition-only; Lesson 004 likewise separates D13 acquisition from RGB behavior. Both publish finite shutdown, named test points, physical weak-bias methods, full P/O/I cards, and current/specimen gates. Identify the actual bench resources before child creation. |
| 005--006 | Software prepared; bench resources gated | Both lessons publish finite input-independent shutdown, independent acquisition and safe-state evidence, exact identity/current fields, and board-aware weak-bias methods. Lesson 006 also provides a compile-time fixed replay artifact; near-simultaneous presses remain physical chord evidence only. Identify the exact bench resources before child creation. |
| 007--008 | Software prepared; bench resources gated | Both lessons publish bounded acquisition and shutdown, exact raw/stage records, analyzer timing, one-pin-at-a-time weak-bias methods, acquisition rollback, reset, and physical power-removal evidence. Lesson 008 now gives an explicit three-terminal potentiometer and LED net layout. Identify the exact bench resources before child creation; never open ground while powered. |
| 009--010 | Software prepared; exact specimens gated | Lesson 009 publishes raw/voltage/output evidence, bounded lifecycle, exact coordinate plates, and specimen-dependent RGB/LDR gates. Lesson 010 publishes TP-DATA analyzer evidence, bounded lifecycle, exact 74HC595 wiring, current worksheets, and the OE-low startup limitation; the display remains unpowered until its marking, common type, and pinout are established. |
| 011--012 | Software and learner flow prepared; exact display gated | Lesson 011 now has the correct eight external LED/resistor paths, safe ordered presentation, actual Mega-header geometry, exact breadboard coordinates, and staged visible payoffs. Lesson 012 adds exact signal and 74HC595 placement, a specimen-gated display worksheet, and a cumulative countdown payoff. Automated verification remains behind the scenes; learner-facing acceptance cards were removed by user direction. |
| 013--014 | Software and learner flow prepared; exact specimens conditional | Lesson 013 now uses a conditional labeled DHT11-module layout, exact Mega/breadboard geometry, immediate RGB payoff, visible DATA fault, and automatic recovery. Lesson 014 conditionally admits an independently identified parallel 16-pin LCD, leaves A/K open, and provides exact placement plus staged visible states. Family listings are not represented as exact pinout evidence. |
| 015--016 | Software and learner flow prepared; exact specimens conditional | Lesson 015 conditionally composes the authorized DHT11-module and parallel-LCD families on one breadboard, including split-rail handling, staged visible payoff, fault recovery, and bounded shutdown. Lesson 016 truthfully uses the authorized 4×4 keypad in a documented 12-key mode with the fourth column insulated; it no longer claims a kit-supplied 4×3 specimen. |
| 017 | Software and learner flow prepared; exact E2 servo conditional | The staged marker-to-signal progression and bounded shutdown are published. Before any powered motion, identify the exact servo and primary limits; use a separately regulated/current-limited supply, common ground, reachable positive load disconnect, and review the signal-with-load-unpowered back-power case before connecting D44. |
| 018 | Software and learner flow prepared; exact specimens conditional | The authorized 4x4 keypad is published in documented 12-key mode with C3 insulated. The base RGB/indicator path is executable without a display; the parallel LCD stage remains optional and conditional on an independently identified 16-pin specimen. The inert trainer has a fixed input-independent shutdown and inherits no servo or real-lock claim. |
| 019 | Software and learner flow prepared; exact specimen conditional | The visible range hunt, reversible fault/recovery, literal wiring layout, and input-independent 120-second shutdown are published. Before connecting the module, identify the exact ultrasonic specimen and confirm its printed pin order and electrical limits; the family listing alone does not establish either. |
| 020 | Software and learner flow prepared; motor E2 deferred | The USB-only three-LED direction chase provides visible forward, reverse, dead-time, stop, and automatic 10-second shutdown behavior on a literal breadboard layout. No motor, driver, or external supply is connected or implied; powered motion remains deferred until exact electrical identities and the E2 boundary are qualified. |
| 021 | Software and learner flow prepared; motor E2 deferred | The USB-only six-lamp route produces three dependent visible wins and an input-independent 12-second shutdown on a literal Mega/breadboard layout. No motor, driver, external supply, or powered rover is connected or claimed; powered motion remains blocked on exact driver, motor, supply, protection, and restraint qualification. |
| 022 | Software and learner flow prepared; physical buses and specimens deferred | The potentiometer/LED model gives a visible record journey and fixed 120-second shutdown without claiming physical bus traffic. No SD specimen is authorized, storage remains model-only, and RTC work remains conditional on exact revision and electrical qualification; a separate Mega bus harness would be required before any physical bus claim. |
| 023 | Software and learner flow prepared; inert-only | Two dependent visible chases surround an exact fail-closed `HardwareFailure` scene, followed by lifecycle recovery and an input-independent 15-second shutdown with a 250-millisecond inactive hold. Literal Mega/breadboard wiring is published; no powered load or timing-instrument claim is made. |
| 024 | Software and learner flow prepared; exact display conditional | The staged LCD, dial, ordinary-LED, RGB, and inhibit story uses literal wiring and a fixed 120-second shutdown. Physical inhibition is visible at D38; stale-record behavior remains recording-model evidence only. The parallel LCD remains conditional on independently established pin numbering and electrical identity, and no physical acceptance is claimed. |
| 025 | Inventory gated | Identify the exact IR receiver and harmless known remote. Keep unknown/timing/overflow cases host-only unless a separately authorized fixture is identified; add reset, power-removal, and current fields. |
| 026 | Card tightening | Add explicit stop conditions, power-removal, actual rail/resistor/LED current, complete identity/tool/reviewer fields, and retain the no-radio claim. |
| 027 | Card tightening | Identify the RGB specimen, record current and supply limit, exclude the optional display from the E1 starter, and add stop/reset/power-removal rows. |
| 028 | Implementation blocker | Add or accurately revise the claimed circuit-native acquisition behavior; reconcile TP28/TP29 names; add current, stop, reset, and power-removal evidence. |
| 029 | Implementation blocker | Reconcile the claimed initial acquisition sweep/steady indication with the canonical sketch and add a deliberate rollback fixture before bench work. |
| 030 | Software defect | Remove the duplicate same-frame `channelButton.update(now)` that erases press events; reconcile the claimed startup sweep and figure description; add rollback evidence. Complete prior numeric cards first. |

## Staging rules

1. Repair software and publication contradictions before asking a human to
   execute the affected card.
2. Capture exact specimens unpowered using the inventory template. Retail
   family names do not establish pinout or ratings.
3. Generate and review the lesson-specific blank card. A structural card check
   verifies headings only.
4. Attach probes and continuity-check wiring with USB and external power
   removed.
5. Execute nominal, boundary, reset, shutdown, and power-removal stages in the
   published order. Stop on any named condition.
6. Commit the completed signed record without converting uncertainty into a
   pass. Only then complete that lesson's physical child and proceed
   numerically.

Lessons 001--030 therefore remain bench open. The absence of named equipment
and operator is an external prerequisite, not permission to infer results.
