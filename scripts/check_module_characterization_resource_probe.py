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
RECORD_IMAGE_EXACT = 192
SIMULTANEOUS_IMAGES_EXACT = 384
# Lessons 070--071 were published against this exact shared-probe source.
# Appending the isolated Lesson 072 branch must not rewrite their evidence
# identity when their selected algorithm and source projection are unchanged.
PUBLISHED_SHARED_PROBE_HASH = {
    "070": "a22675ac1b2457fc88f3666f47a3c073a6d5a53a3cdc2e00339c86bd518e236d",
    "071": "a22675ac1b2457fc88f3666f47a3c073a6d5a53a3cdc2e00339c86bd518e236d",
}

BOUNDARY_CONFIG_PATHS = {
    lesson: ROOT / f"probes/module_characterization_boundary_{lesson}.json"
    for lesson in ("070", "071", "072")
}
ALL_BOUNDARIES = tuple(
    json.loads(BOUNDARY_CONFIG_PATHS[lesson].read_text(encoding="utf-8"))
    for lesson in ("070", "071", "072")
)
FINGERPRINT_CONTRACT = {
    boundary["lesson"]: boundary.pop("fingerprint_contract")
    for boundary in ALL_BOUNDARIES
}
RESOURCE_LAYOUTS = {}
ENRICHED_REVIEWS = {}


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
    if lesson >= "072":
        paths.append("probes/module_characterization_boundary_072.json")
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
    if lesson >= "072":
        paths.extend(
            (
                "src/module_characterization_record.h",
                "src/module_characterization_record.cpp",
                "src/module_characterization_digest.h",
                "src/module_characterization_digest.cpp",
                "src/inert_module_characterization_bench.h",
                "src/inert_module_characterization_bench.cpp",
                "examples/Lesson072ModuleCharacterizationBench/"
                "Lesson072ModuleCharacterizationBench.ino",
            )
        )
    return tuple(paths)


def shared_probe_source_hash(source_text=None, lesson=None):
    if source_text is None and lesson in PUBLISHED_SHARED_PROBE_HASH:
        return PUBLISHED_SHARED_PROBE_HASH[lesson]
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
            hashes[path] = shared_probe_source_hash(lesson=lesson)
        elif path.startswith("probes/module_characterization_boundary_"):
            configured_lesson = pathlib.Path(path).stem.rsplit("_", 1)[1]
            hashes[path] = boundary_configuration_hash(
                configured_lesson, overrides.get(configured_lesson)
            )
        elif path == "probes/module_characterization_object_sizes.cpp":
            text = (ROOT / path).read_text(encoding="utf-8")
            if lesson < "072":
                text = re.sub(
                    r"\n#if defined \(ADK_HAS_LESSON_072\).*?\n#endif\n?",
                    "",
                    text,
                    flags=re.DOTALL,
                )
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
    if (
        (root / "src/inert_module_characterization_bench.h").is_file()
        and (root / "src/module_characterization_record.h").is_file()
        and any(boundary["lesson"] == "072" for boundary in probe.BOUNDARIES)
    ):
        command.append("-DADK_HAS_LESSON_072=1")
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
    if "inertModuleCharacterizationBenchBytes" in symbols:
        RESOURCE_LAYOUTS["072"] = symbols
    return symbols, command


