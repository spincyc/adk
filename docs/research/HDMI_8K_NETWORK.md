# 8K HDMI over a switched network

Status: feasibility study, not a supported ADK interface  
Research date: 2026-07-27

## Conclusion

A full 8K60 HDMI matrix over ordinary packet switches is buildable, but not
with an Arduino in the media path. The credible endpoint is an FPGA or adaptive
SoC with HDMI 2.1 receiver/transmitter IP, high-speed memory where required,
and 25/100 GbE. An Arduino Mega can be a deliberately limited control and
observation plane: select routes, display endpoint health, operate physical
controls, and watchdog the media processor.

Two products are worth separating:

1. A research matrix for unprotected sources using open media transports.
2. A commercial HDMI repeater product, which requires HDMI and HDCP licenses,
   licensed IP, compliance work, and protected key handling.

Do not start by implementing HDMI FRL, HDCP, a codec, and a packet network at
once. Start with generated video and a software or FPGA packet loop.

## What “full 8K” means

The requirements must name resolution, frame rate, sampling, component depth,
HDR metadata, audio, variable refresh, content protection, and acceptable
latency. “8K HDMI” alone is not testable.

Use this first target:

- 7680 × 4320 at 60 progressive frames/s;
- RGB or YCbCr 4:4:4, initially 8 bits/component;
- fixed frame rate;
- eight channels of 48 kHz PCM audio;
- static and dynamic HDR metadata preserved after the video path works;
- no HDCP during research;
- source-to-sink route changes, multicast fan-out, and synchronized outputs;
- fewer than two frames of glass-to-glass latency initially;
- no silent fallback to 4:2:0, lower depth, or lower frame rate.

