# Dynamic USB 3 mesh architecture

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. The final path is an unmodified Windows or Linux computer
> through a physical `ComputerAttachmentUnit` (`Cau`), the shared switched
> network, and a physical `PeripheralAttachmentUnit` (`Pau`). Linux USB/IP and
> virtual-host-controller endpoints below are prototype-only research.

> **Controller decision:** the first supported mesh has one durable,
> authoritative controller. Controller replication, consensus, automatic
> leader election, and high-availability failover are deferred research.
> Endpoints fail closed when that authority is unavailable.

> **Prototype decision:** an early Linux USB/IP experiment may validate control
> behavior, but it is not product conformance.

Status: research architecture  
Research date: 2026-07-27  
Scope: dynamically reconfigurable host-facing and device-facing USB endpoints
over a switched Ethernet/IP fabric

## Decision

The product model has fixed roles. One Cau contains one Type-B `ComputerPort`;
one Pau contains four independently powered Type-A `PeripheralPort` roots.
One route binds one complete real topology root to one computer port. A real
hub subtree is atomic. Product appliances never change role, synthesize a hub,
combine roots, or expose a virtual host port. The generic node, source,
destination, and virtual-host vocabulary below describes the USB/IP prototype
only.

Treat every USB-facing port as an independently addressable endpoint. A node
may expose host-facing ports, device-facing ports, or both. An authorized
operator can assign any eligible device port to any eligible host port without
rewiring the network. The resulting system is a scalable assignment mesh, not
a shared USB bus and not a raw SuperSpeed switch.

The data plane terminates USB at both ends and carries USB transactions over
authenticated point-to-point network sessions. It normally flows directly
between the two endpoint nodes. The control plane discovers endpoints,
authorizes assignments, grants exclusive leases, fences stale participants,
and records every transition. Adding ports or nodes must not place their
payload through one central controller.

The Linux USB/IP prototype remains useful, but its virtual host, temporary
ledger, and single-operator assumptions are not product architecture or mesh
authority. The product routes one complete physical `TopologyRoot` from one
Pau `PeripheralPort` to one Cau `ComputerPort`. A user-provided real hub and
all descendants move atomically. `ColdMove` power-cycles Pau-supplied VBUS and
appears to the computer as unplug followed by fresh enumeration.

## Terms

| Term | Meaning |
|---|---|
| Node | A managed Linux, SoC, or later FPGA/SoC appliance attached to the IP fabric |
| Host port | A logical destination that imports a remote USB device into one host |
| Device port | A logical source backed by one physical USB device or an explicitly supported subtree |
| Endpoint | A host port or device port plus its stable identity, capabilities, and health |
| Route | The desired association between one device port and one host port |
| Lease | The exclusive, time-bounded authority to make one route active |
| Epoch | A monotonically increasing ownership fence for one source device |
| Session | The authenticated data-plane connection for one active lease and epoch |
| Coordinator | The control-plane service that commits route ownership |
| Adapter | Node-local code that binds, detaches, imports, exports, and reports USB state |
| Inventory | The observed nodes, ports, devices, capabilities, topology, and health |

“Source” means the device-facing endpoint that supplies a peripheral.
“Destination” means the host-facing endpoint that receives that peripheral.
These terms describe a matrix route; they do not change USB's host-scheduled
protocol direction.

## Topology

Endpoint nodes connect through an ordinary managed IP network:

```text
 device node A                                  host node X
  device port A1 ----\                       /---- host port X1
  device port A2 -----\                     /----- host port X2
                       \                   /
                        switched IP fabric
                       /                   \
  device port B1 -----/                     \----- host port Y1
  host port B2   ----/                       \----- device port Y2
 node B may carry both roles                      mixed-role node Y
```

Each route creates one direct logical session:

```text
device port A2 -- USB termination -- network session -- virtual host port Y1
```

No payload passes through the route coordinator unless a deliberately selected
relay is required by policy or reachability. A relay is a measured exception,
not the default topology. Multiple independent routes may run concurrently,
subject to endpoint, link, queue, and switch capacity.

The initial ownership rule remains one-to-one:

