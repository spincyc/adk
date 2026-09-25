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
| RFID reader (RC522) | It runs on **3.3 V**. Power it from the Mega's 3.3V pin, never 5V. |
| Active buzzer | It draws up to about 30 mA: fine on its own pin (a Mega pin's limit is 40 mA), but give it a pin to itself. |
| Passive buzzer | Always through its 220 Ω resistor: its coil is only about 16 Ω. |
| 9 V battery | Never let its two terminals touch each other or anything metal. |
| Laser module | Not used in this course. A laser can damage eyes. |
| Clock module | If yours charges its coin cell (see its lesson), use a rechargeable LIR2032, never a CR2032. |

## Grown-ups

The lessons are written for learners from about twelve upwards, working
alone or with a family member or teacher. Younger learners should build with
an adult alongside.
