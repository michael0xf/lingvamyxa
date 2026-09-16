#!/usr/bin/env bash
# Landing onto integration in build/fable/wti with the fixed union base (the coordinator's rule, 2026-09-15: a landing
# set is the union of every runner earlier landings ran, never a per-stage subset):
#   run_self_build (8 of 8) with its annotated tag selfbuild/<merge> and SELF-BUILD log commit pushed at once;
#   run_gates -L2MessageRoot ("gates GREEN: N of N"); run_port_message plain and -LaneCheck (parity PASS, no LANE
#   FAIL line); run_l2trans ("l2trans gen2 ok"); run_port_parser (exit 0); mixa_manager/run_mixa.ps1;
#   mixa_manager/run_ingress_harness.ps1 ("mixa ingress fail_keep_runtime path ok").
# The pin is hash-checked (and re-installed from build/pin_<pin>) before every runner. A stage adds its suites with
# EXTRA (a space-separated list of names this script knows: lmx_cancel). The allowlist is a regex over paths.
# Push only if all green and origin is still the tip.
# land_base3 (the coordinator's rule, 2026-09-15, after a 2.5 h hang in c_scanners' git archive): every step runs
# under a timeout; an expired step (exit 124) prints TIMEOUT and turns the landing red. The step timeouts are printed
# in the log header.
# Usage: land_base3.sh <expected integration tip> <branch> <branch hash> <gate count> <allowlist regex> <title> [EXTRA...]
set -u
TIP="$1"; BRANCH="$2"; BR="$3"; NG="$4"; ALLOW="$5"; TITLE="$6"; shift 6; EXTRA="$*"
L="$(cygpath -u 'C:\Users\mtkra\AppData\Local\Temp\claude\C--Nyasha-Planet-lingvamyxa\d5e9022f-3cf9-4ccd-84d4-145db033bfdc\scratchpad\stage3\lock')"
X=/c/Nyasha_Planet/lingvamyxa/build/fable/wti
LOGS="$L/land_${BRANCH//\//_}_$BR"; rm -rf "$LOGS"; mkdir -p "$LOGS"
cd "$X" || exit 1
git fetch -q origin || exit 1
[ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || { echo "origin is not $TIP; stop"; exit 1; }
[ "$(git rev-parse --short "origin/$BRANCH")" = "$BR" ] || { echo "origin/$BRANCH is not $BR; stop"; exit 1; }
[ -z "$(git status --short -uno)" ] || { echo "wti dirty; stop"; exit 1; }
BAD=0; NA=0; ND=0; NM=0
while IFS=$'\t' read -r st a b; do
  printf '%s\n' "$a" | grep -qE "$ALLOW" || { echo "outside the allowlist: $st $a $b"; BAD=1; continue; }
  case "$st" in
    M) NM=$((NM+1)) ;;
    A) NA=$((NA+1)) ;;
    D) ND=$((ND+1)) ;;
    *) echo "unexpected status $st: $a $b"; BAD=1 ;;
  esac