- one device port has zero or one active host-port lease;
- one host port has zero or one active device-port lease;
- composite devices remain one unit;
- a real hub and all descendants form one atomic `TopologyRoot`; a prototype
  rejects the hub until it can preserve that contract.

This rule permits arbitrary reassignment while preserving USB enumeration and
exclusive host ownership. Multi-host sharing, device-function splitting, and
seamless migration are separate proxy products, not mesh defaults.

## Identity and discovery

Each node has a provisioned cryptographic identity. Each physical port has a
stable identifier scoped by that node, independent of a transient USB bus ID
or virtual-host-controller slot. A discovered peripheral receives an
observation record, not an identity based only on VID/PID:

```text
node identity
port identity
physical connector and controller path
device descriptor digest
serial number when present
device class and interface set
negotiated USB generation and speed
power state and measured fault state
observation generation
```

Nodes publish signed, expiring inventory updates over the management plane.
The coordinator accepts an endpoint only after authenticating the node and
matching policy. Unknown devices enter quarantine and cannot be routed.
Discovery never grants ownership.

Endpoint records carry a revision. A device removal, replacement, controller
reset, or node restart changes that revision and invalidates any route plan
based on the earlier observation. Friendly names are metadata; route commands
use stable identifiers and expected revisions.

At small scale, one coordinator may hold the complete inventory. At larger
scale, nodes publish through a durable event stream and clients query a
materialized inventory index. Inventory can be eventually consistent because
the lease authority revalidates both endpoints before committing a mutation.

## State ownership

Ownership is deliberately split:

| State | Authority |
|---|---|
| Desired route | Control-plane route store |
| Exclusive lease and per-device ownership epoch | Durable controller |
| Destination capacity reservation | Durable controller; never ownership authority |
| Physical presence, USB link, and power | Device endpoint node |
| Virtual import slot and enumeration | Host endpoint node |
| Data-session health | Both endpoint nodes |
| Human-facing summary | Derived view; never authoritative |
| Audit history | Durable append-only control-plane journal |

No node may infer a lease from a remembered route. No coordinator may infer an
active USB attachment from a committed lease alone. A route is reported
`Active` only when:

1. the lease authority has committed the current epoch;
2. both nodes authenticate the same peer, route, and epoch;
3. the device node confirms the expected physical endpoint revision;
4. the host node confirms enumeration for that session;
5. neither endpoint reports a fault or an expired lease.

The lease key is the device-port identity. A host-port index independently
prevents two device leases from targeting the same destination. Both indexes
change in one strongly consistent transaction.

Lease epochs never wrap or reset after restore. A controller that cannot prove
the latest durable epoch must refuse mutation. Wall-clock time is not the
fence; time bounds lease renewal, while the epoch rejects stale commands and
confirmations.

## Dynamic assignment

A fresh assignment follows this transaction:

```text
requested
    -> validating
    -> lease committed at epoch N
    -> endpoints preparing
    -> data session established
    -> host enumerating
    -> active at epoch N
```

Reassignment from host port `H1` to `H2` is break-before-make:

```text
active(D1, H1, N)
    -> quiescing
    -> detached(D1, H1, N)
    -> lease committed(D1, H2, N+1)
    -> attaching(D1, H2, N+1)
    -> enumerating
    -> active(D1, H2, N+1)
```

The old endpoint receives a revocation before the new lease is exposed.
Failure to acknowledge revocation does not authorize simultaneous attachment:
the authority advances the epoch, the device endpoint rejects all traffic for
the old epoch, and the new route remains pending until policy's isolation
evidence is satisfied. For a software USB/IP endpoint, ambiguous old ownership
fails closed and may require operator inspection. Later hardware may provide a
fenced port switch or controller reset as stronger isolation evidence.

Every transition has a deadline and an explicit result. A failed reroute leaves
the device unassigned or faulted; it never silently restores the old route.
Storage and stateful devices receive a quiesce policy and may reject an
operator-requested move that was not safely prepared. Product movement uses
`ColdMove`: remove Pau-supplied VBUS, verify discharge, advance the
`TopologyEpoch`, restore protected VBUS, rediscover, and present a fresh
connection. Externally powered devices may retain independent power but still
receive computer-visible disconnect and fresh enumeration.

