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
make upload EXAMPLE=Lesson007AnalogInput PORT=/dev/ttyACM0
```

The named lesson target becomes available with the lesson. `BOARD_FQBN`
defaults to `arduino:avr:mega` and remains overridable.

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

## Validate and publish

```sh
make check
make quality
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
## monitor                   Interactively watch timestamped serial output.
## monitor-describe          Report the selected port's monitor settings.
## serial-log                Watch and save timestamped serial output.
## host-test                 Run deterministic host tests without exceptions.
## host-test-exceptions      Verify RAII cleanup in an exception environment.
## lessons                   Build all lesson PDFs.
## site                      Build the documentation site.
## site-serve                Serve the site locally.
## quality                   Run the complete supported release gate.
## legacy-check              Validate the frozen legacy implementation.
## clean                     Remove only the marked ADK build directory.
