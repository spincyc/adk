# USB 3 network matrix

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. The product is physical computer USB -> Cau -> shared
> switched Ethernet -> Pau -> physical peripheral or user-provided hub.
> Linux USB/IP and virtual-host-controller paths are prototype-only.

Status: research queue  
Research date: 2026-07-27  
Target: arbitrary, exclusive host-to-device assignments across an ordinary
switched Ethernet/IP network

## Conclusion

A useful USB 3 matrix is plausible research, but it is not a raw wire switch
and the Mega 2560 cannot carry its data plane. The final product must
nevertheless be transparent to unmodified Windows and Linux computers: a Cau
reconstructs the exact topology rooted at one Pau port. The ordinary household
network is shared with HDMI and other traffic. ADK and the Mega may provide a
physical control and observation panel. Each route is exclusive; movement
performs `ColdMove`, producing disconnect and fresh enumeration.

The strongest control-plane prototype uses Linux USB/IP rather than new FPGA logic.
It can establish whether routing, device compatibility, latency, security, and
operator semantics are worth custom hardware. A later FPGA/SoC endpoint may
reduce jitter or remove host software dependencies, but a general SuperSpeed
host/device proxy requires licensed controller IP, PHYs, signal-integrity work,
and substantial protocol engineering. USB/IP success does not satisfy product
acceptance because it consumes a virtual host controller and requires Linux
host software.

“Full USB 3” is therefore a compatibility target to measure, not an initial
claim. Bulk storage, HID, cameras, audio, composite devices, hubs, disconnects,
resets, suspend, and faults all need separate acceptance evidence.

## What the matrix means

Let `H1..Hn` be host-facing endpoints and `D1..Dm` be device-facing endpoints.
The controller maintains a partial one-to-one assignment:

```text
host-facing node             IP fabric              device-facing node

H1 virtual host  <------ authenticated route ------> D4 physical xHCI port
H2 virtual host  <------ authenticated route ------> D1 physical xHCI port
H3 disconnected                                      D2 unassigned
                                                     D3 unassigned
```

One physical USB device belongs to at most one host at a time. USB is
host-scheduled and devices enumerate into one host topology; multicast,
simultaneous multi-host attachment, and seamless live migration are not honest
general-purpose semantics. A composite device should initially be leased as
one unit. A hub should initially be rejected or leased with its complete
subtree.

A route change is a transaction:

1. authorize the requested host, device, and policy;
2. stop new transfers and wait for or cancel outstanding requests;
3. detach the device from the old virtual host;
4. reset tunnel state and, only when policy requires it, cycle protected VBUS;
5. bind the device to the new host;
6. let the new host enumerate it normally;
7. expose attached, active, fault, and unassigned state on the physical panel.

Failures leave the device unassigned. They do not silently restore a stale
route.

## Why raw USB cannot cross an ordinary switch

SuperSpeed USB has its own PHY, link, flow-control, topology, and transfer
scheduling. An Ethernet switch forwards Ethernet frames; it does not extend a
USB electrical link. A matrix must terminate USB locally, convert host requests
and device completions into a network protocol, then recreate the expected USB
software view at the remote host.

This distinction explains the product landscape:

- a CAT cable extender may use proprietary signaling and remain point-to-point;
  the cable is not necessarily an Ethernet/IP link;
- a USB device server packetizes USB for a host driver, but often limits
  throughput, operating systems, hubs, devices, or concurrent ownership;
- a KVM-over-IP product commonly optimizes HID and video rather than exposing
  every USB 3 transfer type;
- a physical USB matrix switches SuperSpeed lanes locally, not through ordinary
  packet switches.

USB 3.2 Gen 1 has a 5 Gb/s line rate. One uncompressed, continuously busy link
plus tunnel and Ethernet overhead already makes 1 GbE inadequate. Use 10 GbE
for one serious Gen 1 route and 25 GbE or faster when multiple routes must be
concurrent. Measured payload rate will remain below nominal USB line rate.

## Candidate architectures

### Phase-one software endpoints

At the device side, a Linux node owns a physical xHCI controller and exports a
USB device. At the host side, a kernel virtual host controller imports it. The
Linux USB/IP protocol transports submitted USB request blocks, completions,
unlink operations, setup packets, and isochronous packet descriptors over TCP.

