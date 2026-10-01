# The button on 22 stands at its home. The LED matrix lies at its home,
# input pins up, so 48, 49 and 47 drop straight onto CLK, CS and DIN. The
# Mega's GND feed into B-3 crosses the three signal wires, which leave the
# header just above it.
bench = Bench ("An LED matrix on pins 47, 48 and 49, and a button on pin 22")

# The matrix lies below the board with its input pins pointing up at it, so
# it is drawn turned half round, and its picture is given upside down.
SMILEY = ["..####..",
          ".#....#.",
          "#.#..#.#",
          "#......#",
          "#.#..#.#",
          "#..##..#",
          ".#....#.",
          "..####.."]

bench.home_button ("22")

bench.home_matrix (pixels=[row[::-1] for row in reversed (SMILEY)])

# Readings to take with a multimeter. The matrix's wires run straight to
# the Mega, so the meter reaches only the button's pin. The rail has no
# free hole between the matrix's GND and the Mega's, B-5 and B-3, so the
# matrix's return current can't be measured along it either.
bench.measure ("Pin 22, button up", red="22", black="GND", expect="about 5 V",
               when="button up, smiley showing")
bench.measure ("Pin 22, button pressed", red="22", black="GND", expect="0 V",
               when="button held down, smiley showing")
