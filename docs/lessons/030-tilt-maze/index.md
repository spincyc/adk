---
lesson: 30
promise: Roll a ball through a maze by tilting the breadboard, level after level.
time: 1 hour
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix, GY-521 and BSS138 I2C level shifter from Lesson 28
  - The rotary encoder from Lesson 29
  - Passive buzzer
  - 220 Ω resistor (red, red, black, black, brown)
  - 12 female-to-male jumper wires
  - 13 jumper wires
ideas:
  - A ball with a position and a speed
  - Walls stored as bits, and testing one bit
  - Choosing a level with the encoder
  - Putting a whole game together
laws:
  - {law: pwm, section: measure-it, for: "Shows high and low notes both average 2.5 V"}
  - {law: logic-levels, section: measure-it, for: "Keeps SDA at the sensor's 3.3 V through the shifter"}
---

## What you'll build

<!-- closeup -->

The wooden maze game where you tilt a tray to roll a marble into the goal,
made of light. Turn the knob to flip through four mazes and click to pick
one. A single glowing ball waits in the top-left corner and an exit blinks
in the bottom-right. Tilt the breadboard and the ball rolls: it speeds up
downhill, stops against the walls, and when it reaches the exit the buzzer
cheers and the next maze is ready.

## The idea

A real marble has two things that matter: **where it is** and **how fast it
is going**. The sketch keeps both, for each direction, as `float` numbers
that can hold fractions: the ball can be 3.4 dots across, moving 0.05 dots
per reading. Fifty times a second, when a new reading arrives from the
accelerometer of Lesson 28, three things happen:

1. The tilt changes the speed. The more the board is tilted, the more speed
   the ball gains downhill: a thousandth of a dot per reading for every
   degree.
2. A tenth of the speed is taken away, like friction, so the ball doesn't
   roll forever and settles at a steady speed for each tilt.
3. The speed moves the ball, unless a wall is in the way.

The steady speed is where the two balance: gaining 0.001 × 10 = 0.01 per
reading at 10° of tilt, and losing a tenth of the speed, balances at
0.1 dots per reading. That's 0.1 × 50 = 5 dots a second: across the matrix
in under two seconds.

The mazes are pictures, like those in Lesson 25: eight bytes, one per row,
where each 1 is a wall. To ask whether the dot at column 5 of a row is a
wall, the sketch makes a byte with a single 1 in column 5. It starts from
`0b10000000`, a 1 in column 0, and `>> 5` shifts the 1 five places to the
right: `0b00000100`. Then `&` keeps only the 1s the two bytes share. If
anything is left, it's a wall.

!!! question "Predict"
    Roll the ball along the top row of the first maze twice: once with the
    board tipped gently, and once tipped about twice as far. Which run will
    be faster, and by about how much? Will the ball keep speeding up all the
    way along the row, or settle at a steady speed?

## How the game plays

| State | On the matrix | What changes it |
|---|---|---|
| Choosing | The maze you're choosing | Turning the knob shows the next or previous maze, with a tick. Clicking starts it. |
| Playing | The maze, the ball, and the blinking exit | Rolling onto the exit in the bottom-right corner: a cheer, and back to choosing, with the next maze ready. |

Whatever tilt the board has at the moment you click counts as **flat** for
that game, so hold the breadboard level when you click. That also cancels
out a sensor that is a degree or two out, or a table that isn't level.

The ball stops dead against a wall. The edges of the matrix are walls too.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep the rotary encoder standing
    in row a, with its two jumpers and three wires, from Lesson 29, and the
    Mega's GND and 5V wires, and take the rest off. The GY-521 and the
    matrix you put aside in Lesson 29 come back: the GY-521 in row j,
    columns 9 to 16, and the matrix below the breadboard, both wired as in
    Lesson 28, including the level shifter between pins 20/21 and SDA/SCL.
    Support the shifter on the table and leave its wires slack as you tilt.
    A black jumper from the bottom − rail to the top − rail, by
    column 41, now brings GND to the top − rail, as the screen's wiring did
    in Lesson 29. The passive buzzer goes through its 220 Ω resistor, as in
    Lesson 27. You will pick up the breadboard to play, so use wires long
    enough to let it move, and keep the Mega flat on the table beside it.

<!-- bench -->

<!-- steps -->

??? info "Holding the maze"
    Hold the breadboard at both ends with the Mega's end on your left, like
    a tray, and tilt it gently: a few degrees is plenty. The ball rolls
    towards the low side: lower the right end and it rolls right, lower the
    near edge and it rolls down the matrix. The matrix itself stays flat on
    the table, the right way up, like a screen showing the tray from above.
    If the ball rolls uphill, see *If it doesn't work* below: the fix is a
    sign in `rollBall ()`.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 030-tilt-maze**:

<!-- sketch -->

