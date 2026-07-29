#!/usr/bin/env python3

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile


EXPECTED_FLASH = 21864
EXPECTED_SRAM = 3531
EXPECTED_OBJECT = 407
EXPECTED_STACK = 888
ISR_RESERVE = 64
RETURN_ALLOWANCE = 3


def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, **kwargs)


def toolBeside(buildDirectory, name):
    commands = json.loads(
        (buildDirectory / "compile_commands.json").read_text(encoding="utf-8")
    )
    entry = commands[0]
    compilerName = (
        entry["arguments"][0]
        if "arguments" in entry
        else entry["command"].split()[0]
    )
    tool = pathlib.Path(compilerName).parent / name
    if not tool.is_file():
        raise RuntimeError(f"{name} was not found beside {compilerName}")
    return tool


def sectionSizes(sizeTool, elfPath):
    output = subprocess.check_output([str(sizeTool), "-A", str(elfPath)], text=True)
    sections = {}
    for line in output.splitlines():
        columns = line.split()
        if len(columns) >= 2 and columns[0].startswith("."):
            sections[columns[0]] = int(columns[1])
    data = sections.get(".data", 0)
    return sections.get(".text", 0) + data, data + sections.get(".bss", 0)


def objectSize(compiler, nm, root, temporary):
    objectPath = temporary / "ir_translator_object_size.o"
    run(
        [
            str(compiler),
            "-c",
            "-mmcu=atmega2560",
            "-std=gnu++11",
            "-Os",
            "-fno-exceptions",
            "-fno-rtti",
            "-Isrc",
            str(root / "probes/ir_translator_object_size.cpp"),
            "-o",
            str(objectPath),
        ],
        cwd=root,
    )
    output = subprocess.check_output(
        [str(nm), "--print-size", "--size-sort", str(objectPath)], text=True
    )
    match = re.search(
        r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+irTranslatorObjectBytes$",
        output,
        re.MULTILINE,
    )
    if match is None:
        raise RuntimeError("object-size probe symbol was not found")
    return int(match.group(1), 16)


def stackUsage(compiler, root, temporary):
    sources = [
        root / "examples/Lesson054IrTranslator/Lesson054IrTranslator.ino",
        root / "src/inert_ir_translator.cpp",
        root / "src/known_ir_emission_policy.cpp",
    ]
    usages = {}
    for source in sources:
        objectPath = temporary / f"{source.stem}.o"
        run(
            [
                str(compiler),
                "-x",
                "c++",
                "-c",
                "-mmcu=atmega2560",
                "-std=gnu++11",
                "-Os",
                "-fno-lto",
                "-fstack-usage",
                "-fno-exceptions",
                "-fno-rtti",
                "-Isrc",
                str(source),
                "-o",
                str(objectPath),
            ],
            cwd=root,
        )
        usagePath = objectPath.with_suffix(".su")
        for line in usagePath.read_text(encoding="utf-8").splitlines():
            match = re.search(r"(?P<name>[^\t]+)\t(?P<bytes>\d+)\t", line)
            if match:
                usages[match.group("name")] = int(match.group("bytes"))

    def one(pattern):
        values = [value for name, value in usages.items() if pattern in name]
        if len(values) != 1:
            raise RuntimeError(f"expected one stack record containing {pattern!r}")
        return values[0]

    loop = one("void loop()")
    translatorPrepare = one("InertIrTranslator::prepareTranslation")
    emissionPrepare = one("KnownIrEmissionPolicy::prepare")
    return (
        loop
        + translatorPrepare
        + emissionPrepare
        + ISR_RESERVE
        + RETURN_ALLOWANCE,
        loop,
        translatorPrepare,
        emissionPrepare,
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    arguments = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    temporaryPath = pathlib.Path(tempfile.mkdtemp(prefix="adk-ir-probe."))
    try:
        buildDirectory = temporaryPath / "build"
        sketchDirectory = temporaryPath / "IrTranslatorMaximumComposition"
        sketchDirectory.mkdir()
        lesson = (
            root / "examples/Lesson054IrTranslator/Lesson054IrTranslator.ino"
        ).read_text(encoding="utf-8")
        insertion = (
            root / "probes/ir_translator_maximum_composition.inc"
        ).read_text(encoding="utf-8")
        lesson = lesson.replace(
            "#include <inert_ir_translator.h>",
            "#include <inert_ir_translator.h>\n"
            "#include <mega_pulse_capture_io.h>\n"
            "#include <pulse_capture.h>",
            1,
        )
        marker = (
            "    adk::InertIrTranslator translator (translatorConfig,\n"
            "                                       {capturedPulseWords,\n"
            "                                        adk::capturedIrPulseCapacity});\n"
        )
        if lesson.count(marker) != 1:
            raise RuntimeError("canonical Lesson 054 composition marker changed")
        lesson = lesson.replace(marker, marker + insertion, 1)
        (sketchDirectory / "IrTranslatorMaximumComposition.ino").write_text(
            lesson, encoding="utf-8"
        )
        run(
            [
                arguments.arduino_cli,
                "compile",
                "--fqbn",
                arguments.fqbn,
                "--library",
                str(root),
                "--build-path",
                str(buildDirectory),
                str(sketchDirectory),
            ],
            cwd=root,
        )
        elfFiles = list(buildDirectory.glob("*.elf"))
        if len(elfFiles) != 1:
            raise RuntimeError("expected exactly one probe ELF")

        sizeTool = toolBeside(buildDirectory, "avr-size")
        compiler = toolBeside(buildDirectory, "avr-g++")
        nm = toolBeside(buildDirectory, "avr-nm")
        flash, sram = sectionSizes(sizeTool, elfFiles[0])
        objectBytes = objectSize(compiler, nm, root, temporaryPath)
        stack, loop, translatorPrepare, emissionPrepare = stackUsage(
            compiler, root, temporaryPath
        )

        print(f"IR maximum composition: flash {flash} B; static SRAM {sram} B")
        print(f"InertIrTranslator object: {objectBytes} B")
        print(
            "Conservative stack: "
            f"{loop} + {translatorPrepare} + {emissionPrepare} + "
            f"{ISR_RESERVE} ISR + {RETURN_ALLOWANCE} return = {stack} B"
        )

        observed = (flash, sram, objectBytes, stack)
        expected = (EXPECTED_FLASH, EXPECTED_SRAM, EXPECTED_OBJECT, EXPECTED_STACK)
        if observed != expected:
            print(
                f"IR resource probe mismatch: observed {observed}, expected {expected}",
                file=sys.stderr,
            )
            return 1
        return 0
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"IR resource probe error: {error}", file=sys.stderr)
        return 1
    finally:
        shutil.rmtree(temporaryPath)


if __name__ == "__main__":
    sys.exit(main())
