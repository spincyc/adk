# The joystick and the matrix stay where Lesson 26 put them. On the
# breadboard, from the end nearest the Mega, in the order the current meets
# them: pin 10, the 220 ohm resistor, the passive buzzer, and the top rail,
# reached from below the buzzer so the wire keeps clear of it.
bench = Bench ("The joystick and LED matrix of Lesson 26, and a passive buzzer on pin 10 "
               "through 220 Ω")

# The matrix is drawn turned half round, so its picture is given upside down.
SNAKE = ["........",
         "........",
         ".####...",
         "....#...",
         "....###.",
         "........",
         "..#.....",
         "........"]

bench.module ("joystick", at=(1.95, 4.0), facing="up")
bench.wire ("A3", "joystick.VRx", color="yellow")
bench.wire ("A4", "joystick.VRy", color="white")
bench.wire ("22", "joystick.SW", color="grey")
bench.wire ("5V", "joystick.+5V")
bench.wire ("GND4", "joystick.GND")

bench.module ("matrix", at=(5.0, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (SNAKE)])
bench.wire ("47", "matrix.DIN", color="green")
bench.wire ("48", "matrix.CLK", color="blue")
bench.wire ("49", "matrix.CS", color="purple")
bench.wire ("5V3", "matrix.VCC")
bench.wire ("GND5", "matrix.GND")

bench.wire ("10", "j1")
bench.resistor ("220 Ω", "f1", "f5")
bench.buzzer ("h5", "h8", kind="passive")
bench.wire ("T-15", "f8")
bench.wire ("GND", "T-3")
