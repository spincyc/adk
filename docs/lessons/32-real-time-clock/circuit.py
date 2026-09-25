# The course's screen at its home, as in Lesson 13. The clock module lies on
# its side above the board, clear of the screen's signal wires, where it
# stays in Lesson 33, whose knob sits above the Mega: GND and VCC drop
# straight into T-29 and T+30, and SDA and SCL come over the top from pins
# 20 and 21 and in from the module's right.
bench = Bench ("A clock module on pins 20 and 21, and the LCD on pins 31 to 36", columns=(1, 35))

bench.screen (text=("Date  2026-09-24", "Time    20:30:05"))
bench.module ("rtc", at=(6.5, -1.4), facing="right")
bench.wire ("rtc.GND", "T-29")
bench.wire ("rtc.VCC", "T+30")
bench.wire ("20", "rtc.SDA", via=[(3.75, -1.6), (8.5, -1.6), (8.5, -0.8)])
bench.wire ("21", "rtc.SCL", via=[(3.85, -1.5), (8.4, -1.5), (8.4, -0.9)])

# Readings to take with a multimeter: the top rails, which carry the
# Mega's 5 V to the clock and the screen, plugged in and then unplugged. The
# probes go at the Mega's end, where the meter lies clear of the LCD.
bench.measure ("The clock's supply, on the top rails", red="T+4", black="T-4",
               expect="about 5 V", when="Mega plugged in")
bench.measure ("The clock's supply, unplugged", red="T+4", black="T-4", expect="0 V",
               when="USB cable out")
