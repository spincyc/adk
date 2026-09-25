# Lesson 29's rotary encoder stays as it was; the screen and LED go. The
# GY-521 and the matrix come back as in Lesson 28, except that the matrix's
# VCC takes the power header's 5V, since the encoder has the long header's
# inner 5V pin. The passive buzzer stands at its home in column 34, pin 10's
# wire going over the encoder, its 220 Ω resistor down to the − rail.
bench = Bench ("The GY-521 and LED matrix of Lesson 28, the rotary encoder of Lesson 29, and a "
               "passive buzzer on pin 10", columns=(1, 40))

# The matrix is drawn turned half round, so its picture is given upside down.
MAZE = ["#.......",
        "######..",
        "........",
        "..######",
        "........",
        "######..",
        "........",
        ".......#"]

bench.module ("encoder", at=(3.0, -2.1))
bench.wire ("18", "encoder.CLK", via=[(3.55, 0.55), (3.2, 0.55)])
bench.wire ("19", "encoder.DT", via=[(3.65, 0.45), (3.3, 0.45)])
bench.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.35), (3.4, 0.35)])
bench.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, 0.25), (3.5, 0.25)])
bench.wire ("GND.top", "encoder.GND", via=[(1.5, 0.15), (3.6, 0.15)])

bench.header_module ("gy521", first=9, row="j")
bench.wire ("T+7", "i9")
bench.wire ("f10", "e10", color="black")
bench.wire ("a10", "B-10")
bench.wire ("21", "g11", via=[(3.85, 0.55), (5.75, 0.55), (5.75, 1.35)])
bench.wire ("20", "h12", via=[(3.75, 0.5), (5.85, 0.5), (5.85, 1.25)])

bench.module ("matrix", at=(4.22, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (MAZE)])
bench.wire ("48", "matrix.CLK")
bench.wire ("49", "matrix.CS")
bench.wire ("47", "matrix.DIN")
bench.wire ("B-5", "matrix.GND")
bench.wire ("5V.power", "matrix.VCC")

bench.wire ("10", "j34", via=[(1.9, -2.5), (8.7, -2.5)])
bench.buzzer ("f34", "e34", kind="passive")
bench.resistor ("220 Ω", "a34", "B-34")
bench.closeup (1, 37)
