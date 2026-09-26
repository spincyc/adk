---
lesson: 50
promise: Tilt one breadboard and watch a ball roll the same way across another board's matrix, while a servo leans to match.
time: 60 minutes
level: 3
parts:
  - "Both boards: the Mega, breadboard and LoRa modem from Lesson 49, with the modem's divider and wires"
  - "Board A: the GY-521 accelerometer from Lesson 28 (or the QMI8658 board some kits have in its place)"
  - "Board B: the LED matrix from Lesson 25, and the servo and power module from Lesson 49"
  - "5 female-to-male and 5 jumper wires"
ideas:
  - Streaming a value that keeps changing
  - How often a value can cross the air
  - Smoothing before sending, and gliding after
  - Sending only what has changed
---

## What you'll build

<!-- closeup A -->

<!-- closeup B -->

A tilt game played across a room. Board A carries the accelerometer from
Lesson 28, and nothing to show; Board B carries the LED matrix and the
servo. Tip Board A's breadboard to the left and a ball of light rolls left
across Board B's matrix, speeding up downhill and stopping at the edge,
while Board B's servo arm leans with it. Board A is the pilot's stick and
Board B the plane.

## The idea

**A value that never stops changing.** A button or a key changes now and
then. A tilt changes all the time. Board A's accelerometer gives a new
reading every 20 ms, fifty times a second,
and the tilt is a little different each time, even when you hold the board
still, because no hand is perfectly steady and no sensor is perfectly
quiet.

**How often can it cross?** Not fifty times a second. At the Quick speed a
message takes about 0.05 s on the air, and the bridge sends at most ten
messages a second, so the radio has time to breathe and the other board
always gets the latest value. Board B's servo and ball therefore get a new
tilt at most every tenth of a second. Moved straight to each one, they
would jump in little steps.

**Smooth before sending.** Board A doesn't share every reading as it
comes. It keeps a smoothed tilt: each new reading moves it only a quarter
of the way from where it was, so a shaky reading barely moves it, while a
real tilt gets there in a few readings, about a tenth of a second. Then it
shares the tilt in whole degrees. The bridge sends a value only when it
changes, so a board held still sends nothing new at all, and the air stays
free.

**Glide after receiving.** Board B fills in between the values it gets.
The ball rolls fifty times a second whatever arrives, speeding up and
slowing down as in Lesson 30's maze, so it moves smoothly even though the
tilt behind it changes in steps. The servo glides to each new angle over a
tenth of a second, about the time until the next one comes, instead of
jumping there.

!!! question "Predict"
    Board B's sketch counts how many new tilts it hears each second and
    prints the count in the Serial Monitor. While you hold Board A still,
    what will it print? And while you rock Board A to and fro? Write down
    your guesses.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables and the power module's adapter before
    you wire, and check your work before you plug them back in.

Both boards keep their LoRa modems and their wires from Lesson 49, and
Board B keeps its power module, still with the top jumper **OFF** and the
bottom one on **5V**, and its servo.

### Board A: the tilt

The keypad and the screen come off. The GY-521 stands in row j, columns 9
to 16, just as in Lesson 28, and pins 20 and 21 reach it over the top
header. The Mega's own **L** LED, beside pin 13, shows whether Board B can
be heard, so it needs no wire.

??? info "A board marked ICM40607&QMI8658 instead"
    Some kits come with this board in the GY-521's place. It carries a
    QMI8658 chip, and ADK reads it through the same calls, in the same
    units. Its pins are in a different order, so go by the names printed
    on them: **5V** to the top + rail, **GND** to the − rail, **SCL** to
    pin 21 and **SDA** to pin 20, and leave **RST**, **SWDIO**, **3V3** and
    **SWCLK** unconnected. Its axes
    should follow its printed arrows as the GY-521's do, but no one has
    checked that on a real board yet: if the ball rolls the wrong way, see
    *If it doesn't work*.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: the ball

The RGB LED and its resistors come off. The LED matrix lies below the gap
between the Mega and the breadboard, at its home from Lessons 25 to 30.
Pin 44's wire now drops beside the board, clear of the matrix, and runs
down the modem's left side to the servo.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

In the Arduino IDE, choose **File → Examples → Adk → lessons → 050-tilt-pilot**:
**Tilt** is for Board A and **Ball** for Board B.

### Board A: Tilt

<!-- sketch A -->

What's new:

- `pitch += (tilt.pitch () - pitch) / 4;` is the smoothing: the gap
  between the new reading and the smoothed tilt, a quarter of it at a time.
  After ten readings, a fifth of a second, a sudden tilt has come about
  95% of the way.
- `lround (pitch)` rounds to the nearest whole degree, since the bridge
  carries whole numbers. 12.4° goes as 12.
- `bridge.share ("sensor", tilt.ok ())` tells Board B whether the
  accelerometer answered, so B can say so.
