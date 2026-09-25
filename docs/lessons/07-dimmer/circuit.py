# Both parts at their homes. The white LED, the one that dims, in column 38:
# pin 3 into j38, its 220 Ω across the middle gap, the LED, and a black
# jumper to the − rail. Then the knob in e45 to e47: its left leg to the −
# rail, its wiper on A0, and its right leg up to the top + rail. Lesson 8
# takes both out; Lesson 9 brings the knob back to the same holes.
bench = Bench ("A white LED on pin 3 through 220 Ω, and a knob on A0 between GND and 5 V",
               columns=(1, 50))

bench.wire ("3", "j38", via=[(2.65, 0.45), (9.10, 0.45)])
bench.resistor ("220 Ω", "g38", "e38")
bench.led ("white", anode="b38", cathode="b39")
bench.wire ("a39", "B-39")

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46", via=[(2.20, 2.85), (9.95, 2.85)])
bench.wire ("d47", "T+49", via=[(10.05, 1.85), (10.25, 1.85), (10.25, 0.75)])
