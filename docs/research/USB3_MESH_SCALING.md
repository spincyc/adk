# USB 3 dynamically routed mesh

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Scale by adding one-port Caus and four-port Paus on the
> shared switched network. Each Pau port owns one complete topology root; a
> user-provided real hub subtree is indivisible.

> **Controller decision:** scale the first supported system under one durable
> authoritative controller. Controller replication, consensus, leader
> election, and automatic HA failover are deferred research.

Status: architecture guidance  
Research date: 2026-07-27  
Scope: independently reconfigurable host-side and peripheral-side edge
appliances on a scalable switched IP fabric

## Decision

Build the matrix as a mesh of interchangeable edge appliances, not as fixed
source/destination pairs. Every physical USB attachment is a named port
resource. A route binds one host-facing port to one peripheral-facing port for
the lifetime of an exclusive, ownership-epoch-fenced lease. Either edge may be
replaced, moved, drained, or failed over without renumbering the other.

```text
host-side edges                 routed fabric          peripheral-side edges

H1 ─┐                                               ┌─ D1
H2 ─┼─ authenticated tunnels ── leaf/spine IP ─────┼─ D2
H3 ─┘                                               └─ D3

directory: stable port identity -> current edge node, health, capabilities
lease:     host port H2 <-> peripheral port D1, ownership epoch 84, exclusive
```

The terms describe USB roles, not traffic direction. USB requests usually
travel from the host-side edge to the peripheral-side edge; completions and
device data travel back. Both sides are dynamically selectable.

The Mega 2560 remains an optional control and observation panel. Linux, an
SoC, or purpose-built endpoint hardware terminates USB and carries the data
plane.

## Objects and identities

Keep location, hardware identity, and assignment separate:

| Object | Stable key | Mutable data |
|---|---|---|
| Edge node | provisioned node ID | addresses, software version, load, health |
| Physical port | provisioned port ID | node placement, USB role, link state |
| Peripheral | observed identity plus port history | descriptors, class, presence |
| Host attachment | provisioned host-port ID | virtual controller, OS session |
| Route | route ID and ownership epoch | two port IDs, state, deadlines, policy |
| Session | random authenticated ID | transport paths, counters, negotiated features |

Do not authorize by network address, USB bus number, or VID/PID alone. Those
values change or are easy to reproduce. A moved appliance retains its
provisioned identity and advertises a new location. A replaced appliance gets
a new node identity; an administrator explicitly transfers its port labels.

The directory is soft state derived from authenticated edge heartbeats. The
lease store is durable authority. Discovery never creates ownership.

## Reconfiguration transaction

Changing either endpoint is the same fenced transaction:

1. validate identity, compatibility, policy, capacity, and expected ownership epoch;
2. reserve bandwidth and resources on both selected edges;
3. fence the current route and stop admission of new USB requests;
4. drain or cancel outstanding requests before the deadline;
5. confirm detachment at the old host-side and peripheral-side sessions;
6. advance the device ownership epoch;
7. create a fresh authenticated session between the selected edges;
8. expose a fresh connect event and allow ordinary USB enumeration;
9. commit the active lease only after both edges report the same ownership epoch;
10. release old reservations and publish the audit record.

Any missing or contradictory confirmation leaves the route disconnected or in
an explicit fault state. A later message from an older ownership epoch cannot
restore it. General USB devices do not support seamless live migration:
reconfiguration is observable as disconnect and reconnect, and volatile device
state may be lost.

Moving both ends uses one transaction, not two independently visible moves.
The controller reserves the new pair before fencing the old pair, but it never
asserts two active owners for the peripheral.

## Controller models

### One controller

A single durable controller with one active writer is the phase-one
implementation. It can prove lease semantics for tens or low hundreds of
ports. Warm standby, external leadership fencing, and automatic promotion are
not phase-one features.

Use this through the USB/IP prototype. It supplies the reference behavior for
every distributed implementation.

### Replicated controller (deferred HA research)

A three- or five-member consensus group removes the controller as a single
availability fault. Put route intent, ownership epochs, policy revisions, and
edge membership in the replicated log. Do not put USB transfers, telemetry
samples, or every heartbeat in it.

