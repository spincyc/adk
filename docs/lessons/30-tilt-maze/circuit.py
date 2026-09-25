# The GY-521 stands in row j as in Lesson 28, moved along to column 7 so the
# top − rail's ground wire fits in at column 3, with room beside it for the
# wires that come down to the GY-521's pins. The buzzer circuit
# follows in the order the current meets it. The matrix stays below the
# breadboard, and the rotary encoder sits below the Mega, powered from the
# power header, where Lesson 26 put the joystick.
bench = Bench ("The GY-521 and LED matrix of Lesson 28, a rotary encoder on 18 and 19 with "
               "its switch on 22, and a passive buzzer on pin 10")

# The matrix is drawn turned half round, so its picture is given upside down.
MAZE = ["#.......",
        "######..",
        "........",
        "..######",
        "........",
        "######..",
        "........",
        ".......#"]

bench.header_module ("gy521", first=7, row="j")
bench.wire ("5V2", "f7")
bench.wire ("GND4", "f8")
bench.wire ("20", "f10")
bench.wire ("21", "f9")

bench.wire ("10", "j16")
bench.resistor ("220 Ω", "f16", "f20")
bench.buzzer ("h20", "h23", kind="passive")
bench.wire ("T-30", "f23")

bench.module ("matrix", at=(5.0, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (MAZE)])
bench.wire ("47", "matrix.DIN", color="green")
bench.wire ("48", "matrix.CLK", color="blue")
bench.wire ("49", "matrix.CS", color="purple")
bench.wire ("5V3", "matrix.VCC")
bench.wire ("GND5", "matrix.GND")

bench.module ("encoder", at=(3.0, 4.0), facing="up")
bench.wire ("18", "encoder.CLK", color="white")
bench.wire ("19", "encoder.DT", color="grey")
bench.wire ("22", "encoder.SW", color="brown")
bench.wire ("5V", "encoder.+")
bench.wire ("GND2", "encoder.GND")
bench.wire ("GND", "T-3")
