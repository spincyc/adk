---
lesson: 16
promise: Read sixteen keys with eight wires, and turn the Mega into a calculator.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - 4×4 membrane keypad
  - 8 jumper wires
ideas:
  - Rows and columns: a key matrix
  - Scanning, one row at a time
  - Building a number from its digits
---

## What you'll build

<!-- closeup -->

A pocket calculator you wired yourself. Tap **12**, press **C** for times, tap
**34** and press **#**: the screen answers **= 408**. **A**, **B**, **C** and
**D** are plus, minus, times and divide, and **\*** wipes the slate clean.

## The idea

The keypad has sixteen keys but only eight wires. If every key had its own
wire, it would need sixteen, plus one for GND. Instead the keys sit in a grid
of four **rows** and four **columns**. Under each key, a row wire crosses a
column wire, and pressing the key joins those two wires, and nothing else.

To find which key is down, the Mega **scans**. It pulls row 1 low and looks at
the four columns. The columns are held high by pull-ups, just like the buttons
in Lesson 2, so a column that reads low must be joined to row 1 by a pressed
key. Then it lets row 1 go and tries row 2, and so on. Pressing **5** joins
row 2 (pin 23) to column 2 (pin 27), so pin 27 reads low only while row 2 is
being pulled low. ADK scans all four rows every time `adk::update ()` runs,
thousands of times a second, so no press is ever missed, and it debounces each
key the way it debounces a button.

Four rows and four columns make 4 × 4 = 16 keys on 4 + 4 = 8 wires. A 10 × 10
grid would give 100 keys on just 20 wires, which is how a computer keyboard
manages.

The keys arrive one at a time, as characters: `'7'`, `'A'`, `'#'`. To build a
number from them, the sketch uses the same trick you use when you write one
down: each new digit shifts the others one place to the left. Typing **4**
then **2** makes 4, then 4 × 10 + 2 = 42. Type **7** next and it's
42 × 10 + 7 = 427.

!!! question "Predict"
    What do you think happens if you hold down **1** and, while it's still
    down, press **2**? Does the screen show 1, 2, 12 or 21, and when? Write
    down your guess and try it once the calculator works.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep the screen just as it is, and
    take out everything else from Lesson 15. The keypad lies above the Mega,
    and its eight wires come from pins 22 to 29, right next to the screen's
    wires. The keypad's ribbon is thin: push the jumper wires into its socket
    gently, and don't fold the ribbon sharply.

<!-- bench -->

<!-- steps -->

??? info "Which way round is the ribbon?"
    Hold the keypad with its keys facing you and the ribbon hanging down. The
    eight contacts, from left to right, are rows 1 to 4 and then columns 1 to
    4, so the leftmost one goes to pin 22 and the rightmost to pin 29. If you
    press **1** and see **D** or **\***, the wires are in back to front: turn
    the order of all eight round.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 016-keypad**:

<!-- sketch -->

What's new:

- `adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};` names the
  keypad: its four row pins, then its four column pins.
- `keypad.key ()` is the key pressed in this pass of `loop ()`, or `'\0'`, "no
  key", in every other pass. Like `wasPressed ()` in Lesson 2, it's an event:
  each press gives it once.
- `char key` holds one character, as in Lesson 14. In single quotes, `'7'` is
  one character; in double quotes, `"7"` would be a piece of text.
- Characters are numbers underneath. `'0'` to `'9'` have codes one after
  another, so `key - '0'` turns `'7'` into the number 7. In the same way
  `key - 'A'` turns **A** to **D** into 0 to 3, the place of each one's
  symbol in `symbols`, an `adk::Array` of characters.
- `number` is the number being typed. Each digit shifts it one place left
  and joins on the end, with `number * 10 + (key - '0')`. When you choose
  an operation, `number` moves into `first`, and starts again from 0 for
  the second number.
- `digits` counts the digits typed into the number and stops at four, so even
  9999 × 9999 fits in a `long`. Counting them, rather than looking at how big
  the number is, stops a row of zeros at four too.
- `answered` remembers that the answer is showing. While it is, a digit
  clears the screen and starts a new sum, and **A** to **D** and **#** do
  nothing.
- `showAnswer ()` writes the answer on the bottom row, or a message instead
  if you ask it to divide by zero, which has no answer.
- `calculate ()` is a `switch` on the operation's symbol, like `colorOf ()`
  in Lesson 15. Divide one whole number by another and the answer is whole
  too, the remainder dropped, as the octave knob's was in Lesson 9: 7 / 2
  is 3.

