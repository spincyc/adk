#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
import pathlib
import re
import shlex
import shutil
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent.parent
BASE_PATH = ROOT / "scripts/check_escape_console_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("adk_exact_resource_probe", BASE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load the shared exact AVR probe implementation")
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)
BASE_COMPILE_SKETCH = probe.compile_sketch

BOARD_SRAM = 8192
ISR_RESERVE = 128
RESIDUAL_SRAM_TARGET = 4096
RESIDUAL_SRAM_HARD = 2048
CALLER_BUFFER_TARGET = 128
CALLER_BUFFER_HARD = 256
FINGERPRINT_CONTRACT = {
    "064": "thermal-gradient-resource-v4-l064",
    "065": "thermal-gradient-resource-v4-l065",
    "066": "thermal-gradient-resource-v4-l066",
}

BOUNDARY_064 = {
    "lesson": "064",
    "sketch": "examples/Lesson064OwnedSingleWireTransactions",
    "object_symbol": "oneWireTransactionPolicyObjectBytes",
    "flash_target": 10 * 1024,
    "flash_hard": 14 * 1024,
    "sram_target": 768,
    "sram_hard": 1024,
    "stack_target": 320,
    "stack_hard": 448,
    "object_target": 192,
    "object_hard": 256,
}

BOUNDARY_065 = {
    "lesson": "065",
    "sketch": "examples/Lesson065Qualified18B20ProbeSet",
    "object_symbol": "qualified18B20ProbeSetPolicyObjectBytes",
    "flash_target": 12 * 1024,
    "flash_hard": 16 * 1024,
    "sram_target": 1024,
    "sram_hard": 1536,
    "stack_target": 448,
    "stack_hard": 640,
    "object_target": 512,
    "object_hard": 768,
    "phase_frame_symbol": "replayCopiedCycle()",
}

BOUNDARY_066 = {
    "lesson": "066",
    "sketch": "examples/Lesson066ThermalGradientMapper",
    "object_symbol": "thermalGradientMapperObjectBytes",
    "flash_target": 16 * 1024,
    "flash_hard": 24 * 1024,
    "sram_target": 2048,
    "sram_hard": 3072,
    "stack_target": 768,
    "stack_hard": 1024,
    "object_target": 512,
    "object_hard": 768,
    "phase_frame_symbol": "loop",
}

BOUNDARIES = (BOUNDARY_064, BOUNDARY_065, BOUNDARY_066)
BOUNDARY_BY_LESSON = {
    boundary["lesson"]: boundary for boundary in BOUNDARIES
}

PUBLIC_VALUES_064 = (
    "OneWireRomCode",
    "OneWireTransactionConfig",
    "OneWireSearchState",
    "OneWireOperationRequest",
    "OneWireStepIntent",
    "OneWireStepReceipt",
    "OneWireTransactionSnapshot",
)

PUBLIC_ENUMS_064 = (
    "OneWireSupplyMode",
    "OneWireOperation",
    "OneWirePhase",
    "OneWireLineIntent",
    "OneWireTransactionQuality",
)

PUBLIC_VALUES_065 = (
    "Ds18b20NormalizedTransactionRef",
    "Ds18b20NormalizedSearchPass",
    "Ds18b20NormalizedProbeWitness",
    "Ds18b20CycleBuilder",
    "Ds18b20ProbeConfig",
    "QualifiedDs18b20Probe",
    "QualifiedDs18b20Snapshot",
    "QualifiedDs18b20SetConfig",
)

PUBLIC_ENUMS_065 = (
    "Ds18b20Resolution",
    "Ds18b20ProbeQuality",
    "Ds18b20SetQuality",
)

PUBLIC_VALUES_066 = (
    "ThermalMapperControl",
    "ThermalGradientPair",
    "ThermalMapperProbeIntent",
    "ThermalGradientIntent",
    "ThermalMapperConfig",
    "ThermalMapperEnvelope",
    "ThermalMapperRecordProbe",
    "ThermalMapperRecordImage",
    "ThermalMapperResult",
)

PUBLIC_ENUMS_066 = (
    "ThermalGradientHealth",
    "ThermalGradientQuality",
    "ThermalMapperPageKind",
)

LAYOUT_SYMBOLS = {}
LAYOUT_COMMANDS = ()
LAYOUT_SOURCE_HASHES = {}
OBJECT_COMMAND = ()
OBJECT_COMMANDS = {}
ORDINARY_EVIDENCE = {}
LOADED_REVIEWS = {}
LINKED_STORAGE = {}
COMPILE_DEPENDENCIES = {}
EXACT_COMMANDS = {}
AUTHORITY_MARKERS = ()
REVIEW_PATH_MARKERS = {}


def canonical_review_value(value, path_markers=None):
    markers = REVIEW_PATH_MARKERS if path_markers is None else path_markers
    if isinstance(value, dict):
        return {
            canonical_review_value(key, markers): canonical_review_value(
                item, markers
            )
            for key, item in value.items()
        }
    if isinstance(value, list):
        return [canonical_review_value(item, markers) for item in value]
    if isinstance(value, tuple):
        return tuple(canonical_review_value(item, markers) for item in value)
    if not isinstance(value, str):
        return value
    result = value
    for path, marker in sorted(
        markers.items(), key=lambda entry: len(entry[0]), reverse=True
    ):
        result = result.replace(path, marker)
    result = re.sub(
        r"/[^\" ]*/(?:\\.cache/arduino/)?sketches/[0-9A-Fa-f]+",
        "<arduino-sketch-cache>",
        result,
    )
    return result


def review_tool_identities(tools):
    return {
        name: identity
        for name, identity in tools.items()
        if name != "arduino_cli"
    }


def valid_review_tuple(metric, observed, target, hard):
    if metric == "residual_sram":
        return hard <= observed < target
    return target < observed <= hard


def canonical_review_string(value):
    result = canonical_review_value(value)
    result = result.replace(str(ROOT), "<repo>")
    return re.sub(r"/tmp/adk-[^/\" ]+", "<temporary>", result)


def canonical_link_recipe(value, sketch):
    result = canonical_review_string(value)
    canonical_sketch = canonical_review_string(str(sketch))
    result = result.replace(canonical_sketch, "<sketch>")
    result = re.sub(
        r"<repo>/(?:examples|extras/probes)/[^\" ]+",
        "<sketch>",
        result,
    )
    result = re.sub(
        r"<arduino-sketch-cache>/[^\" ]+\.ino\.(?:elf|hex|map)",
        "<arduino-sketch-cache>/<sketch-output>",
        result,
    )
    return result


def selected_boundaries(require_through):
    return tuple(
        boundary
        for boundary in BOUNDARIES
        if boundary["lesson"] <= require_through
    )


def fingerprint_source_paths(lesson):
    paths = [
        "scripts/check_escape_console_resource_probe.py",
        "scripts/check_thermal_gradient_resource_probe.py",
        "probes/thermal_gradient_object_sizes.cpp",
        "src/one_wire_transaction_policy.h",
        "src/one_wire_transaction_policy.cpp",
        "examples/Lesson064OwnedSingleWireTransactions/"
        "Lesson064OwnedSingleWireTransactions.ino",
    ]
    if lesson >= "065":
        paths.extend(
            (
                "probes/thermal_gradient_object_sizes_065.cpp",
                "src/qualified_18b20_probe_set_policy.h",
                "src/qualified_18b20_probe_set_policy.cpp",
                "examples/Lesson065Qualified18B20ProbeSet/"
                "Lesson065Qualified18B20ProbeSet.ino",
            )
        )
    if lesson >= "066":
        paths.extend(
            (
                "probes/thermal_gradient_object_sizes_066.cpp",
                "src/thermal_gradient_mapper.h",
                "src/thermal_gradient_mapper.cpp",
                "examples/Lesson066ThermalGradientMapper/"
                "Lesson066ThermalGradientMapper.ino",
            )
        )
    return tuple(paths)


def fingerprint_source_hashes(lesson):
    return {
        path: probe.sha256(ROOT / path)
        for path in fingerprint_source_paths(lesson)
    }


def canonical_compile_units(compile_units):
    units = canonical_review_value(compile_units)
    units = json.loads(normalized(units))
    units.sort(
        key=lambda unit: (
            unit.get("file", ""),
            json.dumps(unit.get("arguments", ()), separators=(",", ":")),
        )
    )
    return units


def boundary_compile_units(compile_units, lesson):
    units = json.loads(json.dumps(compile_units))
    if lesson == "066":
        units = canonical_compile_units(units)
    if lesson in ("064", "065"):
        units = [
            unit
            for unit in units
            if not unit.get("file", "").endswith(
                "/src/thermal_gradient_mapper.cpp"
            )
        ]
    return units


