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
  - 10 female-to-male and 21 jumper wires
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

`friends` is then an `adk::Array` of these, as in Lesson 5, and
`person.card` and `person.name` are the two halves of one friend.

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

In the sketch, `loop ()` is the Locked and Listening states: it watches for
a card, and for knocks, and it is Listening while the `quiet` timer runs.
`openFor ()` and `refuse ()` are the other two: each sets the latch, the
screen and the buzzer, waits out its time with `adk::wait ()`, and locks the
door again.

Every time the door locks, the sketch waits until the servo has swung shut and
the table has stopped shaking before it listens for knocks again, so the latch
can't knock on its own door.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and switch the power module off before you change
    any wiring. Set both of the power module's yellow jumpers to **5V**,
    never 3.3V. The screen and the servo take their power from the power
    module, so switch it on whenever the Mega is running. The RFID reader
    takes **3.3 V** from the Mega's 3.3V pin: never 5V. Its signal wires get
    5 V from the Mega, more than its chip is rated for, as Lesson 34
    explains; the [safety page](../../safety.md) says how to protect them in
    a build that has to last.

<!-- bench -->

<!-- steps -->

Set both of the power module's yellow jumpers to **5V**: the top rails feed
the screen and the bottom rails the servo. The RFID reader and the tap
sensor stay where they were in Lessons 34 and 35, below the Mega.

??? info "Making the latch"
    The servo's arm is the bolt. Tape the servo inside the box, near the top
    of the side opposite the hinge, so that its arm swings across under the
    lid's edge. The sketch holds it at 0° when locked and 90° when open. Try
    it with the lid open first: if the arm swings the wrong way or not far
    enough, change `lockedAngle` and `openAngle`. Mount the reader just
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
- `friends` lists your cards and who carries each. Written as
  `Friend {...}`, one per line, the `adk::Array` counts them itself, so
  another friend is just another line. The two numbers are stand-ins: put
  your own cards' numbers in, from the Serial Monitor.
- `secret`, `longGap`, `rattle` and `finished`, with `rhythm`, `sinceKnock`
  and `quiet`, are Lesson 35's secret knock. `lockedAngle` and `openAngle`
  are the latch's, named as in the Keypad Safe of Lesson 18.
- `setup ()` warns on the screen if the reader didn't answer, then locks the
  door.
- `loop ()` watches for the two keys. It checks for a card first, then a
  knock. When `quiet` runs out, the knocking has stopped: if
  `rhythm == secret`, as in Lesson 35, the door opens, and any other rhythm
  is refused.
- `checkCard ()` prints the card's number, then looks for it in `friends`.
  `const Friend& person` is each friend in turn: the `&` means the real one
  in the list, not a copy, and `const` promises not to change it. A friend's
  card opens the door for them; a stranger's is refused.
- `hearKnock ()` is Lesson 35's, with a star on the screen for every knock.
  The first knock clears the screen to say `Listening...`. Each star goes in
  the column after the last: `rhythm.size ()`, the number of gaps so far.
- `openFor ()`, `refuse ()` and `lock ()` are the three things the door can
  do. Each sets the latch, the screen and the buzzer together, and the
  first two wait out their time with `adk::wait ()` before they lock.
- `latch.moveTo (openAngle, 500)` glides the servo open over half a second,
  so the lid isn't flung.

## Upload it

Switch the power module on, then upload the sketch and open the Serial
Monitor at 9600 baud.

1. The latch swings shut and the screen says `Secret Door`.
2. Hold your card to the reader. It isn't known yet: `Access denied`, a long
   beep, and its number on the Serial Monitor. Copy the number into
   `friends` with your name, and upload again.
3. Hold the card again: `Welcome,` and your name, two short beeps, and the
   latch opens for five seconds before it swings shut.
4. Knock the secret on the lid: two quick knocks, a pause, two more. Four
   stars appear, and after a moment, `Welcome, the knocker`.

You predicted what happens to two knocks when a card comes next. `loop ()`
checks the reader before anything else, so the card is dealt with at once:
the door opens for a friend, or refuses a stranger. The two knocks are never
judged: `quiet` runs out while the door is busy and nothing is looking, and
then `lock ()` forgets them.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo twitches, buzzes, or the Mega resets when it moves | Switch the power module on, and check the servo's red wire goes to the power module's + rail and the Mega's GND to the − rail. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. The screen's power comes from the power module, so it must be switched on. |
| `No card reader!` when it starts | Check the reader's seven wires, as in Lesson 34, and that its 3.3V pin goes to the Mega's 3.3V. |
| Your card always gets `Unknown card` | Copy its number from the Serial Monitor exactly, with `0x` in front. |
| Knocks never make stars | Check the tap sensor's S goes to A12, + to 5V and − to GND, and knock close to it. |
| Stars appear, but the knock is always wrong | Knock the gaps more clearly: quick knocks well under half a second apart, and a pause of about a second. |
| The latch opens the wrong way | Swap `lockedAngle` and `openAngle`, or remount the servo. |
| No beeps | Check the buzzer's + leg, the longer one, is in f51, and the black wire goes from a51 to the − rail. |

??? note "How it works"
    While the door is open, `openFor ()` waits five seconds with
    `adk::wait ()`, and `refuse ()` waits two, so `loop ()` isn't running to
    check the reader or the tap sensor. Cards and knocks that come during
    those moments are simply missed: `wasRead ()` and `activated ()` are
    events, true for one update only.
    `adk::wait ()` still updates every part while it waits, so the servo
    glides and the buzzer stops on time.

    This is a toy lock, and a good one to learn from, but don't guard
    anything that matters with it. A card's number can be copied, and a knock
    can be overheard.

## Make it yours

1. **Your names.** Put every card and fob you own in `friends`, each with
   its own name, and change the secret knock to one of your own.
2. **Three strikes.** After three refusals in a row, show `Locked out` and
   ignore everything for 30 seconds, like the Keypad Safe in Lesson 18.
3. **Stay open.** Make a known card toggle the door: open until the same
   card is shown again. `openFor ()` can't just wait any more, so `loop ()`
   will need to know whether the door is open: a `bool`, or an
   `enum class` of the door's states.
4. **Visitor's book.** Count how many times each friend has come in, and
   show the count after their name: `Welcome, Ada (7)`. Add `int visits`
   to `Friend`, and take `constexpr` off `friends`, so the list can change.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**, never the **10A** socket.
Push each probe tip into its own hole: the rails' + and − holes are only
2.5 mm apart. A 600 ms beep is too short for a meter to settle, so for the
second reading change `buzzer.beep (600)` in `refuse ()` to
`buzzer.beep (2000)`, upload, and show the reader a card it doesn't know.
Put it back afterwards. As in Lesson 34, the reader's wires go straight
from the Mega to the reader, so its 3.3 V is out of reach here.

!!! question "Predict"
    In Lesson 33, the passive buzzer's pin read about half of 5 V while a
    note played. The active buzzer makes its own tone. What will its pin
    read during a beep?

<!-- measure -->

What the numbers tell you:

- **The latch's supply** is the power module's 5 V on the bottom rails; the
  top rails, which feed the screen, read the same. Watch the reading while
  the latch swings: it hardly moves, because the module has plenty to spare
  for the servo.
- **The buzzer's pin** reads nearly the full 5 V for the whole beep, and 0
  the rest of the time. The pin is simply switched on, and the buzzer makes
  its tone inside. It reads a little under 5 V because the buzzer draws
  about 30 mA, and a pin's 5 V sags a little as it gives more current.
