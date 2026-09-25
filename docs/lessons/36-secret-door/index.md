---
lesson: 36
title: Secret Door
arc: Keys you can't see
promise: Build a door latch that opens for your card, or for anyone who knows the secret knock.
time: 2 hours
level: 3
sketch: Lesson36SecretDoor
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard, power module and 9 V adapter
  - LCD1602, 10 kΩ potentiometer and 220 Ω resistor
  - RC522 RFID reader, with its card and fob
  - Tap sensor module (37 in 1)
  - SG90 servo and active buzzer
  - 10 female-to-male and 20 jumper wires
  - A box with a lid, and sticky tape
ideas:
  - Two different keys for one lock
  - Grouping a card and a name together
  - A device as a set of states
  - Putting a whole course together
---

## What you'll build

<!-- closeup -->

A box with a secret. The screen says **Secret Door**, and a servo's arm holds
the lid shut. Hold your card to the reader and the screen says
**Welcome, Ada**, the buzzer beeps twice, and the arm swings aside for five
seconds. No card? Knock the secret rhythm on the lid: a star appears on the
screen for every knock, and if you get it right, the box opens for you too.
A stranger's card, or the wrong knock, gets **Access denied** and a long,
grumpy beep.

## The idea

Every part of this door is one you already know: the LCD from Lesson 13, the
servo and its power from Lesson 17, the RFID reader from Lesson 34, the secret
knock from Lesson 35, and the active buzzer from Lesson 3. What's new is
putting them together, so that **two different keys**, a card or a knock,
open **one lock**. Both end in the same place in the sketch, `openFor ()`,
which only needs to know whom to welcome.

The sketch keeps a list of friends. Each friend is two things that belong
together, a card number and a name, so the sketch groups them into one
`struct`:

```cpp
struct Friend
{
    uint32_t    card;
    const char* name;
};
```

`Friends` is then a list of these, and `person.card` and `person.name` are
the two halves of one friend.

!!! question "Predict"
    You knock twice, then change your mind and hold a card to the reader
    instead. What happens to your two knocks? Read `loop ()` and write down
    your answer before you try it.

## How the door works

The door is always in one of four states:

| State | The screen shows | What changes it |
|---|---|---|
| Locked | `Secret Door` and `Card or knock...` | A card, or a first knock |
| Listening | `Listening...` and a star for each knock | 1.5 s without a knock: the rhythm is judged. A card is checked at once. |
| Open | `Welcome,` and a name, with the latch open | Five seconds pass: back to Locked |
| Refused | The reason, and `Access denied` | Two seconds pass: back to Locked |

Every time the door locks, the sketch waits until the servo has swung shut and
the table has stopped shaking before it listens for knocks again, so the latch
can't knock on its own door.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and switch the power module off before you change
    any wiring. The screen and the servo take their power from the power
    module, so switch it on whenever the Mega is running. The RFID reader
    takes **3.3 V** from the Mega's 3.3V pin: never 5V.

<!-- bench -->

<!-- steps -->

??? info "Making the latch"
    The servo's arm is the bolt. Tape the servo inside the box, near the top
    of the side opposite the hinge, so that its arm swings across under the
    lid's edge. The sketch holds it at 0° when locked and 90° when open. Try
    it with the lid open first: if the arm swings the wrong way or not far
    enough, change `LockedAngle` and `OpenAngle`. Mount the reader just
    inside the front of the box, so a card held against the outside is a
    centimetre or two away, and tape the tap sensor inside the lid.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson36SecretDoor**:

<!-- sketch -->

Read it from the top:

- Five parts: the screen, the reader, the tap sensor, the latch and the
  buzzer.
- `Friends` lists your cards and who carries each. The two numbers are
  stand-ins: put your own cards' numbers in, from the Serial Monitor.
- `setup ()` warns on the screen if the reader didn't answer, then locks the
  door.
- `loop ()` does one thing at a time. When the door is open, it only watches
  the clock. Otherwise it checks for a card first, then a knock, then whether
  the knocking has stopped.
- `checkCard ()` looks for the card in `Friends` and opens for its owner, or
  refuses a stranger.
- `hearKnock ()` and `judgeRhythm ()` are Lesson 35's secret knock, with a
  star on the screen for every knock.
- `openFor ()`, `refuse ()` and `lock ()` are the three things the door can
  do. Each one sets the latch, the screen and the buzzer together.
- `latch.moveTo (OpenAngle, 500)` glides the servo open over half a second,
  so the lid isn't flung.

## Upload it

Switch the power module on, then upload the sketch and open the Serial
Monitor at 9600 baud.

1. The latch swings shut and the screen says `Secret Door`.
2. Hold your card to the reader. It isn't known yet: `Access denied`, a long
   beep, and its number on the Serial Monitor. Copy the number into
   `Friends` with your name, and upload again.
3. Hold the card again: `Welcome,` and your name, two short beeps, and the
   latch opens for five seconds before it swings shut.
4. Knock the secret on the lid: two quick knocks, a pause, two more. Four
   stars appear, and after a moment, `Welcome, the knocker`.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo twitches, buzzes, or the Mega resets when it moves | Switch the power module on, and check the servo's red wire goes to the power module's + rail and the Mega's GND to the − rail. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. The screen's power comes from the power module, so it must be switched on. |
| `No card reader!` when it starts | Check the reader's seven wires, as in Lesson 34, and that its 3.3V pin goes to the Mega's 3.3V. |
| Your card always gets `Unknown card` | Copy its number from the Serial Monitor exactly, with `0x` in front. |
| Knocks never make stars | Check the tap sensor's S goes to A12, + to 5V and − to GND, and knock close to it. |
| Stars appear, but the knock is always wrong | Knock the gaps more clearly: quick knocks well under half a second apart, and a pause of about a second. |
| The latch opens the wrong way | Swap `LockedAngle` and `OpenAngle`, or remount the servo. |
| No beeps | Check the buzzer's + leg, the longer one, is in g32, and the black wire goes from j35 to the − rail. |

??? note "How it works"
    While the door is open, `loop ()` doesn't check the reader or the tap
    sensor at all, and `refuse ()` waits two seconds with `adk::wait ()`.
    Cards and knocks that come during those moments are simply missed:
    `wasRead ()` and `activated ()` are events, true for one update only.
    `adk::wait ()` still updates every part while it waits, so the servo
    glides and the buzzer stops on time.

    This is a toy lock, and a good one to learn from, but don't guard
    anything that matters with it. A card's number can be copied, and a knock
    can be overheard.

## Make it yours

1. **Your names.** Put every card and fob you own in `Friends`, each with
   its own name, and change the secret knock to one of your own.
2. **Three strikes.** After three refusals in a row, show `Locked out` and
   ignore everything for 30 seconds, like the Keypad Safe in Lesson 18.
3. **Stay open.** Make a known card toggle the door: open until the same
   card is shown again.
4. **Visitor's book.** Count how many times each friend has come in, and
   show the count after their name: `Welcome, Ada (7)`.
