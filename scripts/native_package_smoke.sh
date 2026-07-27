#!/bin/sh

set -eu

cxx=${CXX:-c++}
ar_tool=${AR:-ar}
native_package=${NATIVE_PACKAGE:-${NATIVE_PACKAGE_ARCHIVE:-build/package/adk-native.tar.gz}}
temporary_root=$(mktemp -d)
extract_root="${temporary_root}/extract"
consumer_root="${temporary_root}/consumer"
artifact_root="${extract_root}/adk-native"
object_root="${consumer_root}/objects"
library_path="${consumer_root}/libadk.a"

cleanup()
{
  rm -rf -- "${temporary_root}"
}

trap cleanup 0 HUP INT TERM

test -f "${native_package}" || {
  echo "Native package does not exist: ${native_package}" >&2
  exit 1
}

mkdir -p "${extract_root}" "${object_root}"
entry_list="${temporary_root}/archive-entries.txt"
listing_list="${temporary_root}/archive-listing.txt"
tar -tzf "${native_package}" > "${entry_list}"
while IFS= read -r archive_entry
do
  case "${archive_entry}" in
    adk-native|adk-native/*)
      ;;
    *)
      echo "Native package entry escapes its fixed root: ${archive_entry}" >&2
      exit 1
      ;;
  esac

  case "/${archive_entry}/" in
    *"/../"*)
      echo "Native package contains an unsafe path: ${archive_entry}" >&2
      exit 1
      ;;
  esac
done < "${entry_list}"

tar -tvzf "${native_package}" > "${listing_list}"
while IFS= read -r archive_listing
do
  entry_type=$(printf '%s' "${archive_listing}" | cut -c1)
  case "${entry_type}" in
    d|-)
      ;;
    *)
      echo "Native package contains a non-file entry." >&2
      exit 1
      ;;
  esac
done < "${listing_list}"

tar -xzf "${native_package}" -C "${extract_root}"

for required_path in \
  include/adk/Adk.h \
  include/adk/inert_channel_assessor.h \
  manifest/sources.txt \
  manifest/README.md \
  LICENSE \
  README.md
do
  test -f "${artifact_root}/${required_path}" || {
    echo "Native package is missing ${required_path}." >&2
    exit 1
  }
done

test -z "$(find "${extract_root}" -mindepth 1 -maxdepth 1 ! -name adk-native -print)" || {
  echo "Native package contains an unexpected top-level entry." >&2
  exit 1
}

source_count=0
while IFS= read -r source_path
do
  case "${source_path}" in
    src/*.cpp)
      ;;
    *)
      echo "Invalid native source manifest entry: ${source_path}" >&2
      exit 1
      ;;
  esac

  case "${source_path}" in
    *"/../"*|../*|*/..)
      echo "Unsafe native source manifest entry: ${source_path}" >&2
      exit 1
      ;;
  esac

  source_file="${artifact_root}/${source_path}"
  test -f "${source_file}" || {
    echo "Native source is missing: ${source_path}" >&2
    exit 1
  }

  source_name=${source_path#src/}
  object_name=$(printf '%s\n' "${source_name}" | sed 's|/|__|g; s|\.cpp$|.o|')
  "${cxx}" \
    -std=c++17 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Wconversion \
    -Werror \
    -I"${artifact_root}/include/adk" \
    -c "${source_file}" \
    -o "${object_root}/${object_name}"
  source_count=$((source_count + 1))
done < "${artifact_root}/manifest/sources.txt"

test "${source_count}" -gt 0 || {
  echo "Native source manifest is empty." >&2
  exit 1
}

"${ar_tool}" rcsD "${library_path}" "${object_root}"/*.o

cat > "${consumer_root}/consumer.cpp" <<'EOF'
#include <adk/inert_channel_assessor.h>

int main ()
{
    adk::InertChannelAssessor assessor (adk::Duration (10));
    if (!assessor.initialize ().ok ())
    {
        return 1;
    }

    const adk::InertChannelObservation observation = {
        3,
        adk::InertObservation::Closed,
        adk::InertObservation::Closed,
        adk::TimePoint (7)};

    if (!assessor.update (adk::TimePoint (7), &observation, 1).ok ())
    {
        return 2;
    }

    const adk::Result<adk::InertChannelAssessment> assessment =
        assessor.assessment (3, adk::TimePoint (7));
    if (!assessment.ok () ||
        assessment.value ().state != adk::InertChannelState::Closed)
    {
        return 3;
    }

    assessor.shutdown ();
    return 0;
}
EOF

"${cxx}" \
  -std=c++17 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -Wconversion \
  -Werror \
  -I"${artifact_root}/include" \
  "${consumer_root}/consumer.cpp" \
  "${library_path}" \
  -o "${consumer_root}/consumer"

"${consumer_root}/consumer"

echo "Native package consumer smoke test passed for ${native_package}."
