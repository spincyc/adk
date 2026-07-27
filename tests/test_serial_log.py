#!/usr/bin/env python3

import pathlib
import stat
import subprocess
import tempfile
import textwrap
import unittest


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[1]


class SerialLogTest(unittest.TestCase):
    def run_serial_log(self, exit_status):
        with tempfile.TemporaryDirectory() as temporary_directory:
            temporary_path = pathlib.Path(temporary_directory)
            fake_cli = temporary_path / "fake-arduino-cli"
            build_directory = temporary_path / "build"
            serial_log = temporary_path / "monitor.log"

            fake_cli.write_text(
                textwrap.dedent(
                    """\
                    #!/bin/sh
                    printf '%s\\n' 'captured monitor output'
                    exit {exit_status}
                    """
                ).format(exit_status=exit_status),
                encoding="utf-8",
            )
            fake_cli.chmod(fake_cli.stat().st_mode | stat.S_IXUSR)

            result = subprocess.run(
                [
                    "make",
                    "--no-print-directory",
                    "serial-log",
                    f"ARDUINO_CLI={fake_cli}",
                    f"BUILD_DIR={build_directory}",
                    f"SERIAL_LOG={serial_log}",
                    "PORT=/dev/fake-monitor",
                ],
                cwd=REPOSITORY_ROOT,
                check=False,
                capture_output=True,
                text=True,
            )

            self.assertEqual(
                serial_log.read_text(encoding="utf-8"),
                "captured monitor output\n",
            )
            self.assertEqual(
                list(temporary_path.glob("monitor.log.monitor-status.*")), []
            )
            return result.returncode

    def test_monitor_failure_is_logged_and_propagated(self):
        self.assertNotEqual(self.run_serial_log(23), 0)

    def test_successful_monitor_and_log_return_success(self):
        self.assertEqual(self.run_serial_log(0), 0)


if __name__ == "__main__":
    unittest.main()
