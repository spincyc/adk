---
lesson: 41
promise: Send the temperature and humidity by radio, as a line of text, from one LoRa module to another.
time: 60 minutes
level: 2
parts:
  - Your circuit from Lesson 40, without its two modems
  - Two Ebyte E32-433T20D LoRa modules, with their aerials (add-on, not in the kit)
  - DHT11 temperature and humidity module, from Lesson 14
  - 17 female-to-male jumper wires
  - 4 jumper wires
ideas:
  - A transparent link, bytes in at one end and out at the other
  - Modes chosen with two pins
  - Channels and power, set by law
  - Sending numbers as text
---

## What you'll build

<!-- closeup -->

A weather report sent by radio. The DHT11 measures the room, and every two
seconds LoRa module A sends what it read as a line of text:
**Temp 23C Hum 45%**. Module B hears it, and the screen shows the report
with how long ago it came. Press the button and a report goes at once. Here
both modules hang below one breadboard; put module A and the DHT11 on a
second Mega outdoors and you have a weather station that reports from the
bottom of the garden.

!!! danger "433 MHz needs a license in the USA and Canada"
    These modules send on 434 MHz. In Europe, anyone may send there at up
    to 10 mW, which is how ADK sets them. **In the USA and Canada, 433 MHz
    is an amateur radio band: use these modules only if you, or the adult
    you build with, hold an amateur radio license.** Without one, read
    this lesson but don't build it, and carry on to Lesson 42 from your
    Lesson 40 build: Lesson 40's modems do the same job on 915 MHz. Other
    countries have rules of their own, so check yours first.
    [Radios](../../safety.md#radios) on the safety page has the details.

## The idea

**A transparent link.** Lesson 40's modems took commands in words. These
modules take none: in their normal mode, whatever bytes the Mega sends into
module A's **RXD** go out by radio to every module on the same channel, and
come out of their **TXD** pins, just as they went in. It's as if a very long
wire ran from one Mega to the other, and so it's called **transparent**:
the link adds nothing and takes nothing away. ADK sends a line of text at a
time, ending it with a newline, so the other end knows where each line
stops.

**Modes chosen with two pins.** Each module has two mode pins, **M0** and
**M1**. Both low is the normal mode, for sending and receiving. Both high
is the settings mode: the bytes the Mega sends then change the module's
settings instead of going out on the air. (The other two mixes are for
saving battery, and ADK doesn't use them.) This sketch only ever needs both
low or both high, so the two pins are joined and share one Mega pin: 40 for
module A, 42 for module B. And that pin never drives them high. The module
has its own resistors that lift M0 and M1 to 3.3 V, so the Mega only ever
pulls them low or lets them go, and 5 V never reaches them.

A third pin, **AUX**, tells the Mega when the module is busy: it goes low
while the module starts up, changes mode or sends, and high when it is free.
The Mega reads it on pin 41, or 43, and waits for it before changing mode.

**Channels and power, set by law.** Each module sends on a **channel**:
410 MHz plus the channel's number. ADK chooses channel 24, which is 434 MHz,
at the module's lowest power, 10 mW. It could do ten times that, 100 mW, the
*20* in its name, but Europe allows 10 mW without a license, from
433.05 MHz to 434.79 MHz. In dBm, from Lesson 40, 10 mW is 10 dBm and
100 mW is 20 dBm.

**Numbers as text.** The DHT11 gives two numbers, but the report goes as
text: every digit, letter and space is one character, and each character is
one byte, as the LCD's letters were in Lesson 13. That costs more bytes than
the numbers alone would need, but anything can read it: module B's Mega
puts it straight on the screen, and a computer could show it just as easily.

<p class="formula">"Temp 23C Hum 45%" = 16 characters + 1 newline = 17 bytes</p>

!!! question "Predict"
    As two numbers, 23 and 45, the report would need 2 bytes: each is less
    than 256. As text it's 17. When you breathe on the DHT11 and it reads
    24 °C and 60 %, will the report get any longer? And what would make it
    longer?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire,
    and check your work before you plug them back in.

!!! danger "5 V for these modules, and aerials first"
    Screw each module's aerial onto its gold socket before you power
    anything: sending into no aerial can damage a module. These modules
    take **5 V**, so move the power module's **bottom** jumper from 3.3V to
    **5V**, and leave its top jumper off. Should you go back to Lesson 40's
    modems, set it back to 3.3V first: 5 V would damage them.

Keep the screen, the button, both dividers and the Mega's wires to pins 14,
15, 18 and 19 from Lesson 40 just as they are, and take out the two modems
and their wires. The power module stays where it is: the steps list it
again because its bottom jumper moves to **5V**. The DHT11 goes back to its
home above the board, as in Lesson 15, on the top rails, which the Mega's
5V feeds as it feeds the screen.

The modules take the modems' places below the board, their aerials pointing
down and away, module B first and then module A. Each module's wires go
where its modem's did, and three more are new:

- **TXD** comes up into row f and meets the Mega's RX pin's wire in row j,
  as before; **RXD** comes up into row c, the divider's middle; **VCC** goes
  to the bottom + rail and **GND** to the bottom − rail.
- **AUX** comes up into row f of the column just left of TXD, and the
  Mega's pin 41 (A) or 43 (B) lands in row j above it.
- **M0** and **M1** both come up into one column, into rows f and g, two to
  the right of the divider, and the Mega's pin 40 (A) or 42 (B) lands in
  row j above them. The column joins all three.

<!-- bench -->

<!-- steps -->

??? info "The modules' pins"
    The boards print no names by their pins. Hold a module flat, its metal
    shield up and its pins toward you: from the left they are M0, M1, RXD,
    TXD, AUX, VCC and GND. Lying below the breadboard with its aerial down,
    as in the drawing, the order runs the other way, GND on the left.

    | Pin | Job | Goes to |
    |---|---|---|
    | M0, M1 | Choose the mode | Joined, to pin 40 (A) or 42 (B) |
    | RXD | Listens to the Mega | The middle of its divider |
    | TXD | Talks to the Mega | Pin 19 (A) or 15 (B), through row j |
    | AUX | Low while busy | Pin 41 (A) or 43 (B), through row j |
    | VCC | Power, 5 V | The bottom + rail, at 5 V |
    | GND | Ground | The bottom − rail |

    Each module can draw about 100 mA while it sends at full power, as much
    as a small motor, so like the motors in earlier lessons they take the
    power module's 5 V. Their pins still work at 3.3 V, which is why RXD
    keeps its divider.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 041-lora-link**:

<!-- sketch -->

What's new:

- `adk::LoraLink linkA {Serial1, 40, 41};` is a module on `Serial1`, with
  M0 and M1 on pin 40 and AUX on pin 41. Module B is on `Serial3`, with
  pins 42 and 43. A fourth number would choose the channel; it is 24, the
  one Europe allows, unless you say otherwise.
- `linkA.ok ()` says whether the module took its settings when the sketch
  started; the screen says which did.
- `linkA.send (text)` sends a line of text, up to 56 characters, to every
  module on the channel.
- `linkB.wasReceived ()` is true for one pass of `loop ()` when a whole line
  has arrived, and `linkB.text ()` is the line.
- `(dht.measured () || button.wasPressed ()) && dht.ok ()` sends a report
  when a reading finishes or the button is pressed: `||` is *or*, and `&&`
  is *and*. `dht.ok ()` says whether the latest reading came through
  whole, so only a good one goes.
- `adk::Text<32> report;` is text to print into, as in Lesson 40, and
  `adk::fixed (dht.temperature (), 0)` prints the temperature with no
  decimals, as in Lesson 15. `report.size ()` counts its characters.
- `adk::Stopwatch sinceReport;` from Lesson 12 starts again from 0 with
  each report, and `sinceReport.elapsed () / 1000` is how many whole
  seconds ago it came. Until the first report it isn't running, and the
  bottom row keeps saying whether module B is ready.

## Upload it

1. Check both aerials are on, and the power module's bottom jumper is on
   5V. Plug in the USB cable, then the power module's adapter, and switch
   it on.
2. Upload the sketch. The screen says `Module A ready` and
   `Module B ready`.
3. Open the Serial Monitor at 9600 baud. A second or two later, and every
   two seconds after that, it says something like
   `A sends 16 characters: Temp 23C Hum 45%` and then
   `B hears: Temp 23C Hum 45%`. The screen shows the report on the top row,
   and `0 s ago` or `1 s ago` below it.
4. Press the button: a report goes at once, and the bottom row goes back to
   `0 s ago`.
5. Breathe gently on the DHT11. Within a couple of seconds the report
   changes.

You predicted whether a warmer, damper reading makes the report longer.
`Temp 24C Hum 60%` is still 16 characters: each number keeps two digits.
It grows only when a number needs another digit, such as a humidity of
100 %, and shrinks when one needs fewer, such as 9 °C. Text is more than
eight times the size of the two bytes here, but it needs no key to read it.

## If it doesn't work

| What you see | Try this |
|---|---|
| `No reply from A`, or from B | Is the power module on, with its bottom jumper on 5V? Check that module's VCC and GND, its M0 and M1 into f and g of column 57 (A) or 48 (B) with pin 40 or 42 in j above them, and its AUX into f52 (A) or f43 (B) with pin 41 or 43 in j above it. |
| Still no reply | Check TXD and RXD aren't swapped: TXD goes up to f53 (A) or f44 (B), RXD to c55 (A) or c46 (B). Check the divider, as in Lesson 40. |
| `A sends`, but B hears nothing | Check module B's TXD in f44 and pin 15's wire in j44, and that both aerials are on. |
| Nothing is sent at all | The DHT11 hasn't given a good reading: check S goes to pin 16, + to the top + rail (T+36) and − to the top − rail (T-37). |
| A blank lit screen, or a row of blocks | Turn the contrast knob beside the LCD. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    When the sketch starts, ADK lets M0 and M1 go, waits for AUX to say the
    module is free, and sends six bytes of settings, written here in
    hexadecimal as in Lesson 22:

    ```text
    C0 00 00 1A 18 47
    ```

    `C0` means "keep these settings". `00 00` is the module's address, 0
    for both, so they hear each other. `1A` sets 9,600 bits a second
    between the module and the Mega, and 2,400 bits a second on the air.
    `18` is channel 24. `47` chooses transparent sending, error
    correction, and 10 mW. The module
    answers with the same six bytes, and ADK checks them. Then it pulls M0
    and M1 low, waits for AUX again, and the module is in its normal mode.
    It all takes about a fifth of a second.

    In normal mode, the module waits until the bytes from the Mega stop
    coming, then sends all it has as one packet. A packet holds at most 58
    bytes, which is why a line can be up to 56 characters: with its newline,
    it still goes as one.

    The DHT11 holds up `loop ()` for about 4 ms while a reading arrives,
    but not the serial ports: the Mega catches each byte from module B as
    it comes in, between the DHT11's bits, and keeps it until ADK reads
    it. So a report that lands in the middle of a reading still arrives
    whole.

## Make it yours

1. **A station in the garden.** With a second Mega, move module A, its
   divider, the DHT11 and a power module of its own to it, and keep module
   B and the screen indoors. Keep the DHT11 in the shade and out of the
   rain. The age on the bottom row tells you it's still getting through.
2. **Only when it changes.** Send a report only when the temperature or
   the humidity is different from the last one sent, and once a minute
   anyway, so you know the station is alive. Fewer reports mean less time
   on the air.
3. **Numbers again.** On module B's side, turn the text back into a
   number: the temperature starts 5 characters in, after `Temp `, so
   `atoi (linkB.text () + 5)` gives it as an `int`. Show `Warm!` on the
   bottom row when it's 28 or more.
4. **Both ways.** Make module B answer each report with `OK`, and count
   on the Serial Monitor how many module A hears back.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings while
the sketch runs, between reports, and keep each probe tip in its own hole.

!!! question "Predict"
    Once the sketch has started, pin 40 holds module A's M0 and M1 low, and
    AUX is high while the module is free. What will each read? Will AUX
    read the full 5 V the module runs on?

<!-- measure -->

What the numbers tell you:

- **The modules' supply** is the power module's 5 V on the bottom rails,
  where Lesson 40's modems had 3.3 V.
- **Pin 40** reads 0 V: the Mega pulls M0 and M1 low for normal mode.
  Hold the Mega's reset button down while you watch: the Mega lets go of
  every pin, and the module's own resistors lift M0 and M1 to about 3.3 V,
  the settings mode. Let go, and the sketch pulls them low again.
- **AUX** reads about 3.3 V, not 5 V: the module runs on 5 V, but its pins
  work at 3.3 V. That's still high enough for the Mega to read as high. It
  dips while a report goes out, too briefly for the meter to show.
