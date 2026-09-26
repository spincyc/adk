# The screen at its home, as in Lesson 13, and the rotary encoder at its home
# above the Mega, wired as in Lesson 29. The FM radio stands in row j,
# columns 45 to 52, its board over the top rails and its headphone socket
# to the left. Its wires come up from below, round the bottom of the
# screen: pins 42, 41 and 40 into RST, SCLK and SDIO, and the Mega's 3.3V
# into the radio's 3.3V column, 52. The 1 kΩ from RST to 3.3 V lies along
# row h, and a black jumper takes GND down to the bottom − rail. The volume
# knob stands at its home beside the screen, e57 to e59, A0's wire coming
# round the bottom of the screen too.
bench = Bench ("An FM radio on pins 40, 41 and 42, the rotary encoder on 18, 19 and 22, a volume "
               "knob on A0, and the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=(" 98.8 MHz Stereo", "CITY FM  ████"))

bench.module ("encoder", at=(3.0, -2.1))
bench.wire ("18", "encoder.CLK", via=[(3.55, 0.55), (3.2, 0.55)])
bench.wire ("19", "encoder.DT", via=[(3.65, 0.45), (3.3, 0.45)])
bench.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.35), (3.4, 0.35)])
bench.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, 0.25), (3.5, 0.25)])
bench.wire ("GND.top", "encoder.GND", via=[(1.5, 0.15), (3.6, 0.15)])

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

# Readings to take with a multimeter: the data line resting at the radio's
# 3.3 V, RST held up by the 1 kΩ against the board's own 10 kΩ, and the
# supply the Mega's 3.3V pin gives it.
bench.measure ("SDIO, resting between messages", red="40", black="GND", expect="about 3.3 V",
               when="sketch running")
bench.measure ("RST, lifted by the 1 kΩ", red="42", black="GND", expect="about 3.0 V",
               when="sketch running")
bench.measure ("The radio's supply", red="3.3V", black="GND", expect="about 3.3 V",
               when="any time")
