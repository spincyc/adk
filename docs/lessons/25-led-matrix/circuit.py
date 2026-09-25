# Laid out from the end of the breadboard nearest the Mega: pin 22, the button
# across the middle gap, and its ground from the bottom rail. The matrix sits
# below the breadboard on five jumpers from the end of the Mega's long header.
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

bench.wire ("22", "j1")
bench.button (1)
bench.wire ("a3", "B-3")
bench.wire ("GND4", "B-4")

bench.module ("matrix", at=(5.0, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (SMILEY)])
bench.wire ("47", "matrix.DIN", color="green")
bench.wire ("48", "matrix.CLK", color="blue")
bench.wire ("49", "matrix.CS", color="purple")
bench.wire ("5V3", "matrix.VCC")
bench.wire ("GND5", "matrix.GND")
