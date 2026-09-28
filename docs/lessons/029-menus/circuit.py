# The screen at its home, and the red LED on 3 at its home. The rotary
# encoder stands at its home in row a, columns 45 to 49, its knob toward
# you, on 18, 19 and 22.
bench = Bench ("An LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
               "on 22, and a lamp on pin 3", columns=(1, 50))

bench.screen (text=(">Level   60%", " Mode    Steady"))

bench.home_encoder ()

bench.home_led ("3", "red")

# Readings to take with a multimeter, with Mode on Steady: the lamp's pin at
# two levels, and the LED's share.
bench.measure ("Pin 3 at 60%", red="3", black="B-40", expect="about 3 V", when="Level 60%")
bench.measure ("Pin 3 at 20%", red="3", black="B-40", expect="about 1 V", when="Level 20%")
bench.measure ("Across the LED at 60%", red="b38", black="b39", expect="about 1.2 V",
               when="Level 60%")
