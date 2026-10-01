# Lesson 40's screen, button and dividers stay where they were; the modems
# go, and two LoRa modules take their places below the board, B on Serial3
# first and A on Serial1 after it. The power module's red wire moves from
# its 3.3V pin to its 5V pin, for the modules; the Mega's 5V still feeds
# the top rails, for the screen and the DHT11. Each module's TXD
# still comes up into f to meet its Mega RX pin's wire, and its RXD still
# comes up beside its divider; new are its AUX, up into f beside its TXD
# to meet its Mega pin, and its M0 and M1, up into f and g two columns
# past the divider, where a single Mega pin's wire joins them. The DHT11
# is at its home above the board, as in Lesson 15.
bench = Bench ("Two LoRa modules: A on pins 18 and 19, its M0 and M1 on 40 and AUX on 41; B on "
               "pins 14 and 15, its M0 and M1 on 42 and AUX on 43; each RXD through a 1 kΩ and "
               "2 kΩ divider, and both on 5 V from the power module. The DHT11 on pin 16, the "
               "button on 23 and the LCD on 31 to 36", columns=(1, 63))

bench.power_module ()
bench.screen (text=("Temp 23C Hum 45%", "1 s ago"), risers=(4.4, 0.1))

bench.home_dht11 ()

bench.module ("lora_module", "linkB", at=(7.508, 3.75), label="LoRa module B", facing="up")
bench.wire ("linkB.GND", "B-23")
bench.wire ("linkB.VCC", "B+24")
bench.wire ("14", "j28")
bench.resistor ("1 kΩ", "g28", "e28")
bench.resistor ("2 kΩ", "a28", "B-28")
bench.wire ("linkB.RXD", "c28", color="grey")
bench.wire ("linkB.TXD", "f26", color="purple")
bench.wire ("15", "j26")
bench.wire ("linkB.AUX", "f25", color="brown")
bench.wire ("43", "j25")
bench.wire ("linkB.M1", "g30", color="white")
bench.wire ("linkB.M0", "f30", color="white")
bench.wire ("42", "j30")

bench.module ("lora_module", "linkA", at=(8.508, 3.75), label="LoRa module A", facing="up")
bench.wire ("linkA.GND", "B-31")
bench.wire ("linkA.VCC", "B+33")
bench.wire ("18", "j37")
bench.resistor ("1 kΩ", "g37", "e37")
bench.resistor ("2 kΩ", "a37", "B-37")
bench.wire ("linkA.RXD", "c37", color="white")
bench.wire ("linkA.TXD", "f35", color="grey")
bench.wire ("19", "j35")
bench.wire ("linkA.AUX", "f34", color="purple")
bench.wire ("41", "j34")
bench.wire ("linkA.M1", "g39", color="yellow")
bench.wire ("linkA.M0", "f39", color="yellow")
bench.wire ("40", "j39")

bench.home_button ("23")

# Readings to take with a multimeter: the 5 V the modules run on, the pin
# that holds module A's M0 and M1 low, and module A's AUX, high while it
# is free.
bench.measure ("The modules' supply, on the bottom rails", red="B+40", black="B-40",
               expect="about 5 V", when="power module on")
bench.measure ("Pin 40, module A's M0 and M1", red="40", black="GND", expect="0 V",
               when="while the sketch runs")
bench.measure ("Module A's AUX, on pin 41", red="41", black="GND", expect="about 3.3 V",
               when="between reports")
