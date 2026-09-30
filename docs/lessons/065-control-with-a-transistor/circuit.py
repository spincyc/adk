# The red LED and button keep their breadboard positions. The S8050 stays
# in Lesson 3's E–B–C holes. USB 5 V feeds two separate paths: through the
# LED to the collector, and through the button and 1 kΩ to the base.
bench = Bench ("A button switches a red LED through an S8050 transistor",
               columns=(1, 36))

bench.stage ("the red LED and its 220 Ω resistor")
bench.wire ("T+6", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "b31")

bench.stage ("the button")
bench.wire ("T+4", "j2")
bench.button (2)
bench.wire ("a4", "a32")

bench.stage ("the S8050 transistor and its base resistors")
bench.transistor ("a29", "a30", "a31")
bench.resistor ("1 kΩ", "c32", "c30")
bench.resistor ("10 kΩ", "b30", "B-30")
bench.wire ("b29", "B-29")

bench.closeup (1, 34)
