# Lesson 25's matrix stays as it was. The button moves to its home for pin
# 23, since the joystick's switch takes 22. The joystick lies at its home
# below the Mega, on A3 and A4; the switch's wire from 22 comes down beside
# the header, across the matrix's wires.
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

bench.home_button ("23")

bench.home_matrix (pixels=[row[::-1] for row in reversed (DRAWING)])

bench.home_joystick ()

# Readings to take with a multimeter. The joystick's and the matrix's wires
# run straight to the Mega, so the meter reaches only the clear button.
bench.measure ("Pin 23, button up", red="23", black="GND", expect="about 5 V",
               when="button up")
bench.measure ("Pin 23, button pressed", red="23", black="GND", expect="0 V",
               when="button held down")
