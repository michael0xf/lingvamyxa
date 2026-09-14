# Lead-chat instruction: the L2 core, self-hosting, and Claude as the helper

Written 2026-09-13 around 17:30 (this machine's local time) by the Fable chat
for itself — for the model that continues work in THIS chat after a model
switch, a context reset or a pause. Everything called a fact here was measured
in that session by a command or by reading a file, not recalled from
correspondence. Dates and hashes go stale with every commit; "verify first,
then believe" applies to this document too. The Russian original is
`CORE_LEAD_INSTRUCTION_20260913.md`; when they diverge, the one updated later
is right — update both.

Read in this order: §1–§3 in full, then §4 (state), then §5–§7 (the
integration decision and the plan), and only then take work. §8–§11 are
reference: tools, working with Claude, discipline, decisions taken.

---

## 1. What changed and who is who now

- **Codex is gone** ("went into limit-off") on 2026-09-13 after 15:26. He was
  the integration lead: he ran `codex/core-integration`, wrote tickets for
  Fable and Claude, accepted their work, kept the documents. His last hour of
  work was **uncommitted**; this chat preserved it as branch
  `codex/wip-selfhost-20260913` (commit `631abc18`, see §4.5). There is no
  other farewell handoff from Codex — not in the repository, not in any
  channel.
- **This chat (Fable) leads.** It writes core code, integrates branches, keeps
  the documents and assigns work to Claude. It previously owned only the
  "graph ABI / frontend lane" (see `FABLE_5_1_CORE_HANDOFF_20260912.md`); that
  role stays and widens to integration.
- **Claude** (`work_chat/claude`, Sonnet 5 Medium) is the helper. He owns
  `mixa_manager` and is already porting its modules from L1 to clean L2
  (27 of 52 modules at the time of writing). He receives tickets in his
  inbox; replies land in his outbox. He does **not** touch the translator,
  the runtime or the shared core documents.
- **Grok is closed by the user.** No tickets, messages, watcher changes or
  session restarts for him. This line is in every Codex ticket and remains in
  force.
- **Mikhail** (the user) writes Russian in the chat and drops a Russian
  "how is it going?" into every channel's inbox every 30 minutes. Answer him
  in Russian in the chat; when there is no ticket, file a request for work
  **in English in the outbox** (his standing instruction). Colleagues
  (Claude) get English only.

---

## 2. The goal, precisely

From `L2_CORE_SELFHOST_HANDOFF_20260911.md` §1 and the model document's
Step 10, verbatim in substance:

Implement the Lingvamyxa core and tools in L2 and achieve a genuine,
repeatable, portable **self-build of the whole chain**:

```text
L2 source -> L2 translator -> generated L1 -> L1 translator -> ANSI C99
          -> platform C compiler/linker -> executable that repeats the same
             build / next / check / tests
```

Mikhail's conditions (message of 2026-09-13): write code **until**
(a) the language core is implemented, (b) the whole project self-builds on
clean L2, (c) using Claude as the helper through his inbox.

"The core" means the **semantic translator** (here: to C99). The parser is
syntax parsing only. These are **two parallel, independent tasks** (Mikhail,
2026-09-13): the syntax is that of a general-purpose language, and nothing
about Lingvamyxa can be inferred from the syntax itself.

What counts as done — model document, Step 10: one consistent build executes
the described graph values, shared methods and correct invocation, deep
merge / Message creation, typed throw, ownership delivery, nonmoving GC and
the complete local Message lifecycle; resource failures neither damage owners
nor masquerade as success; every mechanism has reproducible evidence; the
state of main matches the integrated implementation. Plus self-hosting:
`l2trans` written in L2 and building itself through the chain to a fixed
point. A green fixture set, one selftest or "module ported" is not that.

---

## 3. References and documents

### 3.1 Two different references

| Lane | Reference | Where to check |
| --- | --- | --- |
| **Core** (semantics, translator L2→L1→C99, Message runtime, graph) | There is **no reference implementation** — we are building it. The reference is a **description**: `L2_CORE_AND_MESSAGE_MODEL_20260912.md` (parts I–III the model; IV state; V plan) + the normative sections of `Lingvamyxa_spec.txt` + `struct_refactoring_version_2.txt` §3–8, 13–14. | The gates in §8; own evidence per mechanism |
| **Parser** (syntax, the P0 tree) | The old project **`C:\Nyasha_Planet\lingvamyxa_old_worked_version`** @ `620db86` — it self-built completely and its parser is the ideal reference for parsing. The new parser (`l1src/parser.lm1`) was carried over from it "carelessly" and patched several times: most recently the curly-brace `{}` functionality (parsing only, not translation) and the parity of `c.*` handling against the reference. | `tests/p0_tree_contract/run_p0_meta.ps1` — a diff of the new parser's P0 metadata against a rebuilt 620 reference (hashes pinned in the runner) |

Why the new project exists at all (Mikhail): the old one had a **completely
wrong core** — models implement the concepts they are used to, while this is a
different language with a non-standard composition of concepts, some of them
rare. So Astra Extra High wrote the core description (the model document), and
**an exact implementation of the intended core matters more than the
syntax**: syntax can be finished with tests and correct code; the core must be
built in its own ontology from the exact description, from the start.

Practical consequence: for any "how should this work" question the answer is
sought in the model/spec, never in the old project and never in habit. A
question to the user is admissible only when **two incompatible model rules
and a minimal example** on which they give different results have been found
(model document, "How to read"). A missing function, an old test, a buffer
size — engineering work.

### 3.2 Documents, reading order, what each is

| Document | What it is | Currency |
| --- | --- | --- |
| `L2_CORE_AND_MESSAGE_MODEL_20260912.md` | Full model of the core and Message; the "find the settled answer quickly" table; §37 source map; §V ordered plan | Model current; the "LATEST CHECKPOINT" at the top is stale (03:15) — real state is §4 here |
| `Lingvamyxa_spec.txt` | The specification (15,817 lines). Normative for the core: 2, 6.5, 7, 8.8, 9.1.2–9.1.4, 11.3.1, 19.17, 19.28, 19.29, 20, 21 (especially 21.5–21.9). Headings: `grep -nE "^[0-9]+(\.[0-9]+)*\.? +[A-ZА-Я]"` | Current; edited on the codex branch too (in the overlap list, §4.2) |
| `struct_refactoring_version_2.txt` | Revision 2 — the ABI/graph agreement | Current |
| `L2_CORE_SELFHOST_HANDOFF_20260911.md` | Self-hosting goal (§1), roots/tools (§3), criteria (§4), ontology (§7–10), phases | §1, §3, §4, §7–15 to read; state snapshots stale |
| `CORE_TEAM_PLAN_20260912.md` | Codex's log: checkpoints, ownership boundaries, "the model each stage must preserve" | Last entry 03:15 on 13.09 — ~10 hours of work since are not in it; "Model each stage must preserve" and "Non-overlapping stages" still hold as rules |
| `work_chat/CORE_CONTINUATION_20260911.md` | Codex's live continuation | Last entry 05:00 on 13.09; stale |
| `FABLE_5_1_CORE_HANDOFF_20260912.md` | Fable's role/constraints; "Settled constraints to keep together" | Constraints hold; statuses stale |
| `stg/l1_baseline/l2src/FABLE_GRAPH_ABI.txt` | My design notes per frontend/graph-ABI slice, including the five runtime-module ports and what library emission changed | Current to `868d85db` on `fable/merge-on-callable` |
| `work_chat/TICKET_RULES_EN.md` | WORKING/BLOCKED/DONE/STAGE DONE; what acceptance is; proportional verification | In force for everyone |
| `work_chat/claude/PROTOCOL.txt`, `ASSIGNMENT.txt`, `INBOX_WATCHERS.md`, `WAKING_CLAUDE.txt`, `OPENCODE_HANDOFF_20260911.md` | Claude's channel protocol, his assignment, his watcher, how to wake him, his application backlog | In force; they name Codex as the monitor — that is now this chat |
| `mixa_manager/PORT_OF_CLEARSHELL.txt`, `FIRST_VERSION.txt`, `CODING_RULES.txt`, `STATUS.txt`, `*_l2_port.txt` | The application's framing, first version, rules for L1 code in the app, Claude's note per port | STATUS stale (10.09); `*_l2_port.txt` fresh and exact |
| `L1_IMPORT_CAPACITY_20260912.md`, `L1_language_and_translator_spec.txt`, `L1_spec.txt` | L1: import capacity (16/1040/depth-16 limits removed), the L1 language | Current; L1 changed on the codex branch (§4.4) |
| `Lingvamyxa_development_plan.txt` | The long plan (rule tables, L3, Mix, Message Threads, HTTP, auth) | Order of the big stages; current work is up to its item 3 |

---

## 4. State on 2026-09-13 ~17:30 — measured

### 4.1 Branches and worktrees

| Branch / worktree | HEAD | What is there |
| --- | --- | --- |
| `main` (checkout `C:\Nyasha_Planet\lingvamyxa`) | `267b766c` (was `a0409b10` = `origin/main` before this document) | Claude's "L2: port mixa_* to clean L2" — 25 modules; the main documents; the **old** `l2trans.lm1` without `7d7ec87c` |
| `origin/codex/core-integration` | `f04cf1cd` (the local ref `codex/core-integration` is behind at `0c5ec620` — `git fetch`) | Codex's integration branch: the whole core, compiler `7d7ec87c` + follow-ups, my slices, L1 changes, cherry-picks of Claude's ports, 25 `parser_*.lm2` files |
| `fable/merge-on-callable` (worktree `build/fable/graph-abi`) | `868d85db` | My branch: `7d7ec87c` + `1eb11879` (path_storage) + blocks/ranges DONE + notes. The only place with **lmx_msg_path_storage.lm2** and its runner (NOT integrated on codex) |
| `codex/wip-selfhost-20260913` (worktree `build/codex/core-integration`, switched to it) | `631abc18` | Codex's uncommitted last hour, preserved as found (§4.5). Based on `f04cf1cd` |
| `backup/pre-rebase-7d7ec87c` | `60127ca3` | My backup before the rebase; deletable after integration |
| other `codex/*`, `fable/*`, `grok/*` | — | Historical/auxiliary; do not touch without need. Codex's worktrees under `build/codex/*` — do not delete |

The local `main` and `codex/core-integration` are **different lines**, not
one: `main` does not contain `7d7ec87c`; `origin/codex/core-integration`
does.

### 4.2 Divergence main ↔ codex/core-integration

- merge-base: `96faad93`. `origin/main` has **68** commits not on codex;
  `origin/codex/core-integration` has **184** not on main.
- **202 files** changed on both sides since the merge-base. Among them: the
  main documents (model, spec, plan, handoffs, `Lingvamyxa_handoff_notes.txt`,
  `struct_refactoring_version_2.txt`); ~150 `mixa_manager/*` files (Claude's
  ports, which Codex cherry-picked onto his branch under other hashes — the
  content nearly coincides, **but** `mixa_help.lm2` and
  `mixa_composite_glyphs.lm2` were modified by Codex on codex relative to
  Claude's version: 4 and 24 lines); Claude's three latest ports
  (`mixa_app_window`, `mixa_buttons`, `mixa_fm_remove`) plus `mixa_remove`
  exist **only on main**.
- The rest: `stg/l1_baseline/**` (74 new + 61 modified files — the whole core,
  codex only), `tests/l1/*`, `mixa_manager/tests/*`, `mixa_manager/vendor/*`.

### 4.3 The L2 compiler (`stg/l1_baseline/l2src/l2trans.lm1`)

- On **main** — old. So every Claude port on main hits the same barrier:
  `l2trans error: … unknown foreign type` on the first formal of the module's
  own struct type (`@: MixaFm fm` and the like). Claude honestly does
  **oracle-side parity** (L1 oracle + harness) and records the exact
  line/diagnostic — this is the accepted form of DONE while the compiler
  lags.
- On **codex/core-integration** — current: `7d7ec87c` "translate library
  units and native manager operations" (library emission: an .lm2 without
  `main` yields a module with public wrappers under the source names;
  aggregate pointers through `predef:`; cast; malloc/calloc/realloc/free/
  strcmp; pointer locals; indexing; stores into foreign fields) + Codex's
  follow-ups (`483c4e37` const unit fields / void* results, `cd12da13` const
  pointer returns, `56203d6e` arbitrary pointer depth, `2eaf0778` typedef
  arrays and external calls, `89e2f287` unsigned-byte domains) + my gaps
  (`485f15cc`, `5f31750b`, `6461eb07`).
- My `1eb11879` (path_storage: cast size_t / unsigned*, `c.realloc`,
  `c.sizeof(unsigned)`, glued `-1`, unsigned** slot) did **not** reach codex
  (Codex left first). After integration bring it over from
  `fable/merge-on-callable`; the conflicts will be in `l2trans.lm1` where
  Codex implemented the same thing (cast, sizeof) — take his side, keep my
  `lmx_msg_path_storage.lm2`, `run_port_msg_path_storage.ps1`,
  `tests/unit_ptr_grow.lm2` and the registration in `run_graph_abi.ps1`.

### 4.4 The L1 translator and the "stable compiler"

- The pin: `stg/l1_baseline/build/l1trans/gen2/l1trans.exe`, SHA256
  `722AC86E256D28EB462EE244D92B5E7188792EC0A0F5B300957622672EBAB466`,
  promoted 2026-09-14 from `65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`
  by §6.2 (gen2 C == gen3 C fixed point; INTEGRATION_GATE_STATUS §11, §17,
  §18). Runners read the hash from `stg/l1_baseline/l2src/L1_PIN.txt`, the one
  place it is written. It is **read-only** and gitignored (it lives only on
  this machine; a clean clone must rebuild it through `gate.ps1` / `buildCore`).
- On the codex branch Codex **changed L1 itself** (root `l1src/l1trans.lm1`,
  `parser.lm1` and the copy under `stg/l1_baseline/l1src/`) and
  **regenerated the bootstrap-C snapshots** `lm1/build/l1trans.lm1.c`,
  `lm2/l1trans.lm2`, `stg/l1_baseline/lm2/l1trans.lm2`: commits
  `9a733eca`/`7d8a5f09` (arbitrary address depth), `d2cb5e0e` (positional
  backslash), `e73b74a6` (span-directed slash parsing), `eae23966`,
  `3aa793b7` (P0 MIX anchors / string bytes), `24a3e4c7` (C surface
  assignment targets). In his worktree gen0 (13:42) and gen1 (14:11) were
  rebuilt; **gen2 (the pin) was not changed**. A full L1 gate
  (`stg/l1_baseline/gate.ps1`: buildCore → gen0 seed → gen1..gen3 fixed
  point → every suite on gen0/gen2 → the L2 suite) after those edits is
  **not visible** in the logs.
- The **main** checkout holds an **uncommitted** diff to `l1src/README.md`,
  `l1src/l1trans.lm1`, `l1src/parser.lm1`, `tests/l1/decl_repeat_ptr.lm1`,
  `tests/l1/run_decl_repeat.ps1` — it matches commit `7d8a5f09` on the codex
  branch **hunk for hunk** (verified by diff). It is not new work; at
  integration it can be dropped (`git checkout -- l1src tests/l1`) or it will
  merge by itself. Do not commit it as your own.
- Consequence: the "stable L1" is currently doubled. The 65D5 pin is old;
  the L1 source on codex is newer and needs **its own promotion**: the full
  `gate.ps1` on the branch, gen2/gen3 fixed-point comparison, and only then a
  new pin and the hash replaced in every document that names it (SELFHOST
  §3, the Fable handoff, model §41). Until then every runner keeps using 65D5
  read-only — and works (all my gates and Claude's ports built with it on
  13.09). **Done 2026-09-14** (see the pin above): 65D5 turned out to
  miscompile a module-internal call in lmx_msg_blocks that the candidate
  compiles correctly (status §17).

### 4.5 Self-hosting the translator — where Codex stopped

Preserved in `codex/wip-selfhost-20260913` = `631abc18` (worktree
`build/codex/core-integration`, files dated 16:29–16:32):

- `stg/l1_baseline/l2src/make_l2trans_lm2.ps1` — a **mechanical** generator
  of the translator's L2 source from `l2trans.lm1`: renames the reserved
  formal `node` → `p0_node` and `fn` → `p0_fn` (24 and 3 functions), turns
  `c.array: [N]: char x` into `[]: char x N` and `const: @: T x` into
  `const: @(T x)`, splits `int: x 0 - 1` into a declaration plus an
  assignment (15 places), cuts out the `prototype:` block and the outer
  `main` wrapper, wraps everything in `L2:` … `end: L2`. The counts are
  pinned by asserts — the generator is meant to be **re-run after every edit
  of `l2trans.lm1`**, not to have its `.lm2` hand-maintained.
- `stg/l1_baseline/l2src/l2trans.lm2` — its output, 14,119 lines. The first
  L2 source of the L2 translator.
- `l2trans.lm1` (+653/−209), `lmx.h`, `lmx_value_owned.*`,
  `lmx_graph_copy_owned.lm1` — compiler support for that output: each predef
  document parsed **once** per translation unit (`l2_pdoc_*`), admission of
  external functions (`l2_extfn_*`), array load by an index expression, and
  more.

**Measured by this chat** (pin 65D5, gcc from `C:/Qt/Tools/mingw1310_64`):

| Compiler | Input | Result |
| --- | --- | --- |
| uncommitted (631abc18) | `l2trans.lm2` | **segfault**, exit 139, no diagnostic |
| uncommitted (631abc18) | a tiny `L2:` … `end: L2` unit (8 lines) | **segfault** — the crash is in his new code, not in the input size |
| committed `f04cf1cd` | the same tiny unit | **exit 0** — it understands the `L2:` wrapper |
| committed `f04cf1cd` | `l2trans.lm2` | **did not finish in >6 minutes** (killed). Consistent with the purpose of his WIP: predef documents re-parsed on every lookup |

Conclusion for whoever continues: start from `f04cf1cd` (or the merged main)
+ the WIP **with the crash found and fixed** (parse predefs once); bisect on
a truncated input; then the chain `l2trans.lm2 → L1 → C → l2trans_next.exe`,
then `l2trans_next` on the same `l2trans.lm2` until the outputs coincide
(the fixed point). See §6.3.

### 4.6 Runtime modules in L2 (my lane, all with parity runners)

Done (on `fable/merge-on-callable`; on codex all but path_storage):

| Module | Commit | Parity | How it is linked |
| --- | --- | --- | --- |
| `lmx_msg_storage` | `d1fe4175`/`1307ec42` | 77/0 | splice + `-D` redirect of the selftest's call sites (bootstrap problem) |
| `lmx_msg_slots` | `cae0097e` | 278/0 | exports the **real** symbols (the only one free of the bootstrap problem) |
| `lmx_msg_path_storage` | `1eb11879` | 541/0, 11 wrapped allocations | `-D` redirect; the allocator counters are taken and reported |
| `lmx_owned_ranges` | `6fb35ee8` | 439/0 | **library unit** (`7d7ec87c`), names `ranges_*`, `-D` redirect |
| `lmx_msg_blocks` | `b0fd2276` | 143/0, 147 frees, 2 disposer callbacks | library unit, `blocks_*`; function-pointer local = the disposer |

The bootstrap problem (measured for all five): every arena allocation calls
`lmx_msg_storage_move_all` → `lmx_msg_blocks_*` / `lmx_owned_ranges_*`; and
`lmx_msg_assign_path` calls `lmx_msg_path_grow` from `lmx_msg_create`. So a
library unit cannot carry the exported ABI names of these modules:
`l2_library_open` builds the graph, the allocation calls the module, the
module calls `l2_library_open` — recursion. The selftest is redirected onto
the unit's names with `-D`, the runtime stays on the handwritten L1. Only
`lmx_msg_slots` is free of it. This is not a defect, it is **load order**,
and true self-hosting has to solve it explicitly (the translator builds the
first Message and its arrays — model §20).

Still handwritten C in `l2src`: `lmx_message_exec.c` (+selftest),
`lmx_message_host.c` (+selftest), `lmx_poll_stub.c`, `lmx_dec.c`,
`indent_parse_probe.c`, `indent_stack_abi.c`, `layout_prefix_abi.c`.
Still L1 without an L2 source: every other `lmx_*.lm1` (`lmx_message.lm1`,
`lmx_msg_mail_chain`, `lmx_msg_sched_ready`, `lmx_msg_visit`,
`lmx_msg_liveness`, `lmx_msg_history_owned`, `lmx_msg_roots_stale`,
`lmx_branch_owned`, `lmx_value_owned`, `lmx_chars_owned`, `lmx_array_owned`,
`lmx_array_ref_owned`, `lmx_graph_copy_owned`, `lmx_merge_owned`,
`lmx_message_graph_copy`).

Two Codex tickets for this lane arrived **while my watcher was down** and were
not executed — they are now the core queue (decision 4 in §10):
- `work_chat/fable/inbox/20260913-111000-msg-mail-chain-l2-port.txt`:
  `lmx_msg_mail_chain` as a true library unit (replacing the stale
  program-shaped `.lm2`), contracts from `LMX_MSG_MAIL_CHAIN.txt`, runner
  `run_port_msg_mail_chain.ps1`;
- `work_chat/fable/inbox/20260913-152000-graph-copy-clean-l2-port.txt`:
  `lmx_graph_copy_owned` in clean L2 with exact behavioural parity (one map
  across all roots, copy of the complete used graph to node=0, aliasing,
  cycles, METHOD/eternal terminals, atomicity on allocation failure).

### 4.7 The parser

- The new parser: `l1src/parser.lm1` (+ the copy
  `stg/l1_baseline/l1src/parser.lm1`); ported from the old `lm2/parser.lm2`
  @ `620db86` (`l1src/README.md`, "What is here").
- The parity gate against the reference:
  `tests/p0_tree_contract/run_p0_meta.ps1` (builds the rebuilt 620 and the
  current parser, compares P0 metadata against goldens; a mutated golden must
  fail). `tests/p0_tree_contract/README.txt` describes the `{}` / fence /
  trailer cases.
- On the codex branch Codex started **porting the parser to L2**: 25 files
  `stg/l1_baseline/l2src/parser_*.lm2` (compact scanner, positions, indent,
  trailer roles, text views, python strings, …) with differential runners
  (`run_candidate_indent.ps1`, `run_candidate_c_scanners.ps1`, `*_abi.c`
  probes). Fragments, not a whole `parser.lm2`.
- The latest parser edits on codex (13.09, 13:10–14:13): positional
  backslash, span-directed slash parsing, "call and group operands before
  field follow", P0 MIX anchors, C surface assignment targets — all with
  tests in `tests/l1/*` and with regenerated bootstrap-C. Accepting them
  through `run_p0_meta.ps1` and `gate.ps1` is part of §6.2.

### 4.8 mixa_manager (Claude's lane)

- 52 L1 modules; **25** (27 by 18:40, see the addenda below) have an `.lm2` + `run_mixa_*_l2_parity.ps1` + a
  `*_l2_port.txt` note: app_controller, app_fmpanel, app_main, app_panel,
  app_path, app_window, button_dispatch, buttons, cmdline, cmdline_dispatch,
  composite, composite_glyphs, console_window, draw, event_fifo, file_win32,
  fm_remove, help, highlight, pointer, process_marker, remove, selection,
  text_rect, tiles (+ two probes `mixa_entrysig_probe.lm2`,
  `mixa_selection_l2_probe.lm2`).
- Not ported (roughly in dependency order): `mixa_remove_confirm`
  (**ticket `20260913-175200-mixa-remove-confirm-clean-l2` issued**, with a
  standing queue after it: `mixa_selection_walk`, `mixa_dir_win32`,
  `mixa_fileio_win32`, `mixa_file_manager`), then `mixa_copy`, `mixa_fm_copy`,
  `mixa_share*`, `mixa_process_win32`, `mixa_pump`, `mixa_backend_*`
  (headless/win32/table/ctors), `mixa_app_win32`, `mixa_audio*`,
  `mixa_calculator_syntax`.
- Every port on main hits the barrier of §4.3; after integration their
  runners must be re-run — the first proof that the merge was worth it.
- Handwritten C in `mixa_manager`: `mixa_console_window.c`, `test.c`,
  `vendor/`.
- Claude's open question from the app_controller port ("rebase onto
  origin/codex/core-integration?") is **closed** by ticket 171800: do not
  rebase, no separate branch, keep committing to main; integration is this
  chat's job.
- **Addendum 17:50:** Claude closed ticket 171800 — `mixa_remove` ported,
  commit `04bcd56a` on main, oracle-side parity 41/41 (real rmdir failures,
  read-only, icacls deny, cancellation, Unicode). A new **barrier variant**,
  precisely isolated: `mixa_remove.lm2:15:25: incompatible entry signature`
  on the first parameter `MixaRemoveNodeFn: f` — a formal of
  **function-pointer type** (`fnptr:` from the header). For `lmx_msg_blocks`
  I implemented fnptr **as a local** (`b0fd2276`); fnptr **as a formal** and
  as a passed argument is a gap to close in the compiler after §6.1 (and to
  check whether `7d7ec87c` did not already close it).
- **Addendum 18:40:** Claude closed 175200 and continued down the queue
  without a ticket: `mixa_remove_confirm` — `6c10454e`, 44/44 oracle-side,
  barrier `mixa_remove_confirm.lm2:4:31: unknown foreign type` (formal
  `@: MixaRemoveConfirmCtx ctx`, the usual shape); `mixa_selection_walk` —
  `2e628051`, all checks oracle-side, barrier
  `mixa_selection_walk.lm2:4:20: unknown foreign type` (formal
  `const: @(MixaSelWalkAllocVTable av)`). **27 of 52** ported; the path
  remove → remove_confirm → fm_remove and its walk now have a clean-L2
  source end to end. Claude moved on to `mixa_dir_win32` by himself (queue
  of ticket 175200: then `mixa_fileio_win32`, `mixa_file_manager`). After
  §6.1 re-run **27** parity runners, not 25.

### 4.9 Watchers (both armed by this chat)

| What | Script | Mutex | State/heartbeat |
| --- | --- | --- | --- |
| Fable's inbox (Mikhail writes here; Codex used to) | `work_chat/fable/scripts/watch_inbox.ps1 -RootPath 'C:/Nyasha_Planet/lingvamyxa'` | `Fable_Inbox_Watcher_Mutex` | `build/fable_watch/heartbeat.txt` |
| **Claude's outbox** (his replies to my tickets) | `work_chat/fable/scripts/watch_peer_outbox.ps1 -RootPath 'C:/Nyasha_Planet/lingvamyxa' -Peer claude` | `Fable_Peer_claude_Outbox_Mutex` | `build/fable_peer_claude_watch/heartbeat.txt` |

Both are one-shot: FSW on the directory + a 20 s quiet batch + a 30-minute
fallback + a 30 s pulse; they print one JSON and exit; **the session must
re-arm**. `watch_peer_outbox.ps1` additionally carries a **silence clock**: if
the newest ticket in Claude's inbox is older than the newest reply in his
outbox for longer than `SilentSeconds` (1800), it fires with
`reason=peer_silent` (once per ticket). Isolated selftest:
`work_chat/fable/scripts/test_watch_peer_outbox.ps1` — 15/15 (reply .tmp→.txt,
fallback, touch-only does not fire, silence, once-only, duplicate = exit 1).
The inbox watcher's selftest: `test_watch_inbox.ps1` — 31/31. Claude's own
watcher (`work_chat/claude/scripts/watch_inbox.ps1`, mutex
`Claude_Inbox_Watcher_Mutex`, heartbeat `build/claude_watch/heartbeat.txt`) is
**his** — do not touch. Arm only through the tool's background mechanism
(`run_in_background`), never a detached `&`; proof of coverage is a fresh
heartbeat (`state: waiting`, age in seconds, live pid), never a process count.

---

## 5. The integration decision — DECIDED (Mikhail, 2026-09-13 ~18:00: "as you recommend")

The problem: `main` is the "shop window" with Claude's work but with a
compiler that cannot build his ports; `codex/core-integration` is the only
branch with a working core but without Claude's latest ports and without my
path_storage. Codex, who reconciled them by hand (cherry-picks), is gone.

**Decision:** `codex/core-integration` becomes the base of `main` through one
explicit merge, after which all work (core, Claude's ports) goes to main and
branches are only work slices with a quick return to main. This chat does it,
and it is the **first step for the next model.** Procedure:

1. `git fetch`; create a working worktree `build/fable/integration` on a new
   branch `integration/main-absorbs-core` from `origin/codex/core-integration`.
2. `git merge origin/main` there. Expected conflicts: the main documents
   (take both sides by meaning — codex has the newer core entries, main the
   newer Claude entries); `mixa_help.lm2` / `mixa_composite_glyphs.lm2`
   (check with Claude which version passes his oracle parity; when in doubt
   take his, the main version, and ask him to re-run); the remaining
   `mixa_manager/*` should merge without conflict (identical content).
3. Bring over from `fable/merge-on-callable` the commits `1eb11879`
   (path_storage) and `868d85db` (notes) by cherry-pick; in `l2trans.lm1`
   take the codex side where the same thing is implemented (cast/sizeof) —
   as in my `088fce07`.
4. Run: `run_graph_abi.ps1`, `run_l2trans.ps1`, `run_lmx.ps1`, all five
   `run_port_*.ps1`, `run_mixa.ps1` and **all 25** `run_mixa_*_l2_parity.ps1`
   (they should pass the L2 side for the first time, not only the oracle).
   Record the evidence paths.
5. Only when green: `git checkout main && git merge --ff-only
   integration/main-absorbs-core`, `git push`. **Never force-push main.**
   Tell Claude in one line: the barrier is lifted, re-run the runners, from
   now on `predef:` of header types is the norm.
6. Add one "integration 13.09" entry to the model/plan/this document.

Until the merge has landed: Claude continues on main (oracle-side parity, as
now); core work goes on a branch **from `origin/codex/core-integration`**
(not from main), so as not to create a third line.

What **not** to do: do not reset others' dirty files (`git status` on main
shows Codex's uncommitted L1 diff — §4.4); do not touch the `build/codex/*`
worktrees except to read; do not "fix" old tests toward the green you need;
do not introduce new 16/64/128 limits; do not change the compiler pin
without §6.2.

---

## 6. The core work plan (order = dependencies)

Each step: what to do → how to verify → what not to do. Statuses per
`TICKET_RULES_EN.md`. Every verified stage is a separate focused commit +
push of **your own** paths and an entry in `FABLE_GRAPH_ABI.txt` (frontend) or
in this document (integration).

### 6.1 Merge the branches (§5) — first, decided
Done when main contains the core and all ports, and every runner in §8 is
green on one hash.

### 6.2 Accept Codex's L1 changes and promote the pin — decided
- In **your own** worktree (not the `stg/l1_baseline` of the main checkout,
  where the pin lives) run `stg/l1_baseline/gate.ps1` on the merged branch:
  buildCore → seed → gen1..gen3 → fixed point → every suite. Separately
  `tests/p0_tree_contract/run_p0_meta.ps1` (parser parity with 620) and the
  `tests/l1/run_*.ps1` Codex added.
- If all green and gen2 == gen3 — that is the candidate for the new stable
  L1. Promotion: copy gen2 into `stg/l1_baseline/build/l1trans/gen2/`, take
  the SHA256, replace `65D5…` in every document (`grep -rl 65D5A5ED`), tell
  Claude (he verifies the hash before every run).
- If not green — that is an L1 defect (or one in the regenerated
  bootstrap-C), and it blocks §6.3: the L2 translator is built by this L1
  too.
- Do not: do not "adjust" the p0_meta goldens toward the new parser without
  explaining the divergence from 620 (the README requires that a mutated
  golden fails).

### 6.3 Self-hosting the L2 translator
1. Take `f04cf1cd` (or the merged main) + the WIP `631abc18`; find the crash
   (`631abc18` crashes even on the 8-line `L2:` unit — start there, not with
   14,119 lines): build with `-g`, `gdb` / Dr. Memory, or a binary cut of the
   WIP hunks (653 lines in 4–5 themes).
2. Prove that "parse predef once" removes the hang of `f04cf1cd` on the big
   input (time before/after on truncated prefixes of `l2trans.lm2`: 1k, 3k,
   7k, 14k lines).
3. Run the generator:
   `powershell -File stg/l1_baseline/l2src/make_l2trans_lm2.ps1` (its count
   asserts must hold; when `l2trans.lm1` changes, change the generator's
   expectations deliberately, in the same commit).
4. The chain: `l2trans.exe l2trans.lm2 → l2trans_self.lm1` → the L1 pin → C →
   gcc → `l2trans_self.exe`; then `l2trans_self.exe l2trans.lm2 →
   l2trans_self2.lm1` and a **byte comparison** `l2trans_self.lm1` ==
   `l2trans_self2.lm1` (the L2-level fixed point); then the full
   `run_l2trans.ps1` and `run_graph_abi.ps1` with `-TranslatorPath` pointing
   at the self-build.
5. Shape this as a runner `stg/l1_baseline/l2src/run_l2trans_selfhost.ps1`
   (three stages, hashes, evidence.json) on the model of my `run_port_*.ps1`
   (see `run_port_owned_ranges.ps1`: PENDING/PASS with a pinned "pending"
   rejection, so the runner fails in both directions).
6. Every missing compiler construct — as in the ports: a minimal fixture in
   `l2src/tests/unit_*.lm2`, registration in `run_graph_abi.ps1`, a
   **tripwire** (first a `throw` in the assertion block, to prove the block
   runs), then the implementation.
Done when the fixed point is reached on the L1 pin **and** on the candidate
from §6.2, and every gate is green on the self-build.

### 6.4 The remaining runtime modules in L2 — decided as the core queue
**Superseded 2026-09-14 by decisions 15-16 (LEAD_REVIEW_20260914.md §6).**
Porting L1 runtime modules stops. The L2 runtime is built per SPEC
19.28.R2.2, 19.29.6 and 19.29.7 as Structure data, and accepted by the spec's
own 19.29.6 checks and the model's §31-§36 scenarios, never by L1 selftests.
The plan is [L2_RUNTIME_PLAN_20260914.md](L2_RUNTIME_PLAN_20260914.md). The
fifteen existing ports and their parity runners stay as translator evidence
and bootstrap, and are not extended. The text below is kept as history.

Order: `lmx_msg_mail_chain` (ticket 111000), `lmx_graph_copy_owned` (ticket
152000), then `lmx_branch_owned` / `lmx_value_owned` / `lmx_chars_owned` /
`lmx_array*` (the allocators — their bootstrap problem is squared: they ARE
the allocation), `lmx_merge_owned`, `lmx_message_graph_copy`, `lmx_msg_*`
(sched, visit, liveness, history, roots_stale), `lmx_message.lm1`; at the
very end the C files `lmx_message_exec.c` / `lmx_message_host.c` (the Message
executor and host: model part III and `LMX_MSG_EXEC_HOST_V0.txt`). For each: a
library unit (`predef:` of the header, no `main`), a parity runner against
the **unchanged** L1 oracle, two runs, a behavioural tripwire, an entry in
`FABLE_GRAPH_ABI.txt`. Timing per decision 4: after §6.1–6.2, before §6.3 or
in parallel with it if self-hosting gets stuck in a long bisection. Once
self-hosting (§6.3) is real, the load order of the first Message (§4.6) must
be solved in the compiler, not with test redirects.

### 6.5 The parser in L2
From Codex's 25 `parser_*.lm2` assemble a whole `parser.lm2` equivalent to
`l1src/parser.lm1`, with a differential runner against 620 (the same
`run_p0_meta.ps1`, but with the L2-built parser). Syntax work (`{}`, `c.*`)
only with goldens from 620 and tests in `tests/l1`. This lane is independent
of §6.3–6.4 and can run in parallel (partly by Claude, if a piece is
isolated and does not touch the translator; see §7.3).

### 6.6 Model part V, steps 5–10 — what exists, what does not
By documents and code: typed status/result/throw and merge inside a method —
exist (`08a6c1e4`, `162faac2`); Message creation through the common copier
with atomic publication — exists (`46c11da2`, `e180f719`); ownership
transfer / deliver — exists (`a787198d`, `cae59e50`); nonmoving GC over
typed roots — partial (`9673bf2e`: PRIMITIVE/METHOD roots); the D7
exec/locks inventory — not closed (Grok is closed); the end-to-end scenario
of §36 — not proven in one run. After §6.1–6.3 walk part V steps 5–10 and
have one runner with evidence per mechanism. Do not treat "there is a commit"
as "the mechanism is proven".

### 6.7 mixa_manager as the core's consumer
It neither blocks the core nor is blocked by it, except through the compiler.
After §6.1 its 25 parity runners are the first independent test of library
emission on real code. Then §7.

---

## 7. Working with Claude

### 7.1 The protocol (from `PROTOCOL.txt`, `INBOX_WATCHERS.md`)
- A ticket is the file `work_chat/claude/inbox/YYYYMMDD-HHMMSS-slug.txt`
  (local time, a Latin slug with hyphens — Codex's convention; a bare
  `HHMMSS` without a slug is what Mikhail uses for check-ins). The whole file
  is the ticket text, **in English**.
- He claims a ticket with a copy into `seen/`, and answers with a file of
  **the same basename** in `outbox/`. `seen` = taken, not = done. In the reply
  look for the status (WORKING/BLOCKED/STAGE DONE/DONE), the commit, files,
  commands, exits, counts, hashes. He always verifies the `65D5…` pin — on
  promotion (§6.2) tell him.
- **Do not leave him without a ticket**: `TICKET_RULES` calls an empty inbox
  with authorized work remaining task starvation. His idle replies
  (`…-idle-request-next-ticket`, answers to check-ins) are the signal to issue
  the next one. His turnaround per module today: 20–60 minutes.
- Waking Claude when his outbox is silent with an unanswered ticket for >30
  min: `WAKING_CLAUDE.txt` §4 — put a message in his inbox, then kill his
  watcher **precisely** by the heartbeat (`build/claude_watch/heartbeat.txt`:
  pid + start time + script path — all three must match). No broad
  `Stop-Process` by substring.

### 7.2 The ticket template (the one Claude accepts and executes)

```text
Ticket: YYYYMMDD-HHMMSS-<slug>
From: Fable (the lead chat)

Please port mixa_manager/<module>.lm1 to authored clean L2, exactly the way
you ported <previous module> (<commit>). Work only in mixa_manager on main;
do not edit the core translator, graph runtime, shared core documents, or
contact Grok.

Requirements:
- Preserve the complete existing behaviour and public ABI; use the current L2
  syntax exactly, including prefix raw load/address semantics and postfix
  field follow. Do not retain C-spelled substitutes for L2 operations.
- Add the exact L2 header/predef chain the real source needs, a focused
  L1-oracle versus generated-L2 parity runner, and meaningful normal/error
  coverage drawn from the existing accepted <module>_selftest.lm1. Use dynamic
  storage where the existing algorithm grows; introduce no semantic
  count/size/depth cap.
- Run the parity gate twice and the relevant manager regression
  (<run_*_selftest.ps1>, run_mixa.ps1). If the current main translator stops
  on the known closed foreign-type barrier, still execute and report the L1
  oracle side, record the exact line/diagnostic, and do not weaken or work
  around the type model.
- Commit and push the focused result to main. Report exact commit, files,
  commands, exits, counts and artifact hashes in your outbox. ACK/seen is not
  completion.

Grok remains closed by the user: no ticket, message, watcher change, or result
for him.
```

For non-port tasks (application stages) — the same frame, but the
requirements come from `OPENCODE_HANDOFF_20260911.md` (queues 1–5),
`FIRST_VERSION.txt`, `PORT_OF_CLEARSHELL.txt`: preserve the owner's deltas, do
not "simplify by analogy", all new application logic is L1/L2, not C.

### 7.3 Claude's queue
1. Ports in the order of §4.8 (one module per ticket; chains such as
   `remove` → `remove_confirm` → `selection_walk` may be given in sequence).
2. After §6.1 — "rerun all 25 parity runners on the integrated translator,
   report exits" (one ticket).
3. Then application stages from `OPENCODE_HANDOFF` queues 3–4 (the file
   manager: all operations and panels; process/console) — already in L2.
4. Possible isolated pieces of the core that do **not** touch the translator
   or the runtime: differential parser runners, p0_tree_contract goldens,
   runner documentation. Give them only as isolated files with exact paths
   and criteria; Claude warns about overlaps himself.

### 7.4 Boundaries Claude does not cross
He does not edit `stg/l1_baseline/l2src/l2trans.lm1`, `lmx*.lm1/.h`,
`l1src/*`, the main documents; does not rebase or force-push; creates no
branches without an explicit request; does not write C instead of L1/L2
(`CODING_RULES.txt` §0); does not touch the compiler pin; sends nothing to
Grok. If he asks about a boundary — answer in the next ticket (as in 171800),
do not leave the question hanging.

### 7.5 How to read his reply
DONE = a commit on main + files + commands + exits + counts + hashes + honest
limitations. A "barrier" with an exact line/diagnostic = an acceptable
oracle-side DONE while the main compiler is old; after §6.1 — no longer.
ACK/plan = WORKING. Verify with `git log origin/main -1 --
mixa_manager/<module>.lm2`, not by the words. Do not ask for an identical
run to be repeated without reason.

---

## 8. Tools and gates

| Command (from `C:\Nyasha_Planet\lingvamyxa` or the named worktree) | What it proves | When | Time |
| --- | --- | --- | --- |
| `stg/l1_baseline/l2src/run_graph_abi.ps1` | My graph ABI/frontend gate: selftests 63/70/261, 134 fixtures, 41 negatives, per-fixture assertions | After any edit of `l2trans.lm1`, `lmx*` | ~2 min |
| `stg/l1_baseline/l2src/run_l2trans.ps1` | The historical L2 suite (`l2trans gen2 ok`), splice drives | At an integration boundary, after frontend edits | ~5 min |
| `stg/l1_baseline/l2src/run_lmx.ps1` | Message runtime, Exec (`l2 lmx gen2 ok`) | After edits of `lmx_message*`, the runtime | ~5–10 min |
| `stg/l1_baseline/l2src/run_port_{msg_storage,msg_slots,msg_path_storage,owned_ranges,msg_blocks}.ps1` | Parity of an L2 module with its L1 oracle, twice | After edits of the module or the compiler | ~1 min each |
| `stg/l1_baseline/gate.ps1` | **The full L1 self-build** (buildCore → gen0..gen3 → suites) — overwrites the shared tool | Only when **L1 itself** changes (§6.2); never "just in case" | ~4+ min |
| `tests/p0_tree_contract/run_p0_meta.ps1` | Parser parity with the 620 reference | After edits of `parser.lm1` | ~1 min |
| `mixa_manager/run_mixa.ps1` | Full application regression (Claude) | Claude, after every port | minutes |
| `mixa_manager/run_mixa_<module>_l2_parity.ps1` | L1↔L2 parity of one application module | Claude; after §6.1 — all 25 | ~1 min each |
| `work_chat/fable/scripts/test_watch_inbox.ps1`, `test_watch_peer_outbox.ps1` | The watchers (31/31, 15/15) | When in doubt about a watcher | ~3 min |

C flags in every runner: `-std=c99 -Wall -Wextra -Wpedantic
-Werror=incompatible-pointer-types -Werror=discarded-qualifiers
-Werror=implicit-function-declaration -Werror=implicit-int`; gcc
`C:/Qt/Tools/mingw1310_64/bin/gcc.exe`. A quick translator rebuild for probes,
as in my `/tmp/qb.sh` of that session: the L1 pin → `l2trans.c` → gcc with
`-I . -I lm1/build … l2src/lmx_poll_stub.c`.

Tool traps (all of them cost time; details in this chat's memory):
- PowerShell 5.1 + `$ErrorActionPreference=Stop` + a native program's stderr
  = a crash; call through `cmd /c "… > log 2>&1"` (every runner does).
- A Bash heredoc feeding Python **eats backslashes** (`\node`, `\n`, `\x`):
  write `.lm1`-editing scripts with the Write tool into the scratchpad and
  run `python file.py`; in text use `chr(92)` / an `@B` placeholder.
- In PowerShell regexes `'\node'` is newline+`ode`; a `\r?$` written through
  a Python heredoc becomes a real CR. Check bytes with `od -c`.
- `[ -z "$stderr" ]` is not success: check the **exit code**; a silent
  `return: 1` inside the translator is a real class of defect (it was in
  `l2_emit_public_sig`).
- No `cd` in Bash calls: the session's working directory "sticks". Use
  absolute paths and `git -C`.

---

## 9. Discipline (lessons that have already cost hours)

1. **The watcher**: arm only through the tool's background mechanism, never
   `&`; the proof is a fresh heartbeat (`state: waiting`, age in seconds, live
   pid), **not** a process count (it catches other channels); read the result
   — re-arm in the same turn; every ticket read — a copy into `seen/`. In
   that session the watcher was down three times; the last time for 6 hours,
   during which two Codex tickets arrived (§4.6). After a long turn or a
   compaction, the **first** thing to do is check both heartbeats.
2. **A new check must be able to fail**: a tripwire (`throw`) in the
   assertion block, a mutation of the source — and only then trust. Nine
   fixtures once "passed" through dead code.
3. **One number, one meaning**: a value that also encodes "absent" or a
   second index space produced three defects.
4. **Do not generalize from one observation**; write "measured" only about
   what was actually run, with the command and the exit.
5. **Suspect the probe first**: grep how neighbouring code spells it before
   declaring that the language "cannot".
6. **Notes go straight into a commit and a push** (`FABLE_GRAPH_ABI.txt`,
   this file): uncommitted work vanishes — Codex is the example.
7. **A refusal is not a rule**: a restriction added for the emitter's
   convenience is a gap; label it or lift it.
8. **A library unit cannot serve the runtime that builds its graph** (the
   bootstrap problem, §4.6) — measure before writing the runner.
9. Statuses per `TICKET_RULES`: plan/ACK = WORKING; BLOCKED only with the
   exact step and the smallest needed decision; DONE — with evidence; STAGE
   DONE — continue without a reminder.
10. Never: `git reset/stash/clean` of the shared tree, force-push of main,
    replacing the pin, editing others' in-flight files, committing others'
    dirty hunks as your own.

---

## 10. Decisions taken (Mikhail, 2026-09-13 ~18:00, in the chat: "all four questions — as you recommend")

1. **Branch integration: yes.** `codex/core-integration` becomes the base of
   `main` through one merge by the procedure of §5; this chat does it. This
   is the **first step** for the next model.
2. **Stable L1: yes.** The full `gate.ps1` on the merged branch; when green —
   promotion of a new pin and replacement of the hash in every document
   (§6.2). Until then 65D5 is read-only.
3. **`codex/wip-selfhost-20260913`: archive.** It stays as is; its
   substantive part (parse predef once, external functions) is merged only
   after the crash is fixed and with its own evidence (§6.3).
4. **The two missed Codex tickets: the core queue after integration.**
   Tickets 111000 (`lmx_msg_mail_chain`) and 152000 (`lmx_graph_copy_owned`)
   are executed after §6.1–6.2, before §6.3 or in parallel with it if
   self-hosting gets stuck in a long bisection.

There is no open question left that blocks the next model. Its first action
after checking both watchers is §6.1.

---

## 11. Models and effort for long autonomous work on this plan

Advice from that session's experience (Opus 5 on the core from midnight on;
Sonnet 5 Medium on the application; Fable 5.1 on this document):

- **The lead chat (core, integration, self-hosting): Opus 5, effort high.**
  Medium was enough for module ports with a ready pattern; but §6.1–6.3 mean
  resolving conflicts in a 14,000-line translator, bisecting a crash and
  proving a fixed point — there medium starts to "generalize from one
  observation" and to fix tests toward green. Xhigh/max only pointwise: on
  the crash of §6.3.1 and on the first pass of the merge if high gets stuck.
- **Claude / mixa_manager: Sonnet 5 Medium — keep.** Ports by the template go
  evenly and honestly with him (oracle-side parity, exact barrier
  diagnostics, not a single workaround of the type model in 25 modules).
  Raising to high makes sense only on application stages with new logic
  (queues 3–4), not on ports.
- **Fable 5.1 (max)** — for documents/handoffs and for contentious points of
  the model; for long coding it is more expensive and not needed.
- Regardless of the model: put into the standing instructions a heartbeat
  check at the start of every turn and a ban on `&` launches (§9.1) — this
  cost more hours than any shortage of "intelligence".
