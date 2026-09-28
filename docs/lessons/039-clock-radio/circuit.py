# Lesson 38's screen and button stay where they were; the 433 MHz modules
# go. The clock module comes back to its home from Lessons 32 and 33, and
# the rotary encoder to its home above the Mega, wired as in Lesson 33. The
# FM radio and the volume knob come back to their homes from Lesson 37.
bench = Bench ("A clock radio: the FM radio on pins 40 to 42, the clock module on 20 and 21, the "
               "rotary encoder on 18, 19 and 22, a button on 23, a volume knob on A0, and the LCD "
               "on pins 31 to 36", columns=(1, 62))

bench.screen (text=("06:58:30  \u237e07:00", "CITY FM     98.8"), risers=(4.55, 0.1))
bench.home_rtc (sda=[(3.75, 0.35), (3.95, 0.35), (3.95, -1.6), (8.5, -1.6), (8.5, -0.8)],
                scl=[(3.85, 0.45), (4.05, 0.45), (4.05, -1.5), (8.4, -1.5), (8.4, -0.9)])

bench.home_encoder (lift=0.3)

bench.home_button ("23", via=[(4.4, 0.85), (4.4, -1.7), (9.1, -1.7)])

bench.home_fm_radio ()

bench.home_knob (via=[(2.19, 4.05), (11.1, 4.05)])

bench.closeup (23, 62)

# Readings to take with a multimeter: the volume knob's wiper, which the
# sketch turns into the radio's volume, halfway and a quarter of the way.
bench.measure ("The volume knob halfway, on A0", red="A0", black="GND", expect="about 2.5 V",
               when="volume knob halfway")
bench.measure ("The volume knob a quarter up, on A0", red="A0", black="GND",
               expect="about 1.25 V", when="volume knob a quarter of the way")
