---
lesson: 40
title: LoRa
arc: Long range
promise: Send messages from one LoRa modem to another, and see how strong each one arrives.
time: 60 minutes
level: 2
sketch: Lesson40Lora
parts:
  - The screen and button from Lesson 39, wired as before
  - Two REYAX RYLR896 LoRa modems (add-on, not in the kit)
  - Breadboard power module and its 9 V adapter
  - 2 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 2 × 2 kΩ resistors (red, black, black, brown, brown)
  - 8 female-to-male jumper wires
  - 4 jumper wires
ideas:
  - LoRa, which trades speed for range
  - Addresses and a network number
  - Signal strength in dBm
  - Commanding a modem in words
---

## What you'll build

<!-- closeup -->

Two little blue LoRa modems hang below the breadboard, each with a gold
spring for an aerial. Press the button and modem A sends **Press 1** by
radio; modem B hears it, and the screen shows the message with two numbers
under it: how strong it arrived, and how far it stood above the hiss of
radio noise. Type a line in the Serial Monitor and it goes the same way.
Here the two modems are a hand's width apart, but LoRa is made to reach a
kilometre or more.

!!! warning "915 MHz is for the Americas and Australia"
    These modems send on 915 MHz, which anyone may use in the Americas and
    Australia. Europe and many other countries use 868 MHz instead, with
    rules of their own. Receiving is fine anywhere; before you send, read
    [Radios](../../safety.md#radios) on the safety page.

## The idea

**Slow, but far.** LoRa is short for *long range*. A LoRa radio sends each
piece of a message as a **chirp**, a sweep from a low note to a high one,
like a bird's call but far too high to hear. The receiver knows exactly
what shape of chirp to listen for, so it can pick one out even when it is
weaker than the hiss of noise all around it, the way you can hear your own
name across a noisy room. The price is speed. The Mega talks to each modem
at 115,200 bits a second, but the modems talk to each other at about 1,000,
so a short message takes about a third of a second on the air. In return,
it can cross a kilometre or more of open ground.

**A link needs two ends.** A radio that sends needs another that listens,
so this lesson uses two modems, and they come in pairs. Both are on one
Mega, so you can see both ends at once: modem A on the serial port
**Serial1** and modem B on **Serial3**. The real fun comes when the second
modem goes on another board, far away; *Make it yours* says how.

**Talking to a modem in words.** A serial port is two wires: one for each
way. The Mega's **TX** pin sends and its **RX** pin listens, so they cross
over: the Mega's TX goes to the modem's **RXD**, and the modem's **TXD** to
the Mega's RX, like a mouth to an ear. What goes along them is plain text.
The Mega sends the modem commands in words, one line at a time, and the
modem answers in words too.

**Addresses and a network.** Every modem has an **address**, a number, and
passes on to its Mega only the messages sent to that address, or to address
0, which means everyone. Modem A is address 1 and modem B is address 2.
Every modem also has a **network number**, here 6, and it ignores modems on
any other network, so two pairs of modems in one street can keep out of each
other's way.

**Signal strength in dBm.** Modem B says how strong each message arrived,
in **dBm**: decibels compared with one milliwatt. Every 10 dBm down is ten
times weaker, and what arrives is always far less than a milliwatt, so the
numbers are negative: about −40 across a room, and −120 at the edge of
range.

<p class="formula">−40 dBm to −120 dBm = 8 steps of 10 dB ≈ 100,000,000 times weaker</p>

It also gives a **margin**: how far the signal stands above the noise, in
dB. Most radios need a margin well above 0; LoRa's chirps can still be heard
down to about −15.

**3.3 V.** The modems work on 3.3 V, like the FM radio and the little
transmitter in Lessons 37 and 38. Each draws about 50 mA while it sends,
about as much as the Mega's 3.3V pin can give at all, so the modems take
their power from the power module, with its bottom jumper set to **3.3V**.
The Mega's TX pins reach them through a divider, as the transmitter's DAT
did in Lesson 38:

<p class="formula">RXD = 5 V × <span class="fraction"><span>2 kΩ</span><span>1 kΩ + 2 kΩ</span></span> ≈ 3.3 V</p>

!!! question "Predict"
    The sketch sends to address 2, modem B. Once it works, you'll change
    that 2 to a 3, and then to a 0. For each, will modem B show the
    message? And will modem A know whether anyone heard it? Write down your
    guesses.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire,
    and check your work before you plug them back in.

!!! danger "3.3 V for the modems"
    Set the power module's **top** jumper to **5V**, for the screen, and its
    **bottom** jumper to **3.3V**, for the modems, before you plug anything
    in. The modems' VDD pins go to the bottom + rail, never 5 V: more than
    3.6 V damages them. Their RXD pins only ever see the Mega's TX through
    the 1 kΩ, with the 2 kΩ to GND.

Keep the screen and the button from Lesson 39 just as they are, and take
everything else off: the clock module, the rotary encoder, the FM radio and
the volume knob, and the red wire from the Mega's 5V to the top + rail. The
power module feeds both pairs of rails now; the black GND wire into B-3
stays. The power module goes on the right end of the board.

The modems stand below the board, past the button, their springs pointing
down and away: modem B first, then modem A, so their wires meet the Mega's
pins in the order the pins sit on the Mega. Each takes a few columns of its
own:

- **TXD** comes straight up into row f, and the Mega's RX pin's wire lands
  in row j of the same column.
- **The divider** stands in the column two to the right: the Mega's TX
  pin's wire into row j, the 1 kΩ across the middle gap from g to e, and
  the 2 kΩ from a straight down into the − rail. Lesson 38's divider lay
  along row h; this one stands across the gap, so the whole lower half of
  its column is the divider's middle.
- **RXD** comes up into row c of that column, the divider's middle.
- **VDD** goes to the bottom + rail and **GND** to the bottom − rail.

<!-- bench -->

<!-- steps -->

??? info "The modems' pins"
    | Pin | Job | Goes to |
    |---|---|---|
    | VDD | Power, 3.3 V | The bottom + rail, at 3.3 V |
    | NRST | Restarts the modem when pulled low | Nothing |
    | RXD | Listens to the Mega | The middle of its divider |
    | TXD | Talks to the Mega | Pin 19 for modem A, pin 15 for modem B, through row j |
    | NC | Not connected inside | Nothing |
    | GND | Ground | The bottom − rail |

    The names are printed on the board beside its pins. Go by the names:
    lying below the breadboard, spring down, a modem's pins run from GND
    on the left to VDD on the right. The radio chip hides under a metal can
    labelled **RYLR890**; that's normal for an RYLR896.

    The spring is the aerial, soldered on. Keep the two springs upright and
    apart, and don't bend them.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson40Lora**:

<!-- sketch -->

What's new:

- `adk::LoraModem modemA {Serial1, 1};` is a modem on the serial port
  `Serial1`, pins 18 (TX1) and 19 (RX1), with address 1. Modem B is on
  `Serial3`, pins 14 (TX3) and 15 (RX3), with address 2. `Serial` itself
  belongs to the USB cable and the Serial Monitor. Settings in braces
  after the address could choose the network, 6 unless you say otherwise.
- `modemA.ok ()` says whether the modem answered when the sketch started;
  the screen says which did.
- `modemA.send (2, text)` sends the text to address 2. It comes straight
  back while the modem sends, and says `false`, sending nothing, if the
  modem is still busy with the last message, or the text is longer than
  60 letters.
- `modemB.wasReceived ()` is true for one pass of `loop ()` when a message
  arrives. `modemB.text ()` is what it said, `modemB.sender ()` the address
  it came from, `modemB.signal ()` how strong it arrived in dBm, and
  `modemB.margin ()` how far above the noise, in dB.
- `adk::LineReader<60> typed;` and `adk::Text<16> message;` gather a typed
  line and make `Press 1`, as in Lesson 38.
- `showMessage ()` puts the message on the top row, and the signal and the
  margin below it: `-32 dBm  9 dB`. The top row shows a long message's
  first 16 letters; the Serial Monitor shows it all.

## Upload it

1. Plug in the USB cable, then the power module's adapter, and press the
   module's button so its LED lights.
2. Upload the sketch. The screen says `Modem A is ready` and
   `Modem B is ready`.
3. Open the Serial Monitor at 9600 baud, and set the menu beside the baud
   rate to **New Line**, so pressing Enter ends your line.
4. Press the button. The screen shows `Press 1` and, under it, the signal
   and the margin. The Serial Monitor says `A sends: Press 1`, then
   `B hears from 1: Press 1` and the two numbers.
5. Type a line of your own in the Serial Monitor and press Enter. It goes
   out and arrives the same way.

With the modems a hand apart, the signal is strong: somewhere up in the
−20s or −30s, perhaps, and the margin well above 0. Your numbers will
differ. Cup your hand round modem B's spring and send again: the signal
should drop a little.

Now test your prediction. In `sendFromA ()`, change `modemA.send (2, text)`
to `modemA.send (3, text)` and upload. Press the button: the Serial Monitor
still says `A sends: Press 1`, but modem B shows nothing. Modem B heard the
radio, but the message was for address 3, so it kept it to itself. And A
has no idea: sending is like shouting, and nothing comes back to say it was
heard. Change the 3 to a 0 and upload: B shows the message again, since 0
means everyone. Put the 2 back when you're done.

## If it doesn't work

| What you see | Try this |
|---|---|
| `No reply from A`, or from B | Is the power module on, with its bottom jumper on 3.3V? Check that modem's VDD goes to the bottom + rail and its GND to the − rail. Check its TXD and RXD aren't swapped: TXD goes up to f53 (A) or f44 (B), RXD to c55 (A) or c46 (B). |
| Still no reply | Check the divider: pin 18's wire in j55, the 1 kΩ from g55 to e55, the 2 kΩ from a55 to the − rail (for B: pin 14 in j46, g46 to e46, a46 to the − rail). And pin 19's wire in j53, pin 15's in j44. |
| `A sends:`, but B shows nothing | Check the address in `sendFromA ()` is 2 or 0, and that modem B says it is ready. |
| `A can't send just now: try again` | Wait a moment, and send again: the last message was still going. If it always says this, modem A didn't answer at the start. |
| Typing does nothing | Set the Serial Monitor's line ending to **New Line**: the sketch waits for the end of the line. |
| A modem gets warm | Unplug everything at once, and check the bottom jumper is on 3.3V, never 5V. |
| A blank lit screen, or a row of blocks | Turn the contrast knob. The screen takes its power from the power module now, so it must be switched on. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    When the sketch starts, ADK talks to each modem at 115,200 bits a
    second and sends it these lines, one at a time, waiting for `+OK` after
    each:

    ```text
    AT
    AT+ADDRESS=1
    AT+NETWORKID=6
    AT+BAND=915000000
    AT+CRFOP=15
    AT+PARAMETER=10,7,1,7
    ```

    The first just checks it's there. The next four set its address, its
    network, its band, 915,000,000 Hz, and its power, 15 dBm. The last sets
    how it chirps: REYAX's choice for up to 3 km. The modem forgets its band
    and settings each time it loses power, so ADK sends them every time.

    To send `Press 1` to modem B, ADK sends modem A this line:

    ```text
    AT+SEND=2,7,Press 1
    ```

    That is the address, how many letters (spaces count), and the text.
    Modem A answers `+OK` and sends it on the air. Modem B then tells its
    Mega:

    ```text
    +RCV=1,7,Press 1,-32,9
    ```

    From address 1, 7 letters, the text, the signal and the margin. ADK
    takes the text by its length, so a message can have commas in it.

    Each chirp of these settings sweeps the whole of a 125 kHz wide band in
    about 8 ms and carries 10 bits' worth, some of them spare, to put right
    what noise gets wrong. That's where the thousand bits a second come
    from: slow, but hard to drown out.

## Make it yours

1. **A kilometre.** The real test needs a second Mega, perhaps a friend's.
   Move modem B, its divider and the screen to it, with a power module of
   its own on a 9 V battery, set as this one is, and run this same sketch
   on both: each Mega ignores the modem it doesn't have. Power the far one
   from a USB power bank, and walk away with it, watching the signal fall.
   Near −120 dBm, with the margin near −15, the messages stop. Outdoors,
   with nothing in the way, that can be a kilometre or more.
2. **Got it.** Make modem B answer: when a message arrives, send
   `Got it` back to address 1 with `modemB.send (1, "Got it")`, and show
   what modem A hears on the Serial Monitor with `modemA.wasReceived ()`
   and `modemA.text ()`. Now A knows it was heard.
3. **Your own network.** Give both modems network 12 in their settings,
   `adk::LoraModem modemA {Serial1, 1, {.network = 12}};`, and the same
   for B. A pair on network 6 next door won't hear you, nor you them.
4. **A signal bar.** Show the signal as a bar on the bottom row, as the
   light meter did in Lesson 8: no blocks at −120 dBm and sixteen at
   −24, with `(modemB.signal () + 120) / 6` blocks. The screen's own full
   block is character 255: `lcd.write (255);`.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings while
nothing is being sent, with the power module on, and keep each probe tip in
its own hole: the rails' + and − holes are only 2.5 mm apart.

!!! question "Predict"
    Between messages, a serial line rests high, so pin 18 sits at 5 V. What
    will modem A's RXD read, at the middle of its divider?

<!-- measure -->

What the numbers tell you:

- **The modems' supply** is the power module's 3.3 V on the bottom rails,
  from its own regulator. The top rails, which feed the screen, read 5 V.
- **Pin 18** reads about 5 V: the Mega's TX1 pin rests high between
  messages. While a message goes out, it flickers between 5 V and 0 V far
  too fast for the meter to follow.
- **Modem A's RXD** reads about 3.3 V: two thirds of 5 V, as the divider
  shares it out. The 1 kΩ takes the other 1.7 V, and the current through
  both is 5 V ÷ 3 kΩ, less than 2 mA. The modem's own input may shift
  the reading a little.
