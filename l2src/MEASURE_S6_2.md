# MEASURE_S6_2: the three legs of the S6-2 acceptance, cold

Written by 0c for the coordinator, 2026-09-15. Every number and marker below was measured here at the
commit named beside it, not transcribed from a ticket; where a value could not be measured yet (the green
leg needs a stage tip that does not exist), it says so instead of guessing.

The probe is `l2src/run_lock_s6_probe.ps1` (d6, on `d6/lock-s6-red` and in the acceptance base). Its
contract, from its own header: it **builds nothing**, scans tracked `l2src` `*.c *.h *.lm1 *.lm2` (tests
included), **comment text counts**, and `-Part 2` exits 0 exactly when `refs` and `runtime_lists` are both
0. It takes its baseline from `$PSScriptRoot/..`, so it must be invoked **by a path inside the tree under
test**: a relative call from another worktree silently measures that other worktree (d6 lost one run to
this).

Acceptance base: `fable/s6-2-acceptance` **96a754e0** (2026-09-16) = the former base `4189dea0` plus the
coordinator's narrowing of the probe's `refs` needle, in two commits: `6b15d8e8` the needle, `96a754e0` the
header stating the narrowing and its reason. `4189dea0` itself is `7b3a8668` merged with `d6/lock-s6-red`
695d54fc. Every number below is re-measured on `96a754e0`; the figures against `4189dea0` are superseded.
The probe is never folded into the stage's own history; the green is measured on a separate measuring
branch.

Timeouts below are bounds, not expectations: a step that hits one is a red landing, not a slow one. Under
`timeout`, exit **124** means the bound expired.

---

## Leg 1 - RED, on the base

Measured by 0c at 96a754e0, 2026-09-16: `refs=163 runtime_lists=36`, `S6-2 RED`, exit 1. (The same tree
read `refs=342` under the old needle; 163 is the holder count alone, without the 178 unrelated spin-cell
lines and with one line matching two needles counted once.)

```bash
W=/c/Nyasha_Planet/wt0c_s62_red
git -C /c/Nyasha_Planet/lingvamyxa worktree add -q --detach "$W" 96a754e0
cd "$W"
timeout 120 powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_lock_s6_probe.ps1 -Part 2
echo "exit=$?"
```

| expect | value |
| --- | --- |
| count line | `S6 probe: exec_lock_calls=0 exec_lock_decl=0 host_lock=0 exec_fields=0 \| refs=163 runtime_lists=36` |
| verdict line | `S6-2 RED` (and `S6-1 GREEN`, since S6-1 has landed) |
| exit | 1 |
| timeout | 120 s |

If `refs` is not 163 here, the base moved: stop and re-derive the number before measuring any green, since
the falsifier's arithmetic below is stated against it.

## Leg 2 - GREEN, on a measuring merge

The measuring branch exists only to measure. It is never merged into the stage and never pushed to the
stage's line.

```bash
STAGE=origin/d6/lock-s6-2          # the stage tip being accepted
M=/c/Nyasha_Planet/wt0c_s62_measure
git -C /c/Nyasha_Planet/lingvamyxa worktree add -q -b fable/s6-2-measure "$M" "$STAGE"
cd "$M"
git merge --no-ff -q -m "measuring merge: the S6-2 stage tip with 96a754e0 for the probe" origin/fable/s6-2-acceptance
timeout 120 powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_lock_s6_probe.ps1 -Part 2
echo "probe exit=$?"
```

| expect | value |
| --- | --- |
| count line | ends with `\| refs=0 runtime_lists=0` |
| verdict line | `S6-2 GREEN` |
| exit | 0 |
| timeout | 120 s |

Assert the printed line **and** the exit, never the exit alone: the probe prints its counts before it
decides, so a 0 exit with a non-zero count line would be a defect in the probe rather than a green stage.

> **The needle was narrowed on 2026-09-16, and why it had to be.** Under the original needle `-Part 2`
> could not reach `refs=0` on **any** tip, including one where the stage's work is complete: `$hRefs`
> counted bare `InterlockedCompareExchange` across a flat `ls-files` of all `l2src` sources. Measured at
> `95de9bf4`, where the holder count is genuinely gone (`lmx_msg_endp_retain(`, `endp_release(`,
> `endp_refs(`, `->refs` and `\refs` all 0, the `refs;` field out of `struct LmxMsg`, both
> `(LONG *)&m->refs` loops deleted), the probe still read **178**: 172 of the exec selftest's own spin
> cells, 5 in `tests/cancel_spin_host.c`, and 1 in `lmx_message_exec.c:352` -- a **comment** recording the
> removal. Comment text counts, so a perfect deletion still left `refs >= 1` while that sentence stood.
> The criterion conflated "the refs holder count is gone" with "no atomic compare-exchange appears anywhere
> in l2src", and only the first is what S6-2 does.
> **The ruling (the coordinator, on his own branch, since the acceptance is his):** the bare
> `InterlockedCompareExchange` needle is dropped; the base's two loops already match `->refs`, so it only
> ever added spin-cell lines and never a holder count. `$hRefs` is now
> `'lmx_msg_endp_retain(', 'lmx_msg_endp_release(', 'lmx_msg_endp_refs(', '->refs', '\refs'` (probe line
> 78), with the header stating the narrowing and its reason, and it was decided **before** the green leg
> was measured -- which is what makes it a criterion change rather than a result fitted to a number.
> **What was not done, and must not be:** editing the selftest's spin cells, or deleting the comment.
> Either turns red to green without changing anything the criterion is about -- a pass that means nothing,
> the mirror of a check that cannot fail. The comment is exactly what a later reader needs.
> Verified here rather than taken from the ruling: probe `-Part 2` on a detached `96a754e0` prints
> `refs=163 runtime_lists=36`, `S6-2 RED`. At the lead's `95de9bf4` merged with this base the coordinator
> measures `refs=0 runtime_lists=32` -- the holder count gone, the registry still to go.

