# Sensor engagement reorder audit

Date: 2026-07-28
Updated: 2026-07-30 for the selected Lessons 073--075 RTC integrity arc
Scope: planned Lessons 040--081

## Decision

Front-load the remaining curriculum for learner engagement, subject to
dependency, safety, inventory, and publication constraints. Lessons 001--039
are published and immutable. Reordering therefore begins at Lesson 040 and
moves complete three-lesson arcs rather than separating a component from its
processing lesson or project.

This is a constrained whole-arc sort, not a subjective list of favorite
parts. The requested preference is specific enough to apply without
clarification: prefer immediate, visible, interactive behavior early; retain
foundational and completeness-oriented work later; and never improve apparent
engagement by weakening an electrical, identity, safety, or dependency gate.

## Engagement rubric

Score each three-lesson arc from 0 through 5 on the following factors. Apply
the weights to the arc as a learner experience, including its project, rather
than scoring a sensor name in isolation.

| Factor | Weight | High score means |
|---|---:|---|
| Immediate physical response | 25% | A learner can cause and see, hear, or feel a bounded response within the first experiment |
| Project payoff and play value | 25% | The third lesson produces a recognizable, replayable object or game-like interaction |
| Sensor novelty | 15% | The observation feels materially different from switches, knobs, and earlier measurements |
| Feedback richness | 15% | The arc offers several clear circuit-native states without depending on Serial |
| Time to first success | 10% | Wiring, calibration, and interpretation permit a quick, reliable first result |
| Narrative accessibility | 10% | The purpose is legible before the learner understands the internal mechanism |

The weighted engagement score is a scheduling input, not the complete ordering
function. Apply these constraints before comparing scores:

1. preserve the published Lessons 001--039 exactly;
2. move only complete three-lesson arcs;
3. keep prerequisites before consumers, reusing earlier components rather
   than duplicating them;
4. preserve exact-specimen, primary-source, voltage, and physical-acceptance
   gates;
5. keep E2 motion and externally powered work behind inert intent and stop-path
   preparation;
6. use only the listing-authorized Elegoo family union unless a separate
   decision explicitly admits an external reference fixture; and
7. leave unresolved replacement arcs unresolved rather than inventing an
   engaging but unauthorized subject.

Within the remaining legal orders, use the weighted score first, then prefer
lower setup burden, broader reuse by later arcs, and less repetition of the
immediately preceding sensing modality.

### Reproducible scores

`Immediate`, `payoff`, `novelty`, `feedback`, `first success`, and `narrative`
are each integer scores from 0 through 5. The weighted total is:

```text
5 × immediate + 5 × payoff + 3 × novelty
  + 3 × feedback + 2 × first_success + 2 × narrative
```

This produces a score out of 100 and is exactly equivalent to the percentage
weights above.

| Old arc and subject | Immediate | Payoff | Novelty | Feedback | First success | Narrative | Weighted total |
|---|---:|---:|---:|---:|---:|---:|---:|
| 040--042 optical course marshal | 5 | 5 | 4 | 5 | 4 | 5 | 95 |
| 043--045 museum-case monitor | 3 | 4 | 4 | 5 | 2 | 5 | 76 |
| 046--048 kinetic light sculpture | 5 | 5 | 4 | 5 | 2 | 5 | 91 |
| 049--051 parts carousel | 4 | 5 | 4 | 5 | 2 | 5 | 86 |
| 052--054 IR command translator | 5 | 5 | 5 | 4 | 3 | 5 | 93 |
| 055--057 characterization bench | 2 | 3 | 3 | 4 | 2 | 3 | 56 |
| 058--060 escape-room console | 5 | 5 | 4 | 5 | 1 | 5 | 89 |
| 061--063 balance-table instrument | 5 | 5 | 5 | 5 | 3 | 5 | 96 |
| 064--066 motion recorder | 3 | 3 | 4 | 4 | 1 | 3 | 62 |
| 067--069 thermal gradient mapper | 4 | 4 | 4 | 4 | 2 | 4 | 76 |
| 070--072 dual-display timing desk | 5 | 4 | 3 | 5 | 3 | 4 | 83 |
| 073--075 RTC integrity and Inert Time-Warp Detective Desk | 3 | 4 | 4 | 4 | 3 | 5 | 75 |
| 076--078 replacement pending | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| 079--081 component qualification bench | 2 | 3 | 2 | 4 | 2 | 3 | 53 |

