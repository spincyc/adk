---
lesson: 13
title: Hello, LCD
arc: Words and weather
promise: Put words on a real screen, and draw your own little characters.
time: 60 minutes
level: 2
sketch: Lesson13HelloLcd
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 display, with its 16 header pins
  - 10 kΩ potentiometer
  - 220 Ω resistor (red, red, black, black, brown)
  - 17 jumper wires
ideas:
  - How a character LCD shows text
  - Contrast and backlight
  - Writing with print (), and moving with at ()
  - Drawing your own characters
---

## What you'll build

<!-- closeup -->

A glowing blue screen says **Hello, LCD!** with a little heart after it, and
underneath, a tiny figure you designed yourself strides along the bottom row,
one step every quarter of a second. Until now the Mega could show you
numbers at best; from today it can *write*.

## The idea

The **LCD1602** is a character display: two rows of sixteen characters. Each
character is a grid of dots, five across and eight down, and a chip on the
back of the board, the **HD44780**, remembers what is on the screen and keeps
it lit. The Mega only has to tell that chip what to show, one character at a
time.

It does that over six wires. Four of them, **D4** to **D7**, carry the
character's code. A character is a byte, eight bits (you met bits in Lesson
10), so it goes across in two halves of four: **H** is 72, which is
`0100 1000` in binary, so the Mega sends `0100` and then `1000`. **RS** says whether the byte is a character to show or a command, such
as "clear the screen", and **E** is the "read it now" pulse after each half.

Two more things make the dots visible:

- **Contrast.** The dots are liquid crystal. They go dark by an amount set by
  the voltage on the **V0** pin, and a potentiometer sets that voltage, just
  as it set a voltage in Lesson 7. Too far one way and every dot is dark; too
  far the other and none is.
- **Backlight.** Behind the glass is an LED, with its own pins **A** (+) and
  **K** (−), and like every LED since [Lesson 1](../01-blink/index.md) it gets
  a resistor. It keeps about 3 V for itself, so:

<p class="formula">current = <span class="fraction"><span>5 V − 3 V</span><span>220 Ω</span></span> ≈ 9 mA</p>

That current comes from the 5 V rail, not from one of the Mega's pins.

The screen also has eight spare characters you can draw yourself. Each is
eight numbers, one per row of dots, written in binary so you can see the
picture: a `1` is a dot that lights. Here is the heart:

```text
. . . . .   0b00000
. # . # .   0b01010
# # # # #   0b11111
# # # # #   0b11111
. # # # .   0b01110
. . # . .   0b00100
. . . . .   0b00000
. . . . .   0b00000
```

Show two pictures one after the other, again and again, and you have an
animation, exactly like a flip book.

!!! question "Predict"
    Before you upload the sketch, the screen will have power but nothing to
    show. What do you think you will see with the contrast knob turned all
    the way one way? And all the way the other? Write down your guess, then
    try it when you first plug in.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire, and check every wire before you plug
    it back in. The LCD has sixteen pins: push it in gently and evenly, and
    rest the part of the screen that hangs off the breadboard on something the
    same height, such as the kit's box, so it can't lever its pins out.

<!-- bench -->

<!-- steps -->

??? info "What each of the LCD's sixteen pins does"
    | Pin | Name | Job | Goes to |
    |--:|---|---|---|
    | 1 | VSS | Ground | GND, through the knob's left leg |
    | 2 | VDD | Power for the chip | 5 V |
    | 3 | V0 | Contrast | The knob's middle leg |
    | 4 | RS | Character or command? | Pin 31 |
    | 5 | RW | Read or write? Tied to GND: the Mega only writes | GND |
    | 6 | E | "Read it now" | Pin 32 |
    | 7–10 | D0–D3 | The lower data wires, not needed with four-wire sending | Nothing |
    | 11–14 | D4–D7 | Data | Pins 33 to 36 |
    | 15 | A | Backlight + | 5 V through 220 Ω |
    | 16 | K | Backlight − | GND |

    GND reaches the knob's left leg from the bottom − rail, and the short
    black jumper carries it on to VSS; the short red jumper brings 5 V from
    VDD's column to the knob's right leg. The brown one joins the knob's
    middle leg to V0. VDD takes its 5 V from the top + rail, and VSS joins
    the top − rail, so that rail is GND too: the backlight's K and RW use it.

    This is the course's screen. It sits in the same holes, wired the same
    way, in every lesson that uses it, so you can leave it on the breadboard
    from one to the next.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson13HelloLcd**:

<!-- sketch -->

What's new:

- `adk::Lcd lcd {31, 32, 33, 34, 35, 36};` names the screen and its six
  pins, in the order RS, E, D4, D5, D6, D7.
- `constexpr uint8_t heart [8] {...};` is a plain list of eight bytes, one
  for each row of dots: the `[8]` says how many. It is what `createChar ()`
  asks for. Each `0b` number is written in binary, as in the picture above.
- `lcd.createChar (1, heart);` stores the picture in slot 1, and
  `lcd.write (1);` shows it. The sketch uses slots 1 to 3.
- `lcd.print ("Hello, LCD! ");` writes text where the cursor is, and moves
  the cursor along. It works just like `Serial.println ()`, numbers
  included, except that it doesn't end the line.
- `lcd.at (column, 1)` moves the cursor. Columns count from 0 to 15 and rows
  from 0 to 1, so `(0, 0)` is the top-left corner. It hands back the screen
  itself, so `lcd.at (column, 1).print (' ');` prints a space right there.
  (Arduino's own LCD library calls this move `setCursor ()`; ADK has that
  too.)
- `takeStep ()` rubs out the figure with a space, moves one column right (the
  `% 16` wraps 16 back round to 0), and draws it again.
  `column % 2 == 0 ? 2 : 3` picks slot 2, standing, on even columns and
  slot 3, striding, on odd ones.

## Upload it

Plug the Mega in before you upload, and check your prediction. The
backlight glows as soon as there is power. With the knob at one end, the top
row is a line of solid blocks, because every dot is dark; at the other end,
the screen is blank. Did you predict both? Nothing has talked to the screen
yet: the knob alone decides.

Now upload the sketch as in [Lesson 1](../01-blink/index.md#upload-it), and
turn the knob slowly until the letters are sharp and the empty squares behind
them almost vanish.

The top row reads **Hello, LCD!** followed by a heart. On the bottom row the
little figure walks from left to right, four steps a second, and after the
last column it starts again at the left.

## If it doesn't work

| What you see | Try this |
|---|---|
| No light at all | Check the red wire from the Mega's 5V to the top + rail (T+3), the resistor from e23 across the gap to f23, and the wires from j23 to + and e24 to −. |
| The backlight glows, but the screen is blank | Turn the contrast knob, slowly, all the way through. If nothing ever appears, check the brown wire from c6 to c11 and the black one from a5 to the bottom − rail. |
| A row of solid blocks on top, nothing below | The screen has power but isn't hearing the Mega. Check pins 31 and 32 go to e12 (RS) and e14 (E), and that the upload finished. |
| Strange symbols instead of letters | Two data wires are swapped: pins 33, 34, 35 and 36 go to e19, e20, e21 and e22, in that order. Check too that RW (e13) goes to −. |
| Text appears, then scrambles when you touch a wire | A loose wire. Push each one in firmly. |
| The heart or the figure is a different shape | Check the rows of the picture: each has exactly five 0s and 1s after `0b`. |
| The **L** LED on the Mega blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    `adk::setup ()` wakes the HD44780 the way its datasheet asks: it waits
    50 ms for the chip to power up, sends a few special halves that work
    whatever state the chip woke up in, and switches it to four-wire mode, two
    rows, cursor hidden. That takes about 60 ms.

    After that, each character is two pulses of E, one per half, and takes
    about a tenth of a millisecond, so a whole screen of text is quick.
    Clearing the screen takes 2 ms. RW is tied to GND because ADK only ever
    writes; reading from the chip isn't needed.

    Each row really holds 40 characters, and only the first 16 are on the
    screen. Text that runs past column 15 goes on into the hidden ones rather
    than wrapping onto the next row.

    One catch with your own characters: to show slot 0 you must write
    `lcd.write (uint8_t (0));`, because a bare `0` could also mean "no text at
    all". That's why the sketch starts at slot 1.

## Make it yours

1. **Your name.** Put your name in the middle of the top row. For a name of
   `n` letters, start at column `(16 - n) / 2`.
2. **There and back.** Make the figure walk back when it reaches the right
   edge, instead of jumping to the left. You'll need a variable that says
   which way it is walking.
3. **Your own characters.** Design a smiley, a rocket or a space invader on
   squared paper, five squares across and eight down, and turn each row into
   `0b` and five digits. There are eight slots; the sketch leaves 4 to 7 free.
4. **A clock of sorts.** Show how many seconds the Mega has been running in
   the three free columns at the top right, once a second:
   `lcd.at (13, 0).print (millis () / 1000);` What goes wrong after 999
   seconds, and how could you make room?
