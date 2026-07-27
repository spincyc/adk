# USB mesh endpoint hardware

Status: research plan; no endpoint has passed physical acceptance  
Scope: physical peripheral mesh over the shared managed household network

## Nomenclature

USB's words `host` and `device` describe electrical protocol roles, while
`source` and `destination` change meaning when a route is viewed from the
computer or peripheral. This document instead uses fixed physical names:

| Name | Location and invariant role |
|---|---|
| Computer | An unmodified Windows or Linux basement computer that must enumerate remote peripherals |
| Computer Attachment Unit (Cau) | The computer-side appliance or card that presents remote peripherals |
| ComputerPort | One physical Cau upstream connection consuming one computer USB port |
| Peripheral Attachment Unit (Pau) | The room-side appliance with physical host ports for keyboards, cameras, storage, and other peripherals |
| PeripheralPort | One named physical Pau port and exactly the real device or real hub subtree plugged into it |
| Controller | The ordinary Linux service that authorizes routes and assigns epochs; never carries USB data |
| Upstream | Toward the computer |
| Downstream | Toward a peripheral |
| Prototype importer | A Linux computer using `vhci-hcd`; not a Cau |

`Cau` and `Pau` are document abbreviations, not proposed public C++ type names.
Mesh identities name the unit and a stable port. They never derive identity
from an IP address, Linux bus ID, USB/IP port, or connector label alone.

A Pau may contain several independently identified, powered, fenced, and
routed `PeripheralPort` roots. One route binds one root to one `ComputerPort`;
that `ComputerPort` consumes exactly one physical USB port on its computer.
Neither appliance combines roots. A Cau with several active routes needs the
same number of physical `ComputerPort` cables.

The Cau adds no hub, hidden management function, or extra USB device. A
directly attached peripheral is reconstructed as that peripheral. If several
peripherals are needed on one route, the user plugs a real hub into one Pau
root; the Cau reconstructs that observed hub and its real subtree without
adding ports or changing topology. A hub subtree is the single routed unit and
is never split across computers.

## Decision

The final product target is:

```text
unmodified basement computer
    <-> physical upstream attachment
        <-> Cau
            <-> switched Ethernet mesh
                <-> Pau
                    <-> physical downstream USB ports
                        <-> room peripherals
```

### Baseline Pau

The first Pau has four USB 3 Type-A host `PeripheralPort` roots. All four roles
are fixed:

- each connector is always a downstream USB host port;
- each root has a stable identity independent of Linux bus numbering;
- each root has its own route epoch and fence;
- each root has its own current-limited protected 5 V VBUS switch;
- each root has independent enable, fault, voltage, and discharge evidence;
- each root is routed and power-cycled without changing another root;
- each active root requires a separate Cau `ComputerPort` and a separate
  physical USB port on the selected computer.

The baseline does not use USB-C connectors, dual-role ports, alternate modes,
or USB Power Delivery. Those are deferred profiles and cannot be inferred from
future connector substitutions. A Type-A root supplies protected 5 V VBUS
within its declared current budget; it never negotiates a higher voltage.

The Pau power manager admits a route only when the negotiated PoE input class,
measured Pau reserve, conversion limits, and the declared budgets of every
enabled root can cover the resulting aggregate load. Admission is fail-closed:
insufficient or ambiguous budget leaves the requested root off and fenced.
Per-port current limiting does not replace aggregate admission, and aggregate
headroom does not replace per-port protection.

The controller is an ordinary Linux service. It stores desired routes, assigns
fencing epochs, reconciles endpoint state, and records the audit journal. It
does not carry USB transfers and needs no Arduino or special interface card.
An existing Arch Linux desktop is the first controller; a small x86 Linux box
may replace it later without changing the protocol.

USB/IP is the control and transport prototype:

```text
physical USB device
    -> prototype exporter USB host controller
        -> USB/IP over switched 10 GbE
            -> prototype importer virtual host controller
                -> Linux driver and application
```

It proves discovery, identity, authorization, fencing, detach-before-attach,
reconciliation, and representative transfer behavior. It is not the final
computer-to-Cau physical topology.

The first concrete USB/IP prototype candidate is a barebone Minisforum MS-01
fitted with supported memory and NVMe storage. Its published interfaces
include two 10 Gb/s SFP+ ports, two USB 3.2 Gen 2 Type-A ports, one USB 3.2
Gen 1 Type-A port, and a PCIe expansion slot. It is not proposed as the final
Cau or Pau. Buy one evaluation unit first and record its USB controller
topology, SFP+ module compatibility, thermals, suspend behavior, IOMMU groups,
Arch kernel support, and sustained USB/IP results before buying more.

