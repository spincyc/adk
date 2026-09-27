# Both boards keep the LoRa modem at its bridge home from Lesson 50, on
# Serial3 (pins 14 and 15).
#
# Board A, the door: the GY-521 comes out. The RFID reader lies at its
# home, as in Lessons 34 and 36, on the Mega's 3.3V pin; the tap sensor at
# its home, as in Lessons 35 and 36. Reader and modem together would ask
# more of that pin than it can give, so here the modem's VDD moves to the
# power module's bottom rails, set to 3.3 V, into B+47 above it: the modem's
# home in Lesson 40. Nothing else takes power from the rails, so the top
# jumper is off and the Mega's 5V stays off them. The doorbell is the
# button on 22 at its home, and the active buzzer on 12 stands at its
# home. The L LED shows the link.
door = Bench ("Board A: an RFID reader on the SPI pins and 45, a tap sensor on A12, a doorbell "
              "button on 22, an active buzzer on 12, and the LoRa modem on Serial3 (pins 14 and "
              "15), powered at 3.3 V by the power module", columns=(1, 63), sketch="Door")

door.power_module ("right", top="off", bottom="3.3V")

door.home_button ("22")

door.home_buzzer ("active", via=[(1.7, -0.45), (8.8, -0.45)])

door.home_rfid ()

door.home_tap ()

door.home_modem (tx=[(3.15, -0.35), (9.9, -0.35)], rx=[(3.25, -0.25), (9.7, -0.25)], power="B+47")

# Board B, inside: the matrix comes out; the power module, the servo latch
# and the modem stay. The screen goes in at its home, powered from the top
# rails by the Mega's 5V. Beside it, at their homes, stand the button on
# 23 and the passive buzzer on 10. Pin 44's wire takes its Lesson 49 way
# to the latch.
inside = Bench ("Board B: the LCD on pins 31 to 36, a button on 23, a passive buzzer on 10, a "
                "servo latch on 44 powered by the power module, and the LoRa modem on Serial3 "
                "(pins 14 and 15)", columns=(1, 63), sketch="Inside")

inside.power_module ("right", top="off", bottom="5V")
inside.screen (text=("Welcome home,", "Ada"))

inside.home_button ("23")

inside.home_modem ()

inside.home_buzzer ("passive")

inside.home_servo (via=[(4.85, 1.95), (4.85, 5.55), (10.5, 5.55)])

boards = {"A": door, "B": inside}

# Readings to take with a multimeter: the two boards' bottom rails, which
# look alike and are not. Board A's carry 3.3 V for its modem, Board B's
# 5 V for its latch.
door.measure ("Board A's bottom rails, for the modem", red="B+49", black="B-49",
              expect="about 3.3 V", when="power module on")
inside.measure ("Board B's bottom rails, for the latch", red="B+58", black="B-58",
                expect="about 5 V", when="power module on")
