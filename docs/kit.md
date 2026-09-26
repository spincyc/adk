# What's in the kit

The course is built around the **Elegoo Mega 2560 Most Complete Starter
Kit**. A few lessons also use modules from the **Elegoo 37 in 1 Sensor
Modules Kit**; those are marked below and in each lesson's parts list.

## The parts

| Part | What it does | First used |
|---|---|---|
| Arduino Mega 2560 and USB cable | The computer that runs your sketches | [Lesson 1](lessons/01-blink/index.md) |
| 830-hole breadboard and jumper wires | Joins parts without soldering | Lesson 1 |
| LEDs, and 220 Ω, 1 kΩ and 10 kΩ resistors | Light, and the current that makes it | Lesson 1 |
| Push buttons | Input you can press | Lesson 2 |
| Active buzzer | Beeps when switched on | Lesson 3 |
| RGB LED | Any color, mixed from red, green and blue | Lesson 4 |
| Passive buzzer | Plays any note you ask for | Lesson 5 |
| 10 kΩ potentiometer | A knob that sets a voltage | Lesson 7 |
| Photoresistor | Senses light | Lesson 8 |
| 74HC595 shift register | Eight outputs from three pins | Lesson 10 |
| One- and four-digit 7-segment displays | Numbers in light | [Lesson 10](lessons/10-dice/index.md), [Lesson 11](lessons/11-four-digits/index.md) |
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

## Add-on radios

Lessons 37 to 42 use radios that aren't in either kit. Each costs a few
dollars; the LoRa radios come in pairs, because it takes two to talk.

| Part | What it does | First used |
|---|---|---|
| Si4703 FM radio board (CJMCU-470) and wired earbuds | FM stations, their names and songs | Lesson 37 |
| 433 MHz transmitter (WL102-341) and receiver (RX470C) | Short messages across a house | Lesson 38 |
| Two REYAX RYLR896 LoRa modems | Messages across a kilometer or more, on 915 MHz | Lesson 40 |
| Two Ebyte E32-433T20D LoRa modules | A 433 MHz link that passes on lines of text | Lesson 41 |
| Two Heltec WiFi LoRa 32 V3 boards running Meshtastic, and a phone | Text messages from a phone, across a mesh | Lesson 42 |

