---
lesson: 55
promise: Measure a radio link. Choose how long each message is, send five, and see how long each took and how many got through.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 display
  - 10 kΩ potentiometer
  - 220 Ω resistor (red, red, black, black, brown)
  - Rotary encoder module
  - 433 MHz receiver, RX470C, and transmitter, WL102-341, from Lesson 38 (add-ons, not in the kit)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 kΩ resistor (red, black, black, brown, brown), or a second 1 kΩ
  - 5 female-to-male jumper wires, and 3 more for the last experiment
  - 25 jumper wires
  - For the last experiment, a small metal tin with a lid, and a plastic bag
ideas:
  - The time every message costs, however short
  - Testing a link by sending messages you know and counting what comes back
  - Why long messages are the first to fail when the signal is weak
---

## What you'll build

<!-- closeup -->

A meter for a radio link. Turn the rotary knob to choose how many letters
each message carries, from 5 to 60, and press it. The Mega sends five
messages through the transmitter and listens for them with the receiver,
as in [Lesson 38](../038-radio-messages/index.md), and while the
transmitter rests between them, the screen counts down to the next. Then
the top row shows how long one message took to send, and the bottom row
how many of the five came back whole: **Heard 5/5 100%**.

Then you make the radio struggle, and find out which messages it loses
first.

## The idea

**Every message pays a toll.** In Lesson 38 each letter went out as two
groups of six bits: 12 bits, at 2000 bits a second, so 6 ms a letter. But a
message is more than its letters. It starts with the warm-up the receiver
locks onto, 48 bits, and it carries seven bytes of its own: its length,
four bytes RadioHead keeps for addresses, and the two-byte checksum. That
is 66 ms before the first letter, whether the message is one letter long
or sixty:

<p class="formula">time on the air = 66 ms + 6 ms × letters</p>

**Testing a link.** To find out how good a link is, send messages you know
and count the ones that come back. The checksum from Lesson 38 makes the
count honest: a message that noise got into is thrown away, so every
message arrives whole or not at all. Each of the meter's messages carries
its number, `#1` to `#5`, so one that turns up late can't be counted as
the next.