## Upload it

Upload the sketch; the screen starts blank. Type **12**, press **C**, type
**34** and press **#**. The top row reads `12 x 34` and the bottom row
`= 408`. Press any digit to start a new sum, or **\*** to clear. Try **7**,
**D**, **2**, **#** for `= 3`, and **5**, **D**, **0**, **#** to see what the
calculator thinks of dividing by zero.

Then test your prediction: hold **1** and press **2**. Only **1** appears at
first; **2** appears when you let **1** go. So the screen ends up showing 12,
but the 2 only arrives when you let go: the keypad counts one key at a time.

## If it doesn't work

| What you see | Try this |
|---|---|
| No key does anything | Check the eight wires go to pins 22 to 29, in order, and that each is pushed fully into the keypad's socket. |
| **1** shows as **D** or **\*** | The ribbon is back to front: see "Which way round is the ribbon?" above. |
| A whole row or column of keys is dead | One wire is loose. Row 1 is pin 22, row 4 pin 25; column 1 is pin 26, column 4 pin 29. |
| Digits appear twice | A dirty or worn contact. Press firmly and squarely; if one key keeps doing it, it's the keypad. |
| The screen is blank or shows blocks | Go back to Lesson 13's table: the LCD wiring or the contrast knob. |

??? note "How it works"
    ADK never drives a row high. It sets a row's pin low first and only then
    makes it an output, and afterwards turns it back into an input, left
    floating. So if you press two keys in one column, the two rows they join
    are never one high and one low, which would be a short circuit between two
    pins.

    Each scan finds the first key held down, or keeps the one already held.
    A new key must read the same for 20 ms before it counts, which is the
    debounce. When it does, `key ()` returns it for exactly one update. That's
    why holding **1** and pressing **2** gives the **2** only when **1** is let
    go: until then, **1** is still the key held.

## Make it yours

1. **Keep going.** After an answer, make **A** to **D** carry on with the
   answer as the first number, so you can type **2 A 3 # C 4 #** and get 20.
2. **Decimals.** Make **7 D 2 #** show `= 3.500`. A whole number can't hold
   the half, but a `float`, from Lesson 14, can. In `showAnswer ()`, for a
   division, print `float (first) / number` with `lcd.print (value, 3)`:
   `float (first)` turns `first` into a `float`, so the division keeps its
   decimals.
3. **Backspace.** Let **\*** delete the last digit instead of clearing
   everything: dividing a number by 10 drops its last digit. How will you
   rub it out on the screen?
4. **Bigger numbers.** Allow six digits. What goes wrong with 999999 × 999999,
   and why? (A `long` holds numbers up to about 2 billion.)
5. **Hide it.** Print a `*` for each digit instead of the digit itself, like a
   password box. You'll need exactly that in
   [Lesson 18](../018-keypad-safe/index.md).

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it to DC volts as in [Lesson 1](../001-blink/index.md#measure-it), black lead
in **COM** and red in **V**. The keypad's wires go straight from the Mega to
its socket, with no hole for a probe, but the screen's data wires land in
the breadboard. After each character the Mega leaves the second half of its
code on D4 to D7 until it sends the next one, so they hold still between
key presses and the sketch needs no change.

!!! question "Predict"
    The character `'5'` is code 53, which is `0011 0101` in binary, and the
    screen gets it in two halves, as in Lesson 13. Which half stays on the
    wires once it has gone? Which of D4 to D7 will read 5 V? Write down your
    guess.

<!-- measure -->

Press **\*** to clear, type **5**, and measure each wire in turn. The black
probe stays in c24, in the column of the backlight's K, which is GND: the
bottom − rail is under the screen. The data wires stand side by side, so
keep the red tip in its own hole.

What the numbers tell you:

- Read from D7 down to D4, the wires say 0 V, 5 V, 0 V, 5 V. Call 5 V a 1
  and 0 V a 0 and that's `0101`, the second half of `0011 0101`: the half
  sent last is the one still on the wires.
- `0101` is 5 in binary, 4 + 1. Every digit's code ends in the digit
  itself: `'0'` is `0011 0000` and `'7'` is `0011 0111`. That's why
  `key - '0'` works: taking away `'0'` takes away the first half, `0011`,
  and leaves the digit.
- Type another digit and measure again: 6 is `0110`, 9 is `1001`. The
  meter reads the key you pressed, in binary, off four wires.
