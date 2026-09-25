---
lesson: 27
title: Snake
arc: Pixels and games
promise: Build the classic game, with a snake that grows, speeds up and sings.
time: 1 hour
level: 3
sketch: Lesson27Snake
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix and joystick from Lesson 26
  - Passive buzzer
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 female-to-male jumper wires
  - 3 jumper wires
ideas:
  - A game loop that moves on a steady beat
  - The snake as an array
  - Collisions with the walls and yourself
  - Speeding up, and sound for every event
---

## What you'll build

<!-- closeup -->

The game that lived on millions of phones, on a screen you wired yourself.
A three-dot snake slides across the matrix, and you steer it with the
joystick towards a blinking dot of food. Every bite plays a little gulp,
adds a dot to the snake and makes it faster. Hit a wall or your own tail
and the game ends with a sad three-note tune and your score scrolling past.
Click the stick to play again.

## The idea

Most games are a **loop** that runs on a beat. Every 400 milliseconds the
snake takes one step, whatever you're doing; between steps the sketch keeps
listening to the joystick, so it always knows which way you want to go
next. An `adk::Every` from Lesson 11 keeps the beat, and each bite shortens
it by 20 ms, down to 120 ms, so the game speeds up as you do better.

The snake is an **array**: a numbered row of boxes, each holding one dot the
snake covers, head first. To give every dot one number instead of two, the
sketch numbers them 0 to 63 as **x + 8 × y**. Dot 35 is x 3, y 4, because
3 + 8 × 4 = 35; to go back, `35 % 8` is 3 (the remainder) and `35 / 8` is 4.

Moving the snake is a shuffle. Every dot's number moves one box along the
array, the old tail drops off the end, and the new head goes in box 0:

<p class="formula">before: 35 34 33 &nbsp;&nbsp;→&nbsp;&nbsp; after a step right: 36 35 34</p>

To grow, the snake simply doesn't drop its tail for one step.

A **collision** is the new head landing somewhere it mustn't. If x or y
would go below 0 or above 7, it's the wall. If the new head's number is
already in the array, the snake has bitten itself. There's one exception:
the tail moves away in the same step, so the head may take its place.

!!! question "Predict"
    The snake starts with a step every 400 ms, and each bite takes 20 ms off,
    down to 120 ms. How many bites until it reaches top speed? How many
    steps a second is that?

## How the game plays

The game is in one of two states, **waiting** or **playing**:

| State | On the matrix | What changes it |
|---|---|---|
| Waiting | A message scrolls: *SNAKE! CLICK TO PLAY*, or your last score | Clicking the stick starts a new game: a fanfare, and a snake of three dots heading right. |
| Playing | The snake, and the blinking food | Hitting a wall or yourself: a crash tune, a pause, and back to waiting with *SCORE* and the number of bites. |

While playing, pushing the joystick turns the snake, but never straight back
into its own neck: going right, it can turn up or down, not left. The food
always appears on a random dot the snake doesn't cover.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. If you have Lesson 26 built, take
    out the button and its two jumpers, but keep the black wire from GND to
    the top − rail: the buzzer uses it. The passive buzzer always goes
    through its 220 Ω resistor: its coil is only about 16 Ω, and on its own
    it would take far more current than a pin should give.

<!-- bench -->

<!-- steps -->

??? info "Why the buzzer needs 220 Ω"
    The passive buzzer is a tiny electromagnet that pulls on a metal disc.
    Its coil is a thin wire of only about 16 Ω. Straight from a pin, Ohm's
    law gives 5 V ÷ 16 Ω, over 300 mA: far past the 40 mA that would damage
    the pin. With the 220 Ω resistor in the way the current is
    5 V ÷ (220 Ω + 16 Ω), about 20 mA while the pin is high, and the pin is
    only high half the time while a note plays. It is quieter than without,
    but plenty loud across a room.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson27Snake**:

<!-- sketch -->

