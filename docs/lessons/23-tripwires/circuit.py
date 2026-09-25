# Laid out from the end of the breadboard nearest the Mega: 5 V and GND onto the rails, the
# tilt switch on A14 first, then the four LEDs in the order their wires leave the header (blue on
# 29 first, so no wire crosses another). The three modules sit below the board and take their
# power from the bottom rails; a long wire at the far end brings 5 V down to the bottom + rail.
bench = Bench ("A PIR sensor on A12, an obstacle sensor on A13, a tilt switch on A14 and a "
               "beam-break sensor on A15, lighting the red, yellow, green and blue LEDs on 26 "
               "to 29", columns=(1, 40))

bench.wire ("5V", "T+3")
bench.wire ("GND", "B-3")

bench.tilt_switch ("c4", "c5")
bench.wire ("A14", "a4", color="brown")
bench.wire ("a5", "B-5")

for pin, color, wire, column in (("29", "blue", "blue", 10), ("28", "green", "green", 17),
                                 ("27", "yellow", "yellow", 24), ("26", "red", "orange", 31)):
    bench.wire (pin, f"e{column}", color=wire)
    bench.led (color, anode=f"a{column}", cathode=f"a{column + 1}")
    bench.resistor ("220 Ω", f"e{column + 1}", f"e{column + 5}")
    bench.wire (f"a{column + 5}", f"B-{column + 5}")

bench.wire ("T+37", "B+37")

bench.module ("sensor", name="beam", at=(5.9, 3.9), label="beam-break sensor",
              pins=("-", "+", "S"))
bench.wire ("A15", "beam.S", color="grey")
bench.wire ("beam.+", "B+10")
bench.wire ("beam.-", "B-11")
bench.module ("sensor", name="obstacle", at=(7.0, 3.9), label="obstacle sensor",
              pins=("GND", "+", "OUT", "EN"))
bench.wire ("A13", "obstacle.OUT", color="white")
bench.wire ("obstacle.+", "B+21")
bench.wire ("obstacle.GND", "B-23")
bench.module ("pir", name="pir", at=(8.4, 3.9))
bench.wire ("A12", "pir.OUT", color="purple")
bench.wire ("pir.GND", "B-37")
bench.wire ("pir.VCC", "B+39")
