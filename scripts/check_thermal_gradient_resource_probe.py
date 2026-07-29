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

BOUNDARY = {
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

PUBLIC_VALUES = (
    "OneWireRomCode",
    "OneWireTransactionConfig",
    "OneWireSearchState",
    "OneWireOperationRequest",
    "OneWireStepIntent",
    "OneWireStepReceipt",
    "OneWireTransactionSnapshot",
)

PUBLIC_ENUMS = (
    "OneWireSupplyMode",
    "OneWireOperation",
    "OneWirePhase",
    "OneWireLineIntent",
    "OneWireTransactionQuality",
)

LAYOUT_SYMBOLS = {}
LAYOUT_COMMANDS = ()
ORDINARY_EVIDENCE = {}
LOADED_REVIEWS = {}
LINKED_STORAGE = {}
COMPILE_DEPENDENCIES = {}
AUTHORITY_MARKERS = ()


def dependency_manifest(build_directory):
    dependencies = {}
    manifests = []
    for dependency_path in sorted(build_directory.rglob("*.d")):
        text = dependency_path.read_text(encoding="utf-8", errors="replace")
        flattened = text.replace("\\\n", " ")
        _, separator, payload = flattened.partition(":")
        if not separator:
            raise probe.ProbeError(
                f"compiler dependency manifest lacks a target: {dependency_path}"
            )
        entries = shlex.split(payload)
        normalized_entries = []
        for entry in entries:
            path = pathlib.Path(entry)
            if not path.is_absolute():
                path = (ROOT / path).resolve()
            normalized_path = normalized(str(path))
            normalized_entries.append(normalized_path)
            if not path.is_file():
                raise probe.ProbeError(
                    "compiler dependency is not an existing regular file: "
                    f"{entry} resolved as {path}"
                )
            dependencies[normalized_path] = probe.sha256(path)
        manifests.append(
            {
                "path": normalized(
                    str(dependency_path.relative_to(build_directory))
                ),
                "dependencies": sorted(set(normalized_entries)),
            }
        )
    if not manifests:
        raise probe.ProbeError(
            f"compiler dependency manifests are absent: {build_directory}"
        )
    return {
        "manifests": manifests,
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
            LINKED_STORAGE[match.group(3)] = int(match.group(1), 16)
    COMPILE_DEPENDENCIES["exact"] = dependency_manifest(build_directory)
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
    global LAYOUT_COMMANDS, LAYOUT_SYMBOLS
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

    layout_path = temporary / "thermal_gradient_public_layouts.cpp"
    layout_path.write_text(
        """
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
""".lstrip(),
        encoding="utf-8",
    )
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
    trait_path = temporary / "thermal_gradient_public_traits.cpp"
    trait_path.write_text(
        """
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
""".lstrip(),
        encoding="utf-8",
    )
    trait_command = [
        host_compiler,
        "-fsyntax-only",
        "-std=c++11",
        "-Isrc",
        str(trait_path),
    ]
    probe.run(trait_command, cwd=root)
    LAYOUT_SYMBOLS = read_symbols(nm, layout_object)
    LAYOUT_COMMANDS = (layout_command, trait_command)
    return object_symbols, object_command


def normalized(value):
    text = json.dumps(value, sort_keys=True, separators=(",", ":"))
    text = text.replace(str(ROOT), "<repo>")
    return re.sub(r"/tmp/adk-[^/\" ]+", "<temporary>", text)


def compile_ordinary(arguments):
    temporary = pathlib.Path(tempfile.mkdtemp(prefix="adk-thermal-ordinary."))
    try:
        sketch = ROOT / BOUNDARY["sketch"]
        build = temporary / "lesson-064"
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
                f"ordinary Lesson 064 build produced {len(elf_paths)} ELF files"
            )
        size_tool = probe.tool_beside(build, "avr-size")
        flash, static_sram = probe.section_sizes(size_tool, elf_paths[0])
        compile_database = build / "compile_commands.json"
        compile_units = json.loads(compile_database.read_text(encoding="utf-8"))
        compile_units.sort(
            key=lambda unit: (
                unit.get("file", ""),
                json.dumps(unit.get("arguments", ()), separators=(",", ":")),
            )
        )
        flattened = " ".join(
            argument
            for unit in compile_units
            for argument in unit.get("arguments", ())
        )
        if "-DF_CPU=16000000L" not in flattened:
            raise probe.ProbeError(
                "ordinary Lesson 064 build does not resolve F_CPU=16000000L"
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
            str(sketch),
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
        ORDINARY_EVIDENCE.update(
            {
                "command": command,
                "properties_command": properties_command,
                "flash_bytes": flash,
                "static_sram_bytes": static_sram,
                "elf_sha256": probe.sha256(elf_paths[0]),
                "compile_units": json.loads(normalized(compile_units)),
                "core_package": "arduino:avr",
                "core_version": core_version,
                "f_cpu_hz": 16000000,
                "linker_executable": str(linker),
                "linker_version": probe.tool_version(linker),
                "resolved_link_recipe": link_recipe,
            }
        )
        COMPILE_DEPENDENCIES["ordinary"] = dependency_manifest(build)
    finally:
        shutil.rmtree(temporary)


def load_reviews(root, review_path):
    global AUTHORITY_MARKERS, LOADED_REVIEWS
    path = root / review_path
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
        r"^Resource-review: lesson=064 metric=([a-z_]+) "
        r"observed=([0-9]+) target=([0-9]+) hard=([0-9]+) "
        r"disposition=accepted-target-miss$"
    )
    authority_markers = {}
    for line in outside_fence:
        stripped = line.strip()
        if not stripped.startswith("Resource-review: lesson=064 "):
            continue
        match = marker_pattern.fullmatch(stripped)
        if match is None:
            raise probe.ProbeError(
                f"malformed Lesson 064 resource-review marker: {stripped}"
            )
        metric = match.group(1)
        if metric in authority_markers:
            raise probe.ProbeError(
                f"duplicate Lesson 064 resource-review marker: {metric}"
            )
        authority_markers[metric] = stripped
    AUTHORITY_MARKERS = tuple(
        authority_markers[metric] for metric in sorted(authority_markers)
    )
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if (
            review["lesson"] != "064"
            or review["authority"] != authority
            or review["disposition"] != "accepted-target-miss"
        ):
            raise probe.ProbeError(f"invalid target-miss review: {review}")
        for field in ("observed_bytes", "target_bytes", "hard_bytes"):
            if type(review[field]) is not int or review[field] < 0:
                raise probe.ProbeError(
                    f"target-miss review has invalid {field}: {review}"
                )
        if not (
            review["target_bytes"]
            < review["observed_bytes"]
            <= review["hard_bytes"]
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
            f"Resource-review: lesson=064 metric={review['metric']} "
            f"observed={review['observed_bytes']} "
            f"target={review['target_bytes']} hard={review['hard_bytes']} "
            f"disposition={review['disposition']}"
        )
        if authority_markers.get(review["metric"]) != marker:
            raise probe.ProbeError(
                f"target-miss review marker is absent from {review['authority']}: "
                f"{marker}"
            )
        key = ("064", review["metric"])
        if key in reviews:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        reviews[key] = review
    LOADED_REVIEWS = reviews
    base_metrics = {"flash", "static_sram", "synchronous_stack", "object"}
    return {
        key: review for key, review in reviews.items() if key[1] in base_metrics
    }


def apply_enriched_reviews(state):
    measurements = state["measurements"]
    limits = {
        "ordinary_flash": (
            measurements["ordinary_flash_bytes"],
            BOUNDARY["flash_target"],
            BOUNDARY["flash_hard"],
        ),
        "ordinary_static_sram": (
            measurements["ordinary_static_sram_bytes"],
            BOUNDARY["sram_target"],
            BOUNDARY["sram_hard"],
        ),
        "request_caller_buffer": (
            measurements["caller_buffers"]["request_bytes"],
            CALLER_BUFFER_TARGET,
            CALLER_BUFFER_HARD,
        ),
        "search_caller_buffer": (
            measurements["caller_buffers"]["search_bytes"],
            CALLER_BUFFER_TARGET,
            CALLER_BUFFER_HARD,
        ),
        "intent_caller_buffer": (
            measurements["caller_buffers"]["intent_bytes"],
            CALLER_BUFFER_TARGET,
            CALLER_BUFFER_HARD,
        ),
        "receipt_caller_buffer": (
            measurements["caller_buffers"]["receipt_bytes"],
            CALLER_BUFFER_TARGET,
            CALLER_BUFFER_HARD,
        ),
        "snapshot_caller_buffer": (
            measurements["caller_buffers"]["snapshot_bytes"],
            CALLER_BUFFER_TARGET,
            CALLER_BUFFER_HARD,
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
                f"stale target-miss review for Lesson 064 {metric}: "
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
    report["commands"][0:0] = [
        ORDINARY_EVIDENCE["command"],
        ORDINARY_EVIDENCE["properties_command"],
    ]
    report["commands"].extend(LAYOUT_COMMANDS)
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
    measurements["ordinary_flash_bytes"] = ORDINARY_EVIDENCE["flash_bytes"]
    measurements["ordinary_static_sram_bytes"] = ORDINARY_EVIDENCE[
        "static_sram_bytes"
    ]
    state["gates"]["ordinary_flash"] = probe.gate(
        measurements["ordinary_flash_bytes"],
        BOUNDARY["flash_target"],
        BOUNDARY["flash_hard"],
    )
    state["gates"]["ordinary_static_sram"] = probe.gate(
        measurements["ordinary_static_sram_bytes"],
        BOUNDARY["sram_target"],
        BOUNDARY["sram_hard"],
    )
    for gate_name in ("ordinary_flash", "ordinary_static_sram"):
        if state["gates"][gate_name] == "target-miss":
            state["gates"][gate_name] = "review-required"

    measurements["public_enums"] = {
        name: {"size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"]}
        for name in PUBLIC_ENUMS
    }
    measurements["public_values"] = {
        name: {
            "size_bytes": LAYOUT_SYMBOLS[f"{name}Bytes"],
            "alignment_bytes": LAYOUT_SYMBOLS[f"{name}Alignment"],
            "standard_layout": True,
            "trivially_copyable": True,
            "trivially_destructible": True,
        }
        for name in PUBLIC_VALUES
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
            for symbol, size in LINKED_STORAGE.items()
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
    apply_enriched_reviews(state)

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

    source_paths = (
        "scripts/check_escape_console_resource_probe.py",
        "scripts/check_thermal_gradient_resource_probe.py",
        "probes/thermal_gradient_object_sizes.cpp",
        "src/one_wire_transaction_policy.h",
        "src/one_wire_transaction_policy.cpp",
        "examples/Lesson064OwnedSingleWireTransactions/"
        "Lesson064OwnedSingleWireTransactions.ino",
    )
    fingerprint_payload = {
        "schema": 3,
        "lesson_through": "064",
        "fqbn": report["fqbn"],
        "core_package": ORDINARY_EVIDENCE["core_package"],
        "core_version": ORDINARY_EVIDENCE["core_version"],
        "f_cpu_hz": ORDINARY_EVIDENCE["f_cpu_hz"],
        "tools": report["tools"],
        "commands": json.loads(normalized(report["commands"])),
        "ordinary_compile_units": ORDINARY_EVIDENCE["compile_units"],
        "compile_dependencies": COMPILE_DEPENDENCIES,
        "linker_executable": ORDINARY_EVIDENCE["linker_executable"],
        "linker_version": ORDINARY_EVIDENCE["linker_version"],
        "resolved_link_recipe": ORDINARY_EVIDENCE["resolved_link_recipe"],
        "source_hashes": {
            path: probe.sha256(ROOT / path) for path in source_paths
        },
        "authority_markers": list(AUTHORITY_MARKERS),
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
    for review in LOADED_REVIEWS.values():
        if review["fingerprint_sha256"] != fingerprint:
            raise probe.ProbeError(
                "target-miss review fingerprint is stale: "
                f"{review['fingerprint_sha256']} != {fingerprint}"
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
    report["status"] = state["status"]
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if state["status"] in ("hard-fail", "review-required", "error") else 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument("--require-through", choices=("064",), default="064")
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/thermal-gradient-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/thermal_gradient_resource_reviews.json",
    )
    arguments = parser.parse_args()
    compile_ordinary(arguments)
    probe.BOUNDARIES = (BOUNDARY,)
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
