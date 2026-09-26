# Lesson 38's screen and button stay where they were; the 433 MHz modules
# go. The clock module comes back to its home from Lessons 32 and 33, on its
# side above the board, and the rotary encoder to its home above the Mega,
# wired as in Lesson 33. The FM radio and the volume knob come back to
# their homes from Lesson 37: the radio standing in row j, columns 45 to
# 52, with its wires coming up from below round the bottom of the screen,
# and the knob in e57 to e59.
bench = Bench ("A clock radio: the FM radio on pins 40 to 42, the clock module on 20 and 21, the "
               "rotary encoder on 18, 19 and 22, a button on 23, a volume knob on A0, and the LCD "
               "on pins 31 to 36", columns=(1, 62))

bench.screen (text=("06:58:30  \u237e07:00", "CITY FM     98.8"), risers=(4.55, 0.1))
bench.module ("rtc", at=(6.5, -1.4), facing="right")
bench.wire ("rtc.GND", "T-29")
bench.wire ("rtc.VCC", "T+30")
bench.wire ("20", "rtc.SDA", via=[(3.75, 0.35), (3.95, 0.35), (3.95, -1.6), (8.5, -1.6), (8.5, -0.8)])
bench.wire ("21", "rtc.SCL", via=[(3.85, 0.45), (4.05, 0.45), (4.05, -1.5), (8.4, -1.5), (8.4, -0.9)])

bench.module ("encoder", at=(3.0, -2.1))
bench.wire ("18", "encoder.CLK", via=[(3.55, 0.25), (3.2, 0.25)])
bench.wire ("19", "encoder.DT", via=[(3.65, 0.15), (3.3, 0.15)])
bench.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.05), (3.4, 0.05)])
bench.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, -0.05), (3.5, -0.05)])
bench.wire ("GND.top", "encoder.GND", via=[(1.5, -0.15), (3.6, -0.15)])

bench.wire ("23", "j38", via=[(4.4, 0.85), (4.4, -1.7), (9.1, -1.7)])
bench.button (38)
bench.wire ("a40", "B-40")

bench.header_module ("fm_radio", first=45, row="j")
bench.wire ("42", "f47", via=[(4.45, 1.85), (4.45, 3.65), (10.0, 3.65)])
bench.wire ("41", "f49", via=[(4.55, 1.75), (4.55, 3.75), (10.2, 3.75)])
bench.wire ("40", "f50", via=[(4.25, 1.7), (4.65, 1.7), (4.65, 3.85), (10.3, 3.85)])
bench.wire ("3.3V", "f52", via=[(1.59, 3.95), (10.5, 3.95)])
bench.wire ("f51", "B-51")
bench.resistor ("1 kΩ", "h47", "h52")

bench.potentiometer ("e57", "e58", "e59")
bench.wire ("a57", "B-57")
bench.wire ("A0", "a58", via=[(2.19, 4.05), (11.1, 4.05)])
bench.wire ("d59", "T+61")

bench.closeup (23, 62)

# Readings to take with a multimeter: the volume knob's wiper, which the
# sketch turns into the radio's volume, halfway and a quarter of the way.
bench.measure ("The volume knob halfway, on A0", red="A0", black="GND", expect="about 2.5 V",
               when="volume knob halfway")
bench.measure ("The volume knob a quarter up, on A0", red="A0", black="GND",
               expect="about 1.25 V", when="volume knob a quarter of the way")
