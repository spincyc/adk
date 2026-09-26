---
lesson: 18
promise: Build a safe that opens only for your secret code, and remembers the code even when it's unplugged.
time: 120 minutes
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Lesson 17's screen, servo and power module (with its 9 V adapter), wired as before
  - The 4×4 keypad from Lesson 16
  - Active buzzer
  - 10 more jumper wires
  - A small cardboard box and some tape, if you want a real safe
ideas:
  - A lock as a set of states
  - A code as a list of keys
  - Handing a list to a function, with adk::Span
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

**A code is a list of keys.** 0123 and 123 are different codes, so the safe
doesn't turn your keys into a number, the way Lesson 16's calculator did. It
keeps the keys themselves, in order, in a list, and a code is right when the
keys you typed and the keys of the code are the same, in the same order.

**A lockout beats guessing.** A four-digit code has 10 × 10 × 10 × 10 =
10,000 possibilities. Trying one every five seconds, a guesser would find
yours after about 5,000 tries, in about seven hours. Make them wait 30 seconds
after every three wrong tries and each try costs 15 seconds instead of five:
now it takes about 21 hours.

**EEPROM keeps the code.** Everything in the Mega's ordinary memory, its
variables, vanishes when the power goes. But the Mega also has 4,096 bytes of
**EEPROM**, memory that keeps what's written in it with the power off, for
many years. Each byte holds a number from 0 to 255, enough for one key, so
the sketch keeps each of the code's four keys in a byte of its own, in
addresses 1 to 4. One more byte, in address 0, is a "mark" that says whether
a code has been saved at all: a fresh EEPROM byte reads 255.

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
none has been saved. Every digit you type clicks the buzzer, and **\***
always does the same thing: it locks the safe and rubs out what you typed.

| State | The screen says | Key | What happens |
|---|---|---|---|
| Locked | `Locked. Code?` | 0 to 9 | A `*` appears; up to four digits |
| Locked | | # | Right code: **Open**. Wrong: a long beep and `Wrong! Try again`, or, at the third wrong code in a row, **Waiting** |
| Locked | | \* | Rub out the digits and start again |
| Open | `Open. # locks` | # or \* | The arm swings back: **Locked** |
| Open | | A | **Choosing** |
| Choosing | `New code, then #` | 0 to 9 | The digit itself appears, so you can check it |
| Choosing | | # | After four digits, save the code in EEPROM: **Open** |
| Choosing | | \* | Change your mind: **Locked**, and the old code stays |
| Waiting | `Too many tries` | any | Nothing, while `Wait 30 s` counts down; then **Locked** |

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire.
    Keep Lesson 17's power module, screen and servo just as they are, with
    their wires and the Mega's GND and 5V wires, and take out the angle
    knob in e45–e47 with its three wires. Put the keypad back where it was in
    Lesson 16, and add the buzzer past the screen. The power module's
    jumpers stay as Lesson 17 set them: the top one **OFF**, so the Mega's
    5V feeds the top rails for the screen, and the bottom one on **5V** for
    the servo. Never connect the servo's red wire to the Mega's 5V or the
    top + rail. Keep fingers clear of the servo's arm.

<!-- bench -->

<!-- steps -->

??? info "Making the box"
    Tape the servo inside a small cardboard box, just below the rim, with its
    shaft pointing at the lid. Upload the sketch first, so the servo sits at
    0°, the locked position, then press the horn onto the shaft so that it
    sticks out under the edge of the lid, holding it shut. At 90° it should
    swing clear. If your box needs different angles, change `lockedAngle`
    and `openAngle` in the sketch. Run the wires out through a small hole,
    and leave a finger hole or a string so you can still open it by hand while
    you are testing.

    It's a toy safe: anyone who can lift the whole box or pull a wire can
    open it. It's here to show how a lock thinks.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 018-keypad-safe**:

<!-- sketch -->

What's new:

- `#include <EEPROM.h>` brings in Arduino's EEPROM library, with its
  `EEPROM.read (address)` and `EEPROM.update (address, value)`.
- `enum class State` names three states. The fourth, Waiting, is simple
  enough to be a pause inside `refuse ()`.
- `adk::Array code {'1', '2', '3', '4'};` is the code, a list of four keys,
  and `adk::Vector<char, 4> typed;` is a list of the keys typed so far, which
  grows as you type, up to four. `typed.full ()` says when it holds four, and
  `typed.clear ()` empties it.
- `loop ()` reads one key and hands it on, depending on the key and the
  state. The rules are the table above.
- `typeDigit ()` adds the key to `typed` and shows a `*`, or the digit itself
  while you are choosing a new code.
- `pressEnter ()` decides what `#` means right now.
  `adk::equal (typed, code)` checks the code: it is true when the two lists
  hold the same keys in the same order, whatever kinds of list they are.
