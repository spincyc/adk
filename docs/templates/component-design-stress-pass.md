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

## Composition pressure scenario

Exercise the boundary inside the most demanding composition that is currently
authorized and sufficiently specified. Name the consumer, instance counts,
update cadences, simultaneous stimuli, finite capacities, observation paths,
and failure/recovery sequence. Do not invent a future API or generic
abstraction to fill an evidence gap.

For each row record `applicable` or `not applicable`, with evidence. A
not-applicable disposition must identify the contract fact that excludes the
pressure; “not used in this example” is insufficient when an authorized
consumer uses it. If an applicable pressure cannot yet be measured or replayed,
record it as an open promotion blocker rather than assuming capacity.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | Under the maximum named composition, bound total update work, worst-case burst order, cadence, latency, simultaneous timestamps, rollover, retry/backoff, and missed-update behavior. Show that one component cannot starve another or silently turn explicit time into scheduler dependence. |
| Total memory and hardware resources | Add all live objects, copied inputs/outputs, fixed buffers, stack peaks, flash, SRAM, claim-registry entries, pins, timers, interrupts, buses, ADC modes/references, power domains, and non-Serial observation paths. Account for coexistence and margin, not isolated component sizes. |
| Shared bus or transport | When any named composition shares a bus or transport, identify its owner, borrower lifetimes, arbitration/order, bounded transaction and queue behavior, addressing, partial initialization rollback, congestion, and one participant failing or restarting. Otherwise name the contract fact proving no bus/transport participation. |
| Persistence and recovery | When state crosses reset or power loss, identify record provenance, schema/configuration identity, commit atomicity, corruption/torn-write behavior, capacity/wear, recovery ordering, and what downstream consumers may observe during retry. Otherwise establish that all state is intentionally volatile. |
| Motion, external power, or stored energy | When the composition can move, switch an external supply, or retain energy, identify authority, interlocks, bounds, de-energized startup/shutdown, command expiry, reset, resource loss, and colliding-fault safe state. Otherwise identify the absence of an actuation path, not merely its omission from a demo. |
| Observation identity and provenance | Define which source, timestamp/sample epoch, calibration/configuration revision, validity/status, and transformation produced each copied observation. Prove simultaneous or delayed values cannot be combined as though they describe one measurement, and that replay retains the same identity rules. |
| Diagnostic interference | Account for diagnostic LEDs, sounders, displays, Serial/telemetry, trace storage, and named test points in the same pin/timer/bus/memory/time budgets. Prove enabling, disabling, filling, or failing diagnostics cannot change primary behavior, hide safe-state evidence, or become required for correctness. |
| Failure collision and recovery | Inject the credible maximum collision: resource contention plus source/timing failure and, when applicable, bus, storage, diagnostic, reset, or actuation failure. Define precedence, attribution, rollback, retained evidence, retry, and safe state so one fault cannot mask or reclassify another accidentally. |

The maximum-composition proof uses deterministic replay or a bounded
composition fixture plus measured aggregate size/resource evidence. Cover
capacity immediately below, at, and above the supported limit; worst-case
simultaneous work; failure during partial acquisition or commit; shutdown and
restart with faults still present; and byte-stable replay when that is part of
the contract. Hardware evidence remains a separate gate and must not be
claimed from the host proof.

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
- Maximum-composition scenario and proof:
- Promotion permitted: yes / no
