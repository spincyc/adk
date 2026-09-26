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
  and power at every start.
- **C++23**, built with avr-gcc 16 (the Arduino core's GCC 7 stops at
  C++17). `adk::Array`, `Vector`, `Deque` and `Span` give sketches the
  standard containers' shape without the heap; `adk::Timer` and
  `adk::Stopwatch` keep time; `adk::println` prints a line in one call.
- **Course.** Forty-two lessons in fourteen arcs, from Blink to texting
  the Mega from a phone, each a component lesson or a project that combines
  them, with a sketch that compiles for the Mega. The last two arcs use
  add-on radios: an FM clock radio, 433 MHz messages, and LoRa.
- **Website.** A new design where each lesson is both a web page and a PDF,
  with pencil drawings generated from one description of the build and
  checked against the lesson's code. Wires route round parts and labels;
  every part has a home on the breadboard, so each lesson's steps say what
  to keep from the one before, what to take out and what to add.
- **Measure it.** Every lesson ends with readings to take with a
  multimeter, each drawn with the probes on the exact holes and the reading
  expected, to turn the lesson's idea into something you can see.
- **ADK Boards.** A package for the Arduino IDE's Boards Manager, published
  with the website: the Mega 2560, built as C++23 with avr-gcc 16, for
  Windows, macOS and Linux (x86-64 and ARM).
- **Build.** One Makefile; everything lands in `build/`. `make pins` runs
  each sketch's `setup ()` on the host and checks the pins it claims are
  exactly the pins its lesson wires.
- **Removed** the 0.3 library, its 74 lessons, their PDFs and drawings, and
  the research, audit and agent-process documents. They remain in the git
  history.

## 0.3.0 and earlier

See the git history.
