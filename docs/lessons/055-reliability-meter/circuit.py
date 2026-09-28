# The screen at its home, its contrast knob across the middle gap, and the
# rotary encoder at its home, wired as in Lesson 29, on 18 and 19 with its
# switch on 22. The 433 MHz receiver on pin 43 and transmitter on pin 46
# stand at their homes from Lesson 38.
bench = Bench ("A 433 MHz receiver on pin 43 and transmitter on pin 46, the rotary encoder on 18 "
               "and 19 with its switch on 22, and the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=("20 letters 186ms", "Heard 5/5 100%"))

bench.home_encoder ()

bench.home_rf_receiver ()

bench.home_rf_transmitter ()

bench.closeup (23, 62)

# Readings to take with a multimeter: the transmitter's DAT through a test,
# where each message is on the air for at most 0.43 s and then rests for
# 10 s or more, so the meter shows 0 V but for a twitch; and the receiver's
# DATA hearing noise between messages.
bench.measure ("The transmitter's DAT", red="h57", black="GND", expect="0 V",
               when="between messages")
bench.measure ("The receiver's DATA, hearing noise", red="43", black="GND", expect="about 2.5 V",
               when="between messages")
