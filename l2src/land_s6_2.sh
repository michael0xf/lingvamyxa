#!/usr/bin/env bash
# S6-2 landing: refs (endp_retain/endp_release/endp_refs/InterlockedCompareExchange/LmxMsg.refs) and the
# runtime slot list (LmxMsgRuntime slots/n, LmxMsg.alloc_next, the lmx_msg_slots module) are deleted, and
# with them the path arrays (path/path_n/path_cap) and the lmx_msg_path_storage module.
#
# Shape, timeouts and marker-line discipline are d6's land_base3.sh as it ran S6-1, used with his word;
# what is added here is stage-specific and named as such:
#   - scenario36 is a step of its own (the coordinator's ruling: it joins the fixed base, it is not an EXTRA);
#   - the S6-2 acceptance probe runs on a SEPARATE measuring merge and is asserted by its printed line AND
#     its exit, never by the exit alone;
#   - the allowlist is baked in rather than passed, because it is this stage's list (d6's design section at
#     bb082d37 1849-1876 plus the six mixa_manager build lists the coordinator added) and a landing must not
#     be able to run with a looser one by accident. --check prints it and the diff and merges nothing.
#
# The union base is unchanged from land_base3: run_self_build (8 of 8, tagged and logged), run_gates
# -L2MessageRoot, run_port_message plain and -LaneCheck, run_l2trans, run_port_parser, run_mixa,
# run_ingress_harness. The pin is re-installed and hash-checked before every runner, because gate.ps1 and
# run_gen rebuild gen2 over it.
#
# Usage:
#   land_s6_2.sh --check <expected integration tip> <branch> <branch hash>
#   land_s6_2.sh <expected integration tip> <branch> <branch hash> <gate count> <title>
# <gate count> is what run_gates prints as "gates GREEN: N of N". S6-2 DELETES two of run_gates.ps1's rows,
# port_slots and port_path_storage, leaving 28 literal rows plus the -L2MessageRoot one, so this landing
# expects 29, not S6-1's 31 (and not the 30 this comment said before the path_storage deletion). Passing
# a stale count reds the landing on a change the stage is supposed to make; passing it explicitly keeps
# that a decision, not a default.
set -u

CHECK=0
if [ "${1:-}" = "--check" ]; then CHECK=1; shift; fi
TIP="${1:-}"; BRANCH="${2:-}"; BR="${3:-}"
if [ "$CHECK" -eq 0 ]; then NG="${4:-}"; TITLE="${5:-}"; else NG=""; TITLE=""; fi
if [ -z "$TIP" ] || [ -z "$BRANCH" ] || [ -z "$BR" ] || { [ "$CHECK" -eq 0 ] && { [ -z "$NG" ] || [ -z "$TITLE" ]; }; }; then
    echo "usage: land_s6_2.sh [--check] <expected integration tip> <branch> <branch hash> [<gate count> <title>]" >&2
    exit 2
fi

X=/c/Nyasha_Planet/lingvamyxa/build/fable/wti
ACCEPT_REF=origin/fable/s6-2-acceptance
# 96a754e0 since 2026-09-16: the coordinator narrowed the probe's refs needle (the bare
# InterlockedCompareExchange match counted 178 unrelated spin-cell lines and no holder count, so -Part 2
# could not reach 0 on any tip). 4189dea0 was the base before that ruling.
ACCEPT_EXPECT=96a754e0
MEAS=/c/Nyasha_Planet/wt0c_s62_measure
# The log directory is derived, not a constant. land_base3.sh's L= is an absolute path into one session's
# scratchpad -- d6 committed that wart rather than hiding it -- and copying the line would tie this script to
# whoever wrote it. LAND_S6_2_LOGS overrides; the default sits under TMPDIR, so any session can run this file.
LOGS="${LAND_S6_2_LOGS:-${TMPDIR:-/tmp}/land_s6_2_logs}/${BR}"
PS="powershell -NoProfile -ExecutionPolicy Bypass -File"