HDMI Licensing Administrator says HDMI 2.1b supports 8K60 and raises link
capacity to 48 Gb/s. HDMI 2.1 uses four Fixed Rate Link lanes rather than the
TMDS data-plus-clock arrangement used by HDMI 1.4/2.0
([HDMI resolutions and bandwidth](https://www.hdmi.org/spec2sub/res-bandwidth),
[Intel HDMI overview](https://www.intel.com/content/www/us/en/docs/programmable/683798/23-3-19-7-2/hdmi-overview.html)).

### Active-picture rate

The useful first calculation excludes blanking and transport overhead:

```text
active bit rate = width × height × frames/s × bits/pixel
```

| Format | Active video |
|---|---:|
| RGB/4:4:4, 8-bit components (24 bpp) | 47.776 Gb/s |
| RGB/4:4:4, 10-bit components (30 bpp) | 59.720 Gb/s |
| YCbCr 4:2:2, 10-bit components (20 bpp) | 39.813 Gb/s |
| YCbCr 4:2:0, 10-bit components (15 bpp) | 29.860 Gb/s |

RTP, IP, UDP, Ethernet, inter-packet gaps, stream redundancy, audio, metadata,
FEC, and engineering margin all add capacity. Thus:

- 10 GbE cannot carry uncompressed 8K60;
- 25 GbE cannot carry any format in the table uncompressed;
- 40 GbE is not a sound design point even for nominal 4:2:2 10-bit;
- 100 GbE is the sensible single-link baseline for uncompressed 8K60;
- dual diverse 100 GbE links are appropriate when seamless path redundancy is
  a requirement.

HDMI’s 48 Gb/s figure is a link capability, not proof that every listed pixel
format fits uncompressed. HDMI also supports Display Stream Compression for
higher combinations of frame rate, depth, and sampling. The endpoint must read
and validate the negotiated format rather than infer it from the “8K” label.

## Three media-plane designs

### A. Uncompressed: correctness reference

Pipeline:

```text
HDMI RX → pixel/metadata extraction → RTP packetizer → 100 GbE
        → switched multicast → RTP receiver → elastic buffer → HDMI TX
```

Use SMPTE ST 2110 concepts: separate video, audio, and ancillary flows;
RTP timestamps; PTP-derived media time; traffic shaping; and SDP descriptions.
SMPTE lists ST 2110-10 for system timing, ST 2110-20 for uncompressed active
video, and ST 2110-21 for video traffic shaping
([SMPTE standards index](https://www.smpte.org/standards/recently-updated-documents)).

Benefits:

- no codec artifacts or codec interoperability question;
- simplest pixel-for-pixel oracle;
- potentially sub-frame latency;
- useful reference against which compressed modes are measured.

Costs:

- 100 GbE endpoint, switch port, and optics per direction;
- large instantaneous FPGA I/O and packet-processing bandwidth;
- careful packet pacing and receive buffering;
- a redundant system doubles media bandwidth.

This should be the laboratory reference, not necessarily the first affordable
multi-endpoint product.

### B. JPEG XS: recommended practical product

Pipeline:

```text
HDMI RX → JPEG XS encoder → ST 2110-22/RTP → 10/25 GbE
        → switched multicast → JPEG XS decoder → HDMI TX
```

JPEG XS is designed for lightweight, line-based, low-latency mezzanine
compression. SMPTE ST 2110-22 carries constant-bit-rate compressed video.
Published FPGA IP supports 8K, fixed latency of a few lines, multiple sampling
formats and bit depths, and adjustable visually-lossless rates
([SMPTE paper](https://journal.smpte.org/periodicals/SMPTE%20Motion%20Imaging%20Journal/129/7/14/),
[intoPIX FPGA IP](https://www.intopix.com/tico-xs-ip-cores)).

An Intel Agilex/intoPIX evaluation design documents 8K60 HDMI 2.1 encode/decode
on one FPGA board and cites a 1.6 Gb/s demonstration mode. Treat that as a
vendor evaluation point, not a universal quality guarantee
([8K evaluation brief](https://www.intopix.com/Ressources/Solution%20Brief/jpeg-xs-8k-fpga-evaluation-kit-solution-brief.pdf)).

Recommended network targets:

- 10 GbE for one aggressively compressed 8K flow or early research;
- 25 GbE for quality margin, audio/metadata, and future codec profiles;
- 100 GbE uplinks/spines for multiple simultaneous endpoints.

JPEG XS is usually described as visually lossless or near-lossless at selected
rates; that is not mathematical losslessness. The test plan must include
synthetic UI/text patterns, HDR gradients, noise, repeated generations, and
objective pixel comparisons.

### C. AVC/HEVC/AV1: distribution mode

Long-GOP codecs give much lower bit rates and suit monitoring, recording, WAN,
or many-viewer distribution. They introduce frame buffering, content-dependent
latency, route-change delay, and error propagation. They are not the primary
design for an interactive matrix.

AMD documents adaptive SoCs that combine 8K-oriented video processing,
Ethernet up to 100 Gb/s, HDMI 2.1 ingestion, and codec options
([Versal AV solution brief](https://www.amd.com/content/dam/amd/en/documents/products/adaptive-socs-and-fpgas/versal/versal-prime-gen-2-solution-brief-avbc.pdf)).
Use a hard codec only as a later, explicitly higher-latency operating mode.

### Mathematically lossless

A reversible codec may reduce typical traffic but cannot provide a useful
worst-case compression guarantee for arbitrary input. Capacity planning must
either reserve the uncompressed worst case or define a bounded constant-rate
mode that may cease to be mathematically lossless. Do not label JPEG XS
“lossless” without naming the exact profile and validated operating point.

## Endpoint architecture

### Media processor

One transmitter endpoint needs:

- licensed HDMI 2.1 RX subsystem and suitable high-speed transceivers;
- EDID, hot-plug, SCDC/FRL training, audio, and metadata handling;
- optional HDCP receiver/repeater block only in the licensed product;
- pixel-domain monitor and deterministic test-pattern generator;
- optional JPEG XS encoder;
- RTP/UDP/IP packetizer and traffic shaper;
- 25 or 100 GbE MAC/PCS/FEC and optical/copper interface;
- PTP hardware timestamping or a disciplined FPGA time counter;
- counters that remain readable while video is flowing.

The receiver mirrors this pipeline, adding a jitter/elastic buffer, clock
reconstruction, underflow behavior, and safe HDMI output.

AMD’s current VEK385 documentation exposes HDMI 2.1 and QSFP28 transceiver
connections on one Versal evaluation platform, including HDMI clock recovery
and control details
([transceiver map](https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd/Transceivers),
[HDMI clocking](https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd/HDMI-Clocking)).
Intel also publishes HDMI 2.1 FRL FPGA PHY material
([Intel HDMI 2.1 PHY](https://www.intel.com/content/www/us/en/docs/programmable/732147/22-3-1-0-1/hdmi-2-1-support-frl-1-43138.html)).
These are credible platforms, but board availability, exact IP licenses, tool
versions, and reference-design access must be confirmed before purchase.

### Arduino control plane

The Mega 2560 should not parse video packets, emulate HDMI sideband protocols,
hold HDCP keys, or participate in PTP timing. Its useful jobs are:

- route-selection buttons and a local route/status display;
- enable/reset/watchdog lines for the media processor;
- read-only health and capability registers over SPI or UART;
- physical locate/identify LED;
- fan, temperature, power-good, and link-alarm observation;
- a deterministic maintenance mode and safe reset sequence.

The FPGA/SoC remains authoritative. A command is accepted only after the media
processor reports the route active and its receive buffer locked.

A narrow register protocol could expose:

```text
identity, firmware compatibility, HDMI lock, negotiated format,
PTP lock/offset, network link, active route, RTP sequence errors,
buffer minimum/maximum, video CRC, audio state, temperature, alarms
```

The local non-Serial verification path should include:

- green: media locked and timestamps advancing;
- amber: control works but media is not locked;
- red: persistent resource, thermal, format, or timing fault;
- a test-pattern button that replaces source video locally;
- an HDMI-output overlay or border generated in the FPGA;
- named scope/test points for PTP PPS, frame start, and buffer alarm.

Serial logs remain supporting evidence, available through CLI targets, never
the only indication that the path works.

## Switching, timing, and routing

An “ordinary switched network” means standards-based Ethernet/IP, not an
unmanaged consumer switch. The fabric needs:

- nonblocking capacity for the declared route set;
- 25/100 GbE endpoint ports and suitably faster spine/uplinks;
- IGMPv3 snooping and an explicit multicast querier;
- VLAN and QoS separation for media, PTP, and control;
- large enough buffers with documented behavior under microbursts;
- PTP transparent or boundary-clock support;
- telemetry for queue drops, multicast state, and clock health;
- optional dual independent A/B fabrics for hitless protection.

IEEE describes TSN as bounded-latency, low-jitter, low-loss service based on
synchronization, reservation, shaping, and queuing. IEEE 802.1AS defines
synchronized time for time-sensitive applications across bridged LANs
([IEEE TSN](https://grouper.ieee.org/groups/802/1/pages/tsn.html),
[IEEE 802.1AS revision](https://grouper.ieee.org/groups/802/1/pages/802.1AS-rev.html)).
ST 2110 systems normally use the SMPTE ST 2059 PTP profile; exact profile and
domain choices must be consistent across all endpoints and clocks.

Commercial examples demonstrate that suitable hardware exists. NETGEAR’s
M4350 family includes 25/100 GbE models and PTP support, but its documentation
also identifies configuration constraints such as stacking being mutually
exclusive with PTP transparent-clock operation
([M4350 data sheet](https://www.downloads.netgear.com/files/GDC/M4350/M4350_Datasheet.pdf)).
NVIDIA ConnectX-7 provides up to 400 GbE and hardware PTP capabilities for
server-based experiments
([ConnectX-7 manual](https://networking-docs.nvidia.com/connectx7hw/introduction)).
These are examples, not a bill of materials.

### Control protocol

Use NMOS rather than inventing discovery and connection semantics:

- IS-04 registers and discovers Nodes, Devices, Senders, Receivers, Sources,
  and Flows;
- IS-05 stages and activates Sender/Receiver connections, including bulk
  “salvo” changes;
- IS-08 maps audio channels when required.

AMWA documents IS-04 as HTTP/JSON plus DNS-SD discovery and IS-05 as connection
management for logical Senders and Receivers
([IS-04](https://specs.amwa.tv/is-04/releases/v1.3.3/docs/Overview.html),
[IS-05](https://specs.amwa.tv/is-05/v1.1/docs/Overview.html)).
Run NMOS on the endpoint’s Arm/Linux processor or a network controller. The
Mega presents physical controls to that controller through a small, deterministic
protocol; it does not host the production NMOS stack.

## HDCP, HDMI, and legal gates

Research first with generated, public-domain, or otherwise unprotected input.
Do not capture, decrypt, store, bridge, or re-encrypt protected media outside a
licensed design.

HDMI LA states that access to current HDMI specifications and rights to build
licensed products are provided through the HDMI Adopter program. It also warns
that unlicensed/noncompliant products do not receive the relevant necessary-
claims license
([HDMI Adopter program](https://www.hdmi.org/adopter/index),
[licensed-product clarification](https://www.hdmi.org/announce/detail/82)).

HDCP 2.3 defines receiver, transmitter, and repeater behavior and is governed
by the Digital Content Protection license
([HDCP 2.3 material](https://www.digital-cp.com/sites/default/files/specifications/HDCP%202.3%20IIA%20CTS%2017%20April%2019.pdf)).
A production matrix is an HDCP repeater topology, not merely two independent
HDMI ports. It requires:

- appropriate HDMI and DCP agreements;
- licensed HDMI/HDCP IP and provisioned device keys;
- secure key storage with no Mega-visible key material;
- repeater topology, revocation, locality, and authentication behavior;
- compliance-test planning and legal review;
- a policy for capture, recording, multicast, and mixed protected/unprotected
  sinks.

IPMX is relevant because its current qualification catalog includes HDMI
InfoFrame transport, HDCP-related key exchange, privacy encryption, and USB
extension
([IPMX qualification requirements](https://ipmx.io/wp-content/uploads/ipmx-tech-library/files/IPMX-Product-Qualification-and-Certification-Requirements-v1.0.pdf)).
Study it before creating a competing wire protocol, but verify access and
licensing for every normative specification.

## Staged prototype

### Stage 0 — requirements and arithmetic

- Freeze one exact 8K format and latency budget.
- Create a spreadsheet/model for every payload and overhead.
- Decide whether the first product goal is 25 GbE JPEG XS or 100 GbE
  uncompressed.
- Obtain written licensing guidance before purchasing HDMI/HDCP IP.

Exit evidence: reviewed bandwidth budget, timing budget, and legal gate list.

### Stage 1 — software packet laboratory

- Generate deterministic color bars, zone plates, ramps, text, noise, and
  moving edges.
- Packetize recorded/generated frames as RTP.
- Route multicast through a managed switch.
- Inject loss, duplication, reordering, jitter, PTP offset, and route changes.
- Build CLI-only capture, replay, inspection, and report targets.

Exit evidence: bit-exact reconstruction for the uncompressed mode and stable
fault classification without HDMI hardware.

### Stage 2 — FPGA 4K loopback

- Bring up HDMI 2.0 or DisplayPort test input/output if that is what the
  available board supports.
- Add internal test pattern, frame CRC, packet sequence counters, buffer
  watermark, and visible fault overlay.
- Demonstrate 10/25 GbE endpoint-to-endpoint transport.

Exit evidence: measured glass-to-glass latency, hours-long soak, cable pulls,
switch reboot, PTP loss, and safe recovery.

### Stage 3 — 8K generated/unprotected input

- Move to HDMI 2.1 FRL-capable hardware.
- Prove the uncompressed 100 GbE reference or JPEG XS 25 GbE path.
- Preserve audio and required metadata.
- Validate negotiated sampling and depth at both ends.

Exit evidence: analyzer-confirmed 8K60 format, pixel/CRC tests, latency
distribution, and no-drop soak.

### Stage 4 — matrix semantics

- Add at least two transmitters and two receivers.
- Implement IS-04 discovery and IS-05 staged/bulk route changes.
- Test multicast fan-out and synchronized switching.
- Add Mega route panel, circuit-native status, watchdog, and CLI logs.

Exit evidence: deterministic salvo activation, named failure modes, and route
state that survives controller restart.

### Stage 5 — resilience and compression comparison

- Add dual A/B networks and seamless protection if required.
- Evaluate JPEG XS rates against uncompressed reference images.
- Quantify latency, power, artifacts, and repeated-generation behavior.
- Establish switch admission control and worst-case route capacity.

Exit evidence: declared quality operating point and fault-injection report.

### Stage 6 — licensed product investigation

- Engage HDMI LA, DCP, FPGA/IP vendors, and a compliance laboratory.
- Design secure provisioning and repeater behavior.
- Do not merge protected-media code or keys into the open teaching library.

Exit evidence: legal approval, licenses, threat model, and compliance plan.

## Tests that define success

| Property | Evidence |
|---|---|
| Format honesty | Analyzer and endpoint registers agree on resolution, rate, sampling, depth, HDR, and audio |
| Pixel integrity | Deterministic frame CRC or pixel diff against generated reference |
| Compression quality | Objective metrics plus reviewed text, ramps, noise, HDR, and repeated generations |
| Latency | Photodiode/high-speed-camera or electrical marker distribution, not one best sample |
| Synchronization | PTP offset record and simultaneous-output frame marker |
| Network correctness | Zero unexplained sequence loss; deliberate loss produces the declared visible fault |
| Safe failure | Cable/PTP/source loss yields bounded freeze, black, or pattern behavior |
| Route atomicity | All members of a salvo activate on the declared media-time boundary |
| Observability | Local LEDs/overlay/test points identify control, media, timing, and thermal states without Serial |
| Recovery | Switch, endpoint, controller, and Mega reset tests return to a documented state |

## Immediate recommendation

Build the project as two compatible laboratories:

1. A software/NIC ST 2110-style packet laboratory with generated 8K frames,
   PTP, multicast, and fault injection.
2. A 4K FPGA endpoint that proves the exact architecture and observability
   model before buying 8K HDMI 2.1 IP and high-end boards.

Target JPEG XS over 25 GbE for the first useful matrix and preserve a 100 GbE
uncompressed mode as the correctness oracle. Treat Arduino as a replaceable,
testable control panel attached to a self-diagnosing endpoint. That makes the
project ambitious but decomposable: each stage produces reusable networking,
timing, observability, and control components even if the licensed 8K HDMI
endpoint remains commercially impractical.

## Open decisions

- Is the primary experience gaming/desktop interaction, live production, or
  signage? Latency and compression priorities differ.
- Must 8K60 4:4:4 10-bit be uncompressed, or is tested visually-lossless JPEG
  XS acceptable?
- Is seamless dual-network protection required?
- Are protected consumer sources in scope for an eventual commercial product?
- Is the first endpoint FPGA-only, adaptive SoC, or server/NIC plus capture
  hardware?
