# The joystick sits below the Mega, under the analog pins it uses, and the
# matrix below the breadboard, as in Lesson 25. On the breadboard, from the end
# nearest the Mega: pin 23, the clear button, and its ground from the top rail.
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

bench.module ("joystick", at=(1.95, 4.0), facing="up")
bench.wire ("A3", "joystick.VRx", color="yellow")
bench.wire ("A4", "joystick.VRy", color="white")
bench.wire ("22", "joystick.SW", color="grey")
bench.wire ("5V", "joystick.+5V")
bench.wire ("GND4", "joystick.GND")

bench.module ("matrix", at=(5.0, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (DRAWING)])
bench.wire ("47", "matrix.DIN", color="green")
bench.wire ("48", "matrix.CLK", color="blue")
bench.wire ("49", "matrix.CS", color="purple")
bench.wire ("5V3", "matrix.VCC")
bench.wire ("GND5", "matrix.GND")

bench.wire ("23", "j1")
bench.button (1)
bench.wire ("T-5", "j3")
bench.wire ("GND", "T-3")