Consensus is needed when separate controller processes can grant the same
peripheral. It is not needed inside the data path. A majority partition may
grant new leases; a minority partition becomes read-only and edges there let
their bounded leases expire. Prefer disconnection to split ownership.

Three voters tolerate one voter failure; five tolerate two, at the cost of
more write latency and operations. Place voters in independent power and
switch failure domains. Two voters are unsafe for useful automatic failover.

### Sharded controller (deferred scaling research)

At larger scale, assign each peripheral-port ID to exactly one lease shard by
consistent hashing or a durable directory range. The peripheral is the scarce
exclusive resource, so it is the natural ownership key. Host capacity is a
separate reservation checked during admission.

Each shard has its own three-member consensus group. A route normally commits
in one lease shard plus reservations at two edges. Avoid cross-shard
transactions in the first design. Moving a peripheral between shards requires
a maintenance workflow: drain, fence, transfer a higher ownership-epoch floor,
and activate the new shard.

Sharding improves write throughput and limits fault blast radius. It does not
increase data-plane bandwidth. A global directory may be eventually
consistent because it only locates the authoritative shard; clients retry
against the shard named by a redirect containing a directory revision.

## Scale envelope

Use measured service times to replace these planning assumptions:

| Stage | Ports | Concurrent routes | Control model | Suggested fabric |
|---|---:|---:|---|---|
| Lab | 4--16 | 2--8 | one controller | isolated 10/25 GbE |
| Rack | 32--256 | 16--128 | three voters | redundant 25/100 GbE |
| Room | 256--2,048 | 128--1,024 | lease shards, three voters each | leaf/spine 100 GbE uplinks |
| Larger | over 2,048 | measured only | regional shards and explicit quotas | engineered fabric |

Port count is primarily directory and policy state; active routes consume
transport sockets, kernel queues, USB controller slots, memory, and bandwidth.
Publish separate limits for registered ports, present peripherals, pending
transactions, active routes, and routes per edge.

One USB 3.2 Gen 1 link signals at 5 Gb/s but useful tunneled throughput is
lower. Budget a serious continuously busy Gen 1 route for a 10 GbE edge link,
then oversubscribe only from measured workloads. Gen 2 and Gen 2x2 require
faster edge and fabric links; a nominally equal Ethernet line rate provides no
headroom for Ethernet, IP, security, and tunnel overhead.

Admission uses:

```text
reserved rate + new route peak <= engineered link budget
```

It checks both access links and every fabric contention point. Maintain
separate queues and budgets for control, bulk, interrupt, and isochronous
traffic. Never infer isochronous support from aggregate throughput.

## Topology and transport

Use a redundant leaf/spine network once traffic exceeds one switch. Dual-home
each production edge when its hardware and transport can preserve session
identity across paths. Multiple paths improve link resilience but must not
duplicate or reorder USB operations beyond the tunnel's rules.

Keep three planes distinct:

- management: provisioning, certificates, software updates, policy;
- control: discovery, leases, route state, audit events;
- data: USB request and completion streams.

The planes may share physical links in the lab but use separate authenticated
services, queues, and VLANs or equivalent policy boundaries. Never expose
USB/IP or an endpoint listener directly to the Internet.

USB/IP over TCP is a prototype measurement baseline only. Product transports
must reconstruct physical USB transparently for unmodified Windows and Linux
computers. Later transports may use
multiple ordered streams or datagrams for bounded classes, but their first gate
is byte-for-byte semantic replay against the reference adapter. Transport
selection cannot weaken cancellation, reset, ordering, or ownership-epoch fencing.

## Latency and switching objectives

Measure distributions, not averages:

| Measurement | Required dimensions |
|---|---|
| Control request | authorize, persist, dispatch, acknowledge |
| Route switch | drain, detach, session setup, enumeration, ready |
| Data path | median, p95, p99, p99.9, maximum during test |
| Jitter | idle and congested fabric, per transfer type |
| Recovery | edge restart, controller restart, link loss, switch failure |

