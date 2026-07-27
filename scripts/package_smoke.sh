#!/bin/sh

set -eu

arduinoCli=${ARDUINO_CLI:-arduino-cli}
boardFqbn=${BOARD_FQBN:-arduino:avr:mega}
packageRef=${PACKAGE_REF:-HEAD}
temporaryRoot=$(mktemp -d)
archivePath="${temporaryRoot}/Adk.tar"
libraryRoot="${temporaryRoot}/libraries/Adk"
buildRoot="${temporaryRoot}/build"

cleanup()
{
    rm -rf -- "${temporaryRoot}"
}

trap cleanup 0 HUP INT TERM

git archive \
    --format=tar \
    --prefix=Adk/ \
    --output="${archivePath}" \
    "${packageRef}"
mkdir -p "${temporaryRoot}/libraries" "${buildRoot}"
tar -xf "${archivePath}" -C "${temporaryRoot}/libraries"

for requiredPath in \
    library.properties \
    LICENSE \
    README.md \
    src/Adk.h \
    examples
do
    test -e "${libraryRoot}/${requiredPath}" || \
        {
            echo "Package is missing ${requiredPath}." >&2
            exit 1
        }
done

for excludedPath in build doc docs legacy mk research scripts site tests
do
    test ! -e "${libraryRoot}/${excludedPath}" || \
        {
            echo "Package unexpectedly contains ${excludedPath}." >&2
            exit 1
        }
done

unsafePath=$(find "${libraryRoot}" \( -type l -o \( -type f -perm /111 \) \) -print -quit)
if test -n "${unsafePath}"
then
    echo "Package contains a symlink or executable file." >&2
    exit 1
fi

for sketchPath in "${libraryRoot}"/examples/*/*.ino
do
    test -f "${sketchPath}" || \
        {
            echo "Package contains no canonical examples." >&2
            exit 1
        }

    sketchDirectory=${sketchPath%/*}
    sketchName=${sketchDirectory##*/}

    "${arduinoCli}" compile \
        --fqbn "${boardFqbn}" \
        --libraries "${temporaryRoot}/libraries" \
        --build-path "${buildRoot}/${sketchName}" \
        "${sketchDirectory}"
done

echo "Installed package smoke test passed for ${packageRef}."
