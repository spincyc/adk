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
| 005--006 | Not executable | Instantiate canonical cards with independent acquisition and safe-state rows, reset and stop conditions, exact identities/current evidence, and a named D6 high-impedance method. Treat near-simultaneous button work as physical chord evidence only. |
| 007--008 | Not executable | Identify specimens and supply; require timing/high-impedance instrumentation; define access to raw ADC/stage evidence; add rollback, reset, power-removal, and reachable shutdown procedures. Never open ground while powered. |
| 009--010 | Not executable | Identify LDR/RGB/74HC595/display and electrical limits; add measured rail/raw/current totals and a controlled shutdown fixture. Lesson 010 must include TP-DATA and analyzer evidence and record the OE-low startup limitation. |
| 011--012 | Not executable | Correct Lesson 011's external LED/resistor count; define pull-fixture high-impedance and resistor-drop current methods; separate synthetic faults from bench actions; identify 74HC595/display; record startup display uncertainty. |
| 013--014 | Inventory gated | Match exact DHT11 and LCD modules to primary sources, including pull-up/backlight topology; add power/current and full identity fields. Lesson 014 also needs a reversible power-off fault, explicit reset step, and named TP14 high-impedance method. |
| 015--016 | Not executable | Resolve DHT/LCD/RGB identities and complete authoritative wiring; add measurement rows and reachable shutdown. Lesson 016 requires a separately authorized exact 4x3 keypad or a formally designed 4x4 adapter; the authorized set must not be represented as containing 4x3. |
| 017 | E2 inventory gated | Identify the exact servo and primary limits; select a separately regulated/current-limited supply and reachable positive disconnect. Review the signal-with-load-unpowered back-power case before connecting D44. |
| 018 | Not executable | Resolve the 4x3-versus-authorized-4x4 keypad boundary, LCD/RGB wiring and current budget, and provide an operator-reachable shutdown fixture. Keep it E1 and exclude servo inheritance. |
| 019 | Inventory gated | Identify the exact ultrasonic module; define controlled target geometries, per-row fault/reset/shutdown evidence, LED current, final disposition, and a nominal shutdown/resource-reuse harness. |
| 020 | Not executable | Replace the nonexistent `make evidence` instruction; provide a controllable fixture that stops during dead time and reaches shutdown; add per-test-point records and measured current. Keep motor/driver E2 deferred. |
| 021 | Not executable | Add an accepted operator stop/stimulus harness for the synthetic E1 sketch and complete its card. Keep motor E2 blocked on exact driver/motor/supply/protection/restraint qualification. |
| 022 | Not executable | Add a physical Mega bus acceptance harness because the canonical sketch uses recording drivers. Identify the exact RTC revision. Removable storage is outside the authorized union and remains host-only absent separate authorization. |
| 023 | Not executable | Define independent acquisition, a weak-pull high-impedance method, LED current budget, and restart procedure. Use analyzer timing or add a deliberate phase; naked-eye LEDs cannot prove request-before-application ordering. |
| 024 | Not executable | Freeze exact LCD/RGB wiring and current budget; split physical inhibition from host-only stale evidence; provide stable acquisition timing, a shutdown trigger, TP38 pull-fixture proof, reviewer, and result. |
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
