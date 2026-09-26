---
lesson: 45
promise: Aim a fan in another room with a joystick, switch it on and off by radio, and read on your screen how close its sensor sees someone.
time: 2 hours
level: 3
parts:
  - "Board A: Lesson 44's Board A, with its screen and LoRa modem"
  - "Board A: joystick module and 5 female-to-male jumper wires"
  - "Board B: Lesson 44's Board B, with its power module, LoRa modem and servo"
  - "Board B: HC-SR04 ultrasonic sensor, L293D motor driver chip, and the DC motor with its fan blade"
  - "Board B: 4 female-to-male jumper wires and 9 jumper wires"
  - "Sticky tape or putty, and a strip of card, for the turret"
ideas:
  - A project split between two boards
  - Steering by how far a stick is pushed
  - Staying safe when the link is lost
  - Sharing a sensor's readings sparingly
---

## What you'll build

<!-- closeup A -->

A fan on a turret, steered from another room. Board B carries the
turret from Lesson 21: the servo, with the fan and the ultrasonic sensor
taped to its horn. Board A is the remote control. Push its joystick left or right
and the turret turns, faster the further you push; let go and it stays
where it points. Press the stick, and the fan starts blowing; press it
again to stop it. Board A's screen shows where the turret points,
whether the fan is on, and how far away the sensor sees something, with
**CLOSE!** when someone is nearer than 30 cm.

It brings together the bridge from Lesson 43, the remote servo from
Lesson 44 and the follow-me fan from Lesson 21. As before, send on
915 MHz only where it's allowed: see [Radios](../../safety.md#radios).

## The idea

**Two halves of one machine.** Lesson 21's fan decided everything for
itself. Here you decide where it points and whether it blows, with
Board A's joystick, and Board B does as it's told. Three names cross the
bridge:

| Name | Shared by | Means |
|---|---|---|
| `aim` | Board A | The angle the turret should point at, from 0 to 180 |
| `fan` | Board A | 1 while the fan should blow, 0 while it shouldn't |
| `dist` | Board B | How far away the sensor sees something, in cm: 400 for nothing within about 4 m |

A few things Board B still decides for itself, because they can't wait
for a radio: it stops the fan when someone is closer than 15 cm, as
Lesson 21's did, and while the turret turns, and whenever it can't hear
Board A.

**Steering by how far the stick is pushed.** Lesson 44's knob set the
angle a click at a time. A joystick springs back to the middle when you
let go, so if the stick's position were the angle, the turret would swing
back to 90° every time you let go of it. Instead, the stick sets how fast
the aim changes. Twenty times a second, Board A moves the aim by how far
the stick is pushed, divided by 25, in whole degrees:

<p class="formula">aim = aim − <span class="fraction"><span>x</span><span>25</span></span></p>

`stick.x ()` runs from −100 to 100, so pushed all the way the aim moves
4° a step, 80° a second. Pushed less than a quarter of the way, x ÷ 25
is less than 1, which counts as 0 in whole numbers: a gentle nudge does
nothing, and the turret keeps still.

**Staying safe when the link is lost.** Lesson 43 showed that a bridge
keeps the last value it heard, even after the other board has gone. For
a fan, that could mean blowing forever with nobody at the controls. So
Board B only lets the fan blow while `bridge.isConnected ()`: a few
seconds after Board A goes quiet, the fan stops on its own.
Radio-controlled models do the same, and call it a **failsafe**.

**Sharing a reading sparingly.** The sensor measures about sixteen times
a second, and each reading is a centimeter or so from the last. Shared
straight away, it would keep the air busy with ten messages a second,
and the stick's messages would often collide with them. So Board B
shares the distance twice a second: plenty for you to read, and it
leaves most of the air to the stick.

**One supply, two motors.** As in Lesson 21, the servo and the fan share
the power module, so the fan rests while the turret turns. Each new aim
starts a 0.3 s countdown, and the fan only blows once it has run out.

!!! question "Predict"
    With the fan blowing, you push the stick hard over for a second and
    let go. What does the fan do while the turret turns, and after? And
    if Board A's USB cable is pulled out while the fan blows, what will
    the fan do, and when?

## How the turret decides

On every pass of `loop ()`, Board B lets the fan blow only if all four
of these are true:

| Check | Why |
|---|---|
| Board A has been heard in the last five seconds | A failsafe: nobody at the controls, no fan |
| Board A's `fan` is 1 | You asked for it |
| Nothing is closer than 15 cm | Nobody wants a fan blade in their face |
| No new aim has arrived for 0.3 s | The servo and the motor never pull hard on the power module together |

## Build it

!!! warning "Unplug first"
    Unplug each board's USB cable, and Board B's power module adapter,
    before you wire. Keep fingers, hair and faces clear of the fan blade
    whenever Board B's power module is on.

!!! danger "The motors' power"
    Board B's power module keeps its **top** jumper **off** and its
    **bottom** jumper on **5V**. The servo and the motor take their power
    from the bottom rails, never from the Mega; the L293D's logic and the
    sensor run on the Mega's own 5 V, from the top rails and the inner 5V
    pin. The modem stays on the Mega's 3.3V pin.

