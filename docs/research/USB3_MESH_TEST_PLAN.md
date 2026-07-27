# USB 3 mesh test plan

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Final acceptance requires an unmodified Windows and Linux
> computer, physical Cau/Pau units, exact topology reconstruction, atomic real
> hub subtrees, `ColdMove`, and operation on the shared switched network.

> **Controller decision:** current acceptance assumes one durable authoritative
> controller. Replication, quorum, leader election, and automatic controller
> failover scenarios are deferred. Controller loss still fails closed.

> **Prototype decision:** Linux USB/IP tests exercise only prototype control
> and measurement behavior. They are never transparent-product conformance.

Status: research test contract  
Scope: dynamically reconfigurable source and destination endpoint mesh  
Physical evidence: deferred

## Purpose

The mesh connects any authorized source-facing USB endpoint to any compatible
destination-facing endpoint through ordinary switched IP networking. Endpoints
may join, leave, move, restart, or change role while the service runs. Routing
remains exclusive: one physical USB device has at most one importing host, and
one host port has at most one attached device. Reconfiguration produces an
observable USB detach and enumeration; it is not seamless migration.

The test oracle is the authoritative route model, not USB/IP output, discovery
caches, endpoint LEDs, or the temporary adapter ledger. A route is active only
when the controller and both endpoint agents agree on route identity, epoch,
topology generation, and session identity.

## Required identities

Every message and durable record carries:

- stable controller, source endpoint, destination endpoint, and physical-port
  identities;
- a controller incarnation and monotonically increasing per-source-device
  ownership epoch;
- a separate destination-capacity reservation;
- a topology generation covering endpoint membership and capabilities;
- a fresh authenticated session identity for each endpoint connection;
- command identity, logical tick, deadline, and protocol version.

An endpoint name, address, USB bus number, or VID/PID is not an identity.
Reusing an IP address, process ID, bus ID, or display name must not revive an
old route.

## Reference model

Use a small, independent model whose state is a partial bijection between
source ports and destination ports. Model endpoints separately from ports so
one appliance can expose many ports and endpoints can be replaced without
changing historical identity.

Each route has one of:

```text
unassigned
reserved
detaching
attaching
enumerating
active
fault
```

The model permits a transition only when:

1. both endpoint sessions are current and authorized;
2. their advertised capabilities are compatible;
3. neither port belongs to another live route;
4. every predecessor route is confirmed detached or fenced;
5. the command term, epoch, topology generation, and deadline are current.

A failed or ambiguous transition converges to `unassigned` or `fault`.
It never restores a prior route implicitly. Reconciliation may discover an
attachment, but only an explicit new transaction may authorize it.

## Observable invariants

Check these after every generated action and delivery:

- no source port or destination port belongs to two routes;
- no device is exposed to two hosts, including during reassignment;
- `active` implies matching confirmation from both current endpoint sessions;
- an old term, epoch, topology generation, or session cannot change state;
- detach is authorized before attach when either port was previously assigned;
- a route never becomes active after its deadline;
- membership loss removes or faults every affected route;
- restart never asserts a pre-restart route as active;
- capability changes invalidate incompatible pending and active routes;
- rejected operations do not mutate routes, epochs, or the audit chain;
- accepted mutations are durably recorded before their success is exposed;
- audit sequence and hashes remain continuous across accepted operations;
- duplicate delivery is idempotent and produces no second hardware action;
- replay depends only on configuration, seed, initial snapshot, and trace;
- bounded queues, tables, epochs, and audit capacity fail closed;
- transport, discovery, persistence, and observability disagree visibly rather
  than being collapsed into a false green state.

## Deterministic harness

Use a manually advanced unsigned logical clock. The harness contains the real
controller, fake endpoint agents, fake durable storage, fake authenticated
transport, and an independent reference model. It must not sleep, read wall
time, open a network socket, enumerate host USB, or depend on thread scheduling.

The scheduler selects one enabled event at a time:

- operator route, release, quarantine, or membership command;
- endpoint join, leave, reconnect, restart, or capability update;
- command delivery, acknowledgement, duplication, delay, reordering, or loss;
- detach, attach, enumeration, or payload completion;
- timer boundary, persistence result, or controller restart.

Equal-tick events preserve recorded trace order. Each failure prints the seed,
smallest useful prefix, initial snapshot, and a versioned replay trace.
Run every checked-in trace twice and require byte-identical public snapshots,
endpoint actions, persistence writes, and audit entries.

## Example tests

### Identity and membership

- Join one source and one destination, route them, release them, and rejoin
  each under a fresh session.
- Reconnect with the same address but a different stable identity.
- Replace an endpoint while retaining its display name.
- Add and remove one port without disturbing unrelated routes.
- Reject duplicate stable identities presented by simultaneous sessions.
- Reject stale discovery after a topology generation change.
- Change endpoint role and require affected routes to detach before reuse.
- Downgrade speed, transfer-type, power, or policy capability while reserved,
  attaching, enumerating, and active.

### Dynamic route changes

- Assign every compatible source/destination pair from an empty mesh.
- Move one destination between sources and one source between destinations.
- Swap two active routes without an interval of dual ownership.
- Rotate `N` routes through `N` endpoints with deterministic detach order.
- Issue a new request while detach, attach, or enumeration is pending.
- Request the already active route and prove idempotence.
- Release an unassigned route and prove no epoch or hardware action changes.
- Cancel at every transition boundary and immediately before its deadline.
- Quarantine an endpoint and prove no ordinary route can include it.
- Re-enable a quarantined endpoint only through a new authorized generation.

### Messages and fencing

For every request and confirmation, test duplicate, delayed, dropped, reordered,
corrupted, wrong-version, wrong-session, wrong-term, wrong-epoch, wrong-route,
wrong-port, wrong-generation, and post-deadline delivery. Test a stale detach
confirmation arriving during a newer attach and a stale attach confirmation
arriving after release. Neither may affect the current transaction.

Deliver an old message after:

- endpoint reconnect;
- controller restart;
- term change;
- endpoint rename or address reuse;
- port removal and recreation;
- device ownership epoch advancement;
- topology generation advancement.

### Failure and recovery

- Lose either endpoint at every route state.
- Restart either endpoint before and after performing a hardware action but
  before its acknowledgement.
- Restart the controller before and after each durable write.
- Partition controller-to-source, controller-to-destination, source-to-data
  plane, and management discovery independently.
- Recover partitions in every order, including after route deadlines.
- Fail detach, attach, enumeration, authorization, persistence, and audit
  append independently.
- Exhaust endpoint, port, route, pending-command, audit, and transport queues.
- Reach the maximum device ownership epoch and refuse reuse or wraparound.
- Advance time immediately before wrap, at wrap, and after wrap.
- Load truncated, malformed, duplicate, incompatible-version, or internally
  inconsistent snapshots transactionally.
- Report an unexpected physical attachment and require quarantine or explicit
  detach, never adoption as an active route.

### Concurrency schedules

Systematically enumerate all event interleavings for two sources, two
destinations, and two competing operators up to a bounded depth. Include:

- two assignments targeting the same source;
- two assignments targeting the same destination;
- assignment racing release;
- assignment racing endpoint removal;
- endpoint reconnect racing an old confirmation;
- timeout racing successful confirmation;
- persistence failure racing endpoint success;
- controller restart between command and acknowledgement;
- stale pre-restart controller terms issuing commands to one endpoint.

The model accepts at most one winner. Losing commands return a stable reason and
cannot perform a late hardware action. Randomized longer schedules supplement,
but do not replace, bounded exhaustive exploration.

## Model and property tests

Generate endpoint inventories, compatibility matrices, policies, initial
snapshots, and event traces from fixed seeds. Bias generation toward the same
port, simultaneous ticks, deadlines, capacity limits, reconnects, and stale
identifiers.

After every step compare implementation and model:

