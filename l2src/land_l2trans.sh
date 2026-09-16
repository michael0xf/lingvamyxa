#!/usr/bin/env bash
# l2trans landing: two translator fixes on fable/l2trans-text-cap --
#   d17eefc3: a define:'d call actual is passed as itself, never boxed into an int temporary;
#   c9c9a405: a statement call's text gets a heap buffer of its own, and a text over the translator's
#             capacity is reported at its statement instead of overflowing one global 1024-byte buffer.
#
# Shape, timeouts, marker-line discipline and the union base are land_s6_2.sh's (it landed S6-2 as
# 07cd6901), used as a pattern. What differs is stage-specific and named as such:
#   - no stage probe and no measuring merge: the acceptance is run_l2trans's own new cases
#     (unit_define_actual, unit_text_capacity, and the two over-capacity negatives _over and _expr).
#     They print nothing on success and throw on failure, and at 5ba818b7 they sit at TOP LEVEL of
#     run_l2trans.ps1 (lines 2414-2438), after its -BuildOnly return (231) and before its
#     "l2trans gen2 ok" banner (5230) -- so that banner cannot print unless they ran and held. The
#     landing also asserts that the two headers those cases generate were written DURING the l2trans
#     step, so a stale header left by an earlier run cannot stand in for them;
#   - l2trans.lm1 feeds generated units, so self_build's "committed generated C 8 of 8" is the check
#     that the fix changed no committed translation. The coordinator's 942-translation comparison is
#     evidence; this step is the proof;
#   - the allowlist is exactly the candidate's own 8 paths (git diff --name-only d3bde8b5 5ba818b7),
#     baked in so the landing cannot run with a looser one. --check prints the diff and merges nothing.
#
# Usage:
#   land_l2trans.sh --check <expected integration tip> <branch> <branch hash>
#   land_l2trans.sh <expected integration tip> <branch> <branch hash> <gate count> <title>
# <gate count> is what run_gates prints as "gates GREEN: N of N": at d3bde8b5, 28 literal rows plus the
# -L2MessageRoot one, so 29. Passed explicitly so it stays a decision, not a default.
set -u

CHECK=0
if [ "${1:-}" = "--check" ]; then CHECK=1; shift; fi
TIP="${1:-}"; BRANCH="${2:-}"; BR="${3:-}"
if [ "$CHECK" -eq 0 ]; then NG="${4:-}"; TITLE="${5:-}"; else NG=""; TITLE=""; fi
if [ -z "$TIP" ] || [ -z "$BRANCH" ] || [ -z "$BR" ] || { [ "$CHECK" -eq 0 ] && { [ -z "$NG" ] || [ -z "$TITLE" ]; }; }; then
    echo "usage: land_l2trans.sh [--check] <expected integration tip> <branch> <branch hash> [<gate count> <title>]" >&2
    exit 2
fi

X=/c/Nyasha_Planet/lingvamyxa/build/fable/wti
# Derived, never a session's scratchpad path. LAND_L2TRANS_LOGS overrides; the default is under TMPDIR.
LOGS="${LAND_L2TRANS_LOGS:-${TMPDIR:-/tmp}/land_l2trans_logs}/${BR}"
PS="powershell -NoProfile -ExecutionPolicy Bypass -File"

# Exactly the candidate's own paths. A path outside this stops the landing before any merge.
ALLOW='^l2src/(l2trans\.lm1|run_l2trans\.ps1'
ALLOW="$ALLOW"'|tests/(unit_define_actual\.h\.lm1|unit_define_actual\.lm2'
ALLOW="$ALLOW"'|unit_text_capacity\.h\.lm1|unit_text_capacity\.lm2'
ALLOW="$ALLOW"'|unit_text_capacity_over\.lm2|unit_text_capacity_expr\.lm2))$'

say() { printf '%s %s\n' "$(date +%H:%M:%S)" "$*"; }
fail() { say "RED: $*"; exit 1; }

rm -rf "$LOGS"; mkdir -p "$LOGS" || fail "cannot make $LOGS"
cd "$X" || fail "cannot cd $X"
git fetch -q origin || fail "fetch failed"
[ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || fail "origin integration is not $TIP"
[ "$(git rev-parse --short "origin/$BRANCH")" = "$BR" ] || fail "origin/$BRANCH is not $BR"
[ -z "$(git status --short -uno)" ] || fail "wti dirty"

# The allowlist, over the same diff the landing merges.
BAD=0; NA=0; ND=0; NM=0
while IFS=$'\t' read -r st a b; do
    [ -n "$st" ] || continue
    printf '%s\n' "$a" | grep -qE "$ALLOW" || { say "outside the allowlist: $st $a $b"; BAD=1; continue; }
    case "$st" in
        M) NM=$((NM + 1)) ;;
        A) NA=$((NA + 1)) ;;
        D) ND=$((ND + 1)) ;;
        *) say "unexpected status $st: $a $b"; BAD=1 ;;
    esac
done < <(git diff --no-renames --name-status "origin/integration/main-absorbs-core...$BR")
say "files: M $NM, A $NA, D $ND"
[ $BAD = 0 ] || fail "branch outside the allowlist; nothing merged"
if [ "$CHECK" -eq 1 ]; then
    say "check only: allowlist matched every path; nothing merged, nothing run"
    exit 0
fi

git checkout -q --detach origin/integration/main-absorbs-core || fail "checkout of the tip failed"
MF="$LOGS/merge_msg.txt"
printf '%s\n\n%s\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n' \
    "Merge $BRANCH ($BR) into integration: $TITLE" \
    "Measured on the merge with the union base: run_self_build (tag and log), gates $NG, run_port_message plain and -LaneCheck, run_l2trans (acceptance: its unit_define_actual and unit_text_capacity cases and the two over-capacity negatives, with both generated headers written during the step), run_port_parser, run_mixa, run_ingress_harness, run_model_scenario36, run_lmx -Suite Cancel." > "$MF"
