# Roadmap

`CURRICULUM.md` is the canonical lesson and project numbering authority.
`COMPONENTS.md` owns the target component inventory.
`WORK_QUEUE.md` is the authoritative active, queued, deferred, physical, and
publication ledger.

## Current host-verified slices

1. no-exception status, wrap-safe time, fixed resource claims, and runtime;
2. `DigitalOutput` and `MonoLed` for early visible diagnostics;
3. `DigitalInput` and deterministic `Button`;
4. deterministic Reaction Timer with lessons 001–003;
5. `PwmOutput`, `RgbLed`, and shared default-timer leases;
6. timer ownership and the nonblocking `PiezoSounder`;
7. deterministic `Simon`, fixed and seeded cue sources, and lessons 004–006;
8. `AnalogInput` with explicit raw sampling and Mega analog-pin validation;
9. deterministic calibration and sampled filtering without hidden time;
10. adaptive `NightLight` intent with hysteresis, bounded duty, explicit
    invalid-sample behavior, and lessons 007–009;
11. shift-register and seven-segment ownership, explicit traffic timing, and
    the traffic-junction project in lessons 010–012;
12. validated climate samples and the owned DHT11 adapter in lesson 013;
13. staged character-display output and stable records in lesson 014;
14. deterministic environmental-station composition in lesson 015;
15. release-gated matrix keypad input in lesson 016;
16. bounded servo intent, versioned configuration records, owned Timer5/D44
    pulse output, and explicit external-power admission in lesson 017;
17. inert access policy with keypad input, visible soft-latch intent, lockout,
    and bounded audit presentation in lesson 018;
18. explicit ultrasonic echo timing, timeout, and range validity in lesson 019;
19. bounded motor intent with reversal dead time and stop dominance in lesson
    020; and
20. deterministic range-aware rover supervision with inert command evidence in
    lesson 021; and
21. owned I2C/SPI devices, direct Mega bus adapters, calibrated moisture
    observations, explicit RTC state, and fixed durable records in lesson 022;
    and
22. transactional inert LED loads and deterministic watering policy in lesson
    023; and
23. coherent greenhouse stages, visible health patterns, and durable record
    retry in lesson 024; and
24. owned bounded infrared capture, classic NEC validity, and stable
    receive-only records in lesson 025; and
25. exact receive-only telemetry packets, freshness tracking, bounded packet
    reception, and visible evidence in lesson 026.

The component APIs and behavior engines pass deterministic host tests and
compile for the Mega 2560. Physical acceptance cards remain open, so this work
is experimental rather than hardware supported.

## Next slice

1. deterministic inert cue scheduling in lesson 029.

Later slices follow the canonical three-lesson cadence through analog sensing,
displays, environmental records, bounded actuators, buses and storage,
receive-only observations, and the inert show-cue simulator.

Every component requires lifecycle tests, deterministic fakes, a canonical Mega
example, size evidence, terse HTML, a rich complementary PDF, and recorded
hardware acceptance. Work that misses a gate remains experimental.

ADK will not implement pyrotechnic ignition, launcher control, remote cloning,
or unknown-protocol transmission.

## Parallel research tracks

Two longer-range investigations sit outside the 30-lesson support promise:

- full 8K HDMI transport and switching over a packet network;
- a many-port USB 3 switching matrix over a packet network.

Both require dedicated high-speed transceivers and FPGA/SoC or Linux-class
data-plane hardware. The Mega 2560 is appropriate only for a deterministic
control panel, status display, power/environment observation, and fault
injection. It cannot terminate HDMI or USB SuperSpeed links.

Research proceeds from standards and measurements, then a one-to-one synthetic
prototype, then authenticated switching. HDMI work starts with unprotected
generated video; USB work starts with owned loopback devices on an isolated
network. Compliance, licensing, content protection, device authorization,
signal integrity, thermal design, electromagnetic compatibility, bandwidth,
latency, and failure recovery are release gates rather than cleanup tasks.

These tracks may produce architecture notes and host simulations before they
produce hardware. They do not change the current API status, safety boundary,
or lesson cadence.
