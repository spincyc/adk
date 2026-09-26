---
lesson: 49
promise: Type a code on a keypad by the door, and let a board inside decide whether the latch opens.
time: 90 minutes
level: 3
parts:
  - "Both boards: an Arduino Mega 2560 with its USB cable, a breadboard, and the LoRa modem with its 1 kΩ and 2 kΩ resistors from Lesson 48 (the modems are add-ons; the second Mega and breadboard aren't in one kit)"
  - "Board A: the LCD, knob and 220 Ω resistor from Lesson 13, and the 4×4 keypad from Lesson 16"
  - "Board B: the SG90 servo, the breadboard power module and its 9 V adapter, and the RGB LED with three 220 Ω resistors"
  - "8 female-to-male and 36 jumper wires in all, the modems' among them"
  - "A small box and some tape, if you want a real latch"
ideas:
  - Which board should decide, and why
  - A key press crosses the air as a count
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

**A key press is an event, and an event crosses as a count.** The bridge
from Lesson 43 keeps a value the same on both boards, and sends it only
when it changes. That is just right for a knob or a tilt, but not for a
key: press **1** twice, and the second **1** is no change at all. So
Board A keeps count of every key pressed, `presses`, and shares it
together with the last key, `key`. Board B watches the count. When it
goes up by one, a new key has arrived, even if it is the same key as
before. If it jumps by more, or goes back to zero, a message was lost or
a board restarted, so Board B only catches up with the count.

**An answer comes back.** The bridge works both ways, so Board B shares
three numbers of its own: `typed`, how many digits it holds; `door`, 1
while the latch is open; and `wrong`, how many wrong codes in a row.
Board A's screen shows those, not what it thinks it has sent: a star
appears only once Board B has heard the key.

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
| \* | Rubs out the digits | `Locked. Code?`, no stars |

Board B's light says the same: dim blue while it waits, green while the
door is open, red after a wrong code, and dim orange while it can't hear
Board A at all.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables and the power module's adapter before
    you wire, and check your work before you plug them back in.

!!! danger "3.3 V for the modems; 5 V for the servo"
    Each modem's VDD goes to its own Mega's **3.3V** pin, never 5V, and
    its RXD only ever sees the Mega's TX pin through the 1 kΩ, with the
    2 kΩ to GND. On Board B, set the power module's **top** jumper to
    **OFF** and its **bottom** jumper to **5V**: the servo takes its power
    from the bottom rails, never from the Mega.

Each board keeps its LoRa modem from Lesson 48 where it is, below the
board, with its divider and its wires: those stay the same in every
two-board lesson. The steps say what else to keep and what to take out.

### Board A: the door

Board A carries the screen at its home, wired as in Lesson 13, and the
keypad high above the Mega on pins 22 to 29, as in Lesson 16. The
keypad's eight wires rise beside the Mega, and the wires from pins 14 and
15 to the modem's divider cross them just below the keypad's plug.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: inside

Board B carries the RGB LED at its home, as in Lesson 34, and the servo
below the board, powered from the bottom rails as in Lesson 17. The servo
lies a little lower than it did then, below the modem, and pin 44's wire
runs down beside the board and under the modem to reach it.

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
- `presses` counts every key, and `lastKey` remembers the latest. Both are
  shared on every pass of `loop ()`. The bridge sends only what changed,
  in one message, so a new key always arrives with its count; the same
  key again sends the count alone, and Board B still has the key.
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
- `presses` is Board A's count as Board B last heard it. When
  `bridge.value ("presses")` is exactly one more, `takeKey ()` gets the
  key; anything else only updates `presses`.
- `bridge.value ("key")` is a whole number, the key's character code: `'1'`
  is 49 and `'#'` is 35. Handing it to `takeKey (char key)` turns it back
  into a character.
- `takeKey ()` is the table above, in code. `adk::equal (typed, code)`
  compares the digits with the code, as in Lesson 18.
- `typed.size ()`, `unlocked` and `wrong` go back to Board A through the
  bridge. A `bool` crosses as 1 or 0.
- `latch.moveTo ()` and `light.fadeTo ()` are called on every pass: asking
  for what they are already doing changes nothing, so the latch and the
  light simply follow `unlocked`, `wrong` and the link. The `? :` chain
  picks the first color whose question is true, top to bottom.

## Upload it

1. Plug Board A into your computer, open **Door**, choose Board A's port
   under **Tools → Port**, and upload it. The screen says `Calling B...`.
2. Plug in Board B and the power module's adapter, and switch the module
   on. Open **Inside**, choose Board B's port, and upload it. (With one
   computer, both can stay plugged in: each has its own port.) The servo
   swings to 0°. Within a second or two, Board B's light turns dim blue
   and Board A's screen says `Locked. Code?`.
3. Type **1 2 3 4** on Board A. A star appears for each key, a moment
   after you press it. Press **#**: the latch swings open, the light turns
   green, and the screen says `Open! # locks`. Press **#** again to lock.
4. Type **5 5 5 5 #**: `Wrong! Tries: 1`, and a red light. Another wrong
   code makes it `Tries: 2`; the right one sets it back to none.

Now your prediction. Each star comes a tenth of a second or two after the
key: Board A's bridge sends the key, the radio carries it, Board B adds
the digit and shares its new `typed`, and the radio carries that back.
Unplug Board B, and after five seconds of silence Board A's screen says
`Calling B...`. Keys pressed now go nowhere, and no stars appear: Board A
never shows a key that Board B hasn't taken.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board A always says `Calling B...` | Is Board B's sketch running, with its light on? Check each modem as Lesson 43 did: VDD on the 3.3V pin, GND in B-42, TXD in f44 beside pin 15's wire in j44, RXD in c46, and the divider in column 46. |
| Board B's light stays dim orange | Board B can't hear Board A. The same checks, on Board A's modem; and are both sketches from this lesson, with addresses 1 and 2? |
| Keys don't make stars | Check the keypad as in Lesson 16: its ribbon on pins 22 to 29, in order. A key pressed while the boards can't hear each other is lost; press it again. |
| A key is sometimes missed | Two keys pressed within a tenth of a second can share a message, and Board B skips a jump in the count. Type at a steady pace; a missed key means a wrong code, never a wrong opening. |
| The servo doesn't move | Is the power module on, with its bottom jumper on 5V? Check the servo's red wire in B+53, its brown in B-54, and pin 44's wire to its orange. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The bridge's messages are plain text. When you press **5** as your
    third key, Board A's modem sends:

    ```text
    @presses=3 key=53
    ```

    and Board B answers:

    ```text
    @typed=3
    ```

    Only what changed goes in each message. Every two seconds each board
    sends everything it shares again, `@typed=3 door=0 wrong=0`, so a
    message lost to noise is soon made good, and each knows the other is
    still there.

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