Read it from the top:

- The three melodies are arrays of `adk::Note`, as in Lesson 5.
  `speaker.play (gulp)` starts one and returns at once, so the game never
  stops to wait for a sound.
- `uint8_t snake [64]` has room for a snake that fills the whole matrix, and
  `length` says how many boxes are in use.
- `playing` is the game's state. While it's `false`, `loop ()` scrolls
  `message` and waits for a click.
- `steer ()` turns the joystick's `direction ()` into a step: `x` of −1 or 1
  for left or right, `y` of −1 or 1 for up or down. It only saves the turn
  if it isn't straight back the way the snake last went.
- `moveSnake ()` runs on every beat of `step`. It works out the new head,
  checks for walls and bites, then shuffles the array with a `for` loop that
  counts *down*, so no dot is overwritten before it has moved.
- `light (dot, lit)` turns a dot number back into x and y for the matrix.
- `blink` flips the food on and off by asking the matrix whether it is lit
  with `matrix.get ()`.
- `placeFood ()` picks random dots until it finds one the snake doesn't
  cover. `randomSeed (analogRead (A0))` in `setup ()` makes the food land
  somewhere different every game, as in Lesson 3.
- `gameOver ()` plays the crash, leaves the dead snake on show for a moment,
  and writes the score into `message` with `snprintf`, which prints into a
  row of characters instead of to the Serial Monitor.

## Upload it

Upload the sketch. *SNAKE! CLICK TO PLAY* scrolls across the matrix. Hold
the joystick with its pins to your left and click the stick: a rising
three-note fanfare, and a three-dot snake appears in the middle, crawling
right, with a dot of food blinking somewhere else. Steer to it. Each bite
gulps, and the snake grows a dot and moves a little faster. Run into an
edge, or into yourself, and three falling notes play; a moment later
*SCORE* and your number of bites scroll by.

## If it doesn't work

| What you see | Try this |
|---|---|
| The snake turns the wrong way | Hold the joystick with its pins pointing to your left. |
| The snake ignores some pushes | It won't reverse into its own neck, and a push has to go more than halfway to count. Push firmly, one way at a time. |
| No sound | Check the buzzer's + leg is in h5, in the resistor's column, and the resistor runs from f1 to f5. The black jumper from f8 must reach the − rail, which needs its GND wire. |
| A click starts nothing | The stick's switch is on pin 22: press straight down until it clicks. |
| The snake dies at once | The steps start as soon as you click: be ready to steer. |
| The matrix shows junk | Check the matrix's wires, especially CLK on 48 and CS on 49. |

??? note "How it works"
    `adk::Every step {400}` ticks once every 400 ms, and
    `step.period (step.period () - 20)` changes its beat while the game
    runs. `max (120UL, …)` keeps it from going below 120 ms; the `UL` makes
    120 the same kind of number as the period, an unsigned long.

    The speaker plays with Timer 2, the Mega's timer that also runs PWM on
    pins 9 and 10, which is why the buzzer lives on pin 10: nothing else in
    the course needs PWM there. The matrix, the joystick and the speaker all
    keep working through `adk::update ()`, including during the 1.5 second
    `adk::wait ()` after a crash, so the crash tune plays to the end.

## Make it yours

1. **Wrap-around.** Instead of dying at the edges, come back on the other
   side: when x becomes 8 make it 0, and when it becomes −1 make it 7.
2. **Pause.** Put Lesson 26's button back on pin 23 and use it to pause:
   while paused, skip `steer ()` and `moveSnake ()`.
3. **High score.** Keep the best score in a variable and scroll *BEST* after
   *SCORE*. Store it in EEPROM, as the safe in Lesson 18 stored its code, so
   it survives unplugging.
4. **Obstacles.** Draw a few walls in the middle of the matrix at the start of
   each game and make `moveSnake ()` treat them like the edges. Or add a
   second food that is worth three dots.
