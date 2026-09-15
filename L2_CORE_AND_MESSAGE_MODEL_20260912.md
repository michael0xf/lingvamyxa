# Ядро L2 и механизм Message: полная модель для продолжения работы

LATEST IMPLEMENTATION CHECKPOINT — 20260913-1347: `8dc3fae5` restores the
strict low-level pointer surface in authored L2 (`@^n`, prefix raw load,
postfix field-follow, and `[]` only for indexing), adds pointer Array ownership
and copy coverage, and makes the graph gate accept an explicit verified L1
translator without weakening the pinned stable default. Evidence
`build/fable/graph_abi/run_20260913_134728_876_96dc03fe` passes graph ABI 63/0,
graph copy 75/0 including 69 allocation-failure positions, pointer Array 21/0,
merge 261/0 including 43 allocation-failure positions, and 134/134 fixtures
plus all 41 negative cases. The same revision completed the full historical
runner with `l2trans gen2 ok` and exit 0.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0810: `49e05f27` makes unit-level
primitive declarations real children of the program's root Structure. Their
stored addresses remain classified by distinct typed ranges; a method reaches
the root through its callable Structure's `node`, loads an activation cache,
and publishes only dirty values at checkpoints. A nested call may publish a
new graph value, but the caller's already-loaded cache is not reloaded. The
fresh next activation observes the published value. `f0cf7ac0` additionally
ports `lmx_msg_slots` to clean L2 with its real exported symbols and 278/0
L1/L2 parity. Combined evidence
`build/fable/graph_abi/run_20260913_080925_901_1997f070` passes graph ABI 63/0,
copy 66/0, merge 261/0 and 132/132 fixtures; the following full historical run
ends `l2trans gen2 ok`.

The next clean-selfhost step is the tracked source
`stg/l1_baseline/l2src/l2trans.lm2`. The trusted seed is used once; generation
1 must translate that same source into generation 2 with deterministic
L1/C/behavioral agreement. `lm2/l1trans.lm2` is the later L1-to-C compiler
port, not a substitute. The normalized probe now passes the root `unsigned` fields. They use a distinct
appended `LMX_TYPE_UNSIGNED` classifier plus owned read/store/copy services,
never `LMX_TYPE_SIZE_T`; graph copy is 68/0 and full LMX ends
`l2 lmx gen2 ok`. The old cancel-spin host was also brought to the already
accepted body-graph ABI by adding the missing `while` body Structure. The next
measured selfhost barrier is the first root pointer field
(`@@: LmP0Node l2_scope_at 0`), so clean selfbuild is still open.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0612: callable recursion is
integrated as `9ffc96f3`; local address slots grow transactionally without an
arbitrary count cap in `163eeffb`, and `748c75e7` keeps their numbering
disjoint from any method arity. `043e1d41` represents every executable
`if`/`else`/`while`/C-style `for` body as an ordinary graph Structure, including
ownless/empty bodies. `6af2b55e` adds the missing containment relation: nested
bodies are children of the containing body, their `node` points to that body,
and generated execution follows the same parent-indexed `l2_h` chain for
dirty publication. This is graph structure, not a fixed stack/depth table.

The assertion repair `8995dc86` makes the per-fixture blocks actually execute.
Current evidence
`build/fable/graph_abi/run_20260913_060819_532_2c753c41` passes graph/copy/merge
63/66/261, 122/122 fixtures and 38 negatives; the body-parent revision also
completed the full historical runner with `l2trans gen2 ok`. Fable's exact
runtime-port staging is integrated as `d582bbfe`/`fbd415f2` (unsigned pointer
forms, two storage-head types, const Message/runtime formals and exactly four
foreign fields). Fable is now implementing method pointer locals and the first
Message runtime module. The complete clean L2 self-build is NOT reached: these
checks still use the pinned handwritten-L1 translator as bootstrap.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0500: runtime retention gaps are
closed by `34805906`, `1f4b61e3`, `6dce6214` and `a79e14c0`. A bare CHILDREN
root now walks
every pointer slot in its registered half-open range; all five typed Array
descriptors support the canonical empty form `{len = 0, data = 0}`; and an
adopted failure whose complete graph is a primitive cell is retained by its
HISTORY root without keeping an unrelated neighbour alive. Eternal and METHOD
classifier entries are now collector roots even when `graph = 0`; an ordinary
unclassified neighbour is still collected. `45a3cce1` then
integrates Fable's executed argument-as-own bind: int joins char and size_t,
parameter membership is tested separately from type code 0, the same activation
variable is published only after the executed bind, and return-only use creates
no field. `770e83e6` completes the empty Array path through L2 source lowering:
own, ordinary Structure and eternal fields use the typed descriptor
`{len = 0, data = 0}`; merge copies an ordinary descriptor and shares an
admitted eternal one. Overflow remains an error. Evidence
`build/fable/graph_abi/run_20260913_045338_136_a57b6409` passes 63/66/261,
115/115 fixtures and 37 negatives; the following full
`run_l2trans.ps1` ends `l2trans gen2 ok`. `9673bf2e` makes the owner-local
explicit root API consistent with HISTORY: PRIMITIVE and METHOD targets can be
retained and released by address; unrelated storage is collected. Full
`run_lmx.ps1` ends `l2 lmx gen2 ok`. Fable owns callable recursion in
parallel. Grok remains closed.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0400: `17fef09a` runs one shared
METHOD through original `A.M`, copied `R.M`, then original `A.M` again. The
observable own counts are 1/1/2, proving distinct callable Structure state;
the method itself performs merge through the dynamic Message and returns via
status plus separate typed normal/throw outputs. Generated-L1 checks require
the selected M receiver, `node\node` lexical unit, status-before-result order,
and forbid METHOD clone/runtime name lookup. Graph evidence
`build/fable/graph_abi/run_20260913_035603_212_eddb45f8` is 63/59/261,
113/113 fixtures and 37 negatives; full `run_l2trans.ps1` is green. Fable's
parallel older ABI edit was not merged because the current backend already
contains the closed throw/Message implementation. The next frontend slice is
the section 11 executed argument-as-own bind, followed by callable recursion.
Grok remains closed.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0315: Array fields in ordinary and
qualified Structure bodies are integrated as `2ab6fccd`/`62daebf0`. They use
the existing `[]: int|char name count` source form and the separate Array
descriptor `len/data`; Structure.len remains only its immediate child count.
Copying ordinary graph storage produces a new descriptor and backing with the
same Array length. Eternal admission covers both the descriptor and backing,
so merge preserves their addresses as terminals. `8ecec1fd` also proves that a
cross-branch reference to a nested Structure does not reparent it. Combined
evidence `build/fable/graph_abi/run_20260913_031427_202_a5751958` passes
63/59/261, 112/112 fixtures and 37 negatives. Fable's next task is the typed
status/result/throw path for a merge-capable callable selected through original
and copied Structure fields. Claude `5669f53e` has a verified same-application
Delete confirmation module (44/0 plus full green manager gate) and now owns its
visible production file-manager panel. Grok remains closed.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0231: Fable's full qualified
branch body is integrated as `44888c12`/`2a34bee8`. The qualification changes
storage/lifetime classification, not Structure shape: `size_t`, char pointer
cells, nested Structures and translation-known references keep ordinary field
order and address-derived types. The qualified root alone has `node = 0`;
nested Structures retain their internal lexical `node`; every owned typed
range in the branch is admitted to the Message's eternal classifier. Merge
stops at these admitted terminals and preserves their addresses while copying
ordinary mutable graph storage with the operation-wide map. Integration fix
`dd1a3ff4` proves the same operation inside callable bodies: a method reaches
unit-owned eternal roots, named Structures and callable occurrences through
its lexical unit `node\node`, not entry-local aliases. Evidence
`build/fable/graph_abi/run_20260913_022917_552_1e8dd72a` is 63/59/261 and
110/110 with all negatives; the subsequent full historical gate ends
`l2trans gen2 ok`. Fable is actively extending this same field model with
ordinary `int` and Array fields. Claude is working only in `mixa_manager`.
Grok remains closed and receives nothing until the user explicitly resumes him.

LATEST IMPLEMENTATION CHECKPOINT — 20260913-0140 (supersedes dated entries
below): `dd9922b5`/`f9ea3fcf` integrate source field paths and prove
post-merge independence; `e1f87652`/`2e67daf3` integrate field paths inside
methods. Combined evidence
`build/fable/graph_abi/run_20260913_012908_074_532849ea` passes graph ABI 63/0,
copy 59/0, merge 261/0 and 108/108 L2 fixtures. `94946f80` makes the complete
root-Message gate use the current callable Structure `M`, fixed pointer child
slots and current support-object set without a fixed object-count ceiling.
Evidence `build/codex/l2_message_root/20260913_013612_545_75769f40` passes all
49 root modes, splice/nested/history, 91,150 quoted-C differential checks and
1,197,191 C-surface differential checks; the following full
`run_l2trans.ps1` ends `l2trans gen2 ok`. Fable's current task is invocation
through a translation-known callable path, including a callable Structure
selected from a merge result. Claude completed the shared headless controller
in main `cd36338e` and now owns the automated real-Win32-entrypoint smoke.
Grok is closed by the user and receives no tickets.

Follow-up `a787198d` closes the local successful no-copy graph carrier. A
handoff-safe direct child can transfer its existing `blocks+ranges` to its
parent while publishing exactly one explicitly selected RETAIN root. Source
graph/root links are cleared, object/node/backing addresses do not change, and
root-allocation failure leaves both owners unchanged. The full
`run_lmx.ps1` gate ends `l2 lmx gen2 ok`; its EXEC selftest records
`graph transfer: exact addresses retained, released once`.

LATEST IMPLEMENTATION CHECKPOINT (supersedes dated entries below):

- `ed36ddc4` integrates named Structure `size_t`, char-pointer-cell, inline
  nested Structure and reference fields with the method-merge ABI. Combined
  evidence `run_20260913_002047_699_9217c0c3` passes 63/59/261 and 105/105
  fixtures; the following historical gate ends `l2trans gen2 ok`. Its current
  completed-only reference resolution was NOT a language rule and is removed
  by `cc1fd410`: multiphase resolution admits forward, self, mutual-cycle and
  unambiguous nested references, preserves aliases/cycles through merge and
  never reparents the target. Combined evidence
  `run_20260913_003948_121_8928ae5f` passes 63/59/261 and 106/106 fixtures;
  the following historical gate ends `l2trans gen2 ok`, and a fresh full
  `run_lmx.ps1` ends `l2 lmx gen2 ok`. Main `cca9989a` adds the real
  FIRST_VERSION F1 Help path; independent focused evidence is 20/0. Main
  `17fd31fc` adds a growable Unicode command line and nonblocking
  Enter-to-process/marker/file dispatch; independent focused runs are 27/0 and
  23/0. Main `b3e517aa` renders the pending line/cursor in the lower TEXT layer;
  independent console evidence is 61/0. Claude now owns the end-to-end
  headless production-controller proof.

- Integration `08a6c1e4` lowers `merge:` inside a method through the current
  compiler-selected `Message` dynamic input and the declared status + typed
  result/throw outputs. The dependency closes transitively over callers and is
  part of `METHOD.sig`; the signature intern also distinguishes return type and
  throw ABI. `1a2ddfce` integrates Fable's ordinary unit-level named Structure
  declarations as ordered merge operands. Evidence
  `run_20260912_235107_855_276fcc92` passes graph ABI 63/0, copier 59/0,
  runtime merge 261/0 and 104/104 L2 fixtures; historical `run_l2trans.ps1`
  ends `l2trans gen2 ok`. Follow-up `162faac2` completes the same ABI for a
  throwing `sub`: it is physically a status-returning function with only the
  throw output, while an ordinary no-throw `sub` remains void. Evidence
  `run_20260912_235540_514_a21c2241` keeps the combined 63/59/261/104 gate
  green, followed by another green historical translator run. The old
  in-method refusal is retired.

- `c8e3738e` removes the frontend's separate five-level body rejection and
  64-byte indentation buffer. Indentation strings and their stable pointer
  table now grow from actual nesting with checked arithmetic and are released
  with the translation state. `unit_deepif` executes 70 nested conditions and
  the fault sweep reaches allocation kind 8 while requiring zero live
  allocations and an unchanged output on every failure. Evidence
  `run_20260912_143755_409_0cc4f9ba`: ABI 63/0, copier 59/0, merge 261/0,
  fixtures 101/101.

- `8a14ab9f` removes the fixed 16-entry canonical contract/signature intern
  table. All eight parallel metadata arrays now grow transactionally with
  checked count/byte arithmetic; allocation failure preserves the published
  table. `e589eb46` removes the separate 32-pass hidden lexical-dependency
  closure limit and converges over the finite discovered-fact set. The combined
  gate at `run_20260912_143143_315_aca57298` passes ABI 63/0, copier 59/0,
  merge 261/0 and all 100/100 L2 fixtures, including 18 distinct contracts,
  the growth-allocation failure path and a reverse-declared 65-call chain.
  These changes introduce no replacement 16/32/64/128 semantic ceiling.

- Claude's process seam is accepted and integrated as `14b1011b` plus
  `1b3b0496`. The corrected six-operation API has owning pointer-to-pointer
  close, real shell delegation, a valid stdin pipe/EOF, one merged output pipe,
  nonblocking read and Job Object tree termination. Independent focused runs
  `run_20260912_144052_018_23920b92` and
  `run_20260912_144104_377_dea4e603` are both 122/0 with identical implementation
  hash; full `run_mixa.ps1` exits 0. Claude's next isolated manager slice is the
  incremental in-band marker stage from `PROCESS_SEAM` 4.1; Message attachment
  remains later.

- Runtime `merge` remains accepted through `f09fc838` with its independent
  63/59/261/98 gate. Fable's later source-lowering commit `b5b56432` is NOT
  accepted: it recognizes bare `merge: E F`, discards the result, accepts only
  eternal operands, rejects the valid one-operand form, misses the actual parse
  tree of `R: merge: A`, and treats process exit 70 as a declared throw.
  Correct lowering must recognize outer result frame R containing a nested
  merge frame, exclude an optional trailing Structure body from the operands,
  bind the returned fresh Structure into the containing graph, and prove an
  ordinary mutable source is deeply copied. L2 failure transport is explicit
  status plus separate typed normal-result and failure outputs; the normal
  result is not published on throw. Current L1's global `l1_throw_code` and
  fixed `long l1_throw_payload[8]` are historical and are NOT the L2 ABI.

- The integration branch now includes Claude's console/native-loop work and
  tracked manager dependency repair through `307e4c0e`. A complete
  Win32-profile `mixa_manager/run_mixa.ps1` also requires the versioned host
  ingress pin and harness. Commit `227b58c0` tracks exactly those seven files;
  all five pinned payload hashes match their manifest, and the full Windows
  profile exits 0, including the harness's intentional exit-1 join-timeout
  subcase. Claude now owns the next independent manager stage: the six-operation
  owner-local Windows process seam from `PROCESS_SEAM.txt`.

- Grok remains closed by the user. Do not send tasks, restart his session, or
  rearm his watcher until the user explicitly resumes him.

CURRENT IMPLEMENTATION CHECKPOINT — 20260912-1600:

- Runtime merge is integrated through `f09fc838`. It uses one copier operation
  for every ordinary operand and body root; constructs a fresh ordered result;
  preserves shared METHOD and eternal terminals through separate classifiers;
  accepts an empty Structure result; rejects admitted primitive operands and
  checked count/byte overflow; and publishes private storage only on success.
  Independent evidence `run_20260912_135850_798_0ef5bc7f`: ABI 63/0, copier
  59/0, merge 261/0, fixtures 98/98 and all 43 allocation-failure positions.
  Source-position lowering and declared `throws merge(args)` remain in progress
  with Fable.