What's new:

- Every part is one you've met: the accelerometer and matrix from Lesson 28,
  the encoder from Lesson 29, and the speaker from Lesson 27.
- `using Maze = adk::Array<uint8_t, 8>;` names a maze's type, as `Picture`
  did in Lesson 25. `mazes` holds four of them, each written `Maze {...}` so
  the array knows what it holds, as the menu's items did in Lesson 29. Every
  maze is open in the top-left corner, where the ball starts, and the
  bottom-right, where the exit is.
- `struct Ball` keeps the ball's place, `x` and `y`, and its speed each way,
  all `float`s. `ball = {0, 0, 0, 0};` puts it back in the corner, standing
  still, in one line.
- `loop ()` has the game's states: *NO SENSOR* if the accelerometer didn't
  answer, `chooseMaze ()` while not playing, and the game itself while
  playing.
- `chooseMaze ()` moves through the mazes with `knob.turned ()`, counting
  round in a circle with `%` as Lesson 25 counted its slides. A click back
  from maze 0 would make −1, and `%` of a number below zero is below zero
  too, so the sketch adds `mazes.size ()` first: 0 + 4 − 1 = 3, and 3 % 4
  is 3, the last maze. The click resets the ball and records `flatPitch`
  and `flatRoll`, the tilt that counts as flat.
- `rollBall ()` runs once per reading. It updates the speed, then tries the
  move in x and in y separately, so a ball rolling along a wall keeps
  sliding the way it's free to go. When a wall is in the way, that speed
  becomes 0 and the ball stops dead. `ball.speedX * 0.9` keeps nine tenths
  of the speed each time, so the ball slows down by itself; written with
  its decimal point, the number keeps its fraction in the sum.
  `ball.x += ball.speedX` adds the speed to the place, with Lesson 21's
  `+=`.
- After each roll, `loop ()` draws the game: the maze, then the exit, lit
  only while `exitLit` is on, then the ball.
- `isFree ()` rounds the ball's position to a dot with `lround ()`, as in
  Lesson 28, and checks that it is on the matrix. `&&` stops at the first
  thing that is false, so a dot off the matrix is never looked up in the
  maze. Then `mazes[maze][row] & (0b10000000 >> column)` tests that dot's
  bit. `>>` is Lesson 10's `<<` the other way: it moves the 1 to the right.
- One `&` is not the same as two. `&&` joins two true-or-false answers, as in
  Lesson 4's box; a single `&` between two numbers works bit by bit, keeping
  only the 1s they share, as *The idea* showed. The brackets round the
  whole `&` matter: C++ does `==` before `&`, so without them it would ask
  whether `(0b10000000 >> column) == 0` first. `isFree ()` hands back the
  answer, true or false, with `return`.
- `celebrate ()` plays the `cheer`, lets it finish with `adk::wait ()`, and
  chooses the next maze.

## Upload it

Upload the sketch with the breadboard lying flat. The matrix shows the
first maze, a zigzag. Turn the knob: each click ticks and shows another
maze; after the fourth, the first comes round again. Choose the zigzag,
hold the breadboard level and click. Two rising notes play, the ball sits
in the top-left corner and the exit blinks in the bottom-right.

Now test your prediction on the top row. Tip the board gently to your
right and watch the ball cross the row, then tip it about twice as far to
your left and watch it come back. The steeper run takes about half the
time: twice the tilt gives twice the push, and the friction balances it at
twice the speed. Either way the ball gathers speed for a moment, then rolls
on at a steady pace. It doesn't keep speeding up, because the faster it
goes, the more the friction takes away.

Then roll the ball down through each gap to the exit, and listen for the
cheer.

## If it doesn't work

| What you see | Try this |
|---|---|
| *NO SENSOR* scrolls | Check the level shifter: pin 20 → B1 / A1 → SDA, pin 21 → B2 / A2 → SCL, HV to 5 V and LV to 3.3 V; then the GY-521: VCC's red jumper from the top + rail to i9 and the red wire from the Mega's 5V to the top + rail by column 3, GND's black jumpers from f10 to e10 and a10 to the − rail, and its pins well down in row j. |
| The ball rolls uphill | The GY-521's arrows point differently on your module. In `rollBall ()`, change `- (tilt.pitch () - flatPitch)` to `+`, or the `+` before `(tilt.roll () - flatRoll)` to `-`, whichever axis is wrong. If you changed a sign in Lesson 28, change the same axis here. |
| The ball drifts on a level board | Hold it level when you click: that tilt is what counts as flat. |
| Turning the knob does nothing | Check the wires from pins 18 and 19 in e19 and e18, above CLK and DT, and the encoder's jumpers from e15 to the top − rail by column 18 and e16 to the top + rail by column 19, and the black jumper from the bottom − rail to the top − rail, by column 41 that grounds the top − rail. |
| Clicking doesn't start the maze | The knob's switch is on pin 22: push the knob straight in, toward the breadboard, steadying its board from behind. |
| No sound | Check the buzzer's + leg is in f33, under pin 10's wire in j33, its other leg in e33, and the resistor runs from a33 to the bottom − rail, which needs its GND wire. |