Enumeration commonly dominates route switching and varies by device and host
software. State objectives per supported device class; do not promise a
universal cutover time. A lease deadline must exceed the measured worst valid
transaction yet remain bounded. Clocks aid measurement, but correctness uses
ownership epochs and controller authority rather than synchronized wall time.

## Failure domains

| Failure | Required result |
|---|---|
| One data link | use an already validated alternate path or disconnect |
| One edge | fence its sessions; affected routes disconnect |
| One leaf switch | contain loss to attached edges; no duplicate lease |
| Controller restarts | pause writes, restore durable epochs, reconcile before mutation |
| Controller is unreachable | no new leases; bounded leases expire |
| Directory outage | existing routes may continue; no discovery-based grant |
| Audit sink outage | fail route changes if durable audit is policy-required |
| Edge restart | restore identity, never restore active USB ownership from cache |
| Stale command or result | reject by route ID, ownership epoch, and session ID |
| Peripheral unplug | revoke route, record physical absence, require re-enumeration |
| Congestion | enforce reservations; expose degraded state before queue exhaustion |

Edges hold short, renewable lease evidence signed or authenticated by the
current controller authority. Loss of control connectivity has an explicit
policy: existing routes either continue for a bounded grace period or detach.
It never permits reassignment elsewhere while an isolated old edge can remain
indefinitely active.

Power control is a separate fenced resource. No controller transition may join
two VBUS sources. Data-route failover does not imply a device power cycle.

## Observability

Every event carries `routeId`, `epoch`, `sessionId`, both stable port IDs,
controller incarnation, policy revision, and a monotonic edge-local sequence.
Record:

- route state and time in each transition;
- request/completion bytes and operations by transfer type;
- queue depth, cancellation, late completion, retry, loss, and reorder counts;
- median and tail latency plus isochronous late/underrun counts;
- negotiated USB speed, resets, enumeration result, and disconnect reason;
- edge CPU, memory, temperature, link utilization, and VBUS telemetry;
- controller incarnation, durable-store health, directory revision, and
  lease-renewal margin;
- authorization decisions and an authenticated append-only route audit.

High-cardinality IDs belong in traces and bounded event records, not every
metrics label. Sample successful data-plane traces but never sample route,
authorization, reset, power, or fault events.

The physical observation path remains independent of logs:

- each port shows present, reserved, active, and fault;
- both selected edges show the same short route and ownership epoch indicator;
- a protected-power indicator reflects measured VBUS, not requested power;
- named heartbeat and load-switch test points distinguish controller state
  from electrical state.

A route is green only when the controller and both edges report the same active
ownership epoch. Contradiction is red and disconnected.

## Deployment sequence

1. Prove one controller, two host-side edges, and two peripheral-side edges.
2. Make either side dynamically selectable and test every pair.
3. Add failure injection, bounded leases, and durable authenticated audit.
4. Add admission control and a nonblocking leaf/spine test under load.
5. Measure the single controller before considering deferred HA.
6. If justified later, add three-voter authority and then consider sharding by
   source-device identity.
7. Add a second supported USB class or speed only with its own evidence matrix.

The compatibility ledger is per host OS, controller, peripheral, USB speed,
transfer type, topology, load, and failure case. “Mesh supported” means the
route-control model scales; it does not make every USB device compatible.

## Open design decisions

- Whether an isolated active route detaches immediately or receives a short
  bounded grace period.
- Which initial device classes are allowed beyond bulk storage and HID.
- Whether rack-scale edges need dual data-plane links in the first hardware
  revision.
- The measured threshold at which lease sharding is justified.
- Whether device-facing ports may be reassigned between shards during normal
  service or only during maintenance.

## Related guidance

- [USB 3 network matrix](USB3_NETWORK_MATRIX.md)
- [USB 3 matrix lab CLI](USB3_MATRIX_CLI.md)
- [Network matrix constraints](../NETWORK_MATRIX_CONSTRAINTS.md)
- [Linux USB/IP protocol](https://docs.kernel.org/usb/usbip_protocol.html)
- [Linux USB/IP tools](https://docs.kernel.org/usb/usbip_tools.html)
