# Changelog

## Unreleased

- **Library: two renames.** `FmRadio::setVolume (n)` is now `volume (n)`
  and `Keypad::key ()` is now `pressedKey ()`, following one rule written
  into the style guide: events read in the past tense or as `was…`,
  settings by their bare noun. Sketches written for 0.4.0 need the new
  names.
- **Library: stopping and timing.** After `adk::stop ()`, an `Every` holds
  its beat until `restart ()`, and a `Bridge` sends nothing until the new
  `start ()`. `StartTime` gains `start ()` and `passed ()`, its `elapsed ()`
  is a plain query, and every part keeps its start times in one; so
  `Stopwatch::reset ()` and a mesh node's 1.5 s gap now count from the
  update after the call. The ultrasonic ranger times its echo with
  `pulseInLong ()`, so other parts' interrupts no longer shorten it. A
  `Span` views only its own type, or adds `const`. `adk::dec (n, 2)` prints
  a number with at least two digits.
- **Tests.** `make smoke` runs every sketch's `setup ()` and 400 passes of
  `loop ()` on the host under the sanitizers; `make avr-test` runs a few
  tests on a simulated Mega, where `int` is 16 bits; the fake core models a
  pin's output latch, so a 3.3 V line pulled up to 5 V fails its test. CI
  runs both.
- **Lessons.** Every piece of C++ is explained where it first appears, and
  *The C++ you've met* lists them. Lessons 2 and 3 carry less at once, with
  a beep test before Lesson 3's game. Lesson 40's European setting
  compiles; two-board lessons say how to tell two Megas' ports apart and
  remind Europe of its airtime; Lesson 42 is optional. Longer sketches say
  on their page why they need the length.
- **Laws and formulas.** Nine new pages state the laws the lessons rely
  on: units and decibels, Ohm's law, Kirchhoff's laws with series and
  parallel, dividers, power, capacitors and coils, diodes, transistors and
  op-amps, waves and sampling, and logic levels and serial. Each works
  through the course's own circuits and ends with questions. Each lesson
  lists the laws it relies on in its front matter; the section that relies
  on one ends with a link to it, and the law's page lists every lesson
  that does. The glossary links to them and gains charge, forward voltage,
  Kirchhoff's laws and power.
- **Electricity.** Every investigation has the same sections and a change
  to try; E05, E10, E18, E21 and E24 now show their idea rather than state
  it; E07, E09, E10, E11 and E13 have schematics.
- **Drawings.** Parts are drawn back to front, so a knob no longer hides
  an LED; labels stay near their own parts with leaders kept off other
  parts; wires keep off other wires' plugs and rail strips, and a wire kept
  from the lesson before keeps its way as well as its color. The LED's
  long leg is bent, film capacitors look like film capacitors, the PIR's
  pin names read whole, meter probes lean in from their own sides, and a
  lesson's opening shows what moves as well as its screen. Build along
  and printed close-ups no longer cut names in half.
- **Site.** Every lesson says it hasn't been built on a real bench yet and
  asks for build reports, through a new issue form. A teacher guide, a
  glossary, a page on what comes next and the changelog join the site, and
  its footer shows the version. Printed lessons put their answers on a
  last page of their own. Every link and anchor is checked when the site
  builds.

## 0.4.0 (2026-09-30)

A new start, built on the library's original 2021 design.

- **Library.** Every part is an object you declare; `adk::setup ()` checks
  and prepares them all, `adk::update ()` keeps them running, `adk::wait ()`
  waits without stopping them, and `adk::stop ()` makes them all safe. A pin
  used twice, a pin that doesn't exist, or a timer two parts both need stops
  setup and blinks the pin number on the built-in LED, and
  `adk::setup (Serial)` names the part that took a busy timer.
  Angles, volumes, brightness and screen places take an `int` and keep a
  value past either end at that end, so `servo.write (90 - 100)` is 0°,
  never 180°.
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
  and power at every start. The 433 MHz transmitter rests after every
  message, 30 times as long as it took and never under 10 s, as the FCC
  and ISED ask of a gadget that sends data (`isReady ()`, `restLeft ()`). `IrTransmitter` sends the kit remote's codes
  from the 37-in-1 kit's IR LED, and `SoundSensor` hears how loud a room is.
- **Two boards.** `Bridge` keeps a few named numbers the same on two boards
  over any radio that is a `Link`: a change goes at once, at most ten
  messages a second, everything again every two seconds, and each board
  knows whether the other is there. Each message carries the two boards'
  start numbers, so a board that restarts is noticed and brought up to
  date by itself, a stale reply is ignored, and an event from before the
  boards met never arrives.
- **One rule for commands.** Asking a part for what it is already doing
  changes nothing, so `blink ()`, `beep ()`, `fadeTo ()`, `play ()`,
  `moveTo ()`, `tune ()` and `setVolume ()` may be called from every pass
  of `loop ()`. `measured ()` means one thing everywhere: a reading finished,
  good or not, and `ok ()` says which.
- **C++23**, built with avr-gcc 16 (the Arduino core's GCC 7 stops at
  C++17). `adk::Array`, `Vector`, `Deque` and `Span` give sketches the
  standard containers' shape without the heap; `adk::Timer` and
  `adk::Stopwatch` keep time; `adk::println` prints a line in one call. A
  blinking LED keeps its beat as `adk::Every` does, and `blink (0)` turns
  it off. The clock is read ten times a second in `update ()`, so asking
  it the time costs nothing; the LoRa modem's commands and the FM bands
  live in flash.
