# 8K HDMI over a switched network

Status: architecture study, not an ADK supported interface.

The useful project is not “Arduino transports HDMI.” A Mega 2560 cannot touch
the media data rate. The credible split is an FPGA or adaptive SoC for every
pixel and packet, a managed Ethernet fabric for switching, and an Arduino-class
controller for buttons, status lamps, environmental sensing, and deliberate
fault injection. That split gives ADK a rich operator-panel project without
pretending an 8-bit controller is a video processor.

## Define “full”

The network should fix one demanding internal operating point:

- 7680 × 4320 at 60 progressive frames/s;
- RGB or YCbCr 4:4:4, 10 bits/component after HDMI receive processing;
- uncompressed active video on the network;
- embedded HDMI audio separated into its own network essence;
- HDR and HDMI InfoFrame preservation;
- one-to-one, one-to-many, and many-to-one routing;
- deterministic switching, PTP timing, and optional path redundancy;
- no HDCP in the laboratory profile.

“HDMI 2.1” alone is not a capability statement. Products may implement only a
subset. The endpoint must explicitly report FRL rate, format, chroma, depth,
audio, HDR, EDID behavior, HDCP state, and whether VRR is accepted. HDMI 2.1b
supports 8K60 and up to 48 Gb/s; HDMI 2.2 raises the ceiling, so a later product
should not bake 48 Gb/s assumptions into its internal interfaces.

There is an important boundary: 8K60 4:4:4 10-bit active pixels alone are
59.72 Gb/s, so that exact format cannot enter uncompressed through a 48-Gb/s
HDMI 2.1 link. An HDMI 2.1 prototype must receive a lower-chroma/depth mode or
decode DSC, then may normalize it to the 4:4:4 network representation. A strict
uncompressed 8K60 4:4:4 10-bit HDMI input requires the 96-Gb/s HDMI 2.2 class
and different endpoint hardware. Calling either one “full 8K” would conceal
the most important design choice.

HDCP is a separate licensed product problem, not an engineering shortcut.
AMD restricts its HDCP IP to listed adopters, and IPMX defines a key exchange
system for protected RTP content. Phase one therefore accepts only owned,
unencrypted test patterns and source material.

## Rate and storage budget

Active-image rate is:

`width × height × frames/s × bits/pixel`

| 8K60 active image | Bits/pixel | Rate | One active frame |
|---|---:|---:|---:|
| 4:4:4, 8 bit | 24 | 47.776 Gb/s | 99.533 MB |
| 4:4:4, 10 bit | 30 | 59.720 Gb/s | 124.416 MB |
| 4:4:4, 12 bit | 36 | 71.664 Gb/s | 149.299 MB |
| 4:2:2, 10 bit | 20 | 39.813 Gb/s | 82.944 MB |
| 4:2:0, 10 bit | 15 | 29.860 Gb/s | 62.208 MB |

These are payload floors, not switch-port reservations. Ethernet preamble,
inter-packet gap, Ethernet/IP/UDP/RTP headers, VLAN tags, media packet headers,
audio, ancillary streams, and headroom all add load. At 59.720 Gb/s, a
1440-byte media payload is about 5.18 million packets/s; an 8880-byte payload is
about 0.84 million packets/s. Jumbo frames are therefore a practical endpoint
requirement even though the protocol design should not make them semantically
necessary.

A 25 GbE link cannot carry even 8K60 4:2:0 10-bit uncompressed. A single
100 GbE port can carry 4:4:4 10- or 12-bit active video with useful headroom.
Reserve no more than 75% of a link for planned media. A nonblocking 32-port
100 GbE leaf has 3.2 Tb/s in each direction and can sustain at most about 40
independent 59.72-Gb/s senders under that policy, before fabric oversubscription
and receiver fan-out are considered.

Useful buffer scales:

- one 4:4:4 10-bit active frame: 124.416 MB;
- one line: 28,800 packed bytes;
- one line time at 8K60: about 3.858 microseconds;
- 1 ms of a 59.72-Gb/s flow: 7.465 MB;
- 2 ms jitter or redundant-path skew: 14.930 MB.

Do not add frame memory merely because it is available. A full-frame store adds
up to 16.67 ms before display scanout. Packet and line FIFOs should absorb
bounded serialization, switch jitter, and clock phase; frame storage is for
explicit format conversion, asynchronous domains, or test capture.

## Recommended media architecture

Use IPMX and the SMPTE ST 2110 family as the interoperability direction:

```text
HDMI source
    |
HDMI 2.1 RX + FRL/FEC + InfoFrames
    |
pixel/audio/metadata split
    |
PTP timestamp + ST 2110-20/30/41 packetizers
    |
100 GbE A fabric --------------+
100 GbE B fabric (optional) ---+--> packet merge + elastic FIFOs
                                      |
                             scheduled pixel/audio/metadata
                                      |
                               HDMI 2.1 TX + EDID
                                      |
                                  HDMI sink
```