The zeroes for Lessons 076--078 mean “no authorized subject exists to score,”
not that the eventual replacement project has no engagement value. That arc
cannot participate in the score sort until its authorized scope is chosen.
Lessons 073--075 now use the listed DS1307 RTC family. Their score reflects a
less immediate first encounter than optical or motion work but a strong,
legible detective-story payoff: copied clock evidence can visibly expose
rollover, jumps, rollback, stopped time, power-loss state, and stale
observations without pretending that E0 has read or qualified hardware.

### Constraint and tie-break overrides

Sorting totals descending would not be a legal curriculum order. Apply these
overrides in sequence to reproduce the final map:

1. keep the already prepared optical arc at 040--042. It is the committed next
   dependency boundary and moving it behind the higher-scoring balance arc
   would discard implementation-depth planning for a one-point score gain;
2. place balance next because it has the highest remaining score and depends
   only on published foundations plus exact inertial identity;
3. keep kinetic before carousel: the carousel consumes the bounded stepper,
   homing, stop, and inert-intent discipline introduced by the kinetic arc;
4. keep carousel before IR despite IR's higher raw score because the existing
   local-identity and motion sequence is a coherent dependency chain and
   because IR transmission retains an exact-emitter and timer gate;
5. place escape-room only after optical, inertial, tactile, identity, and IR
   observations exist, since it is a composition project rather than a source
   of those prerequisites;
6. place the display arc immediately after the escape-room. Its score is next
   among unconstrained arcs and its presentation capability benefits all
   remaining measurement work;
7. keep the corrected museum arc before the equally scored thermal mapper.
   Museum monitoring introduces the broader
   temperature/validity/radiant context; the mapper then deepens temperature
   identity and transport without back-to-back inertial repetition;
8. defer the second inertial arc until after thermal work so it extends the
   earlier balance lesson instead of repeating the same modality immediately;
9. place characterization after the application-led arcs because its purpose
   is comparison and completeness, not first exposure;
10. retain the selected RTC integrity arc at 073--075 and the unscored
    076--078 reservation in place; the RTC arc fills an authorized-family
    coverage gap without disturbing the dependency-sorted earlier sequence;
    and
11. retain 079--081 last because it is the bounded qualification capstone and
    depends on the preceding breadth even though its raw score exceeds the
    unscored reservations.

## Reordered arc map

Lesson numbers in this table identify the first lesson of each three-lesson
arc. Every move carries the entire old `n--n+2` arc to new `m--m+2`.

| Old arc | New arc | Retained project or role | Reason for placement |
|---:|---:|---|---|
| 040--042 | 040--042 | Tabletop course marshal | Keeps the already prepared next arc first; optical checkpoints, presence, range, and visible timing give fast interaction and a strong project payoff |
| 061--063 | 043--045 | Balance-table instrument | Moves responsive inertial sensing and orientation feedback near the front, where motion-to-light/tone behavior has high novelty and immediacy |
| 046--048 | 046--048 | Kinetic light sculpture | Retains early tactile interaction and bounded motion, after inertial observation has supplied a gentler motion concept and while E2 stop and power gates remain explicit |
| 049--051 | 049--051 | Tabletop parts carousel | Retains the tangible identity, homing, and lightweight-mechanism payoff after bounded stepper work |
| 052--054 | 052--054 | IR command translator | Retains a familiar remote-control interaction with visible round-trip evidence after timer and policy prerequisites |
| 058--060 | 055--057 | Inert escape-room console | Pulls the strongest multi-sensor, game-like composition forward once its optical, motion, identity, and IR prerequisites exist |
| 070--072 | 058--060 | Dual-display timing desk | Moves rich display feedback ahead of slower environmental work and makes the presentation capability available to later arcs |
| 043--045 | 061--063 | Museum-case monitor | Retains environmental coverage after the high-interaction sequence, with the authorized sensor correction below |
| 067--069 | 064--066 | Thermal gradient mapper | Keeps identity-rich digital temperature work near the corrected environmental arc |
| 064--066 | 067--069 | Interchangeable motion recorder | Defers the more analytical second inertial arc so it extends, rather than immediately repeats, the balance-table experience |
| 055--057 | 070--072 | Module characterization bench | Places descriptor-driven completeness work after application-led sensor arcs, where comparison and qualification consolidate prior experience |
| 073--075 | 073--075 | Copied RTC Transaction Evidence, Qualified Clock Observation, and Inert Time-Warp Detective Desk | Gives the authorized DS1307 family a late-course integrity investigation with a recognizable “bad clock” payoff after learners have the record and qualification vocabulary to distinguish valid civil time from continuity |
| 076--078 | 076--078 | Authorized-family replacement pending | Retains stable numbers and unresolved scope |
| 079--081 | 079--081 | Component qualification bench | Retains the bounded driver and qualification capstone at the end |

