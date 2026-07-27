# USB mesh cross-platform host architecture

Status: research architecture; no product compatibility claim  
Research date: 2026-07-27  
Scope: unmodified Windows and Linux computers connected by physical USB to a
USB mesh gateway

## Mandatory product boundary

The canonical terms are defined by
[`USB_TRANSPARENT_PRODUCT.md`](USB_TRANSPARENT_PRODUCT.md). Earlier drafts used
`ComputeGateway`, `ConsoleGateway`, `MeshController`, `PhysicalSubtree`,
`UsbRoute`, and `RouteEpoch`; these mean `Cau`, `Pau`, `Controller`,
`TopologyRoot`, `UsbPortRoute`, and `TopologyEpoch`, respectively.

The final product preserves this physical path:

```text
unmodified Windows or Linux computer
    <-> physical SuperSpeed USB
        <-> Computer Attachment Unit
            <-> ordinary switched Ethernet
                <-> Peripheral Attachment Unit
                    <-> physical USB peripheral
```

Windows and Linux must use their ordinary USB host stacks and the peripheral's
ordinary class or vendor driver. The computer requires no mesh-specific
application, configuration, custom kernel driver, virtual USB bus, virtual
host controller, or device-specific transport driver. Administration occurs
through the separate Linux controller.

The `ComputeGateway` therefore presents one standards-compliant native USB
topology to the computer. It consumes exactly one computer USB port and
reconstructs exactly one selected `ConsoleGateway.PeripheralPort`. It never
inserts, synthesizes, or conceals a hub. A directly attached peripheral appears
directly attached. If the selected physical port contains a real hub, that real
hub and its complete downstream subtree are reproduced as one atomic topology.
The gateway reproduces the selected topology's descriptors, endpoints, control
state, transfer behavior, hotplug, reset, and power-management semantics.

The `ConsoleGateway` independently terminates and enumerates the real physical
topology. Ethernet carries a modeled USB transaction service between those two
terminations; it does not extend SuperSpeed symbols or timing electrically.

This is a harder boundary than USB/IP. It is also the boundary that matches the
intended household installation.

The authoritative `MeshController` runs on a normal Linux computer. It manages
identity, authorization, route epochs, desired state, reconciliation, and
audit records. It carries no USB payload. Controller high availability remains
deferred.

## Rejected product shortcuts

Linux USB/IP remains a useful lab prototype for routing, fencing, inventory,
and representative transfer measurements. It is not the product data plane
because it imports into a modified Linux kernel and cannot serve an unmodified
Windows computer through the required physical USB topology.

A vendor-specific USB tunnel function plus custom Windows/Linux host adapters
is also rejected as a product architecture. It may be used as a disposable
engineering instrument to capture traces or exercise the mesh protocol, but:

- it cannot define the product wire contract;
- it cannot appear in a product compatibility claim;
- it cannot become a prerequisite for normal peripheral use;
- success with it does not prove native USB reconstruction.

PCIe, Thunderbolt, and network drivers are likewise outside the declared
computer attachment. They may inform research but do not satisfy
computer USB → gateway → Ethernet → gateway → peripheral.

## Unambiguous nomenclature

| Term | Meaning |
|---|---|
| `MeshController` | Authoritative Linux control service; never a payload relay |
| `ComputeGateway` | Appliance beside a computer; USB device electrically |
| `ConsoleGateway` | Appliance beside peripherals; USB host electrically |
| `ComputerPort` | Physical upstream USB device-mode port on a `ComputeGateway` |
| `PeripheralPort` | Physical downstream USB host port on a `ConsoleGateway` |
| `PhysicalSubtree` | Complete physical topology rooted at one `PeripheralPort` |
| `PresentedTopology` | Native reconstruction of that exact `PhysicalSubtree` |
| `PeripheralAttachment` | One atomic physical subtree and its current presentation |
| `UsbRoute` | Authorized association between one `ComputerPort` and one `PhysicalSubtree` |
| `RouteEpoch` | Durable, monotonically increasing fence for that complete subtree |
| `DataSession` | Authenticated gateway-to-gateway transport for one route |
| `Fabric` | Ordinary switched Ethernet network |

