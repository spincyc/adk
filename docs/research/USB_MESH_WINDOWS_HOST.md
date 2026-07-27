# Windows computer attachment

Status: research design  
Research date: 2026-07-27  
Product path: Windows computer → Computer Attachment Unit → switched Ethernet
→ Peripheral Attachment Unit → USB peripheral

## Product requirement

The final product must work with unmodified Windows and Linux computers. It
must not require a product driver, service, agent, or application on the
computer. The physical topology is:

```text
Windows computer
    -> physical USB 3
    -> Computer Attachment Unit (Cau)
    -> authenticated Ethernet session
    -> Peripheral Attachment Unit (Pau)
    -> physical USB peripheral
```

The controller remains an ordinary Linux service and does not carry USB
payload. The Cau is electrically a USB device toward the computer. The Pau is
electrically a USB host toward the peripheral.

Use these terms consistently:

| Term | Computer software | Purpose |
|---|---|---|
| `TransparentComputerProfile` | None | Required final product |
| `ManagedComputerPrototype` | Research driver/service | Optional laboratory comparison; never the product |

`TransparentComputerProfile` means the computer uses only its native USB host
controller and inbox or peripheral-vendor drivers. “Host” is avoided in the
profile name because it can mean either the computer's electrical USB role or
a mesh endpoint.

## Required Windows architecture

The Cau must electrically present exactly the physical USB topology rooted at
one routed Pau port to the Windows computer:

```text
Windows inbox or peripheral-vendor driver
    -> native Windows USB hub and host-controller stack
    -> physical Cau USB device controller
    -> reconstructed device, configuration, interface, and endpoint behavior
    -> authenticated Ethernet session
    -> Pau USB host controller
    -> one physical Pau port
    -> physical peripheral or user-supplied physical hub and its descendants
```

The gateway terminates both USB relationships. It does not extend the
SuperSpeed wires or forward an undifferentiated byte stream. It must
reconstruct descriptors, enumeration, endpoint transfers, stalls, resets,
disconnects, suspend and wake behavior, timing, and relevant power-state
observations. The computer must see behavior acceptable to the driver that
would have bound to the locally attached peripheral.

Every reconstructed transaction carries or is bound to the current route
epoch inside the gateways. A stale route may never complete a request against
a newly assigned peripheral. Route changes appear to the computer as honest
USB disconnect and reconnect events.

## Exact-topology invariant

One Cau uses exactly one physical USB port on its computer. It presents one
rooted topology selected from exactly one physical Pau port.

The Cau synthesizes no additional hub, no management function, no fixed
composite console device, and no otherwise hidden USB function on the
computer-facing link. If a single peripheral is plugged into the selected Pau
port, the computer sees that peripheral at the root. If the user plugs a real
hub into that Pau port, the computer sees that hub and its physical descendant
topology. Hub ports, connect-change events, power behavior, resets, device
speeds, and descendant attachment generations are reconstructed from that real
hub rather than invented by the Cau.

```text
computer root port
    -> Cau
        -> Ethernet
            -> Pau port 3
                -> physical user hub
                    -> keyboard
                    -> mouse
                    -> storage
```

The observable computer topology must correspond to:

```text
computer root port
    -> physical user hub
        -> keyboard
        -> mouse
        -> storage
```

The Cau and Pau are transport terminations, not extra visible USB topology.
Their product identity, management channel, update path, diagnostics, and
route control must live outside the computer-facing USB topology.

The routed ownership unit is the entire topology rooted at the selected Pau
port. Descendants of a physical hub cannot be independently routed to other
computers in this profile. To route peripherals independently, plug them into
separate Pau ports and assign each port to a separate Cau/computer USB port.
This preserves the rule that one Cau port mirrors one real Pau-port root.

## Route movement

Route movement deliberately reproduces a physical unplug followed by a
physical plug and normal enumeration on the newly selected computer:

```text
old computer
    -> stop new transfers
    -> complete or cancel bounded outstanding transfers
    -> USB disconnect
    -> route epoch advances
    -> old Cau remains disconnected

new computer
    -> new Cau presents the Pau port's complete rooted topology
    -> USB attach and reset
    -> descriptor enumeration
    -> native driver binding
    -> application-visible device
```

There is no transparent live migration, preserved USB address, preserved
driver instance, preserved application handle, or cross-computer session
continuity. Storage must be safely ejected or explicitly forced through a
recorded unsafe-removal policy. Audio, video, and other streams stop and must
restart. A failure before new enumeration leaves the peripheral disconnected;
it never silently reconnects to the old computer.

The controller may coordinate a console-wide move, but USB, video, and audio
each report their own disconnect, attach, enumeration, and readiness evidence.
“Console active” is true only after every required route has reached its
declared ready condition.

## Rejected production approaches

| Technology | Appropriate use here | Important limit |
|---|---|---|
| WinUSB | A laboratory tunnel-device experiment | Requires an application and exposes a product function instead of the exact remote topology |
| UMDF 2 | Comparative research | Requires an installed driver and does not provide the required physical transparency |
| KMDF with UdeCx | A software-only Windows feasibility baseline | Requires an installed kernel driver and virtual controller, contrary to the product requirement |
| Windows service | Development instrumentation only | Requires installed software and cannot make a custom tunnel look like arbitrary native peripherals |

UdeCx remains valuable as a research oracle: it can help separate network and
transaction-model failures from physical device-controller failures. It is not
the production architecture. The final computer-side gateway must need no
Windows driver signing, HLK installation package, computer agent, or equivalent
Linux installation.

