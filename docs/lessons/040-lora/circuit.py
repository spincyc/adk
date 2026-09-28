# The screen and button 23 keep their homes. The power module feeds the
# bottom rails at 3.3 V, while the Mega's 5 V feeds the screen's top rails.
# Modem B stands below columns 24 to 29 on Serial3, with its divider in
# 28 and TXD in 26. Modem A follows below columns 33 to 38 on Serial1,
# with its divider in 37 and TXD in 35. These holes also fit Lesson 41's
# modules, and Lesson 42 keeps A's divider.
bench = Bench ("Two LoRa modems: A on pins 18 and 19, B on pins 14 and 15, each on 3.3 V from "
               "the power module with its RXD through a 1 kΩ and 2 kΩ divider, a button on "
               "pin 23 and the LCD on pins 31 to 36", columns=(1, 63))

bench.power_module ("3.3V")
bench.screen (text=("Press 1", "-32 dBm  9 dB"))

bench.home_button ("23")

bench.module ("lora_modem", "modemB", at=(7.615, 3.45), label="LoRa modem B", facing="up")
bench.wire ("modemB.GND", "B-24")
bench.wire ("modemB.VDD", "B+29")
bench.wire ("14", "j28")
bench.resistor ("1 kΩ", "g28", "e28")
bench.resistor ("2 kΩ", "a28", "B-28")
bench.wire ("modemB.RXD", "c28", color="brown")
bench.wire ("modemB.TXD", "f26", color="purple")
bench.wire ("15", "j26")

bench.module ("lora_modem", "modemA", at=(8.515, 3.45), label="LoRa modem A", facing="up")
bench.wire ("modemA.GND", "B-33")
bench.wire ("modemA.VDD", "B+39")
bench.wire ("18", "j37")
bench.resistor ("1 kΩ", "g37", "e37")
bench.resistor ("2 kΩ", "a37", "B-37")
bench.wire ("modemA.RXD", "c37", color="white")
bench.wire ("modemA.TXD", "f35", color="grey")
bench.wire ("19", "j35")

bench.closeup (1, 63)

# Readings to take with a multimeter: the 3.3 V the modems run on, and
# modem A's divider, which turns pin 18's resting 5 V into 3.3 V.
bench.measure ("The modems' supply, on the bottom rails", red="B+40", black="B-40",
               expect="about 3.3 V", when="power module on")
bench.measure ("Pin 18, TX1, resting", red="18", black="GND", expect="about 5 V",
               when="between messages")
bench.measure ("Modem A's RXD, the middle of its divider", red="d37", black="GND",
               expect="about 3.3 V", when="between messages")
