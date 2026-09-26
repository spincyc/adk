---
lesson: 44
promise: Turn a knob on one board and a servo on the other turns to match, while the screen shows where the servo has got to.
time: 90 minutes
level: 3
parts:
  - "Board A: Lesson 43's Board A, with its LoRa modem and divider"
  - "Board A: LCD1602 display, 10 kΩ potentiometer, 220 Ω resistor (red, red, black, black, brown) and rotary encoder module"
  - "Board A: 5 female-to-male jumper wires and 16 jumper wires"
  - "Board B: Lesson 43's Board B, with its LoRa modem, divider and yellow LED"
  - "Board B: SG90 servo, and the breadboard power module with its 9 V adapter"
  - "Board B: 3 jumper wires, and a piece of card and some tape for the dial"
ideas:
  - Numbers going both ways across the bridge
  - Acting only when a new value arrives
  - Taking turns on the air
  - What a servo knows about where it is
---

## What you'll build

<!-- closeup A -->

A dial on one board that turns a servo on the other. Turn the knob on
Board A a click at a time, and the servo on Board B turns to match,
anywhere from 0° to 180°; the top row of Board A's screen shows the angle
you asked for. Board B answers: once its servo gets there, it says so,
and the bottom row shows the angle it reached. Press the knob, and the
servo goes back to the middle, 90°.