ST 2110-20 carries uncompressed active video over RTP. ST 2110-21 defines
sender traffic shaping. Audio and ancillary/metadata remain separate flows so
they can be routed and diagnosed independently. IPMX adds pro-AV discovery,
connection management, HDMI InfoFrame transport, and an evolving
interoperability test suite. Keep packetization blocks replaceable: the same
endpoint should later host ST 2110-22 with JPEG XS.

For a bandwidth-reduced variant, 4:1 JPEG XS makes 4:4:4 10-bit approximately
14.93 Gb/s before packet overhead and fits 25 GbE. A 10 GbE target needs more
than 6:1 merely to cross the payload threshold, so target at least 7:1 and
measure quality. Compression ratio is a design operating point, not a
guaranteed wire rate. The uncompressed 100 GbE build should establish correct
timing and routing before a codec obscures failures.

## Clock recovery and latency

The HDMI receiver recovers and validates the incoming FRL link. Crossing the
network creates a new timing domain: RTP timestamps refer to a PTP-aligned
media clock, and the transmit endpoint regenerates HDMI timing from a
PTP-disciplined oscillator. A FIFO absorbs phase and bounded frequency error.

Two free-running sources cannot remain joined forever with only a finite FIFO.
For genlocked fixed-rate sources, servo the output timing without changing
sample order. For non-genlocked or VRR sources, either preserve variable timing
as an explicitly supported mode or perform a documented frame repeat/drop.
Never hide this policy behind “clock recovery.”

Set an initial fixed-format glass-to-glass budget:

| Stage | Design allowance |
|---|---:|
| HDMI RX and line assembly | 0.10 ms |
| packetization and sender FIFO | 0.25 ms |
| two lightly loaded switch hops | 0.10 ms |
| receiver reorder/merge FIFO | 1.00 ms |
| HDMI TX scheduling | 0.25 ms |
| scan phase, excluding the display's processing | 0–16.67 ms |

The transport target is therefore under 1.7 ms before scan phase, measured at
named FPGA timestamps. A compressed endpoint receives a separate budget after
the chosen JPEG XS implementation supplies measured line latency.

## Switching, multicast, and protection

Each video, audio, and metadata essence receives its own multicast group and
RTP port set. Receivers join only selected flows. The switches must support
100 GbE at the endpoints, IGMP snooping with a known querier, adequate multicast
tables, QoS queues, PTP boundary or transparent clock behavior, telemetry, and
nonblocking capacity for the intended fan-out. Validate a specific switch;
feature names do not prove lossless media behavior.

Use make-before-break switching:

1. validate receiver capability against the sender's advertised format;
2. join the destination flows;
3. establish timestamp lock and fill the elastic buffer;
4. switch at a frame boundary;
5. leave the old flows after stable output.

For high availability, send identical sequence-numbered RTP packets through
two physically independent fabrics and merge them at the receiver using the
ST 2022-7 model. Each fabric must carry the entire load. The merge buffer is
sized for measured maximum path skew, not average jitter. Redundant media
does not make PTP redundant by itself; provide independent grandmaster paths
and test clock failover separately.

## Endpoint hardware shortlist

| Role | Candidate | Why | Limit or question |
|---|---|---|---|
| Full 8K endpoint | AMD VEK385 | HDMI 2.1 input/output, QSFP28, SFP28, 100G multirate MACs, 20 GB LPDDR5X | $15,995 list price; new platform; prove that HDMI and 100G lanes/IP can operate together |
| Lower-cost architecture rehearsal | AMD VEK280 | HDMI 2.1 input/output and SFP28; available HDMI reference material | 25 GbE implies JPEG XS or a reduced format; $6,995 list price |
| 100G packet-engine exploration | AMD VCU128 | four QSFP-class connectors, deep FPGA and memory resources | HDMI requires an FMC and integration; older, costly platform |
| 4K/IPMX control-plane reference | Macnica MPA1000/ME10 kit | working IPMX/JPEG XS/NMOS concepts and REST control | HDMI 2.0, 4K, 1 GbE, USB 2.0; not an 8K dataplane |
| Operator/diagnostic controller | Arduino Mega 2560 + ADK | buttons, status lamps, fault switches, thermal/fan alarms | Never handles pixels, packets, PTP, EDID, or HDCP keys |

Prototype BOM for two endpoints:

- two VEK385 kits;
- one 100 GbE managed switch with multicast and PTP features;
- two 100GBASE-SR4 optics plus appropriate multimode fiber, or validated
  passive DACs for a same-rack build;
- two certified Ultra High Speed HDMI cables and unencrypted 8K pattern source;
- one 8K sink whose supported modes are documented;
- PTP grandmaster or a laboratory PTP-capable clock source;
- host workstation for synthesis, capture, NMOS control, and automated tests;
- Mega 2560 operator panel with E1 LEDs/buttons only;
- optional second switch, optics, and links for ST 2022-7;
- high-bandwidth oscilloscope or protocol analyzer access should be rented
  rather than silently omitted from validation.

