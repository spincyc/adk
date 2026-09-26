---
lesson: 51
promise: Build a front door that tells you inside who is there, by doorbell, knock or card, and lets them in.
time: 2 hours
level: 3
parts:
  - "Both boards: the Mega, breadboard and LoRa modem from Lesson 50, with the modem's divider and wires"
  - "Board A: the RC522 RFID reader with its card and fob, the tap sensor (37 in 1), a push button, the active buzzer, and a breadboard power module with its 9 V adapter (a second one: the kit has only one)"
  - "Board B: the servo and power module from Lesson 50, the LCD, knob and 220 Ω resistor from Lesson 13, a push button, and the passive buzzer with a 220 Ω resistor"
  - "11 female-to-male and 23 jumper wires, besides the modems' own"
  - "A box with a lid, and some tape, if you want a real door"
ideas:
  - Three events, each crossing as a count
  - The first count heard is not news
  - Why the latch lives inside
  - A card's number in a signed long
  - Putting two arcs together
---

## What you'll build

<!-- closeup A -->

<!-- closeup B -->

A front door and the hall behind it. Board A is the outside of the door:
a doorbell button, a tap sensor to hear knocks, and the card reader from
Lesson 34. Board B is inside: a screen, a chime, a button, and the latch.
Press the doorbell and Board B's screen says **Ding dong!** as it chimes;
press Board B's button to let your visitor in, and the latch swings open
while Board A's buzzer buzzes, like the front door of a block of flats.
Knock, and the screen says **Knock knock!**. Hold your card to the reader
and Board B says **Welcome home, Ada**, plays a little tune, and opens the
door by itself. A card it doesn't know gets **Unknown card**, and the door
stays shut.

## The idea

Almost every piece is one you know: the card and the knock from Lessons
34 to 36, the latch from Lesson 18, the keypad lock's rule from Lesson 49
that the board outside decides nothing, and the bridge between them.
What's new is how they fit together.

**Three events, three counts.** A ring, a knock and a card are all events,
so, as in Lesson 49, each crosses the air as a count that goes up:
`rings`, `knocks` and `cards`. A card also brings its number, `card`, for
Board B to look up. Board B keeps the last count it heard of each, and a
count that has gone up is news.

**The first count heard is not news.** Suppose Board B is switched on
after the doorbell has already rung three times. The first thing it hears
is `rings=3`, which is new to Board B but not news: nobody is ringing now.
So Board B starts each count at −1, meaning *not heard yet*, and the first
count it hears only tells it where Board A has got to. A count that goes
down means Board A restarted from 0, and that isn't news either.

**Why the latch lives inside.** In Lesson 36 one board had the reader and
the latch. Here Board A, at the door, has the reader and a modem, and both
run on 3.3 V. The Mega's 3.3V pin can give about 50 mA; the reader can
draw up to about 26 mA, and the modem draws tens of milliamps each time it
sends. Together they would ask too much of that pin. So on Board A, the
modem takes its 3.3 V from the power module instead, with the bottom
jumper on **3.3V**, as in Lesson 40. Those rails then can't give a servo
its 5 V, and the latch goes to Board B, which has the power module set to
5 V from Lessons 49 and 50. That suits Lesson 49's rule anyway: the board
outside knows no friends, holds no latch, and only reports what happens.
Board B tells Board A one thing back, `door`, 1 while the latch is open,
and Board A buzzes when it changes to 1.

**A card's number in a long.** A card's number fills 32 bits, all of a
`long`. A `long` keeps its top bit for the sign, so a number from
0x80000000 up, such as Sam's 0x9ABCDEF0, crosses as a negative number,
−1698898192. That's no harm: the bits are the same, and Board B turns them
back into a card's number with `uint32_t (...)`.

!!! question "Predict"
    Ring Board A's doorbell three times, then press Board B's **RESET**
    button, the small one beside its USB socket. When Board B has started
    again and hears Board A, will its screen say **Ding dong!**? Write
    down your guess.

## How the door works

| At the door, Board A | Crosses the air | Inside, Board B |
|---|---|---|
| Someone presses the doorbell | `rings` goes up | `Ding dong!`, a two-note chime |
| Someone knocks | `knocks` goes up | `Knock knock!`, a tap on the chime |
| A friend's card | `cards` goes up, with `card` | `Welcome home,` and the name, a tune, and the latch opens for 5 s |
| A stranger's card | `cards` goes up, with `card` | `Unknown card`, a low note, and its number on the Serial Monitor |
| The buzzer buzzes for a second | `door` becomes 1, the other way | The latch opens, for a friend's card or when you press the button: `Door open` |

