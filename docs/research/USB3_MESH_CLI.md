# USB 3 mesh CLI contract

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Product commands address `Cau`, `Pau`, `ComputerPort`,
> `PeripheralPort`, `TopologyRoot`, and `UsbPortRoute`. Linux USB/IP, virtual
> host ports, and the commands that operate them are prototype-only.

> **Controller decision:** the initial CLI talks to one durable authoritative
> controller. Replication, consensus, leader election, and automatic controller
> failover are deferred. Mutating commands fail closed when it is unavailable.

> **Prototype decision:** Linux USB/IP adapters may exercise the controller,
> but an unmodified Windows or Linux computer is the product host.

Status: proposed control-plane contract  
Scope: dynamically routed, exclusive USB attachments across trusted endpoint
nodes on the shared switched network

The mesh treats every host-facing port and device-facing port as an independently
addressable endpoint. Nodes may join or leave without changing the command
model. A route connects one device endpoint to one host endpoint; neither
machine name nor physical port is baked into the controller.

This is a control-plane contract. In the product, both attachment units
terminate their local physical USB relationship and reconstruct the exact
remote topology. A route change performs break-before-make `ColdMove`; a real
hub subtree moves atomically. The Mega 2560 may request and display routes, but
it neither carries USB traffic nor authorizes changes.

## Vocabulary and identity

Use stable controller-issued identifiers:

```text
node:edge-west
host:editing-1
device:camera-3
route:studio-camera
```

A node is a network appliance. A host endpoint presents a virtual USB
attachment to one computer. A device endpoint owns one protected physical USB
port and the device currently connected there. A route is the desired
exclusive connection between those endpoints.

Display names, addresses, USB bus IDs, and virtual-controller ports are mutable
inventory attributes, never identities. A replaced endpoint receives a new
identity unless an authorized adoption transaction proves continuity.

The authoritative controller stores:

- endpoint identity, role, node, health, capabilities, and last observation;
- physical connector and descriptor inventory for device endpoints;
- host policy, USB class policy, and bandwidth budget;
- desired and observed route state;
- monotonically increasing per-source-device ownership epoch and endpoint incarnation;
- destination capacity reservations separate from ownership;
- transaction deadline, reason, and tamper-evident audit reference.

One device endpoint belongs to at most one host endpoint. A host endpoint may
support multiple routes only when its declared virtual-controller capacity and
policy allow them. Capacity is explicit; it is not inferred from a machine
name. Hubs and composite devices initially remain one leased device subtree.

## Command shape

The future executable is named `adk-usb-mesh`. Read-only commands run directly.
Every mutation follows the same two-step grammar:

```sh
adk-usb-mesh <mutation> ... --plan
adk-usb-mesh apply <plan-id>
```

Planning validates identity, authorization, capacity, bandwidth, endpoint
health, current ownership epoch, and the detach-before-attach sequence. It writes
an immutable, expiring plan with a digest. Applying accepts that exact plan
once. It does not silently re-plan against changed state.

All commands support human-readable output by default and stable
machine-readable records through `--output json`. Scripts use identifiers and
JSON fields, never terminal columns or display names. Successful mutations
print their audit event and resulting ownership epoch.

## Discover and inventory

Start by observing the mesh:

```sh
adk-usb-mesh node list
adk-usb-mesh endpoint discover --node node:edge-west
adk-usb-mesh endpoint list --role device
adk-usb-mesh endpoint show device:camera-3
adk-usb-mesh endpoint list --role host --healthy --output json
```

Discovery reports observations; it does not authorize an endpoint. Enrollment
is a planned mutation:

```sh
adk-usb-mesh endpoint enroll device:camera-3 \
    --node node:edge-west \
    --connector usb-a-2 \
    --plan

adk-usb-mesh apply plan:01J...
```

Enrollment binds stable identity to one endpoint incarnation and its
authenticated node. Moving a cable, changing a bus ID, restarting an endpoint,
or replacing hardware updates observed inventory without rewriting a route.
An incarnation change fences confirmations from the previous endpoint process.

