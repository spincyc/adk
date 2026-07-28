#!/usr/bin/env python3

"""Validate the generated ADK site without third-party Python packages."""

from __future__ import annotations

import argparse
import re
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

if __package__:
    from scripts.publication import PublicationConfigError
    from scripts.publication import resolve_latest_publication
else:
    from publication import PublicationConfigError
    from publication import resolve_latest_publication


MAX_PDF_SIZE = 50 * 1024 * 1024
MIN_PDF_SIZE = 1024
EXTERNAL_SCHEMES = {"http", "https", "mailto", "tel", "data"}
PROJECT_BASE_PATH = "/adk"
REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
BUILD_CONFIG = REPOSITORY_ROOT / "mk/config.mk"
FILESYSTEM_LEAKS = (
    re.compile(r"file://", re.IGNORECASE),
    re.compile(r"(?:^|[\"'=()\s])/(?:home|Users|private|tmp|var/tmp)/"),
    re.compile(r"(?:^|[\"'=()\s])[A-Za-z]:[\\/]"),
)


class DocumentParser(HTMLParser):
    """Collect the small set of facts needed for pragmatic site validation."""

    def __init__(self, path: Path) -> None:
        super().__init__(convert_charrefs=True)
        self.path = path
        self.errors: list[str] = []
        self.ids: set[str] = set()
        self.references: list[tuple[str, str, int]] = []
        self.title_count = 0
        self.title_parts: list[str] = []
        self.text_parts: list[str] = []
        self.title_depth = 0
        self.h1_count = 0
        self.main_count = 0
        self.html_count = 0
        self.has_language = False
        self.has_viewport = False

    def handle_starttag(
        self,
        tag: str,
        attributes: list[tuple[str, str | None]],
    ) -> None:
        self._handle_tag(tag, attributes, False)

    def handle_startendtag(
        self,
        tag: str,
        attributes: list[tuple[str, str | None]],
    ) -> None:
        self._handle_tag(tag, attributes, True)

    def handle_endtag(self, tag: str) -> None:
        if tag.lower() == "title" and self.title_depth > 0:
            self.title_depth -= 1

    def handle_data(self, data: str) -> None:
        self.text_parts.append(data)
        if self.title_depth > 0:
            self.title_parts.append(data)

    def error(self, message: str) -> None:
        self.errors.append(f"{self.path}:{self.getpos()[0]}: {message}")

    def _handle_tag(
        self,
        tag: str,
        attributes: list[tuple[str, str | None]],
        self_closing: bool,
    ) -> None:
        tag = tag.lower()
        attrs = {name.lower(): value for name, value in attributes}
        line = self.getpos()[0]

        if tag == "html":
            self.html_count += 1
            self.has_language |= bool((attrs.get("lang") or "").strip())
        elif tag == "title":
            self.title_count += 1
            self.title_depth += 1
            if self_closing:
                self.title_depth -= 1
        elif tag == "h1":
            self.h1_count += 1
        elif tag == "main" or attrs.get("role") == "main":
            self.main_count += 1
        elif tag == "meta":
            name = (attrs.get("name") or "").strip().lower()
            content = (attrs.get("content") or "").strip()
            self.has_viewport |= name == "viewport" and bool(content)

        element_id = attrs.get("id")
        if element_id:
            if element_id in self.ids:
                self.errors.append(
                    f"{self.path}:{line}: duplicate id {element_id!r}"
                )
            self.ids.add(element_id)

        if tag == "a" and attrs.get("name"):
            self.ids.add(attrs["name"] or "")

        if tag == "img" and "alt" not in attrs:
            self.errors.append(f"{self.path}:{line}: img is missing alt")

        for attribute in ("href", "src"):
            value = attrs.get(attribute)
            if value is not None:
                self.references.append((attribute, value.strip(), line))

    def finish(self) -> None:
        if self.html_count != 1:
            self.errors.append(
                f"{self.path}: expected one html element, found {self.html_count}"
            )
        if not self.has_language:
            self.errors.append(f"{self.path}: html element needs a nonempty lang")
        if self.title_count != 1:
            self.errors.append(
                f"{self.path}: expected one title, found {self.title_count}"
            )
        elif not "".join(self.title_parts).strip():
            self.errors.append(f"{self.path}: title is empty")
        if self.h1_count != 1:
            self.errors.append(
                f"{self.path}: expected one h1, found {self.h1_count}"
            )
        if self.path.name == "404.html":
            expected_main = {0, 1}
        else:
            expected_main = {1}
        if self.main_count not in expected_main:
            self.errors.append(
                f"{self.path}: expected one main, found {self.main_count}"
            )
        if not self.has_viewport:
            self.errors.append(f"{self.path}: missing nonempty viewport metadata")