Requests can name exact endpoints or constraints:

```text
assign device-port:A/2 to host-port:X/1

assign camera:bench-east
    to host-group:vision-workers
    requiring usb>=5Gb/s, bandwidth>=3Gb/s, latency-class=isochronous
```

Constraint-based placement chooses an eligible destination, but the committed
record always names exact endpoint identities, revisions, path, policy, and
epoch. Selection is deterministic for equal inputs and records its tie-break.

## Scalable control plane

The control plane separates read-heavy discovery from strongly consistent
ownership:

- an authenticated endpoint registry stores node and port identities;
- an inventory stream carries presence, capability, health, and revision
  observations;
- a policy service answers whether a principal may connect the exact pair;
- a lease service serializes ownership changes;
- a scheduler performs capacity-aware placement;
- a route orchestrator drives the endpoint transaction;
- an append-only audit service records intent, authorization, epoch,
  transitions, faults, and results.

A single process can implement these roles for a bench prototype. Scale comes
from preserving the boundaries, not prematurely distributing every service.
The first system keeps these roles inside one durable controller. A later HA
phase may replicate the lease store and shard leases by stable device-port
identity. Destination capacity remains a separate transactional reservation
even then. Cross-shard moves require a documented atomic protocol and remain
unavailable until proven.

Commands are idempotent and carry:

```text
request identity
principal identity
device and host endpoint identities
expected endpoint revisions
expected prior epoch
desired policy
deadline
```

Retries return the committed result for the same request identity. A different
payload cannot reuse that identity. Watch streams and CLI output are derived
from durable state and can reconnect from a sequence cursor.

The Mega 2560 is an optional operator and observation panel. It proposes a
route through one authenticated Linux node and displays returned state. It
does not own leases, credentials, network topology, or USB payload.

## Scalable data plane

Each active route uses a direct, mutually authenticated endpoint session.
Session keys bind the two node identities, both endpoint identities, route
epoch, and negotiated protocol version. Packets from another epoch, route, or
endpoint are rejected before reaching USB adapter state.

The scheduler admits a route only when the selected path has measured capacity
for its declared traffic class. It accounts for USB payload, tunnel overhead,
Ethernet overhead, retransmission behavior, and safety margin. Nominal USB or
Ethernet link rate is not admission evidence.

Separate queues prevent one bulk route from starving control traffic or
unrelated interrupt and isochronous routes. Switch QoS, VLANs, and path
telemetry support the policy, but do not convert an unbounded network into a
USB timing guarantee. Every supported transfer class needs measured latency,
jitter, loss, cancellation, and recovery evidence under competing load.

Endpoint capacity includes:

- physical USB controller and negotiated port speed;
- virtual host-controller slots;
- CPU, DMA, memory, and buffer limits;
- network interface and path bandwidth;
- supported transfer and device classes;
- protected VBUS budget and current state.

Admission reservations name the source device and its ownership epoch, but are
separate from ownership. They are released on detach, expiry, or fault.
Payload services cannot create routes independently of the lease authority.

## Failure states

| Failure | Required behavior |
|---|---|
| Device removed or replaced | Invalidate endpoint revision, revoke lease, report unassigned or fault |
| Host port disappears | Revoke destination reservation, detach device session, report fault |
| Endpoint node restarts | Restore no active claim; re-register with a new incarnation and reconcile |
| Controller restarts | Restore committed epochs; retry uncommitted operations idempotently only after durable recovery |
| Control-plane partition | Existing lease follows explicit expiry policy; no new assignment without the authoritative controller |
| Data-plane partition | Stop forwarding, expose degraded then fault, and prevent automatic stale reattach |
| Delayed old command | Reject because its epoch or endpoint revision is stale |
| Duplicate endpoint identity | Quarantine both observations until identity conflict is resolved |
| Capacity oversubscription | Reject before detach; do not degrade existing routes silently |
| Attach succeeds but commit/report fails | Fence the session, detach it, and surface an indeterminate fault |
| Audit persistence fails | Reject the mutation before endpoint action |
| Clock disagreement | Do not use clocks to order ownership; use committed epochs |
| Power or overcurrent fault | Disable the affected protected port, revoke route, require physical inspection |

