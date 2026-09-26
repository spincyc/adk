---
lesson: 38
promise: Send short messages through the air at 433 MHz, from a button or the Serial Monitor, and see every one arrive on the screen.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - 433 MHz receiver, RX470C (add-on, not in the kit)
  - 433 MHz transmitter, WL102-341 (add-on, not in the kit)
  - Push button
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 kΩ resistor (red, black, black, brown, brown), or a second 1 kΩ
  - 9 jumper wires
ideas:
  - A radio switched on and off to send bits
  - Noise, and a warm-up the receiver can lock onto
  - A checksum that throws damaged messages away
  - A divider that turns 5 V into 3.3 V
---

## What you'll build

<!-- closeup -->

Two little radio boards stand on the breadboard: a transmitter and a
receiver. Press the button and the screen says **Message 1:** and, below it,
**Hello!**. That message left the Mega through the transmitter, crossed a few
centimeters of air as radio, and came back in through the receiver. Press
again for the next message, or type your own in the Serial Monitor. With a
second Mega, or a friend's, the same messages reach across a room.

## The idea

**A radio switched on and off.** The transmitter makes a radio wave at
433.92 MHz, but only while its **DAT** pin is high. So the Mega sends bits by
switching the radio on for a 1 and off for a 0, 2000 times a second. The
receiver's **DATA** pin goes high while it hears the wave and low while it
doesn't. It's Morse code, very fast.

**Noise.** Between messages the receiver hears nothing, so it turns its
sensitivity right up, and then it hears everything: sparks from motors,
other gadgets, the crackle of the air itself. Its DATA pin flickers at
random all the time nothing is being sent. So a message can't just start;
it begins with a **warm-up**, a run of on-off-on-off the receiver can lock
onto and time itself by, and then a pattern that means "the message starts
now". Each letter goes out as two groups of six bits, every group with three
1s and three 0s, so the signal never stays on or off long enough for the
receiver to lose track.

**A checksum.** The noise can still flip a bit in the middle of a message.
So before sending, the Mega works out a **checksum**, a number made from
every byte of the message in a way that changes if any one bit does, and
sends it at the end. The receiver works out the same number from what it
heard. If the two don't match, the message was damaged, and it is thrown
away. You get the right message, or none at all.

**3.3 V again.** The receiver works on 5 V, and its DATA is safe for the
Mega to read. The transmitter works on **3.3 V**, like the FM radio in
Lesson 37, so its + pin takes the Mega's 3.3V. But its DAT has no resistor
of its own to lift it, so the Mega has to drive it high, and pin 46 reaches
it through a **divider**: two resistors in a row from pin 46 to GND, with
DAT joined between them.

<p class="formula">DAT = 5 V × <span class="fraction"><span>2 kΩ</span><span>1 kΩ + 2 kΩ</span></span> ≈ 3.3 V</p>

When pin 46 is at 0 V, so is DAT.

**An aerial.** A 433 MHz wave is 69 cm long, and a straight wire a quarter
of that, 17 cm, soldered to a module's **ANT** hole makes a good aerial. The
modules often come with little coil springs that do the same job in less
space; they need soldering too. Without either, the two still hear each
other easily across a desk.

