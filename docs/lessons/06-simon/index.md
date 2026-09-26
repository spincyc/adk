---
lesson: 6
promise: Build the classic memory game of lights and tones, and try to beat your best.
time: 1½ hours
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 4 push buttons
  - Red, yellow, green and blue LEDs
  - 5 × 220 Ω resistors (red, red, black, black, brown)
  - Passive buzzer (the one with a green board showing underneath)
  - 18 jumper wires
ideas:
  - A list that grows, in an adk::Vector
  - Taking turns, as states
  - Counting loops, and functions that hand back an answer
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

Simon has to remember its tune, and the tune grows. An `adk::Array`, like
[Lesson 4](../04-mood-lamp/index.md)'s moods, always holds the same number
of things, so Simon keeps its tune in an **`adk::Vector`** instead: a list
that can grow and shrink, up to a size you choose.
`adk::Vector<uint8_t, 100> sequence;` has room for 100 steps and starts
with none. Each step is a key's number, 0 for red up to 3 for blue, and
each round `sequence.push_back (random (4))` adds a random one at the end.
`sequence.size ()` says how many there are so far:

| Round | `sequence.size ()` | `sequence` | Simon shows |
|---|---|---|---|
| 1 | 1 | 2 | green |
| 2 | 2 | 2, 0 | green, red |
| 3 | 3 | 2, 0, 3 | green, red, blue |

The four keys are a fixed set, so they stay in an `adk::Array`, in color
order. Each is a `Key` that groups a button, its light and its tone, as
[Lesson 5](../05-melody-maker/index.md) grouped a button and a pitch. So
`keys[2]` is everything green: `keys[2].button`, `keys[2].light` and
`keys[2].pitch`. One number picks a whole color.

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
| **Your turn** | Each light and its note follow its button while you hold it | Letting go is your answer. Right: the next step. Last step right: Simon's turn again, one step longer. Wrong: game over |
| **Game over** | A low buzz, then a fanfare if you beat your best | Back to *Idle* |

Simon's turn is so short and simple, just show and wait, that the sketch
does it all in one go, inside `nextRound ()`, and game over is just as
quick, inside `gameOver ()`. So only two states need names in the code:
`State::Idle` and `State::YourTurn`.

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
    about 2 V, blue about 3.2 V. Through a shared resistor each color would
    shine at a different brightness, and when all four light together,
    blinking while Simon waits for a player, the red one would take nearly
    all the current and the blue one would barely glow. A resistor each
    keeps every color steady.

    Lesson 5's four buttons, on pins 22 to 25, and its buzzer stay just
    where they are. Each LED goes in beside its own button: red, on pin 26,
    just right of the first button, then yellow, green and blue, on 27 to 29,
    beside the others. So every light sits next to the button that answers
    it.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson06Simon**:

<!-- sketch -->

What's new:

- `struct Key` is Lesson 5's key with a light added.
  `Key {22, 26, adk::note::c4}` fills one in: the button's pin, the light's
  pin, then the pitch.
- `adk::Vector<uint8_t, 100> sequence;` is Simon's tune: room for 100 key
  numbers, empty at first. In the `<>`, `uint8_t` says what the list holds
  and 100 how many it can. `sequence.clear ()` empties it for a new game.
- `int pressedKey ()` is a function that **returns a value**. The `int`
  before its name says it hands back a whole number, and `return number;`
  hands it back and leaves the function at once. When no key was pressed
  it reaches `return -1;`, which is nobody's number. `loop ()` keeps the
  answer: `int pressed = pressedKey ();`.
- `for (int number = 0; number < 4; number++)` is a **counting loop**. It
  sets `number` to 0, runs the lines inside while `number < 4` is true,
  and adds one after each pass, so they run for 0, 1, 2 and 3. A
  range-for hands you each key but not its number, and `pressedKey ()`
  needs the number.
- In `nextRound ()`, `for (auto number : sequence)` walks the steps added
  so far, from the first, and `flash (keys[number])` shows each one: the
  key with that number lights up and sounds its note.
- `yourTurn ()` makes each light follow its button, as in
  [Lesson 2](../02-buttons/index.md), and sounds a key's note when it goes
  down. `held` remembers which key that was, and letting go of it is your
  answer. `held` starts each turn at -1, so a button you were already
  holding while Simon played doesn't count when you let it go.
- `check ()` compares your answer with `sequence[step]`. Wrong ends the
  game. Right moves `step` on, until the last step: steps are numbered
  from 0, so the last is `sequence.size () - 1`. Then Simon takes its turn
  with `nextRound ()`, which puts the lights out, pauses for a second and
  adds the next step, unless `sequence.full ()` says all 100 steps are
  used: then you've beaten Simon.
