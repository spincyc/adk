# What next

After Lesson 55 you can wire a circuit from a drawing, read and change a
sketch, and make two boards talk across a house. This page shows how what
you've learned carries over to the rest of the Arduino world, and where to
go further.

## The same ideas in plain Arduino

Most Arduino sketches you'll find, in books, in forums and in the IDE's
own examples, use the Arduino core's functions and the libraries in the
IDE's **Library Manager**, not ADK. ADK does the same jobs, so once you
know which is which you can read them all.

| In ADK | In plain Arduino | Taught in |
|---|---|---|
| `adk::Led led {26};` and `led.on ();` | `pinMode (26, OUTPUT);` in `setup ()`, then `digitalWrite (26, HIGH);` | Lesson 1 |
| `adk::wait (500);` | `delay (500);`, but nothing else runs while it waits | Lesson 1 |
| `adk::Button button {22};` and `button.wasPressed ()` | `pinMode (22, INPUT_PULLUP);`, then `digitalRead (22) == LOW` while it is held. You notice the press, and debounce it, yourself, or with the Bounce2 library | Lesson 2 |
| `adk::println (Serial, "Presses: ", presses);` | `Serial.print ("Presses: ");` then `Serial.println (presses);` | Lesson 2 |
| `adk::Every`, `adk::Timer`, `adk::Stopwatch` | `millis ()`, kept in an `unsigned long` and compared: the IDE's *Blink Without Delay* example | Lessons 3 and 12 |
| `adk::Speaker` and `speaker.tone (440, 200);` | `tone (10, 440, 200);` and `noTone (10);`; a melody is a loop of them | Lesson 5 |
| `adk::AnalogInput knob {A0};` and `knob.read ()` | `analogRead (A0)`, and `map ()` to scale it | Lesson 7 |
| A dimmed LED, `adk::PwmOutput` | `analogWrite (3, 128);` | Lesson 7 |
| `adk::ShiftRegister` | `shiftOut ()`, then a pulse on the latch pin | Lesson 10 |
| `adk::update ()` | Nothing: each library has its own call in `loop ()`, if it needs one | Lesson 2 |
| `adk::setup ()`'s checks | Nothing: two parts on one pin just misbehave | Lesson 1 |

The kit's other parts each have a library in the Library Manager. These
are the usual ones; search for the name.

| Part | ADK | Library |
|---|---|---|
| LCD1602 | `adk::Lcd` | LiquidCrystal, which comes with the IDE |
| Servo | `adk::Servo` | Servo, which comes with the IDE |
| Stepper | `adk::Stepper` | Stepper, which comes with the IDE, or AccelStepper |
| Keypad | `adk::Keypad` | Keypad |
| LED matrix | `adk::LedMatrix` | LedControl |
| DHT11 | `adk::Dht11` | DHT sensor library |
| 18B20 | `adk::Ds18b20` | OneWire and DallasTemperature |
| Ultrasonic sensor | `adk::Ultrasonic` | NewPing, or `pulseInLong ()`, which keeps counting while interrupts run |
| Remote control and IR LED | `adk::IrReceiver`, `adk::IrTransmitter` | IRremote |
| RFID reader | `adk::Rfid` | MFRC522 |
| Real-time clock | `adk::Rtc` | RTClib |
| Accelerometer | `adk::Mpu6050` | Adafruit MPU6050 |
| Rotary encoder | `adk::RotaryEncoder` | Encoder |
| I2C and SPI | ADK's own buses | Wire and SPI, which come with the IDE |
| LoRa modem | `adk::LoraModem` | None needed: `Serial1.print ()` sends the same text commands that Lesson 40 shows |

A few things change when you leave ADK behind:

- **Libraries each work their own way.** ADK's parts share one way of
  keeping time and use no heap. Libraries each bring their own, cost
  flash and RAM of their own, and two can need the same timer inside the
  Mega without telling you; ADK's `adk::setup ()` stops and says so.
- **Some C++ is newer than the IDE's own compiler.** ADK Boards brings a
  newer compiler for C++23. On the Arduino AVR boards' own compiler, most
  of [the C++ you've met](cpp.md) works, but not all: `auto` in a
  function's parameters, for one.
- **Nobody checks the pins.** Two parts on one pin compile and upload
  without a word; look for the mistake in your wiring and your sketch.

## Other boards

ADK is written for the Mega, but the Arduino IDE programs many boards in
plain Arduino code:

- **The Arduino Uno and Nano** are smaller and cheaper, with fewer pins.
  Their pins work at 5 V, as the Mega's do.
- **ESP32 boards**, such as the Heltec board from Lesson 42, add WiFi and
  Bluetooth. **The Raspberry Pi Pico** is fast and cheap. Both work at
  3.3 V: a 5 V signal can damage their pins, so the dividers and level
  shifter from Lessons 28, 38 and 40 come back.

## Further on

- **The electricity path.** If you haven't yet, [E01 to E24](electricity/index.md)
  explain why the circuits you've built behave as they do, with a meter
  and, later, an oscilloscope.
- **Arduino's own documentation.** The
  [language reference](https://docs.arduino.cc/language-reference/) lists
  every function in the Arduino core, and the
  [built-in examples](https://docs.arduino.cc/built-in-examples/) are the
  sketches under **File → Examples** in the IDE.
- **C++.** [learncpp.com](https://www.learncpp.com/) teaches the language
  from the start, for free.
- **Make a build last.** Solder a circuit onto stripboard, or design a
  board of your own with the free [KiCad](https://www.kicad.org/). Read
  [Soldering](safety.md#soldering) first.
- **Radio.** [Meshtastic](https://meshtastic.org/) has more on meshes. An
  amateur radio license lets you send on many more bands; in the USA the
  [ARRL](https://www.arrl.org/), and in Canada
  [Radio Amateurs of Canada](https://www.rac.ca/), say how to get one.
- **Help the course.** If you build a lesson,
  [report your build](builds.md): every report makes the course more
  trustworthy. [Contributing](contributing.md) shows how the course and
  the library are made.
