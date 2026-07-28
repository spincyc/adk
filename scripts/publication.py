#!/usr/bin/env python3

"""Resolve the latest published lesson from ADK's Make configuration."""

from __future__ import annotations

import re
from dataclasses import dataclass


ASSIGNMENT = re.compile(r"^([A-Z][A-Z0-9_]*)\s*(\?=|:=|\+=)\s*(.*)$")
EXAMPLE = re.compile(r"Lesson([0-9]{3})[A-Za-z0-9_]*")
LESSON = re.compile(r"[0-9]{3}")
SIMPLE_REFERENCE = re.compile(r"\$\([A-Z][A-Z0-9_]*\)")
SAFE_RULES = {"$(BUILD_MARKER):"}
SAFE_RECIPES = {'mkdir -p "$(BUILD_DIR)"', 'touch "$@"'}
SAFE_ASSIGNMENTS = {
    "ARDUINO_CLI": {"?="},
    "CXX": {"?="},
    "PDFLATEX": {"?="},
    "BUILD_DIR": {"?="},
    "BUILD_MARKER": {":="},
    "BOARD_FQBN": {"?="},
    "ARDUINO_AVR_CORE": {"?="},
    "LESSONS": {":="},
    "EXAMPLES": {":="},
    "PORT": {"?="},
    "BAUD": {"?="},
    "SERIAL_LOG": {"?="},
    "HOST_CPPFLAGS": {"+="},
    "HOST_CXXFLAGS": {"+="},
    "HOST_LDFLAGS": {"+="},
}
SAFE_REFERENCES = {
    "BUILD_MARKER": {"BUILD_DIR"},
    "SERIAL_LOG": {"BUILD_DIR"},
}


class PublicationConfigError(ValueError):
    """The publication inventory cannot be resolved unambiguously."""


@dataclass(frozen=True)
class PublishedLesson:
    """Identity and stable public paths for one configured lesson."""

    number: str
    example: str

    @property
    def lesson_page(self) -> str:
        return f"lessons/{self.number}/"

    @property
    def pdf_path(self) -> str:
        return f"downloads/lessons/{self.number}.pdf"

    @property
    def sketch_path(self) -> str:
        return f"downloads/sketches/{self.example}.ino"


def _parse_assignments(source: str) -> dict[str, list[tuple[str, list[str]]]]:
    assignments: dict[str, list[tuple[str, list[str]]]] = {}
    lines = source.splitlines()
    index = 0

    while index < len(lines):
        physical = lines[index]
        index += 1
        stripped = physical.strip()
        if "#" in physical and "\\" in physical.partition("#")[2]:
            raise PublicationConfigError(
                "configuration contains unsupported comment continuation"
            )
        if not stripped or stripped.startswith("#"):
            continue
        if physical.startswith("\t"):
            if stripped not in SAFE_RECIPES:
                raise PublicationConfigError("configuration contains unsupported recipe")
            continue
        if stripped in SAFE_RULES:
            continue

        value_parts: list[str] = []
        logical = physical
        while True:
            if "#" in logical and "\\" in logical.partition("#")[2]:
                raise PublicationConfigError(
                    "configuration contains unsupported comment continuation"
                )
            uncommented = logical.partition("#")[0].rstrip()
            continued = uncommented.endswith("\\")
            if continued:
                uncommented = uncommented[:-1].rstrip()
            value_parts.append(uncommented)
            if not continued:
                break
            if index >= len(lines):
                raise PublicationConfigError("configuration has an unterminated continuation")
            logical = lines[index]
            index += 1

        joined = " ".join(value_parts)
        match = ASSIGNMENT.fullmatch(joined.strip())
        if match is None:
            raise PublicationConfigError(
                "configuration contains unsupported Make syntax"
            )
        variable, operator, value = match.groups()
        if operator not in SAFE_ASSIGNMENTS.get(variable, set()):
            raise PublicationConfigError(
                f"configuration assignment to {variable} is unsupported"
            )
        references = set(SIMPLE_REFERENCE.findall(value))
        references_removed = SIMPLE_REFERENCE.sub("", value)
        if "$" in references_removed or "\\" in references_removed:
            raise PublicationConfigError(
                f"{variable} contains unsupported Make expansion"
            )
        allowed_references = {
            f"$({name})" for name in SAFE_REFERENCES.get(variable, set())
        }
        if not references.issubset(allowed_references):
            raise PublicationConfigError(
                f"{variable} contains unsupported Make reference"
            )
        assignments.setdefault(variable, []).append((operator, value.split()))

    return assignments


def _inventory_tokens(
    assignments: dict[str, list[tuple[str, list[str]]]],
    variable: str,
) -> list[str]:
    configured = assignments.get(variable, [])
    if len(configured) != 1 or configured[0][0] != ":=":
        raise PublicationConfigError(
            f"{variable} must have exactly one literal := assignment"
        )
    tokens = configured[0][1]
    if any("$" in token for token in tokens):
        raise PublicationConfigError(f"{variable} must be literal")
    return tokens


def resolve_latest_publication(config_source: str) -> PublishedLesson:
    """Return the newest lesson and its unique example from literal Make text."""
    assignments = _parse_assignments(config_source)
    lessons = _inventory_tokens(assignments, "LESSONS")
    if not lessons:
        raise PublicationConfigError("LESSONS contains no lesson numbers")
    malformed_lessons = [lesson for lesson in lessons if LESSON.fullmatch(lesson) is None]
    if malformed_lessons:
        raise PublicationConfigError(
            f"LESSONS contains malformed entry {malformed_lessons[0]!r}"
        )
    if len(set(lessons)) != len(lessons):
        raise PublicationConfigError("LESSONS contains duplicate lesson numbers")

    examples = _inventory_tokens(assignments, "EXAMPLES")
    malformed_examples = [
        example for example in examples if EXAMPLE.fullmatch(example) is None
    ]
    if malformed_examples:
        raise PublicationConfigError(
            f"EXAMPLES contains malformed entry {malformed_examples[0]!r}"
        )

    newest = max(lessons, key=int)
    matches = [
        example
        for example in examples
        if EXAMPLE.fullmatch(example).group(1) == newest
    ]
    if len(matches) != 1:
        raise PublicationConfigError(
            f"lesson {newest} must have exactly one configured example"
        )
    return PublishedLesson(newest, matches[0])
