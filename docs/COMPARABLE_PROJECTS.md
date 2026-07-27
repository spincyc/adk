# Comparable projects

ADK occupies a useful gap between electronics-first Arduino sketches and large
hardware abstraction frameworks.

| Project | What ADK should learn | What ADK should keep explicit |
|---|---|---|
| Arduino Starter Kit and built-in examples | Familiar circuit progression and broad component coverage | A coherent object vocabulary and ownership model |
| micro:bit DAL/CODAL | Small C++ components, injected dependencies, board facade, pin capabilities | Scoped lifetime instead of assuming global objects |
| Adafruit Circuit Playground | Approachable semantic facade and polished project guides | Separate interfaces, board adapters, and resource ownership |
| MakeCode curricula | Predict–build–observe–extend progression and staged courses | Native C++ mechanics and physical evidence |
| Johnny-Five | Broad semantic component catalog and platform adapters | Embedded size, deterministic storage, and cleanup |
| Arm Mbed | Constructor-bound endpoints, composition, narrow driver interfaces, RAII guards | Arduino accessibility and measurement-rich lessons |

Primary references:

- [Arduino Starter Kit R4](https://docs.arduino.cc/hardware/starter-kit-r4/)
- [Arduino built-in examples](https://docs.arduino.cc/built-in-examples/)
- [Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/)
- [micro:bit component model](https://lancaster-university.github.io/microbit-docs/concepts/)
- [micro:bit advanced runtime composition](https://lancaster-university.github.io/microbit-docs/advanced/)
- [micro:bit input/output curriculum](https://microbit.org/teach/topics/inputs-and-outputs/)
- [Adafruit Circuit Playground Express](https://learn.adafruit.com/adafruit-circuit-playground-express?view=all)
- [MakeCode micro:bit courses](https://makecode.microbit.org/courses)
- [Johnny-Five component API](https://johnny-five.io/api/board/)
- [Mbed `DigitalOut`](https://os.mbed.com/docs/mbed-os/v6.16/apis/digitalout.html)
- [Mbed library composition example](https://os.mbed.com/cookbook/Writing-a-Library)

The synthesis is deliberate: small concrete C++ components, composition-first
design, an optional board facade, testable resource ownership, and a complete
evidence-centered lesson for every interface.