# Every path the stage may touch, by why it is here (d6's groups). A path outside this stops the landing
# before any merge. (run_entry_turn.ps1 was once listed here as deliberately absent; that reasoning was
# wrong and it is now allowed below, with the reason it was wrong.)
ALLOW='^(l2src/(lmx_message\.h|lmx_message\.lm1|lmx_message\.lm2|lmx_message_exec\.c|lmx_message_exec\.h'
ALLOW="$ALLOW"'|lmx_message_selftest\.lm1|lmx_message_exec_selftest\.c'
ALLOW="$ALLOW"'|lmx_msg_slots\.h\.lm1|lmx_msg_slots\.lm1|lmx_msg_slots\.lm2|LMX_MSG_SLOTS\.txt'
# Added (A), not modified: it does not exist at the base and arrives with b5's mixa branch, which the lead
# merges into the stage. Here on his word, asked for rather than assumed -- an allowlist widened by the
# script's author is how a landing admits a path nobody meant to land.
ALLOW="$ALLOW"'|S6_2_MIXA_BASELINE\.txt'
# Added (A) at 2e2f492c: the lead's mutation harness, kept in the tree for later stages. It is a build
# input to nothing and a gate to nothing -- it exists so a tripwire can be repeated by someone who was not
# there. Here on the coordinator's word, like the baseline document, not on mine.
ALLOW="$ALLOW"'|tripwire_s6_2\.sh'
# Added 2026-09-16 on the lead's confirmation, after the dry run over 51b73274 flagged all three OUTSIDE.
# run_entry_turn.ps1 was excluded here BY ARGUMENT, not by oversight: its support list is Test-Path guarded,
# and I reasoned that it therefore needed no edit. The guard supports "will not break", never "needs no
# edit" -- it stops a build failure when the module file vanishes and does nothing about the stale name
# left in $names. The two .lm2 units are comment-only, rewriting a sentence that cites the module as a
# precedent so it does not read as though lmx_msg_slots still exists.
ALLOW="$ALLOW"'|run_entry_turn\.ps1|lmx_graph_copy_owned\.lm2|lmx_msg_mail_chain\.lm2'
ALLOW="$ALLOW"'|run_port_msg_slots\.ps1|run_msg_slots\.ps1|run_gates\.ps1'
ALLOW="$ALLOW"'|run_port_message\.ps1|run_model_scenario36\.ps1|run_lmx\.ps1|run_graph_abi\.ps1'
ALLOW="$ALLOW"'|run_port_array_owned\.ps1|run_port_array_ref_owned\.ps1|run_port_branch_owned\.ps1'
ALLOW="$ALLOW"'|run_port_chars_owned\.ps1|run_port_graph_copy_owned\.ps1|run_port_merge_owned\.ps1'
ALLOW="$ALLOW"'|run_port_message_graph_copy\.ps1|run_port_msg_blocks\.ps1|run_port_msg_history_owned\.ps1'
ALLOW="$ALLOW"'|run_port_msg_liveness\.ps1|run_port_msg_mail_chain\.ps1|run_port_msg_path_storage\.ps1'
ALLOW="$ALLOW"'|run_port_msg_roots_stale\.ps1|run_port_msg_storage\.ps1|run_port_msg_visit\.ps1'
ALLOW="$ALLOW"'|run_port_owned_ranges\.ps1|run_port_value_owned\.ps1|run_l2trans\.ps1|run_port_parser\.ps1'
ALLOW="$ALLOW"'|tests/(lmx_msg_slots_selftest\.lm1|lmx_model_family_release_17_selftest\.lm1'
ALLOW="$ALLOW"'|lmx_model_orphan_mapped_17_selftest\.lm1|lmx_entry_turn_selftest\.lm1'
ALLOW="$ALLOW"'|unit_msg_adapter\.lm2|unit_msg_cursor\.lm2))'
# Added 2026-09-16 for the path_storage deletion (step 5, 3e1cec49), from the dry run over the landing's
# own diff (integration 7b3a8668...3e1cec49: 67 paths, 13 outside the list above). Three groups:
#   - the module's own files, deleted (D); its port runner run_port_msg_path_storage.ps1 was already allowed;
#   - three runners whose $names support lists still named the module (M), missed by the slots-era list;
#   - the UAF kit under l2src/tools (A, 108b51f0), on the coordinator's word.
ALLOW="$ALLOW"'|l2src/(lmx_msg_path_storage\.h\.lm1|lmx_msg_path_storage\.lm1|lmx_msg_path_storage\.lm2'
ALLOW="$ALLOW"'|LMX_MSG_PATH_STORAGE\.txt|run_msg_path_storage\.ps1'
ALLOW="$ALLOW"'|run_lmx_msg_history_owned\.ps1|run_lmx_msg_roots_stale\.ps1|run_msg_mail_chain\.ps1'
ALLOW="$ALLOW"'|tests/lmx_msg_path_storage_selftest\.lm1|tools/(q_alloc\.c|uaf_run\.sh))'
# Two fixtures NOT in the pre-measured population, named rather than folded in: both call the deleted
# path accessors (lmx_msg_path_n, lmx_msg_path_seg) and step 5 rewrites them onto lmx_msg_get_address.
# They name neither lmx_msg_path_storage nor lmx_msg_path_grow, so no module-name or exported-symbol grep
# finds them -- the accessor API is a reference kind of its own.
ALLOW="$ALLOW"'|l2src/(lmx_message_host_selftest\.c|tests/lmx_msg_send_local_selftest\.lm1)'
# The landing candidate is claude-0c/archive-timeout-3 (= the stage tip + 53ba86bd re-cut), not the bare
# tip, so its own two paths belong here: the c_scanners runner's whole-step bound (M) and the tripwire that
# proves both of that branch's changes against a control (A). Added on the coordinator's word, not the
# author's -- the author of these two paths is also the author of this script.
ALLOW="$ALLOW"'|l2src/(run_candidate_c_scanners\.ps1|tripwire_gate_bounds\.ps1)'
# The R0-round reshape (3e1cec49..4888a7ae: only R0's own lane drains the transport, in R0's round; every
# other lane only pushes), which rewrote these eight fixtures' waits onto R0-round helper calls. Found by the
# dry run over the merged candidate 7b3a8668...3d4eb0be (77 paths, these 8 outside) and added on the
# coordinator's word. lmx_msg_family_handoff_selftest.lm1 is the fixture whose selftest hung for 668 s on a
# tree without the fix; this landing bounds it only chain-wide (run gates under T_GATES), not per gate.
ALLOW="$ALLOW"'|l2src/tests/(lmx_model_checks_19_29_6_selftest\.lm1|lmx_model_family_close_32_selftest\.lm1'
ALLOW="$ALLOW"'|lmx_model_liveness_33_selftest\.lm1|lmx_model_root_ingress_5b_selftest\.lm1'
ALLOW="$ALLOW"'|lmx_model_root_record_5e_selftest\.lm1|lmx_model_scenario36_selftest\.lm1'
ALLOW="$ALLOW"'|lmx_msg_delivery_selftest\.lm1|lmx_msg_family_handoff_selftest\.lm1)'
ALLOW="$ALLOW"'|mixa_manager/(lib_l2_runtime_support\.ps1|run_mixa_app_fmpanel_l2_parity\.ps1'
ALLOW="$ALLOW"'|run_mixa_app_path_l2_parity\.ps1|run_mixa_composite_glyphs_l2_parity\.ps1'
ALLOW="$ALLOW"'|run_mixa_selection_l2_parity\.ps1|run_mixa_tiles_l2_parity\.ps1))$'

