---
lesson: 3
title: Reaction Duel
arc: First light
promise: Build a two-player reflex game, and find out who in your house is fastest.
time: 1 hour
level: 2
sketch: Lesson03ReactionDuel
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 push buttons
  - Red, yellow and green LEDs
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - Active buzzer (the sealed one, often with a sticker on top)
  - 13 jumper wires
ideas:
  - Random numbers, and a random seed
  - Measuring time with millis ()
  - A game as a set of states
  - Catching a false start
  - The active buzzer
---

## What you'll build

<!-- closeup -->

A duel for two. Each player rests a finger on a button: red on the left,
green on the right. The yellow light blinks, somebody presses to start, and
everything goes dark. You wait. And wait. Then, at a moment nobody can
guess, the yellow light snaps on and the buzzer beeps. First to press wins,
their light flashes, and the Serial Monitor tells you exactly how fast they
were: *Red wins in 231 ms!* Jump the gun, and you lose on the spot.

## The idea

Three new tools make this game work.

**Random numbers.** `random (2000, 5000)` picks a whole number from 2000 up
to 4999, which the game uses as a wait in milliseconds, between 2 and 5
seconds. But a computer can't really roll dice. `random ()` follows a fixed
recipe that makes the same list of numbers every time the Mega starts, unless
you give it a different starting point, called the **seed**. The sketch reads
pin A7 for its seed. Nothing is connected to A7, so it floats (as a button's
pin would, without its pull-up), and its reading wanders with stray
electricity: a different seed, and a different game, nearly every time.

**Time.** `millis ()` is the number of milliseconds since the Mega started.
Read it once when the light comes on and again when a button is pressed, and
the difference is the reaction time:

<p class="formula">reaction = 12 631 ms − 12 400 ms = 231 ms</p>

**The active buzzer.** It has a tiny oscillator circuit inside, so it makes
its own tone, a shrill note a little over 2000 vibrations a second, whenever
its pin is HIGH. The Mega only switches it on and off, just like an LED. Its
maker rates it at up to 30 mA: more than an LED takes, and more than the
20 mA a pin gives comfortably all day, but well under the 40 mA that is the
most a Mega pin may ever give, and here it only sounds in short beeps. So it
plugs straight in, with no resistor. In [Lesson 5](../05-melody-maker/index.md)
you'll meet its cousin, the passive buzzer, which can play any note but needs
a resistor and more help from the Mega.

!!! question "Predict"
    Most people react faster to a sound than to a light. Before you play,
    guess your reaction time in milliseconds. Is it nearer 100, 250, or 500?

## How the game works

The game is always in exactly one **state**, and each state knows which
events it cares about. The sketch keeps the state in a variable and, on every
pass of `loop ()`, only does what the current state allows.

| State | What you see | What moves it on |
|---|---|---|
| **Waiting** | The yellow light blinks slowly | Either button: *Ready* |
| **Ready** | All dark, for a random 2 to 5 seconds | Time's up: *Go*. A press: a **false start**, and the other player wins |
| **Go** | Yellow on, and a beep | The first press wins: *Result* |
| **Result** | The winner's light flashes | Either button: a new round, *Ready* |

A false start is only possible because the game knows it is in *Ready*: the
same press in *Go* would win.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Every LED needs its
    220 Ω resistor. Make sure you have the **active** buzzer, the sealed one:
    the passive buzzer, plugged in here with no resistor, would take far more
    current than a pin can give. The active buzzer stands across the middle
    gap: its **+** leg, the longer one, with a **+** on its top, goes in f25,
    above the gap.

<!-- bench -->

<!-- steps -->

??? info "The buzzer across the gap"
    The buzzer's legs are 0.3 inch apart, exactly as far as row f is from
    row e across the middle gap, so it stands across the gap like the
    buttons do. Pin 12's wire reaches its **+** leg through the top half of
    column 25, and its other leg reaches the − rail through the black wire
    from a25.

    To tell the two buzzers in the kit apart, look underneath. The active
    buzzer is sealed with black plastic, and usually has a paper sticker on
    top saying *Remove seal after washing*: the seal kept water out while the
    factory washed the board. Peel it off for a louder beep, or leave it on
    for a quieter one. The passive buzzer is open underneath, and you can see
    a green circuit board.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson03ReactionDuel**:

<!-- sketch -->

What's new:

- `enum State { Waiting, Ready, Go, Result };` makes a new kind of value with
  four names, one for each state. `State state = Waiting;` is a variable that
  holds one of them. Names read far better than numbers: `state == Go` says
  what it means.
- `unsigned long` is a whole number that can't go below zero and can count
  past four billion: the right size for `millis ()`.
- `randomSeed (analogRead (A7));` plants the seed, once, in `setup ()`.
- In `loop ()`, each button is asked `wasPressed ()` once, into `redPressed`
  and `greenPressed`, and then an `if` for each state decides what those
  presses mean.
- `millis () - readyAt >= waitTime` asks "has the random wait passed yet?"
  without stopping the loop, so a false start is still noticed while you
  wait.
- `adk::Buzzer buzzer {12};` and `buzzer.beep (200);` sound the buzzer for
  200 ms and carry straight on; ADK switches it off by itself.
- `win ()`, `falseStart ()` and `showWinner ()` take the winning LED as
  `adk::Led& light`. The `&` means the function works on that very LED, not
  a copy of it, so `light.blink (200)` flashes the real one.
- `adk::wait (1000);` in `showWinner ()` gives the loser a second to finish
  their too-late press. Presses during `adk::wait ()` still update the
  buttons, but no `loop ()` is looking, so they are simply let go by.

## Upload it

Upload the sketch and open the Serial Monitor at 9600 baud. It says
*Reaction Duel! Press a button to start.*, and the yellow LED blinks once a
second.

1. Press either button. All the lights go out and the Serial Monitor says
   *Get ready...*
2. Between 2 and 5 seconds later, the yellow LED lights and the buzzer
   beeps.
3. Press! The winner's LED flashes quickly, and the Serial Monitor prints the
   time, such as *Green wins in 247 ms!*
4. Now try pressing before the yellow light. The buzzer gives a long, cross
   buzz, the other player's light flashes, and the Serial Monitor says who
   pressed too soon.

Press again for another round. How close was your guess? Most people's
times land somewhere around 200 to 300 ms. To find out whether you really
are faster to a sound, try the first challenge below.

## If it doesn't work

| What you see | Try this |
|---|---|
| The yellow LED doesn't blink at the start | Turn it round: its long leg goes in b15. Check its resistor runs from g15, across the gap, to e15. |
| A button never starts a round | Push it firmly into the board, all four legs in. Check its black wire goes from row a (a3 or a7) to the − rail. |
| The loser's light flashes, not the winner's | The LED wires may be swapped: pin 26 goes to j10 (red, on the left), pin 28 to j20 (green, on the right). |
| No beep | The buzzer may be the wrong way round: its **+** leg goes in f25, above the gap. Check the black wire from a25 to the − rail. |
| Only a faint click instead of a beep | That is the passive buzzer. Unplug the USB cable at once and swap it for the sealed, active one. |
| The same wait every game | Make sure nothing is plugged into A7: the seed only changes if the pin is left floating. |

??? note "How it works"
    `buzzer.beep (200)` switches the buzzer on at once and remembers the
    time. On each `adk::update ()` the buzzer checks whether 200 ms have
    passed, and switches itself off. Blinking LEDs work the same way, which
    is why the whole game runs in one quick loop without ever stopping to
    wait for something to finish.

    The times include the 20 ms ADK waits for a button to settle, so your
    real reaction is about 20 ms quicker than the number on screen. Both
    players get the same 20 ms, so the duel stays fair.

    `millis ()` counts up for about 49.7 days and then starts again from 0.
    Taking the difference, `millis () - readyAt`, still gives the right
    answer across that wrap, which is why time is always compared that way.

## Make it yours

1. **Eyes or ears?** Delete `buzzer.beep (200);` from `go ()` and play a few
   rounds by light alone. Then put it back, delete `yellow.on ();` and play
   with your eyes shut. Which way are you faster, and by how much?
2. **Record time.** Keep the fastest time since power-up in a variable, and
   print *New record!* whenever somebody beats it.
3. **Best of five.** Count each player's wins. The first to three wins the
   match: make their light blink slowly, and start a new match on the next
   press.
4. **A fair tie.** Both players could, just possibly, press in the same pass
   of `loop ()`. Right now red would win. Check for `redPressed &&
   greenPressed` first in the *Go* state, and call it a draw with both lights
   flashing.
