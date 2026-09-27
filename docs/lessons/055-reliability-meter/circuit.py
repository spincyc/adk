# The screen at its home, its contrast knob across the middle gap, and the
# rotary encoder at its home above the Mega, wired as in Lesson 29: CLK and
# DT from 18 and 19, SW from 22, + from the inner 5V pin at the top of the
# long header and GND from the GND beside pin 13. The 433 MHz receiver and
# transmitter stand at their homes from Lesson 38, in row j past the
# screen, with every wire coming up from below, round the bottom of the
# screen: the receiver in columns 48 to 51 with 5 V from the power header
# and pin 43 into DATA's column; the transmitter in columns 56 to 59 with
# the Mega's 3.3V, its DAT the middle of a divider from pin 46.
bench = Bench ("A 433 MHz receiver on pin 43 and transmitter on pin 46, the rotary encoder on 18 "
               "and 19 with its switch on 22, and the LCD on pins 31 to 36", columns=(1, 62))

bench.screen (text=("20 letters 186ms", "Heard 10/10 100%"), across=True)

bench.home_encoder ()

bench.home_rf_receiver ()

bench.home_rf_transmitter ()

bench.closeup (23, 62)

# Readings to take with a multimeter: the transmitter's DAT resting, and
# while a test of 60-letter messages runs, when the radio is on half of
# each message's bits and messages follow each other 50 ms apart; and the
# receiver's DATA hearing noise between tests.
bench.measure ("The transmitter's DAT, resting", red="h57", black="GND", expect="0 V",
               when="no test running")
bench.measure ("The transmitter's DAT, during a test", red="h57", black="GND",
               expect="about 1.5 V", when="testing 60 letters")
bench.measure ("The receiver's DATA, hearing noise", red="43", black="GND", expect="about 2.5 V",
               when="no test running")
