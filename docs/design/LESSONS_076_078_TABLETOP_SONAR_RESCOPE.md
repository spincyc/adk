# Lessons 076--078 tabletop sonar rescope decision

Status: curriculum subject selected; implementation not authorized.

## Decision and provenance

Lessons 076--078 are reassigned from the unauthorized color-sensor proposal to
one tabletop-sonar arc:

| Lesson | Selected subject | Role | Learner result |
|---:|---|---|---|
| 076 | Copied Sweep-Range Frames | Component | Inspect attributable copied range observations paired with copied bounded-angle intent, and distinguish usable points from timeout, range, ordering, provenance, and source faults |
| 077 | Bounded Polar Occupancy Map | Component | Reduce qualified copied sweep frames into a fixed-capacity polar occupancy model whose unknown, clear, occupied, and conflicted bins remain explainable |
| 078 | Inert Tabletop Sonar Desk | Project-bearing | Replay a bounded scan, present its polar result and faults, and prepare volatile record intent without acquiring range, moving a servo, or claiming a physical map |

The selected hardware families are authorized by the deduplicated inventory
union in [`AUTHORIZED_ELEGOO_SET.md`](../inventory/AUTHORIZED_ELEGOO_SET.md).
The revisioned Mega manifest lists one HC-SR04 ultrasonic module and one SG90
servo, and the Upgraded 37-in-1 manifest independently lists the HC-SR04
family. Family listing authorizes curriculum planning only. It does not
identify an exact ultrasonic PCB or servo revision, establish ratings, approve
a supply, or prove powered behavior.

The repository already publishes the HC-SR04-shaped `PulseInput` and
`UltrasonicRanger` semantics from Lesson 019 and the `ServoOutput`,
`BoundedServo`, and calibration boundary introduced by Lesson 017. Reusing
those owned concepts makes this a composition and evidence-integrity arc, not
a second device-driver introduction.

## Engagement assessment

This arc scores **93/100** under the curriculum's weighted planning heuristic:
immediate interaction `5`, project payoff `5`, novelty `4`, feedback `5`,
first success `3`, and narrative `5`. An object moved by hand can eventually
change a radar-like polar picture; a swept pointer makes angle, distance,
uncertainty, and failure observable together; and the project has a
recognizable tabletop artifact. Honest E0 publication must still replay a
motionless copied scan until the ranger and servo pass separate physical
gates. The ordering preserves the payoff: Lesson 076 exposes inspectable scan
evidence, Lesson 077 turns it into a picture-shaped model, and Lesson 078
makes the complete diagnostic desk the project result.

The score is a curriculum-ordering judgment, not hardware evidence. It does
not justify bypassing the E1 ranger gate or the E2 servo gate, and it must not
be quoted as a support, accuracy, or safety rating.

## Distinction from published work

| Existing work | Boundary retained by this arc |
|---|---|
| Lesson 017 bounded servo | Lesson 017 owns pulse generation, calibration limits, timer/pin acquisition, command bounds, and the separately powered restrained-motion lesson. Lesson 076 copies an angle **intent or observation** with provenance; it neither emits a pulse nor proves a shaft angle. |
| Lesson 019 ultrasonic ranging | Lesson 019 owns trigger/echo acquisition, microsecond timing, timeout, conversion, and one range reading. Lesson 076 consumes copied range-shaped results and adds sweep correlation, angle association, sequence, and source provenance; it does not reacquire or reinterpret echo edges. |
| Lesson 021 tabletop rover | Lesson 021 decides bounded vehicle motion from range and wheel evidence. Lessons 076--078 issue no drive, steering, avoidance, navigation, or autonomous-motion intent. Their output is an inert map, presentation intent, and volatile record intent. |
| Lessons 040--042 optical observation and course marshal | Lessons 040--042 qualify presence/checkpoint evidence for course state and include one approach-range source. Lessons 076--078 instead preserve a single declared sweep's angle/range correlation and reduce it into polar occupancy. They do not infer a person, authorize a start, marshal a course, or weaken the earlier source-specific qualification rules. |

No occupancy bin establishes object identity, collision clearance, human
presence, safety separation, navigation fitness, or a metrically accurate
world model. “Sonar,” “sweep,” and “map” are instructional names for bounded
evidence transformations.

## Evidence and safety boundaries

