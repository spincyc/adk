# The power module at the right end feeds the bottom rails with 5 V for the
# driver (its top jumper off); the Mega's GND joins them at B-3. The button
# stands at its home: pin 22 into j2, a black jumper from a4 to the − rail.
# The driver lies below the Mega, its + and − from B+5 and B-6. Nothing
# carries on from Lesson 30.
bench = Bench ("A stepper motor's driver on pins A8 to A11, powered from the breadboard power "
               "module, and a button on pin 22", columns=(1, 63))

bench.power_module ("right", top="off", bottom="5V")
bench.wire ("22", "j2", via=[(5.3, 1.05)])
bench.button (2)
bench.wire ("a4", "B-4")
bench.module ("stepper", at=(3.1, 4.5), facing="up")
# Each wire from A8-A11 steps down and across to its IN pin, A11 highest,
# so the four cross square on, in a tidy staircase.
bench.wire ("A8", "stepper.IN1", via=[(3.09, 3.5), (4.45, 3.5)])
bench.wire ("A9", "stepper.IN2", via=[(3.19, 3.3), (4.35, 3.3)])
bench.wire ("A10", "stepper.IN3", via=[(3.29, 3.1), (4.25, 3.1)])
bench.wire ("A11", "stepper.IN4", via=[(3.39, 2.9), (4.15, 2.9)])
bench.wire ("stepper.+", "B+5", via=[(3.56, 3.65), (5.8, 3.65)])
bench.wire ("stepper.−", "B-6", via=[(3.66, 3.75), (5.9, 3.75)])
bench.closeup (1, 28)