Use `computer` only for the Windows or Linux machine consuming peripherals.
Use `host` only for an electrical or USB-protocol role. Avoid `source`,
`destination`, `client`, and `server` in public route names because direction
changes with transfer type and point of view.

The electrical roles are fixed:

| Boundary | USB role |
|---|---|
| Computer port | Host |
| `ComputeGateway.ComputerPort` | Device |
| `ConsoleGateway.PeripheralPort` | Host |
| Peripheral | Device |

## Native reconstruction architecture

Both operating systems see the same standards-defined topology:

```text
ordinary Windows/Linux USB host stack
    -> physical root port
        -> ComputeGateway ComputerPort presentation
            -> exact PresentedTopology
                -> gateway transaction model
                    -> authenticated Ethernet DataSession
                        -> ConsoleGateway USB host controller
                            -> selected PeripheralPort
                                -> exact PhysicalSubtree
```

The computer-side hardware must dynamically reproduce one selected physical
topology. It must not add a hub to multiplex routes or simplify the
implementation. A conventional USB gadget controller that exposes only one
fixed composite device is insufficient for arbitrary changing topology.
Candidate implementations may combine:

- a programmable USB 3 device controller and PHY;
- FPGA or dedicated topology-reconstruction logic;
- bounded DMA and packet memory;
- a Linux-capable SoC for policy, discovery, updates, and certificates;
- 10/25 GbE or faster network interfaces selected from measured requirements.

The architecture does not assume that arbitrary device or real-hub behavior
can be implemented by configuration alone. The first hardware spike must prove
native connect, enumeration, traffic, reset, suspend, resume, and disconnect
without a custom driver. A later real-hub qualification must prove that the
observed hub and downstream topology come from the selected physical subtree,
not gateway-created topology.

### What must be reconstructed

The `ComputeGateway` must reproduce, for every qualified device:

- device, configuration, interface, endpoint, BOS, and string descriptors;
- standard, class, and declared vendor control requests;
- bulk, interrupt, and declared isochronous endpoints;
- endpoint sequencing, data toggles where applicable, short packets, stalls,
  clears, cancellations, and resets;
- device state across address, configuration, alternate setting, and suspend;
- real-hub status/change notifications and downstream connect generations when
  the selected physical subtree contains a hub;
- speed, service intervals, bandwidth admission, and bounded buffering;
- remote-wake behavior and explicitly modeled power observations.

USB link-local handshakes and deadlines terminate at each gateway. The network
cannot wait across Ethernet for every link-level response. Each gateway must
respond locally, buffer within published bounds, and reconcile the resulting
state through the modeled transaction protocol. A device whose semantics
cannot tolerate that separation is unsupported until proven otherwise.

Identity presentation also needs a declared rule. Reusing a peripheral's
VID/PID, strings, and serial number may be necessary for its ordinary driver
to bind, but those values are observations, not mesh authentication. Product
identity, gateway identity, and route authorization remain separate and are
never exposed as counterfeit peripheral credentials.

### Atomic port/subtree ownership

One `ComputeGateway` owns one `ComputerPort`, consumes one physical port on one
computer, and carries at most one active `UsbRoute`. That route selects exactly
one `PeripheralPort`. The selected port and everything physically below it form
one indivisible `PhysicalSubtree`.

```text
ComputerPort C1 <-> PeripheralPort P7 <-> complete subtree rooted at P7
```

If `P7` contains a keyboard, the computer sees that keyboard directly. If `P7`
contains a physical hub with a keyboard, mouse, and storage device, the
computer sees that same hub and complete subtree. The controller cannot route
individual children elsewhere, merge children from different ports, or
manufacture downstream ports. Moving any child requires moving the entire
rooted subtree through a new epoch and native unplug/plug sequence.

Inventory may describe every observed child for policy, capacity, and
diagnosis, but the lease key is the rooted `PhysicalSubtree`. Authorization
evaluates the complete descriptor tree and rejects a changed, added, or removed
child until the subtree generation is reconciled.

## Stable gateway protocol

