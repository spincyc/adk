---
lesson: 30
title: Tilt Maze
arc: Tilt and turn
promise: Roll a ball through a maze by tilting the breadboard, level after level.
time: 1 hour
level: 3
sketch: Lesson30TiltMaze
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix and GY-521 from Lesson 28
  - Rotary encoder module
  - Passive buzzer
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 female-to-male jumper wires
  - 7 jumper wires
ideas:
  - A ball with a position and a speed
  - Walls stored as bits, and testing one bit
  - Choosing a level with the encoder
  - Putting a whole game together
---

## What you'll build

<!-- closeup -->

The wooden maze game where you tilt a tray to roll a marble into the goal,
made of light. Turn the knob to flip through four mazes and click to pick
one. A single glowing ball waits in the top-left corner and an exit blinks
in the bottom-right. Tilt the breadboard and the ball rolls: it speeds up
downhill, knocks against the walls, and when it reaches the exit the buzzer
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
    At 10° the ball settles at 5 dots a second. How fast will it roll with
    the board tilted 5°? About how long will it take to cross the matrix?

## How the game plays

| State | On the matrix | What changes it |
|---|---|---|
| Choosing | The maze you're choosing | Turning the knob shows the next or previous maze, with a tick. Clicking starts it. |
| Playing | The maze, the ball, and the blinking exit | Rolling onto the exit in the bottom-right corner: a cheer, and back to choosing, with the next maze ready. |

Whatever tilt the board has at the moment you click counts as **flat** for
that game, so hold the breadboard level when you click. That also cancels
out a sensor that is a degree or two out, or a table that isn't level.

The ball stops dead against a wall, and if it hit hard enough to hear, the
buzzer knocks. The edges of the matrix are walls too.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep the matrix, and move the
    GY-521 five columns along, to columns 7 to 14, so the top rail's ground
    wire fits in at column 3, with room for the wires beside it. The passive buzzer goes through its 220 Ω
    resistor, as in Lesson 27. You will pick up the breadboard to play, so
    use wires long enough to let it move, and keep the Mega flat on the
    table beside it.

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

Open **File → Examples → Adk → Lesson30TiltMaze**:

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
- `chooseMaze ()` moves through the mazes with `knob.turned ()` and `wrap ()`,
  the same count-round-in-a-circle as Lesson 29's menu. The click resets the
  ball and records `flatPitch` and `flatRoll`, the tilt that counts as flat.
- `rollBall ()` runs once per reading. It updates the speed, then tries the
  move in x and in y separately, so a ball rolling along a wall keeps
  sliding the way it's free to go. When a wall is in the way, `bump ()`
  stops the ball, and knocks if it was going faster than 0.05 dots per
  reading.
- `isFree ()` rounds the ball's position to a dot with `lround ()` and checks
  that it is on the matrix. `&&` stops at the first thing that is false, so
  a dot off the matrix is never looked up in the maze. Then
  `mazes[maze][row] & (0b10000000 >> column)` tests that dot's bit.
- `drawGame ()` shows the maze, lights the exit only while `exitLit` is on,
  and lights the ball.
- `celebrate ()` plays the `cheer`, lets it finish with `adk::wait ()`, and
  chooses the next maze.

## Upload it

Upload the sketch with the breadboard lying flat. The matrix shows the
first maze, a zigzag. Turn the knob: each click ticks and shows another
maze; after the fourth, the first comes round again. Choose the zigzag,
hold the breadboard level and click. Two rising notes play, the ball sits
in the top-left corner and the exit blinks in the bottom-right. Tip the
board to your right: the ball rolls along the top row. Roll it down through
each gap to the exit, and listen for the cheer.

You predicted the speed at 5°. Half the tilt gives half the push, 0.005 of a
dot per reading, and it balances the friction at half the speed: 0.05 dots
per reading, or 2.5 dots a second. It crosses the matrix in about three
seconds, where 10° took under two.

## If it doesn't work

| What you see | Try this |
|---|---|
| *NO SENSOR* scrolls | Check the GY-521: SDA to pin 20, SCL to pin 21, VCC and GND, and its pins well down in row j. |
| The ball rolls uphill | The GY-521's arrows point differently on your module. In `rollBall ()`, change `- (tilt.pitch () - flatPitch)` to `+`, or the `+` before `(tilt.roll () - flatRoll)` to `-`, whichever axis is wrong. |
| The ball drifts on a level board | Hold it level when you click: that tilt is what counts as flat. |
| Turning the knob does nothing | Check CLK on 18 and DT on 19, and + and GND from the power header. |
| Clicking doesn't start the maze | The knob's switch is on pin 22: press the shaft straight down. |
| No sound | Check the buzzer's + leg is in h20, the resistor runs from f16 to f20, and the black jumper from f23 reaches the − rail, which needs its GND wire. |

??? note "How it works"
    The accelerometer takes a reading every 20 ms, so `tilt.measured ()`
    is the game's clock: the ball moves exactly once per reading, and the
    speeds are in dots per reading. The blink and the sounds run on their
    own through `adk::update ()`.

    A ball moving less than one dot per reading can never jump over a
    wall. The fastest it can go, with the board stood on end at 90°, is
    0.001 × 90 ÷ 0.1 = 0.9 dots per reading, so the walls always hold.

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
   written into text with `snprintf` as Lesson 27 wrote the score.
3. **Bouncy walls.** Instead of stopping dead, make the ball bounce back
   at half speed: have `bump ()` return `-speed * 0.5`.
4. **Traps.** Add holes that send the ball back to the start: a second
   `Maze` of holes for each maze, drawn blinking, and checked the same way
   as the walls.
