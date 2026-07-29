#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
import pathlib
import re
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

BOARD_SRAM = 8192
ISR_RESERVE = 128
RESIDUAL_SRAM_TARGET = 3072
RESIDUAL_SRAM_HARD = 2048
FIXED_BUFFER_TARGET = 256
FIXED_BUFFER_HARD = 512

PUBLIC_VALUES = {
    "061": (
        "ResistiveProbeSample",
        "ResistiveProbeConfig",
        "ResistiveProbeObservation",
    ),
    "062": (
        "ConvertedThermalSample",
        "CategoricalThresholdSample",
        "ThermalRadiantEnvelope",
        "ThermalRadiantConfig",
        "ThermalRadiantObservation",
    ),
}

BOUNDARIES = (
    {
        "lesson": "061",
        "sketch": "extras/probes/Lesson061MuseumCaseResourceProbe",
        "object_symbol": "resistiveProbeObservationPolicyObjectBytes",
        "flash_target": 8 * 1024,
        "flash_hard": 12 * 1024,
        "sram_target": 768,
        "sram_hard": 1024,
        "stack_target": 320,
        "stack_hard": 448,
        "object_target": 192,
        "object_hard": 256,
    },
    {
        "lesson": "062",
        "sketch": "extras/probes/Lesson062MuseumCaseResourceProbe",
        "object_symbol": "thermalRadiantObservationPolicyObjectBytes",
        "flash_target": 12 * 1024,
        "flash_hard": 16 * 1024,
        "sram_target": 1024,
        "sram_hard": 1536,
        "stack_target": 384,
        "stack_hard": 576,
        "object_target": 320,
        "object_hard": 448,
    },
)

RESOURCE_LAYOUTS = {}
ORDINARY_EVIDENCE = {}
LOADED_REVIEWS = {}