- `8722dd1c` closes a real second-hop copy failure for shared methods. A
  callable copied from the root Message retained the correct METHOD address,
  but that address was intentionally absent from the child's owned ranges, so
  the child could not classify it when creating another Message. Each Message
  now owns separate non-owning `method_ranges` metadata, distinct from
  `eternal_ranges`; creation clones both classifiers while copying neither
  root array nor terminal payload. `5f02d6ed` makes the root emitter admit each
  METHOD record when it builds the fixed root-owned method array. The test now
  performs root -> child -> next Message and requires fresh ordinary graphs,
  the identical METHOD and E addresses, private classifier metadata and one
  publication. Evidence `run_20260912_135251_489_5a9f6d3e`: ABI 63/0,
  copier 59/0, Message 39/0, fixtures 98/98; full LMX ends `l2 lmx gen2 ok`.

- Main through `a2643ed3` gives each Message a separate, non-owning
  `eternal_ranges` classifier. Bootstrap admission records exactly one typed
  element per qualified address rather than admitting an allocator's whole
  range. E, its children and both root arrays remain owned by the first
  Message's ordinary blocks/ranges. Commit `313f7034` additionally makes
  `lmx_msg_create_graph` clone classifier metadata into the private child before
  publication, so a child can forward an admitted E to another Message without
  a global/root accessor. No payload or retention array is exposed by that
  metadata copy. Evidence `run_20260912_132532_566_733f1c6d`: ABI 63/0,
  copier 55/0, Message graph/create 30/0, fixtures 98/98; full LMX ends
  `l2 lmx gen2 ok`. Qualified emission is integrated and independently rebuilt
  cleanly by Fable as `f5e2d300`; do not use the superseded stale
  `fable/graph-copy` history.

- `a2643ed3` removes the fixed 32-entry duplicate-history ceiling from Message.
  `(from,id)` history now grows with checked doubling; a two-buffer growth
  failure preserves both old arrays and all entries. The focused test records
  160 unique entries and proves a duplicate does not grow the table. Evidence
  `run_20260912_133756_957_3a677166`: ABI 63/0, copier 59/0, Message 30/0,
  fixtures 98/98; full LMX reaches `l2 lmx gen2 ok`.

- Fable's runtime merge helper is accepted and integrated as recorded above.

- Claude completed the first native `mixa_manager` loop in `f389e179`: existing
  backend and pump, real FileManager/Selection/CopyHere context, Ctrl+V exactly
  once, FIFO preservation and CLOSE termination. Two focused runs are 23/0 and
  a Win32 executable links without being launched. The follow-up restores and
  completes the existing file-backed console-window work and connects a real
  presented frame; it must also address the current native main's tight polling
  and error handling. This application lane does not change the L2 graph model.

- Verified graph/callable integration is now on main: one operation-wide copier,
  per-callable Structure M with shared METHOD in physical slot 0, the fixed root
  METHOD array, and the fixed retention ARRAY_OF_LMX for qualified eternal E.
  Qualified source uses the full `independent: const: immutable: (): E ...`
  constructor, E remains a declaration-site unit child with `E.node = 0`, and
  the retention array supplies the second reference. Translator storage grows;
  the 70-E fixture forbids a hidden 16/64/128 cap.
- `lmx_msg_create_graph` now prepares the complete copied graph in a private
  child slot and publishes the child only after copy and path setup succeed.
  Failure leaves the child count reusable. The focused Message graph test is
  22/0. (create_id and the create-retry idempotence removed by Mikhail's order,
  2026-09-15.)
- The repeated Exec timeout was not fully explained by the earlier test observer.
  A live-process backtrace found workers looping in `lmx_msg_find_tree` while
  stop waited in `bind_reap_join_all`: retirement freed a root after removing it
  from `rt->slots` but left its pointer in `rt->root`. Commit 77a20933 unlinks the
  root under the exec lock before freeing it. The new assertion rejects the old
  code deterministically (`root` remains non-null); fixed code passed targeted
  Exec, 30/30 stress runs and full `run_lmx.ps1`.
- Integrated graph evidence `run_20260912_131206_083_f6f67124`: ABI 63/0,
  copier 55/0, Message copy/create 22/0 and fixtures 98/98. The full LMX run ends
  `l2 lmx gen2 ok`.
- Fable commit 09fd8037 is superseded by ownership repair `93405e71`: E and
  every child are again in first-Message storage. The remaining active task is
  to call the Message-owned classifier API from the emitter; no file-scope
  payload owner or classifier is accepted.

CURRENT USER AVAILABILITY — 20260912-114331: the user CLOSED Grok. No new tickets,
coding tasks, reminders or document notifications to Grok until the user
explicitly resumes him. Do not restart his closed session or automatically
rearm its helper. Preserve unfinished112047 nested Exec hang work and evidence.
Codex/Fable continue their owned scope; Codex continuity/outbox receiver stays
active. This supersedes the earlier11:20 ACTIVE instruction below.


Состояние модели: 12 сентября 2026 года. Этот документ предназначен для Codex,
Grok, Fable и человека, который продолжает работу после смены модели, нового
контекста или паузы. Он описывает устройство языка и исполнения прежде, чем
перечислять задачи. План работ находится в последней части.

Документ не требует памяти о переписке. В нём явно разделены **согласованная
модель**, **существующий прототип** и **недостающая реализация**. Наличие функции
в C или зелёного старого теста не превращает её поведение в правило языка.

Нормативные источники: [спецификация](Lingvamyxa_spec.txt), прежде всего разделы
2, 6.5, 7, 8.8, 9.1.2–9.1.4, 11.3.1, 19.17, 19.28, 19.29 и 21;
[Revision 2, EN/RU](struct_refactoring_version_2.txt), прежде всего разделы
3–8 и 13–14. Более позднее явное уточнение пользователя обновляет эти источники
и этот документ вместе. Старая памятка или статус прототипа не отменяет его.

## Как читать и обновлять

При восстановлении контекста прочитать части I–III целиком. Затем прочитать
часть IV о состоянии реализации и только после неё брать задачу из части V.
Для изменения конкретного механизма сверить соответствующий раздел исходной
спецификации, исходники и доказательства проверки нужной ревизии.

Обозначения `copy(P)`, `child(S,i)`, `slot(S,i)`, `typed_entry(...)` ниже —
объясняющие обозначения. Они не вводят новые receivers или синтаксис L2.
Фрагменты C показывают смысл представления; точный заголовок конкретной сборки
проверяется отдельно. Состояние веток и результаты тестов имеют дату, а не
статус вечного архитектурного решения.

Разрешено задавать пользователю вопрос, если найдены **два несовместимых правила
модели и минимальный пример, на котором они дают разные результаты**.
Отсутствующая функция, неподходящий тест, размер буфера, порядок внутренних
шагов аллокатора или непрочитанная спецификация таким противоречием не являются.
Сначала надо найти уже данное решение и выполнить инженерную работу.

## Быстро найти уже согласованный ответ

При похожем вопросе сначала найти **механизм и его владельца**, затем прочитать
указанный раздел. `node` встречается и как имя поля Structure, и как имя первого
параметра C-схем: это разные места хранения, даже когда слово написано одинаково.
Таблица служит указателем; она не заменяет полное описание ниже.

| Что требуется различить | Короткий ответ | Где проверить правило |
| --- | --- | --- |
| Callable-узел M и METHOD D | M — обычная Structure с собственными полями. D — общая immutable запись `{addr,sig}` с адресом native-кода. | Здесь §§8–9; SPEC 7.1, 21.5, 21.8; ABI 6.3 |
| Узел вызова и его лексический родитель | Выбор M через R передаёт M первым аргументом. Поле M.node ведёт к предку; это не сам M и не дескриптор. | Здесь §8; SPEC 21.2; ABI 3.2, 5.4 |
| Физический слот 0 и `[0]name` | M.children[0] держит D. `[0]name` выбирает первое вхождение имени, которое может находиться в другом слоте. | Здесь §§6, 8; SPEC 2.3, 7.1 |
| Что переставляет merge | Поля верхнего уровня результата следуют порядку merge. Внутренний порядок детей скопированной M сохраняется. | Здесь §§16–18, 40; SPEC 2.3, 21.8 |
| Поле собственной M и внешнее x | Own-cache относится к M. Внешнее x — отдельный lexical/dynamic вход или путь; его нельзя сделать own-полем плоского unit ради удобства эмиттера. | Здесь §§10–12, 40; SPEC 21.3–21.5 |
| Копирование и сохранность исходников | A и B остаются; появляется R. Одна карта копирует всё используемое, включая node до 0, сохраняет aliases/cycles и переписывает ссылки копий. | Здесь §§16–19; SPEC 2.3; ABI 8.2 |
| Что остаётся общим | Допущенные вечные ветки и METHOD, а не произвольное immutable или mutable значение. | Здесь §20; SPEC 9.1.4 |
| Квалификация и метаданные копирования | Квалификация задаёт свойства storage; проверка адреса по допущенным диапазонам использует метаданные реализации. Это не новое поле в каждой Structure и не регистрация строковых имён. | Здесь §§3, 20, шаг 4; SPEC 2, 9.1.4 |
| Кто создаёт два массива | Транслятор строит исходный граф первого Message и заполняет его массивы. Дальнейшие Message явно получают нужные графы/ссылки. | Здесь §20; SPEC 9.1.4; ABI 14.9 |
| Одна M и несколько рекурсивных вызовов | Опубликованная M одна; C-активации и их кэши отдельные. Опубликованное значение определяется последней dirty-записью; при возврате кэш caller не перезагружается. | Здесь §§11–12, 40; SPEC 21.7, примеры и Algol; ABI 6.9 |
| Argument и поле тела | Аргумент сам по себе не поле. Исполненный `arg: 5` делает его own-полем с этого места, без смены переменной, адреса или lifetime. | Здесь §11; SPEC 11.3.1, 21.5.1; ABI 6.5 |
| Два `len` | Structure.len — число child slots, включая METHOD-слот у callable. Array.len — длина массива по его контракту. | Здесь §§2–5; SPEC 2, 6.5 |
| `@` и владение | В L2 это обычное взятие адреса по выбранному объекту, не перенос владения и не продление lifetime. | Здесь §13; SPEC 11.3.1 |
| Закрытый вопрос и недописанный код | Ответ в модели не означает готовность эмиттера. Сначала сверить получателя, пути и графовую форму; проверить случай, которого нет в standalone fixtures. | Здесь §§39–41 |

Три проверки формулировки перед новым вопросом:

1. Указать, о чём речь: METHOD-записи, callable Structure, поле `.node`,
   C-активации или Message. Фраза «у метода нет node» без этого уточнения двусмысленна.
2. Для поля назвать путь и конкретную Structure, которой оно принадлежит.
   Сдвиг M во внешнем R не доказывает сдвиг собственного поля внутри M.
3. Найти определение, а не только статус прототипа: `occurrence`, `own`,
   `checkpoint`, `Algol`, `independent`, `Message creation`. Сослаться на номер
   раздела; номера строк меняются при редактировании.

## Содержание

- Часть I: значения, адреса, Structure, Array, методы и вызовы.
- Часть II: копирование, merge, независимые и вечные ветки, память.
- Часть III: Message, очереди, turns, планирование, завершение и диагностика.
- Часть IV: текущая реализация, доказательства, ограничения и карта исходников.
- Часть V: последовательность работ и критерии готовности.

# Часть I. Ядро языка

## 1. Уровни языка и граница задачи

**L2 — низкоуровневый язык.** Он позволяет обычные для выбранного backend
машинные операции: адреса, явные обращения к памяти, native-вызовы и другие
операции выбранного профиля. Нельзя приписывать взятию адреса несуществующий
перенос владения, автоматически удлинять время жизни объекта или запрещать
изменение указателя из соображений, заимствованных из другого языка.

L3 — подмножество L2, в котором исключены низкоуровневые машинные операции,
в частности `@`. Это не основание проектировать само L2 как L3. В то же время
выбор механизма Message означает соблюдение его конкретного контракта:
одна arena владельца, сериализация собственных turns, явные границы передачи.

Код ядра и стандартного комплекта взаимодействует через Message. Низкоуровневые
блокировки, atomics и native wait/wake используются внутри реализации Message.
Из наличия этих возможностей в L2 не следует право завести второй глобальный
менеджер потоков внутри Mix или прикладного модуля.

Путь реализации: **L2 → L1 → C → native executable**. L1 служит доступным
инструментом bootstrap и низкоуровневым backend. Сгенерированный C можно хранить
для переносимого bootstrap. Исполняемые алгоритмы языка постепенно становятся
исходниками L2; необходимые платформенные примитивы остаются узкими адаптерами.

### 1.1. Точное понижение `@ⁿ` и два правила `\`

`@ⁿ:` в L2 задаёт явную глубину машинного адресного слота. L2 сначала выбирает
C-проекцию базового типа, затем прибавляет к её неявной C-глубине ровно `n`
явных уровней. Например, у `char` неявная глубина равна нулю, а профильная
проекция обычной Structure уже является `Lmx *`; поэтому `@: char` понижается
к `char *`, а `@: User` — к адресу слота, хранящего `Lmx *`, то есть к
`Lmx **`. Явную исходную глубину и итоговое число C-звёзд нельзя считать одним
и тем же типовым фактом.

После этой L2-проекции L1 получает уже выбранное C-основание. Поэтому L1
поддерживает любой непустой head из `@`: `@: T`, `@@: T`, `@@@: T`, …, и
выводит ровно столько `*`, сколько символов в head. Специальных пределов 2,
16, 64 или 128 нет. Для `Array<@ⁿ T>` адрес backing добавляет ещё один уровень:
его `data` имеет проекцию `@ⁿ⁺¹ T`. Размерности настоящего C array остаются
размерностями декларатора и не сплющиваются автоматически в `T **`.

