# HDMI mesh endpoint hardware

Status: purchase-oriented research; no hardware has been acquired or verified  
Scope: physical HDMI receiver/network/transmitter appliances  
Controller: an ordinary Linux computer; not an Arduino and not the media path

## Decision

Use a programmable-logic or adaptive-SoC appliance at each physical HDMI
location. The appliance terminates the source-facing HDMI link, transports
timed media over Ethernet, and constructs a new sink-facing HDMI link. It runs
an endpoint agent, but it does not own mesh policy.

Run the authoritative mesh controller as a normal Linux service on a separate
desktop, server, or small-form-factor computer. The initial controller needs
durable storage and management-network access; it does not need an FPGA,
HDMI connector, or 100 GbE media interface. Controller high availability
remains deferred.

The Arduino Mega 2560 is an optional local operator and evidence panel. It may
drive route buttons and status lamps and read a narrow health-register
interface. It does not receive HDMI, process pixels, packetize media, implement
PTP, or carry Ethernet media traffic.

Product endpoints attach to the same ordinary managed household LAN used by
USB, control, telemetry, and other clients. A dedicated media switch,
independent management Ethernet, or physically separate A/B media fabric is
not a product assumption. VLANs and QoS are logical policy tools on that shared
network. Every product route must fit a named profile on the measured complete
path while preserving configured household headroom.

The baseline product investigation starts with one 10GBASE-T access link per
HDMI endpoint. A compressed 1080p or 4K profile may fit after measured overhead
and margin; uncompressed 8K60 cannot. Higher-rate access or aggregation may
exist on the same household network, but it does not create a separate media
fabric and does not remove the slowest-link admission gate.

```text
ordinary Linux controller
        |
        | authenticated management network
        v
endpoint appliance A                       endpoint appliance B
HDMI RX -> FPGA media pipeline -> Ethernet -> FPGA media pipeline -> HDMI TX
             ^                                                    ^
             | optional local health registers                    |
          Mega panel                                           Mega panel
```

## Primary full-8K evaluation platform

Start the full-bandwidth hardware investigation with two AMD VEK385 evaluation
kits, one at each endpoint.

The board is a credible integration candidate because AMD documents:

- one HDMI 2.1 input and one HDMI 2.1 output;
- one QSFP28 and one SFP28 connector;
- 100G multirate Ethernet MACs;
- 20 GB of LPDDR5X component memory;
- HDMI and MRMAC example designs;
- clocking dedicated to HDMI RX, HDMI TX, and QSFP28 operation.

AMD's HDMI 2.1 Receiver and Transmitter Subsystems list VEK385 as a reference
board. The receiver terminates TMDS or FRL and exposes video and audio streams;
the transmitter consumes streams and creates a fresh TMDS or FRL link. Those
IP blocks fit the mesh boundary, but their presence in the catalog is not proof
that an end-to-end HDMI-plus-100GbE image is supplied or licensed.

The VEK385 is an evaluation board, not a production appliance. AMD currently
lists `EK-VEK385-G` at USD 15,995 with a 16-week lead time. Treat that price
only as a dated purchase-planning reference and check it again before ordering.

### Questions to close before ordering

Obtain written vendor confirmation for the selected tool release and board
revision:

1. HDMI 2.1 RX and TX subsystem evaluation or production licenses;
2. usable FRL rates and the exact uncompressed 8K formats in one design;
3. 100G MRMAC, PCS, FEC, and QSFP28 reference-design rights;
4. simultaneous HDMI RX, HDMI TX, LPDDR5X, and 100GbE transceiver placement;
5. hardware PTP timestamping strategy;
6. access to EDID, DDC, HPD, audio, InfoFrames, HDR, and error counters;
7. DSC and JPEG XS licenses, separately from HDMI pass-through support;
8. HDCP licensing and key-provisioning requirements, if protected content is
   ever placed in scope.

Do not purchase two boards merely because every required connector is visible.
First complete a resource, clock, transceiver-lane, and license feasibility
design for one endpoint image.

## Lower-rate rehearsal platform

AMD VEK280 is a useful architecture rehearsal, not the uncompressed 8K target.
AMD documents HDMI 2.1 input and output with redrivers supporting FRL rates up
to 12 Gb/s per lane, while the board exposes SFP28 rather than a native
100GbE endpoint link. Use it for:

