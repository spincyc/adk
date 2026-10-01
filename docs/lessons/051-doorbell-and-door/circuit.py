# Both boards keep the LoRa modem at its bridge home from Lesson 50, on
# Serial3 (pins 14 and 15).
#
# Board A, inside: the GY-521 comes out. The screen goes in at its home,
# powered from the top rails by the Mega's 5V. Beside it, at their homes,
# stand the button on 23 and the passive buzzer on 10. A power module feeds
# the bottom rails at 5 V for the servo latch on 44, at its home below the
# modem.
inside = Bench ("Board A: the LCD on pins 31 to 36, a button on 23, a passive buzzer on 10, a "
                "servo latch on 44 powered by the power module, and the LoRa modem on Serial3 "
                "(pins 14 and 15)", columns=(1, 63), sketch="Inside")

inside.power_module ()
inside.screen (text=("Welcome home,", "Ada"))

inside.home_button ("23")

inside.home_modem ()

inside.home_buzzer ("passive")

inside.home_servo ()

# Board B, the door: the matrix and the servo come out. The RFID reader lies
# at its home, as in Lessons 34 and 36, on the Mega's 3.3V pin; the tap
# sensor at its home, as in Lessons 35 and 36. Reader and modem together
# would ask more of that pin than it can give, so here the modem's VDD
# moves to the bottom rails, which the power module now feeds from its
# 3.3V pin, into B+29 above it: the modem's home in Lesson 40. The active
# buzzer takes its power from the Mega's 5V on the separate top rail. The
# doorbell is the button on 22 at its home, and the active buzzer on 12
# stands at its home. The L LED shows the link.
door = Bench ("Board B: an RFID reader on the SPI pins and 45, a tap sensor on A12, a doorbell "
              "button on 22, an active buzzer on 12, and the LoRa modem on Serial3 (pins 14 and "
              "15), powered at 3.3 V by the power module", columns=(1, 63), sketch="Door")

door.power_module ("3.3V")

door.home_button ("22")

door.home_buzzer ("active")

door.home_rfid ()

door.home_tap ()

door.home_modem (power="B+29")

boards = {"A": inside, "B": door}

# Readings to take with a multimeter: the two boards' bottom rails, which
# look alike and are not. Board A's carry 5 V for its latch, Board B's
# 3.3 V for its modem.
inside.measure ("Board A's bottom rails, for the latch", red="B+40", black="B-40",
                expect="about 5 V", when="power module on")
door.measure ("Board B's bottom rails, for the modem", red="B+40", black="B-40",
              expect="about 3.3 V", when="power module on")