У `\` два строгих позиционных правила. Последовательность перед выражением
делает raw-load: каждый ведущий `\` снимает ровно один адресный уровень, в том
числе слева от присваивания. `value\field` после значения является field-follow
и в L1 понижается к C `->`. Эти формы не заменяют индексирование: `items[i]`
остаётся обращением к элементу массива.

Это полный набор значений `\` в исполняемом L1/L2. В исходнике не используются
C-варианты `*p`, `p->field` или `p[0]` для чтения одного скаляра: они возникают
только в сгенерированном C. Цепочка разбирается позиционно: `\p\field` сначала
снимает один адресный уровень, затем идёт по полю; `p\field[i]` сначала идёт по
полю, затем индексирует настоящий массив или указатель. Круглые скобки вызова
определяются P0-флагом `LM_P0_FRAME_COMPACT`; двоеточие остаётся обновлением
поля/значения. Поэтому одинаковый текст head не превращается в вызов по догадке.

## 2. Единственный общий заголовок Structure

```c
typedef struct Lmx Lmx;
struct Lmx {
    Lmx *node;
    int len;
    void *data;
};
```

| Поле | Единственный смысл |
| --- | --- |
| `node` | Лексический родитель данной Structure; у независимого корня ноль. |
| `len` | Число непосредственных детей данной Structure. |
| `data` | Адрес упорядоченного массива указателей `void *` на детей. |

Рядом с ним — собственная запись Message (Михаил, 2026-09-15: «двигайтесь к»
этой форме; «Все остальное -- не Message и должно в итоге перевестись на LMX
то есть стать внутренностью Message»; флаг handoff подтверждён им в тот же
день):

```c
typedef struct LmxMsg {
    volatile uint_fast8_t running;       /* 1 при запуске; опрашивается при выполнении root;
                                            0 — запрос родителя на остановку, и ставится при выходе */
    volatile uint_fast8_t success;       /* 0 при запуске; 1 пишет пользовательский код */
    volatile uint_fast8_t handoff_safe;  /* 1 пишет сам Message, покинув последний turn,
                                            когда ареной не пользуется ничто нативное:
                                            арену можно присоединять (19.29.7, 19.29.8) */
    Lmx *root;                           /* корень его графа; арена достижима от него */
} LmxMsg;
```

Михаил, 2026-09-15, дословно, о двух флагах: «success && !running, с выходом выставляется running = 0. Просто running опрашивается при выполнении Lmx* root и я же не знаю как кокнретный планировщик устроен и когда выйдет. Но условия выхода очевидно есть!».

Всё остальное — не Message и в итоге записывается на LMX как внутренность
Message (данные его Structure в его арене): запись L3 Thread добавляет его
почтовый ящик (единственная синхронизированная коллекция Messages) и его
состояние планировщика и обслуживания как такие данные.
Михаил, 2026-09-15, дословно, об этой внутренности: «"и транслируется в LMX как внутренность Message." -- то есть записывается в "struct Lmx *root; " , и на L1 нужно сделать имитацию (то есть не имитацию а код вкоторый L2 и L3 будут разворачиваться)» и, поправляя
себя в ту же минуту: «"то есть записывается в "struct Lmx *root;"" это уже на L2 там лежит код который с этими Message и работает. А на L1 просто тот код который _будет порождаться_.». О форме этого кода: «В точности как имитируется L2 на Си -- функция с аргументами ,скрытми ргументами и узлом графа.». И потому: «Поэтому структура Message как данные остается и на Си -- в точности как стуктура Lmx» — запись выше есть тип данных C рядом с Lmx, а не то, что позже переписывается на LMX. О стоявшей здесь фразе координатора «the C keeps only the native primitives (the thread, the mailbox's monitor)», дословно: «"the C keeps only the native primitives (the thread, the mailbox's monitor)»" -- это для L3 Thread? Он тоже пишется на L1,переноситс на L2 и остаток на L3. Разумеется на L2 остается низкий уровень, а причем тут вообще Си? Ты же не пишешьв Си на ассемблере встаквки "потому что надо компилировать". Удаляй». И о том, для чего L1, дословно: «Вы пытаетесь писать на Си, а на Си (L1) надо просто пилить ядро _на котором_ уже пилить дальнейший функционал самого ядра. Это как я бы попросил на goto имитировать все циклы, а вы бы заменили задание на "написать примеры while и for"  -- совершенно разне задачи».

Если детей три, `len == 3`. Это не три байта, не длина строки и не размер
всего достижимого поддерева. Число полей сформированной Structure фиксировано.
Указатели, хранящиеся в её полях, могут изменяться по обычным правилам записи.

В Structure нет обязательного четвёртого слова: ни `owner`, ни arena, ни имени,
ни общего descriptor, ни vtable, ни типа. Нет отдельного C-layout для каждого
исходного имени Structure. Метаданные конкретного аллокатора не расширяют
исходный заголовок.

## 3. Ребёнок — указатель на значение

```c
void **slots = (void **)s->data;
void **slot_address = &slots[i];
void *child_value = slots[i];
```

Это три разных адреса: адрес массива, адрес ячейки указателя и адрес значения,
которое эта ячейка выбирает. Тип ребёнка определяется по **`child_value`**.
Определение типа `slot_address` сообщает о массиве указателей, а не о ребёнке.

Пример физического графа:

```text
S = { node=P, len=4, data=slots }
slots[0] ──► int-ячейка со значением 7
slots[1] ──► вложенная Structure T = { node=S, len=..., data=... }
slots[2] ──► запись Array { len=3, data=backing }
slots[3] ──► общая запись метода { addr, sig }
```

Только второй адресат имеет общий заголовок `node/len/data`. INT, CHAR,
`size_t`, `char *`, запись Array и запись метода не оборачиваются обязательным
`Lmx` ради одинакового способа доступа. Код вида `((Lmx *)any_child)->data`
ошибочен, если тип адресата не установлен как Structure.

В актуальном кандидате этому соответствуют `lmx_branch_slot_known` — адрес
слота, `lmx_branch_child_known` — его значение, `lmx_branch_store_known` —
запись значения слота, `lmx_branch_struct_known` — обращение при доказанном
типе Structure. Суффикс `known` относится к знанию layout на этом месте кода,
а не к универсальному разрешению приводить любой ребёнок к `Lmx *`.

## 4. Тип T и диапазоны адресов

Значения соответствующего типа T располагаются в типизированных массивах или
зарегистрированных диапазонах. При необходимости классификатор устанавливает,
в какой диапазон входит адрес и какой тип/шаг элемента имеет этот диапазон.
Для одного T может существовать несколько физических диапазонов: рост пула
добавляет блоки, сохраняя адреса уже существующих элементов.

Поэтому «отдельный идентификатор для каждого типа T» не надо превращать в
дескриптор на каждом ребёнке. Реализация классификатора может иметь свои коды
типов, но наблюдаемая модель — тип значения определяется его адресом в массивах
соответствующего типа. Имя поля и число звёздочек в записи C этого не заменяют.

| Адресуемое значение | Что известно после классификации |
| --- | --- |
| INT / SIZE_T / CHAR | Как читать и писать соответствующую native-ячейку. |
| CHAR_PTR | Что это ячейка `char *`, содержащая адрес строки. |
| STRUCT | Что можно читать `node`, число детей и массив `void *`. |
| ARRAY_OF_T | Что это запись Array с backing элементов T. |
| METHOD | Что это известная неизменяемая callable-запись с точным `sig`. |

Служебный диапазон child-slots — отдельная физическая сущность. Членство
слота в таком диапазоне не означает, что адресат слота тоже имеет этот тип.
Адрес диапазона, метаданные владения и справочное имя — разные сведения.

## 5. Array, строка и два разных `len`

У Array своя запись **`len/data`**, отдельная от Structure. Её `len` задаёт
длину/число элементов массива по контракту Array, а `data` ведёт на отдельное
непрерывное хранилище элементов. Для многомерного Array используется
прямоугольный backing согласно разделу 6.5; это не дерево отдельно выделенных
строк. Конкретные размерности и операции формы относятся к Array.

```text
поле Structure: void * ──► Array-запись { len, data }
                                          │
                                          └──► элементы T
```

Число записей в служебном пуле Array, длина выбранного Array и число детей
Structure не связаны равенством. Размеры буферов вычисляются для конкретного
объёма данных с проверкой представимости, а не заменяются «ячейкой на 1040 байт».

Исходная форма `@: char "hello"` имеет другой смысл:

```text
child-slot (void *) ──► элемент типизированного массива char *
                                │
                                └──► строка "hello"
```

Это адрес ячейки `char *`, которая содержит адрес строки. В этой форме нет
автоматического `{len,data}` descriptor строки. Отдельную строковую Structure
или длиносодержащую строку можно явно определить; нельзя незаметно заменить
ею смысл данной записи. Обычный CHAR и ячейка CHAR_PTR также различаются.

Нулевой указатель, существующая пустая Structure и запись пустого Array должны
различаться там, где исходная программа их различает. Конкретное хранение
пустого backing — инженерная задача; отсутствие поддержки пустого Array в
первом helper не означает запрета всех пустых значений в языке.

## 6. Имена, порядок и справочная таблица

В обычном исполнении используются адреса и разрешённые индексы детей.
Имена исходника нужны транслятору, а не для поиска каждого значения в runtime.

Поля с одинаковым **исходным именем** остаются отдельными полями в порядке
исходника/результата merge. Обычное имя обозначает первое вхождение:

```text
name == [0]name
[1]name — следующее вхождение того же исходного имени
```

Это не разрешение коллизий хешей. Нельзя описывать идентичность этих полей
через совпадение `ShortNameId`, поскольку справочная таблица не определяет
структуру исполняемого дерева.

Справочная таблица имеет направление **адрес Structure → короткое имя Structure
в исходнике**. `ShortNameId` — старое название этого имени, не обязательный
уникальный ID исполнения. Таблица может помогать строковым инструментам и
диагностике. Обрабатываемая строка может вообще не соответствовать дереву.

Регистрация имени не нужна для создания, копирования, merge или вызова.
Анонимные и позиционные элементы не требуют придуманных имён. Отсутствующая
строка не создаёт узел. Если инструмент явно выполняет строковый поиск от
заданного корня, он использует доступную справочную информацию; обычный
скомпилированный доступ не превращается из-за этого в поиск по строкам.

## 7. Четыре отношения, которые нельзя смешивать

| Отношение | Чем задано | Что оно определяет |
| --- | --- | --- |
| Лексическое дерево | `Structure.node` | Лексический fallback и путь к предкам. |
| Граф значений | Указатели детей, Array-ссылки и другие разрешённые рёбра | Достижимость, aliasing и копирование данных. |
| Владение памятью | Arena и метаданные диапазонов | Кто сохраняет/освобождает storage. |
| Семья Message | Родитель Message и список его прямых детей | Планирование и supervision. |

Пятая связь — физическое отображение Message на OS thread — тоже отдельна.
Нельзя вычислять владельца arena обходом `node`, считать лексического родителя
supervisor-ом или считать объект Message непосредственным ребёнком каждого
содержащегося в нём исходного узла.

В одной arena могут быть несколько лексических деревьев. Обычная передача
arena другому владельцу не переписывает эти деревья. Напротив, глубокое
копирование строит новые объекты и переписывает **ссылки копий**.

## 8. Собственный узел callable, его node и общий дескриптор

Callable-occurrence — обычная Structure M с заголовком node/len/data.
Её физический слот детей 0 содержит указатель на общий immutable METHOD
{addr,sig}; далее идут собственные поля и граф тела. Запись METHOD указывает на
нативный код и не имеет node/заголовка. M.node — лексический родитель M.
Первый зарезервированный аргумент вызова — сама M (invoking_structure в прежних
схемах), а не внешний контейнер, из которого выбрали M. Физический слот 0 для
дескриптора и [0]name для первого вхождения имени — разные правила.
Merge/создание Message копируют M как обычную используемую Structure: порядок
внутренних детей сохраняется, node/обычные ссылки переписываются, адрес METHOD
в слоте 0 остаётся прежним. На callable-occurrence приходится одно опубликованное
состояние M; рекурсия создаёт только отдельные C-активации/dirty flags. Их dirty
stores могут перезаписывать опубликованные значения; reload после вызова нет.

```text
R.children[i] -> M { node, len, data }
M.children[0] -> METHOD D { addr, sig }   // общая immutable запись
M.children[1...] -> собственные поля / граф тела M
вызов выбранного R\m: D.addr(M, лексические, динамические, явные входы...)
node\x внутри вызова: поле x непосредственно в M
лексический родитель: M.node
```

Слово «узел» здесь обозначает обычную Structure, а не новый вид объекта или
новое слово синтаксиса. `invoking_structure` в схемах — выбранная M, не R.
У METHOD нет node; у M есть обычный node. Эти два утверждения совместимы.
Лексический fallback может идти по node к предкам до нуля. independent обрывает
внешнюю цепочку, сохраняя внутренние связи и передачу входов caller.

## 9. Методы общие, данные вызова различаются

Метод уже известен транслятору/линкеру; `fn` задаёт имя обычной C-функции по
существующим правилам. Вызов через разные Structure может использовать один
и тот же код и одну неизменяемую запись **`{addr,sig}`**.

Эта запись — именно запись функции. Упоминание её descriptor не означает
дескриптор каждой Structure, имя типа или дополнительный заголовок ребёнка.
`sig` описывает точный контракт вызова: необходимые структурные, лексические,
динамические и явные входы, их типы/порядок/способ передачи, результат и throws.

На месте вызова известен ожидаемый контракт. Выбранная runtime-функция
должна соответствовать ему; одинакового количества C-аргументов недостаточно.
Вызов с несовместимой сигнатурой не становится допустимым после cast.

Концептуально:

```text
typed_entry(
    invoking_structure,
    lexical_inputs_selected_by_sig...,
    dynamic_inputs_selected_by_sig...,
    explicit_actuals...,
    typed_result_and_throw_outputs_as_defined_by_sig...
)
```

Группы входов заданы моделью; точная физическая сигнатура фиксируется `sig`.
`merge:` создаёт обязательную динамическую зависимость от текущего исполняемого
`Message`. Она передаётся в уже определённой группе
`dynamic_inputs_selected_by_sig` и распространяется транзитивно через
вызывающие методы так же, как остальные динамические зависимости. На уровне C
это значение типа `LmxMsg *`: входная точка уже имеет `process_message` и
передаёт его первому вызову. Метод, вызывающий другой метод, которому нужен
`Message`, принимает и передаёт тот же динамический вход, даже если в его
собственном теле нет `merge:`.

Это не новый исходный аргумент и не новая категория скрытых аргументов.
`Message` нельзя получать обходом `node`, через global/TLS accessor или
угадыванием владельца arena по адресу объекта. Это обычный выбранный
транслятором динамический вход, записанный в `sig`. Вызванный код явно передаёт
его внутреннему helper merge, который использует blocks и typed ranges этого
`Message`.

Нельзя добавлять исходному методу скрытый `MessageThread`, raw arena parts,
Namespace, closure environment или descriptor вместо его согласованных входов.
Передача текущего `Message` как описанного выше динамического входа разрешена и
обязательна для кода, транзитивно достигающего `merge:`.
Внутренний C-helper может явно получать служебные аргументы. Native-адаптер
Message может найти callable и сформировать исходный типизированный вызов.

Вложенные исходные методы компилируются в обычные заранее известные функции.
Здесь не требуются nested C functions, компиляция во время merge или копия
машинного кода вместе с графом.

При копировании/merge указатели METHOD остаются **теми же указателями**.
Если R содержит callable M, вызов R\m получает M; копия M' в другом графе
получает M'. Общая запись не меняется. Время жизни общего descriptor
определено: запись лежит в отдельном const: immutable массиве дескрипторов
методов первого Message и живёт до завершения процесса ОС. Указатель на запись
из освобождённой arena дочернего Message этому правилу не соответствует.

## 10. Передача аргументов и разрешение свободных имён

Примитивные явные и динамические аргументы передаются значениями. Значение
может быть прочитано из типизированной ячейки графа, но параметр вызванной
C-функции от этого не становится общей изменяемой ячейкой вызывающего.
Structure/Array передаются как значения ссылок: локальная копия указателя,
без автоматического клонирования адресата.

Порядок получения требуемого свободного имени x:

1. ближайшее собственное текущее значение x в активации вызывающего;
2. иначе динамически полученное вызывающим значение x;
3. иначе лексический fallback вызываемого через его Structure и `node` до нуля;
4. если входа нет — обычная несовместимость вызова, а не придуманное значение.

Необходимые динамические входы вычисляются транзитивно по вызовам до
неподвижной точки. Если M не читает x, но передаёт его в N, M всё равно обязан
иметь и передать соответствующее текущее значение. Для runtime-выбора callable
требуемые входы задаёт ожидаемый/принятый `sig`.

Перепривязка локальной ссылки не перепривязывает переменную вызывающего.
Явная запись через ссылку меняет выбранный адресат и видна другим ссылкам на
него внутри того же Message. Это обычное aliasing, отдельное от глубокого merge.

## 11. Собственные поля, C-активация и dirty-cache

Опубликованное состояние метода находится в полях его собственной callable Structure.
Текущая активация использует типизированные C-локальные значения только тех
собственных полей, которые ей нужны. Это рабочий cache, а не ещё один
исходный объект Namespace или автоматически построенная история вызовов.

Явный доступ `node\x`, `reference\field` или `array[index]` каждый раз
обращается к выбранному графу/массиву. Он не подменяется правилом «однажды
загрузили own-local и больше ничего не читаем».

Присваивание собственному рабочему значению после его bind делает его dirty.
Недостаточно того, что присваивание написано в непосещённой ветке. Взятие
адреса само по себе не является записью. Нельзя выводить dirty сравнением
устаревшего cache с текущим графом.

Входной параметр без own-bind остаётся приватным значением активации.
Согласованный same-name bind связывает **ту же переменную** параметра с
публикацией в собственное поле: не создаёт новую переменную, не меняет её адрес
или C lifetime, не загружает поверх входа старое значение графа. Записи,
выполненные до bind, не публикуются задним числом.

**Все исполняемые тела являются частью графа. Аргументы callable и `return`
сами по себе частью графа не являются.** Это правило распространяется на все
receivers и trailers: роль потребляемого значения важнее отступа или имени.
Поля тел `if`, `else`, `for`, `while` принадлежат соответствующим Structure тел;
вложенные тела образуют свою реальную иерархию. Вложенность тела сама по себе
не создаёт дополнительный вызов метода.

Если внутри исполняемого тела аргумент используется в объявлении/привязке
собственного поля, **с этого момента это поле структуры тела**. Например:

```text
fn: f (int: arg) int
    arg: 5
    return: arg
