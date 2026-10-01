---
lesson: 55
promise: Measure the radio link between your two boards. Send 64 messages, count how many come back, and find out what the modem's slow, far-reaching setting buys you, and what it costs.
time: 2 hours
level: 3
parts:
  - "Board A: the Mega, breadboard, LoRa modem, LED matrix, joystick and passive buzzer from Lesson 54"
  - "Board A: the LCD1602 screen, its 10 kΩ contrast knob and a 220 Ω resistor (red, red, black, black, brown), wired as in Lesson 53"
  - "Board A: 17 more jumper wires"
  - "Board B: the Mega, breadboard, LoRa modem and LED matrix from Lesson 54"
  - "A USB power bank, to carry Board A about"
  - "Paper and a pencil, for a table of results"
  - "Perhaps, for a small home: a metal biscuit tin big enough for Board B, and a sheet of card"
ideas:
  - Testing a link by sending messages you know and counting what comes back
  - The time every message costs, however short
  - Trading speed for range
---

## What you'll build

<!-- closeup A -->

A meter for the radio link between your two boards. Click the joystick
on Board A, the meter, and it sends 64 numbered messages to Board B, one
after another. Board B sends each one straight back. Every message that
makes it there and back lights its own dot on Board A's matrix, so a test
fills the matrix like a page of writing, and a lost message leaves a dark
gap. Then the screen says what share came back and how long one took
there and back: **95% back 112ms**. Board B's matrix shows which messages
reached it.

Then you carry Board A away until messages start to go missing, and find
out what the modem's slow setting, **Far**, can do there, and what it
costs. This is the course's last lesson: it puts the two boards, the
modems, the screen and the matrix you have built up to work on one
question, how good is the link?

