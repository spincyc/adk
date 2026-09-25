# Laid out from the end of the breadboard nearest the Mega: the button on 22,
# then the RGB LED, its red, green and blue legs fed from pins 5, 6 and 7
# through a resistor each across the middle gap, its common leg in the − rail.
bench = Bench ("An RGB LED on pins 5, 6 and 7, and a button on pin 22", columns=(1, 20))

bench.wire ("22", "j1")
bench.button (1)
bench.wire ("a3", "B-3")

bench.wire ("5", "j8")
bench.wire ("6", "j10")
bench.wire ("7", "j11")
# The blue leg's resistor goes in first, so its value label stands clear of
# the other two.
bench.resistor ("220 Ω", "g11", "e11")
bench.resistor ("220 Ω", "g10", "e10")
bench.resistor ("220 Ω", "g8", "e8")
bench.rgb_led (red="a8", common="B-9", green="a10", blue="a11")

bench.wire ("GND", "B-5")
