# Laid out from the end of the breadboard nearest the Mega, in pin order: the
# buttons on 22 and 23, then the LEDs on 26 and 27. Each signal comes in along
# the top, crosses the middle gap through its part, and leaves by the − rail.
bench = Bench ("Two buttons on pins 22 and 23, and two LEDs on pins 26 and 27", columns=(1, 24))

bench.wire ("22", "j1")
bench.button (1)
bench.wire ("a3", "B-3")

bench.wire ("23", "j5")
bench.button (5)
bench.wire ("a7", "B-7")

bench.wire ("26", "j10")
bench.resistor ("220 Ω", "g10", "e10")
bench.led ("red", anode="b10", cathode="b11")
bench.wire ("a11", "B-11")

bench.wire ("27", "j15")
bench.resistor ("220 Ω", "g15", "e15")
bench.led ("yellow", anode="b15", cathode="b16")
bench.wire ("a16", "B-16")

bench.wire ("GND", "B-5")
