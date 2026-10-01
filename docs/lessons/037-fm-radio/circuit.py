# The screen and rotary encoder keep their homes from Lesson 29. The
# FM radio stands in row j, columns 27 to 34, on pins 42, 41 and 40,
# and takes the Mega's 3.3 V. The volume knob takes its usual home in
# columns 39 to 41, between the radio and the screen's contrast knob.
bench = Bench ("An FM radio on pins 40, 41 and 42, the rotary encoder on 18, 19 and 22, a volume "
               "knob on A0, and the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=(" 98.8 MHz Stereo", "CITY FM  ████"))

bench.home_encoder ()

bench.home_fm_radio ()

bench.home_knob ()

# Readings to take with a multimeter: the data line resting at the radio's
# 3.3 V, RST held up by the 1 kΩ against the board's own 10 kΩ, and the
# supply the Mega's 3.3V pin gives it.
bench.measure ("SDIO, resting between messages", red="40", black="GND", expect="about 3.3 V",
               when="sketch running")
bench.measure ("RST, lifted by the 1 kΩ", red="42", black="GND", expect="about 3.0 V",
               when="sketch running")
bench.measure ("The radio's supply", red="3.3V", black="GND", expect="about 3.3 V",
               when="any time")
