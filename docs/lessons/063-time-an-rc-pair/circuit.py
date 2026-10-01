# Keep E07's supply wire, first resistor, polarized capacitor and the link
# that makes the top − rail GND.
# The new resistor lies between the first resistor and the capacitor.
bench = Bench ("Two 10 kΩ resistors charge a 1000 µF capacitor from USB 5 V",
               columns=(1, 14))

bench.stage ("the RC charging path")
bench.wire ("T+6", "j6", color="red")
bench.resistor ("10 kΩ", "g6", "e6")
bench.wire ("b6", "j8")
bench.resistor ("10 kΩ", "g8", "e8")
bench.capacitor ("1000 µF", "a8", "B-9", polarized=True)

bench.stage ("the link to the top − rail")
bench.wire ("B-11", "T-11")

bench.measure ("Capacitor voltage after about 20 seconds", red="d8", black="GND",
               expect="about 3.2 V", when="charging from near 0 V")
