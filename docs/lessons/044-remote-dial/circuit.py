# Each board keeps its LoRa modem at its bridge home from Lesson 43, on
# Serial3 (pins 14 and 15), its VDD on the Mega's 3.3V pin.
#
# Board A, the dial: the button and the LEDs come out, and the course's
# screen goes in at its home, wired by screen (). The rotary encoder sits
# in row a, columns 15 to 19, as in Lesson 37, on 18, 19 and 22.
dial = Bench ("Board A: the LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
              "on 22, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 50),
              sketch="Dial")

dial.screen (text=("Asked  135°", "Servo  120°"))

dial.home_encoder ()

dial.home_modem ()

# Board B, the servo: the button and the red LED come out; the yellow LED
# on 27 stays at its home, now the link light. The power module lies
# beside the board, its 5V and GND wired to B+42 and B-42: the servo
# takes its power from the bottom rails, never the Mega's. The servo keeps
# its Lesson 17 home below the level of the modem.
servo = Bench ("Board B: a servo on pin 44 powered by the breadboard power module, a yellow LED on "
               "27, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 63),
               sketch="Servo")

servo.power_module ()

servo.home_led ("27", "yellow")

servo.home_modem ()

servo.home_servo ()

boards = {"A": dial, "B": servo}

# Readings to take with a multimeter on Board B: the servo's 5 V from the
# power module, and pin 27, high while Board A can be heard.
servo.measure ("The servo's 5 V, on the bottom rails", red="B+40", black="B-40",
               expect="about 5 V", when="power module on")
servo.measure ("Pin 27, the link light", red="27", black="GND", expect="about 5 V",
               when="Board A on")
