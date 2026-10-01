---
lesson: 49
promise: Type a code on a keypad by the door, and let a board inside decide whether the latch opens.
time: 90 minutes
level: 3
parts:
  - "Board A, the door: Lesson 48's Board A, with its LoRa modem, divider and screen"
  - "Board A: the 4×4 keypad from Lesson 16, and 8 jumper wires"
  - "Board B, inside: Lesson 48's Board B, with its LoRa modem and divider"
  - "Board B: the SG90 servo, the breadboard power module and its 9 V adapter, and the RGB LED with three 220 Ω resistors"
  - "Board B: 6 jumper wires and 2 female-to-male jumper wires"
  - "A small box and some tape, if you want a real latch"
ideas:
  - Which board should decide, and why
  - A key press crosses the air as a count, with its key
  - An answer that comes back
  - A secret that stays on one board
---

## What you'll build

<!-- closeup A -->

<!-- closeup B -->

A keypad by the front door, and a latch inside. Board A, at the door, has
the keypad and the screen; Board B, inside, has the latch and a colored
light. Type **1 2 3 4** on Board A and a star appears for each key, then
press **#**: Board B's servo swings the latch open, its light turns green,
and Board A's screen says **Open! # locks**. Get it wrong and the screen
says **Wrong! Tries: 1**, while Board B glows red. The code itself is only
ever on Board B.

