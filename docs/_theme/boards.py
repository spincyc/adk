"""The Arduino IDE board package, published beside the site.

ADK needs C++23, and Arduino AVR Boards' compiler stops at C++17, so the
Boards Manager offers "ADK Boards": one board, the Mega 2560, built with
avr-gcc 16. When the site builds, this writes into its root:

    package_adk_index.json      the index whose URL learners add to the IDE
    adk-avr-VERSION.tar.bz2     boards/avr, packed the same way every time

The platform's name and boards come from boards/avr, its version from
library.properties, and its compiler from boards/toolchain.json. Its core,
variant and avrdude are Arduino AVR Boards' own, used by reference.
"""

import hashlib
import io
import json
import os
import re
import tarfile

from mkdocs.exceptions import PluginError

ROOT     = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))
PLATFORM = os.path.join (ROOT, "boards", "avr")
TOOL     = os.path.join (ROOT, "boards", "toolchain.json")
INDEX    = "package_adk_index.json"

# Uploads run avrdude through Arduino AVR Boards' recipes; this is the
# version its 1.8.8 release installs.
AVRDUDE = {"packager": "arduino", "name": "avrdude", "version": "8.0.0-arduino1"}

# Every file in the archive gets this time, so the same boards/avr packs to
# the same bytes and the same checksum. 1980-01-01 is the usual choice.
MTIME = 315532800


def on_post_build (config):
    library  = properties (os.path.join (ROOT, "library.properties"))
    platform = properties (os.path.join (PLATFORM, "platform.txt"))
    boards   = properties (os.path.join (PLATFORM, "boards.txt"))
    tool     = json.load (open (TOOL, encoding="utf-8"))
    check (library, platform, tool)

    site    = config["site_url"]
    archive = f"adk-avr-{library['version']}.tar.bz2"
    packed  = pack (PLATFORM, archive.removesuffix (".tar.bz2"))
    with open (os.path.join (config["site_dir"], archive), "wb") as out:
        out.write (packed)

    index = {"packages": [{
        "name":       "adk",
        "maintainer": library["author"],
        "websiteURL": site,
        "help":       {"online": site + "start/"},
        "platforms":  [{
            "name":            platform["name"],
            "architecture":    "avr",
            "version":         library["version"],
            "category":        "Contributed",
            "help":            {"online": site + "start/"},
            "url":             site + archive,
            "archiveFileName": archive,
            "checksum":        "SHA-256:" + hashlib.sha256 (packed).hexdigest (),
            "size":            str (len (packed)),
            "boards":          [{"name": value} for key, value in boards.items ()
                                if re.fullmatch (r"\w+\.name", key)],
            "toolsDependencies": [
                {"packager": "adk", "name": tool["name"], "version": tool["version"]},
                AVRDUDE,
            ],
        }],
        "tools": [tool],
    }]}
    with open (os.path.join (config["site_dir"], INDEX), "w", encoding="utf-8") as out:
        json.dump (index, out, indent=2)
        out.write ("\n")


def properties (path):
    pairs = {}
    for line in open (path, encoding="utf-8"):
        line = line.strip ()
        if line and not line.startswith ("#") and "=" in line:
            key, value = line.split ("=", 1)
            pairs[key.strip ()] = value.strip ()
    return pairs


# The platform is released with the library, and must ask for the tool the
# index installs; a mismatch would only show on a learner's computer.
def check (library, platform, tool):
    if platform["version"] != library["version"]:
        raise PluginError (f"boards/avr/platform.txt is version {platform['version']}, "
                           f"but library.properties is {library['version']}")
    if f"{{runtime.tools.{tool['name']}.path}}" not in platform["compiler.path"]:
        raise PluginError (f"boards/avr/platform.txt: compiler.path does not use the "
                           f"{tool['name']} tool from boards/toolchain.json")


# A tar.bz2 with one top-level folder, as Boards Manager expects, and
# nothing in it that depends on who packed it, or when.
def pack (folder, top):
    paths = sorted (os.path.join (path, name) for path, directories, files in os.walk (folder)
                    for name in directories + files)
    buffer = io.BytesIO ()
    with tarfile.open (fileobj=buffer, mode="w:bz2", format=tarfile.USTAR_FORMAT) as tar:
        tar.addfile (entry (top, None))
        for path in paths:
            name = os.path.join (top, os.path.relpath (path, folder))
            if os.path.isdir (path):
                tar.addfile (entry (name, None))
                continue
            data = open (path, "rb").read ()
            tar.addfile (entry (name, data), io.BytesIO (data))
    return buffer.getvalue ()


def entry (name, data):
    info = tarfile.TarInfo (name)
    info.mtime = MTIME
    info.uid   = info.gid   = 0
    info.uname = info.gname = ""
    if data is None:
        info.type = tarfile.DIRTYPE
        info.mode = 0o755
    else:
        info.size = len (data)
        info.mode = 0o644
    return info
