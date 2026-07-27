# HDMI 2.1 and 8K mesh data plane

Status: architecture research; no hardware or interoperability claim  
Scope: dynamically routed HDMI source and sink endpoints over switched Ethernet  
Controller: ordinary Linux service, one durable authority, never in the media
path; high availability is deferred

## Decision

Do not forward HDMI electrical symbols through the packet network. Terminate
the source-facing HDMI link, recover its media and metadata, packetize timed
essences, route those packets through an admitted network path, and construct a
new HDMI link at every sink-facing endpoint.

AMD's HDMI 2.1 receiver is a concrete example of this boundary: its PHY and
receiver accept TMDS or FRL, remove the HDMI link coding and FEC, and expose
native or AXI4-Stream video plus an AXI4-Stream audio interface. Its transmitter
performs the reverse system function from video and audio streams, choosing
TMDS or FRL for the attached sink. These are independent HDMI links; the
network between them is not an HDMI cable extension.

```text
HDMI source
    -> RX PHY: TMDS or FRL termination and training
    -> RX core: video, audio, timing, InfoFrames, link status
    -> media normalization and timestamping
    -> packetizer and traffic shaper
    -> admitted switched-Ethernet path
    -> jitter buffer and clock reconstruction
    -> media and metadata scheduler
    -> TX core: InfoFrames, audio, video, TMDS or FRL generation
    -> TX PHY and independently trained HDMI link
    -> HDMI sink
```

The Arduino Mega 2560 may operate buttons, lamps, and low-rate diagnostic
signals. It cannot process HDMI symbols, media packets, Precision Time Protocol
timestamps, EDID, or protected-content keys.

## Mesh objects

The data plane follows the USB mesh's dynamic endpoint model without borrowing
USB's exclusive-device semantics:

| Object | Stable identity | Runtime facts |
|---|---|---|
| Ingress endpoint | enrolled node and HDMI input port | source present, TMDS/FRL mode, format, HDCP state |
| Media flow | controller-issued flow identity | video/audio/metadata profile and measured packet rate |
| Egress endpoint | enrolled node and HDMI output port | sink present, EDID digest, accepted modes, link state |
| Route | source flow plus one or more egresses | epoch, admitted path, active profile, health |
| Fabric path | switch ports and traffic class | capacity, MTU, reservation, loss and latency budget |

Nodes may contain ingress and egress ports simultaneously. Sources and
destinations can be enrolled, withdrawn, and reconfigured while the mesh
continues operating. A source epoch fences stale commands. The initial
controller is singular and durable; endpoints do not elect a replacement or
create routes during controller loss. Mutations stop immediately; only the
exact unchanged installed route may run through its pre-authorized bounded
lease, after which its egress mutes.

Phase one connects one ingress to one egress. Phase-three fan-out may feed
several authorized egresses when bandwidth and content policy permit multicast.
Each egress consumes at most one active video route. Changing an egress between
flows is a visible source change, not seamless HDMI state migration.

## Ingress: terminate and interpret

The ingress owns the source-facing sink behavior:

1. expose controller-approved EDID;
2. handle Hot Plug Detect and Data Display Channel transactions;
3. train either TMDS or FRL at an explicitly supported rate;
4. validate the recovered link and report FEC or link errors;
5. separate active video, audio, timing, auxiliary packets, and InfoFrames;
6. identify chroma sampling, component depth, frame rate, HDR metadata, audio
   format, variable-refresh behavior, and DSC state;
7. timestamp the recovered media against the endpoint's disciplined clock.

The receiver must not advertise a mode merely because the HDMI PHY accepts it.
EDID is an admission promise covering the entire route: receiver, processing,
network, egress transmitter, and sink. A route change that invalidates that
promise requires a deliberate HPD/EDID renegotiation policy.

TMDS and FRL end at ingress. FRL FEC parity and 16b/18b coding are link-local
and are not transported as the network payload. The egress later generates
fresh FRL packets, FEC, and coding if its sink selects FRL.

## Network representation

Keep the media plane independent of the HDMI wire representation:

- video essence: active samples plus complete format and timing descriptors;
- audio essence: samples, channel description, sample rate, and timestamps;
- metadata essence: every supported HDMI InfoFrame or auxiliary item needed to
  reproduce the presentation;
- event channel: source presence, format change, link failure, and source epoch;
- clock relation: timestamps that bind every essence to the same media time.

IPMX TR-10-10 defines one primary reference for transporting HDMI InfoFrames
through an IP media system. The mesh must publish an explicit support table
rather than silently dropping unknown InfoFrames. Unsupported required
metadata makes the route incompatible. Metadata that can safely be omitted
must be named as such in the selected profile.

DSC requires an explicit route mode:

- **DSC pass-through:** preserve a compatible compressed stream and its
  parameters end to end;
