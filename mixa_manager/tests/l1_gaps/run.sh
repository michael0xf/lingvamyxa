#!/bin/sh
# Regenerates transcript.txt and RETURNS FAILURE if any expectation is unmet.
# Run from the repository root. Uses the stable STG translator only.
#
# Every status is captured DIRECTLY from the command, then its saved output is
# formatted. An earlier version piped the compiler into sed and read $? from the
# tail of the pipeline, so a negative case that unexpectedly COMPILED still
# printed "REJECTED" and the script still passed - a test suite that could not
# fail. Codex caught it; the shape is the lesson, not the instance.
#
# Optional: FORCE_FAIL=neg_slot (or bridge, neg_pointee, pos_read) inverts one
# expectation, to demonstrate that the runner actually returns failure.
D=mixa_manager/tests/l1_gaps
T=stg/l1_baseline/build/l1trans/gen2/l1trans.exe
O=build/l1gaps
CC="gcc -c -std=c99 -Wall -Wextra -Wpedantic -I."
mkdir -p $O
rc=0
say() { printf '%s\n' "$*" >> $O/t.txt; }

expect() { # name expected(ok|fail) actual_status
    if [ "$2" = ok ] && [ "$3" -ne 0 ]; then
        say "   FAIL: $1 was expected to succeed, status $3"; rc=1
    elif [ "$2" = fail ] && [ "$3" -eq 0 ]; then
        say "   FAIL: $1 was expected to be REJECTED and it compiled"; rc=1
    else
        say "   ok:   $1 ($2, status $3)"
    fi
}
inv() { [ "$FORCE_FAIL" = "$1" ] && { [ "$2" = ok ] && echo fail || echo ok; } || echo "$2"; }

: > $O/t.txt
say "L1 gaps probe transcript"
say "generated  $(date '+%Y-%m-%d %H:%M:%S %z')"
say "translator $T"
say "sha256     $(sha256sum $T | cut -d' ' -f1)"
[ -n "$FORCE_FAIL" ] && say "FORCE_FAIL  $FORCE_FAIL (expectation deliberately inverted)"
say ""

say "== 1. BRIDGE: const: @(MixaBackendTableRef)-shaped return =="
$T $D/bridge_ok.lm1 $O/bridge_ok.c > $O/bridge.trans 2>&1; st=$?
say "   translator status $st"
sed 's/^/     /' $O/bridge.trans >> $O/t.txt
expect "translate bridge_ok.lm1" "$(inv bridge ok)" $st
say "   generated:"
sed -n '4,5p' $O/bridge_ok.c 2>/dev/null | sed 's/^/     /' >> $O/t.txt
$CC -Werror -o $O/bridge_ok.o $O/bridge_ok.c > $O/bridge.cc 2>&1; st=$?
sed 's/^/     /' $O/bridge.cc >> $O/t.txt
expect "bridge matches const VT *const * under -Werror" "$(inv bridge ok)" $st
say ""

say "== 2. NEGATIVE WRITES - must NOT compile =="
for f in neg_slot neg_pointee; do
    $CC -o $O/$f.o $D/$f.c > $O/$f.cc 2>&1; st=$?
    grep -m1 "error:" $O/$f.cc | sed 's/.*error: /     /' >> $O/t.txt
    expect "$f" "$(inv $f fail)" $st
done
say ""

say "== 3. POSITIVE READ - must compile clean =="
$CC -Werror -o $O/pos_read.o $D/pos_read.c > $O/pos.cc 2>&1; st=$?
sed 's/^/     /' $O/pos.cc >> $O/t.txt
expect "pos_read" "$(inv pos_read ok)" $st

say ""
if [ $rc -eq 0 ]; then say "RESULT: all expectations met"; else say "RESULT: FAILURES ABOVE"; fi
cp $O/t.txt $D/transcript.txt
cat $D/transcript.txt
exit $rc
