# USB 3 mesh reconciler

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Reconciliation ultimately controls physical Cau/Pau
> attachment sessions, exact topology presentation, atomic real-hub movement,
> protected VBUS, and `ColdMove`. Virtual host ports are prototype-only.

> **Controller decision:** one durable authoritative controller owns desired
> state in the first supported mesh. High-availability replication, consensus,
> and automatic failover are deferred. Controller loss fails closed.

Status: architecture guidance  
Research date: 2026-07-27  
Scope: dynamic, exclusive USB routes across a scalable endpoint mesh

## Decision

Model the matrix as a mesh of independently addressable source ports and
destination ports. A source port terminates one physical USB device. A
destination port presents one imported device to one host. Either endpoint may
join, leave, move, or change capability without changing the route model.

The controller stores desired assignments. Endpoint agents report observed
state and execute fenced actions. A deterministic reconciler repeatedly turns
the difference into detach, reset, attach, enumerate, or fault work:

```text
desired routes + endpoint inventory + observed routes
    -> validate and order
        -> detach old destination
            -> prove unassigned
                -> attach new destination
                    -> prove enumeration
```

The data plane remains point-to-point for each active route even though the
control plane is a mesh. One source port has at most one destination port, and
one destination port has at most one source port. General USB does not permit
one physical device to be active on several hosts.

## Names and identities

Use stable identities independent of network addresses:

| Object | Stable key | Mutable observations |
|---|---|---|
| Endpoint node | controller-issued `NodeId` | address, boot ID, health, software version |
| Source port | `NodeId` plus `SourcePortId` | device presence, descriptors, speed, power |
| Destination port | `NodeId` plus `DestinationPortId` | host presence, virtual port, speed |
| Route | controller-issued `RouteId` | desired pair, state, epoch, deadlines |
| Endpoint process | node ID plus random `BootId` | session, heartbeat, last report |

A cable move changes the observed physical inventory, not a source identity.
A replacement endpoint receives a new node identity. Network addresses,
USB bus IDs, virtual host-controller port numbers, VID/PID, and serial strings
are observations; none is an authority key.

Each report carries `NodeId`, `BootId`, a monotonically increasing report
sequence, and the controller revision it has observed. A new boot ID invalidates
commands issued to an earlier endpoint process.

## Desired and observed state

The authoritative desired snapshot contains:

- known nodes and administratively enabled ports;
- requested source-to-destination assignments;
- route priority, request sequence, and policy identity;
- the current per-source device ownership epoch;
- maintenance, quarantine, and drain intent.

Each endpoint independently reports:

- its boot ID, capabilities, and health;
- physical device and host presence;
- locally bound, exported, imported, enumerating, and active routes;
- the epoch installed for each local port;
- pending operations and their deadlines;
- protected VBUS and link evidence where available.

Observed state is evidence, never authority. A route is `Active` only when the
source and destination report the same source, destination, epoch, and active
state. A one-sided attachment is `Degraded`; it must not be rendered as active.
Absence of a report means `Unknown`, not detached.

The reconciler is a pure function over a versioned snapshot:

```text
reconcile(desiredRevision, desired, observed, logicalTick)
    -> ordered action batch + next logical state + audit events
```

The same snapshot and logical tick produce byte-identical actions and audit
events. Wall time, network arrival order, hash-map iteration, and worker count
must not affect the result.

## Ownership and fencing

One durable controller owns a source route at a time. Its persistence must
survive restart without reusing an epoch. A temporary JSON ledger, process
lock, heartbeat, or last-writer-wins database is insufficient. Future HA may
shard sources behind a consensus-backed authority, but that is deferred.

Every route mutation advances a non-wrapping source epoch. Every command carries:

```text
authority term, desired revision, route ID, source port, destination port,
source epoch, endpoint boot ID, operation ID
```

Endpoint agents persist the highest accepted authority term and source epoch
before mutating USB state. They reject:

- an older authority term;
- an older or equal epoch for a new operation;
- a command for another boot ID;
- a conflicting operation ID at the same epoch;
- an attach without a detach proof for the immediately preceding ownership.

Retries reuse the same operation ID and return the prior result. They do not
allocate another epoch. Epoch exhaustion is a terminal administrative fault;
epochs never wrap or decrement.

A signed or mutually authenticated command proves origin. The fence proves
freshness. Both are required.

## Reconciliation states

