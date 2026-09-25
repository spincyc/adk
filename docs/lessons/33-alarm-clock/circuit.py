# Lesson 32's clock, unchanged: the screen at its home and the clock module
# on its side above the board, SDA and SCL now stepping right round the
# knob. The knob, the rotary encoder, sits at its home above the Mega: CLK
# and DT from 18 and 19, SW and + from the long header (22 and its inner
# 5V), GND from the GND beside pin 13. The snooze button and the passive
# buzzer take their homes beside the screen, in columns 38 and 51, their pin
# wires coming over the top.
bench = Bench ("Lesson 32's clock and LCD, with a knob on pins 18, 19 and 22, a snooze button on "
               "pin 23 and a passive buzzer on pin 10", columns=(1, 55))

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