say() { printf '%s %s\n' "$(date +%H:%M:%S)" "$*"; }
fail() { say "RED: $*"; exit 1; }

rm -rf "$LOGS"; mkdir -p "$LOGS" || fail "cannot make $LOGS"
cd "$X" || fail "cannot cd $X"
git fetch -q origin || fail "fetch failed"
[ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || fail "origin integration is not $TIP"
[ "$(git rev-parse --short "origin/$BRANCH")" = "$BR" ] || fail "origin/$BRANCH is not $BR"
[ -z "$(git status --short -uno)" ] || fail "wti dirty"
ACC=$(git rev-parse --short "$ACCEPT_REF") || fail "$ACCEPT_REF unknown"
case "$ACC" in "$ACCEPT_EXPECT"*) ;; *) fail "$ACCEPT_REF is $ACC, expected $ACCEPT_EXPECT" ;; esac

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
    say "check only: allowlist matched every path; acceptance base $ACC; nothing merged, nothing run"
    exit 0
fi

# Acceptance, on a measuring merge that is never folded into the stage history (d6's design 1843-1846).
[ -e "$MEAS" ] && fail "$MEAS exists"
git worktree add -q --detach "$MEAS" "$BR" || fail "measuring worktree failed"
probe_rc=1
if ( cd "$MEAS" && git merge --no-ff -q -m "measuring merge: $BR with $ACCEPT_EXPECT for the S6 probe" "$ACCEPT_REF" ); then
    # Run from inside the measuring tree: the probe takes its baseline from $PSScriptRoot, so a relative
    # call from another worktree silently measures that other worktree (d6's warning).
    ( cd "$MEAS" && timeout 120 $PS l2src/run_lock_s6_probe.ps1 -Part 2 ) > "$LOGS/probe.log" 2>&1
    probe_rc=$?
