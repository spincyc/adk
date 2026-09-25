# The power module plugs into all four rails at the far end, both jumpers
# on 5 V; the Mega's GND joins them at B-3. The knob stands at its home in
# e45-e47, its wiper on A0. Its right leg takes the Mega's own 5 V from the
# power header, not the rails': the 5 V the Mega measures A0 against, and
# one that stays on with the power module switched off. That wire runs below
# the board beside A0's. The servo lies below the board at its home, its
# plug under columns 52-54: power from the bottom rails, and its signal from
# pin 44.
bench = Bench ("A servo on pin 44 powered by the breadboard power module, and a knob on A0",
               columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46", via=[(2.2, 2.85), (9.9, 2.85)])
bench.wire ("5V.power", "a47", via=[(1.69, 2.93), (10.0, 2.93)])

bench.module ("servo", "servo", at=(9.85, 3.45), facing="up")
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")
bench.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 3.01), (10.5, 3.01)])

bench.closeup (38, 63)

# Readings to take with a multimeter: the two 5 Vs, the power module's for
# the servo and the Mega's for the knob, and the knob's wiper on A0.
bench.measure ("The servo's 5 V, from the power module", red="B+49", black="B-52",
               expect="about 5 V", when="power module on")
bench.measure ("The knob's 5 V, from the Mega", red="c47", black="GND", expect="about 5 V",
               when="power module on or off")
bench.measure ("The knob's wiper, on A0", red="A0", black="GND", expect="about 2.5 V",
               when="the horn at 90°")