!!! warning "Keep it short and few"
    Receiving is allowed anywhere, but sending is ruled by law. In Europe,
    433 MHz is free for small, low-power gadgets like this one. In the USA
    and Canada it belongs to radio amateurs, and without a license only very
    weak, occasional signals, like a car key's, are allowed: keep your
    messages short and few. [Safety](../../safety.md#radios) has the details.

!!! question "Predict"
    The receiver's DATA flickers with noise the whole time nothing is being
    sent, and the sketch shows every message the receiver hears, with a
    count. Leave it running for a minute without pressing anything. What
    will the screen show?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your work
    before you plug it back in.

!!! danger "3.3 V for the transmitter"
    The transmitter's **+** pin goes to the Mega's **3.3V** pin, never to
    5V, and its DAT only ever sees pin 46 through the 1 kΩ, with the 2 kΩ to
    GND. The receiver's VCC takes 5 V.

Keep the screen from Lesson 37 as it is, and two of the short black
jumpers down to the − rail: the radio's, from f51, which now takes the
receiver's GND, and the volume knob's, from a57, which now finishes the
divider. Take everything else off. The button goes at its home beside the
screen, in columns 38 to 40.

The receiver and the transmitter stand in row j, their boards lying back
over the top rails like the FM radio's in Lesson 37, and their aerials, if
you've fitted them, pointing up and away. Their wires come round the bottom
of the screen and up into row f.

<!-- bench -->

<!-- steps -->

??? info "The divider, and the two modules' pins"
    The divider is two resistors in a row. From pin 46's wire in f54, the
    1 kΩ lies along row h into h57, DAT's column. The 2 kΩ stands across the
    middle gap from g57 to e57, and the black jumper from a57 takes it to
    the − rail. If you have no 2 kΩ, use a second 1 kΩ in place of that
    black jumper: two 1 kΩ in a row make 2 kΩ.

    | Module | Pin | Goes to |
    |---|---|---|
    | Receiver | VCC | 5V, on the power header |
    | Receiver | DATA | Pin 43 |
    | Receiver | DATA2 | Nothing: it's joined to DATA on the board |
    | Receiver | GND | The bottom − rail |
    | Transmitter | EN | Nothing |
    | Transmitter | DAT | The middle of the divider |
    | Transmitter | + | 3.3V, on the power header |
    | Transmitter | − | The bottom − rail |

    The boards print their pin names on the back. If your receiver's pins
    are in a different order, go by the names, not by where they sit.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 038-radio-messages**:

<!-- sketch -->

What's new:

- `adk::RadioReceiver receiver {43};` and
  `adk::RadioTransmitter transmitter {46};` are the two modules. Between
  them they borrow one of the Mega's timers, which looks at the receiver
  16,000 times a second; while they do, pins 11 and 12 can't dim anything.
- `messages` is a list of texts, as the menu's choices were in Lesson 29,
  and `next` says which one the button sends next. `% messages.size ()`
  takes it back to the first after the last, as `% 16` did in Lesson 13.
- `adk::LineReader<60> typed;` gathers what you type in the Serial Monitor.
  `typed.read (Serial)` takes whatever has arrived so far, never waiting,
  and is true once a whole line is in, when you press Enter; `typed.line ()`
  is then the text. It keeps up to 60 letters, the most one message can
  carry.
- `transmitter.send (text)` starts a message going and comes straight back,
  while the message goes out in the background: about a tenth of a second
  for a word. It says `false`, and sends nothing, if the last message is
  still going.
- `receiver.wasReceived ()` is true for one pass of `loop ()` when a whole,
  unbroken message has arrived, and `receiver.text ()` is what it said.
- `adk::Text<16> start` is text you print into, as Snake's message was in
  Lesson 27. It keeps the first 16 letters and leaves off the rest, which
  is just what fits on the screen.

## Upload it

1. Upload the sketch. The screen says `Listening...`.
2. Open the Serial Monitor at 9600 baud, and set the menu beside the baud
   rate to **New Line**, so pressing Enter ends your line.
3. Press the button. The screen shows `Message 1:` and `Hello!`, and the
   Serial Monitor says `Sent: Hello!` and then `Heard: Hello!`.
4. Press it again for the next message, and the next.
5. Type a line of your own in the Serial Monitor and press Enter. It goes
   out and comes back the same way. A long one shows its first 16 letters
   on the screen; the Serial Monitor shows it all.

You predicted what the screen would show while nothing is sent. It shows
the last message, and the count doesn't move. The receiver hears noise all
the time, and now and then the noise even looks like the start of a
message, but random bits almost never add up to the right checksum, so
the noise doesn't get through.

## If it doesn't work

| What you see | Try this |
|---|---|
| `Sent:` appears, but nothing is heard | Check the receiver's DATA goes to pin 43, its VCC to 5V and its GND to the − rail. Check the transmitter's + goes to 3.3V and its − to the − rail. |
| Still nothing is heard | Check the divider: pin 46's wire in f54, the 1 kΩ from h54 to h57, the 2 kΩ from g57 to e57, and the black jumper from a57 to the − rail. |
| Typing does nothing | Set the Serial Monitor's line ending to **New Line**: the sketch waits for the end of the line. |
| `Still sending, try again` | Wait a moment: the last message was still going out. |
| Some messages go missing | Try a 17 cm wire on each module's ANT hole, or turn one module round. Keep them away from the computer and its cable. |
| Nothing on the screen, or a row of blocks | Turn the contrast knob beside the LCD. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    ADK sends messages the way RadioHead's RH_ASK driver does, a library
    many Arduino projects use. Timer 1 interrupts the Mega 16,000 times a
    second, eight times for every bit. Each time, the transmitter's side
    moves on through its message, and the receiver's side reads DATA.

    The receiver keeps count of how many of each bit's eight readings were
    high, and calls the bit a 1 if most were. It keeps in step with the
    sender with a little clock of its own: each time DATA changes, it
    checks whether the change came early or late in the bit, and nudges its
    clock to match. The warm-up gives it plenty of changes to settle on.

    Every message starts with its length and four bytes RadioHead uses for
    addresses, which ADK sets so the message is for everyone, and ends with
    a 16-bit checksum, the kind called a CRC. Only a message whose checksum
    matches reaches your sketch.

    All this takes about a tenth of the Mega's time, which is why the
    receiver and transmitter keep the timer for themselves.

## Make it yours

1. **Across the house.** Build the same circuit on a second Mega, or ask a
   friend with the kit, and send messages between two rooms. Any Arduino
   running RadioHead's RH_ASK at its usual 2000 bits a second, with a
   receiver on its data pin, can listen too.
2. **An aerial.** Solder a 17 cm wire to each module's ANT hole, keep them
   straight, and see how far apart the two Megas can go.
3. **A doorbell.** Make the second Mega beep, with the active buzzer from
   Lesson 3, whenever a message says `Ding dong`. Compare the text with
   `strcmp (receiver.text (), "Ding dong") == 0`.
4. **Your own messages.** Change the list in `messages`, or add more. Each
   can be up to 60 letters, but only 16 fit on the screen.
5. **Lost and found.** Number each message, such as `#12 Hello!`, with an
   `adk::Text<60>`, and on the second Mega count how many go missing as you
   move further away.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take these readings
while nothing is being sent, and keep each probe tip in its own hole.

!!! question "Predict"
    The receiver's DATA is high while it hears a wave and low while it
    doesn't, and with nothing sent it hears only noise. What will the meter
    read on DATA: 0 V, 5 V, or something else?

<!-- measure -->

What the numbers tell you:

- **The receiver's DATA** reads about 2.5 V, and wanders a little. With
  nothing to hear, the receiver turns itself up until the noise sets DATA
  high about half the time and low the rest, far faster than the meter can
  follow, so it shows the average: half of 5 V. Press the button while you
  watch: the reading hardly changes, because a message is on and off about
  half the time too.
- **The transmitter's DAT** reads 0 V. Pin 46 is low, so both ends of the
  divider are at 0 V, and the transmitter's radio is off. It only comes on
  for the tenth of a second a message takes.
- **The transmitter's supply** is the Mega's 3.3V pin, as the FM radio's
  was. The transmitter only draws much current while it sends.
