"""Package history, compiler identity and incremental Make regressions.

Downloads use tiny local archives and firmware compilation uses a recorder;
these check the build graph and verification, not the compiler's correctness.
"""

import copy
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest

ROOT = Path (__file__).resolve ().parents[1]
sys.dont_write_bytecode = True
sys.path.insert (0, str (ROOT / "docs" / "_theme"))

import boards  # noqa: E402
from board_history import compare, records, systems  # noqa: E402
from mkdocs.exceptions import PluginError  # noqa: E402

SCRATCH = Path (os.environ.get ("ADK_TEST_BUILD_DIR", ROOT / "build"))
SCRATCH.mkdir (parents=True, exist_ok=True)
MANIFEST = json.loads ((ROOT / "boards" / "toolchain.json").read_text ())
HISTORY = (ROOT / "boards" / "published.txt").read_text ()


class History (unittest.TestCase):
    def test_duplicate_published_version (self):
        version, (checksum, compiler) = next (iter (records (HISTORY).items ()))
        # Appending used to replace the digest used by package generation.
        duplicate = HISTORY + f"\n{version} {'a' * 64} {compiler}\n"
        with tempfile.TemporaryDirectory (dir=SCRATCH) as folder:
            source = Path (folder) / "published.txt"
            source.write_text (duplicate)
            previous = boards.PUBLISHED
            try:
                boards.PUBLISHED = source
                with self.assertRaisesRegex (PluginError, "duplicate version"):
                    boards.published (version, "a" * 64, compiler)
            finally:
                boards.PUBLISHED = previous
        with self.assertRaisesRegex (ValueError, "duplicate version"):
            records (HISTORY + f"\n{version} {checksum} {compiler}\n")

    def test_immutable_platform_records (self):
        old = records (HISTORY)
        compare (old, records ("# new comment\n" + HISTORY), MANIFEST, MANIFEST)
        new = records (HISTORY + "\n99.0.0 " + "a" * 64 + " compiler@99\n")
        compare (old, new, MANIFEST, MANIFEST)
        with self.assertRaisesRegex (ValueError, "changed or was removed"):
            compare (old, {}, MANIFEST, MANIFEST)
        changed = dict (old)
        changed[next (iter (old))] = ("b" * 64, "compiler@99")
        with self.assertRaisesRegex (ValueError, "changed or was removed"):
            compare (old, changed, MANIFEST, MANIFEST)

    def test_immutable_compiler_artifacts (self):
        old = records (HISTORY)
        # A valid replacement archive still needs a new compiler version.
        buffer = io.BytesIO ()
        with tarfile.open (fileobj=buffer, mode="w:bz2") as archive:
            entry = tarfile.TarInfo ("compiler/bin/avr-g++")
            entry.size = 3
            archive.addfile (entry, io.BytesIO (b"new"))
        replacement = buffer.getvalue ()
        altered = copy.deepcopy (MANIFEST)
        altered["systems"][0].update (
            checksum="SHA-256:" + hashlib.sha256 (replacement).hexdigest (),
            size=str (len (replacement)))
        with self.assertRaisesRegex (ValueError, "new compiler version"):
            compare (old, old, MANIFEST, altered)
        for field, value in (("archiveFileName", "renamed.tar.bz2"), ("size", "1")):
            altered = copy.deepcopy (MANIFEST)
            altered["systems"][0][field] = value
            with self.subTest (field=field), self.assertRaisesRegex (ValueError, "changed"):
                compare (old, old, MANIFEST, altered)
        altered = copy.deepcopy (MANIFEST)
        altered["systems"].pop ()
        with self.assertRaisesRegex (ValueError, "removed"):
            compare (old, old, MANIFEST, altered)

    def test_mirrors_hosts_and_versions (self):
        old = records (HISTORY)
        altered = copy.deepcopy (MANIFEST)
        altered["systems"][0]["url"] = "https://mirror.example/compiler.tar.bz2"
        compare (old, old, MANIFEST, altered)
        extra = dict (altered["systems"][0], host="new-host")
        altered["systems"].append (extra)
        compare (old, old, MANIFEST, altered)
        altered["version"] += "-new"
        altered["systems"][0]["checksum"] = "SHA-256:" + "c" * 64
        compare (old, old, MANIFEST, altered)
        altered["systems"].append (extra)
        with self.assertRaisesRegex (ValueError, "duplicate host"):
            systems (altered)

    def test_ci_compares_base_to_working_files (self):
        with tempfile.TemporaryDirectory (dir=SCRATCH) as folder:
            root = Path (folder)
            (root / "boards").mkdir ()
            (root / "docs" / "_theme").mkdir (parents=True)
            helper = root / "docs" / "_theme" / "board_history.py"
            shutil.copyfile (ROOT / "docs" / "_theme" / helper.name, helper)
            history = root / "boards" / "published.txt"
            history.write_text (HISTORY)
            manifest = root / "boards" / "toolchain.json"
            manifest.write_text (json.dumps (MANIFEST))

            def git (*args):
                return subprocess.run (["git", *args], cwd=root, check=True,
                                       capture_output=True, text=True).stdout.strip ()

            git ("init", "-q")
            git ("add", "boards")
            base = git ("write-tree")

            def check ():
                return subprocess.run ([sys.executable, str (helper), base], cwd=root,
                                       capture_output=True, text=True)

            self.assertEqual (check ().returncode, 0)
            altered = copy.deepcopy (MANIFEST)
            altered["systems"][0]["checksum"] = "SHA-256:" + "d" * 64
            manifest.write_text (json.dumps (altered))
            self.assertIn ("new compiler version", check ().stderr)
            manifest.write_text (json.dumps (MANIFEST))
            history.write_text (HISTORY + "\n" + HISTORY.splitlines ()[-1])
            self.assertIn ("duplicate version", check ().stderr)


