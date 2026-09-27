# Both parts at their homes: the white LED on pin 3, the one that dims, and
# the knob, its wiper on A0. Lesson 8 takes both out; Lesson 9 brings the
# knob back to the same holes.
bench = Bench ("A white LED on pin 3 through 220 Ω, and a knob on A0 between GND and 5 V",
               columns=(1, 50))

bench.home_led ("3", "white", via=[(2.65, 0.45), (9.10, 0.45)])

bench.home_knob (via=[(2.20, 2.85), (9.95, 2.85)],
                 supply=[(10.05, 1.85), (10.25, 1.85), (10.25, 0.75)])

# Readings to take with a multimeter, with the knob turned until the sketch
# reads about 256: a quarter of the way from the GND end.
bench.measure ("The knob's wiper", red="d46", black="GND", expect="about 1.25 V",
               when="Knob reading about 256")
bench.measure ("From 5 V down to the wiper", red="5V", black="c46", expect="about 3.75 V",
               when="Knob reading about 256")
bench.measure ("Pin 3, averaged", red="3", black="GND", expect="about 1.25 V",
               when="Knob reading about 256")
