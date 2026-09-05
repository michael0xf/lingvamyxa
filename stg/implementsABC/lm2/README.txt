stg/implementsABC/lm2 — Consumer uses walk (P0), not an interpreter
=====================================================

Why not a mock interpreter first
--------------------------------

EN: `implements(varA varB Consumer)` uses(Consumer, varB) is a *syntactic*
walk of the remaining P0 tree (implementsTree.txt, spec §2.1 / §19.21.3).
It must see both branches of `if`, not the executed one. A mock interpreter
gives one trace and misses or invents paths. Mock/interpreter is a later
runtime for Mix/LMX, not the first step for implements.

RU: uses — синтаксический обход оставшегося P0, не трасса выполнения.
Интерпретатор с mock увидит один путь (`if`) и пропустит поля в другой
ветке. Сначала обход дерева; интерпретатор — отдельный проект.

What this directory does
------------------------

Walk a P0 document, collect field-follow paths of one name `varB`
(`e\equals`, `e\foo\bar` → `equals`, `foo\bar`).

Assignment tracking (local, same tree only):

    x: e
    y: x
    y\hash

counts `hash` as a use of `e`. After `x: 0`, later `x\toString` is not a
use of `e`. The original name `e` is never dropped. No callee inlining,
no heap/`box\payload`, no computed `e\[nameVar]` (those stay analysis).

    uses.lm2        walk + alias set + CLI main
                    trans stg/implementsABC/lm2/uses.lm2 ; link with libparser+libown
                    ./uses <source.lmx> <varB>
    fixtures/       take_eq (empty uses), take_eq_field, assign_alias
    uses_model.lm2  LmStgUses layout

Empty Consumer (take_eq.lmx e) prints nothing, exit 0.
Field-follow still hits a hosted P0 layout/accessor crash (exit 139);
the walk is in uses.lm2 — fix accessors before wiring into trans.

Live trans still admits with implements(A, B, B) (fat descriptor). This
walk is the staging Consumer; wiring it into trans is a later step.