done < <(git diff --no-renames --name-status "origin/integration/main-absorbs-core...$BR")
[ $BAD = 0 ] || { echo "branch outside the allowlist; stop"; exit 1; }
echo "files: M $NM, A $NA, D $ND"
git checkout -q --detach origin/integration/main-absorbs-core || exit 1
MF="$LOGS/merge_msg.txt"
printf '%s\n\n%s\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n' "Merge $BRANCH ($BR) into integration: $TITLE" "Measured on the merge with the union base: run_self_build (tag and log), gates $NG, run_port_message plain and
-LaneCheck, run_l2trans, run_port_parser, run_mixa, run_ingress_harness${EXTRA:+; stage suites: $EXTRA}." > "$MF"
git merge --no-ff -q -F "$MF" "$BR" || { git merge --abort; echo "merge failed; stop"; exit 1; }
M=$(git rev-parse --short HEAD); echo "merged: $M on $TIP"
T_SELF=600; T_GATES=1200; T_PM=300; T_L2T=900; T_PARSER=900; T_MIXA=180; T_INGRESS=180; T_CANCEL=180
echo "step timeouts (s): self_build $T_SELF, gates $T_GATES, port_message $T_PM, port_message_lane $T_PM, l2trans $T_L2T, port_parser $T_PARSER, mixa $T_MIXA, ingress $T_INGRESS, lmx_cancel $T_CANCEL"
OK=1
run() { local name="$1" to="$2"; shift 2; local s x; s=$(date +%s)
  ( cd "$X" && timeout "$to" "$@" ) > "$LOGS/$name.log" 2>&1; x=$?
  [ $x = 124 ] && echo "$name: TIMEOUT after ${to}s"
  echo "$name: exit=$x seconds=$(( $(date +%s) - s )) :: $(tr -d '\r' < "$LOGS/$name.log" | grep -vE '^\s*$' | tail -n 1 | cut -c1-200)"
  [ $x = 0 ] || tr -d '\r' < "$LOGS/$name.log" | grep -inE 'FAIL|missing|failed|error|RED ' | head -n 8 | cut -c1-220 | sed 's/^/   /'
  return $x; }
has() { tr -d '\r' < "$LOGS/$1.log" | grep -qE "$2" || { echo "$1: marker missing: $2"; OK=0; }; }
PIN=$(git show HEAD:l2src/L1_PIN.txt | tr -d '\r\n')
PSRC=/c/Nyasha_Planet/lingvamyxa/build/pin_$PIN/l1trans.exe
H() { sha256sum "$1" | cut -d' ' -f1 | tr a-f A-F; }
pin_install() {
  if [ -f "$PSRC" ] && [ "$(H "$PSRC")" = "$PIN" ]; then
    for dst in "$X/build/l1trans/gen2/l1trans.exe" "$X/build/l1trans/gen3/l1trans.exe"; do
      if [ -f "$dst" ] && [ "$(H "$dst")" = "$PIN" ]; then continue; fi
      mkdir -p "$(dirname "$dst")" && cp "$PSRC" "$dst" && [ "$(H "$dst")" = "$PIN" ] && echo "installed pin at $dst" || { echo "pin install at $dst failed"; OK=0; }
    done
  else echo "distributed pin $PSRC missing or wrong"; OK=0; fi
}
PS="powershell -NoProfile -ExecutionPolicy Bypass -File"
pin_install
run self_build $T_SELF $PS l2src/run_self_build.ps1 -OutDir "$(cygpath -w "$LOGS/self_build_evidence")" || OK=0
if grep -qE '^self-build PASS: fixed point 8 of 8 .*committed generated C 8 of 8' "$LOGS/self_build.log"; then
  # Mikhail 2026-09-15: "каждую успешную самосборку комитьте и пуште"; "помечать как-то каждые успешно самособранные исходники".
  ST="selfbuild/$M"; SBL=$(tr -d '\r' < "$LOGS/self_build.log" | grep -E '^self-build PASS' | head -n 1)
  CFILES="lm1/build/l1src/p0.lm1.h lm1/build/own.lm1.c lm1/build/parser.lm1.c lm1/build/l1trans.lm1.c lm1/build/printTree.lm1.c lm1/build/finalize.lm1.c lm1/build/make.lm1.c lm1/build/buildCore.lm1.c"
  if git ls-remote --exit-code --tags origin "refs/tags/$ST" >/dev/null 2>&1; then echo "tag $ST exists; left"; else
    SBLOBS=""; for f in $CFILES; do SBLOBS="$SBLOBS
  $f $(git rev-parse HEAD:$f)"; done
    printf 'Self-build PASS on %s (merge of %s %s onto %s)\n\ndate: %s\npin: %s\nrun_self_build: %s\nfixed point, committed generated C blob ids:%s\n' \
      "$(git rev-parse HEAD)" "$BRANCH" "$BR" "$TIP" "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$PIN" "$SBL" "$SBLOBS" > "$LOGS/selfbuild_tag.txt"
    git tag -a "$ST" -F "$LOGS/selfbuild_tag.txt" HEAD && echo "tagged $ST"
  fi
  SLOG=l2src/SELF_BUILD_LOG.txt
  if ! grep -q "$(git rev-parse HEAD)" "$SLOG"; then
    SBL1=$(printf '%s' "$SBL" | sed -E 's/; evidence .*//'); SBL2=""
    for f in $CFILES; do SBL2="$SBL2 $(basename $f)=$(git rev-parse HEAD:$f | cut -c1-12)"; done
    printf '%s | %s | %s | %s |%s | %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$(git rev-parse HEAD)" "$PIN" "$SBL1" "$SBL2" "d6 (lingvamyxa-ba), land_base.sh on $BRANCH $BR onto $TIP; tag $ST" >> "$SLOG"
    git add "$SLOG" && git commit -q -m "SELF-BUILD OK $M: fixed point 8 of 8, pin $(printf '%s' "$PIN" | cut -c1-8)" -m "The only change is the appended l2src/SELF_BUILD_LOG.txt line; the measured commit $M carries the tag $ST." -m "Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>" && echo "self-build log line committed $(git rev-parse --short HEAD)"
  fi
  LB="${BRANCH%-landing}-landing-$M"
  git push -q origin "HEAD:refs/heads/$LB" && git push -q origin "refs/tags/$ST" && echo "self-build pushed: $LB $(git rev-parse --short HEAD) and tag $ST" || { echo "self-build push failed"; OK=0; }
