# The matrix of Lessons 25 to 27 stays as it was; the joystick and buzzer
# go. The GY-521 stands in row j, columns 9 to 16, its board over the top
# rails: a red jumper from T+7, just beside it, feeds VCC, and two black
# jumpers carry GND across the gap to the bottom − rail. Pins 20 and 21 come
# over the top header and down onto the board beside it, clear of the rest.
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

bench.module ("matrix", at=(4.22, 3.9), facing="up",
              pixels=[row[::-1] for row in reversed (LEVEL)])
bench.wire ("48", "matrix.CLK")
bench.wire ("49", "matrix.CS")
bench.wire ("47", "matrix.DIN")
bench.wire ("B-5", "matrix.GND")
bench.wire ("5V.long", "matrix.VCC")

bench.header_module ("gy521", first=9, row="j")
bench.wire ("T+7", "i9")
bench.wire ("f10", "e10", color="black")
bench.wire ("a10", "B-10")
bench.wire ("21", "g11", via=[(3.85, 0.55), (5.75, 0.55), (5.75, 1.35)])
bench.wire ("20", "h12", via=[(3.75, 0.5), (5.85, 0.5), (5.85, 1.25)])
bench.closeup (1, 16)