The product protocol is independent of Linux USB/IP, operating-system internals,
endpoint firmware releases, and network carrier. It has two contracts:

1. `GatewaySessionProtocol` transports canonical USB operations and lifecycle
   events directly between a `ComputeGateway` and `ConsoleGateway`.
2. `ControlProtocol` transports signed route grants, epochs, capabilities,
   health, and reconciliation state between gateways and `MeshController`.

The canonical operation model represents:

- descriptor and device-generation observations;
- control, bulk, interrupt, and isochronous operations;
- submission, completion, cancellation, timeout, and stall;
- endpoint, device, port, and controller reset;
- attach, detach, reconnect, and descriptor change;
- exact lengths, short-packet rules, directions, and status;
- service-interval metadata and bounded buffering;
- suspend, resume, wake request, and power-state observations.

Every operation carries a protocol version, session identity, route identity,
`RouteEpoch`, endpoint generation, bounded sequence number, deadline, and
integrity protection. Payload counts and sizes are bounded. Unknown required
fields, overflow, impossible transitions, stale epochs, and exhausted bounds
fail the affected route closed.

Compatibility negotiation selects an authenticated major/minor protocol pair
and exact capability intersection before activation. A major mismatch prevents
presentation. A minor version may add optional fields only with defined safe
defaults. Downgrade is authenticated and policy bounded.

Ordering is explicit per endpoint and stream, not inferred from TCP arrival
order. Cancellation races, late completions, duplicates, retransmission,
restart, and sequence exhaustion have deterministic outcomes. A network retry
must not execute a non-idempotent USB operation twice.

The first carrier may be a reliable encrypted stream. Later transfer-aware
datagrams or hardware queues must preserve the same observable contract.

## Attachment lifecycle

A `PeripheralAttachment` has these externally visible states:

```text
Absent
    -> Authorized
    -> Preparing
    -> Presenting
    -> Enumerating
    -> Active
    -> Quiescing
    -> Detached

any non-absent state -> Faulted -> Detached
```

Activation requires:

1. the controller commits the `UsbRoute` and a new `RouteEpoch`;
2. both gateways persist and authenticate the same route;
3. the `ConsoleGateway` confirms the expected physical generation;
4. the `ComputeGateway` verifies that its sole `ComputerPort` is unassigned;
5. the `ComputeGateway` exposes the exact reconstructed physical topology;
6. the unmodified computer enumerates it through its ordinary USB stack;
7. all participants report agreement at the same epoch and generation.

Reassignment is break-before-make. The old computer observes a native detach,
outstanding operations finish or cancel within policy, and the old epoch is
fenced at the `ConsoleGateway` before a new computer sees a connection. Failure
leaves the peripheral detached or faulted; it never silently restores the old
route.

Route movement deliberately looks like a person physically unplugging the
complete peripheral subtree from the old computer and plugging it into the new
one:

```text
old computer: native disconnect
    -> old presentation removed
        -> old epoch fenced
            -> new presentation created
                -> new computer: native connect and full enumeration
```

There is no transparent live migration, transfer-session preservation, device
state handoff, or promise that an application remains open. Filesystems,
cameras, audio streams, and vendor applications must tolerate an ordinary
detach before the operator moves a route. A move rejected by a quiesce policy
remains at the old route only if no detach or epoch advance occurred; after
detach, restoration is a distinct newly authorized attachment.
A real hub and every child below it detach and re-enumerate together.

Computer reboot, gateway restart, cable removal, and fabric partition enter
reconciliation. Remembered state is evidence, not authority. No participant
reports `Active` until current identities, generations, route, and epoch agree.

## Power, suspend, and hotplug

Logical routing, data-session state, and electrical power are separate.

- A `PeripheralPort` reports protected VBUS capability and budget.
- A route grant does not authorize a power cycle.
- VBUS changes are explicit, audited, class-aware operations.
- Overcurrent protection acts locally and reports a fault.
- Computer suspend does not imply peripheral power removal.
- Remote wake is presented only when the computer, both gateways, network
  policy, and peripheral support it within a measured wake budget.