Use two identical appliances and a managed 10 GbE SFP+ switch for the USB/IP
prototype. One exports a peripheral and one imports it into Linux. Keep the
controller on a separate Linux computer when testing controller-loss behavior.
It may share an endpoint only for an early desk experiment.

## What the USB/IP prototype proves

USB/IP does not produce a voltage-bearing USB socket at the destination. The
prototype importer kernel creates a virtual host-controller attachment.
Software on that Linux importer sees the remote peripheral as an attached USB
device, subject to driver and USB/IP compatibility. The peripheral remains
physically plugged into the prototype exporter.

This satisfies:

- exported USB peripheral to Ethernet to an importing Linux application;
- dynamic reassignment among enrolled Linux importers;
- one authoritative route per physical peripheral;
- many exporters and prototype importers in the mesh.

It does not satisfy:

- a physical Cau connection to an arbitrary computer;
- an unmodified Windows, macOS, console, instrument, or embedded host;
- transparent electrical extension of the original SuperSpeed link;
- guaranteed compatibility with every USB transfer type or device.

Those are distinct product boundaries. Documentation and CLI output must call
phase-one destinations **virtual Linux import slots**, not a Cau, physical USB
output, or final-product destination.

## Why the Mega 2560 is not the data appliance

The Mega 2560 is a 16 MHz, 8-bit microcontroller with 8 KiB SRAM. Its onboard
USB connection is implemented through a separate ATmega16U2 serial interface;
it is not a general USB 3 host/device data path. USB 3.2 signaling rates begin
at 5 Gb/s. The Mega cannot terminate a SuperSpeed device, buffer its traffic,
run the Linux USB/IP stack, or drive 10 GbE.

The Mega remains useful in the design:

- buttons request a route through the authenticated Linux controller API;
- LEDs or a display show requested, fenced, attaching, active, and fault state;
- a protected enable input can require local operator presence;
- named test points provide non-Serial evidence of controller reachability and
  route state.

The Linux controller is authoritative. The Mega never assigns epochs, retains
leases, bypasses authorization, or interprets a dark status LED as permission
to attach.

## Why Raspberry Pi 5 is not the baseline

A Raspberry Pi 5 is useful for low-cost USB/IP experiments. It has two USB 3
ports capable of simultaneous 5 Gb/s operation, but its onboard Ethernet is
1 Gb/s. Its exposed expansion link is PCIe 2.0 x1, so adding nominal 10 GbE
does not create a balanced 10 Gb/s endpoint. USB traffic, storage, Ethernet,
and CPU overhead still share constrained paths.

Pi 5 USB gadget mode uses the USB-C power/OTG connection and makes that port
exclusive to gadget operation. Linux gadget configuration also requires a USB
Device Controller and exposes selected gadget functions; it does not
automatically recreate an arbitrary remote physical USB 3 device. Therefore a
Pi 5 may be a functional-test node for keyboards, serial adapters, and modest
storage, but it is not the full-USB-3 or 10-GbE reference appliance.

## Final computer attachment options

The Cau is the hard endpoint. It must cause an unmodified computer to enumerate
devices physically attached to one or more Paus. The required physical-USB
path and one standards-only research comparison need separate feasibility
programs.

### Physical USB device-role proxy

This option matches the desired external cable topology:

```text
computer USB host port
    <-> one Cau ComputerPort and SuperSpeed device controller/PHY
        <-> exact reconstruction of one routed PeripheralPort subtree
            <-> Ethernet
                <-> Pau USB host controller
                    <-> one physical PeripheralPort
```

Linux configfs can compose supported gadget functions such as mass storage,
HID, networking, or video when the hardware exposes a suitable USB Device
Controller. That is class emulation, not transparent forwarding of every
descriptor, endpoint behavior, timing dependency, reset, power event, or
vendor command.

A generic proxy would have to reconstruct dynamic hubs, descriptors, control
state, endpoint behavior, reset, suspend, hotplug, power reporting, and
class/vendor-specific timing across a nondeterministic network. USB response
deadlines cannot wait for a switched-network round trip, so the Cau must
answer, schedule, or buffer locally. Merely forwarding packets cannot preserve
link-level timing. A Cau may therefore support a bounded list of device
classes without ever becoming a faithful arbitrary-USB extension.

This path likely needs:

- an SoC or FPGA with a SuperSpeed device controller and PHY;
- exact direct-device or real-hub topology presentation to standard drivers;
- deterministic descriptor and control-transfer reconstruction;
- bounded buffering and explicit isochronous scheduling;
- electrical role, cable, VBUS, reset, suspend, and disconnect handling;
- per-device-class compatibility and adversarial testing;
- USB-IF compliance work for a product claim.

This option remains research until a prototype proves a direct HID, direct
bulk storage, one isochronous device, and a user-supplied real hub subtree
without a custom computer driver. Do not describe class-specific gadget
recreation as generic USB forwarding. The Cau must not make several routed
ports appear behind a synthetic hub.

### PCIe or Thunderbolt xHCI research alternative

Another technically credible abstraction is a standards-compatible xHCI host
controller:

```text
computer PCIe root complex
    <-> Cau PCIe endpoint or Thunderbolt PCIe enclosure
        <-> virtual xHCI controller
            <-> Ethernet
                <-> Pau physical xHCI controller
                    <-> room peripherals
```

The computer's ordinary xHCI driver would see ports and devices. The Cau would
implement the controller's register, ring, interrupt, ordering, hotplug, and
power-management behavior while transporting USB operations to Paus. This
aligns the abstraction with what the operating system expects and avoids
pretending many arbitrary peripherals are one USB gadget.

It is still a major hardware and firmware project. A standards-compatible
virtual xHCI implementation, PCIe endpoint, DMA isolation, resets, interrupt
delivery, bounded completion behavior, and network-partition semantics all
need proof. A Thunderbolt enclosure may preserve an external appliance form,
but host security authorization, platform support, and hotplug policy mean it
cannot yet be called universally unmodified.

This is not the current product path. An internal card violates the required
physical USB attachment. Thunderbolt/USB4 is not a plain USB device connection
and may require platform authorization. A custom xHCI driver is prohibited.
Keep this option only as a research comparison unless a standards-compliant
external implementation demonstrably loads through inbox Windows and Linux
drivers with no Cau package, service, or kernel module installed. Even then,
changing the physical attachment requires an explicit product decision.

## Windows and Linux host contract

The final Cau must support both Windows and Linux computers with no Cau driver,
service, management agent, or package installed. The computer uses only inbox
USB hub/class infrastructure and the peripheral's ordinary class or vendor
driver. Route control occurs elsewhere. A peripheral that normally requires
its vendor driver may still require that ordinary driver; the project adds no
computer-side software.

| Cau presentation | Windows path | Linux path | Compatibility position |
|---|---|---|---|
| Physical USB hub/device reconstruction | Inbox USB hub and applicable class/vendor driver | Mainline USB hub and applicable class/vendor driver | Required product path; compatibility must be proven per peripheral |
| Standards-compatible PCIe xHCI | Inbox xHCI stack only | Mainline `xhci-pci` only | Research comparison; wrong physical connector |
| Thunderbolt/USB4-tunneled PCIe xHCI | Inbox PCIe/xHCI only, after firmware/platform authorization | Inbox PCIe/xHCI only, after platform authorization | Research comparison; not universally plain or authorization-free |
| Custom virtual-host driver | Prohibited | Prohibited | Not a prototype fallback or product profile |

No-driver is a release gate, not a preference. Test machines must not use test
signing, disabled Secure Boot, an out-of-tree module, a filter driver, a
background bridge service, or a manually installed Cau INF. If a spike needs
one, it has disproved that spike as a final interface while remaining useful
research.

The Pau may run Linux internally in either endpoint experiment. Its operating
system is not the computer-side compatibility contract.

## Route movement is unplug then plug

Moving a peripheral between computers is deliberately break-before-make. The
old Cau first presents a real logical disconnect to its computer and confirms
that the route is inactive. Only then may the controller advance the fencing
epoch and authorize the new Cau to present a connect event. The new computer
enumerates the peripheral from reset as though a person unplugged its cable
from the old computer and plugged it into the new one.

The mesh does not preserve open file handles, mounted filesystems, application
sessions, driver state, pending transfers, audio/video streams, authentication
sessions, or device-private state across a move. Application-aware quiesce may
reduce damage, but it cannot turn a move into live migration. If disconnect
confirmation is absent or ambiguous, the new attachment remains fenced off.
At no time may two computers observe the same exclusive physical peripheral as
active.

For a bus-powered topology, the default move sequence is:

1. quiesce when the peripheral class offers a safe operation;
2. present disconnect on the old Cau and confirm it inactive;
3. disable the routed Pau port's protected VBUS supply;
4. measure VBUS discharge below the selected USB-compliant threshold;
5. advance the fencing epoch only after disconnect and discharge evidence;
6. re-enable current-limited VBUS and observe the physical subtree anew;
7. authorize the new Cau to present a fresh plug and enumeration.

The discharge threshold, timeout, measurement accuracy, current limit, inrush
behavior, and fault response remain hardware selection gates. Software must
not infer discharge from elapsed time alone. Failure to confirm discharge
leaves the route powered off and fenced.

An externally powered hub or peripheral cannot be cold-cycled by switching Pau
VBUS. The Pau still disables its port, requests a standards-compliant port or
device reset where supported, discards all prior observations, and requires
fresh topology observation and computer enumeration. Documentation must label
this **reset and re-enumeration**, not a cold power cycle. A stubborn
externally powered device may require the user to remove its power.

## Shared mesh wire protocol

USB/IP may supply early transfer traces, but the final endpoint protocol must
not expose Linux USB/IP bus IDs, `vhci-hcd` ports, or Linux driver structures
as durable wire semantics. Both Cau implementations consume the same
versioned, endian-defined mesh protocol:

- stable unit, physical-port, peripheral, route, and controller identities;
- descriptor and topology snapshots with explicit version and bounds;
- control, bulk, interrupt, and isochronous operations with sequence IDs;
- route epoch and operation fence on every state-changing message;
- hotplug, reset, suspend, resume, cancel, timeout, and disconnect events;
- bounded payload, queue, credit, retry, and completion rules;
- authenticated encryption, replay rejection, and peer authorization;
- desired and observed state suitable for deterministic audit and replay.

Transport adapters translate this protocol to USB/IP during the Linux
prototype, a USB device-role engine in one Cau spike, and virtual xHCI rings in
the other. The controller authorizes topology but stays out of the transfer
path.

The later **transparent profile** means the same supported peripheral can move
between a Windows computer and a Linux computer over a physical USB Cau
connection, with no Cau software installed, while preserving its ordinary
vendor/class driver behavior. It is an acceptance target, not a current claim.
The compatibility matrix must name exact operating-system builds, controllers,
hubs, transfer types, and peripherals. Unsupported classes and timing limits
remain visible. “Full USB 3” remains a research goal until this evidence
supports it; connector speed alone is not transparency.

### Selection gate

Do not select endpoint silicon merely because a board exposes USB-C, PCIe, or
10 GbE connectors. First use USB/IP traces to identify transfer types, latency,
bandwidth, hotplug, reset, and failure requirements. Then run these spikes:

1. present one reconstructed device through physical SuperSpeed USB;
2. present a user-supplied real hub and its changing subtree without adding a
   synthetic hub or port;
3. exercise the physical-USB Cau from clean Windows and Linux computers;
4. prove no Cau software or non-inbox host infrastructure was installed;
5. compare its device coverage, latency, recovery, hardware complexity, and
   compliance burden with an inbox-only virtual-xHCI research spike;
6. narrow the stated compatibility target if arbitrary reconstruction cannot
   be proven; do not substitute a custom driver.

## Initial physical bill of materials

| Quantity | Item | Role |
|---:|---|---|
| 1 | Existing Arch Linux x86-64 computer | Authoritative controller |
| 2 | MS-01 evaluation appliances | USB/IP exporter and prototype importer |
| 1 | Managed 10 GbE SFP+ switch | Isolated data network |
| 4 | Matched supported SFP+ modules or DACs | Two endpoint links plus spares |
| 1 | Externally powered USB 3 hub | Controlled peripheral power budget |
| 1 each | Keyboard, serial adapter, flash drive, UVC camera | Increasing compatibility cases |
| 1 | Mega 2560, buttons, resistor-limited LEDs/display | Optional control/evidence panel |
| TBD | Cau device-role and PCIe/xHCI development platforms | Product-path and comparison spikes |
| 4 channels | Per-port current-limited VBUS switches, discharge paths, and monitors | Four Pau roots |

## Pau power reference

PoE is a candidate Pau supply, not permission to omit per-port USB protection.
TI's TIDA-00617 is a useful reference boundary: an IEEE 802.3at Class 4 powered
device controller and isolated converter producing 5 V at up to 5 A. It is not
an adopted Pau schematic. The final design must budget the Pau processor,
Ethernet PHY, USB controller, conversion loss, cable loss, and worst-case
declared USB loads before selecting a PoE class.

