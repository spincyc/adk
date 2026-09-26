# Each board has its LoRa modem at its bridge home, as in every two-board
# lesson: below the board under columns 42-47, aerial down; its TXD up into
# f44 beside the wire from RX3 (pin 15) in j44; TX3 (pin 14) into j46,
# through 1 kΩ across the gap and 2 kΩ down to the − rail, and its RXD into
# c46, the divider's middle; its GND into B-42 and its VDD on the Mega's
# 3.3V pin.
#
# Board A, the door: the screen at its home, and the keypad high above the
# gap between the Mega and the breadboard, its eight wires rising from pins
# 22-29 as in Lesson 16. Pins 14 and 15 cross them just below its plug.
door = Bench ("Board A: the keypad on pins 22 to 29, the screen on 31 to 36, and the LoRa modem "
              "on Serial3 (pins 14 and 15)", columns=(1, 50), sketch="Door")

door.screen (text=("Locked. Code?", "**"), risers=(4.8, 0.05))

door.module ("keypad", "keypad", at=(3.04, -4.95))
turns = (0.5, 0.45, 0.4, 0.35, None, 0.4, 0.45, 0.5)
for index, name in enumerate (("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")):
    riser, height = 4.25 + 0.05 * index, 0.8 + 0.05 * index
    socket = 4.05 + 0.1 * index
    via = [(riser, height)]
    if turns[index]:
        via += [(riser, turns[index]), (socket, turns[index])]
    door.wire (str (22 + index), f"keypad.{name}", via=via)

door.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
door.wire ("modem.GND", "B-42")
door.wire ("modem.VDD", "3.3V")
door.wire ("14", "j46", via=[(3.15, -0.35), (9.9, -0.35)])
door.resistor ("1 kΩ", "g46", "e46")
door.resistor ("2 kΩ", "a46", "B-46")
door.wire ("modem.RXD", "c46", color="grey")
door.wire ("modem.TXD", "f44", color="purple")
door.wire ("15", "j44", via=[(3.25, -0.25), (9.7, -0.25)])

# Board B, inside: the power module at the right end, its top jumper off
# and its bottom one on 5 V for the servo; the Mega's GND joins the rails
# at B-3. The RGB LED at its home, as in Lesson 34: pins 5, 6 and 7 into
# j6, j9 and j11, a 220 Ω resistor across the gap above each colored leg,
# the common leg in B-7. The servo's plug is under columns 52-54, + into
# B+53 and − into B-54 as at its home, but it lies lower than in Lesson 17,
# below the modem, whose place overlaps its old one; pin 44's wire drops
# beside the board and runs under the modem to it.
inside = Bench ("Board B: a servo latch on pin 44 powered by the power module, the RGB LED on "
                "pins 5, 6 and 7, and the LoRa modem on Serial3 (pins 14 and 15)",
                columns=(1, 63), sketch="Inside")

inside.power_module ("right", top="off", bottom="5V")

inside.wire ("5", "j6")
inside.wire ("6", "j9")
inside.wire ("7", "j11")
inside.resistor ("220 Ω", "g6", "e6")
inside.resistor ("220 Ω", "g9", "e9")
inside.resistor ("220 Ω", "g11", "e11")
inside.rgb_led (red="a6", common="B-7", green="a9", blue="a11")

inside.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
inside.wire ("modem.GND", "B-42")
inside.wire ("modem.VDD", "3.3V")
inside.wire ("14", "j46")
inside.resistor ("1 kΩ", "g46", "e46")
inside.resistor ("2 kΩ", "a46", "B-46")
inside.wire ("modem.RXD", "c46", color="grey")
inside.wire ("modem.TXD", "f44", color="purple")
inside.wire ("15", "j44")

inside.module ("servo", "servo", at=(9.85, 5.7), facing="up")
inside.wire ("servo.+", "B+53")
inside.wire ("servo.−", "B-54")
inside.wire ("44", "servo.signal", via=[(4.85, 1.95), (4.85, 5.55), (10.5, 5.55)])

boards = {"A": door, "B": inside}

# Readings to take with a multimeter on Board B's RGB LED pins: blue at
# 40 parts in 255 while the lock waits, and green full on while it's open.
inside.measure ("The blue pin, waiting", red="7", black="GND", expect="about 0.8 V",
                when="light dim blue")
inside.measure ("The green pin, open", red="6", black="GND", expect="about 5 V",
                when="latch open, light green")
