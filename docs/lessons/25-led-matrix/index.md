---
lesson: 25
promise: Draw pictures and scroll messages on 64 LEDs, all from three pins.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - MAX7219 8×8 LED matrix module
  - Push button
  - 5 female-to-male jumper wires
  - 3 jumper wires
ideas:
  - How one chip runs 64 LEDs from three pins
  - Pixels, and where x and y are
  - Pictures written as binary numbers
  - An array of pictures, each an array of rows
  - Scrolling text
---

## What you'll build

<!-- closeup -->

A grid of 64 red LEDs wakes up by filling itself dot by dot, then smiles at
you. Press the button and the smile becomes a heart, then a space invader,
and then HELLO! slides across the grid like a sign in a shop window. It is
your first screen made of pixels, and the start of three lessons that end
with a game.

## The idea

The matrix is 64 LEDs in 8 rows and 8 columns. Each LED sits where a row
wire crosses a column wire, so a row and a column together pick out one LED.
Wiring 64 LEDs to 64 pins would use most of the Mega. Instead, a chip on the
module, the **MAX7219**, does the work. It lights one row at a time, very
quickly, and your eyes blend the rows into one picture: the same trick
the four-digit display played in Lesson 11, except that the chip does it by
itself, about 800 times a second. It also sets the current for every LED,
so none of them needs its own resistor.

The Mega talks to the chip over three wires, much like the 74HC595 in
Lesson 10: **DIN** carries the bits, **CLK** says when each bit is ready,
and **CS** tells the chip to take them.

Each LED is a **pixel**, a dot in a picture, with an address. **x** counts
columns from 0 on the left to 7 on the right, and **y** counts rows from 0
at the top to 7 at the bottom. `matrix.set (0, 0)` lights the top-left dot;
`matrix.set (7, 7)` lights the bottom-right one.

A whole row is eight dots, and eight on-or-off dots are exactly one byte.
Write it in **binary**, with `0b` in front, and the ones are the lit dots,
left to right:

<p class="formula">0b00111100 &nbsp;→&nbsp; ○ ○ ● ● ● ● ○ ○</p>

Eight of those, one per row, make a picture, and the whole picture takes just
8 bytes of memory.

!!! question "Predict"
    The heart's top row is `0b01100110`. Sketch the eight dots of that row
    on paper: which ones are lit? Check your drawing against the heart when
    it appears.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. The matrix gets its power from the
    Mega's 5V pin: with all 64 LEDs at full brightness it can draw about
    330 mA, most of what a USB port gives. ADK starts it at half brightness,
    which is plenty indoors. Its ground comes from the bottom − rail, which
    the button shares.

<!-- bench -->

<!-- steps -->

??? info "Which end of the matrix is the input?"
    The module has a row of five pins at each end. Wire the end whose pins
    are labeled **DIN**, CS and CLK. The other end says **DOUT**: it passes
    the data on, so a second matrix could be chained after this one.

    Modules are built in different ways round, so yours may show pictures
    sideways at first. That's what the dot-by-dot start is for: it begins
    at the top-left dot, (0, 0), and fills along the top row. Turn the module
    until it does exactly that.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson25LedMatrix**:

<!-- sketch -->

What's new:

- `adk::LedMatrix matrix {47, 48, 49};` names the matrix and its data, clock
  and CS pins, in that order.
- `using Picture = adk::Array<uint8_t, 8>;` gives a type a name of your own.
  A picture is an `adk::Array` of eight `uint8_t`s, one byte for each row,
  and from here on the sketch can simply say `Picture`.
- `constexpr Picture heart { ... };` writes the heart's eight rows one under
  another, so the 1s draw the heart right there in the code.
- `adk::Array pictures {smiley, heart, invader};` is an **array of arrays**:
  three pictures, each made of eight rows. `pictures[1]` is the heart, and
  `pictures[1][0]` is its top row.
- `fillDotByDot ()` has one `for` loop inside another. The outer loop picks a
  row, `y`; the inner one walks along it, `x` from 0 to 7, lighting each dot
  with `matrix.set (x, y)`. `adk::wait (30)` keeps the matrix updating, so
  you can watch every dot arrive.
- `slide` counts button presses. 0, 1 and 2 are the pictures; 3, one past
  the last picture, is the message. `% (pictures.size () + 1)` wraps it back
  to 0, just as `% 5` counted the moods round in Lesson 4.
- `matrix.show (pictures[slide])` puts a whole picture up at once. Calling
  it on every pass of `loop ()` costs almost nothing: the matrix only sends
  rows that changed.
- `matrix.scroll ("HELLO!")` slides the text in from the right, one column
  every 80 milliseconds. Asking again while it scrolls changes nothing, and
  asking once it has finished starts it over, so the message repeats for as
  long as `slide` is 3.

## Upload it

Upload the sketch. The matrix fills from the top-left corner, dot by dot
along each row, row after row, in about two seconds. Half a second later the
smiley appears. Press the button: a heart. Again: a space invader. Again:
HELLO! scrolls past, over and over, until the next press brings back the
smiley.

You predicted the heart's top row, `0b01100110`. Reading from the left, the
dots are off, on, on, off, off, on, on, off: dots 1, 2, 5 and 6, counting
from 0. They are the two bumps on top of the heart.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check VCC goes to 5V and GND to the bottom − rail, that the rail has its black wire from the Mega's GND, and that you used the matrix's input end, marked **DIN**. |
| Random dots, or pictures made of junk | CLK and CS are probably swapped: CLK goes to pin 48 and CS to 49. Push each jumper fully onto its pin. |
| The fill starts in another corner or runs down a column | The module is turned round. Turn it until the fill starts top left and runs along the top row. |
| The button does nothing | The button must straddle the middle gap, with the jumper from a4 to the − rail and the black wire from GND to that rail. |
| The Mega's **L** LED blinks long and short flashes | ADK found a pin problem and is blinking its number. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    ADK keeps a copy of the picture in the Mega's memory: eight bytes, one
    per row. `set ()`, `show ()` and `scroll ()` only change that copy. On
    the next `adk::update ()`, every row that changed is sent to the chip as
    16 bits, the row's number and then its eight dots, clocked out on DIN
    one bit at a time. When CS goes high, the chip stores the row. Each row
    takes about 0.2 ms to send.

    When the sketch starts, ADK tells the chip to scan all 8 rows, to treat
    each byte as eight separate dots rather than a digit, to use brightness
    7 of 15, and to clear its old picture before it lights anything. From
    then on the chip scans the rows by itself, so the Mega only speaks up
    when the picture changes.

    Scrolling uses a small font kept in flash memory: every letter is five
    columns of seven dots, with a blank column after it.

## Make it yours

1. **Your own picture.** Draw an 8 × 8 grid on paper, shade a design, and
   turn each row into a `0b` number. Make it a `Picture` of its own and add
   its name to `pictures`. The button shows it too, with no other change:
   the sketch counts the pictures with `pictures.size ()`.
2. **Your message.** Change `"HELLO!"` to your name. Scroll it faster with
   `matrix.scroll ("SAM", 40);`, where 40 is the milliseconds per step.
3. **Brightness.** Add `matrix.brightness (1);` after `adk::setup ();`. Try
   0 and 15. Then put a potentiometer on A0, as in Lesson 7, and set the
   brightness with it.
4. **Animate.** Draw a second invader with its legs the other way and swap
   between the two on each tick of an `adk::Every` of 300 ms, so it walks.
   Or make a single dot bounce around the edges using `set (x, y)` and
   `set (x, y, false)`.
