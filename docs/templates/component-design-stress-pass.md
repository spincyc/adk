# Component design stress pass

Complete this record before promoting every new endpoint, component, or
component-composing project. The pass asks whether the new boundary extends
the architecture naturally, rather than merely whether its implementation
passes tests.

## Boundary

- Name and lesson/project:
- Reviewer and date:
- Public types and operations:
- Direct dependencies:
- Existing decisions and interfaces reconsidered:

## Fit review

Record evidence for every row. `Natural` means the existing contract expresses
the requirement without special cases, duplicated policy, hidden ownership, or
an abstraction used outside its stated purpose.

| Pressure | Questions | Evidence and disposition |
|---|---|---|
| API and layering | Do dependencies still point downward? Is circuit meaning in the component, electrical lifetime in endpoints, and capability/claims below both? Does the public API describe one coherent responsibility without exposing an implementation detail? | |
| Ownership and lifecycle | Is every owning relationship explicit, inert after construction, non-copyable where required, and bounded by `initialize()`/`shutdown()`? Are borrowed lifetimes valid? Do partial acquisition, destruction, restart, and shutdown preserve claims and safe state? | |
| Time and ordering | Does time still enter explicitly? Are simultaneous inputs, rollover, retries, sampling cadence, latency, and event snapshot lifetime defined without blocking or hidden clocks? | |
| Errors and status | Do existing `Status`/`Result<T>` semantics describe every outcome? Are transient and terminal failures distinguished without diagnostic strings, exceptions, silent fallback, or component-specific status conventions? | |
| Resource budget | Are pins, timers, buses, claim-registry entries, fixed storage, flash, SRAM, stack, update work, and electrical observation paths budgeted? Can the component coexist with its planned consumers on a Mega 2560? | |
| Deterministic proof | Can host tests reproduce behavior from configuration, timestamps, samples, failure injection, and seed? Do tests cover rollback, safe state, every transition, boundaries, replay, and composition pressure rather than only the happy path? | |
| Packaging and public surface | Do the Arduino archive, native source scope, umbrella header, standalone headers, build inventories, and size baselines express the boundary without exceptions or duplicated source? | |
| Example and documentation fit | Does the canonical example retain acquire/configure/start and observe/decide/actuate flow? Do code, HTML, PDF, schematic, pencil drawings, test points, and acceptance record use the same vocabulary and one canonical sketch? | |
| Downstream effects | Which existing components, projects, examples, lessons, tests, status tables, publication checks, packaging promises, and physical acceptance records inherit or contradict this decision? | |

## Prior-decision impact

List every relevant architecture, safety, curriculum, packaging, testing, and
publication decision. For each, record one of:

- `preserved` — the new boundary stays inside the decision, with evidence;
- `extended` — the decision anticipated this pressure and only its enumerated
  scope grows;
- `challenged` — the new boundary needs an exception, reinterpretation,
  duplicated mechanism, compatibility break, or change with broad consumers.

Absence of an explicit decision record is not evidence that no prior decision
exists. Search canonical documents, design records, Git history, tests, and
the durable journal.

## Stress disposition

Choose exactly one:

- `natural fit` — all rows have evidence and no prior decision is challenged;
- `bounded local remediation` — strain is confined to this unpromoted
  boundary, preserves public behavior and prior decisions, and has a named
  owner and verification;
- `architectural remediation required` — the pressure changes a public
  contract, layer boundary, ownership model, timing/status convention,
  resource policy, packaging claim, canonical-source relationship, or several
  prior consumers.

For architectural remediation, stop promotion. Record the evidence, affected
decisions and consumers, alternatives, migration/compatibility cost, safety
and resource consequences, and a recommended bounded next experiment. Discuss
the alternatives with the user before selecting a materially different
outcome. Then record the consequential decision durably before implementation.
Do not disguise the change as a component-local cleanup or retroactively
rewrite earlier decisions.

## Gate result

- Disposition:
- Open risks:
- Required discussion or decision IDs:
- Remediation owner and next action:
- Verification commands and results:
- Promotion permitted: yes / no
