# The button on 22 stands at its home, columns 2 to 4, grounded from the
# bottom − rail. The LED matrix lies below the gap between the Mega and the
# breadboard, input pins up, so 48, 49 and 47 drop straight onto CLK, CS and
# DIN; its GND comes from the bottom − rail (B-5) and its VCC from the inner
# 5V pin at the top of the long header. The Mega's GND feed into B-3 crosses
# the three signal wires, which leave the header just above it.
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

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.module ("matrix", at=(4.22, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (SMILEY)])
bench.wire ("48", "matrix.CLK")
bench.wire ("49", "matrix.CS")
bench.wire ("47", "matrix.DIN")
bench.wire ("B-5", "matrix.GND")
bench.wire ("5V.long", "matrix.VCC")
bench.closeup (1, 16)
