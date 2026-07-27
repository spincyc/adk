# Transparent USB product contract

This document is the canonical product contract for the transparent USB mesh.
It records accepted product decisions. Other USB research documents describe
prototypes, implementation candidates, or unresolved engineering work; they do
not override this contract.

## Product outcome

The product extends one physical USB connection across a normal switched
Ethernet network:

```text
unmodified Windows or Linux computer
    -> physical USB
    -> Computer Attachment Unit
    -> shared switched Ethernet
    -> Peripheral Attachment Unit
    -> physical USB peripheral or user-provided hub
```

The computer uses its normal USB stack and native device drivers. It requires
no custom driver, kernel module, service, installer, virtual host controller,
or mesh-specific configuration. The presented connection must behave like a
physical unplug and plug, not like an application-level device tunnel.

## Canonical terms

| Term | Contract |
|---|---|
| `Controller` | Durable, authoritative route-management service on a normal Linux computer |
| `ComputerAttachmentUnit` (`Cau`) | Appliance beside a computer; presents the routed topology through physical USB |
| `PeripheralAttachmentUnit` (`Pau`) | Appliance beside peripherals; hosts physical USB topology roots |
| `ComputerPort` | The Cau's single computer-facing USB device-role port |
| `PeripheralPort` | One independently powered USB host-role port on a Pau |
| `TopologyRoot` | The complete device or hub tree physically rooted at one `PeripheralPort` |
| `ObservedTopology` | The topology the Pau currently observes |
| `PresentedTopology` | The equivalent topology the Cau currently presents |
| `UsbPortRoute` | Exclusive assignment of one `TopologyRoot` to one `ComputerPort` |
| `TopologyEpoch` | Durable, non-reusable fence invalidating stale work for a routed topology |
| `AttachmentSession` | One computer-visible plug-to-unplug lifetime |
| `Profile` | A named bandwidth, latency, jitter, recovery, and compatibility contract |
| `FailurePolicy` | Configured production response when an active contract cannot be met |

Electrical USB roles remain explicit: the computer is a USB host, the Cau
`ComputerPort` behaves as a USB device, each Pau `PeripheralPort` is a USB
host, and attached peripherals are USB devices.

## Controller

The `Controller` runs as an ordinary Linux service on a normal computer. It
owns inventory, authorization, desired routes, durable epochs, admission,
reconciliation, and audit history. It is never in the USB data path.

The first product uses one durable authoritative controller. High availability,
replication, quorum, leader election, and automatic controller failover are
deferred. Controller loss fails closed: endpoints neither elect authority nor
infer new ownership. Protocol boundaries must not prevent later HA work.

## Computer Attachment Unit

Each Cau has:

- exactly one fixed-role USB 3 Type-B `ComputerPort`;
- one 10GBASE-T connection carrying mesh data and PoE++;
- auxiliary DC input for development and qualified fallback;
- no dependency on power from the computer's USB port;
- protection against back-powering the computer;
- a local visual status display.

Certified Type-A-to-Type-B or Type-C-to-Type-B cables connect the Cau to a
computer. A Cau consumes exactly one physical computer USB port. Scale by
adding independent Caus; a multi-port chassis may package them later without
changing the route model.

The Cau must not create a hidden USB hub, flatten a topology, or expose a
mesh-specific tunnel device. It reconstructs the exact topology rooted at the
selected Pau port.

## Peripheral Attachment Unit

The baseline Pau has:

- four independently routable USB 3 Type-A `PeripheralPort` roots;
- one 10GBASE-T connection carrying mesh data and PoE++;
- auxiliary DC input for development and qualified overload fallback;
- protected 5 V VBUS power on every port;
- aggregate PoE power admission;
- a local visual status display.

Each port independently owns current limiting, short-circuit protection,
inrush control, reverse-current protection, overcurrent detection, controlled
power removal, voltage/current measurement, and fault reporting. Power and
route health are separate states. No port silently exceeds the admitted PoE
budget. USB-C and USB Power Delivery are deferred.

PoE++ operation is a target subject to measured electrical and thermal
qualification. The contract does not imply that an unqualified 10GBASE-T
PoE++ implementation can deliver every peripheral's maximum load. A
self-powered user hub remains appropriate for high aggregate loads.

## Transparent topology

One `UsbPortRoute` selects one complete `TopologyRoot`. If a peripheral is
attached directly, the computer sees that peripheral. If a user attaches a
physical hub, the computer sees that hub and all descendants in an equivalent
tree.

The entire hub subtree moves atomically. Descendants cannot be split across
computers, independently routed, or left owned by an old route. Host-assigned
USB addresses may differ after enumeration; topology, descriptors, interfaces,
endpoints, changes, and observable behavior are the transparency contract.

The four Pau roots remain independent. Routing more than one root to a computer
requires the same number of Caus and physical computer USB ports. A user who
wants several peripherals through one computer port supplies the physical hub
at the Pau.

## Route movement

Moving a route is break-before-make and appears as physical unplug/replug.
`ColdMove` is the default:

```text
quiesce where possible
    -> old computer observes disconnect
    -> Cau rejects stale transfers
    -> Pau removes VBUS
    -> verify VBUS discharge
    -> advance TopologyEpoch
    -> restore protected VBUS
    -> rediscover the exact topology
    -> new computer observes a fresh connection
    -> normal native enumeration
```