The activatable software scope is E0 only. Deterministic tests and examples may
consume caller-supplied, copied HC-SR04-shaped range results, copied
bounded-servo angle values or intents, explicit source/configuration identity,
sequence/correlation fields, and supplied time. At E0 the assembly is
motionless: it owns no pin, pulse, edge capture, timer, interrupt, sensor,
servo, bracket, display, supply, storage device, or physical observation.
Copied values do not prove that either listed device produced them.

Lesson 076 must bind each copied range observation to exactly one declared
sweep source, configuration, angle item, sequence/correlation identity, and
observation time. The implementation-depth plan must freeze admission,
ordering, duplicate/gap, freshness, timeout, below/above-range, source-change,
angle-range mismatch, rollover, terminal, reset, and failure-precedence
semantics. It must state whether an angle is commanded intent, separately
observed position, or both; one must never be silently presented as the other.

Lesson 077 must consume only admitted frames and use fixed-capacity storage.
The plan must freeze angular bins, distance bands, boundary inclusion,
rounding, repeated-bin policy, unknown/clear/occupied/conflicted semantics,
coverage sufficiency, atomic publication, and fault propagation. Missing or
rejected evidence remains unknown rather than becoming clear space. A nearer
sample must not silently erase a contradictory sample without a documented,
tested rule.

Lesson 078 must compose Lessons 076 and 077 without re-decoding the ultrasonic
transport, inventing servo feedback, or weakening either result. It may
produce bounded semantic display cells, LEDs or pointer intents, and a
caller-owned volatile record image. Serial is optional supporting output.
Display, pointer, and persistence remain intent at E0, and presentation failure
must not alter evidence admission or map disposition.

E1 is the separate ranger/electrical gate. Before any powered HC-SR04 claim, a
human must inventory the exact module, cite primary timing and electrical
sources, verify pin order and supply/logic compatibility, analyze every
powered/unpowered combination and back-power path, freeze retrigger and timeout
limits, identify trigger/echo test points, provide an authoritative schematic,
record current and safe-state evidence, and complete a Mega 2560 bench result.
E1 does not authorize servo power or motion.

E2 is the separate servo-motion gate. Before any powered sweep, a human must
identify the exact servo, fixture and mechanical envelope; use a separately
current-limited load supply with common ground and an independent physical
power-removal control; record idle, moving, and stall current; verify pulse
bounds and calibration; restrain the first test; establish clearance and
cable travel; prove inactive and power-removed states; and record the Mega
bench result. The Mega signal pin never powers the servo. Passing E2 does not
retroactively qualify the ranger.

A combined moving-ranger fixture requires both gates plus integration evidence
for supply interaction, noise, timing, cable strain, acoustic self-
interference, current margin, scheduler load, and a physical stop. Until then,
the published desk remains motionless E0 replay.

## Architecture seams and stress risks

The implementation plan must preserve these seams:

- existing ranger acquisition ends at an attributable range result; copied
  sweep framing owns correlation and provenance, not trigger/echo mechanics;
- existing bounded-servo ownership ends at a bounded command/snapshot;
  copied frames distinguish command intent from observed position;
- the polar mapper is a pure, fixed-capacity transform over qualified frames;
- the desk orchestrates lifecycle and presentation but cannot reach through
  those components to pins, timers, source bytes, or mutable map internals; and
- transport, acquisition, mapping, presentation, and record failures remain
  separately attributable.

Mandatory stress work must address fixed-capacity frame and bin growth, AVR
SRAM/stack/flash pressure, timestamp and sequence wrap, scheduler interaction
between echo latency and servo settling, stale evidence after movement,
ambiguous angular quantization, contradictory returns, partial scans,
atomicity under injected failure, replay determinism, diagnostic interference,
resource ownership, shutdown order, and combined supply/noise behavior.
Adding feedback, a larger display, persistence, a second ranger, automatic
rescan, adaptive bins, or vehicle motion reopens the design rather than
entering as a local convenience.

## Alternatives and fallback