## Reconstruction profiles

Native computer operation does not imply that one implementation can
transparently reproduce every USB device. Compatibility must be explicit:

| Reconstruction level | Gateway behavior | Expected reach |
|---|---|---|
| Fixed class | Presents a gateway-owned HID, audio, or other class-compliant function | Predictable first milestones; may normalize the remote device |
| Descriptor-driven class | Mirrors descriptors within a thoroughly implemented USB class | Broader devices inside a bounded class |
| Device-specific | Reconstructs a known VID/PID family and measured behavior | Vendor devices with explicit support |
| Generic transparent | Attempts to reproduce arbitrary descriptors, transfers, timing, and control behavior | Research objective; no universal claim |

A fixed keyboard/mouse composite console would violate the exact-topology
invariant and is not a product profile. A descriptor-driven proxy must instead
reproduce the real peripheral or user-supplied hub topology closely enough for
normal inbox or vendor drivers to bind, including supported vendor requests and
timing behavior.

## Compatibility limits

“Full USB 3” is a product objective, not an initial compatibility claim.
Compatibility must be earned by transfer type and device family:

1. control and bulk loopback;
2. HID interrupt devices;
3. mass storage with explicit unsafe-removal policy;
4. composite devices kept atomic;
5. audio and camera isochronous traffic;
6. suspend, resume, wake, reset, cancellation, and surprise removal;
7. vendor-specific devices;
8. hubs and routed subtrees only after ownership semantics exist.

Windows imposes concrete scheduling and request-contract rules for high-speed
and SuperSpeed isochronous URBs. Network latency and jitter cannot be hidden by
ordinary buffering for every device. Admission control must reject a route
whose latency, bandwidth, or timing contract cannot be met.

Security tokens, biometric devices, USB networking, boot devices, and devices
with timing-sensitive anti-tamper behavior require separate policy and test
profiles. The design must never imply universal compatibility from successful
HID or storage tests.

## Native-system validation

The product has no Windows driver package. Windows driver signing and an
installer are therefore not product requirements. Gateway firmware, update
signing, USB electrical compliance, class compliance, and native operating
system interoperability remain product requirements.

The Windows test lab therefore needs at least:

- supported Windows 10 and Windows 11 targets;
- current x64 systems with Secure Boot enabled;
- checked failure behavior for gateway firmware upgrade and rollback, sleep,
  hibernate, restart, gateway removal, network partition, and controller loss;
- USB protocol captures, native Windows traces, kernel dumps when Windows
  drivers expose gateway defects, and reproducible network captures;
- a compatibility corpus shared with Linux wherever device behavior permits.

No Windows validation is claimed until those results are recorded.

## Why generic transparency is difficult

The transparent profile is valuable for locked-down computers, BIOS/UEFI
interaction, and non-PC hosts, but it is not a generic byte-forwarder.
Descriptor changes require believable disconnect/reconnect behavior; endpoint
timing and power transitions must be reproduced; arbitrary vendor protocols
may depend on undocumented behavior.

Develop compatibility progressively:

1. one real HID keyboard or mouse directly attached to one Pau port;
2. one real, qualified hub with HID descendants;
3. explicitly supported storage attached directly or beneath that real hub;
4. selected audio and camera devices;
5. broader descriptor-driven reconstruction only after measurement.

Each shipped milestone must remain transparent to the computer, even when the
supported peripheral set is initially narrow. It must reproduce the actual
rooted topology rather than normalize it into a gateway-owned composite
device. Do not introduce a managed computer dependency to broaden
compatibility. Do not claim that the product can reproduce every USB
peripheral.

## Shared Windows and Linux product contract

The gateways implement the computer-facing contract in hardware and firmware;
Windows and Linux remain outside the managed system:

```text
ComputerAttachment
    observeGateway()
    stagePeripheral()
    activateRoute(routeEpoch)
    quiesceRoute(routeEpoch)
    removePeripheral(routeEpoch)
    reportObservedState()
```

These names describe responsibilities, not approved C++ interfaces. Both
platforms must share:

- stable gateway, port, peripheral, and route identities;
- monotonic route epochs;
- descriptor and topology digests;
- break-before-make movement;
- bounded request cancellation;
- explicit enumeration and driver-binding evidence;
- fail-closed controller and network loss;
- an audit record distinct from payload logging.

Native operating-system driver identifiers and transient bus addresses never
become mesh identities.

## Open design work

- Prototype UdeCx with a synthetic bulk device only as a comparative model,
  never as a product dependency.
- Select gateway silicon with simultaneous SuperSpeed device-controller,
  host-controller, and multi-gigabit network capabilities.
- Define which descriptors are mirrored and which fixed-class profiles use
  gateway-owned identities.
- Measure end-to-end interrupt and isochronous deadlines on the intended
  switched network.
- Define supported native Windows and Linux versions and validation duration.
- Decide the first transparent compatibility milestone: HID only, or HID plus
  an explicitly supported bulk-storage profile.

## Primary sources

- [Architecture of USB Device Emulation](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-emulated-device--ude--architecture)
- [Write a UDE client driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/writing-a-ude-client-driver)
- [Choose a USB driver model](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/winusb-considerations)
- [Introduction to WinUSB](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/introduction-to-winusb-for-developers)
- [USB client-driver URB practices](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-client-driver-contract-in-windows-8)