else
  echo "self-build not PASS 8 of 8"; OK=0
fi
pin_install
( cd "$X" && timeout "$T_GATES" $PS l2src/run_gates.ps1 -L2MessageRoot > "$LOGS/gates.log" 2>&1 ); gx=$?
[ $gx = 124 ] && echo "gates: TIMEOUT after ${T_GATES}s (newest gate log: $(ls -t "$X"/build/gates/*/*.log 2>/dev/null | head -n 1))"
echo "gates: exit=$gx :: $(grep -E '^gates (GREEN|RED)' "$LOGS/gates.log" | tail -n 1 | tr -d '\r')"
[ $gx = 0 ] && grep -qE "^gates GREEN: $NG of $NG " "$LOGS/gates.log" || OK=0
pin_install
run port_message $T_PM $PS l2src/run_port_message.ps1 || OK=0
has port_message 'parity PASS'
run port_message_lane $T_PM $PS l2src/run_port_message.ps1 -LaneCheck || OK=0
has port_message_lane 'parity PASS'
grep -qE 'LANE (MAP|WRITE|WAKE) FAIL' "$LOGS/port_message_lane.log" && { echo "a LANE FAIL line"; OK=0; }
pin_install
run l2trans $T_L2T $PS l2src/run_l2trans.ps1 || OK=0
has l2trans '^l2trans gen2 ok'
pin_install
run port_parser $T_PARSER $PS l2src/run_port_parser.ps1 || OK=0
pin_install
run mixa $T_MIXA $PS mixa_manager/run_mixa.ps1 || OK=0
run ingress $T_INGRESS $PS mixa_manager/run_ingress_harness.ps1 || OK=0
has ingress 'mixa ingress fail_keep_runtime path ok'
for e in $EXTRA; do
  pin_install
  case "$e" in
    lmx_cancel) run lmx_cancel $T_CANCEL $PS l2src/run_lmx.ps1 -Suite Cancel || OK=0; has lmx_cancel 'selected=Cancel ok' ;;
    *) echo "unknown stage suite $e"; OK=0 ;;
  esac
done
[ -z "$(git status --short -uno)" ] || { echo "tracked files dirty after runs:"; git status --short -uno | head -n 8; OK=0; }
[ $OK = 1 ] || { echo "not green on $M; not pushed"; exit 1; }
git fetch -q origin && [ "$(git rev-parse --short origin/integration/main-absorbs-core)" = "$TIP" ] || { echo "origin moved; $M stays local"; exit 1; }
git push origin HEAD:integration/main-absorbs-core 2>&1 | tail -n 1
echo "landed: $M"
