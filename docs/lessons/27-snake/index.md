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
  - The snake as a deque: on at the front, off at the back
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

The snake is a line of dots, head first. Each dot is a place on the matrix,
x across and y down, so (3, 4) is x 3, y 4. When the snake takes a step,
every dot of its body moves to where the dot in front of it was. So the
middle of the line doesn't change at all: only the ends do. A new head goes
on at the front, and the old tail comes off at the back:

<p class="formula">before: (3, 4) (2, 4) (1, 4) &nbsp;&nbsp;→&nbsp;&nbsp; after a step right: (4, 4) (3, 4) (2, 4)</p>

To grow, the snake simply keeps its tail for one step.

A line you can add to and take from at either end is called a **deque**,
said "deck", short for *double-ended queue*. ADK's `adk::Deque` holds the
snake, with room for 64 dots: enough for a snake that fills the matrix.

A **collision** is the new head landing somewhere it mustn't. If x or y
would go below 0 or above 7, it's the wall. If the new head is already one
of the snake's dots, the snake has bitten itself. There's one exception: the
tail moves away in the same step, so the head may take its place.

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

What's new:

- The three sounds are arrays of `adk::Note`, as in Lesson 5.
  `speaker.play (gulp)` starts one and returns at once, so the game never
  stops to wait for a sound.
- `struct Dot` holds a dot's `x` and `y` together, so the sketch can hand a
  whole dot around: `Dot food` is where the food is.
- `bool operator== (const Dot&) const = default;` lets you compare two dots
  with `==` and `!=`, as you would two numbers. `= default` asks the
  compiler to write the comparison for you: two dots are equal when their
  `x` and their `y` both are. So `head == food` means the head has reached
  the food.
- `adk::Deque<Dot, 64> snake;` is the snake: a line of up to 64 dots, head at
  the front and tail at the back. `snake.front ()` is the head,
  `snake.back ()` the tail, and `snake[1]` the dot just behind the head, its
  neck. `push_front ()` puts a dot on the front and `pop_back ()` takes one
  off the back. Like the `adk::Vector` in Lesson 6, it keeps all its room
  from the start, and `snake.size ()` says how many dots are in use.
- `adk::Joystick::Direction turn` holds one of the stick's directions: `Up`,
  `Down`, `Left`, `Right` or `Center`. `steer ()` saves the stick's
  `direction ()` as the next turn, unless the dot that way is the snake's
  own neck, so it can never turn straight back on itself.
- `playing` is the game's state from the table above. While it's `false`,
  `loop ()` scrolls `message` and waits for a click.
- `ahead ()` finds the dot next to the head in a direction, with a `switch`
  as in Lesson 3: up takes one from `y`, right adds one to `x`, and so on.
  `default:` catches every direction without a `case`, here `Center`.
- `moveSnake ()` runs on every beat of `step`. It finds the new head, checks
  for the wall and for a bite, then takes the tail off unless the snake is
  eating, and puts the new head on. A bite plays `gulp`, makes the beat
  20 ms shorter and places new food.
- `for (auto part : snake)` in `onSnake ()` walks the snake from head to
  tail, as range-`for` walks an `adk::Array`.
- `placeFood ()` uses a `do` ... `while` loop. It is a `while` loop that
  runs its body first and asks afterwards, so it always picks one dot, and
  picks again for as long as that dot is on the snake.
  `randomSeed (analogRead (A7))` in `setup ()` makes the food land somewhere
  different every game, as in Lesson 3.
- `blink` flips the food on and off by asking the matrix whether it is lit
  with `matrix.get ()`.
- `newGame ()` lays out three dots, sets the beat back to 400 ms, and
  `step.restart ()` makes the first step come a whole beat after the click.
- `gameOver ()` plays the crash, leaves the dead snake on show for a moment,
  and writes the score into `message` with `snprintf`. It prints into a row
  of characters instead of to the Serial Monitor, putting the number where
  `%d` stands.

## Upload it

Upload the sketch. *SNAKE! CLICK TO PLAY* scrolls across the matrix. Hold
the joystick with its pins to your left and click the stick: a rising
three-note fanfare, and a three-dot snake appears in the middle, crawling
right, with a dot of food blinking somewhere else. Steer to it. Each bite
gulps, and the snake grows a dot and moves a little faster. Run into an
edge, or into yourself, and three falling notes play; a moment later
*SCORE* and your number of bites scroll by.

You predicted the bites to top speed. From 400 ms down to 120 ms is 280 ms,
and each bite takes 20 ms off, so it takes 280 ÷ 20 = 14 bites. After that
the snake steps every 120 ms: 1000 ÷ 120, a little over 8 steps a second,
more than three times the 2½ steps a second it starts with.

## If it doesn't work

| What you see | Try this |
|---|---|
| The snake turns the wrong way | Hold the joystick with its pins pointing to your left. |
| The snake ignores some pushes | It won't reverse into its own neck, and a push has to go more than halfway to count. Push firmly, one way at a time. |
| No sound | Check the buzzer's + leg is in h5, in the resistor's column, and the resistor runs from f1 to f5. The black jumper from f8 must reach the − rail, which needs its GND wire. |
| A click starts nothing | The stick's switch is on pin 22: press straight down until it clicks. |
| The snake dies at once | The first step comes 400 ms after the click: be ready to steer. |
| The matrix shows junk | Check the matrix's wires, especially CLK on 48 and CS on 49. |

??? note "How it works"
    `adk::Deque` keeps its dots in a ring of 64 boxes, and remembers which
    box holds the front and how many are in use. Putting a dot on the front
    or taking one off the back only changes those two numbers and one box,
    so a step takes the same time for a snake of 3 dots or 60.

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
   side. At the end of `ahead ()`, when `dot.x` becomes 8 make it 0, and
   when it becomes −1 make it 7; then do the same for `dot.y`.
2. **Pause.** Put Lesson 26's button back on pin 23 and use it to pause:
   while paused, skip `steer ()` and `moveSnake ()`.
3. **High score.** Keep the best score in a variable and scroll *BEST* after
   *SCORE*. Store it in EEPROM, as the safe in Lesson 18 stored its code, so
   it survives unplugging.
4. **Obstacles.** Keep a few wall dots in an `adk::Vector<Dot, 8>`, draw
   them at the start of each game, and make `moveSnake ()` treat them like
   the edges. Or add a second food that is worth three dots.
