# The four LEDs stand at their homes: red on 26, yellow on 27, green on 28
# and blue on 29. The tilt switch follows them in columns 32 and 33, its
# wire from A14 coming up from below. Past it, below the board, the
# beam-break sensor lies at its home and the obstacle sensor beside it,
# powered from the bottom rails above it. The PIR sits at its home below
# the Mega, the place it keeps in Lesson 24.
bench = Bench ("A PIR sensor on A12, an obstacle sensor on A13, a tilt switch on A14 and a "
               "beam-break sensor on A15, lighting the red, yellow, green and blue LEDs on 26 "
               "to 29", columns=(1, 63))

for pin, color in (("26", "red"), ("27", "yellow"), ("28", "green"), ("29", "blue")):
    bench.home_led (pin, color)

bench.home_pir ()

bench.tilt_switch ("c32", "c33")
bench.wire ("A14", "a32")
bench.wire ("a33", "B-33")

bench.home_beam ()

bench.module ("sensor", name="obstacle", at=(9.43, 3.45), label="obstacle sensor",
              pins=("GND", "+", "OUT", "EN"), facing="up")
bench.wire ("A13", "obstacle.OUT")
bench.wire ("obstacle.+", "B+45")
bench.wire ("obstacle.GND", "B-46")

bench.closeup (1, 48)

# Readings to take with a multimeter: the tilt switch's pin both ways up,
# and the bottom rails that feed the modules.
bench.measure ("The tilt switch's pin, upright", red="A14", black="GND", expect="0 V",
               when="standing up")
bench.measure ("The tilt switch's pin, tipped", red="A14", black="GND", expect="about 5 V",
               when="on its side")
bench.measure ("The bottom rails", red="B+21", black="B-21", expect="about 5 V", when="any time")
