# Glossary

The words the course uses, in plain terms, each with the lessons that teach
it: a project lesson, an electricity investigation, or both. The words of
C++ are in [The C++ you've met](cpp.md), and ADK's parts in
[the library reference](library/index.md).

## Numbers

**1-Wire** { #1-wire }
:   A way of sending data both ways along a single wire, which the 18B20
    thermometer uses. Taught in
    [Lesson 14](lessons/014-thermometers/index.md#the-idea).

## A

**Accelerometer** { #accelerometer }
:   A sensor that measures how hard it is pushed along three directions, x,
    y and z. Gravity is one of those pushes, so it can tell which way is
    down. Taught in [Lesson 28](lessons/028-tilt/index.md#the-idea).

**Active low, active high** { #active-low }
:   Which level a sensor module's output takes when it notices something:
    low (0 V) or high (5 V). Taught in
    [Lesson 23](lessons/023-tripwires/index.md#the-idea).

**ADC** { #adc }
:   The analog-to-digital converter: the part of the Mega that measures a
    voltage from 0 to 5 V on pins A0 to A15 and turns it into a number from
    0 to 1023. Taught in [Lesson 7](lessons/007-dimmer/index.md#the-idea)
    and [E22](lessons/077-sampling/index.md#explain-what-you-saw).

**Alternating current (AC)** { #ac }
:   Current that flows one way, then the other, again and again. Wall
    sockets give AC; nothing in this course goes near them. Taught in
    [E13](lessons/068-alternating-current/index.md#watch-both-voltages).

**Amplifier, gain** { #amplifier }
:   An amplifier makes a signal larger. Its gain is how many times larger:
    the output's height divided by the input's. Taught in
    [E16](lessons/071-amplifier-gain/index.md#why-it-happens).

**Analog, digital** { #analog }
:   A digital pin is either on or off. An analog pin measures any voltage
    from 0 to 5 V, as a number. See [ADC](#adc). Taught in
    [Lesson 7](lessons/007-dimmer/index.md#the-idea) and
    [E06](lessons/061-tap-a-divider/index.md#upload-and-measure).

**Anode, cathode** { #anode }
:   The two ends of an LED or a diode. Current goes in at the anode (+, an
    LED's long leg) and out at the cathode (−, its short leg, or a diode's
    banded end). Taught in [Lesson 1](lessons/001-blink/index.md#the-idea),
    [Lesson 4](lessons/004-mood-lamp/index.md#the-idea) and
    [E09](lessons/064-one-way-diode/index.md#build-it).

## B

**Baud** { #baud }
:   How many bits a second a serial port sends. At 9600 baud each bit lasts
    1/9600 of a second, and both ends must agree on it. Taught in
    [E24](lessons/079-serial-link/index.md#why-it-happens).

**Bit, byte** { #bit }
:   A bit is a single 0 or 1: off or on. Eight bits make a byte, which can
    count from 0 to 255. Taught in
    [Lesson 10](lessons/010-dice/index.md#the-idea) and
    [E24](lessons/079-serial-link/index.md#why-it-happens).

**Breadboard** { #breadboard }
:   A board of holes with metal strips underneath. Parts pushed into the
    same strip are joined, so a circuit needs no soldering. Taught in
    [Lesson 1](lessons/001-blink/index.md#build-it).

**Bridge** { #bridge }
:   ADK's way of keeping a few named numbers the same on two boards over a
    radio: one board shares a number, and the other reads its latest value.
    Taught in [Lesson 43](lessons/043-the-bridge/index.md#the-idea).

## C

**Capacitor** { #capacitor }
:   A part that stores charge. Its voltage rises as charge gathers and falls
    as it leaves. Taught in
    [E07](lessons/062-charge-a-capacitor/index.md#predict).

**Channel** { #channel }
:   A radio setting that radios must share to hear each other: a frequency
    for the E32 modules, or a named group with its own key in Meshtastic.
    Taught in [Lesson 41](lessons/041-lora-link/index.md#the-idea) and
    [Lesson 42](lessons/042-mesh-messenger/index.md#the-idea).

**Checksum** { #checksum }
:   A number made from every byte of a message. The receiver works it out
    again, and throws the message away if the two don't match. Taught in
    [Lesson 14](lessons/014-thermometers/index.md#the-idea) and
    [Lesson 38](lessons/038-radio-messages/index.md#the-idea).

**Clock** { #clock }
:   A steady beat that says when each bit, or each step of a circuit,
    happens: the clock pin of a shift register, I2C's SCL, or a ticking
    gate. Taught in [Lesson 10](lessons/010-dice/index.md#the-idea) and
    [E21](lessons/076-schmitt-clock/index.md#watch-the-clock). For the clock
    that tells the time, see [Real-time clock](#real-time-clock).

**Closed loop** { #closed-loop }
:   A machine that measures its own result with a sensor and corrects
    itself. Taught in
    [Lesson 52](lessons/052-remote-stepper/index.md#the-idea).

**Current** { #current }
:   The flow of electricity, which passes only around a complete loop. It is
    measured in amps (A), or thousandths of an amp, milliamps (mA). Taught
    in [Lesson 1](lessons/001-blink/index.md#the-idea) and
    [E01](lessons/056-close-the-loop/index.md#test-the-loop).

## D

**dBm** { #dbm }
:   Radio signal strength in decibels compared with one milliwatt. Received
    signals are negative numbers, and every 10 dBm lower is ten times
    weaker. Taught in [Lesson 40](lessons/040-lora/index.md#the-idea).

**Dead zone** { #dead-zone }
:   A small range around a joystick's middle that counts as zero, so a stick
    at rest doesn't drift. Taught in
    [Lesson 26](lessons/026-joystick/index.md#the-idea).

**Debounce** { #debounce }
:   A button's contacts bounce for a few milliseconds when pressed or
    released. Debouncing waits until the reading has stayed the same, 20 ms
    in ADK, before believing it. Taught in
    [Lesson 2](lessons/002-buttons/index.md#the-idea).

**Diode** { #diode }
:   A part that lets current through one way only, from its anode to its
    banded cathode. Taught in
    [Lesson 3](lessons/003-reaction-duel/index.md#the-idea) and
    [E09](lessons/064-one-way-diode/index.md#try-both-directions).

**Divider** { #divider }
:   Two resistances in a row across a voltage. The point between them gives
    a fixed share of it: a knob's wiper, a light sensor's reading, or a 5 V
    signal brought down to 3.3 V for a radio. Taught in
    [Lesson 7](lessons/007-dimmer/index.md#the-idea),
    [Lesson 38](lessons/038-radio-messages/index.md#the-idea) and
    [E06](lessons/061-tap-a-divider/index.md#what-youll-build).

**Duty cycle** { #duty-cycle }
:   The share of the time a PWM pin is on: 64 out of 255 is on a quarter of
    the time. See [PWM](#pwm). Taught in
    [Lesson 4](lessons/004-mood-lamp/index.md#the-idea) and
    [E23](lessons/078-pwm-average/index.md#watch-and-measure).

## E

**EEPROM** { #eeprom }
:   Memory in the Mega that keeps what it holds when the power goes off,
    unlike RAM, which forgets. Taught in
    [Lesson 6](lessons/006-simon/index.md#if-it-doesnt-work).

**Event, state** { #event }
:   A state lasts as long as something is so: a button is pressed. An event
    is true once, at the moment it changes: the button was pressed. Taught
    in [Lesson 2](lessons/002-buttons/index.md#the-idea) and
    [Lesson 3](lessons/003-reaction-duel/index.md#how-the-game-works).

## F

**Failsafe** { #failsafe }
:   What a machine does by itself when it loses its link, such as a fan that
    stops when the radio goes quiet. Taught in
    [Lesson 45](lessons/045-remote-turret/index.md#the-idea).

**Filter** { #filter }
:   A circuit that lets slow changes through and smooths fast ones, such as
    a resistor and a capacitor. Taught in
    [E15](lessons/070-filter-and-phase/index.md#why-it-happens).

**Floating** { #floating }
:   An input joined to nothing. It picks up stray electricity and reads high
    or low at random; a [pull-up](#pull-up) cures it. Taught in
    [Lesson 2](lessons/002-buttons/index.md#the-idea).

**Flyback diode** { #flyback-diode }
:   A diode across a coil, in a buzzer, a motor or a relay. When the coil is
    switched off, its current carries on for a moment, and the diode gives
    it a safe loop. Taught in
    [Lesson 3](lessons/003-reaction-duel/index.md#the-idea) and
    [E12](lessons/067-coil-diode/index.md#try-it).

**FM** { #fm }
:   Frequency modulation: radio that carries sound by wobbling the wave's
    frequency in step with it. Taught in
    [Lesson 37](lessons/037-fm-radio/index.md#the-idea).

**Frequency, hertz** { #frequency }
:   How many times a second something repeats, measured in hertz (Hz): a
    note's pitch, or a wave on a scope. Taught in
    [Lesson 5](lessons/005-melody-maker/index.md#the-idea) and
    [E14](lessons/069-frequency-and-period/index.md#the-idea).

## G

**GND, ground** { #gnd }
:   The 0 V side of the circuit, where current returns to the Mega. Every
    voltage is measured from somewhere, usually GND. Taught in
    [Lesson 1](lessons/001-blink/index.md#the-idea) and
    [E01](lessons/056-close-the-loop/index.md#test-the-loop).

## H

**H-bridge** { #h-bridge }
:   Four switches round a motor that can send current through it either way,
    so it can spin either way. The L293D chip holds two. Taught in
    [Lesson 20](lessons/020-fan/index.md#the-idea).

**Hexadecimal** { #hexadecimal }
:   Counting in sixteens, with the digits 0 to 9 and then A to F. `0x45` is
    a number written that way. Taught in
    [Lesson 34](lessons/034-rfid/index.md#the-idea).

**Hysteresis** { #hysteresis }
:   Two thresholds with a gap between them, so a reading that wobbles near
    one doesn't flip the result back and forth. Taught in
    [Lesson 15](lessons/015-weather-station/index.md#the-idea).

## I

**I2C** { #i2c }
:   A bus that many chips share over two wires: SDA carries the data and SCL
    the clock. Each chip answers to its own address. Taught in
    [Lesson 28](lessons/028-tilt/index.md#the-idea).

**Inductor** { #inductor }
:   A coil of wire. It pushes back against any change in the current through
    it. Taught in
    [E11](lessons/066-inductor-current/index.md#why-it-happens).

**Infrared** { #infrared }
:   Light just beyond red, which eyes can't see. A remote control flickers
    it to send its codes. Taught in
    [Lesson 22](lessons/022-remote-control/index.md#the-idea).

**Interrupt** { #interrupt }
:   A signal that makes the Mega drop what it is doing for a moment to note
    a change on a pin. Taught in
    [Lesson 22](lessons/022-remote-control/index.md#if-it-doesnt-work).

## L

**Latch** { #latch }
:   A circuit that remembers: two gates that feed each other hold one bit
    after the button that set it is let go. Taught in
    [E20](lessons/075-set-reset-latch/index.md#why-it-remembers).

**Level shifter** { #level-shifter }
:   A small board that joins a 5 V board to a 3.3 V chip, so each side keeps
    its own voltage. A 3.3 V chip's pins must never see 5 V. Taught in
    [Lesson 28](lessons/028-tilt/index.md#the-idea).

**Library** { #library }
:   Ready-made code that a sketch brings in with `#include`. ADK is a
    library. Taught in [Lesson 1](lessons/001-blink/index.md#code-it).

**LoRa** { #lora }
:   Short for long range: radio that sends slowly, in chirps that sweep
    across a band of frequencies, so a message can reach a kilometer or
    more. Taught in [Lesson 40](lessons/040-lora/index.md#the-idea).

## M

**Margin** { #margin }
:   How far a received signal stands above the radio noise, in decibels. A
    LoRa modem reports it with each message. Taught in
    [Lesson 40](lessons/040-lora/index.md#the-idea).

**Mesh, node** { #mesh }
:   In a mesh, each radio, or node, passes on any message it hasn't heard
    before, so a message can hop from node to node further than one radio
    reaches. Taught in
    [Lesson 42](lessons/042-mesh-messenger/index.md#the-idea). In a circuit,
    a node is something else: see [Node](#node).

**millis ()** { #millis }
:   The number of milliseconds since the Mega started. Code that checks the
    time instead of stopping to wait is non-blocking: everything else keeps
    running. Taught in
    [Lesson 3](lessons/003-reaction-duel/index.md#the-idea) and
    [Lesson 11](lessons/011-four-digits/index.md#the-idea).

**Multiplexing** { #multiplexing }
:   Lighting several displays one at a time, so fast that the eye sees them
    all at once. Taught in
    [Lesson 11](lessons/011-four-digits/index.md#the-idea).

## N

**NAND, logic gate** { #nand }
:   A logic gate turns its input levels into an output level. A NAND gate's
    output is low only when both its inputs are high. Taught in
    [E19](lessons/074-nand-logic/index.md#why-it-happens).

**NEC** { #nec }
:   The code the kit's remote speaks: bursts of infrared and gaps whose
    lengths spell out an address and a command. Taught in
    [Lesson 22](lessons/022-remote-control/index.md#the-idea).

**Node** { #node }
:   In a circuit, every point joined by wire or by one breadboard strip is
    one node, however far apart a drawing shows its parts. [Read a
    schematic](electricity/schematics.md#follow-the-nodes) explains. For a
    radio in a mesh, see [Mesh, node](#mesh).

## O

**Ohm's law** { #ohms-law }
:   Current is the voltage across a part divided by its resistance: 3 V
    across 220 Ω is about 14 mA. Taught in
    [Lesson 1](lessons/001-blink/index.md#measure-it) and
    [E02](lessons/057-measure-across-and-through/index.md#find-the-current-through-the-path).

**Op-amp** { #op-amp }
:   An operational amplifier: a chip that changes its output until the
    voltages at its two inputs, + and −, nearly match. Taught in
    [E17](lessons/072-negative-feedback/index.md#why-it-happens).

**Oscilloscope** { #oscilloscope }
:   A meter that draws a voltage as it changes over time, so you can see a
    wave. [The scope primer](electricity/skills.md#scope-and-generator)
    shows how to read one. Taught in
    [E11](lessons/066-inductor-current/index.md#why-it-happens).

## P

**Parallel** { #parallel }
:   Paths side by side between the same two points. Each has the same
    voltage across it, and their currents add up. Taught in
    [E05](lessons/060-branches-in-parallel/index.md#the-idea).

**Period** { #period }
:   How long one cycle of a repeating signal takes. It is 1 divided by the
    frequency. Taught in
    [E14](lessons/069-frequency-and-period/index.md#the-idea).

**Persistence of vision** { #persistence-of-vision }
:   Your eye blends flashes too fast to follow into one steady picture,
    which is what [multiplexing](#multiplexing) relies on. Taught in
    [Lesson 11](lessons/011-four-digits/index.md#the-idea).

**Phase** { #phase }
:   How far one wave runs behind another. A filter's output has a phase lag:
    its peaks come late. Taught in
    [E15](lessons/070-filter-and-phase/index.md#why-it-happens).

**Photoresistor** { #photoresistor }
:   A resistor that light controls: the more light falls on it, the lower
    its resistance. Taught in
    [Lesson 8](lessons/008-light-meter/index.md#the-idea).

**Pitch, octave** { #pitch }
:   How high a sound seems, set by its frequency. Twice the frequency is the
    same note an octave higher. Taught in
    [Lesson 5](lessons/005-melody-maker/index.md#the-idea).

**Pixel** { #pixel }
:   One dot of a picture: on the LED matrix, one LED, found by its x and y.
    Taught in [Lesson 25](lessons/025-led-matrix/index.md#the-idea).

**Potentiometer, wiper** { #potentiometer }
:   A knob over a strip of resistance. Its middle leg, the wiper, slides
    along the strip and picks off a share of the voltage across it. Taught
    in [Lesson 7](lessons/007-dimmer/index.md#the-idea) and
    [E06](lessons/061-tap-a-divider/index.md#what-youll-build).

**Pull-up, pull-down** { #pull-up }
:   A resistor that holds an input high (a pull-up, to 5 V) or low (a
    pull-down, to GND) until a button or a sensor changes it. The Mega has
    pull-ups inside it, which ADK switches on for buttons. Taught in
    [Lesson 2](lessons/002-buttons/index.md#the-idea),
    [E19](lessons/074-nand-logic/index.md#why-it-happens) and
    [E20](lessons/075-set-reset-latch/index.md#rewire-the-two-buttons).

**PWM** { #pwm }
:   Pulse-width modulation: switching a pin on and off hundreds of times a
    second. The share of the time it is on sets the average, such as an
    LED's brightness. Taught in
    [Lesson 4](lessons/004-mood-lamp/index.md#the-idea) and
    [E23](lessons/078-pwm-average/index.md#watch-and-measure).

## R

**Rail** { #rail }
:   The long + and − rows along a breadboard's edges, each joined all the
    way along. A rail carries 5 V or GND only once a wire connects it.
    Taught in [Lesson 1](lessons/001-blink/index.md#build-it).

**RDS** { #rds }
:   A slow trickle of text that many FM stations send with their sound: the
    station's name and what is playing. Taught in
    [Lesson 37](lessons/037-fm-radio/index.md#the-idea).

**Real-time clock** { #real-time-clock }
:   A chip that keeps counting the date and time from its own coin cell
    while the Mega is off. Taught in
    [Lesson 32](lessons/032-real-time-clock/index.md#the-idea).

**Relay** { #relay }
:   A switch worked by an electromagnet, so a Mega's pin can switch a
    completely separate circuit. Taught in
    [Lesson 35](lessons/035-knocks-and-relays/index.md#the-idea).

**Resistance, ohm** { #resistance }
:   How strongly a part holds back current, measured in ohms (Ω). More
    resistance in a loop means less current. Taught in
    [Lesson 1](lessons/001-blink/index.md#the-idea) and
    [E03](lessons/058-resist-the-flow/index.md#change-one-resistor).

**RFID, UID** { #rfid }
:   Radio-frequency identification: a card with no battery that answers a
    reader's radio field with its UID, a number no other card should have.
    Taught in [Lesson 34](lessons/034-rfid/index.md#the-idea).

**Ripple** { #ripple }
:   The small, quick dips and bumps in a supply's voltage when what it
    powers draws changing current. A capacitor beside the load shrinks them.
    Taught in
    [E18](lessons/073-power-integrity/index.md#compare-the-ripples).

**Rotary encoder** { #rotary-encoder }
:   A knob that turns forever and reports each click and its direction,
    rather than where it points. Taught in
    [Lesson 29](lessons/029-menus/index.md#the-idea).

## S

**Sample** { #sample }
:   One reading of a voltage, taken at one moment. Taught in
    [E22](lessons/077-sampling/index.md#code-it).

**Schmitt trigger** { #schmitt-trigger }
:   A gate input with two switching levels, so a slow or noisy signal still
    switches cleanly, once. Taught in
    [E21](lessons/076-schmitt-clock/index.md#watch-the-clock).

**Serial port, UART, TX and RX** { #serial-port }
:   Two wires, one each way, that carry bytes as timed bits. A board's TX
    pin sends and its RX pin listens, so the wires cross over: TX to RX.
    Taught in [Lesson 40](lessons/040-lora/index.md#the-idea) and
    [E24](lessons/079-serial-link/index.md#why-it-happens).

**Series** { #series }
:   Parts one after another in a single path. The same current passes
    through each, and they share the voltage. Taught in
    [E04](lessons/059-resistors-in-series/index.md#why-it-happens).

**Servo** { #servo }
:   A motor you give an angle, not a speed: it turns to that angle and holds
    it. Taught in [Lesson 17](lessons/017-servo/index.md#the-idea).

**Seven-segment display** { #seven-segment }
:   A digit made of seven bar-shaped LEDs, a to g, plus a dot. Taught in
    [Lesson 10](lessons/010-dice/index.md#the-idea).

**Shift register** { #shift-register }
:   A chip, the 74HC595, that takes bits one at a time on three pins and
    sets eight outputs from them. Taught in
    [Lesson 10](lessons/010-dice/index.md#the-idea).

**Short circuit** { #short-circuit }
:   A path with almost no resistance, such as a wire from 5 V straight to
    GND. It lets far too much current flow and can damage the Mega. Taught
    in [Lesson 7](lessons/007-dimmer/index.md#measure-it).

**Sketch** { #sketch }
:   A program for an Arduino board. The Arduino IDE compiles it and uploads
    it to the Mega. Taught in
    [Lesson 1](lessons/001-blink/index.md#code-it).

**SPI** { #spi }
:   A fast bus: MOSI carries bits from the Mega to a chip, MISO carries them
    back, SCK times each bit, and a select line picks which chip listens.
    Taught in [Lesson 34](lessons/034-rfid/index.md#the-idea).

**State** { #state }
:   What a program is doing now, such as waiting or playing. A game is
    always in exactly one state, and each state decides which events matter.
    Taught in
    [Lesson 3](lessons/003-reaction-duel/index.md#how-the-game-works).

**Stepper motor** { #stepper }
:   A motor that moves in small, exact steps and stops after each one, so it
    can turn to a counted position. Taught in
    [Lesson 31](lessons/031-stepper/index.md#the-idea).

## T

**Thermistor** { #thermistor }
:   A resistor that heat controls: the warmer it is, the lower its
    resistance. Taught in
    [Lesson 14](lessons/014-thermometers/index.md#the-idea).

**Time constant** { #time-constant }
:   The time a capacitor charging through a resistor takes to reach about 63
    % of the way: R × C. Taught in
    [E08](lessons/063-time-an-rc-pair/index.md#the-idea).

**Transistor** { #transistor }
:   A switch with no moving parts. A small current into its base lets a
    larger one flow from its collector to its emitter. Taught in
    [Lesson 3](lessons/003-reaction-duel/index.md#the-idea) and
    [E10](lessons/065-control-with-a-transistor/index.md#try-it).

**Truth table** { #truth-table }
:   A list of every combination of a circuit's inputs, with the output each
    one gives. Taught in
    [E19](lessons/074-nand-logic/index.md#what-youll-build).

## U

**Ultrasound** { #ultrasound }
:   Sound too high for ears to hear. The distance sensor times its echo to
    measure how far away something is. Taught in
    [Lesson 19](lessons/019-parking-sensor/index.md#the-idea).

**Upload** { #upload }
:   Sending a compiled sketch from the computer to the Mega over USB. Taught
    in [Lesson 1](lessons/001-blink/index.md#upload-it).

## V

**Voltage follower** { #voltage-follower }
:   An op-amp whose output feeds straight back to its − input, so its output
    copies its input without loading it. Taught in
    [E17](lessons/072-negative-feedback/index.md#what-youll-build).

**Voltage, volt** { #voltage }
:   The push that drives current, measured in volts (V), and always between
    two points, such as a pin and GND. Taught in
    [Lesson 1](lessons/001-blink/index.md#the-idea) and
    [E02](lessons/057-measure-across-and-through/index.md#measure-across).
