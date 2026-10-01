# The red LED, passive buzzer and S8050 keep their home positions. USB 5 V
# feeds the LED through 1 kΩ and the buzzer through 220 Ω in separate
# branches. The diode bridges only the buzzer's own two nodes.
bench = Bench ("A button switches a red LED and passive buzzer through an S8050, "
               "with a flyback diode",
               columns=(1, 40))

bench.stage ("the red LED and its 1 kΩ resistor")
bench.wire ("T+6", "j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "b33")

bench.stage ("the passive buzzer and its 220 Ω resistor")
bench.wire ("T+36", "j36")
bench.resistor ("220 Ω", "g36", "e36")
bench.wire ("a36", "j33", color="red")
bench.buzzer ("f33", "e33", kind="passive")

bench.stage ("the flyback diode")
bench.diode (anode="c33", cathode="c36")

bench.stage ("the S8050 transistor")
bench.transistor ("a29", "a30", "a31")
bench.wire ("a33", "b31", color="black")
bench.wire ("b29", "B-29")

bench.stage ("the button and base resistors")
bench.wire ("T+4", "j2")
bench.button (2)
bench.wire ("a4", "a32")
bench.resistor ("1 kΩ", "c32", "c30")
bench.resistor ("10 kΩ", "b30", "B-30")

bench.measure ("Collector to GND, button held", red="e31", black="GND",
               expect="about 0.1 V", when="Hold the button down")
bench.measure ("Collector to GND, button released", red="e31", black="GND",
               expect="about 5 V", when="Button released")
