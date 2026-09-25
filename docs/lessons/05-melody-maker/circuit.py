# Laid out from the end of the breadboard nearest the Mega, in pin order: the
# four key buttons on 22 to 25, then the passive buzzer on 10, standing across
# the middle gap, with its 220 Ω resistor from row a down into the − rail.
bench = Bench ("A four-key keyboard: buttons on pins 22 to 25, and a passive buzzer on pin 10 "
               "through 220 Ω", columns=(1, 24))

bench.wire ("22", "j1")
bench.button (1)
bench.wire ("a3", "B-3")

bench.wire ("23", "j5")
bench.button (5)
bench.wire ("a7", "B-7")

bench.wire ("24", "j9")
bench.button (9)
bench.wire ("a11", "B-11")

bench.wire ("25", "j13")
bench.button (13)
bench.wire ("a15", "B-15")

bench.wire ("10", "j19")
bench.buzzer ("f19", "e19", kind="passive")
bench.resistor ("220 Ω", "a19", "B-19")

bench.wire ("GND", "B-5")
