# The LCD lies off the bottom edge with its pins in row a, from column 9.
# The top rails carry 5 V and GND, taken down to the LCD's power pins; the
# contrast knob sits just before it, joined to its first three pins; and the
# Mega's pins 31-36 reach its signal pins. The clock module sits above the
# Mega, on the I2C pins 20 and 21.
bench = Bench ("A clock module on pins 20 and 21, and the LCD on pins 31 to 36", columns=(1, 30))

bench.wire ("5V", "T+3")
bench.wire ("GND", "T-3")
bench.potentiometer ("e5", "e6", "e7")
bench.lcd (9, row="a", text=("Date  2026-09-24", "Time    20:30:05"))
bench.wire ("d5", "d9")
bench.wire ("c6", "c11")
bench.wire ("b7", "b10")
bench.wire ("e9", "T-9")
bench.wire ("e10", "T+10")
bench.wire ("e13", "T-13")
bench.wire ("31", "e12")
bench.wire ("32", "e14")
bench.wire ("33", "e19")
bench.wire ("34", "e20")
bench.wire ("35", "e21")
bench.wire ("36", "e22")
bench.resistor ("220 Ω", "e23", "f23")
bench.wire ("j23", "T+23")
bench.wire ("e24", "T-24")
bench.module ("rtc", at=(3.15, -1.25))
bench.wire ("GND", "rtc.GND")
bench.wire ("5V", "rtc.VCC")
bench.wire ("20", "rtc.SDA", color="green")
bench.wire ("21", "rtc.SCL", color="blue")
