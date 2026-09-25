# Laid out from the end of the breadboard nearest the Mega, in pin order: the
# four buttons on 22 to 25, the red, yellow, green and blue LEDs on 26 to 29,
# then the passive buzzer on 10, across the middle gap, with its 220 Ω
# resistor into the − rail. Each signal comes in along the top, crosses the
# gap, and leaves by the − rail. The close-up starts at the last button.
bench = Bench ("Simon: buttons on pins 22 to 25, LEDs on 26 to 29, and a passive buzzer on 10",
               columns=(1, 44))

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

bench.wire ("26", "j17")
bench.resistor ("220 Ω", "g17", "e17")
bench.led ("red", anode="b17", cathode="b18")
bench.wire ("a18", "B-18")

bench.wire ("27", "j23")
bench.resistor ("220 Ω", "g23", "e23")
bench.led ("yellow", anode="b23", cathode="b24")
bench.wire ("a24", "B-24")

bench.wire ("28", "j29")
bench.resistor ("220 Ω", "g29", "e29")
bench.led ("green", anode="b29", cathode="b30")
bench.wire ("a30", "B-30")

bench.wire ("29", "j35")
bench.resistor ("220 Ω", "g35", "e35")
bench.led ("blue", anode="b35", cathode="b36")
bench.wire ("a36", "B-36")

bench.wire ("10", "j40")
bench.buzzer ("f40", "e40", kind="passive")
bench.resistor ("220 Ω", "a40", "B-40")

bench.wire ("GND", "B-5")

bench.closeup (13, 43)