- route table and endpoint membership;
- required next action and deadline;
- accepted or rejected status;
- audit action and generation;
- fake endpoint command log;
- persisted snapshot visibility.

Useful metamorphic properties:

- renaming display labels does not alter routing;
- permuting unrelated endpoint identifiers only permutes corresponding output;
- inserting duplicate messages does not alter final state;
- inserting a stale message does not alter current state;
- delaying an event within its valid interval preserves the route result;
- reordering independent routes preserves each route's result;
- serialization followed by restart preserves logical state while converting
  ambiguous active work to fault or unassigned;
- adding an unused endpoint does not change existing routes;
- removing one disconnected endpoint does not change existing routes.

Use fixed short runs in normal checks and longer opt-in seed ranges. Reduce a
failure to a checked-in regression trace before fixing it.

## Scale evidence

Test powers of two and every boundary around configured capacities. At minimum:

| Topology | Purpose |
|---|---|
| 1 × 1 | lifecycle and fault baseline |
| 2 × 2 | conflicting and swapping schedules |
| 8 × 8 | small managed switch and panel model |
| 32 × 32 | multi-appliance topology |
| configured maximum − 1, maximum, maximum + 1 | capacity behavior |

For each scale, measure deterministic operation counts, memory use, snapshot
size, audit growth, and reconciliation work. Tests must assert configured
bounds, not host elapsed time. A full topology rescan must not silently reorder
routes or change epochs. Churn tests repeatedly replace one percent, half, and
all endpoints while unrelated routes remain stable.

Network performance belongs to later physical evidence. Host scale tests prove
control-plane bounds and correctness, not USB 3 throughput.

## Adapter conformance

Run the Python USB/IP adapter against a fake command runner and a fake
authoritative controller. Prove:

- the adapter never invents an epoch, route, or endpoint identity;
- every mutation has a stable dry-run representation;
- arguments are passed without a shell or implicit privilege escalation;
- detach precedes attach and all commands have deadlines;
- a successful OS command followed by failed persistence becomes an explicit
  ambiguous fault;
- reconciliation compares controller, both endpoint agents, `usbip port`, and
  physical presence without declaring any single surface authoritative;
- discovery output cannot mutate the route table;
- concurrent adapter processes are rejected by the single-writer lease.

No host test loads a kernel module, invokes `sudo`, binds a real device, or
touches a real USB port.

## Physical evidence deferred

Physical testing begins only after the model, controller, persistence, adapter,
and security gates pass. Record it separately; never convert a skipped check
into a pass.

Later evidence includes:

- endpoint discovery, reconnect, power loss, cable loss, and switch-port move;
- dynamic reroute and controller restart under controlled bulk transfers;
- generated payload hashes before and after detach;
- enumeration, reset, cancellation, latency, jitter, throughput, and loss by
  USB transfer type;
- simultaneous traffic at each supported topology scale;
- protected VBUS, overcurrent, and power-state observations independent of
  logical route state;
- endpoint and host agreement on the same device ownership epoch and session;
- compatibility records for each device, host OS, kernel, controller, switch,
  VLAN, MTU, link rate, and repository revision.

Start with disposable, non-sensitive bulk test media on an isolated wired lab
network. Keyboards, cameras, personal storage, security tokens, Internet
exposure, and production hosts remain excluded from initial evidence. The Mega
may display requested, pending, active, and fault states, but it neither carries
USB data nor authorizes a route.

## Acceptance gate

The mesh control plane is host verified only when:

1. unit, reference-model, property, bounded-schedule, replay, persistence,
   adapter, and capacity tests pass under ordinary and sanitizer builds;
2. every failure is replayable from a checked-in or printed versioned trace;
3. all invariants hold after every event, not only at final state;
4. restart and reconciliation fail closed under ambiguous ownership;
5. the CLI and documentation state that physical USB compatibility,
   throughput, recovery timing, and electrical behavior remain unverified.

It becomes physically verified only for the exact recorded topology and device
corpus that later passes the hardware evidence. That result does not imply
universal USB 3 compatibility.
