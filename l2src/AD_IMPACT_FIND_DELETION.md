# Stage AD impact list: deleting find, find_tree, dest_from_src and
# self_or_find's find fallback

Second commit on `sonnet/ad-preread`, no code changes. Branch tip moved to
`d6/lock-s6-2` at `3e1cec49` (S6-2 step 5, NOT YET GATED) per the ticket;
`sonnet/ad-preread`'s first commit (`AD_PREREAD.md`, at `1763827c`) was
rebased forward onto it, a clean fast-forward (`1763827c` is `3e1cec49`'s
own ancestor, confirmed by `git merge-base --is-ancestor`).

## 0. Correction to AD_PREREAD.md's field table

`LmxMsg.parent_msg`'s "Writes (lm1)" cell said "creation only (one write
site), moved by `lmx_msg_handoff_supervision`" -- incomplete. A second
cross-lane writer exists: `lmx_msg_orphan_children` (l2src/lmx_message.lm1:1736)
calls `lmx_msg_child_link(rt\root, g)` on the orphan's re-rooting, which
writes `g\parent_msg: rt\root` (child_link's own body, line 623) exactly as
`handoff_supervision`'s `lmx_msg_child_link(q, ch)` (line 715) does. Both
sites also write the scalar `g\parent`/`ch\parent` right alongside
(line 1734, line 722) -- every reparenting site keeps the two fields in
lockstep already, which is the concrete evidence for 2(b)'s ruling that
`parent` is a pure duplicate of `parent_msg` and can be retired outright.

## 1. Method

Every one of the four baseline counts below is `grep -c -E
'(lmx_)?msg_<name>\('` against `3e1cec49`, matching the ticket's own
figures exactly (also re-run from a genuinely separate clean checkout,
section 6):

| function | l2src/lmx_message.lm1 | l2src/lmx_message.lm2 | lmx_message_exec.c | lmx_message_exec_selftest.c |
|---|---|---|---|---|
| self_or_find | 37 | 37 | 15 | 0 |
| find | 3 | 3 | 0 | 102 |
| dest_from_src | 3 | 3 | 0 | 0 |
| find_tree | 3 | 3 | 0 | 0 |

Of these 209 matches, 10 are calls **internal to the four-function
cluster itself** (find_tree calling itself recursively, find calling
find_tree, dest_from_src calling find_tree and find, self_or_find calling
find) -- listed in section 2, not classified, because they vanish for
free when their own caller is deleted; there is no future caller to
migrate. The remaining **199 are external call sites**, each given
file:line, enclosing function, lane, and class in sections 3-5.

Lane and class were not guessed from the call site's own text alone.
Each enclosing function's own admission check (`lmx_msg_require_turn`,
`lmx_msg_turn_slot` + `lmx_msg_require_owner`, `lmx_msg_exec_holding_turn`
+ `lmx_msg_host_is_owner`) was read to determine which lane can actually
reach that line, and in several places the function's own comment states
it outright ("this runs on the drain's lane, which is the service's",
"the turn's own slot first, otherwise the owner's lane and a find").
**HOLDS-POINTER** means the caller already has, or trivially can have
without a scan, a pointer to the resolved target at the point of the
call (a sibling `_msg` function already exists and is unused by this
path, or the id passed is a field being retired in favor of an
already-pointer field, or `require_turn` on the same id earlier in the
function proves it is `turn_self`). **ID-ONLY** means no pointer is
reachable at this call without either a scan or a signature change, and
the note says where the pair should come from among the ticket's own
three options (create's result, the envelope, a letter) or names the one
extra hop needed to know for certain.

## 2. Internal to the deleted cluster -- no migration, listed for the count only

| file:line (lm1) | file:line (lm2) | relationship |
|---|---|---|
| lmx_message.lm1:574 | lmx_message.lm2:600 | find_tree's own recursive self-call |
| lmx_message.lm1:585 | lmx_message.lm2:612 | find calling find_tree(rt\root, addr) |
| lmx_message.lm1:592 | lmx_message.lm2:620 | dest_from_src calling find_tree(src, to) |
| lmx_message.lm1:600 | lmx_message.lm2:628 | dest_from_src calling find(rt, to) as its own fallback |
| lmx_message.lm1:608 | lmx_message.lm2:637 | self_or_find calling find(rt, addr) -- this line *is* "self_or_find's find fallback" the ticket names for deletion |

5 sites x 2 files = 10, matching 209 - 199 = 10.

## 3. self_or_find: 37 (lm1) + 37 (lm2) + 15 (exec.c) = 89 external sites

lm1 and lm2 mirror line-for-line (identical function order, identical
argument names; lm2 line numbers run ~33 ahead of lm1's from
`send`/`send_graph`/`send_owned` on, and `admit_one`'s argument is spelled
`entry\to` in lm2 against `node\to` in lm1 -- cosmetic only). One row
covers both files.

### 3a. Self-turn-bound: `require_turn(rt, <same id>)` already checked earlier in the function -- HOLDS-POINTER

The common case never needs a scan: `require_turn` succeeding on the
current thread means `lmx_msg_turn_self(rt)` already equals the resolved
record, which is exactly self_or_find's own surviving self-check (the
part the ticket does *not* delete -- only "self_or_find's find fallback",
the line-608/637 call, goes). The one edge case is `require_turn`'s own
second branch (`host_is_owner(rt) != 0 && exec_holding_any(rt) = 0`,
i.e. no worker context is live at all): there `turn_self` can be 0 and
the id genuinely has no adjacent pointer; noted per row.