??? note "How it works"
    The accelerometer takes a reading every 20 ms, so `tilt.measured ()`
    is the game's clock: the ball moves exactly once per reading, and the
    speeds are in dots per reading. The blink and the sounds run on their
    own through `adk::update ()`.

    A ball moving less than one dot per reading can never jump over a
    wall. The fastest it can go, with the board stood on end at 90°, is
    0.001 × 90 ÷ 0.1 = 0.9 dots per reading, so the walls hold however
    steeply you tip it, short of turning it over.

    The ball's position is a `float`, and `lround ()` decides which dot it
    is in: from 2.5 up to 3.5 it's dot 3. So a ball in dot 3 has to move
    past 3.5 before it enters dot 4, and a wall in dot 4 stops it at the
    edge of dot 3, as if the ball had a size.

## Make it yours

1. **Your own maze.** Design one on squared paper, keeping the top-left
   and bottom-right corners open, and add it to `mazes` as a fifth `Maze`.
   Nothing else needs to change: the sketch counts the mazes with
   `mazes.size ()`.
2. **Against the clock.** Start an `adk::Stopwatch`, as in Lesson 3, when
   the maze starts. When the ball escapes, scroll the time in seconds,
   printed into an `adk::Text` as Lesson 27 printed the score.
3. **A knock.** Make a wall knock when the ball hits it hard enough to
   hear. In `rollBall ()`, just before a speed becomes 0, play a short low
   note, `speaker.tone (adk::note::c3, 20);`, but only if that speed is
   more than 0.05 dots per reading either way: `fabs (ball.speedX) > 0.05`,
   with `fabs ()` from Lesson 28. A gentle roll into a wall stays silent.
4. **Bouncy walls.** Instead of stopping dead, make the ball bounce back
   at half speed: set the speed to `-ball.speedX * 0.5` instead of 0, and
   the same for y.
5. **Traps.** Add holes that send the ball back to the start: a second
   `Maze` of holes for each maze, drawn blinking, and checked the same way
   as the walls.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**, never in **10A**. Keep
each probe tip in its own hole, so it can't bridge two.

The tick you hear while choosing a maze lasts 10 ms, far too short to read.
In `chooseMaze ()`, change `speaker.tone (adk::note::c6, 10);` to
`speaker.tone (adk::note::c6, 3000);` and upload: now each click of the
knob, as you turn it, holds a high C for three seconds. Turn it, don't
press it, and read. Then change `c6` to `c3`, a C three octaves lower,
upload, and read again. Put the line back as it was when you're done. The
last reading is on SDA, as in Lesson 28, with the black probe in c10, the
GY-521's GND column.

!!! question "Predict"
    The high C switches pin 10 on and off about 1047 times a second, the
    low C about 131 times. Which one will the meter read higher?

<!-- measure -->

What the numbers tell you:

- **Both notes read the same**, about 2.5 V, or a little less as in
  Lesson 27. Every wave is high for half its time, however long the wave
  is, so the average is always half of 5 V. The meter shows how much of the
  time the pin is on; the pitch is how often it switches, and that the
  meter can't see at all. Your ear is the other way round.
- **SDA on a tilted board** reads about 3.3 V, flat or tipped, just
  as it did in Lesson 28. The level shifter keeps the Mega’s 5 V pull-ups
  separate from the sensor’s 3.3 V signals.
  Hold the probes in their holes and tip the breadboard gently, or have
  someone tip it for you: the number doesn't follow. Tilt Lesson 26's
  joystick and its two knobs' voltages follow the stick; tilt the
  accelerometer and the angle travels as numbers instead, a burst of bits
  every 20 ms, far too quick for the meter. All it sees is the bus resting
  high between them.

## Check yourself

1. Holding the same tilt, why does the ball settle at a steady speed
   instead of speeding up for ever?
2. Why does `rollBall ()` try the move across and the move down
   separately?
3. A row of a maze is `0b00111111`. How does the sketch find out that the
   dot in column 1 is free?

??? note "Answers"
    1. The tilt adds the same push every reading, but the friction takes
       away a tenth of the speed. The faster the ball goes, the more it
       loses, until the loss matches the push.
    2. So that a wall in one direction only stops that half of the move.
       The ball keeps going the other way and slides along the wall,
       instead of sticking to it.
    3. `0b10000000 >> 1` is `0b01000000`, a single 1 in column 1. `&` with
       the row keeps only the 1s both share, and there are none, so the
       answer is 0: no wall.
