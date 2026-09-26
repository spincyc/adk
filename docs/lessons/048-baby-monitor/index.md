---
lesson: 48
promise: Build a baby monitor that graphs every sound in the nursery, tells you when the baby cried, and raises the alarm for a leak, or for silence.
time: 2 hours
level: 3
parts:
  - "Board A, the nursery: Lesson 47's Board A, with its LoRa modem, divider and green LED"
  - "Board A: the sound sensor module and the water level sensor, both from the Mega kit, and the photoresistor and a 10 kΩ resistor, as in Lesson 46"
  - "Board A: 3 jumper wires, 6 female-to-male jumper wires, and a small cup of water"
  - "Board B, the parent: Lesson 47's Board B, with its LoRa modem, screen, clock module and IR receiver, and the kit's remote"
  - "Board B: the LED matrix, the passive buzzer and a 220 Ω resistor, 1 jumper wire and 5 female-to-male jumper wires"
ideas:
  - A microphone's swing, as a loudness
  - The loudest moment in each half second
  - A water sensor powered only while it reads
  - Alarms that fail loud, because no news isn't good news
---

## What you'll build

<!-- closeup B -->

A baby monitor, for a nursery across the house. Board A, in the nursery,
listens with the kit's sound sensor, feels the floor with its water
sensor, and watches the light. Board B, with the parent, draws the sound
on the LED matrix as a moving bar graph, half a second a column, so you
can see the room at a glance. Its screen says whether the nursery is
quiet, lit and dry, and when the baby last cried. A cry brings a soft
chime. Water on the floor sounds an alarm until you hush it with the
remote. And if the nursery goes silent on the radio, the parent's board
beeps and says so: a monitor that quietly stops working is worse than
none.

This project puts Lessons 46 and 47 together, and adds the two sensors in
the kit that no lesson has used yet.

## The idea

**Hearing how loud.** The sound sensor's microphone turns sound into a
voltage that wobbles above and below the middle of its range, faster for a
high note, further for a loud one. In a quiet room it barely moves.
`adk::SoundSensor` reads the pin again and again, and every 50 ms says how
far it swung:

<p class="formula">level = highest reading − lowest reading, over 50 ms</p>

That's near 0 in a quiet room and hundreds for a clap nearby. It says how
loud, not what: no voice crosses the bridge, only a number.

**The loudest in each half second.** Ten levels come every half second.
Sending every one would keep the radio busy ten times a second, as
Lesson 46 found. Sending an average would hide a short cry among quiet
moments.
So the nursery keeps the loudest level of each half second and sends that:
nothing loud is ever lost between messages.

**Feeling for water.** The water sensor is a comb of copper traces, every
other one joined to its **+**. Dry, nothing joins them. Water between the
traces lets a little current through, and a transistor on the board turns
that into a voltage on **S**: about 0 when dry, and higher the more of the
traces are wet. But current through wet copper slowly eats the copper
away, and a sensor left powered in water all day soon corrodes. So its
**+** isn't on a rail, but on a pin, A7, which the sketch switches on for
10 ms every two seconds, long enough to read S, and then off again:

<p class="formula">10 ms ÷ 2000 ms = 1/200 of the time</p>

The sensor needs only a few milliamps, well within what a pin can give.

**Fail loud.** Lesson 46 showed stale news on the screen. A baby monitor
must do more: silence from the nursery could mean a sleeping baby or a
flat battery, and you can't tell which. So the parent's board treats "not
heard for five seconds" as news in itself: the screen says `No news!`, the
graph drops to nothing instead of showing the last sound forever, and it
beeps every five seconds until the nursery is heard again.

!!! question "Predict"
    Once it's all running, you'll dip the tip of the water sensor in a cup
    of water, and press POWER on the remote. Then you'll lift it out, dry
    it, and dip it again. What will the alarm do at each step? Write down
    your guesses.

## Build it

!!! warning "Unplug first, and keep water away from the boards"
    Unplug both boards before you wire. Stand the cup of water well away
    from both boards, and dip only the sensor's copper traces, never the
    part with its parts and pins. Neither board has a motor or a servo, so
    neither needs the power module.

