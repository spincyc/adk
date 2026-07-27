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
to `SERIAL_LOG`; stop either with Ctrl-C. If the monitor cannot open, loses the
device, or rejects its configuration, `serial-log` preserves that nonzero
status even when the log sink succeeds. Bytes received before failure remain
in the log. A lesson must still be diagnosable from its LED, status pattern,
test point, meter reading, or other circuit-native signal when no serial
terminal is open.

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

## Explore the USB research models

The phase-one adapter is an experimental Linux USB/IP workflow. It uses a
temporary local ledger and is not the authoritative fenced mesh controller.
Setup, dependency diagnosis, tests, discovery, status, logging, and plan
targets do not attach, detach, bind, or load kernel modules:

```sh
make usb-matrix-setup
make usb-matrix-doctor
make usb-matrix-check
make usb-matrix-discover
make usb-matrix-status
make usb-matrix-log

make usb-export-plan USB_BUS_ID=1-2
make usb-assign-plan \
    USB_DEVICE_NODE=source-a USB_BUS_ID=1-2 USB_HOST_NODE=destination-a
make usb-matrix-dry-run \
    USB_DEVICE_NODE=source-a USB_BUS_ID=1-2 USB_HOST_NODE=destination-a
make usb-release-plan USB_HOST_NODE=destination-a USB_PORT=0
```

`usb-matrix-dry-run` is an alias for one assignment plan. Discovery invokes
read-only `usbip` commands and therefore requires the stock Arch `usbip`
package and access to the named nodes. Only `usb-export`, `usb-assign`, and
`usb-release`, plus their `usb-import` and `usb-route` aliases, request kernel
or USB/IP mutation. They never invoke `sudo`; arrange privileges separately.

The dynamic mesh controller and action adapter are host-only research models.
They model multiple source devices, destination slots, exclusive routes,
fencing, and break-before-make movement without accessing USB hardware:

```sh
make usb-mesh-check
make usb-mesh-test
```

There is no root `usb-mesh-dry-run` target yet. The future mesh CLI and its
plan/apply commands are documented as planned interfaces in
`docs/research/USB3_MESH_CLI.md`; they are not runnable commands.

The HDMI research model provides deterministic read-only reports for one
synthetic route. These commands never discover endpoints, transport media,
access a network, or touch hardware:

```sh
make hdmi-mesh-routes
make hdmi-mesh-route \
    HDMI_SOURCE=input:camera-a HDMI_DESTINATION=output:wall-center
make hdmi-mesh-trace HDMI_ROUTE=route:camera-to-wall
make hdmi-mesh-crc HDMI_ROUTE=route:camera-to-wall
make hdmi-mesh-latency HDMI_ROUTE=route:camera-to-wall
```

Each report begins with `fixture synthetic`. CRC and latency values verify the
model's inspection shape; they are not physical measurements.

## Validate and publish

```sh
make check
make quality
make headers-check
make package-smoke
make lessons-check
make site-check
make legacy-check
```

Run `make quality` before committing a supported boundary.
The default `make check` includes both HDMI control-model and shared
route-profile tests.

## Available targets
## bootstrap                 Install stock Arch dependencies and AVR core 1.8.8.
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
## host-test-sanitize        Run host tests with sanitizer instrumentation.
## headers-check             Compile every public header alone under strict C++11.
## size                      Report firmware sizes against reviewed baselines.
## size-check                Reject firmware-size budget regressions.
## size-update               Refresh reviewed firmware-size baselines.
## usb-matrix-check          Test the experimental Linux USB/IP adapter.
## usb-matrix-doctor         Check phase-one USB/IP command dependencies.
## usb-matrix-discover       Discover local and optionally remote USB/IP devices.
## usb-matrix-status         Print the non-authoritative temporary lease ledger.
## usb-matrix-dry-run        Preview one phase-one USB/IP assignment.
## usb-mesh-check            Test the host-only dynamic mesh research models.
## usb-mesh-test             Build and run the mesh controller and adapter tests.
## hdmi-mesh-check           Test the host-only HDMI control and observation models.
## hdmi-mesh-routes          List the deterministic synthetic HDMI route.
## hdmi-mesh-route           Inspect a synthetic route by input and output.
## hdmi-mesh-trace           Print a synthetic route's ordered state trace.
## hdmi-mesh-crc             Print synthetic model-only CRC evidence.
## hdmi-mesh-latency         Print synthetic model-only latency evidence.
## lessons                   Build all lesson PDFs.
## site                      Build the documentation site.
## site-serve                Serve the site locally.
## quality                   Run the complete supported release gate.
## package-smoke             Build every example from an installed clean archive.
## legacy-check              Validate the frozen legacy implementation.
## clean                     Remove only the marked ADK build directory.