def boundary_compile_dependencies(dependencies, lesson):
    result = json.loads(json.dumps(dependencies))
    if lesson == "066":
        result = json.loads(normalized(result))
    if "manifests" not in result:
        return {
            key: boundary_compile_dependencies(value, lesson)
            for key, value in result.items()
        }
    if lesson not in ("064", "065"):
        return result
    excluded = (
        "/src/thermal_gradient_mapper.cpp",
        "/src/thermal_gradient_mapper.h",
    )
    result["manifests"] = [
        manifest
        for manifest in result["manifests"]
        if "thermal_gradient_mapper.cpp" not in manifest["path"]
    ]
    for manifest in result["manifests"]:
        manifest["dependencies"] = [
            path
            for path in manifest["dependencies"]
            if not any(marker in path for marker in excluded)
        ]
    result["dependency_hashes"] = {
        path: digest
        for path, digest in result["dependency_hashes"].items()
        if not any(marker in path for marker in excluded)
    }
    return result


def dependency_sha256(path):
    if path.name.endswith(".ino.cpp"):
        text = path.read_text(encoding="utf-8", errors="strict")
        canonical = canonical_review_string(text)
        return hashlib.sha256(canonical.encode("utf-8")).hexdigest()
    return probe.sha256(path)


def is_orchestration_dependency(path):
    return path.name.endswith(".ino.cpp.merged")


def is_orchestration_manifest(dependency_path, target):
    if dependency_path.name.endswith(".libsdetect.d"):
        return True
    identities = (dependency_path.name, target)
    return any(
        re.search(r"\.ino\.cpp\.merged(?:\.(?:d|o))?(?:$|[\"' ])", identity)
        for identity in identities
    )


def dependency_manifest(build_directory):
    dependencies = {}
    manifests = []
    for dependency_path in sorted(build_directory.rglob("*.d")):
        text = dependency_path.read_text(encoding="utf-8", errors="replace")
        flattened = text.replace("\\\n", " ")
        target, separator, payload = flattened.partition(":")
        if not separator:
            raise probe.ProbeError(
                f"compiler dependency manifest lacks a target: {dependency_path}"
            )
        if is_orchestration_manifest(dependency_path, target):
            continue
        entries = shlex.split(payload)
        canonical_entries = []
        for entry in entries:
            path = pathlib.Path(entry)
            if not path.is_absolute():
                path = (ROOT / path).resolve()
            if is_orchestration_dependency(path):
                continue
            canonical_path = canonical_review_string(str(path))
            canonical_entries.append(canonical_path)
            if not path.is_file():
                raise probe.ProbeError(
                    "compiler dependency is not an existing regular file: "
                    f"{entry} resolved as {path}"
                )
            digest = dependency_sha256(path)
            prior_digest = dependencies.get(canonical_path)
            if prior_digest is not None and prior_digest != digest:
                raise probe.ProbeError(
                    "canonical compiler dependencies have conflicting content: "
                    f"{canonical_path}"
                )
            dependencies[canonical_path] = digest
        manifests.append(
            {
                "path": canonical_review_string(
                    str(dependency_path.relative_to(build_directory))
                ),
                "dependencies": sorted(set(canonical_entries)),
            }
        )
    if not manifests:
        raise probe.ProbeError(
            f"compiler dependency manifests are absent: {build_directory}"
        )
    return {
        "manifests": sorted(
            manifests,
            key=lambda manifest: (
                manifest["path"],
                json.dumps(manifest["dependencies"], separators=(",", ":")),
            ),
        ),
        "dependency_hashes": dict(sorted(dependencies.items())),
    }


