# Lesson 32's clock, unchanged: the screen at its home and the clock module
# on its side above the board, SDA and SCL now stepping right round the
# knob. The knob, the rotary encoder, sits at its home above the Mega: CLK
# and DT from 18 and 19, SW and + from the long header (22 and its inner
# 5V), GND from the GND beside pin 13. The snooze button and the passive
# buzzer take their homes beside the screen, in columns 38 and 51, their pin
# wires coming over the top. The flag is Lesson 31's stepper, its driver
# below the Mega as it was there; the power module at the right end feeds
# only the bottom rails, for the driver, its top jumper off, so the screen
# and clock keep the Mega's 5V on the top rails. The screen's knob jumper
# takes column 5 and the LCD covers the bottom rails from column 6, so the
# driver's + and − come in one column nearer the Mega than in Lesson 31, at
# B+4 and B-4.
bench = Bench ("Lesson 32's clock and LCD, with a knob on pins 18, 19 and 22, a snooze button on "
               "pin 23, a passive buzzer on pin 10, and a stepper flag on pins A8 to A11 powered "
               "from the power module", columns=(1, 63))

bench.screen (text=("Time    06:58:30", "Alarm   07:00   "), risers=(4.55, 0.1))
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

bench.wire ("10", "j51", via=[(1.9, -2.4), (10.4, -2.4)])
bench.buzzer ("f51", "e51", kind="passive")
bench.resistor ("220 Ω", "a51", "B-51")

bench.power_module ("right", top="off", bottom="5V")
bench.module ("stepper", at=(3.1, 4.5), facing="up")
# The IN wires cross in Lesson 31's staircase; the + wire rises between
# B-3 and B-4 to reach B+4.
bench.wire ("A8", "stepper.IN1", via=[(3.09, 3.5), (4.45, 3.5)])
bench.wire ("A9", "stepper.IN2", via=[(3.19, 3.3), (4.35, 3.3)])
bench.wire ("A10", "stepper.IN3", via=[(3.29, 3.1), (4.25, 3.1)])
bench.wire ("A11", "stepper.IN4", via=[(3.39, 2.9), (4.15, 2.9)])
bench.wire ("stepper.+", "B+4", via=[(3.56, 3.65), (5.65, 3.65), (5.65, 2.5)])
bench.wire ("stepper.−", "B-4", via=[(3.66, 3.75), (5.7, 3.75)])

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The buzzer's pin while it rings", red="10", black="GND", expect="about 2.4 V",
               when="While a note plays")
bench.measure ("The flag's supply, on the bottom rails", red="B+57", black="B-57",
               expect="about 5 V", when="Power module on")
