#!/usr/bin/env sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_root"

: "${LM_CMAKE:=cmake}"
: "${LM_CC:=gcc}"

source_file="build/lm1/buildCore/buildCore.lm1.c"
output_file="build/lm0/buildCore.lm0"

if [ ! -f "$source_file" ]; then
    echo "buildCore.lm0.sh: source file not found: $source_file" >&2
    exit 1
fi

if ! command -v "$LM_CMAKE" >/dev/null 2>&1; then
    echo "buildCore.lm0.sh: cmake not found: $LM_CMAKE" >&2
    echo "Set LM_CMAKE to the cmake path and retry." >&2
    exit 1
fi

if ! command -v "$LM_CC" >/dev/null 2>&1; then
    echo "buildCore.lm0.sh: C compiler not found: $LM_CC" >&2
    echo "Set LM_CC to the gcc/cc path and retry." >&2
    exit 1
fi

"$LM_CMAKE" -E make_directory build/lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$source_file" -o "$output_file"

echo "built $output_file"