Each route has one externally visible state:

```text
Unassigned
Requested
WaitingForSource
WaitingForDestination
Detaching
ProvingDetached
Attaching
Enumerating
Active
Degraded
Fault
```

Route state alone does not prove local USB state. The audit record retains the
last source and destination observations that justified each transition.

For a new assignment:

1. validate identities, capability, authorization, health, and capacity;
2. allocate the next source epoch and persist the desired revision;
3. command both endpoints to remove stale local state for that source;
4. wait for matching detached observations from every previously involved
   destination and the source exporter;
5. enable or bind the source export when required;
6. attach at the selected destination;
7. wait for matching import and enumeration evidence;
8. declare active only after both endpoints agree on the fenced epoch.

For reassignment, steps three and four always precede attachment to the new
destination.

## Why switching is break-before-make

For product `ColdMove`, confirmed old-computer disconnect precedes Pau VBUS
removal. Measured discharge precedes the durable epoch advance. Protected
repower and a new exact-topology observation precede the new Cau plug event.
No timeout substitutes for voltage evidence. Externally powered topology is
reset and re-enumerated from discarded prior observation. Any ambiguous step
converges to powered-off/fenced fault, never rollback to an implicit live
session.

General USB migration cannot be make-before-break. The old host schedules the
device and owns its topology and software state. Allowing a second host to
enumerate before the first host is detached creates concurrent control requests,
ambiguous endpoint state, duplicate writes, and possible storage corruption.

The reconciler therefore uses detach-before-attach and exposes a real disconnect
to both host operating systems. A route may reduce downtime by pre-authenticating
the new endpoint, reserving bandwidth, loading policy, and warming transport
connections. It may not present the physical device to the new host before
detachment is proved.

A purpose-built application proxy may replicate application state for one
documented USB class. That is a separate component with class-specific
semantics, not transparent USB migration.

## Work scheduling

The scheduler has one serialized lane per source port. Independent source lanes
may execute concurrently when their destination ports and endpoint capacity do
not conflict. A destination port also admits at most one in-flight route.

Requests enter bounded queues. Ordering is deterministic:

1. recovery and detach work;
2. operator-requested release;
3. approved assignment by descending policy priority;
4. oldest request sequence;
5. ascending route ID as the final tie-break.

Priority is a small configured domain, not an arbitrary client number.
Repeated requests from one tenant cannot reset their age. A bounded
weighted-deficit scheduler across policy groups prevents a busy group from
starving another. Every scheduling decision records the winning priority,
age, deficit, and tie-break.

The controller rejects work when a queue is full. It does not discard an older
accepted request. Per-node and per-tenant admission limits bound memory and
reconciliation cost. Cancellation is itself a fenced desired-state revision;
it cannot erase an already required detach.

For scale:

- shard work by source ID;
- keep destination capacity reservations separate from per-source ownership,
  but update both atomically inside the durable controller;
- index dirty routes and reconcile only affected dependencies;
- use bounded batches and explicit continuation cursors;
- cap concurrent attach, enumerate, and reset operations per endpoint;
- reserve data-plane bandwidth before attach when policy requires it.

Changing the number of workers changes latency, not action order or decisions.

## Dynamic endpoint reconfiguration

Endpoint configuration is desired state with its own revision. Ports can be
enabled, disabled, quarantined, drained, reassigned to another policy group, or
updated with capability and bandwidth limits while the mesh is running.

Use these transitions:

- **join:** authenticate the node, assign stable IDs, inventory ports, quarantine
  unknown devices, then make eligible ports schedulable;
- **drain:** reject new assignments, detach existing routes in scheduler order,
  and wait for detached evidence before maintenance;
- **disable:** persist disabled intent, fence every active route, detach, then
  leave the port unavailable;
- **address change:** preserve node identity and boot ID; reconnect without
  changing route identity;
- **agent restart:** accept the new boot ID, mark its observations unknown,
  fence prior commands, and reconcile from fresh inventory;
- **port replacement:** create a new port identity and require policy approval;
  do not inherit routes from connector position alone;
- **capability change:** revalidate affected desired routes and detach any route
  that no longer meets policy.

Bulk reconfiguration is a versioned transaction. Validation covers the complete
proposed desired graph before publication. Execution remains incremental and
observable; a desired revision does not imply that all routes changed atomically.

## Deadlines and recovery

