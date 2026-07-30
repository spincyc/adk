# Roadmap

`CURRICULUM.md` is the canonical lesson and project numbering authority.
`COMPONENTS.md` owns the target component inventory.
`WORK_QUEUE.md` is the authoritative active, queued, deferred, physical, and
publication ledger.

## Current host-verified slices

1. no-exception status, wrap-safe time, fixed resource claims, and runtime;
2. `DigitalOutput` and `MonoLed` for early visible diagnostics;
3. `DigitalInput` and deterministic `Button`;
4. deterministic Reaction Timer with lessons 001–003;
5. `PwmOutput`, `RgbLed`, and shared default-timer leases;
6. timer ownership and the nonblocking `PiezoSounder`;
7. deterministic `Simon`, fixed and seeded cue sources, and lessons 004–006;
8. `AnalogInput` with explicit raw sampling and Mega analog-pin validation;
9. deterministic calibration and sampled filtering without hidden time;
10. adaptive `NightLight` intent with hysteresis, bounded duty, explicit
    invalid-sample behavior, and lessons 007–009;
11. shift-register and seven-segment ownership, explicit traffic timing, and
    the traffic-junction project in lessons 010–012;
12. validated climate samples and the owned DHT11 adapter in lesson 013;
13. staged character-display output and stable records in lesson 014;
14. deterministic environmental-station composition in lesson 015;
15. release-gated matrix keypad input in lesson 016;
16. bounded servo intent, versioned configuration records, owned Timer5/D44
    pulse output, and explicit external-power admission in lesson 017;
17. inert access policy with keypad input, visible soft-latch intent, lockout,
    and bounded audit presentation in lesson 018;
18. explicit ultrasonic echo timing, timeout, and range validity in lesson 019;
19. bounded motor intent with reversal dead time and stop dominance in lesson
    020;
20. deterministic range-aware rover supervision with inert command evidence in
    lesson 021;
21. owned I2C/SPI devices, direct Mega bus adapters, calibrated moisture
    observations, explicit RTC state, and fixed durable records in lesson 022;
22. transactional inert LED loads and deterministic watering policy in lesson
    023;
23. coherent greenhouse stages, visible health patterns, and durable record
    retry in lesson 024;
24. owned bounded infrared capture, classic NEC validity, and stable
    receive-only records in lesson 025;
25. exact receive-only telemetry packets, freshness tracking, bounded packet
    reception, and visible evidence in lesson 026;
26. deterministic multi-source telemetry scheduling, attention, presentation,
    and stable record replay in lesson 027;
27. synthetic inert-channel continuity assessment with explicit open, short,
    stale, and contradictory evidence in lesson 028;
28. deterministic inert cue scheduling, confirmation, stable snapshots, and
    bounded audit replay in lesson 029; and
29. physically inert show-cue composition with complete channel observations,
    continuity gating, cancellation dominance, and replayable audit evidence
    in lesson 030;
30. calibrated two-axis joystick observations and explicit selection events in
    lesson 031;
31. deterministic quadrature decoding with invalid-transition evidence in
    lesson 032; and
32. atomic calibration preview, trim, commit, and cancel composition in lesson
    033;
33. qualified analog and contact magnetic observations with distinct specimen,
    dwell, hysteresis, and fault evidence in lesson 034; and
34. deterministic passage qualification with direction, timeout, duplicate
    suppression, and optional position corroboration in lesson 035; and
35. durable magnetic passage logging with explicit clock, two-slot recovery,
    retry, and post-commit presentation evidence in lesson 036;
36. qualified contact and acoustic evidence with deterministic percussion
    sequencing in lessons 037--039; and
37. source-specific optical observations, bounded presence composition, and
    explicitly authorized course-marshal timing in lessons 040--042;
38. copied inertial evidence, pure orientation and presentation policy, and a
    stationary balance-table composition in lessons 043--045; and
39. copied tactile/directional evidence, bounded logical stepper intent, and
    transactional kinetic-sculpture composition in lessons 046--048; and
40. copied local-identity evidence, bounded logical homing, and acknowledged
    inert parts-carousel composition in lessons 049--051;
