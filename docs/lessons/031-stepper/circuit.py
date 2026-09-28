# The power module at the right end feeds the bottom rails with 5 V for the
# driver (its top jumper off); the Mega's GND joins them at B-3. The button
# on 22 stands at its home, and the driver lies at its home below the Mega.
# Nothing carries on from Lesson 30.
bench = Bench ("A stepper motor's driver on pins A8 to A11, powered from the breadboard power "
               "module, and a button on pin 22", columns=(1, 63))

bench.power_module ()
bench.home_button ("22", via=[(5.3, 1.05)])
bench.home_stepper ()
bench.closeup (1, 28)

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The driver's supply, on the bottom rails", red="B+10", black="B-10",
               expect="about 5 V", when="Power module on")
