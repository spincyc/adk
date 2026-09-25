# From the end nearest the Mega: the button on pin 22 at its home (columns
# 2-4); a black jumper joins the top − rail to the bottom one in column 6;
# the 74HC595 in e18-e25/f18-f25, VCC and MR to the top + rail, OE to the
# top − rail, GND to the bottom one, pins 37, 39 and 38 dropping into its
# data, latch and clock columns. Each output then meets its own 1 kΩ: Q0 (a)
# runs along row h to one lying in row i; Q5, Q1 and Q6 (f, b, g) cross the
# gap through resistors standing in columns 27, 28 and 30; Q4, Q3 and Q2
# (e, d, c) step up over the gap to three standing just before the digit,
# whose lower legs drop into its bottom pins. Lesson 11 keeps all of this
# but the digit and its wires.
X = lambda column: round (5.30 + 0.1 * column, 3)    # inches, as bench.py draws
Y = lambda row: round (0.55 + row, 3)               # row offsets from the board's top

bench = Bench ("A button on pin 22, and a digit behind a 74HC595 on pins 37, 38 and 39",
               columns=(1, 52))

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("T-6", "B-6")

bench.chip ("74HC595", first=18)
bench.wire ("j18", "T+18")
bench.wire ("37", "j20", via=[(4.95, 1.55), (4.95, 0.45), (X (20), 0.45)])
bench.wire ("j21", "T-21")
bench.wire ("39", "j22", via=[(5.05, 1.65), (5.05, 0.40), (X (22), 0.40)])
bench.wire ("38", "j23", via=[(4.20, 1.60), (5.00, 1.60), (5.00, 0.35), (X (23), 0.35)])
bench.wire ("j24", "T+24")
bench.wire ("a25", "B-25")

bench.digit (46, shows="5")

# Q0, segment a: along row h to its resistor in row i, then over to the top.
bench.wire ("h19", "h26")
bench.resistor ("1 kΩ", "i26", "i29")
bench.wire ("j29", "j49", via=[(X (29), Y (0.40)), (X (49), Y (0.40))])

# Q5, Q1 and Q6 (f, b, g) cross the middle gap through their resistors.
bench.wire ("d22", "d27")
bench.resistor ("1 kΩ", "g27", "e27")
bench.wire ("h27", "j47", via=[(X (27), Y (0.70)), (X (29.5), Y (0.70)), (X (29.5), Y (0.45)),
                               (X (47), Y (0.45))])
bench.wire ("d18", "d28", via=[(X (18), Y (1.40)), (X (28), Y (1.40))])
bench.resistor ("1 kΩ", "g28", "e28")
bench.wire ("h28", "i50", via=[(X (28), Y (0.80)), (X (30.5), Y (0.80)), (X (30.5), Y (0.60)),
                               (X (50), Y (0.60))])
bench.wire ("c23", "d30", via=[(X (30), Y (1.45))])
bench.resistor ("1 kΩ", "g30", "e30")
bench.wire ("h30", "j46", via=[(X (30), Y (0.50)), (X (46), Y (0.50))])

# Q4, Q3 and Q2 (e, d, c) step up over the gap to their resistors.
bench.wire ("c21", "h41", via=[(X (21), Y (1.50)), (X (31), Y (1.50)), (X (31), Y (0.65)),
                               (X (41), Y (0.65))])
bench.wire ("b20", "h40", via=[(X (31.5), Y (1.55)), (X (31.5), Y (0.70)), (X (40), Y (0.70))])
bench.wire ("b19", "h39", via=[(X (19), Y (1.60)), (X (32), Y (1.60)), (X (32), Y (0.75))])
bench.resistor ("1 kΩ", "g39", "e39")
bench.resistor ("1 kΩ", "g40", "e40")
bench.resistor ("1 kΩ", "g41", "e41")

# Into the digit's bottom pins, nested, and its common back to the − rail.
bench.wire ("a39", "a49", via=[(X (39), Y (1.80)), (X (49), Y (1.80))])
bench.wire ("a40", "a47", via=[(X (40), Y (1.75)), (X (47), Y (1.75))])
bench.wire ("a41", "a46", via=[(X (41), Y (1.70)), (X (46), Y (1.70))])
bench.wire ("b48", "B-37", via=[(X (48), Y (1.60)), (X (37), Y (1.60))])

# Readings to take with a multimeter while the digit shows its dash, the
# byte 0b01000000: Q6 (g) high, Q5 (f) low, and g's resistor carrying its
# segment's current.
bench.measure ("Q6, segment g's output", red="a23", black="GND", expect="about 5 V",
               when="The dash showing")
bench.measure ("Q5, segment f's output", red="a22", black="GND", expect="0 V",
               when="The dash showing")
bench.measure ("Across segment g's resistor", red="e30", black="g30", expect="about 3 V",
               when="The dash showing")
