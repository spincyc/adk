---
lesson: 6
title: Simon
arc: Color and sound
promise: Build the classic memory game of lights and tones, and try to beat your best.
time: 1½ hours
level: 3
sketch: Lesson06Simon
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 4 push buttons
  - Red, yellow, green and blue LEDs
  - 5 × 220 Ω resistors (red, red, black, black, brown)
  - Passive buzzer (the one with a green board showing underneath)
  - 19 jumper wires
ideas:
  - Arrays, and a sequence that grows
  - Taking turns, as states
  - Lights and tones together
  - Keeping a high score
---

## What you'll build

<!-- closeup -->

Simon, the memory game from 1978. Four lights blink, waiting for you. Press
any button and Simon takes its turn: one light flashes with its own note. You
press that light's button. Simon plays it again and adds another, then
another. Every round the tune grows by one step, and you must repeat it all,
in order. One wrong press and it's over, with a low buzz. Beat your best and
Simon plays you a fanfare.

## The idea

You met **arrays** in [Lesson 5](../05-melody-maker/index.md): numbered rows
of things of one kind, under one name, such as the four keys. An array can
also be a row of empty boxes waiting to be filled. `uint8_t sequence [100];`
makes 100 boxes for small whole numbers, numbered from `sequence[0]` to
`sequence[99]` (the sketch gives that 100 a name, `longest`). Simon keeps
its tune there, one color number in each box, 0 for red up to 3 for blue,
and a separate variable, `length`, says how many boxes are in use so far.
Each round adds one:

| Round | `length` | `sequence` | Simon shows |
|---|---|---|---|
| 1 | 1 | 2 | green |
| 2 | 2 | 2, 0 | green, red |
| 3 | 3 | 2, 0, 3 | green, red, blue |

The sketch also keeps the four buttons, the four LEDs and the four tones in
arrays of their own, in the same color order, so `lights[2]` is the green
LED, `buttons[2]` its button and `tones[2]` its note. One loop can then do
the same thing for every color: `lights[color]` works whatever `color` is.

Each color's tone comes from one chord: C, E, G and the C above. That's why
any sequence Simon picks sounds like a little tune.

Every step Simon shows takes 400 ms, with a 150 ms gap after it, so showing
a ten-step sequence takes

<p class="formula">10 × (400 ms + 150 ms) = 5500 ms = 5.5 seconds</p>

!!! question "Predict"
    Each step can be any of four colors. How many different ten-step
    sequences could Simon choose? About forty? About a thousand? More than a
    million?

## How the game works

Simon and the player take **turns**, and the game is always in one state:

| State | What you see | What happens |
|---|---|---|
| **Idle** | All four lights blink slowly | Any button starts a new game |
| **Simon's turn** | Simon adds a random step, then shows the whole sequence | Your presses are ignored |
| **Your turn** (`Listening`) | Each light and its note follow its button while you hold it | Letting go is your answer. Right: the next step. Last step right: Simon's turn again, one step longer. Wrong: game over |
| **Game over** | All four lights on and a low buzz, then a fanfare if you beat your best | Back to *Idle* |

Simon's turn is so short and simple, just show and wait, that the sketch
does it all in one go, inside `nextRound ()`. So only two states need names
in the code: `Idle` and `Listening`.

Your score is how many steps you remembered: the length of the last round
you completed. The best score is kept in a variable, `best`, for as long as
the Mega has power.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Every LED needs its
    own 220 Ω resistor, and the passive buzzer needs its 220 Ω resistor too,
    as in [Lesson 5](../05-melody-maker/index.md): never leave it out.

<!-- bench -->

<!-- steps -->

??? info "Why not one resistor for all four LEDs?"
    Only one light is on at a time, so it's tempting to share a single
    resistor. But the colors keep different voltages for themselves: red
    about 2 V, blue about 3 V. Through a shared resistor each color would
    shine at a different brightness, and when all four light together at
    game over, the red one would take nearly all the current and the blue
    one would barely glow. A resistor each keeps every color steady.

    The layout follows the pins in order: the four buttons on pins 22 to 25,
    then the four LEDs on 26 to 29, red, yellow, green and blue, so the first
    button belongs to the first light. The buzzer comes last.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson06Simon**:

<!-- sketch -->

What's new:

