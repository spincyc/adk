# Getting started

Start with the kit. For [E01: Close the Loop](lessons/056-close-the-loop/index.md),
the Mega supplies USB power to a circuit you build by hand. You do not need
the Arduino IDE, ADK Boards, a sketch or an upload. To follow the project
lessons from [Lesson 1](lessons/001-blink/index.md), set up the IDE and ADK
below. You only do that setup once.

!!! note "Not yet built on a real bench"
    ADK's sketches compile and its checks pass, but nobody has yet built
    the lessons on real hardware and recorded the result, so what each
    lesson says you'll see is a careful prediction. If your build doesn't
    match, the mistake may be the lesson's, not yours. Either way, please
    [report your build](builds.md).

## 1. The kit

The course uses an **Arduino Mega 2560** and the parts in the Elegoo *Mega
2560 Most Complete Starter Kit*, with a few extras from the Elegoo
*37 in 1 Sensor Modules Kit*. [What's in the kit](kit.md) lists every part
and the lesson that first uses it, the other equipment some lessons need,
and [what to buy](kit.md#what-to-buy) for each path, with rough costs.
Each lesson's own parts list says exactly what that lesson needs.

Before you build anything, read [Safety](safety.md). It is short, and it
keeps you and your parts safe.

## Before the first wire

1. Put the breadboard beside the Mega with **column 1 nearest the Mega**.
   Find the breadboard's **+** and **−** rails and the Mega's **5V** and
   **GND** labels. Follow the first build's drawing for the exact holes.
2. Find the LED's long and short legs and the **220 Ω resistor** (red, red,
   black, black, brown bands). Every LED in this course needs its resistor.
3. Keep USB **unplugged** while you place or move wires. Before plugging it
   in, compare each connection with the drawing, especially 5V and GND.

For the circuit-only start, go straight to [E01](lessons/056-close-the-loop/index.md)
after these checks. For the project lessons, continue with the setup below.

## 2. The Arduino IDE and ADK Boards

Download the free **Arduino IDE 2** from
[arduino.cc/en/software](https://www.arduino.cc/en/software) and install it.
It runs on Windows, macOS and Linux. ADK needs IDE 2 on one of those: it
doesn't support the older IDE 1.8, or the Arduino Cloud Editor that
Chromebooks use, which can't install ADK Boards. The first time the IDE
opens, it installs **Arduino AVR Boards**, the files for the Mega. Let it
finish.

ADK is written in a newer C++ than the IDE's own compiler understands, so
it comes with a board of its own: **ADK Boards**. It is the same Mega 2560,
with a newer compiler.

1. Choose **File → Preferences** (on a Mac, **Arduino IDE → Settings**).
   Paste this address into **Additional boards manager URLs**, then click
   **OK**:

    ```text
    https://spincyc.github.io/adk/package_adk_index.json
    ```

2. Choose **Tools → Board → Boards Manager…** and search for **ADK**. Click
   **Install** under **ADK Boards**. The compiler is a large download, about
   100 to 170 MB depending on the computer, so give it a few minutes.
3. Check that **Arduino AVR Boards** says **Installed** too. ADK Boards uses
   its files, so install it if it doesn't.

## 3. The ADK library

1. Download the library as a ZIP file:
   [github.com/spincyc/adk](https://github.com/spincyc/adk) → **Code →
   Download ZIP**.
2. In the Arduino IDE, choose **Sketch → Include Library → Add .ZIP
   Library…** and pick the file you downloaded.
3. Check it worked: **File → Examples → Adk → lessons** now lists every
   lesson's sketch, from `001-blink` on.

## 4. Connect the Mega

1. Plug the Mega into your computer with the USB cable. Its green **ON** LED
   lights.
2. Choose **Tools → Board → ADK Boards → ADK Mega 2560**. Not *Arduino Mega
   ADK*: that is a different board. If a sketch stops with "ADK needs
   C++23", this is the setting to check.
3. Choose the port under **Tools → Port**. On Windows it is a `COM` port; on
   macOS and Linux its name contains `usbmodem` or `ttyACM`, or `usbserial`
   or `ttyUSB` on a compatible board with a CH340 USB chip.

## If setup stalls

| What you see | Try this |
|---|---|
| The Mega's green **ON** light stays dark | Try another USB cable or computer port. |
| **ON** lights, but the circuit's LED stays dark | Unplug USB. Check the LED's direction, its resistor, and each wire against the lesson's drawing. |
| **ADK Boards** is missing from the board menu | Recheck the Boards Manager address above, then install ADK Boards and Arduino AVR Boards. |
| Installing ADK Boards stops partway | The compiler is 100 to 170 MB. On a slow connection, wait, then click **Install** again if it failed. |
| The port is missing or an upload fails | Use a USB **data** cable, select **ADK Mega 2560** and the Mega's port, then try again. A power-only cable can light **ON** but cannot upload. |
| On Linux, the port is greyed out or the upload says **Permission denied** | Let your account use serial ports: run `sudo usermod -aG dialout $USER` (on Arch Linux, `uucp` in place of `dialout`), then log out and back in. |
| No port appears for a compatible Mega | Many compatible boards use a CH340 USB chip in place of the Mega's own. Windows and newer macOS usually find its driver themselves; if not, install the CH340 driver from its maker, WCH. |
| The IDE says **ADK needs C++23** | Select **ADK Boards → ADK Mega 2560**, not **Arduino Mega ADK**. |
| You have Arduino IDE 1.8, or a Chromebook | ADK needs Arduino IDE 2 on Windows, macOS or Linux. The Cloud Editor that Chromebooks use can't install ADK Boards. |

If a part gets hot or smells, unplug USB at once and check for a wire joining
5V straight to GND. [Safety](safety.md) has the rules for later parts too.

## How a lesson works

Each lesson has a printable PDF. Project lessons include code and an upload;
passive electricity investigations work from their wired circuit and need
no upload. Both paths ask you to predict, build and check a result.
The time shown is an estimate. **Level 1** is a starter build with a few
connections; **Level 2** joins several parts or paths; **Level 3** has dense
wiring or needs more advanced instruments.

| Section | What happens |
|---|---|
| **What you'll build** | A close-up of the finished circuit, so you know where you're heading. |
| **The idea** | The one new idea, with a question to predict the answer to before you try it. |
| **Build it** | A drawing of the whole bench, and the wiring step by step. |
| **Code it** | In programmed builds, the sketch and what each part does. |
| **Upload it** | In programmed builds, what you should see when it works. |
| **If it doesn't work** | The usual mistakes, and how to spot them. |
| **Make it yours** | Challenges, from a small change to something new. |
| **Measure it** | For anyone with a multimeter: where to touch the probes, and what the meter should say. |

You're ready. The first lesson makes an LED blink.

[Start Lesson 1](lessons/001-blink/index.md){ .md-button .md-button--primary }

You can also follow the parallel [electricity investigations](electricity/index.md).
Their first build uses the same LED and shows what happens when its return
path is open. Later modules add a meter, then other parts and instruments;
the syllabus lists what each needs.

## Updating ADK

The ZIP file has no version number to choose: it is always the newest ADK
on GitHub, and the IDE doesn't update it for you. To update the library,
download the ZIP again and add it as before. If the IDE asks whether to
replace the library it already has, say yes. If it says the library is
already installed, close the IDE, delete the `adk-main` folder from the
`libraries` folder in your sketchbook (**File → Preferences** shows where
that is), then open the IDE and add the ZIP again.

When ADK Boards has a new version, Boards Manager shows **Update** under it.
Update the library and ADK Boards together.

## From the command line

If you prefer a terminal to the IDE, the repository's `Makefile` builds and
uploads every example with [`arduino-cli`](https://arduino.github.io/arduino-cli/):

```sh
make examples                            # compile every lesson
make upload-001-blink PORT=/dev/ttyACM0  # compile Lesson 1 and upload it
make lessons                             # every lesson's own targets
make monitor                             # watch Serial output
make help                                # everything else
```
