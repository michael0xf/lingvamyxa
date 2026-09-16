# Stage AD pre-read: address field consolidation

Pre-read only, no code changes. Branch `sonnet/ad-preread`, cut from
`d6/lock-s6-2` at `1763827c` (S6-2 step 4: send by string address as the
common API's second entry). Every count below is a literal grep against
that checkout; the exact command is given beside each number so it
reproduces without guessing at file sets. Section 5 records a from-clean
re-run of every count.

No written plan document naming "stage AD" exists in this checkout
(`grep -rl "stage AD" --include=*.md .` and the same for "address field
consolidation" both return nothing outside `l2src/lmx_message.h` itself,
where the phrase "stage AD" appears once, in the `live_m`/`live_id`
field comment -- see below). This pre-read is built by reading
`l2src/lmx_message.h/.lm1/.lm2` directly and grepping for every
address-bearing site, per the ticket's own instruction to add fields
"by grep, not by the list."

## 1. Address-bearing fields

Declaration line is from `l2src/lmx_message.h` (the canonical C struct
layout both `.lm1` and `.lm2` must match). "Reads"/"writes" name the
`.lm1` functions that touch the field (verified by grepping every
`\fieldname` occurrence in `l2src/lmx_message.lm1` and reading each
site, not just counting them).

| Struct | Field | h:line | Type | Reads (lm1) | Writes (lm1) |
|---|---|---|---|---|---|
| LmxMsgEnv | reply_to | 113 | LmxMsgAddr | copied into a `LmxMsgCopy` at send | set by the caller building the envelope |
| LmxMsgCopy | reply_to | 124 | LmxMsgAddr | `lmx_msg_delivery_receive`, live-query dead-check (:2444-2455) | `lmx_msg_send`/`_graph` staging (13 sites) |
| LmxMsgCopy | from | 125 | LmxMsgAddr | `admit_one`'s live-query compare (:2957, :2985), delivery | send staging (10 sites) |
| LmxMsgCopy | to | 126 | LmxMsgAddr | `admit_one`, register/unregister letters (:2126-2155) | send staging (10 sites) |
| LmxMsgCopy | dest_msg | 134 | `struct LmxMsg *` | `admit_one`'s REGISTER/UNREGISTER handling (:2126-2155), delivery (:3174) | `lmx_msg_create`'s REGISTER letter (:1494), release's UNREGISTER letter (:1546), INGRESS staging (:1598, :2374, :2392) |
| LmxMsg | addr | 140 | LmxMsgAddr | `lmx_msg_find`/`self_or_find` and everywhere a handle is returned/compared (35 sites) | once, at creation (`lmx_msg_addr_take`, :1392) |
| LmxMsg | parent | 141 | LmxMsgAddr | root detection (`m\parent = 0U`, :1652/:2510), family walks (:2440/:2489/:2840/:2861/:2897/:2911), live-query compare (:2957/:2985) | creation (:713/:1393/:1395), R0's own root record (:1750) |
| LmxMsg | service | 151 | `struct LmxMsg *` | `lmx_msg_service_of` and every register/unregister call site | creation (delegates to parent's service), R0's own self-assignment |
| LmxMsg | live_m | 172 | `struct LmxMsg **` | the live-set binary search (`lmx_msg_service_is_live`) | grow/shift in `lmx_msg_service_register`/`_unregister`, both on the service's own lane only |
| LmxMsg | live_id | 173 | LmxMsgAddr\* | same binary search, paired with `live_m[i]` | same register/unregister sites, always alongside `live_m` |
| LmxMsg | index | 182 | unsigned | `lmx_msg_get_address`'s own parent walk | creation only (one write site) |
| LmxMsg | exec_from | 198 | LmxMsgAddr | post-turn dead-notification (:2955/:2983) | snapshotted from `LmxMsgCopy.from` at materialization (:2281/:2313) |
| LmxMsg | exec_reply | 199 | LmxMsgAddr | dead-check guard (:2444/:2455) | snapshotted from `LmxMsgCopy.reply_to` at materialization (:2282/:2314) |
| LmxMsg | parent_msg | 207 | `struct LmxMsg *` | family/tree walks, direct-child list maintenance | creation only (one write site), moved by `lmx_msg_handoff_supervision` |

Not included as a field row, but load-bearing for the whole design and
directly named in `lmx_message.h`'s own comment at `LmxMsg.service`
(h:142-150): `LmxMsgRuntime.root_record` is cited there as "the
precedent for an LMX field" -- the model stage AD is moving `service`,
and eventually the rest of this table, toward.

`first_child`/`last_child`/`next_sibling` (h:208-210) are pointers too,
but they are the sibling-list plumbing `parent_msg` requires, not
themselves an addressing mechanism -- listed here for completeness, not
given their own row.

## 2. LmxMsgAddr signature-site counts

Every number below is `grep -c "LmxMsgAddr" <path>` (or the summed
`grep -rc` shown), run at `1763827c`.

- **Declarations, `l2src/lmx_message.h`**: `grep -c "LmxMsgAddr" l2src/lmx_message.h` -> **69**
  (this mixes struct-field types, the `typedef`, and function-prototype
  parameter/return types -- the .h does not separate them syntactically).
- **Definitions, `l2src/lmx_message.lm1`**: `grep -c "LmxMsgAddr" l2src/lmx_message.lm1` -> **67**
- **Definitions, `l2src/lmx_message.lm2`**: `grep -c "LmxMsgAddr" l2src/lmx_message.lm2` -> **60**
- **Call sites, selftests and runners** (`l2src/tests/*`, `l2src/*_selftest.c`,
  `l2src/lmx_message_exec.c`, `l2src/lmx_message_exec.h`,
  `mixa_manager/tests/mixa_ingress_host_harness.c` -- the full file-by-file
  breakdown is in section 5):
  `grep -rc "LmxMsgAddr" l2src/tests/ l2src/*_selftest.c l2src/lmx_message_exec.c l2src/lmx_message_exec.h mixa_manager/tests/mixa_ingress_host_harness.c | awk -F: '{sum+=$2} END{print sum}'` -> **357**

None of these four numbers, nor their sum (553), nor any subtotal tried
(with/without `l2src/l2trans.lm1`, the `stg/l1_baseline` retired build
tree, or `mixa_manager/vendor/lmx_msg_host_ingress_v0`'s own pinned
copy) equals 436 -- checked directly, not assumed:

- All matching files except `build/`: **864**
- Same, also excluding `stg/` (the retired second build root): **603**
- Same, also excluding the vendor copy: **559**

436 does not match any grouping tried here. Reported as a genuine
miss, not forced to fit -- if the plan's own 436 was counting something
narrower (a single function's own call sites, say, rather than every
`LmxMsgAddr` token), that unit is not identifiable from this checkout
alone.

## 3. Fate per field

Citations are unique phrases from `Lingvamyxa_spec.txt`, section 19.29.7
(section 2, "Value Ontology", was checked directly -- lines 1035-2112 --
and its own "address" language is a different domain entirely: typed
memory-block/range classification for L2 own-fields, not Message
routing or capability. Nothing there decides a Message-addressing
field's fate, so every citation below is 19.29.7). Commit is the one
that introduced the field's own current shape and rationale, found by
`git log -S` on the field name in `l2src/lmx_message.h`.

Every commit below is verified by `git log --oneline --follow
-S"<exact field text>" --fixed-strings -- l2src/lmx_message.h`, taking
the OLDEST (last-listed) result as the introducing commit -- re-run at
`1763827c` in the clean worktree, not assumed from memory (an earlier
pass of this same search, without `--fixed-strings`, mis-attributed two
of these; corrected here after re-checking).

- **`LmxMsg.addr`** -- KEPT. Spec: "The internal mailbox slot
  (`LmxMsgAddr`) is not the Mix path" (19.29.7, the `create`/`spawn`
  language-facing operation). This is the sentence that separates the
  internal handle from the hierarchical address `index` composes --
  both are kept, for different jobs. Commit: `93d74ea1` ("L2: copy-only
  Message mailbox with death notify") -- long-predates S6-2, untouched
  by it.
- **`LmxMsg.index`** -- KEPT. Spec: "parent `34.3` yields `34.3.1`, then
  `34.3.2`... First allocated child is 1" (19.29.7) -- `index` is this
  per-level path segment; `lmx_msg_get_address` composes the full
  address by walking parents rather than storing it, per `lmx_message.h`'s
  own comment at that field. Commit: `2b0ddad7` ("S6-2 step 2... the
  mail service reference, the live set, and the common mail API's first
  bodies") -- the field is new at S6-2, added alongside `service`.
- **`LmxMsg.live_m` / `LmxMsg.live_id`** -- KEPT, explicitly, by the
  field's own comment: "stage AD keeps it for exactly this reason" (the
  id-pairing that survives address reuse). Spec: "no shared process-wide
  Message registry or management lock" (19.29.7, the language-facing
  operations preamble) -- this pair is what replaces the deleted
  registry per-service rather than process-wide. Commit: `4ffacf05`
  ("S6-2 step 3... the live set's free-partner, and the getAddress
  wrappers") -- one step after `index`/`service`/the `live_n`/`live_cap`
  counters (`2b0ddad7`).
- **`LmxMsg.service`** -- KEPT for now, marked scaffolding for later
  removal by its own comment: "the same status as the other scaffolding
  fields the plan lists, and it goes when the record is reduced." Spec:
  "the kernel record is running, success, handoff_safe, root, index"
  (19.29.7-adjacent field comment, quoting Mikhail 2026-09-16) names
  what stays when it goes. Not itself an AD deletion -- a later-stage
  one. Commit: `2b0ddad7`.
- **`LmxMsgCopy.dest_msg`** -- KEPT, core to the REGISTER/UNREGISTER
  letter mechanism (the field's own comment, h:52-73). Spec: "a held
  capability" (19.29.7's own "no shared process-wide Message registry
  or management lock; ... a held capability") is the model this letter
  mechanism implements per-service instead. The field's own NAME is
  older -- `git log --follow -S"dest_msg"` finds `0533351b` ("L2:
  parent-owned sched_step and map_child; growable path") as the true
  origin, reached only with `--follow` (a plain, unfollowed `git log -S`
  stops at the `e68fb874` "ONE ROOT" file-move and wrongly looks like
  the origin) -- but its CURRENT shape and rationale, the register/
  unregister letter use this pre-read cites, is `4ffacf05`'s.
- **`LmxMsgCopy.from`/`.to`/`.reply_to`, `LmxMsg.parent`,
  `LmxMsg.parent_msg`, `LmxMsg.exec_from`/`.exec_reply`** -- fate not
  determined here. `parent` (an address, 17 active read/write sites,
  none of them dead code by inspection) and `parent_msg` (a pointer,
  used for the direct-child list 19.28.R2.2/19.29.6 both require) are
  NOT redundant on the evidence gathered -- they answer different
  questions (compare against an incoming envelope's own address-typed
  field vs. walk the tree directly) -- but confirming that neither is a
  fold/delete candidate needs reading every call site in
  `lmx_message.lm2` too (not done here) and the executor's own use of
  `exec_from`/`exec_reply`, which this pre-read did not reach. Reporting
  this honestly rather than asserting a fate the evidence does not yet
  support.

## 4. Open question

Given `LmxMsg.addr` (the internal mailbox slot) and `LmxMsg.index`
(the Mix-path segment) are both spec-confirmed KEPT for different jobs,
and `parent`/`parent_msg` look like the same kept-for-different-jobs
shape on the evidence above -- is "address field consolidation" about
finding and removing genuine duplicates (of which none turned up in
this pass), or about documenting/enforcing the KEPT-BOTH relationships
this file's own comments already describe piecemeal, so a future reader
does not have to re-derive it the way this pre-read just did?

## 5. Falsify: every count re-run from a clean checkout of 1763827c

```
$ git worktree add /tmp/ad-preread-verify 1763827c
$ cd /tmp/ad-preread-verify
$ grep -c "LmxMsgAddr" l2src/lmx_message.h
69
$ grep -c "LmxMsgAddr" l2src/lmx_message.lm1
67
$ grep -c "LmxMsgAddr" l2src/lmx_message.lm2
60
$ grep -rc "LmxMsgAddr" l2src/tests/ l2src/*_selftest.c l2src/lmx_message_exec.c l2src/lmx_message_exec.h mixa_manager/tests/mixa_ingress_host_harness.c | awk -F: '{sum+=$2} END{print sum}'
357
```

Per-file breakdown of the 357 (selftests + runners), each its own
`grep -c`, for anyone re-checking a single file rather than the sum:
`lmx_message_exec_selftest.c` 169, `lmx_message_exec.c` 70,
`lmx_message_exec.h` 19, `tests/cancel_spin_host.c` 17,
`tests/lmx_model_liveness_33_selftest.lm1` 10,
`tests/lmx_model_family_release_17_selftest.lm1` 8,
`tests/lmx_entry_turn_selftest.lm1` 6,
`tests/lmx_model_turn_arena_o1_selftest.lm1` 5,
`tests/lmx_model_scenario36_selftest.lm1` 5,
`tests/lmx_model_root_record_5e_selftest.lm1` 5,
`tests/lmx_model_orphan_mapped_17_selftest.lm1` 5,
`tests/lmx_model_family_close_32_selftest.lm1` 5,
`tests/lmx_msg_family_handoff_selftest.lm1` 4,
`lmx_message_host_selftest.c` 4,
`mixa_manager/tests/mixa_ingress_host_harness.c` 3,
`tests/unit_msg_cursor.lm2` 3, `tests/lmx_model_root_ingress_5b_selftest.lm1` 3,
`tests/lmx_model_checks_19_29_6_selftest.lm1` 3,
`tests/lmx_msg_send_local_selftest.lm1` 2,
`tests/lmx_generated_for_arrays_collect.c` 2,
`tests/lmx_generated_array_index_collect.c` 2,
`tests/lmx_generated_array_char_index_collect.c` 2,
`tests/l2_message_root_driver.lm1` 2, `tests/lmx_msg_adopt_unrooted_selftest.lm1` 1,
`tests/l2_index_store_reassigned_formal.lm2` 1,
`tests/l2_c_scanners_parse_driver.lm1` 1. Sum: 357, matching the
`awk` total above.