| Alternative | Disposition |
|---|---|
| Retain the color-sensor proposal | Rejected: the cited authorized inventory union contains no color-sensor family. |
| Add a new sonar or scanning module | Rejected: it would introduce an unlisted family when the authorized HC-SR04 and SG90 families already establish the teaching boundary. |
| Rebuild Lesson 019 acquisition inside Lesson 076 | Rejected: duplicate trigger/echo ownership would fork timing, fault, and resource semantics. |
| Treat commanded servo angle as measured position | Rejected: open-loop intent is not shaft-position evidence. A feedback sensor would be a separately authorized and qualified source. |
| Navigate or avoid obstacles from the map | Rejected: this would duplicate and materially expand Lesson 021 while converting an inert evidence desk into a moving safety problem. |
| Publish only a replay fixture | Retained as the required E0 path: it gives deterministic, motionless evidence and permits full policy testing before physical activation. |
| Breadboard topology fallback | Retained if the exact moving fixture, servo supply isolation, bracket, wiring clearance, or combined qualification is not ready. Mount no sensor on the servo; place the HC-SR04 stationary on the breadboard or a nonconductive support, keep the servo mechanically unloaded and separately unpowered, and exercise each electrical family only under its own accepted gate. The E0 desk continues to replay copied angle/range pairs. This fallback cannot claim a physical sweep or map. |

## Source-readiness matrix

| Evidence source | Ready for this decision | Required before implementation or physical promotion |
|---|---|---|
| `AUTHORIZED_ELEGOO_SET.md` revisioned manifests | Yes: establishes HC-SR04 and SG90 family provenance | Exact shipped specimens remain required for powered claims |
| Lesson 017 design and current bounded-servo contracts | Yes: establishes the existing calibration, pulse, ownership, and E2 seam | Consumer/API inventory and resource measurements required before composition; exact servo and acceptance card required for E2 |
| Lesson 019 design and current ultrasonic contracts | Yes: establishes the existing acquisition, range-state, timing, and E1 seam | Consumer/API inventory and resource measurements required before composition; exact HC-SR04 and acceptance card required for E1 |
| Primary HC-SR04 device/module evidence | Not accepted by this decision | Named durable sources for electrical limits, timing, retrigger interval, beam/target limitations, and exact module topology required before E1 |
| Primary SG90 evidence and exact fixture documentation | Not accepted by this decision | Exact ratings, pulse bounds, current evidence, supply, restraint, calibration, mechanical envelope, and stop procedure required before E2 |
| Polar-mapping policy precedent | Conceptually available from existing fixed-capacity copied-evidence work | Exact frame/bin model, failure precedence, goldens, resource budget, and stress-pass acceptance required before code |
| Exact combined fixture and bench evidence | No | Both independent gates plus combined scheduling, supply, noise, acoustic, restraint, and safe-state acceptance required before any moving-sonar claim |

## Dependencies and activation gates

The implementation-depth plan must depend explicitly on the published Lessons
017, 019, 021, and 040--042 boundaries; the copied-evidence, provenance,
fixed-capacity, and record-integrity patterns from Lessons 064--072; the
architecture stress-pass template; the safety model; the PDF visual policy;
and exact Mega 2560 resource budgets. It must inventory current
`BoundedServo`, `ServoOutput`, `PulseInput`, and `UltrasonicRanger` consumers
before selecting a public API.

Activation requires all of the following:

1. an implementation-depth three-lesson plan that freezes public types,
   lifecycle, capacities, provenance, correlation, time/order, angle meaning,
   mapping, terminal states, failure precedence, example narrative, and record
   format before first-class code;
2. separate pre-implementation architecture stress passes for Lessons 076,
   077, and 078, including the most demanding aggregate composition and exact
   SRAM, stack, flash, frame, bin, and record budgets;
3. primary-source-backed copied HC-SR04 semantics and an explicit declaration
   of which existing Lesson 019 result is reused without claiming acquisition;
4. a reviewed servo seam that keeps command intent distinct from observed
   position and leaves all pulse generation and motion outside E0;
5. deterministic golden fixtures covering nominal, partial, empty,
   contradictory, corrupt, stale, duplicate, gap, rollover, source-change, and
   injected-failure cases, plus every-byte corruption testing for any encoded
   image;
6. an explicit demonstration that the arc does not duplicate rover autonomy
   or optical course/presence policy;
7. HTML and pencil-drawing PDF outcomes that label replay, uncertainty, open
   physical gates, observation points, and the breadboard fallback honestly;
   and
8. independent acceptance cards for the E1 ranger and E2 servo gates, followed
   by a separate combined-fixture card, before any powered or moving
   publication claim.

Acceptance of this rescope means only that the lesson numbers, exact titles,
authorized families, engagement rationale, E0 composition boundary,
architecture seams, fallbacks, and activation gates are durable. It does not
authorize implementation, identify a physical specimen, support an HC-SR04 or
servo adapter, establish electrical or mechanical safety, prove motion,
produce a physical map, or record bench acceptance.
