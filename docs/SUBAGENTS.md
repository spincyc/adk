# Historical subagent status

Snapshot recorded: 2026-07-27

This table preserves an earlier delegation snapshot; its in-progress and
queued labels are historical, not live scheduling authority. Current work is
owned by [the work queue](WORK_QUEUE.md) and the durable journal task state.

| Workstream | Status | Result |
|---|---|---|
| Build portability | Complete | Found Arduino-Makefile coupling and defined CLI/host matrix |
| Independent code review | Complete | Found RGB dangling-registry defect and API issues |
| Comparable projects | Complete | Compared Arduino, micro:bit, Adafruit, MakeCode, Johnny-Five, and Mbed |
| Curriculum architecture | Complete | Defined evidence-centered component lesson structure |
| RAII architecture | Complete | Proposed hardware endpoint, component, and behavior layers |
| Lesson 001 authoring | Complete | Corrected commands, lifecycle claims, timing, and page flow |
| Lesson 002 authoring | Complete | Corrected measurement timing, power wording, lifecycle claims, and boxes |
| Lesson 003 authoring | Complete | Corrected schematic, experiment parity, PWM, lifecycle claims, and boxes |
| Object lifecycle review | Complete | Flagged callback mutation and incomplete shutdown contract |
| Pin review | Complete | Flagged PWM naming, ownership, ranges, and shutdown gaps |
| LED/color review | Complete | Flagged polarity, PWM validation, and shutdown gaps |
| Host-test review | Complete | Flagged registry regression and sanitizer coverage gaps |
| Make review | Complete | Fixed dependencies, cleanup safety, factoring, and explicit format checks |
| Arch bootstrap review | Complete | Removed invalid package and corrected permission guidance |
| Style review | Complete | Tightened namespace and parenthesis enforcement |
| Arduino layout review | Complete | Confirmed Mega FQBN, sketch layout, and CLI commands |
| Lesson 001 audit | Complete | Report delivered to lesson author |
| Lesson 002 audit | Complete | Report delivered to lesson author |
| Lesson 003 audit | Complete | Report delivered to lesson author |
| Documentation review | Complete | Separated current API from planned hierarchy |
| Git-history review | Complete | Proposed four buildable hierarchy-ordered commits |
| Safety review | Complete | Bounded RF/fireworks work and identified diagram/lifecycle claims |
| Library metadata review | Complete | Flagged missing license and Arduino example/public-header concerns |
| API naming review | Complete | Found `InputPullup` and parenthesis-style inconsistencies |
| Header review | Complete | Standalone/header/archive consumer builds passed |
| Size review | Complete | Quantified host flags and identified virtual-registry AVR cost |
| Pages architecture | Complete | Added self-contained stock-MkDocs staging and build |
| Documentation taxonomy | Complete | Added learner, library-user, and contributor navigation |
| API documentation | Complete | Separated current compatibility API from planned RAII API |
| Pages workflow | Complete | Added SHA-pinned, least-privilege build and deploy jobs |
| Site accessibility | Complete | Added WCAG, diagram, mobile, print, and PDF requirements |
| Site content audit | Complete | Added build, contribution, project, and about pages |
| Site build-tool review | Complete | Compared official Arch generators and selected MkDocs |
| Site validation | Complete | Added semantic HTML, link, fragment, asset, and PDF checks |
| Simon design documentation | Complete | Added deterministic engine, replay, test, and lesson plan |
| Release code audit | Complete | Verified host/AVR builds and catalogued deferred RAII and test work |
| Release Arduino audit | Complete | Verified Mega builds; identified future Library Manager layout work |
| Release lesson audit | Complete | Corrected timing, metadata, and independently buildable source downloads |
| Release documentation audit | Complete | Corrected accessibility and lifecycle status claims |
| Release site audit | Complete | Verified links, fragments, assets, PDFs, and `/adk/` base paths |
| Release workflow audit | Complete | Verified pinned Actions, permissions, artifact scope, and publication sequence |
| Release Git audit | Complete | Verified a linear fast-forward path from remote `main` |
| Live Pages audit | Complete | Confirmed Pages must be enabled after the first `main` publication |
## First-class implementation slice

The coordinator assigned all available delegation slots during the second
slice. Work was split by dependency boundary and file ownership; review agents
also used nested agents where an independent mechanical audit was useful.

