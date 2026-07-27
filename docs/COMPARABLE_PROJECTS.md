# Comparable projects

ADK sits between sketch-first electronics courses and general hardware
frameworks. These projects offer useful patterns, not APIs to copy.

## Curricula and project sequences

| Project | Useful evidence | Pattern for ADK |
|---|---|---|
| [Arduino Starter Kit](https://docs.arduino.cc/hardware/starter-kit-r4/) | Fourteen projects advance from LEDs to sensors and motors. | Introduce one physical idea at a time, then require it in a project. Keep Mega 2560 wiring explicit. |
| [Arduino built-in examples](https://github.com/arduino/arduino-examples) | Tiny, independently compilable examples are discoverable in the IDE. | Ship every supported component under `examples/`, with one obvious behavior and no hidden prerequisites. |
| [SparkFun Inventor's Kit](https://learn.sparkfun.com/tutorials/sparkfun-inventors-kit-experiment-guide---v40) and [source](https://github.com/sparkfun/SIK-Guide-Code) | Sixteen circuits form five themed projects: light, sound, motion, display, and robot. Each project names new parts and concepts. | Make every third lesson an integration project. Start each project with its dependency map, new concepts, and a measurable finish line. |
| [Adafruit Circuit Playground Express](https://learn.adafruit.com/adafruit-circuit-playground-express?view=all) | A friendly board facade supports projects that combine buttons, light, sound, motion, and sensing. | Offer semantic components and later an optional board facade; keep ownership and pin claims visible underneath. |
| [MakeCode micro:bit courses](https://makecode.microbit.org/courses) and [I/O curriculum](https://microbit.org/teach/topics/inputs-and-outputs/) | Short courses progress through prediction, construction, observation, and extension. | Give each lesson a pre-build prediction, evidence table, fault exercise, and open-ended extension. |
| [Arduino Plug and Make Kit](https://docs.arduino.cc/hardware/plug-and-make-kit/) | Seven composable modules reduce wiring friction so projects can emphasize programming concepts. | Test composition independently of wiring, but retain a wiring audit and hardware acceptance step. |

## Component and runtime designs

| Project | Useful evidence | Pattern for ADK |
|---|---|---|
| [micro:bit component model](https://lancaster-university.github.io/microbit-docs/concepts/) and [runtime composition](https://lancaster-university.github.io/microbit-docs/advanced/) | Small C++ components use injected dependencies, capabilities, and a board facade. | Prefer constructor-injected composition; avoid mandatory globals and implicit lifetime. |
| [Johnny-Five component API](https://johnny-five.io/api/board/) | A broad semantic catalog separates user concepts from platform adapters. | Keep learner-facing names concrete while adapters isolate Arduino calls. |
| [Mbed `DigitalOut`](https://os.mbed.com/docs/mbed-os/v6.16/apis/digitalout.html) and [library guidance](https://os.mbed.com/cookbook/Writing-a-Library) | Endpoints are constructor-bound and drivers compose through narrow interfaces. | Bind resources at construction, report initialization failure explicitly, and guarantee safe, idempotent teardown. |
| [Wokwi AVR8js](https://github.com/wokwi/avr8js) and [part tests](https://github.com/wokwi/wokwi-part-tests) | AVR firmware and peripherals can run in repeatable simulation and CI. | Keep host fakes authoritative first; add firmware simulation as a second integration layer, never as a substitute for Mega hardware checks. |

## Adopted curriculum pattern

Digital output comes first because visible state makes wiring, timing, and
lifecycle faults easy to diagnose. Input and button handling follow.

| Lessons | Integration project | Components exercised |
|---|---|---|
| 001–003 | Reaction timer | `DigitalOutput`, `DigitalInput`, `Button`, clock |
| 004–006 | Simon | PWM LED, buttons, tone, deterministic sequence |
| 007–009 | Adaptive night light | analog input, calibrated sensors, RGB output |
| 010–012 | Traffic junction | shift register, display, timed state machine |
| 013–015 | Environmental station | sensors, display, stable records |
| 016–018 | Inert access trainer | keypad, servo, persistent settings |
| 019–021 | Bench rover | distance sensing, motor driver, emergency stop |
| 022–024 | Greenhouse controller | RTC, logging, simulated loads |
| 025–027 | Telemetry console | receive-only observations, display, logs |
| 028–030 | Inert cue simulator | controls, deterministic scheduler, fault audit |

Each component earns a narrow interface, deterministic fake, correctness tests,
Mega sketch, HTML reference, lesson PDF, wiring diagram, size result, and
hardware checklist. Each integration project adds replay fixtures, fault
injection, and a component dependency map.

## Deliberate differences

ADK keeps resource ownership, cleanup, timing, and error handling explicit.
Internals do not throw, but RAII must remain correct when callers do. Lessons
use the full Mega 2560 rather than assuming an Uno or R4. HTML is the concise,
linkable reference; PDFs carry printable lab work, pencil-style diagrams, and
evidence sheets. Radio work stops at lawful, receive-only observation, and the
cue controller remains inert.
