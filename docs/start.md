# Getting started

You need three things: the kit, the Arduino IDE, and the ADK library. Setting
them up takes about fifteen minutes, and then you're ready for
[Lesson 1](lessons/01-blink/index.md).

## 1. The kit

The course uses an **Arduino Mega 2560** and the parts in the Elegoo *Mega
2560 Most Complete Starter Kit*, with a few extras from the Elegoo
*37 in 1 Sensor Modules Kit*. [What's in the kit](kit.md) lists every part,
and which lessons use it.

## 2. The Arduino IDE

Download the free **Arduino IDE 2** from
[arduino.cc/en/software](https://www.arduino.cc/en/software) and install it.
It runs on Windows, macOS and Linux.

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
2. Choose **Tools → Board → Arduino AVR Boards → Arduino Mega or Mega 2560**.
3. Choose the port under **Tools → Port**. On Windows it is a `COM` port; on
   macOS and Linux its name contains `usbmodem` or `ttyACM`.

Now open [Lesson 1](lessons/01-blink/index.md).

## How a lesson works

Every lesson follows the same path, and every lesson is also a printable PDF.

| Section | What happens |
|---|---|
| **What you'll build** | The finished circuit, so you know where you're heading. |
| **The idea** | The one new idea, with a question to predict the answer to before you try it. |
| **Build it** | A drawing of the whole bench, a close-up of the breadboard, and step-by-step wiring. |
| **Code it** | The sketch, and what each part of it does. |
| **Upload it** | What you should see when it works. |
| **If it doesn't work** | The usual mistakes, and how to spot them. |
| **Make it yours** | Challenges, from a small change to something new. |

## From the command line

If you prefer a terminal, the repository's `Makefile` builds and uploads
every example with [`arduino-cli`](https://arduino.github.io/arduino-cli/):

```sh
make examples                                     # compile every lesson
make upload EXAMPLE=Lesson01Blink PORT=/dev/ttyACM0
make monitor                                      # watch Serial output
make help                                         # everything else
```