def compile_exact_sketch(arguments, root, temporary, boundary):
    build_directory, elf_path, command = BASE_COMPILE_SKETCH(
        arguments, root, temporary, boundary
    )
    nm = probe.tool_beside(build_directory, "avr-nm")
    for line in probe.output(
        (str(nm), "--print-size", "--size-sort", str(elf_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+([bBdD])\s+(.+)$",
            line,
        )
        if match:
            LINKED_STORAGE.setdefault(boundary["lesson"], {})[
                match.group(3)
            ] = int(match.group(1), 16)
    COMPILE_DEPENDENCIES.setdefault(boundary["lesson"], {})[
        "exact"
    ] = dependency_manifest(build_directory)
    EXACT_COMMANDS[boundary["lesson"]] = command
    return build_directory, elf_path, command


def read_symbols(nm, object_path):
    symbols = {}
    for line in probe.output(
        (str(nm), "--print-size", "--size-sort", str(object_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+"
            r"(\w+(?:Bytes|Alignment))$",
            line,
        )
        if match:
            symbols[match.group(2)] = int(match.group(1), 16)
    return symbols


def object_sizes(compiler, nm, root, temporary, unused):
    global LAYOUT_COMMANDS, LAYOUT_SOURCE_HASHES, LAYOUT_SYMBOLS
    global OBJECT_COMMAND, OBJECT_COMMANDS
    global REVIEW_PATH_MARKERS
    include_065 = any(
        boundary["lesson"] == "065" for boundary in probe.BOUNDARIES
    )
    include_066 = any(
        boundary["lesson"] == "066" for boundary in probe.BOUNDARIES
    )
    object_path = temporary / "thermal_gradient_object_sizes.o"
    object_command = [
        str(compiler),
        "-c",
        "-mmcu=atmega2560",
        "-std=gnu++11",
        "-Os",
        "-fno-lto",
        "-fno-exceptions",
        "-fno-rtti",
        "-Isrc",
        str(root / "probes/thermal_gradient_object_sizes.cpp"),
        "-o",
        str(object_path),
    ]
    probe.run(object_command, cwd=root)
    object_symbols = read_symbols(nm, object_path)
    OBJECT_COMMANDS = {"064": object_command}
    if include_065:
        object_path_065 = temporary / "thermal_gradient_object_sizes_065.o"
        object_command_065 = [
            *object_command[:-3],
            str(root / "probes/thermal_gradient_object_sizes_065.cpp"),
            "-o",
            str(object_path_065),
        ]
        probe.run(object_command_065, cwd=root)
        object_symbols.update(read_symbols(nm, object_path_065))
        OBJECT_COMMANDS["065"] = object_command_065
    if include_066:
        object_path_066 = temporary / "thermal_gradient_object_sizes_066.o"
        object_command_066 = [
            *object_command[:-3],
            str(root / "probes/thermal_gradient_object_sizes_066.cpp"),
            "-o",
            str(object_path_066),
        ]
        probe.run(object_command_066, cwd=root)
        object_symbols.update(read_symbols(nm, object_path_066))
        OBJECT_COMMANDS["066"] = object_command_066

    layout_path = temporary / "thermal_gradient_public_layouts.cpp"
    layout_source_064 = """
#include <one_wire_transaction_policy.h>
#define ADK_LAYOUT(type) \\
    unsigned char type##Bytes[sizeof (adk::type)]; \\
    unsigned char type##Alignment[alignof (adk::type)]; \\
    static_assert (__is_standard_layout (adk::type), \\
                   #type " must remain standard-layout"); \\
    static_assert (__is_trivially_copyable (adk::type), \\
                   #type " must remain trivially copyable"); \\
    static_assert (__has_trivial_destructor (adk::type), \\
                   #type " must remain trivially destructible")

ADK_LAYOUT (OneWireRomCode);
ADK_LAYOUT (OneWireTransactionConfig);
ADK_LAYOUT (OneWireSearchState);
ADK_LAYOUT (OneWireOperationRequest);
ADK_LAYOUT (OneWireStepIntent);
ADK_LAYOUT (OneWireStepReceipt);
ADK_LAYOUT (OneWireTransactionSnapshot);

unsigned char OneWireSupplyModeBytes[sizeof (adk::OneWireSupplyMode)];
unsigned char OneWireOperationBytes[sizeof (adk::OneWireOperation)];
unsigned char OneWirePhaseBytes[sizeof (adk::OneWirePhase)];
unsigned char OneWireLineIntentBytes[sizeof (adk::OneWireLineIntent)];
unsigned char OneWireTransactionQualityBytes
    [sizeof (adk::OneWireTransactionQuality)];

unsigned char oneWireRequestCallerBufferBytes
    [sizeof (adk::OneWireOperationRequest)];
unsigned char oneWireSearchCallerBufferBytes
    [sizeof (adk::OneWireSearchState)];
unsigned char oneWireIntentCallerBufferBytes
    [sizeof (adk::OneWireStepIntent)];
unsigned char oneWireReceiptCallerBufferBytes
    [sizeof (adk::OneWireStepReceipt)];
unsigned char oneWireSnapshotCallerBufferBytes
    [sizeof (adk::OneWireTransactionSnapshot)];
""".lstrip()
    layout_source_065 = """

#include <qualified_18b20_probe_set_policy.h>
ADK_LAYOUT (Ds18b20NormalizedTransactionRef);
ADK_LAYOUT (Ds18b20NormalizedSearchPass);
ADK_LAYOUT (Ds18b20NormalizedProbeWitness);
ADK_LAYOUT (Ds18b20CycleBuilder);
ADK_LAYOUT (Ds18b20ProbeConfig);
ADK_LAYOUT (QualifiedDs18b20Probe);
ADK_LAYOUT (QualifiedDs18b20Snapshot);
ADK_LAYOUT (QualifiedDs18b20SetConfig);
unsigned char Ds18b20ResolutionBytes[sizeof (adk::Ds18b20Resolution)];
unsigned char Ds18b20ProbeQualityBytes[sizeof (adk::Ds18b20ProbeQuality)];
unsigned char Ds18b20SetQualityBytes[sizeof (adk::Ds18b20SetQuality)];
unsigned char ds18b20BuilderCallerBufferBytes
    [sizeof (adk::Ds18b20CycleBuilder)];
unsigned char ds18b20SnapshotCallerBufferBytes
    [sizeof (adk::QualifiedDs18b20Snapshot)];
"""
    layout_source_066 = """

#include <thermal_gradient_mapper.h>
ADK_LAYOUT (ThermalMapperControl);
ADK_LAYOUT (ThermalGradientPair);
ADK_LAYOUT (ThermalMapperProbeIntent);
ADK_LAYOUT (ThermalGradientIntent);
ADK_LAYOUT (ThermalMapperConfig);
ADK_LAYOUT (ThermalMapperEnvelope);
ADK_LAYOUT (ThermalMapperRecordProbe);
ADK_LAYOUT (ThermalMapperRecordImage);
ADK_LAYOUT (ThermalMapperResult);
unsigned char ThermalGradientHealthBytes
    [sizeof (adk::ThermalGradientHealth)];
unsigned char ThermalGradientQualityBytes
    [sizeof (adk::ThermalGradientQuality)];
unsigned char ThermalMapperPageKindBytes
    [sizeof (adk::ThermalMapperPageKind)];
unsigned char thermalMapperEnvelopeCallerBufferBytes
    [sizeof (adk::ThermalMapperEnvelope)];
unsigned char thermalMapperResultCallerBufferBytes
    [sizeof (adk::ThermalMapperResult)];
"""
    layout_source = layout_source_064
    if include_065:
        layout_source += layout_source_065
    if include_066:
        layout_source += layout_source_066
    layout_path.write_text(layout_source, encoding="utf-8")
    layout_object = temporary / "thermal_gradient_public_layouts.o"
    layout_command = [
        str(compiler),
        "-c",
        "-mmcu=atmega2560",
        "-std=gnu++11",
        "-Os",
        "-fno-lto",
        "-fno-exceptions",
        "-fno-rtti",
        "-Isrc",
        str(layout_path),
        "-o",
        str(layout_object),
    ]
    probe.run(layout_command, cwd=root)

    host_compiler = shutil.which("c++")
    if host_compiler is None:
        raise probe.ProbeError("host C++ compiler is unavailable for trait checks")
    REVIEW_PATH_MARKERS[host_compiler] = "<host-cxx>"
    trait_path = temporary / "thermal_gradient_public_traits.cpp"
    trait_source_064 = """
#include <one_wire_transaction_policy.h>
#include <type_traits>

static_assert (
    !std::is_copy_constructible<adk::OneWireTransactionPolicy>::value,
    "OneWireTransactionPolicy must remain non-copyable");
static_assert (
    !std::is_move_constructible<adk::OneWireTransactionPolicy>::value,
    "OneWireTransactionPolicy must remain non-movable");
static_assert (
    !std::is_copy_assignable<adk::OneWireTransactionPolicy>::value,
    "OneWireTransactionPolicy must remain non-copy-assignable");
static_assert (
    !std::is_move_assignable<adk::OneWireTransactionPolicy>::value,
    "OneWireTransactionPolicy must remain non-move-assignable");
""".lstrip()
    trait_source_065 = """
#include <qualified_18b20_probe_set_policy.h>
static_assert (
    !std::is_copy_constructible<adk::Qualified18B20ProbeSetPolicy>::value,
    "Qualified18B20ProbeSetPolicy must remain non-copyable");
static_assert (
    !std::is_move_constructible<adk::Qualified18B20ProbeSetPolicy>::value,
    "Qualified18B20ProbeSetPolicy must remain non-movable");
static_assert (
    !std::is_copy_assignable<adk::Qualified18B20ProbeSetPolicy>::value,
    "Qualified18B20ProbeSetPolicy must remain non-copy-assignable");
static_assert (
    !std::is_move_assignable<adk::Qualified18B20ProbeSetPolicy>::value,
    "Qualified18B20ProbeSetPolicy must remain non-move-assignable");
"""
    trait_source_066 = """
#include <thermal_gradient_mapper.h>
static_assert (
    !std::is_copy_constructible<adk::ThermalGradientMapper>::value,
    "ThermalGradientMapper must remain non-copyable");
static_assert (
    !std::is_move_constructible<adk::ThermalGradientMapper>::value,
    "ThermalGradientMapper must remain non-movable");
static_assert (
    !std::is_copy_assignable<adk::ThermalGradientMapper>::value,
    "ThermalGradientMapper must remain non-copy-assignable");
static_assert (
    !std::is_move_assignable<adk::ThermalGradientMapper>::value,
    "ThermalGradientMapper must remain non-move-assignable");
"""
    trait_source = trait_source_064
    if include_065:
        trait_source += trait_source_065
    if include_066:
        trait_source += trait_source_066
    trait_path.write_text(trait_source, encoding="utf-8")
    trait_command = [
        host_compiler,
        "-fsyntax-only",
        "-std=c++11",
        "-Isrc",
        str(trait_path),
    ]
    probe.run(trait_command, cwd=root)
    LAYOUT_SYMBOLS = read_symbols(nm, layout_object)
    LAYOUT_SOURCE_HASHES = {
        "064": {
            "layouts_sha256": hashlib.sha256(
                layout_source_064.encode("utf-8")
            ).hexdigest(),
            "traits_sha256": hashlib.sha256(
                trait_source_064.encode("utf-8")
            ).hexdigest(),
        }
    }
    if include_065:
        LAYOUT_SOURCE_HASHES["065"] = {
            "layouts_sha256": hashlib.sha256(
                layout_source_065.encode("utf-8")
            ).hexdigest(),
            "traits_sha256": hashlib.sha256(
                trait_source_065.encode("utf-8")
            ).hexdigest(),
        }
    if include_066:
        LAYOUT_SOURCE_HASHES["066"] = {
            "layouts_sha256": hashlib.sha256(
                layout_source_066.encode("utf-8")
            ).hexdigest(),
            "traits_sha256": hashlib.sha256(
                trait_source_066.encode("utf-8")
            ).hexdigest(),
        }
    OBJECT_COMMAND = object_command
    LAYOUT_COMMANDS = (layout_command, trait_command)
    return object_symbols, object_command


def normalized(value):
    text = json.dumps(
        canonical_review_value(value), sort_keys=True, separators=(",", ":")
    )
    text = text.replace(str(ROOT), "<repo>")
    return re.sub(r"/tmp/adk-[^/\" ]+", "<temporary>", text)


def compile_ordinary(arguments, boundaries):
    global REVIEW_PATH_MARKERS
    temporary = pathlib.Path(tempfile.mkdtemp(prefix="adk-thermal-ordinary."))
    try:
        for boundary in boundaries:
            lesson = boundary["lesson"]
            sketch = ROOT / boundary["sketch"]
            build = temporary / f"lesson-{lesson}"
            command = [
                arguments.arduino_cli,
                "compile",
                "--fqbn",
                arguments.fqbn,
                "--library",
                str(ROOT),
                "--build-path",
                str(build),
                str(sketch),
            ]
            probe.run(command, cwd=ROOT)
            elf_paths = list(build.glob("*.elf"))
            if len(elf_paths) != 1:
                raise probe.ProbeError(
                    f"ordinary Lesson {lesson} build produced "
                    f"{len(elf_paths)} ELF files"
                )
            size_tool = probe.tool_beside(build, "avr-size")
            flash, static_sram = probe.section_sizes(size_tool, elf_paths[0])
            compile_database = build / "compile_commands.json"
            compile_units = json.loads(
                compile_database.read_text(encoding="utf-8")
            )
            flattened = " ".join(
                argument
                for unit in compile_units
                for argument in unit.get("arguments", ())
            )
            if "-DF_CPU=16000000L" not in flattened:
                raise probe.ProbeError(
                    f"ordinary Lesson {lesson} build does not resolve "
                    "F_CPU=16000000L"
                )
            ORDINARY_EVIDENCE[lesson] = {
                "command": command,
                "flash_bytes": flash,
                "static_sram_bytes": static_sram,
                "elf_sha256": probe.sha256(elf_paths[0]),
                "compile_units": canonical_compile_units(compile_units),
            }
            COMPILE_DEPENDENCIES.setdefault(lesson, {})[
                "ordinary"
            ] = dependency_manifest(build)

        last_lesson = boundaries[-1]["lesson"]
        compile_units = json.loads(
            (
                temporary
                / f"lesson-{last_lesson}"
                / "compile_commands.json"
            ).read_text(encoding="utf-8")
        )
        flattened = " ".join(
            argument
            for unit in compile_units
            for argument in unit.get("arguments", ())
        )
        core_document = json.loads(
            probe.output(
                (
                    arguments.arduino_cli,
                    "core",
                    "list",
                    "--format",
                    "json",
                ),
                stderr=subprocess.STDOUT,
            )
        )
        avr_platforms = [
            platform
            for platform in core_document.get("platforms", ())
            if platform.get("id") == "arduino:avr"
        ]
        if len(avr_platforms) != 1:
            raise probe.ProbeError("installed Arduino AVR core is ambiguous")
        core_version = avr_platforms[0].get("installed_version")
        if not core_version:
            raise probe.ProbeError("Arduino AVR core version is unavailable")
        properties_command = [
            arguments.arduino_cli,
            "compile",
            "--fqbn",
            arguments.fqbn,
            "--show-properties=expanded",
            str(ROOT / boundaries[-1]["sketch"]),
        ]
        properties = {}
        for line in probe.output(
            properties_command, stderr=subprocess.STDOUT
        ).splitlines():
            if "=" in line:
                key, value = line.split("=", 1)
                properties[key] = value
        link_recipe = properties.get("recipe.c.combine.pattern")
        if not link_recipe:
            raise probe.ProbeError("resolved AVR linker recipe is unavailable")
        compiler = pathlib.Path(compile_units[0]["arguments"][0])
        linker = compiler.parent / "avr-gcc"
        if not linker.is_file():
            raise probe.ProbeError("resolved AVR linker executable is unavailable")
        platform_roots = {
            str(pathlib.Path(unit["file"]).parents[2])
            for unit in compile_units
            if "/cores/arduino/" in unit.get("file", "")
        }
        if len(platform_roots) != 1:
            raise probe.ProbeError(
                "ordinary Lesson 064 build has an ambiguous AVR platform root"
            )
        REVIEW_PATH_MARKERS = {
            str(compiler.parent.parent): "<avr-toolchain>",
            platform_roots.pop(): "<arduino-avr-platform>",
            arguments.arduino_cli: "<arduino-cli>",
        }
        shared = {
            "core_package": "arduino:avr",
            "core_version": core_version,
            "f_cpu_hz": 16000000,
            "linker_executable": canonical_review_value(str(linker)),
            "linker_version": probe.tool_version(linker),
            "resolved_link_recipe": canonical_review_value(link_recipe),
        }
        for evidence in ORDINARY_EVIDENCE.values():
            evidence.update(shared)
        for boundary in boundaries:
            lesson_properties_command = [
                arguments.arduino_cli,
                "compile",
                "--fqbn",
                arguments.fqbn,
                "--show-properties=expanded",
                str(ROOT / boundary["sketch"]),
            ]
            lesson_properties = {}
            for line in probe.output(
                lesson_properties_command, stderr=subprocess.STDOUT
            ).splitlines():
                if "=" in line:
                    key, value = line.split("=", 1)
                    lesson_properties[key] = value
            lesson_link_recipe = lesson_properties.get(
                "recipe.c.combine.pattern"
            )
            if canonical_link_recipe(
                lesson_link_recipe, ROOT / boundary["sketch"]
            ) != canonical_link_recipe(
                link_recipe, ROOT / boundaries[-1]["sketch"]
            ):
                raise probe.ProbeError(
                    f"Lesson {boundary['lesson']} resolves a different "
                    "AVR linker recipe"
                )
            ORDINARY_EVIDENCE[boundary["lesson"]].update(
                {
                    "properties_command": lesson_properties_command,
                    "resolved_link_recipe": canonical_link_recipe(
                        lesson_link_recipe, ROOT / boundary["sketch"]
                    ),
                }
            )
    finally:
        shutil.rmtree(temporary)


def load_reviews(root, review_path):
    global AUTHORITY_MARKERS, LOADED_REVIEWS
    path = root / review_path
    selected_lessons = {
        boundary["lesson"] for boundary in probe.BOUNDARIES
    }
    document = json.loads(path.read_text(encoding="utf-8"))
    if (
        set(document) != {"reviews", "schema"}
        or type(document.get("schema")) is not int
        or document.get("schema") != 1
        or not isinstance(document.get("reviews"), list)
    ):
        raise probe.ProbeError(f"invalid target-miss review document: {path}")
    required = {
        "lesson",
        "metric",
        "observed_bytes",
        "target_bytes",
        "hard_bytes",
        "authority",
        "disposition",
        "rationale",
        "reviewer",
        "fingerprint_sha256",
    }
    authority = (
        "docs/design/LESSONS_064_066_THERMAL_MAPPER_PLAN.md"
        "#resource-budgets"
    )
    authority_text = (root / authority.split("#", 1)[0]).read_text(encoding="utf-8")
    heading = "## Resource budgets"
    start = authority_text.find(heading)
    end = authority_text.find("\n## ", start + len(heading))
    if start < 0:
        raise probe.ProbeError("thermal resource-review authority section is absent")
    if end < 0:
        end = len(authority_text)
    reviews = {}
    supported_metrics = {
        "flash",
        "static_sram",
        "synchronous_stack",
        "object",
        "ordinary_flash",
        "ordinary_static_sram",
        "request_caller_buffer",
        "search_caller_buffer",
        "intent_caller_buffer",
        "receipt_caller_buffer",
        "snapshot_caller_buffer",
        "builder_caller_buffer",
        "recurring_owned_storage",
        "lifetime_peak_storage",
        "envelope_caller_buffer",
        "result_caller_buffer",
        "recurring_composition_storage",
        "composition_lifetime_peak_storage",
        "residual_sram",
    }
    authority_section = authority_text[start:end]
    outside_fence = []
    fence = None
    for line in authority_section.splitlines():
        if fence is None:
            opening = re.match(r"^ {0,3}(`{3,}|~{3,})(.*)$", line)
            if opening and not (
                opening.group(1)[0] == "`" and "`" in opening.group(2)
            ):
                fence = (opening.group(1)[0], len(opening.group(1)))
                outside_fence.append("")
            else:
                outside_fence.append(line)
        else:
            closing = re.match(
                rf"^ {{0,3}}{re.escape(fence[0])}{{{fence[1]},}}[ \t]*$",
                line,
            )
            if closing:
                fence = None
            outside_fence.append("")
    authority_section = "\n".join(outside_fence)
    marker_pattern = re.compile(
        r"^Resource-review: lesson=(064|065|066) metric=([a-z_]+) "
        r"observed=([0-9]+) target=([0-9]+) hard=([0-9]+) "
        r"disposition=accepted-target-miss$"
    )
    authority_markers = {}
    for line in outside_fence:
        stripped = line.strip()
        if not stripped.startswith("Resource-review: lesson="):
            continue
        marker_lesson = re.match(
            r"^Resource-review: lesson=([0-9]{3})\b", stripped
        )
        if (
            marker_lesson is not None
            and marker_lesson.group(1) in BOUNDARY_BY_LESSON
            and marker_lesson.group(1) not in selected_lessons
        ):
            continue
        match = marker_pattern.fullmatch(stripped)
        if match is None:
            raise probe.ProbeError(
                f"malformed thermal resource-review marker: {stripped}"
            )
        lesson_metric = (match.group(1), match.group(2))
        if lesson_metric in authority_markers:
            raise probe.ProbeError(
                f"duplicate thermal resource-review marker: {lesson_metric}"
            )
        authority_markers[lesson_metric] = stripped
    AUTHORITY_MARKERS = tuple(
        authority_markers[key] for key in sorted(authority_markers)
    )
    for review in document["reviews"]:
        if (
            isinstance(review, dict)
            and review.get("lesson") in BOUNDARY_BY_LESSON
            and review.get("lesson") not in selected_lessons
        ):
            continue
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if (
            review["lesson"] not in BOUNDARY_BY_LESSON
            or review["authority"] != authority
            or review["disposition"] != "accepted-target-miss"
        ):
            raise probe.ProbeError(f"invalid target-miss review: {review}")
        for field in ("observed_bytes", "target_bytes", "hard_bytes"):
            if type(review[field]) is not int or review[field] < 0:
                raise probe.ProbeError(
                    f"target-miss review has invalid {field}: {review}"
                )
        if not valid_review_tuple(
            review["metric"],
            review["observed_bytes"],
            review["target_bytes"],
            review["hard_bytes"],
        ):
            raise probe.ProbeError(
                f"target-miss review tuple is not a reviewable miss: {review}"
            )
        for field in ("metric", "rationale", "reviewer"):
            if type(review[field]) is not str or not review[field].strip():
                raise probe.ProbeError(
                    f"target-miss review has invalid {field}: {review}"
                )
        if review["metric"] not in supported_metrics:
            raise probe.ProbeError(
                f"target-miss review names unsupported metric: {review}"
            )
        if (
            type(review["fingerprint_sha256"]) is not str
            or re.fullmatch(r"[0-9a-f]{64}", review["fingerprint_sha256"])
            is None
        ):
            raise probe.ProbeError(
                f"target-miss review fingerprint is invalid: {review}"
            )
        marker = (
            f"Resource-review: lesson={review['lesson']} "
            f"metric={review['metric']} "
            f"observed={review['observed_bytes']} "
            f"target={review['target_bytes']} hard={review['hard_bytes']} "
            f"disposition={review['disposition']}"
        )
        if authority_markers.get(
            (review["lesson"], review["metric"])
        ) != marker:
            raise probe.ProbeError(
                f"target-miss review marker is absent from {review['authority']}: "
                f"{marker}"
            )
        key = (review["lesson"], review["metric"])
        if key in reviews:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        reviews[key] = review
    LOADED_REVIEWS = reviews
    base_metrics = {"flash", "static_sram", "synchronous_stack", "object"}
    return {
        key: review for key, review in reviews.items() if key[1] in base_metrics
    }


def apply_enriched_reviews(state, boundary):
    measurements = state["measurements"]
    limits = {
        "ordinary_flash": (
            measurements["ordinary_flash_bytes"],
            boundary["flash_target"],
            boundary["flash_hard"],
        ),
        "ordinary_static_sram": (
            measurements["ordinary_static_sram_bytes"],
            boundary["sram_target"],
            boundary["sram_hard"],
        ),
    }
    if boundary["lesson"] == "064":
        for metric, key in (
            ("request_caller_buffer", "request_bytes"),
            ("search_caller_buffer", "search_bytes"),
            ("intent_caller_buffer", "intent_bytes"),
            ("receipt_caller_buffer", "receipt_bytes"),
            ("snapshot_caller_buffer", "snapshot_bytes"),
        ):
            limits[metric] = (
                measurements["caller_buffers"][key],
                CALLER_BUFFER_TARGET,
                CALLER_BUFFER_HARD,
            )
    elif boundary["lesson"] == "065":
        limits["builder_caller_buffer"] = (
            measurements["caller_buffers"]["builder_bytes"],
            448,
            512,
        )
        limits["snapshot_caller_buffer"] = (
            measurements["caller_buffers"]["snapshot_bytes"],
            256,
            512,
        )
        recurring = (
            measurements["object_bytes"]
            + measurements["caller_buffers"]["builder_bytes"]
        )
        phase_peak = max(
            measurements["caller_buffers"]["snapshot_bytes"],
            LAYOUT_SYMBOLS["OneWireTransactionSnapshotBytes"]
            + LAYOUT_SYMBOLS["OneWireSearchStateBytes"],
        )
        limits["recurring_owned_storage"] = (recurring, 960, 1280)
        limits["lifetime_peak_storage"] = (
            recurring + phase_peak,
            1216,
            1792,
        )
    else:
        for metric, key in (
            ("envelope_caller_buffer", "envelope_bytes"),
            ("result_caller_buffer", "result_bytes"),
        ):
            limits[metric] = (
                measurements["caller_buffers"][key],
                256,
                384,
            )
        limits["recurring_composition_storage"] = (
            measurements["lifetime_placement"]["recurring_total_bytes"],
            2048,
            3072,
        )
        limits["composition_lifetime_peak_storage"] = (
            measurements["lifetime_placement"]["lifetime_peak_bytes"],
            2560,
            3840,
        )
        limits["residual_sram"] = (
            measurements["residual_sram_bytes"],
            RESIDUAL_SRAM_TARGET,
            RESIDUAL_SRAM_HARD,
        )
    supported = {
        "flash",
        "static_sram",
        "synchronous_stack",
        "object",
        *limits,
    }
    for (lesson, metric), review in LOADED_REVIEWS.items():
        if lesson != boundary["lesson"]:
            continue
        if metric not in supported:
            raise probe.ProbeError(
                f"unsupported reviewed metric for Lesson {lesson}: {metric}"
            )
        if metric not in limits:
            continue
        expected = limits[metric]
        reviewed = (
            review["observed_bytes"],
            review["target_bytes"],
            review["hard_bytes"],
        )
        if reviewed != expected or state["gates"][metric] != "review-required":
            raise probe.ProbeError(
                f"stale target-miss review for Lesson {lesson} {metric}: "
                f"reviewed {reviewed}, measured {expected}"
            )
        state["gates"][metric] = "reviewed-target-miss"
        state.setdefault("accepted_reviews", []).append(review)


def enrich_evidence(evidence_path):
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    if (
        not report.get("boundaries")
        or "measurements" not in report["boundaries"][0]
        or not LAYOUT_SYMBOLS
    ):
        return 1
    ordinary_064 = ORDINARY_EVIDENCE["064"]
    report["commands"][0:0] = [
        command
        for lesson, evidence in sorted(ORDINARY_EVIDENCE.items())
        if lesson in {state["lesson"] for state in report["boundaries"]}
        for command in (evidence["command"], evidence["properties_command"])
    ]
    report["commands"].extend(LAYOUT_COMMANDS)
    if "065" in OBJECT_COMMANDS:
        report["commands"].append(OBJECT_COMMANDS["065"])
    report["constants"].update(
        {
            "thermal_gradient_isr_reserve_bytes": ISR_RESERVE,
            "thermal_gradient_aggregate_residual_target_bytes":
                RESIDUAL_SRAM_TARGET,
            "thermal_gradient_aggregate_residual_target_applies_from": "066",
            "thermal_gradient_residual_sram_hard_bytes": RESIDUAL_SRAM_HARD,
            "caller_buffer_target_bytes": CALLER_BUFFER_TARGET,
            "caller_buffer_hard_bytes": CALLER_BUFFER_HARD,
        }
    )
    state = report["boundaries"][0]
    measurements = state["measurements"]
    measurements["ordinary_flash_bytes"] = ordinary_064["flash_bytes"]
    measurements["ordinary_static_sram_bytes"] = ordinary_064[
        "static_sram_bytes"
    ]
    state["gates"]["ordinary_flash"] = probe.gate(
        measurements["ordinary_flash_bytes"],
        BOUNDARY_064["flash_target"],
        BOUNDARY_064["flash_hard"],
    )
    state["gates"]["ordinary_static_sram"] = probe.gate(
        measurements["ordinary_static_sram_bytes"],
        BOUNDARY_064["sram_target"],
        BOUNDARY_064["sram_hard"],
    )
    for gate_name in ("ordinary_flash", "ordinary_static_sram"):
        if state["gates"][gate_name] == "target-miss":
            state["gates"][gate_name] = "review-required"

    measurements["public_enums"] = {
        name: {"size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"]}
        for name in PUBLIC_ENUMS_064
    }
    measurements["public_values"] = {
        name: {
            "size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"],
            "alignment_bytes": LAYOUT_SYMBOLS[f"{name}Alignment"],
            "standard_layout": True,
            "trivially_copyable": True,
            "trivially_destructible": True,
        }
        for name in PUBLIC_VALUES_064
    }
    measurements["policy_traits"] = {
        "copy_constructible": False,
        "move_constructible": False,
        "copy_assignable": False,
        "move_assignable": False,
    }
    caller_buffers = {
        "request_bytes": LAYOUT_SYMBOLS["oneWireRequestCallerBufferBytes"],
        "search_bytes": LAYOUT_SYMBOLS["oneWireSearchCallerBufferBytes"],
        "intent_bytes": LAYOUT_SYMBOLS["oneWireIntentCallerBufferBytes"],
        "receipt_bytes": LAYOUT_SYMBOLS["oneWireReceiptCallerBufferBytes"],
        "snapshot_bytes": LAYOUT_SYMBOLS["oneWireSnapshotCallerBufferBytes"],
    }
    caller_buffers["aggregate_bytes"] = sum(caller_buffers.values())
    measurements["caller_buffers"] = caller_buffers
    required_linked = {
        "policy": (
            "fixturePolicyE",
            measurements["object_bytes"],
        ),
        "request": (
            "fixtureRequestE",
            caller_buffers["request_bytes"],
        ),
        "search": (
            "fixtureSearchStateE",
            caller_buffers["search_bytes"],
        ),
        "intent": (
            "fixtureIntentE",
            caller_buffers["intent_bytes"],
        ),
        "receipt": (
            "fixtureReceiptE",
            caller_buffers["receipt_bytes"],
        ),
        "snapshot": (
            "fixtureSnapshotE",
            caller_buffers["snapshot_bytes"],
        ),
    }
    linked_evidence = {}
    for name, (suffix, expected_size) in required_linked.items():
        matches = [
            (symbol, size)
            for symbol, size in LINKED_STORAGE["064"].items()
            if symbol.endswith(suffix)
        ]
        if len(matches) != 1:
            raise probe.ProbeError(
                f"Lesson 064 linked fixture requires exactly one {suffix} "
                f"symbol, found {matches}"
            )
        symbol, observed_size = matches[0]
        if observed_size != expected_size:
            raise probe.ProbeError(
                f"Lesson 064 linked {name} is {observed_size} B, "
                f"expected {expected_size} B"
            )
        linked_evidence[name] = {
            "symbol": symbol,
            "size_bytes": observed_size,
            "symbol_count": 1,
        }
    measurements["linked_maximum_fixture"] = {
        "canonical_example": (
            "examples/Lesson064OwnedSingleWireTransactions/"
            "Lesson064OwnedSingleWireTransactions.ino"
        ),
        "global_storage": linked_evidence,
        "caller_storage_instantiated_once": True,
    }
    for metric, key in (
        ("request_caller_buffer", "request_bytes"),
        ("search_caller_buffer", "search_bytes"),
        ("intent_caller_buffer", "intent_bytes"),
        ("receipt_caller_buffer", "receipt_bytes"),
        ("snapshot_caller_buffer", "snapshot_bytes"),
    ):
        state["gates"][metric] = probe.gate(
            caller_buffers[key], CALLER_BUFFER_TARGET, CALLER_BUFFER_HARD
        )
        if state["gates"][metric] == "target-miss":
            state["gates"][metric] = "review-required"
    for metric, disposition in tuple(state["gates"].items()):
        if disposition == "target-miss":
            state["gates"][metric] = "review-required"
    apply_enriched_reviews(state, BOUNDARY_064)

    residual = (
        BOARD_SRAM
        - measurements["static_sram_bytes"]
        - measurements["synchronous_stack_bytes"]
        - ISR_RESERVE
    )
    measurements["isr_reserve_bytes"] = ISR_RESERVE
    measurements["residual_sram_bytes"] = residual
    state["gates"]["residual_sram_hard_floor"] = (
        "pass" if residual >= RESIDUAL_SRAM_HARD else "hard-fail"
    )

    fingerprint_payload = {
        "schema": 4,
        "probe_contract": FINGERPRINT_CONTRACT["064"],
        "lesson_through": "064",
        "fqbn": report["fqbn"],
        "core_package": ordinary_064["core_package"],
        "core_version": ordinary_064["core_version"],
        "f_cpu_hz": ordinary_064["f_cpu_hz"],
        "tools": review_tool_identities(report["tools"]),
        "commands": json.loads(
            normalized(
                [
                    ordinary_064["command"],
                    ordinary_064["properties_command"],
                    EXACT_COMMANDS["064"],
                    OBJECT_COMMANDS["064"],
                    *LAYOUT_COMMANDS,
                ]
            )
        ),
        "ordinary_compile_units": boundary_compile_units(
            ordinary_064["compile_units"], "064"
        ),
        "compile_dependencies": boundary_compile_dependencies(
            COMPILE_DEPENDENCIES["064"], "064"
        ),
        "linker_executable": ordinary_064["linker_executable"],
        "linker_version": ordinary_064["linker_version"],
        "resolved_link_recipe": ordinary_064["resolved_link_recipe"],
        "source_hashes": fingerprint_source_hashes("064"),
        "generated_layout_hashes": LAYOUT_SOURCE_HASHES["064"],
        "authority_markers": [
            marker
            for marker in AUTHORITY_MARKERS
            if marker.startswith("Resource-review: lesson=064 ")
        ],
        "measurement_payload": json.loads(normalized(measurements)),
        "gate_payload": {
            metric: (
                "target-miss"
                if disposition
                in ("target-miss", "review-required", "reviewed-target-miss")
                else disposition
            )
            for metric, disposition in sorted(state["gates"].items())
        },
        "thresholds": report["constants"],
    }
    fingerprint = hashlib.sha256(
        json.dumps(
            fingerprint_payload, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()
    state["fingerprint_sha256"] = fingerprint
    report["boundary_fingerprints"] = {
        "064": {**fingerprint_payload, "sha256": fingerprint}
    }
    report["fingerprint"] = report["boundary_fingerprints"]["064"]

    state_by_lesson = {
        boundary_state["lesson"]: boundary_state
        for boundary_state in report["boundaries"]
    }
    if "065" in state_by_lesson and "measurements" in state_by_lesson["065"]:
        state_065 = state_by_lesson["065"]
        measurements_065 = state_065["measurements"]
        ordinary_065 = ORDINARY_EVIDENCE["065"]
        measurements_065["ordinary_flash_bytes"] = ordinary_065["flash_bytes"]
        measurements_065["ordinary_static_sram_bytes"] = ordinary_065[
            "static_sram_bytes"
        ]
        for metric, value, target, hard in (
            (
                "ordinary_flash",
                measurements_065["ordinary_flash_bytes"],
                BOUNDARY_065["flash_target"],
                BOUNDARY_065["flash_hard"],
            ),
            (
                "ordinary_static_sram",
                measurements_065["ordinary_static_sram_bytes"],
                BOUNDARY_065["sram_target"],
                BOUNDARY_065["sram_hard"],
            ),
        ):
            state_065["gates"][metric] = probe.gate(value, target, hard)
            if state_065["gates"][metric] == "target-miss":
                state_065["gates"][metric] = "review-required"
        measurements_065["public_enums"] = {
            name: {"size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"]}
            for name in PUBLIC_ENUMS_065
        }
        measurements_065["public_values"] = {
            name: {
                "size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"],
                "alignment_bytes": LAYOUT_SYMBOLS[f"{name}Alignment"],
                "standard_layout": True,
                "trivially_copyable": True,
                "trivially_destructible": True,
            }
            for name in PUBLIC_VALUES_065
        }
        measurements_065["policy_traits"] = {
            "copy_constructible": False,
            "move_constructible": False,
            "copy_assignable": False,
            "move_assignable": False,
        }
        caller_buffers_065 = {
            "builder_bytes": LAYOUT_SYMBOLS[
                "ds18b20BuilderCallerBufferBytes"
            ],
            "snapshot_bytes": LAYOUT_SYMBOLS[
                "ds18b20SnapshotCallerBufferBytes"
            ],
        }
        measurements_065["caller_buffers"] = caller_buffers_065
        for metric, key, target, hard in (
            ("builder_caller_buffer", "builder_bytes", 448, 512),
            ("snapshot_caller_buffer", "snapshot_bytes", 256, 512),
        ):
            state_065["gates"][metric] = probe.gate(
                caller_buffers_065[key], target, hard
            )
            if state_065["gates"][metric] == "target-miss":
                state_065["gates"][metric] = "review-required"
        for metric, disposition in tuple(state_065["gates"].items()):
            if disposition == "target-miss":
                state_065["gates"][metric] = "review-required"
        residual_065 = (
            BOARD_SRAM
            - measurements_065["static_sram_bytes"]
            - measurements_065["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        measurements_065["isr_reserve_bytes"] = ISR_RESERVE
        measurements_065["residual_sram_bytes"] = residual_065
        state_065["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual_065 >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        linked_065 = LINKED_STORAGE["065"]
        linked_roles = {
            "policy": ("fixturePolicyE", measurements_065["object_bytes"]),
            "builder": (
                "fixtureBuilderE",
                caller_buffers_065["builder_bytes"],
            ),
        }
        linked_evidence_065 = {}
        for name, (suffix, expected_size) in linked_roles.items():
            matches = [
                (symbol, size)
                for symbol, size in linked_065.items()
                if symbol.endswith(suffix)
            ]
            if len(matches) != 1 or matches[0][1] != expected_size:
                raise probe.ProbeError(
                    f"Lesson 065 linked {name} evidence mismatch: "
                    f"expected one {expected_size} B {suffix}, found {matches}"
                )
            linked_evidence_065[name] = {
                "symbol": matches[0][0],
                "size_bytes": matches[0][1],
                "symbol_count": 1,
            }
        measurements_065["linked_maximum_fixture"] = {
            "canonical_example": (
                "examples/Lesson065Qualified18B20ProbeSet/"
                "Lesson065Qualified18B20ProbeSet.ino"
            ),
            "global_storage": linked_evidence_065,
            "caller_storage_instantiated_once": True,
        }
        forbidden_phase_globals = {
            "snapshot": "fixtureSnapshotE",
            "transaction": "fixtureTransactionE",
        }
        unexpected_phase_globals = {
            name: [
                {"symbol": symbol, "size_bytes": size}
                for symbol, size in linked_065.items()
                if symbol.endswith(suffix)
            ]
            for name, suffix in forbidden_phase_globals.items()
        }
        if any(unexpected_phase_globals.values()):
            raise probe.ProbeError(
                "Lesson 065 phase-local storage leaked into global storage: "
                f"{unexpected_phase_globals}"
            )
        phase_search_bytes = (
            LAYOUT_SYMBOLS["OneWireTransactionSnapshotBytes"]
            + LAYOUT_SYMBOLS["OneWireSearchStateBytes"]
        )
        phase_peak_bytes = max(
            caller_buffers_065["snapshot_bytes"], phase_search_bytes
        )
        recurring_bytes = (
            measurements_065["object_bytes"]
            + caller_buffers_065["builder_bytes"]
        )
        lifetime_peak_bytes = recurring_bytes + phase_peak_bytes
        observe_nodes = [
            node
            for node in state_065["linked_call_graph"]["nodes"]
            if BOUNDARY_065["phase_frame_symbol"] in node["demangled"]
        ]
        if len(observe_nodes) != 1:
            raise probe.ProbeError(
                "Lesson 065 requires one linked phase-storage frame, "
                f"found {observe_nodes}"
            )
        observe_frame_bytes = observe_nodes[0]["local_bytes"]
        if observe_frame_bytes < phase_peak_bytes:
            raise probe.ProbeError(
                "Lesson 065 linked observe frame cannot hold the required "
                f"{phase_peak_bytes} B phase-local peak: {observe_frame_bytes} B"
            )
        measurements_065["lifetime_placement"] = {
            "recurring_policy_bytes": measurements_065["object_bytes"],
            "recurring_builder_bytes": caller_buffers_065["builder_bytes"],
            "recurring_total_bytes": recurring_bytes,
            "phase_search_bytes": phase_search_bytes,
            "phase_snapshot_bytes": caller_buffers_065["snapshot_bytes"],
            "phase_peak_bytes": phase_peak_bytes,
            "lifetime_peak_bytes": lifetime_peak_bytes,
            "observe_frame_bytes": observe_frame_bytes,
            "phase_globals_absent": unexpected_phase_globals,
        }
        state_065["gates"]["recurring_owned_storage"] = probe.gate(
            recurring_bytes, 960, 1280
        )
        state_065["gates"]["lifetime_peak_storage"] = probe.gate(
            lifetime_peak_bytes, 1216, 1792
        )
        for metric in ("recurring_owned_storage", "lifetime_peak_storage"):
            if state_065["gates"][metric] == "target-miss":
                state_065["gates"][metric] = "review-required"
        apply_enriched_reviews(state_065, BOUNDARY_065)
        payload_065 = {
            "schema": 4,
            "probe_contract": FINGERPRINT_CONTRACT["065"],
            "lesson_through": "065",
            "predecessor_fingerprint_sha256": fingerprint,
            "fqbn": report["fqbn"],
            "core_package": ordinary_065["core_package"],
            "core_version": ordinary_065["core_version"],
            "f_cpu_hz": ordinary_065["f_cpu_hz"],
            "tools": review_tool_identities(report["tools"]),
            "commands": json.loads(
                normalized(
                    [
                        ordinary_065["command"],
                        ordinary_065["properties_command"],
                        EXACT_COMMANDS["065"],
                        OBJECT_COMMANDS["064"],
                        OBJECT_COMMANDS["065"],
                        *LAYOUT_COMMANDS,
                    ]
                )
            ),
            "ordinary_compile_units": boundary_compile_units(
                ordinary_065["compile_units"], "065"
            ),
            "compile_dependencies": boundary_compile_dependencies(
                COMPILE_DEPENDENCIES["065"], "065"
            ),
            "linker_executable": ordinary_065["linker_executable"],
            "linker_version": ordinary_065["linker_version"],
            "resolved_link_recipe": ordinary_065["resolved_link_recipe"],
            "source_hashes": fingerprint_source_hashes("065"),
            "generated_layout_hashes": {
                "064": LAYOUT_SOURCE_HASHES["064"],
                "065": LAYOUT_SOURCE_HASHES["065"],
            },
            "authority_markers": [
                marker
                for marker in AUTHORITY_MARKERS
                if marker.startswith("Resource-review: lesson=065 ")
            ],
            "measurement_payload": json.loads(normalized(measurements_065)),
            "gate_payload": {
                metric: (
                    "target-miss"
                    if disposition
                    in (
                        "target-miss",
                        "review-required",
                        "reviewed-target-miss",
                    )
                    else disposition
                )
                for metric, disposition in sorted(state_065["gates"].items())
            },
            "thresholds": report["constants"],
        }
        fingerprint_065 = hashlib.sha256(
            json.dumps(
                payload_065, sort_keys=True, separators=(",", ":")
            ).encode("utf-8")
        ).hexdigest()
        state_065["fingerprint_sha256"] = fingerprint_065
        report["boundary_fingerprints"]["065"] = {
            **payload_065,
            "sha256": fingerprint_065,
        }
        report["fingerprint"] = report["boundary_fingerprints"]["065"]

    if "066" in state_by_lesson and "measurements" in state_by_lesson["066"]:
        state_066 = state_by_lesson["066"]
        measurements_066 = state_066["measurements"]
        ordinary_066 = ORDINARY_EVIDENCE["066"]
        measurements_066["ordinary_flash_bytes"] = ordinary_066["flash_bytes"]
        measurements_066["ordinary_static_sram_bytes"] = ordinary_066[
            "static_sram_bytes"
        ]
        for metric, value, target, hard in (
            (
                "ordinary_flash",
                measurements_066["ordinary_flash_bytes"],
                BOUNDARY_066["flash_target"],
                BOUNDARY_066["flash_hard"],
            ),
            (
                "ordinary_static_sram",
                measurements_066["ordinary_static_sram_bytes"],
                BOUNDARY_066["sram_target"],
                BOUNDARY_066["sram_hard"],
            ),
        ):
            state_066["gates"][metric] = probe.gate(value, target, hard)
            if state_066["gates"][metric] == "target-miss":
                state_066["gates"][metric] = "review-required"
        measurements_066["public_enums"] = {
            name: {"size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"]}
            for name in PUBLIC_ENUMS_066
        }
        measurements_066["public_values"] = {
            name: {
                "size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"],
                "alignment_bytes": LAYOUT_SYMBOLS[f"{name}Alignment"],
                "standard_layout": True,
                "trivially_copyable": True,
                "trivially_destructible": True,
            }
            for name in PUBLIC_VALUES_066
        }
        measurements_066["policy_traits"] = {
            "copy_constructible": False,
            "move_constructible": False,
            "copy_assignable": False,
            "move_assignable": False,
        }
        caller_buffers_066 = {
            "envelope_bytes": LAYOUT_SYMBOLS[
                "thermalMapperEnvelopeCallerBufferBytes"
            ],
            "result_bytes": LAYOUT_SYMBOLS[
                "thermalMapperResultCallerBufferBytes"
            ],
        }
        measurements_066["caller_buffers"] = caller_buffers_066
        for metric, key in (
            ("envelope_caller_buffer", "envelope_bytes"),
            ("result_caller_buffer", "result_bytes"),
        ):
            state_066["gates"][metric] = probe.gate(
                caller_buffers_066[key], 256, 384
            )
            if state_066["gates"][metric] == "target-miss":
                state_066["gates"][metric] = "review-required"

        linked_066 = LINKED_STORAGE["066"]
        linked_roles_066 = {
            "one_wire_policy": (
                "fixtureOneWirePolicyE",
                next(
                    state["measurements"]["object_bytes"]
                    for state in report["boundaries"]
                    if state["lesson"] == "064"
                ),
            ),
            "probe_set_policy": (
                "fixtureProbeSetPolicyE",
                next(
                    state["measurements"]["object_bytes"]
                    for state in report["boundaries"]
                    if state["lesson"] == "065"
                ),
            ),
            "active_builder": (
                "fixtureBuilderE",
                LAYOUT_SYMBOLS["ds18b20BuilderCallerBufferBytes"],
            ),
            "mapper": (
                "fixtureMapperE",
                measurements_066["object_bytes"],
            ),
        }
        linked_evidence_066 = {}
        for name, (suffix, expected_size) in linked_roles_066.items():
            matches = [
                (symbol, size)
                for symbol, size in linked_066.items()
                if symbol.endswith(suffix)
            ]
            if len(matches) != 1 or matches[0][1] != expected_size:
                raise probe.ProbeError(
                    f"Lesson 066 linked {name} evidence mismatch: expected "
                    f"one {expected_size} B {suffix}, found {matches}"
                )
            linked_evidence_066[name] = {
                "symbol": matches[0][0],
                "size_bytes": matches[0][1],
                "symbol_count": 1,
                "lifetime": "recurring",
            }
        forbidden_phase_globals_066 = {
            "qualified_snapshot": "fixtureProbeSnapshotE",
            "mapper_envelope": "fixtureEnvelopeE",
            "mapper_result": "fixtureResultE",
            "record_image": "fixtureRecordE",
            "transaction": "fixtureTransactionE",
        }
        unexpected_phase_globals_066 = {
            name: [
                {"symbol": symbol, "size_bytes": size}
                for symbol, size in linked_066.items()
                if symbol.endswith(suffix)
            ]
            for name, suffix in forbidden_phase_globals_066.items()
        }
        if any(unexpected_phase_globals_066.values()):
            raise probe.ProbeError(
                "Lesson 066 phase-local storage leaked into global storage: "
                f"{unexpected_phase_globals_066}"
            )
        recurring_bytes_066 = sum(
            evidence["size_bytes"]
            for evidence in linked_evidence_066.values()
        )
        phase_peak_bytes_066 = max(
            caller_buffers_066["envelope_bytes"]
            + caller_buffers_066["result_bytes"],
            LAYOUT_SYMBOLS["OneWireTransactionSnapshotBytes"],
        )
        lifetime_peak_bytes_066 = (
            recurring_bytes_066 + phase_peak_bytes_066
        )
        phase_nodes_066 = [
            node
            for node in state_066["linked_call_graph"]["nodes"]
            if BOUNDARY_066["phase_frame_symbol"] in node["demangled"]
        ]
        if len(phase_nodes_066) != 1:
            raise probe.ProbeError(
                "Lesson 066 requires one linked update-storage frame, "
                f"found {phase_nodes_066}"
            )
        phase_frame_bytes_066 = phase_nodes_066[0]["local_bytes"]
        if phase_frame_bytes_066 < phase_peak_bytes_066:
            raise probe.ProbeError(
                "Lesson 066 linked update frame cannot hold the required "
                f"{phase_peak_bytes_066} B phase-local peak: "
                f"{phase_frame_bytes_066} B"
            )
        measurements_066["linked_maximum_fixture"] = {
            "canonical_example": (
                "examples/Lesson066ThermalGradientMapper/"
                "Lesson066ThermalGradientMapper.ino"
            ),
            "global_storage": linked_evidence_066,
            "phase_storage_global_absence": unexpected_phase_globals_066,
            "caller_storage_instantiated_once": True,
        }
        measurements_066["lifetime_placement"] = {
            "recurring_one_wire_policy_bytes":
                linked_evidence_066["one_wire_policy"]["size_bytes"],
            "recurring_probe_set_policy_bytes":
                linked_evidence_066["probe_set_policy"]["size_bytes"],
            "recurring_builder_bytes":
                linked_evidence_066["active_builder"]["size_bytes"],
            "recurring_mapper_bytes":
                linked_evidence_066["mapper"]["size_bytes"],
            "recurring_total_bytes": recurring_bytes_066,
            "phase_envelope_bytes": caller_buffers_066["envelope_bytes"],
            "phase_result_bytes": caller_buffers_066["result_bytes"],
            "nested_record_image_bytes":
                LAYOUT_SYMBOLS["ThermalMapperRecordImageBytes"],
            "phase_transaction_bytes":
                LAYOUT_SYMBOLS["OneWireTransactionSnapshotBytes"],
            "phase_peak_bytes": phase_peak_bytes_066,
            "lifetime_peak_bytes": lifetime_peak_bytes_066,
            "update_frame_bytes": phase_frame_bytes_066,
            "phase_globals_absent": unexpected_phase_globals_066,
        }
        state_066["gates"]["recurring_composition_storage"] = probe.gate(
            recurring_bytes_066, 2048, 3072
        )
        state_066["gates"]["composition_lifetime_peak_storage"] = probe.gate(
            lifetime_peak_bytes_066, 2560, 3840
        )
        for metric in (
            "recurring_composition_storage",
            "composition_lifetime_peak_storage",
        ):
            if state_066["gates"][metric] == "target-miss":
                state_066["gates"][metric] = "review-required"
        residual_066 = (
            BOARD_SRAM
            - measurements_066["static_sram_bytes"]
            - measurements_066["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        measurements_066["isr_reserve_bytes"] = ISR_RESERVE
        measurements_066["residual_sram_bytes"] = residual_066
        state_066["gates"]["residual_sram"] = (
            "pass" if residual_066 >= RESIDUAL_SRAM_TARGET else "target-miss"
        )
        if state_066["gates"]["residual_sram"] == "target-miss":
            state_066["gates"]["residual_sram"] = "review-required"
        state_066["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual_066 >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        for metric, disposition in tuple(state_066["gates"].items()):
            if disposition == "target-miss":
                state_066["gates"][metric] = "review-required"
        apply_enriched_reviews(state_066, BOUNDARY_066)

        payload_066 = {
            "schema": 4,
            "probe_contract": FINGERPRINT_CONTRACT["066"],
            "lesson_through": "066",
            "predecessor_fingerprint_sha256":
                report["boundary_fingerprints"]["065"]["sha256"],
            "fqbn": report["fqbn"],
            "core_package": ordinary_066["core_package"],
            "core_version": ordinary_066["core_version"],
            "f_cpu_hz": ordinary_066["f_cpu_hz"],
            "tools": review_tool_identities(report["tools"]),
            "commands": json.loads(
                normalized(
                    [
                        ordinary_066["command"],
                        ordinary_066["properties_command"],
                        EXACT_COMMANDS["066"],
                        OBJECT_COMMANDS["064"],
                        OBJECT_COMMANDS["065"],
                        OBJECT_COMMANDS["066"],
                        *LAYOUT_COMMANDS,
                    ]
                )
            ),
            "ordinary_compile_units": boundary_compile_units(
                ordinary_066["compile_units"], "066"
            ),
            "compile_dependencies": boundary_compile_dependencies(
                COMPILE_DEPENDENCIES["066"], "066"
            ),
            "linker_executable": ordinary_066["linker_executable"],
            "linker_version": ordinary_066["linker_version"],
            "resolved_link_recipe": ordinary_066["resolved_link_recipe"],
            "source_hashes": fingerprint_source_hashes("066"),
            "generated_layout_hashes": {
                lesson: LAYOUT_SOURCE_HASHES[lesson]
                for lesson in ("064", "065", "066")
            },
            "authority_markers": [
                marker
                for marker in AUTHORITY_MARKERS
                if marker.startswith("Resource-review: lesson=066 ")
            ],
            "measurement_payload": json.loads(normalized(measurements_066)),
            "gate_payload": {
                metric: (
                    "target-miss"
                    if disposition
                    in (
                        "target-miss",
                        "review-required",
                        "reviewed-target-miss",
                    )
                    else disposition
                )
                for metric, disposition in sorted(state_066["gates"].items())
            },
            "thresholds": report["constants"],
        }
        fingerprint_066 = hashlib.sha256(
            json.dumps(
                payload_066, sort_keys=True, separators=(",", ":")
            ).encode("utf-8")
        ).hexdigest()
        state_066["fingerprint_sha256"] = fingerprint_066
        report["boundary_fingerprints"]["066"] = {
            **payload_066,
            "sha256": fingerprint_066,
        }
        report["fingerprint"] = report["boundary_fingerprints"]["066"]

    for review in LOADED_REVIEWS.values():
        current = report["boundary_fingerprints"].get(review["lesson"], {}).get(
            "sha256"
        )
        if review["fingerprint_sha256"] != current:
            raise probe.ProbeError(
                "target-miss review fingerprint is stale: "
                f"{review['fingerprint_sha256']} != {current}"
            )

    state["status"] = (
        "hard-fail"
        if "hard-fail" in state["gates"].values()
        else (
            "review-required"
            if any(
                disposition in ("target-miss", "review-required")
                for disposition in state["gates"].values()
            )
            else (
                "reviewed-target-miss"
                if "reviewed-target-miss" in state["gates"].values()
                else "pass"
            )
        )
    )
    if "065" in state_by_lesson and "gates" in state_by_lesson["065"]:
        state_065 = state_by_lesson["065"]
        state_065["status"] = (
            "hard-fail"
            if "hard-fail" in state_065["gates"].values()
            else (
                "review-required"
                if any(
                    disposition in ("target-miss", "review-required")
                    for disposition in state_065["gates"].values()
                )
                else (
                    "reviewed-target-miss"
                    if "reviewed-target-miss" in state_065["gates"].values()
                    else "pass"
                )
            )
        )
    if "066" in state_by_lesson and "gates" in state_by_lesson["066"]:
        state_066 = state_by_lesson["066"]
        state_066["status"] = (
            "hard-fail"
            if "hard-fail" in state_066["gates"].values()
            else (
                "review-required"
                if any(
                    disposition in ("target-miss", "review-required")
                    for disposition in state_066["gates"].values()
                )
                else (
                    "reviewed-target-miss"
                    if "reviewed-target-miss" in state_066["gates"].values()
                    else "pass"
                )
            )
        )
    statuses = [
        item.get("status", "error") for item in report["boundaries"]
    ]
    report["status"] = max(
        statuses, key=lambda status: probe.STATUS_SEVERITY[status]
    )
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return (
        1
        if report["status"] in ("hard-fail", "review-required", "error")
        else 0
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through", choices=("064", "065", "066"), default="066"
    )
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/thermal-gradient-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/thermal_gradient_resource_reviews.json",
    )
    arguments = parser.parse_args()
    required_boundaries = selected_boundaries(arguments.require_through)
    compile_ordinary(arguments, required_boundaries)
    probe.BOUNDARIES = required_boundaries
    probe.compile_sketch = compile_exact_sketch
    probe.object_sizes = object_sizes
    probe.load_reviews = load_reviews
    sys.argv = [
        sys.argv[0],
        "--arduino-cli",
        arguments.arduino_cli,
        "--fqbn",
        arguments.fqbn,
        "--require-complete",
        "--evidence-json",
        arguments.evidence_json,
        "--review-file",
        arguments.review_file,
    ]
    result = probe.main()
    evidence_result = enrich_evidence(ROOT / arguments.evidence_json)
    return result or evidence_result


if __name__ == "__main__":
    raise SystemExit(main())
