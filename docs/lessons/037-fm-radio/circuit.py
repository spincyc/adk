# The screen at its home, as in Lesson 13, and the rotary encoder at its home
# above the Mega, wired as in Lesson 29. The FM radio stands at its home,
# its headphone socket to the left, on pins 42, 41 and 40 (RST, SCLK and
# SDIO) and the Mega's 3.3V. The volume knob stands at its home past the
# radio, across the middle gap, A0's wire coming round the bottom of the
# screen as the radio's wires do.
bench = Bench ("An FM radio on pins 40, 41 and 42, the rotary encoder on 18, 19 and 22, a volume "
               "knob on A0, and the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=(" 98.8 MHz Stereo", "CITY FM  ████"))

bench.home_encoder (above=True)

bench.home_fm_radio ()

bench.home_knob (via=[(2.19, 4.05), (11.1, 4.05)])

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
