#!/usr/bin/env bash
# UAF kit runner, committed at l2src/tools/uaf_run.sh. It locates q_alloc.c through
# dirname "$0", so the two files MUST stay in the same directory. Read q_alloc.c's header
# for what a clean run does and does not rule out (no overflow detection; its lock can
# hide a race) before concluding anything from "N of N clean".
# Example, from the repo root after a red run_port_message:
#   bash l2src/tools/uaf_run.sh "$PWD" build/port_message/<stamp> 2 /tmp/uaf_out
# Rebuilds the executor selftest from a finished
# run_port_message evidence directory with a poisoning quarantine allocator (q_alloc.c) and the
# repo's own crash report main, then runs it N times and counts clean runs.
#
# A run is clean when: exit code 0, stdout carries "lmx_message_exec ok", and stderr carries no
# "STALE WRITE", no "DOUBLE FREE" and no q_alloc sweep with a nonzero stale count. Anything else is
# kept (stdout, stderr, exit) under OUT/runs/<i>.
#
# Load shapes alternate per run: even runs plain, odd runs with a slow stderr reader (the runner hands
# the process pipes). HOGS=1 also keeps 2x-cores busy loops alive for the whole loop; only use it when
# the machine is yours.
#
# Usage: uaf_run.sh <tree> <evidence dir relative to tree, e.g. build/port_message/20260915_...> <N> <OUT>
set -u
T="${1:?tree}"; E="${2:?evidence dir}"; N="${3:?runs}"; OUT="${4:?out dir}"
K="$(cd "$(dirname "$0")" && pwd)"
cd "$T" || exit 1
[ -d "$E/support" ] && [ -d "$E/hdrs" ] || { echo "no support/ or hdrs/ under $E; stop"; exit 1; }
mkdir -p "$OUT/obj" "$OUT/runs" "$OUT/work" || exit 1
CF="-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -g -O0"
Q="-Dmalloc=q_malloc -Dcalloc=q_calloc -Drealloc=q_realloc -Dfree=q_free"
T0=-DLMX_MSG_EXEC_TEST
fail=0
cc() { local src="$1" obj="$2"; shift 2
  gcc $CF "$@" -I "$E/hdrs" -I lm1/build -c "$src" -o "$obj" > "$obj.log" 2>&1 || { echo "compile failed: $src"; head -n 12 "$obj.log"; fail=1; }; }
objs=""
for src in l2src/lmx_message_host.c l2src/lmx_message_exec.c "$E"/support/*.c; do
  stem=$(basename "$src" .c); obj="$OUT/obj/$stem.o"
  if [ "$stem" = lmx_msg_history_owned ]; then
    cc "$src" "$obj" $T0 -Dmalloc=lmx_msg_history_test_malloc -Dfree=lmx_msg_history_test_free
  else
    cc "$src" "$obj" $T0 $Q
  fi
  objs="$objs $obj"
done
cc l2src/lmx_message_exec_selftest.c "$OUT/obj/selftest.o" $T0 -Dmain=exec_selftest_main $Q
cc l2src/tests/lmx_exec_crash_report.c "$OUT/obj/crash_report_main.o" -DLMX_EXEC_CRASH_REPORT_MAIN
gcc -std=c99 -g -O0 -c "$K/q_alloc.c" -o "$OUT/obj/q_alloc.o" > "$OUT/obj/q_alloc.o.log" 2>&1 || { echo "compile failed: q_alloc.c"; fail=1; }
units=$(ls "$E"/l2units/*.o 2>/dev/null | tr '\n' ' ')
[ $fail = 0 ] || { echo "not linked"; exit 1; }
gcc $CF "$OUT/obj/crash_report_main.o" "$OUT/obj/selftest.o" $objs $units "$OUT/obj/q_alloc.o" -o "$OUT/uaf.exe" > "$OUT/link.log" 2>&1 \
  || { echo "link failed"; head -n 20 "$OUT/link.log"; exit 1; }
echo "built $OUT/uaf.exe from $E ($(echo $objs | wc -w) objects, units: $(echo $units | wc -w))"
hogs=""
if [ "${HOGS:-0}" = 1 ]; then
  for i in $(seq 1 $(( $(nproc) * 2 ))); do ( while :; do :; done ) & hogs="$hogs $!"; done
  echo "hogs: $(echo $hogs | wc -w)"
fi
clean=0; s=$(date +%s)
for i in $(seq 1 "$N"); do
  o="$OUT/runs/$i.out"; e="$OUT/runs/$i.err"
  if [ $((i % 2)) = 1 ]; then
    ( cd "$OUT/work" && timeout 900 "$OUT/uaf.exe" > "$o" 2> "$e" ); x=$?
  else
    ( cd "$OUT/work" && timeout 900 "$OUT/uaf.exe" 2>&1 > "$o" | perl -ne 'select(undef,undef,undef,rand(0.003)); print' > "$e" ); x=${PIPESTATUS[0]}
  fi
  bad=""
  [ "$x" = 0 ] || bad="exit=$x"
  grep -q 'lmx_message_exec ok' "$o" || bad="$bad no-ok"
  grep -qE 'STALE WRITE|DOUBLE FREE' "$e" && bad="$bad stale"
  grep -qE 'q_alloc sweep: [0-9]+ quarantined blocks, [1-9][0-9]* stale' "$e" && bad="$bad sweep"
  if [ -z "$bad" ]; then clean=$((clean + 1)); rm -f "$o" "$e"; else echo "run $i:$bad"; echo "$bad" > "$OUT/runs/$i.verdict"; fi
done
[ -n "$hogs" ] && kill $hogs 2>/dev/null
echo "uaf kit: $clean of $N clean in $(( $(date +%s) - s )) s ($OUT)"
[ "$clean" = "$N" ]