News stays on Board B's screen for ten seconds; then it says
`Front door` and `All quiet`, or `Can't hear it` while Board A is silent.
Board A's **L** LED is lit while it can hear Board B.

## Build it

!!! warning "Unplug first"
    Unplug both boards' USB cables and both power modules' adapters before
    you wire, and check your work before you plug them back in.

!!! danger "Two power modules, set differently"
    - **Board A:** top jumper **OFF**, bottom jumper **3.3V**, for the
      modem. Never 5V: more than 3.6 V damages the modem.
    - **Board B:** top jumper **OFF**, bottom jumper **5V**, for the servo.

    Label the two boards, so their power modules never swap. The RFID
    reader takes **3.3 V** from Board A's own 3.3V pin, never 5V; Lesson 34
    explains its signal wires.

Both boards keep their LoRa modems where they were, with the same
dividers and wires, but for one change on Board A: its modem's VDD moves
from the Mega's 3.3V pin to the bottom + rail, B+47, right above it.

### Board A: the door

The GY-521 comes off, and with it the red wire from the Mega's 5V to the
top rails: nothing on Board A uses the top rails now. The reader and the
tap sensor lie below the Mega in their places from Lesson 36, wired the
same way. The doorbell is the button on pin 22 at its home, and the active
buzzer stands at its home in column 34.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B: inside

The LED matrix comes off; the power module, the latch and the modem stay.
The screen goes back to its home, taking 5 V from the Mega on the top
rails. Beside it stand the button on pin 23, in columns 38 to 40, and the
passive buzzer on pin 10, in column 51, with its 220 Ω resistor.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

??? info "Making the door"
    Board B's servo is the bolt, on the inside of a box's lid, as in Lesson
    18: tape it just below the rim so its arm swings out under the lid's
    edge, with the sketch holding it at 0° to lock and 90° to open. Board A
    sits outside the box: tape the reader where a card held against the
    front is a centimeter or two from it, and the tap sensor to the lid, so
    it feels a knock. Keep Board A's buzzer off the tap sensor's board:
    Board A ignores knocks while it buzzes, but not after.

## Code it

In the Arduino IDE, choose **File → Examples → Adk →
lessons/051-doorbell-and-door**: **Door** is for Board A and **Inside** for
Board B.

### Board A: Door

<!-- sketch A -->

Read it from the top:

- The reader, the tap sensor and its `rattle` are Lessons 34 and 35's. The
  bell is a button, and the buzzer and the **L** LED are Board A's only
  outputs.
- `setup ()` says on the Serial Monitor if the reader doesn't answer.
- `loop ()` counts rings, knocks and cards, and shares the counts and the
  latest card's number on every pass. `long (reader.uid ())` puts the
  card's 32 bits in a `long`, as *The idea* explains.
- A knock heard while the buzzer sounds is skipped: the buzzer shakes the
  board, and the tap sensor would hear it.
- `bridge.changed ("door") && bridge.value ("door") == 1` is true once,
  when Board B opens the door, and `buzzer.beep (1000)` buzzes for a
  second.

### Board B: Inside

<!-- sketch B -->

Read it from the top:

- `friends` is Lesson 36's list of cards and names, now kept inside. The
  two numbers are stand-ins: put in your own, from Board B's Serial
  Monitor.
- `dingDong`, `knock`, `welcome` and `stranger` are four little tunes for
  the chime, as in Lesson 5.
- `unlocked` runs for five seconds each time the door opens, and the
  latch follows `unlocked.isRunning ()` on every pass: open while it runs,
  shut once it stops. `news` runs while the screen shows news.
- `rings`, `knocks` and `cards` start at −1: not heard yet.
- `loop ()` asks `wentUp ()` about all three counts first, every pass, so
  none is missed, then deals with a card before the bell, and the bell
  before a knock.
- `wentUp ()` is the heart of it. When a count changes, it remembers the
  new count in `seen`, and says whether it went up from a count heard
  before. `long& seen` means it changes the caller's own variable, not a
  copy: `rings`, `knocks` or `cards`.
- `checkCard ()` looks the card up in `friends`, as in Lesson 36. A friend
  is welcomed and the door opens; a stranger's number goes to the Serial
  Monitor.
