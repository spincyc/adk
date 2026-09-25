# Laid out from the end of the breadboard nearest the Mega: the power module on the rails there,
# then the knob (5 V from the Mega, A0, and GND), the reverse button, and the L293D. The chip's
# two halves are alike, and its upper half drives the motor, so the wires from pins 8, 9 and 4
# come straight down onto its top row and the motor's leads come up from below.
bench = Bench ("A DC motor on an L293D (enable 4, forward 8, backward 9), powered from the "
               "breadboard power module, with a knob on A0 and a button on 22", columns=(1, 30))

bench.power_module ("left")
bench.wire ("GND", "B-6")

bench.potentiometer ("e7", "e9", "e11")
bench.wire ("5V", "c7")
bench.wire ("A0", "a9")
bench.wire ("a11", "B-11")

bench.button (13)
bench.wire ("22", "j13")
bench.wire ("a15", "B-15")

bench.chip ("L293D", first=17)
bench.wire ("j17", "T+17")
bench.wire ("j21", "T-21")
bench.wire ("a24", "B+24")
bench.wire ("8", "j18")
bench.wire ("9", "j23")
bench.wire ("4", "j24")
bench.module ("motor", name="motor", at=(4.6, 3.15), facing="down")
bench.wire ("motor.+", "g19")
bench.wire ("motor.-", "g22")
