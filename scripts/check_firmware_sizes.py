#!/usr/bin/env python3

import argparse
import csv
import json
import pathlib
import subprocess
import sys


SCHEMA_VERSION = "1"
FLASH_CAPACITY = 253952
SRAM_CAPACITY = 8192


def readTsv(path):
    with path.open(newline="", encoding="utf-8") as source:
        return list(csv.DictReader(source, delimiter="\t"))


def writeTsv(path, fieldNames, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as destination:
        writer = csv.DictWriter(destination, fieldnames=fieldNames, delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)


def findTool(buildDirectory, toolName):
    commands = json.loads(
        (buildDirectory / "compile_commands.json").read_text(encoding="utf-8")
    )
    entry = commands[0]
    compilerName = entry["arguments"][0] if "arguments" in entry else entry["command"].split()[0]
    compiler = pathlib.Path(compilerName)
    tool = compiler.parent / toolName
    if not tool.is_file():
        raise RuntimeError(f"{toolName} was not found beside {compiler}")
    return tool


def coreIdentity(buildDirectory):
    options = json.loads(
        (buildDirectory / "build.options.json").read_text(encoding="utf-8")
    )
    hardwareFolders = options.get("hardwareFolders", "").split(",")
    versions = {
        pathlib.Path(folder).name
        for folder in hardwareFolders
        if folder
    }
    if len(versions) != 1:
        raise RuntimeError("unable to identify one Arduino AVR core version")
    return f"arduino-avr-{versions.pop()}"


def toolchainIdentity(compiler):
    output = subprocess.check_output(
        [str(compiler), "-dumpfullversion", "-dumpversion"],
        text=True,
    )
    return f"avr-gcc-{output.strip()}"


def sectionSizes(sizeTool, elfPath):
    output = subprocess.check_output([str(sizeTool), "-A", str(elfPath)], text=True)
    sections = {}
    for line in output.splitlines():
        columns = line.split()
        if len(columns) >= 2 and columns[0].startswith("."):
            sections[columns[0]] = int(columns[1])
    data = sections.get(".data", 0)
    text = sections.get(".text", 0)
    bss = sections.get(".bss", 0)
    return text + data, data, bss, data + bss


def measure(arguments):
    rows = []
    for example in arguments.examples:
        buildDirectory = arguments.build_root / example
        elfFiles = list(buildDirectory.glob("*.elf"))
        if len(elfFiles) != 1:
            raise RuntimeError(f"{example}: expected exactly one ELF artifact")
        sizeTool = findTool(buildDirectory, "avr-size")
        compiler = findTool(buildDirectory, "avr-g++")
        flash, data, bss, staticRam = sectionSizes(sizeTool, elfFiles[0])
        rows.append(
            {
                "schema": SCHEMA_VERSION,
                "example": example,
                "fqbn": arguments.fqbn,
                "core": coreIdentity(buildDirectory),
                "toolchain": toolchainIdentity(compiler),
                "flash_bytes": flash,
                "data_bytes": data,
                "bss_bytes": bss,
                "static_ram_bytes": staticRam,
            }
        )
    writeTsv(
        arguments.report,
        [
            "schema",
            "example",
            "fqbn",
            "core",
            "toolchain",
            "flash_bytes",
            "data_bytes",
            "bss_bytes",
            "static_ram_bytes",
        ],
        rows,
    )


def check(arguments):
    measured = {row["example"]: row for row in readTsv(arguments.report)}
    baselines = {row["example"]: row for row in readTsv(arguments.baseline)}
    expected = set(arguments.examples)
    errors = []
    if len(readTsv(arguments.report)) != len(measured):
        errors.append("measurement contains duplicate example rows")
    if len(readTsv(arguments.baseline)) != len(baselines):
        errors.append("baseline contains duplicate example rows")
    if set(measured) != expected:
        errors.append("measurement rows do not match supported examples")
    if set(baselines) != expected:
        errors.append("baseline rows do not match supported examples")

    for example in sorted(expected & measured.keys() & baselines.keys()):
        actual = measured[example]
        baseline = baselines[example]
        try:
            baselineVersion = int(baseline["baseline_version"])
        except (KeyError, ValueError):
            errors.append(f"{example}: invalid baseline version")
            baselineVersion = 0
        if baselineVersion < 1:
            errors.append(f"{example}: baseline version must be positive")
        for field in ("schema", "fqbn", "core", "toolchain"):
            if actual[field] != baseline[field]:
                errors.append(
                    f"{example}: {field} {actual[field]} != {baseline[field]}"
                )
        for resource, capacity in (
            ("flash", FLASH_CAPACITY),
            ("static_ram", SRAM_CAPACITY),
        ):
            actualBytes = int(actual[f"{resource}_bytes"])
            baselineBytes = int(baseline[f"{resource}_bytes"])
            budgetBytes = int(baseline[f"{resource}_budget_bytes"])
            growthBytes = actualBytes - baselineBytes
            growthPercent = 100.0 * growthBytes / max(baselineBytes, 1)
            if actualBytes > budgetBytes:
                errors.append(
                    f"{example}: {resource} {actualBytes} exceeds {budgetBytes}"
                )
            if growthBytes > 64 and growthPercent > 2.0:
                errors.append(
                    f"{example}: {resource} grew {growthBytes} B "
                    f"({growthPercent:.1f}%) without a baseline update"
                )
            usedPercent = 100.0 * actualBytes / capacity
            print(
                f"{example}: {resource} {actualBytes}/{budgetBytes} B "
                f"({usedPercent:.2f}% of Mega 2560)"
            )
    if errors:
        for error in errors:
            print(f"firmware size error: {error}", file=sys.stderr)
        return 1
    return 0


def update(arguments):
    measured = {row["example"]: row for row in readTsv(arguments.report)}
    baselines = {row["example"]: row for row in readTsv(arguments.baseline)}
    expected = set(arguments.examples)
    if set(measured) != expected or set(baselines) != expected:
        raise RuntimeError(
            "measurement and baseline rows must match supported examples"
        )
    rows = []
    for example in sorted(expected):
        actual = measured[example]
        previous = baselines[example]
        changed = (
            actual["flash_bytes"] != previous["flash_bytes"]
            or actual["static_ram_bytes"] != previous["static_ram_bytes"]
        )
        version = int(previous["baseline_version"]) + (1 if changed else 0)
        rows.append(
            {
                "schema": SCHEMA_VERSION,
                "baseline_version": version,
                "example": example,
                "fqbn": actual["fqbn"],
                "core": actual["core"],
                "toolchain": actual["toolchain"],
                "flash_bytes": actual["flash_bytes"],
                "flash_budget_bytes": previous["flash_budget_bytes"],
                "static_ram_bytes": actual["static_ram_bytes"],
                "static_ram_budget_bytes": previous["static_ram_budget_bytes"],
            }
        )
    writeTsv(
        arguments.baseline,
        [
            "schema",
            "baseline_version",
            "example",
            "fqbn",
            "core",
            "toolchain",
            "flash_bytes",
            "flash_budget_bytes",
            "static_ram_bytes",
            "static_ram_budget_bytes",
        ],
        rows,
    )


def parseArguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("measure", "check", "update"))
    parser.add_argument("--build-root", type=pathlib.Path)
    parser.add_argument("--report", type=pathlib.Path, required=True)
    parser.add_argument("--baseline", type=pathlib.Path)
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument("--examples", nargs="+", required=True)
    arguments = parser.parse_args()
    if arguments.command == "measure" and arguments.build_root is None:
        parser.error("measure requires --build-root")
    if arguments.command in ("check", "update") and arguments.baseline is None:
        parser.error(f"{arguments.command} requires --baseline")
    return arguments


def main():
    arguments = parseArguments()
    try:
        if arguments.command == "measure":
            measure(arguments)
            return 0
        if arguments.command == "check":
            return check(arguments)
        update(arguments)
        return 0
    except (OSError, RuntimeError, subprocess.CalledProcessError, ValueError) as error:
        print(f"firmware size error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