- `adk::Button buttons [] {{22}, {23}, {24}, {25}};` and `adk::Led lights []
  {{26}, {27}, {28}, {29}};` make the four buttons and the four LEDs as
  arrays, in color order, as the keys were in Lesson 5. So
  `buttons[color].wasPressed ()` asks the button of whichever color
  `color` is.
- `sequence[length] = random (4);` fills the next empty box with 0, 1, 2 or
  3, and `length = length + 1;` counts it in.
- `lights[sequence[index]].on ();` reads from the inside out: the color
  number at place `index` in the sequence, then the light with that number.
- `followButton ()` copies each button to its light, as the yellow LED did
  in [Lesson 2](../02-buttons/index.md), starts the tone on `wasPressed ()`,
  and checks your answer on `wasReleased ()`. The variable `held` remembers
  which button went down on your turn, so a button you were already holding
  while Simon played doesn't count when you let it go.
- `check ()` compares your color with `sequence[step]`. Wrong ends the game.
  Right moves `step` on, and when `step` reaches the end of the sequence,
  Simon takes its turn with `nextRound ()`. The sequence can hold 100 steps;
  if you ever fill it, you've beaten Simon.
- `best` holds the high score. `speaker.play (fanfare);` celebrates a new
  one and returns at once, so the lights start blinking while it plays.

## Upload it

Upload the sketch, and open the Serial Monitor at 9600 baud if you want to
see your scores. All four lights blink together.

1. Press any button. The lights go out, and a second later Simon shows one
   light with its note.
2. Press that light's button. The light and its note stay on while you
   hold it; let go.
3. After a short pause Simon shows two steps. Repeat them, and keep going.
4. Make a mistake on purpose. All four lights come on with a low buzz for a
   second. The Serial Monitor says *You remembered 3 steps. Best so far: 3*,
   and if that's a new best, you hear a fanfare. The lights blink again,
   ready for the next game.

Your prediction: four colors for each of ten steps is 4 × 4 × 4 × 4 × 4 ×
4 × 4 × 4 × 4 × 4 = 1 048 576 different sequences. More than a million.

## If it doesn't work

| What you see | Try this |
|---|---|
| One light never lights | Turn that LED round: long leg in row b, on the left. Check its resistor runs from row g, across the gap, to row e. |
| A light and its button don't match | The signal wires are out of order. Buttons: pins 22 to 25 into j1, j5, j9 and j13. Lights: pins 26 to 29 into j17, j23, j29 and j35. |
| The lights blink, but one button never starts a game | Push that button firmly in, all four legs, and check its black wire from row a to the − rail. |
| No sound | Follow pin 10: j40, the buzzer's legs in f40 and e40, and the resistor from a40 down into the − rail. |
| Every game starts with the same steps | Leave A7 unconnected: the random seed comes from it floating. |
| The Mega's **L** LED blinks long and short flashes | A pin in the sketch is wrong. The Serial Monitor says which. |

??? note "How it works"
    During Simon's turn the sketch sits in `adk::wait ()`. The buttons are
    still read, because `adk::wait ()` keeps updating every part, but no
    `loop ()` is running to act on them, so a press then is simply let go
    by. That's what makes the turns fair.

    The high score lives in the Mega's working memory, its **RAM**: 8 KB
    that forgets everything when the power goes off. The Mega also has 4 KB
    of **EEPROM**, memory that remembers without power; Lesson 18, *Keypad
    Safe*, uses it to keep a secret code.

    The sequence takes 100 bytes of RAM, one per step. `uint8_t` is the
    smallest whole number there is, 0 to 255, which is plenty for a color
    number.

## Make it yours

1. **Faster and faster.** Real Simon speeds up. Make each step shorter as
   the sequence grows, say `400 - length * 10` milliseconds, but never less
   than 150: `max (150, 400 - length * 10)`.
2. **Your score in lights.** After a game, blink the blue light once for each
   step you remembered, so you don't need the Serial Monitor.
3. **Hurry up.** Give the player three seconds for each press. Keep
   `millis ()` when your turn starts and after each answer, and if three
   seconds pass without one, it's game over.
4. **Remember forever.** Keep the best score in EEPROM so it survives being
   unplugged. Add `#include <EEPROM.h>`, read it in `setup ()` with
   `best = EEPROM.read (0);` (a Mega that has never stored anything reads
   255 there, so treat 255 as 0), and save a new best with
   `EEPROM.update (0, best);`.
5. **Pass the Simon.** A two-player version: instead of a random step, each
   player repeats the sequence and then adds one step of their own for the
   other to remember.