def object_sizes(compiler, nm, root, temporary, unused):
    object_path = temporary / "museum_case_object_sizes.o"
    command = [
        str(compiler),
        "-c",
        "-mmcu=atmega2560",
        "-std=gnu++11",
        "-Os",
        "-fno-lto",
        "-fno-exceptions",
        "-fno-rtti",
        "-Isrc",
    ]
    if (root / "src/thermal_radiant_observation.h").is_file():
        command.append("-DADK_HAS_LESSON_062=1")
    command.extend(
        (
            str(root / "probes/museum_case_object_sizes.cpp"),
            "-o",
            str(object_path),
        )
    )
    probe.run(command, cwd=root)
    sizes = read_symbols(nm, object_path)

    layout_path = temporary / "museum_case_public_layouts.cpp"
    layout_path.write_text(
        """
#include <resistive_probe_observation.h>
#if defined (ADK_HAS_LESSON_062)
#include <thermal_radiant_observation.h>
#endif
#define ADK_LAYOUT(type) \\
    unsigned char type##Bytes[sizeof (adk::type)]; \\
    unsigned char type##Alignment[alignof (adk::type)]; \\
    static_assert (__is_standard_layout (adk::type), \\
                   #type " must remain standard-layout"); \\
    static_assert (__is_trivially_copyable (adk::type), \\
                   #type " must remain trivially copyable"); \\
    static_assert (__has_trivial_destructor (adk::type), \\
                   #type " must remain trivially destructible")

unsigned char ProbeQualityBytes[sizeof (adk::ProbeQuality)];
ADK_LAYOUT (ResistiveProbeSample);
ADK_LAYOUT (ResistiveProbeConfig);
ADK_LAYOUT (ResistiveProbeObservation);

unsigned char resistiveProbeInputCallerBufferBytes
    [sizeof (adk::ResistiveProbeSample)];
unsigned char resistiveProbeOutputCallerBufferBytes
    [sizeof (adk::ResistiveProbeObservation)];

#if defined (ADK_HAS_LESSON_062)
unsigned char ThresholdStateBytes[sizeof (adk::ThresholdState)];
unsigned char ThermalQualityBytes[sizeof (adk::ThermalQuality)];
unsigned char RadiantQualityBytes[sizeof (adk::RadiantQuality)];
ADK_LAYOUT (ConvertedThermalSample);
ADK_LAYOUT (CategoricalThresholdSample);
ADK_LAYOUT (ThermalRadiantEnvelope);
ADK_LAYOUT (ThermalRadiantConfig);
ADK_LAYOUT (ThermalRadiantObservation);

unsigned char thermalRadiantInputCallerBufferBytes
    [sizeof (adk::ThermalRadiantEnvelope)];
unsigned char thermalRadiantOutputCallerBufferBytes
    [sizeof (adk::ThermalRadiantObservation)];
#endif
""".lstrip(),
        encoding="utf-8",
    )
    layout_object = temporary / "museum_case_public_layouts.o"
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
    ]
    if (root / "src/thermal_radiant_observation.h").is_file():
        layout_command.append("-DADK_HAS_LESSON_062=1")
    layout_command.extend((str(layout_path), "-o", str(layout_object)))
    probe.run(layout_command, cwd=root)

    host_compiler = shutil.which("c++")
    if host_compiler is None:
        raise probe.ProbeError("host C++ compiler is unavailable for trait checks")
    trait_path = temporary / "museum_case_public_traits.cpp"
    trait_path.write_text(
        """
#include <resistive_probe_observation.h>
#if defined (ADK_HAS_LESSON_062)
#include <thermal_radiant_observation.h>
#endif
#include <type_traits>

static_assert (
    !std::is_copy_constructible<
        adk::ResistiveProbeObservationPolicy>::value,
    "ResistiveProbeObservationPolicy must remain non-copyable");
static_assert (
    !std::is_move_constructible<
        adk::ResistiveProbeObservationPolicy>::value,
    "ResistiveProbeObservationPolicy must remain non-movable");
static_assert (
    std::is_trivially_destructible<adk::ResistiveProbeSample>::value,
    "ResistiveProbeSample must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<adk::ResistiveProbeConfig>::value,
    "ResistiveProbeConfig must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<
        adk::ResistiveProbeObservation>::value,
    "ResistiveProbeObservation must remain trivially destructible");
#if defined (ADK_HAS_LESSON_062)
static_assert (
    !std::is_copy_constructible<
        adk::ThermalRadiantObservationPolicy>::value,
    "ThermalRadiantObservationPolicy must remain non-copyable");
static_assert (
    !std::is_move_constructible<
        adk::ThermalRadiantObservationPolicy>::value,
    "ThermalRadiantObservationPolicy must remain non-movable");
static_assert (
    std::is_trivially_destructible<
        adk::ConvertedThermalSample>::value,
    "ConvertedThermalSample must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<
        adk::CategoricalThresholdSample>::value,
    "CategoricalThresholdSample must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<
        adk::ThermalRadiantEnvelope>::value,
    "ThermalRadiantEnvelope must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<
        adk::ThermalRadiantConfig>::value,
    "ThermalRadiantConfig must remain trivially destructible");
static_assert (
    std::is_trivially_destructible<
        adk::ThermalRadiantObservation>::value,
    "ThermalRadiantObservation must remain trivially destructible");
#endif
""".lstrip(),
        encoding="utf-8",
    )
    trait_command = [
        host_compiler,
        "-fsyntax-only",
        "-std=c++11",
        "-Isrc",
    ]
    if (root / "src/thermal_radiant_observation.h").is_file():
        trait_command.append("-DADK_HAS_LESSON_062=1")
    trait_command.append(str(trait_path))
    probe.run(trait_command, cwd=root)
    RESOURCE_LAYOUTS["061"] = {
        "commands": (layout_command, trait_command),
        "symbols": read_symbols(nm, layout_object),
    }
    if (root / "src/thermal_radiant_observation.h").is_file():
        RESOURCE_LAYOUTS["062"] = RESOURCE_LAYOUTS["061"]
    return sizes, command


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


def normalized(value):
    text = json.dumps(value, sort_keys=True, separators=(",", ":"))
    text = text.replace(str(ROOT), "<repo>")
    return re.sub(r"/tmp/adk-[^/\" ]+", "<temporary>", text)


