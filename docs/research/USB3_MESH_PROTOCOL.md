# USB 3 mesh control protocol

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Protocol `Source` and `Destination` names below are older
> abstractions; product routes bind one Cau `ComputerPort` to one complete Pau
> `TopologyRoot`, fenced by a durable `TopologyEpoch`.

> **Controller decision:** phase one uses one durable authoritative controller.
> Replication, quorum consensus, leader election, and automatic controller
> failover are deferred. Protocol terms remain fencing fields for safe restart.

Status: research contract; not a supported ADK interface.

This protocol lets any admitted USB source endpoint be assigned to any
compatible destination endpoint without giving endpoint names, process
restarts, or delayed messages authority over the current route. It defines the
control plane only. USB transfer framing, timing, bandwidth reservation,
isochronous behavior, and device virtualization belong to the data plane.

## Terms and identities

| Term | Stable identity | Meaning |
|---|---|---|
| Node | `nodeId` | One authenticated mesh appliance |
| Source | `sourceId` | One exported USB device or explicitly grouped composite device |
| Destination | `destinationId` | One importing USB host-facing port or virtual host attachment |
| Route | `routeId` | One desired source-to-destination relationship |
| Controller incarnation | `term` | Monotonic generation advanced on durable controller recovery |
| Ownership fence | `epoch` | Monotonic generation for one source device |

Identifiers are opaque, globally unique byte strings. A physical port number,
USB address, bus number, IP address, hostname, serial number, or descriptor is
metadata, not identity. A node persists its identity across restart. A source
or destination keeps its identity while its physical ownership remains the
same; replacing hardware creates a new identity unless an administrator
explicitly adopts the replacement.

Every endpoint advertisement includes its `nodeId`, stable endpoint identity,
kind, capabilities, health generation, and a monotonically increasing
`advertisementRevision`. Capability fields include at least USB generation,
speed, transfer types, power role, data-plane transports, and whether the
source is indivisible or exposes independently routable interfaces.

## Authority and fencing

One durable controller authors desired state for a controller `term`. It
advances the term after recovering durable state on restart. Endpoints reject
a command whose term is lower than the greatest term they have durably
accepted. Replicated authority and leadership terms remain a later HA design.

Each source device has one authoritative `epoch`. A route change increments
that epoch before detach or attach begins. Destination capacity reservations
are separate records: they prevent oversubscription but never grant ownership.
Every prepare, detach, attach, transfer, heartbeat, acknowledgement, and
teardown message carries:

```text
protocolVersion
meshId
term
destinationId
epoch
operationId
```

The destination and source reject a lower term or epoch. They accept an exact
duplicate idempotently. They reject a conflicting command with the same
`operationId`, term, and epoch. Data-plane packets carry the destination
identity and epoch; an endpoint discards stale packets even if an old
connection remains alive.

Epoch exhaustion is a hard administrative fault. An implementation must not
wrap or silently reset a term, epoch, revision, or sequence value.

## Desired and observed state

Desired state is a complete, revisioned snapshot, not a sequence of imperative
edits:

```text
DesiredMesh
    meshRevision
    term
    routes[]

DesiredRoute
    routeId
    sourceId
    destinationId
    epoch
    policy
```

`policy` records compatibility requirements, admission labels, bandwidth
class, reconnect policy, and optional expiry. Absence from a newer complete
snapshot means detached. A controller may distribute snapshot deltas, but each
delta names its base revision and is rejected when that base is unavailable.

Observed state is reported independently by each participant:

```text
ObservedEndpoint
    endpointId
    advertisementRevision
    health
    activeRouteId
    activePeerId
    acceptedTerm
    acceptedEpoch
    phase
    lastError
    observationRevision
```

Valid phases are `Detached`, `Preparing`, `Quiesced`, `Attached`, `Draining`,
and `Faulted`. The controller derives convergence by comparing desired and
observed state. It never treats delivery of a command as proof of attachment.
Wall-clock timestamps may aid operators but do not decide protocol ordering.

## Route transaction

A destination has at most one source. A source is exclusive by default.
Broadcast or multi-destination export requires an explicit source capability
and separate USB semantic analysis; it is never inferred from topology.

Reconfiguration uses this deterministic sequence:

1. The controller validates identities, admission policy, compatibility, and
   capacity against one topology revision.
2. It allocates the source device's next epoch and durably records desired
   state plus the separate destination-capacity reservation.
3. `PrepareRoute` asks both endpoints to reserve bounded resources without
   exposing a USB attachment.
4. If replacing a route, `QuiesceRoute` and `DetachRoute` fence the old epoch.
   Both endpoints report `Detached` or the operation reaches a declared
   failure policy.
5. `ActivateRoute` authorizes the prepared source and destination at the new
   epoch.
6. The route is converged only when both observed records report the same
   route, peers, term, epoch, and `Attached` phase.

Activation is detach-before-attach. A failed preparation leaves the prior
route desired and active. A failure after the new epoch is committed leaves the
destination detached or faulted; it never restores an old epoch automatically.
Retrying the same operation is safe. Choosing a different outcome requires a
new operation and epoch.

