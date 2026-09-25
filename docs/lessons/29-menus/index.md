---
lesson: 29
title: Menus
arc: Tilt and turn
promise: Turn and click a knob to move through a menu and change settings.
time: 1 hour
level: 2
sketch: Lesson29Menus
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 display
  - 10 kΩ potentiometer
  - Rotary encoder module
  - Red LED
  - 2 × 220 Ω resistors (red, red, black, black, brown)
  - 5 female-to-male jumper wires
  - 18 jumper wires
ideas:
  - A knob that turns forever and counts clicks
  - How the encoder tells which way it turned
  - Browsing a menu, and editing a setting
  - A menu as an array of items, each with its own choices
---

## What you'll build

<!-- closeup -->

A lamp with a control panel. The screen shows a menu: **Level**, **Mode**
and **Speed**. Turn the knob and the arrow moves through the items; click
it and the arrow jumps across to the setting, so turning now changes it:
brighter or dimmer, steady, blinking or slowly breathing, slow or fast.
Click again to go back to the list. It is how the menus on cookers,
printers and car radios work.

## The idea

A **rotary encoder** looks like a potentiometer, but it has no ends: it
turns forever, and you can feel it click into place about 20 times a turn.
Each click is called a **detent**. The encoder doesn't report *where* it
is, only that it has turned, and which way.

Inside are two switch contacts, called **CLK** and **DT**, that open and
close as you turn, a quarter of a step out of time with each other.
Turning clockwise, CLK changes first and DT follows; turning the other way,
DT changes first. So by watching which contact changes first, the Mega knows
the direction:

<p class="formula">clockwise: 11 → 01 → 00 → 10 → 11 &nbsp;&nbsp; anticlockwise: 11 → 10 → 00 → 01 → 11</p>

Each pair of digits is CLK then DT: 1 while the contact is open, 0 while it's
closed. Only one digit changes at a time, and on this encoder one detent is
four changes. ADK counts them for you: `knob.turned ()` is 1 in the update
where the knob clicked clockwise, −1 anticlockwise, and 0 the rest of the
time. Pressing the knob's shaft works a push switch, **SW**, which is just a
button.

A **menu** needs two states, and the click swaps between them:

| State | The arrow points at | Turning the knob |
|---|---|---|
| Browsing | The item's name, **>Level** | Moves to the next or previous item |
| Editing | The item's setting, **>60%** | Changes that setting |

Each item has a list of **choices**, and a **setting**: which choice it's
set to, counting from 0. Level's choices go 0%, 10%, 20% and so on up to
100%, so a setting of 6 means 60%, and the lamp gets 6 × 255 ÷ 10 = 153 out
of 255 on its PWM pin, just as the dimmer's `write ()` set it in Lesson 7.

!!! question "Predict"
    The knob has about 20 clicks in a full turn, and each click moves Level
    by 10%. From 0% to 100%, how far round do you turn the knob? What
    happens if you keep turning past 100%?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire: this is a long build, so check
    each wire against the list as you go. The LED gets its own 220 Ω
    resistor, and so does the LCD's backlight.

<!-- bench -->

<!-- steps -->

??? info "The LCD's sixteen pins, one by one"
    The LCD is wired as in Lesson 13, from pin 1 on the left:

    | Pin | Name | Goes to |
    |---|---|---|
    | 1 | VSS | GND |
    | 2 | VDD | 5V |
    | 3 | V0 | the potentiometer's middle leg: the contrast |
    | 4 | RS | pin 31 |
    | 5 | RW | GND, because the Mega only ever writes |
    | 6 | E | pin 32 |
    | 7–10 | D0–D3 | nothing: ADK sends four bits at a time |
    | 11–14 | D4–D7 | pins 33 to 36 |
    | 15 | A | 5V through 220 Ω: the backlight's + |
    | 16 | K | GND: the backlight's − |

    Here the potentiometer stands just above the LCD's first pins, on the
    other side of the middle gap, and three short jumpers cross the gap to
    VSS, VDD and V0. The rotary encoder module's pins are labeled CLK, DT, SW,
    + and GND.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson29Menus**:

<!-- sketch -->

What's new:

- `adk::RotaryEncoder knob {18, 19};` names the encoder's CLK and DT pins.
  `adk::Button click {22};` is its push switch, and `adk::PwmOutput lamp {3};`
  is the LED, dimmed with PWM.
- `levels`, `modes` and `speeds` are the choices, as lists of text.
- `struct Item` is one line of the menu: its `name`, its `choices` and its
  `setting`. `adk::Span<const char* const> choices` is a view of a list kept
  somewhere else, as in Lesson 18, so every item can point at a list of a
  different length. The second `const` says the texts in the list are fixed
  too.
