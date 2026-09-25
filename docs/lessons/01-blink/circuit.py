# The red LED at its home in column 6, where it stays for the lessons that
# follow: from pin 26 into the top half, through the resistor across the
# middle gap, the LED, and back to GND by the bottom − rail, which the
# Mega's GND feeds at B-3.
bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND", columns=(1, 20))

bench.wire ("26", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

# Readings to take with a multimeter while the LED is on.
bench.measure ("The pin's 5 V, from pin 26 to GND", red="26", black="GND", expect="about 5 V",
               when="LED on")
bench.measure ("Across the resistor", red="g6", black="a6", expect="about 3 V", when="LED on")
bench.measure ("Across the LED", red="b6", black="b7", expect="about 2 V", when="LED on")
