# Review of the core and mixa_manager lanes — 2026-09-14, ~01:30 local

Written by the review chat (Claude Fable 5.1, desktop session titled "Общение
между чатами", peer name `lingvamyxa-e2` on 2026-09-14) at Mikhail's request:
"check that the two working chats follow the project and language
architecture, especially `L2_CORE_AND_MESSAGE_MODEL_20260912.md`, and advise on
organization and speed." Everything marked **measured** was run or read in this
session between 00:25 and 01:30; nothing below is recalled from correspondence.

The two chats: the lead (Opus 5, high; peer `lingvamyxa-d6`; worktree
`build/fable/integration`, branch `integration/main-absorbs-core`) and the
helper (Sonnet 5, medium; peer `lingvamyxa-5e`; the main checkout, branch
`main`, owner of `mixa_manager`). Since 2026-09-14 they talk by direct session
messages; the `work_chat` file channels and watchers are retired.

Read: the model document (all five parts), `CORE_LEAD_INSTRUCTION_20260913_en.md`,
`CORE_TEAM_PLAN_20260912.md` (archive), `INTEGRATION_GATE_STATUS_20260913.md`
(sections 1–15), `Lingvamyxa_spec.txt` sections 1, 9.0–9.1.4, 20.2.1–20.2.2, 21,
the `mixa_manager` documents (README, CODING_RULES, STATUS, PORT_OF_CLEARSHELL §1,
PROCESS_SEAM), three `*_l2_port.txt` notes, the last eight tickets and replies,
`FABLE_GRAPH_ABI.txt` (tail), and the `.lm1`/`.lm2` pairs themselves.

---

## 0. Verdict

1. **Core lane: conforms.** The lead's notes and evidence follow the model
   where it is specific: one copy map with aliases and cycles (§17–18),
   field paths resolved at translation with no runtime name table (§6, spec
   21.1), methods reaching the unit through `node\node` and never deriving
   the Message from `node` (§9, spec 21.2–21.3), own fields as dirty-only
   caches (§11–12, spec 21.5–21.6), refusals labelled as gaps and lifted.
   No deviation found.
2. **Application lane: the 52 ".lm2 ports" are the L1 bodies re-labelled.**
   Measured: 13 of the 52 module `.lm2` files are byte-identical to their
   `.lm1` apart from the `predef` line; the largest difference is 79 lines
   (`mixa_draw`). Nothing in them uses a Structure, a Message, `merge`, own
   fields or a callable Structure. That is legal L2 (spec 1.3, 20.2.2: the
   machine surface and explicit C interop are part of L2) and it is what the
   lead planned (§6.7: "the first independent test of library emission on
   real code"). It is a **differential compiler corpus**, and a good one.
   It is not a port to the L2 model, and "27 of 52 ported" / "52 of 52"
   should not be read as one.
3. **The application's Message architecture has no owner.** PROCESS_SEAM §6
   ("required NOW for the L1 application so the later L2 port preserves the
   architecture"), PORT_OF_CLEARSHELL §1.1 ("design the eventual Message
   boundary now") and the owner's 10.09 priority (STATUS §3: parallel
   execution of different Message handlers) have not moved since 10.09.
   Grok is closed, Codex is gone, the lead is on core and compiler, the
   helper is on verbatim ports.
4. **The two lines should be joined now.** The lead's own criteria for the
   ff-merge (§5) are met or moot; every day apart costs one lead round trip
   per runner change, because the helper cannot execute the L2 side on main.
5. **One contract change needs Mikhail's explicit yes:** the L1 gate was
   re-founded on `gen2 C == gen3 C` (gate status §11). I agree with it
   (see 2.3), but it was the criterion your decision 2 gated the promotion
   on, and it was changed by the lead alone.
6. **Speed:** 16,591 lines of near-identical runner scripts (52 files) and
   6,328 lines of near-identical port notes are where the helper's hours go.
   Every cross-cutting fix tonight was a 7-, 12- or 22-file edit followed by
   two re-runs per file.

---

## 1. Measured facts

| Item | Value |
| --- | --- |
| `main` HEAD | `311ee403` (helper's `mixa_audio_win32`) |
| `integration/main-absorbs-core` HEAD | `2cef53a2` |
| commits on `main` not on integration / on integration not on `main` | 1 / 222 |
| `mixa_manager/*.lm2` | 54 files on `main` (52 modules + 2 probes); 53 on the integration branch, which lacked `mixa_audio_win32` (`311ee403`) when the lead counted |
| module `.lm2` byte-identical to `.lm1` apart from `predef` | 13 (both trees) |
| module `.lm2` differing by ≤ 5 lines | 29 |
| largest `.lm1`→`.lm2` difference | `mixa_draw`: 79 changed lines by classic `diff` (`<`/`>` lines, `predef` excluded); the lead counted 77 on the same file. The file is identical on both trees; the two lines are the removed `---` cutters, which a unified-diff count that skips lines starting with a repeated marker drops. |
| `run_mixa_*_l2_parity.ps1` | 52 files, 16,591 lines |
| `*_l2_port.txt` | 6,328 lines |
| modules passing oracle-vs-L2 on integration (lead, 01:05) | 7: fm_remove, event_fifo, cmdline, buttons, app_panel, fm_copy, pump; plus app_fmpanel, app_path, help, selection passed the first sweep |
| core gates on integration (lead, gate status §1) | all green |
| full L1 gate on integration | green after re-founding (§11); `gen1 C ≠ gen2 C` by 18 lines, reported on every run |
| uncommitted in the main checkout | Codex's L1 hunks (`l1src/*`, `tests/l1/*`, identical to `7d8a5f09` on codex); the helper's runner consolidation (3 `.ps1` files) |
| stray | `Nyasha_Planetlingvamyxabuildfable_watch/` at the repo root: a watcher state path with its separators flattened |

Remaining translator gaps on the corpus (lead's table, gate status §15, plus
the helper's notes):

| gap | modules | what the spec says |
| --- | --- | --- |
| `const: @(Foreign)` return admits only `char` | app_controller | spec 20.2.2: declarator qualifiers are ordinary receiver composition (`const: @: T p` → `const T *p`). A gap, not a design question. |
| `<windows.h>` type | dir_win32 | out of scope by design, agreed |
| own array with a `define:`-named count | remove_confirm, selection_walk, copy, file_manager | engineering |
| `ulong` | process_win32, file_win32, process_marker | spec 6.6 line 4822: `ulong -> C unsigned long`. A new typed domain, as the lead says; spec-backed. |
| `wchar_t` | app_win32, the audio chain | foreign C type; same class as `ulong` |
| function-pointer formal | remove | fnptr as a local exists (`b0fd2276`); the formal is the gap |
| bodied `fn: … void` | calculator_syntax | spec 7.2–7.3 do not mention a void `fn`. Either admit it or rule it and have the port spell `sub:`. Decide and write it down; a refusal is not a rule. |
| unsupported index / unsupported body | share, cmdline_dispatch | read the run directory first (lead's own rule) |

---

## 2. Architecture conformance

### 2.1 Core lane

Checked against the model's own "find the settled answer" table and spec 21:

- Callable Structure `M` with METHOD in physical slot 0, `M` as the reserved
  first argument, copied by the common map with the descriptor address kept
  (model §8, spec 21.5, 21.8): implemented and proven (`d27e2b74`, callable
  paths, merge-on-callable).
- Message reaches a method only as a compiler-selected dynamic input in
  `sig`, transitively (model §9, spec 20.2.1): `08a6c1e4`, `162faac2`.
- References admit forward/self/mutual/nested targets, never reparent
  (spec 2.3, 19.29.3): the lead's own correction `ceb911bc`, with the
  right lesson written next to it.
- Field paths resolved at translation, no runtime name lookup (spec 21.1):
  `60ed1f16`, `b6165de8`, with the "two names collapsing onto one slot"
  assertion added after a green fixture hid it.
- Eternal branches and the two root arrays (spec 9.1.4): integrated and
  proven with 70-root fixtures; no runtime growth.

One design point to keep in view, not a defect today: a **library unit**
(`7d7ec87c`) builds its graph in `l2_library_open`. When the application is
assembled from many L2 units, that must not become N "first Messages" with N
METHOD arrays and N retention arrays. Spec 9.1.4 has exactly one OS-root
Message per process. The lead already names the bootstrap/load-order problem
(§4.6); the multi-unit assembly is the same problem seen from the app side,
and the mixa corpus will be the first place it shows.

Tonight's compiler work (typedef through `#include`, C function declarations
behind a header) is C-interop surface, needed by the corpus, and each landed
with a fixture that failed first. Fine.

### 2.2 Application lane

The helper's discipline is exemplary in the small: oracle-side runs twice,
exact barrier line and column, "unverified on my side" said every time, real
files never touched, a genuine finding (the MCI path-length limit) isolated by
experiment rather than by story. Nothing in 52 modules weakened the type model.

The problem is what the lane is producing. A `.lm2` that is the `.lm1` plus a
different `predef` line proves that `l2trans` lowers the machine surface the
way `l1trans` does. That is worth having once per construct, and the seven
passes plus the gap table above are that value, extracted. It says nothing
about the model in parts I–III, and it does not move the application toward
the architecture its own documents require:

- PROCESS_SEAM §6: "concurrent work and cross-thread coordination use Message
  … required NOW for the L1 application so the later L2 port preserves the
  architecture."
- PROCESS_SEAM §6.1: each background job is a Message with its own arena;
  `runOnUiThread` becomes a send to the UI Message's inbox; a job's chunk is a
  turn; cancel is a message in the other direction.
- PORT_OF_CLEARSHELL §1.1: "Design the eventual Message boundary now: each
  module has an explicit owner, inputs/results and ordinary callable
  operations."
- STATUS §3 (10.09, the owner): "complete parallel execution of different
  Message handlers, not just foreign ingress into a serial runtime."

None of that has an owner or a date. The real L2 port of the application is
**additive** to the verbatim corpus: the pointer-struct code stays as the
machine layer, and the new code is the owner Structures, the job Messages and
their turns. That new code is also the only outside consumer the core's
Message semantics (model §§25–34) will ever get before self-hosting.

The port notes and the ticket language should say this plainly: "52 `.lm2`
compile units exist; they are the L1 bodies; the L2-model port has not
started." Otherwise the next model to read "52 of 52 ported" will believe it.

### 2.3 The gate re-founding (a decision for Mikhail)

Gate status §5–§11: the full L1 gate was red on the merged branch because
`run_gen` required `gen1 C == gen2 C`, and gen1 is produced by the frozen
seed (`lm2/l1trans.lm2` through the old L2-profile chain) while gen2 is
produced by the current source. The seed drifts from the source by 59
functions (measured with `l2src/tools_seed_drift.py`). The lead changed the
gate: the certified fixed point is now `gen2 C == gen3 C`, three gen0 suite
steps report SEED, the 18-line gen1/gen2 difference is printed on every run,
and the gate was proved to go red when the fixed point is inverted.

My assessment: this is the standard bootstrap criterion (stage 2 == stage 3),
it is consistent with spec 1.2 ("a snapshot is a bootstrap artifact, not the
source implementation"), and the lead documented it rather than hiding it. I
would confirm it. But it is a change to the criterion your decision 2 gated
the pin promotion on, made by the lead alone after writing "I am not
choosing" one section earlier. It deserves an explicit yes from you, in
writing, in the model document §41.

Two things to add when you confirm it:

- Record the seed drift as debt with an owner and a plan. Spec 1.2 permits a
  **generated C snapshot** to seed a platform; the codex branch already
  regenerates `lm1/build/l1trans.lm1.c`. Seeding gen0 from that snapshot
  instead of from the frozen `.lm2`/`libparser.lm0.a` chain would retire
  this whole class of drift. Worth a decision, not a quiet change.
- The pin promotion changes a constant in 52 runner scripts (see 3.2).
  Sequence it as the lead already proposed: new hash announced to the helper
  first, then the swap, then a re-run.

---

## 3. Organization: what to change, in order

### 3.1 Join the lines now

Criteria from `CORE_LEAD_INSTRUCTION` §5 step 4: core gates green (yes),
L1 gate green (yes, after 2.3), "all 25 parity runners pass the L2 side" —
too strong; they never passed before, and the remaining failures are a
catalogued gap table, not regressions against `main`, where every one of them
fails earlier. `main` has one commit the integration branch lacks. There is no
reason left to keep two lines, and the lead has already made four merge
commits from `origin/main` to keep up.

Sequence (one evening, the lead drives, the helper does the constant swap):

1. Lead: promote the pin on the integration branch (copy gen2 into
   `stg/l1_baseline/build/l1trans/gen2/`, take the SHA256).
2. Helper: put the hash in ONE place (3.2) and switch it. One commit.
3. Lead: `git merge origin/main` once more, then `git checkout main &&
   git merge --ff-only integration/main-absorbs-core && git push`. Never
   force-push.
4. In the main checkout: `git checkout -- l1src tests/l1` (Codex's hunks are
   identical to the merged content). Do not touch the helper's uncommitted
   runner files.
5. Helper: run all 52 runners himself on `main`. From then on the lead stops
   being the measurement service.

### 3.2 One pin constant

The stable-translator hash is asserted in each of 52 runners. Move it to one
file (`mixa_manager/lib_l2_runtime_support.ps1` already exists; a
`L1_PIN.txt` read by it is even simpler) and have every runner assert through
it. The promotion then is a one-line change instead of a 52-file edit.

### 3.3 One runner

52 runners × ~320 lines, each a paste of the previous one. Tonight: the
fixture-root fix touched 22 files; the runtime trio touched 12; a missing
`$env:L2_RUNTIME_ROOT` line was missing from 7 because it was pasted without
it. Replace them with one `run_mixa_l2_parity.ps1 -Module <name>` and a
per-module table (harness file, fixture preparation, known-barrier
expectation, checks count). The per-module `tests/*_parity_harness.lm1` files
are legitimately module-specific and stay. `lib_l2_runtime_support.ps1` is the
first step; finish it before the next cross-cutting change, not after.

Also: the "run twice, byte-identical" rule is for evidence of a port. It is
being applied to runner-only edits on `main` where the L2 path cannot execute
at all — 44 runs for a one-line change. Run once in that case, and say so.

### 3.4 One table instead of 52 notes; refresh STATUS.txt

The `*_l2_port.txt` notes repeat one template (scope, verbatim copy, barrier
line, oracle runs, regression, files). Replace them with one table in
`mixa_manager/STATUS.txt` — module, commit, `.lm1`→`.lm2` diff lines, barrier
bucket, oracle checks, parity verdict on the integrated compiler — and keep
prose only for a genuinely new finding (the MCI path-length limit, the `fn …
void` shape, the "missing main in isolation" observation). `STATUS.txt` is
dated 10.09 and still names Codex, OpenCode and grok_bot as owners; the README
too. One pass to fix both.

### 3.5 Messages: short, measured, with the falsifying command

The tickets are 40–80 lines of narrative and three of them tonight were
corrections of the previous one (232000, 235500, and gate status §7–§9).
Each wrong cause sent the helper somewhere for an hour ("I am taking
app_panel; do not spend time"). The lead wrote the lesson itself: the cheap
check before the story. Concretely: before sending a diagnosis, include the
one command that would falsify it, or ask the helper to run that command
instead of sending the diagnosis. With direct messages, five lines with the
numbers is a complete ticket; the rules in `TICKET_RULES_EN.md` still apply to
the content, not to the length.

### 3.6 One live state file; archive the stack

The model document opens with six stacked "LATEST CHECKPOINT (supersedes
below)" entries; `CORE_TEAM_PLAN` is Codex's log; `STATUS.txt` is 10.09;
`INTEGRATION_GATE_STATUS` is the actual live state. Model §41 says old
material is either brought into line or explicitly archived. Do that: the
model document's top points to one live state file and nothing else;
`CORE_TEAM_PLAN` gets an "archive" line; `CORE_LEAD_INSTRUCTION` §4.9, §7.1,
§9.1 (watchers, inbox protocol, heartbeat discipline) are replaced by "direct
session messages; find the peer with the session list".

### 3.7 The helper's next lane

The port lane is now blocked on compiler gaps that only the lead can close
(`ulong`, own-array counts, fnptr formals, the `const: @` return). Re-running
audio-style barriers while waiting is the worst use of the helper. Two
candidates, both within his boundaries (§7.4), in the order I would pick:

1. **The parser in L2** (§6.5): assemble one `parser.lm2` from Codex's 25
   `parser_*.lm2` fragments with a differential runner against the 620
   reference (`run_p0_meta.ps1` with the L2-built parser). It is on the
   self-hosting critical path — the user's stated goal — it has an oracle,
   it is exactly the oracle-parity discipline the helper already applies,
   and it does not touch `l2trans.lm1`. Effort: high, per the lead's own
   §11 (new logic, not template ports).
2. **The Message boundary for one real job** (PROCESS_SEAM §6.1: command
   execution is the smallest of the four): owner Structure, job Message with
   inbox/outbox, turn per chunk, cancel in the other direction, against the
   versioned ingress seam and the current `lmx_message.h`. Design document
   first (one page, against model §§25–34), implementation when the lead
   says the API is stable enough. This is the "required NOW" item from
   10.09 and the only external consumer the Message runtime will have.

Mikhail decides which comes first; both are better than a 53rd verbatim port.

### 3.8 Effort settings

The lead instruction §11 is right: Opus 5 high for the lead; Sonnet 5 medium
was right for template ports. When the helper moves to 3.7, raise him to
high; the instruction itself says so for stages with new logic.

---

## 4. Sent to the chats

Both chats received a direct message on 2026-09-14 pointing at this file, with
the items that concern each of them (lead: 2.3, 3.1, 3.5, 3.6, the gap table
and the library-unit note; helper: 3.2, 3.3, 3.4, the run-twice note, 3.7).
Nothing in this review overrides the lead's ownership of the plan: it is
advice to Mikhail and to them, and the decisions in 2.3 and 3.7 are his.

## 5. Responses (2026-09-14, ~02:00)

Helper (`lingvamyxa-5e`): numbers confirmed (52 runners; 22/12/7 files
touched). Was mid-ticket on a narrower per-runner conversion onto the shared
helper; raised the overlap with the lead and held the pin constant, STATUS
table and note consolidation until the lead decided.

Lead (`lingvamyxa-d6`): checked the parts that concern it against sources.
Counts differ only by tree and method (table in §1 updated). Confirmed the
`const: @(Foreign)` return (spec 8002, 14614–14616) and `ulong` (spec 4822)
as compiler gaps, not design questions, and told the helper so. Wrote into
`INTEGRATION_GATE_STATUS` §16 that the `.lm2` units are the L1 bodies and the
L2-model port has not started. Closed the own-array define-count gap in the
compiler (four modules moved to a new first stop; graph gate 138/138; the
seven passes hold), commit pending the full `run_l2trans` pass. Stopped the
per-runner conversion (nine converted as `769c7581`); the helper is
inventorying all runners before proposing one parameterized runner with the
pin hash in one place, and will send the falsifying check before building.
Run-twice dropped for runner-only edits. The join sequence of 3.1 starts after
the pending commit: new hash to the helper first, swap, re-run, merge
`origin/main` once more, ff-merge, push. The lead will decide the bodied
`fn: … void` question and write it down.

Still on Mikhail's desk: confirming the `gen2 C == gen3 C` re-founding in
model §41, and whether gen0 should be seeded from the generated C snapshot.

## 6. Mikhail's decisions and the new division of work (2026-09-14, ~02:30)

Decisions, verbatim in substance:

1. **The L1 gate re-founding is confirmed.** `gen2 C == gen3 C` is the
   certified fixed point; the gen0 steps report SEED; the drift is printed.
   To be recorded by the lead in model §41 with this date.
2. **gen0 is to be seeded from the generated C snapshot**, not from the
   frozen `lm2/l1trans.lm2` + `libparser.lm0.a` chain. Acceptance: the full
   gate is green with `gen1 C == gen2 C == gen3 C` restored, and the gate
   also requires `gen2 C == committed snapshot C`, so a stale snapshot fails
   the gate instead of drifting silently. A separate stage after the merge
   and the pin promotion; owner: the lead.
3. **All of §3 is to be applied:** one pin constant, one parameterized
   runner, run-twice only for port evidence, one table in `STATUS.txt` with
   README/STATUS ownership lines fixed, five-line messages with the
   falsifying command.
4. **The review chat coordinates the group and codes the hardest
   parallelizable L2 tickets itself.** The helper's effort was raised to
   high by Mikhail.

Later the same night (~03:40–03:55), from the copier port's findings:

5. **The translator's `c.*` admission checks are removed.** Mikhail: "Зачем
   это нужно?! Уберите из кода всю эту паранойю." `c.` is the explicit
   foreign door: `c.name(args)` is emitted as `name(args)` as written,
   `c.NAME` as `NAME`; whether the name exists is the C compiler's job
   (every runner builds with `-Werror=implicit-function-declaration`). The
   header substring scan in `l2_quoted_has_function` and the closed
   known-C list go, with their diagnostics. Falsifier: a fixture with
   `c.no_such_function(1)` fails at gcc, not at l2trans. Owner: the lead.
6. **Foreign types are emitted as written.** Mikhail: "Да, и на типы тоже,
   выписывать как написано." A type name the translator does not know as an
   L2 Structure is a C type spelled verbatim (`@: wchar_t p` → `wchar_t *p`,
   `q\field` on a foreign pointer → `q->field`, any cast target as written).
   The "unknown foreign type" family and the closed type allowlist go the
   same way. Falsifiers: `@: no_such_type p` fails at gcc; the mixa modules
   stopped at `wchar_t`/`ulong`/`<windows.h>` re-run and are reported per
   module. Owner: the lead; the helper re-runs his side.
7. **Review-chat ruling, reversible by Mikhail:** a failed `l2_library_open`
   in a generated public wrapper takes the diagnostic route already
   confirmed for a failed checkpoint publication (spec 21.6 / 21.13.3): a
   stderr line naming the unit and operation on every failed call, abort
   under the abort policy, otherwise the same zero as before; no status code
   is invented. To be written next to the checkpoint rule in the model
   document by the lead.

8. **Mikhail (~04:05): a C function called without `c.` and outside a `C:`
   receiver is a gross architectural bug, in L1 as well as L2**, because LMX
   has its own namespace; otherwise L1 can never be ported to real L2.
   Consequence (Stage C, after the lead's Stages A and B): every `.lm1` in
   `l2src`, `l1src` and `mixa_manager` that calls a C-implemented function
   bare is fixed by its lane owner; then `l1trans` resolves bare names
   against LMX declarations only (unit functions and predef'd prototypes)
   with the unresolved diagnostic; full gate; pin promotion. Sources first,
   translator second, or the pin goes red on everything.
9. **Review-chat ruling, reversible by Mikhail: by-value `T: name` with a
   foreign type is spelled `c.T: name`.** Spec 9.2 makes `T: name` a
   constructor only when T resolves as a declared type; otherwise the form
   is a rebind (14.1). Foreign things take the `c.` door (20.2.2, 6.6.6), and
   `c.T` as a type spelling already exists (`c.sizeof(c.LmxMsgCopy)`). So a
   by-value local declares when T is a primitive, a type parsed from a
   predef'd `.h.lm1` declaration, or `c.T`; pointer, const, cast, sizeof,
   formal and return positions take any T verbatim with no `c.`. The
   C-header typedef scan is deleted either way.

10. **Review-chat ruling, reversible by Mikhail (~04:50): a by-value local
    of any language primitive type declared in a body is an own field**, a
    Message-owned typed cell published at the dirty checkpoint, not
    activation-local C storage. Spec 21.5.1 makes every declared body field
    a graph field with no exception by type; 6.6 lists `ulong`, `u8…i64`,
    `float`, `double` as language numeric types; 9.0 calls the i/u family
    payload domains over C storage. The runtime's four types (int, char,
    size_t, unsigned) are the implemented subset. Foreign C aggregates stay
    activation-local substrate (spec 20.2.1; commit a94461b2). Sequencing:
    new domains are added as the corpus needs them (`ulong` first); a
    by-value local of a not-yet-implemented numeric type is refused with a
    named diagnostic, never emitted as C storage and never silently.
11. **Lead's decision, accepted (~04:30):** `lm_own_*`, `lm_p0_*`,
    `l2_immut_query_fill` and `l2_hash_compare_q` are LMX names installed by
    the L2 profile (spec 9.0/9.2), with their own lowering; they need neither
    `c.` nor an explicit header, and the list is the adapter of 6.6.6, not an
    admission allowlist. Bare `getenv`, `strcmp`, `memcpy`, `fopen` and the
    like are C and need `c.`.
12. **Mikhail (2026-09-14, on the escape poll after a successful `complete`):**
    a Message, one-shot or a longer goal-directed process, must always be
    known to have arrived / been done; after its completion the parent no
    longer needs it; the child decides its own fate, it is the one polling the
    parent, and a parent closing a child is the exception, not the norm.
    Review-chat reading carried back for confirmation: the escape unwinds a
    turn only for a REQUESTED stop (running=0 with success=0, the closing
    protocol), never after the child's own `complete` (success=1 then
    running=0, model §31); after `complete(self)` the turn runs to its normal
    end and `end_turn` reports success. Consequence for the translator:
    user-profile units get a success-aware poll; runtime-profile units (the
    executor's own primitives: complete, end_turn, exec_ready, closing) are
    not polled at all, since the executor calls them during an unwind. Found
    porting `lmx_message.lm1`: the unit passes the whole executor suite
    without the poll and is red only at `complete` with it
    (RUNTIME_L2_PORTS.txt, "THE ESCAPE-POLL FINDING").
13. **Mikhail (2026-09-14, on the port boundary of the executor C files),
    corrected by him after a wrong first reading:** one arena strictly per
    Message; arenas only attach. "A Message knows only its minimum" is not a
    translator restriction but the model itself: a Message is created by
    merge, so it holds only what the merge gave it. L2 sees ALL of C through
    the `c.` door; nothing additional is to be built, no provider contract
    for the executor's platform layer. Only L3 sees no C at all, and nearly
    everything moves to L3 later. A Message is already isolated three times:
    at creation (merge), at validation by the receiving side (the parent
    decides what lies in it), and, when needed, by libsodium (spec 19.32.8,
    19.32.14). Isolation is a ladder whose top rung is N isolated OS
    processes; the transport can manage the flags itself across them. The
    review chat's first reading (a documented provider contract that hides C
    from L2, with a secure-memory API as the first task) is withdrawn.
    Consequence for the port: `lmx_message_exec.c` / `lmx_message_host.c`
    are ported to L2 calling their platform functions through `c.` as they
    are; what L2 cannot spell (the setjmp/longjmp turn root, thread-local
    declarations, the Win32/pthread conditional blocks) stays C behind its
    existing functions; libsodium only when needed.
14. **Mikhail (2026-09-14, on `complete` and the flags), superseding the
    reading in 12:** the child has the right to declare that it has done all
    its work; the consuming side decides by validation whether it received
    the Message and never touches the child's flags. `complete` sets only
    success=1. running=0 follows success=1 ONLY in `end_turn`, at the turn
    boundary; inside a turn running stays 1, so running=0 means exactly a
    stop request (the closing protocol). Consequences: the escape poll of
    user-profile units stays a single-flag check of running (no two-flag
    predicate); runtime-profile units are emitted without escape polls,
    marked by the top-level directive `profile: runtime` (checkpoint abort
    paths kept); the runtime change (complete no longer clears running;
    end_turn clears it when the success flag is set) is made in
    `lmx_message.lm1` and `lmx_message.lm2` identically by the review chat.
    Mikhail's addendum: running is touched by the one who EXECUTED the
    Message; not every Message has its own thread and turn; a plain letter
    that never executed itself gets running=0 from its consumer; the pair is
    tracking: (1,0) not yet arrived or in work, (0,1) done, (0,0) finished
    without success, which starts the parent's cleanup. Implemented as
    2494848a on `fable/runtime-l2`: besides complete and end_turn, the
    executor's turn entry settles a Message with success=1 without running
    its body, and its boundary after the body clears running from success=1
    even when the body neither received nor ended its turn.
    Mikhail's correction of the review chat's "letter in transit is an
    envelope copy": no such entity exists in the model. The Message wrapper
    is the same for executed and non-executed Messages, and every Message
    carries the running/success pair; the L3 Thread wrapper (execution, its
    own mail) appears only for launched Messages; a consumer that only read
    a Message by attaching it uses its own thread and mail for every action,
    including executing that Message as part of itself, and sets its
    running=0. Consequence: nothing to add in the runtime; the pair already
    lives on every LmxMsg, the Thread wrapper is today's exec.c binding of
    launched Messages, and the byte-envelope send/recv (LmxMsgCopy) is an
    L1-era transport mechanism, not a model entity, kept until transport
    becomes Message attachment unless Mikhail asks for that now.
15. **Mikhail (2026-09-14, on "not now"):** "how could it be not now?" L1
    was deliberately left to the agents in a freer form because its purpose
    is preparation for a fairly direct translation to C; L2 must be
    implemented exactly per the specification (written by Astra from his
    words). And: "copy-only mailbox" is only a retelling of what merge does,
    a copy, and the Message has its own arena; that is all, and it is true
    for every Message. Review-chat reading, carried back for confirmation:
    the L2 runtime is not a port of the L1 prototype but an implementation
    of spec 19.28.R2.2, 19.29.6 and 19.29.7: Message management state as
    Structure data (arena, flags, FIFO mailbox, parent capability, direct
    children, parent-owned scheduler state); delivery of a Message created
    by merge (one copy, its own arena) by ownership transfer of that arena
    to the receiver; explicit launch makes a child with its own arena; no
    envelope entity, no shared runtime table, no exec host; native wait/wake
    through `c.`; acceptance by the spec's own checks (19.29.6
    implementation/integration checks and the model's §31-§36 scenarios),
    not by L1 selftests; L1 stays the temporary bootstrap that 19.29.6
    allows. The existing L2 ports remain evidence that the translator
    expresses the runtime (seven translator gaps found) and are not the L2
    runtime. §6.4 of the lead instruction is to be rewritten in these
    terms once Mikhail confirms.
    **Confirmed by Mikhail (2026-09-14): "да, подтверждаю."** The plan is
    written out in `L2_RUNTIME_PLAN_20260914.md`.
16. **Mikhail (2026-09-14, working rules):** clarifications the spec needs
    (for example that "copy-only mailbox" is not an additional mechanism)
    are made, committed and pushed by the review chat as docs (done in
    19.29.7.1 the same day); all L2 code is committed at every stage; the
    whole project is committed every time a full self-build passes.

17. **Mikhail (2026-09-14, family release):** a released branch is never
    retained, and release does not wait for the children. (1) Forced release
    of a child by its parent starts a chain reaction down the subtree; (2) a
    child's self-close on timeout starts the same chain for its subtree;
    (3) success cannot appear on a parent whose children are running=1 with
    success=0; a parent whose own algorithm sets it declares the children's
    work unneeded and the chain closes them. (4) No release "into the open": the
    only way a running Message survives its parent's closing is a handoff of
    supervision to another live parent, chosen by the closing parent among
    the capabilities it holds, not necessarily its own parent (corrected the
    same day); supervision moves, storage does not (storage adoption still
    needs a non-executing handoff-safe source); for the root the only new
    parent is the virtual World Wide Mix ancestor at the OS-process level, a
    stub until stage 5. Written into the spec (19.29.6)
    and the model (section 32). The L1 runtime keeping stopped and disposed
    children linked until runtime_delete (found during 3b-8) is a stage-4
    defect in the plan, acceptance test first.

Division of work from here:

| Who | Owns | Now |
| --- | --- | --- |
| Review chat (Fable 5.1, `lingvamyxa-e2`) | coordination: assignments, acceptance of stages, decisions carried to Mikhail; core L2 runtime modules that parallelize with the lead | §6.4 of the lead instruction: `lmx_msg_mail_chain` (ticket 111000), then `lmx_graph_copy_owned` (ticket 152000), then the allocators; branch `fable/runtime-l2` from the integration branch, worktree `build/fable/runtime-l2` |
| Lead (Opus 5, `lingvamyxa-d6`) | compiler and runtime core, integration and merge, pin promotion, core documents | pending commit → pin 722AC86E to the helper → ff-merge → decision 2 (C-snapshot seed) → §6.3 self-hosting; compiler gaps from the corpus and from the review chat's ports |
| Helper (Sonnet 5 high, `lingvamyxa-5e`) | `mixa_manager`; isolated core pieces that do not touch `l2trans.lm1` | one runner + one pin constant + run-once rule + STATUS/README table; then the parser in L2 (§6.5) with the differential runner against 620 |

Message rules for all three: direct session messages; first line states the
result; measured numbers; the command that would falsify the claim; the
`TICKET_RULES_EN.md` statuses apply to content, not length. In-flight
mechanics of the merge (hash swap, re-runs) go lead ↔ helper directly; new
assignments come from the review chat.
