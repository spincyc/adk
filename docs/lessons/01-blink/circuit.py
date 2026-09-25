# Laid out from the end of the breadboard nearest the Mega, in the order the
# current flows: pin 26, the resistor, the LED, back to GND.
bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND")

bench.wire ("26", "a1")
bench.resistor ("220 Ω", "b1", "b5")
bench.led ("red", anode="c5", cathode="c6")
bench.wire ("a6", "B-6")
bench.wire ("GND", "B-3")