Then, in the same measuring tree, the runners. The pin is rebuilt over by any runner that regenerates gen2,
so re-install it (`l2src/install_pin.ps1 -Tree "$M"`, exit 2 on mismatch) **before each** of these.

| step | command (from `$M`) | expected marker | timeout |
| --- | --- | --- | --- |
| port_message | `run_port_message.ps1` | `parity PASS` | 300 s |
| port_message_lane | `run_port_message.ps1 -LaneCheck` | `parity PASS`, and **no** line matching `LANE (MAP\|WRITE\|WAKE) FAIL` | 300 s |
| lmx_message | `run_lmx.ps1 -Suite Message` | `selected=Message` | 600 s |
| scenario36 | `run_model_scenario36.ps1` | `core tests PASS` | 600 s |

Each is `timeout <s> powershell -NoProfile -ExecutionPolicy Bypass -File l2src/<command>`.

### The gates this stage's section names, run by name

The design's rule (LOCK_REMOVAL_DESIGN.txt, "the gates whose fixtures this section names are run locally by
name, never a proxy suite"). Markers are `run_gates.ps1`'s own, quoted from its rows so a proxy cannot pass
for the gate:

| gate | runner | marker | timeout |
| --- | --- | --- | --- |
| lane_oracle | `run_port_message.ps1 -TranslatorPath <pin> -LaneCheck` | `lmx_message parity PASS`, forbidden line `LANE WRITE FAIL` | 300 s |
| scenario36 | `run_model_scenario36.ps1` | `core tests PASS` | 600 s |
| lmx_message | `run_lmx.ps1 -Suite Message` | `selected=Message` | 600 s |
| entry_turn | `run_entry_turn.ps1` | `entry turn PASS` | 600 s |
| graph_abi | `run_graph_abi.ps1` | `graph ABI runner PASS` | 900 s |
| port_slots | **deleted by this stage** | see below | - |

`port_slots` is the sixth gate the section names, through `tests/lmx_msg_slots_selftest.lm1`, and S6-2
deletes its runner and its module. On the measuring merge there is nothing to run, so its **absence is the
check**, three ways, none of which needs the runner.

The third check is the ROW form, and an earlier version of it could not fail (the coordinator caught it,
2026-09-16): it read `grep -c "port_slots" l2src/run_gates.ps1 # expect 0`, and that grep reads **2** at
2d8f2b6a, because two header prose lines name `port_slots` -- the note that it was deleted at S6-2 (line 32)
and the count-reconciliation note (line 45). A check expecting 0 from a grep that cannot reach 0 is
unfalsifiable, in the one document whose subject is falsifiable measurement. The row form reads 0 at
2d8f2b6a and moves only when the actual row does. Checks 1 and 2 are sound: both files are genuinely gone
there.

```bash
test ! -e l2src/run_port_msg_slots.ps1 && echo "runner gone: ok"
test ! -e l2src/lmx_msg_slots.lm1 && echo "module gone: ok"
grep -c -E "^\s*@\('port_slots'" l2src/run_gates.ps1   # expect 0: the ROW is removed with the module
```

lane_oracle and lmx_message repeat the two runner rows above with different arguments; run each form once,
do not treat one as standing for the other (`-LaneCheck` is what pins the lane, the plain form is not).

Gate-count consequence, stated here because it reds a correct landing if missed: with the `port_slots` row
gone, `run_gates.ps1 -L2MessageRoot` prints `gates GREEN: 30 of 30`, not S6-1's 31.

## Leg 3 - FALSIFIER, one retain put back

A green that cannot be made red proves nothing. This mutation puts one `refs` line back into the working
tree of the measuring merge, expects the probe to go red, then restores it.

What is put back is **an original call this stage deleted**, taken from the base by line number, not a
comment: the probe counts either, but the record should say a call was restored, so that the thing put back
is the thing the stage removed. The line used below is `l2src/lmx_message.lm1:1237` at 7b3a8668,
`if: c.lmx_msg_endp_retain(src) = 0`, one of that file's twenty retain sites.