- `adk::Array menu { Item {...}, ... };` is the whole menu, an array of
  structs as in Lesson 5. Writing `Item` in front of each one says what they
  are, so the array knows its type without being told.
- `Item& level = menu[0];` makes `level` another name for the first item,
  with `&` just as in Lesson 3. So `level.setting` in `brightness ()` says
  which setting it means, where `menu[0].setting` wouldn't.
- `current` is the item on the top row, and `editing` says which state the
  menu is in.
- `turnKnob ()` does the right thing for the state. Editing, it adds the
  clicks to the setting, and `constrain` stops it at the first and last
  choice. Browsing, `wrap ()` moves through the items and round from the last
  to the first, and back.
- `wrap ()` counts round in a circle. `%` gives the remainder, but for a
  number below zero the remainder is below zero too: −1 % 3 is −1. So when
  it is, `wrap ()` adds the count once more, and −1 becomes 2, the last item.
- `showMenu ()` redraws the screen only when something changed: the current
  item on the top row, the next one below it, then the arrow in front of
  either the name or the setting. `lcd.at (9, row)` moves to column 9 of that
  row and hands back the screen, so `.print ()` can follow straight on.
- `brightness ()` turns the settings into a brightness for this moment.
  `lampTime` is an `adk::Stopwatch`, started in `setup ()`, and
  `lampTime.elapsed () % period` is how far through the current blink or
  breath the lamp is. The `switch` picks by `mode.setting`: Blink is full
  brightness for the first half and off for the second; Breathe rises
  steadily through the first half and falls through the second; Steady,
  the `default`, is always full.

## Upload it

Upload the sketch. The LCD shows `>Level   60%` on top and `Mode    Steady`
below, and the LED glows at a bit over half brightness. If the text is faint
or missing, turn the potentiometer until it's sharp.

Turn the knob one click clockwise: `>Mode` moves to the top. Click the knob:
the arrow jumps to `>Steady`. Turn it: `Blink`, `Breathe`. Click again, turn
to `Speed`, click, and choose `Fast`: the LED now breathes quickly. Go back
to `Level` and turn it down to 0%, or up to 100%.

You predicted how far to turn from 0% to 100%. That's 10 clicks, and with
about 20 clicks in a turn it is about half a turn. Keep turning past 100%
and nothing changes: `constrain` holds Level at its last choice, and the
first click back brings it straight down to 90%.

## If it doesn't work

| What you see | Try this |
|---|---|
| The backlight is on but there is no text | Turn the potentiometer slowly from end to end: that's the contrast. |
| A row of solid blocks | The LCD has power but isn't hearing the Mega: check RS on 31 and E on 32. |
| Strange characters | Check D4 to D7 go to pins 33 to 36, in order. |
| Turning clockwise goes backwards | CLK and DT are swapped: CLK goes to 18, DT to 19. |
| One click moves two items, or it takes two clicks to move one | Your encoder makes a different number of changes per click. Try `adk::RotaryEncoder knob {18, 19, 2};`. |
| Clicking does nothing | Press the shaft straight down until it clicks, and check SW goes to pin 22. |
| The menu works but the LED never lights | Check the LED's long leg is in j18, and the resistor runs from h22 to h18. |

??? note "How it works"
    On every `adk::update ()` the encoder reads both contacts and compares
    them with the last reading. A table of the sixteen possible pairs says
    whether that change was a step clockwise (+1), anticlockwise (−1), or
    no step at all. When the contacts bounce, a step forward is followed by
    a step back, and the two cancel out. Four steps in the same direction
    make a detent, and `turned ()` reports it.

    Because it only watches for changes, the encoder must be read often. ADK
    reads it about a thousand times a second as long as `loop ()` keeps
    calling `adk::update ()` or `adk::wait ()`. A long `delay ()` would miss
    the steps made while it waited.

## Make it yours

1. **A fourth item.** Add `Color` with the choices `Red`, `Green` and
   `Blue`: a list of its own, and one more `Item` in `menu`. The menu shows
   it with no other change. Then wire the RGB LED from Lesson 4 on pins 5, 6
   and 7 instead of the single LED, and light it in the chosen color.
2. **Finer steps.** Let Level go from 0% to 100% in steps of 5%: give
   `levels` 21 choices, and divide by 20 instead of 10 in `brightness ()`.
3. **Remember.** Save each item's `setting` in EEPROM, as the safe in
   Lesson 18 saved its code, so the lamp wakes up the way you left it.
4. **Long press.** Hold the knob down for two seconds to put every setting
   back to its starting value. `click.isPressed ()` and an `adk::Stopwatch`,
   as in Lesson 3, will tell you how long it has been held.