41. copied infrared evidence, closed local emission intent, and inert
    translation in lessons 052--054;
42. copied clue evidence, fault-aware operator policy, and inert console
    composition in lessons 055--057;
43. supplied-time digit and matrix presentation intent composed into a
    dual-display timing desk in lessons 058--060;
44. copied environmental evidence composed into an inert museum-case monitor
    in lessons 061--063; and
45. copied single-wire receipts, fixed-probe qualification, and deterministic
    thermal-gradient, page, and volatile record intent in lessons 064--066.

The component APIs and behavior engines pass deterministic host tests and
compile for the Mega 2560. Physical acceptance cards remain open, so this work
is experimental rather than hardware supported.

## Next slice

Lessons 034--042 are host verified and published with their bench gates open.
Lessons 037--039 use documented external reference fixtures and retain open
incoming-conformance gates. Earlier Elegoo exact-specimen questions are
superseded as canonical-publication blockers, not answered; they remain
historical optional substitution-conformance work. Lessons 040--042 publish
pure policy and replay evidence; powered optical/PIR adapters, exact specimens,
and E1 acceptance remain open.

Lessons 043--045 are the host-verified
[implementation-depth E0 replay-first slice](design/LESSONS_043_045_BALANCE_TABLE_PLAN.md):
`InertialObservationPolicy`, `OrientationPolicy`,
`BalancePresentationPolicy`, and `BalanceInstrument` process copied synthetic
inertial samples and inputs for a stationary hand-operated tabletop
instrument. Deterministic host tests, compile-only Mega replays, size gates,
and lesson publication pass. No powered adapter, I2C transaction, wiring
table, formal schematic, or physical claim is authorized. Exact MPU6050 and
QMI8658 variants remain independently gated. Lessons 067--069 retain inertial
normalization, source qualification, cross-device comparison, and
motion-recorder scope.

Lessons 046--048 are the host-verified
[implementation-depth E0 slice](design/LESSONS_046_048_KINETIC_SCULPTURE_PLAN.md):
`InteractionIntentPolicy`, `BoundedStepperSequence`, and
`KineticLightSculpture` process copied tactile/directional evidence and
logical coil intent. Mega replays measure 6,956/733, 8,068/1,053, and
22,216/1,470 bytes of flash/static SRAM. Powered inputs and indicators remain
E1-open; the exact 28BYJ-48/ULN2003 and energized sculpture remain E2-open.

Lessons 049--051 are the host-verified
[implementation-depth E0 slice](design/LESSONS_049_051_PARTS_CAROUSEL_PLAN.md):
fixed local identity records over copied evidence, bounded logical homing, and
an inert parts-carousel composition. Deterministic host tests, compile-only
Mega replays, measured size gates, HTML references, and pencil-drawing PDFs
pass. The Lesson 051 coordinator intentionally publishes zero coil intent
after each atomic one-step application and reconciles an attributable terminal
after every exposed start record.
RFID/keypad/home inputs, powered indicators, and nonvolatile storage remain
E1-gated; exact stepper/driver and servo hardware, powered motion, restraint,
independent stop and power removal, and measured acceptance remain E2-gated.

Lessons 052--054 are the host-verified E0 slice under the clean-reviewed
[infrared-translator plan](design/LESSONS_052_054_IR_TRANSLATOR_PLAN.md).
This boundary adds copied receive evidence, a pure immutable local-command
emission policy, and an inert fixed-allowlist translator without owning an
optical or electrical endpoint. Exact receiver, emitter, driver, resistor,
timer, pin, supply, observation path, and powered acceptance remain gated to
the separately qualified E1 fixture; E0 does not authorize unknown-protocol
replay or make an eye-safety claim. Canonical Mega replays measure
5,530/1,096, 4,854/276, and 16,162/1,343 bytes of flash/static SRAM. The
maximum composition measures 21,864/3,531 bytes; the Lesson 054 object is
407 B plus a caller-owned 400 B pulse buffer, and its conservative 888 B stack
estimate leaves 3,773 B after static storage and stack. These host results do
not claim powered or optical verification.