- HDMI termination and reconstruction;
- EDID, HPD, format-change, and safe-mute behavior;
- 1080p or 4K packet transport;
- bounded JPEG XS experiments over 25GbE;
- controller, endpoint-agent, diagnostics, and mesh-route integration.

It cannot make an uncompressed 8K60 stream fit on 25GbE. A VEK280 result must
not be presented as proof of the 100GbE endpoint.

Obtain a current official quotation, included tool entitlement, and lead time
before using VEK280 in a purchase request.

## Alternative split platform

A network-oriented FPGA board such as AMD VCU118 can explore a 100GbE packet
engine, but it requires compatible HDMI mezzanine hardware and substantially
more integration. AMD lists the VCU118 as an HDMI 2.1 RX reference board and
currently lists the evaluation kit at USD 14,995. It is an alternative when
the VEK385 resource study fails, not the first purchase.

An Intel Agilex-based endpoint is technically plausible: Intel documents HDMI
2.1 FRL IP and hard Ethernet IP for 25G and 100G operation. No single current
official Intel evaluation-board source reviewed here establishes the same
HDMI-RX, HDMI-TX, and 100GbE board-level integration. Keep it as a vendor
diversity study until a specific baseboard, HDMI daughtercard, clocks,
transceiver map, IP licenses, and reference design form a complete BOM.

intoPIX publishes an 8K HDMI 2.1 JPEG XS FPGA evaluation solution. It is useful
for the compressed branch, but it does not replace the uncompressed reference
path and its exact board, license, Ethernet, availability, and price require a
vendor quotation.

## Staged bill of materials

Do not buy the final laboratory in one step. Each stage retires a different
risk.

### Stage 0: controller and deterministic model

| Quantity | Item | Purpose |
|---:|---|---|
| 1 | Existing x86-64 Linux computer | Authoritative controller and CLI |
| 1 | Existing managed household Ethernet switch | Shared management and synthetic-media model |
| 1 | Existing Mega 2560 kit | Optional buttons and visible health evidence |

No media FPGA is required. Prove enrollment, desired and observed state,
fencing epochs, admission, route movement, controller restart, and fail-closed
behavior with simulated endpoint agents.

### Stage 1: one endpoint, local loop

| Quantity | Item | Purpose |
|---:|---|---|
| 1 | VEK385 kit | HDMI RX/TX and packet-pipeline feasibility |
| 1 | Unencrypted pattern source | Deterministic source and frame identity |
| 1 | Display with documented modes | Sink and EDID testing |
| 2 | Certified Ultra High Speed HDMI cables | Source-to-board and board-to-sink |
| 1 | QSFP28 passive loopback supplied with the kit | Network MAC/PCS loopback |
| 1 | Mega 2560 plus LEDs/buttons | Optional circuit-visible diagnostics |

First prove generated color bars, RX-to-TX local pass-through, a visible frame
counter, video CRC, DDC/HPD policy, safe mute, and QSFP28 packet loopback. Do
not claim networked HDMI at this stage.

### Stage 2: two endpoints over one switch

| Quantity | Item | Purpose |
|---:|---|---|
| 1 additional | VEK385 kit | Second physical endpoint |
| 1 | Managed nonblocking 100GbE-capable switch | Shared-LAN laboratory feasibility, not a dedicated product fabric |
| 2 | Validated 100GbE DACs, or two optics plus fiber | Endpoint links |
| 1 | PTP-capable laboratory clock or proved switch clock | Time discipline |
| 1 | Second sink or capture/analyzer access | Phase-three fan-out and multicast evidence |

Select the switch only after verifying 100GbE port type, MTU, PTP behavior,
multicast tables, IGMP snooping and querier behavior, QoS counters, telemetry,
and nonblocking capacity. Connector speed alone is insufficient.

Begin at 1080p, then 4K, then a named 8K profile. Record measured on-wire rate,
loss, reorder, latency, buffer high-water marks, clock offset, output CRC, and
route-change behavior at every step.

### Stage 3: compressed and redundant modes

Add JPEG XS only after the uncompressed reference is correct. Obtain the codec
IP, tools, and profile license explicitly; then measure latency and degradation
using UI text, noise, HDR gradients, and repeated generations.

Evaluate redundant paths only as a topology within the same managed household
network after single-path behavior is stable. A second dedicated media fabric
is outside the product constraint. Media-path redundancy remains separate from
controller high availability, which is deferred.

