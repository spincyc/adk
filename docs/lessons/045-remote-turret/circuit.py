# Board A, the joystick: Lesson 44's screen and LoRa modem stay where they
# were; the rotary encoder comes out, and the joystick lies at its home
# below the Mega under A3 and A4, as in Lesson 26: its +5V from the power
# header, its GND from the inner GND pin at the end of the long header, and
# its switch on 22.
stick = Bench ("Board A: the LCD on pins 31 to 36, a joystick on A3 and A4 with its switch on "
               "22, and the LoRa modem on Serial3 (pins 14 and 15)", columns=(1, 50),
               sketch="Joystick")

stick.screen (text=("Aim 90°  Fan on", "Ahead  85 cm"))

stick.module ("joystick", at=(2.08, 3.9), facing="up")
stick.wire ("A3", "joystick.VRx")
stick.wire ("A4", "joystick.VRy")
stick.wire ("5V.power", "joystick.+5V")
stick.wire ("GND.long", "joystick.GND")
stick.wire ("22", "joystick.SW")

stick.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
stick.wire ("modem.GND", "B-42")
stick.wire ("modem.VDD", "3.3V")
stick.wire ("14", "j46")
stick.resistor ("1 kΩ", "g46", "e46")
stick.resistor ("2 kΩ", "a46", "B-46")
stick.wire ("modem.RXD", "c46", color="grey")
stick.wire ("modem.TXD", "f44", color="purple")
stick.wire ("15", "j44")

# Board B, the turret: Lesson 44's power module, modem and servo stay, and
# the yellow LED comes out, as the L293D takes its column; the Mega's own
# L LED on pin 13 shows the link instead. The ultrasonic sensor takes
# pins 14 and 15, so the modem moves to Serial2: TX2 (pin 16) into j46 and
# RX2 (pin 17) into j44, the holes 14 and 15 had. Lesson 21's turret goes
# in as it was there: the sensor above the Mega, its VCC on the inner 5V
# pin and its GND in T-5; the L293D across the gap from column 12, its
# logic on the Mega's 5 V from the top rails and the motor's supply from
# the power module's bottom rails; the motor above the board with its
# leads down into j14 and j17.
turret = Bench ("Board B: a servo on pin 44 carrying an ultrasonic sensor on 14 and 15 and a fan on "
                "an L293D (4, 8, 9), both powered from the breadboard power module, and the LoRa "
                "modem on Serial2 (pins 16 and 17)", columns=(1, 63), sketch="Turret")

turret.power_module ("right", top="off", bottom="5V")

turret.module ("ultrasonic", name="sensor", at=(2.715, -1.2))
turret.wire ("14", "sensor.Trig")
turret.wire ("15", "sensor.Echo")
turret.wire ("sensor.VCC", "5V.long")
turret.wire ("sensor.GND", "T-5")

turret.chip ("L293D", first=12)
turret.wire ("j12", "T+12")
turret.wire ("a15", "B-15")
turret.wire ("a19", "B+19")
turret.wire ("8", "j13", via=[(2.09, -1.47), (4.6, -1.47), (4.6, 0.35), (6.6, 0.35)])
turret.wire ("9", "j18", via=[(1.99, -1.57), (7.1, -1.57)])
turret.wire ("4", "j19", via=[(2.55, -1.67), (7.2, -1.67)])
turret.module ("motor", name="motor", at=(4.7, -1.25), facing="down")
turret.wire ("motor.−", "j14", via=[(6.7, -0.41)])
turret.wire ("motor.+", "j17", via=[(7.0, -0.59)])

turret.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
turret.wire ("modem.GND", "B-42")
turret.wire ("modem.VDD", "3.3V")
turret.wire ("16", "j46", via=[(3.35, 0.45), (2.6, 0.45), (2.6, -1.85), (9.65, -1.85)])
turret.resistor ("1 kΩ", "g46", "e46")
turret.resistor ("2 kΩ", "a46", "B-46")
turret.wire ("modem.RXD", "c46", color="grey")
turret.wire ("modem.TXD", "f44", color="purple")
turret.wire ("17", "j44", via=[(3.45, 0.4), (2.65, 0.4), (2.65, -1.8), (9.45, -1.8)])

turret.module ("servo", "servo", at=(9.85, 5.7), facing="up")
turret.wire ("servo.+", "B+53")
turret.wire ("servo.−", "B-54")
turret.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 5.45), (10.5, 5.45)])
turret.note ("tape the sensor and the fan to the horn", "servo.signal", offset=(-1.6, 1.2))

boards = {"A": stick, "B": turret}

# Readings to take with a multimeter on Board B: the fan's enable pin, set
# from Board A's joystick button, and what happens to it when Board A
# goes quiet.
turret.measure ("The enable pin, fan on", red="4", black="GND", expect="about 3.9 V",
                when="fan on, turret still")
turret.measure ("The enable pin, Board A off", red="4", black="GND", expect="0 V",
                when="five seconds after")
