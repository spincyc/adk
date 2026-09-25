# The button on 22 stays at its home across the middle gap in columns 2-4,
# with its wires, as in Lessons 2 to 4. The buttons on 23, 24 and 25 join it
# at their homes in columns 8-10, 14-16 and 20-22, and the passive buzzer on
# 10 stands across the gap at its home in column 34, its 220 Ω resistor from
# a34 down into the bottom − rail. Pin 22's wire reaches row j from above,
# 23 to 25 from nested lanes just below it, and pin 10's over the top of the
# board, so no two cross.
bench = Bench ("A four-key keyboard: buttons on pins 22 to 25, and a passive buzzer on pin 10 "
               "through 220 Ω", columns=(1, 40))

bench.wire ("22", "j2", via=[(4.15, 0.8), (5.5, 0.8)])
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("23", "j8", via=[(4.65, 0.85), (4.65, 1.15), (5.95, 1.15)])
bench.button (8)
bench.wire ("a10", "B-10")

bench.wire ("24", "j14", via=[(4.15, 0.9), (4.55, 0.9), (4.55, 1.25), (6.55, 1.25)])
bench.button (14)
bench.wire ("a16", "B-16")

bench.wire ("25", "j20", via=[(4.45, 0.95), (4.45, 1.35), (7.15, 1.35)])
bench.button (20)
bench.wire ("a22", "B-22")

bench.wire ("10", "j34", via=[(1.9, 0.45), (8.65, 0.45)])
bench.buzzer ("f34", "e34", kind="passive")
bench.resistor ("220 Ω", "a34", "B-34")