As before, send on 915 MHz only where it's allowed: see
[Radios](../../safety.md#radios). In Europe the band allows a tenth of the
time, far more than a keypad's few messages need.

!!! warning "A classroom model"
    This is a model latch, not a lock to trust. Anyone nearby with a LoRa
    modem could read the keys as they cross the air: *How it works* says
    more.

## The idea

**Which board should decide?** The keypad has to be at the door, where
anyone can reach it. If Board A knew the code, a burglar could unscrew it,
plug it into a computer, and read the code out of it. And if Board A
decided and simply told Board B **open**, anyone who could make a radio
say the same thing could open the door. So Board A decides nothing. It
passes on every key, just as it was pressed, and shows whatever Board B
says. Board B, inside, keeps the code, compares the keys with it, and
works the latch. Take Board A away and all you have is a keypad and a
screen.

**A key press crosses as a count.** The bridge sends a value only when it
changes. That suits a knob, but not a key: press **1** twice, and the
second **1** is no change at all. Lesson 47 had the answer: count. Board A
counts every key pressed, `presses`, and the count goes up even when the
key is the same.

But Board B needs the key too, not just the count. Shared under two
names, the count and the key could travel in different messages, and a
lost message could leave a new count with an old key. So the bridge has a
way to share the two together, as one **event**:

```cpp
bridge.shareEvent ("key", presses, lastKey);
```

The first number is the count, and the second is what the event carries,
its **payload**: here, the key. On Board B, `bridge.changed ("key")` is
true once for each new key, even the same key twice, and
`bridge.payload ("key")` is the key that came with it. The bridge also remembers when the two boards found each other, so a
key pressed before then, or before either board restarted, never arrives
later as a surprise.

**An answer comes back.** Board B shares three numbers of its own:
`typed`, how many digits it holds; `door`, 1 while the latch is open; and
`wrong`, how many wrong codes in a row. Board A's screen shows those, not
what it thinks it has sent: a star appears only once Board B has the
digit. So if a key is lost on the way, as
[Lesson 43](../043-the-bridge/index.md#what-the-bridge-can-lose) says one
can be, you see it: its star never comes.

!!! question "Predict"
    Type a digit on Board A and watch the screen. Will the star appear the
    moment you press the key, or a little later? And if you unplug Board
    B's USB cable, what will Board A's screen do when you press keys?
    Write down your guesses.

## How the lock works

Every key goes to Board B, which does one of these:

| Key | Board B | Board A's screen |
|---|---|---|
| 0 to 9 | Adds the digit, up to four, unless the door is open | One more `*` |
| # | Checks the four digits: right opens the latch, wrong counts a try | `Open! # locks`, or `Wrong! Tries: 1` |
| # or \* while open | Closes the latch | `Locked. Code?` |
| \* | Rubs out the digits | `Locked. Code?`, or still `Wrong! Tries:` after a wrong code; no stars |

Board B's light says the same: dim blue while it waits, green while the
door is open, red after a wrong code, and dim orange while it can't hear
Board A at all.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables and the power module's adapter before
    you wire, and check your work before you plug them back in.

!!! danger "3.3 V for the modems; 5 V for the servo"
    Each modem's VDD goes to its own Mega's **3.3V** pin, never 5V, and its
    RXD only ever sees the Mega's TX pin through the 1 kΩ, with the 2 kΩ to
    GND. On Board B, the power module lies beside the board as in Lesson 17,
    never plugged into it, both its jumpers **off**, its red wire from
    **5V** to the bottom + rail and its black wire from **GND** to the
    bottom − rail, both by column 42: the servo takes its power from the
    bottom rails, never from the Mega.

Each board keeps its LoRa modem from Lesson 48 where it is, below the
board, with its divider and its wires: those stay where they are in every
two-board lesson (Lesson 51 moves one wire). The steps say what else to
keep and what to take out.

### Board A: the door

Board A keeps its screen from Lesson 48, and everything else comes out.
The keypad goes high above the Mega on pins 22 to 29, as in Lesson 16. Its
eight wires rise beside the Mega, and the wires from pins 14 and 15 to the
modem's divider cross them just below the keypad's plug.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: inside

Lesson 48's sensors and green LED come out. Board B carries the RGB LED
at its home, as in Lesson 34, and the servo below the board, powered from
the bottom rails as in Lesson 17. The servo keeps its Lesson 17 home below
the modem, and pin 44's wire runs down beside the board and under the
modem to reach it.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

??? info "Making the latch"
    As in Lesson 18: tape the servo inside a small box, just below the
    rim, so its arm swings out under the lid's edge. Upload Board B's
    sketch first, so the servo sits at 0°, the locked position, then press
    the arm on so it holds the lid shut. At 90° it should swing clear. Run
    the wires out through a small hole, and leave a finger hole so you can
    still open it while you're testing. It's a toy latch: it shows how a
    lock thinks, not how to keep a burglar out.

## Code it

Each board runs its own sketch. In the Arduino IDE, choose **File →
Examples → Adk → lessons → 049-remote-keypad**: it holds two, **Door** for
Board A and **Inside** for Board B.

### Board A: Door

<!-- sketch A -->

What's new:

- The modem and the bridge are Lesson 43's: address 1, talking to address
  2, at the Quick speed and 10 dBm.
- `presses` counts keys and `lastKey` remembers the latest. A key counts
  only while `bridge.isConnected ()`: one pressed while Board B can't hear
  would otherwise arrive much later, when nobody expects it.
- `bridge.shareEvent ("key", presses, lastKey)` shares the count and the
  key together, on every pass; the bridge sends them only when they
  change. A `char` is a number underneath, `'5'` is 53, so it crosses like
  any other.
- `bridge.changed ("typed")`, `"door"` and `"wrong"` say when Board B's
  answer changes, and `bridge.isConnected () != linked` when Board B is
  lost or found. Either way, `showAnswer ()` draws the screen again.
- `showAnswer ()` shows only what Board B says: `Calling B...` until it is
  heard, then `Open! # locks`, `Wrong! Tries: 2` or `Locked. Code?`, and a
  star for each digit B holds. The `for` loop counts from 0 up to
  `bridge.value ("typed")`, printing one `*` each time round.

### Board B: Inside

<!-- sketch B -->

What's new:

- `code` is the secret, as in Lesson 18. It is in this sketch only, and
  nothing ever sends it.
- `bridge.changed ("key")` is true once for each new key from Board A.
  `bridge.payload ("key")` is the key as a number: `'1'` is 49 and `'#'`
  is 35. `char (...)` makes it a character again, just as `char (223)`
  made the degree sign in Lesson 14: a type's name with a value in
  brackets makes a value of that type.
- `takeKey ()` is the table above, in code. `adk::equal (typed, code)`
  compares the digits with the code, as in Lesson 18.
- `typed.size ()`, `unlocked` and `wrong` go back to Board A through the
  bridge. A `bool` crosses as 1 or 0.
- `latch.moveTo ()` and `light.fadeTo ()` are called on every pass: asking
  for what they are already doing changes nothing, so the latch and the
  light simply follow `unlocked`, `wrong` and the link. The light's color
  is a chain of `? :` from Lesson 12: each `?` asks a question, and if the
  answer is no, the `:` goes on to the next. The first question answered
  yes picks the color, and `waiting` is what's left when none is.

## Upload it

1. Plug Board A into your computer, open **Door**, choose Board A's port
   under **Tools → Port**, and upload it. The screen says `Calling B...`.
2. Plug in Board B and the power module's adapter, and switch the module
   on. Open **Inside**, choose Board B's port, and upload it. (With one
   computer, both can stay plugged in: each has its own port, and
   [Two Megas on one computer](../043-the-bridge/index.md#two-megas-on-one-computer)
   says which is which.) The servo swings to 0°. Within a second or two,
   Board B's light turns dim blue and Board A's screen says
   `Locked. Code?`.
3. Type **1 2 3 4** on Board A. A star appears for each key, a moment
   after you press it. Press **#**: the latch swings open, the light turns
   green, and the screen says `Open! # locks`. Press **#** again to lock.
4. Type **5 5 5 5 #**: `Wrong! Tries: 1`, and a red light. Another wrong
   code makes it `Tries: 2`; the right one sets it back to none.

You predicted when the star would come. It comes a tenth of a second or
two after the key: Board A's bridge sends the key, the radio carries it,
Board B adds the digit and shares its new `typed`, and the radio carries
that back. Unplug Board B, and after five seconds of silence Board A's
screen says `Calling B...`. Keys pressed now go nowhere. Plug Board B back
in: once it has started, the screen says `Locked. Code?` again, and you can
type straight away.

!!! question "Predict: a restart"
    Type **1 2**, then press Board A's **RESET** button. When Board A has
    started again, how many stars will its screen show?

Two. Board A forgot everything, but Board B didn't restart: it still holds
the two digits, and Board A's screen shows what Board B says. Press **\***
to rub them out. Neither board needed resetting to carry on.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board A always says `Calling B...` | Is Board B's sketch running, with its light on? Check each modem as Lesson 43 did: VDD on the 3.3V pin, GND in the bottom − rail by column 24, TXD in f26 beside pin 15's wire in j26, RXD in c28, and the divider in column 28. |
| Board B's light stays dim orange | Board B can't hear Board A. The same checks, on Board A's modem; and are both sketches from this lesson, with addresses 1 and 2? |
| Keys don't make stars | Check the keypad as in Lesson 16: its ribbon on pins 22 to 29, in order. |
| A key is sometimes missed | Two keys pressed within a tenth of a second travel as one, and only the second arrives. Type at a steady pace and watch the stars; if one is missing, press **\*** and start again. |
| The servo doesn't move | Is the power module on, with its LED lit? Check its red wire from 5V to the bottom + rail and its black wire from GND to the bottom − rail, both by column 42, then the servo's red wire in the bottom + rail by column 35, its brown in the bottom − rail by column 36, and pin 44's wire to its orange. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The bridge's messages are plain text. When you press **5** as your
    third key, Board A's modem sends:

    ```text
    @1/1 key=3:53
    ```

    and Board B answers:

    ```text
    @1/1 typed=3
    ```

    The colon keeps the count, 3, and the key, 53, in one word, so they
    always arrive together. Only what changed goes in each message. Every
    two seconds each board sends everything it shares again,
    `@1/1 typed=3 door=0 wrong=0`, so a message lost to noise is soon made
    good, and each knows the other is still there. A repeat brings no new
    count, so it is never a new key.

    That also shows the weak spot of this lock: anyone nearby with a LoRa
    modem on the same settings could read the keys as they cross the air.
    Real wireless locks scramble their messages with a secret key, so a
    listener hears only noise and can't send a message of their own. This
    one is for learning how the pieces fit, not for a real door.

## Make it yours

1. **Three strikes.** On Board B, ignore every key for 30 seconds after the
   third wrong code in a row, and share how many seconds are left, so
   Board A's screen can count them down. Which board should keep the
   time?
2. **A new code, from inside.** Let Board B change its code, as Lesson 18
   did: while the door is open, **A** on the keypad starts a new code and
   **#** saves it. Keep it in Board B's EEPROM, so it survives a power cut.
   Board A needs no change at all. Why not?
3. **Six digits.** Give `code` six digits and `typed` room for six. What
   has to change on Board A?

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. The readings are on
Board B's RGB LED pins, whose wires land in row j above their resistors.

!!! question "Predict"
    Board B's light glows dim blue while it waits: the blue pin is on for
    40 parts in 255. What will a meter on the blue pin read?

<!-- measure B -->

What the numbers tell you:

- **The blue pin, waiting,** reads about 0.8 V: 40/255 of 5 V. The pin is
  really switching fully on and off hundreds of times a second, as in
  Lesson 7, and the meter shows the average.
- **The green pin, open,** reads about 5 V: green at full brightness is
  on all the time.

## Check yourself

1. Why does Board A share each key with a count, and not just the key?
2. Why does Board A's screen show the stars Board B says it holds, and
   not the keys Board A has sent?
3. Why does the code live only on Board B?

??? note "Answers"
    1. The bridge sends a value only when it changes, so pressing **1**
       twice would send nothing new. The count goes up with every press,
       and `shareEvent ()` keeps each key with its own count.
    2. A key can be lost on the way. Showing Board B's answer means a lost
       key shows as a missing star, and you can start again.
    3. Board A is outside, where anyone can reach it. A burglar could read
       the code out of it, or make it say open. Board B, inside, decides.