Automatic reconciliation compares committed leases with authenticated reports
from both endpoints. It may complete an idempotent pending step or force an
unassigned state. It never creates a lease from endpoint observation and never
chooses a new destination without an authorized request or recorded placement
policy.

## Operator and circuit-native observability

The CLI and panel use the same route vocabulary:

```text
discovered, quarantined, unassigned, requested, detaching,
attaching, enumerating, active, degraded, fault
```

Every display identifies the exact device port, host port, and shortened epoch.
Green means both endpoints and the authority agree on that epoch. Amber means a
bounded transition. Red means no route is asserted active. A separate per-port
indicator or test point reports protected VBUS and overcurrent state; route
state never substitutes for electrical evidence.

Required CLI surfaces include:

```text
mesh nodes
mesh endpoints
mesh routes
mesh plan assign DEVICE_PORT HOST_PORT
mesh apply REQUEST_ID
mesh plan move DEVICE_PORT HOST_PORT
mesh release DEVICE_PORT
mesh watch
mesh audit ROUTE_OR_REQUEST
mesh reconcile --plan
```

Every mutating command first produces a stable plan with expected revisions,
epoch, policy result, capacity reservation, and detach/attach effects.
Applying a changed plan requires a new request identity.

## Prototype progression

### Phase 0: deterministic mesh simulator

Model many nodes, host ports, device ports, topology links, capacities, leases,
endpoint revisions, deadlines, and failures. Replay the same input trace to the
same state and audit bytes. Prove exclusivity, fencing, idempotence, restart,
partition, deterministic placement, and capacity accounting.

### Phase 1: one authority, many USB/IP endpoints

Connect the fenced controller to node-local USB/IP adapters. Support dynamic
discovery and arbitrary assignment among at least two device ports and two host
ports. Keep one authoritative controller, direct endpoint sessions, and a
durable single-writer journal. This phase validates semantics, not
availability.

### Phase 2: routed lab mesh

Add multiple endpoint nodes, management/data VLAN separation, authenticated
identity, policy, path telemetry, concurrent bulk and interrupt routes, and
fault reconciliation. Measure route-switch interruption and competing-load
behavior.

### Phase 3: replicated authority (deferred)

Replicate the durable lease service, test leader loss and partitions, and
retain monotonic fencing across restore. Introduce sharding only after the
single consensus group has measured limits.

### Phase 4: bounded high-performance endpoint

Replace selected USB/IP paths with a class-specific or hardware-assisted
endpoint while keeping the same route, lease, identity, policy, and audit
contracts. Requalify every device and transfer class; compatibility does not
carry over by architecture alone.

## Non-goals and safety limits

- no Internet-exposed USB/IP or management listener;
- no simultaneous general-purpose multi-host attachment;
- no transparent live migration claim;
- no borrowed USB identity, compliance mark, or certification claim;
- no Mega 2560 high-speed data path, USB-C role emulation, or VBUS switching
  from an unprotected GPIO;
- no routing of unknown, unauthorized, credential-bearing, or production
  peripherals during research;
- no assumption that encryption makes a malicious USB device safe.

## Prototype-only open decisions

1. Which measured compatibility requirements should trigger the later
   transparent physical proxy/appliance milestone?
2. Does “full USB 3” initially mean USB 3.2 Gen 1 at 5 Gb/s, or must the first
   acceptance target include Gen 2 or Gen 2x2?
3. Which first device classes matter: bulk storage, HID, UVC video, USB audio,
   or a controlled synthetic test device?
4. How many virtual ports should the disposable USB/IP importer expose?
5. During control-plane loss, should active routes expire promptly or remain
   active under a bounded renewable lease?
6. Which controlled segment of the shared household network should host the
   first USB/IP measurements?
7. What switch and endpoint link rates define the first concurrency target:
   10, 25, or 100 GbE?
8. Must route policy support human-selected exact endpoints only, or also
   deterministic placement into destination groups?

Phase one uses Linux USB/IP clients on a controlled segment of the shared
network. Its virtual-port choices do not alter the product contract.