## Create a route

The operator names intent before mechanics:

```sh
adk-usb-mesh route create route:studio-camera \
    --device device:camera-3 \
    --host host:editing-1 \
    --plan

adk-usb-mesh plan show plan:01J...
adk-usb-mesh apply     plan:01J...
```

The plan reads as a transaction:

```text
authorize endpoints and policy
reserve host capacity and network bandwidth
fence the next device ownership epoch
attach device:camera-3 to host:editing-1
wait for matching endpoint observations
publish active only after both sides report the same ownership epoch
```

No active route is inferred from controller intent alone. `active` requires
device edge, host edge, and controller agreement on identity, incarnation, and
ownership epoch.

## Move either endpoint

Both sides are dynamically reconfigurable. Change a route's host:

```sh
adk-usb-mesh route move route:studio-camera \
    --to-host host:editing-2 \
    --plan

adk-usb-mesh apply plan:01J...
```

Change its device while retaining the named consumer route:

```sh
adk-usb-mesh route move route:studio-camera \
    --to-device device:camera-4 \
    --plan

adk-usb-mesh apply plan:01J...
```

Change both endpoints atomically:

```sh
adk-usb-mesh route move route:studio-camera \
    --to-device device:camera-4 \
    --to-host host:editing-2 \
    --plan
```

A move always quiesces and detaches the old attachment before attaching the
new one. Its plan names the expected device ownership epoch and both endpoint
incarnations. Any intervening change makes the plan stale. Failure leaves the
route unassigned or explicitly faulted; it never resurrects the prior route
without a new plan.

Moving a physical endpoint appliance is separate from moving a logical route.
Node relocation changes inventory and reachability. Stable endpoint identity
survives only when the same authenticated appliance and port incarnation
report at the new location.

## Release and remove

Release disconnects the attachment but preserves named route intent:

```sh
adk-usb-mesh route release route:studio-camera --plan
adk-usb-mesh apply         plan:01J...
```

Remove deletes an already released route record:

```sh
adk-usb-mesh route remove route:studio-camera --plan
adk-usb-mesh apply        plan:01J...
```

Release completes only after the host edge confirms detach at the planned
ownership epoch. Storage receives an orderly detach policy and is never
power-cycled as an ordinary routing operation. Ambiguous detach, endpoint
loss, or deadline expiry produces a visible fault and quarantines reassignment
until reconciliation.

## Status and watch

Status shows desired and observed state separately:

```sh
adk-usb-mesh status
adk-usb-mesh route show route:studio-camera
adk-usb-mesh status --node node:edge-west --output json
```

Watch emits a snapshot followed by ordered events:

```sh
adk-usb-mesh watch
adk-usb-mesh watch --route route:studio-camera --since-event 1842
adk-usb-mesh watch --output json
```

Each event includes event ID, controller incarnation, device ownership epoch, endpoint
incarnations, logical time, transition, reason, and audit digest. Reconnecting
with `--since-event` either resumes exactly or reports that the retained event
window is insufficient and requires a fresh snapshot. Watch output is
observation only; it cannot confirm or apply a mutation.

The concise route states are:

```text
unassigned -> reserving -> detaching -> attaching -> enumerating -> active
                                      \-> fault
```

Status additionally reports `degraded` when the data path remains attached but
health or reserved service level is no longer satisfied. Policy decides
whether degradation triggers a planned detach; it does not silently reroute.

## Scalable mesh rules

A scalable deployment separates desired state from endpoint execution:

- one durable authoritative controller allocates ownership epochs and plan IDs;
- endpoint agents reconcile only commands signed for their identity and
  current incarnation;
- node-local adapters translate intent to USB/IP or later transports;
- route locks are granular per involved endpoint, not one global mesh lock;
- admission accounts for host slots, device exclusivity, switch path,
  bandwidth, and periodic-transfer budget;
