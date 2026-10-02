# Sensors

`Dht11`, `Ds18b20`, `Ultrasonic` and `Mpu6050` share a reading pattern:
`measured ()` is true in the one update in which a reading finished, good
or not, and `ok ()` says whether that device's latest reading succeeded.
Each entry below explains what counts as success.

The analog sensors have different interfaces. `Thermistor` keeps its
latest smoothed temperature in `celsius ()` and `fahrenheit ()`, with no
`measured ()` or `ok ()`. `SoundSensor` supplies `level ()` and a
`measured ()` event every 50 ms, but no `ok ()` signal. For the clock,
read `Rtc::now ()`, check communication with `ok ()`, and check whether
it is ticking with `isRunning ()`; it has no `measured ()` event.

<!-- api thermistor.h Thermistor -->

<!-- api dht11.h Dht11 -->

<!-- api ds18b20.h Ds18b20 -->

<!-- api ultrasonic.h Ultrasonic -->

<!-- api sound_sensor.h SoundSensor -->

<!-- api ir_receiver.h IrReceiver -->

The kit remote's buttons are named in `adk::remote`: `power`, `volumeUp`,
`volumeDown`, `stop`, `back`, `play`, `forward`, `up`, `down`, `eq`,
`repeat` and `digit0` to `digit9`.

<!-- api rtc.h Rtc -->

<!-- api rtc.h DateTime -->

<!-- api mpu6050.h Mpu6050 -->

<!-- api rfid.h Rfid -->
