# ADK recovery and review handoff

Review date: 2026-10-01. This document records the recovered work and the
changes proposed by a cold review of the library, all 55 project lessons,
all 24 electricity investigations, reference material, build tooling, website,
and PDFs. It records implemented fixes and the remaining evidence backlog.

**Status, 2026-10-01:** 36 of the 37 ranked corrections are implemented and
verified locally. R01 (print) and R03 (explanations) are complete. S03's
power-connection recipe and R02's complete purchasing routes remain pending
external specifications and validation; their unresolved requirements are now
visible before purchase. R04 still needs actual hardware builds. The recovered
patch and implementation are committed on local `main`; nothing was pushed or
published. Read the [implementation record](#implementation-record) and current
[contributor instructions](AGENTS.md) before continuing.

The architecture and course structure are worth preserving. Prioritize concrete
behavior, instruction, accessibility, and validation defects, then the unfinished
equipment and print work. Physical validation remains the largest readiness gap:
[the build record](docs/builds.md) contains no recorded hardware builds.

## Contents

- [Implementation record](#implementation-record)
- [Recovered state and verification](#recovered-state)
- [Order of work](#order-of-work)
- [Safety and equipment](#safety-and-equipment-corrections)
- [Course routes and lesson behavior](#course-routes-and-lesson-behavior)
- [C++ lifecycle and behavior](#c-lifecycle-and-behavior)
- [Technical explanations and reference](#technical-explanations-and-generated-reference)
- [Build tooling and circuits](#build-tooling-and-circuit-generation)
- [Visual presentation and accessibility](#visual-presentation-and-accessibility)
- [Unfinished recovery work and hardware](#unfinished-recovery-work-and-physical-validation)
- [Teaching and design improvements](#teaching-and-design-improvements-to-consider)
- [Implementation workflow](#implementation-and-verification-workflow)

## Implementation record

The unchecked entries below remain open. Checked entries retain the review's
original problem and acceptance criteria as historical context; this record
states the resulting behavior, evidence and limits.

| Local commit | Completed work |
| --- | --- |
| `297a882` | Preserved the complete recovered 55-file patch before further edits. |
| `625141b` | C01–C11 and D06: setup rollback, IR lifetime, fair Bridge sends, legacy E32 backpressure, FM requests, Mesh capacity, display overloads and AVR arithmetic; also L06's AVR regression. |
| `e90328e` | L02–L07: full countdown, per-field weather report identity, reliability run identity including the simultaneous old-echo/new-test boundary, shared switch state, wide throughput arithmetic and NEC address learning. |
| `f455c01` | D07 and T02–T05: API comments/declarations, compiler identity, immutable package records and complete validation dependencies. |
| `95eea32` | S01, S02, S04, D01–D05 and R03: safety and reference corrections, capacitor comparison, plus the S03/R02 equipment audit and explicit purchasing gates. |
| `19f87dd` | L01, T01, T06, V01, V02 and R01: complete builds from each circuit, net validation, explicit waypoints, accessible instructions and PDF pagination. |
| `5ecf9f6` | V01 mobile follow-up: focus the heading with the full instruction description, including when the responsive layout uses `display: contents`; regress both desktop and phone layouts. |

### Verification of the implementation

- `make -j4 test sanitize avr-test examples smoke pins style` passed with
  the newly downloaded, checksum-verified managed compiler. All 505 library
  cases passed normally and under ASan/UBSan; all 15 actual-sketch runners
  passed, including Reaction Duel's preserved 530 checks. AVR simulation
  passed 69 checks, including long-range scaling and throughput boundaries.
- `make check -j4` passed after integration and the additional Meter boundary
  correction, then passed again after the mobile focus correction. It reran
  all host and sanitizer cases, rebuilt the affected
  firmware and sketch checks, and passed the strict website, browser fixtures,
  PDF and board-package gates. The preceding code gate rebuilt every example
  with `--clean` and without library or sketch warnings.
- The first sandboxed code gate could not resolve the compiler download host.
  The successful code and full gates used approved execution outside the
  sandbox, also required by LeakSanitizer's process-tracing limitation. No
  sanitizer was disabled.
- Circuit and route checks cover all 79 circuits / 92 boards, reconstruct
  complete generated steps and compare their nets, including
  Lesson 3 → E09 → E10 → Lesson 4. Negative fixtures reject source shorts,
  incorrect module supplies and undeclared GPIO supplies. Waypoint checks
  cover fresh, inherited and cached drawings.
- Eight API tests and eight tooling tests passed, including original-failure
  controls, rejected package rewrites, accepted mirrors/new hosts, compiler
  replacement with an unchanged filename, and incremental dependency changes.
  The board package installed, both installation examples compiled, and the
  bootloader dry runs passed. Its published archive identity is unchanged.
- All 119 site pages passed links, anchors and duplicate-ID checks. Chromium
  DOM/accessibility-tree checks cover instruction focus, endpoint crop names,
  keyboard controls and independent complete-build progress on desktop and
  390-pixel layouts, including both boards in Lesson 44. No actual
  screen-reader speech or native Firefox/Safari rendering was tested.
- All 79 PDFs were rebuilt: 165 MB total, largest 4.5 MB; whole site 269 MB.
  All 3,617 checked code lines were found in the printed output, and
  all 55 project lessons start their answers on their final page. All final
  flagged pages and their successors were visually inspected.
- `git diff --check` passed. No hardware builds, RF emissions measurements,
  native Windows/macOS installs or physical equipment combinations were tested.

Build logs and reproduction evidence are under `build/handoff-fixes/`:
`coordinator/code-gates.log`, `coordinator/make-check.log`, and the per-lane
subdirectories. They are ignored build artifacts; the task status and material
limits are recorded here so losing them does not lose the handoff.

### Accepted print spacing

The audit dropped from 19 flags to four. These remain deliberately accepted:

| Lesson | Page | Occupancy | Reason |
| --- | --- | --- | --- |
| 032 Real Time Clock | 10 of 13 | 47.9% | The next page's tall clock/probe drawing cannot fit after the prediction. |
| 041 LoRa Link | 15 of 18 | 35.8% | Three tall modem/probe comparisons and their table need the next page. |
| 048 Baby Monitor | 19 of 22 | 36.7% | The two buzzer/probe comparisons need more height than remains. |
| 060 Branches in Parallel | 5 of 9 | 47.1% | The following measurement drawings cannot fit below the procedure. |

Keep their readable labels and complete probe paths. Empty SVG-wrapper
paragraph margins were removed; type and drawing scales were preserved.
Short complete sketches now stay together, and answer separation also works
when a final Go deeper paragraph follows the answers.

### Remaining evidence needed

**S03:** UNI-T's current US page permits 5 V / 2 A USB power and names a
USB-to-DC cable, but its accessory list conflicts and neither source supplies
the barrel outer/inner diameter or insertion length. Obtain those dimensions,
confirm the illustrated centre-positive polarity, and identify the exact
supplied or replacement lead rated for at least 2 A. The V25/lead/UTG932E
combination has no recorded bench test. The shopping list now includes the
unresolved lead and excludes its unknown price from the provisional subtotal.

**R02:** The independent specifications audit is done, and its sources and
qualifications are in [What to buy](docs/buy.md). Complete purchase routes still
need the S03 lead; Murata 19R107C lead reach and breadboard fit; exact current
kit/module and RTC variants; a documented CJMCU-470 board; the complete RYLR896
current maximum at 10 dBm; applicable radio configuration/authorization evidence;
legacy E32 revision availability; and a selected EU/UK solder alternative.
Unquoted accessories and regional availability remain explicit. No generic
module name, nominal supply rating or radio pacing rule substitutes for those
checks. Do not remove a gate until its specific missing evidence is obtained.

**R04:** This session cannot assemble or observe physical circuits. Leave
[the build record](docs/builds.md) unchanged until someone constructs and
records the circuits. Prioritize the unresolved supplies, mechanical fit,
instrument combination and guided transitions above.

The optional teaching/design suggestions remain considerations, not completed
requirements. Adjacent radio retry promises in Lessons 43/46 were corrected;
no broad course rewrite, legacy-worktree merge or publication was performed.

## Recovered state

The reviewed baseline was `de402e915eb1d028a0450b8855edc625e863dbf5` on `main`,
plus **55 modified tracked files, 683 added lines and 298 removed lines**.
There were no staged changes, untracked source files, or stash at recovery.
The review preserved all 55 files byte for byte.

The prior session stopped at its weekly usage limit. Its completed lesson,
electricity, and wiring changes survive in the working tree, together with
the equipment-page rewrite. In particular, preserve:

- Reaction Duel false-start handling around the debounce boundary and its
  expanded 530-check regression.
- Remote Alarm's obstacle sensor restored to its A13 home in the sketch,
  circuit, and lesson.
- Radio Pong's handling of initial and falling remote miss counts, with
  corresponding regression coverage in both player roles.
- Doorbell and Door's explanation of simultaneous-event priority.
- The equipment guidance, beginner explanations, measurement predictions,
  glossary, kit, safety, and drawing adjustments throughout the 55-file patch.

Inspect the current diff before editing these files. The recovery record is a
snapshot, not an instruction to overwrite later work. Review and integrate the
recovered changes as coherent commits when implementation is authorized.

Two older worktrees were left intact. The clean September worktree at `44d7288`
has a patch-equivalent commit already on `main`; it needs no merge. The July
legacy-course worktree at `742bcb1` has 82 staged file changes from an older
layout. Its diff was preserved separately. Do not merge or remove it as part of
the current fixes without reconciling that separate work.

The old session contained a request to publish. That historical text is recovery
evidence, not current authorization to push or publish. The current implementation
request authorizes local fixes and integration; publication remains separate.

### Local evidence

The detailed review is at
[`build/recovery-review-20261001/index.html`](build/recovery-review-20261001/index.html).
Everything in that directory is ignored build output and may disappear during
cleanup; the implementation requirements are therefore also recorded below.

| Artifact under `build/recovery-review-20261001/` | Contents |
| --- | --- |
| `recovered.patch` | Complete binary-capable diff of the recovered 55 files |
| `recovered-status.txt`, `recovery-manifest.json` | Initial status, baseline, and SHA256 hashes |
| `recovery.txt`, `historical-worktree.patch` | Recovery account and separate legacy-worktree snapshot |
| `coordinator.txt` | Guided-route and introductory-claim findings |
| `cpp-core/`, `cpp-protocols/` | Reports, reproducer sources, diagnostics, host and AVR evidence |
| `lessons-01-28/`, `lessons-29-55/` | Reports and actual-sketch reproductions |
| `electricity/`, `reference-docs/` | Reports, calculations, and selected manufacturer documents |
| `tooling/` | Circuit, packaging, dependency, and routing reproductions |
| `visuals/` | Report, browser screenshots, accessibility tree, and PDF samples |
| `make-check-unsandboxed.log` | Successful full gate; earlier sandbox failure is in `make-check.log` |

### Verification already completed

- `git diff --check` passed. All 55 hashes matched the recovery manifest, and
  the working-tree diff exactly matched `recovered.patch` at review completion.
- `make check` returned zero outside the earlier sandbox. The first attempt
  stopped at LeakSanitizer's process-tracing limitation; no sanitizer was
  disabled to obtain the successful result.
- All 474 library cases passed normally and under ASan/UBSan. All 13 lesson
  behavior runners passed, including Reaction Duel's 530 checks.
- The ordinary incremental gate satisfied example compilation, pin checks,
  sketch smoke runs, AVR simulation, style, size, website, and print targets.
  This used existing caches and was not a clean rebuild of every binary.
- The strict site build checked links, anchors, and IDs across 119 pages.
  All 79 lesson PDFs were freshly rebuilt. The PDF audit still reported
  19 sparse pages; see R01.
- The board package installed, both installation samples compiled, and the
  bootloader dry-run checks passed.
- After discovering T05, `tests/circuits.py`, `tests/build_steps.py`, and
  `tests/navigation_ids.py` were explicitly rerun successfully without trusting
  their Make stamps.
- An inventory of all 92 boards found no net joining distinct GND, 5 V, and
  3.3 V sources. This is narrower than a complete electrical safety check.
- Visual review sampled 12 site pages at desktop and phone sizes and 10 PDF
  pages across four lessons. No major clipping or overlap was found in that
  sample. V01 and V02 are supported by DOM/accessibility-tree evidence.

No physical circuits, RF emissions, native Windows/macOS installs, physical
phones, Firefox/Safari rendering, or screen-reader speech were tested. Existing
passing tests do not cover the failures below. Do not add a hardware-verification
entry on the strength of software checks.

## Order of work

Priorities below retain the review's meaning: **P2** is a concrete correction
to schedule; **P3** is a narrower defect or documentation gap. No P0/P1 finding
was established. Teaching and design judgments are listed separately.

1. Correct safety and equipment instructions, repair guided-route continuity,
   and resolve setup/interrupt lifetime defects.
2. Repair actual lesson and protocol behavior, API bounds, and AVR arithmetic.
3. Repair generated documentation, validation dependencies, and package
   immutability; improve Build along accessibility.
4. Finish equipment verification and PDF pagination, then consider the
   optional teaching and maintainability improvements.
5. Validate the resulting course on hardware and record actual evidence.

Each task below has a stable ID. Mark it complete only after its acceptance
check, and record the implementing commit and any remaining limitation here.
Source line numbers refer to the reviewed working tree and will drift.

## Safety and equipment corrections

### S01 Remove the powered fan flick instruction

- [x] **P2.** Source: [Fan lesson](docs/lessons/020-fan/index.md), line 178;
  related instructions at 101–102 and 222–225.
- **Problem:** troubleshooting tells the learner to flick a blade while the
  motor is powered, contradicting the instruction to keep fingers clear.
- **Change:** retain increasing the knob setting as the first action. Require
  power off before inspecting an obstruction or removing the blade, including
  the blade-removal step before measurement.
- **Accept when:** every troubleshooting and measurement path agrees on the
  power-off condition. No powered-blade experiment is needed to verify this edit.

### S02 Correct the explanation of floating scope grounds

- [x] **P2.** Source: [Instrument skills](docs/electricity/skills.md), 63–74.
- **Problem:** battery operation is said to prevent a slipped ground clip from
  shorting the supply. A second clip touching 5 V still shorts through an
  internally common ground when the first clip is on GND.
- **Change:** explain that batteries remove the mains-earth return path, while
  common clips can still connect circuit nodes. Keep the GND-only clip rule.
- **Accept when:** the explanation is correct for two-channel scopes and combined
  scope/generators with a connected common lead. Check the written topology;
  do not demonstrate the short on hardware.

### S03 Complete the generator power connection recipe

- [ ] **P2.** Source: [What to buy](docs/buy.md), 203–231, especially 223–227.
- **Problem:** the only named candidate, UNI-T UTG932E, specifies 5 V / 2 A,
  but the proposed power-bank lead has no identified connector dimensions,
  polarity, or part. The page acknowledges that the maker has not confirmed
  this alternative supply. The scope shopping route is incomplete for a novice.
- **Change:** verify one complete generator, bank, and lead combination; specify
  connector dimensions, polarity, how to check them, and the lead in the list
  and price estimate. Separate manufacturer specifications from bench results.
  Until complete, state the unresolved dependency at the shopping gate.
- **Accept when:** a learner can identify and connect every required item from
  sourced specifications, and any claimed bench verification is recorded.
  Matching 5 V / 2 A alone is insufficient. This is not evidence that the
  generator is incompatible with every power bank. R02 covers the broader audit.
- **Evidence:** [UNI-T product page](https://instruments.uni-trend.com/products/waveform-generators/UTG900E)
  and its [manual, page 8](https://unitrend.oss-cn-hongkong.aliyuncs.com/uploads/attach/20250624/10092357fe28aabb6c2d6cc206583.pdf).

### S04 Correct the unsupported radio compliance recipe

- [x] **P2.** Sources: [Safety](docs/safety.md), 123;
  [Kit](docs/kit.md), 88;
  [Radio Messages](docs/lessons/038-radio-messages/index.md), 91–98.
- **Problem:** aerial removal and desk distance are presented as a license-free
  operating recipe without evidence of the required emissions limits. The
  implemented pacing rule does not establish emissions compliance.
- **Change:** state the limit of that advice. Identify a module and operating
  configuration supported by applicable authorization or measurements, or
  explicitly describe the missing verification. Recheck current primary rules
  before revising country-specific claims.
- **Accept when:** every assurance has evidence for the actual configuration;
  timing, emissions, and any authorization requirements are distinguished.
  No actual violation was demonstrated by this review.
- **Evidence:** [Canada RSS-210, A.1.5](https://ised-isde.canada.ca/site/spectrum-management-telecommunications/en/devices-and-equipment/radio-equipment-standards/radio-standards-specifications-rss/rss-210-licence-exempt-radio-apparatus-category-i-equipment)
  and [US 2024 CFR 15.231](https://www.govinfo.gov/content/pkg/CFR-2024-title47-vol1/pdf/CFR-2024-title47-vol1-sec15-231.pdf).
  The US source retrieved was the 2024 edition; current eCFR was inaccessible.
  The review was not a complete 2026 legal audit.

## Course routes and lesson behavior

### L01 Make guided route returns buildable

- [x] **P2.** Sources: [Guided route](docs/guided.md), 21–27;
  [generation hooks](docs/_theme/hooks.py), 442–449;
  [Mood Lamp](docs/lessons/004-mood-lamp/index.md), 96–98;
  [E10 circuit](docs/lessons/065-control-with-a-transistor/circuit.py), 13–16.
- **Problem:** after Lesson 3, the E09/E10 detour clears and changes the board.
  E10's button feeds 5 V and the transistor base. Returning to Lesson 4 says
  to keep the button on pin 22 and its two wires, then adds only six RGB steps.
  Nothing restores the button's missing pin-22 and ground connections.
- **Change:** generate a complete-build option from the same `circuit.py` for
  route entries, or model the actual predecessor for every route transition.
  Audit every guided return and optional entry from an empty board. Preserve
  component homes and the common power/ground entry arrangement.
- **Accept when:** applying generated steps to each actual predecessor yields
  the target nets; Lesson 3 → E09 → E10 → Lesson 4 is a regression case.
  Correct page links alone do not prove circuit continuity.

### L02 Keep the full Room Alarm exit countdown

- [x] **P2.** Source: [Room Alarm sketch](examples/lessons/024-room-alarm/024-room-alarm.ino),
  36–41, 108, 122–128; [lesson](docs/lessons/024-room-alarm/index.md), 145–148.
- **Problem:** POWER arriving on an existing `Every` tick enters `Leaving`
  with 10, restarts the timer, then consumes that same update's old tick and
  displays 9. The actual-sketch NEC reproduction confirms this collision.
- **Change:** skip countdown processing on the pass that enters the state, or
  dispatch behavior from the state at the start of the pass. Preserve the
  library contract that restart does not erase an already emitted event.
- **Accept when:** actual-sketch tests cover POWER both on and between ticks;
  the display starts at 10 and the first decrement is a full second later.

### L03 Give weather readings honest completeness and freshness

- [x] **P2.** Sources:
  [Indoors](examples/lessons/046-remote-weather/Indoors/Indoors.ino), 40, 59, 110;
  [Garden](examples/lessons/046-remote-weather/Garden/Garden.ino), 38;
  [lesson](docs/lessons/046-remote-weather/index.md), 243.
- **Problem:** the report counter and measurements occupy separate packets.
  Receiving `report=1 air=215 humid=45 probe=211 ntc=214` without `light=64`
  displays missing light as valid zero. A later report counter can put a new
  timestamp beside an old light value.
- **Change:** use a complete snapshot with identity, track presence and freshness
  per field, or fit the whole report into one packet. Presence alone does not
  repair a newer timestamp attached to an older measurement.
- **Accept when:** dropping the second packet at startup and on a later report
  produces an honest missing/stale display. Keep existing silence and sensor
  failure tests passing, and explain the chosen model in the lesson.

### L04 Identify each reliability test on Echo

- [x] **P2.** Sources:
  [Echo](examples/lessons/055-reliability-meter/Echo/Echo.ino), 38;
  [Meter](examples/lessons/055-reliability-meter/Meter/Meter.ino);
  [lesson](docs/lessons/055-reliability-meter/index.md), 227, 407.
- **Problem:** Echo clears only when a number decreases. If test A delivers
  only 1 and 2, and test B first delivers 17, dots 1 and 2 remain from A.
  The claimed forward/return loss diagnosis is then false.
- **Change:** carry a run identity in every numbered packet, or use an
  acknowledged reset handshake. A single reset packet or clearing on message 1
  is insufficient under the loss this project measures. Update both boards,
  minimum payload length, and on-air timing explanations together.
- **Accept when:** lost early packets, equal boundary numbers, and descending
  boundary numbers all leave only the current run's successes on Echo.

### L05 Teach shared state correctly in the Bridge switch exercise

- [x] **P2.** Source: [The Bridge](docs/lessons/043-the-bridge/index.md), 344.
- **Problem:** toggling on `changed("presses")` toggles on the first received
  zero, and toggles only once for a coalesced change from zero to two.
- **Change:** preferably toggle a state on the sender and set the receiver's
  LED from that shared state. If retaining a count exercise, explicitly teach
  initial baseline, difference parity, and reset/restart handling.
- **Accept when:** initial zero, two coalesced presses, lost intermediate packets,
  and reconnect/restart behave as explained. Keep the exercise short and traceable.

### L06 Widen the throughput calculation before multiplying

- [x] **P2.** Source:
  [Reliability Meter](docs/lessons/055-reliability-meter/index.md), 373.
- **Problem:** `length * 2000 / back` overflows the Mega's 16-bit `int` at the
  default length of 20. The AVR reproduction gave 42,949,417 instead of 400
  for `back=100`; signed overflow itself has no defined C++ result.
- **Change:** use `length * 2000UL / back`, require a successful nonzero
  round-trip time, and briefly explain why the literal is wide.
- **Accept when:** the default and supported boundary values give the intended
  result on AVR, with division by zero excluded.

### L07 Expose the address needed by the universal remote exercise

- [x] **P3.** Sources: [Remote Lamp](docs/lessons/053-remote-lamp/index.md), 293;
  [Receiver](examples/lessons/053-remote-lamp/Receiver/Receiver.ino), 68.
- **Problem:** the exercise needs a TV's address and command, but the supplied
  screen shows only its command, event count, and relay/link state.
- **Change:** show the address on a learning screen, or explicitly make printing
  `receiver.address()` the first exercise step. Retain the NEC-only scope.
- **Accept when:** the written procedure supplies both values, including when
  two remotes have the same command but different addresses.

## C++ lifecycle and behavior

### C01 Stop setup at the first fault and undo activated devices safely

- [x] **P2.** Sources: [Object](src/adk/object.cpp), 52–57;
  [Board](src/adk/board.cpp), 305–313; [FM](src/adk/fm_radio.cpp), 154–162;
  [RFID](src/adk/rfid.cpp), 105; [reference](docs/library/index.md), 52–55.
- **Problem:** `Led{26}`, conflicting `Led{26}`, then `FmRadio` makes `start()`
  fail, yet the fake Si4703 is enabled, unmuted, and at volume 8. Releasing
  GPIO as `halt()` does leaves chip registers active. RFID likewise needs a
  bus write to disable its field. No dangerous motor activation was established.
- **Change:** abort setup traversal on the first fault and safely roll back
  successfully initialized devices before releasing pins, or separate claims,
  configuration, and activation. Do not call `stop()` blindly on failed or
  uninitialized objects. Align the documented guarantee with the chosen design.
- **Accept when:** regressions prove no setup runs after a known fault and
  earlier activated devices are cleaned up safely. Cover failure before and
  after bus-device initialization, including invalid/unclaimed pins.

### C02 Detach the IR interrupt when its receiver dies

- [x] **P2.** Sources: [IR implementation](src/adk/ir_receiver.cpp), 53, 97–98,
  144; [header](src/adk/ir_receiver.h).
- **Problem:** destroying a scoped `IrReceiver` unlinks ordinary updates but
  leaves its interrupt callback pointing into destroyed storage. A subsequent
  pin edge produces ASan stack-use-after-scope. Course globals hide this case.
- **Change:** add destruction cleanup that detaches and clears only the slot
  still owned by this receiver, protecting interrupt state. Follow the existing
  radio receiver/transmitter ownership pattern.
- **Accept when:** an edge after destruction is harmless, replacement ownership
  is preserved, and the sanitizer regression passes without resetting away
  the dangling callback before exercising it.

### C03 Make Bridge packet scheduling fair

- [x] **P2.** Source: [Bridge](src/adk/bridge.cpp), 425–445.
- **Problem:** scanning from index zero for every packet lets continuously
  changing early values starve later ones indefinitely. Eight seven-letter
  names with large 32-bit values fit only two entries per 56-character packet;
  after 99 messages the eighth still reads zero on a perfect simulated link.
- **Change:** retain a round-robin cursor or equivalent bounded fairness across
  packets, advancing after successful sends while preserving retry behavior.
- **Accept when:** all eight values progress under continuous updates whose
  aggregate exceeds one packet. Include rejected sends and periodic refresh;
  separate tests of static splitting and one changing value are insufficient.

### C04 Respect LoRa module busy state without blocking

- [x] **P2.** Source: [LoraLink](src/adk/lora_link.cpp), 79–88.
- **Problem:** with AUX LOW after setup, `send("should wait")` returns true
  and writes 12 serial bytes. Bridge then clears its pending value although
  the module is busy. This proves missing backpressure, not physical packet loss.
- **Change:** refuse the send without UART output when busy, and honor the
  supported E32 revision's AUX transition timing so consecutive sends cannot
  race its assertion. Keep setup's blocking `ready()` wait out of loop traffic.
- **Accept when:** AUX LOW/HIGH, immediate consecutive calls, and later retry
  obey the `Link` contract. Verify timing against the exact supported hardware
  manual before encoding it.
- **Evidence:** [E32-433T20D specification](https://www.cdebyte.com/products/E32-433T20D/1)
  and [older E32 family manual](https://www.cdebyte.com/Uploadfiles/Files/2022-1-12/20221121117404622.pdf).
  The older manual is for E32-433T20DT; the newer download uses an AT protocol.
  Neither should silently replace ADK's legacy six-byte configuration contract.

### C05 Honor a newer tuning request during FM seek

- [x] **P2.** Source: [FM](src/adk/fm_radio.cpp), 319–321, 372–385;
  Lesson 037 already permits dial movement during a seek.
- **Problem:** tune to 95.0 MHz, start seeking, then request 98.0. Seek completion
  overwrites the newer target with its found station, 101.1 in the fake.
  The added regression fails while the 12 existing FM cases pass.
- **Change:** distinguish the active seek result from a newer tune/step request;
  honor the newer request after the handshake or safely cancel the seek.
- **Accept when:** absolute tuning and dial stepping during seek settle at the
  newest requested target, with normal tune/seek tests still passing.

### C06 Reserve MeshNode framing space separately from payload space

- [x] **P2.** Sources: [MeshNode header](src/adk/mesh_node.h), 52;
  [implementation](src/adk/mesh_node.cpp), 88–102.
- **Problem:** the 100-character line reader includes the sender prefix, so
  `PHNE: ` plus a valid 100-character payload is reported received with only
  94 payload characters. A meaningful suffix is silently lost.
- **Change:** allow the sender prefix and separator in frame capacity, then
  bound the copied payload to its own capacity, including the no-prefix path.
- **Accept when:** exact-limit named and unnamed payloads retain their last
  character, overlength handling is explicit, and termination stays in bounds.

### C07 Let FourDigitDisplay accept mutable text buffers

- [x] **P2.** Source: [FourDigitDisplay](src/adk/four_digit_display.h), 38–51.
- **Problem:** `char text[] = "1234"; display.show(text);` selects the
  unconstrained numeric template and fails compilation. String literals pass.
- **Change:** constrain the numeric overload to intended number types, or
  provide deliberate text forwarding. Preserve the useful fractional-number
  diagnostic without capturing pointers.
- **Accept when:** mutable arrays, `char*`, const text, literals, and supported
  numbers resolve correctly; inappropriate numeric input still has a clear
  diagnostic. Coordinate the same header change with C11.

### C08 Make AnalogInput scaling safe for AVR long ranges

- [x] **P2.** Sources: [Analog implementation](src/adk/analog.cpp), 22–24;
  [contract](src/adk/analog.h), 14–15.
- **Problem:** at ADC 1023, `read(0, 3600000L)` gives -598404 on AVR instead
  of 3600000. The signed product overflows before division; subtraction can
  also overflow for endpoints of opposite signs. Current lessons use small ranges.
- **Change:** widen subtraction and product before performing them, for example
  to 64 bits for the Mega's 32-bit `long`, while preserving endpoint behavior.
- **Accept when:** AVR tests cover the one-hour example, both endpoints,
  descending ranges, and opposite-sign endpoints. Host success alone is inadequate.

### C09 Bound the Smoother shift

- [x] **P2.** Sources: [Analog implementation](src/adk/analog.cpp), 64–89;
  [header](src/adk/analog.h), 47–51.
- **Problem:** `Smoother{17}.add(65535)` yields 32767 for its first sample;
  shift 32 invokes undefined behavior. The public `uint8_t` argument is unchecked,
  including in `value()` before the first sample. Course shifts 2 and 3 are safe.
- **Change:** clamp or reject shifts outside 0–16 and document the policy.
- **Accept when:** shifts 0, 16, 17, 31, 32, and 255 behave according to that
  policy, with maximal samples, pre-sample reads, and UBSan coverage.

### C10 Avoid overflow in the longest LED blink period

- [x] **P3.** Source: [LED](src/adk/led.cpp), 75–76.
- **Problem:** `(period_ + 1) / 2` wraps at `UINT32_MAX`, making a newly
  blinking LED immediately dark. This extreme period is outside course use.
- **Change:** compute the rounded-up half without overflow, such as
  `period_ / 2 + period_ % 2`.
- **Accept when:** the maximum representable period starts lit and retains
  the intended phase behavior, alongside ordinary odd/even periods.

### C11 Check display bounds before narrowing unsigned values

- [x] **P3.** Source: [FourDigitDisplay](src/adk/four_digit_display.h), 44–46.
- **Problem:** `show(UINT64_MAX)` converts to signed -1 before bounds checking,
  displaying -1 instead of the four dashes promised outside -999 through 9999.
- **Change:** validate the original signed/unsigned value before narrowing.
- **Accept when:** signed and unsigned boundaries, very large unsigned input,
  and ordinary values obey the documented range. Keep C07's overload behavior.

## Technical explanations and generated reference

### D01 Correct Schmitt inverter threshold language

- [x] **P2.** Sources: [Digital laws](docs/laws/digital.md), 78–87;
  [law cards](docs/_theme/laws.yml), 175.
- **Problem:** the reference describes a rising input switching the output
  HIGH, then applies that language to the inverting 74HC14. E21 and the
  reference's worked example correctly give the opposite output behavior.
- **Change:** distinguish input recognition from output level. At the upper
  rising-input threshold the inverter output goes LOW; at the lower falling-input
  threshold it goes HIGH. Label the generated card's thresholds accordingly.
- **Accept when:** reference, generated card, E21 explanation, and predicted LED
  transitions agree with [TI SN74HC14 Table 7-1](https://www.ti.com/lit/ds/symlink/sn74hc14.pdf).

### D02 Stop presenting transistor gain of 100 as a guaranteed minimum

- [x] **P3.** Sources: [Transistor laws](docs/laws/diodes-transistors-op-amps.md),
  56–57; [law cards](docs/_theme/laws.yml), 123.
- **Problem:** the linked SS8050 data gives minimum DC gain of 45 at 5 mA and
  85 at 100 mA, at VCE = 1 V. A universal minimum of 100 is unsupported, and
  active-region gain does not guarantee saturation.
- **Change:** explain dependence on part, grade, and current; label any example
  gain as an assumption. Explain conservative collector/base current ratio
  where a switch design rule is needed.
- **Accept when:** the lesson and card distinguish assumptions from guaranteed
  specifications. The present course's generous base drive is not established
  to fail. Check against the [onsemi electrical table](https://www.onsemi.com/pdf/datasheet/ss8050-d.pdf).

### D03 Explain the E18 ripple without claiming most charge is lost

- [x] **P3.** Source: [Power Integrity](docs/lessons/073-power-integrity/index.md),
  234–238.
- **Problem:** the page says capacitors give up most of their charge at 100 Hz,
  although about 130 mV ripple near 5 V is only about 2.6% of charge. The feed
  stays connected. The independent nominal calculation gives 4.8746–4.9992 V.
- **Change:** explain approaching the slightly lower loaded steady voltage over
  roughly five time constants, making the full resistive supply dip visible.
  Retain the approximately 30 mV / 130 mV predictions, which the equations support.
- **Accept when:** voltage, charge, and time-constant explanations agree; reconcile
  the separate capacitor-size comparison requested in R03.

### D04 Align the introductory timing promise with actual blocking limits

- [x] **P2.** Sources: [README](README.md), 52–54;
  [homepage](docs/index.md), 69–70;
  [architecture](docs/ARCHITECTURE.md), 159–168.
- **Problem:** the introductions promise that nothing blocks and displays stay
  lit. The architecture correctly documents roughly 4 ms DHT work, up to 25 ms
  ultrasonic work, about 1.5 ms FM work, and about 70 ms IR transmission, with
  possible multiplexed-display flicker.
- **Change:** describe cooperative updates and link the bounded blocking
  exceptions. Keep the accurate detailed explanation.
- **Accept when:** a reader combining supported parts is told the relevant timing
  limits without an absolute nonblocking guarantee. No architecture rewrite is needed.

### D05 Describe the actual sensor interfaces

- [x] **P3.** Source: [Sensor reference](docs/library/sensors.md), 3–6.
- **Problem:** the introduction promises both `measured()` and `ok()` for all
  periodic sensors. Thermistor exposes neither, SoundSensor has no `ok()`, and
  Rtc has no `measured()`.
- **Change:** restrict the common pattern to devices implementing it and explain
  the analog-sensor and clock alternatives.
- **Accept when:** every introductory example names methods present in the
  corresponding public header. This requires a reference correction, not new APIs.

### D06 Separate absolute FM tuning from relative stepping

- [x] **P3.** Source: [FM header](src/adk/fm_radio.h), 48–56,
  included by [radio reference](docs/library/radio.md).
- **Problem:** a shared comment says repeating either `tune()` or `step()` changes
  nothing. Every nonzero `step()` advances again; only the absolute tune request
  has the stated property.
- **Change:** document the two methods separately. Keep `step(encoder.turned())`
  as the valid event/delta example and explain why its ordinary zero is harmless.
- **Accept when:** the generated reference makes repeated nonzero steps explicit
  and is consistent with C05's newer-request handling.

### D07 Keep every LoraSettings declaration in the generated API

- [x] **P2.** Sources: [API generator](docs/_theme/api.py), 132–148;
  [settings header](src/adk/lora_modem.h), 28–32;
  [radio reference](docs/library/radio.md), 72.
- **Problem:** trailing comments prevent declaration termination. `partner`
  absorbs `speed` into its comment, and `power`, `network`, and `band` disappear.
- **Change:** end declarations before trailing comments while preserving those
  comments as documentation. Check the broader generated API for the same case.
- **Accept when:** `api.document('src/adk/lora_modem.h', 'LoraSettings')` emits
  all five fields separately with their defaults and applicable range notes;
  a regression prevents comment/declaration merging.

## Build tooling and circuit generation

### T01 Validate power nets and actual module supply voltage

- [x] **P2.** Sources: [Bench checks](docs/_theme/bench.py), 1158–1167;
  [module definitions](docs/_theme/modules.py), 61.
- **Problem:** power-only nets bypass the relevant check. Both a 5 V-to-GND
  rail jumper and a 5 V connection to a 3.3 V LoRa module's printed `VDD` pin
  pass `Bench.finish()` and produce normal SVGs. Name-alias checks miss the
  latter's electrically equivalent connection.
- **Change:** reject incompatible sources on a net and validate a module's supply
  requirement against its connected source. Model deliberate GPIO-switched
  supplies and voltage translation explicitly so valid circuits remain valid.
- **Accept when:** direct and breadboard-mediated negative fixtures reject both
  failures, and all existing circuits pass. The inventory found no present
  distinct-source short; this finding concerns a missing guard.

### T02 Invalidate compiler installations when their bytes change

- [x] **P2.** Source: [Makefile](Makefile), 39–42, 374, 392–394;
  [toolchain manifest](boards/toolchain.json).
- **Problem:** the install path depends on archive basename, and the installed
  binary lacks a manifest prerequisite. Changing version/checksum while retaining
  the basename leaves `make -n -W boards/toolchain.json toolchain` with no work,
  and firmware can remain cached against the old compiler.
- **Change:** key a verified installation stamp by tool, version, host, and
  checksum, and make firmware depend on that identity. Avoid redownloading
  identical bytes for a URL-only mirror change.
- **Accept when:** a changed compiler identity schedules install and rebuild even
  with the same archive filename, while unchanged bytes remain reusable.
  No mismatch between today's local compiler and manifest was demonstrated.

### T03 Reject duplicate published platform versions

- [x] **P2.** Sources: [Boards generator](docs/_theme/boards.py), 113–123;
  [quality workflow](.github/workflows/quality.yml), 147–155.
- **Problem:** appending a second record for existing version `0.4.0` replaces
  its digest in a last-entry-wins dictionary. CI's removed-line check permits
  this append, bypassing the published artifact's immutability guard.
- **Change:** reject duplicate version keys and compare parsed old/new immutable
  records in CI, rather than relying only on text-line deletion checks.
- **Accept when:** an append-only duplicate replacement fails, unchanged history
  passes, and a genuinely new platform version is allowed.

### T04 Protect existing compiler host artifacts under a published identity

- [x] **P2.** Sources: [Boards generator](docs/_theme/boards.py), 49–57, 83;
  [quality workflow](.github/workflows/quality.yml), 147–155;
  [published records](boards/published.txt), 21–24.
- **Problem:** recording only `name@version` permits changed existing-host
  checksum/size under the same compiler version. Package generation accepts it.
  The synthetic invalid digest proves generation acceptance, not a download
  integrity bypass; a valid replacement archive is the consequential case.
- **Change:** compare existing host entries with the base manifest when tool name
  and version are unchanged. Protect checksum, size, and filename; permit new
  hosts and URL-only mirrors. Changed host bytes need a new version.
- **Accept when:** valid replacement bytes under an old identity are rejected,
  while new versions, additional hosts, and equivalent mirrors pass.

### T05 Make incremental checks depend on every input they read

- [x] **P2.** Source: [Makefile](Makefile), 462, 499–501;
  [circuit tests](tests/circuits.py), [build-step tests](tests/build_steps.py),
  [navigation tests](tests/navigation_ids.py).
- **Problem:** `circuits.ok` omits lesson circuit fixtures; `steps.ok` omits
  `course.yml` and circuit sources. Editing only those inputs can skip dedicated
  regressions during incremental `make check`. Clean CI runs them, and ordinary
  site checks still run; the finding is about these dedicated suites.
- **Change:** include every read input or make inexpensive runner targets phony.
- **Accept when:** after a successful build, touching each representative input
  schedules the appropriate suite. Preserve positive controls for changes to the
  test scripts themselves. Explicit reruns already passed on the reviewed tree.

### T06 Give explicit drawing waypoints a defined precedence

- [x] **P3.** Sources: [Drawing](docs/_theme/drawing.py), 294–301;
  [Simon circuit](docs/lessons/006-simon/circuit.py), 11.
- **Problem:** inherited routes are accepted by endpoint/free-space checks while
  ignoring newly specified `via` points. Lesson 6's pin-22 route misses its
  explicit waypoints. Connectivity remains correct.
- **Change:** preferably reuse a route only if it honors the current waypoints
  in order. Alternatively document conditional `via` semantics and provide a
  deliberate override. Preserve continuity where constraints are compatible.
- **Accept when:** a fresh and inherited draw both obey the documented rule, with
  a regression for changed waypoints. Inspect the affected drawings; the review
  did not establish that the current route is impossible or unreadable.

## Visual presentation and accessibility

### V01 Announce the instruction when Build along advances

- [x] **P2.** Source: [Build along script](docs/assets/steps.js), 140–182,
  302–329.
- **Problem:** Next and Done & next retain button focus while the only live
  region announces the step count. The changed action, connection endpoints,
  and care note have no announcement semantics. The user must navigate backward
  to discover what to build.
- **Change:** announce a concise instruction with endpoints and relevant care
  text, or move focus to a suitable instruction region on deliberate navigation.
  Focusing a heading such as “White wire” alone still omits essential information.
- **Accept when:** Next, Previous, Done & next, and list jumps expose the new
  instruction coherently, without duplicate speech or losing keyboard controls.
  Inspect DOM/AX semantics and run an actual screen reader if available; state
  accurately which verification was performed.

### V02 Give enlarged step crops distinct accessible names

- [x] **P3.** Source: [Build along script](docs/assets/steps.js), 37–60, 390–395.
- **Problem:** copied SVGs retain `aria-labelledby`, which overrides the later
  `aria-label`. The step crop and whole map therefore have the same full-circuit
  name instead of identifying the crop's connection.
- **Change:** provide each crop a suitable title/description, or remove inherited
  labeling before assigning a name with the relevant endpoints. Retain the
  complete map's original title.
- **Accept when:** the accessibility tree distinguishes step detail, both endpoint
  crops when present, and whole-board map. Verify after V01 to avoid conflicting
  announcements. Existing nearby text means this was a narrower gap than V01.

## Unfinished recovery work and physical validation

### R01 Finish PDF pagination refinement

- [x] **Recovered unfinished task.** No print source edits from the interrupted
  lane landed. Relevant sources are [print script](docs/assets/print.js),
  [styles](docs/assets/adk.css), [PDF builder](docs/_theme/print_pdfs.py),
  and [page audit](tests/pdf_pages.py).

Before implementation, fresh PDFs produced these 19 flags. The final four
accepted flags are recorded in [Accepted print spacing](#accepted-print-spacing). Percentages are rounded audit output;
50% and 15% entries can fall just below their respective thresholds.

| Lesson | Flagged page and occupancy |
| --- | --- |
| 003 Reaction Duel | 17/17, 12% |
| 004 Mood Lamp | 7/13, 50% |
| 007 Dimmer | 11/11, 15% |
| 024 Room Alarm | 5/18, 47% |
| 032 Real Time Clock | 10/13, 48% |
| 038 Radio Messages | 5/16, 47%; 6/16, 49% |
| 041 LoRa Link | 15/18, 36% |
| 044 Remote Dial | 5/21, 47%; 10/21, 47% |
| 046 Remote Weather | 5/25, 47%; 22/25, 39% |
| 047 Remote Alarm | 11/24, 47% |
| 048 Baby Monitor | 4/22, 49% |
| 049 Remote Keypad | 5/21, 49% |
| 055 Reliability Meter | 5/22, 47% |
| 064 One Way Diode | 6/6, 12% |
| 066 Inductor Current | 8/8, 11% |
| 069 Frequency and Period | 6/6, 9% |

Inspect the flagged pages before changing layout. Favor measured grouping and
break rules over lesson-specific blank-space patches. Retain readable type,
enlarged connection details, complete code, repeated table headings, and the
intended separation of answers. Do not remove useful material to satisfy a
heuristic. After content fixes, rebuild and visually inspect affected PDFs,
then audit all 79. Record any deliberately accepted sparse page and why.

### R02 Independently verify every equipment recommendation

- [ ] **Recovered unfinished task.** The rewrite of [What to buy](docs/buy.md)
  landed; the final independent verification did not. The cold review checked
  selected specifications and S03, not every product, price, or compatibility claim.

For each required or recommended item, map its actual lesson needs to current
manufacturer specifications: electrical range, output amplitude and offset under
the relevant load, bandwidth, supply, connector/polarity, lead type, and required
accessories. Identify exact models and distinguish specified capability from
physical verification. Reconcile shopping totals after any substitution.

Finish S03 as part of this task. Do not recommend cheap unverified equipment or
silently redesign the course around a narrower generator output range. A proposed
0–2.5 V alternative was not accepted as a replacement for the existing instrument
envelope. Any such redesign needs an explicit decision and matching lesson changes.

Acceptance is a complete, source-supported shopping route for each course path,
with unresolved dependencies clearly visible before purchase. Purchasing sources
and availability need refreshing when this task is resumed.

### R03 Reconcile the three interrupted explanation edits

- [x] **Recovered unfinished task.** The prior lane returned only a usage-limit
  failure. Check current text first because related wording already exists.

1. Make it explicit at the relevant generator-use instructions that generator
   output stays off every Mega pin, not only its analog inputs.
2. State that a combined scope/generator must reach the course's required 4 V
   output. No checked combination had been established to meet the full set
   of requirements. Coordinate with S03/R02 and source the actual capability.
3. Clarify E18's capacitor-size comparison in
   [Power Integrity](docs/lessons/073-power-integrity/index.md), alongside D03's
   correction of the charge explanation.

Check [What to buy](docs/buy.md), [skills](docs/electricity/skills.md), and the
scope investigations together. Acceptance is a consistent instruction set and
comparison that a beginner can carry out with the listed equipment.

### R04 Build and record the course on actual hardware

- [ ] **Readiness work identified by the review.** Software and mathematical
  checks cannot establish physical fit, lead reach, module orientation, signal
  integrity, sensor behavior, or the visibility of the intended observations.

Use [the existing build record](docs/builds.md). Start with recurring parts and
the equipment-dependent investigations, then cover the lessons and route
transitions systematically. Record exact module variants, wiring, measured
results, deviations, and evidence in the repository's established format without
personal information. Prioritize timing-sensitive combinations and separate
validation of the proposed generator/bank/lead combination.

Only an actually built and recorded circuit satisfies this task. Keep unbuilt
lessons explicitly unverified; neither a successful compile nor a simulator
result warrants changing their hardware status.

## Teaching and design improvements to consider

These are judgments or additional coverage opportunities. They are separate from
the 37 ranked corrections above and do not justify a broad rewrite.

| Area | Proposed improvement and completion evidence |
| --- | --- |
| Lesson 3 pacing | Lesson 2 is 36 lines; the full duel is 156 and introduces several concepts together. Keep the useful standalone beep checkpoint. Consider a short one-player timing sketch or a complete state-trace exercise before the full duel. Preserve false-start behavior and tests. |
| Predictions | E08 supplies 10/20-second answers before asking; E14 supplies cycle counts; E19/E20 put truth-table answers beside blanks. Move answers after the prediction, or distinguish a worked example from a new condition. Check that the learner has a real prediction to make. |
| Independent synthesis | Add a bounded closing challenge using existing parts: choose a divider/filter target, current budget, and settling time within a supplied safe envelope. Assess transfer without adding many more lessons. |
| Signed overflow teaching | Lesson 16's keypad extension can explain that signed overflow has no defined result and follow the failure with rejecting input before multiplication. |
| Radio retry wording | Replace promises such as recovery “within two seconds” in Lessons 043/046 and similar pages with the refresh/retry interval and its dependence on successful delivery. A later refresh packet can also be lost. |
| Const views | Consider a constrained `Span<T>` to `Span<const T>` conversion in `src/adk/containers.h`. A deduced mutable view currently cannot be passed to `Speaker::play`, although the original array can. Preserve const and derived-type restrictions if adding it. |
| API completeness | Show container template construction, document the `Axes` result, retain low-level bus namespaces, and list `blend()`/`wheel()` signatures. Verify generated examples compile and match headers. |
| Schematic maintenance | Inline SVG schematics duplicate connectivity from `circuit.py`; the inspected copies currently agree. Consider linking a small schematic model to circuit node identities before a later pin/value change causes drift. Avoid a wholesale drawing rewrite. |
| Classroom reproducibility | Consider a matched archive of lesson PDFs/pages and library version. Current version-pinning guidance candidly says the live pages continue changing. |
| Protocol test coverage | Serial fakes are permissive; consider capacity/timing coverage after C04. FM failed writes after probe and a seek that never completes also deserve targeted tests if their recovery guarantees are to be strengthened. These were not reproduced additional defects. |

One portability concern needs confirmation: [Makefile](Makefile), line 142,
expects `avr-gdb`, while [.github/workflows/toolchain.yml](.github/workflows/toolchain.yml),
103–129, builds only binutils, GCC, and avr-libc for macOS/ARM hosts. Inspect the
actual archives or test those hosts before deciding whether to supply the
simulator separately or document/configure its dependency. No native failure
on those systems was observed during this review.

## Implementation and verification workflow

Preserve the existing strengths: fixed storage, one `Object` per part, explicit
time, one-update events, short sketches, concrete predictions, and wiring generated
from a single circuit description. Follow [architecture](docs/ARCHITECTURE.md),
[style](docs/STYLE.md), and [contributing](docs/contributing.md) for implementation
details. Keep generated files, experiments, downloaded references, and caches in
`build/`.

If work is delegated, assign exclusive write ownership. Useful independent lanes
are core C++, protocol C++, lesson behavior, reference/equipment text, circuit/build
tooling, and visual/print work. Coordinate shared files explicitly: C01 touches
protocol device cleanup; C07/C11 share a header; D01/D02 share `laws.yml`;
S03/R02/R03 share buying and instrument guidance; L01/T06 share circuit generation;
V01/V02 share `steps.js`. Give each lane its own scratch directory under `build/`
and one coordinator ownership of shared builds and integration.

For each implemented correction:

1. Reproduce the stated failure against the current tree before changing it.
   Use the saved reproducer where available; its intended failure is evidence,
   not a gate to weaken.
2. Make the smallest coherent change and add a behavioral regression where a
   runtime, parser, arithmetic, or dependency defect warrants one. For prose,
   inspect the rendered result and source agreement rather than writing tests
   that merely repeat the sentence.
3. Run the focused acceptance check. For AVR-width issues, use the actual AVR
   simulator as well as applicable host tests. For generated UI/PDF changes,
   inspect the affected rendered output.
4. After integrating related fixes, run `make check`. Until T05 is fixed,
   explicitly run the three omitted-dependency suites with the Makefile's
   cache settings, for example from the repository root:

   ```sh
   export PYTHONPYCACHEPREFIX="$PWD/build/pycache"
   export ADK_DRAWINGS="$PWD/build/drawings"
   python3 tests/circuits.py
   build/venv/bin/python tests/build_steps.py
   build/venv/bin/python tests/navigation_ids.py
   ```

5. Verify `git diff --check`, review the complete diff, retain unrelated work,
   and record completed task IDs, commit, commands, outcomes, and limitations
   in this handoff. Use the repository's configured project identity for commits.

Run additional gates to answer a remaining risk, not simply to repeat successful
checks. Equipment, RF, and hardware conclusions require their own evidence.
Local integration does not imply authorization to push or publish.
