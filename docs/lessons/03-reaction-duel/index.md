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
  - Measuring time with a timer and a stopwatch
  - A game as a set of states
  - Grouping parts that belong together
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

ADK does that sum for you in two parts. An `adk::Timer` counts down, like a
kitchen timer: the game sets one for the random wait. An `adk::Stopwatch`
counts up: the game starts one when the light comes on, and reads it when
somebody presses.

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
| **Go** | Yellow on, and a beep | The first press wins: *Over* |
| **Over** | The winner's light flashes | Either button: a new round, *Ready* |

A false start is only possible because the game knows it is in *Ready*: the
same press in *Go* would win.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Every LED needs its
    220 Ω resistor. Make sure you have the **active** buzzer, the sealed one:
    the passive buzzer, plugged in here with no resistor, would take far more
    current than a pin can give. The active buzzer stands across the middle
    gap: its **+** leg, the longer one, with a **+** on its top, goes in f34,
    above the gap.

<!-- bench -->

<!-- steps -->

??? info "The buzzer across the gap"
    The buzzer's legs are 0.3 inch apart, exactly as far as row f is from
    row e across the middle gap, so it stands across the gap like the
    buttons do. Pin 12's wire reaches its **+** leg through the top half of
    column 34, and its other leg reaches the − rail through the black wire
    from a34.

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

- `struct Player` makes a new type that bundles what belongs together: a
  player's name, button and light. `Player red {"Red", 22, 26};` fills them
  in, in order: the name, the button's pin, the light's pin. Then
  `red.button` and `red.light` reach inside, so the red player's button and
  light can never get mixed up with green's.
- `const char* name` holds a piece of text, such as `"Red"`.
- `enum class State { Waiting, Ready, Go, Over };` makes a new kind of value
  with four names, one for each state, and `State state` is a variable that
  holds one of them. `state == State::Go` says what it means, which a number
  never would.
- `adk::Timer suspense;` counts down. `suspense.start (random (2000, 5000))`
  sets it going, and `suspense.expired ()` is true for the one update in
  which it runs out, just as a button's `wasPressed ()` is true once per
  press. Nothing stops to wait, so a false start is still noticed.
- `adk::Stopwatch reaction;` counts up. `reaction.restart ()` sets it back to
  zero and running as the light comes on; `reaction.elapsed ()` reads it.
- `randomSeed (analogRead (A7));` plants the seed, once, in `setup ()`.
- `loop ()` hands every press to `pressed ()`, saying who pressed and who
  they are up against: `pressed (red, green)`.
- `switch (state)` jumps to the `case` for the current state and runs it, up
  to its `break`. Two labels in a row share their code: a press while
  *Waiting* or when the round is *Over* both start a new round. Those five
  lines are the whole game.
- `Player& player`: the `&` means the function works on that very player,
  not a copy, so `winner.light.blink (200)` flashes the real light.
- `auto time = reaction.elapsed ();` keeps the reaction time in a variable.
  `auto` tells the compiler to give `time` whatever type `elapsed ()` hands
  back, so you don't have to spell it out.
- `adk::println (Serial, winner.name, " wins in ", time, " ms!");` prints a
  line from its pieces, as in Lesson 2.
- `adk::Buzzer buzzer {12};` and `buzzer.beep (200);` sound the buzzer for
  200 ms and carry straight on; ADK switches it off by itself.
- `adk::wait (1000);` in `celebrate ()` gives the loser a second to finish
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
| The yellow LED doesn't blink at the start | Turn it round: its long leg goes in b12. Check its resistor runs from g12, across the gap, to e12. |
| A button never starts a round | Push it firmly into the board, all four legs in. Check its black wire goes from row a (a4 or a10) to the − rail. |
| The loser's light flashes, not the winner's | The LED wires may be swapped: pin 26 goes to j6 (red, on the left), pin 28 to j18 (green, on the right). |
| No beep | The buzzer may be the wrong way round: its **+** leg goes in f34, above the gap. Check the black wire from a34 to the − rail. |
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
    The timer and the stopwatch only ever take the difference of two
    readings, which still gives the right answer across that wrap.

## Make it yours

1. **Eyes or ears?** Delete `buzzer.beep (200);` from `go ()` and play a few
   rounds by light alone. Then put it back, delete `yellow.on ();` and play
   with your eyes shut. Which way are you faster, and by how much?
2. **Record time.** Keep the fastest time since power-up in a variable, and
   print *New record!* whenever somebody beats it.
3. **Best of five.** Add `int wins = 0;` to `Player`, and count each
   player's wins. The first to three wins the match: make their light blink
   slowly, and start a new match on the next press.
4. **A fair tie.** Both players could, just possibly, press in the same
   update. Right now red would win, because `loop ()` asks red's button
   first. In the *Go* state, check whether both were pressed, and call it a
   draw with both lights flashing.