def display_path(path: Path, root: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return str(path)


def resolve_reference(
    document: Path,
    raw_reference: str,
    root: Path,
) -> tuple[Path | None, str, str | None]:
    if not raw_reference:
        return document, "", None

    parsed = urlsplit(raw_reference)
    scheme = parsed.scheme.lower()
    if scheme in EXTERNAL_SCHEMES or raw_reference.startswith("//"):
        return None, parsed.fragment, None
    if scheme:
        return None, parsed.fragment, f"unsupported URL scheme {scheme!r}"

    decoded_path = unquote(parsed.path)
    if "\x00" in decoded_path:
        return None, parsed.fragment, "URL path contains a null byte"

    if decoded_path == PROJECT_BASE_PATH:
        decoded_path = "/"
    elif decoded_path.startswith(PROJECT_BASE_PATH + "/"):
        decoded_path = decoded_path[len(PROJECT_BASE_PATH) :]

    if decoded_path.startswith("/"):
        candidate = root / decoded_path.lstrip("/")
    else:
        candidate = document.parent / decoded_path

    candidate = candidate.resolve(strict=False)
    try:
        candidate.relative_to(root)
    except ValueError:
        return candidate, parsed.fragment, "local reference escapes the site root"

    if decoded_path.endswith("/") or candidate.is_dir():
        candidate /= "index.html"

    return candidate, unquote(parsed.fragment), None


def validate_pdf(path: Path, root: Path, errors: list[str]) -> None:
    shown = display_path(path, root)
    try:
        size = path.stat().st_size
        with path.open("rb") as stream:
            header = stream.read(5)
    except OSError as exception:
        errors.append(f"{shown}: cannot read PDF: {exception}")
        return

    if size < MIN_PDF_SIZE:
        errors.append(f"{shown}: PDF is suspiciously small ({size} bytes)")
    if size > MAX_PDF_SIZE:
        errors.append(f"{shown}: PDF exceeds {MAX_PDF_SIZE} bytes ({size} bytes)")
    if header != b"%PDF-":
        errors.append(f"{shown}: file does not have a PDF header")


def newest_configured_lesson(errors: list[str]) -> tuple[str, str] | None:
    try:
        source = BUILD_CONFIG.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exception:
        errors.append(f"{BUILD_CONFIG}: cannot read lesson configuration: {exception}")
        return None

    try:
        publication = resolve_latest_publication(source)
    except PublicationConfigError as exception:
        errors.append(f"{BUILD_CONFIG}: {exception}")
        return None
    return publication.number, publication.example


def has_reference(
    document: Path,
    parser: DocumentParser,
    expected: Path,
    root: Path,
) -> bool:
    for _attribute, reference, _line in parser.references:
        target, _fragment, problem = resolve_reference(document, reference, root)
        if problem is None and target is not None and target == expected:
            return True
    return False


def validate_newest_lesson(
    root: Path,
    documents: dict[Path, DocumentParser],
    errors: list[str],
) -> None:
    configured = newest_configured_lesson(errors)
    if configured is None:
        return
    lesson, example = configured

    landing = (root / "index.html").resolve()
    lesson_index = (root / "lessons/index.html").resolve()
    lesson_page = (root / f"lessons/{lesson}/index.html").resolve()
    expected = (
        (landing, lesson_page, f"landing page link to lesson {lesson}"),
        (lesson_index, lesson_page, f"lesson index link to lesson {lesson}"),
        (
            lesson_page,
            (root / f"downloads/lessons/{lesson}.pdf").resolve(),
            f"lesson {lesson} PDF download",
        ),
        (
            lesson_page,
            (root / f"downloads/sketches/{example}.ino").resolve(),
            f"lesson {lesson} example download",
        ),
    )

    for document, target, description in expected:
        parser = documents.get(document)
        if parser is None:
            errors.append(
                f"{display_path(document, root)}: required page is missing for "
                f"{description}"
            )
        elif not has_reference(document, parser, target, root):
            errors.append(
                f"{display_path(document, root)}: missing {description} "
                f"({display_path(target, root)})"
            )

    parser = documents.get(lesson_page)
    if parser is None:
        return
    visible_text = " ".join(" ".join(parser.text_parts).split())
    markers = (
        f"Lesson {lesson}",
        "Status:",
        "Energy class:",
        "Safety boundary:",
    )
    for marker in markers:
        if marker not in visible_text:
            errors.append(
                f"{display_path(lesson_page, root)}: missing newest-lesson "
                f"content marker {marker!r}"
            )


def validate_site(root: Path) -> list[str]:
    errors: list[str] = []
    root = root.resolve()

    if not root.is_dir():
        return [f"{root}: generated site directory does not exist"]

    documents: dict[Path, DocumentParser] = {}
    html_paths = sorted(root.rglob("*.html"))
    if not html_paths:
        errors.append(f"{root}: generated site contains no HTML files")

    for path in html_paths:
        shown = display_path(path, root)
        if path.is_symlink() and not path.resolve().is_relative_to(root):
            errors.append(f"{shown}: symlink escapes the site root")
            continue

        try:
            source = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exception:
            errors.append(f"{shown}: cannot read UTF-8 HTML: {exception}")
            continue

        for pattern in FILESYSTEM_LEAKS:
            match = pattern.search(source)
            if match:
                errors.append(
                    f"{shown}: possible absolute filesystem path leak "
                    f"{match.group(0).strip()!r}"
                )

        parser = DocumentParser(Path(shown))
        try:
            parser.feed(source)
            parser.close()
        except Exception as exception:
            errors.append(f"{shown}: HTML parser failed: {exception}")
            continue
        parser.finish()
        errors.extend(parser.errors)
        documents[path.resolve()] = parser

    checked_pdfs: set[Path] = set()
    for path in sorted(root.rglob("*")):
        if path.is_symlink() and not path.resolve().is_relative_to(root):
            errors.append(f"{display_path(path, root)}: symlink escapes the site root")
        if path.is_file() and path.suffix.lower() == ".pdf":
            resolved = path.resolve()
            validate_pdf(resolved, root, errors)
            checked_pdfs.add(resolved)

    for document, parser in documents.items():
        shown_document = display_path(document, root)
        for attribute, reference, line in parser.references:
            target, fragment, problem = resolve_reference(document, reference, root)
            if problem:
                errors.append(
                    f"{shown_document}:{line}: {attribute}={reference!r}: {problem}"
                )
                continue
            if target is None:
                continue

            shown_target = display_path(target, root)
            if not target.is_file():
                errors.append(
                    f"{shown_document}:{line}: {attribute}={reference!r}: "
                    f"missing local target {shown_target}"
                )
                continue

            if target.suffix.lower() == ".pdf" and target not in checked_pdfs:
                validate_pdf(target, root, errors)
                checked_pdfs.add(target)

            if fragment and target.suffix.lower() in {".html", ".htm"}:
                target_parser = documents.get(target.resolve())
                if target_parser is None:
                    errors.append(
                        f"{shown_document}:{line}: cannot inspect fragment "
                        f"{fragment!r} in {shown_target}"
                    )
                elif fragment not in target_parser.ids:
                    errors.append(
                        f"{shown_document}:{line}: missing fragment "
                        f"{fragment!r} in {shown_target}"
                    )

    validate_newest_lesson(root, documents, errors)
    return errors


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "site_root",
        nargs="?",
        default="build/site",
        type=Path,
        help="generated site directory (default: build/site)",
    )
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    errors = validate_site(arguments.site_root)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(
            f"ADK site validation failed with {len(errors)} error(s).",
            file=sys.stderr,
        )
        return 1

    print(f"ADK site validation passed: {arguments.site_root}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
