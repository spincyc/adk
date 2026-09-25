# From the end nearest the Mega: a black jumper joins the top − rail to the
# bottom one in column 6 (the display covers the usual link at 60); the
# 74HC595 in e18-e25/f18-f25, VCC and MR to the top + rail, OE to the top −
# rail, GND to the bottom one, pins 37, 39 and 38 dropping into its data,
# latch and clock columns. Each output then meets its own 1 kΩ: Q0 (a) runs
# along row h to one lying in row i above two that stand across the gap for
# Q5 (f) and Q1 (b); Q7, Q4, Q3 and Q2 (dp, e, d, c) step up over the gap
# to a ladder of four standing across it just before the display, whose
# lower legs drop into the display's bottom pins; Q6 (g) keeps below, its
# resistor lying in row b. The display's digit pins take pins 40 to 43.
X = lambda column: round (5.30 + 0.1 * column, 3)    # inches, as bench.py draws
Y = lambda row: round (0.55 + row, 3)               # row offsets from the board's top

bench = Bench ("A four-digit display behind a 74HC595 on pins 37 to 39, its digits on pins 40 to 43",
               columns=(1, 63))

bench.wire ("T-6", "B-6")

bench.chip ("74HC595", first=18)
bench.wire ("j18", "T+18")
bench.wire ("37", "j20", via=[(4.95, 1.55), (4.95, 0.45), (X (20), 0.45)])
bench.wire ("j21", "T-21")
bench.wire ("39", "j22", via=[(5.05, 1.65), (5.05, 0.40), (X (22), 0.40)])
bench.wire ("38", "j23", via=[(4.20, 1.60), (5.00, 1.60), (5.00, 0.35), (X (23), 0.35)])
bench.wire ("j24", "T+24")
bench.wire ("a25", "B-25")

bench.four_digits (51, shows="12.34")

# Q0, segment a: along row h to its resistor in row i, then over to the top.
bench.wire ("h19", "h26")
bench.resistor ("1 kΩ", "i26", "i29")
bench.wire ("j29", "j52", via=[(X (29), Y (0.45)), (X (52), Y (0.45))])

# Q5 and Q1, segments f and b, cross the middle gap through their resistors.
bench.wire ("d22", "d27")
bench.resistor ("1 kΩ", "g27", "e27")
bench.wire ("h27", "j53", via=[(X (27), Y (0.70)), (X (29.5), Y (0.70)), (X (29.5), Y (0.40)),
                               (X (53), Y (0.40))])
bench.wire ("d18", "d28", via=[(X (18), Y (1.40)), (X (28), Y (1.40))])
bench.resistor ("1 kΩ", "g28", "e28")
bench.wire ("h28", "i56", via=[(X (30), Y (0.75)), (X (30), Y (0.50)), (X (43), Y (0.50)),
                               (X (43), Y (0.60)), (X (56), Y (0.60))])

# Q7, Q4, Q3 and Q2 (dp, e, d, c) step up over the gap to the ladder.
bench.wire ("c24", "h42", via=[(X (30.5), Y (1.45)), (X (30.5), Y (0.60)), (X (42), Y (0.60))])
bench.wire ("c21", "h41", via=[(X (21), Y (1.50)), (X (31), Y (1.50)), (X (31), Y (0.65)),
                               (X (41), Y (0.65))])
bench.wire ("b20", "h40", via=[(X (31.5), Y (1.55)), (X (31.5), Y (0.70)), (X (40), Y (0.70))])
bench.wire ("b19", "h39", via=[(X (19), Y (1.60)), (X (32), Y (1.60)), (X (32), Y (0.75))])
bench.resistor ("1 kΩ", "g39", "e39")
bench.resistor ("1 kΩ", "g40", "e40")
bench.resistor ("1 kΩ", "g41", "e41")
bench.resistor ("1 kΩ", "g42", "e42")

# Q6, segment g, stays below the gap.
bench.wire ("a23", "a33", via=[(X (23), Y (1.70)), (X (33), Y (1.70))])
bench.resistor ("1 kΩ", "b33", "b36")

# Into the display's bottom pins, nested so none crosses another.
bench.wire ("a36", "a55", via=[(X (36), Y (1.85)), (X (55), Y (1.85))])
bench.wire ("a39", "a54", via=[(X (39), Y (1.80)), (X (54), Y (1.80))])
bench.wire ("a40", "a52", via=[(X (40), Y (1.75)), (X (52), Y (1.75))])
bench.wire ("a41", "a51", via=[(X (41), Y (1.70)), (X (51), Y (1.70))])
bench.wire ("b42", "b53", via=[(X (42), Y (1.60)), (X (53), Y (1.60))])

# The digit pins, straight from the Mega.
bench.wire ("40", "j51", via=[(4.20, 1.70), (4.90, 1.70), (4.90, 0.30), (X (51), 0.30)])
bench.wire ("41", "j54", via=[(4.85, 1.75), (4.85, 0.25), (X (54), 0.25)])
bench.wire ("42", "j55", via=[(4.20, 1.80), (4.80, 1.80), (4.80, 0.20), (X (55), 0.20)])
bench.wire ("43", "a56")