Lessons 055--057 publish copied clue evidence, fault-aware operator policy,
and an inert escape-console composition. Lessons 058--060 publish supplied-time
digit and matrix presentation intent composed into a dual-display timing desk.
Lessons 061--063 publish copied environmental evidence composed into an inert
museum-case monitor. Lessons 064--066 publish bounded copied single-wire
transactions, fixed-probe qualification, and `ThermalGradientMapper`, which
produces deterministic interval-gradient, fault, page, and volatile record
intent. It validates structure rather than source authenticity, drives no
display, and writes no storage. The Lesson 066 canonical replay measures
16,662/2,210 bytes flash/static SRAM; exact no-LTO evidence measures
18,822/2,210 bytes with 855 bytes conservative stack and 4,999 bytes residual
SRAM. Six target misses were independently reviewed below their hard limits.

Lessons 070--072 publish copied threshold-module characterization and an inert
module-characterization bench. The
[RTC integrity re-scope decision](design/LESSONS_073_075_RTC_INTEGRITY_RESCOPE.md)
selects Lessons 073--075 for the next planning boundary: **Copied RTC
Transaction Evidence**, **Qualified Clock Observation**, and the **Inert
Time-Warp Detective Desk**.
This is planning-selected, not implementation-ready. An implementation-depth
plan, per-component architecture stress passes, deterministic acceptance
matrices, resource budgets, and publication design are required before code.
Exact DS1307 or documented DS3231-family specimen identification and E1 bench
acceptance remain open, including module topology, register-map revision,
pull-up rails, backup-supply and cell-chemistry safety, oscillator behavior,
wiring, primary sources, and measured evidence.

The
[Lessons 076--078 re-scope decision](design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md)
selects **Copied Sweep-Range Frames**, **Bounded Polar Occupancy Map**, and the
**Inert Tabletop Sonar Desk**. This is planning-selected, not
implementation-ready: an implementation-depth plan and initial architecture
stress passes for all three lessons are mandatory before code. E0 is limited
to copied angle/range evidence and deterministic intent; exact ultrasonic
acquisition and physical ranging remain E1-gated, while restrained powered
servo motion remains E2-gated. Physical mapping accuracy, powered
presentation, and bench acceptance remain open. Lessons 079--080 publish the
E0 bounded low-side-driver and small-indicator semantics policies; Lesson 081
retains the unfinished inert qualification-bench continuation. The historical DS1302, BMP180,
PCF8591, and color-sensor assignments remain excluded rather than silently
restored.
Lessons 067--069 remain published E0 copied-record policy; exact motion
acquisition, presentation, and persistence remain open E1a--E1c work.
Authorization, prerequisite, exact-specimen, safety, and evidence gates still
control activation; the ordering is not a support claim.

Every component requires lifecycle tests, deterministic fakes, a canonical Mega
example, size evidence, terse HTML, a rich complementary PDF, and recorded
hardware acceptance. Work that misses a gate remains experimental.

ADK will not implement pyrotechnic ignition, launcher control, remote cloning,
or unknown-protocol transmission.

## Parallel research tracks

Two longer-range investigations sit outside the Arduino lesson curriculum and
support claim:

- full 8K HDMI transport and switching over a packet network;
- a many-port USB 3 switching matrix over a packet network.

Both require dedicated high-speed transceivers and FPGA/SoC or Linux-class
data-plane hardware. The Mega 2560 is appropriate only for a deterministic
control panel, status display, power/environment observation, and fault
injection. It cannot terminate HDMI or USB SuperSpeed links.

Research proceeds from standards and measurements, then a one-to-one synthetic
prototype, then authenticated switching. HDMI work starts with unprotected
generated video; USB work starts with owned loopback devices on an isolated
network. Compliance, licensing, content protection, device authorization,
signal integrity, thermal design, electromagnetic compatibility, bandwidth,
latency, and failure recovery are release gates rather than cleanup tasks.

These tracks may produce architecture notes and host simulations before they
produce hardware. They do not change the current API status, safety boundary,
or lesson cadence.
