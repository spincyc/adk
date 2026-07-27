# Command-line workflow

The Make interface is authoritative. No supported lesson requires an IDE.

## Discover

```sh
make help
make boards
make monitor-describe PORT=/dev/ttyACM0
```

`make boards` reports connected devices. Copy the port exactly; the build does
not guess which board may be programmed.

## Build and upload

```sh
make arduino
make arduino-Lesson007AnalogInput
make upload-Lesson007AnalogInput PORT=/dev/ttyACM0
```

The named lesson target becomes available with the lesson. `BOARD_FQBN`
defaults to `arduino:avr:mega` and remains overridable. The generic form remains
available as `make upload EXAMPLE=Lesson007AnalogInput PORT=/dev/ttyACM0`.

## Observe

Circuit-native evidence is primary. Serial is an optional second view:

```sh
make monitor PORT=/dev/ttyACM0 BAUD=115200
make serial-log PORT=/dev/ttyACM0 BAUD=115200
make serial-log PORT=/dev/ttyACM0 SERIAL_LOG=build/serial/lesson007.log
```

`monitor` is interactive. `serial-log` timestamps the same stream and copies it
to `SERIAL_LOG`; stop either with Ctrl-C. A lesson must still be diagnosable
from its LED, status pattern, test point, meter reading, or other circuit-native
signal when no serial terminal is open.

## Record hardware evidence

Create the card before wiring, then append observations from the shell:

```sh
make hardware-card LESSON=007
make analog-note LESSON=007 \
    TEST_POINT=A0-GND EXPECTED_V=2.50 MEASURED_V=2.48 \
    RAW_SAMPLE=508 OUTPUT='D6 PWM near half duty'
make hardware-note LESSON=007 SIGNAL=D6 \
    PREDICTION='high impedance after shutdown' \
    OBSERVATION='measured with meter: ...' \
    INTERPRETATION='...'
make hardware-card-check LESSON=007
```

The generated card defaults to `build/hardware/lesson007.md`; set
`HARDWARE_RECORD` to select another path. `hardware-card-check` checks structure
only. It cannot validate a measurement or promote a lesson to hardware-verified.

Lesson 007 compares the potentiometer wiper voltage, raw ADC sample, PWM duty,
and LED brightness. Lesson 008 records raw and filtered values against the same
physical input so lag is observable. Lesson 009 records threshold crossings,
hysteresis, sensor-open and sensor-short behavior, and the corresponding RGB
diagnostic state. Each record separately proves resource acquisition and the
electrical safe state.

## Validate and publish

```sh
make check
make quality
make package-smoke
make lessons-check
make site-check
make legacy-check
```

Run `make quality` before committing a supported boundary.

## Available targets
## bootstrap                 Install stock Arch dependencies and the AVR core.
## boards                    List connected Arduino ports.
## arduino                   Compile every supported Mega example.
## arduino-<example>         Compile one named example.
## upload                    Compile and upload EXAMPLE to PORT.
## upload-<example>          Compile and upload one named example to PORT.
## monitor                   Interactively watch timestamped serial output.
## monitor-describe          Report the selected port's monitor settings.
## serial-log                Watch and save timestamped serial output.
## hardware-card             Create a draft Mega acceptance card for LESSON.
## hardware-note             Append one predict-observe-interpret record.
## analog-note               Append one voltage, ADC, and visible-output record.
## hardware-card-check       Check a hardware card's required structure.
## host-test                 Run deterministic host tests without exceptions.
## host-test-exceptions      Verify RAII cleanup in an exception environment.
## lessons                   Build all lesson PDFs.
## site                      Build the documentation site.
## site-serve                Serve the site locally.
## quality                   Run the complete supported release gate.
## package-smoke             Build every example from an installed clean archive.
## legacy-check              Validate the frozen legacy implementation.
## clean                     Remove only the marked ADK build directory.
