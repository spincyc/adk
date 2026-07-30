#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
import pathlib
import re
import sys


ROOT = pathlib.Path(__file__).resolve().parent.parent
BASE_PATH = ROOT / "scripts/check_escape_console_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("adk_exact_resource_probe", BASE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load the shared exact AVR probe implementation")
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)

BOARD_SRAM = 8192
ISR_RESERVE = 128
RESIDUAL_SRAM_TARGET = 4096
RESIDUAL_SRAM_HARD = 3072
DESCRIPTOR_TARGET = 64
DESCRIPTOR_HARD = 96
EVIDENCE_TARGET = 320
EVIDENCE_HARD = 384
POINT_TARGET = 96
POINT_HARD = 128

BOUNDARY_CONFIG_PATHS = {
    lesson: ROOT / f"probes/module_characterization_boundary_{lesson}.json"
    for lesson in ("070", "071")
}
ALL_BOUNDARIES = tuple(
    json.loads(BOUNDARY_CONFIG_PATHS[lesson].read_text(encoding="utf-8"))
    for lesson in ("070", "071")
)
FINGERPRINT_CONTRACT = {
    boundary["lesson"]: boundary.pop("fingerprint_contract")
    for boundary in ALL_BOUNDARIES
}
RESOURCE_LAYOUTS = {}


def selected_boundaries(require_through):
    return tuple(
        boundary
        for boundary in ALL_BOUNDARIES
        if boundary["lesson"] <= require_through
    )


def fingerprint_source_paths(lesson):
    if lesson not in BOUNDARY_CONFIG_PATHS:
        raise ValueError(f"unsupported lesson boundary: {lesson}")
    paths = [
        "scripts/check_escape_console_resource_probe.py",
        "scripts/check_module_characterization_resource_probe.py",
        "probes/module_characterization_boundary_070.json",
    ]
    if lesson >= "071":
        paths.append("probes/module_characterization_boundary_071.json")
    paths.extend(
        (
            "probes/module_characterization_object_sizes.cpp",
            "src/module_threshold_descriptor.h",
            "src/module_threshold_descriptor.cpp",
            "examples/Lesson070ThresholdDescriptor/"
            "Lesson070ThresholdDescriptor.ino",
        )
    )
    if lesson >= "071":
        paths.extend(
            (
                "src/module_characterization.h",
                "src/module_characterization.cpp",
                "examples/Lesson071Characterization/"
                "Lesson071Characterization.ino",
            )
        )
    return tuple(paths)


def shared_probe_source_hash(source_text=None):
    text = (
        pathlib.Path(__file__).read_text(encoding="utf-8")
        if source_text is None
        else source_text
    )
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def boundary_configuration_hash(lesson, source_text=None):
    text = (
        BOUNDARY_CONFIG_PATHS[lesson].read_text(encoding="utf-8")
        if source_text is None
        else source_text
    )
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def fingerprint_source_hashes(lesson, boundary_config_overrides=None):
    overrides = (
        {} if boundary_config_overrides is None else boundary_config_overrides
    )
    hashes = {}
    for path in fingerprint_source_paths(lesson):
        if path == "scripts/check_module_characterization_resource_probe.py":
            hashes[path] = shared_probe_source_hash()
        elif path.startswith("probes/module_characterization_boundary_"):
            configured_lesson = pathlib.Path(path).stem.rsplit("_", 1)[1]
            hashes[path] = boundary_configuration_hash(
                configured_lesson, overrides.get(configured_lesson)
            )
        elif path == "probes/module_characterization_object_sizes.cpp":
            text = (ROOT / path).read_text(encoding="utf-8")
            if lesson < "071":
                text = re.sub(
                    r"\n#if defined \(ADK_HAS_LESSON_071\).*?\n#endif\n?",
                    "\n",
                    text,
                    flags=re.DOTALL,
                )
            hashes[path] = hashlib.sha256(text.encode("utf-8")).hexdigest()
        else:
            hashes[path] = probe.sha256(ROOT / path)
    return hashes


