# Laid out from the end of the breadboard nearest the Mega: the button on
# pin 22; the rails fed at that end; the 74HC595, with its supply, its three
# wires from pins 37, 38 and 39, and GND; then a 1 kΩ resistor from each
# output the dice uses to its segment of the digit. The bottom row of
# segments (e, d, c) is reached straight along the bottom half; the top row
# (g, f, a, b) along the top half, Q6, Q5 and Q1 first crossing the middle
# gap on short wires. The digit's common pin goes to GND.
bench = Bench ("A button on pin 22, and a digit behind a 74HC595 on pins 37, 38 and 39",
               columns=(1, 36))

bench.button (1)
bench.wire ("22", "a1")
bench.wire ("a3", "B-3")

bench.wire ("5V", "T+4")
bench.wire ("GND", "T-4")
bench.wire ("GND", "B-4")

bench.chip ("74HC595", first=6)
bench.wire ("j6", "T+6")
bench.wire ("37", "j8")
bench.wire ("j9", "T-9")
bench.wire ("39", "j10")
bench.wire ("38", "j11")
bench.wire ("j12", "T+12")
bench.wire ("a13", "B-13")

bench.digit (27, shows="5")
bench.wire ("b29", "B-29")

bench.resistor ("1 kΩ", "d9", "c27")
bench.resistor ("1 kΩ", "c8", "b28")
bench.resistor ("1 kΩ", "b7", "a30")

bench.wire ("a11", "g21")
bench.wire ("a10", "g20")
bench.wire ("d6", "d5")
bench.wire ("e5", "g5")
bench.resistor ("1 kΩ", "j21", "j27")
bench.resistor ("1 kΩ", "i20", "i28")
bench.resistor ("1 kΩ", "i7", "j30")
bench.resistor ("1 kΩ", "h5", "i31")