Externally powered devices may retain their independent power but still receive
computer-visible disconnect and fresh enumeration. Device, application, and
filesystem state are not migrated. Storage must be safely ejected before an
operator-requested move. An unexpected network failure has ordinary unexpected
unplug semantics; the product must prevent stale post-fence transfers but
cannot promise filesystem consistency.

A proposed replacement is transactional. Planning or preparation failure
leaves an existing healthy route unchanged.

## Shared switched network

USB, HDMI, controller traffic, telemetry, and ordinary household traffic share
one switched Ethernet network. VLANs and QoS may provide logical isolation but
must not require a separate physical fabric.

Every route is admitted against an observed end-to-end path contract covering
bandwidth, burst capacity, latency, jitter, endpoint capacity, and power.
Admission reserves ordinary-LAN headroom and rejects an unsupported route
before changing the active connection. The controller reports the limiting
constraint rather than silently accepting corruption.

The baseline USB appliance link is 10GBASE-T. Different shared-fabric links may
run at different rates. No document may equate a link's nominal rate with
proven transparent support for every USB 3 device or transfer type.

## Profiles

Profiles make bandwidth and latency tradeoffs explicit. USB profiles describe
measured contracts appropriate to device behavior, such as interactive
interrupt traffic, reliable bulk traffic, bounded audio/video traffic, or a
conservative compatibility mode. A profile cannot relax a USB timing rule that
the host or peripheral requires.

The controller supports:

- `PinnedProfile`: use exactly one profile or fail;
- `AllowedProfiles`: choose only from an ordered approved set;
- `BestAvailableWithinBounds`: choose any profile satisfying explicit bounds.

The active profile is versioned, audited, and visible at both attachment units.
No route silently changes profile. A pinned-profile planning failure leaves the
current working route unchanged.

## Production failure and recovery

`FailurePolicy` configures the production response to contract loss. It is
distinct from the selected profile. A USB route that can no longer meet its
timing or correctness contract must:

```text
disconnect the Cau presentation
    -> reject stale transfers
    -> remove Pau-supplied VBUS
    -> verify discharge
    -> advance TopologyEpoch
    -> display and audit the exact cause
```

Automatic recovery is the transparent default. After the path continuously
satisfies ownership, endpoint health, power, bandwidth, latency, and jitter for
the profile's configured stability interval, the Pau restores protected power,
rediscovers topology, and the Cau presents a fresh native plug event. Any
regression restarts the interval. Short intervals may suit keyboards and mice;
longer intervals may suit storage, cameras, audio, and hub subtrees.

Manual recovery remains an explicit exceptional policy, not the ordinary
transparent behavior.

## Visual evidence

Every Cau and Pau must show its actual applied state locally without requiring
Serial output or controller access. A display or equivalent accessible
indicator reports at least:

- route identity and endpoint role;
- requested and active profile;
- attached USB generation or topology summary;
- negotiating, active, disconnected, and recovery-wait states;
- power state and admitted load on a Pau;
- pinned-profile, capacity, ownership, topology, power, and endpoint faults;
- remaining stability interval during automatic recovery;
- unmistakable `TEST` state during authorized fault injection.

Color may reinforce state but cannot be the sole indication.

## Deterministic fault injection

Disabled-by-default deterministic fault injection is lab and instructional
infrastructure, not ordinary production adaptation. It has separate
authorization, named test routes, bounded duration, an immediate cancel path,
distinct audit events, and unmistakable endpoint indication. It may inject
observations such as constrained bandwidth, latency, jitter, endpoint loss,
stale epochs, enumeration failure, or controlled power faults into the normal
reconciler.

Real electrical and network faults always dominate injected observations.
Injection cannot bypass protection or report unsafe hardware as safe.

## Prototype boundary

Linux USB/IP is useful for studying discovery, routing, fencing, recovery, and
measurement. A Linux destination using `vhci-hcd`, a custom host driver, or a
mesh tunnel device is a prototype only. It does not satisfy the transparent
product contract.

The final Cau and Pau terminate and reconstruct physical USB relationships.
They require USB 3 host/device capability, DMA and buffering, deterministic
local responses, and a multi-gigabit network data plane beyond an Arduino Mega
2560. The Mega may provide an independent control and evidence panel; it does
not carry USB payloads.

Compatibility must advance through measured device classes and topology cases.
Early support for HID or controlled bulk devices is not evidence of arbitrary
USB 3 transparency. Hubs, composite devices, mass storage, interrupt-heavy
devices, isochronous audio/video, resets, suspend/resume, and vendor-specific
behavior each require explicit validation.

## Deferred work

The contract intentionally defers:

- controller high availability;
- USB-C and USB Power Delivery;
- claims of universal USB 3 device compatibility;
- final Cau/Pau silicon and FPGA selection;
- qualified PoE++ power and thermal limits;
- physical USB signal-integrity and compliance certification;
- measured profile thresholds and recovery defaults;
- physical bench acceptance.

These are engineering gates, not permission to weaken the topology,
transparency, native-driver, power, network-sharing, or failure semantics above.
