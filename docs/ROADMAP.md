# Component and curriculum roadmap

Each item receives a host-tested interface, Mega sketch, pencil orientation
plate, exact schematic, and rich PDF lesson.

Lessons 001--003 are imported compatibility material built on the current
`led::Mono` and `led::Rgb` API. The sequence below is the hierarchy for new
interface development and commits. Those lessons will migrate after their
replacement endpoint and component interfaces are established.

1. Lifecycle, resource claims, and board context
2. `DigitalInput` and `Button`
3. `DigitalOutput` and `MonoLed`
4. Nonblocking clock, debounce, and blink behavior
5. `PwmOutput` and `RgbLed`
6. `AnalogInput`, potentiometer, and photoresistor
7. Piezo sounder and tone/timer ownership
8. Servo and external-power boundaries
9. Simon simulator midpoint project
10. Temperature, distance, motion, and environmental sensors
11. Seven-segment, character LCD, and matrix displays
12. `I2cBus`, `SpiBus`, and address/chip-select ownership
13. Keypad, joystick, rotary encoder, and operator panels
14. SD logging, RTC, and repeatable experiment records
15. Infrared and radio receive-only protocol observation
16. Relays and motors using isolated, rated driver modules and inert loads
17. Composite diagnostic console and dynamically selected circuit modes

## Midpoint: deterministic Simon

The Simon project composes buttons, LEDs, nonblocking time, sound, and a finite
state machine without introducing hidden global state.

- The game engine receives a clock and sequence source through explicit
  interfaces.
- Production hardware may use a seeded pseudorandom sequence; tests use fixed
  sequences.
- A seed and complete input trace reproduce every game exactly.
- `update(TimePoint now)` is the only way game time advances.
- Button events are non-consuming snapshots for one update cycle.
- Outputs are derived from observable state rather than issued as hidden
  callbacks.
- Host tests cover legal transitions, exact cue timing, input windows, success,
  mismatch, timeout, restart, sequence growth, maximum length, and timestamp
  wraparound.
- Table-driven traces double as executable examples of correct API use.
- Mega hardware tests verify the same traces with human-scale timing.

The lesson PDF will include the state graph, timing diagrams, trace tables,
fault injection, deterministic replay, a test-writing exercise, and a final
claim-evidence-reasoning acceptance record.

The capstone is a safety-oriented show-cue simulator: timed cues, redundant
arming state, operator confirmation, logs, continuity simulation with inert
loads, fault injection, and emergency shutdown. ADK will not clone or transmit
a pyrotechnic launch protocol or implement an ignition circuit. Integration
with a real show must remain behind a certified commercial controller and its
documented safety interface.
