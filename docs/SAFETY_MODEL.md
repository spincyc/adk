# Safety model

This file is the review authority for supported ADK hardware and projects.
`site/pages/safety.md` explains the rules to learners; lessons apply them.
When documents disagree, use the stricter boundary and stop the release until
the disagreement is fixed.

ADK is a low-voltage educational library. It is not a safety controller.
Software state, RAII cleanup, tests, and deterministic replay never replace
power isolation, rated protection, or qualified supervision.

## Non-negotiable boundaries

Supported work must not:

- connect learner-built circuits directly to mains, vehicle, building, medical,
  life-safety, or public infrastructure;
- power a motor, relay coil, solenoid, heater, servo, lamp, or radio from a
  Mega 2560 I/O pin;
- switch mains or an unknown load, even when a relay module claims isolation;
- defeat, emulate, or bypass a physical interlock or certified controller;
- transmit an unknown, captured, private, protected, or safety-relevant radio
  protocol;
- connect to a pyrotechnic launcher, initiator, ignition circuit, or live
  continuity circuit;
- use energetic, combustible, explosive, high-temperature, or sharp-moving
  loads as a lesson prop;
- describe a software command, destructor, reset, or high-impedance pin as an
  emergency stop.

An agent must reject code, diagrams, examples, links, or instructions that make
one of these paths easier. Reframing the work as testing, research, simulation,
or receive-only observation does not relax the boundary.

## Energy classes

Every hardware lesson declares one class. An undeclared lesson does not ship.

| Class | Allowed work | Required controls |
|---|---|---|
| E0 | Host simulation; unpowered inspection | No hardware side effect |
| E1 | Mega 2560, USB or documented low-voltage current-limited supply, LEDs, switches, passive sensors, piezo | Exact schematic, ratings, unpowered wiring check, stop conditions |
| E2 | Externally powered servo, motor, or rated low-voltage driver; inert relay load | E1 controls plus separate load supply, driver protection, physical power removal, restrained workspace, measured current limit |
| E3 | Mains, energetic devices, ignition, certified safety systems, unknown RF control | Outside ADK; no implementation or bench procedure |

E2 work remains draft until a named person records physical bench acceptance.
CI, compilation, photos, and an agent review cannot supply that evidence.

## Component gates

### Outputs and LEDs

- Construction is electrically inert.
- Capability checks precede claims and writes.
- Generic output shutdown becomes high impedance.
- A semantic output documents its inactive state before releasing its endpoint.
- Tests cover failed initialization, reset, repeated shutdown, and destruction.
- Instructions require power removal before wiring; cleanup is not permission
  to touch a powered circuit.

### Inputs and controls

- Pull policy, active level, valid voltage range, and floating-input behavior
  are explicit.
- Raw and interpreted state remain distinguishable.
- Stuck, disconnected, bouncing, simultaneous, and contradictory inputs are
  injectable faults.
- An operator confirmation is not an interlock. A button cannot be credited as
  an emergency stop unless independent rated hardware removes actuator power.

### PWM, tone, and timers

- Documentation calls PWM a switched waveform, not analog voltage.
- Pin and timer conflicts fail before hardware changes.
- Duty, frequency, current, and audible-exposure limits are bounded.
- Shutdown behavior is tested from every active mode.

### Analog and bus devices

- Input range, reference voltage, source impedance, and common-ground rules are
  stated.
- External signals cannot back-power an unpowered board.
- Bus ownership, device supply, address or chip-select, and voltage compatibility
  are checked before connection.
- Storage and display failure cannot leave an actuator enabled.

### Servos, motors, and relays

- Only documented low-voltage modules and inert loads are supported.
- Logic and load power paths are shown separately; grounding and isolation
  follow the module manufacturer's primary documentation.
- The lesson specifies driver current, stall current where applicable,
  protection, supply limit, wire rating, and stored-energy handling.