Each `PeripheralPort` then needs its own current-limited high-side switch,
fault observation, intentional discharge path, and independent VBUS
measurement. The TPS2560/TPS2561 datasheet supplies a reference for USB
current-limited distribution switches, but the selected circuit must also
prove controlled discharge and voltage measurement; the named switch alone
does not establish those properties.

SFP+ optical modules and passive DACs do not deliver PoE. A PoE-powered Pau
therefore needs a compatible copper PoE data interface or a separately
engineered PoE power input. The physical network choice must show that path
explicitly instead of assuming the current SFP+ prototype cable supplies
power.

Storage testing begins with a disposable filesystem and expendable media.
Never move a mounted writable device without an application-aware quiesce and
confirmed detach. USB/IP and management listeners stay on the isolated lab
network.

## Arch Linux bring-up

Install stock repository packages on the controller and both endpoints:

```sh
sudo pacman -Syu --needed usbip ethtool iproute2 usbutils pciutils
```

Inspect every endpoint before routing:

```sh
uname -r
usbip version
ip -brief link
ethtool <data-interface>
lsusb -t
lspci -nnk
```

On a prototype exporter:

```sh
sudo modprobe usbip-host
sudo systemctl enable --now usbipd.service
usbip list --local
sudo usbip bind --busid=<bus-id>
```

On a prototype importer:

```sh
sudo modprobe vhci-hcd
usbip list --remote=<source-address>
sudo usbip attach --remote=<source-address> --busid=<bus-id>
usbip port
```

These commands establish only a manual laboratory baseline. The mesh adapter
must later translate stable controller identities into current bus IDs and
USB/IP ports, enforce fencing, detach before move, impose deadlines, and
reconcile observed kernel state. Do not expose `usbipd` to the Internet.

## Acceptance sequence

1. Verify one low-rate HID route on an isolated direct link.
2. Restart the exporter, importer, and controller independently; prove
   fail-closed fencing and explicit recovery.
3. Move one device repeatedly between two virtual import slots.
4. Add concurrent devices while proving one active owner per physical device.
5. Measure throughput, latency, jitter, CPU load, and detach behavior for
   serial, storage, bulk camera, interrupt, and isochronous cases separately.
6. Record unsupported devices and failure modes; do not generalize from one
   successful class.
7. Run the physical-USB Cau spike from clean Windows and Linux computers and
   prove no Cau software was installed.
8. Move a device between two computers; observe disconnect on the old computer
   before connect and fresh enumeration on the new computer.
9. Keep PCIe/xHCI as an inbox-only research comparison, not a substitute for a
   failed physical-USB path.
10. Promote no interface until its host and compatibility claims pass the
    declared cross-platform profile.

No physical result is claimed until the endpoint models, firmware, kernel,
switch, optics, commands, instruments, observations, and date are recorded.

## Primary references

- [Linux USB/IP commands and client/server roles][arch-usbip-man]
- [Arch Linux `usbip` package][arch-usbip-package]
- [Linux USB gadget configuration and UDC requirement][kernel-gadget]
- [USB-IF USB 3.2 signaling rates][usb32]
- [Minisforum MS-01 published interfaces][ms01]
- [Raspberry Pi 5 product brief][pi5-brief]
- [Raspberry Pi I/O controller topology][pi-io]
- [Raspberry Pi USB gadget-mode hardware notes][pi-gadget]
- [Arduino Mega 2560 Rev3 specifications][mega]
- [Windows inbox USB class drivers][windows-usb-classes]
- [TI 5 V/5 A Class 4 PoE powered-device reference][ti-poe]
- [TI USB current-limited power-distribution switches][ti-usb-power]

[arch-usbip-man]: https://man.archlinux.org/man/usbip.8
[arch-usbip-package]: https://archlinux.org/packages/extra/x86_64/usbip/
[kernel-gadget]: https://docs.kernel.org/usb/gadget_configfs.html
[usb32]: https://www.usb.org/usb-32-0
[ms01]: https://store.minisforum.com/products/minisforum-ms-01
[pi5-brief]: https://datasheets.raspberrypi.com/rpi5/raspberry-pi-5-product-brief.pdf
[pi-io]: https://www.raspberrypi.com/documentation/computers/io-controllers.html
[pi-gadget]: https://www.raspberrypi.com/news/usb-gadget-mode-in-raspberry-pi-os-ssh-over-usb/
[mega]: https://docs.arduino.cc/hardware/mega-2560
[windows-usb-classes]: https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/supported-usb-classes
[ti-poe]: https://www.ti.com/tool/TIDA-00617
[ti-usb-power]: https://www.ti.com/lit/ds/symlink/tps2560.pdf
