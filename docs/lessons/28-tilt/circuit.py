# The GY-521 stands at the end of the breadboard nearest the Mega, lying flat
# over the top rails, and four wires in the same columns bring it power and
# the I2C bus. The matrix stays below the breadboard, as in Lesson 25.
bench = Bench ("A GY-521 accelerometer on the I2C pins 20 and 21, and the LED matrix")

# The matrix is drawn turned half round, so its picture is given upside down.
LEVEL = ["########",
         "#......#",
         "#......#",
         "#..##..#",
         "#..##..#",
         "#......#",
         "#......#",
         "########"]

bench.header_module ("gy521", first=2, row="j")
bench.wire ("5V2", "f2")
bench.wire ("GND4", "f3")
bench.wire ("20", "f5")
bench.wire ("21", "f4")

bench.module ("matrix", at=(5.0, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (LEVEL)])
bench.wire ("47", "matrix.DIN", color="green")
bench.wire ("48", "matrix.CLK", color="blue")
bench.wire ("49", "matrix.CS", color="purple")
bench.wire ("5V3", "matrix.VCC")
bench.wire ("GND5", "matrix.GND")
