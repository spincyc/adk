# Safety

Everything in this course runs on 5 volts from a USB port or a small battery,
which is safe to touch. These rules keep it that way, and keep your parts
alive.

!!! danger "Never mains electricity"
    Nothing in this course connects to a wall socket, and nothing ever
    should. A relay module can *switch* mains, which is exactly why it is
    only ever used here with a battery and an LED. Mains wiring can kill.

## Every lesson

- **Unplug before you wire.** Take the USB cable out before changing any
  wiring, and check your work before you plug it back in.
- **Every LED needs a resistor.** Without one, an LED takes too much current
  and can damage itself and the Mega's pin. The lessons say which resistor
  to use.
- **Watch for heat and smell.** If a part gets hot or smells, unplug at once
  and look for a wire in the wrong place, usually a short from 5 V straight
  to GND.
- **Keep the bench dry and tidy.** Loose wire snippets on a desk can bridge
  pins underneath a board.

## Motors and servos

- **Never power a motor from a pin.** A pin can give about 20 mA; a small
  motor wants hundreds. Motors run through a driver chip, the L293D or the
  ULN2003, powered from the breadboard power module.
- **Servos get their own supply too.** Join the power module's GND to the
  Mega's GND so their signals agree.
- **Keep fingers and hair clear** of fan blades and anything that turns.

## Parts that need care

| Part | Take care |
|---|---|
| RFID reader (RC522) | It runs on **3.3 V**. Power it from the Mega's 3.3V pin, never 5V. The Mega's 5 V signals on its SDA, SCK, MOSI and RST pins are above what the chip is rated for. It usually copes, and the lessons wire it that way; for a build that has to last, put a 1 kΩ and 2 kΩ divider on each of those four lines. |
| Active buzzer | It draws up to about 30 mA: fine on its own pin (a Mega pin's limit is 40 mA), but give it a pin to itself. |
| Passive buzzer | Always through its 220 Ω resistor: its coil is only about 16 Ω. |
| 9 V battery | Never let its two terminals touch each other or anything metal. |
| Laser module | Not used in this course. A laser can damage eyes. |
| Clock module | If yours charges its coin cell (see its lesson), use a rechargeable LIR2032, never a CR2032. |

## Radios

The add-on radios in Lessons 37 to 42 need two kinds of care: their pins
work at 3.3 V, and a radio that sends is ruled by law.

- **Never put 5 V on a 3.3 V pin.** The FM radio, the 433 MHz
  transmitter, the LoRa radios and the Meshtastic board all work at 3.3 V.
  Where the Mega drives one of their pins, the signal goes through a
  divider: 1 kΩ from the Mega's pin to the radio's, and 2 kΩ from the
  radio's pin to GND, which turns 5 V into 3.3 V. Where the Mega only pulls
  a pin low or lets it go (the FM radio's three pins, the LoRa module's M0
  and M1), the radio's own resistors lift it to 3.3 V instead. Their
  outputs are safe for the Mega to read directly.
- **Power them as the lesson says.** The FM radio and the 433 MHz
  transmitter take little enough for the Mega's 3.3V pin. The LoRa modem
  draws more than that pin can give when it sends, so it runs from the
  breadboard power module set to 3.3 V. The Meshtastic board runs from its
  own USB cable.
- **Fit the aerial before powering a LoRa radio.** Sending into no aerial
  can damage it, and the Meshtastic board starts sending as soon as it is
  set up.
- **Send only where it's allowed.** Receiving is fine anywhere; sending is
  not. Check your country's rules; in outline:

| Radio | Band | License-free |
|---|---|---|
| FM radio | 87.5–108 MHz | Receive only, so anywhere |
| 433 MHz modules, E32 LoRa module | 433 MHz | In Europe, 433.05–434.79 MHz at up to 10 mW, which is how ADK sets the E32. In the USA and Canada it is an amateur band: unlicensed transmitters there are limited to very weak, occasional signals, like a car key's, so use the E32 only with an amateur license, and keep the little transmitter's messages short and few. |
| RYLR896 modem, Heltec board | 915 MHz | In the Americas and Australia, 902–928 MHz. Europe uses 868 MHz instead, with different modules and settings, at up to 25 mW for 1% of the time. |

- **Mesh messages are public.** Anyone nearby with Meshtastic can read
  the default channel. Lesson 42 sets up a private one; even so, never send
  anything personal.

## Grown-ups

The lessons are written for learners from about twelve upwards, working
alone or with a family member or teacher. Younger learners should build with
an adult alongside.