- **DSC decode:** decode at ingress and carry normalized samples;
- **DSC transcode:** decode and encode to a separately admitted network
  profile.

The presence of DSC pass-through ports in HDMI endpoint IP is not evidence that
the design contains a DSC decoder or encoder. Those are separately licensed,
sized, timed, and verified functions. A route is rejected unless both endpoint
implementations agree on the same DSC treatment.

## Bandwidth envelope

The following rates are derived from active pixels only:

```text
activeRate = 7680 * 4320 * 60 * bitsPerPixel
```

| 8K60 active video | Bits per pixel | Active payload |
|---|---:|---:|
| RGB/YCbCr 4:4:4, 8-bit components | 24 | 47.776 Gb/s |
| RGB/YCbCr 4:4:4, 10-bit components | 30 | 59.720 Gb/s |
| YCbCr 4:2:2, 10-bit components | 20 | 39.813 Gb/s |
| YCbCr 4:2:0, 10-bit components | 15 | 29.860 Gb/s |

These figures exclude packet headers, Ethernet framing, interpacket gaps,
audio, metadata, redundancy, forward-error correction, traffic shaping, and
operational headroom. Admission uses the measured on-wire profile rate, not
this table.

Consequences:

- 25 GbE cannot carry any listed uncompressed 8K60 profile, including
  4:2:0 10-bit, on one link.
- 25 GbE is suitable for lower formats or a bounded low-latency compressed
  profile such as JPEG XS; codec latency, quality, and peak packet rate become
  part of admission.
- 100 GbE can contain one listed active-video payload, but the implementation
  still needs a proved on-wire budget. Redundant packet paths or multiple 8K
  flows require more fabric capacity.
- Link aggregation is not a promise that one flow will use the sum of member
  links. A striped media transport must be designed and measured explicitly.

HDMI.org states that HDMI 2.1b supports link bandwidth up to 48 Gb/s. That wire
rate is not equivalent to 48 Gb/s of active pixels. A named 8K mode must always
include resolution, refresh, chroma, component depth, DSC state, and network
profile; “full 8K” is not an admission profile.

## Admission and switching

A controller may activate a route only after all of these checks pass:

1. ingress reports a stable, supported, unprotected or lawfully handled mode;
2. every egress reports a compatible sink and transmitter capability;
3. the selected media profile has a bounded peak packet rate;
4. every fabric hop has reserved capacity and a compatible MTU/traffic class;
5. queue, jitter-buffer, and clock-recovery budgets fit the latency target;
6. metadata, audio, HDR, variable-refresh, and DSC policies are compatible;
7. the new source epoch has reached all participating endpoints;
8. egress remains muted until the new stream and clock are stable.

USB, HDMI, controller, telemetry, and ordinary household traffic share the same
managed switched network. Admission preserves configured ordinary-LAN
headroom. A route profile states format, codec, peak rate, latency, jitter,
buffering, and quality bounds. Policy may pin one profile, allow an ordered
set, or select the best profile within explicit bounds; it never changes mode
silently or disturbs a working route when a candidate cannot be admitted.

Make-before-break multicast may prepare a new network subscription, but an
egress must never present samples from two source epochs. On loss, stale epoch,
buffer underflow, incompatible format, or clock unlock, the transmitter enters
the documented mute/test-pattern state and reports the cause.

Fabric scaling depends on traffic replication:

- ingress replication spends one uplink copy per destination;
- switch multicast spends one copy on shared links and replicates at branches;
- hitless redundant transport sends two independently routed copies;
- oversubscribed paths are rejected rather than tested with live media.

## Clocks and buffering

The source pixel and audio clocks are not forwarded electrically. The ingress
maps recovered media timing to a disciplined network timebase. The egress
uses timestamps and its local oscillator to schedule samples and regenerate
the HDMI video and audio clocks.

The proposed endpoint design therefore needs:

- a documented network synchronization profile and holdover limit;
- timestamp placement at a stable media boundary;
- elastic buffering for packet-delay variation;
- overflow and underflow detection;
- bounded rate matching without hidden frame insertion or deletion;
- explicit resynchronization after a format or route change;
- audio/video phase-error and drift limits;
- buffer-depth and worst-case latency evidence.

Line-scale buffering reduces latency but leaves little tolerance for network
jitter. Frame-scale buffering simplifies switching and recovery but adds about
16.67 ms per frame at 60 Hz. The product profile must choose; it cannot promise
both arbitrary switched-network jitter and effectively zero buffering.

Variable Refresh Rate is not ordinary constant-rate video. Defer it until the
timestamp model, egress scheduler, metadata transport, and sink behavior have a
separate tested profile.

## Egress: reconstruct a fresh link

The egress owns the sink-facing source behavior:

1. read and fingerprint the attached sink's EDID;
2. report HPD and sink capabilities to the controller;
3. admit only a media profile the sink and transmitter can reproduce;
4. lock its media scheduler and regenerated clocks before unmuting;
5. schedule video, audio, InfoFrames, and supported auxiliary data together;
6. select and train TMDS or FRL for this sink;
7. generate fresh TMDS coding or FRL packetization, FEC, and 16b/18b coding;
8. expose link, buffer, clock, metadata, and source-epoch evidence.

The endpoint display names requested and applied profile, link rate, latency
class, and exact fault without relying on color. Loss of a pinned profile's
active contract blanks video and mutes audio until that exact profile can be
admitted and retrained. Disabled-by-default deterministic fault injection uses
separate authorization, a bounded duration, an unmistakable `TEST` indication,
and distinct audit events; real faults always take precedence.

The output need not use the input's HDMI link mode. A TMDS ingress can feed an
FRL egress, or the reverse, when the recovered media format and both endpoint
profiles permit it. The media contract, not copied link symbols, connects them.

## Protected-content boundary

The first research profile accepts only generated or otherwise authorized
unprotected content. It contains no HDCP keys and does not attempt to remove,
bypass, inspect, or replay protected media.

Protected content is a later licensed product boundary:

- HDMI Adopter status governs access to the complete HDMI specification and
  adopter benefits;
- HDCP receiver, repeater, and transmitter functions require the applicable
  licenses, agreements, key handling, and approved implementation;
- encrypted HDMI cannot be terminated, exposed as clear network media, and
  reconstructed by an unlicensed shortcut;
- topology, authentication, revocation, key storage, and protected transport
  require specialist legal and security review;
- IPMX TR-10-5 is relevant to an authorized HDCP-capable IPMX system, not
  permission to implement HDCP independently.

HDCP protects content, not route authorization, endpoint identity, controller
integrity, or network confidentiality. The mesh still needs mutually
authenticated nodes, authorized routes, encrypted control traffic, auditable
epochs, and a separately designed media-network security policy.

## Evidence plan

Advance in measured stages:

1. synthetic timestamped color bars and audio over a host network model;
2. FPGA packet generator to receiver with loss, reorder, jitter, and epoch
   injection;
3. unprotected 1080p TMDS ingress to one egress;
4. dynamic one-to-one route moves without mixed epochs;
5. phase-three one-to-many fan-out with independent sink reservations;
6. 4K FRL with InfoFrames, audio, clock recovery, and fault evidence;
7. compressed 8K over 25 GbE and uncompressed 8K over 100 GbE as separate
   profiles;
8. independent HDMI, Ethernet, latency, clock, thermal, and interoperability
   measurements.

No phase may claim HDMI compliance, 8K operation, losslessness, bounded
latency, or sink interoperability before its instruments and recorded results
support that exact statement.

## Primary references

- [AMD HDMI 2.1 Receiver Subsystem PG351][amd-rx] — TMDS/FRL receive paths,
  FEC removal, video/audio stream boundaries, EDID, clocks, and auxiliary data
- [AMD HDMI 2.1 Transmitter Subsystem PG350][amd-tx] — video/audio input
  streams, TMDS/FRL generation, FEC, clocks, DDC, and auxiliary data
- [AMD HDMI 2.1 Transmitter features][amd-tx-features] — supported InfoFrames,
  dynamic HDR, gaming features, audio, and DSC pass-through
- [HDMI 2.1b overview][hdmi-21] — public feature and 48-Gb/s capability
  summary
- [HDMI Adopter overview][hdmi-adopter] — specification access, licensing,
  trademarks, and adopter benefits
- [VSF TR-10-10][tr-10-10] — IPMX carriage of HDMI InfoFrames
- [VSF TR-10-5][tr-10-5] — HDCP key exchange within the licensed IPMX boundary
- [AMD 100G Ethernet subsystem][amd-cmac] — FPGA 100-Gigabit Ethernet MAC/PCS
  implementation boundary

[amd-rx]: https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/HDMI-2.1-Receiver
[amd-tx]: https://docs.amd.com/r/en-US/pg350-v-hdmi-txss1/HDMI-2.1-Transmitter
[amd-tx-features]: https://docs.amd.com/r/en-US/2026.1/pg350-v-hdmi-txss1/Features
[hdmi-21]: https://www.hdmi.org/spec/hdmi2_1
[hdmi-adopter]: https://www.hdmi.org/adopter/index
[tr-10-10]: https://static.vsf.tv/download/technical_recommendations/VSF_TR-10-10_2024-10-07.pdf
[tr-10-5]: https://static.vsf.tv/download/technical_recommendations/VSF_TR-10-5_2022-03-22.pdf
[amd-cmac]: https://www.amd.com/en/products/adaptive-socs-and-fpgas/intellectual-property/cmac.html