## Inventory corrections required during migration

The reorder does not authorize every specimen named by the old prose. Apply
the listing-authorized inventory boundary while rewriting the moved arcs.

- The kinetic-light arc at Lessons 046--048 must remove capacitive-touch and
  finger-heartbeat coverage. Its input scope is limited to authorized Metal
  Touch, contact or switch families, and joystick observations, with each
  physical module still subject to exact identity and electrical
  qualification. It makes no physiological claim.
- The museum-case arc moved to Lessons 061--063 must use authorized Water
  Level, thermistor, the separately listed Digital Temperature family,
  radiant or Flame-family observation, and reed contact. It must remove rain,
  soil-moisture, and generic analog-temperature claims. Similar retail labels
  do not make those families interchangeable.
- Lessons 073--075 now use the authorized DS1307 family for copied
  register/request/receipt evidence, civil-time and continuity qualification,
  and a volatile timeline-integrity project. This E0 selection does not restore
  DS1302, BMP180, or PCF8591, and it does not claim a powered RTC.
- Lessons 076--078 remain pending re-scope. The prior color-sensor subject is
  not silently restored.
- Lessons 079--081 remain qualification work; their position does not turn an
  unidentified transistor, load, or indicator board into a supported
  specimen.

## Dependency and safety effects

The new sequence preserves the three-lesson component, policy, project rhythm.
The balance-table arc can move to 043 because it depends on already published
analog input, joystick, LEDs, sound, display foundations, and an
inventory-qualified inertial adapter—not on the later motion-recorder arc.
The motion recorder remains later because normalization, source
qualification, stable records, RTC, and storage make it a deeper analytical
follow-up.

The RTC integrity arc remains late because Lessons 067--072 first establish
copied records, provenance, source qualification, discontinuity-aware policy,
and byte-stable evidence. Lesson 073 can then preserve DS1307-family register
framing and correlated copied request/receipt evidence; Lesson 074 can
distinguish civil-time validity, freshness, jumps, stopped-clock state, and
power-loss evidence; and Lesson 075 can turn those distinctions into the
volatile Inert Time-Warp Detective Desk. DS3231 is a separately identified
shipping variant, never an interchangeable DS1307 implementation: its
identity, register and feature semantics, electrical behavior, and physical
acceptance retain independent gates.

The kinetic and carousel arcs remain ordered so bounded stepper ownership,
intent mirroring, travel budgets, de-energized shutdown, independent stop
evidence, and separable motor power precede more elaborate motion. Reordering
does not relax their E2 bench gates. The IR arc retains the known-code-only
transmission policy and cannot replay unknown captures. The escape-room
console moves only after the sensor and operator-input families it composes;
all actuator outputs remain inert or lightweight and stop-dominant.

Environmental and thermal arcs may move later without becoming prerequisites
for the front-loaded projects. Their identity, corrosion, calibration,
radiant-source, single-wire timing, and physical acceptance requirements
remain intact. The characterization bench remains useful late in the course:
learners first encounter sensors in purposeful applications, then compare
identified low-voltage analog/comparator families with a common disciplined
method.

## Coverage preservation

Reordering changes presentation, not the support claim. Preserve:

- every authorized family already assigned a legitimate role;
- raw evidence, validity, age, identity, and faults beside interpreted state;
- deterministic host fixtures and byte-identical replay where specified;
- a non-Serial observation path and separate acquisition and safe-state
  evidence in every lesson;
