---
lesson: 43
promise: Join two Megas by radio, so that a button on each lights an LED on the other, and find out what it means for two boards to be connected.
time: 1½ hours
level: 3
parts:
  - "Board A: the Mega 2560 and breadboard you've used so far, and the USB cable"
  - "Board B: a second Mega 2560 with its USB cable, and a second breadboard (not in one kit)"
  - "Two REYAX RYLR896 LoRa modems, one for each board: the pair from Lesson 40 (add-on, not in the kit)"
  - "For each board: a push button, a red LED and a yellow LED"
  - "For each board: 2 × 220 Ω resistors (red, red, black, black, brown), a 1 kΩ (brown, black, black, brown, brown) and a 2 kΩ (red, black, black, brown, brown)"
  - "For each board: 4 female-to-male jumper wires and 9 jumper wires"
  - "A USB phone charger or power bank, to run Board B away from the computer"
ideas:
  - Two boards sharing named numbers over a radio
  - Changes that go at once, but at most ten a second
  - Everything sent again every two seconds
  - What "connected" means
laws:
  - {law: decibels, section: the-idea, for: "Sends at 10 dBm, about a third of full power"}
  - {law: divider, section: measure-it, for: "Drops TX3's 5 V to about 3.3 V for RXD"}
  - {law: budgets, section: the-idea, for: "Fits one 10 dBm modem on the 3.3 V pin"}
  - {law: logic-levels, section: build-it, for: "Keeps the 3.3 V modem's RXD off 5 V"}
  - {law: serial, section: measure-it, for: "Explains TX3 resting high between messages"}
---

## What you'll build

<!-- closeup A -->

Two Megas, each with a button, a red LED, a yellow LED and a LoRa modem.
Hold the button on Board A, and a moment later Board B's red LED lights;
let go, and it goes out. Board B's button does the same to Board A's red
LED. The yellow LEDs say that the two boards can hear each other: unplug
one board, and a few seconds later the other's yellow LED goes out too.
Board B can run from a power bank in another room: LoRa is made to reach
through a few walls.

This is the first of the lessons for two boards. Each project in them is
split between two Megas, **Board A** and **Board B**, that talk by radio.
Board A is the Mega you have used all along; Board B is a second one.