## Messages

| Message | Direction | Purpose |
|---|---|---|
| `AdvertiseEndpoint` | node to controller | Publish stable identity, capabilities, and health |
| `PublishDesired` | controller to nodes | Supply a complete revision or based delta |
| `PrepareRoute` | controller to endpoints | Reserve resources for one fenced route |
| `Prepared` / `Rejected` | endpoint to controller | Report deterministic preparation result |
| `QuiesceRoute` | controller to source | Stop accepting new work and drain bounded work |
| `DetachRoute` | controller to endpoints | Remove exposure and reject the fenced epoch |
| `ActivateRoute` | controller to endpoints | Expose the new route after both preparations |
| `ReportObserved` | node to controller | Publish local state and accepted fences |
| `Heartbeat` | bidirectional | Prove session liveness without changing state |
| `RequestSnapshot` | either direction | Recover a missing base revision |

Every reply echoes `operationId`, term, destination, and epoch. Results use
stable status codes plus optional human diagnostic text. Implementations must
not branch on diagnostic text. Messages use a canonical encoding for audit
hashing and signatures; unordered maps and floating-point fields are excluded.

## Idempotency and ordering

`operationId` is globally unique and retained for at least the maximum message
and reconnect lifetime. Each endpoint durably stores its accepted term, the
latest epoch per source device it observes, and the terminal result of recent
operations. Duplicate messages return the stored result without repeating USB
side effects.

Messages may be duplicated, delayed, lost, or reordered. Correctness depends
only on term, epoch, revision, and operation identity. Transport connection
order and arrival time confer no authority. All waits have explicit monotonic
deadlines; timeout creates an observation, not proof that the remote action
failed.

## Disconnect and restart

On control-session loss, an endpoint applies its configured lease policy:

- `detachOnExpiry`: quiesce and detach after a bounded lease;
- `holdUntilExpiry`: retain the exact fenced route until the lease expires;
- `manualRecovery`: enter a visible fault state and accept only a higher-term
  recovery command.

The safe default is `detachOnExpiry`. Lease renewal carries the exact term and
epoch and cannot revive an older route.

After restart, a node first restores durable fences with USB exposure disabled.
It advertises observed `Detached` or recoverable prepared state, obtains the
current desired snapshot, and reconciles. It must not reconstruct authority
from a surviving TCP connection, local cache alone, or enumeration order.

Network partitions can leave a source and destination unable to coordinate.
Neither side may attach to a new peer at the same or lower fence. A source that
supports multiple destinations maintains independent fenced sessions; it does
not share endpoint-global mutable USB state unless its capability explicitly
permits that composition.

## Discovery, admission, and scale

Discovery locates candidates; it does not authorize them. A node joins only
after mutual authentication and assignment to a `meshId`. Admission policy
binds stable identities to roles and capability constraints.

Controllers paginate endpoint and route snapshots and watch revisions from a
declared starting point. Compaction returns `SnapshotRequired`; it never
silently skips history. Future sharding may partition authority by source
device identity, but one source device must have exactly one epoch authority
at a time. Topology changes and endpoint health updates do not increment
device ownership epochs
unless they cause a desired route change.

Audit records include the actor, canonical request, prior and new desired
revision, term, epoch, endpoint observations, result, and previous-record hash.
Audit storage failure prevents a state-changing command from becoming
authoritative.

## Versioning

The wire envelope has a major and minor protocol version plus a feature-bit
set. A major mismatch rejects the session. Within one major version, unknown
fields are preserved or ignored as specified by their field class; unknown
required features reject the affected operation. Senders omit semantics they
did not negotiate.

Stored desired state, observed state, and audit records carry schema versions.
Migration is explicit and deterministic. A node upgrades durable state before
enabling USB exposure and cannot downgrade while records use unsupported
semantics.

## Deterministic verification

A reference simulator should accept a topology, initial durable records, and
an ordered fault script. It should emit canonical desired revisions, endpoint
observations, and audit records. Tests must cover duplicates, every message
reordering, lost replies, endpoint and controller restart, partition, stale
pre-restart commands, delayed old data packets, epoch exhaustion, malformed
snapshots, capability changes, simultaneous reconfiguration attempts, and
audit failure.
Replaying identical inputs must produce byte-identical output.

## Open design questions

1. Is a source normally exclusive, or must the first implementation support
   deliberate fan-out for read-only HID or other safely virtualized functions?
2. Does a physical composite USB device move atomically, or may independently
   virtualizable interfaces receive separate stable source identities?
3. Which failure policy is acceptable when an old destination cannot confirm
   detach: remain unavailable, or permit a higher-epoch attach after a bounded
   lease and explicit operator-visible risk?
4. Which USB semantics are mandatory for the first data plane: bulk and
   control only, or interrupt and isochronous transfers as well?
5. What authentication root and node-enrollment workflow is appropriate for a
   lab mesh while remaining replaceable for production deployments?
