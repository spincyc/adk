bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND")

bench.wire ("26", "a10")
bench.resistor ("220 Ω", "b10", "b14")
bench.led ("red", anode="c14", cathode="c15")
bench.wire ("a15", "B-15")
bench.wire ("GND", "B-9")
