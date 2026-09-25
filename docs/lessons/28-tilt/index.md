---
lesson: 28
title: Tilt
arc: Tilt and turn
promise: Sense which way is down, and turn the matrix into a spirit level.
time: 45 minutes
level: 2
sketch: Lesson28Tilt
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix from Lessons 25 to 27
  - GY-521 accelerometer module (MPU-6050)
  - 5 female-to-male jumper wires
  - 4 jumper wires
ideas:
  - The I2C bus and addresses
  - An accelerometer feels gravity
  - Pitch and roll from three readings
  - A spirit level on the matrix
---

## What you'll build

<!-- closeup -->

A digital spirit level. A small board on your breadboard feels which way
gravity pulls, and a square bubble on the matrix floats towards whichever
side of the breadboard is higher, just like the bubble in a builder's level.
Get the breadboard perfectly flat and the bubble settles in the middle while
a frame lights up around it. Try it on a table, a book, a windowsill.

## The idea

The GY-521 module carries an **MPU-6050**, a chip that measures
acceleration. It talks to the Mega over **I2C** (say "I squared C"), a bus
that many chips can share using just two wires: **SDA** carries the data and
**SCL** the clock, the beat the bits follow. Every chip on the bus has an
**address**, a number it answers to; the MPU-6050's is `0x68`, a hexadecimal
(base 16) number, 104 in ordinary counting. The Mega's I2C pins are 20 (SDA)
and 21 (SCL). The module has its own 3.3 V regulator, so it runs from 5V,
and its own resistors that hold the two wires high between bits.

An **accelerometer** measures how hard something is pushed, along three
directions at right angles, x, y and z. Lying still, it isn't pushed by
movement at all, but it still feels gravity: the table holds it up with a
push of exactly 1 g. The axis pointing straight up reads 1000 milli-g, and
the others read 0.

Tilt the board and that 1 g is shared between the axes. Lift the end the X
arrow points to by 30°, and part of gravity now lies along x:

<p class="formula">x = 1000 × sin 30° = 500 mg &nbsp;&nbsp; z = 1000 × cos 30° ≈ 866 mg</p>

Working backwards from the two readings gives the angle: that's the
**pitch**, how far the X end is raised. The **roll** is the same for the
side the Y arrow points to. ADK works them out for you in degrees with
`tilt.pitch ()` and `tilt.roll ()`.

!!! question "Predict"
    The breadboard lies flat, so z reads about 1000 and x and y about 0.
    What will z read if you stand the breadboard on one of its long edges?
    And what about x or y?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep the LED matrix and its five
    wires from the last three lessons; take the joystick, the buzzer and
    everything else off. Push the GY-521's pins firmly into row j: it
    should lie flat, parallel to the breadboard.

<!-- bench -->

<!-- steps -->

??? info "The GY-521's pins, and its arrows"
    Only four of the eight pins are needed. **XDA** and **XCL** are a second
    I2C bus for adding a compass chip, **AD0** changes the address to `0x69`
    when connected to 3.3 V, and **INT** can signal the Mega when a reading
    is ready. ADK uses none of them, so they stay unconnected.

    Printed near the chip are two arrows, **X** and **Y**. With the module
    in row j and its board lying over the top edge of the breadboard, X
    points along the breadboard, away from the Mega, and Y points away from
    you. Modules differ, so check yours: the bubble test below tells you if
    the sketch needs turning round.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson28Tilt**:

<!-- sketch -->

The new parts:

- `adk::Mpu6050 tilt {0x68};` names the accelerometer and its address on the
  bus. It needs no pin numbers: I2C is always pins 20 and 21 on the Mega.
- `tilt.measured ()` is an event: true in the update where a new reading
  arrived, which happens every 20 ms.
- `tilt.ok ()` is false if the chip didn't answer. Then the matrix scrolls
  *NO SENSOR* instead of a bubble, so a loose wire is easy to spot.
- `showBubble ()` turns degrees into dots: `lround (pitch / 3)` rounds to
  the nearest whole number, so every 3° moves the bubble one dot, and
  `constrain` stops it at the edge, 9° either way. The bubble is 2 × 2 dots,
  so its top-left corner goes from 0 to 6, with 3 in the middle.
- `roll` has a minus sign because the matrix counts y downwards: raising the
  far edge should send the bubble up.
- `drawFrame ()` lights the border when both angles are under 1°, using
  `matrix.row (0, 0b11111111)` for the top and bottom rows, binary just as
  in Lesson 25.

## Upload it

Upload the sketch and lay the breadboard flat on a table. A 2 × 2 bubble
sits in the middle of the matrix; if the table is level, the frame lights up
around it. Lift the right-hand end of the breadboard, the end away from the
Mega: the bubble slides right. Lift the far edge: the bubble slides up. The
bubble always rises to the high side, like a real bubble in a real level.

## If it doesn't work

| What you see | Try this |
|---|---|
| *NO SENSOR* scrolls | Check SDA goes to pin 20 and SCL to 21: they can't be swapped. Check VCC and GND, and that the header is pushed well into row j. |
| The bubble moves the wrong way left and right | Your module's X arrow points the other way. Change `3 + lround (pitch / 3)` to `3 - lround (pitch / 3)`. |
| The bubble moves the wrong way up and down | Change `3 - lround (roll / 3)` to `3 + lround (roll / 3)`. |
| Up and down follow left and right instead | The arrows are turned a quarter round. Swap `pitch` and `roll` in the call to `showBubble ()`, then fix any direction as above. |
| The frame never lights, even on a level table | Cheap MPU-6050s can be a degree or two out. Try the calibration in *Make it yours*. |
| The bubble shivers | Tap the table and watch: the chip feels every bump. Keep the breadboard still. |

??? note "How it works"
    ADK drives the Mega's own I2C hardware at 100,000 bits a second. Every
    20 ms it reads 14 bytes from the MPU-6050 in one go, starting at
    register 0x3B: acceleration x, y and z, the chip's temperature, and
    rotation x, y and z, so all the numbers belong to the same moment.
    When the sketch starts it wakes the chip, which powers up asleep, sets
    it to measure up to ±2 g, and turns on its filter, which smooths out
    vibrations faster than about 44 times a second.

    Pitch and roll come from gravity alone:
    pitch = atan2 (x, √(y² + z²)) and roll = atan2 (y, z). While the board is
    being shaken the chip feels your hand as well as gravity, so the angles
    are only right when it's held still.

## Make it yours

1. **A finer level.** Make each dot worth 1.5° instead of 3°. What happens
   to the shivering?
2. **Watch the numbers.** Start Serial with `Serial.begin (9600);` before
   `adk::setup ()`, print pitch and roll on one line separated by a tab, and
   open the Serial Plotter to see both angles as moving lines.
3. **Zero it.** Add a button on pin 22. When it's pressed, remember the
   current pitch and roll and subtract them from every reading, so any
   surface you like becomes "level".
4. **Tilt alarm.** Add the passive buzzer from Lesson 27 and sound a warning
   whenever the board tips more than 20°.