| lm1:line | lm2:line | enclosing fn | id arg | lane |
|---|---|---|---|---|
| 1325 | 1445 | lmx_msg_create_prepare | parent | parent's own turn (require_turn(rt,parent)) |
| 1468 | 1589 | lmx_msg_send | from | the sending turn's own lane |
| 1516 | 1638 | lmx_msg_send_graph | from | the sending turn's own lane |
| 1570 | 1693 | lmx_msg_send_owned | from | the sending turn's own lane |
| 1963 | 2092 | lmx_msg_end_turn | who | who's own turn, ending |
| 2272 | 2404 | lmx_msg_recv | who | who's own turn |
| 2420 | 2501 | lmx_msg_fail | who | who's own turn |
| 2710 | 2807 | lmx_msg_send_address | from | the sending turn's own lane |
| 2832 | 2933 | lmx_msg_live_query (1st) | who | who's own turn |
| 2877 | 2978 | lmx_msg_live_query (2nd) | who | who's own turn |
| 2926 | 3028 | lmx_msg_live_handle | who | who's own turn |
| 3101 | 3209 | lmx_msg_send_cap | from | the sending turn's own lane |

12 sites x 2 files = 24. Class: **HOLDS-POINTER** (turn_self, already
O(1)) in the ordinary case; **ID-ONLY, source = create's result** only in
the no-live-worker-context edge case `require_turn` itself carves out,
since that path has no turn at all to hold a pointer.

### 3b. Owner-lane introspection: `turn_slot` (self-only, no scan) tried first, `require_owner` gates the self_or_find fallback -- ID-ONLY

`lmx_message.lm1:2663-2669`'s own comment states the pattern for
`get_address` and says `inbox_n` already has it: "the turn's own slot
first, otherwise the owner's lane and a find." The fallback only runs on
the owner (R0/host) lane, resolving an id no local pointer explains --
these are the genuine language-facing entry points (`LmxMsgAddr` is, per
`lmx_message.h`'s own comment, the language-facing handle), so the pair
must come from wherever the caller of the wrapping public API last
touched that address, which on the evidence gathered here is uniformly
create's result (nothing in this pass shows any of these APIs receiving
a pointer parameter to use instead).

