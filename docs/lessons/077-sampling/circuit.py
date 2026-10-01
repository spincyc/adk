# Rebuild Lesson 7's dimmer at its usual homes. The meter compares the
# knob's continuous wiper voltage with A0's numbered samples.
bench = Bench ("A knob on A0 sets a white LED on pin 3", columns=(1, 50))

bench.home_led ("3", "white", via=[(2.55, 0.45), (9.10, 0.45)])
bench.home_knob ()

bench.measure ("Knob wiper", red="c40", black="GND", expect="about 2.5 V",
               when="Hold the knob near the middle of its electrical range")
bench.measure ("Mega supply", red="5V", black="GND", expect="about 5 V",
               when="Leave the knob still")