- endpoint heartbeats carry observations, never authority to invent routes;
- retry uses the same transaction ID and is idempotent;
- topology loss cannot produce two active owners;
- batch changes use an explicit transaction and deterministic lock order;
- sharding, if needed, keeps both endpoints of a route under one transaction
  authority.

Dynamic configuration does not mean automatic movement. An optional scheduler
may propose placement, but policy still emits an inspectable plan. The first
implementation supports operator-directed routes only.

## Circuit-native observation

Product inspection always reports one display mode:

```text
Startup Normal Night Attention Fault Test ControllerLost Maintenance
```

CLI names and local labels match. A mode change never implies a route change.
Plans also report shared-LAN reservation, ordinary-LAN headroom, active
profile, negotiated PoE budget, auxiliary-power state, and the limiting
constraint.

Every physical endpoint shows state without relying on logs:

- blue: enrolled and unassigned;
- pulsing amber: planned transaction in progress;
- green: both endpoints and controller agree on active ownership epoch;
- red: fault or quarantine;
- protected-power LED: measured local VBUS, independent of route state.

Named test points expose protected VBUS, load-switch enable, and endpoint
heartbeat. The panel proves controller state; these points separately prove
electrical state. Neither proves USB payload correctness. Payload tests record
hashes, transfer errors, throughput, and latency at the host.

## Failure and security contract

The mesh defaults to no route. Management and data planes use mutually
authenticated identities on isolated networks. Per-endpoint policy is
default-deny, and plans are bound to actor, controller incarnation, ownership epochs,
incarnations, expiry, and digest.

Endpoint disappearance, controller restart, stale confirmation, duplicate
request, host crash, device removal, overcurrent, bandwidth loss, and audit
failure are explicit test traces. A partitioned endpoint cannot extend its
lease locally. Reconciliation favors an observable disconnect over ambiguous
ownership.

Do not route sensitive or untrusted devices during this research phase. Do not
expose USB/IP or management ports to the Internet. The temporary Python ledger
is not a distributed authority and must not coordinate mesh endpoints.

## Compatibility migration

The current phase-one CLI names one device node, bus ID, host node, and virtual
port:

```text
USB_DEVICE_NODE + USB_BUS_ID -> device endpoint
USB_HOST_NODE                -> host endpoint
USB_PORT                     -> observed adapter attachment
```

Migration preserves the safe plan/apply habit:

| Phase-one target | Mesh equivalent |
|---|---|
| `usb-local`, `usb-remote`, `usb-ports` | `endpoint discover`, `endpoint list` |
| `usb-assign-plan` | `route create ... --plan` |
| `usb-assign` | `apply <plan-id>` |
| `usb-route-plan` | `route create` or `route move ... --plan` |
| `usb-route` | `apply <plan-id>` |
| `usb-release-plan` | `route release ... --plan` |
| `usb-release` | `apply <plan-id>` |
| `usb-matrix-status` | `status` |
| `usb-matrix-log` | `watch` or audited event export |

During one compatibility release, a local adapter may translate old variables
into explicit endpoint records and print the equivalent mesh command. It must
not invent stable identities from DNS names after that import. Old apply
targets require a warning and remain single-controller only. They are removed
once plan IDs, endpoint enrollment, durable ownership epochs, and reconciliation
have replaced the temporary ledger.

## Initial acceptance traces

Before hardware performance work, deterministic tests cover:

1. create, release, host move, device move, and two-sided move;
2. simultaneous plans competing for either endpoint;
3. stale plan, ownership epoch, incarnation, and controller-term rejection;
4. detach timeout, attach failure, enumeration failure, and retry;
5. endpoint restart, node loss, controller restart, and event resumption;
6. host capacity, route count, and bandwidth admission boundaries;
7. idempotent apply and duplicate endpoint reports;
8. audit write failure without unauthorized state mutation;
9. identical replay producing identical ordered events;
10. no trace in which one device has two active hosts.

Hardware acceptance remains deferred. Until endpoint agents, durable authority,
authentication, and reconciliation pass these gates, this document defines a
research interface rather than a deployable USB mesh.
