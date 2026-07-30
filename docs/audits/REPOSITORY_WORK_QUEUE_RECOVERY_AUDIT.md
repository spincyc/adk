# Repository work-queue recovery audit

Date: 2026-07-27  
Audited HEAD: `6b8af81` (`main`)  
Audit task: `3a7d0556-e29c-4a9f-9f71-0cce6c8caf23`

## Purpose

This audit reconstructs work that could otherwise be lost between agent
contexts. It does not promote an interface, report physical acceptance, or
authorize a release, publication, push, branch change, or external mutation.
`docs/WORK_QUEUE.md` remains the authoritative product ledger. Current task
execution is tracked separately by AIQ in repository-local, Git-internal
state; this audit is not a queue and does not seed that state automatically.

The audit covered the canonical contracts, tracked first-class source and
tests, examples, lesson sources and generated PDFs, site sources and
publication workflow, build and packaging machinery, research tracks, Git
branches and unreachable objects, status documents, and explicit deferred
physical work. `legacy/` was checked only for isolation and retained history;
it remains frozen.

## Recovery conclusions

1. No newer work is stranded on `feature/alpha`. Its tip is an ancestor of
   `main`, and `main..feature/alpha` is empty.
2. Git reports five unreachable commits. They are early 2026-07-27 development
   commits whose subjects duplicate work later incorporated into the main
   history. They are not evidence of a newer lost lesson or research boundary.
3. The durable product ledger retained the major work categories, but it did
   not decompose them into executable tasks. That historical finding does not
   describe current AIQ task state.
4. Lessons 001--028 have a complete tracked package shape: one canonical
   example, HTML page, TeX source, and generated PDF per lesson. Their physical
   acceptance remains genuinely open.
5. Lesson 029 is the immediate incomplete dependency boundary. Its brief,
   public source, and two focused tests exist, but it has no normal host-test
   registration, umbrella export, Mega example, size baseline, HTML page, TeX
   lesson, PDF, or site/navigation integration.
6. Lesson 030 has a committed design only. Its implementation is correctly
   blocked on reviewed promotion of lesson 029.
7. Lessons 031--081 are retained in canonical numbering. Only 031--033 has an
   implementation-depth block plan. Every later three-lesson block requires a
   detailed plan before code.
8. USB and HDMI work is retained as research. No evidence supports promoting a
   transparent endpoint, physical interoperability, performance, compliance,
   security, or shared-LAN deployment claim.

## Confirmed findings

### A. Active lesson boundary

- `src/cue_audit.cpp`, `src/inert_cue_scheduler.cpp`,
  `tests/test_cue_audit.cpp`, and `tests/test_inert_cue_scheduler.cpp` are
  tracked but absent from `HOST_TESTS` and host build recipes.
- `cue_audit.h` and `inert_cue_scheduler.h` compile alone, but are absent from
  `src/Adk.h`.
- The lesson 029 package is absent from `EXAMPLES`, `LESSONS`,
  `docs/size_baseline.tsv`, site lesson navigation, downloadable sketches,
  TeX/PDF rules, and generated artifacts.
- Lesson 030 remains a design recommendation and explicitly says not to
  implement until lessons 028 and 029 are reviewed dependencies.

### B. Release and build debt

The five restart-checkpoint blockers remain actionable:

1. `docs/lessons/014/main.tex` still teaches nonexistent
   `make serial-monitor`. Its generated PDF therefore also needs a reproducible
   refresh after the source correction. The queue's reference to lesson 012
   appears stale: lesson 012 source and PDF were committed together and the
   obsolete command is in lesson 014.
2. `mk/bootstrap.mk` omits `git`, installs unversioned `arduino:avr`, and
   therefore differs from CI's `arduino:avr@1.8.8`.
3. `serial-log` pipes the monitor through `tee` under POSIX `make` shell
   semantics without a pipe-failure mechanism. A monitor failure can be hidden
   by a successful `tee`.
4. Arduino installed-archive smoke coverage exists, but there is no native C++
   archive/export plus clean consumer compile/link smoke target. General
   non-Arduino installation must remain unclaimed.
5. The Pages workflow deploys but performs no post-deployment HTTP checks for
   the landing page, newest lesson, newest PDF, or downloads.

The local `make quality` path also runs only one Arduino Lint mode, while
`docs/PACKAGING.md` describes strict submit and update modes. CI currently runs
the submit mode only. This needs an explicit policy decision before a release,
not an inferred relaxation.

