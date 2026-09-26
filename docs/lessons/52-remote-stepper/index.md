---
lesson: 52
promise: Turn a knob on one board and watch a turntable on the other swing to the same angle, then say it has arrived.
time: 60 minutes
level: 3
parts:
  - "Board A: Arduino Mega 2560, its USB cable and a breadboard"
  - "Board A: the LCD1602 screen, with its 10 kΩ contrast knob and 220 Ω resistor, wired as in Lesson 13"
  - "Board A: a 10 kΩ potentiometer, the knob"
  - "Board B: a second Mega 2560 and breadboard (not in one kit), and a USB power bank or charger"
  - "Board B: the power module and its 9 V adapter"
  - "Board B: the 28BYJ-48 stepper and ULN2003 driver, and a paper disc about 6 cm across"
  - "Each board: a REYAX RYLR896 LoRa modem (add-on, not in the kit), a 1 kΩ and a 2 kΩ resistor, at the bridge's home"
  - "Board A: 22 jumper wires and 4 female-to-male; Board B: 3 jumper wires and 10 female-to-male"
ideas:
  - A machine that reports back
  - Two values, one going each way
  - Degrees into half-steps, and back
  - A knob that doesn't twitch
---

## What you'll build

<!-- closeup A -->

On Board A, a knob and the screen. Turn the knob and the screen says
**Knob says 90°**. In another room, Board B's stepper motor swings a paper
turntable round to 90 degrees, and as it goes it tells Board A where it
has got to: the screen's bottom row counts **Turning: 23°**, **Turning:
67°**, and then **Arrived: 90°**. Turn the knob somewhere else and the
turntable follows, while the screen keeps you told.

<!-- closeup B -->

## The idea

**A machine that reports back.** Sending an order across the house is
easy; knowing it was carried out is the hard part. So here Board B
answers: Board A shares the angle it wants, and Board B shares the angle
it is at. When the
two agree, the job is done, and Board A can say so. Engineers call a
system that checks its result and acts on it a **closed loop**: the
command goes out, the report comes back, and the loop closes.

**Two values, one going each way.** Each board shares its own value and
reads the other's:

| Board | Shares | Reads |
|---|---|---|
| A, the knob | `angle`, the angle the knob asks for | `at`, where the turntable is |
| B, the turntable | `at`, where the turntable is | `angle`, the angle to turn to |

A name only has to be unique on the board that shares it. Board A's
`angle` and Board B's `at` travel in opposite directions, each in its
own messages.

**Degrees into half-steps, and back.** The stepper from Lesson 31 counts
half-steps: 4096 of them make one turn. The knob speaks in degrees, 360 to
a turn, so each degree is

<p class="formula">4096 ÷ 360 ≈ 11.38 half-steps</p>

and 90 degrees is 90 × 11.38 = 1024 half-steps, a quarter turn. Board B
works that out, rounds it to a whole half-step, and moves there. To report
back, it goes the other way: its position divided by 11.38, rounded to the
nearest degree.

**A knob that doesn't twitch.** The knob gives an angle from 0 to 360,
which Board A rounds down to fives. A knob resting exactly on the line
between two angles, say 44.9 and 45.1, would flick between 40 and 45
many times a second, and the turntable would twitch back and forth with
it. So Board A only changes its angle once the knob has moved a whole 5
degrees from it. Between those points, a tiny wobble changes nothing.

!!! question "Predict"
    Board B turns its turntable at 500 half-steps a second, and the bridge
    sends at most ten messages a second. Turn the knob from 0 to 90 in one
    go. How long will the turntable take to get there? And while it turns,
    will the screen count 1, 2, 3, ... or jump? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables, and Board B's power module adapter,
    before you change any wiring. Set Board B's power module jumpers before
    you plug it in: the bottom one to **5V**, for the motor, and the top one
    **OFF**.

Each board keeps its LoRa modem at the bridge's home: lying below the
breadboard under columns 42 to 47, its spring pointing down, its divider
in column 46 and its VDD fed from the Mega's 3.3V pin. The steps begin
with what to keep from Lesson 51 and what to take out.

