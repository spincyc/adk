# The power module plugs into both pairs of rails at the end nearest the
# Mega, and one GND wire joins the Mega to them. Then the knob, in the first
# free columns, fed from the bottom rails with its wiper on A0; the servo
# lies above the board, its power from the top rails and its signal from
# pin 44.
bench = Bench ("A servo on pin 44 powered by the breadboard power module, and a knob on A0",
               columns=(1, 30))

bench.power_module ("left")
bench.wire ("GND", "B-6")

bench.potentiometer ("e7", "e8", "e9")
bench.wire ("a7", "B+7")
bench.wire ("A0", "a8")
bench.wire ("a9", "B-9")

bench.module ("servo", "servo", at=(8.3, -1.7))
bench.wire ("servo.−", "T-23")
bench.wire ("servo.+", "T+24")
bench.wire ("44", "servo.signal")