def compile_ordinary(arguments, boundaries):
    temporary = pathlib.Path(tempfile.mkdtemp(prefix="adk-museum-ordinary."))
    try:
        for boundary in boundaries:
            lesson = boundary["lesson"]
            example_names = {
                "061": "Lesson061ResistiveProbeObservation",
                "062": "Lesson062ThermalRadiantObservation",
            }
            example_path = ROOT / "examples" / example_names[lesson]
            if not example_path.is_dir():
                raise probe.ProbeError(
                    f"ordinary Lesson {lesson} example is unavailable: {example_path}"
                )
            build_directory = temporary / f"lesson-{lesson}"
            command = [
                arguments.arduino_cli,
                "compile",
                "--fqbn",
                arguments.fqbn,
                "--library",
                str(ROOT),
                "--build-path",
                str(build_directory),
                str(example_path),
            ]
            probe.run(command, cwd=ROOT)
            elf_paths = list(build_directory.glob("*.elf"))
            if len(elf_paths) != 1:
                raise probe.ProbeError(
                    f"ordinary Lesson {lesson} build produced "
                    f"{len(elf_paths)} ELF files"
                )
            size_tool = probe.tool_beside(build_directory, "avr-size")
            flash, static_sram = probe.section_sizes(size_tool, elf_paths[0])
            compile_commands = json.loads(
                (build_directory / "compile_commands.json").read_text(
                    encoding="utf-8"
                )
            )
            normalized_units = []
            for unit in compile_commands:
                arguments_list = unit.get("arguments")
                if not isinstance(arguments_list, list):
                    raise probe.ProbeError(
                        "ordinary compile database lacks argument arrays"
                    )
                normalized_units.append(
                    {
                        "file": normalized(unit.get("file", "")),
                        "arguments": json.loads(normalized(arguments_list)),
                    }
                )
            normalized_units.sort(
                key=lambda unit: (
                    unit["file"],
                    json.dumps(unit["arguments"], separators=(",", ":")),
                )
            )
            flattened = " ".join(
                argument
                for unit in normalized_units
                for argument in unit["arguments"]
            )
            if "-DF_CPU=16000000L" not in flattened:
                raise probe.ProbeError(
                    "ordinary build does not resolve F_CPU=16000000L"
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
                for platform in core_document.get("platforms", [])
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
                str(example_path),
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
            compiler = pathlib.Path(compile_commands[0]["arguments"][0])
            linker = compiler.parent / "avr-gcc"
            if not linker.is_file():
                raise probe.ProbeError(
                    "resolved AVR linker executable is unavailable"
                )
            ORDINARY_EVIDENCE[lesson] = {
                "command": command,
                "properties_command": properties_command,
                "flash_bytes": flash,
                "static_sram_bytes": static_sram,
                "compile_units": normalized_units,
                "core_package": "arduino:avr",
                "core_version": core_version,
                "f_cpu_hz": 16000000,
                "elf_sha256": probe.sha256(elf_paths[0]),
                "linker_executable": str(linker),
                "linker_version": probe.tool_version(linker),
                "resolved_link_recipe": link_recipe,
            }
    finally:
        shutil.rmtree(temporary)


def load_reviews(root, review_path):
    global LOADED_REVIEWS
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(document.get("reviews"), list):
        raise probe.ProbeError(f"invalid target-miss review document: {path}")
    reviews = {}
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
        "docs/design/LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md"
        "#canonical-e0-resource-gates"
    )
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if review["lesson"] not in {boundary["lesson"] for boundary in BOUNDARIES}:
            raise probe.ProbeError(f"target-miss review names unknown lesson: {review}")
        if review["disposition"] != "accepted-target-miss":
            raise probe.ProbeError(f"invalid target-miss disposition: {review}")
        if review["authority"] != authority:
            raise probe.ProbeError(
                f"target-miss review lacks controlling authority: {review}"
            )
        marker = (
            f"Resource-review: lesson={review['lesson']} "
            f"metric={review['metric']} observed={review['observed_bytes']} "
            f"target={review['target_bytes']} hard={review['hard_bytes']} "
            f"disposition={review['disposition']}"
        )
        authority_path = root / review["authority"].split("#", 1)[0]
        authority_text = authority_path.read_text(encoding="utf-8")
        heading = "## Canonical E0 resource gates"
        section_start = authority_text.find(heading)
        section_end = authority_text.find("\n## ", section_start + len(heading))
        if section_start < 0:
            raise probe.ProbeError(
                f"target-miss authority section is absent: {review['authority']}"
            )
        if section_end < 0:
            section_end = len(authority_text)
        if marker not in authority_text[section_start:section_end]:
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
        key: review
        for key, review in reviews.items()
        if review["metric"] in base_metrics
    }