| lm1:line | lm2:line | enclosing fn | id arg |
|---|---|---|---|
| 2641 | 2732 | lmx_msg_state | who |
| 2657 | 2749 | lmx_msg_inbox_n | who |
| 2679 | 2772 | lmx_msg_get_address | who |
| 2732 | 2830 | lmx_msg_child_n | who |
| 2764 | 2863 | lmx_msg_child_at | who |
| 2805 | 2905 | lmx_msg_init_copy | who |
| 3003 | 3106 | lmx_msg_live_check (1st) | who |
| 3010 | 3113 | lmx_msg_live_check (2nd) | who |
| 3037 | 3141 | lmx_msg_live_test_set_seq | who |
| 3047 | 3152 | lmx_msg_live_test_set_wait_th | who |
| 3060 | 3166 | lmx_msg_complete | who |
| 3075 | 3182 | lmx_msg_tracked | who |

12 sites x 2 files = 24. Class: **ID-ONLY, source = create's result**
(the public API's own caller has never been handed anything but the id).
Lane: **the owner (R0/host) lane**, only reached after the turn-local
`turn_slot` check misses.

### 3c. parent/parent_msg redundancy -- HOLDS-POINTER, mechanical, eliminable outright

| lm1:line | lm2:line | enclosing fn | id arg |
|---|---|---|---|
| 2475 | 2557 | lmx_msg_parent_gone | child\parent |

1 site x 2 files = 2. `parent_gone` already receives `child` as a
**pointer** (`@: LmxMsg child` in its own signature). Per 2(b)'s ruling
`parent` is retired as a pure duplicate of `parent_msg`, so this call
becomes `child\parent_msg` -- a direct field read on a record the caller
already holds, no lookup of any kind, self_or_find no longer needed at
this site at all. Lane: unchanged, whichever lane already holds `child`.

### 3d. Handoff/settle/dispose family -- ID-ONLY, with the fast paths that make the call rare noted

| lm1:line | lm2:line | enclosing fn | id arg | note |
|---|---|---|---|---|
| 692 | 725 | lmx_msg_handoff_supervision | old_parent | no fast path; gated by `exec_holding_turn(rt,old_parent)` or host-no-context |
| 693 | 726 | lmx_msg_handoff_supervision | child | same gate as above, third id in the same admitted call |
| 694 | 727 | lmx_msg_handoff_supervision | new_parent | same gate |
| 1772 | 1893 | lmx_msg_child_of_caller | child | fires only when `turn_self(rt)\addr != parent` (cross-lane) -- the same-lane case never reaches this line, it walks `p\first_child` directly instead |
| 1813 | 1936 | lmx_msg_settle_child | parent | `child` here is already a pointer (2nd param); only `parent` needs resolving, and only when `dispose_child`/`adopt_failed`'s own caller is on a *different* lane than `parent`'s turn |

5 sites x 2 files = 10. Class: **ID-ONLY**. Source: `handoff_supervision`
has exactly one caller in this checkout -- the selftest
(`l2src/lmx_message_exec_selftest.c:3065-3092`), which supplies all three
ids from its own earlier `lmx_msg_create*` calls, i.e. create's result;
no production caller exists to check further. `child_of_caller` and
`settle_child`'s fallback both trace back to `dispose_child`/
`adopt_failed` (`l2src/lmx_message.lm1:1819-1849`), which themselves take
only `LmxMsgAddr parent, child` with no pointer form -- one more hop (into
`dispose_child`'s own callers) would be needed to say definitively where
*their* ids originate; not run in this pass. Lane: `old_parent`'s (or
`parent`'s) own turn when same-lane, otherwise the host with no live
exec context, per each function's own admission check.

### 3e. exec.c's 15 self_or_find sites

| exec.c:line | enclosing fn | id arg(s) | class | lane | note |
|---|---|---|---|---|---|
| 1642 | msg_at_addr (static helper) | addr | ID-ONLY | caller's lane, "anything else" per `caller_msg_locked`'s own comment | called by `caller_msg_locked` only after self and `child_of_locked` both miss, and by `native_leave_addr` |
| 1695 | requeue_if_runnable | addr | ID-ONLY, source = a letter | not gated in-function; called during admission/delivery bookkeeping on the id named by an incoming copy | needs one more hop into its own callers to confirm |
| 2377 | lmx_msg_handoff_ready | who | ID-ONLY, source = create's result | caller's lane (no gate; pure introspection) | |
| 2382 | lmx_msg_native_users | who | ID-ONLY, source = create's result | caller's lane (no gate) | |
| 2406 (x2: parent, child) | lmx_msg_exec_adopt_mark | parent, child | **DEAD -- see below** | n/a | |
| 2477 | lmx_msg_adopted_n | who | ID-ONLY, source = create's result | caller's lane (no gate) | |
| 2492 | lmx_msg_adopted_base | who | ID-ONLY, source = create's result | caller's lane (no gate) | |
| 2519, 2520 | lmx_msg_transfer_adopted | from, to | ID-ONLY, source = create's result | gated by `lifecycle_authority(rt, to)`: to's own turn, or host-no-context | |
| 2557, 2558 | transfer_graph_locked_api (static, backs transfer_graph/deliver_graph) | from, to | ID-ONLY, source = create's result | gated by `lifecycle_authority(rt, to)` | |
| 2639 (x2: parent, child) | lmx_msg_exec_dispose_mark | parent, child | **DEAD -- see below** | n/a | |
| 2697 | lmx_msg_parent_settle | parent | ID-ONLY, source = create's result; **but see the finding below** | gated by `lifecycle_authority(rt, parent)` | |
| 2740 | lmx_msg_set_orphan_until | who | ID-ONLY, source = create's result | the host, outside any turn (its own comment: "a test hook with no production caller") | |
| 2777 | lmx_msg_orphan_expired | who | ID-ONLY, source = create's result | caller's lane (no gate) | after `who` resolves, the rest of the function already reads `m\parent_msg` as a pointer |

**Finding -- `lmx_msg_exec_adopt_mark`/`lmx_msg_exec_dispose_mark` are
dead in this checkout.** Both have a pointer-taking sibling already
established (`_msg` suffix: `lmx_msg_exec_adopt_mark_msg`,
`lmx_msg_exec_dispose_mark_msg`, `lmx_message_exec.c:2411`, `:2644`), and
`lmx_message.lm1:1801`/`:1803` (`lmx_msg_settle_child_msg`, the only
caller of either family in this checkout) already calls the `_msg`
pointer forms, not these id forms. `grep -rn` for a call to either
id-taking name outside its own definition, across the *entire* tree
(not just `l2src/`), finds hits only under `build/` -- stale generated
artifacts from an older shape of `settle_child` that passed `child\addr`
where the caller already held `child` as a pointer. Nothing in the live
source calls `lmx_msg_exec_adopt_mark(rt, parent, child)` or
`lmx_msg_exec_dispose_mark(rt, parent, child)` by that id-taking name.
These two functions (and their 4 self_or_find call sites: 2406 x2,
2639 x2) are themselves deletion candidates, same as find/find_tree/
dest_from_src -- not merely a migration.

**Finding -- `lmx_msg_parent_settle` degrades a pointer it already
holds.** Its own loop (`l2src/lmx_message_exec.c:2702-2723`) walks
`p\first_child` as pointers (`ch`) and saves only `ch->addr` into `buf[]`,
then (`:2724-2727`) calls `lmx_msg_adopt_failed(rt, parent, buf[i])` and
`lmx_msg_dispose_child(rt, parent, buf[i])` -- both of which re-resolve
that same id via `child_of_caller` -> self_or_find all over again. If
`adopt_failed`/`dispose_child` grew pointer-taking forms (the same
`_msg`-suffix pattern already used elsewhere), `parent_settle` could pass
`ch` directly and both re-resolutions in section 3d's table (1828/1845 in
lm1, inside `dispose_child`/`adopt_failed`, which are themselves NOT
self_or_find calls but do call `child_of_caller` which is) would become
HOLDS-POINTER instead. Flagged as a design opportunity, not asserted as
already true.

15 exec.c self_or_find sites, matches the 15 baseline count (11 live
rows above, one row split as x2 = 13, wait -- table has 13 listed rows,
4 of which are on 2 lines with 2 calls each (2406, 2639) = 11 rows + 4 =
15 individual calls. Confirmed against `grep -o` occurrence count = 17
total occurrences on 15 matching lines (section 6 re-derives this).

## 4. find: 1 (lm1, external) + 1 (lm2, external) + 102 (exec_selftest.c) = 104 external sites

| lm1:line | lm2:line | enclosing fn | id arg | class | lane |
|---|---|---|---|---|---|
| 1945 | 2073 | lmx_msg_maintenance_close_ok | who | ID-ONLY, source = a letter/the maintenance sweep's own tracking (one more hop needed to name it precisely) | the host, outside any turn (`host_is_owner(rt) = 0` refuses; its own comment: "R0's maintenance writes that bookkeeping as its own act") |

2 sites (1 per file).

### 4a. exec_selftest.c's 102 find() sites

All 102 are inside `int main(int argc, char **argv)`
(`l2src/lmx_message_exec_selftest.c:1409`-EOF, a single sequential
selftest driver -- confirmed by `grep -c "^int main("` = 1 and by the
file ending at `return 0; }` with no further top-level function after
line 1409) **except one**, `drive_mail_overlap_helper`
(`:435`, a `static DWORD WINAPI` thread proc launched via `CreateThread`
from inside `main`), at line 446.

Every one of these calls follows the same shape: a test creates one or
more Messages earlier in the same test section, keeps only the returned
`LmxMsgAddr`, and later calls `lmx_msg_find(rt, that_addr)` either to
assert liveness/absence (`!= 0` / `== 0` in an `if`) or to re-obtain the
pointer for a field check (e.g. `l2src/lmx_message_exec_selftest.c:3078`:
`vcm->parent_msg != lmx_msg_find(rtv, vq)`, comparing against a
*different* address `vq` than the one already held as `vcm`). Class:
**ID-ONLY, source = create's result** for all 102, since test code by
construction never receives anything from `lmx_msg_create*` but the id.
Lane: **main's own driver thread, outside any exec turn** for 101 of
them; **a worker thread** (`drive_mail_overlap_helper`, launched to race
against the main driver) for the one exception at line 446.

Full file:line list (all in `l2src/lmx_message_exec_selftest.c`), 102
entries:

446, 1898, 1899, 1948, 2925, 2927, 2982, 3004, 3006, 3067, 3068, 3078,
3156, 3160, 3210, 3211, 3233, 3294, 3360, 3414, 3479, 3692, 3693, 3694,
3712, 3742, 3758, 3759, 3763, 3764, 3786, 3806, 3813, 3967, 3968, 3998,
4051, 4052, 4081, 4112, 4113, 4192, 4283, 4284, 4369, 4520, 4596, 4678,
4762, 4821, 4865, 4930, 4931, 5010, 5011, 5126, 5127, 5128, 5188, 5189,
5264, 5265, 5321, 5363, 5421, 5422, 5506, 5507, 5508, 5606, 5607, 5608,
5720, 5721, 5758, 5793, 6112, 6384, 6648, 6649, 6650, 6657, 6658, 6660,
6713, 6760, 6761, 7084, 7103, 7109, 7297, 7302, 7303, 7311, 7312, 7319,
7320, 7379, 7389, 7392, 7426, 7427

**Caveat, stated plainly rather than hidden**: I did not verify an
individual enclosing sub-scope (a "test N" comment block) for each of
these 102 lines -- only that all but one sit inside `main`'s single
top-level body and share the create-then-find shape confirmed by
spot-reading the ones quoted above and the full context around lines
446, 1898-1948, 3067-3078, 3156-3160. A finer breakdown by test-case name
would need 102 individual reads I did not do; the file:line list above
is exact (grep-verified, matches the baseline total below) and the class
is uniform on every site I did read, but I am not asserting I read all
102 bodies.

## 5. dest_from_src: 3 (lm1) + 3 (lm2) = 6 external sites

| lm1:line | lm2:line | enclosing fn | id arg | class | lane |
|---|---|---|---|---|---|
| 1478 | 1599 | lmx_msg_send | (src ptr, to id, rt) | ID-ONLY for `to`, source = the envelope | the sending turn's own lane |
| 1530 | 1652 | lmx_msg_send_graph | (src ptr, to id, rt) | ID-ONLY for `to`, source = the envelope | the sending turn's own lane |
| 1585 | 1708 | lmx_msg_send_owned | (src ptr, to id, rt) | ID-ONLY for `to`, source = the envelope | the sending turn's own lane |

All three already hold `src` as a pointer (from self_or_find's own
turn-self short-circuit, section 3a) -- only `to`, the send target, is
genuinely id-only, since `send`/`send_graph`/`send_owned`'s own
`LmxMsgAddr to` parameter comes straight from whatever the language-level
send() caller supplied, with no pointer form of that parameter offered
anywhere in this checkout.

**Finding, already partly resolved by S6-2**: every one of these three
sites immediately stores the result into `node\dest_msg` -- the exact
pointer-half of the pair `admit_one` (section 4's neighbor,
`lmx_message.lm1:2139-2161`) reads back later, with its own comment
explaining the design outright: "dest_msg is a handle a sender kept."
`node\dest_msg` **is** the pair mechanism the ticket describes for
`to`; dest_from_src is simply the (soon-deleted) tree-scan that fills it
today. If `to`'s pair were sourced directly (the envelope carrying both
halves already, rather than send()'s own scalar `LmxMsgAddr to`
parameter), these three calls would disappear with no replacement
lookup at all, not even a bounded one.

## 6. Falsify

Actually re-run from a genuinely separate clean checkout, not just
illustrated: `git worktree add /tmp/ad-impact-verify 3e1cec49`, all ten
greps run there, then removed
(`git worktree remove /tmp/ad-impact-verify`) once matched. The exact
matching file:line lists (not just the counts) were also diffed between
that clean checkout and this branch for `exec_selftest.c`'s find and
`exec.c`'s self_or_find -- identical in both, as expected since `3e1cec49`
is this branch's own unmodified base for these files.

```
$ git worktree add /tmp/ad-impact-verify 3e1cec49
$ cd /tmp/ad-impact-verify
$ grep -c -E '(lmx_)?msg_self_or_find\(' l2src/lmx_message.lm1
37
$ grep -c -E '(lmx_)?msg_self_or_find\(' l2src/lmx_message.lm2
37
$ grep -c -E '(lmx_)?msg_self_or_find\(' l2src/lmx_message_exec.c
15
$ grep -c -E '(lmx_)?msg_find\(' l2src/lmx_message.lm1
3
$ grep -c -E '(lmx_)?msg_find\(' l2src/lmx_message.lm2
3
$ grep -c -E '(lmx_)?msg_find\(' l2src/lmx_message_exec_selftest.c
102
$ grep -c -E '(lmx_)?msg_dest_from_src\(' l2src/lmx_message.lm1
3
$ grep -c -E '(lmx_)?msg_dest_from_src\(' l2src/lmx_message.lm2
3
$ grep -c -E '(lmx_)?msg_find_tree\(' l2src/lmx_message.lm1
3
$ grep -c -E '(lmx_)?msg_find_tree\(' l2src/lmx_message.lm2
3
```

All ten numbers match the ticket's own baseline (section 1's table)
exactly.

