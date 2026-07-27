# USB mesh PAU power architecture

Status: research design, not a schematic or hardware acceptance record.

## Decision

The `PeripheralAttachmentUnit` (`Pau`) should accept standards-compliant PoE
when practical and should source managed 5 V power at its physical USB ports.
PoE is an input option, not an unconditional USB-power promise. The PAU
advertises only the USB power that remains after its negotiated PoE class,
conversion losses, compute load, Ethernet load, thermal limits, and reserve
have been accounted for.

The baseline PAU has exactly four independently protected USB 3 Type-A
downstream-facing, fixed-host-role `PeripheralPort` connectors. They provide
5 V only and are four independent topology roots; they are not an internal or
synthetic hub. USB-C, dual-role operation, alternate modes, and USB Power
Delivery are deferred because they add cable orientation, Configuration
Channel, role, advertised-current, contract, and electronically marked-cable
requirements. A Type-C connector must not be fitted until those requirements
are implemented and compliance-reviewed.

## Terms

| Term | Meaning |
|---|---|
| `Pse` | Standards-compliant PoE power-sourcing equipment, normally the switch |
| `Pd` | The PAU's standards-compliant PoE powered-device input |
| `PauSystemRail` | Isolated intermediate rail feeding compute, network, and USB converters |
| `UsbPowerBudget` | Power safely available to all enabled USB ports |
| `PortPowerLimit` | Current and energy limit for one physical peripheral port |
| `PowerProfile` | Negotiated input class plus the derived system and port allocations |

`Pse` and `Pd` appear here as power-system terms only. They are not USB
electrical roles.

## Available PoE power

IEEE 802.3 power is specified at both ends of the cable. Design from the power
available at the PD, not the larger PSE nameplate value.

| PoE type | Maximum PSE power | Maximum PD power | PAU use |
|---|---:|---:|---|
| 802.3at Type 2 (`PoE+`) | 30 W | 25.5 W | Reduced-power mode |
| 802.3bt Type 3, Class 6 | 60 W | 51 W | Preferred practical baseline |
| 802.3bt Type 4, Class 8 | 90 W | 71.3 W | High-power option |

