---
lesson: 18
title: Keypad Safe
arc: Keys and motion
promise: Build a safe that opens only for your secret code, and remembers the code even when it's unplugged.
time: 120 minutes
level: 3
sketch: Lesson18KeypadSafe
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13
  - The 4×4 keypad from Lesson 16
  - SG90 servo
  - Breadboard power module and its 9 V adapter
  - Active buzzer
  - 28 jumper wires
  - A small cardboard box and some tape, if you want a real safe
ideas:
  - A lock as a set of states
  - Stopping guessers with a lockout
  - Memory that survives the power going off (EEPROM)
  - A servo as a latch
---

## What you'll build

<!-- closeup -->

A safe with a secret code. The screen asks **Locked. Code?**, each key clicks
as you type, and stars hide the digits from anyone looking over your
shoulder. Get it right and the servo swings its arm out of the way: the lid is
free. Get it wrong three times and the keypad goes dead for thirty seconds,
counting down on the screen. And you can change the code, which the Mega
remembers even after it has been unplugged for a year.

## The idea

**A lock is a small set of states.** At any moment the safe is in exactly
one of them, and each key does something different depending on which. You
met states in Lesson 3's Reaction Duel; here they are:

- **Locked**: the arm blocks the lid, and digits are a guess.
- **Open**: the arm is out of the way.
- **Choosing**: you're typing a new code.
- **Waiting**: too many wrong guesses, so the keys are ignored for a while.

**A lockout beats guessing.** A four-digit code has 10 × 10 × 10 × 10 =
10,000 possibilities. Trying one every five seconds, a guesser would find
yours after about 5,000 tries, in about seven hours. Make them wait 30 seconds
after every three wrong tries and each try costs 15 seconds instead of five:
now it takes about 21 hours.

**EEPROM keeps the code.** Everything in the Mega's ordinary memory, its
variables, vanishes when the power goes. But the Mega also has 4,096 bytes of
**EEPROM**, memory that keeps what's written in it with the power off, for
many years. Each byte holds a number from 0 to 255, so the sketch keeps a
four-digit code in two of them: 1234 is stored as 12 and 34, and read back as
12 × 100 + 34. A third byte, a "mark", says whether a code has been saved at
all: a fresh EEPROM byte reads 255.

EEPROM wears out, slowly: each byte can be rewritten about 100,000 times. The
sketch writes with `EEPROM.update ()`, which only writes a byte when its value
actually changes, so even changing your code ten times a day would take 27
years to wear it out.

!!! question "Predict"
    Once it works, you'll change the code to 2468, then unplug everything and
    plug it back in. Which code will open the safe then: 1234 or 2468? And if
    you upload the sketch again, which then? Write down your guesses.

## How the safe works

The sketch starts **Locked**, with the code it finds in EEPROM, or 1234 if
none has been saved. Every digit you type clicks the buzzer.

| State | The screen says | Key | What happens |
|---|---|---|---|
| Locked | `Locked. Code?` | 0 to 9 | A `*` appears; up to four digits |
| Locked | | # | Right code: **Open**. Wrong: a long beep and `Wrong! Try again`, or, at the third wrong code in a row, **Waiting** |
| Locked | | \* | Rub out the digits and start again |
| Open | `Open. # locks` | # | The arm swings back: **Locked** |
| Open | | A | **Choosing** |
| Choosing | `New code, then #` | 0 to 9 | The digit itself appears, so you can check it |
| Choosing | | # | After four digits, save the code in EEPROM: **Open** |
| Waiting | `Too many tries` | any | Nothing, while `Wait 30 s` counts down; then **Locked** |

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire.
    In this build the power module powers everything on the breadboard, so
    **nothing** connects to the Mega's 5V pin: if you still have Lesson 13's
    red wire from 5V to the top + rail, take it out. The screen's black GND
    wire joins the Mega's GND to the rails. Plug the power module in at the
    far end with its **+** and **−** matching the rails' stripes on both
    sides, and set both jumpers to **5V**. Keep fingers clear of the servo's
    arm.

<!-- bench -->

<!-- steps -->

??? info "Making the box"
    Tape the servo inside a small cardboard box, just below the rim, with its
    shaft pointing at the lid. Upload the sketch first, so the servo sits at
    0°, the locked position, then press the horn onto the shaft so that it
    sticks out under the edge of the lid, holding it shut. At 90° it should
    swing clear. If your box needs different angles, change `closedAngle`
    and `openAngle` in the sketch. Run the wires out through a small hole,
    and leave a finger hole or a string so you can still open it by hand while
    you are testing.

    It's a toy safe: anyone who can lift the whole box or pull a wire can
    open it. It's here to show how a lock thinks.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson18KeypadSafe**:

<!-- sketch -->

Reading it from the top:

- `#include <EEPROM.h>` brings in Arduino's EEPROM library, with its
  `EEPROM.read (address)` and `EEPROM.update (address, value)`.
- `enum State` names three states. The fourth, Waiting, is simple enough to
  be a pause inside `refuse ()`.
- `loop ()` reads one key and hands it on, depending on the key and the state.
  The rules are the table above.
- `typeDigit ()` builds the number the way the calculator in
  [Lesson 16](../16-keypad/index.md) did, and shows a `*`, or the digit when
  you are choosing a new code.
- `pressEnter ()` decides what `#` means right now. A code only counts with
  all four digits typed, so 123 and 0123 are different codes.
- `refuse ()` counts wrong codes. On the third, it counts down 30 seconds with
  `adk::wait (1000)`. Keys pressed meanwhile are scanned but never read, so
  they do nothing.
- `enter (Open, "Open. # locks")` is how the safe changes state. It moves
  the latch to match the new state, writes the message on the top row, and
  clears the bottom row for typing. Every change goes through it, so the
  latch and the screen can never disagree.
- `loadCode ()` and `saveCode ()` are the EEPROM: address 0 holds the mark,
  42, once a code is saved; addresses 1 and 2 hold its two halves.

## Upload it

Plug in the USB cable, then the power module's adapter, and switch the module
on. Upload the sketch. The arm swings to 0° and the screen says
`Locked. Code?`.

Type **1 2 3 4** and press **#**. Each key clicks and puts a star on the
screen; then the arm swings to 90° and the screen says `Open. # locks` and
`A: new code`. Press **#** and it locks again.

Now type a wrong code three times. Each wrong code gets a long beep, and the
third one blanks the keypad: `Too many tries`, then `Wait 30 s` counting
down, and the keys do nothing until it reaches zero.

Test your prediction: open the safe, press **A**, type **2 4 6 8** and **#**.
The screen says `New code saved`. Unplug the adapter and the USB cable, plug
them back in, and only 2468 will open it. Upload the sketch again and it's
still 2468: uploading a sketch doesn't touch the EEPROM.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo never moves | Is the power module on, with its LED lit and both jumpers on 5V? Check the servo's red and black wires reach the bottom rails (B+45, B-46) and its orange wire pin 44. |
| The screen is dark and nothing works | The screen now takes its power from the power module: switch it on. |
| The screen is blank, but the backlight is on | Turn the contrast knob. |
| Keys come out wrong | See Lesson 16's "Which way round is the ribbon?" |
| The right code says `Wrong!` | You may have saved a different code. If you've forgotten it, change `savedMark` to 43 and upload: the sketch then ignores the saved code and starts again from 1234. |
| No clicks or beeps | Check the buzzer's + leg is in f23 with pin 12's wire in j23, and j26 goes to the − rail. |
| The servo buzzes when locked | It's pressing against its stop or the lid. Try a `closedAngle` of 10. |
| The Mega resets when the servo moves | Something takes power from the Mega's 5V pin. Nothing should in this build. |

??? note "How it works"
    The Mega 2560's EEPROM is 4,096 bytes inside the chip, separate from the
    flash that holds your sketch and the RAM that holds its variables. Writing
    one byte takes about 3.3 ms, so `EEPROM.update ()` first reads the byte
    and skips the write when it would change nothing. Uploading a sketch
    rewrites the flash but leaves the EEPROM alone, which is why the saved
    code survives it.

    `adk::wait ()` keeps calling `adk::update ()` while it waits, so during
    the lockout the servo holds its position, the long beep ends on time and
    the keypad is still scanned. The keys it sees are events that last one
    update each, and nothing reads them, so they are gone.

## Make it yours

1. **Auto-lock.** Lock the safe again by itself 20 seconds after it opens.
   An `adk::Every` from Lesson 11, restarted when the safe opens, is one way.
2. **Longer lockouts.** Double the wait after each lockout: 30 s, then 60,
   then 120. Should a correct code reset it?
3. **Can't unplug your way out.** At the moment, unplugging the Mega forgets
   how many wrong codes were typed. Keep the count in EEPROM too, so a
   guesser can't reset it.
4. **Six digits.** Allow six-digit codes. An `int` can't hold 999999, but a
   `long` can, and the code then needs three bytes of EEPROM.
