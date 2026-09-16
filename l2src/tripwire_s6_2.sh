#!/usr/bin/env bash
# Tripwire: delete exactly one line of a tracked source, prove a NAMED assertion
# fires because of it, and restore the tree unconditionally.
#
# Written for S6-2 (l2src/LOCK_REMOVAL_DESIGN.txt) and kept for later stages: an
# assertion that has never been seen to fail is not evidence, and a green that
# cannot be broken says nothing about the mechanism under it.
#
# Every rule below exists because an earlier attempt of mine broke it:
#   * the needle only LOCATES the line (grep -F); the edit is "delete line N".
#     A backslash pattern (m\parent_msg) is silently under-matched by sed through
#     this shell, and that once mutated the wrong text, broke the build for an
#     unrelated reason, and proved nothing about the assertion.
#   * it refuses to run on an already-modified file: a tripwire on top of
#     unrelated edits says nothing about the line it deletes.
#   * the mutation is verified as exactly +0 -1 BEFORE anything is built.
#   * the restore is a trap, so it runs even if the gate hangs, the timeout
#     fires, or the script exits early. An earlier tripwire left the tree
#     mutated because the restore was merely the next statement.
#
# Usage, from the worktree root or anywhere:
#   l2src/tripwire_s6_2.sh <file> <needle> <gate.ps1> <expected-failure> <log>
# Example:
#   l2src/tripwire_s6_2.sh l2src/lmx_message.lm1 'lmx_msg_drain_settled(rt, m)' \
#       l2src/run_lmx.ps1 'no refusal status for a send through a settled capability' /tmp/tw.log
set -u

# The worktree is this script's parent directory, never a hardcoded path: the
# same script has to work in a review worktree and in the shared checkout.
W=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd) || exit 2
cd "$W" || { echo "TRIPWIRE ABORT: cannot enter $W"; exit 2; }

F=${1:?file to mutate}
NEEDLE=${2:?fixed string locating the single line}
GATE=${3:?gate .ps1 to run}
EXPECT=${4:?fixed string expected in the output when the assertion fires}
LOG=${5:?log path}

restore() {
  cd "$W" || return
  git checkout -- "$F"
  local dirty
  dirty=$(git status --porcelain -- "$F" | wc -l | tr -d ' ')
  echo "restore: $F dirty=$dirty (0 means the tree is clean again)"
}
trap restore EXIT

if [ ! -f "$F" ]; then echo "TRIPWIRE ABORT: no such file: $F"; exit 3; fi
if [ "$(git status --porcelain -- "$F" | wc -l | tr -d ' ')" != "0" ]; then
  echo "TRIPWIRE ABORT: $F is already modified; commit or restore it first"
  exit 3
fi

MATCHES=$(grep -n -F -- "$NEEDLE" "$F" | cut -d: -f1)
COUNT=$(printf '%s\n' "$MATCHES" | grep -c .)
if [ "$COUNT" != "1" ]; then
  echo "TRIPWIRE ABORT: needle matched $COUNT lines, need exactly 1"
  printf '%s\n' "$MATCHES"
  exit 4
fi
N=$MATCHES
echo "mutating: deleting line $N of $F"
echo "  was: $(sed -n "${N}p" "$F")"

sed -i "${N}d" "$F"

ADDED=$(git diff --numstat -- "$F" | awk '{print $1}')
DELETED=$(git diff --numstat -- "$F" | awk '{print $2}')
echo "mutation verified: +${ADDED:-0} -${DELETED:-0}"
if [ "${ADDED:-0}" != "0" ] || [ "${DELETED:-0}" != "1" ]; then
  echo "TRIPWIRE ABORT: expected exactly one deleted line and no additions"
  exit 5
fi

timeout 900 powershell -NoProfile -ExecutionPolicy Bypass -File "$GATE" > "$LOG" 2>&1
X=$?
echo "gate exit=$X (nonzero is what a tripwire wants)"

if tr -d '\r' < "$LOG" | grep -q -F -- "$EXPECT"; then
  echo "TRIPWIRE PASS: the named assertion fired -- \"$EXPECT\""
else
  echo "TRIPWIRE FAIL: \"$EXPECT\" is NOT in the output; the assertion cannot fail this way"
  echo "  last lines of the gate output:"
  tr -d '\r' < "$LOG" | grep -vE '^\s*$' | tail -n 6 | sed 's/^/    /'
fi
