---
lesson: 42
title: Mesh Messenger
arc: Long range
promise: Text your Mega from your phone, across a mesh of LoRa radios, to switch its lamp, ask the temperature, or leave a message on its screen.
time: 2 hours
level: 3
sketch: Lesson42MeshMessenger
parts:
  - Your circuit from Lesson 41, without its LoRa modules, second divider and power module
  - Two Heltec WiFi LoRa 32 V3 boards, with their aerials (add-on, not in the kit)
  - A USB-C cable and a USB charger or port for each board
  - A phone with the Meshtastic app, and a computer with Chrome or Edge
  - RGB LED
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - 3 female-to-male jumper wires
  - 4 jumper wires
  - A soldering iron and an adult to help, unless your boards come with their pins soldered
ideas:
  - A mesh, where radios pass messages on for each other
  - A second computer that does one job for the Mega
  - Messages as commands
  - A private channel
---

## What you'll build

<!-- closeup -->

Texts from your phone to your Mega. Type `lamp on` in the Meshtastic app
and, a moment later, a lamp on the breadboard fades up and your phone gets
a reply: **The lamp is on**. Ask `temp?` and the Mega texts back the
temperature and humidity. Any other message appears on the Mega's screen
with the name of the radio it came from, scrolling if it's long, and the
button sends **Hello from the Mega** to your phone. Between the phone and
the Mega are two LoRa radios running Meshtastic, which can carry a message
across a town if there are enough of them.