Both modems stay at their bridge homes. On each board the modem, its
divider and its four wires stay just as they are.

### Board A

Keep the screen and the modem from Lesson 44, with all their wires. Take
out the rotary encoder and its five wires. The joystick lies at its home
below the Mega, under A3 and A4, as in Lesson 26: VRx to A3, VRy to A4,
its switch, SW, to pin 22, its +5V to the 5V pin on the power header and
its GND to the inner GND pin at the end of the long header.

<!-- bench A -->

<!-- steps A -->

### Board B

Keep the power module, the modem and the servo from Lesson 44. Take out
the yellow LED, with its resistor and wires: the L293D needs its column,
so the Mega's own **L** LED, on pin 13, shows the link now, and needs no
wire at all.

The ultrasonic sensor needs pins 14 and 15, its homes, so the modem
moves to the Mega's other spare serial port, **Serial2**, on pins 16
(TX2) and 17 (RX2). Take out the wires from pins 14 and 15 to j46 and
j44, and put wires from pins 16 and 17 into those same holes: 16 into
j46, the divider's top, and 17 into j44, beside the modem's TXD.

Then build Lesson 21's turret, just as it was there: the sensor above
the Mega, its VCC on the inner 5V pin at the top of the long header and
its GND in T-5; the L293D across the middle gap from column 12, its
logic on the top rails and the motor's supply on the bottom ones; and
the motor above the board, its leads down into j14 and j17. The wires
from pins 16 and 17 go up past the sensor's left end and over the
motor, to reach the modem's holes from above.

<!-- bench B -->

<!-- steps B -->

??? info "Making the turret"
    As in Lesson 21: tape a strip of card across the servo's longest
    horn, then stick the sensor on it facing forwards and the motor
    beside it, its fan blade pointing the same way and clear of
    everything when it spins. Stand the servo on the desk with putty or
    tape so it can't walk, and leave the wires slack, so the turret can
    swing from end to end without tugging on them.

When you are done, these are the connections each board makes. Board A:

<!-- connections A -->

Board B:

<!-- connections B -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk →
lessons/045-remote-turret → Joystick**, for Board A:

<!-- sketch A -->

Read it from the top:

- The modem, the bridge and the screen are Lesson 44's. `stick` is the
  joystick on A3 and A4, and `press` its switch on 22, as in Lesson 26.
  `steer` beats twenty times a second, and `refresh` redraws the screen
  four times a second.
- `nothing` is 400 cm, what Board B shares when no echo comes back, and
  `closeBy` is where the warning starts, 30 cm.
- `aim` and `fan` are what Board A asks for: the turret in the middle,
  and the fan off.
- At each beat of `steer`, `aim - stick.x () / 25` moves the aim, and
  `constrain` keeps it between 0 and 180. The minus makes pushing right
  turn the turret right on most servos; if yours turns the other way,
  make it a plus.
- `fan = !fan` swaps the fan between on and off at each press, as
  `editing = !editing` did in Lesson 29.
- Both are shared on every pass of `loop ()`; only a change goes out.
- `showTurret ()` puts the aim and the fan on the top row. On the bottom
  row it shows `bridge.value ("dist")`, what Board B's sensor sees, with
  `CLOSE!` in front when it's nearer than `closeBy`; or `Nothing ahead`
  for no echo; or `Not connected` while Board B can't be heard.
  `cm < closeBy ? "CLOSE! " : "Ahead  "` picks one of two texts, as
  `modemA.ok () ? ... : ...` did in Lesson 40.

Then choose **lessons/045-remote-turret → Turret**, for Board B:

<!-- sketch B -->

Read it from the top:

- `adk::LoraModem radio {Serial2, 2, ...}` is the modem on `Serial2`,
  pins 16 and 17, still address 2 with partner 1.
- `turret`, `sensor` and `fan` are Lesson 21's. `connected` is the L
  LED, on pin 13 on the Mega itself.
- `settling` is an `adk::Timer`, as in Lesson 3: each new aim starts it
  for 300 ms, and the fan rests until it runs out. `report` beats twice a
  second.
- When a new aim arrives, the turret glides there over 100 ms. While you
  hold the stick over, a new aim arrives about every tenth of a second,
  so the glides join into one smooth turn.
- Each new reading from the sensor updates `distance`; no echo counts as
  `nothing`, 400 cm, as in Lesson 21.
- At each beat of `report`, `bridge.share ("dist", distance)` shares the
  latest distance. Between beats the shared value stays the same, so
  nothing is sent.
- `shouldBlow ()` is the table above, in one line: `&&` means *and*, so
  it's true only when all four checks are. `fan.speed ()` gets 200 when
  it is, and 0 when it isn't, on every pass, as in Lesson 20.

## Upload it

1. Plug Board B into the computer, choose its port in **Tools → Port**,
   and upload **Turret**. Plug in its power module's adapter and press
   the module's button, so its LED lights. The **L** LED stays dark, the
   servo lies limp, and the fan stays still.