Before purchase, obtain written confirmation of the exact HDMI RX/TX, 100G MAC,
ST 2110 packetizer, PTP, JPEG XS, and tool licenses. Evaluation-board connectors
alone do not imply production-usable IP.

## Build sequence and observable evidence

1. Simulate packetization with a small synthetic raster. Prove RTP sequence,
   timestamp, row reconstruction, loss, reorder, rollover, and deterministic
   replay.
2. Loop an FPGA-generated 1080p pattern through one endpoint. Use a checkerboard
   and frame counter embedded in visible pixels; use GPIO test points for RX
   lock, PTP lock, buffer healthy, and TX active.
3. Move 1080p across one switch, then multicast to two receivers. Capture loss,
   reorder, latency, and join time from hardware counters.
4. Scale to 4K and then 8K uncompressed over 100 GbE. No stage advances until
   the exact rate and buffer high-water marks are recorded.
5. Add make-before-break switching and visible old/new source identifiers.
6. Add a second fabric and inject link, packet, PTP, and power faults. Prove
   seamless merge separately from clock continuity.
7. Add JPEG XS on 25 GbE as a parallel operating mode, not a replacement for
   the known-correct uncompressed reference.
8. Only after interoperability tests pass, evaluate HDR, audio channel maps,
   InfoFrames, VRR, EDID policy, and licensed protected-content requirements.

The circuit-native diagnostic panel should show input lock, PTP lock, network
health, buffer underflow/overflow, output lock, and active route without
requiring Serial. A test-pattern frame counter proves that pixels, not merely
control software, traversed the system. The CLI records richer counters and
timestamp traces; it complements those signals.

Suggested top-level commands for a future repository:

```text
make simulate
make bitstream BOARD=vek385 ROLE=sender
make bitstream BOARD=vek385 ROLE=receiver
make program BOARD=vek385 DEVICE=...
make discover
make route SOURCE=... SINK=...
make monitor
make capture-counters
make fault-test
make interoperability
```

## Principal risks

- HDMI and HDCP licenses can dominate the development budget.
- A dev board can expose both connectors while internal transceiver placement
  prevents the desired simultaneous design; close timing and lane planning
  before purchase.
- 100 GbE line rate is not evidence of correctly shaped ST 2110 traffic.
- multicast fan-out can oversubscribe uplinks or exhaust switch tables.
- unconstrained path skew defeats a finite ST 2022-7 merge window.
- asynchronous and VRR timing can force visible frame adaptation.
- EDID, HPD, CEC, HDR metadata, and audio clocking are product features, not
  incidental side channels.
- an ordinary television may add far more latency than the transport; measure
  endpoint timestamps separately from glass-to-glass latency.

## Primary references

- [HDMI resolutions and bandwidth](https://www.hdmi.org/spec2sub/res-bandwidth)
- [HDMI 2.2 overview](https://www.hdmi.org/spec/hdmi2)
- [SMPTE ST 2110 suite](https://www.smpte.org/standards/st2110)
- [SMPTE ST 2110-20: uncompressed active video](https://pub.smpte.org/doc/st2110-20/20170918-pub/st2110-20-2017.pdf)
- [SMPTE ST 2022-7: seamless RTP protection switching](https://pub.smpte.org/latest/st2022-7/st2022-7-2019.pdf)
- [VSF IPMX technical recommendations](https://vsf.tv/technical-recommendations/)
- [IPMX HDMI InfoFrame transport, TR-10-10](https://static.vsf.tv/download/technical_recommendations/VSF_TR-10-10_2024-10-07.pdf)
- [IPMX HDCP key exchange, TR-10-5](https://static.vsf.tv/download/technical_recommendations/VSF_TR-10-5_2022-03-22.pdf)
- [AMD HDMI connectivity and IP](https://www.amd.com/en/solutions/broadcast-and-pro-av/any-to-any-connectivity.html)
- [AMD VEK385 product brief](https://www.amd.com/content/dam/amd/en/documents/products/adaptive-socs-and-fpgas/boards-kits/product-briefs/vek385-evaluation-kit-product-brief.pdf)
- [AMD VEK385 evaluation kit](https://www.amd.com/en/products/adaptive-socs-and-fpgas/evaluation-boards/vek385.html)
- [AMD VEK280 evaluation kit](https://www.amd.com/en/products/adaptive-socs-and-fpgas/evaluation-boards/vek280.html)
- [AMD HDMI 2.1 receiver licensing](https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/License-Type)
- [Macnica FPGA live-video architecture](https://www.macnica.com/content/dam/macnicagwi/americas/mai/public/en/images/pdfs/inte/fpgas-for-live-production-workflows-white-paper.pdf)