**A rest after every message.** As in Lesson 38, the transmitter rests
after each message, because the law lets it send only now and then: 30
times as long as the message took, and never less than 10 seconds. So
the meter sends only when you press the knob, and only five messages,
one each time the rest is over. [Lesson 38](../038-radio-messages/index.md#the-idea)
and [Safety](../../safety.md#radios) have the details.

!!! question "Predict"
    Use the toll to work it out before you build: how long will one
    20-letter message take to send? And a 60-letter one? Then use the
    rest: about how long will a whole test of five 20-letter messages
    take? Write down your answers.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your work
    before you plug it back in.

!!! danger "3.3 V for the transmitter"
    The transmitter's **+** pin goes to the Mega's **3.3V** pin, never to
    5V, and its DAT only ever sees pin 46 through the 1 kΩ, with the 2 kΩ to
    GND. The receiver's VCC takes 5 V.

!!! warning "Soldered pins only"
    Each module's pins must be soldered to its board. Pins that are only
    pushed through the holes don't make a connection, and the module will
    stay silent.

Everything goes back to a home it has had before: the screen at its home
from Lesson 13, the rotary encoder above the Mega as in Lesson 37, and the
receiver and transmitter in row j past the screen, as in Lesson 38, with
their wires coming round the bottom of the screen and up into row f.
None of Lesson 54's parts stay, so take its Board A apart first, all but
the Mega's GND wire, and build this one from the start.

<!-- bench -->

<!-- steps -->

??? info "The encoder's and the modules' pins"
    | Part | Pin | Goes to |
    |---|---|---|
    | Encoder | CLK | Pin 18 |
    | Encoder | DT | Pin 19 |
    | Encoder | SW | Pin 22 |
    | Encoder | + | The inner 5V pin at the top of the long header |
    | Encoder | GND | The GND beside pin 13 |
    | Receiver | VCC | 5V, on the power header |
    | Receiver | DATA | Pin 43 |
    | Receiver | GND | The bottom − rail |
    | Transmitter | DAT | The middle of the divider: pin 46 through 1 kΩ, and 2 kΩ to the − rail |
    | Transmitter | + | 3.3V, on the power header |
    | Transmitter | − | The bottom − rail |

    The receiver's DATA2 and the transmitter's EN connect to nothing. The
    boards print their pin names on the back: go by the names.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 055-reliability-meter**:

<!-- sketch -->

What's new:

- `adk::Stopwatch onAir;` times each message, as the stopwatch timed your
  reactions in Lesson 3. `onAir.restart ()` starts it from zero as a
  message goes out, and once `transmitter.isSending ()` turns false the
  message has gone, so `onAir.stop ()` holds the time: `onAir.elapsed ()`.
- `transmitter.isReady ()` is true when the transmitter will take a
  message: nothing is going out, and its rest is over. The sketch sends
  the next message as soon as it is.
- `adk::Every refresh {250};` beats four times a second, as `tick` beat in
  Lesson 11, and on each beat `showRest ()` shows how much of the rest is
  left: `transmitter.restLeft ()`, rounded up to whole seconds as in
  Lesson 38.
- `adk::Timer pause;` waits 50 ms after the last message before the
  result shows. `pause.start (50)` sets it going, and `pause.expired ()`
  is true once, when the time is up. The pause lets the receiver finish
  with the last message.
- `sendMessage ()` builds each message in an `adk::Text<60>`, as Snake's
  message was built in Lesson 27: its number, such as `#3 `, and then
  letters `a` to `z` over and over, until it is exactly as long as the dial
  says. `message.size ()` is how many letters it has so far. It shows
  `Sent 3 of 5` before it sends, so the stopwatch times only the message.
- `message == receiver.text ()` is true when what arrived is exactly what
  went out. Only then does `heard` count it.
- While `testing` is true the knob is ignored, so the length can't change
  in the middle of a test. `tested` says whether the length on the screen
  has been tested yet.

## Upload it

1. Upload the sketch. The screen says `20 letters` and `Press to test`.
2. Press the knob. The top row says `Sent 1 of 5`, and the bottom row
   counts down to the next message: `Next in 10 s`. If the transmitter
   is still resting, from its last message or from when the sketch
   started, the countdown comes first.
3. After the fifth message, about 41 seconds in all, the screen shows the
   result: `20 letters 186ms`, or a millisecond either side, and
   `Heard 5/5 100%`.
4. Turn the knob to 60 and press again. Each rest is longer this time,
   `Next in 13 s`, and the top row says about `426ms`.

Did you predict 186 ms and 426 ms? A 60-letter message carries twelve
times the letters of a 5-letter one, but takes less than five times as
long, because the 66 ms toll is paid either way. Long messages carry
letters more cheaply. And the whole test? Four rests of 10 seconds, and
five messages of 186 ms: about 41 seconds. From 45 letters up, the rest
is 30 times the message instead, 12.78 seconds after a 60-letter one, so
that test takes about 53 seconds.

With the two modules a few centimeters apart, every test should say
5/5. The signal is far stronger than the noise, so nothing goes missing.

## Make it struggle

!!! question "Predict"
    When the signal is only just strong enough, noise now and then flips a
    bit, and one flipped bit loses the whole message. Which will lose more:
    5-letter messages or 60-letter ones? Why?

1. Unplug the USB cable. Take the receiver out of the breadboard, and join
   its pins to the holes it came out of with three female-to-male jumper
   wires: **VCC** to j48, **DATA** to j49, **GND** to j51.
2. Plug in, and test 5 letters and then 60 letters with the receiver lying
   beside the board. Both should still say 5/5. Each test takes most of a
   minute, so let the countdown run.
3. Put the receiver in the plastic bag, so none of its pins can touch
   metal, and shut it in the tin, its wires coming out under the lid.
   Metal all round stops most of a radio wave. Test 5 and 60 letters again.
4. Move the tin as far from the transmitter as its wires let you, and turn
   it round, testing 60 letters each time, until some messages go missing.
   Then test 5 letters in the same place.

Where the signal is weak, the 60-letter messages go missing first. Each
has more than four times the bits of a 5-letter one, so noise has more
than four times the chances to spoil it. That is why radios that must get
through, such as the LoRa radios in Lessons 40 to 54, send short messages.
With only five messages a test, one lost message is 20%, so test each
length twice or more in one place before you compare them.

How much the tin holds back depends on the tin, and these modules hear
each other easily, so on one board you may never see a message go
missing. Then the experiment waits for a second Mega: see
[Make it yours](#make-it-yours).

## If it doesn't work

| What you see | Try this |
|---|---|
| Pressing the knob does nothing | Check the encoder's SW goes to pin 22, its + to the inner 5V pin, and its GND to the GND beside pin 13. |
| Turning the knob does nothing | Check CLK goes to pin 18 and DT to pin 19. If it counts the wrong way, swap those two wires. |
| `Heard 0/5` | Check the receiver's DATA goes to pin 43, its VCC to 5V and its GND to the − rail; then the transmitter's + to 3.3V, its − to the − rail, and the divider: pin 46's wire in f54, the 1 kΩ from h54 to h57, the 2 kΩ from g57 to e57, and the black jumper from a57 to the − rail. |
| Still `Heard 0/5` | Check the modules' pins are soldered to their boards, not just pushed through. |
| The test seems stuck on `Next in` | It isn't: the transmitter rests 10 seconds or more after every message, so a test of five takes most of a minute. |
| Fewer than 5 heard with both modules on the board | Keep them away from the computer and its cable, and from other 433 MHz gadgets: a doorbell or weather station sending at the same moment spoils a message. |
| Nothing on the screen, or a row of blocks | Turn the contrast knob beside the LCD. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

## Make it yours

1. **Two boards.** Build Lesson 38's circuit on a second Mega, and make
   it send five numbered messages when its button is pressed, each one as
   soon as `transmitter.isReady ()`. Change this sketch to count what
   arrives without sending. Where the rules allow aerials and distance, in
   Europe or with an amateur radio license and your call sign in the
   messages, fit 17 cm wires to both modules' ANT holes and carry one Mega
   further away between tests, to find how far the link reaches.
2. **Letters a second.** Add a line that shows how many letters a second
   each length carries: `length * 1000 / onAir.elapsed ()`. Which length
   carries the most?
3. **Your own toll.** Work out the time for every length from 5 to 60 from
   the formula, and the rest after it, and check each against the meter.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Keep each probe tip in
its own hole.

!!! question "Predict"
    While a message goes out, the transmitter's radio is on for half of its
    bits and off for the other half. But a message lasts less than half a
    second, and then the transmitter rests for 10 seconds or more. What
    will the meter read on DAT through a test of 60-letter messages?

<!-- measure -->

Press the knob with the probes already in place, and watch the meter
while the bottom row counts down.

What the numbers tell you:

- **The transmitter's DAT** reads 0 V between messages, which is nearly
  all of a test: pin 46 is low, and the radio is off. A 60-letter message
  is on the air for 0.43 seconds in every 13, about 3% of the time, so as
  one goes out the meter may twitch, or may not move at all: it is too
  quick for the meter to settle on.
- **The receiver's DATA** reads about 2.5 V between messages, as it did in
  Lesson 38: with nothing to hear, it turns itself up until noise sets it
  high about half the time.