This is a **text** restoration for a grep: the line is appended without the body its `if:` governed, so the
tree would no longer translate. That is sound here and only here, because the probe builds nothing and
counts lines naming the needle. Do not carry this shortcut into any step that compiles.

```bash
cd "$M"
git show 7b3a8668:l2src/lmx_message.lm1 | sed -n '1237p' >> l2src/lmx_message.lm1
git diff --numstat -- l2src/lmx_message.lm1     # expect "1  0  l2src/lmx_message.lm1"
timeout 120 powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_lock_s6_probe.ps1 -Part 2
echo "exit=$? (expect 1)"
git checkout -- l2src/lmx_message.lm1
timeout 120 powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_lock_s6_probe.ps1 -Part 2
echo "exit=$? (expect 0); dirty=$(git status --porcelain | wc -l) (expect 0)"
```

| stage | expect |
| --- | --- |
| after the mutation | `refs` one higher than the tree's own figure -- **0 -> 1** on a green measuring merge, **163 -> 164** on the red base -- `S6-2 RED`, exit 1, and `git diff --numstat` showing exactly one added line |
| after the restore | `refs=0 runtime_lists=0`, `S6-2 GREEN`, exit 0, `git status --porcelain` empty |

**Check the diff, not only the count.** A mutation that matches nothing prints a clean "red-to-green" story
while proving nothing; that has happened here before, which is why `--numstat` is in the sequence and why
the mechanism was proven before this file was written.

### The reusable shape for any mutation like this

The lead's S6-2 tripwire script is the worked form of the same discipline, and every rule in it exists
because an earlier attempt broke that rule. Borrow the shape, not just the command:

1. **Locate with `grep -F`** -- a pattern carrying a backslash under-matches here, and one beginning `->` is
   taken as an option, so a located-by-regex line is a line you have not actually found.
2. **Edit by line number**, not by pattern substitution -- a substitution that matches nothing edits nothing
   and still exits 0.
3. **Refuse to run on an already-modified file** -- otherwise a leftover mutation is measured as the result.
4. **Verify `+0 -1` (or the exact expected shape) before building** -- the check that the mutation is real
   must happen before the expensive step, not after the red.
5. **Restore in a trap**, and assert `dirty=0` afterwards -- a tripwire that leaves the tree dirty has
   contaminated whatever runs next.

Its two S6-2 uses also show what a tripwire pair should prove: deleting `lmx_message.lm1:1449` (the settled
push in `release_slot`) reds `run_port_message` with "not settled into its parent", while deleting
`lmx_message.lm1:1829` (`drain_settled`, the owner's round) reds `run_lmx -Suite Message` with the planted
case. They fail **differently**, each firing its own named assertion -- which is how you tell two halves of
one mechanism from one mechanism carrying a redundant check. A pair that reds the same assertion has tested
one thing twice.

Proven by 0c on the red tree at 96a754e0, 2026-09-16, since no green tree exists yet, and proven with this
exact line rather than inherited from an earlier draft that used a comment: appending
`git show 7b3a8668:l2src/lmx_message.lm1 | sed -n '1237p'` took `refs` **163 -> 164**,
`git diff --numstat` printed exactly `1  0  l2src/lmx_message.lm1`, and `git checkout --` returned it to
**163** with a clean tree. On a green tree the same step reads **0 -> 1**, `S6-2 GREEN` -> `S6-2 RED`,
exit 0 -> exit 1.
These figures were re-measured on 96a754e0 after the needle was narrowed, not converted arithmetically from
the old base's 342 -> 343: the narrowing changes which lines count, so the old delta is not evidence about
the new criterion even though both are "one more".

Any of the twenty retain sites in that file serves; 1237 is chosen because it is a plain call in `send`,
not inside a macro or a generated block. If a future base renumbers the file, take the line number from
`git grep -n 'lmx_msg_endp_retain(' 7b3a8668 -- l2src/lmx_message.lm1` rather than trusting 1237.

---

## Falsify this document

- Every command above is plain `bash`; extracting the fenced blocks and running `bash -n` on them exits 0.
- Every step names a marker line and a timeout; a step with neither is a step that cannot fail.
- The red numbers: `run_lock_s6_probe.ps1 -Part 2` in a tree at 96a754e0 prints
  `| refs=163 runtime_lists=36`, `S6-2 RED`, exit 1. (Against the superseded base 4189dea0 the same tree
  read `refs=342`, under the needle the coordinator narrowed on 2026-09-16.)
- The markers are `run_gates.ps1`'s own: `git show <tip>:l2src/run_gates.ps1 | sed -n '70,101p'` shows each
  quoted row.
- The green leg's numbers are the only ones not yet measured: they are stated as expectations, and the
  stage tip that would make them measurable does not exist at the time of writing
  (`origin/d6/lock-s6-2` was 6e2ea936, whose diff against 7b3a8668 is eight paths and no deletion yet).
