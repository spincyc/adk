# What's in the kit

The course is built around the **Elegoo Mega 2560 Most Complete Starter
Kit**. A few lessons also use modules from the **Elegoo 37 in 1 Sensor
Modules Kit**; those are marked below and in each lesson's parts list. The
drawings show that kit's black boards, as its first two versions have them;
the blue boards of its third version carry the same parts, so check each
one’s printed pin names. Lessons 28, 30 and 50 also need the I2C level
shifter below. [What to buy](#what-to-buy) adds up each path's parts, with
rough costs.

## The parts

| Part | What it does | First used |
|---|---|---|
| Arduino Mega 2560 and USB cable | The computer that runs your sketches | [Lesson 1](lessons/001-blink/index.md) |
| 830-hole breadboard and jumper wires | Joins parts without soldering | Lesson 1 |
| LEDs, and 220 Ω, 1 kΩ, 2 kΩ and 10 kΩ resistors | Light, and the current that makes it | Lesson 1 |
| Push buttons | Input you can press | Lesson 2 |
| Active buzzer | Beeps when switched on | Lesson 3 |
| S8050 transistor and 1N4007 diode | Switch and protect the active buzzer | Lesson 3 |
| RGB LED | Any color, mixed from red, green and blue | Lesson 4 |
| Passive buzzer | Plays any note you ask for | Lesson 5 |
| 10 kΩ potentiometer | A knob that sets a voltage | Lesson 7 |
| Photoresistor | Senses light | Lesson 8 |
| 74HC595 shift register | Eight outputs from three pins | Lesson 10 |
| One- and four-digit 7-segment displays | Numbers in light | [Lesson 10](lessons/010-dice/index.md), [Lesson 11](lessons/011-four-digits/index.md) |
| LCD1602 character display | Two lines of sixteen letters | Lesson 13 |
| DHT11 sensor and thermistor | Temperature and humidity | Lesson 14 |
| 18B20 temperature module *(37 in 1)* | A precise digital thermometer | Lesson 14 |
| 4×4 membrane keypad | Sixteen keys on eight wires | Lesson 16 |
| SG90 servo | A motor that turns to an angle | Lesson 17 |
| Breadboard power module and 9 V adapter | Power for motors and servos | Lesson 17 |
| HC-SR04 ultrasonic sensor | Measures distance with sound | Lesson 19 |
| DC motor, fan blade and L293D | Spinning things, both ways | Lesson 20 |
| IR receiver and remote | Commands from across the room | Lesson 22 |
| PIR, tilt, obstacle and beam-break sensors *(some 37 in 1)* | Things that notice | Lesson 23 |
| MAX7219 8×8 LED matrix | Pictures, scrolling text and games | Lesson 25 |
| Joystick | Two-axis control | Lesson 26 |
| GY-521 accelerometer (MPU-6050) | Knows which way is down | Lesson 28 |
| Rotary encoder | A knob that turns forever | Lesson 29 |
| 28BYJ-48 stepper motor and ULN2003 driver | Exact, countable steps | Lesson 31 |
| DS1307 real-time clock | Keeps time when unplugged | Lesson 32 |
| RC522 RFID reader, card and fob | Knows which card is which | Lesson 34 |
| Tap sensor and relay *(37 in 1)* | Knocks, and switching a separate circuit | Lesson 35 |
| 9 V battery and its snap lead | Powers the circuit the relay switches | Lesson 35 |
| Sound sensor module and water level sensor | How loud it is, and water on the floor | Lesson 48 |
| IR LED module (KY-005) *(37 in 1)* | Sends a remote's codes | Lesson 53 |

## Other equipment

Some lessons need things that don't go on the breadboard. Costs are rough
US dollar prices, checked in 2026.

| Equipment | Used for | Rough cost |
|---|---|---|
| A digital multimeter that reads DC volts | The *Measure it* section that ends most project lessons, which you can skip without one; the electricity path from E02 on | US$15–35 |
| A small screwdriver | The relay's screw terminals, in Lessons 35 and 53 | US$2–5 |
| A soldering iron, its stand and lead-free solder | Header pins on add-on boards that arrive without them fitted: the level shifter (Lesson 28), the FM radio board (Lesson 37), some 433 MHz modules (Lesson 38) and a Heltec board (Lesson 42). Buy them with pins fitted if you can, or ask someone experienced, and read [Soldering](safety.md#soldering) first | US$20–40 |
| The electricity path's extra parts and instruments | Capacitors, an inductor, logic and amplifier chips, and for E11, E13–E16 and E18 a battery-powered scope and signal generator: the syllabus's [equipment gates](electricity/index.md#equipment-gates) list them module by module | See [What to buy](#what-to-buy) |

## I2C level shifter

Lessons 28, 30 and 50 need one **BSS138 bidirectional I2C level shifter**,
an extra part beyond the two kits. The drawings use the
[Adafruit 757 board](https://www.adafruit.com/product/757): LV and A1–A4 on
one side, HV and B1–B4 on the other, with 10 kΩ pull-ups fitted. Use one
with its headers already soldered, or have someone experienced fit them
before the lesson. The GY-521 regulator accepts 5 V power, but its I2C
signals need 3.3 V; the shifter keeps the Mega’s 5 V pull-ups separate.
A plain unidirectional buffer or resistor divider cannot replace this
bidirectional I2C interface.

Check the transistor and diode in your kit too. Lesson 3 requires an S8050
with the documented E–B–C pin order and a diode marked 1N4007. Kit versions
and transistor packages vary; the similarly shaped PN2222 is not a drop-in
replacement. If those parts are missing, obtain the specified parts before
building the active-buzzer lessons.

## Add-on radios

Lessons 37 to 55 use radios that aren't in either kit. The LoRa radios
come in pairs, because it takes two to talk. Costs are rough US dollar
prices for the pair or set, checked in 2026; they vary by shop and
country. Sending is ruled by law, and the last column says what you may
need: [Safety](safety.md#radios) has the details.

| Part | What it does | First used | Rough cost | To send |
|---|---|---|---|---|
| Si4703 FM radio board (CJMCU-470) and wired earbuds | FM stations, their names and songs | Lesson 37 | US$5–15 | Receives only |
| 433 MHz transmitter (WL102-341) and receiver (RX470C) | Short messages, one every 10 seconds at most | Lesson 38 | US$3–8 | No license, within the limits; in the USA and Canada, aerials off |
| Two REYAX RYLR896 LoRa modems | Messages across a kilometer or more, on 915 MHz | Lesson 40 | US$32–50 | No license, within the limits |
| Two Ebyte E32-433T20D LoRa modules, with aerials *(optional)* | A 433 MHz link that passes on lines of text | Lesson 41 | US$12–25 | **An amateur radio license** in the USA and Canada, so there Lesson 41 is optional: read it, and carry on to Lesson 42 from Lesson 40's build |
| Two Heltec WiFi LoRa 32 V3 boards, the 863–928 MHz version, running Meshtastic, and a phone | Text messages from a phone, across a mesh | Lesson 42 | US$52–60, besides the phone | No license, within the limits |

Their pins work at 3.3 V, so the Mega's signals reach them through a
resistor divider, 1 kΩ and 2 kΩ; [Safety](safety.md#radios) explains why, and
which bands you may send on where you live. If your resistor card has no
2 kΩ, obtain the specified 2 kΩ resistors so their holes match the drawings. Heltec's shop lists the
863–928 MHz boards by band: 902–928 MHz for the Americas and Australia,
863–870 MHz for Europe. Its 433 MHz and 470–510 MHz boards won't do.

## Two boards

Lessons 43 to 55 join two boards over radio: a dial on one turns a servo on
the other. Each board is a whole setup of its own, so besides the kit you
need a second of each thing the other board uses:

| Part | Why |
|---|---|
| A second Arduino Mega 2560, USB cable and breadboard | Board B |
| Two REYAX RYLR896 LoRa modems, the pair from Lesson 40 | One on each board, the bridge between them |
| A second breadboard power module, when both boards need one, as in Lesson 51: one board's modem, the other's servo | Each board's own supply |
| Two USB power sources, or a long cable, if the boards are to be far apart | Each board runs on its own |

Each board's page says what it needs. A second Elegoo kit covers all of
Board B's parts.

## What to buy

Each path's shopping list, with rough US dollar prices checked in 2026.
Prices vary by shop and country, and postage is extra. ADK sells nothing
and earns nothing from any shop.

### Project path, Lessons 1 to 36

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit: the Mega, USB cable, breadboard, parts, power module, 9 V adapter and 9 V battery | US$55–70 |
| 1 | Elegoo 37 in 1 Sensor Modules Kit, for the modules marked *(37 in 1)* in [the parts](#the-parts) | US$30–40 |
| 1 | BSS138 I2C level shifter with its headers fitted, such as Adafruit 757, for Lessons 28, 30 and 50 | US$4–8 |
| 1 | Small screwdriver | US$2–5 |
| 1 | Digital multimeter, for *Measure it* (optional) | US$15–35 |
| | **About** | **US$90–160** |

### Radio arcs, Lessons 37 to 42

| Qty | Item | Rough cost |
|---|---|---|
| 1 set | Each of the [add-on radios](#add-on-radios), with aerials | US$105–160 |
| | Without Lesson 41's E32 modules, where they need a license | US$90–135 |

### Two boards, Lessons 43 to 55

Besides the pair of RYLR896 modems from Lesson 40:

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A second Elegoo Mega 2560 Most Complete Starter Kit, which covers all of Board B's parts | US$55–70 |
| | *or* a second Mega 2560 (a compatible board costs less than an Arduino one), USB cable and breadboard, a second breadboard power module for Lesson 51, and a second MAX7219 matrix, joystick and passive buzzer for Lessons 54 and 55 | US$40–75 |
| 1–2 | USB power banks, or a long USB cable, to put the boards in different rooms | US$10–25 |

### Electricity path, E01 to E24

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit, the same as the project path's | US$55–70 |
| 1 | Digital multimeter with DC volts, from E02 on | US$15–35 |
| 1 set | The extra parts in each module's [equipment gate](electricity/index.md#equipment-gates): capacitors, a 100 mH inductor, an MCP6002, a 74HC00 and a 74HC14, a 10 Ω and two 100 kΩ resistors | US$10–20 |
| | **About** | **US$80–125** |

### Scope extension, for E11, E13 to E16 and E18

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A battery-powered two-channel oscilloscope and a battery-powered, floating signal generator, as the [syllabus](electricity/index.md#equipment-gates) describes them; some handheld scopes have both | US$60–150 |

## Home pins

Each part has a home: the pins it uses in every lesson. Keep to them and a
circuit from an earlier lesson can often stay on the breadboard.

| Part | Mega pins |
|---|---|
| Buttons | 22, 23, 24, 25 |
| LEDs: red, yellow, green, blue, white | 26, 27, 28, 29, 30 |
| Keypad: rows 1–4, columns 1–4 (lessons without the buttons and LEDs) | 22–25, 26–29 |
| LCD: RS, E, D4, D5, D6, D7 | 31, 32, 33, 34, 35, 36 |
| 74HC595: data, clock, latch | 37, 38, 39 |
| Four-digit display: digits 1 to 4 | 40, 41, 42, 43 |
| Servo | 44 |
| RFID reset | 45 |
| LED matrix: DIN, CLK, CS | 47, 48, 49 |
| RFID: MISO, MOSI, SCK, SDA | 50, 51, 52, 53 |
| IR receiver | 2 |
| Dimmable LED | 3 |
| Motor: enable, forward, backward | 4, 8, 9 |
| RGB LED: red, green, blue | 5, 6, 7 |
| Passive buzzer (through 220 Ω) | 10 |
| Relay | 11 |
| Active buzzer, through its S8050 driver | 12 |
| Ultrasonic sensor: trigger, echo | 14, 15 |
| DHT11 | 16 |
| 18B20 temperature | 17 |
| Rotary encoder: CLK, DT (its push switch is a button on 22) | 18, 19 |
| I2C (clock, accelerometer): SDA, SCL | 20, 21 |
| Potentiometer | A0 |
| Photoresistor | A1 |
| Thermistor | A2 |
| Joystick: X, Y (its button on 22) | A3, A4 |
| Sound sensor | A5 |
| Water sensor: S, and + (powered only while it reads) | A6, A7 |
| Obstacle and tap sensors, when they share a board with the bridge's modem and the PIR | 16, 17 |
| Stepper driver: IN1 to IN4 | A8, A9, A10, A11 |
| On/off sensor modules | A12, A13, A14, A15 |
| FM radio: SDIO, SCLK, RST (lessons without the four-digit display) | 40, 41, 42 |
| 433 MHz radio: receiver DATA, transmitter DAT | 43, 46 |
| A serial radio (LoRa modem or module, Meshtastic board) on Serial1: TX1, RX1 (lessons without the rotary encoder) | 18, 19 |
| A second serial radio, on Serial3: TX3, RX3 (lessons without the ultrasonic sensor) | 14, 15 |
| A bridge board's LoRa modem, on Serial3: TX3, RX3 (on Serial2, 16 and 17, beside the ultrasonic sensor) | 14, 15 |
| IR LED (KY-005), through 220 Ω (lessons without the dimmable LED) | 3 |
| LoRa module: M0 and M1 joined, AUX; the second module's | 40, 41; 42, 43 |

The passive buzzer sits on pin 10 because a sounding buzzer borrows the timer
that makes PWM on pins 9 and 10; pin 10 could not dim anything anyway.

## Breadboard homes

Each part also has a home on the breadboard: the same holes in every lesson
that uses it, so a build carries on from one lesson to the next and you seldom
move anything that works. Columns count from 1 at the end nearest the Mega;
rows **a**–**e** are below the middle gap and **f**–**j** above it. The LCD's
pins stand at the far right, in a47–a62. Its body hangs off the bottom edge
and beyond the end of the board, leaving the other parts in their homes
nearer the Mega. Support the overhang at breadboard height.

The drawings color each wire by what it carries: black for GND, red for
5 V and orange for 3.3 V, which a 3.3 V module such as the LoRa modem needs
and 5 V would ruin. Every other color is a signal, and wires that cross or
sit side by side differ where the kit's colors allow. Your wires needn't
match, but keeping black, red and orange for those three makes a build easy
to check.

| Part | Home |
|---|---|
| GND from the Mega | The GND at the end of the long header, outer pin, into the bottom − rail by column 3 |
| 5V from the Mega | The 5V at the top of the long header, outer pin, into the top + rail by column 3 |
| Joining the rails, when a lesson needs both pairs | The bottom − rail to the top − rail by column 41 (GND), the top + rail to the bottom + rail by column 42 (5V) |
| Button on 22, 23, 24, 25 | Across the gap in columns 2–4, 8–10, 14–16, 20–22: the pin into row j of the left column, a black jumper from row a of the right column to the − rail |
| LED on 26, 27, 28, 29, 30 | Columns 6, 12, 18, 24, 30: the pin into j, 220 Ω from g across the gap to e, the long leg in b, the short leg in b of the next column, a black jumper from a of that column to the − rail |
| Dimmable LED on 3 | Column 38, laid out like the other LEDs. Lesson 8's optional automatic lamp uses column 34 to leave room beside the photoresistor |
| Passive buzzer on 10 | Across the gap in column 33: + in f33, − in e33, pin 10 into j33, 220 Ω from a33 to the bottom − rail |
| Active buzzer on 12 | + in f33, − in e33; h33 to the top + rail. S8050 emitter a29, base a30, collector a31; b29 to bottom −29, b31 to a33. Pin 12 into a32, 1 kΩ c32–c30, 10 kΩ b30–bottom −30. Diode unbanded c33, banded c36; a36 to j33. In Lesson 12 the buzzer moves to column 35, its supply and collector wires follow, and the diode becomes c35–c38 with a38 to j35 |
| RFID input dividers | For SDA, SCK, MOSI and RST use columns 18, 20, 22 and 24: Mega pin into j, 1 kΩ g–e, reader signal into c, 2 kΩ from a to bottom − rail at 18, 21, 22 and 25 respectively. MISO goes directly to pin 50 |
| RGB LED on 5, 6, 7 | Legs in a6 (red), a9 (green), a11 (blue), the common leg in the bottom − rail by column 7; a 220 Ω resistor across the gap above each colored leg, and its pin into j. In Lesson 42 the button covers e9, so the green resistor stands in g13–e13, pin 6 enters j13, and a jumper joins b13 to b9 |
| Light or temperature divider, on A1 or A2 | Column 37: a red jumper from j37 to the top + rail by column 37, the sensor across the gap in f37 and e37, the pin into c37, and 10 kΩ from a37 to the bottom − rail by column 37. Lesson 46 uses both: the thermistor moves to f33–e33, A2 into a33, 10 kΩ from c33 to c36, and a jumper from a36 to the bottom − rail by column 36 |
| Knob on A0 | Across the middle gap: its outer legs in f39 and f41, with jumpers from j39 to the top − rail and j41 to the top + rail; its wiper in d40, A0 into a40 |
| The screen (LCD, contrast knob, backlight) | Knob across the middle gap (outer legs in f43 and f45, wiper in d44), the LCD's pins in a47–a62, wired by `bench.screen ()` |
| Rotary encoder | Standing in row a, columns 15–19, its knob toward you: GND, +, SW, DT and CLK from the left; jumpers from e15 to the top − rail by column 18 and e16 to the top + rail by column 19, and 22, 19 and 18 into e17, e18 and e19 |
| Power module | Lying to the right of the breadboard and above the LCD's overhang, never plugged in, both jumpers off: a red wire from its 5V pin (or an orange one from its 3.3V pin) into the bottom + rail by column 42, a black wire from its GND into the bottom − rail by column 42 |
| FM radio | Standing in row j, columns 27–34 (GPIO2 in j27 to 3.3V in j34), its board over the top rails: pins 42, 41 and 40 up from below into f29, f31 and f32, the Mega's 3.3V into f34, 1 kΩ from h29 to h34, a black jumper from f33 to the bottom − rail by column 33 |
| 433 MHz receiver | Standing in row j, columns 30–33 (VCC in j30): 5V from the power header into f30, pin 43 into f31, a black jumper from f33 to the bottom − rail by column 33 |
| 433 MHz transmitter | Standing in row j, columns 38–41 (EN in j38): pin 46 into f36, 1 kΩ from h36 to h39 and 2 kΩ from g39 to e39, a black jumper from a39 to the bottom − rail by column 39; the Mega's 3.3V into f40, a black jumper from f41 to the bottom − rail by column 41 |

Some homes share space: the FM radio covers the buzzer's upper holes, the
433 MHz receiver uses its column, and the transmitter covers the A0 knob's
upper holes. No lesson uses those pairs together. The encoder's power
jumpers reach past the GY-521's board, so both keep their places in Lesson 30.

Modules on jumper wires have homes beside the board too, so their wires run
the same way each time. The Mega's inner 5V and GND pins at the ends of the
long header, and the ones on the power header, are free for modules; the
outer pair feed the rails.

| Module | Home | Power |
|---|---|---|
| Ultrasonic sensor | Above the Mega, just right of pins 14 and 15 | VCC from the inner 5V pin; GND into the top − rail by column 5 |
| Motor (on the L293D) | Above the board, its leads straight down into j14 (black) and j17 (red) | From the chip |
| Servo | Below the board and any bridge modem, its plug under columns 34–36 | + into the bottom + rail by column 35, − into the bottom − rail by column 36 |
| Keypad | High above the gap between the Mega and the breadboard, its eight wires rising from pins 22–29 | — |
| DHT11 | Above the board, its pins over columns 35–37 | + into the top + rail by column 36, − into the top − rail by column 37 |
| 18B20 | Above the board, its pins (printed G, R, Y) over columns 26–28 | R (+) into the top + rail by column 27, G (−) into the top − rail by column 25 |
| IR receiver | Above the gap between the Mega and the breadboard; with the screen, above columns 28–30 (its pins printed G, R, Y). In Lessons 47–48, above columns 38–40 to leave room for the clock and the modem's wires | The inner 5V and GND pins; with the screen, R into the top + rail by column 29 and G into the top − rail by column 28. In Lessons 47–48, R into the top + rail by column 39 and G into the top − rail by column 37 |
| PIR sensor | Below the Mega, under the power header | The power header's 5V and GND |
| Obstacle and beam-break sensors | Below the board, under columns 45 and 36 | From the bottom rails beside them |
| Obstacle sensor, beside the bridge's modem | Above the board, pins over columns 35–38 (the DHT11's place) | + into the top + rail by column 36, GND into the top − rail by column 35 |
| Tap sensor, beside the PIR | Above the board, pins over columns 26–28 (the 18B20's place) | + into the top + rail by column 27, − into the top − rail by column 28 |
| Sound sensor | Below the Mega, under the power header | + and G from the power header's 5V and GND |
| Water sensor | Below the Mega, beside the sound sensor | + from A7, − into the power header's second GND |
| LED matrix | Below the gap between the Mega and the breadboard, facing up | VCC from the inner 5V pin; GND into the bottom − rail by column 5 |
| Joystick | Below the Mega, under pins A3 and A4 | The power header's 5V and the inner GND pin |
| Clock module | Above the board on its side, clear of the screen's wires | GND into the top − rail by column 13, VCC into the top + rail by column 15 |
| Stepper driver | Below the Mega, under pins A8–A11 | From the power module's bottom rails: + into the bottom + rail by column 5, − into the bottom − rail by column 6 |
| I2C level shifter | Above the gap between Mega and breadboard, supported on a nonconductive surface; B pins toward the Mega, A pins toward the board. Pin 20 to B1, A1 to h12; pin 21 to B2, A2 to g11 | HV from top +5; Mega 3.3V into a5 and b5 to LV; GND into bottom −12 |
| RFID reader | Below the Mega, facing up | The Mega's 3.3V and the inner GND pin |
| Tap sensor | Below the Mega, at its left end | The power header's 5V and GND |
| Relay | Above the board, its pins toward the Mega and its screw terminals away from it | The inner 5V and GND pins |
| LoRa modems (RYLR896) | Below the board past the button, aerials down: B (on Serial3) under columns 24–29, A (on Serial1) under columns 33–39. Each modem's TXD comes up into row f (26 for B, 35 for A), beside its RX pin in row j; its TX pin goes into j28 or j37, then 1 kΩ across the gap from g to e and 2 kΩ from a down to the − rail, and its RXD into row c of that column | The power module's bottom rails at 3.3 V: B's VDD into the bottom + rail by column 29 and GND into the bottom − rail by column 24, A's into the bottom + rail by column 39 and the bottom − rail by column 33 |
| A bridge board's LoRa modem (Lessons 43–54) | Where Lesson 40's modem B lies: below the board under columns 24–29, aerial down, its TXD into f26 beside the RX pin in j26; the TX pin into j28, 1 kΩ from g28 to e28, 2 kΩ from a28 to the bottom − rail by column 28, and its RXD into c28 | Its VDD from the Mega’s 3.3V pin, GND into the bottom − rail by column 24. Beside the I2C level shifter in Lesson 50, VDD uses e5 to share the 3.3 V feed at a5. Beside the RFID reader, which shares that pin, its VDD comes from the power module's bottom rails at 3.3 V, into the bottom + rail by column 29 (Lesson 51) |
| IR LED (KY-005) | Below the board under columns 36–38, its LED pointing away: pin 3 into j38, 220 Ω from g38 across the gap to e38, S into a38 | − into the bottom − rail by column 36; its middle pin empty |
| LoRa modules (E32) | The same places and dividers as the modems, aerials down. AUX comes up into f25 beside pin 43 (B) or f34 beside pin 41 (A); M0 and M1 into f and g of column 30 beside pin 42 (B) or column 39 beside pin 40 (A) | The power module's bottom rails at 5 V: B's VCC into the bottom + rail by column 24 and GND into the bottom − rail by column 23, A's into the bottom + rail by column 33 and the bottom − rail by column 31 |
| Meshtastic board | In LoRa modem A's place below the board, its pins up: its 48 into f35, its 47 into c37 through A's divider, its GND into the bottom − rail by column 41 | Its own USB-C cable |

Chips and parts that stand in the board without a home above keep one place
too: the 74HC595 across the gap in columns 18–25, the one-digit display from
column 46 and the four-digit display from column 51; the GY-521 in row j,
columns 9–16; the L293D across the gap from column 12; the tilt switch in c32 and c33; the
relay's lamp in the LED shape in column 13.

## Reading resistors

The kit's resistors are blue, with five colored bands. Hold the resistor
with the lone band that sits apart (brown, for 1%) on the right, and read
from the left: three digits, then how many zeros follow.

<ul class="color-code" aria-label="The digit each color stands for">
  <li><span class="band black"></span><b>0</b> black</li>
  <li><span class="band brown"></span><b>1</b> brown</li>
  <li><span class="band red"></span><b>2</b> red</li>
  <li><span class="band orange"></span><b>3</b> orange</li>
  <li><span class="band yellow"></span><b>4</b> yellow</li>
  <li><span class="band green"></span><b>5</b> green</li>
  <li><span class="band blue"></span><b>6</b> blue</li>
  <li><span class="band violet"></span><b>7</b> violet</li>
  <li><span class="band gray"></span><b>8</b> gray</li>
  <li><span class="band white"></span><b>9</b> white</li>
</ul>

| Resistor | Bands |
|---|---|
| 220 Ω | red, red, black, black, brown |
| 330 Ω | orange, orange, black, black, brown |
| 1 kΩ | brown, black, black, brown, brown |
| 2 kΩ | red, black, black, brown, brown |
| 10 kΩ | brown, black, black, red, brown |

So red, red, black, then black, is 2, 2, 0 and no more zeros: 220 Ω. Some
resistors have four bands instead (two digits, then the zeros); 220 Ω is
then red, red, brown. The kit's resistor card is labeled too, so check the
card when in doubt.