### C. Status drift

- `docs/ROADMAP.md` enumerates host-verified slices only through lesson 026,
  despite 027 and 028 being promoted elsewhere.
- `docs/PROJECTS.md` says “Projects 029--030” even though 029 is a component
  lesson, and its surrounding status prose is older than the current boundary.
- `docs/SUBAGENTS.md` retains historical “in progress,” “queued,” and “active
  reconciliation” entries that conflict with later commits and the
  authoritative queue. It should be labeled as history or reconciled, not used
  as live scheduling state.
- `site/pages/lessons/022.md` still calls lesson 022 active integration and says
  shared package gates remain, conflicting with the repository-wide
  host-verified-through-028 claim.
- The restart checkpoint names a stale lesson 012 PDF, while the directly
  evidenced obsolete command is in lesson 014. The ledger must correct this
  target before implementation.

### D. Packaging and generated artifacts

- All first-class lesson source/PDF pairs 001--028 are present.
- `library.properties` archives compiled objects through `dot_a_linkage=true`;
  this is intentional Arduino behavior, not native packaging.
- Ignored root object files and build trees exist locally. They are generated,
  untracked artifacts and did not contaminate the audit commit.
- Public-header isolation passes for all tracked headers, including the lesson
  029 headers. Passing header isolation does not substitute for normal
  registration or component tests.

### E. Physical and inventory work

- Every lesson 001--028 still lacks a recorded physical Mega 2560 acceptance
  result. This is a retained campaign, not 28 software blockers.
- Exact module identity, markings, voltage limits, and primary sources remain
  prerequisites for claimed kit coverage and later physical blocks.
- Physical RTC/removable storage, USB/HDMI endpoints, shared-LAN
  qualification, and high-energy or unknown-protocol work remain explicitly
  deferred or out of scope.

## Verification evidence

Passed at the original audited boundary:

```text
python3 .journal/bin/journal.py validate
make check
make headers-check
make lessons-check
make site-check
```

The repository later replaced that journal with AIQ. At the migration
boundary, `aiq journal check` separately verified the private AIQ database;
that later result does not rewrite the evidence recorded by this audit.

`make check` initially raced with a separately invoked `make site-check` over
`build/site-source`; the isolated rerun passed. This demonstrates that shared
site build targets must be serialized, not a product failure.

Not run as audit evidence:

- the full `make quality` release gate;
- all Mega firmware builds and measured size checks;
- installed-archive smoke builds;
- Arduino Lint;
- live Pages HTTP checks;
- physical bench checks.

Those belong to the relevant implementation or release task and must not be
reported as passing from this audit.

## Ordered recovery outcomes

These are historical recovery briefs, not pending work merely because they
appear in this document. Reconcile each outcome against
`docs/WORK_QUEUE.md` and current repository evidence before enqueueing it
through AIQ. The coordinating agent owns inbox interpretation, task creation,
dependencies, leases, and settlement; delegates do not write AIQ state.

### Prompt 1 — reconcile the recovered ledger

> Create a high-priority task to reconcile the repository status surfaces with
> the 2026-07-27 recovery audit. Correct the stale lesson-PDF target using
> direct evidence, update `docs/WORK_QUEUE.md`, `docs/ROADMAP.md`,
> `docs/PROJECTS.md`, `docs/SUBAGENTS.md`, and stale lesson/site status prose,
> and preserve historical records explicitly rather than silently rewriting
> them. Acceptance: every active/queued/deferred claim agrees with
> `CURRICULUM.md`; lesson 029 is the only active lesson integration boundary;
> 030 is queued behind it; 031--081 and all physical/research deferrals remain
> visible; documentation checks pass. Do not implement lesson code or claim
> hardware evidence.

### Prompt 2 — independently review lesson 029 core

> Create a high-priority task, dependent on the recovered-ledger task, to
> independently audit lesson 029's `CueAudit` and `InertCueScheduler` contracts
> against `docs/lessons/029/IMPLEMENTATION_BRIEF.md` and
> `docs/lessons/030/design.md`. Cover lifecycle, invalid plans, confirmation
> windows, delayed updates, same-time ordering, rollover, stop dominance,
> resume, fixed-capacity exhaustion, audit encoding, and shutdown. Fix only
> confirmed core/test defects. Acceptance: focused strict-warning tests pass,
> interfaces remain inert-only and C++11/AVR compatible, no launcher or
> energetic semantics appear, and a review event records remaining integration
> work.