!!! warning "Mesh messages can travel"
    The boards send on 915 MHz in the Americas and Australia, and 868 MHz in
    Europe: setting the region, below, makes Meshtastic keep to your
    country's rules. Anyone nearby with Meshtastic can read its default
    channel, so this lesson sets up a private one; even so, never send
    anything personal. [Radios](../../safety.md#radios) on the safety page
    has the details.

## The idea

**A mesh.** Meshtastic is free software for small LoRa radios. Each radio
is a **node**, and every node that hears a message it hasn't heard before
sends it on once more, up to three times over. So a message can hop from
node to node, past hills and buildings no single radio could reach across:
a **mesh**. Here there are just two nodes, one wired to the Mega and one
beside your phone, but a friend's node down the street would join in.

**A second computer that does one job for the Mega.** The Heltec board is a
whole computer of its own: an ESP32-S3, faster than the Mega, with its own
LoRa radio, a little screen, Bluetooth and WiFi. It runs Meshtastic, not
ADK, and the Mega talks to it over a serial port, just as it talked to the
modems in Lesson 40. One of Meshtastic's modules, **Serial**, does the rest:
set to **TEXTMSG**, it sends whatever the Mega writes to it as a text
message, and writes every text message it hears back to the Mega as one
line: the sender's short name, a colon, and the text.

```text
4f2a: lamp on
```

Your phone talks to the other board over Bluetooth, and that board's short
name is what the Mega sees as the sender.

**Messages as commands.** The Mega reads each message and decides what to
do, just as it decided what each remote button meant in Lesson 22. A
command is the whole message: `lamp on`, `lamp off`, `temp?` or `help`.
Phones like to start a message with a capital, so the sketch compares the
words without caring about capitals. Anything else is simply a message,
and goes on the screen.

**A private channel.** Meshtastic nodes talk on **channels**. Every node
starts on the same public one, which anyone nearby can read. A channel with
a name and a random key of your own keeps your messages between the boards
that share the key: Meshtastic scrambles every message with it, and a node
without it hears only noise.

!!! question "Predict"
    Once it's all working, you'll send the Mega three texts: `Lamp on`,
    `lamp on please` and `LAMP OFF`. Which will it obey, and what will it do
    with the others? Read `obey ()` in the sketch, and write down your
    answer.

## Set up the two boards

You only do this once. Call the board you'll wire to the Mega **board 1**,
and the one that stays with your phone **board 2**; a sticker on each helps.
Meshtastic's menus move about a little from one version of the app to the
next; [its own guide](https://meshtastic.org/docs/getting-started/) has
pictures of each.

1. **Pins.** Heltec boards usually come with their pin headers loose. Board
   1 needs its pins soldered on; board 2 needs none. Ask an adult to help
   with the soldering iron, or buy boards with their pins already fitted.
2. **Aerials.** Screw each board's aerial on before you ever plug it in.
   Sending into no aerial can damage the radio.
3. **Meshtastic.** Plug board 1 into the computer with its USB-C cable. In
   Chrome or Edge, open [flasher.meshtastic.org](https://flasher.meshtastic.org),
   choose **Heltec V3** and the newest stable version, and press **Flash**.
   If the computer doesn't find the board, install the **CP210x** USB
   driver from Silicon Labs and try again. Do the same for board 2. Each
   board's screen then shows **Meshtastic** as it starts.
4. **Pair board 1.** Install the Meshtastic app on your phone, power board
   1 from a USB charger, and add it in the app over Bluetooth. The board's
   screen shows a six-digit PIN; type it into the phone.
5. **Region.** In the app's LoRa settings, set the **Region** to where you
   are: **US** in the USA and Canada, **EU_868** in Europe. Until the
   region is set, the board won't send at all.
6. **Serial.** In the app's module settings, under **Serial**, set:
   **enabled** on, **echo** off, **RX** 47, **TX** 48, **baud** 38400 and
   **mode** TEXTMSG, and save. The board may restart to take them.
7. **A private channel.** In the app's channel settings, give the primary
   channel a name of your own, such as `ADKmesh`, and a new random key,
   and save. Then use the app's **share** button for the channel to copy
   its link, and keep it in a note on your phone.
8. **Pair board 2.** Disconnect from board 1 in the app, power board 2,
   and pair it the same way. Set its region too, as in step 5. Then open
   the channel link from your note: the app asks whether to use its
   channel; say yes. Now both boards share the channel.
9. **Names.** Each board has a four-letter short name, which starts as the
   end of its number, such as `4f2a`. You can change board 2's in the
   app's user settings: pick something like `ME` or `PHNE`, never your real
   name, as everyone on the channel sees it.

??? info "The same settings from a computer"
    If you have Python on a computer, Meshtastic's command-line tool can
    set up board 1 over its USB cable instead of steps 5 and 6. Install it
    with `pip install meshtastic`, then run:

    ```text
    meshtastic --set lora.region US
    meshtastic --set serial.enabled true --set serial.mode TEXTMSG --set serial.rxd 47 --set serial.txd 48 --set serial.baud BAUD_38400 --set serial.echo false
    ```

    Use your own region in place of `US`.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable, the power module's adapter and board 1's cable
    before you wire, and check your work before you plug them back in.

!!! danger "Board 1 has its own cable, and 3.3 V pins"
    Board 1 runs from its own USB-C cable, never from the Mega: its 5V pin
    stays unconnected. Its pins work at 3.3 V, so GPIO47 only ever sees
    pin 18 through the 1 kΩ, with the 2 kΩ to GND, the divider module A
    had in Lesson 41. GPIO48 goes straight to pin 19; the Mega is happy to
    read 3.3 V.

Keep the screen, the button, the DHT11, module A's divider in column 55 and
the Mega's wires to pins 18 and 19 from Lesson 41 just as they are. Take
out both LoRa modules and their wires, module B's divider and the wires to
pins 14, 15 and 40 to 43, and the power module: nothing here needs it, so
the red wire from the Mega's 5V goes back into T+3. If you skipped building
Lesson 41, start from Lesson 40 instead, and put the DHT11 at its home above
the board as in Lesson 15.

The lamp is the RGB LED, at its home beside the screen in columns 41 to 46,
as in Lesson 15: its longest leg in the − rail at B-42, a 220 Ω resistor
across the gap above each colored leg, and pins 5, 6 and 7 into row j.

Board 1 lies below the breadboard in module A's place, its pins toward the
board, with three wires: **48** up into f53, where pin 19's wire waits in
j53; **47** up into c55, the middle of the divider; and **GND** to the − rail
at B-59.

<!-- bench -->

<!-- steps -->

??? info "Board 1's pins"
    The board has two rows of 18 pins, with their names printed beside
    them. This lesson uses three, all in the row marked **J2**, the one
    whose first pins are GND and 5V:

    | Pin | Job | Goes to |
    |---|---|---|
    | 47 | Listens to the Mega (the Serial module's RX) | The middle of the divider, c55 |
    | 48 | Talks to the Mega (the Serial module's TX) | Pin 19, through f53 and j53 |
    | GND | Ground | The bottom − rail |

    The Mega's GND and the board's GND must be joined, or the two can't
    agree what 0 V is, and the signals between them mean nothing.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose
**File → Examples → Adk → Lesson42MeshMessenger**:

<!-- sketch -->

Read it from the top:

- Six parts: `node`, board 1 on `Serial1`; the screen; the lamp, as in
  Lesson 22; the DHT11; the button; and `scroll`, a beat for sliding a long
  message along the bottom row.
- `message` is the message on the screen, kept in an `adk::Text` as long
  as the longest message, 100 letters, and `first` is the letter at its
  left edge.
- `loop ()` hands each message to `obey ()`, with who sent it, and sends a
  hello when the button is pressed. Three times a second, if the message
  is longer than the screen, it moves `first` along one letter and redraws
  the bottom row.
- `obey ()` checks the message against each command in turn.
  `strcasecmp (text, "lamp on")` compares two pieces of text letter by
  letter, taking no notice of capitals, and gives 0 when they are the
  same. Each command acts and replies; anything else goes to `show ()`.
- `sendWeather ()` prints the reading into text, as Lesson 41's report did,
  or says there's no reading yet.
- `reply ()` sends a message to everyone on the channel with
  `node.send (text)`, and shows it on the screen as from `Mega`. The node
  takes a message at most every 1.5 seconds, so `send ()` says `false`,
  sending nothing, if one comes sooner.
- `show ()` keeps a copy of the message, puts who sent it on the top row
  (`4f2a says:`), and draws the bottom row from the start.
- `showBottomRow ()` prints sixteen letters of the message from `first`.
  For a long message, `% (length + 3)` wraps round to the start after a
  gap of three spaces, the way `% 16` wrapped the walking figure round in
  Lesson 13.

## Upload it

1. Plug board 1 into a USB charger, and board 2 too. Both screens light.
2. Plug in the Mega and upload the sketch. The screen says
   `Mesh Messenger` and `Waiting...`. Open the Serial Monitor at 9600 baud.
3. In the app, open your private channel and send `help`. After a few
   seconds a reply arrives from board 1: `Try lamp on, lamp off or temp?`.
   The Mega's screen shows `Mega says:` and the reply.
4. Send `lamp on`. The lamp fades up to white, and your phone gets
   `The lamp is on`. Send `lamp off` to fade it out.
5. Send `temp?`. The reply is the room's temperature and humidity, such as
   `It's 23C and 45% humid`.
6. Send `Hi Mega!`. The screen shows your board 2's name and the message.
   Send something long, and watch it scroll.
7. Press the button on the breadboard. `Hello from the Mega` arrives on
   your phone.

You predicted which of three texts the Mega would obey. `Lamp on` and
`LAMP OFF` work: `strcasecmp ()` pays no attention to capitals. But
`lamp on please` is not the whole command, so the Mega doesn't obey it; it
shows it on the screen as a message instead.

## If it doesn't work

| What you see | Try this |
|---|---|
| The screen stays on `Waiting...` | Check board 1's Serial settings: enabled, TEXTMSG, RX 47, TX 48, 38400. Check its 48 goes to f53, with pin 19's wire in j53, and its GND to the − rail. |
| Still nothing arrives | Send in the channel's chat, not as a direct message to board 1. Check both boards have the same region and the same private channel. |
| Messages arrive, but the phone never gets a reply | Check board 1's 47 goes to c55, pin 18's wire is in j55, and the divider's 1 kΩ is from g55 to e55 and 2 kΩ from a55 to the − rail. |
| `Too soon to send` on the Serial Monitor | The node takes a message at most every 1.5 seconds: press the button more slowly. |
| The Mega shows a command instead of obeying it | A command must be the whole message, with nothing after it: no full stop, no space. Some keyboards add a space after a word they finish for you. |
| The reply to `temp?` is `No reading yet` | Wait a few seconds after starting. If it stays, check the DHT11: S to pin 16, + to T+36, − to T-37. |
| The lamp stays dark, or a color is missing | Check the RGB LED's longest leg is in B-42, and follow each color from its pin: 5 to j41, 6 to j44, 7 to j46, each through its resistor. |
| The app can't find board 2 | Make sure it has power, and that Bluetooth is on. Only one phone at a time can pair with a board. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The Mega talks to board 1 at 38,400 bits a second. When the sketch
    sends `The lamp is on`, ADK writes just those letters, with no newline:
    the Serial module waits until the Mega has been quiet for a second,
    then sends everything it got as one text to the private channel. A
    newline would go out as part of the message. That second of quiet is
    why ADK allows a message only every 1.5 seconds.

    When a text arrives, board 1 writes a blank line, then `4f2a: lamp on`,
    then another blank line. ADK skips the blank lines, takes the short
    name from before the colon and the text from after it, and
    `node.wasReceived ()` is true for one pass of `loop ()`.

    On the air, Meshtastic's usual settings chirp much like Lesson 40's
    modems, slow but far. Each message is scrambled with the channel's key
    before it goes, and every node that shares the key unscrambles it and
    sends it on, up to three times, so nodes out of each other's reach can
    still pass messages along. Your phone never uses LoRa itself: it hands
    messages to board 2 over Bluetooth.

## Make it yours

1. **Colors.** Add `lamp red`, `lamp green` and `lamp blue`, each fading
   the lamp to its color, like the number buttons in Lesson 22.
2. **Too hot.** Send `It's getting hot!` to the channel when the DHT11
   first reads 28 °C or more, and once it's back under 27, `Cooler now`.
   Keep the gap between them, as the weather station did in Lesson 15, so
   a wobbly reading doesn't send a stream of texts.
3. **Status.** Add a `status?` command that replies whether the lamp is
   on, and how long the Mega has been running.
4. **Further.** Put board 2 on a windowsill, or ask a friend with
   Meshtastic to join your channel from their own board, and see how far
   apart the Mega and the phone can be. A third board on a high shelf in
   between passes messages on for both.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Take the readings
between messages, and keep each probe tip in its own hole.

!!! question "Predict"
    Pin 18 rests at 5 V between messages, and the divider is the one
    module A had. What will board 1's GPIO47 read? And pin 5, the lamp's
    red, once you've sent `lamp on`?

<!-- measure -->

What the numbers tell you:

- **Board 1's GPIO47** reads about 3.3 V: two thirds of pin 18's 5 V, safe
  for its 3.3 V pin. It flickers down while the Mega sends, far too fast
  for the meter.
- **Pin 5** reads about 5 V with the lamp on: white is red, green and blue
  all fully on. Send `lamp off` and it falls to 0 V over half a second, as
  the lamp fades.
