# Changelog

## 0.4.0 (unreleased)

A new start, built on the library's original 2021 design.

- **Library.** Every part is an object you declare; `adk::setup ()` checks
  and prepares them all, `adk::update ()` keeps them running, `adk::wait ()`
  waits without stopping them, and `adk::stop ()` makes them all safe. A pin
  used twice, a pin that doesn't exist, or a timer two parts both need stops
  setup and blinks the pin number on the built-in LED.
- **Parts** for the whole Elegoo Mega and 37-in-1 kits: LEDs, RGB LEDs,
  buzzers, speakers with melodies, relays, buttons and switches, analog
  inputs, the thermistor, keypad, rotary encoder and joystick, the 74HC595
  and seven-segment displays, the LCD1602, the MAX7219 LED matrix, servos,
  the 28BYJ-48 stepper, DC motors, the HC-SR04, DHT11 and DS18B20, the IR
  remote, the RC522 RFID reader, the DS1307 clock and the MPU-6050, with
  small register-level I2C and SPI masters in place of Wire and SPI.
- **Radios**, add-ons beyond the kits: `FmRadio` for an Si4703 FM board,
  tuned, stepped and seeking while the sketch runs, with the station's RDS
  name and text; `RadioTransmitter` and `RadioReceiver` for 433 MHz modules,
  speaking RadioHead's RH_ASK; and three LoRa radios over the Mega's spare
  serial ports, `LoraModem` (REYAX RYLR896), `LoraLink` (Ebyte E32) and
  `MeshNode` (a Meshtastic board). Their 3.3 V pins are only ever pulled low
  or reached through a divider, and the E32 is set to a license-free channel
  and power at every start. `IrTransmitter` sends the kit remote's codes
  from the 37-in-1 kit's IR LED, and `SoundSensor` hears how loud a room is.
- **Two boards.** `Bridge` keeps a few named numbers the same on two boards
  over any radio that is a `Link`: a change goes at once, at most ten
  messages a second, everything again every two seconds, and each board
  knows whether the other is there.
- **One rule for commands.** Asking a part for what it is already doing
  changes nothing, so `blink ()`, `beep ()`, `fadeTo ()`, `play ()`,
  `moveTo ()`, `tune ()` and `setVolume ()` may be called from every pass
  of `loop ()`. `measured ()` means one thing everywhere: a reading finished,
  good or not, and `ok ()` says which.
- **C++23**, built with avr-gcc 16 (the Arduino core's GCC 7 stops at
  C++17). `adk::Array`, `Vector`, `Deque` and `Span` give sketches the
  standard containers' shape without the heap; `adk::Timer` and
  `adk::Stopwatch` keep time; `adk::println` prints a line in one call.
- **Course.** Fifty-four lessons in eighteen arcs, from Blink to Pong played
  between two rooms, each a component lesson or a project that combines
  them, with a sketch that compiles for the Mega. Arcs 13 and 14 use add-on
  radios: an FM clock radio, 433 MHz messages, and LoRa. Arcs 15 to 18 join
  two boards over a LoRa bridge, carrying every sensor in the kits across a
  house: a dial that turns a servo, a garden's weather read indoors, a baby
  monitor, a keypad by the door, a doorbell that says who's there. Each
  lesson is NNN-name, the same in docs/lessons and examples/lessons.
- **Website.** A new design where each lesson is both a web page and a PDF,
  with pencil drawings generated from one description of the build and
  checked against the lesson's code. Wires route round parts and labels;
  every part has a home on the breadboard, so each lesson's steps say what
  to keep from the one before, what to take out and what to add. On a
  phone the drawings scroll sideways at a legible size; lines stay near
  seventy characters; printing works in either theme.
- **Measure it.** Every lesson ends with readings to take with a
  multimeter, each drawn with the probes on the exact holes and the reading
  expected, to turn the lesson's idea into something you can see.
- **ADK Boards.** A package for the Arduino IDE's Boards Manager, published
  with the website: the Mega 2560, built as C++23 with avr-gcc 16, for
  Windows, macOS and Linux (x86-64 and ARM).
- **Build.** One Makefile, laid out as tables; everything lands in `build/`.
  `make pins` runs each sketch's `setup ()` on the host and holds it to its
  lesson's circuit, pin for pin and input or output; the circuits are
  checked for pins that reach nothing and shorts. `make 001-blink` builds a
  lesson and `make upload-001-blink` uploads it; `make deps` installs what
  the build needs. The site's Python comes from a hashed lock, and a
  published board package version can never change.
- **Removed** the 0.3 library, its 74 lessons, their PDFs and drawings, and
  the research, audit and agent-process documents. They remain in the git
  history.

## 0.3.0 and earlier

See the git history.