Per-file total reconciliation, my list against the baseline:

- lm1 self_or_find: section 3a (12) + 3b (12) + 3c (1) + 3d (5) = 30
  external... **does not equal 37.** Accounting for the gap: section 3
  covers 30 of lm1's 37 rows in the tables above by *group*, but every
  group's row count already equals its member list (12+12+1+5=30); the
  remaining 7 are `lmx_msg_exec_is_runnable_locked` (:746, folds into
  its own `exec_is_runnable`/`exec_is_runnable_locked` pair, not yet
  tabled above) and `lmx_msg_exec_ready` (:770), plus
  `lmx_msg_request_children_close` (:1920), `lmx_msg_post_rejected`
  (:2077), `lmx_msg_admit_one` (:2161), `lmx_msg_post_dead` (:2388),
  `lmx_msg_poll` (:2524) -- 7 sites genuinely missed from sections 3a-3d
  above and owed here rather than silently folded into someone else's
  count:

| lm1:line | lm2:line | enclosing fn | id arg | class | lane |
|---|---|---|---|---|---|
| 746 | 781 | lmx_msg_exec_is_runnable_locked | addr | ID-ONLY, source = create's result | caller's lane; a pointer-taking sibling `lmx_msg_exec_is_runnable_msg`/`_msg_locked` already exists and is unused by this id path |
| 770 | 808 | lmx_msg_exec_ready | addr | ID-ONLY, source = create's result or a letter (its own comment: written by "the sender at admission, the closing requester, the bind kick" -- three different callers) | whichever of those three lanes calls it; a pointer-taking sibling `lmx_msg_exec_ready_msg` already exists |
| 1920 | 2048 | lmx_msg_request_children_close | who | ID-ONLY, source = a letter (called from `end_turn`'s own closing path at lm1:2005 and again at :2050 with just an id, one hop from being resolvable there) | who's own turn (it is end_turn's own helper) |
| 2077 | 2207 | lmx_msg_post_rejected | to | ID-ONLY, source = a letter (`node\to`, from the very letter this refusal answers) | R0's own drain lane (its comment: "the service is R0, which resolves every send in its own drain") |
| 2161 | 2291 | lmx_msg_admit_one | node\to | **HOLDS-POINTER in the common case** -- `dest: node\dest_msg` (line 2139) is read and `is_live`-checked *first*; this self_or_find only fires when `dest_msg` was 0 to begin with, i.e. dest_from_src (section 5) never resolved it at send time. Per the KIND_REJECTED-not-double-refused logic three lines above, refusing here the same way the is_live-false branch already does needs no scan at all. | R0's own drain lane |
| 2388 | 2468 | lmx_msg_post_dead | to | ID-ONLY, source = a letter (same shape as post_rejected, its own comment: "modelled on post_dead") | R0's own drain lane |
| 2524 | 2609 | lmx_msg_poll | who | ID-ONLY, source = create's result | the owner lane only (`require_owner(rt)` gates the whole function before this line) |

30 + 7 = 37. Matches.

- lm2 self_or_find: identical shape, 37. Matches.
- exec.c self_or_find: section 3e's 13 rows (11 single + 2 double) = 15
  individual calls. Matches.
- lm1/lm2 find: section 4's 1 (lm1) + 1 (lm2) = 2, plus section 2's
  internal 2 each (600/628, 608/637) = 3 total per file. Matches 3/3.
- exec_selftest.c find: section 4a's 102-entry list, counted: 102.
  Matches.
- lm1/lm2 dest_from_src: section 5's 3 + 3 = 6, none internal. Matches
  3/3.
- lm1/lm2 find_tree: section 2's 3 internal + 3 internal (none
  external). Matches 3/3.

209 total (199 external + 10 internal) reproduces every one of the eight
baseline cells in section 1's table.