!!! warning "915 MHz is for the Americas and Australia"
    The RYLR896 modems send on 915 MHz, which anyone may use in the USA
    and Canada. In Australia, add `.band = 921500000` to the settings on
    both boards, after `.power`, to keep inside Australia's 915–928 MHz.
    Europe limits how much of the time a radio may send, and a bridge
    sends often, so there add `.band = 869525000` on both boards, after
    `.power`: that band allows a tenth of the time. Receiving is fine
    anywhere; before you send, read [Radios](../../safety.md#radios) on
    the safety page.

## The idea

**Sharing numbers, not messages.** In Lesson 40 one modem sent a message
and the other showed it. Most projects for two boards want something
simpler: a number on one board that is the same on the other. It might
be whether a button is held, an angle, or how far away something is. ADK
has a part for that, a **bridge**. One board **shares** a number under a
name, and the other reads the latest **value** by that name:

| Board A | | Board B |
|---|---|---|
| `bridge.share ("button", 1)` | → radio → | `bridge.value ("button")` is 1 |

A name is a short word in quotes, up to 7 letters, and each board can
share up to eight of them. Both boards do both jobs: each shares its own
numbers and reads the other's.

**Changes go at once, but at most ten a second.** The bridge sends a
number only when it changes, and straight away, so a press reaches the
other board in about a tenth of a second. It never sends more than ten
messages a second, though. A knob turned fast would otherwise fill the
air with messages, and the other board only ever wants the latest value.
When several numbers change together, they go together in one message.

**Everything again every two seconds.** A message can be lost: to noise,
or because both boards spoke at the same moment and neither heard the
other. So every two seconds each board sends everything it shares,
changed or not, and a lost change is made good within two seconds. The
messages are plain text, which you'll see in the Serial Monitor:

```text
@1/1 button=0
```

The `@` says it's a bridge's message. Then come two start numbers, `1/1`,
which *How it works* explains, and then each name, `=`, and its value.

**Connected.** Those messages every two seconds mean that each board
hears from the other at least that often, even when nothing changes. So
a board that hears nothing for five seconds knows the other has gone:
switched off, unplugged, or out of range. `bridge.isConnected ()` is
true when it heard from the other board in the last five seconds; five,
not two, so that one lost message doesn't count as gone.

| What happens | What the other board hears |
|---|---|
| A number changes | The new value, within about a tenth of a second |
| Nothing changes | Everything, every two seconds |
| Nothing for five seconds | Nothing: `isConnected ()` turns false |

**Two addresses.** Each modem has an address, as in Lesson 40: Board A's
is 1 and Board B's is 2, and each names the other as its **partner**,
where its messages go. That is the only difference between the two
boards' sketches. They send at the **Quick** speed: a message takes
about 0.05 s on the air instead of 0.3 s, and reaches about a kilometer
instead of a few, which is plenty across a house and makes the bridge
answer quickly.

**One modem on the Mega's 3.3 V.** In Lesson 40 the two modems ran from
the power module, because a modem sending at its full power draws up to
about 50 mA by REYAX's datasheet: as much as the Mega's 3.3V pin can
give. Here each board has one modem, set to send at 10 dBm, about a
third of that power. REYAX gives no figure for that, but the radio chip
inside the modem draws about 29 mA even at 13 dBm, by its maker's
datasheet, so the modem's VDD goes straight to the Mega's own 3.3V pin
and neither board needs a power module. The Mega's TX3 pin still reaches
the modem's RXD through a divider, as in Lesson 40.

!!! question "Predict"
    Once both boards are running, you'll unplug Board B. How long will it
    be before Board A's yellow LED goes out: at once, after two seconds,
    or later? And what will Board A's red LED do if Board B's button is
    held down as you unplug it? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug each board's USB cable before you wire it, and check your work
    before you plug it back in.

!!! danger "3.3 V for the modems"
    Each modem's VDD goes to its Mega's **3.3V** pin, on the power
    header, never to 5V or a rail: more than 3.6 V damages it. Its RXD
    only ever sees the Mega's TX3 through the 1 kΩ, with the 2 kΩ to GND.
    The spring is the aerial: never power a modem without it.

Stick a label on each Mega, **A** and **B**, so you always know which is
which. Then build the two boards the same way. Board A carries on from
Lesson 42: keep the 220 Ω resistor from g6 to e6 and the black GND wire
into the bottom − rail by column 3, and take everything else off. That
resistor will feed the red LED. If you skipped Lesson 42, keep only the
GND wire, and put a 220 Ω resistor across the middle gap from g6 to e6.
Board B starts from an empty breadboard.

- **The button** on 22 and **the LEDs** on 26 (red) and 27 (yellow) go
  in at their homes, as in Lesson 2: the button across the middle gap in
  columns 2 to 4, the red LED in column 6 and the yellow in column 12.
- **The modem** lies below the board with its spring pointing down, under
  columns 24 to 29: modem B's place in Lesson 40, wired the same way but
  for its power. This is the modem's **bridge home**. It stays in these
  same holes, on both boards, in every lesson for two boards, so later
  lessons only change what's around it.

### Board A

<!-- bench A -->

<!-- steps A -->

### Board B

Board B is built just as Board A is, on its own Mega and breadboard:

<!-- bench B -->

<!-- steps B -->

??? info "The modem's pins"
    | Pin | Job | Goes to |
    |---|---|---|
    | VDD | Power, 3.3 V | The Mega's 3.3V pin |
    | NRST | Restarts the modem when pulled low | Nothing |
    | RXD | Listens to the Mega | c28, the middle of the divider |
    | TXD | Talks to the Mega | f26, beside pin 15's wire in j26 |
    | NC | Not connected inside | Nothing |
    | GND | Ground | The bottom − rail by column 24 |

    The names are printed on the board beside its pins. Lying below the
    breadboard, spring down, the pins run from GND on the left to VDD on
    the right, as in Lesson 40.

When you are done, these are the connections each board makes; Board B's
are the same as Board A's:

<!-- connections A -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → lessons →
043-the-bridge → BoardA**:

<!-- sketch A -->

What's new:

- `adk::LoraModem radio {Serial3, 1, {...}};` is the modem on `Serial3`,
  pins 14 and 15, with address 1. The settings in braces name only what
  differs from the usual: `.partner = 2` is where its messages go,
  `.speed` is `Quick`, and `.power` is 10 dBm. Anything left out keeps
  its usual value, such as network 6. Settings keep the order
  partner, speed, power, network, band, as in Lesson 40, so a band
  or a network you add goes after `.power`, and the band last.
- `adk::Bridge bridge {radio};` is the bridge, over that modem. It goes
  straight after the modem it uses.
- `bridge.share ("button", button.isPressed ())` shares this board's
  button: `true` travels as 1 and `false` as 0. Calling it in every pass
  of `loop ()` costs nothing, as only a change is sent.
- `bridge.value ("button")` is the other board's latest value for
  `button`, or 0 until one has arrived. A board's own shares and the
  other's values are kept apart, so both boards can use the name
  `button` without mixing them up. `light.set (...)` lights the red LED
  while it's 1.
- `bridge.isConnected ()` lights the yellow LED while the other board
  can be heard.
- `radio.ok ()` and `radio.wasReceived ()` come from Lesson 40. The
  sketch says if the modem didn't answer, and prints every message that
  arrives, so you can watch the bridge talk.

Board B's sketch, **BoardB**, is the same but for one line: its modem is
address 2, and its partner is 1.

<!-- sketch B -->

## Upload it

!!! tip "Two Megas on one computer"
    Each Mega plugged in gets a port of its own, and **Tools → Port** lists
    both, with names that don't say which is which. To tell them apart,
    plug in one board at a time and see which port appears; or, with both
    in, unplug one and watch which port goes. A label on each Mega's USB
    cable saves asking again.
    {: #two-megas-on-one-computer }

    Each Arduino IDE window keeps its own Board, Port and Serial Monitor,
    so open each board's sketch in a window of its own and choose that
    board's port there: then the two never mix.

1. Plug in Board A, choose its port in **Tools → Port**, and upload
   **BoardA**. Its yellow LED stays dark: there's nobody to hear yet.
2. Plug in Board B, choose its port (a different one: see
   [Two Megas on one computer](#two-megas-on-one-computer)), and upload
   **BoardB**. Within a second or two, both yellow LEDs light.
3. Hold Board A's button: Board B's red LED lights at once, and goes out
   when you let go. Board B's button does the same to Board A.
4. Open the Serial Monitor at 9600 baud, on either board's port. Every two
   seconds it says `Heard: @1/1 button=0`. Press the other board's button,
   and `Heard: @1/1 button=1` arrives straight away.
5. Once a board has its sketch, any USB power will run it. Plug Board B
   into a phone charger or a power bank, carry it into another room, and
   try the buttons again.

Now test your prediction. Unplug Board B, and count. Board A's yellow LED
goes out three to five seconds later. Board B's last message came at
most two seconds before you unplugged it, and Board A waits five seconds
from the last message it heard. Plug Board B back in: once it has
started, its first message goes out at once, and both yellow LEDs light
again.

Then hold Board B's button down and unplug it while you hold it. Board
A's red LED stays lit, even after its yellow LED goes out. The bridge
keeps the last value it heard, and nothing ever came to say the button
was let go. A board that must act safely when the other goes quiet has
to check `isConnected ()` as well: Lesson 45's fan does.

!!! note "What the bridge can lose"
    A radio message can go missing, and the bridge is built for that. This
    is everything it can lose, and what the sketches do about it. Later
    lessons point back here.
    {: #what-the-bridge-can-lose }

    - **A lost message.** Noise, or both boards sending at once, can lose
      one. Every two seconds the bridge sends everything again, so a lost
      value is back within two seconds.
    - **A quick change.** The bridge sends at most ten messages a second,
      and only the latest value. A number that goes 1, 2, 3 within a
      tenth of a second arrives as 3.
    - **A moment.** A press lasts one pass of `loop ()`, too short to
      share. A sketch sends a count of presses instead, and the other
      board watches it go up: *Make it yours* shows how.
    - **Old news.** When the other board goes quiet, its last values stay,
      as you just saw. A sketch that must not trust them checks
      `isConnected ()`.
    - **A restart.** When either board restarts, both notice, and each
      sends the other everything again at once. Nothing from before the
      restart is taken for new, and you never need to reset the other
      board.

## If it doesn't work

| What you see | Try this |
|---|---|
| The Serial Monitor says `No reply from the modem` | Check that board's modem: VDD to the Mega's 3.3V pin, GND to the bottom − rail by column 24, TXD up to f26 and RXD to c28, not swapped. Then the divider: pin 14 in j28, the 1 kΩ from g28 to e28, the 2 kΩ from a28 to the − rail. And pin 15 in j26. |
| Neither yellow LED lights | Check each board has its own sketch: Board A runs **BoardA**, Board B **BoardB**. Two boards with the same sketch have the same address, and both send to an address neither has. |
| The yellow LEDs light, but a red one never does | Check the button on the other board: pin 22 in j2, the button across the gap in columns 2 to 4, and a4 to the − rail. Then this board's red LED: pin 26 in j6, the resistor from g6 to e6, the long leg in b6, and a7 to the − rail. |
| The yellow LEDs flicker on and off | The boards barely hear each other. Bring them closer, keep the springs upright and away from metal, and keep your hand off them. |
| A modem gets warm | Unplug that board at once, and check the modem's VDD goes to the 3.3V pin, never 5V. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    When the sketch starts, ADK sets up each modem as in Lesson 40, but
    with these two lines for its power and speed:

    ```text
    AT+CRFOP=10
    AT+PARAMETER=7,7,1,4
    ```

    The second chirps with a spreading factor of 7 instead of 10: each
    chirp is eight times shorter, so a message takes about a sixth of the
    time on the air, and needs a stronger signal to be heard.

    When Board A's button is pressed, the bridge hands its modem the
    line `@1/1 button=1`, and ADK sends it to the partner, address 2:

    ```text
    AT+SEND=2,13,@1/1 button=1
    ```

    Board B's modem hears it and tells its Mega
    `+RCV=1,13,@1/1 button=1,-35,11`, and Board B's bridge reads each
    name and value after the start numbers. A board with several numbers
    to share sends them in one line, such as `@1/1 angle=90 fan=1`, as
    many as fit in 56 letters. Every two seconds each bridge counts
    everything as changed and sends it all again; a board with nothing to
    share sends just its start numbers, `@1/1`, so the other still hears
    from it.

    The start numbers are how the boards notice a restart. A board that
    has just started has none, and says `@0/0`. It takes one from the
    first message it hears: one more than the other board remembers for
    it, so never the one it had before. In every message the first number
    is the sender's own, and the second the other board's, as the sender
    last heard it. When Board B restarts, Board A sees a new number and
    sends everything again at once, and Board B ignores any message that
    still carries its old number: that was meant for it as it was before.

## Make it yours

1. **Switch it.** Make a press switch the other board's red LED on and
   off, instead of lighting it while held. A press lasts a single pass
   of `loop ()`, far too short to share, so share a count instead: add
   one to `presses` at each `button.wasPressed ()`, share it with
   `bridge.share ("presses", presses)`, and on the other board toggle
   the LED whenever `bridge.changed ("presses")`. A count only ever goes
   up, so even if a message is lost, the next one still brings a number
   that has changed.
2. **How strong.** Print `radio.signal ()` beside each message, as
   Lesson 40 showed it, and walk Board B away on its power bank. Watch
   the signal fall, and see where the yellow LEDs start to flicker.
3. **Far.** Change `.speed` to `adk::LoraSpeed::Far` on both boards. Does
   the red LED answer the button more slowly? Does the link reach
   further? In Europe, at Far each board's refresh alone fills more than
   a tenth of the time, so keep Quick there.
4. **Your own network.** Add `.network = 12` to the settings on both
   boards, so another pair of boards nearby won't hear yours.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings on
Board A while both boards run and nobody presses a button, and keep each
probe tip in its own hole. Board B reads the same.

!!! question "Predict"
    Between messages, a serial line rests high, so pin 14, TX3, sits at
    5 V. What will the modem's RXD read, at the middle of its divider?

<!-- measure A -->

What the numbers tell you:

- **Pin 14** reads about 5 V: TX3 rests high between messages. Every two
  seconds the bridge's message flickers it between 5 V and 0 V for a few
  thousandths of a second, far too fast for the meter to show.
- **The modem's RXD** reads about 3.3 V: two thirds of 5 V, as the
  divider shares it out, safe for the modem's 3.3 V pin. The 1 kΩ takes
  the other 1.7 V.

## Check yourself

1. You unplug Board B. Why does Board A's yellow LED go out a few seconds
   later, and not at once?
2. You hold Board B's button and unplug Board B. Why does Board A's red LED
   stay lit, and what should a careful sketch also check?
3. In Lesson 40 the modems ran from the power module. Why can each modem
   here run from its Mega's 3.3V pin?

??? note "Answers"
    1. `bridge.isConnected ()` turns false only after five seconds with
       nothing heard. Board B's last message came up to two seconds before
       you unplugged it, so the LED goes out three to five seconds later.
    2. The bridge keeps the last value it heard, and no message ever came to
       say the button was let go. A sketch that must act safely checks
       `bridge.isConnected ()` as well.
    3. Each board has just one modem, and it sends at lower power, so it
       draws less current than a modem at full power. That fits within what
       the Mega's 3.3V pin can give.