def enrich_evidence(evidence_path):
    if not evidence_path.is_file():
        return 1
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    report["status"] = "pass"
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
            "module_characterization_record_image_exact_bytes":
                RECORD_IMAGE_EXACT,
            "module_characterization_simultaneous_images_exact_bytes":
                SIMULTANEOUS_IMAGES_EXACT,
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
        if lesson == "071":
            state["gates"]["evidence"] = probe.gate(
                evidence, EVIDENCE_TARGET, EVIDENCE_HARD
            )
            state["gates"]["caller_phase_local_point"] = probe.gate(
                point, POINT_TARGET, POINT_HARD
            )
        if lesson >= "072":
            record_image = symbols["moduleCharacterizationRecordImageBytes"]
            simultaneous_images = symbols[
                "moduleCharacterizationSimultaneousImagesBytes"
            ]
            state["measurements"].update(
                {
                    "record_image_bytes": record_image,
                    "simultaneous_record_images_bytes": simultaneous_images,
                }
            )
            state["gates"]["record_image_exact"] = exact_gate(
                record_image, RECORD_IMAGE_EXACT
            )
            state["gates"]["simultaneous_record_images_exact"] = exact_gate(
                simultaneous_images, SIMULTANEOUS_IMAGES_EXACT
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
        accepted = list(state.get("accepted_reviews", ()))
        for review in accepted:
            if review["fingerprint_sha256"] != state["fingerprint_sha256"]:
                raise probe.ProbeError(
                    f"stale Lesson {lesson} {review['metric']} resource review"
                )
        measurement_keys = {
            "flash": "flash_bytes",
            "static_sram": "static_sram_bytes",
            "synchronous_stack": "synchronous_stack_bytes",
            "evidence": "evidence_bytes",
        }
        boundary = next(
            item for item in ALL_BOUNDARIES if item["lesson"] == lesson
        )
        gate_limits = {
            "flash": (
                boundary["flash_target"],
                boundary["flash_hard"],
            ),
            "static_sram": (
                boundary["sram_target"],
                boundary["sram_hard"],
            ),
            "synchronous_stack": (
                boundary["stack_target"],
                boundary["stack_hard"],
            ),
            "evidence": (EVIDENCE_TARGET, EVIDENCE_HARD),
        }
        for (review_lesson, metric), review in ENRICHED_REVIEWS.items():
            if review_lesson != lesson:
                continue
            if state["gates"].get(metric) != "target-miss":
                raise probe.ProbeError(
                    f"stale Lesson {lesson} {metric} resource review"
                )
            measurement_key = measurement_keys.get(metric)
            limits = gate_limits.get(metric)
            if (
                measurement_key is None
                or limits is None
                or state["measurements"].get(measurement_key)
                != review["observed_bytes"]
                or limits != (
                    review["target_bytes"],
                    review["hard_bytes"],
                )
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


def minimum_gate(measured, target, hard):
    if measured >= target:
        return "pass"
    if measured >= hard:
        return "target-miss"
    return "hard-fail"


def exact_gate(measured, required):
    return "pass" if measured == required else "hard-fail"


def markdown_heading_slug(heading):
    normalized = re.sub(r"[^a-z0-9 -]", "", heading.lower())
    return re.sub(r"-+", "-", normalized.replace(" ", "-")).strip("-")


def authority_section(root, authority, review):
    authority_parts = authority.split("#", 1)
    if len(authority_parts) != 2 or not authority_parts[1]:
        raise probe.ProbeError(
            f"target-miss review authority lacks heading fragment: {review}"
        )
    authority_text = (root / authority_parts[0]).read_text(encoding="utf-8")
    headings = list(
        re.finditer(
            r"^(#{1,6})[ \t]+(.+?)[ \t]*#*[ \t]*$",
            authority_text,
            re.MULTILINE,
        )
    )
    for index, heading in enumerate(headings):
        if markdown_heading_slug(heading.group(2)) != authority_parts[1]:
            continue
        level = len(heading.group(1))
        end = len(authority_text)
        for following in headings[index + 1:]:
            if len(following.group(1)) <= level:
                end = following.start()
                break
        return authority_text[heading.start():end]
    raise probe.ProbeError(
        f"target-miss review authority heading not found: {review}"
    )


def load_reviews(root, review_path):
    global ENRICHED_REVIEWS
    ENRICHED_REVIEWS = {}
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(
        document.get("reviews"), list
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
        "fingerprint_sha256",
    }
    known_lessons = {boundary["lesson"] for boundary in probe.BOUNDARIES}
    authorities = {
        "071": (
            "docs/design/LESSON_071_THRESHOLD_CHARACTERIZATION_STRESS_PASS.md"
            "#gate-result"
        ),
        "072": (
            "docs/design/"
            "LESSON_072_INERT_MODULE_CHARACTERIZATION_BENCH_STRESS_PASS.md"
            "#terminal-gate-result"
        ),
    }
    supported_metrics = {
        "071": ("static_sram", "evidence"),
        "072": ("flash", "static_sram", "synchronous_stack"),
    }
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if review["lesson"] not in known_lessons:
            raise probe.ProbeError(
                f"target-miss review names unknown lesson: {review}"
            )
        if review["metric"] not in supported_metrics.get(review["lesson"], ()):
            raise probe.ProbeError(f"unsupported target-miss review: {review}")
        if review["disposition"] != "accepted-target-miss":
            raise probe.ProbeError(f"invalid target-miss disposition: {review}")
        authority = authorities[review["lesson"]]
        if review["authority"] != authority:
            raise probe.ProbeError(
                f"target-miss review lacks controlling authority: {review}"
            )
        authority_text = authority_section(root, authority, review).replace(",", "")
        if (
            not isinstance(review["rationale"], str)
            or not review["rationale"].strip()
            or str(review["observed_bytes"]) not in authority_text
        ):
            raise probe.ProbeError(
                f"target-miss tuple is absent from {review['authority']}: "
                f"{review}"
            )
        key = (review["lesson"], review["metric"])
        if key in ENRICHED_REVIEWS:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        ENRICHED_REVIEWS[key] = review
    return {}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through", choices=("070", "071", "072"), default="070"
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
    probe.main()
    enrichment_result = enrich_evidence(ROOT / arguments.evidence_json)
    return enrichment_result


if __name__ == "__main__":
    raise SystemExit(main())