| Agent workstream | Final status | Result |
|---|---|---|
| Architecture contract | Complete | Defined first-class layers, ordered commits, exact gates, and handoffs |
| Core design | Complete | Added shared timer leases, rollback, Mega PWM mapping, and 17-byte registry |
| Core tests | Complete | Covered shared claims, exhaustion, rollback, piezo timing, and teardown |
| Host fake | Complete | Added Mega pin count and deterministic tone/no-tone traces |
| Digital input integration | Complete | Reconciled names and style with the new sound path |
| PWM endpoint tests | Complete | Covered the complete Mega PWM map, conflicts, traces, and RAII |
| RGB component | Complete | Added three-endpoint composition, transactional color, and safe shutdown |
| RGB tests | Complete | Covered values, polarity, conflicts, rollback, traits, and destruction |
| Piezo component tests | Complete | Covered timer ownership, deadlines, rollover, replacement, and cleanup |
| Simon engine | Complete | Added fixed-capacity deterministic state machine and replay sources |
| Simon API review | Complete | Stabilized cue values, source lifetime, and C++11 construction |
| Test strategy review | Complete | Added high-value PWM, piezo, Simon, and replay cases |
| Style audit | Complete | Fixed alignment and reported contract, filename, and virtual-dispatch issues |
| Formatter capability review | Complete | Reported rules that require repository checks beyond clang-format |
| C++ style scan | Complete | Reported type duplication, lifecycle wording, and integration gaps |
| Size-budget audit | Complete | Enforced the 17-byte registry ceiling and flagged virtual Simon dispatch |
| Make integration | Complete | Integrated focused host, exception, Mega, umbrella, and PDF gates |
| Make independent review | Complete | Specified opt-in legacy isolation and stock-Arch quality gates |
| Quality CI | Complete | Added host, unwind, size, lessons, Mega, lint, and gated Pages jobs |
| Packaging review | Complete | Verified current artifacts and identified remaining release blockers |
| Git boundary review | Complete | Defined dependency-safe lesson, sound, Simon, site, and release commits |
| Curriculum | Complete | Canonicalized lessons and projects through lesson 030 |
| Project catalog | Complete | Defined a cumulative project at every third lesson |
| Component catalog | Complete | Reconciled experimental status, numbering, and registry size |
| Lesson 004 example | Complete | Added and measured the Mega PWM/RGB sketch |
| Lesson 005 PDF | Complete | Added pencil orientation art and verified the six-page PDF |
| Lesson 006 PDF/example | Complete | Added Simon art, RGB composition, and measured the Mega sketch |
| Lesson accessibility | Complete | Added language metadata and verified stable, extractable PDFs |
| Safety review | Complete | Added E1 limits, current budgets, inert-load rules, and RF exclusions |
| Site curriculum | Complete | Added lessons 004–006 and current component/project status |
| Supported API page | Complete | Documented experimental RAII ownership, timing, errors, and migration |
| Documentation taxonomy | Complete | Rebuilt concise learner pages and source/PDF navigation |
| Circuit observability policy | Complete | Required non-Serial test points and predict-observe-interpret evidence |
| Final documentation audit | Complete | Rechecked claims, links, release state, and deferred work |
| Analog input slice | Complete | Added raw Mega ADC ownership, tests, example, lesson, and circuit evidence |
| Calibration and filtering slice | Complete | Added deterministic integer mapping, sample filtering, tests, and lesson |
| Adaptive night-light project | Complete | Added hysteresis, bounded output intent, fault state, replay tests, and lesson |
| CLI and packaging gates | Complete | Added per-example build/upload, serial capture, archive smoke, and release checks |
| Firmware size baselines | Complete | Measured supported sketches and enforced versioned flash/SRAM budgets |
| HDMI 8K network research | Complete | Defined FPGA/SoC media planes, bandwidth limits, 25/100 GbE options, licensing gates, and phased prototypes |
| USB 3 matrix research | Complete | Defined terminated-edge tunneling, exclusive leases, authorization, SuperSpeed bandwidth, and phased prototypes |
| Lessons 010--030 delivery plan | Complete | Fixed uninterrupted dependency order, observability evidence, and explicitly deferred bench acceptance |
| Shift-register and display slice | Complete | Landed deterministic serialized output, presentation, tests, example, lesson, size evidence, and open bench card |
| Traffic timing and project slice | Complete | Landed explicit timing, `TrafficJunction`, replay tests, lessons 011--012, size evidence, and open bench cards |
| Lesson 013 implementation | Complete | Landed validated climate samples, host tests, example, lesson, and open bench card |
| Lessons 014--030 delivery | Complete | Landed each dependency-ordered boundary and project with host evidence and open bench cards |
| USB mesh control models | Complete | Landed deterministic routing, fencing, durable action planning, and build gates |
| Transparent USB product contract | Active reconciliation | Native Windows/Linux, CAU/PAU topology, electrical baseline, displays, profiles, and failure behavior |
| HDMI mesh control model | Complete | Landed deterministic route control and profile/failure-policy gates |
| HDMI mesh research | Complete | Documented interpreted media transport, reconstruction, dynamic routes, endpoint evidence, and compliance limits |
| Shared USB/HDMI fabric | Active reconciliation | Household coexistence, admission, profiles, QoS, congestion, and ordinary-LAN headroom |
| USB electrical baseline | Documented; qualification queued | Type-B/Type-A roles, Cat6A 10GBASE-T PoE++, auxiliary DC, VBUS isolation, ESD, thermal, and signal-integrity gates |
| USB topology vocabulary | Documented; implementation queued | One-port Cau, four-root Pau, atomic real hubs, no synthesis, and prototype-term isolation |
| USB ColdMove evidence | Documented; bench queued | Disconnect, discharge, epoch, repower, rediscovery, enumeration, and externally powered reset |
| USB display modes | Documented; implementation queued | Startup, normal, night, attention, fault, test, controller-lost, maintenance, and deterministic self-test |
| High-speed endpoint hardware | Queued research | No transparent USB or HDMI appliance, interoperability, compliance, or measured performance claim |