### Board A, the nursery

Keep the modem, its divider and the green LED from Lesson 47, and take out
the five tripwires, the red LED and their wires. The light sensor's
divider comes back to its home in column 40, as in Lesson 46. The sound
sensor lies below the Mega where the PIR was, its **+** and **G** on the
power header's 5V and GND and its **AO** on A5; its **DO** stays
unconnected. The water sensor lies beside it: **S** to A6, **+** to A7, and
**−** to the power header's other GND. Their wires cross on the way to the
pins; that's fine.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B, the parent

Keep the modem, the screen, the clock module and the IR receiver from
Lesson 47. The passive buzzer takes the active buzzer's place in column 51,
on pin 10 now, with its 220 Ω resistor from a51 down to the − rail, as in
Lesson 33. The LED matrix lies at its home below the gap between the Mega
and the board, as in Lesson 25, but the screen's contrast knob already has
B-5, so the matrix's GND goes one hole nearer the Mega, into B-4. The
modem's 3.3 V wire runs below the matrix.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Open **File → Examples → Adk → lessons → 048-baby-monitor → Nursery** for
Board A:

<!-- sketch A -->

What's new:

- `adk::SoundSensor sound {A5};` measures the swing on A5, and
  `sound.measured ()` is true each time a new `sound.level ()` is ready,
  every 50 ms. `max (loudest, sound.level ())` keeps the bigger of the
  two, so after ten levels `loudest` is the loudest of them.
- Every half second the loudest level and the light go over the bridge,
  and `loudest` starts again from 0.
- `adk::DigitalOutput waterPower {A7};` is the plainest output ADK has:
  `write (true)` puts 5 V on the pin and `write (false)` 0 V. An analog
  pin can be an output like any other. `feel` ticks every two seconds: A7
  goes high to power the sensor, and `settle`, an `adk::Timer` as in
  Lesson 3, counts 10 ms. When it `expired ()`, the sketch reads the water
  on A6, shares the reading, and switches A7 off again.

Then **File → Examples → Adk → lessons → 048-baby-monitor → Parent** for Board B:

<!-- sketch B -->

What's new:

- `perDot`, `cryLevel`, `wetAbove` and `litAbove` are yours to tune: each
  sensor is a little different.
- `chime` and `alarm` are melodies for the passive buzzer, as in Lesson 6.
  `speaker.play (alarm)` on every pass keeps the alarm going: a melody that
  has finished plays again when asked again.
- `hushed` becomes `true` when POWER is pressed, and
  `hushed = hushed && wet;` makes it `false` again as soon as the floor is
  dry, so the next leak rings afresh.
- `listen ()` runs every half second. It moves each column of `bars` one
  place to the left, and puts the newest sound on the right: `level /
  perDot` dots, but never more than 8, which is what `min (..., 8L)`
  makes sure of (the `L` makes 8 a `long`, like `level`). Then two loops,
  one inside the other, light each column's dots from the bottom up:
  `y < bars[x]` is `true` for the lit ones.
- The parent draws a column every half second whether a new message came
  or not. The bridge sends only a change, so a nursery that stays at the
  same level sends nothing new, and `bridge.value ("sound")` is still the
  latest. With no news, though, `level` is 0: a silent nursery isn't
  allowed to look like a steady one.
- `loud && !crying` is true only for the half second a cry begins, so the
  chime sounds once and the clock is read once.
- `beat` ticks every five seconds, and while `!bridge.isConnected ()`, the
  buzzer beeps.

## Upload it

1. Upload **Nursery** to Board A and **Parent** to Board B. Until Board B
   hears Board A, it says `No news!` and beeps every five seconds.
2. Once they hear each other, the top row says something like
   `Quiet  Lit  Dry`, and the matrix fills with low bars from the right.
3. Talk near the sound sensor: the bars jump with your voice. Clap: a tall
   bar, and if it reaches `cryLevel`, a chime, and the bottom row says when,
   `Cried   02:14:07`. If a whisper fills the matrix, make `perDot` bigger;
   if a shout barely shows, make it smaller.
