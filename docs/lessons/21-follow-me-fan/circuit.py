# Laid out like Lesson 20 from the end nearest the Mega: the power module, the servo's plug on the
# rails with pin 44 beside it, then the L293D, its upper half driving the fan. The sensor sits
# above the Mega with Trig and Echo straight over pins 14 and 15; in the finished fan, the sensor
# and the motor ride together on the servo's horn.
bench = Bench ("A servo on pin 44 carrying an ultrasonic sensor on 14 and 15 and a fan on an "
               "L293D (4, 8, 9), the servo and the fan powered from the breadboard power module",
               columns=(1, 30))

bench.power_module ("left")
bench.wire ("GND", "B-6")

bench.module ("ultrasonic", name="sensor", at=(2.315, -1.26))
bench.wire ("sensor.VCC", "T+6")
bench.wire ("sensor.GND", "T-7")
bench.wire ("14", "sensor.Trig")
bench.wire ("15", "sensor.Echo")

bench.module ("servo", name="servo", at=(5.3, 3.2), facing="up")
bench.wire ("servo.-", "B-7")
bench.wire ("servo.+", "B+7")
bench.wire ("servo.signal", "a8")
bench.wire ("44", "c8")

bench.chip ("L293D", first=11)
bench.wire ("j11", "T+11")
bench.wire ("j15", "T-15")
bench.wire ("a18", "B+18")
bench.wire ("8", "j12")
bench.wire ("9", "j17")
bench.wire ("4", "j18")
bench.module ("motor", name="motor", at=(8.3, 3.2), facing="up")
bench.wire ("motor.+", "g13")
bench.wire ("motor.-", "g16")
bench.note ("tape the sensor and the fan to the horn", (6.32, 4.43), offset=(1.2, 0.35))
