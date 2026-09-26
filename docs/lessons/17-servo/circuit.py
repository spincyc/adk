# Lesson 16's screen stays where it was; the keypad goes. The power module
# plugs into all four rails at the far end, its top jumper off and its
# bottom one on 5 V: the Mega's 5V feeds the top rails, for the screen and
# the knob, and the module feeds only the bottom rails, for the servo. The
# Mega's GND joins them all at B-3. The knob stands at its home in e45-e47,
# its left leg to the − rail, its wiper on A0 and its right leg up to the
# top + rail. The servo lies below the board at its home, its plug under
# columns 52-54: power from the bottom rails, and its signal from pin 44.
bench = Bench ("A servo on pin 44 powered by the breadboard power module, a knob on A0, and the "
               "screen from Lesson 13", columns=(1, 63))

bench.power_module ("right", top="off", bottom="5V")

bench.screen (text=("Knob   90°", "Needle 90°"), risers=(4.8, 0.05))

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46", via=[(2.2, 2.85), (9.9, 2.85)])
bench.wire ("d47", "T+49", via=[(10.05, 1.85), (10.25, 1.85), (10.25, 0.75)])

bench.module ("servo", "servo", at=(9.85, 3.45), facing="up")
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")
bench.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 3.01), (10.5, 3.01)])

# Readings to take with a multimeter: the two 5 Vs, the power module's on
# the bottom rails for the servo and the Mega's on the top rails for the
# knob and the screen, and the knob's wiper on A0.
bench.measure ("The servo's 5 V, from the power module", red="B+49", black="B-52",
               expect="about 5 V", when="power module on")
bench.measure ("The knob's 5 V, from the Mega", red="c47", black="GND", expect="about 5 V",
               when="power module on or off")
bench.measure ("The knob's wiper, on A0", red="A0", black="GND", expect="about 2.5 V",
               when="the horn at 90°")