- `refuse ()` counts wrong codes. On the third, it counts down 30 seconds with
  `adk::wait (1000)`, printing the seconds left on the bottom row with one
  `adk::print ()`. Keys pressed meanwhile are scanned but never read, so they
  do nothing.
- `enter (State::Open, "Open. # locks")` is how the safe changes state. It
  moves the latch to match the new state, writes the message on the top row,
  and clears the bottom row and `typed` for typing. Every change goes through
  it, so the latch and the screen can never disagree.
- `loadCode ()` and `saveCode ()` are the EEPROM: address 0 holds the mark,
  42, once a code is saved, and addresses 1 to 4 hold its keys.
- `saveCode ()` takes its keys as **`adk::Span<const char>`**, a view of a
  list of characters kept somewhere else: not a copy, but a way to see the
  list's `size ()` and its items. A Span can be handed an Array or a Vector
  alike, so `saveCode (typed)` takes the Vector of keys you typed straight
  in, and its loop runs to `keys.size ()`, however many that is. The
  `const` means `saveCode ()` may look at the keys but not change them.
- `size_t` is the type that sizes and places in a list come in, a whole
  number that is never negative, so the loops over the keys count with a
  `size_t i`.

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
still 2468: uploading a sketch doesn't touch the EEPROM. So the answer is
2468 both times.

Pressing **A** and then **\*** instead leaves the code as it was: the safe
locks, and the old code still opens it.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo never moves | Is the power module on, with its LED lit and its bottom jumper on 5V? Check the servo's red and black wires reach the bottom rails (B+53, B-54) and its orange wire pin 44. |
| The screen is dark | It runs on the Mega's 5 V, as in Lesson 17: check the red wire from the Mega's 5V into the top + rail (T+3). |
| The screen is blank, but the backlight is on | Turn the contrast knob. |
| Keys come out wrong | See Lesson 16's "Which way round is the ribbon?" |
| The right code says `Wrong!` | You may have saved a different code. If you've forgotten it, change `savedMark` to 43 and upload: the sketch then ignores the saved code and starts again from 1234. |
| No clicks or beeps | Check the buzzer's + leg is in f51 with pin 12's wire in j51, and the black wire from a51 goes to the − rail. |
| The servo buzzes when locked | It's pressing against its stop or the lid. Try a `lockedAngle` of 10. |
| The Mega resets when the servo moves | The servo is getting power from the Mega. Its red wire must go to the bottom + rail (B+53), which only the power module feeds. |

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
   An `adk::Timer` from Lesson 3, started in `enter ()` when the safe opens,
   is one way.
2. **Longer lockouts.** Double the wait after each lockout: 30 s, then 60,
   then 120. `lockoutTime` will need to be a variable rather than a
   `constexpr`. Should a correct code reset it?
3. **Can't unplug your way out.** At the moment, unplugging the Mega forgets
   how many wrong codes were typed. Keep the count in EEPROM too, so a
   guesser can't reset it.
4. **Six digits.** Allow six-digit codes: give `code` six keys, and `typed`
   room for six. Why does no function need to change? (Change `savedMark`
   too, so an old four-key code isn't read back as six.)

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it to DC volts as in [Lesson 1](../001-blink/index.md#measure-it), black lead
in **COM** and red in **V**. The screen's wires hold still between key
presses, as they did in Lesson 16, so the sketch needs no change. Keep
fingers clear of the servo's arm while you work.

!!! question "Predict"
    While the safe is locked, each digit you type shows as a star. Could
    someone with a meter on the screen's wires tell which digit it was?
    Write down your guess.

<!-- measure -->

The bottom − rail is under the screen, so the black probe goes in a column
the screen joins to GND: RW's, c13, for the first reading, and the
backlight's K, c24, for D4, as in Lesson 16. Type any digit while the safe
is locked and measure D4; then open the safe, press **A**, type **5** and
measure it again.

What the numbers tell you:

- **The screen's 5 V** comes from the Mega, down the top + rail, as in
  Lesson 17: switch the power module off and it stays, though the servo
  goes limp. The screen's signals come from the Mega too, so screen and
  signals share one supply, and only the servo runs on the module's.
- **D4 after a star** reads 0 V whatever digit you typed, and the other
  three wires don't change with the digit either: the answer is no. The
  star is character 42, `0010 1010`, and its second half, `1010`, leaves D4
  at 0 V (and D5 and D7 at 5 V). The digit never reaches the screen's
  wires at all, because `typeDigit ()` sends a `'*'` while the safe is
  locked.
- **D4 after a 5** reads 5 V: while you choose a code, the digit itself goes
  to the screen, `0101`, as in Lesson 16. D4 is the digit's lowest bit, so
  it reads 5 V after an odd digit and 0 V after an even one. The same key,
  in a different state, puts a different voltage on the wire.
