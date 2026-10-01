---
lesson: 3
promise: Build a two-player reflex game, and find out who in your house is fastest.
time: 1½ hours
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 push buttons
  - Red, yellow and green LEDs
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - Active buzzer (the sealed one, often with a sticker on top)
  - S8050 transistor, 1 kΩ resistor, 10 kΩ resistor and 1N4007 diode
  - 17 jumper wires
ideas:
  - Random numbers, and a random seed
  - Measuring time with a timer and a stopwatch
  - A game as a set of states
  - The active buzzer, switched by a transistor
laws:
  - {law: voltage-law, section: measure-it, for: "Shows the buzzer taking nearly all the 5 V rail"}
  - {law: budgets, section: build-it, for: "Keeps the buzzer's 30 mA off a 20 mA pin"}
  - {law: flyback, section: build-it, for: "Puts a diode across the buzzer for its switch-off spike"}
  - {law: forward-voltage, section: measure-it, for: "Shows the yellow LED keeping about 2 V"}
  - {law: transistor-switch, section: build-it, for: "Switches the active buzzer through an S8050 from pin 12"}
  - {law: pull, section: build-it, for: "Holds the transistor off with 10 kΩ at start-up"}
---

## What you'll build

<!-- closeup -->

A duel for two. Each player rests a finger on a button: red on the left,
green on the right. The yellow light blinks, **one of you** presses to
start, and everything goes dark. You wait. And wait. Then, at a moment
nobody can guess, the yellow light snaps on and the buzzer beeps. First to
press wins, their light flashes, and the Serial Monitor tells you exactly
how fast they were: *Red wins in 231 ms!* Jump the gun, and you lose on the
spot.

## The idea

Two new tools make this game work.

**Random numbers.** `random (2000, 5000)` picks a whole number from 2000 up
to 4999, which the game uses as a wait in milliseconds, between 2 and 5
seconds. But a computer can't really roll dice. `random ()` follows a fixed
recipe that makes the same list of numbers every time the Mega starts, unless
you give it a different starting point, called the **seed**. The sketch reads
pin A7 for its seed. Nothing is connected to A7, so it floats (as a button's
pin would, without its pull-up), and its reading wanders with stray
electricity: a different seed, and different waits, nearly every time the
Mega starts.

**Time.** `millis ()` is the number of milliseconds since the Mega started.
Read it once when the light comes on and again when a button is pressed, and
the difference is the reaction time:

<p class="formula">reaction = 12 631 ms − 12 400 ms = 231 ms</p>

ADK does that sum for you in two parts. An `adk::Timer` counts down, like a
kitchen timer: the game sets one for the random wait. An `adk::Stopwatch`
counts up: the game starts one when the light comes on, and reads it when
somebody presses.

!!! question "Predict"
    Most people react faster to a sound than to a light. Before you play,
    guess your reaction time in milliseconds. Is it nearer 100, 250, or 500?

## How the game works

The game is always in exactly one **state**, and each state knows which
events it cares about. The sketch keeps the state in a variable and, on every
pass of `loop ()`, only does what the current state allows.

| State | What you see | What moves it on |
|---|---|---|
| **Waiting** | The yellow light blinks slowly, or, after a round, the winner's light flashes | One player's press: *Ready* |
| **Ready** | All dark, for a random 2 to 5 seconds | Time's up: *Go*. A press: a **false start**, the other player wins, and back to *Waiting* |
| **Go** | Yellow on, and a beep | The first press wins: back to *Waiting* |

A false start is only possible because the game knows it is in *Ready*: the
same press in *Go* would win. So only one of you presses to start: after
that, any press before the light is a false start.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Every LED needs its
    220 Ω resistor. Make sure you have the **active** buzzer, the sealed one:
    use the transistor driver shown below, even for short beeps. The active
    buzzer stands across the middle gap: its **+** leg, the longer one, with a **+** on its top, goes in f33,
    above the gap.

