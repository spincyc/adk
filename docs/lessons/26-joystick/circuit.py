# Lesson 25's matrix stays as it was. The button moves to its home for pin
# 23, columns 8 to 10, since the joystick's switch takes 22. The joystick
# lies below the Mega under A3 and A4, its +5V from the power header and its
# GND from the inner GND pin at the end of the long header; the switch's
# wire from 22 comes down beside the header, across the matrix's wires.
bench = Bench ("A joystick on A3 and A4 with its switch on pin 22, the LED matrix, "
               "and a clear button on pin 23")

# The matrix is drawn turned half round, so its picture is given upside down.
DRAWING = ["........",
           ".##.....",
           ".#.#....",
           ".#..#...",
           ".#...###",
           ".#......",
           ".######.",
           "........"]

bench.wire ("23", "j8")
bench.button (8)
bench.wire ("a10", "B-10")

bench.module ("matrix", at=(4.22, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (DRAWING)])
bench.wire ("48", "matrix.CLK")
bench.wire ("49", "matrix.CS")
bench.wire ("47", "matrix.DIN")
bench.wire ("B-5", "matrix.GND")
bench.wire ("5V.long", "matrix.VCC")

bench.module ("joystick", at=(2.08, 3.9), facing="up")
bench.wire ("A3", "joystick.VRx")
bench.wire ("A4", "joystick.VRy")
bench.wire ("5V.power", "joystick.+5V")
bench.wire ("GND.long", "joystick.GND")
bench.wire ("22", "joystick.SW")
bench.closeup (1, 16)
