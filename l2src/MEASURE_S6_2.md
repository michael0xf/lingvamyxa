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

Acceptance base: `fable/s6-2-acceptance` **4189dea0** = `7b3a8668` merged with `d6/lock-s6-red` 695d54fc.
The probe is never folded into the stage's own history; the green is measured on a separate measuring
branch.

Timeouts below are bounds, not expectations: a step that hits one is a red landing, not a slow one. Under
`timeout`, exit **124** means the bound expired.

---

## Leg 1 - RED, on the base

Measured by 0c at 4189dea0, 2026-09-15: `refs=342 runtime_lists=36`, `S6-2 RED`, exit 1.

```bash
W=/c/Nyasha_Planet/wt0c_s62_red
git -C /c/Nyasha_Planet/lingvamyxa worktree add -q --detach "$W" 4189dea0
cd "$W"
timeout 120 powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_lock_s6_probe.ps1 -Part 2
echo "exit=$?"
```

| expect | value |
| --- | --- |
| count line | `S6 probe: exec_lock_calls=0 exec_lock_decl=0 host_lock=0 exec_fields=0 \| refs=342 runtime_lists=36` |
| verdict line | `S6-2 RED` (and `S6-1 GREEN`, since S6-1 has landed) |
| exit | 1 |
| timeout | 120 s |

If `refs` is not 342 here, the base moved: stop and re-derive the number before measuring any green, since
the falsifier's arithmetic below is stated against it.

## Leg 2 - GREEN, on a measuring merge

The measuring branch exists only to measure. It is never merged into the stage and never pushed to the
stage's line.

```bash
STAGE=origin/d6/lock-s6-2          # the stage tip being accepted
M=/c/Nyasha_Planet/wt0c_s62_measure
git -C /c/Nyasha_Planet/lingvamyxa worktree add -q -b fable/s6-2-measure "$M" "$STAGE"
cd "$M"
git merge --no-ff -q -m "measuring merge: the S6-2 stage tip with 4189dea0 for the probe" origin/fable/s6-2-acceptance
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
check**, three ways, none of which needs the runner:

```bash
test ! -e l2src/run_port_msg_slots.ps1 && echo "runner gone: ok"
test ! -e l2src/lmx_msg_slots.lm1 && echo "module gone: ok"
grep -c "port_slots" l2src/run_gates.ps1        # expect 0: the row at 94 is removed with it
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
| after the mutation | `refs=1`, `S6-2 RED`, exit 1, and `git diff --numstat` showing exactly one added line |
| after the restore | `refs=0 runtime_lists=0`, `S6-2 GREEN`, exit 0, `git status --porcelain` empty |

**Check the diff, not only the count.** A mutation that matches nothing prints a clean "red-to-green" story
while proving nothing; that has happened here before, which is why `--numstat` is in the sequence and why
the mechanism was proven before this file was written.

Proven by 0c on the red tree at 4189dea0, 2026-09-15, since no green tree exists yet, and proven with this
exact line rather than inherited from an earlier draft that used a comment: appending
`git show 7b3a8668:l2src/lmx_message.lm1 | sed -n '1237p'` took `refs` **342 -> 343**,
`git diff --numstat` printed exactly `1  0  l2src/lmx_message.lm1`, and `git checkout --` returned it to
**342** with a clean tree. On a green tree the same step reads **0 -> 1**, `S6-2 GREEN` -> `S6-2 RED`,
exit 0 -> exit 1.

Any of the twenty retain sites in that file serves; 1237 is chosen because it is a plain call in `send`,
not inside a macro or a generated block. If a future base renumbers the file, take the line number from
`git grep -n 'lmx_msg_endp_retain(' 7b3a8668 -- l2src/lmx_message.lm1` rather than trusting 1237.

---

## Falsify this document

- Every command above is plain `bash`; extracting the fenced blocks and running `bash -n` on them exits 0.
- Every step names a marker line and a timeout; a step with neither is a step that cannot fail.
- The red numbers: `run_lock_s6_probe.ps1 -Part 2` in a tree at 4189dea0 prints
  `| refs=342 runtime_lists=36`, `S6-2 RED`, exit 1.
- The markers are `run_gates.ps1`'s own: `git show <tip>:l2src/run_gates.ps1 | sed -n '70,101p'` shows each
  quoted row.
- The green leg's numbers are the only ones not yet measured: they are stated as expectations, and the
  stage tip that would make them measurable does not exist at the time of writing
  (`origin/d6/lock-s6-2` was 6e2ea936, whose diff against 7b3a8668 is eight paths and no deletion yet).
