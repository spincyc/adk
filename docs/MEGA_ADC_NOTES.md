# Mega 2560 ADC constraints

This note is the hardware review basis for lessons 007--009. The supported
`AnalogInput` endpoint currently uses the Arduino AVR core's default AVCC
reference and single-ended reads.

- Mega pins A0--A15 are Arduino pin identifiers 54--69. The core subtracts 54
  and uses `MUX5` for ADC channels 8--15.
- A reading is a 10-bit code from 0 through 1023. One code interval is
  `Vref / 1024`; code 1023 represents the top interval below the reference, not
  an exact 5.000 V measurement.
- AVCC is the default reference. Measure the actual reference before converting
  codes to estimated voltage, and do not describe ADC resolution as accuracy.
- Use a 10 kOhm linear potentiometer in lesson 007. Its worst-case 2.5 kOhm
  Thevenin resistance remains below the datasheet's approximately 10 kOhm
  source-impedance recommendation.
- Keep the input between GND and the selected reference, use a common ground,
  and never let an external source drive an unpowered board.
- Do not drive AREF while AVCC or an internal reference is selected. Reference
  changes and multi-channel acquisition require a future ADC owner with an
  explicit settling policy.
- `analogRead()` is blocking. A normal conversion takes 13 ADC clocks; the
  first after enabling the ADC takes 25. High-impedance or rapidly multiplexed
  sources can need additional settling because the AVR core starts conversion
  immediately after changing channels.

Primary references:

- [Microchip ATmega640/1280/1281/2560/2561 complete datasheet, sections
  26.2--26.6](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf)
- [Arduino AVR core `wiring_analog.c`](https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring_analog.c)
- [Arduino Mega 2560 Rev3 board datasheet](https://docs.arduino.cc/resources/datasheets/A000067-datasheet.pdf)
