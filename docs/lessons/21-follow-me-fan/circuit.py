# Lesson 20's fan stays as it was: the power module on the right end, the
# L293D in columns 12-19 and the motor above the board, with the same wires
# from 4, 8 and 9; the knob and the button come out. The ultrasonic sensor
# comes back from Lesson 19 to the same place above the Mega, the wires
# from 4, 8 and 9 passing over it, and the servo goes in at its home below
# the board, its plug on the power module's bottom rails and its signal
# from pin 44.
bench = Bench ("A servo on pin 44 carrying an ultrasonic sensor on 14 and 15 and a fan on an "
               "L293D (4, 8, 9), the servo and the fan powered from the breadboard power module",
               columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")

bench.module ("ultrasonic", name="sensor", at=(2.715, -1.2))
bench.wire ("14", "sensor.Trig")
bench.wire ("15", "sensor.Echo")
bench.wire ("sensor.VCC", "5V.long")
bench.wire ("sensor.GND", "T-5")

bench.chip ("L293D", first=12)
bench.wire ("j12", "T+12")
bench.wire ("a15", "B-15")
bench.wire ("a19", "B+19")
bench.wire ("8", "j13", via=[(2.09, -1.47), (4.6, -1.47), (4.6, 0.35), (6.6, 0.35)])
bench.wire ("9", "j18", via=[(1.99, -1.57), (7.1, -1.57)])
bench.wire ("4", "j19", via=[(2.55, -1.67), (7.2, -1.67)])
bench.module ("motor", name="motor", at=(4.7, -1.25), facing="down")
bench.wire ("motor.−", "j14", via=[(6.7, -0.41)])
bench.wire ("motor.+", "j17", via=[(7.0, -0.59)])

bench.module ("servo", name="servo", at=(9.85, 3.45), facing="up")
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")
bench.wire ("44", "servo.signal")
bench.note ("tape the sensor and the fan to the horn", "servo.signal", offset=(-1.6, 1.2))

bench.closeup (1, 32)

# Readings to take with a multimeter while the fan blows at a book.
bench.measure ("The enable pin, a book at 70 cm", red="4", black="GND", expect="about 2.6 V",
               when="blowing")
bench.measure ("The enable pin, a book at 40 cm", red="4", black="GND", expect="about 3.9 V",
               when="blowing")
