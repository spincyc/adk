# The course's screen at its home, and the button on 23 beside it in
# columns 38-40. The power module at the right end feeds the top rails at
# 5 V, for the screen, and the bottom rails at 3.3 V, for the modems; the
# Mega's GND joins them at B-3. The modems stand below the board past the
# button, B on Serial3 (pins 14 and 15) first and A on Serial1 (18 and 19)
# after it, as their pins lie along the Mega's header. Each takes two
# columns and the one between: its TXD comes straight up into f and meets
# its Mega RX pin's wire in j, and its RXD comes up beside the divider and
# into it, the Mega's TX pin through 1 kΩ across the gap and 2 kΩ down to
# the − rail. B's columns 41-48 and A's 49-57 leave room for Lesson 41's
# modules; Lesson 42 keeps A's, with the RGB LED's home free in 41-46.
bench = Bench ("Two LoRa modems: A on pins 18 and 19, B on pins 14 and 15, each on 3.3 V from "
               "the power module with its RXD through a 1 kΩ and 2 kΩ divider, a button on "
               "pin 23 and the LCD on pins 31 to 36", columns=(1, 63))

bench.power_module ("right", top="5V", bottom="3.3V")
bench.screen (text=("Press 1", "-32 dBm  9 dB"))

bench.wire ("23", "j38")
bench.button (38)
bench.wire ("a40", "B-40")

bench.module ("lora_modem", "modemB", at=(9.415, 3.45), label="LoRa modem B", facing="up")
bench.wire ("modemB.GND", "B-42")
bench.wire ("modemB.VDD", "B+47")
bench.wire ("14", "j46")
bench.resistor ("1 kΩ", "g46", "e46")
bench.resistor ("2 kΩ", "a46", "B-46")
bench.wire ("modemB.RXD", "c46", color="grey")
bench.wire ("modemB.TXD", "f44", color="purple")
bench.wire ("15", "j44")

bench.module ("lora_modem", "modemA", at=(10.315, 3.45), label="LoRa modem A", facing="up")
bench.wire ("modemA.GND", "B-51")
bench.wire ("modemA.VDD", "B+57")
bench.wire ("18", "j55")
bench.resistor ("1 kΩ", "g55", "e55")
bench.resistor ("2 kΩ", "a55", "B-55")
bench.wire ("modemA.RXD", "c55", color="white")
bench.wire ("modemA.TXD", "f53", color="grey")
bench.wire ("19", "j53")

bench.closeup (33, 63)

# Readings to take with a multimeter: the 3.3 V the modems run on, and
# modem A's divider, which turns pin 18's resting 5 V into 3.3 V.
bench.measure ("The modems' supply, on the bottom rails", red="B+58", black="B-58",
               expect="about 3.3 V", when="power module on")
bench.measure ("Pin 18, TX1, resting", red="18", black="GND", expect="about 5 V",
               when="between messages")
bench.measure ("Modem A's RXD, the middle of its divider", red="d55", black="GND",
               expect="about 3.3 V", when="between messages")
