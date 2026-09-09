#!/usr/bin/env sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_root"

: "${LM_CMAKE:=cmake}"
: "${LM_CC:=gcc}"
: "${LM_THREAD_PROVIDER:=single}"

case "$LM_THREAD_PROVIDER" in
    auto)
        case "$(uname -s 2>/dev/null || printf '%s\n' unknown)" in
            CYGWIN*|MINGW*|MSYS*) thread_provider=win32 ;;
            Darwin|Linux|FreeBSD|NetBSD|OpenBSD|DragonFly|SunOS|AIX) thread_provider=pthread ;;
            *) thread_provider=single ;;
        esac
        ;;
    pthread|win32|single)
        thread_provider=$LM_THREAD_PROVIDER
        ;;
    *)
        echo "buildCore.lm0.sh: unsupported LM_THREAD_PROVIDER: $LM_THREAD_PROVIDER" >&2
        echo "Expected one of: auto, pthread, win32, single." >&2
        exit 1
        ;;
esac

case "$thread_provider" in
    pthread)
        thread_provider_define="-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_PTHREAD"
        thread_native_flag="-pthread"
        ;;
    win32)
        thread_provider_define="-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_WIN32"
        thread_native_flag=
        ;;
    single)
        thread_provider_define="-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE"
        thread_native_flag=
        ;;
esac

l1trans_source="lm1/build/l1trans.lm1.c"
make_source="lm1/build/make.lm1.c"
finalize_source="lm1/build/finalize.lm1.c"
build_core_source="lm1/build/buildCore.lm1.c"

for source_file in "$l1trans_source" "$make_source" "$finalize_source" "$build_core_source"; do
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

posix_feature_define="-D_POSIX_C_SOURCE=200809L"

"$LM_CMAKE" -E make_directory build/lm0
"$LM_CMAKE" -E make_directory build/obj

"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$thread_provider_define" "$posix_feature_define" ${thread_native_flag:+"$thread_native_flag"} -I. -I lm1/build "$l1trans_source" -o build/lm0/l1trans.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$thread_provider_define" "$posix_feature_define" ${thread_native_flag:+"$thread_native_flag"} "$make_source" -o build/lm0/make.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$thread_provider_define" "$posix_feature_define" ${thread_native_flag:+"$thread_native_flag"} "$finalize_source" -o build/lm0/finalize.lm0
"$LM_CC" -std=c99 -Wall -Wextra -Wpedantic "$thread_provider_define" "$posix_feature_define" ${thread_native_flag:+"$thread_native_flag"} -I. "$build_core_source" -o build/lm0/buildCore.lm0

echo "built build/lm0 bootstrap tools"
