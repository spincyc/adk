# The power module at its home beside the board feeds only the bottom rails,
# for the motor; the Mega's 5V feeds the top rails, for the chip's logic and
# the knob, and the Mega's GND joins them all at B-3.
# The reverse button on 22 and the knob on A0 stand at their homes. The
# L293D's upper half drives the motor: the chip across the gap in columns
# 12-19, its logic power from the top + rail, its GND and the motor's power
# from the bottom rails. Forward, pin 8, drives 3A and so 3Y, which takes
# the motor's red lead, so forward puts + on the red lead. The motor lies
# at its home above the board, the wire from 9 passing under it and those
# from 8 and 4 over it.
bench = Bench ("A DC motor on an L293D (enable 4, forward 8, backward 9), powered from the "
               "breadboard power module, with a button on 22 and a knob on A0", columns=(1, 63))

bench.power_module ()

bench.home_button ("22")

bench.chip ("L293D", first=12)
bench.wire ("j12", "T+12")
bench.wire ("a15", "B-15")
bench.wire ("a19", "B+19")
bench.wire ("9", "j13", via=[(6.6, 0.35)])
bench.wire ("8", "j18", via=[(1.99, -1.57), (7.1, -1.57)])
bench.wire ("4", "j19", via=[(2.45, -1.67), (7.2, -1.67)])
bench.home_motor ()

bench.home_knob ()

bench.closeup (1, 50)

# Readings to take with a multimeter, the fan at one fixed speed set in the
# sketch.
bench.measure ("The enable pin", red="4", black="B-21", expect="about 2.5 V",
               when="speed 128, forward")
bench.measure ("The forward pin", red="8", black="B-21", expect="about 5 V",
               when="speed 128, forward")
bench.measure ("Across the motor", red="h17", black="h14", expect="about 3 V",
               when="speed 255, forward")
