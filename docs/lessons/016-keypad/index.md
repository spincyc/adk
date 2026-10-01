---
lesson: 16
promise: Read sixteen keys with eight wires, and turn the Mega into a calculator that keeps a running total.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - 4×4 membrane keypad
  - 8 more jumper wires
ideas:
  - "Rows and columns: a key matrix"
  - Scanning, one row at a time
  - Building a number from its digits
  - A running total
---

## What you'll build

<!-- closeup -->

An adding machine you wired yourself, the simplest kind of calculator. Type
**12** and press **A**: the top row says **Total 12**. Type **30** and press
**A** again: **Total 42**. **B** takes a number away instead, and **\***
starts the total again from 0.

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
thousands of times a second here. It follows one held key at a time and
debounces changes the way it debounces a button.

Four rows and four columns make 4 × 4 = 16 keys on 4 + 4 = 8 wires. A 10 × 10
grid would give 100 keys on just 20 wires, which is how a computer keyboard
manages.

The keys arrive one at a time, as characters: `'7'`, `'A'`, `'#'`. To build a
number from them, the sketch uses the same trick you use when you write one
down: each new digit shifts the others one place to the left. Typing **4**
then **2** makes 4, then 4 × 10 + 2 = 42. Type **7** next and it's
42 × 10 + 7 = 427.

!!! question "Predict"
    Pressing **5** joins row 2 to column 2, the wire on pin 27. Suppose
    that wire came out of pin 27. Which keys would stop working: only
    **5**, a whole row, a whole column, or every key? Write down your guess;
    you'll test it once the adding machine works.

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
    4, so the leftmost one goes to pin 22 and the rightmost to pin 29. If
    pressing **5** puts a 9 on the screen, and **1** does nothing (the
    keypad read it as **D**), the wires are in back to front: turn the
    order of all eight round.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 016-keypad**:

<!-- sketch -->

What's new:

- `adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};` names the
  keypad: its four row pins, then its four column pins.
- `keypad.pressedKey ()` is the key pressed in this pass of `loop ()`, or `'\0'`, "no
  key", in every other pass. Like `wasPressed ()` in Lesson 2, it's an event:
  each press gives it once.
- `char key` holds one character, as in Lesson 14. In single quotes, `'7'` is
  one character; in double quotes, `"7"` would be a piece of text.
- Characters are numbers underneath. `'0'` to `'9'` have codes one after
  another, so `key - '0'` turns `'7'` into the number 7, and
  `key >= '0' && key <= '9'`, with Lesson 4's `&&`, asks whether the key is
  a digit at all.
- `number` is the number being typed. Each digit shifts it one place left
  and joins on the end, with `number * 10 + (key - '0')`.
- `digits` counts the digits typed into the number and stops at four, so a
  number is at most 9999. Counting them, rather than looking at how big the
  number is, stops a row of zeros at four too.
- `total` is a `long`, from Lesson 8, which holds numbers past two billion:
  room to add 9999 more than 200 000 times. `total = total + number` works
  out the right-hand side first, then puts the answer back into `total`.
- Keys **C**, **D** and **#** match none of the questions in the `if` and
  its `else if`s, so they do nothing yet.
- `showTotal ()` clears the screen, writes the total on the top row, and
  moves to the start of the bottom row with `lcd.at (0, 1)`, ready for the
  next number, which starts again from 0. `setup ()` calls it too, so the
  screen starts at **Total 0**.

## Upload it

Upload the sketch. The top row says `Total 0`. Type **12**: it appears on
the bottom row. Press **A**: the top row says `Total 12`, and the bottom row
is empty again. Type **30** and press **A**: `Total 42`. Type **50** and
press **B**: `Total -8`, below zero. Press **\*** to start again from 0.

Then test your prediction. Unplug the USB cable and take out the wire from
pin 27 altogether, easing its other end gently out of the keypad's sixth
contact from the left, so no loose end can touch anything. Plug the cable
back in and press every key. **2**, **5**, **8** and **0** do nothing. Every other digit
still appears, even **4** and **6**, which share row 2 with **5**, and
**A**, **B** and **\*** still work. Those four keys are column 2. Each
joins its row to column 2's wire, and with that wire gone no scan ever sees
pin 27 go low. So a whole row or column that stops at once points to one
wire. Unplug the USB cable again and put the wire back, from pin 27 to the
keypad's sixth contact.

## If it doesn't work

| What you see | Try this |
|---|---|
| No key does anything | Check the eight wires go to pins 22 to 29, in order, and that each is pushed fully into the keypad's socket. |
| Pressing **5** shows 9 | The ribbon is back to front: see "Which way round is the ribbon?" above. |
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
    debounce. When it does, `pressedKey ()` returns it for exactly one update. That's
    why holding both **1** and **2**, then releasing **1** first, gives the
    **2** only after **1** is let go: until then, **1** is still the key held.

## Make it yours

1. **Times and divide.** Give **C** and **D** jobs: **C** multiplies the
   total by the number, and **D** divides it. Whole numbers divide into
   whole numbers, as the octave knob did in Lesson 9: 7 / 2 is 3. Nothing
   can be divided by 0, so make **D** do nothing while `number` is 0. Then
   add 9999 and multiply by 9999 twice: a `long` stops at about 2 billion,
   and the total comes out wrong.
2. **Backspace.** Let **#** rub out the last digit you typed: dividing a
   number by 10 drops its last digit. How will you rub it out on the
   screen?
3. **Hide it.** Print a `*` for each digit instead of the digit itself, like a
   password box. You'll need exactly that in
   [Lesson 18](../018-keypad-safe/index.md).
4. **A two-number calculator.** Make **12 C 34 #** show `12 x 34` on the top
   row and `= 408` below it. When **A** to **D** is pressed, keep `number`
   in a second `long`, `first`, keep the key in a `char`, and start the
   number again from 0. On **#**, a `switch` on that key, like `colorOf ()`
   in Lesson 15, prints `first + number`, `first - number`,
   `first * number` or `first / number`. Refuse to divide by 0, which has
   no answer.

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
probe stays in c62, in the column of the backlight's K, which is GND: the
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

## Check yourself

1. If the wire on pin 22, row 1, came loose, which keys would stop working,
   and why those?
2. Why does `key - '0'` turn the character `'7'` into the number 7?
3. Holding **5** down for a whole second puts just one 5 on the screen,
   though `loop ()` runs thousands of times. Why?

??? note "Answers"
    1. **1**, **2**, **3** and **A**, the top row. Each of them joins row 1
       to a column, and with row 1's wire gone the Mega can no longer pull
       that row low, so no column ever reads low for them.
    2. Characters are codes, and `'0'` to `'9'` come one after another.
       Taking away the code of `'0'` leaves how far along the digits `'7'`
       is: 7.
    3. `keypad.pressedKey ()` is an event: it gives each press once, in one pass
       of `loop ()`, and `'\0'` in every other pass, just as `wasPressed ()`
       does for a button.
