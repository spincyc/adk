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

bench.home_encoder ()

bench.home_gy521 ()

bench.home_matrix (vcc="5V.power", pixels=[row[::-1] for row in reversed (MAZE)])

bench.home_buzzer ("passive", via=[(1.9, -2.5), (8.7, -2.5)])
bench.closeup (1, 37)

# Readings to take with a multimeter: the buzzer's pin on a high note and a
# low one, and the data line while the board tilts.
bench.measure ("Pin 10 on a high C", red="10", black="GND", expect="about 2.5 V",
               when="a long c6 tick")
bench.measure ("Pin 10 on a low C", red="10", black="GND", expect="about 2.5 V",
               when="a long c3 tick")
bench.measure ("SDA on a tilted board", red="f12", black="c10", expect="3.5 to 4 V",
               when="board tipped")
