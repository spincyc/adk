# Laid out from the end of the breadboard nearest the Mega, in pin order: the
# players' buttons on 22 and 23, the red, yellow and green LEDs on 26 to 28,
# then the active buzzer on 12, standing across the middle gap. Each signal
# comes in along the top, crosses the gap, and leaves by the − rail.
bench = Bench ("A two-player reaction game: buttons on 22 and 23, LEDs on 26 to 28, "
               "and an active buzzer on 12", columns=(1, 30))

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

bench.wire ("28", "j20")
bench.resistor ("220 Ω", "g20", "e20")
bench.led ("green", anode="b20", cathode="b21")
bench.wire ("a21", "B-21")

bench.wire ("12", "j25")
bench.buzzer ("f25", "e25", kind="active")
bench.wire ("a25", "B-25")

bench.wire ("GND", "B-5")
