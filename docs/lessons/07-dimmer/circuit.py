# Laid out from the end of the breadboard nearest the Mega: the bottom rails
# fed at that end; then the LED's loop, from pin 3 into the LED's long leg,
# out of its short leg through the resistor across the middle gap, and down
# to the GND rail; then the knob, its outer legs on the GND and 5 V rails and
# its wiper on A0.
bench = Bench ("A white LED on pin 3 through 220 Ω, and a knob on A0 between GND and 5 V")

bench.wire ("GND", "B-3")
bench.wire ("5V", "B+3")

bench.wire ("3", "j2")
bench.led ("white", anode="f2", cathode="f4")
bench.resistor ("220 Ω", "g4", "e4")
bench.wire ("a4", "B-4")

bench.potentiometer ("e8", "e9", "e10")
bench.wire ("B-7", "a8")
bench.wire ("A0", "a9")
bench.wire ("a10", "B+10")
