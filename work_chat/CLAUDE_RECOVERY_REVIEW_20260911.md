# Claude recovery and watcher checkpoint

Reviewed 2026-09-11 21:33 America/Sao_Paulo, read-only against the shared
checkout. Reply `work_chat/claude/outbox/20260911-201500.txt`, SHA256
`577D1A108A0E7721C1D141A635D8F927AC0BFAB0A1BCF0BDFCDA97A3A4C96BC6`, matched
the original request and seen marker. This is not app recovery acceptance.

Claude explicitly admitted `git reset --hard 480e4f1` at 20:12:42 -03:00 without
checking dirty files or making a backup. Reflog independently shows that reset
after fc11d00, then the 20:14:42 fast-forward to 0ce7957. Both 1c3d62b and
0ce7957 are current HEAD ancestors (verified exit zero). That recovers committed
core history, not the vanished uncommitted app changes. Claude reports no local
backup; recovery202547 already specifies eight exact destination paths and
read-only reference Git commit a68d2b735bae524b77db4e1b72731776c6657860. The
reference working tree is not the preserved source. Recovery remains pending.

Claude withdrew the unsupported assertion that +/-1000ms means track skip.
He reports existing next/prev seek and separate next_file/prev_file operations,
but explicitly has not inspected the ClearShell reference for this question.
No new source-parity claim or semantic change is accepted from that statement.

Watcher commit f715714 is present in HEAD. The reviewed script SHA256 is
`D449D30C70EF03F06452DD7BAC03DE654BBCBE58976063547B1826C92B7F9435`.
Source inspection confirms event-queue polling without Action-scope sharing,
UTC deadline parsing with RoundtripKind, persisted hash state, Error-event
reconciliation and cleanup of exact run-owned subscriptions in finally.
The commit describes isolated tests but gives no durable test-root/log paths;
those empirical claims have not been independently accepted.

Production rearm is not established. At review, heartbeat recorded PID6340,
start00:30:19Z, state=fired00:30:40Z, fallback due00:39:36Z, timings20/1800/30.
The log had no later start. Win32_Process enumeration found no powershell/pwsh
process running watch_inbox.ps1. A finished one-shot helper cannot watch again
until Claude rearms its native background completion bridge.

Two source-level delivery gaps remain: seen+outbox excludes changed requests
from startup hash detection and treats any response as completed; live events
trigger settled reports without comparing hashes, so touching mtime still wakes.
These are distinct from substantive ACK/WORKING/DONE acceptance by the agent.

One actionable follow-up, inbox213500, supersedes the outstanding211430 review.
It requests rearm evidence, precise isolated evidence paths and bounded fixes,
while preserving MP3 closeout, eight-file recovery202547, queued Share210200
and the original app backlog. Do not send duplicate probes for this same issue.
No Claude watcher process or app source was changed by this review.

Codex's independent outbox watcher was healthy (PID21772, pulse00:33:04Z) and
delivered this reply to replacement task01a092e9-eb6e-7d61-8880-c0f88197317a.
That proves Codex delivery, not Claude rearm or application completion.

## Recovery accepted at 21:42

New reply202547 SHA C6B75862FC982D81237332BC428FE9F424BC217E4269E902D89F9D861BDE4A90
matched the recovery ticket and commit2396063. All seven non-STATUS Git blobs
match the immutable reference exactly. STATUS adds a recovery entry, so the
reply's blanket eight-file byte-identical claim is imprecise. The GDI return-
width checks, per-instance injection and KEEP_BG code/test changes are restored.
STATUS still includes stale uncommitted/next-step wording; requested reconciliation.

Both existing draw/backend selftest binaries executed successfully without any
rebuild. Evidence with source/generated-C/executable/compiler hashes frozen
before/after execution is in
build/codex/claude_recovery_review/20260911_214231_373/evidence.json.
This verifies restored source content and execution of saved binaries; it is
not a fresh source build or full App/runner acceptance. The ingress expected-
failure runner issue is documented and queued as a bounded app-runner fix.

Claude explicitly confirms MP3 is only3ea8c95, its five183400 fixes remain
unapplied and it is not wired into App buttons. Share210200 is now his next
active stage. Inbox214300 records recovery acceptance and those limits without
duplicating the outstanding watcher213500 rearm request.

## Follow-up acceptance at 22:04

213500 reply SHA74E3D3930A709FB06EE477B95BDC16B32EA6B99D20B3F98028B58853D89A4349:
143220c diff fixes both live-event dedup and hash coverage of completed inbox
requests. PID22308 independently exists running watch_inbox.ps1, with waiting
pulse01:03:58Z and future fallback01:23:33Z. This closes the rearm probe at this
sample. Exact isolated test paths are still absent; quoted logs are not saved
test artifacts. No watcher restart or app source edit performed by Codex.