### Prompt 3 — register lesson 029 core

> Create a high-priority task dependent on the lesson 029 core review to add
> `CueAudit` and `InertCueScheduler` to the normal host-test build and supported
> umbrella surface. Preserve dependency order and add no example or lesson
> prose yet. Acceptance: both focused tests run under ordinary, exception,
> sanitizer, style, and standalone-header gates; all earlier tests pass; public
> exports match the reviewed contract; the work queue records the completed
> component/test sub-boundary.

### Prompt 4 — complete and promote lesson 029

> Create a high-priority task dependent on lesson 029 core registration to
> deliver the complete lesson 029 boundary: narrative Mega 2560 example,
> explicit E1 pin/current budget, TP29 and separate acquisition/safe-state
> evidence, measured firmware size and baseline, HTML reference, rich
> monochrome TeX/PDF lesson, pencil orientation asset, downloads, navigation,
> changelog and status reconciliation, and open hardware card. Acceptance:
> host, header, style, Mega compile, size, lessons, PDF, site, and package-smoke
> gates pass; no physical result is invented; lesson 029 becomes host verified
> with bench acceptance open.

### Prompt 5 — implement lesson 030 capstone

> Create a high-priority task hard-dependent on promoted lessons 028 and 029 to
> implement the E0/E1 inert show-cue simulator exactly within
> `docs/lessons/030/design.md` and the safety model. Deliver the component
> composition, deterministic replay and complete fault matrix, narrative Mega
> example, size evidence, HTML, rich PDF, indexes, packaging, and open bench
> record. Acceptance: no output can represent or drive ignition, continuity is
> synthetic/inert, stop dominance and restart lockout are proven, all
> non-hardware gates pass, and lesson 030 is host verified with bench acceptance
> open.

### Prompt 6 — repair bootstrap and serial tooling

> Create a normal-priority tooling task, parallel-safe with lesson core work
> but serialized for shared docs, to add `git` to Arch bootstrap packages, pin
> the local AVR core to the CI version, preserve Arduino monitor failures
> through `serial-log`, replace the stale `make serial-monitor` command in
> lesson 014, and reproducibly rebuild the affected PDF. Acceptance: shell/Make
> behavior is portable and failure-tested where practical; local and CI version
> guidance agrees; lesson/PDF/site checks pass; unrelated generated PDFs do not
> change.

### Prompt 7 — add native C++ consumer packaging

> Create a normal-priority packaging task to define and implement a
> repository-native C++ archive/export and clean consumer compile/link smoke
> test without weakening Arduino packaging. Specify supported headers,
> compiled sources, install/archive layout, C++ standard, and absence of
> repository-relative dependencies. Acceptance: a temporary clean consumer
> builds from the exported artifact, Arduino package-smoke still passes, docs
> make only the demonstrated installation claim, and no framework or package
> manager is introduced.

### Prompt 8 — finish site navigation and deployment verification

> Create a normal-priority publication-readiness task dependent on the current
> active lesson boundary to audit landing-page hierarchy and navigation, then
> add post-deployment checks for the live landing page, newest lesson HTML,
> newest PDF, and downloadable example. Keep external deployment mutations out
> of ordinary tests. Acceptance: local site validation covers the newest
> boundary; the Pages workflow fails on bad HTTP responses or missing expected
> content; permissions remain least-privilege; live publication itself is not
> performed without separate authority.

### Prompt 9 — reconcile release and lint policy

> Create a normal-priority release-policy task, dependent on tooling,
> packaging, site, and the intended lesson boundary, to reconcile the documented
> Arduino Lint submit/update requirements with local Make and CI. Audit version,
> changelog, metadata, archive contents, generated artifacts, status tables,
> and release notes. Acceptance: a documented decision names which lint modes
> run for initial inclusion and later updates; the complete release gate is
> reproducible; failures remain queued with owners. Do not tag, push, publish,
> or submit to Library Manager.

### Prompt 10 — execute the full clean release-readiness gate

