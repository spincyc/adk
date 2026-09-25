# Laid out from the end of the breadboard nearest the Mega: the rails fed at
# that end; the 74HC595, with its supply, its wires from pins 37, 38 and 39,
# and GND; then a 1 kΩ resistor from each output to its segment of the
# display, the bottom row of segments (e, d, dp, c, g) along the bottom half
# and the top row (a, f, b) along the top half, Q5 and Q1 first crossing the
# middle gap on short wires. Last, the display's four digit pins, wired
# straight to pins 40 to 43.
bench = Bench ("A four-digit display behind a 74HC595 on pins 37 to 39, its digits on pins 40 to 43",
               columns=(1, 46))

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

bench.four_digits (30, shows="12.34")

bench.resistor ("1 kΩ", "d9", "c30")
bench.resistor ("1 kΩ", "c8", "b31")
bench.resistor ("1 kΩ", "b12", "a32")
bench.resistor ("1 kΩ", "b7", "b33")
bench.resistor ("1 kΩ", "c11", "c34")
bench.resistor ("1 kΩ", "h7", "i31")
bench.wire ("a10", "g18")
bench.resistor ("1 kΩ", "h18", "i32")
bench.wire ("d6", "d5")
bench.wire ("e5", "g5")
bench.resistor ("1 kΩ", "h5", "j35")

bench.wire ("40", "j30")
bench.wire ("41", "j33")
bench.wire ("42", "j34")
bench.wire ("43", "a35")
