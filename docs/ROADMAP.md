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
9. Temperature, distance, motion, and environmental sensors
10. Seven-segment, character LCD, and matrix displays
11. `I2cBus`, `SpiBus`, and address/chip-select ownership
12. Keypad, joystick, rotary encoder, and operator panels
13. SD logging, RTC, and repeatable experiment records
14. Infrared and radio receive-only protocol observation
15. Relays and motors using isolated, rated driver modules and inert loads
16. Composite diagnostic console and dynamically selected circuit modes

The capstone is a safety-oriented show-cue simulator: timed cues, redundant
arming state, operator confirmation, logs, continuity simulation with inert
loads, fault injection, and emergency shutdown. ADK will not clone or transmit
a pyrotechnic launch protocol or implement an ignition circuit. Integration
with a real show must remain behind a certified commercial controller and its
documented safety interface.
