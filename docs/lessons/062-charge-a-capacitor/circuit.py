# USB 5 V reaches the capacitor only through 10 kΩ. The capacitor's
# striped − leg stays on GND while its + leg charges in column 8.
bench = Bench ("A 1000 µF capacitor charging through 10 kΩ from 5 V",
               columns=(1, 16))

bench.stage ("the charging path and capacitor")
bench.wire ("T+6", "j6", color="red")
bench.resistor ("10 kΩ", "g6", "e6")
bench.wire ("b6", "b8")
bench.capacitor ("1000 µF", "a8", "B-9", polarized=True)

bench.measure ("Across the capacitor", red="d8", black="GND",
               expect="rises toward 5 V, then falls during discharge",
               when="After plugging in USB, then after moving the red rail end to −")
