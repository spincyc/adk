# Lesson 2's build stays as it is: the buttons on 22 and 23 across the middle
# gap in columns 2-4 and 8-10, the red LED on 26 in column 6 and the yellow
# LED on 27 in column 12. The green LED on 28 joins them at its home in
# column 18, and the active buzzer on 12 stands across the gap at its home in
# column 34, its − leg returning to the bottom − rail by a black jumper. The
# LEDs' wires reach row j from nested lanes just below it; pin 12's comes
# over the top of the board.
bench = Bench ("A two-player reaction game: buttons on 22 and 23, LEDs on 26 to 28, "
               "and an active buzzer on 12", columns=(1, 40))

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("23", "j8")
bench.button (8)
bench.wire ("a10", "B-10")

bench.wire ("26", "j6", via=[(4.45, 1.0), (4.45, 1.15)])
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.wire ("27", "j12", via=[(4.35, 1.05), (4.35, 1.25)])
bench.resistor ("220 Ω", "g12", "e12")
bench.led ("yellow", anode="b12", cathode="b13")
bench.wire ("a13", "B-13")

bench.wire ("28", "j18", via=[(4.25, 1.2), (4.25, 1.35), (7.05, 1.35)])
bench.resistor ("220 Ω", "g18", "e18")
bench.led ("green", anode="b18", cathode="b19")
bench.wire ("a19", "B-19")

bench.wire ("12", "j34", via=[(1.7, 0.45), (8.65, 0.45)])
bench.buzzer ("f34", "e34", kind="active")
bench.wire ("a34", "B-34")