Share210200 reply SHAB15CF1E2FCD0A666D9794BD62120A3CD739EA3122083B5DE7740B1011A97810F:
c4e76a0 fake-backed stage verified from saved run215154_497_300b8a42/logs:
SUCCESS, translate/compile/test exits0,37checks0failures; six source/compiler
hashes match. Not native Windows adapter, COM lifecycle or App integration.
Source review found terminal-begin contract mismatch and portable header's
transitive windows.h dependency; header also misattributes 32 UTF-8 bytes to
the Java reference. Ticket220700 queues fixes at next Share checkpoint while
preserving native feasibility work and open MP3 review183400.

214300 reply SHAFB1B2C7D3C01DA046F3D20E5C183C12D976A6EE3885A58C81965CBD9AF5B12CA:
ccbf351 adds the previously untracked ingress runner. Explicit native exit
handling, expected diagnostic assertion and final exit0 are source-reviewed.
Saved normal/failure logs and owner-local fail_keep_runtime markers present.
Full run_mixa successful outer exit/source manifest was not supplied; requested
exact existing evidence or capture on next justified run, no duplicate rebuild.
215056 SHA050D7594DA74A2DB0FE4B62980C2787D6A303CD5F4A67B782B41EF1FBFE2C6AD
is matching progress commentary, not additional implementation evidence.

## Follow-up at 22:12

220700 reply SHAC7BEF50678884F35907DD18AFC7C0E5454D221B308AAD7640810CE6AC4067A09
matched ticket. dd4f078 now permits begin only from NONE, tests all terminal
events and fixes fake cancellation polling. Windows include removed from the
stub; UTF-8/UTF-16 fallback difference is explicitly documented. Saved Share
run220849_982_46b35838/logs has SUCCESS,45checks0failures, three exits0 and
six matching source/compiler hashes. Accepted bounded seam update.

Ingress run221018 under build/mixa/claude/ingress_fix_evidence contains full
outer stdout/stderr and exit0, manifest's two runner hashes match, stable pin
matches. Log contains normal ingress success, expected quiescence diagnostic,
fail_keep_runtime success and remaining app-gate results. Wrapper evidence gap
closed for this run; not full native Share or MP3 feature acceptance.

Ticket221400 preserves the native implementation step and notes that adding
COM includes to the transitively predef'd win32 header would restore the leak;
native include boundary must be separated as the adapter is implemented.
No duplicate run or watcher probe requested, no app source edits by Codex.

221400 updated reply SHAC79C789021AA48D9E119C15C06A16E89AEDACE3A049A3AD23897186D01DA8681
supersedes F487CD4765D44EF0DCEDBDA7A5F15A46E47039788ECB23C015C395DA51A353B8.
e7027cf corrects the future-header-boundary comments;67aeff1 documents a
scratch pointer-declaration/header translation+compile/run probe with SDK
-idirafter. This is a reported feasibility result, not implementation or
verified COM calls/event-handler lifetime; exact scratch source/log hashes
were not provided. Current native adapter implementation remains assigned.
222056 SHA59EFA1EABCA7D18D071D9E9A3CEEB4E49A371C0EF1E30C146E2553EAD4A7275C
is progress commentary only and slightly overstates pointer-probe evidence
as successful Share API calls. Keep full native cancel-only smoke and actual
selection/button integration as unfinished acceptance stages. No new probe.

## Native Share checkpoint at22:55

fd1c634 pushed;225056 reply SHA
B046000B81759F46A825CAE2890784EA14930F47DE1D35392B37E8F3F23AFBC0
matches progress request. Saved native smoke
build/mixa/claude/share_native_smoke/run_20260911_224222_363_2e99d4ec/logs
has SUCCESS, translate/compile/test exits0, six matching source/compiler hashes,
real RoInitialize/GetForWindow/DataRequested after16iterations/~800ms. This
establishes callback delivery; poll's success does not prove all setters worked.

Source review identifies four concrete defects queued in225400: QueryInterface
accepts arbitrary IID; acquired COM interfaces never released; token allocation
after successful registration leaves no rollback token on OOM; callback errors
and ignored SetText/put_Title HRESULTs become HANDED_OFF/OK. Claude owns fixes,
failure/lifetime regression and file attachment, then actual selection/button
wiring. No duplicate real-window run requested. Native backend acceptance stays
partial until these defects are fixed. MP3 five183400 items/backlog still open.