Advantages:

- upstream protocol and kernel implementation;
- ordinary IP switching and routing;
- physical SuperSpeed ports can use commodity xHCI hardware;
- enough visibility to instrument every route and failure;
- lowest-cost way to build the compatibility matrix.

Limits:

- each importing host needs kernel/client support;
- TCP head-of-line blocking and network jitter can harm time-sensitive traffic;
- nominal USB port speed does not imply equivalent network throughput;
- device and OS compatibility must be demonstrated, especially for
  isochronous and vendor-specific devices;
- it does not create transparent attachment for an unmodified host OS.

This is the recommended research baseline.

### Host-facing gadget proxy

A small Linux SoC with a SuperSpeed USB device controller can present a gadget
to an otherwise unmodified host. Linux configfs composes known gadget
functions; FunctionFS or Raw Gadget permits more userspace control.

This is attractive for a class-specific matrix: HID, mass storage, audio, or
video can each have a deliberate proxy. It is not automatically a transparent
proxy for arbitrary devices. The endpoint must reproduce descriptors, control
requests, endpoint behavior, stalls, resets, timing, and class/vendor quirks.
Identity and state cannot safely move while the old host still believes the
device is attached.

Use this architecture only after the USB/IP baseline, beginning with one
explicit class. Do not present cloned vendor/product identity outside a
controlled interoperability lab.

### FPGA/ASIC endpoint

A hardware endpoint needs, per direction:

- a compliant SuperSpeed PHY and connector channel;
- an xHCI-capable host controller on the device-facing side;
- a SuperSpeed device controller on the host-facing side;
- packet buffers, DMA, reset and power control;
- at least 10/25 GbE MAC/PCS and transport logic;
- a processor for enumeration policy, certificates, updates, and telemetry.

Commercial USB host controller IP exists for ASIC/FPGA designs, but licensing,
PHY availability, verification, compliance testing, and board layout dominate
the project. An FPGA is not a shortcut around USB protocol behavior.

A realistic custom endpoint is an FPGA/SoC or application processor plus
commercial controller/PHY devices. First implement a bounded transfer class or
a deterministic accelerator around a software control stack. Do not begin by
writing a universal xHCI and SuperSpeed stack.

## Timing and transfer behavior

USB control, bulk, interrupt, and isochronous transfers have different failure
surfaces:

| Transfer | Initial expectation | Evidence |
|---|---|---|
| Control | Required for enumeration; modest bandwidth | descriptor corpus, reset and retry traces |
| Bulk | Tolerates delay, expects correctness | hashes, sustained rate, cancellation, fault recovery |
| Interrupt | Small scheduled transfers; latency matters | latency histogram and missed-input count |
| Isochronous | Timely delivery matters; no ordinary retry guarantee | loss, late packet, jitter, underrun/overrun counts |

Do not claim isochronous support because a packet format contains isochronous
fields. Reserve switch queues and bandwidth, isolate the test VLAN, measure
tail latency under competing traffic, and test each audio/video device. A
jitter buffer can trade latency for continuity but cannot make an unbounded IP
path behave like a local bus.

Enumeration and hotplug are observable state-machine operations, not transport
details. Record at least:

```text
unassigned -> detaching -> resetting -> attaching -> enumerating -> active
                                                \-> fault
```

Every transition has a deadline and a reason code. Network loss, endpoint
restart, stale controller state, host crash, device removal, overcurrent, and
duplicate assignment all converge on an unassigned or explicitly degraded
state.

## Power and connector policy

USB data routing and device power are separate resources.

- Never connect VBUS from two host-facing ports.
- Device-facing VBUS comes from a protected local supply with current
  measurement, current limiting, and independently controllable load switches.
- A route change does not imply a power cycle. Storage devices need orderly
  detach and may be corrupted by surprise power removal.
- Type-C source/sink and USB Power Delivery roles require dedicated compliant
  controllers; the Mega must not synthesize CC or PD behavior.
- Log overcurrent, undervoltage, attach, detach, and commanded power changes.
- A hardware power button removes endpoint/device power; software shutdown is
  not an emergency stop.

The first prototype should use USB-A device-facing ports and separately powered
endpoint computers. Add Type-C data roles only after the routing model works.

