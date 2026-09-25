# The power module at its home on the right end feeds both pairs of rails;
# the Mega's GND joins them at B-3. The reverse button on 22 and the knob on
# A0 stand at their homes, in columns 2 and 45. The L293D's upper half
# drives the motor: the chip across the gap in columns 12-19, its logic
# power from the top + rail, its GND and the motor's power from the bottom
# rails. The motor lies above the board with its leads straight down into
# the chip's outputs, the wire from 8 passing under it and those from 9
# and 4 over it.
bench = Bench ("A DC motor on an L293D (enable 4, forward 8, backward 9), powered from the "
               "breadboard power module, with a button on 22 and a knob on A0", columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.chip ("L293D", first=12)
bench.wire ("j12", "T+12")
bench.wire ("a15", "B-15")
bench.wire ("a19", "B+19")
bench.wire ("8", "j13", via=[(6.6, 0.35)])
bench.wire ("9", "j18", via=[(1.99, -1.57), (7.1, -1.57)])
bench.wire ("4", "j19", via=[(2.55, -1.67), (7.2, -1.67)])
bench.module ("motor", name="motor", at=(4.7, -1.25), facing="down")
bench.wire ("motor.−", "j14", via=[(6.7, -0.41)])
bench.wire ("motor.+", "j17", via=[(7.0, -0.59)])

bench.potentiometer ("e45", "e46", "e47")
bench.wire ("a45", "B-45")
bench.wire ("A0", "a46")
bench.wire ("d47", "T+49")

bench.closeup (1, 50)
