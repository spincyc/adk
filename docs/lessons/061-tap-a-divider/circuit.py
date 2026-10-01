# Lesson 7's knob home: its two outer legs cross the 5 V supply, and its
# wiper shares A0's strip. Only the Mega's USB supply powers this build.
bench = Bench ("A knob between 5 V and GND, with its wiper on A0", columns=(1, 50))

bench.home_knob ()

bench.measure ("Wiper to GND", red="c40", black="GND", expect="about 2.5 V",
               when="Knob near the middle of its turn")
