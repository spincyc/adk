# The power module at the Mega's end of the board feeds the rails; the button
# sits just past it, from pin 22 across the gap to the − rail, and the driver
# lies below the Mega, on four wires from A8-A11 and two from the rails.
bench = Bench ("A stepper motor's driver on pins A8 to A11, powered from the breadboard power "
               "module, and a button on pin 22", columns=(1, 30))

bench.power_module ("left")
bench.wire ("GND", "B-7")
bench.wire ("22", "j8")
bench.button (8)
bench.wire ("a10", "B-10")
bench.module ("stepper", at=(2.0, 3.9), facing="up")
bench.wire ("A8", "stepper.IN1", color="green")
bench.wire ("A9", "stepper.IN2", color="blue")
bench.wire ("A10", "stepper.IN3", color="purple")
bench.wire ("A11", "stepper.IN4", color="white")
bench.wire ("B+6", "stepper.+")
bench.wire ("B-6", "stepper.-")
bench.closeup (3, 30)