!!! warning "915 MHz is for the Americas and Australia"
    The RYLR896 modems send on 915 MHz, which anyone may use in the USA
    and Canada. In Australia, add `.band = 921500000` to the settings on
    both boards, after `.power`. In Europe, add `.band = 869525000`, as
    for the bridge: that band lets each radio send for a tenth of each
    hour, six minutes. A test at Far keeps each board on the air for up
    to about 45 seconds, so there run no more than seven Far tests in an
    hour; Quick tests use far less. Receiving is fine anywhere; before you
    send, read [Radios](../../safety.md#radios) on the safety page.

## The idea

**Testing a link.** The bridge in Lessons 43 to 54 hid its lost
messages: it sent everything again every two seconds, so a lost change
came a moment late, and you hardly noticed. To find out how good a link
really is, send messages you know and count the ones that come back.
Each of the meter's messages starts with its number, from 1 to 64, so a
late one can't be counted as the next. And the modem's checksum throws
away any message that noise got into, so every message arrives whole or
not at all. Board B sends back whatever it hears; a message lost on the
way there, or on the way back, leaves a gap on Board A's matrix.

Why 64? With five messages, one lost is 20%, and two tests in the same
place could differ by 20% by luck alone. With 64, one lost message is
less than 2%, so a difference between two settings of more than a few
percent means something.

**Every message pays a toll.** In [Lesson 40](../040-lora/index.md) a
LoRa modem sent each piece of a message as a chirp. Before the first
letter, it sends a warm-up of chirps for the other modem to lock onto,
and a header that says how long the message is: a toll, paid by a
message of one letter or sixty. Then each letter costs a little more. The
two speeds from Lesson 43 chirp at different rates: a chirp at **Quick**
lasts about 1 ms, and at **Far**, 8 ms. From the radio chip's datasheet,
in round numbers:

<p class="formula">Quick: time on the air ≈ 20 ms + 1.5 ms × letters</p>

<p class="formula">Far: time on the air ≈ 200 ms + 8 ms × letters</p>

The meter times each message there and back: twice its time on the air,
and a little more while the Megas and the modems pass it along their
wires.

**Speed for range.** At Far, every chirp lasts eight times as long, so
the receiving modem has eight times as long to pick it out of the hiss.
Lesson 40 said LoRa can still hear a message whose margin above the noise
is as low as −15 dB: that was at Far. At Quick the margin must stay above
about −7.5 dB, by Semtech's datasheet for the chip. So Far can hear a
signal between five and six times weaker than Quick can: further away,
or through more walls. The price is time. Lessons 43 to 54 chose Quick,
so a button press crossed the house in a tenth of a second.

**The lowest power.** The meter's two modems send at 0 dBm, one
milliwatt: a tenth of the bridge's 10 dBm. That brings the edge of the
link close enough to find on foot.

!!! question "Predict"
    Use the toll to work it out before you build. At Quick, about how
    long will a 20-letter message take there and back? And a 60-letter
    one? About how long will a whole test of 64 twenty-letter messages
    take? Then the same test at Far? Write down your answers.

## Build it

!!! warning "Unplug first"
    Unplug each board's USB cable, or its power bank, before you change
    any wiring, and check your work before you plug it back in.

!!! danger "3.3 V for the modems"
    Each modem's VDD stays on its Mega's **3.3V** pin, never 5V: more than
    3.6 V damages it. Its RXD only ever sees the Mega's TX3 through the
    1 kΩ, with the 2 kΩ to GND. Never power a modem without its spring
    aerial.

Both boards carry on from Lesson 54, and both modems stay at the
bridge's home, with their dividers, just as they are. If Board B is still
in another room, bring it back to the computer.

- **Board A**, the meter, keeps everything: the modem, the matrix, the
  joystick and the buzzer. The screen comes back to its home, as in
  Lesson 53: its contrast knob across the middle gap in columns 43 to 45,
  its pins in a47 to a62, and its six signal wires from pins 31 to 36.
  Its body hangs off the bottom edge beyond the end of the breadboard:
  prop it up at breadboard height. The screen takes its power from the
  top rails, so a red wire comes from the Mega's 5V into the top + rail
  by column 3, as in Lesson 3.
- **Board B**, the echo, keeps its modem and its matrix. Take out its
  joystick and the joystick's five wires, and the buzzer, its 220 Ω
  resistor and pin 10's wire.

!!! warning "Pause points while you build"
    Stop after each board:

    - **Board A's screen:** With the USB cable out, check that the
      screen's header pins sit in separate strips, its contrast knob
      crosses the middle gap, and its backlight has the 220 Ω resistor
      shown in the drawing. Check that the + and − rails are separate and
      no loose wire touches both. **Predict:** what will holding the stick
      to the right do to `Quick 20 letters`? Plug in and upload **Meter**
      to Board A, leaving the stick alone while it starts. The screen
      should say `Quick 20 letters` and `Click to test`; holding the stick
      right should count up, `25 letters`, `30 letters`, and holding it
      left count back down. Don't click yet: Board B isn't answering.
    - **Board B:** With its USB cable out, check that nothing is left in
      the holes of the joystick's and buzzer's wires, and that the
      modem's wires and the matrix's are as they were.

### Board A

<!-- bench A -->

<!-- steps A -->

### Board B

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections each board makes. Board A:

<!-- connections A -->

Board B:

<!-- connections B -->

## Code it

Open **File → Examples → Adk → lessons → 055-reliability-meter → Meter**
for Board A:

<!-- sketch A -->

What's new:

- `constexpr adk::LoraSpeed speed = adk::LoraSpeed::Quick;` names the
  speed once, at the top, and the modem's settings use it: `.speed =
  speed`. Board B's sketch has the same line. Two modems at different
  speeds can't hear each other at all, so to try Far you change it in
  both sketches.
- `.power = 0` is the modem's lowest power, 0 dBm.
- There is no bridge. The meter talks to the modem itself, with
  `radio.send ()`, `radio.wasReceived ()` and `radio.text ()`, as in
  Lesson 40. A bridge sends everything again every two seconds, which
  would cover up the very losses the meter is there to count.
- `sendNext ()` builds each message in an `adk::Text<60>`, as Lesson 54
  built its score: its number, such as `17 `, and then letters of the
  alphabet until it is as long as the screen says. If the modem takes it,
  `++sent` counts it, and `trip.restart ()` starts the stopwatch.
- `adk::Timer giveUp;` is how long to wait for an echo:
  `2 * airTime () + 500`, the formula's time on the air there and back,
  and half a second to spare. `airTime ()` is the formula from
  [The idea](#the-idea), in code. Whole numbers can't hold 1.5, so
  `length * 3 / 2` stands for 1.5 × letters.
- `message == radio.text ()` is true when what arrived is exactly the
  message that is out. Then `back = trip.elapsed ()` keeps its time there
  and back, `giveUp.stop ()` stops the wait, and `++heard` counts it. A
  late echo of message 2 starts with a 2, so it can't count as message 3.
- `!giveUp.isRunning ()` is true when nothing is out: the echo came back
  and stopped the timer, or the timer ran out and the message is lost.
  Either way, the next one goes, until all 64 have gone.
- `matrix.set ((sent - 1) % 8, (sent - 1) / 8)` lights the dot for
  message number `sent`. As in Lesson 54's flight, `% 8` is what is left
  over after dividing by 8, which counts 0 to 7 along a row, and `/ 8`,
  a whole-number division, says which row: messages 1 to 8 light the top row, 9 to 16 the next,
  and 64 the bottom right corner.
- `joystick.x () / 60` is −1, 0 or 1, as it moved the paddle in
  Lesson 54. Every quarter second while the stick is held, the length
  goes up or down by 5, and `constrain ()` keeps it from 5 to 60.
- `finish ()` shows `heard * 100 / tries`, the share that came back in
  percent, and the latest time there and back. It beeps high when all 64
  came back and low when any were lost, so you can tell without looking.

Board B's sketch, **Echo**, is short:

<!-- sketch B -->

- `radio.send (radio.text ())` sends every message straight back, word
  for word, to its partner, Board A.
- `atoi (radio.text ())` reads the number at the start of the message.
  `atoi`, short for "ASCII to integer", reads digits until the first thing
  that isn't one, so `17 defg` gives 17.
- `showHeard ()` lights that message's dot, worked out as on Board A.
  A number lower than the last one means a new test has begun, so it
  clears the matrix first.

## Upload it

1. Plug in Board B, choose its port in **Tools → Port**, and upload
   **Echo**. Its matrix stays dark.
2. Plug in Board A, choose its port, and upload **Meter**, if it isn't
   there already, leaving the stick alone while it starts. The screen
   says `Quick 20 letters` and `Click to test`.
3. With the boards a meter or two apart, click the stick. Dots fill both
   matrices, row by row, and the bottom row of the screen counts:
   `Sent 23 heard 22`, the message just sent and the echoes back so far.
   About ten seconds later the meter beeps, and the screen shows the
   result, such as `100% back 117ms`.
4. Hold the stick right until the screen says `60 letters`, and click
   again; then try `5 letters`.

Did you predict about 100 ms for 20 letters? Each way, the toll is 20 ms
and twenty letters add 30 ms: 50 ms, so 100 ms there and back. Your
meter shows a little more, for the time the Megas and the modems take
to pass each message along their wires. Sixty letters take about 220 ms
there and back, five about 55 ms. Sixty letters carry twelve times as
much as five, but take only about four times as long, because the toll
is paid either way: long messages carry their letters more cheaply. A
whole test of 64 twenty-letter messages takes 64 times 100 ms, between
six and seven seconds, and a little more. At Far, a 20-letter message
takes 200 + 160 = 360 ms each way, 720 ms there and back, about seven
times as long, and a test most of a minute.

On the desk, every test should say 100%: the signal is far stronger than
the noise.

## Find the edge

!!! question "Predict"
    Somewhere away from Board B, the link at Quick will start to lose
    messages. In that same place, will Far lose more of the 64, fewer, or
    the same? And how long will each test take? Write down your answers.

1. Leave Board B where it is, plugged into the computer. Unplug Board A,
   and run it from the power bank instead: it keeps its sketch.
2. Carry Board A away from Board B, testing at Quick (20 letters) every
   few steps: into the next room, through more walls, up or down stairs,
   to the far end of the home or the garden. Concrete and metal stop the
   most; a body between the boards stops some too.
3. Stop at the first place where a test comes back below 100%, and test
   twice more there. Between about 30% and 90% is best: move a little
   further or closer to find it. Mark the place, and write down what each
   test said, in a table like this:

    | Place | Quick | Far |
    |---|---|---|
    | On the desk | 100%, 100% | |
    | By the back door | 64%, 58%, 70% | |

4. Change the speed to `adk::LoraSpeed::Far` in both sketches. Upload
   **Echo** to Board B, and bring Board A back to upload **Meter** too.
   The screen says `Far 20 letters`.
5. Take Board A back to the mark, and test there twice. Each test takes
   most of a minute now: wait for the beep.
6. If you like, carry on further until Far starts to lose messages too.

If even the far end of your home gets 100% at Quick, the edge is
further than you can walk. Then make it harder for the signal: stand
Board B on a sheet of card, so nothing on it can touch metal, and put it
inside a metal biscuit tin, its USB cable out under the lid. Metal all
round stops most of a radio wave. Start again from step 2.

You predicted what Far would do. In the place where Quick lost some of
its messages, Far should get most or all of them back, but each test
takes far longer: most of a minute instead of a few seconds, because
every message is about seven times as long on the air. That is the
trade the modem's two settings offer. Far waits longer on every chirp,
so it can pick out a weaker signal, and reach further; Quick answers
fast, and needs the stronger signal. The bridge chose Quick, at ten times
the meter's power, because a house is well within its reach.

Radio is changeable: the same place can give 60% one minute and 80% the
next, as people move and doors open and close. That's why each test sends
64 messages, and why you test more than once before you compare. Look at
the gaps on the matrix too. Gaps scattered one by one are noise at the
edge; a long run of dark dots means something changed while the test ran,
such as someone walking between the boards.

## If it doesn't work

| What you see | Try this |
|---|---|
| `None came back`, and Board B's matrix stays dark | Board B hears nothing. Check that Board B runs **Echo** and Board A **Meter**, and that `speed` is the same in both sketches: at different speeds, the modems can't hear each other at all. Then check Board B's modem against the steps. |
| Board B's matrix fills, but Board A's stays dark | Board B hears the messages and sends them back, but Board A hears nothing. Check Board A's modem: its TXD up into f26, and pin 15's wire in j26. |
| The screen says `No modem reply` | Check Board A's modem: VDD to the Mega's 3.3V pin, GND to the bottom − rail by column 24, TXD up to f26 and RXD to c28, not swapped. Then the divider: pin 14 in j28, the 1 kΩ from g28 to e28, the 2 kΩ from a28 to the − rail. Press the reset button to try again. |
| A test crawls, every dot dark | Nothing is answering, so the meter waits for each message in turn, up to two seconds at Far. Press Board A's reset button to stop it, and see the first row of this table. |
| Fewer than 100% with the boards side by side | Keep them a meter or two apart, away from the computer and its cable, and keep the springs straight, pointing down. Another LoRa radio on 915 MHz nearby can spoil a message now and then. |
| Holding the stick does nothing, or works the wrong way | Hold the joystick with its pins to your left, as in Lesson 54, and leave it alone while the sketch starts: where it rests then is its middle. Check its VRx goes to A3 and VRy to A4. |
| Clicking does nothing | Check the joystick's SW goes to pin 22. A click is ignored while a test runs. |
| No beep | Check the buzzer's + leg is in f33, beside pin 10's wire in j33, and its 220 Ω from a33 to the − rail. |
| A blank lit screen, or a row of blocks | Turn the contrast knob beside the LCD. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    With the screen at `Quick 20 letters`, message 17 goes from Board A to
    its modem as this line:

    ```text
    AT+SEND=2,20,17 defghijklmnopqrst
    ```

    Board B's modem hears it and tells its Mega the sender, the length,
    the text, the signal in dBm and the margin in dB:

    ```text
    +RCV=1,20,17 defghijklmnopqrst,-58,9
    ```

    Board B sends the text straight back with `AT+SEND=1,20,...`, and
    Board A's modem tells its Mega `+RCV=2,20,17 defghijklmnopqrst,...`.

    Where the formula comes from: at Quick, each chirp sweeps the 125 kHz
    band in 1.024 ms and carries 7 bits; at Far, 8.192 ms and 10 bits. For
    every four bits of message the modem adds a spare one, to put right
    what noise gets wrong, so a letter, 8 bits, needs 10: about one and a
    half chirps at Quick, 1.5 ms, and one chirp at Far, 8 ms. The warm-up
    is about 8 chirps at Quick and 11 at Far, and the header and the
    checksum take about a dozen more: the toll, about 20 ms or 200 ms. The
    modem adds a few bytes of its own, such as who the message is from,
    so the real time is a little longer.

## Check yourself

1. Why does the meter send 64 messages in a test, and not 5?
2. After a test, Board B's matrix has the dot for message 41 lit, but
   Board A's doesn't. What happened to message 41?
3. In one place, Quick got 38 of 64 back and Far all 64. What did Far
   give up to get them through? Why did the bridge use Quick anyway?

??? note "Answers"
    1. With five messages, one lost is 20%, and two tests in the same
       place can differ that much by luck alone. With 64, one lost
       message is less than 2%, so a real difference between two settings
       stands out.
    2. It reached Board B, which lit its dot and sent it back, but the
       echo was lost on the way back to Board A, or came back after Board
       A had given up waiting for it.
    3. Time: each message took about seven times as long on the air, so
       a test took most of a minute instead of a few seconds. Far's longer
       chirps let a modem hear a weaker signal. The bridge needed to
       answer quickly, a press in a tenth of a second, and across a house
       Quick at 10 dBm has signal to spare.

## Make it yours

1. **Long or short, at the edge.** Go back to a place where Quick loses
   some messages, and test 5 letters and then 60 letters there, twice
   each. Which loses more? A long message has more chirps for noise to
   spoil, and the modem's spare bits can put right only so much.
2. **More power.** Change `.power = 0` to `.power = 10` on both boards,
   the bridge's power, and find Quick's edge again. How much further is
   it? Never go above 10: the modem's VDD comes from the Mega's 3.3V pin,
   which can't feed it at full power.
3. **How close to the edge.** At the end of `finish ()`, show the
   margin of the latest echo over the top row:
   `adk::print (lcd.at (0, 0), "Margin ", radio.margin (), " dB   ")`.
   At Quick, messages start to go missing as it nears −7.5 dB; at Far,
   nearer −15.
4. **Letters a second.** After a test in which some came back, show how
   many letters a second it carried one way: `length * 2000 / back`.
   Which length and which speed carries the most?

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings on
Board A between tests, while the serial lines to the modem rest, and keep
each probe tip in its own hole.

!!! question "Predict"
    The modem's RXD gets the Mega's 5 V through the divider. Its TXD goes
    straight to pin 15, with no divider. Between messages, both lines
    rest high. What will each read? Is the modem's high enough for the
    Mega to read as high?

<!-- measure A -->

What the numbers tell you:

- **The modem's RXD** reads about 3.3 V: the divider shares pin 14's 5 V
  between the 1 kΩ and the 2 kΩ, so the modem's pin never sees more than
  it can take.
- **The modem's TXD** reads about 3.3 V too, but this is the modem's own
  high, from its 3.3 V supply, and nothing divides it. The Mega reads
  anything above 3 V as high (0.6 × 5 V, by the ATmega2560's
  datasheet), so the modem's 3.3 V gets through with 0.3 V to spare: a
  margin, like the radio's margin above the noise. While a test runs,
  both lines flicker far too fast for the meter to follow.
