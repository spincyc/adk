# Lesson 29's rotary encoder stays standing in row a; the screen and LED
# go. The GY-521 and the matrix come back as in Lesson 28, and the passive
# buzzer stands at its home.
bench = Bench ("The GY-521 and LED matrix of Lesson 28, the rotary encoder of Lesson 29, and a "
               "passive buzzer on pin 10", columns=(1, 50))

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

bench.home_matrix (pixels=[row[::-1] for row in reversed (MAZE)])

bench.home_buzzer ("passive")
bench.closeup (1, 37)

# Readings to take with a multimeter: the buzzer's pin on a high note and a
# low one, and the data line while the board tilts.
bench.measure ("Pin 10 on a high C", red="10", black="GND", expect="about 2.5 V",
               when="a long c6 tick")
bench.measure ("Pin 10 on a low C", red="10", black="GND", expect="about 2.5 V",
               when="a long c3 tick")
bench.measure ("SDA on a tilted board", red="f12", black="c10", expect="3.5 to 4 V",
               when="board tipped")