- `tell ()` puts news on the screen for ten seconds, and `showQuiet ()`
  says the door is quiet, or that Board A can't be heard.
- `bridge.share ("door", unlocked.isRunning ())` tells Board A whether the
  door is open.

## Upload it

1. Set Board A's power module's bottom jumper to **3.3V** and Board B's to
   **5V**, both top jumpers off. Plug in both adapters and switch both
   modules on.
2. Upload **Door** to Board A and **Inside** to Board B, each by its own
   port. Board B's latch swings to 0°, its screen says `Front door` and,
   once it hears Board A, `All quiet`. Board A's **L** LED lights.
3. Press the doorbell: `Ding dong!` and the chime. Press Board B's button:
   `Door open`, the latch swings open, and Board A buzzes. Five seconds
   later the latch shuts.
4. Knock on Board A's tap sensor, or the lid it's taped to: `Knock knock!`.
5. Open the Serial Monitor on Board B's port at 9600 baud, and hold your
   card to Board A's reader. It isn't known yet: `Unknown card`, a low
   note, and its number, such as `Card 0x1A2B3C4D`. Copy the number into
   `friends` in **Inside** with your name, and upload **Inside** again.
   Board A needs no change: the list lives inside.
6. Hold the card again: `Welcome home,` and your name, a tune, the latch
   opens, and Board A buzzes.

Now your prediction. After Board B's reset, the first `rings` it hears is
3, and it has never heard a count before, so `wentUp ()` only remembers
it: no `Ding dong!`. Ring once more and the count goes up to 4: that is
news.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board B says `Can't hear it` | Is Board A's power module on, with its bottom jumper on 3.3V? Its modem runs from those rails now. Check its VDD wire goes to B+47, and each modem's wires as in Lesson 49. |
| Board A's modem gets warm | Unplug everything at once, and check Board A's bottom jumper is on 3.3V, never 5V. |
| Board A's Serial Monitor says `No card reader` | Check the reader's seven wires as in Lesson 34, and that its 3.3V pin goes to the Mega's 3.3V. |
| Your card always gets `Unknown card` | Copy its number from Board B's Serial Monitor exactly, with `0x` in front, into **Inside**, and upload it to Board B. |
| Knocks never reach Board B | Check the tap sensor's S goes to A12, + to the power header's 5V and − to its GND, and knock close to it. |
| The doorbell does nothing | Check pin 22's wire in j2, the button across the gap in columns 2 to 4, and the black wire from a4 to the − rail. |
| The latch doesn't move | Is Board B's power module on, with its bottom jumper on 5V? Check the servo's wires in B+53 and B-54, and pin 44's to its orange. |
| No chime | Check the passive buzzer's + in f51 with pin 10's wire in j51, and the 220 Ω from a51 to the − rail. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. |

??? note "How it works"
    When Sam's card is held to the reader, Board A's modem sends:

    ```text
    @cards=4 card=-1698898192
    ```

    Board B turns −1698898192 back into 0x9ABCDEF0, finds Sam, and
    answers with the door:

    ```text
    @door=1
    ```

    Five seconds later it sends `@door=0`, and every two seconds each board
    repeats everything it shares: `@rings=2 knocks=5 cards=4
    card=-1698898192`, so a lost message is soon made good. A repeat is no
    change, so it's never news.

    A card's number and a knock are easy to copy, and anyone nearby with a
    LoRa modem could read these messages, or send a `cards` of their own.
    This door is for learning how the parts fit, not for keeping anyone
    out.

## Make it yours

1. **The secret knock.** Let Lesson 35's knock open the door too. Only
   Board A hears the gaps between knocks: Board B gets counts, a tenth of
   a second late at best. So have Board A turn the rhythm into a number,
   1 for each short gap and 2 for each long one, so `SLS` is 121, and
   share it when the knocking stops. Which board should know the secret?
2. **Visitor's book.** Count how many times each friend has come home,
   and show it on the screen: `Welcome home, Ada (7)`.
3. **Nobody home.** Add a switch to Board B, or use a long press of its
   button, that tells visitors on Board A's buzzer, with three short buzzes,
   that nobody will answer.
4. **Who rang?** Let Board B remember the last five events with the time
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

- **Board A's bottom rails** read about 3.3 V, from the power module's own
  3.3 V regulator: the modem's supply, and never enough for a servo.
- **Board B's bottom rails** read about 5 V: the latch's supply. A modem
  on these rails would be damaged, which is why each modem's VDD wire
  goes only where its lesson says.