else
    say "measuring merge of $ACCEPT_EXPECT onto $BR does not apply cleanly"
fi
say "probe: exit=$probe_rc :: $(grep -E '^S6 probe:' "$LOGS/probe.log" | tail -n 1 | tr -d '\r')"
grep -qE '^S6-2 GREEN$' "$LOGS/probe.log" || { cd "$X"; git worktree remove --force "$MEAS"; fail "probe did not print S6-2 GREEN (log $LOGS/probe.log)"; }
[ "$probe_rc" = 0 ] || { cd "$X"; git worktree remove --force "$MEAS"; fail "probe printed S6-2 GREEN but exited $probe_rc"; }
grep -qE '\| refs=0 runtime_lists=0$' "$LOGS/probe.log" || { cd "$X"; git worktree remove --force "$MEAS"; fail "probe counts are not refs=0 runtime_lists=0"; }
cd "$X" || fail "cannot cd back to $X"
git worktree remove --force "$MEAS" || say "measuring worktree $MEAS not removed"
say "acceptance: S6-2 GREEN, refs=0 runtime_lists=0, exit 0 on the measuring merge with $ACC"

git checkout -q --detach origin/integration/main-absorbs-core || fail "checkout of the tip failed"
MF="$LOGS/merge_msg.txt"
printf '%s\n\n%s\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n' \
    "Merge $BRANCH ($BR) into integration: $TITLE" \
    "Measured on the merge with the union base: run_self_build (tag and log), gates $NG, run_port_message plain and -LaneCheck, run_l2trans, run_port_parser, run_mixa, run_ingress_harness, run_model_scenario36, run_lmx -Suite Cancel; acceptance S6-2 GREEN on a measuring merge with $ACC." > "$MF"
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
run l2trans $T_L2T $PS l2src/run_l2trans.ps1 || OK=0
has l2trans '^l2trans gen2 ok'
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
# lmx_cancel is a step here, not an EXTRA. The union base is "every runner earlier landings ran, never a
# per-stage subset" (land_base3.sh's header), and S6-1's landing ran it -- so leaving it to an optional
# argument would let this landing measure LESS than the one before it, which is the subset the rule forbids.
run lmx_cancel $T_CANCEL $PS l2src/run_lmx.ps1 -Suite Cancel || OK=0
has lmx_cancel 'selected=Cancel ok'

[ -z "$(git status --short -uno)" ] || { say "tracked files dirty after the runs:"; git status --short -uno | head -n 8; OK=0; }
[ $OK = 1 ] || fail "not green on $M; not pushed (logs $LOGS)"
git fetch -q origin && [ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || fail "origin moved during the runs; $M stays local"
git push origin HEAD:integration/main-absorbs-core 2>&1 | tail -n 1
say "landed: $M"
exit 0