- exact-specimen and primary-source gates before powered work;
- all open physical acceptance records; and
- deferred work for unsupported, unidentified, or unauthorized specimens.

Removing unauthorized names is a correction, not a loss of promised coverage.
The coverage audit must distinguish listing-authorized planned families,
exactly qualified specimens, external teaching references, and unsupported
retail aliases.

## Exhaustive Upgraded 37-family placement

The following table accounts for every numbered family in the official
Upgraded 37-in-1 V2 image. A lesson assignment is a current or planned
instructional role, not an electrical-support claim. “Gate” identifies where
the listing cannot by itself authorize powered work.

| # | Authorized family | Current or planned lesson placement | Qualification boundary |
|---:|---|---|---|
| 1 | Joystick | 031--033; reused 043--048 and 055--057 | Exact axes, switch, center, voltage, and pinout remain specimen-gated |
| 2 | Two-Color | 079--081 indicator semantics and qualification | Exact polarity, common terminal, and resistor sizing open |
| 3 | IR Emission | 052--054 known local codes | Exact emitter topology, wavelength, current limit, timer use, and eye-safe acceptance open |
| 4 | Membrane Switch (4×4 keypad) | 016; reused 049--057 | Exact tail order and Mega bench record open |
| 5 | RGB LED | 004; reused throughout planned projects | Published circuit assumes the documented common-cathode configuration; specimen polarity remains open |
| 6 | 7 Color Flash | 079--081 indicator semantics and qualification | Treat only as an identified bounded indicator; exact internal behavior and polarity open |
| 7 | Laser Emit | No powered lesson; explicitly excluded | Remains unpowered unless separately identified, rated, reviewed, and admitted by a safety decision |
| 8 | SMD RGB | 079--081 indicator semantics and qualification; RGB semantics may be reused only after identification | Package pinout, common terminal, onboard resistors, and current limits open |
| 9 | Tilt-Switch | 034--036; optional qualified contact in 037--039 and 046--048 | Exact contact topology, orientation, bounce, and safe stimulus open |
| 10 | Photo-Resistor | 008--009; reused 040--042 | Divider, tolerance, surface/ambient calibration, and physical acceptance open |
| 11 | Ultrasonic Sensor (HC-SR04) | 019; reused 040--042 presence/passage | Exact timing, supply, echo level, range environment, and Mega bench evidence open |
| 12 | Button | 002--003 and many later projects | Exact switch and pull policy remain bench-gated |
| 13 | Active Buzzer | 079--081 indicator semantics and qualification | Exact active polarity, voltage, current, and sounder behavior open |
| 14 | Shock | Optional substitution coverage in 037--039 | Exact specimen conformance remains open; external contact references are canonical there |
| 15 | Water Level Sensor | 061--063 corrected museum monitor | Exact topology and corrosion-aware switched-power acceptance open; no unattended leak-safety claim |
| 16 | IR Receiver | 025; reused 052--054 | Exact receiver identity and known-remote bench trace open; unknown replay excluded |
| 17 | Passive Buzzer | 005--006; reused in later projects | Exact transducer and acoustic bench acceptance open |
| 18 | DS1307 RTC Module | Deterministic RTC state in 022; Copied RTC Transaction Evidence in 073, Qualified Clock Observation in 074, and the Inert Time-Warp Detective Desk in 075 | Lessons 073--075 are E0 copied-evidence policy only; physical DS1307 acquisition remains deferred pending exact chip, pull-up, charging, cell, electrical, register, accuracy, and bench qualification. DS3231 is a separate shipping variant and is never accepted as an interchangeable DS1307 |
| 19 | LCD 1602 Module | 014--015; reused in later projects | Exact controller and pinout bench record open; no implied PCF8574 support |
| 20 | 18B20 Temp | 064--066 single-wire thermal mapper | Exact marking, package, pull-up, power mode, timing, and CRC acceptance open |
| 21 | Rotary Encoder | 032--033; reused in 055--057 operator panel | Exact phase order, pull policy, and pushbutton conformance open |
| 22 | Relay | Inert intent in 023--024 and 061--063 | No physical relay adapter or mains load; exact coil/driver/contact identity requires separate E2 acceptance |
| 23 | HC-SR501 | 040--042 presence/passage | Warm-up, retrigger mode, output level, age, and exact specimen acceptance open |
| 24 | GY-521 | 043--045, then 067--069 | Build only the adapter matching the inventoried MPU-6050/QMI-8658 revision; bus, address, rails, and orientation open |
| 25 | Power Supply | Construction infrastructure across lessons; bounded qualification may appear in 079--081 | Not a sensor or generic software adapter; regulator, jumpers, polarity, backfeed, thermal, and loaded rails must be inventoried |
| 26 | Temp and Humidity | 013--015 DHT11 climate work | Exact module identity, timing, electrical acceptance, and no-safety-alarm boundary remain open |
| 27 | Photo-Interrupter | 040--042 optical observations | Exact output topology, slot geometry, pull policy, current, and ambient behavior open |
| 28 | Tap Module | Optional substitution coverage in 037--039 | Exact specimen conformance remains open; no generic equivalence to the canonical contact fixture |
| 29 | Tracking | 040--042 optical observations | Exact emitter/receiver topology, polarity, range, crosstalk, and surface calibration open |
| 30 | Magnetic Spring | 034--036; reed contact reused in 061--063 | Exact contact behavior, magnet/orientation, pull policy, and acceptance open |
| 31 | Avoidance | 040--042 optical observations | Exact IR topology, polarity, threshold control, crosstalk, and range open |
| 32 | Digital Temperature | 061--063 corrected museum monitor | Distinct from 18B20; exact IC, interface, units, voltage, and pinout unresolved, so no powered adapter yet |
| 33 | Flame | 061--063 radiant observation using a controlled low-energy IR source | No open flame required; exact spectral response, topology, voltage, polarity, and acceptance open |
| 34 | Linear Hall | 034--036 magnetic observation | Exact analog range, sensitivity, polarity response, supply, and device identity open |
| 35 | Big Sound | Optional substitution coverage in 038--039; 070--072 only after identification | Exact analog/comparator topology and conformance open; no SPL claim |
| 36 | Metal Touch | 046--048 kinetic input | Listing-authorized only; exact topology, pinout, threshold, safe stimulus, and conformance open |
| 37 | Small Sound | Optional substitution coverage in 038--039; 070--072 only after identification | Exact analog/comparator topology and conformance open; no SPL claim |

