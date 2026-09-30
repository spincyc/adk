# Lesson 1's LED path stays in its home holes. The meter readings use
# free holes in the same strips as the pin, resistor, LED and GND rail.
bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND", columns=(1, 20))

bench.home_led ("26", "red")

bench.measure ("Pin 26 to GND", red="26", black="GND", expect="about 5 V",
               when="LED on")
bench.measure ("Across the 220 Ω resistor", red="g6", black="a6", expect="about 3 V",
               when="LED on")
bench.measure ("Across the red LED", red="b6", black="b7", expect="about 2 V",
               when="LED on")