- Wake-on-LAN is not described as equivalent to USB remote wake.
- Firmware maintenance quiesces or detaches affected routes first.

Physical hotplug changes the peripheral generation. A replacement never
inherits authorization merely because it uses the same connector, VID/PID,
serial string, or descriptor set. Descriptor mutation causes quarantine or an
explicit re-enumeration policy.

Compatibility records distinguish normal removal, surprise removal, logical
detach, USB reset, VBUS loss, gateway restart, network partition, computer
suspend, hibernate, fast startup, and shutdown. Storage testing uses disposable
media until flush, quiescence, cancellation, and failure behavior are measured
on both operating systems.

## Security and updates

Treat a remote peripheral exactly as a physically local hostile peripheral.
The mesh adds hostile-network, compromised-gateway, stale-route, and
supply-chain threats.

- Controller, gateways, and operators have distinct cryptographic identities.
- Gateway and controller sessions use mutual authentication, confidentiality,
  replay resistance, and route/epoch binding.
- Gateway firmware and FPGA images are signed, measured, rollback protected,
  and recoverable.
- Production boot verifies all executable endpoint components.
- Updates are staged, compatibility checked, health checked, and reversible
  without rolling back epochs or trust revocations.
- Failed updates leave an endpoint unavailable, not permissive.
- Management and payload traffic use distinct policy and preferably distinct
  network security domains.
- Logs contain identity, state, status, timing, and byte counts, not payloads.

No host kernel package exists in the supported architecture. An optional
administrative application uses only the authenticated controller API and is
not required for enumeration or data transfer.

A mixed-version mesh activates a route only when the signed compatibility
manifest has a nonempty intersection:

```text
gateway protocol range
ComputeGateway hardware and firmware capabilities
ConsoleGateway hardware and firmware capabilities
controller API range
supported computer operating-system releases
supported USB generations and transfer classes
qualified peripheral identities and device classes
known incompatibilities
upgrade and rollback constraints
```

## Honest compatibility staging

Transparent does not mean universal. Compatibility is qualified per operating
system release, gateway hardware/firmware, network profile, and peripheral:

| Level | Meaning |
|---|---|
| `Presents` | The OS observes the exact native device or real-hub subtree without custom software |
| `Enumerates` | The ordinary class or vendor driver binds |
| `Basic` | Representative control and low-rate operations pass |
| `Sustained` | Published throughput and duration pass without corruption |
| `Recovery` | Hotplug, reset, suspend, restart, and partition cases pass |
| `Qualified` | Complete published device-specific matrix passes |

Passing one device does not qualify its class. Passing Linux does not imply
Windows compatibility or the reverse. A 5 Gb/s connector does not establish
5 Gb/s payload service.

Implementation expands through bounded, independently honest profiles:

1. synthetic fixed-descriptor control and bulk loopback;
2. one HID keyboard and one mouse on isolated test computers;
3. selected USB serial adapters;
4. selected disposable mass-storage devices;
5. declared composite devices;
6. selected UVC cameras and USB audio after latency instrumentation;
7. vendor-specific devices one model and firmware at a time;
8. real hub subtrees and security devices only after separate ownership and threat
   reviews.

Unsupported devices remain visibly unsupported. The gateway must not partially
present an unknown composite device, substitute a generic class silently, or
claim arbitrary USB compatibility.

Isochronous audio and video are likely the hardest household devices because
the computer expects bounded service timing while the fabric introduces jitter.
They require admission control, local buffering, clock-domain handling, drift
measurement, and explicit underflow/overflow behavior. Storage is sensitive to
detach and failure ordering. HID is lower bandwidth but security-sensitive
because it can inject trusted input.

## Cross-platform test matrix

Deterministic synthetic endpoints precede real peripherals. Network fault tests
inject delay, loss, reordering, duplication, corruption, disconnect, and
contention without deriving correctness from wall-clock scheduling.

