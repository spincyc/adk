# Each board has its LoRa modem at its bridge home, as in every two-board
# lesson, on Serial3 (pins 14 and 15), its VDD on the Mega's 3.3V pin.
#
# Board A, the door: the screen and the keypad at their homes, the
# keypad's eight wires from pins 22-29 as in Lesson 16. Pins 14 and 15
# cross them just below its plug.
door = Bench ("Board A: the keypad on pins 22 to 29, the screen on 31 to 36, and the LoRa modem "
              "on Serial3 (pins 14 and 15)", columns=(1, 50), sketch="Door")

door.screen (text=("Locked. Code?", "**"), risers=(4.8, 0.05))

door.home_keypad ()

door.home_modem (tx=[(3.05, -0.35), (8.1, -0.35)],
                 rx=[(3.15, -0.25), (7.9, -0.25)])

# Board B, inside: the power module beside the board, its 5V and GND
# wired to B+42 and B-42 for the servo; the Mega's GND joins the rails at
# B-3. The RGB LED at its home on pins 5, 6 and 7, as in Lesson 34.
# The servo keeps its Lesson 17 home below the modem.
inside = Bench ("Board B: a servo latch on pin 44 powered by the power module, the RGB LED on "
                "pins 5, 6 and 7, and the LoRa modem on Serial3 (pins 14 and 15)",
                columns=(1, 63), sketch="Inside")

inside.power_module ()

inside.home_rgb_led ()

inside.home_modem ()

inside.home_servo ()

boards = {"A": door, "B": inside}

# Readings to take with a multimeter on Board B's RGB LED pins: blue at
# 40 parts in 255 while the lock waits, and green full on while it's open.
inside.measure ("The blue pin, waiting", red="7", black="GND", expect="about 0.8 V",
                when="light dim blue")
inside.measure ("The green pin, open", red="6", black="GND", expect="about 5 V",
                when="latch open, light green")