4. Cover the light sensor: the top row says `Dark`.
5. Now test your prediction. Dip the water sensor's traces in the cup: in
   up to two seconds, `WET!` and the alarm. Press POWER: the alarm stops,
   though the screen still says `WET!`. Lift the sensor out and dry it:
   `Dry`. Dip it again: the alarm rings again. Hushing lasts only until the
   floor is dry, so a second leak is never missed.
6. Unplug Board A. After five seconds, the screen says `No news!`, the
   graph runs down to nothing, and Board B beeps every five seconds.

## If it doesn't work

| What you see | Try this |
|---|---|
| `No news!` all the time | The boards don't hear each other: check each modem as in Lesson 43. |
| The bars never move | Check the sound sensor's AO goes to A5, + to 5V and G to GND on the power header. Clap right beside it. If they still don't move, make `perDot` smaller. |
| The matrix is full all the time | The room is louder than you think, or `perDot` is too small: make it bigger. |
| The matrix stays dark, even when the screen shows sound | Check pins 47, 48 and 49 go to DIN, CLK and CS, VCC to the inner 5V pin and GND to B-4. |
| The graph runs the wrong way, or upside down | The matrix is turned: turn it round, as Lesson 25 explains. |
| Never `WET!` | Check the water sensor's S goes to A6, + to A7, which powers it, and − to GND. Dip deeper, or make `wetAbove` smaller. |
| `WET!` when dry | Dry the traces well, or make `wetAbove` bigger. |
| No sound at all | Check the passive buzzer's + in f51, pin 10's wire in j51, and the 220 Ω from a51 to the − rail. |
| POWER doesn't hush | Aim the remote at the receiver, as in Lesson 47. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The nursery sends two kinds of message: every half second the sound
    and the light, and every two seconds the water, each only if it
    changed:

    ```text
    @sound=37 light=64
    @water=0
    ```

    A quiet room's level wobbles by a few, so most half seconds bring a
    message: two a second, each about a twentieth of a second on the air.

    The sound sensor reads A5 four times on every pass of `loop ()`, about
    half a millisecond, and keeps the highest and lowest readings. It hears
    best when `loop ()` comes round often, which is why nothing in either
    sketch ever waits: even the water sensor's 10 ms is a `Timer`, not a
    `delay ()`.

    The parent's own half-second beat and the nursery's aren't in step,
    so a bar can appear up to half a second late, or a level can show in
    two columns. A graph doesn't mind.

## Make it yours

1. **Crying, calmly.** A cry that wobbles around `cryLevel` chimes again and
   again. Give it Lesson 15's gap: start crying at `cryLevel`, but stop
   only below half of it.
2. **Cries as a count.** Move the cry decision to the nursery, and send a
   count of cries, as the door sent its tripwires in Lesson 47. Now a lost
   message can't lose a cry.
3. **Too warm.** Put the DHT11 back on the nursery's Board A, as in
   Lesson 46, and show the temperature on the parent's screen, in tenths.
4. **A night light.** When the parent's screen says `Dark`, share a
   setting back to the nursery, as the den did in Lesson 47, and light an
   LED there, so the baby isn't in the dark.
5. **Quiet hours.** Make POWER also silence the chime for ten minutes with
   an `adk::Timer`, while the graph and the leak alarm carry on.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Measure Board B while
the water sensor is in the cup.

!!! question "Predict"
    The passive buzzer plays by switching pin 10 between 5 V and 0 V, as
    in Lesson 5. What will the meter show while the alarm plays, and once
    POWER has hushed it?

<!-- measure B -->

What the numbers tell you:

- **While the alarm sounds**, pin 10 is high half of each wave, which
  would average 2.5 V, but each note is silent for its last eighth, so the
  meter settles a little lower, about 2.2 V.
- **Hushed**, the pin rests at 0 V, even though the nursery still says
  `WET!`: the choice to be quiet was made on Board B, not by the sensor.