- Motion has a guarded envelope, no pinch or entanglement exposure, a stable
  fixture, a low-energy first test, and an independent means to remove load
  power.
- Direction reversal, conflicting commands, sensor loss, reset, timeout, and
  communication loss reach a documented inactive state.
- A relay exercise switches only an inert, current-limited low-voltage
  indicator. No ADK lesson provides mains wiring guidance.

### Infrared and radio

- Infrared transmission is limited to a documented, owned, harmless lab target.
- Radio curriculum is passive receive-only observation of lawful signals and
  may use prerecorded or synthetic captures.
- Lessons provide no replay, cloning, brute force, jamming, access-control
  bypass, protocol-to-command mapping, antenna amplification, or transmitter
  implementation for observed RF.
- Frequency, licensing, privacy, equipment, and location remain learner
  responsibilities; uncertainty stops the exercise.
- Prefer shielded, conducted, synthetic, or prerecorded evidence when it meets
  the learning objective.

### Cue simulation

- Cue identifiers have no electrical firing meaning or launcher protocol.
- Outputs are screen state, logs, or low-voltage indicators incapable of
  ignition.
- Continuity is synthetic or uses isolated inert fixtures only; no part of the
  project connects to an initiator or live firing circuit.
- Arming, confirmation, faults, and emergency shutdown are state-machine
  subjects, not claims of functional safety.
- ADK does not export a launcher driver, transmitter, waveform, pinout, adapter,
  or real-show operating procedure.

## Project hazard gates

Every project first passes all component gates it composes. It then records the
following project-specific evidence.

| Lesson | Project | Class | Release gate |
|---:|---|:---:|---|
| 003 | Circuit heartbeat | E1 | Pin safe-state measurement; unplug-and-rewire drill |
| 006 | Reaction timer | E1 | No startling high-energy output; stuck and simultaneous input traces |
| 009 | Simon | E1 | Bounded light and sound; timeout, rollover, and shutdown traces |
| 012 | Adaptive night-light | E1 | Sensor open/short cannot request an out-of-range output |
| 015 | Bench instrument panel | E1 | Invalid controls and display loss leave outputs inactive |
| 018 | Environmental logger | E1 | Bus/storage faults preserve sensing and shutdown; no safety alarm claim |
| 021 | Lock simulator | E2 | Inert latch model first; restrained servo; independent load-power removal |
| 024 | Tabletop rover | E2 | Wheels raised for first test; bounded area; physical load-power disconnect; loss-of-control stop test |
| 027 | Telemetry console | E1 | Receive-only or synthetic radio; stale data is explicit; no safety dispatch claim |
| 030 | Show-cue simulator | E0/E1 | Inert outputs only; no launcher/initiator connection; complete deterministic fault and audit trace |

Project 021 does not secure property. Project 024 carries no person, animal, hot
item, sharp tool, or hazardous material. Project 027 does not monitor a
safety-critical condition. Project 030 does not operate a real show.

## Lesson release record

The lesson owner records:

1. energy class, exact board, parts, supplies, ratings, and primary sources;
2. authoritative schematic and an all-power-off inspection;
3. normal, startup, reset, failure, shutdown, and power-removed states;
4. current limit and measured voltage/current where relevant;
5. foreseeable shorts, reversals, disconnections, stalls, conflicts, and stale
   data;
6. explicit stop conditions and the physical power-removal method;
7. simulation or inert-load evidence before E2 hardware;
8. instruments, observations, deviations, reviewer, and date.

Do not mark hardware accepted without this record. Do not infer electrical
safety from a successful compile, unit test, diagram, LED, or destructor.

## Stop and escalation rule

Stop development or publication when identity, rating, polarity, supply,
isolation, safe state, legality, load behavior, or physical acceptance is
unknown. Record the gap as blocked. Resolve it with the manufacturer's primary
documentation and competent human review; do not discover a safety limit by
experiment.
