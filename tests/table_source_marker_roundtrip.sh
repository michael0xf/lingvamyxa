#!/usr/bin/env sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <table.lm0>" >&2
    exit 2
fi

case "$1" in
    /*) table_tool=$1 ;;
    *) table_tool=$(pwd)/$1 ;;
esac

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
fixture="$project_root/tests/fixtures/serializer_source_marker_roundtrip.lmx"
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/lm-table-source-marker-roundtrip.XXXXXX")
trap 'rm -rf -- "$test_dir"' EXIT HUP INT TERM

first="$test_dir/first.lmx"
second="$test_dir/second.lmx"

"$table_tool" "$fixture" >"$first"
"$table_tool" "$first" >"$second"
cmp "$first" "$second"

awk '
    /^table:$/ {
        source_marker = 0
        next
    }
    /^    source$/ {
        source_marker += 1
        source_marker_count += 1
        next
    }
    /^    name: marked$/ {
        if (source_marker != 1) exit 1
        marked_count += 1
        next
    }
    /^    name: unmarked$/ {
        if (source_marker != 0) exit 1
        unmarked_count += 1
        next
    }
    END {
        if (source_marker_count != 1 || marked_count != 1 || unmarked_count != 1) exit 1
    }
' "$first"

for invalid_fixture in \
    "$project_root/tests/fixtures/parser_registry_source_table_marker_order_invalid.lm2" \
    "$project_root/tests/fixtures/parser_registry_source_table_marker_duplicate_invalid.lm2"
do
    if "$table_tool" "$invalid_fixture" >/dev/null 2>&1; then
        echo "table source-marker fixture unexpectedly formatted: $invalid_fixture" >&2
        exit 1
    fi
done
