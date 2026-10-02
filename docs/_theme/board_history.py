"""Parse published package records and protect them against a base revision.

This uses only the standard library so CI can check history before installing
the site's dependencies. Mirrors may move; published artifacts may not change.
"""

import argparse
import json
from pathlib import Path
import re
import subprocess


def records (text):
    result = {}
    for number, line in enumerate (text.splitlines (), 1):
        fields = line.split ("#", 1)[0].split ()
        if not fields:
            continue
        if len (fields) != 3 or not re.fullmatch (r"[0-9a-f]{64}", fields[1]):
            raise ValueError (f"boards/published.txt:{number}: expected version, SHA-256, compiler")
        version, checksum, compiler = fields
        if version in result:
            raise ValueError (f"boards/published.txt:{number}: duplicate version {version}")
        result[version] = (checksum, compiler)
    return result


def systems (tool):
    result = {}
    for system in tool["systems"]:
        host = system["host"]
        if host in result:
            raise ValueError (f"boards/toolchain.json: duplicate host {host}")
        result[host] = tuple (system[field] for field in ("archiveFileName", "checksum", "size"))
    return result


def compare (old_records, new_records, old_tool, new_tool):
    for version, record in old_records.items ():
        if new_records.get (version) != record:
            raise ValueError (f"ADK Boards {version} changed or was removed; publish a new version")
    before, after = systems (old_tool), systems (new_tool)
    identity = (old_tool["name"], old_tool["version"])
    if identity != (new_tool["name"], new_tool["version"]):
        return
    for host, artifact in before.items ():
        if after.get (host) != artifact:
            raise ValueError (f"{identity[0]}@{identity[1]} for {host} changed or was removed; "
                              "publish a new compiler version")


def main ():
    parser = argparse.ArgumentParser (description=__doc__)
    parser.add_argument ("base", help="Git revision containing the published records")
    args = parser.parse_args ()
    root = Path (__file__).resolve ().parents[2]

    def before (path):
        return subprocess.run (["git", "show", f"{args.base}:{path}"], cwd=root,
                               check=True, text=True, capture_output=True).stdout

    try:
        compare (records (before ("boards/published.txt")),
                 records ((root / "boards/published.txt").read_text (encoding="utf-8")),
                 json.loads (before ("boards/toolchain.json")),
                 json.loads ((root / "boards/toolchain.json").read_text (encoding="utf-8")))
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        parser.exit (1, f"Package history: {error}\n")
    print ("Published platform and compiler artifacts are unchanged")


if __name__ == "__main__":
    main ()