!!! danger "3.3 V for the modems"
    The modem's VDD takes 3.3 V, never 5 V, and gets it by a wire from the
    Mega's **3.3V** pin. Its RXD only ever sees the Mega's TX pin through
    the 1 kΩ, with the 2 kΩ to GND. [Safety](../../safety.md#radios) says
    why.

### Board A: the knob

The screen goes at its home, from column 5, exactly as in Lesson 13. The
knob can't stand at its usual home in columns 45 to 47, because the
modem's divider is in column 46, so it takes its home beside the screen,
columns 57 to 59; A0's wire comes round below the modem. The four-digit
display of Lesson 11 would cover the modem's divider too, which is why the
screen shows the angles here.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: the turntable

The driver board lies below the Mega, wired as in Lesson 31, and takes its
power from the power module at the right end of the breadboard. The
modem's GND joins the same bottom − rail, and the Mega's GND reaches it at
B-3, so every part agrees where 0 V is.

<!-- bench B -->

<!-- steps B -->

Push the motor's white plug into the driver's socket. Draw an arrow from
the middle of the paper disc to its edge, push the disc's center onto the
motor's shaft, and stand the motor with its shaft pointing up. Put a mark
on the table beside the arrow and label it 0.

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Each board has a sketch of its own. Open **File → Examples → Adk →
Lesson52RemoteStepper → Knob** for Board A:

<!-- sketch A -->

And **Lesson52RemoteStepper → Turntable** for Board B:

<!-- sketch B -->

What's new:

- Both boards set up the modem and the bridge as in Lesson 43. Board A is
  address 1 with Board B, address 2, as its partner, and Board B the
  other way round.
- `readKnob ()` reads the knob in degrees with `knob.read (0, 360)`, and
  only takes a new angle when `abs (reading - angle)`, how far the knob is
  from the angle either way, is 5 or more: `abs ()` from Lesson 21 drops
  the sign, so −7 and 7 are both 7. `reading / 5 * 5` rounds down to
  fives: whole numbers divide without the fraction, so 47 / 5 is 9, and
  9 × 5 is 45.
- `bridge.share ("angle", angle);` on Board A and
  `bridge.value ("angle")` on Board B carry the angle across, as the
  bridge has carried numbers since Lesson 43. `bridge.share ("at", at);`
  on Board B and `bridge.value ("at")` on Board A carry the answer back.
- `stepsPerDegree` is a `float`, from Lesson 14, because 4096 ÷ 360 isn't
  a whole number; the `.0` in `360.0` makes the division keep its
  fraction. `lround ()` rounds to the nearest whole number, as in
  Lesson 28, since `moveTo ()` takes whole half-steps.
- `motor.moveTo ()` goes to a position counted from where the motor was
  when Board B started, and asking again for where it is already going
  changes nothing, so Board B can ask on every pass of `loop ()`.
- `showAngles ()` compares the two: the same number means the turntable
  has arrived. If the bridge hasn't heard Board B for five seconds, it says
  so instead. The spaces after each number wipe out what a longer number
  left behind.
- On Board B, the Mega's own **L** LED shows `bridge.isConnected ()`.

## Upload it

1. Plug Board A into your computer and upload **Knob** to it. If both
   Megas are plugged in at once, each has its own port: choose the right
   one in **Tools → Port** before each upload.
2. Plug Board B in, and upload **Turntable** to it. Then unplug it, carry
   it to another room, and power it there from a USB power bank or a phone
   charger. Plug in its power module's adapter and press the module's
   button, so its LED lights.
3. Twist the paper disc until its arrow points at the 0 mark, and press
   Board B's reset button: Board B counts every angle from where the
   turntable is when it starts.
4. Board B's **L** LED lights within a couple of seconds, and Board A's
   screen shows the knob's angle and **Arrived: 0°**, or starts turning
   the turntable to wherever the knob already points.
5. Turn the knob. The screen shows the new angle at once, then counts the
   turntable round until it says **Arrived**.

You predicted how long 90 degrees takes: 1024 half-steps at 500 a second
is about two seconds, and the radio adds only a tenth or so. And the count
jumps. The turntable turns about 44 degrees a second, and the bridge sends
at most ten messages a second, so each report is about 4 or 5 degrees on
from the last, and now and then a longer jump.

Once in a while the count may stop for a second or two, then catch up.
Both modems share one channel, and a message sent while the other modem is
sending is lost, as two people talking at once both go unheard. The bridge
sends everything again every two seconds, so a lost message is soon made
good.

## If it doesn't work

| What you see | Try this |
|---|---|
| **No word from B** | Is Board B powered and running **Turntable**? Its **L** LED lights when it hears Board A. Check both modems' wiring against the steps: TXD into f44, RXD into c46, pin 14 into j46, pin 15 into j44, and VDD to the Mega's 3.3V pin. |
| The screen counts, but the turntable doesn't move | Is Board B's power module on, with its bottom jumper on **5V**? Push the motor's white plug fully into its socket. |
| The motor hums or shakes but hardly turns | Two of Board B's IN wires are swapped: IN1 to A8, IN2 to A9, IN3 to A10, IN4 to A11. |
| **Arrived** shows the wrong place on the table | Board B counts from where the arrow was when it started. Point the arrow at 0 and press Board B's reset button. |
| The turntable goes anticlockwise | That's fine: it is the way positive steps turn your motor. Lesson 31 says more. |
| The count stops for a moment, then catches up | A message was lost, and the bridge sent it again. If it happens all the time, move the boards closer, or keep their aerials upright. |
| A blank lit screen, or a row of blocks | Turn the screen's contrast knob, the one in columns 5 to 7. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The bridge turns each shared value into plain text. When the knob
    moves, Board A's modem sends:

    ```text
    @angle=90
    ```

    and while the turntable turns, Board B's sends a new position each
    time the bridge may, at most ten times a second:

    ```text
    @at=23
    ```

    A value that hasn't changed isn't sent again, so a still knob and a
    still turntable are quiet, but every two seconds each board sends
    everything it shares, changed or not. That makes good any message that
    was lost, and it is how each board knows the other is still there.

    Board B's sketch asks the motor to go to `lround (angle *
    stepsPerDegree)` on every pass. The motor only starts a new move when
    that number changes, and it can change its mind halfway: turn the
    knob back while the turntable is still going, and it turns round and
    heads for the new angle.

## Make it yours

1. **Home.** Put a button on Board A's pin 23, at its home beside the
   screen in columns 38 to 40, and make it send the turntable to 0 with
   `angle = 0;`. Try it: why does nothing happen? Make the knob take over
   again only once it moves.
2. **Slow and steady.** Change Board B's `motor.speed (500)` to
   `motor.speed (100)`. How big are the jumps in the count now?
3. **The short way round.** From 350 to 10 degrees, the turntable goes all
   the way back round rather than 20 degrees forward. Make Board B choose
   the shorter way: `motor.position ()` says where it is now.
4. **A bell on arrival.** Add the passive buzzer on pin 10, through its
   220 Ω, to Board A, and make it ding once when the screen changes from
   **Turning** to **Arrived**.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Both readings are on
Board A. Turn the knob until the screen says the angle, then take the
reading; the knob stays where you leave it.

!!! question "Predict"
    The knob shares 5 V out along its track, and the sketch turns 0 to
    5 V into 0 to 360 degrees. What will A0 read when the screen says
    **Knob says 90°**?

<!-- measure A -->

What the numbers tell you:

- **At 90°** the knob's middle leg reads about 1.25 V: 90 is a quarter of
  360, and a quarter of 5 V is 1.25 V.
- **At 180°** it reads about 2.5 V, half of 5 V. Every volt is 72 degrees,
  so the angle on the far side of the house is set by a voltage on this
  side, turned into a number, sent through the air and turned into steps.
