# The screen at its home, and the red LED on 3 at its home in column 38. The
# rotary encoder sits at its home above the Mega, its wires rising from 18,
# 19 and 22, its + from the inner 5V pin at the top of the long header and
# its GND from the GND beside pin 13; pin 3's wire goes over the encoder to
# the far end of the board.
bench = Bench ("An LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
               "on 22, and a lamp on pin 3", columns=(1, 40))

bench.screen (text=(">Level   60%", " Mode    Steady"))

bench.home_encoder ()

bench.home_led ("3", "red", via=[(2.65, -2.5), (9.1, -2.5)])

# Readings to take with a multimeter, with Mode on Steady: the lamp's pin at
# two levels, and the LED's share.
bench.measure ("Pin 3 at 60%", red="3", black="B-40", expect="about 3 V", when="Level 60%")
bench.measure ("Pin 3 at 20%", red="3", black="B-40", expect="about 1 V", when="Level 20%")
bench.measure ("Across the LED at 60%", red="b38", black="b39", expect="about 1.2 V",
               when="Level 60%")
