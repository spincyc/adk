---
lesson: 29
promise: Turn and click a knob to move through a menu and change settings.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 display
  - 10 kΩ potentiometer
  - Rotary encoder module
  - Red LED
  - 2 × 220 Ω resistors (red, red, black, black, brown)
  - 5 female-to-male jumper wires
  - 19 jumper wires
ideas:
  - A knob that turns forever and counts clicks
  - How the encoder tells which way it turned
  - Browsing a menu, and editing a setting
  - A menu as an array of items, each with its own choices
---

## What you'll build

<!-- closeup -->

A lamp with a control panel. The screen shows a menu of two items,
**Level** and **Mode**. Turn the knob and the arrow moves from one to the
other; click it and the arrow jumps across to the setting, so turning now
changes it: brighter or dimmer, steady or blinking. Click again to go back
to the items. It is how the menus on cookers, printers and car radios
work.

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
| Browsing | The item's name, **>Level** | Moves the arrow to the next or previous item |
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
    each wire against the list as you go. The LED matrix and the GY-521
    come off for this lesson: put them aside with their wires, because
    Lesson 30 brings them back, in the same places. The LED gets its own
    220 Ω resistor, and so does the LCD's backlight.

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

    As in Lesson 13, the potentiometer stands in e5 to e7, just left of the
    LCD's first pins, and three short jumpers carry GND, 5 V and its middle
    leg to VSS, VDD and V0. The rotary encoder module's pins are labeled CLK,
    DT, SW, + and GND: its + goes to the inner 5V pin at the top of the long
    header, its GND to the GND pin beside pin 13.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson29Menus**:

<!-- sketch -->

What's new:

- `adk::RotaryEncoder knob {18, 19};` names the encoder's CLK and DT pins.
  `adk::Button click {22};` is its push switch, and `adk::PwmOutput lamp {3};`
  is the LED, dimmed with PWM.
- `levels` and `modes` are the choices, as lists of text.
- `struct Item` is one line of the menu: its `name`, its `choices` and its
  `setting`. `adk::Span<const char* const> choices` is a view of a list kept
  somewhere else, as in Lesson 18, so every item can point at a list of a
  different length. The second `const` says the texts in the list are fixed
  too.
- `adk::Array menu { Item {...}, ... };` is the whole menu, an array of
  structs as in Lesson 5. Writing `Item` in front of each one says what they
  are, so the array knows its type without being told.
- `Item& level = menu[0];` makes `level` another name for the first item,
  with `&` just as in Lesson 3. So `level.setting` in `loop ()` says
  which setting it means, where `menu[0].setting` wouldn't.
- `current` is the item the arrow points at, and `editing` says which
  state the menu is in.
- `turnKnob ()` does the right thing for the state. Editing, it adds the
  clicks to the setting; browsing, it adds them to `current`. Either way
  `constrain` stops it at the first and the last.
- `showMenu ()` redraws the screen only when something changed: each item
  on its own row with its setting beside it, then the arrow in front of
  either the current item's name or its setting. `lcd.at (9, row)` moves to
  column 9 of that row and hands back the screen, so `.print ()` can follow
  straight on.
- The last lines of `loop ()` light the lamp. `blink` flips `blinkOn` every
  half second. The lamp is lit if the mode is Steady, choice 0, or if
  `blinkOn` is true, and then it gets `level.setting * 255 / 10`.

## Upload it

Upload the sketch. The LCD shows `>Level   60%` on top and `Mode    Steady`
below, and the LED glows at a bit over half brightness. If the text is faint
or missing, turn the potentiometer until it's sharp.

Turn the knob one click clockwise: the arrow moves down to `>Mode`. Click
the knob: the arrow jumps to `>Steady`. Turn it to `Blink`, and the LED
blinks, once a second. Click again, turn back up to `Level`, click, and
turn it down to 0%, or up to 100%. It blinks at whatever level you set.

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
| One click moves Level two steps, or it takes two clicks to move one | Your encoder makes a different number of changes per click. Try `adk::RotaryEncoder knob {18, 19, 2};`. |
| Clicking does nothing | Press the shaft straight down until it clicks, and check SW goes to pin 22. |
| The menu works but the LED never lights | Check pin 3's wire goes to j38, the resistor runs from g38 across the gap to e38, the LED's long leg is in b38 and its short leg in b39, and the black jumper runs from a39 to the − rail. |

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

1. **Breathe.** Add a third mode, `Breathe`, in which the lamp rises and
   falls smoothly. Time it with an `adk::Stopwatch`, as in Lesson 3:
   `elapsed () % 2000` counts from 0 to 1999 and starts again, so it says
   how far through a two-second breath the lamp is. Rise through the first
   second and fall through the next.
2. **A third item.** Add `Speed`, with the choices `Slow` and `Fast`, and
   set the blink's beat from it with `blink.period ()`. The screen has only
   two rows, so make the menu scroll: the current item on the top row, the
   next one below it.
3. **Finer steps.** Let Level go from 0% to 100% in steps of 5%: give
   `levels` 21 choices, and divide by 20 instead of 10 in `loop ()`.
4. **Remember.** Save each item's `setting` in EEPROM, as the safe in
   Lesson 18 saved its code, so the lamp wakes up the way you left it.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**, never in **10A**. Keep
each probe tip in its own hole, so it can't bridge two.

The encoder's wires run straight from the Mega to the module, out of the
probes' reach, so these readings are on the lamp, where the menu's settings
end up. Leave Mode on **Steady**, so the brightness holds still, and set
Level with the knob before each reading. The black probe goes in the bottom
− rail at column 40, just past the end of the LCD.

!!! question "Predict"
    At 60% the lamp gets 153 out of 255: as in Lesson 7, pin 3 switches
    fully on and off about 490 times a second, on at 5 V for 153 parts of
    every 255 and off at 0 V for the rest. What will the meter show? And at
    20%?

<!-- measure -->

What the numbers tell you:

- **Pin 3 at 60%** reads about 3 V. The meter is far too slow to follow
  490 switches a second, so it shows the average: 153 ÷ 255 × 5 V = 3 V.
- **At 20%**, `level.setting` is 2, the lamp gets 2 × 255 ÷ 10 = 51, and
  51 ÷ 255 × 5 V = 1 V. Each click of Level adds a tenth of 255, so each
  click is half a volt on the meter: turn it a click at a time from 0% and
  watch 0, 0.5, 1, 1.5 ... up to 5 V at 100%.
- **Across the LED at 60%** is about 1.2 V. A red LED held at a steady
  1.2 V would stay dark: it needs about 2 V to light. It glows because it is
  really at 2 V for 60% of the time and at 0 V for the rest, and 60% of 2 V
  is 1.2 V. PWM never dims the LED's voltage; it switches the LED fully on
  and off, and your eye, like the meter, sees the average.
- Now try **Blink**, with Level back at 60%: the meter jumps between
  about 3 V and 0 V, half a second each, too quick for it to settle.
