# Network matrix research constraints

These constraints apply to the proposed 8K video and USB 3 switched-network
research. They are architecture and publication guidance, not legal advice or a
claim of product compliance.

## Honest scope

The Mega 2560 is the deterministic management and observation plane. It may
select routes, read health signals, drive status outputs, supervise reset and
safe-state transitions, and expose a CLI. It cannot terminate, buffer, inspect,
or forward either high-rate payload.

The data plane requires purpose-built PHYs plus an FPGA, SoC, ASIC, or
commercial extender endpoint:

- HDMI 2.1b advertises as much as 48 Gbit/s and uncompressed 8K60. HDMI 2.2
  raises the interface ceiling to 96 Gbit/s. Do not describe a design as
  "full 8K" without naming resolution, refresh, chroma, bit depth, HDR, audio,
  compression, latency, and link redundancy.
- USB 3.2 defines 5, 10, and 20 Gbit/s signaling modes. "Full USB 3" is
  ambiguous; state the required signaling rate, host/device classes, transfer
  types, power behavior, hot-plug behavior, and latency tolerance.
- A switched fabric must budget payload, encoding and packet overhead,
  synchronization, retransmission or forward-error correction, and
  head-of-line blocking. A link rate equal to the connector headline rate is
  not an adequate design budget.

Prototype claims use the progression `control-plane simulation`, `data-plane
emulation`, `single-link bench prototype`, then `matrix interoperability
prototype`. Never call a prototype compliant, certified, lossless, transparent,
or production-ready without the corresponding evidence.

## Licensed interoperability

Use documented APIs and legally obtained components. Keep restricted
specifications, keys, test vectors, and vendor materials out of the repository.

For HDMI:

- Access to the current HDMI specification and HDMI trademark rights are
  benefits of the HDMI Adopter license. Public documentation may describe
  concepts and measured behavior but must not reproduce restricted material.
- A protected-content path is a licensed HDCP transmitter, receiver, or
  repeater design. Do not extract keys, bypass authentication, decrypt protected
  content, downgrade protection, or publish circumvention mechanisms.
- Initial research uses self-generated, unprotected test patterns. Protected
  source testing is deferred until the complete path uses licensed components
  under the applicable agreements.
- Preserve source/sink semantics such as hot-plug detection, capability
  exchange, link training, audio, and control channels; enumerate exactly which
  functions a prototype does and does not carry.

For USB:

- Use an assigned vendor ID for a product identity. Do not borrow another
  vendor's identifier.
- Do not display USB-IF logos or call a product certified unless the company has
  the applicable trademark agreement and that product has passed the compliance
  program and is listed.
- Treat the network tunnel as a distributed host-controller problem, not an
  electrical cable extension. Preserve enumeration, endpoint state, transfer
  ordering, cancellation, reset, suspend/resume, hot-plug, and error semantics.
- Test only devices and hosts the operator owns or is authorized to connect.
  Protocol observation must not become credential capture, access-control
  bypass, firmware extraction, or unauthorized device impersonation.

United States copyright law includes a narrow interoperability provision for
reverse engineering of lawfully obtained computer programs, subject to stated
conditions. It is not a general authorization to defeat HDCP or other access
controls. Obtain qualified counsel before any work involving circumvention,
restricted specifications, protected media, or distribution of reverse-
engineering results.

## Electrical and physical safety

- Keep high-speed signaling off Mega pins. Use rated PHYs, connectors,
  controlled-impedance boards, ESD protection, and vendor reference designs.
- Never connect two independently sourced USB VBUS rails. Each port needs an
  explicit power role, current limit, over-current shutdown, discharge behavior,
  and backfeed protection. USB-C and Power Delivery use compliant controllers,
  not GPIO bit-banging.
- Define common-ground or galvanic-isolation boundaries before connecting
  equipment on different mains circuits. Bench development uses protected,
  current-limited supplies.
- HDMI and multi-gigabit USB layouts require signal-integrity, EMC, thermal, and
  ESD review. Production hardware may also require applicable equipment
  authorization and safety evaluation; using a certified module does not
  automatically certify the finished product.
- A failed controller, lost network, brownout, watchdog reset, or partial boot
  must release routes and port power into documented safe states.

## Security boundary

A network USB matrix can remotely attach keyboards, storage, cameras, network
adapters, and other privileged peripherals. Treat every endpoint and peripheral
as hostile until authorized.

The baseline design requires:

- mutual device identity and authenticated, encrypted transport;
- explicit allowlists for user, source, destination, and USB device class;
- physical confirmation for a new route or newly seen peripheral;
- one authoritative owner for each routed USB device;
- replay protection, monotonic session state, bounded queues, and fail-closed
  reconnect behavior;
- signed firmware, verified boot where supported, rollback protection, and
  secret rotation;
- management/data-plane separation, least privilege, rate limits, and auditable
  route changes;
- deliberate handling of DMA-capable devices, HID injection, malicious
  descriptors, storage exfiltration, cameras, microphones, and firmware-update
  interfaces.

Video routing still needs authentication, authorization, confidentiality where
required, tamper-evident route logs, and a clear policy for capability metadata.
Do not imply that HDCP alone secures the control plane or network fabric.

## Evidence required for each milestone

Every experiment has a circuit-native observation path in addition to logs:
link/route/fault LEDs, test points, or analyzer outputs. Record:

1. the exact topology, hardware revisions, firmware commit, and test media or
   peripheral;
2. negotiated modes and measured useful throughput;
3. end-to-end latency, jitter, loss, reconnect, and route-switch behavior;
4. unplug, reset, brownout, congestion, malformed-input, and unauthorized-route
   results;
5. port power, temperature, safe shutdown, and recovery evidence;
6. unsupported features and all tests not yet run.

Serial and network logs support the evidence but are never the only indication
that a route is correct or safe.

## Primary references

- [HDMI Adopter overview](https://www.hdmi.org/adopter/index)
- [HDMI 2.1b capabilities](https://www.hdmi.org/spec/hdmi2_1/index.aspx)
- [HDMI 2.2 bandwidth and resolutions](https://www.hdmi.org/spec2sub/res-bandwidth)
- [Digital Content Protection licensing](https://www.digital-cp.com/licensing)
- [USB-IF compliance program](https://www.usb.org/compliance)
- [USB-IF logo license](https://usb.org/logo-license)
- [USB-IF vendor IDs](https://www.usb.org/getting-vendor-id)
- [USB 3.2 naming and rates](https://www.usb.org/sites/default/files/usb_3_2_language_product_and_packaging_guidelines_final.pdf)
- [17 U.S.C. section 1201](https://uscode.house.gov/view.xhtml?edition=prelim&f=treesort&fq=true&hl=false&num=0&req=granuleid%3AUSC-2021-title17-section1201)
- [NIST guidance for securing network connections](https://www.nist.gov/itl/smallbusinesscyber/guidance-topic/securing-network-connections)
- [Arduino Mega 2560 Rev3 datasheet](https://docs.arduino.cc/resources/datasheets/A000067-datasheet.pdf)
