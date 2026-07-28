# USB host-only durable-controller milestone

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> remains authoritative. This milestone proves controller persistence and
> recovery logic only; it performs no USB, VBUS, network, or kernel action.

Status: bounded next-milestone decision
Scope: deterministic single-process host model and tests
Physical evidence: deferred

## Decision

The next USB research milestone is one host-only durable-controller harness
around the existing product-native `Cau`/`Pau` `ColdMove` model. It replaces
the model's non-durable `FakeJournal` only at a test seam with a transactional
fake durable store, then proves fail-closed restart and reconciliation through
deterministic traces.

This milestone does not implement a USB/IP adapter, endpoint daemon, network
transport, kernel integration, physical attachment unit, VBUS control, or
production database. Its result is evidence about controller decisions, not
USB transparency.

## Single authority

Exactly one `ControllerProcess` is the writer. It owns desired routes,
controller term, per-topology epochs, operation results, and the ordered audit
chain. A fixed-capacity exclusive-writer lease prevents a second controller
instance from opening the store concurrently.

Endpoints are scripted observations in this milestone. They never allocate an
epoch, infer a route from cached state, elect a controller, or turn discovery
into desired state. Losing the writer lease or encountering ambiguous durable
state makes the controller unavailable and leaves every route detached or
faulted. High availability and replicated authority remain deferred.

## Persistence contract

One committed store image contains:

- schema version and store identity;
- current controller term and last committed transaction sequence;
- complete revisioned desired-route snapshot;
- highest allocated `TopologyEpoch` for every known topology;
- terminal result for every retained operation ID;
- observed-state evidence stored separately from desired state; and
- an append-only, hash-chained audit record for every accepted mutation.

A mutation becomes externally successful only after one atomic transaction
persists its desired revision, allocated fences, operation result, and audit
record. A rejected or failed transaction exposes none of them. Capacity,
sequence, term, or epoch exhaustion is a stable fail-closed result; values
never wrap, reset, or get silently evicted.

The fake store exposes explicit crash points before and after each record,
flush, commit marker, and success publication. It offers no filesystem,
database, or power-loss durability claim.

## Recovery contract

On restart, the controller:

1. acquires the exclusive-writer lease;
2. validates the complete committed image, version, bounds, sequence, and audit
   chain without partially loading it;
3. advances and commits the controller term before accepting work;
4. treats all pre-restart endpoint observations and in-flight operations as
   untrusted;
5. preserves every committed epoch and terminal idempotency result;
6. converts remembered `Preparing`, `ColdMove`, or `Active` work to a visible
   detached/faulted recovery state;
7. requests fresh scripted inventory and observations; and
8. permits a new attachment only through a new authorized operation with a
   current term and a strictly newer durable epoch.

Malformed, truncated, unsupported, internally inconsistent, or audit-invalid
storage opens read-only for diagnosis and authorizes no mutation. Recovery
never reconstructs an attachment from a prior `Active` record.

## Threat boundary

The harness protects the model from stale, duplicated, reordered, or
conflicting commands; controller-process crashes at declared persistence
boundaries; accidental concurrent writers; malformed durable input; capacity
exhaustion; and replay of an old term, epoch, operation ID, or observation.

It does not protect against a malicious operating-system administrator,
compromised process memory, forged or stolen credentials, rollback by an
attacker controlling storage, audit truncation by a privileged adversary,
side channels, hostile USB traffic, or physical tampering. Test digests detect
model inconsistency; they are not signatures or tamper-proof audit evidence.

## Deterministic failure tests

The gate uses a manually advanced logical clock, fixed-capacity containers,
scripted observations, and an enumerated scheduler. It opens no socket, reads
no wall clock, invokes no USB command, and touches no real USB device.

Required checked tests are:

- crash immediately before and after every transaction boundary, followed by
  restart from the last complete committed image;
- persistence failure for desired state, epoch allocation, operation result,
  audit append, flush, term advance, and commit marker;
- restart from every `ColdMove` phase, including an `Active` snapshot, with no
  implicit reattachment;
- duplicate operation replay before and after restart, returning one retained
  terminal result without a second action;
- stale term, epoch, observation, and completion delivery after restart;
- two controller instances racing for the single-writer lease;
- truncated, malformed, wrong-version, hash-invalid, non-monotonic, and
  internally inconsistent images, each loading transactionally or failing
  closed;
- maximum-minus-one, maximum, and maximum-plus-one boundaries for routes,
  operations, audit records, terms, epochs, and transaction sequences;
- ambiguity between committed desired state and scripted observed state,
  converging to detached/faulted rather than active; and
- byte-identical public state, writes, actions, and audit output when each
  checked-in trace is replayed twice.

The milestone passes only when the independent reference oracle agrees after
every scheduled event, sanitizer and ordinary host builds pass, every failure
prints a versioned replay trace, and documentation continues to label the
result host-only.

## Explicit deferrals

| Area | Deferred claim or work |
|---|---|
| Transparency | No native Windows/Linux enumeration, exact topology reconstruction, physical unplug/replug equivalence, hub-subtree proof, or USB-class compatibility. |
| Physical | No Cau/Pau hardware, connector role, VBUS removal/discharge, backfeed, PoE, auxiliary-power, thermal, EMC, ESD, or signal-integrity evidence. |
| Security | No authenticated enrollment, key storage, signed command, encrypted data plane, hostile-device isolation, secure boot/update, independent audit sink, or adversarial storage guarantee. |
| Performance | No USB throughput, latency, jitter, isochronous timing, network congestion, scale timing, recovery duration, or resource-footprint claim beyond deterministic configured bounds. |
| Compliance | No USB-IF, Ethernet, PoE, radio, safety, privacy, accessibility, or jurisdictional certification or conformance claim. |

## Exit and successor boundary

Completion produces only the durable-store interface contract, deterministic
host implementation, replay traces, and passing host gates described here.
The next decision may select an authenticated fake transport and endpoint
protocol harness, but only after this persistence/recovery contract passes.
Transport, kernel, and physical work do not enter this milestone by inference.
