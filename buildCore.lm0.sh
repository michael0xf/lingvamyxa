#!/usr/bin/env sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_root"

: "${LM_CMAKE:=cmake}"
: "${LM_CC:=gcc}"
: "${LM_AR:=ar}"
: "${LM_RANLIB:=ranlib}"

parser_source="build/lm1/parser.lm1.c"
own_source="build/lm1/own.lm1.c"
trans_source="build/lm1/trans.lm1.c"
make_source="build/lm1/make.lm1.c"
finalize_source="build/lm1/finalize.lm1.c"
build_core_source="build/lm1/buildCore.lm1.c"

for source_file in "$parser_source" "$own_source" "$trans_source" "$make_source" "$finalize_source" "$build_core_source"; do
    if [ ! -f "$source_file" ]; then
        echo "buildCore.lm0.sh: source file not found: $source_file" >&2
        exit 1
    fi
done

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

if ! command -v "$LM_AR" >/dev/null 2>&1; then
    echo "buildCore.lm0.sh: ar not found: $LM_AR" >&2
    echo "Set LM_AR to the ar path and retry." >&2
    exit 1
fi

if ! command -v "$LM_RANLIB" >/dev/null 2>&1; then
    echo "buildCore.lm0.sh: ranlib not found: $LM_RANLIB" >&2
    echo "Set LM_RANLIB to the ranlib path and retry." >&2
    exit 1
fi

"$LM_CMAKE" -E make_directory build/lm0
"$LM_CMAKE" -E make_directory build/obj

"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic -Ilm1 -c "$parser_source" -o build/obj/parser.lm1.o
"$LM_AR" rcs build/lm0/libparser.lm0.a build/obj/parser.lm1.o
"$LM_RANLIB" build/lm0/libparser.lm0.a

"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic -Ilm1 -c "$own_source" -o build/obj/own.lm1.o
"$LM_AR" rcs build/lm0/libown.lm0.a build/obj/own.lm1.o
"$LM_RANLIB" build/lm0/libown.lm0.a

"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic -Ilm1 "$trans_source" build/lm0/libparser.lm0.a build/lm0/libown.lm0.a -o build/lm0/trans.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$make_source" -o build/lm0/make.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$finalize_source" -o build/lm0/finalize.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$build_core_source" -o build/lm0/buildCore.lm0

echo "built build/lm0 bootstrap tools"