- `best` holds the high score. `speaker.play (fanfare);` celebrates a new
  one and returns at once, so `waitForPlayer ()` sets the lights blinking
  while it plays.

## Upload it

Upload the sketch, and open the Serial Monitor at 9600 baud if you want to
see your scores. All four lights blink together.

1. Press any button. The lights go out, and a second later Simon shows one
   light with its note.
2. Press that light's button. The light and its note stay on while you
   hold it; let go.
3. After a short pause Simon shows two steps. Repeat them, and keep going.
4. Make a mistake on purpose. A low buzz sounds for a second. The Serial
   Monitor says *You remembered 3 steps. Best so far: 3*,
   and if that's a new best, you hear a fanfare. The lights blink again,
   ready for the next game.

You predicted how many ten-step sequences there are. Four colors for each
of ten steps is 4 × 4 × 4 × 4 × 4 × 4 × 4 × 4 × 4 × 4 = 1 048 576 different
sequences. More than a million.

## If it doesn't work

| What you see | Try this |
|---|---|
| One light never lights | Turn that LED round: long leg in row b, on the left. Check its resistor runs from row g, across the gap, to row e. |
| A light and its button don't match | The signal wires are out of order. Buttons: pins 22 to 25 into j2, j8, j14 and j20. Lights: pins 26 to 29 into j6, j12, j18 and j24. |
| The lights blink, but one button never starts a game | Push that button firmly in, all four legs, and check its black wire from row a to the − rail. |
| No sound | Follow pin 10: j34, the buzzer's **+** leg in f34 and its other leg in e34, and the resistor from a34 down into the − rail. |
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

    The sequence takes 100 bytes of RAM, one `uint8_t` per step, even
    before the first round. An `adk::Vector` sets aside all its room when
    the sketch starts and never asks for more while it runs, so the RAM the
    Arduino IDE reports after compiling is all the sketch will ever use.

## Make it yours

1. **Faster and faster.** Real Simon speeds up. Keep the time each step is
   shown in an `int` variable: set it to 400 where `loop ()` starts a new
   game, use it in `flash ()`, and take 10 off in `nextRound ()` as long as
   it is still over 150.
2. **Your score in lights.** After a game, blink the blue light once for each
   step you remembered, with a counting loop, so you don't need the Serial
   Monitor.
3. **Hurry up.** Give the player three seconds for each press. Start an
   `adk::Timer`, as in Lesson 3, for 3000 ms when your turn starts and after
   each answer, and if it `expired ()`, it's game over.
4. **Remember forever.** Keep the best score in EEPROM so it survives being
   unplugged. Add `#include <EEPROM.h>`, read it in `setup ()` with
   `best = EEPROM.read (0);` (a Mega that has never stored anything reads
   255 there, so treat 255 as 0), and save a new best with
   `EEPROM.update (0, best);`.
5. **Pass the Simon.** A two-player version: instead of a random step, each
   player repeats the sequence and then adds one step of their own for the
   other to remember.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red in **V**. Never move the red lead to the **A**
jack for these: set for current, the meter is just a wire, and would short
out whatever you put it across.

The sketch needs no changes. In your turn a light stays on, with its note,
for as long as you hold its button, right or wrong; letting go is your
answer. So start a game, and when it is your turn, hold a button, read its
LED, and let go. Ask a helper to hold the button while you hold the probes,
or hold both probes in one hand like chopsticks. The two legs of an LED are
in neighboring holes, so keep each tip on its own leg.

!!! question "Predict"
    In Lesson 4, the RGB LED's red kept about 2 V for itself, and its green
    and blue about 3.2 V. Which do you think Simon's yellow LED will be
    nearer?

<!-- measure -->

What the numbers tell you:

- **Red and yellow** keep about 2 V; **green and blue** about 3.2 V. An
  LED's color sets the voltage it keeps: the bluer the light, the more it
  needs.
- Each LED's resistor takes the rest of the 5 V, so the currents differ:
  (5 V − 2 V) ÷ 220 Ω ≈ 14 mA for red and yellow, and
  (5 V − 3.2 V) ÷ 220 Ω ≈ 8 mA for green and blue. Each resistor sets its
  own LED's current, whatever the others do.
- That is why the LEDs don't share one resistor. With all four on
  together, through one resistor, the red LED would let current through as soon
  as it had 2 V across it, and hold every LED near 2 V: too little for green
  and blue, which need about 3.2 V, so they would stay nearly dark.
