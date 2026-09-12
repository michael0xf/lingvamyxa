# Next Claude ticket: ClearShell file sharing through the platform share UI

Prepared for the user on 2026-09-11. Language of colleague correspondence: English.
Status: published to Claude inbox as `20260911-210200.txt` by the replacement
Codex task. Queued after MP3/recovery reconciliation; no claim or implementation
completion is implied by publication.
Workspace: `C:\Nyasha_Planet\lingvamyxa`.

## 1. Objective and place in the queue

Port ClearShell's existing send/share-files action to mixa_manager. Keep selection,
text preparation and button behavior from the reference, except where the current
mixa_manager requirements explicitly differ. The Windows implementation should
open the operating system's share interface with the requested content.

Take this as the next bounded feature ticket at a safe checkpoint. Do not discard
or interrupt work already in progress, the MP3 completion report, or the recovery
checkpoint in `work_chat/claude/inbox/20260911-202547.txt`. Coordinate the exact
integration files before editing shared UI/dispatch code. The complete remaining
app queue is in `work_chat/claude/OPENCODE_HANDOFF_20260911.md`; this ticket adds
the share feature and does not cancel the file-manager port.

Follow `work_chat/TICKET_RULES_EN.md`: plan/ACK is WORKING; a blocker identifies
the failed step, observed attempts and needed decision; DONE includes changes,
verification, commit/push and limitations; STAGE DONE advances to the next agreed
stage without waiting for another reminder.

## 2. Reference behavior verified by Codex

Read:

`C:\Nyasha_Planet\lingvamyxa\mixa_manager\clearshell_reference\com\mtk\shell\ClearShell.java`

The relevant methods are `email`, `email2`, and `sendEmail`, around lines
12500–12635 at this checkpoint. Locate by method name, not permanently by line.

The observed implementation:

- Gets the selected file list through `fSync().popSelectedListFroEmail()`.
- Prepares message text and a title derived from the first two nonempty words,
  with the existing fallback for an empty title.
- Supplies optional email recipients, subject and text to Android's intent.
- Uses `ACTION_SEND` or `ACTION_SEND_MULTIPLE` according to the file count.
- Supplies file content URIs via FileProvider and grants read access.
- Opens an application chooser. ClearShell itself is not an SMTP client or a
  general network file-transfer service in this action.

Inspect the selection helper and actual button dispatch as well. Do not infer
selection, directory filtering, side effects or recipient extraction from this
short summary. Preserve reference behavior that is supported; document deliberate
platform differences rather than silently presenting them as exact parity.

## 3. Architecture and API boundary

All new application logic and tests are L1; new authored headers are `.h.lm1`.
Keep platform-specific handles and WinRT/COM details out of the common interface.
Use a small platform adapter behind the existing app/Message boundaries. A native
OS callback must not concurrently mutate an application's Message graph.

Propose the smallest interface fitting the current project conventions:

- A request containing the selected files and optional text/title.
- An opaque platform/window context where the OS requires one.
- Capability/unsupported and error reporting.
- An asynchronous result/event that distinguishes request setup, handoff and
  cancellation/failure to the extent the platform actually reports them.
- Explicit ownership/lifetime of request data and platform resources until the
  OS has finished using them. Never retain a pointer into a returned activation
  or another Message's arena.

Do not freeze speculative names from this document into the language standard.
If new files are appropriate, a bounded family such as `mixa_share.h.lm1`,
`mixa_share.lm1`, `mixa_share_win32.h.lm1`, `mixa_share_win32.lm1`, a focused
`tests/mixa_share_selftest.lm1`, runner and module note is a suggested allocation,
not a statement that those files already exist. Check for existing equivalents
before adding anything.

No application-private thread pool, global mutable controller, mutex or atomic
workaround. OS-affine work stays on the proper UI execution lane; requests and
results cross Message boundaries. Until the accepted Message integration is
available, the request/selection logic and a fake-backend test can be completed
independently. Do not call that partial stage full UI integration.

## 4. Windows implementation route

Microsoft documents a desktop route through `IDataTransferManagerInterop`:
obtain the DataTransferManager for the owning HWND, register `DataRequested`,
provide a titled DataPackage with the requested content, and invoke
`ShowShareUIForWindow`. For files use the platform's storage-item representation;
handle asynchronous preparation and deferrals where required. Match the actual
UI/COM apartment and retain/release handlers and data correctly.

The application does not need to become a UWP application merely to use this
desktop integration. Check the available Windows SDK/compiler bindings before
choosing the narrow ABI implementation. Prefer L1-generated calls and minimal
foreign declarations. If the toolchain cannot express a required ABI, give Codex
a minimal reproducer and the exact missing facility; do not move application
business logic into handwritten C or silently add a C++ framework/dependency.

