# Getting started

You need three things: the kit, the Arduino IDE, and the ADK library. Setting
them up takes about fifteen minutes, and you only do it once. Then you're
ready for Lesson 1.

## 1. The kit

The course uses an **Arduino Mega 2560** and the parts in the Elegoo *Mega
2560 Most Complete Starter Kit*, with a few extras from the Elegoo
*37 in 1 Sensor Modules Kit*. [What's in the kit](kit.md) lists every part,
and which lessons use it.

Before you build anything, read [Safety](safety.md). It is short, and it
keeps you and your parts safe.

## 2. The Arduino IDE and ADK Boards

Download the free **Arduino IDE 2** from
[arduino.cc/en/software](https://www.arduino.cc/en/software) and install it.
It runs on Windows, macOS and Linux. The first time it opens, it installs
**Arduino AVR Boards**, the files for the Mega. Let it finish.

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
   **Install** under **ADK Boards**. The compiler is a large download, so
   give it a few minutes.
3. Check that **Arduino AVR Boards** says **Installed** too. ADK Boards uses
   its files, so install it if it doesn't.

## 3. The ADK library

1. Download the library as a ZIP file:
   [github.com/spincyc/adk](https://github.com/spincyc/adk) → **Code →
   Download ZIP**.
2. In the Arduino IDE, choose **Sketch → Include Library → Add .ZIP
   Library…** and pick the file you downloaded.
3. Check it worked: **File → Examples → Adk** now lists every lesson's
   sketch.

## 4. Connect the Mega

1. Plug the Mega into your computer with the USB cable. Its green **ON** LED
   lights.
2. Choose **Tools → Board → ADK Boards → ADK Mega 2560**. Not *Arduino Mega
   ADK*: that is a different board. If a sketch stops with "ADK needs
   C++23", this is the setting to check.
3. Choose the port under **Tools → Port**. On Windows it is a `COM` port; on
   macOS and Linux its name contains `usbmodem` or `ttyACM`.

## How a lesson works

Every lesson follows the same path, and every lesson is also a printable PDF.

| Section | What happens |
|---|---|
| **What you'll build** | A close-up of the finished circuit, so you know where you're heading. |
| **The idea** | The one new idea, with a question to predict the answer to before you try it. |
| **Build it** | A drawing of the whole bench, and the wiring step by step. |
| **Code it** | The sketch, and what each part of it does. |
| **Upload it** | What you should see when it works. |
| **If it doesn't work** | The usual mistakes, and how to spot them. |
| **Make it yours** | Challenges, from a small change to something new. |
| **Measure it** | For anyone with a multimeter: where to touch the probes, and what the meter should say. |

You're ready. The first lesson makes an LED blink.

[Start Lesson 1](lessons/01-blink/index.md){ .md-button .md-button--primary }

## From the command line

If you prefer a terminal to the IDE, the repository's `Makefile` builds and
uploads every example with [`arduino-cli`](https://arduino.github.io/arduino-cli/):

```sh
make examples                                     # compile every lesson
make upload EXAMPLE=Lesson01Blink PORT=/dev/ttyACM0
make monitor                                      # watch Serial output
make help                                         # everything else
```