- `light.set (bridge.isConnected ())` lights the **L** LED while Board B
  can be heard.

### Board B: Ball

<!-- sketch B -->

What's new:

- `bridge.value ("pitch")` and `("roll")` are Board A's latest tilt, in
  degrees. Between messages they simply stay as they were.
- `frame` ticks every 20 ms, and `rollBall ()` moves the ball on each
  tick, as `rollBall ()` did in Lesson 30: the tilt changes the speed, a
  tenth of the speed is lost, and the speed moves the ball. `constrain`
  keeps it on the matrix, and at an edge its speed that way is set to 0.
- `arm.moveTo (90 + pitch, 100)` glides the servo to the angle over
  100 ms. A new angle part way through starts a new glide from wherever
  the arm has got to. `constrain` keeps the angle from 0 to 180.
- `heard` counts the updates in which a new pitch or roll arrived, and
  `second` prints and clears it once a second.
- The matrix scrolls `CALLING A` while Board A can't be heard, and
  `NO SENSOR` if Board A says its accelerometer didn't answer.

## Upload it

1. Plug in Board A, open **Tilt**, choose its port under **Tools → Port**,
   and upload it.
2. Plug in Board B and the power module's adapter, switch the module on,
   open **Ball**, choose Board B's port, and upload it. The matrix scrolls
   `CALLING A` for a moment; then the ball appears in the middle, and
   Board A's **L** LED lights.
3. Open the Serial Monitor on Board B's port at 9600 baud.
4. Lay Board A's breadboard flat, the Mega's end on your left. Now lift
   its right-hand end: the ball rolls left, downhill, and stops against
   the edge, and the servo arm leans over. Lower the near edge instead,
   and the ball rolls down the matrix, as in Lesson 30.

Now your prediction. Held still on the table, Board A sends nothing new,
and the Serial Monitor prints `New tilts this second: 0`. Rock it to and
fro and the count climbs, but never past 10: that's the bridge's limit,
and the radio's. Yet the ball and the arm move smoothly, because Board B
fills in between.

Try it without the smoothing. In **Tilt**, change both `/ 4` to `/ 1`, so
each reading is taken as it is, and upload. Hold Board A still: the count
may not drop to 0 any more, because the rounded tilt flickers between two
whole degrees. Put the `/ 4` back. Then, in **Ball**, change the servo's
glide from 100 to 0 and upload: the arm now jumps to each new angle, and
you can see it step.

## If it doesn't work

| What you see | Try this |
|---|---|
| The matrix keeps scrolling `CALLING A` | Is Board A's sketch running? Check each modem as in Lesson 49, and that the sketches are **Tilt** on Board A and **Ball** on Board B. |
| The matrix scrolls `NO SENSOR` | Board A can't find its accelerometer. Check the GY-521 as in Lesson 28: VCC from T+7, GND to the − rail, SCL to pin 21 and SDA to pin 20. |
| The ball rolls uphill | The GY-521's arrows point differently on your module, as Lesson 30 warned. In Board B's `rollBall ()`, change the `-` before `pitch` to `+`, or the `+` before `roll` to `-`, whichever way is wrong. |
| The ball drifts with Board A flat | Your table isn't quite level, or your sensor reads a degree or two off. See *Make it yours*. |
| The servo doesn't move | Is the power module on, with its bottom jumper on 5V? Check the servo's red wire in B+53, its brown in B-54 and pin 44's wire to its orange. |
| The matrix is blank | Check its wires as in Lesson 25: DIN on 47, CLK on 48, CS on 49, VCC and GND. |

??? note "How it works"
    Tilt Board A quickly from flat to 20°, and here is what crosses the
    air, a tenth of a second or so apart, as the smoothed tilt catches up:

    ```text
    @pitch=5
    @pitch=16
    @pitch=19
    @pitch=20
    ```

    Then nothing, until Board A moves again, except that every two seconds
    each board repeats everything it shares, `@pitch=20 roll=0 sensor=1`,
    so each knows the other is still there.

    The smoothing costs a little time: a sudden tilt reaches Board B about
    a tenth of a second later than it would unsmoothed. Every smoothing
    trades quickness for calm; dividing by 2 instead of 4 is quicker but
    shakier, and by 8 calmer but slower.

## Make it yours

1. **Level first.** When Board A starts, take its tilt as flat, as Lesson
   30 did with `flatPitch` and `flatRoll`, and share the tilt from there.
   Which board should do it, and why?
2. **Both arms.** Add a second servo on pin 45 that follows the roll. Up
   to three servos can share Timer 5, on pins 44, 45 and 46.
3. **The maze.** Bring Lesson 30's mazes to Board B, with the walls, the
   exit and the cheer. Board A needs no change.
4. **Slow mail.** Change both sketches' `adk::LoraSpeed::Quick` to
   `adk::LoraSpeed::Far`, whose messages take about 0.3 s. How often does
   Board B hear a new tilt now, and how does the ball feel?
