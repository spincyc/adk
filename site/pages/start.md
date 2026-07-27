# Start on Arch Linux

## Install and verify

```sh
git clone https://github.com/spincyc/adk.git
cd adk
make bootstrap
make check
make arduino
make lessons
```

`bootstrap` uses stock Arch packages and installs the official Arduino AVR
core. Normal builds do not use the frozen legacy tree.

## Upload

Connect an Arduino Mega 2560 and identify its serial port:

```sh
arduino-cli board list
make upload EXAMPLE=Lesson001DigitalOutput PORT=/dev/ttyACM0
```

Disconnect every power source before changing wiring. Start with
[Lesson 001](lessons/001.md), which uses the board’s built-in LED and requires
no breadboard.

## Useful targets

| Target | Purpose |
|---|---|
| `make check` | Host tests, style, and site checks |
| `make arduino` | Compile every first-class Mega example |
| `make lessons` | Build printable PDFs |
| `make site-check` | Build and validate the publication tree |
| `make legacy-check` | Explicitly verify the unsupported preview |

See [Build locally](build.md) for variables and troubleshooting.
