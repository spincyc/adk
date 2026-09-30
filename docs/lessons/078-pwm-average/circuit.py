# Keep Lesson 7's white LED and knob at their homes. The filter branches
# from pin 3's upper strip; its capacitor is only on the resistor's far side.
bench = Bench ("A white PWM LED and a separate RC branch from pin 3",
               columns=(1, 50))

bench.home_led ("3", "white", via=[(2.55, 0.45), (9.10, 0.45)])
bench.home_knob ()

bench.stage ("the PWM filter and capacitor")
bench.wire ("h38", "j44")
bench.resistor ("10 kΩ", "g44", "e44")
bench.capacitor ("100 µF", "a44", "B-45", polarized=True)

bench.measure ("Pin 3 before the filter", red="h38", black="GND",
               expect="about 1.25 V", when="Knob reading about 256")
bench.measure ("Filtered point at the capacitor", red="c44", black="GND",
               expect="about 1.25 V", when="Knob reading about 256; wait 3 seconds")