2. Plug in Board A, choose its port, and upload **Joystick**. Leave the
   stick alone while it starts: the sketch takes where it rests as the
   middle. The top row says `Aim 90°  Fan off`.
3. Board B's **L** LED lights, and the turret turns to 90°. Within a
   couple of seconds the bottom row shows what the sensor sees, such as
   `Ahead  85 cm`.
4. Push the stick to one side. The turret turns, faster the further you
   push; let go, and it stays.
5. Press the stick. The top row says `Fan on`, and the fan spins up. Step
   in front of it and come closer: under 30 cm the bottom row says
   `CLOSE!`, and under 15 cm the fan stops. Step back, and it starts
   again.
6. Press the stick again to stop the fan.

Now test your predictions. With the fan blowing, push the stick hard
over: the fan stops while the turret turns, and starts again a third of
a second after you let go. Then pull out Board A's USB cable while the
fan blows. Three to five seconds later, Board B's **L** LED goes out and
the fan stops: the failsafe. Plug Board A back in. Its sketch starts
again with the fan off and the aim at 90°, so the turret swings back to
the middle, and the fan stays still until you press the stick.

## If it doesn't work

| What you see | Try this |
|---|---|
| `Not connected` stays on the screen, and the **L** LED stays dark | Check Board A runs **Joystick** and Board B runs **Turret**. Check Board B's modem wires now come from pins 16 and 17: 16 into j46, 17 into j44. |
| The turret never moves | Is the power module on, with its bottom jumper on 5V? Check the servo's plug: brown to B-54, red to B+53, orange to pin 44. |
| The turret turns the wrong way | Change `aim - stick.x () / 25` to `aim + stick.x () / 25` in **Joystick**. |
| The turret creeps when nobody touches the stick | The stick moved while the sketch started. Press Board A's reset button with your hands off the stick. |
| `Fan on`, but the fan never spins | Is anything closer than 15 cm, or the sensor facing a wire or the desk? Then check the L293D as in Lesson 20: pin 4 in j19, 8 in j13, 9 in j18, the motor's leads in j14 and j17, j12 to T+12, a15 to the − rail and a19 to B+19. |
| The bottom row always says `Nothing ahead`, or jumps about | Check the sensor: Trig to 14, Echo to 15, VCC to the inner 5V pin, GND to T-5, and that it faces forwards, clear of the fan. |
| The Mega resets, or the servo jerks, when the fan starts | Check the servo's red wire and a19 go to the bottom + rail, fed by the power module, never the Mega's 5V. |
| The **L** LED blinks long and short flashes, and nothing works | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    While you push the stick, Board A's bridge sends a line like
    `@aim=94` every tenth of a second, and every two seconds everything,
    `@aim=94 fan=1`. Board B sends `@dist=85` when the distance it
    shares changes, twice a second at most, and everything every two
    seconds too.

    The sensor holds Board B's `loop ()` up for as long as 25 ms each
    time it pings, as in Lesson 19. A message that arrives meanwhile
    waits in the Mega's serial port, which holds 64 letters, more than
    any of this project's messages, and the bridge reads it on the next
    pass.

    The servo's pulses come from Timer 5 and the fan's speed from pin 4's
    PWM, both made in hardware, so neither stumbles while the sensor
    waits for its echo.

## Make it yours

1. **Harder the closer.** Set the fan's speed from the distance, as
   Lesson 21 did: `map (distance, tooClose, 80, 255, 110)`, and full
   speed beyond 80 cm.
2. **A beep.** Put the active buzzer on Board A at its home beside the
   screen, in column 51 on pin 12, and beep while the screen says
   `CLOSE!`.
3. **Look around.** Push the stick up to make Board B sweep, as the
   turret did in Lesson 21, and turn to the nearest thing it finds. A
   push is an event, so share a count of pushes, as Lesson 43's
   *Switch it* challenge did, and sweep whenever it changes.
4. **Where it has got to.** Share `at` from Board B, as Lesson 44
   did, and show it on the screen beside the aim. How far does it lag
   behind while you steer?

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Never use the **10A**
jack here: it joins the two probes, and across the power module's rails
that is a short circuit. The readings are on Board B's pin 4, the L293D's
enable pin, which sets the fan's speed. The fan blows while you take
them, so work from beside the breadboard, and keep your fingers, the
probes and their leads out of the blade's reach.

!!! question "Predict"
    The fan's speed is 200 out of 255. What will pin 4 read while it
    blows? What will it read while you push the stick, and five seconds
    after you pull out Board A's cable?

<!-- measure B -->

What the numbers tell you:

- **With the fan on** and the turret still, pin 4 reads about 3.9 V: PWM
  switches it on for 200 parts of every 255, and 200 ÷ 255 × 5 V ≈ 3.9 V,
  as in Lesson 20.
- **While you push the stick** it drops to 0, and comes back a third of a
  second after you let go.
- **With Board A unplugged** it falls to 0 three to five seconds later,
  and stays there: the failsafe, measured.
