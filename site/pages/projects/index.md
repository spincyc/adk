# Projects

Projects combine already-proven components. They do not weaken endpoint
ownership, timing, shutdown, or safety contracts to make a demonstration
shorter.

## Simon simulator

The midpoint project is a deterministic Simon-style memory game with four
active-low pull-up buttons and four matching LEDs. The engine uses neutral cues
rather than colors, so wiring, physical position, accessible presentation, and
future sound output remain adapter concerns.

Its stable design includes:

- explicit `update(TimePoint now)` time advancement;
- immutable validated timing and capacity configuration;
- non-consuming button-event snapshots for each update cycle;
- complete press-release cycles and explicit invalid simultaneous input;
- a documented, versioned pseudorandom generator;
- identical sequences for the same algorithm version and seed;
- fixed-capacity storage with no heap allocation; and
- replayable timestamped traces and table-driven correctness tests.

The hardware lesson will add LEDs and buttons only after their endpoint and
component interfaces are complete. Tone output is a later composition because
it introduces timer ownership.

## Diagnostic console

The later diagnostic console combines operator inputs, displays, logging, and
dynamically selected circuit modes. Buttons come early in the hierarchy because
they support observable debugging and deliberate reconfiguration without
reflashing.

## Safety-oriented show-cue simulator

The capstone explores deterministic scheduling, redundant arming state,
operator confirmation, logging, continuity simulation with inert loads, fault
injection, and emergency shutdown.

This project is a simulator. ADK will not clone or transmit a pyrotechnic
launcher protocol, build an ignition circuit, or bypass the interlocks of a
certified commercial controller. Any real show integration must use that
controller's documented, approved safety interface and qualified operators.

Project implementation follows the hierarchy and completion gates in the
[roadmap](../docs/ROADMAP.md) and
[contributing guide](../contributing.md).