class Make (unittest.TestCase):
    def setUp (self):
        self.temporary = tempfile.TemporaryDirectory (dir=SCRATCH)
        self.addCleanup (self.temporary.cleanup)
        self.root = Path (self.temporary.name).resolve ()
        self.write ("Makefile", (ROOT / "Makefile").read_text ())
        self.env = dict (os.environ, PYTHONPYCACHEPREFIX=str (self.root / "build/pycache"))
        # A parent make exports command-line settings as well as MAKEFLAGS.
        # The miniature repository must use its own build and compiler paths.
        for setting in ("MAKEFLAGS", "MFLAGS", "MAKEOVERRIDES", "GNUMAKEFLAGS",
                        "BUILD_DIR", "VENV", "TOOLCHAIN"):
            self.env.pop (setting, None)
        self.manifest = copy.deepcopy (MANIFEST)
        self.system = self.manifest["systems"][0]
        self.manifest["systems"] = [self.system]
        self.save_manifest ()

    def write (self, name, text):
        path = self.root / name
        path.parent.mkdir (parents=True, exist_ok=True)
        path.write_text (text)
        return path

    def save_manifest (self):
        self.write ("boards/toolchain.json", json.dumps (self.manifest))

    def make (self, *args, success=True):
        result = subprocess.run (["make", "--no-print-directory", "HOST=Linux-x86_64",
                                  f"PYTHON={sys.executable}", *args], cwd=self.root,
                                 env=self.env, text=True, capture_output=True)
        if success:
            self.assertEqual (result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual (result.returncode, 0, result.stdout + result.stderr)
        return result.stdout + result.stderr

    def archive (self, contents):
        path = self.root / "compiler.tar.bz2"
        with tarfile.open (path, "w:bz2") as archive:
            entry = tarfile.TarInfo ("same-basename/bin/avr-g++")
            entry.mode = 0o755
            entry.size = len (contents)
            archive.addfile (entry, io.BytesIO (contents))
        self.system.update (url=path.as_uri (), archiveFileName=path.name,
                            checksum="SHA-256:" + hashlib.sha256 (path.read_bytes ()).hexdigest (),
                            size=str (path.stat ().st_size))
        self.save_manifest ()

    def test_compiler_identity_and_firmware_rebuild (self):
        self.archive (b"#!/bin/sh\necho first\n")
        first = copy.deepcopy (self.manifest)
        self.write ("examples/lessons/001-fixture/001-fixture.ino", "void setup () {}\n")
        self.write ("library.properties", "version=1\n")
        cli = self.write ("bin/arduino-cli", f"#!{sys.executable}\n"
                          "from pathlib import Path\nimport sys\n"
                          "assert '--clean' in sys.argv\n"
                          "with Path('build/compiled').open('a') as out:\n"
                          "    out.write(' '.join(sys.argv[1:]) + '\\n')\n")
        cli.chmod (0o755)
        self.env["PATH"] = str (cli.parent) + os.pathsep + self.env["PATH"]
        self.make ("examples")
        compiled = self.root / "build/compiled"
        self.assertEqual (len (compiled.read_text ().splitlines ()), 1)
        self.make ("examples")
        self.assertEqual (len (compiled.read_text ().splitlines ()), 1)
        self.system["url"] = (self.root / "missing-mirror.tar.bz2").as_uri ()
        self.save_manifest ()
        self.assertNotIn ("curl", self.make ("-n", "-W", "boards/toolchain.json", "toolchain"))
        self.make ("toolchain", "examples")
        self.assertEqual (len (compiled.read_text ().splitlines ()), 1)
        self.archive (b"#!/bin/sh\necho different-bytes\n")
        # The checksum alone changes identity, even with the same version/name.
        planned = self.make ("-n", "examples")
        self.assertIn ("curl", planned)
        self.assertIn ("arduino-cli compile", planned)
        self.make ("examples")
        self.assertEqual (len (compiled.read_text ().splitlines ()), 2)
        self.manifest = first
        self.save_manifest ()
        # The original download no longer exists, but its verified install does.
        self.make ("examples")
        self.assertEqual (len (compiled.read_text ().splitlines ()), 3)
        self.assertEqual (len (list ((self.root / "build/toolchain").glob ("*/.verified"))), 2)
        for field in ("name", "version"):
            self.manifest[field] += "-new"
            self.save_manifest ()
            self.assertIn ("curl", self.make ("-n", "toolchain"))
        # Select identical bytes for another host: it still has its own install.
        self.manifest = copy.deepcopy (first)
        self.manifest["systems"][0]["host"] = "aarch64-linux-gnu"
        self.save_manifest ()
        self.assertIn ("curl", self.make ("-n", "HOST=Linux-aarch64", "toolchain"))

    def test_failed_checksum_never_marks_install_verified (self):
        self.archive (b"#!/bin/sh\necho unchecked\n")
        self.system["checksum"] = "SHA-256:" + "0" * 64
        self.save_manifest ()
        self.assertIn ("SHA-256", self.make ("toolchain", success=False))
        self.assertEqual (list ((self.root / "build/toolchain").glob ("*/.verified")), [])

    def test_incremental_suite_inputs (self):
        for name in ("tests/circuits.py", "tests/build_steps.py", "tests/navigation_ids.py",
                     "docs/_theme/hooks.py", "docs/lessons/001-fixture/circuit.py"):
            self.write (name, "pass\n")
        for name in ("docs/requirements.txt", "docs/_theme/course.yml",
                     "docs/lessons/001-fixture/index.md", "docs/electricity/skills.md"):
            self.write (name, "fixture\n")
        python = self.root / "build/venv/bin/python"
        python.parent.mkdir (parents=True)
        python.symlink_to (sys.executable)
        self.write ("build/venv/.installed", "")
        self.make ("build/circuits.ok", "build/steps.ok")
        self.assertNotIn ("tests/circuits.py", self.make ("-n", "build/circuits.ok"))
        self.assertNotIn ("tests/build_steps.py", self.make ("-n", "build/steps.ok"))
        checks = {
            "build/circuits.ok": ["tests/circuits.py", "docs/_theme/hooks.py",
                                  "docs/lessons/001-fixture/circuit.py"],
            "build/steps.ok": ["tests/build_steps.py", "tests/navigation_ids.py",
                               "docs/_theme/hooks.py", "docs/_theme/course.yml",
                               "docs/lessons/001-fixture/circuit.py",
                               "docs/lessons/001-fixture/index.md", "docs/electricity/skills.md"],
        }
        for target, inputs in checks.items ():
            for source in inputs:
                with self.subTest (target=target, source=source):
                    output = self.make ("-n", "-W", source, target)
                    self.assertIn ("tests/circuits.py" if "circuits" in target
                                   else "tests/build_steps.py", output)
                    if "steps" in target:
                        self.assertIn ("tests/navigation_ids.py", output)


if __name__ == "__main__":
    unittest.main ()
