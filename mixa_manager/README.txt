Mixa Manager — ClearShell desktop port in Lingvamyxa
===================================================

Purpose
-------
Port ClearShell's functionality and appearance, applying the owner's recorded
changes to individual UI elements and to desktop execution. This is not a new
file-manager design inspired by ClearShell. The changes are substantial even
where they preserve the source's user-visible intent; do not dismiss them as
cosmetic, or copy Android widgets and Java shared-heap internals literally.

The checked-in, read-only reference is clearshell_reference/. In particular,
the original path/list navigation and recursive selection-with-exceptions model
must not be replaced by a conventional '..' row or a flat selected-path set.
Claude's investigation and the tests that vindicated the consolidation loop are
recorded in PORT_OF_CLEARSHELL.txt and SELECTION_MODEL.txt. They are evidence for
preserving that algorithm, not proof that every possible file operation is bug-free.

Reading order and authority
---------------------------
1. PORT_OF_CLEARSHELL.txt: port contract, agreed deltas, reference-source map.
2. The subject document below: current decisions for that element or interface.
3. STATUS.txt: dated implementation evidence, active owners and next work.

Latest explicit owner decisions override older notes. If a recorded delta is
silent, consult ClearShell for its functionality/behaviour; FAR is the keyboard
reference, not a replacement UI. Old_Mixa_Manager_Qt_model.txt preserves useful
visual requirements only where not superseded; its Qt/PTY implementation plan
is not the current architecture. Ask about a genuine conflict before changing
behaviour. An unfamiliar algorithm is not itself a defect.

The current mixa_manager specification takes priority over the ClearShell
reference: it describes the intended differences. Reference code supplies only
unchanged/unspecified behaviour, never grounds for undoing a documented delta.

    UI_MODEL.txt          layers, grids, lists, panels, focus and pointer
    DRAWING.txt           cells, glyphs, tiles and double-line frames
    BACKEND_SEAM.txt      current platform dispatch/glyph/input contract
    FILE_SEAM.txt         file-backed console and filesystem boundary
    PROCESS_SEAM.txt      OS commands, job lifetime and Message integration
    SELECTION_MODEL.txt   original recursive selection and required tests
    FIRST_VERSION.txt     first runnable milestone: mode 0.1.2
    CODING_RULES.txt      L1 spelling, qualification, tests and ownership rules
    PORT_OF_CLEARSHELL.txt deltas and source evidence; not a second status log
    STATUS.txt            current checkpoint and remaining implementation

A decision belongs in its subject document; other documents link to it.
Do not append a new current rule below a contradictory old current rule.
Keep useful examples and explanations; superseded work assignments belong in
Git history, not in startup instructions. Record new decisions promptly
and commit/push documentation separately from implementation.

Architecture and first runnable scope
------------------------------------
Develop independent modules against ClearShell now; only their concurrent
Message wiring waits for the verified core runtime. PORT_OF_CLEARSHELL.txt 1.1
defines this workflow and records the owner-confirmed reference location.
Explicit state ownership and copyable/transferable inputs/results are designed
up front. A module is not necessarily a Message or a thread. Direct-call module
tests are useful, but do not prove the final parallel application works.

App logic is authored in L1 now and will move to L2. The chain is L2 -> L1 -> C.
L1 is the lasting intermediate language, not permission to move application
logic into hand-written C. Narrow platform ABI headers/adapters are identified
exceptions; header-unit migration is tracked separately in STATUS.txt.

Rendering uses our cell grids, pseudographics and native backend glyphs, not
Qt widgets. Platform backends coexist through a per-handle dispatch table.
FIRST_VERSION.txt is the current milestone boundary: file-backed editable
console, command execution and one F1 Help button. No terminal-emulator/PTY/
ConPTY behaviour in this milestone. Later file-manager features remain part
of the ClearShell port; they are deferred, not removed from the product.

Build and ownership
-------------------
Run from the repository root:

    ./mixa_manager/run_mixa.ps1

This uses stg/l1_baseline/build/l1trans/gen2/l1trans.exe read-only. The runner
prints binary and generated-source identities; neither identity alone proves
they correspond. Do not confuse this tool with build/l1trans/gen2 in the root.
Never run the shared STG gate for app-only changes or replace its compiler.

Codex currently coordinates OpenCode's bounded app work; Grok develops core L2
independently. Preserve Claude/grok_bot's earlier unfinished work; their current
quota pause is not permission to overwrite it. Live inbox/outbox assignments
take precedence over dated ownership snapshots in STATUS.txt.
No waiting on the core lane for app work that can use the installed compiler.
Compiler defects go to Codex/Grok with a reproducer. File ownership and current
tickets are in STATUS.txt and work_chat; do not overwrite another active task.

No Valued aggregates or ordinary dot member access. Use pointer-based structs
and backslash paths; c. is the explicit foreign door. Reference Java is read-only.
Runnable app artefacts go in untracked mixa_manager/bin, not in source commits.