These limits come from the
[Ethernet Alliance IEEE 802.3bt overview](https://ethernetalliance.org/wp-content/uploads/2019/12/WP_EA_Overview8023bt_V2p1_FINAL.pdf).
The PAU must accept a lower assigned class without repeatedly restarting.
802.3bt testing explicitly requires an underpowered indication when a PD is
assigned less power than it requested; the PAU therefore exposes that state
locally and to the controller. See the
[Ethernet Alliance Gen2 PoE test plan](https://ethernetalliance.org/poecert/EA_Gen2_PoE_Certification_Test_Plan.pdf).

The controller must not compute USB capacity from the three headline values.
Each hardware revision publishes measured worst-case figures:

```text
UsbPowerBudget =
    PdAssignedPower
    - PoE conversion loss
    - compute and memory maximum
    - Ethernet PHY and link maximum
    - management and indicator maximum
    - thermal derating
    - transient and aging reserve
```

For illustration only, a 51 W PD input at 88% end-to-end conversion leaves
44.9 W before system consumption. If the PAU itself requires 20 W and reserves
5 W, at most 19.9 W remains for USB. The baseline's four 900 mA, 5 V port
allocations require 18 W before distribution losses, so that example fits on
paper but is not a release rating until measured over voltage, temperature,
cable length, and traffic. Type 2 may operate the PAU with fewer enabled ports
or lower negotiated allocations; it must not imply that all four ports can
simultaneously deliver 900 mA. Class 8 provides more margin, but enclosure
temperature can still govern the usable budget.

## 10 GbE and PoE

PoE does not imply a 1 Gb/s data ceiling. The 802.3bt certification plan
includes 2.5GBASE-T, 5GBASE-T, and 10GBASE-T when the device declares those
capabilities. The PAU should use:

- a PHY, connector/magnetics module, common-mode protection, and layout
  qualified together for 10GBASE-T and the intended four-pair PoE class;
- Category 6A permanent cabling and patch cords for the initial 10GBASE-T
  deployment;
- temperature-rated connectors and magnetics whose DC resistance imbalance
  and current rating cover the selected class;
- separate data-integrity and power/thermal qualification under maximum link
  traffic and maximum USB load.

The
[Ethernet Alliance cabling guidance](https://archive.nbaset.ethernetalliance.org/wp-content/uploads/2019/02/NBASET-Wireless-Access-WP-0219-print.pdf)
identifies Category 6A as supporting 10GBASE-T and 802.3bt power up to the
71 W PD range. This is a system design requirement, not permission to combine
arbitrary 10 GbE magnetics with a high-power PD front end.

## Power tree

```text
10GBASE-T + PoE cable
    -> surge and ESD network
    -> 802.3bt single-signature PD interface
    -> isolated, current-limited DC/DC
    -> PauSystemRail
       -> compute and memory regulators
       -> Ethernet PHY regulators
       -> 5 V USB regulator
          -> per-port eFuse/load switch
             -> USB Type-A VBUS
       -> isolated-low-voltage observability interface
          -> optional Mega 2560 panel
```

Use a reviewed PD controller and isolated converter reference architecture
rather than a learner-designed power supply. As feasibility evidence, TI's
[PMP21318 reference design](https://www.ti.com/tool/PMP21318) converts a
Type 4 Class 8 input to 12 V at up to 71 W, while Analog Devices'
[MAXREFDES1266](https://www.analog.com/media/en/reference-design-documentation/reference-designs/maxrefdes1266.pdf)
demonstrates a 65 W 802.3bt isolated flyback design with controlled startup
inrush. Neither reference is adopted as the final PAU circuit without
10GBASE-T, EMC, isolation, thermal, auxiliary-input, and production review.

The PoE-to-system converter provides the required network isolation boundary.
No Mega ground, USB shield treatment, debug cable, auxiliary supply, or bench
instrument connection may silently bridge it. Shield-to-chassis coupling,
protective components, creepage, clearance, transformer construction, and
dielectric test requirements belong to qualified hardware review.

## Four-port managed USB 3 Type-A baseline

`PeripheralPort0` through `PeripheralPort3` are physical USB 3 Type-A
downstream-facing ports with a fixed USB host role and fixed 5 V VBUS. Each
receives its own independently controlled protection channel. A shared current
limiter for several connectors cannot isolate a failed cable or identify which
topology caused a fault.

The four ports remain independent topology roots. The PAU never combines them,
flattens them, or synthesizes a hub. Every port has:

- a stable physical identity independent of USB bus address and route;
- its own power switch, current limit, telemetry, fault state, and
  `TopologyEpoch`;
- at most one exclusive route to one CAU `ComputerPort`;
- an independent cold-cycle and verified-off move sequence.

Each simultaneously active root consumes one physical USB port at the
destination computer through its corresponding CAU connection. Routing all
four roots at once therefore requires four CAU `ComputerPort` connections,
which may belong to one or several computers. A user who wants several
peripherals through one computer port attaches a physical hub to one PAU root;
that hub and all descendants remain one atomic topology.

Each channel must provide:

- disabled VBUS until the local USB host controller and route state permit it;
- a defined USB-compliant current limit, initially no more than 900 mA for a
  configured SuperSpeed standard downstream port;
- soft start or controlled output slew to bound connector and device inrush;
- short-circuit and thermal protection;
- reverse-current blocking so an externally powered hub cannot backfeed the
  PAU 5 V rail or an unpowered PAU;
- output discharge when disabled, where compatible with the USB controller
  and compliance requirements;
- deglitched overcurrent reporting independent of the Mega;
- voltage and current measurement on the protected side;
- a latched or bounded-retry policy, never an unlimited restart loop.

USB-IF states that a USB 3 standard downstream port supports up to 900 mA,
while a USB 2 device remains subject to the applicable USB 2 limit; see the
[USB-IF compliance update](https://compliance.usb.org/index.asp?Format=Standard&UpdateFile=Policies).
The controller must coordinate configuration state and power availability
rather than enabling every port at 900 mA at boot.

Port count therefore scales independently from available watts. Before
enabling a root, the PAU admission controller reserves its worst-case declared
port allocation plus conversion and thermal margin from `UsbPowerBudget`.
Admission is atomic: a route whose reservation does not fit remains
disconnected with `UsbBudgetLimited`; existing roots are not silently
overcommitted. Released or faulted ports return their reservations only after
VBUS-off is verified. Measured low consumption may inform later power profiles
but never authorizes an attached device to exceed the power advertised by its
current USB configuration.

Candidate protection functions include a current-limited USB power switch or
eFuse with reverse blocking, fault output, controlled rise time, and a rating
above every upstream single-fault voltage. TI describes these functions and
the need to consider overvoltage single faults in its
[USB eFuse application brief](https://www.ti.com/document-viewer/lit/html/SLVAFB9).
Part selection remains deferred until the exact 5 V converter, host
controller, connector, and compliance plan are fixed.

The SuperSpeed pairs, USB 2 data pair, VBUS, ground, and shield need coordinated
ESD protection and layout. A power switch does not protect data pins from every
miswire or powered-hub fault.

## User-provided hubs

A hub attached at the PAU is part of the exact routed topology and moves
atomically with its descendants.

- A bus-powered hub and all descendants share the one root port's advertised
  and measured budget. The PAU does not infer descendant demand from connector
  count.
- A self-powered hub may supply its descendants, but the root port still
  receives normal managed VBUS for attach behavior.
- The PAU must tolerate a compliant self-powered hub without allowing reverse
  current into its 5 V rail.
- A hub that backfeeds VBUS, exceeds the root limit, oscillates during startup,
  or drives an unpowered PAU is faulted and disconnected.
- The UI reports `ExternallyPowered`, `BusPowered`, `OverBudget`, and
  `BackfeedFault` only from electrical evidence and USB topology information;
  it does not guess from product type.

The PAU cannot guarantee power for an arbitrary user hub or every descendant.
High-power room installations should use a reputable self-powered hub.

## Brownout, priority, and recovery

Power policy is local and deterministic because a controller round trip is too
slow to protect a rail.

1. Preserve the PD interface, management processor, audit state, and network
   long enough to report and execute a controlled disconnect.
2. Do not enable a new peripheral port unless its complete declared allocation
   fits.
3. On declining input power or temperature margin, block new attachments
   before shedding active ones.
4. If shedding is unavoidable, disconnect the lowest configured priority port
   as a normal USB removal, then remove its VBUS.
5. Storage defaults to `Protected`: it is not automatically shed while mounted
   merely to keep a display or indicator alive.
6. If the core rail cannot remain valid, remove all USB VBUS, fence the active
   topology session, persist the cause if energy permits, and restart only
   after a stable-power qualification interval.

A brownout must never result in rapid connect/disconnect cycling. Retry count,
off time, stable interval, and hysteresis are fixed configuration values and
host-tested. The remote controller may request policy, but local hardware
protection always wins.

## Route-move power sequence

Every route move cold-cycles PAU-supplied VBUS by default. A route is not
reassigned while the old topology remains electrically attached:

```text
authorize move and require storage-safe confirmation
    -> disconnect the complete presented topology from the old computer
    -> confirm old-session teardown
    -> disable the root PeripheralPort power switch
    -> verify current decay, discharge, and VBUS-off
    -> fault and stop if VBUS remains driven or the topology backfeeds
    -> advance the non-reusable TopologyEpoch
    -> enable protected VBUS under the new power reservation
    -> re-observe the complete physical topology
    -> present a fresh attachment to the new computer
    -> enumerate through the unmodified host USB stack
```

Moving a route is therefore equivalent to unplugging the complete physical
topology and plugging it into the other computer. It does not preserve USB
addresses, configurations, streams, application state, or mounted storage.
Storage requires operating-system safe removal before teardown. A forced move
is permitted only as an explicit fault-recovery operation and records possible
data loss; it is never described as safe eject.

A self-powered peripheral or hub may remain internally energized when PAU VBUS
is removed. It must still observe the USB disconnect/reset sequence and
freshly enumerate in the new attachment session. Reverse-current protection
must prevent its supply from holding PAU VBUS above the verified-off threshold.
If discharge cannot be confirmed, if current persists unexpectedly, or if a
port reports overcurrent, backfeed, or thermal fault, the PAU leaves that root
disconnected and does not advance to attachment at the new computer.

## Auxiliary DC input

Provide a listed, SELV external DC input where PoE power is absent or
insufficient. It is a fallback and service option, not an invitation to parallel
supplies.

- Use a power-path controller or ideal-diode arrangement with defined source
  precedence, reverse blocking, and break-before-make or validated seamless
  transfer.
- The auxiliary input must not feed the Ethernet pairs or falsify PoE maintain
  power signature behavior.
- Its connector, polarity, voltage range, current rating, fuse/eFuse, surge
  protection, and isolation relationship must be explicit.
- Recalculate and publish the `PowerProfile` after every source transition.
- If a transition cannot preserve valid USB VBUS, issue a normal disconnect
  before the rail leaves tolerance.

For the first prototype, prefer a manual cold transfer: disable ports, power
down, select PoE or DC, then restart. Seamless transfer is a later measured
feature.

## Thermal design

Power availability is bounded by safe component and enclosure temperatures,
not only PoE classification.

- Instrument the PD hot-swap FET or bridge area, isolation transformer,
  converter switches and magnetics, 5 V regulator, each port switch, Ethernet
  PHY, and enclosure inlet/ambient.
- Characterize idle, maximum network traffic, port inrush, sustained maximum
  USB load, short circuit, blocked ventilation, high ambient, and long-cable
  cases.
- Derate the USB budget before a thermal limit; then block new ports; then
  perform deterministic shedding if the core cannot remain within limits.
- Use independent hardware overtemperature protection for conditions that
  software cannot safely manage.
- Publish the permitted ambient range, mounting orientation, ventilation
  clearance, cable category, PoE class, and simultaneous port-power rating.

No port-count or wattage claim is final until a production-like enclosure
passes conducted/radiated emissions, immunity, Ethernet, PoE, USB, isolation,
fault, and thermal evaluation.

## Measurement and evidence

The endpoint agent reports, per sample:

- negotiated and assigned PoE type/class and power;
- PoE input voltage/current/power where supported;
- intermediate and 5 V rail voltage;
- total USB allocation and measured consumption;
- each port's enable, voltage, current, peak/inrush, fault, retry, and priority;
- temperatures and active derating reason;
- auxiliary-input presence and selected source;
- brownout count and last shutdown cause.

Measurements have stated accuracy, sample period, saturation behavior, and
calibration provenance. They support diagnosis and budgeting; protective
limits remain enforceable without the Linux agent.

Local, non-Serial evidence should include:

- `PowerReady`: the system rail is qualified;
- `PoELimited`: assigned PoE power is below the requested profile;
- `UsbBudgetLimited`: one or more requested ports cannot be powered;
- per-port `Powered` and `Fault`;
- `ThermalDerate`;
- `AuxiliaryPower`.

The optional Mega 2560 is an observer and operator panel only. It receives
galvanically appropriate low-voltage status signals or a bounded telemetry
interface after the isolation boundary. It does not perform PoE classification,
regulation, current limiting, thermal shutdown, USB timing, or the sole fault
indication. Removing, resetting, or crashing the Mega cannot enable VBUS or
defeat protection.

## USB-C and Power Delivery deferral

USB-C is not a connector substitution for Type-A. A later PAU USB-C source port
requires, at minimum:

- correct source and data-role behavior on both plug orientations;
- Configuration Channel attach, detach, and current advertisement;
- VCONN and electronically marked cable handling where applicable;
- a USB PD policy engine for voltages or currents beyond default USB power;
- per-contract power reservation and immediate budget reconciliation;
- safe discharge, overvoltage, reverse-current, dead-battery, and role-swap
  behavior;
- SuperSpeed signal multiplexing/redriving appropriate to the negotiated mode;
- USB-IF compliance work and a connector/cable marking policy.

Until those requirements are designed, the baseline PAU exposes four
fixed-host-role Type-A 5 V ports only and does not claim dual-role operation,
USB PD, Type-C current, alternate modes, or USB4.

## Prototype acceptance gates

Physical work remains deferred until a qualified design review. The later
bench plan must include:

1. PoE detection, classification, lower-class assignment, maintain-power
   signature, disconnect, and restart behavior with certified test equipment;
2. 10GBASE-T link/error performance at minimum and maximum power;
3. isolation, leakage, creepage/clearance, surge, ESD, and fault review;
4. 5 V static regulation and transient response at every valid load;
5. per-port open, attach, capacitive inrush, overload, hard short, backfeed,
   thermal trip, retry, and discharge;
6. user-powered and bus-powered hub cases;
7. auxiliary-source insertion, removal, precedence, and cold transfer;
8. brownout during idle, enumeration, bulk traffic, and topology change;
9. worst-case enclosure thermal mapping;
10. proof that Mega removal and endpoint-agent failure cannot enable or
    unprotect a port.

Before those records exist, the accurate status is `architecture researched`;
it is not electrically verified, USB compliant, PoE certified, or safe for
unattended installation.