> Create a high-priority verification task hard-dependent on completion of the
> intended lesson, tooling, packaging, site, and release-policy tasks. From a
> clean tree, run the complete host, exception, sanitizer, research-model,
> header, style, Mega, size, lesson, monochrome/PDF, site, archive-consumer, and
> lint gates. Record exact tool versions, measurements, failures, owners, and
> next actions in AIQ task state and the work queue. Acceptance: all runnable
> gates pass or have durable blockers; no hardware, release, tag, push, or
> live publication claim is made.

### Prompt 11 — plan the physical acceptance campaign

> Create a normal-priority, human-bench-dependent parent task for physical Mega
> 2560 acceptance of lessons 001--030 in numeric order. Decompose one child per
> lesson only when the exact board, specimen, supply, instruments, and operator
> are available. Each child must separately record resource acquisition,
> primary behavior, non-Serial evidence, shutdown/reset/power-removal safe
> state, and any E2/external-power boundary. Software agents may prepare cards
> and review evidence but must never fabricate observations or mark a child
> complete without the signed bench record.

### Prompt 12 — complete exact kit inventory

> Create a normal-priority inventory task to fill the exact-module records for
> the owned kit using markings, photographs supplied by the operator, primary
> datasheets, voltage/current limits, polarity, and ambiguity flags. Acceptance:
> every later lesson claim cites an identified specimen or remains
> inventory-blocked; retail kit names are not treated as electrical identity;
> no unsafe limit is discovered experimentally; curriculum numbering does not
> change.

### Prompt 13 — implement lessons 031--033

> Create a normal-priority task hard-dependent on lesson 030 promotion and the
> relevant exact inventory to implement the already detailed 031--033 input
> expansion block in strict dependency order: analog joystick, quadrature
> encoder, then calibration console. Use
> `docs/design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md` as the contract.
> Acceptance: each lesson receives its complete component/test/example/size/
> HTML/PDF/open-bench package; lesson 033 proves composition; all earlier gates
> remain green; status documents advance together.

### Prompt 14 — deepen the next post-033 block

> Create a normal-priority planning-only task, hard-dependent on the prior
> three-lesson block's promotion and relevant inventory, for lessons 034--036.
> Expand the canonical subjects to implementation depth: public values and
> interfaces, resource/pin/current budgets, deterministic fixtures and failure
> matrices, narrative examples, staged circuit experiments, HTML/PDF division,
> exact-specimen gates, safety class, and open bench acceptance. Acceptance: an
> independent review finds the block implementable without inventing hardware
> details. Do not write first-class code in this task.

For lessons 037--081, repeat Prompt 14 one three-lesson block at a time, using
the canonical subjects in `docs/WORK_QUEUE.md`. Make each planning task depend
on promotion of the preceding block. After independent review, create a
separate implementation task for that block. Do not pre-activate all sixteen
blocks; queue them to preserve order and avoid stale detailed plans.

### Prompt 15 — bound the next USB research milestone

> Create a low-priority research task to reconcile the USB research documents
> and executable models into one next host-only milestone. Select a bounded
> gap such as durable controller persistence plus an idempotent endpoint-agent
> protocol simulator; do not implement physical USB transport, claim
> transparency, or deploy kernel mutations. Acceptance: the decision names the
> single authority, persistence and recovery contract, threat boundary,
> deterministic failure tests, and explicit physical/compliance deferrals.

### Prompt 16 — bound the next HDMI/shared-fabric milestone

> Create a low-priority research task to reconcile HDMI and shared-fabric
> documents into one next synthetic, host-only milestone. Prefer deterministic
> reconciliation or capacity/admission simulation before hardware. Acceptance:
> generated unprotected media only, no HDCP bypass, no physical 4K/8K or latency
> claim, explicit controller and fail-closed behavior, deterministic tests, and
> retained licensing, endpoint, thermal, EMC, and shared-LAN qualification
> blockers.

## Scheduling summary

The safe critical path is:

```text
ledger reconciliation
  -> lesson 029 review
  -> lesson 029 registration
  -> lesson 029 package/promotion
  -> lesson 030 implementation/promotion
  -> lessons 031--033
  -> one detailed three-lesson block at a time
```

Tooling, native packaging, and inventory can proceed beside lesson 029 when
their file ownership does not overlap shared ledgers or generated artifacts.
Site integration and release-readiness serialize at the current promoted
lesson boundary. Physical work requires human equipment and observations.
USB/HDMI research remains low priority and must not displace the first-class
curriculum dependency path.
