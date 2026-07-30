#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
import pathlib
import re
import shutil
import sys


ROOT = pathlib.Path(__file__).resolve().parent.parent
BASE_PATH = ROOT / "scripts/check_escape_console_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("adk_exact_resource_probe", BASE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load the shared exact AVR probe implementation")
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)
probe_assign_stack = probe.assign_stack
probe_exact_stack = probe.exact_stack

BOARD_SRAM = 8192
ISR_RESERVE = 128
RESIDUAL_SRAM_TARGET = 4096
RESIDUAL_SRAM_HARD = 3072
FIXED_BUFFER_TARGET = 128
FIXED_BUFFER_HARD = 160

RESOURCE_LAYOUTS = {}
ENRICHED_REVIEWS = {}
BOUNDARY_CONFIG_PATHS = {
    lesson: ROOT / f"probes/motion_recorder_boundary_{lesson}.json"
    for lesson in ("067", "068", "069")
}
ALL_BOUNDARIES = tuple(
    json.loads(BOUNDARY_CONFIG_PATHS[lesson].read_text(encoding="utf-8"))
    for lesson in ("067", "068", "069")
)
FINGERPRINT_CONTRACT = {
    boundary["lesson"]: boundary.pop("fingerprint_contract")
    for boundary in ALL_BOUNDARIES
}


def selected_boundaries(require_through):
    return tuple(
        boundary
        for boundary in ALL_BOUNDARIES
        if boundary["lesson"] <= require_through
    )