### Stage 4: production endpoint study

Replace evaluation kits with a custom appliance only after the endpoint image
and resource measurements are stable. The production study must cover:

- adaptive SoC/FPGA part and configuration storage;
- HDMI redrivers, ESD protection, DDC/HPD level handling, and certified layout;
- QSFP28 cage, module power and cooling, MAC address allocation, and clocks;
- LPDDR memory and worst-case buffering;
- secure boot, signed updates, endpoint identity, and key storage;
- shared-LAN management and media isolation, admission, and fault containment;
- watchdog, power sequencing, thermal sensors, fan control, and visible alarms;
- regulatory, HDMI adopter, HDCP, Ethernet, EMC, and safety obligations.

This is a custom high-speed digital product, not an Arduino shield project.

## Software placement

| Function | Normal Linux controller | Endpoint adaptive SoC/FPGA | Mega 2560 |
|---|:---:|:---:|:---:|
| Desired routes and policy | yes | no | no |
| Durable route and audit state | yes | no | no |
| Endpoint enrollment | yes | agent only | no |
| HDMI RX/TX and EDID/HPD | no | yes | no |
| Video/audio packet data | no | yes | no |
| PTP timestamps and media clocks | no | yes | no |
| Route execution and health | observes | yes | displays |
| Buttons and local lamps | optional | health registers | yes |

The controller may use an ordinary lower-rate household LAN connection.
Keeping policy off the endpoint makes a route change independent of the media
board vendor and allows the same controller model to govern USB and HDMI
meshes.

## Purchase gate

The first likely physical purchase is one VEK385, not two, after the license
and transceiver feasibility questions are answered. One board can retire the
largest HDMI/clock/resource risks with local loopback. Buy the second board and
a higher-rate shared-LAN switch only after that evidence and a complete
household-network admission plan are recorded.

Physical verification is deferred. Nothing in this document claims that ADK
has compiled a VEK385 design, trained an HDMI link, transported a frame, or
interoperated with a switch.

## Primary sources

- [AMD VEK385 evaluation kit and current official price][vek385]
- [AMD VEK385 product brief][vek385-brief]
- [AMD VEK385 user guide][vek385-guide]
- [AMD VEK385 transceiver map][vek385-transceivers]
- [AMD VEK385 HDMI clocking][vek385-clocking]
- [AMD HDMI 2.1 Receiver reference boards][rx-boards]
- [AMD HDMI 2.1 Receiver subsystem][rx]
- [AMD HDMI 2.1 Transmitter subsystem][tx]
- [AMD VEK280 HDMI input and output][vek280-hdmi]
- [AMD VCU118 evaluation kit and current official price][vcu118]
- [Intel HDMI FPGA IP overview][intel-hdmi]
- [Intel E-Tile 25G/100G Ethernet IP features][intel-ethernet]
- [intoPIX JPEG XS 8K FPGA evaluation brief][intopix]

[vek385]: https://www.amd.com/en/products/adaptive-socs-and-fpgas/evaluation-boards/vek385.html
[vek385-brief]: https://www.amd.com/content/dam/amd/en/documents/products/adaptive-socs-and-fpgas/boards-kits/product-briefs/vek385-evaluation-kit-product-brief.pdf
[vek385-guide]: https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd
[vek385-transceivers]: https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd/Transceivers
[vek385-clocking]: https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd/HDMI-Clocking
[rx-boards]: https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/Reference-Boards
[rx]: https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/HDMI-2.1-Receiver
[tx]: https://docs.amd.com/r/en-US/pg350-v-hdmi-txss1/HDMI-2.1-Transmitter
[vek280-hdmi]: https://docs.amd.com/r/en-US/ug1612-vek280-eval-bd/HDMI-Video-Input/Output
[vcu118]: https://www.amd.com/en/products/adaptive-socs-and-fpgas/evaluation-boards/vcu118.html
[intel-hdmi]: https://www.intel.com/content/www/us/en/docs/programmable/683798/23-3-19-7-2/hdmi-overview.html
[intel-ethernet]: https://www.intel.com/content/www/us/en/docs/programmable/683468/22-3/intel-fpga-ip-supported-features.html
[intopix]: https://www.intopix.com/Ressources/Solution%20Brief/jpeg-xs-8k-fpga-evaluation-kit-solution-brief.pdf
