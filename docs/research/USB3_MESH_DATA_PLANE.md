# USB 3 mesh data plane

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. The final data plane connects an unmodified Windows or
> Linux computer to a Cau and an exact physical topology to a Pau across the
> shared household network. Linux USB/IP is prototype-only.

> **Controller decision:** the initial data plane accepts routes from one
> durable authoritative controller. Controller HA, replication, consensus, and
> automatic leader failover are deferred; endpoints cannot promote themselves.

> **Prototype decision:** Linux USB/IP may test orchestration and measurements,
> but cannot satisfy transparent product acceptance.

Status: architecture research  
Research date: 2026-07-27  
Scope: dynamically assign USB consumers to USB providers across an ordinary
switched network

## Decision

Build a mesh of independently registered endpoint nodes, not a fixed central
USB switch. Every physical host-facing port and device-facing port is an
inventory object with a stable identity, capabilities, health, and current
lease. The controller may reconfigure any compatible consumer/provider pair at
runtime. The data plane remains a set of point-to-point sessions across the
shared packet fabric.

“Mesh” describes reachability and orchestration:

```text
consumer ports       switched IP fabric       provider ports

C1 -----------\
C2 ------------+---- any authorized pair ---- P1
C3 ------------+---- any authorized pair ---- P2
               \---- any authorized pair ---- P3
```

It does not make one physical USB device concurrently attachable to arbitrary
hosts. USB remains host-controlled and stateful. The default assignment unit
is one complete physical device, exclusively leased to one consumer. Explicit
class-aware services may later virtualize a shareable function, but that is a
different contract from transparent USB forwarding.

Use `ConsumerPort` for the USB view presented to a computer and `ProviderPort`
for the locally hosted physical peripheral. Avoid “source” and “destination”
in the protocol: USB transfers are bidirectional, so those words reverse
meaning with each transaction.

## Dynamic node model

Each appliance registers one or more ports. A port advertises facts rather
than a permanent role:

- node and physical-port identity;
- `Consumer`, `Provider`, or hardware-supported `DualRole` capability;
- USB generation, connector, controller, and transfer-type support;
- protected VBUS and Type-C/Power Delivery capabilities, if any;
- network interfaces, measured capacity, and supported transports;
- software/firmware version and current health;
- permitted device classes and security zone.

A dual-role declaration is valid only when the hardware controller and
connector power design support it. For example, AMD documents two independent
USB 3 dual-role controllers in Zynq UltraScale+ MPSoC, each configurable as
host or device. A normal xHCI add-in card or USB-A hub is not made dual-role by
software. See the [AMD USB 3.0 controller documentation][amd-drd].

Discovery never grants ownership. The authoritative controller selects a
compatible pair and issues a fenced route transaction:

```text
discover -> reserve -> quiesce old route -> detach -> establish tunnel
         -> enumerate -> confirm matching epoch -> active
```

Both endpoints persist the route identifier and monotonically increasing epoch.
Data packets, acknowledgements, and health reports carry that identity. A
stale endpoint cannot reactivate an expired route. A failed handoff leaves the
provider unassigned; it does not restore an old route without a new
authorization.

The control plane scales with nodes and ports. The data plane scales with
active routes and their traffic. No central appliance should forward payload
bytes. Phase one has one durable controller. A later replicated design still
needs one logical lease authority before multiple processes may participate.

## Data-plane candidates

| Data plane | Host modification | Generality | Mesh fit | Main constraint |
|---|---|---|---|---|
| Linux USB/IP | Kernel/client support | Broad USB request forwarding | Strong prototype | TCP latency, jitter, and compatibility |
| Commercial USB-over-IP | Vendor driver | Vendor-qualified set | Useful benchmark | Closed protocol and product policy |
| Class-specific gadget proxy | None beyond normal class driver | One designed class | Strong for bounded services | Not transparent to arbitrary devices |
| Custom FPGA/SoC proxy | Driver or physical gadget | Potentially broad | Long-term research | Controller IP, PHY, buffering, verification |
| Proprietary CAT/fiber extender | None | Often broad | Weak | Usually fixed point-to-point, not IP switching |

### Linux USB/IP

Linux USB/IP terminates the physical device at a server and presents it through
a virtual host controller at a client. Its documented protocol transports USB
request submissions, completions, cancellation, setup fields, and isochronous
packet descriptors over TCP. This matches the mesh boundary well: every
provider can export inventory and every consumer can import an authorized
device. See the [Linux USB/IP protocol][usbip].

USB/IP is the recommended prototype data plane because it tests the difficult
questions without custom electronics:

- arbitrary node registration and route selection;
- detach, re-enumeration, cancellation, and endpoint failure;
- real device/driver compatibility;
- throughput and tail-latency behavior under switch contention;
- lease fencing and authorization around an existing transport.

It is not a proof of full USB 3 behavior. TCP retransmission can create
head-of-line delay, and a protocol field for isochronous transfers does not
prove that a camera or audio interface meets its deadline. The documented
USB/IP wire protocol also does not define endpoint authentication or
encryption; deploy it only inside an authenticated tunnel or isolated lab
network. This security conclusion is an inference from the documented
protocol, not a claim about every distribution's surrounding tools.

### Commercial USB-over-IP

Commercial products show that dynamic assignment and multiport scale are
practical, while also showing the performance gap between a connector label
and an end-to-end service. Digi AnywhereUSB Plus offers 2-, 8-, and 24-port
USB 3.1 Gen 1 appliances, per-port or per-group multi-host assignment, and
1/10 GbE variants. Digi explicitly says throughput is below a direct 5 Gb/s
connection because of protocol and network constraints. It also encrypts and
authenticates its USB-over-IP traffic by default. See the
[Digi product and specifications][digi].

These are valuable compatibility and operational benchmarks. They are not a
foundation for an open mesh unless the vendor exposes the required routing,
fencing, telemetry, and automation interfaces. “Multi-host” must also be read
carefully: configurable access to independent ports does not necessarily mean
simultaneous shared ownership of one stateful USB device.

Icron's Raven family illustrates the other category. The Raven 3104 carries
USB 3.1 Gen 1 over a point-to-point CAT 6a/7 link; its Ethernet connection is a
pass-through. A CAT cable and RJ45 connector therefore do not establish a
switched Ethernet data plane. See the [Raven 3104 data sheet][icron-3104].

### Class-specific gadget proxy

A provider node may interpret a known device class and a consumer node may
reconstruct a fresh USB gadget for the host. Linux configfs creates gadgets
from explicitly selected functions; Raw Gadget exposes lower-level endpoint
and control-event access. See the kernel documentation for
[configfs gadgets][configfs] and [Raw Gadget][raw-gadget].

This can be excellent for a bounded mesh service:

- a read-only sensor feed;
- a deliberately modeled HID control surface;
- a controlled mass-storage image;
- a designed camera or audio stream with known buffering.

It is not a universal identity-preserving proxy. A transparent implementation
must reproduce descriptors, endpoint behavior, control requests, reset and
stall semantics, timing, and vendor-specific quirks. Once the proxy interprets
a class, it should expose a new service identity and documented semantics
rather than pretending every detail of the original peripheral survived.

### Custom FPGA/SoC endpoint

A custom transparent endpoint needs a USB host controller beside the physical
peripheral, a USB device controller beside the computer, compliant
SuperSpeed PHYs, DMA and packet memory, a network MAC, and a processor for
enumeration, policy, updates, and certificates. A peripheral controller alone
is insufficient. Infineon's FX3, for example, supplies a 5 Gb/s USB peripheral
controller with 32 endpoints and a programmable interface to a processor,
FPGA, or ASIC; it is useful on the consumer-facing gadget side, not as the
complete two-sided proxy. See the [Infineon FX3 overview][fx3].

An FPGA can reduce copy cost, timestamp transfers, schedule queues, and provide
bounded buffering. It cannot remove host/device protocol state or the need for
software policy. Start with a dual-role application processor or SoC and
accelerate measured bottlenecks. Only design a board after the software mesh
has fixed:

- supported USB generations and transfer classes;
- maximum active routes per node;
- buffer and latency budgets;
- connector and protected-power roles;
- network link rates and QoS policy;
- compliance and interoperability test plan.

## Capacity and latency

USB-IF defines USB 3.2 signaling rates of 5 Gb/s, 10 Gb/s, and 20 Gb/s. Those
are link rates, not application payload guarantees. See the
[USB-IF USB 3.2 overview][usb32].

A conservative fabric plan treats every route as an independently admitted
flow:

| USB capability | Minimum credible access link | Planning note |
|---|---:|---|
| 5 Gb/s Gen 1 | 10 GbE | One busy route plus encapsulation and control margin |
| 10 Gb/s Gen 2 | 25 GbE | 10 GbE cannot carry a 10 Gb/s source plus overhead |
| 20 Gb/s Gen 2x2 | 25/40/50 GbE | Benchmark directionality and reserve margin |
| Several concurrent routes | Sum admitted envelopes | Do not rely on connector count or averages |

These are engineering starting points, not measured guarantees. Real devices
rarely sustain their signaling rate, while several bursty devices can still
converge on the same uplink. Admission control must consider both directions,
switch oversubscription, MTU, encapsulation, encryption, and retransmission.