def fingerprint_source_paths(lesson):
    paths = [
        "scripts/check_escape_console_resource_probe.py",
        "scripts/check_motion_recorder_resource_probe.py",
        f"probes/motion_recorder_boundary_{lesson}.json",
        "probes/motion_recorder_object_sizes.cpp",
        "src/inertial_record.h",
        "src/inertial_record.cpp",
        "examples/Lesson067InertialRecordNormalization/"
        "Lesson067InertialRecordNormalization.ino",
    ]
    if lesson >= "068":
        paths.extend(
            (
                "src/signed_axis_mapping.h",
                "src/inertial_record_qualification.h",
                "src/inertial_record_qualification.cpp",
                "examples/Lesson068InertialRecordQualification/"
                "Lesson068InertialRecordQualification.ino",
            )
        )
    if lesson >= "069":
        paths.extend(
            (
                "src/qualified_motion_recorder.h",
                "src/qualified_motion_recorder.cpp",
                "examples/Lesson069InterchangeableMotionRecorder/"
                "Lesson069InterchangeableMotionRecorder.ino",
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
        source = ROOT / path
        if path == "scripts/check_motion_recorder_resource_probe.py":
            hashes[path] = shared_probe_source_hash()
        elif path == f"probes/motion_recorder_boundary_{lesson}.json":
            hashes[path] = boundary_configuration_hash(
                lesson, overrides.get(lesson)
            )
        elif path == "probes/motion_recorder_object_sizes.cpp":
            text = source.read_text(encoding="utf-8")
            if lesson < "069":
                text = re.sub(
                    r"\n#if defined \(ADK_HAS_LESSON_069\).*?\n#endif\n?",
                    "\n",
                    text,
                    flags=re.DOTALL,
                )
            if lesson < "068":
                text = re.sub(
                    r"\n#if defined \(ADK_HAS_LESSON_068\).*?\n#endif\n?",
                    "\n",
                    text,
                    flags=re.DOTALL,
                )
            hashes[path] = hashlib.sha256(text.encode("utf-8")).hexdigest()
        else:
            hashes[path] = probe.sha256(source)
    return hashes


def motion_assign_stack(functions, records):
    normalized_records = []
    for record in records:
        normalized = dict(record)
        normalized["signature"] = normalized["signature"].replace(
            "const SourceAxisMapping&", "const adk::SignedAxisMapping&"
        )
        normalized["signature"] = normalized["signature"].replace(
            "int64_t", "long long"
        )
        normalized["signature"] = normalized["signature"].replace(
            "uint64_t", "unsigned long long"
        )
        normalized_records.append(normalized)
    probe_assign_stack(functions, normalized_records)


def motion_exact_stack(functions, graph, dynamic, unresolved):
    for function in functions.values():
        if function["mangled"] in ("__prologue_saves__", "__epilogue_restores__"):
            function["stack"] = 0
            function["stack_kind"] = "static-objdump"
    filtered_dynamic = {
        address: (
            []
            if functions[address]["mangled"]
            in ("__prologue_saves__", "__epilogue_restores__")
            else transfers
        )
        for address, transfers in dynamic.items()
    }
    return probe_exact_stack(functions, graph, filtered_dynamic, unresolved)


def motion_object_sizes(compiler, nm, root, temporary, unused):
    object_path = temporary / "motion_recorder_object_sizes.o"
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
    for lesson, header in (
        ("068", root / "src/inertial_record_qualification.h"),
        ("069", root / "src/qualified_motion_recorder.h"),
    ):
        if header.is_file() and any(
            boundary["lesson"] == lesson for boundary in probe.BOUNDARIES
        ):
            command.append(f"-DADK_HAS_LESSON_{lesson}=1")
    command.extend(
        (
            str(root / "probes/motion_recorder_object_sizes.cpp"),
            "-o",
            str(object_path),
        )
    )
    probe.run(command, cwd=root)
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
    RESOURCE_LAYOUTS["067"] = {"commands": (command,), "symbols": symbols}
    if "InertialRecordQualificationConfigBytes" in symbols:
        RESOURCE_LAYOUTS["068"] = {"commands": (), "symbols": symbols}
    if "MotionRecorderConfigBytes" in symbols:
        RESOURCE_LAYOUTS["069"] = {"commands": (), "symbols": symbols}
    return symbols, command


def motion_enrich_evidence(evidence_path):
    if not evidence_path.is_file():
        return 0
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    report["constants"].update(
        {
            "motion_recorder_isr_reserve_bytes": ISR_RESERVE,
            "motion_recorder_residual_sram_target_bytes": RESIDUAL_SRAM_TARGET,
            "motion_recorder_residual_sram_hard_bytes": RESIDUAL_SRAM_HARD,
            "motion_record_cell_target_bytes": FIXED_BUFFER_TARGET,
            "motion_record_cell_hard_bytes": FIXED_BUFFER_HARD,
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
        symbols = RESOURCE_LAYOUTS.get(lesson, {}).get("symbols", {})
        residual = (
            BOARD_SRAM
            - state["measurements"]["static_sram_bytes"]
            - state["measurements"]["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        state["measurements"]["isr_reserve_bytes"] = ISR_RESERVE
        state["measurements"]["residual_sram_bytes"] = residual
        state["gates"]["residual_sram"] = (
            "pass" if residual >= RESIDUAL_SRAM_TARGET else "hard-fail"
        )
        state["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        if lesson == "067":
            image = symbols.get("inertialRecordImageCallerBufferBytes")
            codec = symbols.get("inertialRecordCodecObjectBytes")
            if image != 64 or codec is None:
                raise probe.ProbeError(
                    "Lesson 067 codec/image symbols do not prove the locked image"
                )
            state["measurements"]["canonical_image_bytes"] = image
            state["measurements"]["codec_object_bytes"] = codec
            state["gates"]["canonical_image"] = (
                "pass" if image == 64 else "hard-fail"
            )
            state["gates"]["codec_mutable_state"] = (
                "pass" if codec <= 1 else "hard-fail"
            )
        if lesson == "068":
            evidence = symbols.get("InertialQualificationEvidenceBytes")
            if evidence is None:
                raise probe.ProbeError(
                    "Lesson 068 qualification evidence symbol is missing"
                )
            state["measurements"]["qualification_evidence_bytes"] = evidence
            aggregate = state["measurements"]["object_bytes"] + evidence
            state["measurements"][
                "qualifier_plus_evidence_bytes"
            ] = aggregate
            state["gates"]["qualification_evidence"] = probe.gate(
                evidence, 224, 320
            )
            state["gates"]["qualifier_plus_evidence"] = probe.gate(
                aggregate, 640, 960
            )
        if lesson == "069":
            cell = symbols.get("MotionRecordImageBytes")
            if cell is None:
                raise probe.ProbeError(
                    "Lesson 069 canonical record-cell symbol is missing"
                )
            state["measurements"]["canonical_record_cell_bytes"] = cell
            state["gates"]["canonical_record_cell"] = probe.gate(
                cell, FIXED_BUFFER_TARGET, FIXED_BUFFER_HARD
            )
        if "hard-fail" in state["gates"].values():
            state["status"] = "hard-fail"
        elif "target-miss" in state["gates"].values():
            state["status"] = "review-required"
        fingerprint_payload = {
            "contract": FINGERPRINT_CONTRACT[lesson],
            "boundary": next(
                boundary
                for boundary in ALL_BOUNDARIES
                if boundary["lesson"] == lesson
            ),
            "measurements": state["measurements"],
            "source_hashes": fingerprint_source_hashes(lesson),
            "tools": {
                key: value
                for key, value in report.get("tools", {}).items()
                if key != "arduino_cli"
            },
        }
        state["fingerprint_contract"] = FINGERPRINT_CONTRACT[lesson]
        state["fingerprint_source_hashes"] = fingerprint_payload["source_hashes"]
        state["fingerprint_sha256"] = hashlib.sha256(
            json.dumps(
                fingerprint_payload,
                sort_keys=True,
                separators=(",", ":"),
            ).encode("utf-8")
        ).hexdigest()
        accepted = list(state.get("accepted_reviews", ()))
        for review in accepted:
            if review["fingerprint_sha256"] != state["fingerprint_sha256"]:
                raise probe.ProbeError(
                    f"stale Lesson {lesson} {review['metric']} resource review"
                )
        for metric, disposition in tuple(state["gates"].items()):
            if disposition != "target-miss":
                continue
            review = ENRICHED_REVIEWS.get((lesson, metric))
            if review is None:
                continue
            measurement_key = {
                "qualification_evidence": "qualification_evidence_bytes",
                "qualifier_plus_evidence": "qualifier_plus_evidence_bytes",
                "canonical_record_cell": "canonical_record_cell_bytes",
            }.get(metric)
            if (
                measurement_key is None
                or state["measurements"][measurement_key]
                != review["observed_bytes"]
                or review["fingerprint_sha256"]
                != state["fingerprint_sha256"]
            ):
                raise probe.ProbeError(
                    f"stale Lesson {lesson} {metric} resource review"
                )
            state["gates"][metric] = "reviewed-target-miss"
            accepted.append(review)
        state["accepted_reviews"] = accepted
        if "hard-fail" in state["gates"].values():
            state["status"] = "hard-fail"
        elif "target-miss" in state["gates"].values():
            state["status"] = "review-required"
        elif "reviewed-target-miss" in state["gates"].values():
            state["status"] = "reviewed-target-miss"
        else:
            state["status"] = "pass"
        report["status"] = probe.merge_status(report["status"], state["status"])
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if report["status"] in ("hard-fail", "review-required", "error") else 0


def load_reviews(root, review_path):
    global ENRICHED_REVIEWS
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(document.get("reviews"), list):
        raise probe.ProbeError(f"invalid target-miss review document: {path}")
    reviews = {}
    known_lessons = {boundary["lesson"] for boundary in probe.BOUNDARIES}
    authorities = {
        "docs/design/LESSONS_067_069_MOTION_RECORDER_PLAN.md"
        "#resource-and-ownership-budgets",
        "docs/design/LESSON_067_INERTIAL_RECORD_STRESS_PASS.md"
        "#initial-resource-gates",
        "docs/design/LESSON_068_INERTIAL_RECORD_QUALIFICATION_STRESS_PASS.md"
        "#maximum-composition-stress",
        "docs/design/LESSON_069_QUALIFIED_MOTION_RECORDER_STRESS_PASS.md"
        "#maximum-composition-stress",
    }
    required = {
        "lesson",
        "metric",
        "observed_bytes",
        "target_bytes",
        "hard_bytes",
        "authority",
        "disposition",
        "rationale",
        "fingerprint_sha256",
    }
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if review["lesson"] not in known_lessons:
            raise probe.ProbeError(f"target-miss review names unknown lesson: {review}")
        if review["disposition"] != "accepted-target-miss":
            raise probe.ProbeError(f"invalid target-miss disposition: {review}")
        if review["authority"] not in authorities:
            raise probe.ProbeError(
                f"target-miss review lacks controlling authority: {review}"
            )
        authority_path = root / review["authority"].split("#", 1)[0]
        authority_text = authority_path.read_text(encoding="utf-8").replace(
            ",", ""
        )
        if str(review["observed_bytes"]) not in authority_text:
            raise probe.ProbeError(
                f"target-miss tuple is absent from {review['authority']}: {review}"
            )
        key = (review["lesson"], review["metric"])
        if key in reviews:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        if review["metric"] in (
            "qualification_evidence",
            "qualifier_plus_evidence",
            "canonical_record_cell",
        ):
            ENRICHED_REVIEWS[key] = review
        else:
            reviews[key] = review
    return reviews


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through",
        choices=("067", "068", "069"),
        default="067",
    )
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/motion-recorder-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/motion_recorder_resource_reviews.json",
    )
    arguments = parser.parse_args()
    probe.BOUNDARIES = selected_boundaries(arguments.require_through)
    probe.object_sizes = motion_object_sizes
    probe.assign_stack = motion_assign_stack
    probe.exact_stack = motion_exact_stack
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
    evidence_result = motion_enrich_evidence(ROOT / arguments.evidence_json)
    return result or evidence_result


if __name__ == "__main__":
    raise SystemExit(main())
