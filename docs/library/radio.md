# Radio

Parts for radios that aren't in the Elegoo kits: an FM radio to listen to,
a 433 MHz link between two small modules, and three kinds of LoRa radio that
reach a kilometre or more. [The kit page](../kit.md#add-on-radios) lists
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
radios on this page: a dial on one board and a servo on the other, say.
Each radio is a `Link`, and a bridge needs nothing more from it.

<!-- api bridge.h Bridge -->

<!-- api link.h Link -->

## LoRa, over a serial port

These three talk to the Mega over one of its spare serial ports, `Serial1`,
`Serial2` or `Serial3`, which the part claims with its two pins. `Serial`
itself stays with the USB cable and the Serial Monitor.

<!-- api lora_modem.h LoraModem -->

<!-- api lora_modem.h LoraSettings -->

`adk::LoraSpeed` is `Far`, REYAX's choice for up to 3 km at about 0.3 s a
message, or `Quick`, about 0.05 s a message and a kilometre or so, for a
bridge that steers something.

<!-- api lora_link.h LoraLink -->

<!-- api mesh_node.h MeshNode -->

### Serial ports

For a part of your own that talks over a serial port. `adk::LineReader<N>`
gathers what arrives a line at a time, up to `N` characters, without
waiting: its `read (port)` is true once a whole line is in `line ()`.

<!-- api serial_port.h -->