Track latency by transfer class:

- control: enumeration completion, reset, and retry distributions;
- bulk: goodput, fairness, integrity, and cancellation time;
- interrupt: median and worst-case completion latency;
- isochronous: late/lost packet counts, jitter, underruns, and overruns.

Publish percentiles and failure counts under competing traffic. Average
throughput cannot establish interactive or isochronous correctness. Reserve
queues and bandwidth only after measurement; QoS cannot repair an
under-provisioned link or unbounded routed path.

## Mesh invariants

The controller and every data-plane adapter must preserve:

1. one physical provider device has at most one active consumer lease;
2. one port participates in at most one route unless its explicit capacity
   model allows several independent downstream devices;
3. active requires matching route identifier and epoch at both endpoints;
4. detach completes or times out before a new consumer enumerates;
5. stale packets and acknowledgements cannot mutate the current route;
6. loss of lease authority cannot create a new route;
7. endpoint restart is fail-closed until inventory and epochs reconcile;
8. power state and logical assignment are separate resources;
9. every route has an admitted bandwidth envelope and observable health;
10. every mutation and fault enters the tamper-evident audit record.

The product preserves a user-provided real hub as one atomic topology root. A
prototype may reject hubs until it can preserve and fence the whole subtree;
it must not flatten, split, or synthesize the topology. The first prototype
should reject hubs or lease the complete subtree. Per-interface splitting is a
different class-specific service and is not transparent product behavior.

## Recommended progression

1. Extend the deterministic controller model from fixed host/device names to
   registered nodes, ports, capabilities, health, and fenced device ownership epochs.
2. Run a two-consumer/two-provider USB/IP mesh on isolated Arch Linux systems.
3. Add and remove endpoint nodes while routes are idle; then test controlled
   loss and restart while routes are active.
4. Measure known bulk, interrupt, and composite devices on 10 GbE.
5. Add 25 GbE before claiming a Gen 2 service or concurrent saturated routes.
6. Evaluate one commercial appliance with the same corpus and evidence format.
7. Prototype one class-specific SuperSpeed gadget only when measurements show
   that host software is the limiting requirement.
8. Decide on FPGA/SoC acceleration from profiles, not from nominal link rates.

The Mega 2560 remains outside this data plane. It may provide the local
request, route, traffic, and fault display, but a Linux/SoC controller
authenticates commands and the endpoint appliances carry every USB byte.

## Open decisions

1. Which host operating systems and device classes should qualify the later
   transparent proxy/appliance milestone?
2. Is one physical peripheral always exclusive, or are any named classes
   required to support intentional multi-client virtualization?
3. What is the first compatibility corpus: storage, HID, cameras, audio,
   composite devices, vendor-specific devices, or hubs?
4. How many simultaneous routes, at which USB generations, define the first
   scale target?
5. Is isochronous support a first-release requirement?
6. Is the fabric confined to one managed Layer-2 site, or must routes cross a
   WAN?
7. Are endpoint electrical roles fixed per port, or is genuine dual-role
   hardware required at every node?

The first decision remains host transparency. It determines whether the next
prototype is primarily orchestration around USB/IP or a much larger physical
gadget-proxy program.

## Primary references

- [USB-IF USB 3.2 overview][usb32]
- [USB-IF USB 3.2 Revision 1.1][usb32-spec]
- [Linux USB/IP protocol][usbip]
- [Linux configfs USB gadgets][configfs]
- [Linux Raw Gadget][raw-gadget]
- [AMD Zynq UltraScale+ USB 3.0 dual-role controllers][amd-drd]
- [Infineon EZ-USB FX3 peripheral controller][fx3]
- [Digi AnywhereUSB Plus][digi]
- [Icron Raven 3104 data sheet][icron-3104]

[amd-drd]: https://docs.amd.com/r/2021.2-English/ug1137-zynq-ultrascale-mpsoc-swdev/USB-3.0
[configfs]: https://docs.kernel.org/usb/gadget_configfs.html
[digi]: https://www.digi.com/products/networking/infrastructure-management/usb-connectivity/usb-over-ip/anywhereusb
[fx3]: https://www.infineon.com/products/universal-serial-bus/usb-3-2-peripheral-controllers/ez-usb-fx3-usb-5gbps-peripheral-controller
[icron-3104]: https://www.icron.com/assets/usb-3-2-1-raven-3104-turnkey-datasheet.pdf
[raw-gadget]: https://docs.kernel.org/usb/raw-gadget.html
[usb32]: https://www.usb.org/usb-32-0
[usb32-spec]: https://www.usb.org/document-library/usb-32-revision-11-june-2022
[usbip]: https://docs.kernel.org/usb/usbip_protocol.html