git merge --no-ff -q -F "$MF" "$BR" || { git merge --abort; fail "merge of $BR onto $TIP does not apply cleanly"; }
M=$(git rev-parse --short HEAD)
say "merged: $M on $TIP"

T_SELF=600; T_GATES=1200; T_PM=300; T_L2T=900; T_PARSER=900; T_MIXA=180; T_INGRESS=180; T_SCEN=600; T_CANCEL=180
say "step timeouts (s): self_build $T_SELF, gates $T_GATES, port_message $T_PM, port_message_lane $T_PM, l2trans $T_L2T, port_parser $T_PARSER, mixa $T_MIXA, ingress $T_INGRESS, scenario36 $T_SCEN, lmx_cancel $T_CANCEL"
OK=1
run() {
    local name="$1" to="$2"; shift 2; local s x
    s=$(date +%s)
    ( cd "$X" && timeout "$to" "$@" ) > "$LOGS/$name.log" 2>&1
    x=$?
    [ $x = 124 ] && say "$name: TIMEOUT after ${to}s"
    say "$name: exit=$x seconds=$(( $(date +%s) - s )) :: $(tr -d '\r' < "$LOGS/$name.log" | grep -vE '^[[:space:]]*$' | tail -n 1 | cut -c1-200)"
    [ $x = 0 ] || tr -d '\r' < "$LOGS/$name.log" | grep -inE 'FAIL|missing|failed|error|RED ' | head -n 8 | cut -c1-220 | sed 's/^/   /'
    return $x
}
has() { tr -d '\r' < "$LOGS/$1.log" | grep -qE "$2" || { say "$1: marker missing: $2"; OK=0; }; }

PIN=$(git show HEAD:l2src/L1_PIN.txt | tr -d '\r\n')
PSRC=/c/Nyasha_Planet/lingvamyxa/build/pin_$PIN/l1trans.exe
H() { sha256sum "$1" | cut -d' ' -f1 | tr a-f A-F; }
pin_install() {
    if [ -f "$PSRC" ] && [ "$(H "$PSRC")" = "$PIN" ]; then
        for dst in "$X/build/l1trans/gen2/l1trans.exe" "$X/build/l1trans/gen3/l1trans.exe"; do
            if [ -f "$dst" ] && [ "$(H "$dst")" = "$PIN" ]; then continue; fi
            mkdir -p "$(dirname "$dst")" && cp "$PSRC" "$dst" && [ "$(H "$dst")" = "$PIN" ] && say "installed pin at $dst" || { say "pin install at $dst failed"; OK=0; }
        done
    else say "distributed pin $PSRC missing or wrong"; OK=0; fi
}

pin_install
run self_build $T_SELF $PS l2src/run_self_build.ps1 -OutDir "$(cygpath -w "$LOGS/self_build_evidence")" || OK=0
has self_build '^self-build PASS: fixed point 8 of 8 .*committed generated C 8 of 8'
pin_install
run gates $T_GATES $PS l2src/run_gates.ps1 -L2MessageRoot || OK=0
has gates "^gates GREEN: $NG of $NG "
pin_install
run port_message $T_PM $PS l2src/run_port_message.ps1 || OK=0
has port_message 'parity PASS'
run port_message_lane $T_PM $PS l2src/run_port_message.ps1 -LaneCheck || OK=0
has port_message_lane 'parity PASS'
grep -qE 'LANE (MAP|WRITE|WAKE) FAIL' "$LOGS/port_message_lane.log" && { say "a LANE FAIL line"; OK=0; }
pin_install
L2T_START=$(date +%s)
run l2trans $T_L2T $PS l2src/run_l2trans.ps1 || OK=0
has l2trans '^l2trans gen2 ok'
# The acceptance, shown to have RUN: the headers the new cases generate, written during this step.
for h in lm1/build/l2src/tests/unit_define_actual.lm1.h lm1/build/l2src/tests/unit_text_capacity.lm1.h; do
    if [ -f "$X/$h" ] && [ "$(stat -c %Y "$X/$h")" -ge "$L2T_START" ]; then
        say "l2trans acceptance: $h written during the step"
    else
        say "l2trans acceptance: $h NOT written during the step"; OK=0
    fi
done
pin_install
run port_parser $T_PARSER $PS l2src/run_port_parser.ps1 || OK=0
pin_install
run mixa $T_MIXA $PS mixa_manager/run_mixa.ps1 || OK=0
run ingress $T_INGRESS $PS mixa_manager/run_ingress_harness.ps1 || OK=0
has ingress 'mixa ingress fail_keep_runtime path ok'
pin_install
run scenario36 $T_SCEN $PS l2src/run_model_scenario36.ps1 || OK=0
has scenario36 '^core tests PASS'
pin_install
run lmx_cancel $T_CANCEL $PS l2src/run_lmx.ps1 -Suite Cancel || OK=0
has lmx_cancel 'selected=Cancel ok'

[ -z "$(git status --short -uno)" ] || { say "tracked files dirty after the runs:"; git status --short -uno | head -n 8; OK=0; }
[ $OK = 1 ] || fail "not green on $M; not pushed (logs $LOGS)"
git fetch -q origin && [ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || fail "origin moved during the runs; $M stays local"
git push origin HEAD:integration/main-absorbs-core 2>&1 | tail -n 1
say "landed: $M"
exit 0