Every command has a logical deadline and a bounded retry policy. Retries are
idempotent. On exhaustion, the route becomes `Fault` or `Degraded`; it never
advances optimistically.

| Failure | Required recovery |
|---|---|
| Source disappears | Fence route, mark ownership unknown, forbid new attach |
| Destination disappears | Fence route, require source-side detach/reset proof |
| Controller restarts | Recover committed desired state, advance authority term, treat all observed state as unknown |
| Agent restarts | Reject old-boot commands, inventory before reconciliation |
| Network partition | Endpoints stop accepting mutations without the authoritative controller and keep rejecting stale terms |
| Attach succeeds but reply is lost | Retry same operation ID; endpoint returns persisted result |
| Detach is unconfirmed | Keep source unavailable; never attach elsewhere |
| Enumeration fails | Detach destination, preserve fault evidence, leave unassigned |
| Conflicting reports | Enter degraded state, advance fence, command cleanup |
| Audit persistence fails | Stop mutation before changing desired state |
| Data-plane congestion | Apply admission policy; do not hide loss as an active healthy route |

Recovery prefers an honestly unavailable device over ambiguous dual ownership.
Automatic restoration is allowed only from current fenced observations and the
current desired revision. It never reconstructs ownership from a stale active
snapshot.

Storage devices receive longer drain policy and explicit operator warnings.
The controller cannot guarantee filesystem integrity after host, endpoint, or
network failure.

## Audit and observation

Persist an append-only event for every desired-state edit, scheduling decision,
command, acknowledgement, observation change, timeout, rejection, and recovery.
Include authority term, desired revision, route and endpoint identities, source
epoch, operation ID, logical tick, reason, and previous-record digest. A hash
chain detects accidental modification; deployment needs authenticated durable
storage for tamper evidence.

Expose four independent observations:

- desired route;
- source-local state and epoch;
- destination-local state and epoch;
- data-plane health and protected power.

The operator panel may show requested, moving, active, degraded, and fault
states. Green requires source and destination agreement at one epoch. Named
link, VBUS, and heartbeat test points remain separate evidence. Serial and
network logs are supporting evidence only.

## Deterministic proof plan

The host model must replay:

- simultaneous route requests in every arrival permutation;
- fair progress under a continuously busy high-priority tenant;
- queue capacity and cancellation boundaries;
- endpoint join, drain, disable, restart, and replacement;
- stale authority terms, epochs, boot IDs, and operation IDs;
- detach acknowledgement lost before reassignment;
- attach success with lost acknowledgement;
- controller restart before and after every durable transition;
- partitions, duplicate reports, reordered reports, and report gaps;
- capability and policy changes during an active route;
- tick and sequence boundaries, including explicit exhaustion behavior;
- identical traces with one worker and many workers.

Properties:

- no source or destination is active in two routes;
- attach is never emitted before matching detach proof;
- stale evidence cannot create `Active`;
- every accepted request completes, is explicitly rejected, is cancelled, or
  reaches a recorded fault;
- equal inputs produce equal actions and audit records;
- restart never converts unknown ownership into active ownership.

Physical USB/IP compatibility, transfer integrity, throughput, tail latency,
power behavior, and device-class behavior remain separate bench evidence.

## Delivery boundaries

1. Extend the host-only controller to store stable source and destination IDs,
   desired revisions, boot IDs, operation IDs, and independent observations.
2. Add the pure reconciler and deterministic bounded scheduler.
3. Add durable single-authority persistence and idempotent endpoint-agent
   protocol.
4. Add a simulated multi-node agent mesh with partitions and restarts.
5. Connect Linux USB/IP only after the model gates pass.
6. Add consensus-backed sharding only after one authoritative controller is
   correct and measured.

The current Python ledger remains a single-operator experiment. It is not a
mesh authority and must not independently allocate ownership epochs.

## Open design decisions

These choices do not block the deterministic host model:

- the maximum source, destination, route, and queue capacities;
- the first permitted USB device classes and prototype hub limitations;
- per-class detach deadlines and storage drain policy;
- the bandwidth-reservation policy for interrupt and isochronous traffic;
- the authenticated endpoint protocol and durable audit backend.

The first USB/IP adapter may reject hubs while its subtree handling is
incomplete. The product reconciler must accept a user-provided real hub only as
one indivisible `TopologyRoot`; it never rejects hubs as product policy,
flattens them, or routes descendants independently.