“Complete” means the assigned slice was delivered and locally checked; it does
not by itself promote an interface to supported status. Physical Mega evidence,
full integration gates, and publication remain coordinator responsibilities.

## Current checkpoint

Lessons 001--035 are host verified with their physical cards open. Lesson 036
is the active planned project boundary. Lessons 037--081 remain the queued
three-lesson expansion.

Transparent USB, HDMI mesh, and shared-fabric material remains research and
host-model evidence only. Bounded next USB and HDMI/shared-fabric research
milestones are queued at low priority; endpoint hardware, interoperability,
performance, compliance, and shared-LAN qualification remain deferred.
High-speed endpoint hardware is not supported.

## Durable integration boundaries

The durable journal assigns each active task a lease and records its file
ownership and checkpoints. Task owners may commit attributable implementation
and journal boundaries locally. Shared indexes, build fragments, navigation,
generated artifacts, release metadata, and status ledgers retain one named
integrator. No agent pushes, changes branches, merges, rebases, or rewrites
history without authority.

The historical integration sequence was:

1. isolate the imported preview API under `legacy/`;
2. land status, time, platform, board, and resource ownership;
3. land `DigitalOutput`, then `MonoLed`;
4. land `DigitalInput`, then `Button`;
5. land the Reaction Timer project and lessons 001–003;
6. land `PwmOutput` and `RgbLed`, then lesson 004;
7. land timer ownership and `PiezoSounder`, then lesson 005;
8. land the hardware-independent Simon engine, then lesson/project 006;
9. land analog input, sampled-signal behavior, and the Night Light project;
10. land shift-register output and display behavior;
11. land explicit traffic timing, then the Traffic Junction project;
12. continue the canonical three-lesson blocks through lesson 030;
13. land site, packaging, CI, release metadata, and publication at each
    coherent release boundary.

That sequence is complete through lesson 030. Current dependency order is
lessons 031--081, one implementation-depth three-lesson arc at a time.

Each boundary must build and pass its applicable checks before the next
consumer is committed. Review findings that cannot be closed at that boundary
remain explicit deferred work; no agent may weaken a gate or silently change a
public contract to make integration pass.

Physical Mega 2560 acceptance is a parallel campaign, not a prerequisite for
continuing later host-verified lesson implementation. Every unperformed bench
check stays visible in its lesson, project status, release notes, and
acceptance card; no agent may relabel compiled or simulated evidence as a
physical result.
