# The matrix of Lessons 25 to 27 stays as it was; the joystick and buzzer
# go. The GY-521 stands at its home, on pins 20 and 21, which come over the
# top header and down onto the board beside it, clear of the rest.
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

bench.home_matrix (pixels=[row[::-1] for row in reversed (LEVEL)])

bench.home_gy521 ()

# Readings to take with a multimeter, the black probe in the GY-521's GND
# column: its supply, the data line resting high between readings, and the
# address pin that the module's own resistor holds low.
bench.measure ("The module's supply", red="f9", black="c10", expect="about 5 V", when="any time")
bench.measure ("SDA, the data line", red="f12", black="c10", expect="about 3.3 V",
               when="sketch running")
bench.measure ("AD0, the address pin", red="f15", black="c10", expect="about 0 V", when="any time")
