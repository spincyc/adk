# Lesson 26's matrix and joystick stay as they were; the button goes. The
# passive buzzer stands at its home in column 34, pin 10's wire coming over
# the top of the board and its 220 Ω resistor going down to the bottom − rail.
bench = Bench ("The joystick and LED matrix of Lesson 26, and a passive buzzer on pin 10 "
               "through 220 Ω", columns=(1, 40))

# The matrix is drawn turned half round, so its picture is given upside down.
SNAKE = ["........",
         "........",
         ".####...",
         "....#...",
         "....###.",
         "........",
         "..#.....",
         "........"]

bench.module ("matrix", at=(4.22, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (SNAKE)])
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

bench.wire ("10", "j34")
bench.buzzer ("f34", "e34", kind="passive")
bench.resistor ("220 Ω", "a34", "B-34")
bench.closeup (1, 40)