## Follow-up at23:04

225400 reply SHA74CBE92A9BE99A8F0C2EC4ADEBD8119F4AD1849BAA81C537FEF1F2EAD19939CD
delivers0002c13 and2593b4b. Native smoke230029_405_0ff95aa3 has SUCCESS,
three exits0,six hashes match, bogus/IUnknown QI checks and real callback600ms.
Source fixes IID filtering, token-before-register, callback status and most
COM releases. Still leaks local interop on earlier GetForWindow/handler-calloc/
vtbl-calloc failures; no injected native API/OOM reference-balance evidence.
Ticket230500 identifies these exact omissions, asks existing requested failure
cases and STA integration contract while preserving file-attachment work.

Audio2593b4b removes global alias counter and adds close after failed MCI set
time format. New alias masks pointer to15bits, so distinct live contexts may
collide;230500 asks full-width identity and a same-low-bits regression. Audio
runner is explicitly fake-backend/controller evidence, not Windows MCI proof.
Remaining183400 items include list/clear device reconciliation, arbitrary64cap,
lazy-vtable/reference behavior and UI integration. Latest observed audio run
230310_756_132ed6bd exits-1073741819 during advancing uncommitted controller/
header work; not accepted as green and not yet an unexplained monitoring stall.

## Follow-up at23:13

230500 reply SHA62AADF061B3FA25E61026C0965682A36071FD35474BE88B72AB3872B0AF53E55
matches request.9043707 source fixes all three identified interop-return leaks.
Share smoke231009_831_522ffc15 SUCCESS/native0; five current hashes match,
portable-header hash now differs during active fault-hook edits. Do not reuse
that run as acceptance of current uncommitted fault coverage. Claude explicitly
keeps begin/API fault injection and STA enforcement open despite broad opening
sentence. No additional duplicate request needed; he is implementing coverage.

6724950 source widens alias to size_t/full-width formatting and removes64cap
with overflow check. Native test adds same-low15bits identities1/0x8001; fake
test now supplies a real65-entry source. This explains and fixes the earlier
test crash after removing the cap. Saved audio_seam230819_799_dd1f272f has
68checks/native0; audio_native_seam230915_640_680ce629 has24checks/native0.
Each runner's five saved header/test/compiler/runner hashes match. Those
manifests omit implementation-source hashes, so source diffs and bounded
evidence are recorded separately; MCI set-time-format failure remains unforced.
Remaining playback list/clear reconciliation, lazy-vtable/reference review,
button integration, Share file attachments and native failure/STA cases stay
open. Advancing edits confirm substantive work; no stall/probe/restart.

## Follow-up at23:45

232056 reply SHA707F98C76BB6E7EBC7B7BC73D8843F2FF3BBE45F8A8826356400F83C023EBFEA
matches the progress request; cc3f10c pushed. Latest native smoke
233955_396_4173eb9a SUCCESS/native0 and six matching hashes. Four injected
begin/get_Data/SetText/put_Title failures propagate FAILED/non-OK; subsequent
clean request succeeds without retries. Three earlier green logs differ in
implementation hash (233702 also test source), so four identical-revision
passes are not established. Manifest still omits portable implementation.

Concrete thread-contract contradiction: header claims STA-only plain refcount,
but smoke uses RoInitialize(1U). Installed SDK10.0.26100.0/winrt/roapi.h lines
38/40 define SINGLETHREADED=0 and MULTITHREADED=1. Ticket234500 asks supported
apartment/owner-thread enforcement and focused negative coverage before UI
integration, then same-live-window sequential requests. Fresh HWND per request
does not cover app window reuse. Callback non-delivery cause remains unresolved;
successful status when invoked cannot establish adapter correctness when not
invoked. Requested correction of categorical OS-broker blame in docs/diagnostics.
Later successful request is not proof of zero leaked COM refs. No colleague
rebuild or duplicate probe. File attachment/deferrals and original audio/UI
backlog remain assigned to Claude after the thread-contract checkpoint.

## Follow-up at00:12 on September12

234500 reply SHA3FEA4C7FA12CC924308DF6D22B9A671C6B379429BE95B980BBAB2A4AFB48684D
delivers7aa7a7f. Symbolic STA initialization fixed; latest native run
20260912_000408_608_0a3b71e5 matches six saved hashes but FAILS with exit4.
SET_TEXT/PUT_TITLE/clean requests remain PENDING, same-window diagnostics also
pending. Cause unknown; prior MTA green runs do not validate this STA revision.