It is the knob from Lesson 29 and the servo from Lesson 17, with the
bridge from Lesson 43 between them. The two boards can be in different
rooms. Remember the band: send on 915 MHz only where it's allowed, as
Lesson 43 said, and see [Radios](../../safety.md#radios).

## The idea

**Both ways.** In Lesson 43 both boards shared the same name. Here each
board shares a name of its own and reads the other's. Board A shares
`angle`, the angle the knob asks for; Board B shares `at`, the angle its
servo has reached.

| Board A | | Board B |
|---|---|---|
| shares `angle`, the knob's angle | → | reads `angle`, and turns the servo |
| reads `at`, and shows it | ← | shares `at`, where the servo got to |

**Acting only when a new value arrives.** Until the first message
arrives, `bridge.value ("angle")` says 0. A servo sent there would swing
to one end of its travel before Board A had said a word. So Board B only
moves the servo when `bridge.changed ("angle")` is true: in the one pass
of `loop ()` in which a new angle arrives, just like a button's
`wasPressed ()`. The first angle to arrive counts as new.

**Taking turns on the air.** A modem can't listen while it sends, and
when two modems send at once, they drown each other out and neither is
heard. While you turn the knob, Board A sends up to ten messages a
second, which fills about half the time on the air. If Board B reported
every step of the servo's glide at the same time, many messages each way
would be lost. So Board B keeps quiet while its servo moves, and shares
`at` only once the servo has stopped. The air is Board A's while you
turn, and then Board B's for the one message that answers. A message
lost anyway is made good by the next refresh, within two seconds.

**What a servo knows.** An SG90 can't tell the Mega where its horn
really points. `servo.angle ()` is the angle ADK last told it to go to,
so `at` says where Board B has sent the servo, not where the horn is. A
servo held back by your finger, or with no power, doesn't get there, and
nothing tells Board B.

**Steps of 5°.** Each click of the knob adds 5° or takes 5° away, and
`constrain` keeps the angle between 0 and 180, as it kept Level between
0% and 100% in Lesson 29.

!!! question "Predict"
    Turn the knob quickly from 90° to 180°: 18 clicks in about half a
    second. How many messages can Board A send in that time? What will the
    bottom row of the screen show while you turn, and just after? Later,
    you'll switch off Board B's power module and turn the knob again.
    What will the bottom row say then?

## Build it

!!! warning "Unplug first"
    Unplug each board's USB cable, and Board B's power module adapter,
    before you wire. Keep fingers and hair away from the servo's horn
    when it moves, and don't force it round by hand.

!!! danger "The servo's power"
    Board B's power module goes on the right end of its breadboard, with
    its **top** jumper **off** and its **bottom** jumper on **5V**, never
    3.3V. The servo takes its power from the bottom rails; its red wire
    never goes to the Mega's 5V, and the modem stays on the Mega's 3.3V
    pin. The Mega's GND at B-3 joins the module's GND, so that the servo
    can read the pulses on pin 44.

Both modems stay at their bridge homes from Lesson 43, with their
dividers and wires, just as they are. Everything else changes around
them.

### Board A

Take out the button, the two LEDs and their wires. The screen goes in at
its home, as in Lesson 13, with its red wire from the Mega's 5V to the
top + rail. The rotary encoder sits at its home above the Mega, as in
Lesson 29: CLK and DT on pins 18 and 19, its switch, SW, on 22, its + on
the inner 5V pin at the top of the long header and its GND on the GND
pin beside pin 13. The wires from pins 14 and 15 rise past the
encoder's on their way to the modem.

<!-- bench A -->

<!-- steps A -->

### Board B

Take out the button, the red LED and their wires; the yellow LED stays,
to show the link. The power module goes on the right end of the board.
The servo lies below the board with its plug under columns 52 to 54, its
+ into B+53 and its − into B-54, as in Lesson 17. It lies lower than it
did there, below the modem, which now takes the servo's old place.

<!-- bench B -->

<!-- steps B -->

??? info "The dial"
    Fit the longest horn on the servo and turn it gently by hand to find
    the ends of its travel. Cut a half circle from card, mark 0° at one
    end, 90° at the top and 180° at the other end, and tape it behind the
    horn, its middle on the servo's shaft, as in Lesson 17.

When you are done, these are the connections each board makes. Board A:

<!-- connections A -->

Board B:

<!-- connections B -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk →
lessons/044-remote-dial → Dial**, for Board A:

<!-- sketch A -->

What's new:

- The modem and the bridge are Lesson 43's, and so are the addresses:
  Board A is 1, and its partner is 2.
- `angle` is the angle asked for, starting in the middle, 90°.
  `knob.turned ()` is 1 for a click clockwise, −1 for a click
  anticlockwise and 0 otherwise, as in Lesson 29, so each click moves the
  angle by `step`, 5°. `constrain` stops it at 0 and 180.
- `click` is the knob's push switch: a press puts `angle` back to 90.
- `bridge.share ("angle", angle)` shares it on every pass of `loop ()`.
  Only a change goes out, and at most ten times a second.
- `refresh` redraws the screen five times a second. `showAngles ()` puts
  the angle asked for on the top row, and below it `bridge.value ("at")`,
  where Board B says the servo has got to, or `Not connected` while Board
  B can't be heard. `degree` is the screen's own degree sign, as in
  Lesson 14.

Then choose **lessons/044-remote-dial → Servo**, for Board B:

<!-- sketch B -->

What's new:

- `if (bridge.changed ("angle"))` moves the servo only when a new angle
  arrives. `servo.moveTo (..., 300)` glides there over 300 ms, as the
  needle did in Lesson 17; an angle that arrives during a glide starts a
  new one from wherever the horn has got to.
- `if (!servo.isMoving ())` shares `at` only while the servo is still.
  During a glide, `servo.angle ()` changes on every pass, but nothing is
  shared, so nothing is sent.
- `connected` is the yellow LED from Lesson 43, showing that Board A can
  be heard.

## Upload it

1. Plug Board B into the computer, choose its port in **Tools → Port**,
   and upload **Servo**. Plug in its power module's adapter and press the
   module's button, so its LED lights. The yellow LED stays dark, and the
   servo lies limp, waiting for its first angle.
2. Plug in Board A, choose its port, and upload **Dial**. The top row
   says `Asked  90°`. Board B's yellow LED lights, and the servo moves to
   90°, if it wasn't there already. Within two seconds the bottom row
   says `Servo  90°`.
3. Turn the knob a click at a time. The top row changes at once, and the
   servo follows it; each time the servo stops, the bottom row catches
   up.
4. Press the knob: the servo glides back to 90°.
5. If the screen is blank or faint, turn the contrast knob on Board A's
   breadboard until the letters are sharp.

Now test your predictions. Turn the knob quickly from 90° to 180°. In
half a second, Board A can send five or six messages, far fewer than the
18 clicks, so the servo gets every third or fourth angle and glides from
one to the next. The bottom row doesn't change while you turn, because
Board B keeps quiet while the servo moves. About a third of a second
after the last angle arrives, the servo stops and the row says
`Servo  180°`.

Then switch off Board B's power module and turn the knob. The servo
can't move, but the bottom row still catches up: Board B reports where
it sent the servo, and has no way to know it didn't get there. Switch
the module back on, and the servo jumps to the angle it was sent.

## If it doesn't work

| What you see | Try this |
|---|---|
| `Not connected` stays on the screen, and Board B's yellow LED stays dark | Check Board A runs **Dial** and Board B runs **Servo**. Then upload Lesson 43's sketches to both boards: if their yellow LEDs light there, the modems are fine, and the trouble is in this lesson's wiring or sketches. |
| The screen shows the angles, but the servo never moves | Is the power module on, with its bottom jumper on 5V? Check the servo's plug: brown to B-54, red to B+53, orange to pin 44. |
| The Mega resets, or the USB disconnects, when the servo moves | The servo is getting power from the Mega. Its red wire must go to the bottom + rail, B+53, fed by the power module. |
| The servo turns the opposite way to the knob | Nothing is wrong. To swap it, change `knob.turned () * step` to `-knob.turned () * step` in **Dial**. |
| One click moves 10°, or it takes two clicks to move 5° | Your encoder makes a different number of steps per click. Try `adk::RotaryEncoder knob {18, 19, 2};`, as in Lesson 29. |
| It buzzes at one end of its travel | It's pushing against its end stop. Use `adk::Servo servo {44, 600, 2300};` to narrow the pulses a little. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. A row of blocks means the screen has power but isn't hearing the Mega: check pins 31 to 36. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Turn the knob from 90° to 95°, and Board A's bridge sends one line:

    ```text
    @angle=95
    ```

    Board B's bridge reads it, and in that pass of `loop ()`
    `bridge.changed ("angle")` is true, so the servo starts a 300 ms
    glide. When the glide ends, `at` changes from 90 to 95, and Board B's
    bridge sends `@at=95` back. Each message takes about 0.05 s on the
    air.

    Every two seconds, whether anything changed or not, Board A sends
    `@angle=95` and Board B sends `@at=95`. So if Board B restarts, it
    has the angle again within two seconds; and if one of the messages
    is lost, the next brings it.

## Make it yours

1. **A failsafe.** Radio-controlled models have one: when the signal is
   lost, they go somewhere safe. When Board B stops hearing Board A,
   glide the servo slowly back to 90° with
   `servo.moveTo (90, 1000)` while `!bridge.isConnected ()`.
2. **A gauge across the house.** Take the knob away, put the thermistor
   from Lesson 14 on Board A's A2 at its home in column 40, and share the
   temperature instead of the angle. On Board B, turn the needle to it on
   a dial from 15 °C to 35 °C, as Lesson 17 suggested.
3. **Coarse and fine.** Make a press of the knob switch between steps of
   1° and 10°, instead of centring the servo, and show the step on the
   screen's top row. How many clicks is it from end to end in each?
4. **How strong.** Show `radio.signal ()` at the end of the bottom row,
   in dBm, as Lesson 40 did, and walk Board B away.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. The readings are on
Board B, with both boards running. Keep fingers clear of the horn, and
take care on the rails: a probe tip touching the + and − rails together
would short the power module.

!!! question "Predict"
    Pin 27 lights the yellow LED while Board A can be heard. Unplug Board
    A while the meter is on pin 27: how long before the reading falls to
    0 V?

<!-- measure B -->

What the numbers tell you:

- **The servo's 5 V** comes from the power module's own regulator. Switch
  the module off and it falls to 0, and the servo goes limp, though
  Board B and its modem carry on.
- **Pin 27** reads about 5 V while the yellow LED is lit. Unplug Board A,
  and it falls to 0 V three to five seconds later: Board A's last message
  came at most two seconds before, and Board B waits five seconds from
  the last message it heard.
