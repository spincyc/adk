---
lesson: 28
promise: Sense which way is down, and turn the matrix into a spirit level.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix from Lessons 25 to 27
  - GY-521 accelerometer module (MPU-6050)
  - BSS138 bidirectional I2C level shifter (Adafruit 757 layout), with headers fitted; an extra part
  - 12 female-to-male jumper wires
  - 6 jumper wires
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
and its own resistors that hold the two wires high between bits. Its
**signals still need 3.3 V**. A bidirectional I2C level shifter lets each
side use its own voltage: 5 V at the Mega and 3.3 V at the sensor.

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
    The sketch draws a bubble on the matrix that moves with the tilt. Lift
    the far edge of the breadboard, the long edge away from you: will the
    bubble move toward the raised edge or away from it? And if you keep
    lifting until the board stands on its near long edge, where will the
    bubble end up?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep the LED matrix and its five
    wires from the last three lessons, and the Mega's GND wire; take the
    joystick, the buzzer and everything else off. Push the GY-521's pins
    firmly into row j, in columns 9 to 16: it should lie flat, parallel to
    the breadboard, over the top rails. Its board hides the rail holes under
    it, so its 5V comes from the top + rail at column 7, beside it, and its
    GND crosses the middle gap in column 10 to reach the bottom − rail.

    Add the **BSS138 bidirectional I2C level shifter**, an extra part
    described in [the kit list](../../kit.md#i2c-level-shifter). Lay it
    above the gap between the Mega and breadboard, on a nonconductive
    support. Its fitted headers take female-to-male wires. HV takes 5 V,
    LV takes the Mega’s 3.3 V through column 5, and GND joins GND. Pin 20
    reaches B1, with A1 to SDA; pin 21 reaches B2, with A2 to SCL. Leave
    channels 3 and 4 empty. Do not join A and B with jumper wires.

If your level shifter came with its header pins loose, they must be
soldered on before it can take its wires: see
[Soldering](../../safety.md#soldering).

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

??? info "A QMI8658 board in the GY-521's place"
    Some kits have a board marked **ICM40607&QMI8658** where the GY-521
    should be. It carries a different chip, the QMI8658, and ADK reads it
    too, through the same calls: `adk::Mpu6050` looks for it first, at its
    own address, `0x6B`. Its eight pins are named **5V**, **GND**, **SCL**,
    **SDA**, **3V3**, **RST**, **SWDIO** and **SWCLK**, in their own order,
    so stand it in the same holes, then move each of the four wires to the
    column of the pin with its name: the red one from the top + rail, by
    column 7, to **5V**'s column, the two black ones that carry GND across
    the gap to **GND**’s, the level shifter’s **A2** wire to **SCL**’s
    and its **A1** wire to **SDA**’s.
    Its other four pins stay unconnected. Its axes should follow the arrows
    printed on it, but that hasn't been checked on a real board: the bubble
    test tells you.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 028-tilt**:

<!-- sketch -->

What's new:

- `adk::Mpu6050 tilt {0x68};` names the accelerometer and its address on the
  bus. It needs no pin numbers: I2C is always pins 20 and 21 on the Mega.
- `tilt.measured ()` is an event: true in the update where a reading
  finished, which happens every 20 ms.
- `tilt.ok ()` is false if the chip didn't answer that reading. Then the
  matrix scrolls *NO SENSOR* instead of a bubble, so a loose wire is easy
  to spot.
- `tilt.pitch ()` and `tilt.roll ()` are `float`s, numbers with decimals, as
  in Lesson 14: 2.7 degrees, say.
- `showBubble ()` turns degrees into dots. `degreesPerDot` is 3, and
  `lround (pitch / degreesPerDot)` rounds to the nearest whole number, so
  every 3° moves the bubble one dot. `constrain` stops it at the edge, 9°
  either way. The bubble is 2 × 2 dots, so its top-left corner goes from 0
  to 6, with 3 in the middle.
- `roll` has a minus sign because the matrix counts y downwards: raising the
  far edge should send the bubble up.
- `fabs (pitch) < 1` asks whether the pitch is under 1° either way: `fabs ()`
  is a number's size without its sign, so `fabs (-0.4)` is 0.4. When both
  angles are that small, `drawFrame ()` lights the border, using
  `matrix.row (0, 0b11111111)` for the top and bottom rows, binary just as
  in Lesson 25.

## Upload it

Upload the sketch and lay the breadboard flat on a table. A 2 × 2 bubble
sits in the middle of the matrix; if the table is level, the frame lights up
around it. Whenever you lift the breadboard, keep the Mega flat on the
table beside it, support the level shifter on the table too, and leave
their wires slack, so nothing pulls loose.

Lift the right-hand end of the breadboard, the end away from the Mega: the
bubble slides right. Now test your prediction and lift the far edge: the
bubble slides up, toward the raised edge. The bubble always rises to the
high side, like a real bubble in a real level.

Keep lifting until the board stands on its near long edge. Now no part of
gravity lies along z, so z reads about 0, and the whole 1000 mg moves to
the axis that points up, y. The roll is 90°, far past the 9° the bubble
can show, so it runs to the top edge of the matrix and stays there. To see
the numbers themselves, try the second challenge below.

## If it doesn't work

| What you see | Try this |
|---|---|
| *NO SENSOR* scrolls | Check pin 20 → B1 and A1 → SDA, and pin 21 → B2 and A2 → SCL on the level shifter. HV needs 5 V and LV needs 3.3 V. Check VCC's red jumper from the top + rail to i9 and the red wire from the Mega's 5V to the top + rail by column 3, GND's two black jumpers (f10 to e10, a10 to the − rail), and that the header is pushed well into row j. |
| The bubble moves the wrong way left and right | Your module's X arrow points the other way. In `showBubble ()`, change `3 + lround (pitch / degreesPerDot)` to `3 - lround (pitch / degreesPerDot)`. |
| The bubble moves the wrong way up and down | Change `3 - lround (roll / degreesPerDot)` to `3 + lround (roll / degreesPerDot)`. |
| Up and down follow left and right instead | The arrows are turned a quarter round. Swap `pitch` and `roll` in the call to `showBubble ()`, then fix any direction as above. |
| The frame never lights, even on a level table | Cheap MPU-6050s can be a degree or two out. Try *Zero it* in *Make it yours*. |
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

1. **A finer level.** Make each dot worth 1.5° instead of 3°: change
   `degreesPerDot` to 1.5. What happens to the shivering?
2. **Watch the numbers.** Start Serial with `Serial.begin (9600);` before
   `adk::setup ()`. With each reading, print pitch and roll on one line
   separated by a tab, `adk::println (Serial, pitch, '\t', roll);`, and open
   the Serial Plotter to see both angles as moving lines. Print
   `tilt.acceleration ().z` too, and stand the board on its edge.
3. **Zero it.** Add a button on pin 22. When it's pressed, remember the
   current pitch and roll and subtract them from every reading, so any
   surface you like becomes "level".
4. **Tilt alarm.** Add the passive buzzer from Lesson 27 and sound a warning
   whenever the board tips more than 20°.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**, never in **10A**.

The black probe stays in c10 for every reading: column 10 is the GY-521's
GND, carried across the gap by its black jumper. The red probe goes in row
f, below the module, where each column is joined to one of its pins. Hold
the tip straight: f9 carries 5 V and f10, right beside it, is GND, so a tip
that touched both would join them. Nothing needs slowing down. On the
QMI8658 board, use the columns of its GND, 5V and SDA pins instead; it has
no AD0.

!!! question "Predict"
    Between readings, nothing is being sent on SDA. Will it read 0 V, 5 V,
    or something else?

<!-- measure -->

What the numbers tell you:

- **The module's supply** is the 5 V of the top + rail. The MPU-6050 itself
  runs on 3.3 V, which a small regulator on the module makes from the 5 V.
- **SDA at the sensor** reads about 3.3 V between messages. On an I2C bus,
  each chip can pull the wire low or release it; resistors lift it high.
  The level shifter passes lows between the sides while allowing separate
  high voltages: 3.3 V at A1 and about 5 V at B1. Do not connect those
  sides directly. The [MPU-6050 datasheet](https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/mpu-6000-datasheet1.pdf)
  limits an input to its supply plus 0.5 V; a reading near 4 V at the
  sensor is a reason to unplug and check the interface, not a safe target.
- Every 20 ms the Mega asks for a reading, and 14 bytes go past in under
  2 ms, far too fast for the meter. It only averages them in, which nudges
  its number down a little.
- **AD0** reads 0 V: a resistor on the module holds it low, and a low AD0
  makes the chip answer to address `0x68`, the number in
  `adk::Mpu6050 tilt {0x68};`. Joined to 3.3 V instead, it would answer to
  `0x69`.

## Check yourself

1. The GY-521 takes its power from the 5 V rail. Why do its SDA and SCL
   wires still go through the level shifter?
2. Many chips can share the same two I2C wires. How does the Mega talk to
   just the MPU-6050, and what would happen if its AD0 pin were joined to
   3.3 V?
3. Why are the pitch and roll right only while the board is held still?

??? note "Answers"
    1. The module's regulator makes 3.3 V for the chip, but SDA and SCL go
       straight to the chip, and its signals need 3.3 V. The shifter passes
       the lows between the sides while each is held high at its own voltage:
       5 V at the Mega, 3.3 V at the sensor.
    2. Every chip answers only to its own address, and the sketch asks for
       `0x68`. With AD0 joined to 3.3 V the chip would answer to `0x69`
       instead, so nothing would answer and the matrix would scroll
       *NO SENSOR*.
    3. ADK works them out from how gravity's 1 g is shared between x, y and z.
       While the board moves, the chip feels the push of your hand as well,
       and that throws the angles off.
