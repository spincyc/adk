# Lesson 20's fan stays as it was: the power module beside the board, its
# wires in B+42 and B-42, the L293D in columns 12-19 and the motor above
# the board, with the same wires from 4, 8 and 9; the knob and the button
# come out.
# The ultrasonic sensor comes back from Lesson 19 to the same place above
# the Mega, the wires from 4, 8 and 9 passing over it, and the servo goes in
# at its home below the board, its plug on the power module's bottom rails
# and its signal from pin 44. On the real bench the motor rides on the
# servo's horn, its leads lengthened with female-to-male jumpers.
bench = Bench ("A servo on pin 44 carrying an ultrasonic sensor on 14 and 15 and a fan on an "
               "L293D (4, 8, 9), the servo and the fan powered from the breadboard power module",
               columns=(1, 63))

bench.power_module ()

bench.home_ultrasonic ()

bench.chip ("L293D", first=12)
bench.wire ("j12", "T+12")
bench.wire ("a15", "B-15")
bench.wire ("a19", "B+19")
bench.wire ("9", "j13", via=[(1.89, -1.47), (4.6, -1.47), (4.6, 0.35), (6.6, 0.35)])
bench.wire ("8", "j18", via=[(1.99, -1.57), (7.1, -1.57)])
bench.wire ("4", "j19", via=[(2.45, -1.67), (7.2, -1.67)])
bench.home_motor ()

bench.home_servo ()
bench.note ("tape the sensor and the fan to the horn", "servo.signal", offset=(-1.6, 1.2))

bench.closeup (1, 32)

# Readings to take with a multimeter while the fan blows at a book.
bench.measure ("The enable pin, a book at 70 cm", red="4", black="B-21", expect="about 2.6 V",
               when="blowing")
bench.measure ("The enable pin, a book at 40 cm", red="4", black="B-21", expect="about 3.9 V",
               when="blowing")