| Axis | Required cases |
|---|---|
| Computer OS | Declared Windows releases; declared Linux distributions and kernels |
| Native binding | No ADK driver installed; ordinary drivers bind to the exact physical topology |
| Computer lifecycle | Cold boot, warm reboot, suspend, hibernate, fast startup, shutdown |
| Gateway lifecycle | Cold boot, service restart, firmware update, power loss, recovery |
| Route lifecycle | Attach, detach, move, failed move, retry, stale epoch, controller loss |
| USB speed | Low, full, high, SuperSpeed 5 Gb/s; later speeds only when supported |
| Transfer type | Control, bulk, interrupt, isochronous |
| Peripheral shape | Single function, composite, later qualified physical hub subtree |
| Power | Suspend, resume, remote wake, overcurrent, explicit VBUS cycle |
| Fabric | Idle, contention, congestion, link loss, switch restart, MTU variation |
| Security | Unknown gateway, revoked identity, replay, downgrade, malformed frame |
| Integrity | Exact payload, cancellation race, short packet, stall, reset, exhaustion |
| Scale | Multiple ports, gateways, computers, independent routes, route churn |

Each Windows/Linux parity run uses the same canonical trace and records:

- native descriptor and enumeration transcript;
- exact lifecycle and physical-hub change event order when a hub is present;
- operation result and data digest;
- latency distribution by transfer type;
- goodput, late/lost isochronous packets, and queue bounds;
- route, epoch, generation, and reconciliation result;
- OS, ordinary peripheral driver, firmware, controller, and protocol versions;
- gateway resource use, crash evidence, kernel warnings, and recovery time.

The trace must lead to the same protocol state and route outcome. Timing budgets
may differ by operating system, but differences are published.

Fuzz gateway parsers and lifecycle state machines before physical computer
attachment. Run hostile descriptor and transfer corpora on disposable
computers. Physical acceptance later records gateways, cables, switch,
peripheral, instruments, and observations; no software test is reported as
USB compliance.

## Delivery sequence

1. Keep Linux USB/IP as a lab-only route/fence/measurement prototype.
2. Freeze a canonical operation model from captured prototype traces.
3. Implement a user-space gateway simulator and deterministic fault corpus.
4. Prove one native, fixed SuperSpeed bulk-loopback `PresentedDevice`.
5. Prove native dynamic connect/disconnect without inserting a hub.
6. Connect one `ConsoleGateway` port and reconstruct its synthetic device.
7. Pass identical no-custom-driver enumeration on Windows and Linux.
8. Qualify HID, serial, storage, composite, then isochronous profiles.
9. Add multi-port and multi-gateway dynamic mesh routing.
10. Pursue higher USB generations only after measured 5 Gb/s behavior and
    hardware architecture justify them.

A milestone is cross-platform only when unmodified Windows and Linux pass the
declared common compatibility level. USB/IP or a custom-driver tunnel can
inform development but can never close that milestone.

## Consequences

This decision removes a large software shortcut and moves complexity into the
computer-side gateway:

- programmable native topology reconstruction becomes mandatory;
- arbitrary USB compatibility is not credible as an early claim;
- gateway silicon selection depends on dynamic topology and endpoint behavior,
  not merely a SuperSpeed device-mode connector;
- hardware-in-the-loop Windows/Linux testing begins earlier;
- firmware and FPGA verification become product-critical;
- per-device qualification may remain necessary indefinitely;
- full USB 3 bandwidth and isochronous behavior require measured fabric and
  buffer budgets;
- the first useful household product may support a deliberately small device
  list while preserving the final topology.

The benefit is equally clear: a computer in the utility closet needs no special
kernel software and remote-room peripherals bind through the operating system's
normal USB model.

## Deferred work

- controller high availability and replicated route authority;
- macOS and non-PC hosts;
- USB 3.2 Gen 2, Gen 2x2, and USB4 claims;
- unqualified real hubs, subtree splitting, and simultaneous multi-host sharing;
- live migration or session preservation without native detach and enumeration;
- security tokens and unqualified vendor-specific devices;
- product USB-IF, regulatory, electromagnetic, and broad interoperability
  certification.

## Open decisions

- Which exact Windows releases and Linux distributions/kernels form the first
  compatibility envelope?
- Which one HID device should follow synthetic bulk as the first useful native
  reconstruction?
- What network rate and worst-case latency budget define the first household
  installation?
