# The power module at the right end feeds the bottom rails with 5 V for the
# driver (its top jumper off); the Mega's GND joins them at B-3. The button
# stands at its home: pin 22 into j2, a black jumper from a4 to the − rail.
# The driver lies below the Mega, its + and − from B+5 and B-6. Nothing
# carries on from Lesson 30.
bench = Bench ("A stepper motor's driver on pins A8 to A11, powered from the breadboard power "
               "module, and a button on pin 22", columns=(1, 63))

bench.power_module ("right", top="off", bottom="5V")
bench.home_button ("22", via=[(5.3, 1.05)])
# Each wire from A8-A11 steps down and across to its IN pin, A11 highest,
# so the four cross square on, in a tidy staircase.
bench.home_stepper ()
bench.closeup (1, 28)

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The driver's supply, on the bottom rails", red="B+10", black="B-10",
               expect="about 5 V", when="Power module on")
bench.measure ("The top rails, their jumper off", red="T+10", black="T-10", expect="0 V",
               when="Power module on")