def read_size_symbols(nm, object_path):
    symbols = {}
    for line in probe.output(
        (str(nm), "--print-size", "--size-sort", str(object_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+(\w+Bytes)$",
            line,
        )
        if match:
            symbols[match.group(2)] = int(match.group(1), 16)
    return symbols


def object_sizes(compiler, nm, root, temporary, unused):
    object_path = temporary / "module_characterization_object_sizes.o"
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
    if (
        (root / "src/module_characterization.h").is_file()
        and any(boundary["lesson"] == "071" for boundary in probe.BOUNDARIES)
    ):
        command.append("-DADK_HAS_LESSON_071=1")
    command.extend(
        (
            str(root / "probes/module_characterization_object_sizes.cpp"),
            "-o",
            str(object_path),
        )
    )
    probe.run(command, cwd=root)
    symbols = read_size_symbols(nm, object_path)
    RESOURCE_LAYOUTS["070"] = symbols
    if "moduleCharacterizationPolicyBytes" in symbols:
        RESOURCE_LAYOUTS["071"] = symbols
    return symbols, command


def enrich_evidence(evidence_path):
    if not evidence_path.is_file():
        return 1
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    report["constants"].update(
        {
            "module_characterization_isr_reserve_bytes": ISR_RESERVE,
            "module_characterization_residual_sram_target_bytes":
                RESIDUAL_SRAM_TARGET,
            "module_characterization_residual_sram_hard_bytes":
                RESIDUAL_SRAM_HARD,
            "module_threshold_descriptor_target_bytes": DESCRIPTOR_TARGET,
            "module_threshold_descriptor_hard_bytes": DESCRIPTOR_HARD,
            "module_characterization_evidence_target_bytes": EVIDENCE_TARGET,
            "module_characterization_evidence_hard_bytes": EVIDENCE_HARD,
            "module_characterization_point_target_bytes": POINT_TARGET,
            "module_characterization_point_hard_bytes": POINT_HARD,
        }
    )
    for state in report["boundaries"]:
        if "measurements" not in state or not {
            "static_sram_bytes",
            "synchronous_stack_bytes",
            "object_bytes",
        }.issubset(state["measurements"]):
            continue
        lesson = state["lesson"]
        symbols = RESOURCE_LAYOUTS[lesson]
        descriptor = symbols["moduleThresholdDescriptorBytes"]
        frame = symbols["moduleThresholdFrameBytes"]
        residual = (
            BOARD_SRAM
            - state["measurements"]["static_sram_bytes"]
            - state["measurements"]["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        state["measurements"].update(
            {
                "descriptor_bytes": descriptor,
                "frame_bytes": frame,
                "isr_reserve_bytes": ISR_RESERVE,
                "residual_sram_bytes": residual,
            }
        )
        state["gates"]["descriptor"] = probe.gate(
            descriptor, DESCRIPTOR_TARGET, DESCRIPTOR_HARD
        )
        if lesson >= "071":
            evidence = symbols["moduleCharacterizationEvidenceBytes"]
            point = symbols["moduleCharacterizationPointBytes"]
            state["measurements"].update(
                {
                    "evidence_bytes": evidence,
                    "caller_phase_local_point_bytes": point,
                }
            )
            state["gates"]["evidence"] = probe.gate(
                evidence, EVIDENCE_TARGET, EVIDENCE_HARD
            )
            state["gates"]["caller_phase_local_point"] = probe.gate(
                point, POINT_TARGET, POINT_HARD
            )
        state["gates"]["residual_sram"] = minimum_gate(
            residual, RESIDUAL_SRAM_TARGET, RESIDUAL_SRAM_HARD
        )
        state["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        payload = {
            "boundary": next(
                item for item in ALL_BOUNDARIES if item["lesson"] == lesson
            ),
            "contract": FINGERPRINT_CONTRACT[lesson],
            "measurements": state["measurements"],
            "source_hashes": fingerprint_source_hashes(lesson),
            "tools": {
                key: value
                for key, value in report.get("tools", {}).items()
                if key != "arduino_cli"
            },
        }
        state["fingerprint_contract"] = FINGERPRINT_CONTRACT[lesson]
        state["fingerprint_source_hashes"] = payload["source_hashes"]
        state["fingerprint_sha256"] = hashlib.sha256(
            json.dumps(payload, sort_keys=True, separators=(",", ":")).encode(
                "utf-8"
            )
        ).hexdigest()
        if "hard-fail" in state["gates"].values():
            state["status"] = "hard-fail"
        elif "target-miss" in state["gates"].values():
            state["status"] = "review-required"
        report["status"] = probe.merge_status(report["status"], state["status"])
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if report["status"] in ("hard-fail", "review-required", "error") else 0


def minimum_gate(measured, target, hard):
    if measured >= target:
        return "pass"
    if measured >= hard:
        return "target-miss"
    return "hard-fail"


def load_reviews(root, review_path):
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document != {"reviews": [], "schema": 1}:
        raise probe.ProbeError(
            "Lesson 070--071 target-miss reviews are not enabled before "
            "measurement"
        )
    return {}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through", choices=("070", "071"), default="070"
    )
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/module-characterization-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/module_characterization_resource_reviews.json",
    )
    arguments = parser.parse_args()
    probe.BOUNDARIES = selected_boundaries(arguments.require_through)
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
    return result or enrich_evidence(ROOT / arguments.evidence_json)


if __name__ == "__main__":
    raise SystemExit(main())
