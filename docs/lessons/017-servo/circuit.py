# Lesson 16's screen stays where it was; the keypad goes. The power module
# lies to the right of the board, not plugged in, both its jumpers off, and
# its 5V and GND feed only the bottom rails, for the servo, at B+61 and
# B-61. The Mega's 5V feeds the top rails, for the screen and the knob. The
# Mega's GND joins them all at B-3. The knob stands at its home, its wiper
# on A0, and the servo lies at its home below the board, its signal from
# pin 44.
bench = Bench ("A servo on pin 44 powered by the breadboard power module, a knob on A0, and the "
               "screen from Lesson 13", columns=(1, 63))

bench.power_module ()

bench.screen (text=("Knob   90°", "Needle 90°"), risers=(4.8, 0.05))

bench.home_knob (via=[(2.36, 2.85), (9.9, 2.85)])

bench.home_servo (via=[(4.55, 1.90), (4.55, 3.01), (10.5, 3.01)])

# Readings to take with a multimeter: the two 5 Vs, the power module's on
# the bottom rails for the servo and the Mega's on the top rails for the
# knob and the screen, and the knob's wiper on A0.
bench.measure ("The servo's 5 V, from the power module", red="B+49", black="B-52",
               expect="about 5 V", when="power module on")
bench.measure ("The knob's 5 V, from the Mega", red="h47", black="GND", expect="about 5 V",
               when="power module on or off")
bench.measure ("The knob's wiper, on A0", red="A0", black="GND", expect="about 2.5 V",
               when="the horn at 90°")