- **Course.** Fifty-five lessons: eighteen arcs of three, from Blink to Pong
  played between two rooms, each a component lesson or a project that combines
  them, with a sketch that compiles for the Mega. Arcs 13 and 14 use add-on
  radios: an FM clock radio, 433 MHz messages, and LoRa. Arcs 15 to 18 join
  two boards over a LoRa bridge, carrying every sensor in the kits across a
  house: a dial that turns a servo, a garden's weather read indoors, a room
  monitor, a keypad by the door, a doorbell that says who's there. The board
  with the screen is always Board A. An extra, the Reliability Meter, sends
  64 numbered messages between the two boards and shows how many came back,
  at Quick and at Far. Each piece of C++ is explained where it first
  appears, and *The C++ you've met* lists them all; each lesson ends with a
  short *Check yourself* with its answers folded away. Each lesson is
  NNN-name, the same in docs/lessons and examples/lessons.
- **Electricity.** Twenty-four investigations, E01 to E24 (lessons 056 to
  079), in eight modules from a closed loop to a byte sent down a wire, each
  a prediction, one change and a result to see or measure. A kit-and-meter
  route needs only the kit and a multimeter; a scope-and-generator
  extension has a primer on its instruments and an output check before
  anything is wired. Schematics sit beside the breadboard drawings where a
  circuit is hard to read from the board, and a guided route weaves the
  investigations into the project course.
- **Website.** A new design where each lesson is both a web page and a PDF,
  with pencil drawings generated from one description of the build and
  checked against the lesson's code. Wires route round parts and labels;
  every part has a home on the breadboard, so each lesson's steps say what
  to keep from the one before, what to take out and what to add. The steps
  come in stages, a part and its wires under a heading naming the pins
  they use, with each wire, resistor and LED drawn between the holes it
  joins, lined up from step to step; a stage built before folds away. A
  learner can tick steps off, or open **Build along** for a large close-up
  of the current step beside its instruction, with the whole board below.
  Rings mark the exact holes and pins; a long wire shows its two ends in
  separate close-ups. Wire close-ups stay centered on those connections,
  even when the wire takes a long detour around another part. Part-placement
  close-ups use drawing coordinates directly, keeping the potentiometer,
  LCD and other parts in frame in Firefox as well as Chromium. Progress is
  saved separately for each board and starts fresh when its instructions
  change. The full steps remain on the
  page and in print. Pages name a rail's hole in
  words, "the bottom − rail by column 3", never as B-3, whose B reads as
  row b. The
  drawings follow the kit: the Mega's headers and the breadboard at their
  real sizes, modules with their pins in their real order, every wire a
  color the kit has, the potentiometers across the middle gap, the rotary
  encoder standing in its home in columns 15–19, and the power
  module beside the board, since its pins fit the kit's breadboard the
  right way round only at the Mega's end. Each parts list counts the
  wires its build needs. On a phone the drawings scroll sideways at a
  legible size; lines stay near seventy characters; printing works in
  either theme. A rebuild draws only what changed, every lesson at once.
- **Screen at the far right.** The LCD's sixteen pins stand in a47–a62,
  with its contrast knob in columns 43–45 and its body overhanging the end.
  Shared homes, wires, probe points and lesson instructions follow that
  layout throughout the course. Adding or removing the screen leaves the
  other controls at their homes; the kit page records the few exceptions
  needed when other parts occupy the same holes.
- **Measure it.** Nearly every project lesson ends with readings to take
  with a multimeter, each drawn with the probes on the exact holes and the reading
  expected, to turn the lesson's idea into something you can see.
- **Drawings you can read.** Every part is named on the bench, a crowded
  name on a leader of its own; leaders keep clear of other labels. Wires
  are colored by what they carry, black for GND, red for 5 V and orange for
  3.3 V, and nothing else; wires that sit side by side or cross take
  different colors, and a wire kept from the lesson before keeps its color.
  When a lesson swaps a chip in the same holes, its steps name every wire to
  keep and every one to take out. Oscilloscope probes and the signal
  generator are drawn where a lesson uses them. The RGB LED's legs, the
  buzzer's + and the S8050's flat face are marked; the level shifter faces
  so its signals go straight through.
- **Honest about hardware.** The home, start and course pages say plainly
  that the circuits are predictions not yet built on a real bench, and
  *Builds on real hardware* says how to report one. The kit page prices
  the kit and the add-ons, says which radios need a license, and lists what
  to buy for each path; Safety covers soldering and the 9 V battery; Getting
  started helps with Linux ports, CH340 boards and updating ADK.
- **ADK Boards.** A package for the Arduino IDE's Boards Manager, published
  with the website: the Mega 2560, built as C++23 with avr-gcc 16, for
  Windows, macOS and Linux (x86-64 and ARM).
- **Build.** One Makefile, laid out as tables; everything lands in `build/`.
  `make pins` runs each sketch's `setup ()` on the host and holds it to its
  lesson's circuit, pin for pin and input or output; the circuits are
  checked for pins that reach nothing and shorts. `make 001-blink` builds a
  lesson and `make upload-001-blink` uploads it; `make deps` installs what
  the build needs. The site's Python comes from a hashed lock, and a
  published board package version can never change. Lesson PDFs print
  their drawings as lines, a few megabytes each instead of up to twenty;
  each is checked for a whole file, its pages, title and font, and held to
  a size budget. CI also installs
  ADK Boards on Windows and macOS and compiles Blink with it.
- **Removed** the 0.3 library, its 74 lessons, their PDFs and drawings, and
  the research, audit and agent-process documents. They remain in the git
  history.

## 0.3.0 and earlier

See the git history.
