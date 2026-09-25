# The four LEDs at their homes: red on 26 in column 6, yellow on 27 in 12,
# green on 28 in 18 and blue on 29 in 24. The tilt switch follows them in
# columns 32 and 33, its wire from A14 coming up from below, and the
# beam-break and obstacle sensors lie below the board past it, each
# powered from the bottom rails above it. The PIR sits below the Mega at
# the place it keeps in Lesson 24, powered from the Mega's own 5V and GND.
bench = Bench ("A PIR sensor on A12, an obstacle sensor on A13, a tilt switch on A14 and a "
               "beam-break sensor on A15, lighting the red, yellow, green and blue LEDs on 26 "
               "to 29", columns=(1, 63))

for pin, color, column in (("26", "red", 6), ("27", "yellow", 12), ("28", "green", 18),
                           ("29", "blue", 24)):
    bench.wire (pin, f"j{column}")
    bench.resistor ("220 Ω", f"g{column}", f"e{column}")
    bench.led (color, anode=f"b{column}", cathode=f"b{column + 1}")
    bench.wire (f"a{column + 1}", f"B-{column + 1}")

bench.module ("pir", name="pir", at=(1.26, 3.8), facing="up")
bench.wire ("A12", "pir.OUT", via=[(3.49, 3.05), (1.89, 3.05)])
bench.wire ("5V.power", "pir.VCC", via=[(1.69, 2.9), (1.99, 2.9)])
bench.wire ("GND.power", "pir.GND")

bench.tilt_switch ("c32", "c33")
bench.wire ("A14", "a32")
bench.wire ("a33", "B-33")

bench.module ("sensor", name="beam", at=(8.58, 3.45), label="beam-break sensor",
              pins=("−", "+", "S"), facing="up")
bench.wire ("A15", "beam.S")
bench.wire ("beam.+", "B+36")
bench.wire ("beam.−", "B-37")

bench.module ("sensor", name="obstacle", at=(9.43, 3.45), label="obstacle sensor",
              pins=("GND", "+", "OUT", "EN"), facing="up")
bench.wire ("A13", "obstacle.OUT")
bench.wire ("obstacle.+", "B+45")
bench.wire ("obstacle.GND", "B-46")

bench.closeup (1, 48)