## Security model

A remote USB attachment has the authority of a local peripheral. A malicious
device may impersonate HID, expose a vulnerable class/vendor interface, or
attack a host driver. A malicious client may read storage, cameras, security
tokens, or other private devices.

Required controls:

- mutually authenticated endpoint and controller identities;
- encryption in transit, with replay-resistant sessions;
- default-deny host/device assignments and per-port policy;
- exclusive leases with monotonically increasing generation numbers;
- descriptor and physical-port inventory, not trust based on VID/PID alone;
- isolated management and data VLANs;
- least-privilege endpoint services, signed updates, and rollback protection;
- tamper-evident audit records for route, reset, power, and fault events;
- device-class allowlists and an explicit quarantine route;
- host OS USB authorization where available;
- no Internet exposure of USB/IP or management listeners.

The route controller is authoritative. An Arduino panel proposes commands and
renders state; it does not hold network credentials or override policy.

## ADK and Mega 2560 role

The Mega supplies a useful, inspectable control plane:

- one guarded assignment button or encoder per operation;
- route and fault LEDs or a small display;
- physical device-presence and protected-power telemetry;
- a deterministic model of requested, pending, active, and fault states;
- a serial link to one authenticated Linux controller appliance;
- a hardware heartbeat whose loss leaves routes unchanged or safely
  unassigned according to configured policy.

It never handles SuperSpeed bytes, Ethernet tunnel traffic, certificates,
descriptor emulation, CC negotiation, or Power Delivery. The serial protocol
uses framed messages, sequence numbers, checksums, explicit acknowledgements,
timeouts, and a version. Linux validates every request before acting.

The non-Serial verification path is the panel itself:

- blue: requested route;
- pulsing amber: detach/enumeration transaction;
- green: endpoint and host both report the same active lease generation;
- red: fault, with the route unassigned;
- per-port power LED: measured protected VBUS, independent of logical route.

Named test points expose device VBUS, load-switch enable, and endpoint
heartbeat. LEDs prove controller state; the test points separately prove power
state.

## Existing partial solutions

These products establish pieces of the design, not the complete target:

- [Linux USB/IP protocol](https://docs.kernel.org/usb/usbip_protocol.html)
  documents server export, client import, URB submission/completion, unlink,
  and isochronous descriptors over TCP.
- [Digi AnywhereUSB Plus](https://www.digi.com/products/networking/usb-connectivity/usb-over-ip/anywhereusb-plus)
  offers remote USB 3.1 Gen 1 ports, per-port/group multi-host connectivity,
  and 1/10 GbE models. Digi explicitly warns that network/protocol performance
  is below the direct 5 Gb/s USB rate.
- [Silex DS-700](https://www.silextechnology.com/connectivity-solutions/usb-network-connectivity/ds-700)
  provides one USB 3 device port over Gigabit Ethernet, but does not support a
  downstream hub or USB display adapter on that port.
- [Icron Raven 3104](https://www.icron.com/pdf/icron-usb-3-2-1-raven-3104-datasheet.pdf)
  carries USB 3.1 Gen 1 over 100 m of CAT 6a/7, but it is a proprietary
  point-to-point link; its Ethernet jack is a pass-through, not the USB fabric.
- [Extron USB matrix systems](https://www.extron.com/USB-Matrix-Switchers/prodsubtype-482)
  demonstrate distributed IP-controlled USB switching, primarily in
  professional AV workflows.
- [Black Box USB connectivity](https://www.blackbox.com/products/solutions/usb-connectivity)
  distinguishes CAT/fiber USB 3 extenders from IP-oriented products; its
  Agility IP platform exposes USB 2 rather than a general USB 3 matrix.
- [Synopsys USB 3 xHCI host IP](https://www.synopsys.com/designware-ip/interface-ip/usb/usb-3-0-host-controller.html)
  demonstrates the kind of licensed controller block a custom FPGA/ASIC
  endpoint requires.

The market gap is a documented, vendor-neutral, arbitrary USB 3 assignment
fabric with strong Linux support, deterministic ownership, measurable
isochronous behavior, and open control-plane semantics.

## Staged prototype

### Stage 0 — semantics and simulator

Build a deterministic host/device/route model with no USB hardware. Inject
requests, detach acknowledgements, timeouts, network partitions, endpoint
restarts, and stale generations. Add an ADK panel simulator and CLI.

Acceptance:

- no device is active on two hosts;
- stale acknowledgements cannot resurrect an old route;
- every fault has a visible non-Serial state;
- recorded inputs replay to the same transition log.

Proposed targets:

```text
make usb-matrix-sim
make usb-matrix-test
make usb-matrix-replay TRACE=...
```

### Stage 1 — one Linux USB/IP route

Use two Arch Linux systems on an isolated wired network. Export one known USB 3
bulk-storage test device and import it into one test host. Automate discovery,
attach, detach, inventory, and logs from the CLI.

Acceptance:

- enumeration identity matches the inventory;
- repeated hash/read/write tests are correct;
- detach is orderly and repeatable;
- cable loss and daemon restart do not create ambiguous ownership;
- throughput and latency distributions are recorded, not inferred.

### Stage 2 — two-by-two matrix

Use two clients and two device endpoints. Add the lease controller, mTLS,
generation numbers, policy, audit log, route transaction, and an ADK Mega
panel. Test HID and storage before cameras or audio.

Acceptance:

- all four assignments and repeated cross-switches pass;
- simultaneous conflicting commands choose one winner;
- host crash, device removal, and network partition converge safely;
- logical route LEDs and measured VBUS evidence remain distinguishable.

### Stage 3 — compatibility and load

Add known composite, UVC camera, USB audio, vendor-specific, and hub cases one
at a time. Run competing 10/25 GbE traffic and collect throughput, median,
tail latency, jitter, packet loss, USB errors, underruns, and recovery time.

Publish a compatibility table with exact device, firmware, host kernel,
controller, switch, topology, and test version. “Unsupported” is a valid
result.

### Stage 4 — class-specific gadget endpoint

Choose one class justified by Stage 3. Present it through a Linux SuperSpeed
UDC using configfs/FunctionFS, retaining the same route controller. Compare
host transparency, latency, failure behavior, and engineering cost with
USB/IP.

Relevant kernel references:

- [USB gadgets through configfs](https://docs.kernel.org/usb/gadget_configfs.html)
- [USB gadget testing](https://docs.kernel.org/usb/gadget-testing.html)
- [Raw Gadget](https://docs.kernel.org/usb/raw-gadget.html)

### Stage 5 — hardware acceleration decision

Only now select FPGA/SoC hardware. Freeze the supported transfer classes,
bandwidth, port count, connector/power roles, network QoS, latency budget, and
compliance plan. Prototype one direction with evaluation boards and licensed
IP before designing a PCB.

## Research questions

1. Is mandatory host software acceptable, or must each host see a physical
   gadget with no installed client?
2. Is the assignment unit a complete physical device, a hub subtree, or a
   deliberately supported interface class?
3. Which device corpus defines “full USB 3” for the project?
4. How many simultaneous 5 Gb/s routes and what oversubscription are required?
5. What measured interruption is acceptable during reassignment?
6. Must isochronous cameras/audio work under load, or can the first release be
   control/bulk/interrupt only?
7. Are routes confined to one managed Layer-2 site, or expected to cross a
   routed/WAN path?
8. Which Type-C data and power roles, if any, are required after USB-A proves
   the architecture?

The first hard decision is host transparency. If installed Linux client
software is acceptable, USB/IP gives the project a credible beginning. If not,
the project becomes a much larger USB gadget-proxy and compatibility effort.

## Primary references

- [USB-IF USB 3.2 specifications and compliance material](https://www.usb.org/usb-32)
- [USB-IF document library](https://www.usb.org/documents)
- [Linux USB/IP protocol](https://docs.kernel.org/usb/usbip_protocol.html)
- [Linux USB gadget configfs](https://docs.kernel.org/usb/gadget_configfs.html)
- [Linux USB gadget testing](https://docs.kernel.org/usb/gadget-testing.html)
- [Linux Raw Gadget](https://docs.kernel.org/usb/raw-gadget.html)
- [USB Type-C connector specification](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25)
- [USB Type-C source power tests](https://www.usb.org/sites/default/files/USB-C%20Source%20Power%20Test%20Specification%202021%2005%2024.pdf)