def apply_enriched_reviews(state):
    lesson = state["lesson"]
    boundary = next(
        boundary for boundary in BOUNDARIES if boundary["lesson"] == lesson
    )
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
        "input_caller_buffer": (
            measurements["caller_buffers"]["input_sample_bytes"],
            FIXED_BUFFER_TARGET,
            FIXED_BUFFER_HARD,
        ),
        "output_caller_buffer": (
            measurements["caller_buffers"]["output_observation_bytes"],
            FIXED_BUFFER_TARGET,
            FIXED_BUFFER_HARD,
        ),
    }
    supported = {
        "flash",
        "static_sram",
        "synchronous_stack",
        "object",
        *limits,
    }
    for (lesson, metric), review in LOADED_REVIEWS.items():
        if lesson != state["lesson"]:
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
                f"reviewed {reviewed}, measured {expected}, "
                f"current disposition {state['gates'][metric]}"
            )
        state["gates"][metric] = "reviewed-target-miss"
        state.setdefault("accepted_reviews", []).append(review)


def enrich_evidence(evidence_path):
    if not evidence_path.is_file() or "061" not in RESOURCE_LAYOUTS:
        return 0
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    ordinary_commands = []
    for lesson in sorted(ORDINARY_EVIDENCE):
        ordinary_commands.extend(
            (
                ORDINARY_EVIDENCE[lesson]["command"],
                ORDINARY_EVIDENCE[lesson]["properties_command"],
            )
        )
    report["commands"][0:0] = ordinary_commands
    report["commands"].extend(RESOURCE_LAYOUTS["061"]["commands"])
    report["constants"].update(
        {
            "museum_case_isr_reserve_bytes": ISR_RESERVE,
            "museum_case_residual_sram_target_bytes": RESIDUAL_SRAM_TARGET,
            "museum_case_residual_sram_hard_bytes": RESIDUAL_SRAM_HARD,
            "museum_case_residual_target_miss_reviewable": False,
            "fixed_buffer_target_bytes": FIXED_BUFFER_TARGET,
            "fixed_buffer_hard_bytes": FIXED_BUFFER_HARD,
        }
    )
    for state in report["boundaries"]:
        lesson = state["lesson"]
        if lesson not in RESOURCE_LAYOUTS or "measurements" not in state:
            continue
        boundary = next(
            boundary for boundary in BOUNDARIES if boundary["lesson"] == lesson
        )
        ordinary = ORDINARY_EVIDENCE[lesson]
        symbols = RESOURCE_LAYOUTS[lesson]["symbols"]
        measurements = state["measurements"]
        measurements["ordinary_flash_bytes"] = ordinary["flash_bytes"]
        measurements["ordinary_static_sram_bytes"] = ordinary["static_sram_bytes"]
        state["gates"]["ordinary_flash"] = probe.gate(
            measurements["ordinary_flash_bytes"],
            boundary["flash_target"],
            boundary["flash_hard"],
        )
        state["gates"]["ordinary_static_sram"] = probe.gate(
            measurements["ordinary_static_sram_bytes"],
            boundary["sram_target"],
            boundary["sram_hard"],
        )
        for gate_name in ("ordinary_flash", "ordinary_static_sram"):
            if state["gates"][gate_name] == "target-miss":
                state["gates"][gate_name] = "review-required"
        public_values = {}
        for name in PUBLIC_VALUES[lesson]:
            size_symbol = f"{name}Bytes"
            alignment_symbol = f"{name}Alignment"
            if size_symbol not in symbols or alignment_symbol not in symbols:
                raise probe.ProbeError(
                    f"Lesson {lesson} public layout symbol is missing: {name}"
                )
            public_values[name] = {
                "size_bytes": symbols[size_symbol],
                "alignment_bytes": symbols[alignment_symbol],
                "standard_layout": True,
                "trivially_copyable": True,
                "trivially_destructible": True,
            }
        enum_names = (
            ("ProbeQuality",)
            if lesson == "061"
            else ("ThresholdState", "ThermalQuality", "RadiantQuality")
        )
        measurements["public_enums"] = {
            name: {"size_bytes": symbols[f"{name}Bytes"]} for name in enum_names
        }
        measurements["public_values"] = public_values
        measurements["policy_traits"] = {
            "copy_constructible": False,
            "move_constructible": False,
        }
        prefix = "resistiveProbe" if lesson == "061" else "thermalRadiant"
        input_bytes = symbols[f"{prefix}InputCallerBufferBytes"]
        output_bytes = symbols[f"{prefix}OutputCallerBufferBytes"]
        aggregate = input_bytes + output_bytes
        measurements["caller_buffers"] = {
            "input_sample_bytes": input_bytes,
            "output_observation_bytes": output_bytes,
            "aggregate_bytes": aggregate,
            "linked_once_only_fixture": True,
        }
        for name, value in (
            ("input_caller_buffer", input_bytes),
            ("output_caller_buffer", output_bytes),
        ):
            state["gates"][name] = probe.gate(
                value, FIXED_BUFFER_TARGET, FIXED_BUFFER_HARD
            )
            if state["gates"][name] == "target-miss":
                state["gates"][name] = "review-required"
        apply_enriched_reviews(state)
        residual = (
            BOARD_SRAM
            - measurements["static_sram_bytes"]
            - measurements["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        measurements["isr_reserve_bytes"] = ISR_RESERVE
        measurements["residual_sram_bytes"] = residual
        state["gates"]["residual_sram"] = (
            "pass" if residual >= RESIDUAL_SRAM_TARGET else "hard-fail"
        )
        state["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        state["status"] = (
            "hard-fail"
            if "hard-fail" in state["gates"].values()
            else (
                "review-required"
                if "review-required" in state["gates"].values()
                else (
                    "reviewed-target-miss"
                    if "reviewed-target-miss" in state["gates"].values()
                    else state["status"]
                )
            )
        )
        report["status"] = probe.merge_status(report["status"], state["status"])
    source_paths = [
        "src/resistive_probe_observation.h",
        "src/resistive_probe_observation.cpp",
        "examples/Lesson061ResistiveProbeObservation/"
        "Lesson061ResistiveProbeObservation.ino",
        "extras/probes/Lesson061MuseumCaseResourceProbe/"
        "Lesson061MuseumCaseResourceProbe.ino",
        "probes/museum_case_object_sizes.cpp",
        "scripts/check_museum_case_resource_probe.py",
    ]
    if any(state["lesson"] == "062" for state in report["boundaries"]):
        source_paths.extend(
            (
                "src/thermal_radiant_observation.h",
                "src/thermal_radiant_observation.cpp",
                "examples/Lesson062ThermalRadiantObservation/"
                "Lesson062ThermalRadiantObservation.ino",
                "extras/probes/Lesson062MuseumCaseResourceProbe/"
                "Lesson062MuseumCaseResourceProbe.ino",
            )
        )
    source_hashes = {
        path: probe.sha256(ROOT / path)
        for path in source_paths
    }
    fingerprint_payload = {
        "schema": 1,
        "fqbn": report["fqbn"],
        "core_package": ORDINARY_EVIDENCE["061"]["core_package"],
        "core_version": ORDINARY_EVIDENCE["061"]["core_version"],
        "f_cpu_hz": ORDINARY_EVIDENCE["061"]["f_cpu_hz"],
        "tools": report["tools"],
        "commands": json.loads(normalized(report["commands"])),
        "ordinary_compile_units": {
            lesson: evidence["compile_units"]
            for lesson, evidence in sorted(ORDINARY_EVIDENCE.items())
        },
        "linker_executable": ORDINARY_EVIDENCE["061"]["linker_executable"],
        "linker_version": ORDINARY_EVIDENCE["061"]["linker_version"],
        "resolved_link_recipe": ORDINARY_EVIDENCE["061"][
            "resolved_link_recipe"
        ],
        "source_hashes": source_hashes,
        "thresholds": report["constants"],
    }
    fingerprint_sha256 = hashlib.sha256(
        json.dumps(
            fingerprint_payload, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()
    report["fingerprint"] = {
        **fingerprint_payload,
        "sha256": fingerprint_sha256,
        "optimization": "ordinary core defaults plus exact no-LTO -Os probe",
        "language_standard": "gnu++11",
    }
    for review in LOADED_REVIEWS.values():
        if review["fingerprint_sha256"] != fingerprint_sha256:
            raise probe.ProbeError(
                "target-miss review fingerprint is stale: "
                f"{review['fingerprint_sha256']} != {fingerprint_sha256}"
            )
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if report["status"] in ("hard-fail", "review-required", "error") else 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through", choices=("061", "062"), default="061"
    )
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/museum-case-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/museum_case_resource_reviews.json",
    )
    arguments = parser.parse_args()
    selected_boundaries = tuple(
        boundary
        for boundary in BOUNDARIES
        if boundary["lesson"] <= arguments.require_through
    )
    compile_ordinary(arguments, selected_boundaries)
    probe.BOUNDARIES = selected_boundaries
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
