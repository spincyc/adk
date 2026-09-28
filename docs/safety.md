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
| IR LED module | Always through its 220 Ω resistor: on its own the LED would take more current than it or the pin should. |
| Water sensor | Dip only its copper traces, never its parts or pins, and keep the water away from the boards. Current through wet traces corrodes them, so the lessons power it from a pin only while they read it. |
| 9 V battery | Never let its two terminals touch each other or anything metal. |
| Laser module | Not used in this course. A laser can damage eyes. |
| Clock module | If yours charges its coin cell (see its lesson), use a rechargeable LIR2032, never a CR2032. |

## Radios

The add-on radios in Lessons 37 to 55 need two kinds of care: their pins
work at 3.3 V, and a radio that sends is ruled by law.

- **Never put 5 V on a 3.3 V pin.** The FM radio, the 433 MHz
  transmitter, the LoRa radios and the Meshtastic board all work at 3.3 V.
  Where the Mega drives one of their pins, the signal goes through a
  divider: 1 kΩ from the Mega's pin to the radio's, and 2 kΩ from the
  radio's pin to GND, which turns 5 V into 3.3 V. Where the Mega only pulls
  a pin low or lets it go (the FM radio's SDIO and SCLK, the LoRa module's
  M0 and M1), the radio's own resistors lift it to 3.3 V instead. The FM
  radio's board holds its RST low, so Lesson 37 lifts it with a 1 kΩ to
  3.3 V. Their outputs are safe for the Mega to read directly.
- **Power them as the lesson says.** The FM radio and the 433 MHz
  transmitter take little enough for the Mega's 3.3V pin. A LoRa modem
  sending at full power draws up to about 50 mA by REYAX's datasheets: all
  that pin can give, so Lesson 40's two run from the breadboard power
  module set to 3.3 V. In the two-board lessons each board has one modem
  sending at 10 dBm. REYAX gives no figure for that, but the radio chip
  inside draws about 29 mA even at 13 dBm (Semtech's SX1276 datasheet), so
  its VDD goes to the Mega's 3.3V pin; a board that also has the RFID
  reader, which shares that pin, powers its modem from the power module set
  to 3.3 V instead. Label a power module set to 3.3 V: a 5 V one in its
  place would ruin the modem. The Meshtastic board runs from its own USB
  cable.
- **Fit the aerial before powering a LoRa radio.** Sending into no aerial
  can damage it, and the Meshtastic board starts sending as soon as it is
  set up.
- **Send only where it's allowed.** Receiving is fine anywhere; sending is
  not. Check your country's rules; in outline:

| Radio | Band | License-free |
|---|---|---|
| FM radio | 87.5–108 MHz | Receive only, so anywhere |
| 433 MHz transmitter | 433.92 MHz | In Europe, up to 10 mW, sending at most a tenth of the time; its rest (below) keeps it well inside the time. In the USA and Canada, only a weak signal, far weaker than this module with an aerial: leave the aerials off and keep both modules on one desk. |
| E32 LoRa module | 434 MHz | In Europe, 433.05–434.79 MHz at up to 10 mW, sending at most a tenth of the time. The 10 mW counts what the aerial sends out, so ADK sets the E32 to its lowest power, 10 mW: don't give it a bigger aerial. In the USA and Canada it is an amateur band: use the E32 only with an amateur license, and send your call sign (below). |
| RYLR896 modem, Heltec board | 915 MHz, or 868 MHz in Europe | In the USA and Canada, 902–928 MHz. In Australia, 915–928 MHz: a signal on 915 MHz spills over its lower edge, so give the modems `.band = 921500000`, the middle of the band, and set Meshtastic's region to ANZ. In Europe, the same modems with `.band = 868100000` send at 868.0–868.6 MHz, up to 25 mW for 1% of the time, so give them `.power = 14` too: ADK's usual 15 dBm is more than 25 mW. A bridge sends more often than 1%, so in Europe give its modems `.band = 869525000`, in the 869.4–869.65 MHz band, where 10% of the time is allowed, and don't leave a knob turning for long. Meshtastic's region EU_868 keeps to that band by itself. |

- **The 433 MHz transmitter rests.** In the USA and Canada, a license-free
  gadget at 433 MHz may send a control signal, like a car key's, but one
  that sends data must stop after at most a second, then stay silent 30
  times as long and never less than 10 seconds (47 CFR 15.231(e); RSS-210
  A.1.5). ADK's `RadioTransmitter` keeps that rule for every sketch: after
  each message it won't send again until its rest is over, and after it
  starts it rests as long as the longest message would need, 12.78
  seconds, so a reset can't cut a rest short. The same rest keeps it well
  within Europe's tenth of the time (ERC Recommendation 70-03, annex 1,
  band g1).
- **With an amateur license**, 433 MHz is part of the 70 cm band, and the
  aerials and the range are yours to try. The license holder must send
  their call sign at least every 10 minutes and at the end of each contact
  (47 CFR 97.119 in the USA): the easiest way is to put it in the
  messages.
- **Mesh messages are public.** Anyone nearby with Meshtastic can read
  the default channel. Lesson 42 sets up a private one; even so, never send
  anything personal.

## Grown-ups

The lessons are written for learners from about twelve upwards, working
alone or with a family member or teacher. Younger learners should build with
an adult alongside.
