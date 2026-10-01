---
lesson: 51
promise: Build a front door that tells you inside who is there, by doorbell, knock or card, and lets them in.
time: 2 hours
level: 3
parts:
  - "Both boards: the Mega, breadboard and LoRa modem from Lesson 50, with the modem's divider and wires"
  - "Board A: the LCD, knob and 220 Ω resistor from Lesson 13, a push button, the passive buzzer with a 220 Ω resistor, the servo, and a breadboard power module with its 9 V adapter"
  - "Board B: the RC522 RFID reader with its card and fob, the tap sensor (37 in 1), a push button, the active buzzer, and the power module from Lesson 50 (Board A's is a second one: the kit has only one)"
  - "Board B: 4 × 1 kΩ and 4 × 2 kΩ resistors for the reader, plus an S8050 transistor, 1 kΩ base resistor, 10 kΩ pull-down resistor and 1N4007 diode for the buzzer"
  - "Board A: 26 jumper wires and 6 female-to-male wires in total, including the modem's; Board B: 15 jumper wires and 16 female-to-male wires in total"
  - "A box with a lid, and some tape, if you want a real door"
ideas:
  - Three events, each crossing as a count
  - Why the latch lives inside
  - A card's number in a long
  - Putting two arcs together
---

## What you'll build

<!-- closeup A -->

<!-- closeup B -->

A front door and the hall behind it. Board B is the outside of the door:
a doorbell button, a tap sensor to hear knocks, and the card reader from
Lesson 34. Board A is inside: a screen, a chime, a button, and the latch.
Press the doorbell and Board A's screen says **Ding dong!** as it chimes;
press Board A's button to let your visitor in, and the latch swings open
while Board B's buzzer buzzes, like the front door of a block of flats.
Knock, and the screen says **Knock knock!**. Hold your card to the reader
and Board A says **Welcome home, Ada**, plays a little tune, and opens the
door by itself. A card it doesn't know gets **Unknown card**, and the door
stays shut.

