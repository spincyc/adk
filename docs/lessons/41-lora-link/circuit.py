# Lesson 40's screen, button and dividers stay where they were; the modems
# go, and two LoRa modules take their places below the board, B on Serial3
# first and A on Serial1 after it. The power module's bottom jumper moves
# to 5V, for the modules. Each module's TXD still comes up into f to meet
# its Mega RX pin's wire, and its RXD still comes up beside its divider;
# new are its AUX, up into f beside its TXD to meet its Mega pin, and its
# M0 and M1, up into f and g two columns past the divider, where a
# single Mega pin's wire joins them. The DHT11 is at its home above the
# board, as in Lesson 15.
bench = Bench ("Two LoRa modules: A on pins 18 and 19, its M0 and M1 on 40 and AUX on 41; B on "
               "pins 14 and 15, its M0 and M1 on 42 and AUX on 43; each RXD through a 1 kΩ and "
               "2 kΩ divider, and both on 5 V from the power module. The DHT11 on pin 16, the "
               "button on 23 and the LCD on 31 to 36", columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")
bench.screen (text=("Temp 23C Hum 45%", "1 s ago"), risers=(4.4, 0.1))

bench.module ("dht11", "dht", at=(8.58, -1.27))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+36")
bench.wire ("dht.−", "T-37")

bench.module ("lora_module", "linkB", at=(9.308, 3.75), label="LoRa module B", facing="up")
bench.wire ("linkB.GND", "B-41")
bench.wire ("linkB.VCC", "B+42")
bench.wire ("14", "j46", via=[(3.15, -1.95), (9.9, -1.95)])
bench.resistor ("1 kΩ", "g46", "e46")
bench.resistor ("2 kΩ", "a46", "B-46")
bench.wire ("linkB.RXD", "c46", color="grey")
bench.wire ("linkB.TXD", "f44", color="purple", via=[(9.7, 2.4)])
bench.wire ("15", "j44", via=[(3.25, -1.85), (9.7, -1.85)])
bench.wire ("linkB.AUX", "f43", color="brown", via=[(9.6, 2.4)])
bench.wire ("43", "j43", via=[(4.3, 1.85), (5.1, 1.85), (5.1, -1.75), (9.6, -1.75)])
bench.wire ("linkB.M1", "g48", color="white", via=[(9.9, 2.95), (10.05, 2.95), (10.05, 1.4)])
bench.wire ("linkB.M0", "f48", color="white", via=[(10.0, 3.05), (10.1, 3.05)])
bench.wire ("42", "j48", via=[(4.3, 1.8), (5.05, 1.8), (5.05, -2.05), (10.1, -2.05)])

bench.module ("lora_module", "linkA", at=(10.308, 3.75), label="LoRa module A", facing="up")
bench.wire ("linkA.GND", "B-49", via=[(10.4, 3.1), (10.2, 3.1)])
bench.wire ("linkA.VCC", "B+51", via=[(10.5, 3.05), (10.4, 3.05)])
bench.wire ("18", "j55", via=[(3.55, -2.35), (10.8, -2.35)])
bench.resistor ("1 kΩ", "g55", "e55")
bench.resistor ("2 kΩ", "a55", "B-55")
bench.wire ("linkA.RXD", "c55", color="white", via=[(10.8, 2.9), (10.7, 2.9)])
bench.wire ("linkA.TXD", "f53", color="grey", via=[(10.7, 2.95), (10.6, 2.95)])
bench.wire ("19", "j53", via=[(3.65, -2.25), (10.6, -2.25)])
bench.wire ("linkA.AUX", "f52", color="purple", via=[(10.6, 3.0), (10.5, 3.0)])
bench.wire ("41", "j52", via=[(4.3, 1.75), (5.0, 1.75), (5.0, -2.15), (10.5, -2.15)])
bench.wire ("linkA.M1", "g57", color="orange", via=[(10.9, 3.0), (10.95, 3.0), (10.95, 1.4)])
bench.wire ("linkA.M0", "f57", color="orange")
bench.wire ("40", "j57", via=[(4.3, 1.7), (4.95, 1.7), (4.95, -2.45), (11.0, -2.45)])

bench.wire ("23", "j38", via=[(8.45, 0.95), (9.1, 0.95)])
bench.button (38)
bench.wire ("a40", "B-40")

bench.closeup (33, 63)

# Readings to take with a multimeter: the 5 V the modules run on, the pin
# that holds module A's M0 and M1 low, and module A's AUX, high while it
# is free.
bench.measure ("The modules' supply, on the bottom rails", red="B+58", black="B-58",
               expect="about 5 V", when="power module on")
bench.measure ("Pin 40, module A's M0 and M1", red="40", black="GND", expect="0 V",
               when="while the sketch runs")
bench.measure ("Module A's AUX, on pin 41", red="41", black="GND", expect="about 3.3 V",
               when="between reports")