The game keeps Lesson 2's buttons and lights, adds a green light, and gets
a voice: the **active buzzer**. It has a tiny oscillator circuit inside, so
it makes its own tone, a shrill note a little over 2000 vibrations a
second, whenever it has power. It can need up to 30 mA, above the Mega
pin’s recommended 20 mA, so a **transistor** switches it: pin 12 supplies a
small control current through a 1 kΩ resistor, and the S8050 switches the
buzzer’s current from the 5 V rail to GND. A 10 kΩ resistor keeps it off
while the Mega starts, and a diode across the buzzer catches a voltage
spike when it switches off. In [Lesson 5](../005-melody-maker/index.md)
you'll meet its cousin, the passive buzzer, which can play any note but
needs a resistor and more help from the Mega.

<!-- bench -->

<!-- steps -->

??? info "The buzzer across the gap"
    The buzzer's legs are 0.3 inch apart, exactly as far as row f is from
    row e across the middle gap, so it stands across the gap like the
    buttons do. Its **+** leg takes 5 V through h33. Its other leg reaches
    the transistor’s collector through a33. Pin 12 reaches a32, then the
    1 kΩ resistor leads to the transistor’s base in column 30. Its holes
    are close together: if its body cannot lie flat, stand it upright and
    bend one lead back alongside it, keeping bare leads from touching.

    Before inserting the transistor, check that its marking is **S8050**
    and its own supplier’s pin diagram says **E–B–C**, left to right with
    the marked flat face toward you and the legs down. The
    [onsemi SS8050 drawing](https://www.onsemi.com/download/data-sheet/pdf/ss8050-d.pdf)
    shows that order. Do not substitute the kit’s PN2222 by its shape.
    Spread the legs into a29 (emitter), a30 (base), a31 (collector).
    If the marking or order differs, get the correct pin diagram first.
    The **1N4007** diode’s silver band goes toward column 36, which
    connects to buzzer **+**; its unbanded end is in column 33. Check its
    marking too: a band identifies the cathode, not the part’s rating.
    The [Vishay 1N4007 datasheet](https://www.vishay.com/docs/88503/1n4001.pdf)
    identifies the banded end and rates the diode for this use.

    To tell the two buzzers in the kit apart, look underneath. The active
    buzzer is sealed with black plastic, and usually has a paper sticker on
    top saying *Remove seal after washing*: the seal kept water out while the
    factory washed the board. Peel it off for a louder beep, or leave it on
    for a quieter one. The passive buzzer is open underneath, and you can see
    a green circuit board.

When you are done, these are the connections your circuit makes:

<!-- connections -->

### Test the beep first

Before the game, check the buzzer and its transistor on their own: a
buzzer that won't beep is much easier to puzzle out now than in the middle
of a game. Choose **File → New**, and replace everything in the new window
with these lines:

```cpp
// Lesson 03, first: test the beep.

#include <Adk.h>

adk::Buzzer buzzer {12};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    buzzer.beep (200);
    adk::wait (1000);
}
```

!!! question "Predict"
    `buzzer.beep (200)` sounds a 200 ms beep. Does the sketch wait for the
    beep to end before `adk::wait (1000)` starts, or start the beep and
    carry straight on? If it waits, each pass of `loop ()` takes 1.2
    seconds; if not, one second. So in 12 seconds, will you count 10 beeps
    or 12?

Upload it and count the beeps against a clock: about 12 in 12 seconds,
each short with a longer quiet after it. `beep ()` starts the beep and carries straight on,
and ADK switches the buzzer off by itself 200 ms later, partway through the
wait. The game counts on that: at *Go* it starts the beep and the stopwatch
in the same instant.

If you hear nothing, or only a faint click, the beep's rows in
[If it doesn't work](#if-it-doesnt-work) say what to check. Fix it now,
while the buzzer is the only thing running.

## Code it

Now open the game, **File → Examples → Adk → lessons → 003-reaction-duel**:

<!-- sketch -->

Try the game in two passes. First replace `random (2000, 5000)` in
`getReady ()` with `3000`, and play alone using the red button. Follow the
three states: start, wait, press at the signal. Deliberately press early once.
Then put the random wait back and invite a second player. The extra player
changes who wins; the three states stay the same.

What's new:

- `enum class State { Waiting, Ready, Go };` makes a new kind of value with
  three names, one for each state, and `State state` is a variable that
  holds one of them. `state = State::Ready;` moves the game on, and says
  what it means, which a number never would.
- `switch (state)` in `redPressed ()` jumps to the `case` for the current
  state and runs its lines, down to its `break`, which ends the `switch`.
  So the same press starts a round, loses it or wins it, depending on the
  state: those three cases are the whole game.
- `greenPressed ()` is `redPressed ()` again with the colors swapped.
  Writing it out twice keeps each player's lines plain to read. In
  [Lesson 5](../005-melody-maker/index.md) you'll meet a `struct`, which
  bundles parts that belong together, such as a button and a light, so
  that one piece of code can serve them all.
- `falseStart ()` and `endRound ()` hold the lines both players share, so
  they are written once.
- In `loop ()`, **`else if`** asks about the green button only when the
  answer about the red one was no, so each pass handles one press at most.
  *How it works* below says why that matters.
- `adk::Timer suspense;` counts down. `suspense.start (random (2000, 5000))`
  sets it going, and `suspense.expired ()` is true for the one update in
  which it runs out, just as a button's `wasPressed ()` is true once per
  press. `adk::Stopwatch reaction;` counts up: `reaction.restart ()` sets
  it to zero and running as the light comes on, and `reaction.elapsed ()`
  reads it. Neither stops the sketch to wait, so a false start is still
  noticed.
- `randomSeed (analogRead (A7));` plants the seed, once, in `setup ()`.
- `adk::Buzzer buzzer {12};` and `buzzer.beep (200);` are the beep you
  tested: it starts, and the sketch carries straight on.

## Upload it

Upload the sketch and open the Serial Monitor at 9600 baud. It says
*Reaction Duel! Press a button to start.*, and the yellow LED blinks once a
second.

1. Press either button. All the lights go out and the Serial Monitor says
   *Get ready...*
2. Between 2 and 5 seconds later, the yellow LED lights and the buzzer
   beeps.
3. Press! The winner's LED flashes quickly, and the Serial Monitor prints the
   time, such as *Green wins in 247 ms!* That time includes the 20 ms ADK
   waits for a button to settle (see *How it works*).
4. Now try pressing before the yellow light. The buzzer gives a long, cross
   buzz, the other player's light flashes, and the Serial Monitor says who
   pressed too soon.

Press again for another round. How close was your guess? Most people's
times land somewhere around 200 to 300 ms. To find out whether you really
are faster to a sound, try the first challenge below.

## If it doesn't work

| What you see | Try this |
|---|---|
| The yellow LED doesn't blink at the start | Turn it round: its long leg goes in b12. Check its resistor runs from g12, across the gap, to e12. |
| A button never starts a round | Push it firmly into the board, all four legs in. Check its black wire goes from row a (a4 or a10) to the − rail. |
| The loser's light flashes, not the winner's | The LED wires may be swapped: pin 26 goes to j6 (red, on the left), pin 28 to j18 (green, on the right). |
| No beep | Upload the beep test from *Test the beep first* to try the buzzer on its own. The buzzer may be the wrong way round: its **+** leg goes in f33, above the gap. Check h33 reaches the top + rail, pin 12 reaches a32, and the S8050’s emitter reaches GND. Check its E–B–C order and the diode’s band at column 36. |
| Only a faint click instead of a beep | That is the passive buzzer. Unplug the USB cable at once and swap it for the sealed, active one. |
| The first wait is the same every time the Mega starts | Make sure nothing is plugged into A7: the seed only changes if the pin is left floating. |

??? note "How it works"
    `buzzer.beep (200)` switches the buzzer on at once and remembers the
    time. On each `adk::update ()` the buzzer checks whether 200 ms have
    passed, and switches itself off. Blinking LEDs work the same way, which
    is why the whole game runs in one quick loop without ever stopping to
    wait for something to finish.

    The times include the 20 ms ADK waits for a button to settle, and a
    little more if its contacts bounce, since each bounce starts the 20 ms
    again. So your real reaction is about 20 ms quicker than the number on
    screen. Both players get the same wait, so the duel stays fair.

    `endRound ()` ends with `adk::wait (1000)`, which gives the loser a
    second to finish their too-late press. A press during `adk::wait ()`
    still updates its button, but no `loop ()` is looking, so it is let go
    by instead of starting the next round. Only a press in the wait's very
    last update is still news when `loop ()` carries on, and the `else`
    keeps `loop ()` from asking about it: the next `adk::update ()` forgets
    it.

    `millis ()` counts up for about 49.7 days and then starts again from 0.
    The timer and the stopwatch only ever take the difference of two
    readings, which still gives the right answer across that wrap.

## Make it yours

1. **Eyes or ears?** Delete `buzzer.beep (200);` from `go ()` and play a few
   rounds by light alone. Then put it back, delete `yellow.on ();` and play
   with your eyes shut. Which way are you faster, and by how much?
2. **Record time.** Keep the fastest time since power-up in a variable, and
   print *New record!* whenever somebody beats it.
3. **Best of five.** Add `int redWins = 0;` and `int greenWins = 0;` beside
   `state`, and count each player's wins. The first to three wins the
   match: make their light blink slowly, and start a new match on the next
   press.
4. **A fair tie.** Both players could, just possibly, press in the same
   update. Right now red would win, because `loop ()` asks red's button
   first. In the *Go* state, check whether both were pressed, and call it a
   draw with both lights flashing.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red in **V**. Never move the red lead to the **A**
jack for these: set for current, the meter is just a wire, and would short
out whatever you put it across.

A 200 ms beep is over before the meter settles, so change
`buzzer.beep (200);` in `go ()` to `buzzer.beep (5000);` and upload again.
Now at *Go* the buzzer sounds for five seconds, and the yellow light stays
on until somebody presses, so start a round and don’t press. The
transistor carries the buzzer’s current throughout the beep.
Put 200 back when you have finished.

!!! question "Predict"
    The yellow LED keeps about 2 V for itself and leaves the other 3 V to its
    resistor. The buzzer takes its current through a transistor from the
    5 V rail. Will it get most of that voltage, or only half?

<!-- measure -->

What the numbers tell you:

- **Across the yellow LED** is about 2 V, like the red LED in Lesson 1: the
  LED shares the pin's 5 V with its resistor, and the resistor sets the
  current.
- **Across the buzzer** is nearly the whole 5 V, about 4.8 V: the
  transistor takes only a small share when switched on. The buzzer’s
  current comes from the rail; pin 12 supplies only the base’s few milliamps.
  Supply voltage and the particular transistor affect the exact reading.
- The number holds steady while you hear a note of over 2000 vibrations a
  second. The pin is simply on, like an LED's; the buzzer makes the
  vibration inside itself. The passive buzzer in
  [Lesson 5](../005-melody-maker/index.md) is different, and the meter shows
  it.

## Check yourself

1. Why does the sketch read the floating pin A7 for its seed, and what would
   happen if you plugged something into A7?
2. A press during the dark wait loses, but the same press after the yellow
   light wins. What in the sketch tells the two apart?
3. Pin 12 doesn't power the active buzzer itself. Where does the buzzer's
   current come from, and what does pin 12 do?

??? note "Answers"
    1. `random ()` makes the same list of numbers every time the Mega starts
       unless it gets a different seed, and A7's floating reading wanders,
       so the seed changes nearly every time the Mega starts. With something
       plugged in, the reading would hold still, and every time the Mega
       started, the rounds would bring back the same waits in the same
       order.
    2. The game's state. The sketch only does what the current state
       allows: in *Ready* a press is a false start, and in *Go* the first
       press wins.
    3. Its current comes from the 5 V rail and flows through the transistor
       to GND. Pin 12 only sends a small control current to the
       transistor's base, because the buzzer can need more than a pin
       should give.

!!! tip "Go deeper"
    The electricity course tries the buzzer driver's parts one at a time:
    [E09: One-way diode](../064-one-way-diode/index.md),
    [E10: Control with a transistor](../065-control-with-a-transistor/index.md)
    and [E12: Give a coil a safe path](../067-coil-diode/index.md). They need
    only the kit's parts, and E12 a multimeter too.
