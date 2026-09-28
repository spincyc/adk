# Board A, the joystick: Lesson 44's screen and LoRa modem stay where they
# were; the rotary encoder comes out, and the joystick goes in at its home
# on A3 and A4, as in Lesson 26, its switch on 22.
stick = Bench ("Board A: the LCD on pins 31 to 36, a joystick on A3 and A4 with its switch on "
               "22, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 50),
               sketch="Joystick")

stick.screen (text=("Aim 90°  Fan on", "Ahead  85 cm"))

stick.home_joystick ()

stick.home_modem ()

# Board B, the turret: Lesson 44's power module, modem and servo stay, and
# the yellow LED comes out, as the L293D takes its column; the Mega's own
# L LED on pin 13 shows the link instead. The ultrasonic sensor takes
# pins 14 and 15, so the modem moves to Serial2, TX2 (pin 16) and RX2
# (pin 17), in the holes 14 and 15 had. Lesson 21's turret goes in as it
# was there: the sensor at its home; the L293D across the gap from column
# 12, its logic on the Mega's 5 V from the top rails and the motor's
# supply from the power module's bottom rails; the motor at its home, on
# the L293D's outputs.
turret = Bench ("Board B: a servo on pin 44 carrying an ultrasonic sensor on 14 and 15 and a fan on "
                "an L293D (4, 8, 9), both powered from the breadboard power module, and the LoRa "
                "modem on Serial2 (pins 16 and 17)", columns=(1, 63), sketch="Turret")

turret.power_module ()

turret.home_ultrasonic ()

turret.chip ("L293D", first=12)
turret.wire ("j12", "T+12")
turret.wire ("a15", "B-15")
turret.wire ("a19", "B+19")
turret.wire ("8", "j13", via=[(1.99, -1.47), (4.6, -1.47), (4.6, 0.35), (6.6, 0.35)])
turret.wire ("9", "j18", via=[(1.89, -1.57), (7.1, -1.57)])
turret.wire ("4", "j19", via=[(2.45, -1.67), (7.2, -1.67)])
turret.home_motor ()

turret.home_modem (tx=[(3.25, 0.45), (2.6, 0.45), (2.6, -1.85), (9.65, -1.85)],
                   rx=[(3.35, 0.4), (2.65, 0.4), (2.65, -1.8), (9.45, -1.8)])

turret.home_servo (via=[(4.55, 1.90), (4.55, 5.45), (10.5, 5.45)])
turret.note ("tape the sensor and the fan to the horn", "servo.signal", offset=(-1.6, 1.2))

boards = {"A": stick, "B": turret}

# Readings to take with a multimeter on Board B: the fan's enable pin, set
# from Board A's joystick button, and what happens to it when Board A
# goes quiet.
turret.measure ("The enable pin, fan on", red="4", black="B-21", expect="about 3.9 V",
                when="fan on, turret still")
turret.measure ("The enable pin, Board A off", red="4", black="B-21", expect="0 V",
                when="five seconds after")
