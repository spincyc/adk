# Both boards keep the LoRa modem's holes from Lesson 50: below the board
# under columns 42-47, TX3 (pin 14) through the divider in column 46, RX3
# (pin 15) in j44, GND into B-42.
#
# Board A, the door: the GY-521 comes out. The RFID reader lies below the
# Mega, as in Lessons 34 and 36, on the Mega's 3.3V pin; the tap sensor
# beside it, as in Lessons 35 and 36. Reader and modem together would ask
# more of that pin than it can give, so here the modem's VDD moves to the
# power module's bottom rails, set to 3.3 V, into B+47 above it: the modem's
# home in Lesson 40. Nothing else takes power from the rails, so the top
# jumper is off and the Mega's 5V stays off them. The doorbell is the
# button on 22 at its home in columns 2-4, and the active buzzer on 12
# stands at its home in column 34. The L LED shows the link.
door = Bench ("Board A: an RFID reader on the SPI pins and 45, a tap sensor on A12, a doorbell "
              "button on 22, an active buzzer on 12, and the LoRa modem on Serial3 (pins 14 and "
              "15), powered at 3.3 V by the power module", columns=(1, 63), sketch="Door")

door.power_module ("right", top="off", bottom="3.3V")

door.wire ("22", "j2")
door.button (2)
door.wire ("a4", "B-4")

door.wire ("12", "j34", via=[(1.7, -0.45), (8.8, -0.45)])
door.buzzer ("f34", "e34", kind="active")
door.wire ("a34", "B-34")

door.module ("rfid", at=(1.65, 4.17), facing="up")
door.wire ("3.3V", "rfid.3.3V", via=[(1.6, 2.8), (2.1, 2.8)])
door.wire ("GND.long", "rfid.GND", via=[(4.25, 2.4), (4.25, 3.0), (2.3, 3.0)])
door.wire ("53", "rfid.SDA", via=[(4.35, 3.1), (2.8, 3.1)])
door.wire ("52", "rfid.SCK", via=[(4.45, 3.2), (2.7, 3.2)])
door.wire ("51", "rfid.MOSI", via=[(4.55, 3.3), (2.6, 3.3)])
door.wire ("50", "rfid.MISO", via=[(4.65, 3.4), (2.5, 3.4)])
door.wire ("45", "rfid.RST", via=[(4.75, 3.5), (2.2, 3.5)])

door.module ("sensor", name="tap", at=(0.43, 3.62), pins=["S", "+", "−"], label="tap sensor",
             facing="up")
door.wire ("A12", "tap.S", via=[(3.5, 2.95), (0.85, 2.95)])
door.wire ("5V.power", "tap.+", via=[(1.7, 2.9), (0.75, 2.9)])
door.wire ("GND.power", "tap.−", via=[(1.8, 2.85), (0.65, 2.85)])

door.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
door.wire ("modem.GND", "B-42")
door.wire ("modem.VDD", "B+47")
door.wire ("14", "j46", via=[(3.15, -0.35), (9.9, -0.35)])
door.resistor ("1 kΩ", "g46", "e46")
door.resistor ("2 kΩ", "a46", "B-46")
door.wire ("modem.RXD", "c46", color="grey")
door.wire ("modem.TXD", "f44", color="purple")
door.wire ("15", "j44", via=[(3.25, -0.25), (9.7, -0.25)])

# Board B, inside: the matrix comes out; the power module, the servo latch
# and the modem stay. The screen goes in at its home, powered from the top
# rails by the Mega's 5V. Beside it stand the button on 23, in columns
# 38-40, and the passive buzzer on 10 in column 51, its 220 Ω resistor
# down to the − rail. Pin 44's wire takes its Lesson 49 way to the latch.
inside = Bench ("Board B: the LCD on pins 31 to 36, a button on 23, a passive buzzer on 10, a "
                "servo latch on 44 powered by the power module, and the LoRa modem on Serial3 "
                "(pins 14 and 15)", columns=(1, 63), sketch="Inside")

inside.power_module ("right", top="off", bottom="5V")
inside.screen (text=("Welcome home,", "Ada"))

inside.wire ("23", "j38")
inside.button (38)
inside.wire ("a40", "B-40")

inside.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
inside.wire ("modem.GND", "B-42")
inside.wire ("modem.VDD", "3.3V")
inside.wire ("14", "j46")
inside.resistor ("1 kΩ", "g46", "e46")
inside.resistor ("2 kΩ", "a46", "B-46")
inside.wire ("modem.RXD", "c46", color="grey")
inside.wire ("modem.TXD", "f44", color="purple")
inside.wire ("15", "j44")

inside.wire ("10", "j51")
inside.buzzer ("f51", "e51", kind="passive")
inside.resistor ("220 Ω", "a51", "B-51")

inside.module ("servo", "servo", at=(9.85, 5.7), facing="up")
inside.wire ("servo.+", "B+53")
inside.wire ("servo.−", "B-54")
inside.wire ("44", "servo.signal", via=[(4.85, 1.95), (4.85, 5.55), (10.5, 5.55)])

boards = {"A": door, "B": inside}

# Readings to take with a multimeter: the two boards' bottom rails, which
# look alike and are not. Board A's carry 3.3 V for its modem, Board B's
# 5 V for its latch.
door.measure ("Board A's bottom rails, for the modem", red="B+49", black="B-49",
              expect="about 3.3 V", when="power module on")
inside.measure ("Board B's bottom rails, for the latch", red="B+58", black="B-58",
                expect="about 5 V", when="power module on")
