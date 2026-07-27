import tempfile
import unittest
from pathlib import Path

from scripts.check_deployed_site import CHECKS
from scripts.check_deployed_site import check_deployment
from scripts.check_deployed_site import normalize_base_url


class DeployedSiteTest(unittest.TestCase):
    def test_file_url_fixture_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            canonical_example = (
                Path(__file__).resolve().parents[1]
                / "examples/Lesson030InertShowSimulator/"
                "Lesson030InertShowSimulator.ino"
            ).read_bytes()
            contents = {
                "index.html": (
                    b"<!doctype html><title>ADK</title>"
                    b"lessons 001\xe2\x80\x93030 physically inert show-cue"
                ),
                "lessons/030/index.html": (
                    b"<!doctype html><title>030</title>"
                    b"Reviewed inert show-cue simulator "
                    b"synthetic observations and inert visual cues only"
                ),
                "downloads/lessons/030.pdf": b"%PDF-1.7\nfixture",
                "downloads/sketches/Lesson030InertShowSimulator.ino": (
                    canonical_example
                ),
            }
            for relative, data in contents.items():
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)

            self.assertEqual(
                check_deployment(root.as_uri(), retries=0),
                [],
            )

    def test_retry_is_bounded_and_reports_every_failed_artifact(self):
        calls: list[str] = []
        delays: list[float] = []

        def fail(url: str, timeout: float) -> bytes:
            calls.append(url)
            self.assertEqual(timeout, 3.0)
            raise OSError("fixture unavailable")

        errors = check_deployment(
            "https://example.invalid/adk",
            retries=1,
            timeout=3.0,
            retry_delay=0.0,
            fetch=fail,
            sleep=delays.append,
        )

        self.assertEqual(len(errors), len(CHECKS))
        self.assertEqual(len(calls), len(CHECKS) * 2)
        self.assertEqual(delays, [0.0] * len(CHECKS))
        self.assertTrue(all("after 2 attempt(s)" in error for error in errors))

    def test_validation_failure_can_recover_on_retry(self):
        attempts: dict[str, int] = {}

        def eventually_valid(url: str, timeout: float) -> bytes:
            attempts[url] = attempts.get(url, 0) + 1
            if attempts[url] == 1:
                return b""
            if url.endswith(".pdf"):
                return b"%PDF-1.7\n"
            if url.endswith(".ino"):
                return (
                    Path(__file__).resolve().parents[1]
                    / "examples/Lesson030InertShowSimulator/"
                    "Lesson030InertShowSimulator.ino"
                ).read_bytes()
            if url.endswith("lessons/030/"):
                return (
                    b"Reviewed inert show-cue simulator "
                    b"synthetic observations and inert visual cues only"
                )
            return b"lessons 001\xe2\x80\x93030 physically inert show-cue"

        self.assertEqual(
            check_deployment(
                "https://example.invalid/adk/",
                retries=1,
                retry_delay=0.0,
                fetch=eventually_valid,
                sleep=lambda delay: None,
            ),
            [],
        )
        self.assertEqual(set(attempts.values()), {2})

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
