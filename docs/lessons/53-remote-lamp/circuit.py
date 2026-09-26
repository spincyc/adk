# Two boards, each with its LoRa modem at the bridge's home: below the
# board under columns 42-47, aerial down, its TXD up into f44 beside RX3
# (pin 15) in j44, TX3 (pin 14) into j46 and down through 1 kΩ and 2 kΩ
# to the − rail, the modem's RXD into c46 between them, its GND into B-42
# and its VDD from the Mega's 3.3V pin.
#
# Board A: Lesson 52's screen stays at its home and the knob goes. The IR
# receiver sits at its home beside the screen, above the board over
# columns 28-30, as in Lesson 24: its signal to pin 2, its power from the
# top rails below it.
#
# Board B: the relay above the board and its lamp, laid out as in Lesson
# 35: NO into j13, the 1 kΩ across the gap, the red LED in b13-b14. The
# 9 V battery stands below the board under the lamp, its black lead into
# a14 and its red lead round to COM. The IR LED module's home is pin 3's,
# column 38: pin 3 into j38, 220 Ω across the gap to e38, and the module
# below the board, its LED pointing away, S into a38 and − into B-36; its
# middle pin stays empty. Nothing here needs the power module: the relay's
# coil runs from the Mega's inner 5V pin, as in Lesson 35.
# Over the top, above the IR receiver or the relay: 14 in the higher lane, so the
# two nest rather than cross.
def modem (bench, lanes=None):
    bench.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
    bench.wire ("modem.GND", "B-42")
    bench.wire ("3.3V", "modem.VDD")
    bench.wire ("14", "j46", via=lanes and [(3.15, lanes[0]), (9.9, lanes[0])])
    bench.resistor ("1 kΩ", "g46", "e46")
    bench.resistor ("2 kΩ", "a46", "B-46")
    bench.wire ("modem.RXD", "c46", color="grey")
    bench.wire ("modem.TXD", "f44", color="purple")
    bench.wire ("15", "j44", via=lanes and [(3.25, lanes[1]), (9.7, lanes[1])])


receiver = Bench ("Board A: an IR receiver on pin 2, the LCD on pins 31 to 36, and a LoRa modem "
                  "on pins 14 and 15, its VDD from the Mega's 3.3V pin", columns=(1, 50),
                  sketch="Receiver")

receiver.screen (text=("Sent 0x0C, #3", "Lamp is on"))
modem (receiver, lanes=(-2.15, -2.05))
receiver.module ("ir_receiver", name="eye", at=(7.89, -1.7))
receiver.wire ("2", "eye.S", via=[(2.75, -0.2), (8.1, -0.2)])
receiver.wire ("eye.+", "T+29")
receiver.wire ("eye.−", "T-30")
receiver.closeup (26, 50)

repeater = Bench ("Board B: a relay on pin 11 switching a 9 V battery, 1 kΩ resistor and LED; an "
                  "IR LED on pin 3 through 220 Ω; and a LoRa modem on pins 14 and 15, its VDD from "
                  "the Mega's 3.3V pin", columns=(1, 50), sketch="Repeater")

modem (repeater, lanes=(-1.05, -0.95))

repeater.module ("relay", at=(5.05, -0.75), facing="left")
repeater.wire ("11", "relay.S", via=[(1.8, -0.35)])
repeater.wire ("5V.long", "relay.+", via=[(4.25, 0.7), (4.25, -0.25)])
repeater.wire ("GND.long", "relay.−", via=[(4.35, 2.4), (4.35, -0.15)])

repeater.wire ("relay.NO", "j13", via=[(6.6, -0.45)])
repeater.resistor ("1 kΩ", "g13", "e13")
repeater.led ("red", anode="b13", cathode="b14")
repeater.module ("battery9v", name="battery", at=(6.2, 3.3), facing="up")
repeater.wire ("battery.−", "a14")
repeater.wire ("battery.+", "relay.COM")

repeater.wire ("3", "j38", via=[(2.65, -1.15), (9.1, -1.15)])
repeater.resistor ("220 Ω", "g38", "e38")
repeater.module ("ir_transmitter", name="irled", at=(8.69, 3.45), facing="up")
repeater.wire ("irled.S", "a38")
repeater.wire ("irled.−", "B-36")

repeater.closeup (1, 50)

# Readings to take with a multimeter, all in the lamp's own circuit, as in
# Lesson 35: switched from the other board now.
repeater.measure ("The battery, through the relay", red="h13", black="b14", expect="about 9 V",
                  when="Lamp on")
repeater.measure ("The same, lamp off", red="h13", black="b14", expect="0 V",
                  when="Lamp off")

boards = {"A": receiver, "B": repeater}
