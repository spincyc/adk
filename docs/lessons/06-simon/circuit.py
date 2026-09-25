# Lesson 5's build stays as it is: the buttons on 22 to 25 across the middle
# gap in columns 2-4, 8-10, 14-16 and 20-22, and the passive buzzer on 10 in
# column 34 with its 220 Ω resistor into the bottom − rail. The LEDs on 26 to
# 29 stand at their homes in columns 6, 12, 18 and 24, each beside its button.
# Eight wires from the double header into interleaved columns can't all nest,
# so the buttons' wires fan out over the top of the board and drop straight
# into their columns through the gaps in the top rails, crossing the lanes of
# the ones going further; 26 comes in just above row j, and 27 to 29 in
# nested lanes just below it.
bench = Bench ("Simon: buttons on pins 22 to 25, LEDs on 26 to 29, and a passive buzzer on 10",
               columns=(1, 40))

bench.wire ("22", "j2", via=[(4.15, 0.8), (4.25, 0.8), (4.25, 0.15), (5.5, 0.15)])
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("23", "j8", via=[(4.35, 0.85), (4.35, 0.25), (6.1, 0.25)])
bench.button (8)
bench.wire ("a10", "B-10")

bench.wire ("24", "j14", via=[(4.15, 0.9), (4.45, 0.9), (4.45, 0.35), (6.7, 0.35)])
bench.button (14)
bench.wire ("a16", "B-16")

bench.wire ("25", "j20", via=[(4.55, 0.95), (4.55, 0.45), (7.3, 0.45)])
bench.button (20)
bench.wire ("a22", "B-22")

bench.wire ("26", "j6", via=[(4.15, 1.0), (4.65, 1.0), (4.65, 0.9), (5.9, 0.9)])
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.wire ("27", "j12", via=[(4.45, 1.05), (4.45, 1.15), (6.4, 1.15)])
bench.resistor ("220 Ω", "g12", "e12")
bench.led ("yellow", anode="b12", cathode="b13")
bench.wire ("a13", "B-13")

bench.wire ("28", "j18", via=[(4.15, 1.1), (4.35, 1.1), (4.35, 1.25), (7.0, 1.25)])
bench.resistor ("220 Ω", "g18", "e18")
bench.led ("green", anode="b18", cathode="b19")
bench.wire ("a19", "B-19")

bench.wire ("29", "j24", via=[(4.25, 1.15), (4.25, 1.35), (7.6, 1.35)])
bench.resistor ("220 Ω", "g24", "e24")
bench.led ("blue", anode="b24", cathode="b25")
bench.wire ("a25", "B-25")

bench.wire ("10", "j34", via=[(1.9, 0.05), (8.65, 0.05)])
bench.buzzer ("f34", "e34", kind="passive")
bench.resistor ("220 Ω", "a34", "B-34")
