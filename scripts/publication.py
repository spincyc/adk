#!/usr/bin/env python3

"""Resolve the latest published lesson from ADK's Make configuration."""

from __future__ import annotations

import re
from dataclasses import dataclass


ASSIGNMENT = re.compile(r"^\s*(LESSONS|EXAMPLES)\s*([:+?]?=)\s*(.*)$")
EXAMPLE = re.compile(r"Lesson([0-9]{3})[A-Za-z0-9_]*")
LESSON = re.compile(r"[0-9]{3}")


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


def _configured_tokens(source: str, variable: str) -> list[str]:
    assignments: list[list[str]] = []
    lines = source.splitlines()
    index = 0

    while index < len(lines):
        line = lines[index]
        index += 1
        match = ASSIGNMENT.match(line)
        if match is None or match.group(1) != variable:
            continue
        if match.group(2) != ":=":
            raise PublicationConfigError(
                f"{variable} must use a single literal := assignment"
            )

        value_parts: list[str] = []
        remainder = match.group(3)
        while True:
            uncommented = remainder.partition("#")[0].rstrip()
            continued = uncommented.endswith("\\")
            if continued:
                uncommented = uncommented[:-1].rstrip()
            if "\\" in uncommented or "$" in uncommented:
                raise PublicationConfigError(
                    f"{variable} contains unsupported Make syntax"
                )
            value_parts.append(uncommented)
            if not continued:
                break
            if index >= len(lines):
                raise PublicationConfigError(
                    f"{variable} has an unterminated continuation"
                )
            remainder = lines[index]
            index += 1

        assignments.append(" ".join(value_parts).split())

    if len(assignments) != 1:
        raise PublicationConfigError(
            f"{variable} must have exactly one literal := assignment"
        )
    return assignments[0]


def resolve_latest_publication(config_source: str) -> PublishedLesson:
    """Return the newest lesson and its unique example from literal Make text."""
    lessons = _configured_tokens(config_source, "LESSONS")
    if not lessons:
        raise PublicationConfigError("LESSONS contains no lesson numbers")
    malformed_lessons = [lesson for lesson in lessons if LESSON.fullmatch(lesson) is None]
    if malformed_lessons:
        raise PublicationConfigError(
            f"LESSONS contains malformed entry {malformed_lessons[0]!r}"
        )
    if len(set(lessons)) != len(lessons):
        raise PublicationConfigError("LESSONS contains duplicate lesson numbers")

    examples = _configured_tokens(config_source, "EXAMPLES")
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