Primary references, checked 2026-09-11:

- [Share data, including the desktop-app note](https://learn.microsoft.com/en-us/windows/uwp/app-to-app/share-data)
- [IDataTransferManagerInterop](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-idatatransfermanagerinterop)
- [ShowShareUIForWindow](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-idatatransfermanagerinterop-showshareuiforwindow)

These references establish the API route, not successful operation on our machine;
native behavior still needs a test using a test-owned file and the actual window.

## 5. Do not copy the proposed PowerShell/system workaround

The user's attached discussion correctly identified Windows sharing, but its
sample `system("powershell ... InvokeVerb('share') ...")` is not the chosen
production implementation:

- `InvokeVerb` operates on verbs available for a particular Shell item; its
  documentation does not guarantee the literal `share` for every file/installation.
  [FolderItem.InvokeVerb](https://learn.microsoft.com/en-us/windows/win32/shell/folderitem-invokeverb)
- The sample inserts an unescaped filename into a shell program. Quotes and
  metacharacters can alter the command. Do not execute it or interpolate paths
  into shell commands.
- Its fixed buffers and narrow strings do not establish Unicode/long-path support.
- A PowerShell process exit is not proof that the share target received the data,
  much less that a remote person received a file.

Do not add execution-policy changes, spawned PowerShell processes, Explorer menu
automation, or a new address-book/network service to implement this button.
Do not promise that a particular messenger or contact will appear in the OS UI.

## 6. Honest platform behavior

"Share UI opened", "handed to the selected application" and "delivered to a
remote recipient" are different outcomes. Report only what the backend knows.
This distinction does not change the core Message success contract: success is
completion of the particular assigned operation, not an invented later outcome.

Android email-recipient/subject extras do not imply every Windows share target
honors the same fields. Preserve them in the request model only with documented
backend behavior; report a real gap to Codex if it affects the existing action.
Do not silently discard required semantics or implement an unrelated email client.

On platforms/features without a backend, return an honest unsupported result.
The application presents missing functionality using its agreed nested instance
of the same interface (one line and a dismiss button is enough), not a native
MessageBox or a console-only print. The Windows system share sheet itself is the
intended OS integration, not an app error dialog to reimplement.

## 7. Acceptance and focused tests

Build with the stable compiler read-only:

`C:\Nyasha_Planet\lingvamyxa\stg\l1_baseline\build\l1trans\gen2\l1trans.exe`

Expected SHA256 at assignment:
`65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`.
If a coordinated promotion changes it, verify the replacement manifest rather
than editing the pin to bypass a mismatch. Use unique `build/mixa/claude` outputs.

The focused module tests should establish:

1. Single and multiple selected files; optional text; no-selection behavior
   consistent with the actual reference and supported platform contract.
2. Unicode, spaces and quote/metacharacter-containing paths treated as data.
   No silent path/request truncation.
3. Missing/unreadable files, unsupported backend, invalid context and preparation
   failures produce explicit results and clean up temporary resources.
4. Fake asynchronous completion/cancellation cannot use freed request state;
   a repeated action does not leak old handlers or change a different request.
5. No invented delivery confirmation and no application-private synchronization.
6. Button dispatch uses the real selection and the existing nested failure UI.

Reuse one compatible build across test cases. Do not rebuild L1 or run the full
core gate for this module. A platform-independent fake test and a native smoke
test prove different things; label them separately.

For the native smoke test use a harmless test-owned file, open the share UI, then
cancel. Do not actually send a user's file/email or contact a person/account as
part of testing without explicit user authorization. Record untested recipient
delivery and unsupported targets as limitations.

## 8. Delivery

Implement and checkpoint the common module, then Windows adapter, then button
integration in sensible verified slices. Keep scope/file ownership coordinated;
do not wait after an ACK, and do not steal another active file to force progress.

Update the module/app documentation with the supported platform behavior and
remaining limitations. Commit and push every verified stage, exact owned paths
only, after checking the shared index. Preserve all unrelated dirty/untracked
work and recovery artifacts. No reset, stash-all, clean-all or force push.

Publish the reply under the final inbox ticket basename via `.tmp` -> `.txt` in
`work_chat/claude/outbox`. Include files, tests/log paths, commit/push status,
actual UI integration status and limitations. Continue the existing app backlog.

The replacement Codex task published English inbox ticket `20260911-210200.txt`
pointing here and preserving the unresolved MP3/recovery checkpoint before this
feature. Publication is not proof of Claude receipt, claim or implementation.
