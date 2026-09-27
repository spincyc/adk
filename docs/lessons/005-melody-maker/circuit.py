# The button on 22 stays at its home across the middle gap in columns 2-4,
# with its wires, as in Lessons 2 to 4. The buttons on 23, 24 and 25 join it
# at their homes in columns 8-10, 14-16 and 20-22, and the passive buzzer on
# 10 stands across the gap at its home in column 34, its 220 Ω resistor from
# a34 down into the bottom − rail. Pin 22's wire reaches row j from above,
# 23 to 25 from nested lanes just below it, and pin 10's over the top of the
# board, so no two cross.
bench = Bench ("A four-key keyboard: buttons on pins 22 to 25, and a passive buzzer on pin 10 "
               "through 220 Ω", columns=(1, 40))

bench.home_button ("22", via=[(4.15, 0.8), (5.5, 0.8)])

bench.home_button ("23", via=[(4.65, 0.85), (4.65, 1.15), (5.95, 1.15)])

bench.home_button ("24", via=[(4.15, 0.9), (4.55, 0.9), (4.55, 1.25), (6.55, 1.25)])

bench.home_button ("25", via=[(4.45, 0.95), (4.45, 1.35), (7.15, 1.35)])

bench.home_buzzer ("passive", via=[(1.9, 0.45), (8.65, 0.45)])

# Readings to take with a multimeter while a key is held and its note
# sounds: pin 10 switches between 5 V and 0 V, so the meter shows about half,
# which the resistor and the buzzer's 16 Ω coil share in proportion.
bench.measure ("Pin 10, playing a note", red="10", black="GND", expect="a little under 2.5 V",
               when="Holding a key")
bench.measure ("Across the buzzer", red="i34", black="b34", expect="about 0.15 V",
               when="Holding a key")
bench.measure ("Across the resistor", red="b34", black="GND", expect="about 2.1 V",
               when="Holding a key")
