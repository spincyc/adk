# Each board keeps its LoRa modem at its bridge home from Lesson 43: below
# the board under columns 42-47, TX3 (pin 14) through the divider in
# column 46, RX3 (pin 15) in j44, VDD on the Mega's 3.3V pin.
#
# Board A, the dial: the button and the LEDs come out, and the course's
# screen goes in at its home, wired by screen (). The rotary encoder sits
# at its home above the Mega, as in Lesson 29: CLK and DT from 18 and 19,
# SW from 22, + from the inner 5V pin at the top of the long header and
# GND from the GND beside pin 13.
dial = Bench ("Board A: the LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
              "on 22, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 50),
              sketch="Dial")

dial.screen (text=("Asked  135°", "Servo  120°"))

dial.module ("encoder", at=(3.0, -2.1))
dial.wire ("18", "encoder.CLK", via=[(3.55, 0.55), (3.2, 0.55)])
dial.wire ("19", "encoder.DT", via=[(3.65, 0.45), (3.3, 0.45)])
dial.wire ("22", "encoder.SW", via=[(4.3, 0.8), (4.3, 0.35), (3.4, 0.35)])
dial.wire ("5V.long", "encoder.+", via=[(4.25, 0.7), (4.25, 0.25), (3.5, 0.25)])
dial.wire ("GND.top", "encoder.GND", via=[(1.5, 0.15), (3.6, 0.15)])

dial.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
dial.wire ("modem.GND", "B-42")
dial.wire ("modem.VDD", "3.3V")
dial.wire ("14", "j46", via=[(3.15, 0.05), (4.3, 0.05), (4.3, -0.22), (9.65, -0.22)])
dial.resistor ("1 kΩ", "g46", "e46")
dial.resistor ("2 kΩ", "a46", "B-46")
dial.wire ("modem.RXD", "c46", color="grey")
dial.wire ("modem.TXD", "f44", color="purple")
dial.wire ("15", "j44", via=[(3.25, 0.1), (4.35, 0.1), (4.35, -0.17), (9.45, -0.17)])

# Board B, the servo: the button and the red LED come out; the yellow LED
# on 27 stays at its home in column 12, now the link light. The power
# module goes on the right end, its top jumper off and its bottom one on
# 5 V: the servo takes its power from the bottom rails, never the Mega's.
# The servo lies below the board with its plug under columns 52-54, +
# into B+53 and − into B-54, as at its home, but lower than in Lesson 17,
# below the level of the modem, whose place overlaps its usual one.
servo = Bench ("Board B: a servo on pin 44 powered by the breadboard power module, a yellow LED on "
               "27, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 63),
               sketch="Servo")

servo.power_module ("right", top="off", bottom="5V")

servo.wire ("27", "j12")
servo.resistor ("220 Ω", "g12", "e12")
servo.led ("yellow", anode="b12", cathode="b13")
servo.wire ("a13", "B-13")

servo.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
servo.wire ("modem.GND", "B-42")
servo.wire ("modem.VDD", "3.3V")
servo.wire ("14", "j46")
servo.resistor ("1 kΩ", "g46", "e46")
servo.resistor ("2 kΩ", "a46", "B-46")
servo.wire ("modem.RXD", "c46", color="grey")
servo.wire ("modem.TXD", "f44", color="purple")
servo.wire ("15", "j44")

servo.module ("servo", "servo", at=(9.85, 5.7), facing="up")
servo.wire ("servo.+", "B+53")
servo.wire ("servo.−", "B-54")
servo.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 5.45), (10.5, 5.45)])

boards = {"A": dial, "B": servo}

# Readings to take with a multimeter on Board B: the servo's 5 V from the
# power module, and pin 27, high while Board A can be heard.
servo.measure ("The servo's 5 V, on the bottom rails", red="B+58", black="B-58",
               expect="about 5 V", when="power module on")
servo.measure ("Pin 27, the link light", red="27", black="GND", expect="about 5 V",
               when="Board A on")