As before, send on 915 MHz only where it's allowed: see
[Radios](../../safety.md#radios). In Europe the band allows a tenth of the
time, far more than a doorbell's few messages need.

## The idea

Almost every piece is one you know: the card and the knock from Lessons
34 to 36, the latch from Lesson 18, the keypad lock's rule from Lesson 49
that the board outside decides nothing, and the bridge between them.
What's new is how they fit together.

**Three events, three counts.** A ring, a knock and a card are all events,
so, as in Lesson 49, each crosses the air as a count that goes up:
`rings`, `knocks` and `cards`, each shared with `shareEvent ()`. A card
also carries its number, for Board A to look up; a ring or a knock
carries nothing more. Inside, `bridge.changed ("rings")` is true once for
each new ring, and the same for the others. A ring from before the boards
found each other never arrives: switch Board A on after the bell has rung
three times, and it says nothing until the bell rings again.

**Why the latch lives inside.** In Lesson 36 one board had the reader and
the latch. Here Board B, at the door, has the reader and a modem, and both
run on 3.3 V. The Mega's 3.3V pin can give about 50 mA; the reader can
draw up to about 26 mA, and the modem draws tens of milliamps each time it
sends. Together they would ask too much of that pin. So on Board B, the
modem takes its 3.3 V from a power module instead, by an orange wire from
the module's **3.3V** pin to the bottom rails, as in Lesson 40. Those rails
then can't give a servo its 5 V, so the latch goes inside, to Board A,
whose own power module feeds its bottom rails from its **5V** pin. That
suits Lesson 49's rule anyway: the board outside knows no friends, holds
no latch, and only reports what happens. Board A tells Board B one thing
back, `door`, 1 while the latch is open, and Board B buzzes when it
changes to 1.

**A card's number in a long.** A card's number fills 32 bits, all of a
`long`. But a `long` uses its top bit to say whether the number is below
zero, so a card number from 0x80000000 up crosses as a negative number:
Sam's 0x9ABCDEF0 goes as −1698898192. The bits are the same; only the
way of reading them differs. `int32_t (...)` on Board B reads the card's
number as a `long` would, and `uint32_t (...)` on Board A reads the bits
back as a card's number, which is never negative.

!!! question "Predict"
    Ring Board B's doorbell three times, then press Board A's **RESET**
    button, the small one beside its USB socket. When Board A has started
    again and hears Board B, will its screen say **Ding dong!**? Write
    down your guess.

## How the door works

| At the door, Board B | Crosses the air | Inside, Board A |
|---|---|---|
| Someone presses the doorbell | `rings` goes up | `Ding dong!`, a two-note chime |
| Someone knocks | `knocks` goes up | `Knock knock!`, a tap on the chime |
| A friend's card | `cards` goes up, carrying the card number | `Welcome home,` and the name, a tune, and the latch opens for 5 s |
| A stranger's card | `cards` goes up, carrying the card number | `Unknown card`, a low note, and its number on the Serial Monitor |
| The buzzer buzzes for a second | `door` becomes 1, the other way | The latch opens, for a friend's card or when you press the button: `Door open` |

News stays on Board A's screen for ten seconds; then it says
`Front door` and `All quiet`, or `Can't hear it` while Board B is silent.
Board B's **L** LED is lit while it can hear Board A.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables and both power modules' adapters before
    you wire, and check your work before you plug them back in.

!!! danger "Two power modules, wired differently"
    - **Board A:** the red wire from the module's **5V** pin to the
      bottom + rail by column 42, for the servo.
    - **Board B:** the orange wire from the module's **3.3V** pin to the
      bottom + rail by column 42, for the modem. Never 5V: more than 3.6 V
      damages the modem. Board B's module fed 5 V in Lesson 50, so take
      its red wire off the **5V** pin first, before anything else on
      Board B.

    On both boards the module lies beside the breadboard, never plugged into
    it, both its jumpers **off**, and its black wire goes from its **GND**
    pin to the bottom − rail by column 42. Label the two boards, so their
    power modules never swap. The RFID reader takes **3.3 V** from Board B's
    own 3.3V pin, never 5V; Lesson 34 explains its signal wires.

Both boards keep their LoRa modems where they were, with the same
dividers and wires, but for one change on Board B: its modem's VDD moves
from the Mega's 3.3V pin to the bottom + rail by column 29, right above it.

### Board A: inside

The GY-521 and its level shifter come off, and the modem's VDD goes back to
the Mega's 3.3V pin. The screen goes back to its home, taking 5 V from the
Mega on the top rails. The button on pin 23 stands in columns 8 to 10, and
the passive buzzer on pin 10 stands in column 33 with its 220 Ω resistor.
A power module lies to the right of the board, its red wire from **5V**
to the bottom + rail and its black wire from **GND** to the bottom − rail,
both by column 42, and the servo goes below the board at its home.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: the door

The LED matrix and the servo come off; the power module and the modem
stay, but the module's red wire comes off its 5V pin, and an orange wire
goes from its **3.3V** pin to the bottom + rail by column 42. The Mega's 5V
wire goes to the top rails again: these power the active buzzer through
its transistor driver. The reader and the tap sensor lie below the Mega in
their places from Lesson 36, wired the same way. The doorbell is the button
on pin 22 at its home, and the active buzzer stands at its home in column
33, with the S8050 driver, base resistor, pull-down and diode from
Lesson 47.

The reader still takes 3.3 V from the Mega. Its four inputs, SDA, SCK,
MOSI and RST, each need Lesson 34's 1 kΩ/2 kΩ divider in columns 18, 20,
22 and 24. Do not connect those inputs directly to the Mega's 5 V signals.
MISO goes directly back to pin 50.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

??? info "Making the door"
    Board A's servo is the bolt, on the inside of a box's lid, as in Lesson
    18: tape it just below the rim so its arm swings out under the lid's
    edge, with the sketch holding it at 0° to lock and 90° to open. Board B
    sits outside the box: tape the reader where a card held against the
    front is a centimeter or two from it, and the tap sensor to the lid, so
    it feels a knock. Keep Board B's buzzer off the tap sensor's board:
    Board B ignores knocks while it buzzes, but not after.

## Code it

In the Arduino IDE, choose **File → Examples → Adk → lessons →
051-doorbell-and-door**: **Inside** is for Board A and **Door** for
Board B.

### Board A: Inside

<!-- sketch A -->

Read it from the top:

- `friends` is Lesson 36's list of cards and names, now kept inside. The
  two numbers are stand-ins: put in your own, from Board A's Serial
  Monitor.
- `dingDong`, `knock`, `welcome` and `stranger` are four little tunes for
  the chime, as in Lesson 5.
- `unlocked` runs for five seconds each time the door opens, and the
  latch follows `unlocked.isRunning ()` on every pass: open while it runs,
  shut once it stops. `news` runs while the screen shows news.
- `bridge.changed ("cards")`, `("rings")` and `("knocks")` are each true
  once for each new event at the door. If two arrive together, the card
  comes first, then the bell, then the knock.
- `uint32_t (bridge.payload ("cards"))` turns the number that crossed the
  air back into a card's number, as *The idea* explains, and
  `checkCard ()` looks it up in `friends`, as in Lesson 36. A friend is
  welcomed and the door opens; a stranger's number goes to the Serial
  Monitor as eight hex digits, `adk::hex (card, 8)`, as in Lesson 34.
- `tell ()` puts news on the screen for ten seconds, and `showQuiet ()`
  says the door is quiet, or that Board B can't be heard.
- `bridge.share ("door", unlocked.isRunning ())` tells Board B whether the
  door is open.

### Board B: Door

<!-- sketch B -->

Read it from the top:

- The reader, the tap sensor and its `rattle` are Lessons 34 and 35's. The
  bell is a button, and the buzzer and the **L** LED are Board B's only
  outputs.
- `setup ()` says on the Serial Monitor if the reader doesn't answer.
- `loop ()` counts rings, knocks and cards, and shares each count with
  `shareEvent ()` on every pass. A ring or a knock has nothing more to
  say, so its payload is 0; a card's payload is its number,
  `int32_t (reader.uid ())`, as *The idea* explains.
- A knock heard while the buzzer sounds is skipped: the buzzer shakes the
  board, and the tap sensor would hear it.
- `bridge.changed ("door") && bridge.value ("door") == 1` is true once,
  when Board A opens the door, and `buzzer.beep (1000)` buzzes for a
  second.

## Upload it

1. Check the power modules' supply wires: Board A's red one from its
   **5V** pin, Board B's orange one from its **3.3V** pin, and both jumpers off on each. Plug in
   both adapters and switch both modules on before you plug in the USB
   cables: Board B's sketch sets its modem up only as it starts.
2. Upload **Inside** to Board A and **Door** to Board B, each by its own
   port, as in
   [Two Megas on one computer](../043-the-bridge/index.md#two-megas-on-one-computer).
   Board A's latch swings to 0°, its screen says `Front door` and, once
   it hears Board B, `All quiet`. Board B's **L** LED lights.
3. Press the doorbell: `Ding dong!` and the chime. Press Board A's button:
   `Door open`, the latch swings open, and Board B buzzes. Five seconds
   later the latch shuts.
4. Knock on Board B's tap sensor, or the lid it's taped to: `Knock knock!`.
5. Open the Serial Monitor on Board A's port at 9600 baud, and hold your
   card to Board B's reader. It isn't known yet: `Unknown card`, a low
   note, and its number, such as `Card 0x1A2B3C4D`. Copy the number into
   `friends` in **Inside** with your name, and upload **Inside** again.
   Board B needs no change: the list lives inside.
6. Hold the card again: `Welcome home,` and your name, a tune, the latch
   opens, and Board B buzzes.

You predicted what Board A would say after its reset. Nothing: the three
rings happened before the boards found each other again, so the bridge
never passes them on. Ring once more, and that ring is news.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board A says `Can't hear it` | Is Board B's power module on, with its LED lit? If you switched the module on after Board B started, press Board B's **RESET** button. Its modem runs from the bottom rails now: check the module's orange wire from its 3.3V pin to the bottom + rail and its black wire from GND to the bottom − rail, both by column 42. Check the modem's VDD wire goes to the bottom + rail by column 29, and each modem's wires as in Lesson 49. |
| Board B's modem gets warm | Unplug everything at once, and check the orange wire of Board B's power module comes from its **3.3V** pin, never 5V. |
| Board B's Serial Monitor says `No card reader` | Check the reader's seven wires as in Lesson 34, and that its 3.3V pin goes to the Mega's 3.3V. |
| Your card always gets `Unknown card` | Copy its number from Board A's Serial Monitor exactly, with `0x` in front, into **Inside**, and upload it to Board A. |
| Knocks never reach Board A | Check the tap sensor's S goes to A12, + to the power header's 5V and − to its GND, and knock close to it. |
| The doorbell does nothing | Check pin 22's wire in j2, the button across the gap in columns 2 to 4, and the black wire from a4 to the − rail. |
| The latch doesn't move | Is Board A's power module on, with its LED lit? Check its red wire from 5V to the bottom + rail and its black wire from GND to the bottom − rail, both by column 42, then the servo's wires in the bottom + rail by column 35 and the bottom − rail by column 36, and pin 44's to its orange. |
| No chime | Check the passive buzzer's + in f33 with pin 10's wire in j33, and the 220 Ω from a33 to the − rail. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. |

??? note "How it works"
    When Sam's card is held to the reader, Board B's modem sends:

    ```text
    @1/1 cards=4:-1698898192
    ```

    Board A turns −1698898192 back into 0x9ABCDEF0, finds Sam, and
    answers with the door:

    ```text
    @1/1 door=1
    ```

    Five seconds later it sends `@1/1 door=0`, and every two seconds each
    board repeats everything it shares, such as
    `@1/1 rings=2:0 knocks=5:0 cards=4:-1698898192`, so a lost message is
    soon made good. A repeat brings no new count, so it's never news.

    A card's number and a knock are easy to copy, and anyone nearby with a
    LoRa modem could read these messages, or send a `cards` of their own.
    This door is for learning how the parts fit, not for keeping anyone
    out.

## Make it yours

1. **The secret knock.** Let Lesson 35's knock open the door too. Only
   Board B hears the gaps between knocks: Board A gets counts, a tenth of
   a second late at best. So have Board B turn the rhythm into a number,
   1 for each short gap and 2 for each long one, so `SLS` is 121, and
   share it when the knocking stops. Which board should know the secret?
2. **Visitor's book.** Count how many times each friend has come home,
   and show it on the screen: `Welcome home, Ada (7)`.
3. **Nobody home.** Add a switch to Board A, or use a long press of its
   button, that tells visitors on Board B's buzzer, with three short buzzes,
   that nobody will answer.
4. **Who rang?** Let Board A remember the last five events with the time
   each came, and show them one by one with its button.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take each reading with
that board's power module on, and keep each probe tip in its own hole: the
rails' + and − holes are only 2.5 mm apart.

!!! question "Predict"
    The two boards' bottom rails look the same. What will each read?

<!-- measure A -->

<!-- measure B -->

What the numbers tell you:

- **Board A's bottom rails** read about 5 V: the latch's supply. A modem
  on these rails would be damaged, which is why each modem's VDD wire
  goes only where its lesson says.
- **Board B's bottom rails** read about 3.3 V, from the power module's own
  3.3 V regulator: the modem's supply, and never enough for a servo.

## Check yourself

1. Why does the latch live on Board A, inside, and not at the door?
2. Sam's card, 0x9ABCDEF0, crosses the air as −1698898192. Why, and how
   does Board A get the card's number back?
3. You ring three times, then reset Board A. Why doesn't it say
   **Ding dong!** when it starts?

??? note "Answers"
    1. Board B's reader and modem both need 3.3 V, so its power module
       gives 3.3 V, never enough for a servo. And the board outside should
       decide nothing.
    2. A `long` uses its top bit to say whether a number is below zero.
       The bits are the same; `uint32_t (...)` reads them back as a card's
       number.
    3. Those rings happened before the boards found each other again, so
       the bridge never passes them on. The next ring is news.
