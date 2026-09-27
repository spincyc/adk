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

bench.module ("encoder", at=(3.0, -2.1))
bench.wire ("18", "encoder.CLK", via=[(3.55, 0.55), (3.2, 0.55)])
bench.wire ("19", "encoder.DT", via=[(3.65, 0.45), (3.3, 0.45)])
bench.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.35), (3.4, 0.35)])
bench.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, 0.25), (3.5, 0.25)])
bench.wire ("GND.top", "encoder.GND", via=[(1.5, 0.15), (3.6, 0.15)])

bench.header_module ("rf_receiver", first=48, row="j")
bench.wire ("5V.power", "f48", via=[(1.69, 3.65), (10.1, 3.65)])
bench.wire ("43", "f49", via=[(4.45, 1.85), (4.45, 3.75), (10.2, 3.75)])
bench.wire ("f51", "B-51")

bench.header_module ("rf_transmitter", first=56, row="j")
bench.wire ("46", "f54", via=[(4.25, 2.0), (4.55, 2.0), (4.55, 3.85), (10.7, 3.85)])
bench.resistor ("1 kΩ", "h54", "h57")
bench.resistor ("2 kΩ", "g57", "e57")
bench.wire ("a57", "B-57")
bench.wire ("3.3V", "f58", via=[(1.59, 3.95), (11.1, 3.95)])
bench.wire ("f59", "B-59")

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
