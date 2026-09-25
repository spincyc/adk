# What's in the kit

The course is built around the **Elegoo Mega 2560 Most Complete Starter
Kit**. A few lessons also use modules from the **Elegoo 37 in 1 Sensor
Modules Kit**; those are marked below and in each lesson's parts list.

## The parts

| Part | What it does | First used |
|---|---|---|
| Arduino Mega 2560 and USB cable | The computer that runs your sketches | [Lesson 1](lessons/01-blink/index.md) |
| 830-hole breadboard and jumper wires | Joins parts without soldering | Lesson 1 |
| LEDs, 220 Ω and 1 kΩ resistors | Light, and the current that makes it | Lesson 1 |
| Push buttons | Input you can press | Lesson 2 |
| Active buzzer | Beeps when switched on | Lesson 3 |
| RGB LED | Any color, mixed from red, green and blue | Lesson 4 |
| Passive buzzer | Plays any note you ask for | Lesson 5 |
| 10 kΩ potentiometer | A knob that sets a voltage | Lesson 7 |
| Photoresistor | Senses light | Lesson 8 |
| 74HC595 shift register | Eight outputs from three pins | Lesson 10 |
| One- and four-digit 7-segment displays | Numbers in light | Lessons 10, 11 |
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

## Home pins

Each part has a home: the pins it uses in every lesson. Keep to them and a
circuit from an earlier lesson can often stay on the breadboard.

| Part | Mega pins |
|---|---|
| Buttons | 22, 23, 24, 25 |
| LEDs: red, yellow, green, blue, white | 26, 27, 28, 29, 30 |
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
| Passive buzzer | 10 |
| Relay | 11 |
| Active buzzer | 12 |
| Ultrasonic sensor: trigger, echo | 14, 15 |
| DHT11 | 16 |
| 18B20 temperature | 17 |
| Rotary encoder: CLK, DT | 18, 19 |
| I2C (clock, accelerometer): SDA, SCL | 20, 21 |
| Potentiometer | A0 |
| Photoresistor | A1 |
| Thermistor | A2 |
| Joystick: X, Y (its button on 22) | A3, A4 |
| Sound or water sensor | A5 |
| Stepper driver: IN1 to IN4 | A8, A9, A10, A11 |
| On/off sensor modules | A12, A13, A14, A15 |

The passive buzzer sits on pin 10 because a sounding buzzer borrows the timer
that makes PWM on pins 9 and 10; pin 10 could not dim anything anyway.

## Reading resistors

The kit's resistors are blue, with five colored bands. Hold the resistor
with the lone band that sits apart (brown, for 1%) on the right, and read
from the left: three digits, then how many zeros follow.

| Color | Black | Brown | Red | Orange | Yellow | Green | Blue | Violet | Gray | White |
|---|---|---|---|---|---|---|---|---|---|---|
| Digit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |

| Resistor | Bands |
|---|---|
| 220 Ω | red, red, black, black, brown |
| 330 Ω | orange, orange, black, black, brown |
| 1 kΩ | brown, black, black, brown, brown |
| 10 kΩ | brown, black, black, red, brown |

So red, red, black, then black, is 2, 2, 0 and no more zeros: 220 Ω. Some
resistors have four bands instead (two digits, then the zeros); 220 Ω is
then red, red, brown. The kit's resistor card is labelled too, so check the
card when in doubt.
