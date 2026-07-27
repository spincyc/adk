---
title: Safety
description: Electrical, radio, and pyrotechnic safety boundaries for ADK lessons and projects.
---

# Safety

ADK is an educational electronics library, not a substitute for rated
equipment, competent supervision, local rules, or a device manufacturer's
instructions. Stop when the hardware or procedure differs from the lesson.
Never improvise around an interlock, protective device, or uncertain circuit.

## Before applying power

- Disconnect USB, batteries, barrel-jack power, bench supplies, shields, and
  external signals before changing wiring.
- Work on a dry, nonconductive surface. Remove loose metal and jewelry. Inspect
  for damaged insulation, bent pins, conductive debris, heat damage, and
  reversed or misplaced parts.
- Compare every connection with the lesson's exact schematic. Pencil drawings
  are orientation aids, never authoritative wiring diagrams.
- Confirm component identity, polarity, resistor value, voltage, current
  rating, supply polarity, and common-ground requirements.
- Set a current-limited supply before connection when the lesson calls for
  one. Do not exceed the board, pin, component, cable, or supply rating.

## While powered

- Do not move conductors or components in a live circuit unless an explicitly
  rated procedure requires it.
- Keep conductors from shorting adjacent headers. Do not drive two outputs
  against each other or power a board through an unapproved signal path.
- Stop immediately for heat, smoke, odor, sparks, unexpected motion, unstable
  power, damaged insulation, repeated resets, or behavior that disagrees with
  the predicted safe state. Disconnect power before investigating.
- Motors, relays, solenoids, heaters, servos, and other substantial loads
  require correctly rated driver, flyback, isolation, wiring, and external
  power arrangements. An Arduino pin is not a load power source.
- Treat batteries with their chemistry-specific precautions. Prevent shorts,
  charging mistakes, crushing, puncture, and unattended charging.

## Software and lifecycle safety

The following rules define the target RAII component contract. The current
compatibility API does not yet restore pins during object destruction or
provide component-level `shutdown()`. With that API, remove all power before
rewiring and do not treat C++ scope exit as an electrical safety mechanism.

- Outputs must enter a documented inactive state during construction,
  initialization failure, shutdown, reset, and destruction. A generic output
  defaults to high impedance; semantic components use an explicit safe
  shutdown policy.
- Initialization must be transactional: failure releases every resource
  already claimed and leaves hardware safe. Shutdown must be idempotent and
  non-throwing.
- Tests, simulation, and inert dummy loads come before live hardware. Fault
  tests must include startup, disconnect, stuck input, simultaneous input,
  timeout, clock wraparound, brownout or reset, and emergency shutdown where
  relevant.
- Diagnostic and dynamic-reconfiguration features must not bypass electrical
  ratings, ownership checks, safe-state policies, or physical interlocks.

## Radio-frequency work

- ADK radio lessons are limited to lawful, low-risk work with equipment and
  frequencies the learner is permitted to use.
- Receive-only observation does not by itself authorize transmitting,
  decoding private communications, defeating access controls, or cloning a
  remote. Follow local spectrum rules, equipment certification, duty-cycle,
  power, antenna, and licensing requirements.
- Use shielded or conducted laboratory methods and dummy loads where
  appropriate. Prevent unintended radiation. Never transmit on emergency,
  aviation, navigation, public-safety, medical, or other protected services.
- Do not replay an observed signal until ownership, authorization, frequency,
  modulation, power, and the effect on nearby equipment are all established.
  ADK documentation will not provide instructions to clone or transmit an
  unknown control protocol.

## Fireworks and energetic devices

ADK must not initiate pyrotechnics, energize an igniter, reproduce a launcher
remote, or bypass a certified controller's safety system.

The fireworks capstone is limited to a non-pyrotechnic show-cue simulator:
deterministic cue scheduling, operator interface, redundant arming-state
simulation, logging, fault injection, continuity simulation with inert dummy
loads, and emergency shutdown behavior. Testing uses lamps, indicators, or
other harmless loads that cannot ignite material.

Any real show remains the responsibility of qualified operators using
certified commercial firing equipment and the manufacturer's documented,
approved interface. Physical separation, site control, storage, transport,
weather limits, emergency plans, and legal permissions are outside ADK's
software scope and must be handled by the responsible professionals.

## Lesson author requirements

Every hardware lesson must state:

- the exact board and components;
- expected voltage, current, polarity, and power source;
- an all-power-off wiring and inspection procedure;
- a schematic with component values and labelled endpoints;
- the expected safe state before, during, and after execution;
- explicit stop conditions and a power-removal procedure;
- foreseeable wiring, configuration, and software faults;
- a simulation or inert-load step before any higher-energy device;
- disposal, cooldown, or stored-energy precautions when applicable.

If a component's identity, rating, documentation, legality, or safe failure
state is uncertain, the lesson stops there. Replace uncertainty with verified
information rather than an experiment.