Their pins work at 3.3 V, so the Mega's signals reach them through a
resistor divider, 1 kΩ and 2 kΩ; [Safety](safety.md#radios) explains why, and
which bands you may send on where you live. If your resistor card has no
2 kΩ, two 1 kΩ resistors in a row make one.

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
| Active buzzer | 12 |
| Ultrasonic sensor: trigger, echo | 14, 15 |
| DHT11 | 16 |
| 18B20 temperature | 17 |
| Rotary encoder: CLK, DT (its push switch is a button on 22) | 18, 19 |
| I2C (clock, accelerometer): SDA, SCL | 20, 21 |
| Potentiometer | A0 |
| Photoresistor | A1 |
| Thermistor | A2 |
| Joystick: X, Y (its button on 22) | A3, A4 |
| Sound or water sensor | A5 |
| Stepper driver: IN1 to IN4 | A8, A9, A10, A11 |
| On/off sensor modules | A12, A13, A14, A15 |
| FM radio: SDIO, SCLK, RST (lessons without the four-digit display) | 40, 41, 42 |
| 433 MHz radio: receiver DATA, transmitter DAT | 43, 46 |
| A serial radio (LoRa modem or module, Meshtastic board) on Serial1: TX1, RX1 (lessons without the rotary encoder) | 18, 19 |
| A second serial radio, on Serial3: TX3, RX3 (lessons without the ultrasonic sensor) | 14, 15 |
| LoRa module: M0 and M1 joined, AUX; the second module's | 40, 41; 42, 43 |

The passive buzzer sits on pin 10 because a sounding buzzer borrows the timer
that makes PWM on pins 9 and 10; pin 10 could not dim anything anyway.

## Breadboard homes

Each part also has a home on the breadboard: the same holes in every lesson
that uses it, so a build carries on from one lesson to the next and you seldom
move anything that works. Columns count from 1 at the end nearest the Mega;
rows **a**–**e** are below the middle gap and **f**–**j** above it. The screen
lies over the bottom rails from column 6 to 37, so the parts that share a
lesson with it have a second home past it.

| Part | Home | Beside the screen |
|---|---|---|
| GND from the Mega | The GND at the end of the long header, outer pin, into B-3 | Same |
| 5V from the Mega | The 5V at the top of the long header, outer pin, into T+3 | Same |
| Joining the rails, when a lesson needs both pairs | B-60 to T-60 (GND), T+61 to B+61 (5V) | Same |
| Button on 22, 23, 24, 25 | Across the gap in columns 2–4, 8–10, 14–16, 20–22: the pin into row j of the left column, a black jumper from row a of the right column to the − rail | 23 in columns 38–40 |
| LED on 26, 27, 28, 29, 30 | Columns 6, 12, 18, 24, 30: the pin into j, 220 Ω from g across the gap to e, the long leg in b, the short leg in b of the next column, a black jumper from a of that column to the − rail | — |
| Dimmable LED on 3 | Column 38, laid out like the other LEDs | Same |
| Buzzer, active on 12 or passive on 10 | Across the gap in column 34: + in f, − in e, the pin into j; a black jumper (active) or the 220 Ω resistor (passive) from a to the − rail. With the four-digit display's wiring (Lesson 12), column 35, clear of the wires that rise over the gap in column 32 | Column 51 |
| RGB LED on 5, 6, 7 | Legs in a6 (red), a9 (green), a11 (blue), the common leg in B-7; a 220 Ω resistor across the gap above each colored leg, and its pin into j | Columns 41–46, the same shape |
| Light or temperature divider, on A1 or A2 | Column 40: a red jumper from j40 to T+40, the sensor across the gap in f40 and e40, the pin into a40, 10 kΩ from c40 to c43, a black jumper from a43 to the − rail | Same |
| Knob on A0 | Legs in e45, e46, e47: a black jumper from a45 to the − rail, A0 into a46, a red jumper from d47 to T+49 | The same, or legs in e57, e58, e59 with the jumper to T+61 when the RGB LED takes columns 41–46 |
| The screen (LCD, contrast knob, backlight) | Knob in e5–e7, the LCD's pins in a9–a24, wired by `bench.screen ()` | — |
| Power module | The right end, in columns 60 and 61 of all four rails | Same |
| FM radio | Standing in row j, columns 45–52 (GPIO2 in j45 to 3.3V in j52), its board over the top rails: pins 42, 41 and 40 up from below into f47, f49 and f50, the Mega's 3.3V into f52, 1 kΩ from h47 to h52, a black jumper from f51 to B-51 | Same |
| 433 MHz receiver | Standing in row j, columns 48–51 (VCC in j48): 5V from the power header into f48, pin 43 into f49, a black jumper from f51 to B-51 | Same |
| 433 MHz transmitter | Standing in row j, columns 56–59 (EN in j56): pin 46 into f54, 1 kΩ from h54 to h57 and 2 kΩ from g57 to e57, a black jumper from a57 to B-57; the Mega's 3.3V into f58, a black jumper from f59 to B-59 | Same |

The radios' homes overlap others beside the screen: the FM radio's the RGB
LED's and the buzzer's, the receiver's the buzzer's, and the transmitter's
the knob's. No lesson uses those together.

Modules on jumper wires have homes beside the board too, so their wires run
the same way each time. The Mega's inner 5V and GND pins at the ends of the
long header, and the ones on the power header, are free for modules; the
outer pair feed the rails.

| Module | Home | Power |
|---|---|---|
| Ultrasonic sensor | Above the Mega, just right of pins 14 and 15 | VCC from the inner 5V pin; GND into T-5 |
| Motor (on the L293D) | Above the board, its leads straight down into j14 (black) and j17 (red) | From the chip |
| Servo | Below the board, its plug under columns 52–54 | + into B+53, − into B-54 |
| Keypad | High above the gap between the Mega and the breadboard, its eight wires rising from pins 22–29 | — |
| DHT11 | Above the board, its pins over columns 35–37 | + into T+36, − into T-37 |
| 18B20 | Above the board, its pins over columns 26–28 | + into T+27, − into T-28 |
| IR receiver | Above the gap between the Mega and the breadboard; beside the screen, above columns 28–30 | The inner 5V and GND pins; beside the screen, T+29 and T-30 |
| PIR sensor | Below the Mega, under the power header | The power header's 5V and GND |
| Obstacle and beam-break sensors | Below the board, under columns 45 and 36 | From the bottom rails beside them |
| LED matrix | Below the gap between the Mega and the breadboard, facing up | VCC from the inner 5V pin; GND into B-5 |
| Joystick | Below the Mega, under pins A3 and A4 | The power header's 5V and the inner GND pin |
| Rotary encoder | High above the Mega, over pins 18 and 19 | The inner 5V pin and the GND beside pin 13 |
| Clock module | Above the board on its side, over the screen's wires | GND into T-29, VCC into T+30 |
| Stepper driver | Below the Mega, under pins A8–A11 | From the power module's bottom rails: + into B+5, − into B-6 |
| RFID reader | Below the Mega, facing up | The Mega's 3.3V and the inner GND pin |
| Tap sensor | Below the Mega, at its left end | The power header's 5V and GND |
| Relay | Above the board, its terminals facing left | The inner 5V and GND pins |
| LoRa modems (RYLR896) | Below the board past the button, aerials down: B (on Serial3) under columns 42–47, A (on Serial1) under columns 51–57. Each modem's TXD comes up into row f (44 for B, 53 for A), beside its RX pin in row j; its TX pin goes into j46 or j55, then 1 kΩ across the gap from g to e and 2 kΩ from a down to the − rail, and its RXD into row c of that column | The power module's bottom rails at 3.3 V: B's VDD into B+47 and GND into B-42, A's into B+57 and B-51 |
| LoRa modules (E32) | The same places and dividers as the modems, aerials down. AUX comes up into f43 beside pin 43 (B) or f52 beside pin 41 (A); M0 and M1 into f and g of column 48 beside pin 42 (B) or column 57 beside pin 40 (A) | The power module's bottom rails at 5 V: B's VCC into B+42 and GND into B-41, A's into B+51 and B-49 |
| Meshtastic board | In LoRa modem A's place below the board, its pins up: its 48 into f53, its 47 into c55 through A's divider, its GND into B-59 | Its own USB-C cable |

Chips and parts that stand in the board without a home above keep one place
too: the 74HC595 across the gap in columns 18–25, the one-digit display
from column 46 and the four-digit display from column 51 (with the rails
joined by a black jumper from T-6 to B-6, since the display covers the far
end); the GY-521 in row j, columns 9–16; the L293D across the gap from
column 12; the tilt switch in c32 and c33; the relay's lamp in the LED shape
in column 13.

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