end: f
```

До строки `arg: 5` arg — вход callable. Строка `arg: 5` создаёт его own-binding
в структуре тела и записывает 5 в то же рабочее значение; оно становится dirty
и публикуется checkpoint-ом. Это не изменение аргумента у вызывающего.
`return: arg` читает это значение как аргумент return и не создаёт ещё одного
поля. Если строка bind не исполнена, она не превращает аргумент в поле задним
числом. Адрес и C lifetime той же переменной аргумента не меняются.

Число физических слотов Structure остаётся фиксированным. Транслятор может
заранее предусмотреть место для объявленного в теле поля; это не означает,
что до строки bind вход уже является own-полем или должен загрузиться из графа.
Физическая подготовка storage и момент появления own-binding различаются.

Ссылочный аргумент может указывать на существующий граф; этот граф никуда не
исчезает, но сама передача ссылки не создаёт отдельное поле. Выражение-аргумент
может вызвать код: тело того кода входит в граф, а внешнее выражение остаётся
аргументом. Отступы у многострочного аргумента `return` не делают его новым
исполняемым телом. Только явное использование в роли тела либо own-bind
выполняет соответствующее правило, а не факт нахождения внутри скобок.

## 12. Checkpoint: точные наблюдаемые действия

Перед исходящим вызовом LMX, перед внешней/повторно входящей/публикующей
границей, перед выходом из метода и перед `yield` публикуются **только dirty
собственные значения**. Порядок — порядок полей. Успешно записанное поле
становится clean. Параметры без соответствующего bind автоматически не
публикуются.

Если store checkpoint-а не удался — **диагностический assert**. Предстоящий
вызов после проваленного checkpoint не выполняется. Не вводятся rollback,
новый выбор error carrier или бесконечный повтор падающего cleanup.
Это относится ко всей обязательной записи checkpoint: если её внутренний
lookup не может получить корректный слот для уже связанного dirty-поля,
публикация не выполнена и также используется assert, а не успешный `return 0`.
Проверка совместимости выбранного получателя до входа в новую активацию —
отдельная стадия admission; её нельзя переносить задним числом на checkpoint.

После вложенного вызова автоматической перезагрузки own-cache нет:

```text
outer загрузил x=1, cache clean
inner явно записал в граф x=2
outer не присваивал собственному x и выходит
результат в графе остаётся 2: clean cache ничего не публикует

