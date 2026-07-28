import tempfile
import unittest
from pathlib import Path

from scripts.check_deployed_site import canonical_example
from scripts.check_deployed_site import check_deployment
from scripts.check_deployed_site import configured_publication
from scripts.check_deployed_site import deployment_checks
from scripts.check_deployed_site import normalize_base_url


class DeployedSiteTest(unittest.TestCase):
    def test_file_url_fixture_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            publication = configured_publication()
            canonical_sketch = canonical_example(publication).read_bytes()
            contents = {
                "index.html": (
                    b"<!doctype html><title>ADK</title>"
                    b"Lessons 001\xe2\x80\x93"
                    + publication.number.encode()
                    + b" Planned course"
                ),
                f"lessons/{publication.number}/index.html": (
                    b"<!doctype html><title>036</title>"
                    b"Lesson "
                    + publication.number.encode()
                    + b" Status: Energy class: Safety boundary:"
                ),
                f"downloads/lessons/{publication.number}.pdf": b"%PDF-1.7\nfixture",
                f"downloads/sketches/{publication.example}.ino": canonical_sketch,
            }
            for relative, data in contents.items():
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)

            self.assertEqual(
                check_deployment(
                    root.as_uri(),
                    retries=0,
                    publication=publication,
                ),
                [],
            )

    def test_retry_is_bounded_and_reports_every_failed_artifact(self):
        calls: list[str] = []
        delays: list[float] = []

        def fail(url: str, timeout: float) -> bytes:
            calls.append(url)
            self.assertEqual(timeout, 3.0)
            raise OSError("fixture unavailable")

        publication = configured_publication()
        errors = check_deployment(
            "https://example.invalid/adk",
            retries=1,
            timeout=3.0,
            retry_delay=0.0,
            fetch=fail,
            sleep=delays.append,
            publication=publication,
        )

        checks = deployment_checks(publication)
        self.assertEqual(len(errors), len(checks))
        self.assertEqual(len(calls), len(checks) * 2)
        self.assertEqual(delays, [0.0] * len(checks))
        self.assertTrue(all("after 2 attempt(s)" in error for error in errors))

    def test_validation_failure_can_recover_on_retry(self):
        publication = configured_publication()
        attempts: dict[str, int] = {}

        def eventually_valid(url: str, timeout: float) -> bytes:
            attempts[url] = attempts.get(url, 0) + 1
            if attempts[url] == 1:
                return b""
            if url.endswith(".pdf"):
                return b"%PDF-1.7\n"
            if url.endswith(".ino"):
                return canonical_example(publication).read_bytes()
            if url.endswith(publication.lesson_page):
                return (
                    b"Lesson "
                    + publication.number.encode()
                    + b" Status: Energy class: Safety boundary:"
                )
            return (
                b"Lessons 001\xe2\x80\x93"
                + publication.number.encode()
                + b" Planned course"
            )

        self.assertEqual(
            check_deployment(
                "https://example.invalid/adk/",
                retries=1,
                retry_delay=0.0,
                fetch=eventually_valid,
                sleep=lambda delay: None,
                publication=publication,
            ),
            [],
        )
        self.assertEqual(set(attempts.values()), {2})

    def test_checks_follow_newest_published_lesson(self):
        publication = configured_publication()
        checks = deployment_checks(publication)

        self.assertEqual(checks[1].path, publication.lesson_page)
        self.assertEqual(checks[2].path, publication.pdf_path)
        self.assertEqual(checks[3].path, publication.sketch_path)
        self.assertEqual(canonical_example(publication).parent.name, publication.example)

    def test_base_url_validation(self):
        self.assertEqual(
            normalize_base_url("https://example.test/adk"),
            "https://example.test/adk/",
        )
        with self.assertRaisesRegex(ValueError, "https or file"):
            normalize_base_url("ftp://example.test/adk")
        with self.assertRaisesRegex(ValueError, "https or file"):
            normalize_base_url("http://example.test/adk")
        with self.assertRaisesRegex(ValueError, "credentials"):
            normalize_base_url("https://user@example.test/adk")
        with self.assertRaisesRegex(ValueError, "query or fragment"):
            normalize_base_url("https://example.test/adk?draft=1")


if __name__ == "__main__":
    unittest.main()