This table is exhaustive for the numbered V2 37-family image. Mega-kit-only
families—such as RC522, stepper/ULN2003, displays, discrete thermistor, motors,
servo, and shift register—remain covered by the union inventory and the arc
map, but they are not extra entries in this 37-row reconciliation.
With the DS1307 RTC integrity placement selected for 073--075, no authorized,
admissible sensor family in this table is left unused; each has a published or
planned instructional role, while a prohibited family retains its explicit
non-coverage disposition. “Planned” remains a scheduling statement rather
than a powered-support or specimen-interchangeability claim.

## Migration checklist

Implement the reorder as one coordinated curriculum migration before beginning
new first-class Lesson 040 work:

1. record the scheduling decision and update the authoritative work queue;
2. renumber the complete arc briefs and headings in the curriculum, project
   catalog, roadmap, cadence, coverage audit, and safety taxonomy;
3. repair every prerequisite, next-lesson, project-number, planned-range, and
   cross-document link affected by the moves;
4. apply the kinetic and museum-case inventory corrections before presenting
   their briefs as implementation-ready;
5. reconcile task dependencies and queued task titles without erasing their
   immutable history;
6. verify that Lessons 001--039, their URLs, downloads, examples, and size
   records did not change;
7. update the site’s planned rows and scan-first arc order while leaving
   unpublished lessons linkless;
8. run curriculum, site, link, safety-policy, and queue-consistency checks;
9. inspect the final diff for stale old-to-new numbering and unauthorized
   sensor names; and
10. commit and publish the migration as a coherent boundary before activating
    the reordered Lesson 040 arc.

## Clarification assessment

No clarification is required to enqueue or plan this migration. “Coolest”
becomes an explicit, reviewable engagement objective through the rubric above,
and the hard constraints prevent it from overriding safety or curriculum
integrity. A later preference for a specific audience, such as younger
learners, classroom teams, or instrumentation-focused learners, could justify
different weights, but it is not needed to make this ordering decision.
