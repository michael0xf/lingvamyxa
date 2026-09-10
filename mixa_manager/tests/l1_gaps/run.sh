#!/bin/sh
# Regenerates transcript.txt. Run from the repository root.
# Uses the stable STG translator only - never build/l1trans in the root tree.
set -e
D=mixa_manager/tests/l1_gaps
T=stg/l1_baseline/build/l1trans/gen2/l1trans.exe
mkdir -p build/l1gaps
{
  echo "L1 gaps probe transcript"
  echo "generated $(date '+%Y-%m-%d %H:%M:%S %z')"
  echo "translator $T"
  echo "sha256     $(sha256sum $T | cut -d' ' -f1)"
  echo
  echo "== 1. BRIDGE: const: @(VTRef) reaches const VT *const * =="
  $T $D/bridge_ok.lm1 build/l1gaps/bridge_ok.c 2>&1 | sed 's/^/   stderr: /'
  echo "   generated:"; sed -n '4,6p' build/l1gaps/bridge_ok.c | sed 's/^/     /'
  echo "   compat vs header prototype, -Werror:"
  gcc -c -std=c99 -Wall -Wextra -Wpedantic -Werror -I. -o build/l1gaps/bridge_ok.o build/l1gaps/bridge_ok.c 2>&1 | sed 's/^/     /'
  echo "     exit=$? (0 = compatible with const VT *const *)"
  echo
  echo "== 2. NEGATIVE WRITES (must be rejected) =="
  for f in neg_slot neg_pointee; do
    printf "   %-12s " $f
    gcc -c -std=c99 -Wall -Wextra -I. -o build/l1gaps/$f.o $D/$f.c 2>&1 | grep -m1 "error:" | sed 's/.*error: /REJECTED: /'
  done
  printf "   %-12s " pos_read
  gcc -c -std=c99 -Wall -Wextra -Wpedantic -Werror -I. -o build/l1gaps/pos_read.o $D/pos_read.c 2>&1 | head -1
  echo "compiles clean (exit=$?)"
} > $D/transcript.txt 2>&1
cat $D/transcript.txt