если outer затем выполнил x=3, cache dirty
следующий checkpoint публикует 3
```

Нельзя заменить это flush-all. Нельзя автоматически искать новое `[0]` после
вложенной записи. Число полей не меняется; ссылки полей изменяемы, в том числе
на адрес другого типа. Используется обычная обработка записи/типа, без особой
ошибки «вложенный код удалил активное own occurrence».

## 13. `@` без дополнительных смыслов

`@` — взятие адреса, разрешённое в L2 и запрещённое в L3. Оно не переносит
владение и не продлевает жизнь адресата.

Нужно различать уже согласованные адресаты выражения:

- у собственного graph-backed поля это адрес выбранного payload, не скрытого
  C-cache;
- у несвязанного declared/through параметра это C-адрес самого параметра;
- после same-name own-bind параметра это всё ещё адрес той же C-переменной.

Запись через взятый адрес действует на указанный объект. Например, запись
в payload поля не делает отдельную C-local dirty автоматически и не вызывает
её reload. Если иной receiver действительно изменил рабочую переменную,
lowering должен отразить именно эту выполненную запись.

Стабильность живого адреса не означает бессмертия памяти. Обычные ошибки
использования освобождённой памяти не устраняются оператором `@`. Нельзя
переименовать этот факт в новый механизм владения L2.

## 14. Return, throw, assert, retry и yield

Согласованная физическая форма объявленного результата/throw: **status return
плюс типизированные out-параметры** результата и throw-payload. Хранилище
принадлежит активации/Message. Declared throw и runtime failure различаются.
Если вызов бросил исключение, обычный результат присваивания не записывается.

`longjmp` относится к диагностическому assert, а не к обычному declared throw.
Процесс-статический `throw_code` или фиксированный общий payload из старого
`lm2` не являются ABI нового ядра.

Диагностический маршрут генератора (stderr-строка, затем политика
`lmx_msg_poll_abort`, никогда не тихий успех) покрывает: публикацию checkpoint
(SPEC 21.6 / ABI 14.23) и открытие библиотечного юнита — каждый неудачный вызов
публичной обёртки печатает `lmx: library open failed: <unit> <operation>`; без
abort обёртка возвращает прежний ноль, отдельного кода для произвольного типа
результата не вводится (решение e2 2026-09-14, обратимо).

Выход выполняет в порядке: вычислить и удержать результат/диагностику один раз;
dirty-checkpoint перед cleanup; нужные `finally`; финальный dirty-checkpoint
изменений cleanup; передать исход и убрать активацию. Ранее выполненные
изменения графа и внешние эффекты не откатываются автоматически.

`retry` — локальный переход управления: сохраняет активацию, не создаёт
вызова и отдельного checkpoint только из-за этого перехода. Достигнутые затем
вызовы и выходы имеют обычные границы.

`yield` публикует dirty-поля и сохраняет текущую активацию для продолжения.
Backend может удержать стек или эквивалентную приватную continuation. Он должен
восстановить рабочие значения и dirty-состояние, а не загрузить всё заново из
графа. Если continuation переживает end-turn, её живые ссылки удерживаются
как корни. Из этого не следует обязанность bootstrap произвольно приостанавливать
любой native-вызов или переключать детей внутри незавершённого turn.

## 15. Арифметика, физические размеры и пределы

Арифметика следует native-поведению выбранного backend. Для C это поведение C,
включая unsigned-модульную арифметику `u64`, ширины типов и собственные пределы
переносимости C. Не добавлять новый универсальный режим checked/wrapping
арифметики языка. Отличия другой VM остаются её native-отличиями.

Проверки размера выделения памяти, переполнения `count * stride`, диапазона
индекса и отказа ресурса — отдельные инженерные обязанности. Отказ должен
сообщаться, а не превращаться в усечённые данные или успешный пустой результат.

16 записей, 1040 байт, 64 или 128 детей не являются правилами ядра. Начальный
размер chunk и реальный предел представимого `int`/`size_t` — разные вещи.
Рост добавляет необходимое хранилище. У каждого оставшегося фиксированного
прототипного массива должно быть ясное назначение, наблюдаемый отказ и запись
в инвентаре миграции; нельзя выдать его за топологию языка.

# Часть II. Копирование графа и память

## 16. Что именно делает merge

`merge` выполняется там, где до него дошло исполнение. Результат получает
прямые поля операндов в порядке перечисления операндов и полей внутри каждого
операнда, затем поля собственного тела merge. Корни операндов не становятся
дополнительными видимыми детьми результата. Повторяющиеся исходные имена
остаются отдельными позициями. Число прямых полей результата определяется
этим составом, а не числом объектов, потребовавшихся для копирования.

`result.node` указывает на Structure, в теле которой находится receiver merge.
Внутри переносимого графа ссылки и `node` **явно переписываются на копии**.
Это не означает, что любой скопированный объект получает result своим node:
внутренние лексические отношения восстанавливаются через карту копирования.
Нельзя спутать отдельно заданного родителя нового результата с родителями
всех объектов, включённых в его используемое окружение.

Корень операнда копируется, когда он входит в используемое замыкание: например,
на него ведёт явная ссылка или node нужного вложенного объекта. Отсутствие
операндного корня среди прямых полей R не доказывает его недостижимость.
Если ни одна требуемая связь не использует корень, один лишь факт перечисления
операнда не вводит дополнительный обязательный живой root в R. Helper может
подготовить промежуточную копию; это ещё не основание постоянно удерживать её.

Копируется полное **используемое** дерево/граф тем же обходом, что при создании
Message. В него входят нужные поля, их значения и ссылки, содержимое массивов
и лексические зависимости с проходом `node` до нуля. Если используется всё
дерево, копируется всё дерево. Нет запрета на большое копирование. Если
анализатор не может доказать ненужность части допустимых зависимостей, он
не вправе молча выбросить их и оставить обращение к старой arena.

Слово «используемое» не означает «прямые дети одного операнда». Также оно не
означает безусловно весь heap исходного процесса. Это достижимое замыкание
нужных данных и лексического окружения операции. `independent` даёт явную
границу лексической цепочки, а не эвристику с ограничением глубины.

## 17. Один алгоритм копирования для merge и создания Message

У операции есть набор исходных корней/путей и одна карта
`исходный адрес → адрес копии`. **Карта общая для всей операции**, включая
все операнды merge. Последовательное независимое копирование каждого операнда
неэквивалентно: оно разорвало бы общие ячейки и общих лексических предков.

Объясняющий алгоритм:

1. Определить тип исходного target по его typed range. Нулевую ссылку сохранить
   нулевой. Для внешнего ресурса применить его явный контракт.
2. Общий дескриптор метода и явно допущенную вечную ветку сохранить по прежнему
   адресу как терминалы копирования; правила этих двух случаев различны.
3. Если обычный исходный адрес уже есть в карте, вернуть существующую копию.
4. Выделить место для объекта соответствующего типа в подготавливаемом
   хранилище назначения. Сразу записать пару адресов в карту.
5. Скопировать scalar payload либо подготовить длину/слоты/backing массива.
   Обойти все требуемые reference edges и `node`; записать полученные адреса
   копий. Запись в карту до рекурсии делает циклы конечными.
6. После успешного построения опубликовать подготовленное хранилище и результат.
   При отказе освободить подготовку и сообщить установленный исход ошибки.

Это описание не требует C-рекурсии. Рабочая очередь/стек и карта должны расти
по необходимости; фиксированная глубина 16 или таблица 128 не являются
реализацией общего алгоритма. Проверки переполнения и нехватки памяти нужны.

Одинаковые исходные адреса дают одинаковые адреса копий в пределах операции.
Разные mutable-объекты не склеиваются только потому, что их payload равен.
Циклы сохраняются. Обычные скопированные mutable-объекты имеют новые адреса,
а исходные объекты и их `node` не изменяются. Эти требования совместимы:
«не портить источник» не означает «оставить node копии указывающим в источник».

Чужой raw pointer нельзя автоматически считать безопасным терминалом лишь
потому, что его нет в одном переданном наборе ranges. Это может быть ошибка
инвентаря собственного графа. Для native handle/foreign memory нужен явный
контракт ресурса; для обычного LMX target требуется корректное копирование.

## 18. Примеры идентичности при копировании

**Число.** В `A.x` лежит указатель на INT-ячейку со значением 1. После
`R: merge: A` поле `R.x` указывает на новую INT-ячейку со значением 1.
Запись 5 в ячейку через `R.x` не меняет `A.x`. Этот результат обеспечен
глубоким копированием, а не требованием превращать каждую числовую запись
в создание новой ячейки.

**Общий target двух операндов.** Пусть `A.x` и `B.y` до merge указывают на
одну mutable-ячейку p. После одного merge обоих операндов соответствующие
поля указывают на одну копию p', отличную от p. Два отдельных вызова copier
с отдельными картами дали бы p1 и p2 и нарушили бы исходную связь.

**Лексическая зависимость.** Callable Structure M находится в S и использует
поле предка P. Обход копирует M, нужные S/P и дальнейшее окружение до нуля.
M'.node ведёт к S', S'.node — к P', без ссылок на исходные S/P. Физический
child[0] M' сохраняет METHOD. При выборе M' через S' первый аргумент — M';
лексический путь идёт по скопированному окружению.

**Цикл.** X содержит ссылку на Y, Y — на X. Сначала регистрируется X', затем
Y'. При обработке обратной ссылки карта уже содержит X'. Получается цикл
X' → Y' → X', без бесконечного обхода и ссылки назад в исходную arena.

**Два разных запуска.** Общая карта обязательна внутри одной операции.
Две независимые операции создания Message имеют свои карты и свои mutable
копии. Общие методы и допущенные вечные ветки остаются общими в обоих случаях.

## 19. independent и граница лексического окружения

`independent` при построении задаёт корню ветки `node=0`. Внутри ветки
структуры сохраняют обычные лексические связи. Внешнее текстовое окружение
не становится доступным через fallback выше этого нуля.

Метод такой ветки может использовать её собственные поля, получать явные
и динамические входы от вызывающего и вызывать функцию с известной сигнатурой,
даже если конкретная реализация выбирается во время исполнения. `independent`
не означает pure, immutable, отсутствие Message-операций, запрет ссылочных
аргументов или обязанность статически раскрыть все вызываемые функции.

Этот receiver позволяет не тянуть ненужное большое внешнее окружение при
копировании. Он не делает маленьким большой массив, явно переданный аргументом,
и сам по себе не разрешает разделять mutable-объекты между Message.

## 20. Вечные ветки и массив первого Message

Согласованное отдельное правило SPEC 9.1.4:
**`independent: const: immutable`** помещает построенную ветку в вечное
хранилище, удерживаемое первым, корневым относительно ОС Message.
У него находится **массив ссылок на все такие ветки всех Message запущенного
процесса ОС**. Набор известен при трансляции. **Сам массив тоже const: immutable;
в runtime он не растёт и не пополняется.** Это не один обязательный корень Lmx,
не общее лексическое дерево и не только ветки собственной прикладной Structure
первого Message. Merge/create не регистрируют в массиве новые ветки.

У корня каждой такой ветки `node=0`. Место объявления держит свою ссылку;
другие места получают явно переданные ссылки. Добавление ссылки в массив
удержания не меняет `node` ветки и не делает первый Message её лексическим
родителем. Внутренние node ветки остаются обычными.

После публикации ветка и её удерживаемое хранилище неизменяемы, не перемещаются
и живут **до завершения процесса ОС**. Исчезновение обычных ссылок, завершение
Message-заёмщика и его сборка мусора не освобождают ветку. Если допустимая
инициализация использует runtime-значения, она заполняет заранее известное
хранилище до публикации; это не добавление записей в массив при каждом вызове.
Известный при трансляции набор, инициализация и immutable lifetime — разные
вещи. Одинаковое содержимое не требует interning. Runtime-рост этого массива
из прежней формулировки исключён.

Все ссылки в опубликованной ветке должны оставаться действительными этот срок.
Квалификация не превращает адрес освобождаемого объекта в вечный. Массив
удержания принадлежит корневому Message, оставаясь const: immutable. Он не
является изменяемым списком регистраций. Получатель одной ветки не приобретает
доступ к массиву, настройкам корня или другим его веткам.

При merge и создании Message **явно допущенная ссылка на вечную ветку остаётся
по тому же адресу**, обход не копирует её содержимое и автоматически не импортирует массив
удержания. Это терминал копирования внутри процесса. Отдельные `const`,
`immutable` или `independent` не дают такого права. Передача на другую машину
по-прежнему требует codec: адрес RAM не является сетевой идентичностью.

Пример: A содержит обычное mutable x и вечную E. Новый B получает собственную
копию x, но ту же E. A и B завершаются — E остаётся. Их обычные поля не стали
вечными и не получили общее владение от соседства с E.

Это согласованная модель; текущий прототип ещё не реализует весь механизм.
Общие дескрипторы методов не следует оформлять как mutable-поля или копировать
с node.

Первый, корневой относительно ОС Message владеет ДВУМЯ отдельными массивами
const: immutable: один удерживает заранее известные ветки independent: const:
immutable, второй содержит известные дескрипторы методов. Дескрипторы живут до
завершения процесса ОС; завершение исходного или заимствующего дочернего Message
не освобождает их. Это хранилище корневого Message, а не безвладельный global и не
arena каждого вызывающего ребёнка. Merge/создание Message сохраняют допущенные
ссылки на вечные ветки и адреса дескрипторов методов как два отдельных случая.
METHOD по-прежнему не хранит node; размещение дескриптора не меняет аргумент
собственного узла callable Structure.

Транслятор строит исходный граф первого Message и заполняет два его массива
const: immutable. Само это построение не порождает дальнейшие Message.
Последующее создание Message — явная операция копирования всего нужного общим
обходом: вход может явно содержать ссылку на весь допущенный immutable массив,
отдельную квалифицированную ветку, METHOD или системное значение. Передача
одной ветки не выдаёт автоматически её массив удержания и настройки корня.
Ребёнку не требуется глобальный accessor поиска первого Message. В текущем
эмиттере process_message уже обозначает первый Message, поэтому выделение
дескрипторов в его storage корректно по владельцу. Незавершённая работа — форма
двух выделенных массивов, emission квалификаций и согласованные метаданные
копирования, а не перенос записей из дочерней arena и не ожидаемый API Grok.

## 21. Ошибка merge и атомарность подготовки

На уровне языка ошибка выражается **`throws merge(args)`**. Нельзя вернуть
нулевой/пустой обычный результат как будто merge успешно закончен. Backend
должен провести ошибку через статус и типизированное хранилище throw.
Внутренние коды copier OK/INVALID/NOMEM/UNSUPPORTED не являются новым
публичным языковым ABI и должны правильно отображаться на него.

Частично построенный результат не публикуется. Исходные графы не портятся.
Подготовленные allocations и карта либо передаются назначению после успеха,
либо освобождаются при отказе. Это гарантия подготовки конкретной операции,
а не транзакционный rollback всех действий программы перед merge.

Если frontend ещё не умеет typed throw данного выражения, он должен явно
отклонить пока неподдержанный lowering. Он не может скрыть пробел через
`return 0`, C longjmp для обычного throw или постоянный успешный статус.

## 22. Arena, typed ranges и стабильные адреса

Один Message владеет одной логической arena. Она может состоять из многих
блоков и typed chunks. Для типа T может быть несколько диапазонов; проверка
принадлежности адресу должна видеть все актуальные диапазоны своего владельца.

Живые L1/L2 узлы, ячейки и дескрипторы **не перемещаются**. Рост добавляет
блоки; collector освобождает недостижимое, сохраняя адреса живого. Нельзя
ввести moving GC или realloc живого typed pool с последующим обещанием
«перепишем все ссылки»: обычный `@` мог уже отдать адрес native-коду.

Глубокая копия создаёт другие объекты. Передача владения блоками меняет
владельца существующих объектов. Ни одна из этих операций не является
перемещением живого объекта внутри его arena. Изменение backing конкретного
Array допустимо лишь по контракту операции над ним; это не разрешение
сдвигать сами descriptor/cell адреса в typed pools.

Рабочая карта копирования и очередь/стек обхода могут перераспределять свои
приватные буферы: они хранят адреса стабильных объектов и не опубликованы как
живые LMX targets. Nonmoving-правило относится к адресуемому хранилищу языка,
а не к любому временному C-буферу аллокатора, copier или collector.

Длина массива и размер выделенного backing — разные величины. Количество
объектов в block/range также не равно числу прямых детей Structure.
Недопустимо снова выводить 1040 байт для строки из размера старого узла.

## 23. Передача владения без копирования

Локальная доставка или диагностический handoff могут передавать уже
подготовленные блоки/ranges в arena получателя без второго глубокого копирования.
При этом адреса и node объектов сохраняются, потому что передаются **те же
объекты**. Это отдельная операция от merge/создания нового независимого графа.

До передачи необходимо установить единственного владельца и безопасную
границу доступа; после передачи старый владелец не продолжает обычные
мутации этих блоков. Перенос списка blocks и typed ranges должен быть
согласованным: получатель обязан классифицировать все переданные targets.

Владение block не делает каждый объект этого block корнем GC. Получатель
сохраняет только реально требуемые корни графа/диагностики. Заимствованные
общие методы и вечные ветки не освобождаются как его обычные owned blocks.

## 24. Сборка мусора и корни

Сборка локальная, nonmoving. Основная граница — **один end-turn**, после
успешной публикации outbox либо его отбрасывания при неуспехе и снятия
временных корней завершённого turn. Обычный возврат метода, retry и пойманный
throw сами по себе не являются отдельными GC-точками. Запрос из-за давления
памяти обрабатывается на соответствующей безопасной end-turn границе.

Корни включают живой граф Message, активные входы, удерживаемые own occurrences,
результаты/throw, continuation, явно retained значения и необходимые
планировщику handles. Конкретный список зависит от фазы; нельзя забыть
результат перед finally или активный вход до публикации checkpoint.

Tracer классифицирует **target** каждой ссылки: STRUCT требует обхода своих
слотов и node, primitive cell помечается как ячейка, Array — descriptor,
backing и его ссылочные элементы по типу. METHOD и допущенная eternal-ветка
обрабатываются как общие терминалы с внешним длительным удержанием.
Нельзя читать `target->node` у INT или у descriptor массива.

Недостижимые циклы могут быть освобождены; достижимый цикл должен завершать
mark через visited-состояние. Нехватка памяти у рабочего стека mark не даёт
права освободить ещё не посещённый потенциально живой объект. При ошибке
нужен безопасный отказ/сохранение живого, а не частичный sweep как успех.

Message-local interning обычных значений не отменяет владение. Оно не делает
любой immutable LMX target общим между Message. Вечные ветки и общие методы
имеют свои явные правила выше. Контракт foreign handle также задаётся явно.

# Часть III. Механизм Message

## 25. Что представляет собой Message

Message — единица владения обычным графом, последовательного исполнения его
turns и взаимодействия через сообщения. Его управляющая политика выражается
состоянием L2 Structure; native-часть предоставляет необходимые атомарные
операции, очереди, блокировки, wait/wake и исполнение. Нельзя подменить это
внешним процесс-статическим C-менеджером прикладной политики.

Михаил, 2026-09-15, дословно: «Одна арена на Message, арены присоединяются
при потреблении -- это ядро».
Он же, тот же день, дословно, о службах поверх ядра: «если myxa_manager, к примеру, нужен почтовый сервис как место где перемешиваются все письма и клиенты забирают что хотят -- он может его сделать его для себя базовыми механизмами. Для ядра такое не нужно» (подтверждено
координатором: такой сервис — обычный L3 Thread приложения на базовых
механизмах; спецификация 19.28.R2.2: необязательный маршрутизатор — сам
Message, координатор владеет своим состоянием и получает запросы).

Разделять четыре отношения:

| Отношение | Что оно определяет |
| --- | --- |
| `Structure.node` | Лексических предков для lookup |
| Граф ссылок данных | Доступные значения и зависимости |
| Arena ownership | Кто удерживает/освобождает обычное хранилище |
| Родитель и дети Message | Запуск, наблюдение, завершение и передачу диагностики |

Отображение на потоки ОС — пятое, исполнительное отношение. Ни родитель
Message, ни поток ОС не становятся автоматически `node` пользовательской
Structure. Установка готового `graph` в контексте Message сама по себе лишь
запоминает ссылку; глубокое построение входного графа — отдельная операция.

Первый Message соответствует корню процесса ОС. Настройки принадлежат ему;
обычные требуемые настройки попадают к детям через явные шаблоны/копирование.
Его массив вечных веток удерживает ветки по §20, но не открывает детям
неявный доступ к общему изменяемому состоянию корня.

Уточнения этапа 5 (2026-09-14, чат ревью; выведены из 19.29.6, 9.1.4 и 1.7
спецификации для дизайна лида, новых решений не потребовали): (i) рантайма
без корня не бывает: создание рантайма создаёт корневой Message R0 (адрес 1,
единственный Message без родительской capability); «рантайм без корня для
встраивания» не существует, потому что встраивающая программа находится вне
LMX и достигает рантайма только через admission почты R0; (ii) точка входа
процесса (1.7) исполняется внутри turns самого R0: Message первого потока
исполнения — это R0, его начальное сообщение несёт настройки и
пользовательский ввод, полученный на входе, а программа входа — обработчик
R0, а не порождённый для неё ребёнок (9.1.4: транслятор не порождает
Messages неявно); (iii) от «хоста» вне turn ничего не остаётся: все
полномочия, которые L1-bootstrap давал хосту (ingress, drive, шаг полосы UI,
шаг родителя над Messages верхнего уровня, логические часы, teardown),
принадлежат циклу turns R0; исторический host ingress V0 не нормативен;
(iv) верхний уровень один: Messages, созданные без родителя, становятся
детьми R0; сироты перекореняются под R0, сохраняя политику удержания
(19.29.8: R0 — та полоса, которая назначает срок неуспешному сироте и
освобождает его; арену сироты никто не усыновляет); полоса UI из 19.28.R2.2
— ребёнок R0 (о её потоке — слова Михаила 2026-09-15 в §29: полоса на экране
не может не быть L3 Thread; прежняя запись «отображённый на полосу R0,
главный поток» была выводом ревью-чата и снята); (v) передача надзора
от R0 виртуальному прародителю World Wide Mix остаётся отказывающей
заглушкой со своим статусом, пока не реализованы 19.28.10 и транспорт §35;
этап 5 фиксирует только место вызова и адрес прародителя.

Уточнение (Михаил, 2026-09-14). L3 Thread — это Message: Message — просто
изолированный LMX-граф, а L3 Thread — Message, который может присоединять
и/или исполнять любой Message. Каждый L3 Thread — Message, но не каждый
Message — L3 Thread (у шаблона и у письма нет turn, у запущенного ребёнка
есть). Почта и задел под turn есть только у L3 Thread — даже если его
исполняют снаружи и свой поток не запущен. L3 Thread запускает Messages;
быть запущенным — не обязательное свойство Message. Планировщик есть только у
разновидностей L3 Thread: он выделяет исполнение присоединённым Messages
(потребитель сам планирует полученные Messages после их присоединения) и
снимает по тайм-ауту по проверкам от детей; запущенными детьми он не
управляет полностью — в нормальном режиме дети сами себя закрывают и так же
планируют своих детей.

## 26. Создание дочернего Message

Родитель в своём turn резервирует дочерний Message, его идентичность и отдельную
arena. Граф начальных данных строится общим с merge алгоритмом, с переносом
обычных mutable-значений и лексического окружения. Общие методы и явные вечные
ветки сохраняют свои адреса.

Дочерний путь расширяет путь родителя локальным счётчиком; первое значение — 1.
(Идемпотентность повтора create по `(parent, create_id)` снята распоряжением
Михаила 2026-09-15: «убирайте».)

До успешной публикации родительского turn ребёнок не становится доступным
для обычного исполнения входящих. При неуспехе родителя неопубликованные
резервации/подготовленное владение убираются. Статусы управления описывают
назначенную работу, поэтому начальное `running=1, success=0` не означает,
что ребёнок уже запущен на потоке ОС.

Создание не должно оставлять в новой обычной arena скрытых ссылок на mutable
payload исходной arena. Запуск ребёнка после teardown исходного шаблона —
существенная проверка копирования и длительности жизни общих методов.

## 27. Отправка, публикация, приём и выполнение

Нельзя считать одним событием эти разные шаги:

1. Текущий turn готовит исходящие сообщения в своём outbox.
2. Успешное завершение turn публикует подготовленный outbox.
3. Транспорт доставляет элемент и приёмник линеаризует admission в своей очереди.
4. Планировщик выбирает Message и один вход для следующего turn.
5. Обработчик выполняет turn; его результат может ещё не завершать назначенную
   Message операцию целиком.

Михаил, 2026-09-15, дословно, об отправленном Message: «У нас нет никаких
копий message. Есть только он сам и у него своя арена. Пускай это будет один
alloc но это его собственный. При получении Message который не L3 Thread
присоединяется к арене получателя. Всё».

Capability у Message, не являющегося родителем цели, после закрытия цели
(согласовано с Михаилом 2026-09-15, «давай так», в этой формулировке):
capability — это дескриптор почтового ящика цели; ящик закрывающегося
Message осаживается в родителя вместе с остальным его хранилищем, так что
поздняя отправка попадает в ящик, которым владеет родитель, и отвергается
там, на полосе родителя, статусом отправителю; счётчика держателей нет.

FIFO задаётся порядком admission у конкретного получателя. У двух одновременно
отправляющих до линеаризации нет заранее заданного общего порядка. Доставка
не означает обработку, а окончание одного обработчика не обязательно означает
`success=1` всего Message.

Полученный локальный LMX-текст может перейти в arena получателя передачей
владения; не требуется второй deep copy того же доставляемого объекта.
Получение обычного сообщения не создаёт новый поток/Message. Для этого нужна
явная операция создания/запуска ребёнка.

Уточнение (Михаил, 2026-09-14). «Copy-only mailbox» — не отдельный механизм,
а пересказ того, что merge делает копию и у Message своя arena; это верно
для каждого Message, сущности «конверт» нет, обёртка Message одинакова у
исполняемых и неисполняемых. Изоляция Message — лестница: merge (одна копия в
своей arena) и валидация при admission; при необходимости транспорта —
аутентифицированное шифрование (уровня libsodium) передаваемых байтов;
вершина — N процессов ОС, где управляющими флагами ведает транспорт. L2 видит
весь C через `c.`; только L3 не видит C.

Текущий байтовый/числовой прототип send → copied outbox → transport → fresh
inbox → материализация `recv` — этап реализации. Нельзя объявлять эти
промежуточные byte copies полной реализацией передачи произвольного LMX-графа.

## 28. Turn и границы эффектов

В один момент Message выполняет не более одного своего полного turn.
Turn принимает не более одного входящего сообщения. Он может выполнить много
обычных вызовов, изменить собственный граф, подготовить outbox и создать детей.

При успехе публикуются исходящие и подготовленные дети. При неуспехе
неопубликованный outbox отбрасывается. Затем освобождаются корни завершённого
turn и выполняется локальная сборка. Это не автоматический rollback ранее
изменённого графа, native I/O или уже опубликованного предшествующего turn.

Checkpoint перед вызовом публикует dirty-own в граф внутри Message. Публикация
outbox на end-turn — другая граница. У них разные объекты и наблюдатели.
Не добавлять внешний вызов до обязательного успешного checkpoint.

Exactly-once для произвольного внешнего эффекта не возникает из FIFO или
дедупликации сообщения. Таблица обработанных идентичностей должна иметь
описанный контракт; нехватка её памяти не разрешает молча забыть запись
и продолжать обещать ту же идемпотентность.

## 29. Планирование детей и потоки ОС

Каждый родитель обслуживает своих **непосредственных** детей. У ребёнка есть
собственное обслуживание, даже если у него нет детей. Нет требования к одному
глобальному обходчику всех Message процесса.

Родитель P может выполнить ребёнка A на своём физическом потоке. A при этом
может распределить собственных A1/A2 на другие потоки. Родство Message
не задаёт отношение «один Message = один поток ОС».

Соседи на одном исполнителе обслуживаются последовательными полными turns.
Модель не требует обязательной вытесняющей паузы посреди turn, глобального
round-robin или новой универсальной гарантии fairness. Длинный native-вызов
нельзя сделать прерываемым одним упоминанием `running`.

Доступ к изменяемой таблице детей требует короткого получения устойчивого
snapshot: удержать нужные handles, отпустить структурную блокировку, сделать
потенциально блокирующее обслуживание, затем при необходимости проверить,
что наблюдавшаяся идентичность всё ещё актуальна. После этого снять pins.
Фиксированный snapshot на 64/128 не является ограничением дерева Message.

Уточнение (Михаил, 2026-09-14). Одна arena — одна полоса — один писатель:
состояние планировщика и управления Message пишется только на полосе этого
Message; ни один другой поток в данные Message не пишет, и это верно для
C-рантайма так же, как для L2, потому что это свойство Message, а не системы
графов. Родитель не держит списка, в который дописывает чужой поток:
ребёнок, вписывающий себя в список готовых родителя со своего потока, и
UI-поток, пишущий в списки многих родителей, — ошибки реализации прототипа
L1, которые убираются из него, а не переносятся в L2. Запрос к другой полосе
(отобразить ребёнка на UI-полосу) — сообщение в почту той полосы; лок
admission почты — единственная межполосная синхронизация, которую допускает
модель.
Владение ячейками для правила «пишет только полоса владельца» (2026-09-14):
(1) Message владеет своей arena, своей записью планировщика и курсором,
своей почтой кроме admission, своими
native-пользователями и отметками handoff-safe, своим success и снятием
своего running на end_turn; его полоса — тот поток, который держит его turn,
так что ход, ведомый хостом, — полоса этого Message. (2) Родитель владеет
ячейками надзора за прямыми детьми, где бы они физически ни лежали:
committed, tracked, время последнего ответа, отображение исполнения при
привязке (поэтому право привязать, перепривязать или отвязать Message — у полосы
его родителя либо у хоста вне turn, пока bootstrap остаётся хостом; Message
не отображает и не снимает отображение сам, и turn сиблинга этого не делает;
отвязка при освобождении — запись осаживающей полосы: родителя, а для сироты
— хоста; уточнено 2026-09-14), семейная цепочка (первый и последний ребёнок, ссылки сиблингов,
ссылка на родителя при link/unlink/передаче надзора) и записи при settling
(хранилище, переезжающее в родителя, graph и init ребёнка). (3) Управляющие
флаги — одиночные ячейки с одним назначенным межполосным писателем, по
философии флагов: родитель пишет ребёнку running=0 и closing (stop).
Что убирает это правило: списки готовых и списки контекстов
родителя, в которые дописывали полоса ребёнка, отправителя или UI-поток;
ребёнок, вычёркивающий себя из списка родителя; вместо этого шаг родителя
читает собственные ячейки детей.
Михаил, 2026-09-15, о флаге готовности ребёнка, который называл этот абзац
(писал отправитель при admission, читал шаг родителя), дословно: «"готовность ребёнка его собственным" --готовность к чему?! Удаляй» —
флаг и его фразы выше сняты; почта Message — то, что его собственный цикл
находит в его ящике. В тот же день об истории доставки, которую держал
рантайм L1 (последние id по отправителям, отбрасывание повторов), дословно:
«"ящик помнил последние id от каждого отправителя и отбрасывал повторы («не более одного раза»)." -- какая-то выдумка. Удаляй все.» — в ядре такого механизма нет.

Уточнение (2026-09-15, Михаил, дословно): «Там не должно быть общих
блокировок. У очереди писем только своя внутренняя блокировка коллекции,
всё»; «блокировка только локальная у коллекции сообщений может быть, больше
блокировокне требуется даже локальных -- их в проекте не длолжно быть больше
вообще»; «значит реализуйте полноценную арену для каждого MEssage, как
описано в спеке. Без этого Message не имеет смысла» (arena — 19.29.2). В тот
же день по его распоряжению удалён прежний пункт (5) списка владения выше
(структуры уровня рантайма под локом исполнителя) — его записал 2026-09-14
чат ревью как собственное следствие, это никогда не были слова Михаила.

Уточнение (Михаил, 2026-09-15) о закрытии родителя, у детей которого нет
своего потока, дословно: «Закрытие родителя это авария. Каждый ребенок
закрывает себя сам и для его детей это такая же авария. Если дети L3 Thread
но реализация такая что у них нет своего потока, а они последовательно
выполняются, то это нюансы реализации. Разумеется в этом случае закрытие
родителя затягивается так как планировщик должен пройтись и выполнить чужую
работу -- но зачем вам вообще сейчас такие потоки если вы не можете даже
ядро архитектуры запилить точно по спеке?» Подтверждено им в тот же день
(«да») в такой формулировке: сейчас каждый L3 Thread исполняется на своём
потоке ОС, а последовательное отображение (дети без своего потока, которых
шагает поток родителя) из рантайма убирается, пока ядро не сделано точно по
спецификации.
Подтверждено Михаилом в тот же день («да») в такой формулировке о структурах
уровня рантайма, которые реализация L1 держала под локом исполнителя: списка
корней нет (корень один, R0, его дети — обычный список детей в арене R0);
обхода при удалении нет (закрытие — собственный закрывающий конец turn R0);
список слотов служил только циклу удаления L1 и поиску по адресу и уходит без
замены; очередь retire уходит (хранилище закрытого ребёнка — запись родителя
при осадке); очередь полосы — это почтовый ящик Message этой полосы.
Михаил, 2026-09-15, дословно: «Синхронизация только локальная в почтовом
ящике каждого L3 Thread -- больше в проекте нет (за исключением поддержки
ресивера "synchronized")».
Михаил, 2026-09-15, дословно, об имитациях механизма Message в коде (второй
рантайм потоков сообщений на локах в lm2/own.lm2 и каркас, который эмитировал
транслятор lm2): «да, все имиации из кода немедленно убираем. Это не L2 код если он так себя ведет. Переводите его в L1 и помечайте в комментариях что механизм MEssage временно не используется. На самом деле пользовательский L2 может синхронизировать что хочет _но мы пишем в своей филососфии Message чтобы потом переводить на L3_».
Михаил, 2026-09-15, дословно, о полосе UI (до того — Message без своего
потока, которую шагал turn корня; «это просто кривая реализация»): «Полоса на экране не может не быть L3 Thread. Message без L3 Thread это просто письмо. Полоса же это полноценный объект. Объектов у нас не так много как в SmallTalk но все равно много. Поэтому реализация L3 Thread планировщика который не запускает потоки детей а бегает по ним сам -- ближайшая задача для myxa_manager».

## 30. Блокировки и безопасное наблюдение

Message синхронизирует только admission своего почтового ящика; таблиц
исполнения нет. Прикладным модулям не нужен второй scheduler и никакой
глобальный lock. (Согласовано с Михаилом 2026-09-15; его слова: «блокировка
только локальная у коллекции сообщений может быть, больше блокировок не
требуется даже локальных -- их в проекте не должно быть больше вообще»; на
вопрос о прежней редакции этого раздела, описывавшей «таблицы исполнения» и
их короткую блокировку: «да, все описания L1 промежуточны и их нужно
приводить к спецификации L2». Тот же день, дословно: «сделайте синхронизацию
в почтовых коллекциях в точности как это делается в Java, причем максимально
упрощенно (ресивер synchronized в спеке описан), в старом проекте он был даже
до какой-то степени реализован C:\Nyasha_Planet\lingvamyxa_old_worked_version».
Он же, тот же день, дословно: «тупейший вложенный synchronized по локу на чтение и запись как в Java, без остальных усложнений вообще. "notify получает не «поток», а поток-владелец ящика" -- ну а смысл его нотифицировать если он занят в turn?!!! Выйдет и посмотрит».)

Для чтения родителем ячеек своих детей: получить и удержать нужные объекты,
посмотреть mailbox/control, заново проверить существенное состояние перед
действием. Параллельное закрытие не должно превратить snapshot в dangling
handle или позволить исполнить уже отозванный turn.

## 31. running, success и остановка

Михаил, 2026-09-15, дословно, на вопрос, завершается ли поток владельца при
пустом ящике после выхода из turn'а: «"завершается ли поток владельца при пустом ящике после выхода из turn'а" -- нет!!!! L3 Thread точно так же завершается только по success пользовательским кодом. Каким образом вы будете код писать иначе?». И на слова координатора «поток с
бесконечным циклом», в тот же день, дословно: «"поток с бесконечным циклом" -- цикл в L3 Thread очеивдно прерывается по success==1» — цикл владельца по
turn'ам идёт, пока success равен 0, и кончается, когда пользовательский код
ставит success в 1. И в тот же час, дословно: «success && !running, с выходом выставляется running = 0. Просто running опрашивается при выполнении Lmx* root и я же не знаю как кокнретный планировщик устроен и когда выйдет. Но условия выхода очевидно есть!» — условия выхода
есть: по модели ни один turn root не повторяется, когда success == 1, потому что
success относится к Lmx* root, к основной функциональности; что планировщик
L3 Thread делает после этого со своими детьми — реализация, так как дети могут
крутиться на том же потоке, как и место return (дословно, в тот же день, на слова лида «цикл выходит по
success == 1 — это описание реализации, а не модели»: ««цикл выходит по success == 1», а это описание реализации, а не модели." -- просто по модели L3 Thread не может повторять turn когда success == 1, это не деталь реализации. Деталь реализации это где будет стоять return.» и, сужая: «Однако планировщик L3 Thread может что-то делать со своими детьми (вот это зависит отреализации так как дети могут крутиться на том же потоке) так как success относится к Lmx* root -- то есть к основной функциональности.» и, закрывая вопрос: «Однако так как при остановке родителя останавливаются и дети (там в обе стороны проверки) то очеивдно что в конечном итоге по success==1 ыикл прервется. От реализации зависит только в каком месте» — раз дети
останавливаются с родителем, проверки в обе стороны, цикл в конечном итоге
прерывается по success == 1, а от реализации зависит только место); пользовательский код ставит
success, running опрашивается при выполнении root, при выходе ставится
running = 0; завершённый L3 Thread читается как !running, а success говорит,
завершился ли он успехом, дословно: «"завершённый L3 Thread читается как success && !running" -- если пользовательский код не поставит success=1 то success останется 0 если running стал 0». И:
«то есть цикл не бесконечный -- из него есть выход хоть по return хоть как». О круге
между turn'ами, дословно: «L3 Thread нечего делать на endturn???» — круг после turn'а есть собственная работа
L3 Thread на end_turn (надзор за прямыми детьми и их опрос, 19.29.7.1; закрытие
и settling, 19.29.8; обслуживание арены), затем взгляд в ящик; пустых кругов в
модели нет. И на слова
лида «running = 0 — запрос остановки от родителя, а не выход из цикла», дословно:
«L3 Thread это модель же а не реализация. Да, 0 это запрос остановки от родителя. Что значит "не выход из цикла"? Как можно остановить не выходя в конечном итоге из цикла?» — running = 0 от родителя есть запрос остановки, который в конечном
итоге выводит из цикла.

Начальное состояние назначенной операции: **running=1, success=0**.
Только фактическое успешное выполнение всей назначенной работы устанавливает
success=1, затем running=0. Нельзя ставить success от одного heartbeat,
пустого inbox, отсутствия физического потока или завершения отдельного turn.

Уточнение (Михаил, 2026-09-14). complete ставит только success=1; running=0
следует за success=1 только на границе turn, в end_turn. Внутри turn running
остаётся 1, поэтому running=0, увиденный внутри turn, — запрос остановки, а не
собственное завершение Message. Флаг running трогает тот, кто Message
исполнял: исполняемый на своём turn Message снимает его на своём end_turn;
обычное письмо, потребляемое присоединением, исполняется потоком и почтой
потребителя, и running=0 ставит потребитель на границе своего turn. В пути
письмо остаётся running=1, success=0; running=0 при success=0 на границе —
неуспешный исход, который родитель разбирает через границу handoff (§34).
Слова «success=1, затем running=0» выше читаются с этим порядком: две записи
разделены границей turn.

Кооперативная отмена устанавливает running=0, не выдумывая успешный исход
или диагностический failure graph. Этот флаг — запрос прекращения обычной
работы; сам по себе он ещё не доказывает quiescence, окончание native-вызова
и безопасность немедленно освобождать хранилище.

Код проверяет флаг на входе до загрузки own, на обратных переходах циклов
и при выходе, в безопасных точках без удерживаемых внутренних locks. Выход
покидает соответствующий корень исполнения Message, а не убивает общий OS worker.

Оптимизация чтения через relaxed `atomic_uint_fast8_t` допустима для выбранных
compiler/target/options, если проверено требуемое соответствие машинных
инструкций варианту volatile. Это не переносимое утверждение, будто plain
volatile даёт межпоточную синхронизацию C. Лишние RMW/fence/helpers не надо
вводить в каждую проверку без необходимости контракта.

## 32. Закрытие семейства

Закрывающийся Message снимает running у непосредственных детей и выполняет
свою необходимую очистку. Каждый ребёнок аналогично обслуживает собственных
детей. Из этого не следует обязательный сквозной обход всех потомков предком.

Внутренний `KIND_STOP` обслуживает протокол закрытия; это не обычное
пользовательское сообщение `KIND_CANCELLED` с искусственно выдуманным payload.
Уже выполненные эффекты обычных turns не откатываются из-за stop.

Спящий исполнитель должен получать wake либо выходить из ограниченного
ожидания, чтобы заметить закрытие. Непроверяемый зависший native-вызов требует
своего native-контракта; кооперативный stop не обещает его насильственно оборвать.

Уже выставленный running=0 у родителя не отменяет его роль lifecycle endpoint:
ему ещё может понадобиться принять окончательное состояние/хранилище ребёнка.
Освобождать управление и память надо после завершения этого протокола.

Уточнение (Михаил, 2026-09-14). Освобождённая ветвь не удерживается:
освобождение Message освобождает всё его поддерево и не ждёт завершения
детей. Все правила закрытия и освобождения следуют из трёх: (1) принудительное
освобождение ребёнка родителем запускает цепную реакцию — ребёнок так же
закрывает и освобождает своих прямых детей, и слоты с аренами ветви
освобождаются этой цепочкой, а не при завершении процесса; (2) самозакрытие
ребёнка по тайм-ауту при недоступном родителе запускает ту же цепочку для его
поддерева; (3) success не может появиться у родителя, чьи дети running=1 и
success=0; если его пользовательский алгоритм всё же выставил success, он тем
самым объявил работу детей ненужной, и та же цепочка закрывает и освобождает
их. Оговорка про lifecycle endpoint выше (родитель может сначала принять
итоговое состояние или хранилище остановленного ребёнка, §34) — шаг внутри
цепочки, а не основание держать слоты закрытой ветви. (4) Освобождения ребёнка
«в свободное плавание» нет: единственный способ сохранить Message с running=1
после закрытия его родителя — передать его надзор другому ЖИВОМУ родителю,
которого закрывающийся родитель выбирает среди Messages, чьи capability он
держит, и это не обязательно родитель закрывающегося (уточнено Михаилом
2026-09-14). Передача надзора переносит родительскую capability ребёнка и его
место в планировщике; arena, почта и turn остаются его собственными. Это
отличается от усыновления хранилища, которому по-прежнему нужен
неисполняющийся handoff-safe источник (19.29.7). Не переданный ребёнок
закрывается цепочкой. Для корневого Message единственный возможный новый
родитель — виртуальный прародитель World Wide Mix (19.28.10) на уровне
процесса ОС, то есть передача от корня означает «запустить процесс ОС»; это
отказывающая заглушка со своим статусом, пока не реализованы World Wide Mix
(19.28.10) и транспорт §35: дерево над корнем описано алгоритмически, не
реализовано; этап 5 (запуск ОС как корневой Message) фиксирует место вызова
и адрес прародителя (Михаил, 2026-09-14: заглушка до реализации World Wide
Mix). (5) Усыновление закрывает
усыновлённого (Михаил, 2026-09-14): передача хранилища при усыновлении арены
закрывает усыновлённый Message; он не остаётся живым рядом с переданным
хранилищем. Всё, что усыновивший затем породил из принятого содержимого, —
ребёнок усыновившего, того, кто обработал, а не усыновлённого. Развилка
строгая, и это и есть различие L3 Thread / Message из §25: Message либо
работает сам — тогда он L3 Thread (свой turn, своё место под тем, кто его
запустил), либо за него работают — тогда он просто Message (присоединён,
усыновлён, закрыт тем L3 Thread, который его обработал); у них разный генезис
и потому разное место в иерархии. Передача надзора (правило 4)
переносит живой Message; усыновление переносит хранилище и завершает Message.
Отсюда ровно два способа сохранить линию потомков после закрытия их родителя
(Михаил, 2026-09-14): (i) породить простой Message и отправить — то, что он
породит у приёмника, уже линия приёмника; (ii) породить L3 Thread и передать
его надзор в другую ветвь. Поддерево, с которым не сделано ни того, ни
другого, закрывается цепочкой. Три следствия правил (1)–(3) для
рантайма, читаемые вместе со спецификацией 19.29.8, где выбрана политика
settling (ревью-чат, 2026-09-14; в тот же день исправлено по 19.29.8):
(i) dispose родителем завершённого неуспешного ребёнка — это его settling по
19.29.8: рантайм передаёт логическую arena ребёнка родителю без копирования и
удерживает там корень неуспеха, затем освобождает слот ребёнка; родитель не
сбрасывает неуспешную arena и не обязан сам вызывать усыновление, а dispose
неуспешного ребёнка с хранилищем не отвергается; история успешного ребёнка
освобождается по умолчанию; (ii) освобождение не ждёт dispose детей по
одному: dispose или усыновление ребёнка родителем «оседает» цепочкой всё его
завершённое поддерево снизу вверх, как описано в 19.29.8 (arena G входит в C,
затем C — в P); остаются условия «ребёнок не бежит, handoff-safe, нет
native-пользователей»; (iii) потомок, ещё бегущий в момент освобождения,
закрывается цепочкой (флаг closing, как при любом stop) и становится
сиротой: успешный сирота оседает на своём end-turn, а освобождается, арена и слот,
осаживающей полосой на ближайшей точке обслуживания корня (сегодня — drive
хоста, с этапа 5 (c) — обслуживание корня между turns), одинаково на пути
хоста и на отображённом пути (уточнено 2026-09-14: одно место освобождения,
без удержания и без усыновления);
неуспешный удерживает handoff-safe arena под отдельным тайм-аутом потери
родителя из 19.29.8 и освобождает себя по его истечении; вызов родителя
возвращается сразу, ветвь в это время стекает, а не удерживается. Сирота на
время стока перекореняется у рантайма (Message верхнего уровня без родителя,
достижимый как любой корень, закрывающийся цепочкой); этап 5, делающий
запуск ОС корневым Message, делает его ребёнком этого корня. Дети
остановленного Message (уточнено 2026-09-14): у остановленного Message нет
полосы, поэтому с него ничьи turns не исполняются; его остановка запрашивает
закрытие детей; ребёнок с пустым inbox закрывается обслуживанием без turn,
ребёнок на собственном контексте исполняет свой закрывающий turn там; ребёнок
с ожидающим входом под остановленным родителем, отображённым на полосу
хоста, ждёт, пока родителя не выведет (dispose) его родитель, — тогда
цепочка перекореняет его под корневой Message, чей шаг исполняет его
закрывающий turn как у любого ребёнка; больше его никто не шагает (ни
правило предка, ни шаг обслуживания, ни авторитет остановленного). У
непривязанного закрывающегося Message нет ни обработчика, ни полосы, поэтому
его закрытие — бухгалтерия конца turn, которую пишет обслуживающая полоса
(сегодня drive хоста, потом обслуживание корня), а не шаг; оно оставляет
Message остановленным и handoff-safe, чтобы dispose или adopt родителя
могли его осадить; Message, которого никто никогда не сможет осадить, был
бы удержанием, запрещённым правилом (1) (уточнено 2026-09-14). Тайм-аут
удержания сироты — политика рантайма со значением по умолчанию, задаваемая
на рантайм, как сроки liveness. У завершённого Message (не бежит,
handoff-safe) нет полосы; его ячейки пишет та полоса, которая его «осаживает»
(шаг endpoint), — так записи цепочки укладываются в правило одной полосы
(§29). Найдено 2026-09-14 в
ходе 3b-8: рантайм L1 оставляет остановленных и dispose'нутых детей в списке
родителя до runtime_delete — дефект, закрывается на этапе 4 плана.
Об удалении рантайма, закрытии корневого Message (Михаил, 2026-09-15,
дословно): «выставляется running=0, и в конце своего turn закрывающийся
Message делает то же со своими детьми; это всё»; «исполнитель освобождает
свою память сам»; «у каждого нашего L3 Thread свой планировщик и свой
сборщик мусора».

## 33. Liveness и таймеры

Ребёнок автоматически сообщает liveness родителю на своей end-turn/каденсной
границе. Родитель при своём обслуживании проверяет control и deadline прямых
детей; отдельный второй автоматический probe для того же правила не нужен.

У ребёнка и родителя свои локальные сроки наблюдения тишины. Timeout означает
«нет ожидаемого наблюдения в срок», а не доказанную смерть другого исполнителя,
не success и не гарантированный failure graph.

Если физический поток родителя сейчас целиком занят выполнением ребёнка,
родительский таймер сам по себе не исполнится одновременно на том же потоке.
Для параллельного наблюдения нужен другой исполнитель или соответствующая
граница возврата. Нельзя скрыть это свойство диаграммой виртуального родства.

Уточнение (Михаил, 2026-09-14). Опрашивает ребёнок родителя, а не наоборот;
ребёнок сам закрывает себя после долгого отсутствия родителя; закрытие ребёнка
сверху — второй, аварийный механизм. Планировщик родителя — только выделение
исполнения и снятие по тайм-ауту по проверкам от детей (§25).

## 34. Неуспех, история и передача графа родителю

Доступный граф неуспешного ребёнка — реальное состояние и история для
диагностики, а не только строка журнала. Когда ребёнок безопасно завершён,
его обычное хранилище можно передать arena родителя без глубокого копирования,
с согласованным переносом blocks/ranges и необходимых корней. Такая передача
закрывает переданного (§32, правило 5): он не остаётся живым Message.

Передача владения и регистрация корней должны быть согласованы: нельзя
сначала освободить ребёнка, потом обнаружить, что родитель не удержал graph.
Успешная история по умолчанию освобождается; нужное длительное сохранение
оформляется явно. Переданные blocks не объявляют автоматически каждый объект
вечным или постоянно rooted.

Родитель согласует исходы своих прямых детей. Если P принял историю неуспешного
ребёнка, а затем сам успешно завершился, это не означает автоматическую
вечную пересылку той истории всем предкам. Для продолжения удержания нужна
явная причина/ссылка по контракту диагностики.

Retention budgets, remote codecs и формат отображения могут быть дальнейшей
инженерной работой. Они **не блокируют локальное удержание и передачу уже
существующего графа**. Если графа нет из-за аварии процесса/потери машины,
его нельзя выдумать. Native-ресурсы внутри диагностики также требуют своего
контракта; наличие указателя не восстанавливает закрытый файл или сеть.

## 35. Удалённое исполнение

Локальный proxy Message может управлять удалённой работой через явный транспорт.
Для передачи графа между процессами нужны представления значений, типов,
функциональных ссылок и ресурсов, понятные обеим сторонам. Локальные адреса
typed ranges, методов и вечных веток не являются готовым wire format.

Потеря связи не равна подтверждённому завершению удалённого процесса.
Полученный failure graph можно удержать; неполученное состояние остаётся
неизвестным. Это не повод откладывать корректное локальное ядро до реализации
всех remote codecs и распределённых сценариев.

## 36. Один сквозной сценарий

P имеет шаблон T с callable Structure F, её mutable own-счётчиком и явной
eternal-веткой E. F.child[0] указывает на общий дескриптор функции f.
В своём turn P создаёт ребёнка A. Copier выделяет собственные ячейки и
используемых лексических предков T в arena A, переписывает их node/ссылки,
сохраняет адрес METHOD функции f и E; сама F копируется в F'. До успеха turn
A остаётся неопубликованной резервацией.

P успешно завершает turn: outbox/создание публикуются, временные корни
снимаются, P выполняет end-turn GC. A принимает один вход, выбирает F' в своём
графе и вызывает f из F'.child[0] с первым аргументом F'. Caller inputs
разрешаются раньше fallback по скопированному F'.node.

f меняет рабочий own-счётчик. Перед исходящим вызовом checkpoint записывает
только dirty-поля. Во время turn A готовит ответ; успешный end-turn публикует
его и делает локальную сборку. P принимает ответ позже в своём отдельном turn.

Если A вместо этого окончательно завершается с неуспехом, неопубликованный
outbox отбрасывается, а доступное безопасно удержанное состояние A передаётся
родителю по lifecycle-протоколу. E продолжает жить у первого Message независимо
от исхода A. Ни один из этих шагов не меняет лексический node на «родитель P»
только из-за родства Message и не превращает один ответ в новый поток ОС.

# Часть IV. Реализация, доказательства и границы готовности

## 37. Что читать в исходниках

Основной replacement-код находится в `stg/l1_baseline/l2src/`. Старый `lm2/`
полезен для сопоставления прежнего поведения, но его Namespace/descriptor
projection и process-static throw нельзя принять за ABI нового ядра.

| Механизм | Исходники/документ для проверки |
| --- | --- |
| Structure, категории адресов | `lmx.h`, `lmx_owned*.lm1`, `struct_refactoring_version_2.txt` 3–5 |
| Построение own-графа | `lmx_branch_owned*.lm1`, `lmx_value_owned*.lm1`, `L2_MESSAGE_ROOT.txt` |
| Копирование графа | `lmx_graph_copy_owned*.lm1`; Message-публикация — `lmx_message_graph_copy*.lm1` в `codex/core-integration`; SPEC 2.3, ABI 8 |
| GC и roots | `lmx_message.lm1` (collector), `lmx_owned_ranges*.lm1`, `lmx_msg_roots_stale*.lm1`, SPEC 19.29 |
| Message и исполнитель | `lmx_message.lm1`, `lmx_msg*.lm1`, `lmx_msg*.h`, `LMX_MSG_CONTEXT_V0.txt`, `LMX_MSG_EXEC_HOST_V0.txt` |
| Frontend/генерация | `l2trans.lm1`, его parser/emitter modules, SPEC 21 |
| Runtime-прогоны | `tests/l2/`, `tests/lmx_graph_abi_selftest.lm1`, `run_l2trans.ps1` |
| Граница стабильного L1 | `L1_IMPORT_CAPACITY_20260912.md`, `stg/l1_baseline/l1src/l1trans.lm1` и корневой L1 source |
| Живая координация | `CORE_TEAM_PLAN_20260912.md`, `work_chat/CORE_CONTINUATION_20260911.md`, именованные inbox/outbox |

Названия с `*` в таблице — группы файлов, не обещание, что любой helper уже
есть на main. Перед сборкой проверить `git rev-parse HEAD`, состояние рабочей
копии и конкретный header. Одинаковое имя файла в двух worktree может обозначать
разные ABI. Нельзя линковать случайно взятые objects от разных layouts.

## 38. Снимок состояния на 12 сентября, около 12:35 местного времени

Это датированный инвентарь, который устаревает с новыми commits. Согласованная
модель в частях I–III от него не зависит.

| Этап | Ревизия/доказательство | Что действительно установлено |
| --- | --- | --- |
| Последняя синхронизация eternal-правила на main | `f9cc446d` | Документы: root-owned массив вечных веток, терминал копирования; не реализация retention |
| Graph ABI Fable | rescue `00500bbb` | 63/0 selftest и 95/95 исторических fixtures дважды на новом представлении; это ещё не полное ядро |
| Collector по target-адресам | `54b5802c`, `73c9f1ef` | Приняты targeted Exec/collect и различение STRUCT/primitive/Array, roots/history; не moving collector |
| INT/CHAR/for-array drivers | `75ac8ddc` | Проверены native значения и удержание/сборка массивов |
| Исторический runner | `2c4a12d1` | 19 support objects, недостающие зависимости и method-slot assertions; это не новый полный 95-прогон |
| Native scanner driver | `e06966ee` | Сверены scanner outputs, ошибки, владельцы, fault/retry и cleanup на новом ABI |
| Динамический scheduler snapshot | main `040af5d2` | 22/0, размеры 1/65/129/257 и отказы выделения/удержания; нет semantic cap 64/128 |
| Совмещённая ветка ABI + snapshot | `7fd9f1bb` | Приняты targeted Exec и три collect-прогона именно указанной конфигурации |
| drive snapshot/close | `598487cc`, `70759d0e` | Приняты изменения владения/locks; отдельный Codex probe различает прежний и новый порядок |
| Усиление постоянного overlap-теста | Grok сообщил `e5ba5119` | Отчёт получен; новая ревизия требует отдельной сверки evidence, production split не менялся |
| Copier с общей картой и eternal terminals | Fable до `d27e2b74`; интеграция `ff407a85` | Codex воспроизвёл 55/0 copy, 63/0 ABI и 95/95 fixtures в общей ветке; aliases/cycles/node fixups, METHOD/eternal terminals и allocation failures проверены |
| Per-callable Structure | Fable `d27e2b74`; интеграция `6bd6cdf9` | Каждый callable получил собственную Structure M с METHOD в slot0; Cancel host и полный `run_lmx.ps1` переведены на передачу M и проходят |
| Атомарная установка копии в Message | Codex `e180f719`, `377564d4` | `lmx_msg_graph_copy_install` публикует граф только после полного успеха; focused selftest 17/0 также вызывает общий METHOD-код с копией M; полный `run_lmx.ps1`, ABI 63/0, copier 55/0 и fixtures 95/95 проходят |
| Массив METHOD первого Message | Fable `51191a4b`; интеграция `cd3c2520` | Транслятор строит отдельный фиксированный ARRAY_OF_METHOD из тех же descriptor-адресов, что лежат в slot0 callable; совместный прогон 63/0, 55/0, 17/0, 95/95 и `run_lmx.ps1` проходит |
| L1 import capacity | `b41af667`, `3cacecc2`, `5704f616` | Убраны 16/1040, временные path-buffer ограничения и глубина 16; итоговые 34 проверки |

Сохранённые доказательства Codex относительно корня repository:

- `build/codex/fable_stage_review.json` — принятый первый graph ABI этап.
- `build/codex/collector_73c9_review.json` — source/compiler hashes collector.
- `build/codex/graph_array_migration/20260912_094912/evidence.json` — Array drivers.
- `build/codex/historical_runner/20260912_102450/evidence.json` — runner prerequisites.
- `build/codex/graph_abi_scanners/20260912_103246/evidence.json` — scanner migration.
- `build/codex/combined_exec_20260912_102718/review.json` — combined revision.
- `build/codex/drive_tree_598487cc_review.json` — snapshot; смотреть
  `revision_evidence_passed`, поскольку последующая правка header-comment
  меняет live hash, не прошлую проверенную ревизию.
- `build/codex/drive_close_70759d0e_review.json` — проверка production close.
- `build/codex/drive_close_order/20260912_104409/evidence.json` — независимое
  различение порядка: старый путь дождался timeout, новый завершил helper
  внутри удержания mailbox.
- `build/codex/message_graph_copy_full/run_20260912_123122_942_4204d756` —
  общий прогон интеграционной ветки: ABI 63/0, copier 55/0 с 37 позициями
  allocation failure, fixtures 95/95 и Message install 15/0.

Часть build-артефактов игнорируется Git. Их отсутствие в свежем clone не даёт
права утверждать, что локальный evidence просмотрен. Сверить сохранённый путь
на рабочей машине, hashes и фактические exits либо честно указать пробел.

## 39. Что пока нельзя называть завершённым

Main всё ещё нельзя автоматически считать полной миграцией нового graph ABI:
принятые slices находятся и в отдельных ветках. Сначала сверить и интегрировать
конкретные изменения. Старый зелёный тест inline-Lmx children не доказывает
правильность `void *` slots. Полный self-host требует своих ступеней проверки.

Первое review Grok подтвердило соответствие частей II–III прочитанным разделам
SPEC, отдельно от готовности кода. Два найденных runtime-пробела закрыты:
`34805906` обходит все referents голого CHILDREN-root, а `6dce6214` удерживает
primitive-only failure graph через HISTORY и не удерживает соседний мусор.
`a79e14c0` также включает eternal и METHOD classifier ranges в collector root
set; этот пункт инвентаря закрыт. «Живой по GC» означает достижимый/удержанный;
liveness Message в §33 означает наблюдаемую активность и сроки тишины — это
разные механизмы, несмотря на одно английское слово.

Очистка test-only observer `g_admit_dest` в `ff407a85` устранила одну ошибку,
но не объяснила весь repeated Exec hang. Живой backtrace показал фактический
production use-after-free: retirement удалял root из `rt->slots`, освобождал его,
но оставлял указатель в `rt->root`; workers могли бесконечно идти по повторно
использованной памяти через `lmx_msg_find_tree`, пока stop ждал их join.
`77a20933` вырезает root из корневого списка под exec lock до освобождения.
Регрессия детерминированно отклоняет старый код по non-null `rt->root`; новый
прошёл targeted Exec, 30/30 stress и полный `run_lmx.ps1`. Остальные пересечения
exec/table/mail и shutdown всё ещё требуют конечного инвентаря.

Нижний copier `lmx_graph_copy_many_owned`
использует одну карту для всех roots и принимает явные `eternal_ranges`;
single-root API является обёрткой. Незнакомый raw target отклоняется. Это
принято в интеграционной ветке. `e180f719` добавляет атомарную установку уже
скопированного графа в пустой Message, но пока не реализует автоматически:

- исходную операцию создания Message и runtime merge;
- source-visible status/typed throw для merge;
- lifetime массива eternal-веток первого Message;
- оформление массива вечных веток и emission квалификаций/метаданных. Отдельный
  фиксированный массив METHOD уже строится в `51191a4b`, но его наблюдаемая
  const: immutable квалификация ещё не завершена; прежний вывод о размещении
  METHOD в дочерней arena отозван;
- все типы пустых Array/foreign resources;
- source-level выбор скопированного callable после изменения внешней композиции;
  нижний focused test `377564d4` уже вызывает общий код с копией M и различает
  её mutable own-поле от поля исходной M.

В отчёте Fable eternal root как непосредственный copy-source пока отклоняется,
хотя eternal references внутри копируемого графа сохраняются. Это ограничение
конкретного helper, не новое правило языка; проверить, что интегрирующий caller
обрабатывает допустимый терминальный корень по общему контракту §20.

## 40. Закрыто: смешение callable Structure и дескриптора в эмиттере

Разобранное «противоречие индексов после merge» не является противоречием
модели. В прежней формулировке этот документ ошибочно передавал внешний R
как собственный узел каждого выбранного через него метода. Ошибка исправлена
в SPEC 7.1/21.2/21.5/21.8 и ABI: первый аргумент — callable Structure M.

При merge(C, B) положение ссылки на M' во внешнем R может измениться.
Внутренние поля скопированной M' сохраняют порядок: собственное x остаётся
относительно M', а не становится полем плоского program-unit. Если x находится
снаружи M, это отдельный лексический путь/вход; его нельзя назвать own-полем
и читать старым индексом внешнего B из нового R. Исходные C/B остаются прежними.

Общий METHOD хранится в M'.children[0] и указывает на тот же native entry.
M' и её обычные node/children копируются общим обходом. Никакой копии B с прежним
layout ради метода, новой таблицы layout/имён или клонирования кода не требуется.
Предложение передавать новые own-field locations как обход этого мнимого
противоречия отозвано Fable; вопрос пользователю закрыт.

Дефекты frontend, отмеченные Fable 12 сентября в 114853, были реальными:
callable не имели собственных Structure и получали общий unit. Этап `d27e2b74`
исправил построение и передачу M; интеграция `6bd6cdf9` перевела Cancel host на
вложенные callable Structure и прошла полный runtime-прогон. `377564d4` доказал
вызов общего METHOD-кода с копией M. Остаток frontend-проверки — source-level
изменение композиции и аудит более сложных path/for scopes.

SPEC 2.3 требует семантического обхода occurrences при меняющемся составе.
Это не запрет физического child[0] для METHOD или доказанного offset внутри
сохраняемой внутренней Structure. Нужны корректные selection и path lowering,
а не новая языковая семантика. Старые 95 standalone fixtures этой ошибки не
обнаруживают: нужны отдельные callable, изменение композиции и вызов копии.

Рекурсия разрешена: одна опубликованная M на occurrence, отдельные C-активации,
dirty-only spill, без reload при возврате. Последний dirty store определяет
опубликованное значение. Новый узел на каждую рекурсию не создаётся.

Статус: модель уточнена и вопрос закрыт; per-callable Structure принята через
`d27e2b74`/`6bd6cdf9`, а исходная и скопированная M через полный
throw/Message ABI проверены в `17fef09a`. `independent: const: immutable`, оба
массива первого Message, обычные Array-поля и cross-branch nested reference
уже приняты последующими срезами, перечисленными в верхних checkpoints. Fable
реализовал executed argument-as-own bind в `780c58c1`; текущая интеграция
`45a3cce1` проверена на новом backend. Рекурсия одной опубликованной M с
раздельными C-активациями закрыта в `9ffc96f3`: activation-local cache/dirty
сохраняются, а post-call reload отсутствует. Коммиты `f12ea87f` и `0c2494df`
остаются только документационными предшественниками.

## 41. Как проверять и сохранять знание

Стабильный L1 используется read-only:
`stg/l1_baseline/build/l1trans/gen2/l1trans.exe`, SHA256
`0B3D85B36E72A5935CA43D76B71B8CBBB060AF041CBB6FAE805796595810B2A2`
(продвинут 2026-09-15 с `722AC86E256D28EB462EE244D92B5E7188792EC0A0F5B300957622672EBAB466` — обновлением среза, привязка к исходникам в L1_PIN_SOURCE.txt;
раннеры читают хеш из `stg/l1_baseline/l2src/L1_PIN.txt`).

Критерий L1-гейта — неподвижная точка **gen2 C == gen3 C** (побайтово), а не
gen1 C == gen2 C: семя gen0 — bootstrap-артефакт (spec 1.2), и его C может
расходиться с текущим источником. Подтверждено Михаилом 2026-09-14
(`LEAD_REVIEW_20260914.md` §6; обоснование — `INTEGRATION_GATE_STATUS_20260913.md`
§11, `tests/l1/run_gen.ps1`). Следующий этап — семя gen0 из закоммиченного
снимка сгенерированного C, и гейт дополнительно требует gen2 C == снимок C.
Текущий native toolchain: `C:/Qt/Tools/mingw1310_64/bin/gcc.exe`; его реальный
hash, flags, defines и зависимости фиксируются в evidence конкретной проверки.

Готовность этапа требует точного source revision/dirty overlay, hashes
compiler и inputs, команды, успешных exits и наблюдаемого результата.
Доставленный ticket, ACK, `seen`, живой watcher и план не являются выполнением.
Проверка другой ревизии не покрывает новые изменения автоматически.

Для правки helper нужны относящиеся к ней проверки, включая реалистичные
отказы ресурсов и владение. Полный historical/self-host прогон — на границе
интеграции, а не после каждого комментария. Совместимые objects переиспользуются
только при совпадающих source/toolchain/flags/defines; тестовая instrumentation
создаёт другую конфигурацию. Не повторять одинаковую принятую сборку ради ACK.

При изменении решения исправляются: исходное определение SPEC, обе версии
ABI, примеры, acceptance requirements, текущие ledgers/handoffs и этот документ.
Старый материал либо приводится в соответствие, либо явно помечается архивом.
Одного примечания «ниже было иначе» недостаточно для действующего документа.

При восстановлении работы сначала прочитать эту модель, затем актуальные
верхние записи продолжения и реальные файлы своей ветки. Сводка модели не
является основанием пропускать исходный раздел перед изменением его механизма.

# Часть V. План работ по шагам

Порядок ниже следует из зависимостей реализации. Он не вводит новых правил
языка. Grok закрыт пользователем; Fable ведёт graph ABI/frontend, Codex —
Message-интеграцию, проверку и L1. Согласованная совместная работа разрешена.
Claude продолжает `mixa_manager`. Точные пересечения файлов согласуются
между существующими участниками через их именованные mailboxes.

Текущая доступность: пользователь сообщил, что закрыл Grok. **Новых тикетов
ему не давать до явного возобновления пользователем.** Его сессию автоматически
не перезапускать. Незавершённая задача 20260912-112047-nested-exec-hang и её
файлы сохраняются; сообщение о причине нужно сверить с фактическим отчётом.
Это часть шага 7. Fable продолжает свой frontend/graph участок, Codex —
документацию и интеграцию. Собственный приём outbox у Codex остаётся активным.

## Шаг 1. Исправить callable Structure и выбор пути в эмиттере

Раздел40 закрыт как неверное смешение собственного узла с дескриптором.
Этап реализован Fable в `d27e2b74` и принят Codex в `6bd6cdf9`: обычная
Structure на callable, METHOD в физическом child[0], own-поля/граф тела внутри
неё и передача выбранной Structure первым аргументом. Grok закрыт пользователем;
новых сообщений и заданий ему нет до явного возобновления.

Нижний Message-copy test `377564d4` уже проверяет remap узла, сохранение адреса
METHOD и разные mutable own-значения при вызове исходной/скопированной M.
Source-level проверки композиции, выбранной callable M и вызова копии закрыты
текущими fixtures; `45a3cce1` закрывает arg-as-own только с исполненного bind.
Рекурсия без нового графового узла закрыта `9ffc96f3`. Исполняемые control-body
Structures закрыты `043e1d41`/`6af2b55e`, включая вложенную родительскую цепь.
Полный bootstrap остаётся отдельной границей интеграции.

## Шаг 2. Принять последние ограниченные slices

Copier принят в `ff407a85`: общая карта нескольких roots, cycle/ancestor fixup,
mutable independence, METHOD/eternal terminal, raw rejection и failure cleanup
прошли. Усиление overlap `e5ba5119` включено в тот же интеграционный snapshot;
repeated hang закрыт в `77a20933` удалением освобождаемого Message из `rt->root`;
очистка test-only observer была полезна, но недостаточна.
Для overlap убедиться, что timeout не может дать PASS после освобождения lock.
Уже принятые идентичные runs не повторять.

Выход: принятые конкретные ревизии и явный список оставшихся ограничений;
не общий ярлык «ядро готово».

## Шаг 3. Довести интеграцию typed graph ABI

Совместимые graph/collector/runner/scanner slices сведены в
`codex/core-integration` через `ff407a85` и `6bd6cdf9`. Активный Cancel host
переведён с inline-Lmx assumptions на настоящий граф. Прямой
primitive/Array/METHOD target нельзя читать как Structure.
У checkpoint-store должен быть assert на ошибке, dirty-only поведение сохранено.

Выход: единый header и callers, типы по target ranges, owner roots и cleanup
на одном source snapshot; релевантные native tests и конечный инвентарь debt.

## Шаг 4. Завершить shared copy API и lifetime общих объектов

Fable оформляет исходный граф/квалификации; Codex проверяет roots/ranges и
failure boundary копирования. Grok закрыт, зависимости на новый его API нет.
Одна операция использует одну карту для всех operands. Поддержать нужные пустые
объекты и не смешивать raw foreign target с потерянным own target.

`51191a4b` собрал уже принадлежащие первому Message дескрипторы в отдельный
фиксированный ARRAY_OF_METHOD вместо разрозненных ссылок среди unit children.
Остаётся связать его с наблюдаемой const: immutable квалификацией/retention
metadata. Эмиттер строит исходный граф первого Message; перенос записей из
дочерней arena не нужен. Срок жизни — до завершения процесса.
Проверка: A создаёт граф с методом, B получает копию, A завершается, B вызывает
тот же descriptor/code через свой node и видит свои mutable-значения.

Отдельно реализовать root-owned const: immutable ARRAY удержания всех заранее
известных вечных веток всех Messages процесса: корректная инициализация до
публикации, lifetime ссылок, явная передача E, отсутствие deep copy/node fixups
внутри E, отсутствие добавлений в массив при merge/create, исчезновение
заёмщиков и end-turn GC без освобождения E. Никакой автоматической выдачи
массива или настроек корня. Явно переданная ссылка на весь допущенный массив
разрешена. Сохранить существующий internal eternal_ranges: получать допущенные
диапазоны из построенного квалифицированного графа, не из догадки о raw pointer.
Это метаданные helper-а, не новый аргумент исходного метода или поле Structure.

## Шаг 5. Typed result/throw и runtime merge

Закончить нужный status + typed outputs lowering, если его ещё нет на выбранной
ветке. Затем включить merge через тот же copier. Сохранить порядок полей,
result.node, внутренние node fixups и терминалы. Ошибка — throws merge(args),
не пустой успех; источник и опубликованное назначение переживают failed prepare.

Проверки: несколько операндов, общий target между ними, используемый предок,
independent, цикл, Array, общий метод на изменившемся составе из §40 и eternal E.
Выход: наблюдаемая семантика на L2 → L1 → C, а не только unit test C helper.

## Шаг 6. Message creation и передача графа

`lmx_msg_graph_copy_install` подключён к `lmx_msg_create_graph`: отдельная arena
ребёнка и полный граф готовятся до публикации inactive child; ошибка не
публикует ребёнка. Проверка root -> child -> следующий
Message сохраняет METHOD/eternal terminals и заново копирует mutable-данные.

Failure graph уже передаётся родителю через согласованный перенос
`blocks+ranges` и HISTORY roots. `a787198d` добавляет отдельный успешный
`lmx_msg_transfer_graph`: без второго copy переносит те же blocks/ranges,
атомарно прикрепляет один выбранный RETAIN root, очищает старые owner-local
graph/roots и затем использует обычный `dispose_child`. Проверка различает его
от copy по неизменным адресам Structure, цикла и Array backing и доказывает
OOM без изменений владельцев. `cae59e50` добавляет общий локальный carrier
`lmx_msg_deliver_graph`: завершённый handoff-safe источник может передать те же
blocks/ranges и выбранный корень любому живому Message, в том числе sibling.
Старый lifecycle-parent источника не меняется, получатель не становится его
supervisor; источник очищает graph/roots и снимается с active tracking. Byte
send/recv остаётся отдельным старым envelope-прототипом; frontend/source
lowering обычной доставки входящего LMX-текста ещё не закрыт.

## Шаг 7. Закрыть оставшийся Message exec/D7

После возвращения Grok может продолжить конечный инвентарь lock crossings и
lifetime paths; пока он закрыт, работу ведёт Codex. Устранить
оставшиеся exec→mail ожидания, проверить stop/fail/retirement/error paths,
не теряя FIFO, done state, pins и ready membership. Repeated timeout после
`m0_acc` закрыт причинной регрессией `77a20933`, а не успешным retry.

Проверки должны различать старый и новый порядок, включать отказ retain/alloc,
изменение дерева между snapshot/revalidate и больше 128 детей. Не вводить
новый фиксированный максимум как замену динамическому snapshot.

## Шаг 8. Сквозной lifecycle и GC

Проверить сценарий §36, success/stop/failure, родителя с running=0, завершение
прямых детей, удержание failure graph, снятие временных roots и один nonmoving
end-turn collection. После сборки все живые адреса остаются прежними.

Локальное сохранение доступного графа не ждёт remote codec, UI диагностики
или retention budget. Недоступные удалённые данные явно остаются неизвестными.

## Шаг 9. Полный frontend/self-hosting контроль

Executable body hosting, callable recursion и текущий executed own-bind уже
закрыты указанными выше срезами. Закрыть оставшиеся документированные frontend
gaps, включая method pointer locals, caller inputs и ещё не перенесённые
выходы/finally.
Подготовить одну согласованную конфигурацию и выполнить положенные historical
fixtures, candidate/self/next/check/bootstrap проверки. Сравнить реальные
результаты/генерации, зафиксировать compiler/source hashes.

Стабильный L1 не подменяется промежуточным candidate. Его promotion — отдельная
подтверждённая граница. Наличие 95 зелёных fixtures или одного selftest не
равнозначно полному self-host и не доказывает ещё не проверенные формы языка.

## Шаг 10. Условия готовности минимального ядра

Готово, когда одна согласованная сборка исполняет описанные графовые значения,
общие методы и правильный вызов, deep merge/Message create, typed throw,
ownership delivery, nonmoving GC и полный локальный Message lifecycle; ошибки
ресурсов не повреждают владельцев и не маскируются под успех. Для каждого
механизма есть воспроизводимое относящееся к нему evidence, а статус main
совпадает с интегрированной реализацией.

Оставшиеся расширения, VM/remote profiles и прикладные modules перечисляются
отдельно с точными границами. Они не меняют уже согласованные `len`, `node`,
`@`, методы, копирование и Message при следующем восстановлении контекста.
