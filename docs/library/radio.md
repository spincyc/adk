# Radio

Parts for radios that aren't in the Elegoo kits: an FM radio to listen to,
a 433 MHz link between two small modules, and three kinds of LoRa radio that
reach a kilometer or more. [The kit page](../kit.md#add-on-radios) lists
them, and [Safety](../safety.md#radios) says which pins need their 5 V
divided down, and which bands you may send on where you live.

<!-- api fm_radio.h FmRadio -->

`adk::FmBand` picks the band: `World` (87.5 to 108 MHz in steps of 0.1 MHz,
the default), `Americas` (the same band in steps of 0.2 MHz, 88.1, 88.3 and
so on) or `Japan` (76 to 90 MHz).

<!-- api radio.h RadioTransmitter -->

<!-- api radio.h RadioReceiver -->

## Two boards

A bridge keeps a few named numbers the same on two boards, over any of the
LoRa radios on this page: a dial on one board and a servo on the other,
say. Each radio is a `Link`, and a bridge needs nothing more from it. The
433 MHz modules are a `Link` too, but their transmitter rests at least
10 s after every message, longer than a bridge waits to hear from the
other board, so a bridge over them is slow, and says half the time that
the other board has gone.

<!-- api bridge.h Bridge -->

Use `share ("angle", angle)` for a value, such as a dial's position: the
other board reads the latest with `value ("angle")`. Use
`shareEvent ("key", presses, key)` for something that happens, such as a
key press: add one to `presses` for every press, even when the same key is
pressed twice. On the other board, `changed ("key")` is true once for each
new event that arrives, and `payload ("key")` is the key that came with it.
The count and the key travel together, as `key=3:53`, so a key never
arrives with another press's count.

Each message starts with two start numbers, as in `@1/2 angle=90`: the
sending board's own, then the other board's, as the sender last heard it.
A board takes its start number from the first message it hears, one more
than the other board remembers, so when either board restarts, the other
notices at once and sends everything again. From then on each counts the
other's events from zero: an event from before the restart never arrives
again, and a reply meant for a board as it was before it restarted is
ignored. Neither board needs resetting by hand.

What a bridge can't do: it keeps only the latest value or event for each of
its eight names. Two events within a tenth of a second, or one whose message
is lost before the next is sent, arrive as one, and the event's `value ()`,
its count since the boards found each other, goes up by two. Nothing says
whether a message arrived, so let the other board share back what it did, as
Lesson 49's lock shares how many digits it holds.

Both numbers use the Mega's signed 32-bit range, −2147483648 to 2147483647.
For a 32-bit RFID ID, send `static_cast<int32_t> (reader.uid ())` as the
payload, and recover its bits with
`static_cast<uint32_t> (bridge.payload ("card"))`.
A value's payload is zero, as is an unknown name's payload.

<!-- api link.h Link -->

## LoRa, over a serial port

These three talk to the Mega over one of its spare serial ports, `Serial1`,
`Serial2` or `Serial3`, which the part claims with its two pins. `Serial`
itself stays with the USB cable and the Serial Monitor.

<!-- api lora_modem.h LoraModem -->

<!-- api lora_modem.h LoraSettings -->

`adk::LoraSpeed` is `Far`, REYAX's choice for up to 3 km at about 0.3 s a
message, or `Quick`, about 0.05 s a message and a kilometer or so, for a
bridge that steers something.

<!-- api lora_link.h LoraLink -->

<!-- api mesh_node.h MeshNode -->

### Serial ports

For a part of your own that talks over a serial port. `adk::LineReader<N>`
gathers what arrives a line at a time, up to `N` characters, without
waiting: its `read (port)` is true once a whole line is in `line ()`.

<!-- api serial_port.h -->