Thread enforcement not accepted as complete. Source proves a callback UAF path:
backend wrong-thread destroy returns, leaving ctx/handler/registration alive;
portable mixa_share_destroy then unconditionally frees r/text/title/files while
ctx.req still points to r. Refusal must precede portable cleanup and preserve
the request for correct-thread teardown, tested via the public API from a real
second thread. Recording owner_tid at begin is not actual apartment validation;
an MTA caller would pass later same-thread checks. Invoke's mismatch branch also
writes shared status/pending off-owner, contradicting claimed thread confinement.
Ticket001200 queues precise fixes and decisive hang diagnosis before adding
file deferrals; original files/audio/UI backlog preserved. No native rerun by
Codex, no hypothesis promoted to root cause, no acceptance from a committed
checkpoint alone. Claude owns all implementation and negative lifecycle tests.

## Follow-up at00:49 on September12

001200 reply SHA4E24B4FBC7D8F300EB3017D43F128436412C572FC9F661040B59F9DC09227648
delivers3f49ac7/1dbce11. Portable destroy now checks backend status before
freeing request/payload; begin checks real CoGetApartmentType; Invoke mismatch
does not mutate status/pending. Source corrections supported. Latest native
run003727_024_bfa2f4d0 matches all6logged hashes and generated C includes the
portable early-return guard. Overall exit4 remains, new regression lines pass.
Cited001943/002243 concrete header hashes differ from current; portable
mixa_share.lm1, containing the critical fix, is absent from the hash manifest.

Native regression still needs decisive proof: CreateThread failure can count
as successful refusal; WaitForSingleObject result is ignored, so owner cleanup
can run without a joined worker; window setup failure can silently skip. A
PENDING enum read alone does not prove no free occurred (freed bytes can remain
unchanged).004800 requests actual request/ctx/payload free observation, zero
frees on wrong-thread PUBLIC destroy, exactly-once owner teardown, checked
setup/join/real MTA initialization, narrow test mode and portable source hash.

Instrumented003313_f88c6c1b logs successful GetForWindow/add/Show/remove HRESULTs
with its distinct implementation hash. This does not establish an OS/broker
root cause or that app-level diagnosis is exhausted. Ticket004800 asks one
fresh-process clean-first vs failure-first comparison with exact lifetime/
window/pump state and reproducible instrumentation; no identical smoke loop.
File deferrals remain behind this unresolved Share lifecycle. If diagnosis
has an exact external blocker, continue the independent authorized audio
set_list/clear reconciliation. Full app backlog remains open and owned by Claude.

## Follow-up at01:11 on September12

004800 reply SHA EBBA60B3C2DF8B9FAA67CD313C54F8E2DEE57FE54580CC6E5F545F51A300302B
(event3673f8bc8fc244e192b5f94517e4fa03) delivers pushed84e2f00 Share and
50e562c audio. Source adds foreground activation in adapter/test paths;
portable cleanup-entry counter, checked worker join and real MTA result;
audio unloads before replacing/clearing the old playlist, retaining it on
unload failure. Generated saved C contains those changed paths.

Native010334_022_ab21fcf7 exits0, all faults, public destroy/owner teardown,
MTA and two same-window calls pass without retries. Six of seven logged
hashes match at review; test_source differs. Earlier cited005853/010115/
010209 all exit0 without retries but implementation AND test_source differ.
Thus three identical-current-revision passes are not established. Portable
implementation is now correctly included in the manifest. No colleague
checkout rebuilt. Audio saved004550_556_e15e982f reports76checks/0failures,
exit0; generated C contains both unload paths, but its implementation is
still omitted from the runner hash manifest. No native-MCI proof claimed.

Baseline005741_674_3e3ee602 logs five foreground-matched requests with
expected events, but both scenarios execute in ONE process and main returns0
without asserting those results. Useful observation, not a strict test or
two independent fresh-process scenarios. Foreground change is a supported
local diagnosis, not an established universal OS behavior guarantee.

011300 sends bounded remaining corrections then activates the existing
StorageFile/deferral stage without an ACK wait: both regression window-setup
failures silently skip; both still destroy HWND after unjoined timeout;
CreateThread failure leaks the registered request; fc!=0 is followed by
another destroy on a possibly freed pointer. Counter observes portable
cleanup entry, not actual ctx/handler/token/individual-payload frees.
Add meaningful public poll/cancel refusal coverage at this affected stage,
preserve exact source/evidence, make diagnostic mode fail on wrong results.
Then files, cancellation/async lifetimes and native test-owned files, followed
by selection/button/nested failure UI. No actual recipient send. Audio
lazy-vtable/reference review, native failure coverage and unload-failure
state-preservation checks remain assigned. App files remain Claude-owned.
