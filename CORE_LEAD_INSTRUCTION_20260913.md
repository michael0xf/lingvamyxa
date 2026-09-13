# Инструкция ведущего чата: ядро L2, самосборка, Claude как помощник

Составлено 2026-09-13 около 17:30 (локальное время машины) чатом Fable для
самого себя — для модели, которая продолжит работу в ЭТОМ чате после смены
модели, контекста или паузы. Всё, что здесь названо фактом, было измерено в
этой сессии командой или чтением файла, не выведено из памяти о переписке.
Даты и хеши устаревают с каждым коммитом; правило «сначала проверь, потом
верь» относится и к этому документу.

Читать в таком порядке: §1–§3 целиком, затем §4 (состояние), затем §5–§7
(решение об интеграции и план), и только потом брать работу. §8–§11 —
справочник: инструменты, работа с Claude, дисциплина, открытые вопросы.

---

## 1. Что изменилось и кто теперь кто

- **Codex выбыл** («ушёл в limit off») 2026-09-13 после 15:26. Он был
  интеграционным лидом: вёл `codex/core-integration`, писал тикеты Fable и
  Claude, принимал их работу, правил документы. Его последний час работы был
  **не закоммичен** и сохранён этим чатом как ветка
  `codex/wip-selfhost-20260913` (коммит `631abc18`, см. §4.5). Никаких других
  следов «прощального» handoff'а от Codex нет — ни в репозитории, ни в
  каналах.
- **Этот чат (Fable) — ведущий.** Он пишет код ядра, интегрирует ветки,
  ведёт документы и ставит задачи Claude. Раньше он владел только «graph ABI /
  frontend lane» (см. `FABLE_5_1_CORE_HANDOFF_20260912.md`); эта роль
  сохраняется и расширяется до интеграции.
- **Claude** (`work_chat/claude`, Sonnet 5 Medium) — помощник. Он владеет
  `mixa_manager` и уже переносит его модули с L1 на чистый L2 (24 из 52 модулей
  на момент написания). Ему выдаются тикеты в его inbox; ответы приходят в его
  outbox. Он **не** трогает транслятор, runtime и общие документы ядра.
- **Grok закрыт пользователем.** Никаких тикетов, сообщений, изменений его
  ватчеров, перезапусков сессии. Это стоит в каждом тикете Codex и остаётся в
  силе.
- **Михаил** (пользователь) пишет по-русски в чат и раз в 30 минут кладёт
  русские «как дела?» в inbox каждого канала. Ответы ему — по-русски в чат;
  запросы задания при отсутствии тикета — **по-английски в outbox** (его
  стоящая инструкция). Коллегам (Claude) — только по-английски.

---

## 2. Цель, точно

Из `L2_CORE_SELFHOST_HANDOFF_20260911.md` §1 и модели §Шаг 10, дословно по
смыслу:

Реализовать ядро Lingvamyxa и инструменты на L2 и получить настоящую,
повторяемую, переносимую **самосборку всей цепочки**:

```text
L2 source -> L2 translator -> generated L1 -> L1 translator -> ANSI C99
          -> platform C compiler/linker -> executable, который повторяет
             тот же build / next / check / tests
```

Условия Михаила (сообщение от 2026-09-13): писать код, **пока**
(а) ядро языка не реализовано, (б) не будет **полной самосборки проекта на
чистом L2**, (в) используя Claude как помощника через его inbox.

«Ядро» — это **транслятор семантики** (здесь: в C99). Парсер — это только
разбор синтаксиса. Это **две параллельные, независимые задачи** (слова
Михаила, 2026-09-13): синтаксис языка общего назначения, и из самого
синтаксиса нельзя сделать никаких выводов о Lingvamyxa.

Что считается готовым — модель, §Шаг 10: одна согласованная сборка исполняет
описанные графовые значения, общие методы и правильный вызов, deep
merge / создание Message, typed throw, ownership delivery, nonmoving GC и полный
локальный lifecycle Message; ошибки ресурсов не портят владельцев и не
маскируются под успех; для каждого механизма есть воспроизводимое evidence;
статус main совпадает с интегрированной реализацией. Плюс — самосборка:
`l2trans` написан на L2 и собирает себя через цепочку до неподвижной точки.
Зелёный набор fixtures, один selftest или «модуль портирован» — ещё не это.

---

## 3. Эталоны и документы

### 3.1 Два разных эталона

| Полоса | Эталон | Где проверять |
| --- | --- | --- |
| **Ядро** (семантика, транслятор L2→L1→C99, runtime Message, граф) | Реализации-эталона **нет** — мы её делаем. Эталон — **описание**: `L2_CORE_AND_MESSAGE_MODEL_20260912.md` (части I–III — модель; IV — состояние; V — план) + нормативные разделы `Lingvamyxa_spec.txt` + `struct_refactoring_version_2.txt` §3–8, 13–14. | Гейты §8; своё evidence на каждый механизм |
| **Парсер** (синтаксис, P0-дерево) | Старый проект **`C:\Nyasha_Planet\lingvamyxa_old_worked_version`** @ `620db86` — он полностью самособирался и его парсер — идеальный эталон разбора. Новый парсер (`l1src/parser.lm1`) перенесён оттуда «небрежно» и несколько раз допиливался: последний раз — перенос функциональности фигурных скобок `{}` (только парсинг, не трансляция) и сверка обработки `c.*` с эталоном. | `tests/p0_tree_contract/run_p0_meta.ps1` — дифф P0-метаданных нового парсера против пересобранного эталона 620 (хеши закреплены в раннере) |

Почему новый проект вообще существует (Михаил): у старого было **совершенно
неправильное ядро** — модели реализуют привычные концепции, а здесь другой
язык с нестандартной композицией концепций, часть которых редка. Поэтому
Astra Extra High написал описание ядра (модель), и **точная реализация
задуманного ядра важнее синтаксиса**: синтаксис можно довести тестами и
правильным кодом, ядро надо делать сразу в его онтологии по точному описанию.

Практическое следствие: при любом вопросе «а как это должно работать» ответ
ищется в модели/спеке, а не в старом проекте и не в привычках. Вопрос
пользователю допустим только при **двух несовместимых правилах модели и
минимальном примере**, где они дают разные результаты (модель, «Как читать»).
Отсутствующая функция, старый тест, размер буфера — инженерная работа.

### 3.2 Документы, порядок чтения, что есть что

| Документ | Что это | Актуальность |
| --- | --- | --- |
| `L2_CORE_AND_MESSAGE_MODEL_20260912.md` | Полная модель ядра и Message; таблица «быстро найти ответ»; §37 карта исходников; §V план по шагам | Модель актуальна; «LATEST CHECKPOINT» наверху устарел (03:15) — реальное состояние в §4 здесь |
| `Lingvamyxa_spec.txt` | Спецификация (15 817 строк). Нормативные разделы для ядра: 2, 6.5, 7, 8.8, 9.1.2–9.1.4, 11.3.1, 19.17, 19.28, 19.29, 20, 21 (особенно 21.5–21.9). Заголовки: `grep -nE "^[0-9]+(\.[0-9]+)*\.? +[A-ZА-Я]"` | Актуальна; на ветке codex есть правки (в списке пересечений §4.2) |
| `struct_refactoring_version_2.txt` | Revision 2 — соглашение по ABI/графу | Актуально |
| `L2_CORE_SELFHOST_HANDOFF_20260911.md` | Цель самосборки (§1), корни/инструменты (§3), критерии (§4), онтология (§7–10), фазы | §1, §3, §4, §7–15 — читать; снимки состояния устарели |
| `CORE_TEAM_PLAN_20260912.md` | Журнал Codex: чекпоинты, границы владения, «модель, которую должен сохранять каждый этап» | Последняя запись 03:15 13.09 — с тех пор ~10 часов работы не отражены; раздел «Model each stage must preserve» и «Non-overlapping stages» по-прежнему верны как правила |
| `work_chat/CORE_CONTINUATION_20260911.md` | Живое продолжение Codex | Последняя запись 05:00 13.09; устарело |
| `FABLE_5_1_CORE_HANDOFF_20260912.md` | Роль/ограничения Fable; «Settled constraints to keep together» | Ограничения верны; статусы устарели |
| `stg/l1_baseline/l2src/FABLE_GRAPH_ABI.txt` | Мои проектные заметки по каждому срезу frontend/graph ABI, включая пять портов runtime-модулей и что показала библиотечная эмиссия | Актуально до `868d85db` на `fable/merge-on-callable` |
| `work_chat/TICKET_RULES_EN.md` | Статусы WORKING/BLOCKED/DONE/STAGE DONE; что такое приёмка; пропорциональная верификация | Действует для всех |
| `work_chat/claude/PROTOCOL.txt`, `ASSIGNMENT.txt`, `INBOX_WATCHERS.md`, `WAKING_CLAUDE.txt`, `OPENCODE_HANDOFF_20260911.md` | Протокол канала Claude, его назначение, его ватчер, как его будить, его бэклог приложения | Действуют; в них Codex назван монитором — теперь это этот чат |
| `mixa_manager/PORT_OF_CLEARSHELL.txt`, `FIRST_VERSION.txt`, `CODING_RULES.txt`, `STATUS.txt`, `*_l2_port.txt` | Рамка приложения, первая версия, правила кода L1 в приложении, заметки Claude по каждому порту | STATUS устарел (10.09); `*_l2_port.txt` — свежие и точные |
| `L1_IMPORT_CAPACITY_20260912.md`, `L1_language_and_translator_spec.txt`, `L1_spec.txt` | L1: ёмкость импортов (снят лимит 16/1040/глубина 16), язык L1 | Актуально; на ветке codex L1 изменён (§4.4) |
| `Lingvamyxa_development_plan.txt` | Длинный план (таблицы правил, L3, Mix, Message Threads, HTTP, auth) | Порядок больших этапов; текущая работа — до его п.3 |

---

## 4. Состояние на 2026-09-13 ~17:30 — измерено

### 4.1 Ветки и worktrees

| Ветка / worktree | HEAD | Что там |
| --- | --- | --- |
| `main` (checkout `C:\Nyasha_Planet\lingvamyxa`) | `a0409b10` = `origin/main` | Претензии Claude: «L2: port mixa_* to clean L2» — 24 модуля; главные документы; **старый** `l2trans.lm1` без `7d7ec87c` |
| `origin/codex/core-integration` | `f04cf1cd` (локальный ref `codex/core-integration` отстал: `0c5ec620` — сделать `git fetch`) | Интеграционная ветка Codex: весь core, компилятор `7d7ec87c` + доработки, мои срезы, правки L1, черри-пики портов Claude, 25 файлов `parser_*.lm2` |
| `fable/merge-on-callable` (worktree `build/fable/graph-abi`) | `868d85db` | Моя ветка: `7d7ec87c` + `1eb11879` (path_storage) + blocks/ranges DONE + заметки. Единственное место, где есть **lmx_msg_path_storage.lm2** и его раннер (на codex НЕ интегрирован) |
| `codex/wip-selfhost-20260913` (worktree `build/codex/core-integration`, переключён на неё) | `631abc18` | Незакоммиченный последний час Codex, сохранён как есть (§4.5). Основана на `f04cf1cd` |
| `backup/pre-rebase-7d7ec87c` | `60127ca3` | Мой бэкап перед rebase; можно удалить после интеграции |
| остальные `codex/*`, `fable/*`, `grok/*` | — | Исторические/вспомогательные; не трогать без нужды. Worktrees Codex под `build/codex/*` — не удалять |

Локальные ветки `main` и `codex/core-integration` — **разные линии**, а не одна:
`main` не содержит `7d7ec87c`; `origin/codex/core-integration` содержит.

### 4.2 Расхождение main ↔ codex/core-integration

- merge-base: `96faad93`. На `origin/main` — **68** коммитов, которых нет на
  codex; на `origin/codex/core-integration` — **184**, которых нет на main.
- **202 файла** изменены с обеих сторон с merge-base. Из них: главные документы
  (модель, спека, план, handoff'ы, `Lingvamyxa_handoff_notes.txt`,
  `struct_refactoring_version_2.txt`); ~150 файлов `mixa_manager/*`
  (порты Claude, которые Codex черри-пикал на свою ветку под другими хешами —
  содержимое почти совпадает, **но** `mixa_help.lm2` и
  `mixa_composite_glyphs.lm2` на codex изменены Codex'ом относительно версии
  Claude: 4 и 24 строки); три последних порта Claude (`mixa_app_window`,
  `mixa_buttons`, `mixa_fm_remove`) есть **только на main**.
- Остальные различия: `stg/l1_baseline/**` (74 новых + 61 изменённых файлов —
  весь core только на codex), `tests/l1/*`, `mixa_manager/tests/*`,
  `mixa_manager/vendor/*`.

### 4.3 Компилятор L2 (`stg/l1_baseline/l2src/l2trans.lm1`)

- На **main** — старый. Поэтому каждый порт Claude на main упирается в один и
  тот же барьер: `l2trans error: … unknown foreign type` на первом формальном
  параметре собственного struct-типа (`@: MixaFm fm` и т.п.). Claude честно
  делает **oracle-side parity** (L1-оракул + харнесс) и фиксирует
  line/diagnostic — это принятая форма DONE, пока компилятор отстаёт.
- На **codex/core-integration** — текущий: `7d7ec87c` «translate library units
  and native manager operations» (библиотечная эмиссия: .lm2 без `main` даёт
  модуль с публичными обёртками под исходными именами; агрегатные указатели
  через `predef:`; cast; malloc/calloc/realloc/free/strcmp; pointer locals;
  индексация; store в foreign поля) + доработки Codex после него
  (`483c4e37` const unit fields/void* results, `cd12da13` const pointer
  returns, `56203d6e` arbitrary pointer depth, `2eaf0778` typedef arrays и
  external calls, `89e2f287` unsigned-byte domains) + мои гэпы (`485f15cc`,
  `5f31750b`, `6461eb07`).
- Мой `1eb11879` (path_storage: cast size_t/unsigned*, `c.realloc`,
  `c.sizeof(unsigned)`, glued `-1`, unsigned** slot) на codex **не** попал
  (Codex ушёл раньше). После интеграции его надо принести с
  `fable/merge-on-callable`; конфликты будут в `l2trans.lm1` там, где Codex
  реализовал то же самое (cast, sizeof) — брать его сторону, оставлять мои
  `lmx_msg_path_storage.lm2`, `run_port_msg_path_storage.ps1`,
  `tests/unit_ptr_grow.lm2`, регистрацию в `run_graph_abi.ps1`.

### 4.4 Транслятор L1 и «стабильный компилятор»

- Пин: `stg/l1_baseline/build/l1trans/gen2/l1trans.exe`, SHA256
  `65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`
  (проверено 17:00; тот же файл в worktree Codex). Он **read-only** и
  gitignored (живёт только на этой машине; в чистом clone его надо
  пересобирать через `gate.ps1`/`buildCore`).
- На codex-ветке Codex **менял сам L1** (root `l1src/l1trans.lm1`, `parser.lm1`
  и копию `stg/l1_baseline/l1src/`), а также **перегенерировал bootstrap-C
  снимки** `lm1/build/l1trans.lm1.c`, `lm2/l1trans.lm2`,
  `stg/l1_baseline/lm2/l1trans.lm2`: коммиты `9a733eca`/`7d8a5f09` (arbitrary
  address depth), `d2cb5e0e` (positional backslash), `e73b74a6` (span-directed
  slash parsing), `eae23966`, `3aa793b7` (P0 MIX anchors/string bytes),
  `24a3e4c7` (C surface assignment targets). В его worktree gen0 (13:42) и gen1
  (14:11) пересобраны; **gen2 (пин) не менялся**. Полный L1-гейт
  (`stg/l1_baseline/gate.ps1`: buildCore → gen0 seed → gen1..gen3 fixed point →
  все suites на gen0/gen2 → L2 suite) после этих правок в логах **не виден**.
- В checkout'е **main** лежит **незакоммиченный** дифф `l1src/README.md`,
  `l1src/l1trans.lm1`, `l1src/parser.lm1`, `tests/l1/decl_repeat_ptr.lm1`,
  `tests/l1/run_decl_repeat.ps1` — он **hunk-в-hunk совпадает** с коммитом
  `7d8a5f09` на codex-ветке (проверено diff'ом). Это не новая работа; при
  интеграции его можно сбросить (`git checkout -- l1src tests/l1`) или он
  сольётся сам. Не коммитить как своё.
- Следствие: «стабильный L1» сейчас двоится. Пин 65D5 старый; source L1 на
  codex новее и требует **своего** промоушена: полный `gate.ps1` на ветке,
  сравнение gen2/gen3 неподвижной точки, и только потом — новый пин и
  запись хеша во все документы, где он указан (SELFHOST §3, FABLE handoff,
  модель §41). До этого все раннеры продолжают использовать 65D5 read-only —
  и работают (все мои гейты и порты Claude собирались им 13.09).

### 4.5 Самосборка транслятора — где остановился Codex

Сохранено в `codex/wip-selfhost-20260913` = `631abc18` (worktree
`build/codex/core-integration`, файлы датированы 16:29–16:32):

- `stg/l1_baseline/l2src/make_l2trans_lm2.ps1` — **механический** генератор
  L2-исходника транслятора из `l2trans.lm1`: переименовывает зарезервированный
  формальный `node` → `p0_node` и `fn` → `p0_fn` (24 и 3 функции), переводит
  `c.array: [N]: char x` в `[]: char x N`, `const: @: T x` в `const: @(T x)`,
  разбивает `int: x 0 - 1` на объявление + присваивание (15 мест), вырезает
  блок `prototype:` и внешнюю обёртку `main`, оборачивает всё в
  `L2:` … `end: L2`. Счётчики закреплены asserts — генератор рассчитан на
  **перезапуск после каждой правки `l2trans.lm1`**, а не на ручную правку
  `.lm2`.
- `stg/l1_baseline/l2src/l2trans.lm2` — его выход, 14 119 строк. Первый
  L2-исходник L2-транслятора.
- `l2trans.lm1` (+653/−209), `lmx.h`, `lmx_value_owned.*`,
  `lmx_graph_copy_owned.lm1` — поддержка компилятора под этот выход: разбор
  каждого predef-документа **один раз** на единицу трансляции
  (`l2_pdoc_*`), допуск внешних функций (`l2_extfn_*`), загрузка массива по
  выражению-индексу, и др.

**Измерено этим чатом** (пин 65D5, gcc из `C:/Qt/Tools/mingw1310_64`):

| Компилятор | Вход | Результат |
| --- | --- | --- |
| незакоммиченный (631abc18) | `l2trans.lm2` | **segfault**, exit 139, без диагностики |
| незакоммиченный (631abc18) | крошечный `L2:` … `end: L2` unit (8 строк) | **segfault** — краш в его новом коде, не в размере входа |
| закоммиченный `f04cf1cd` | тот же крошечный unit | **exit 0** — обёртку `L2:` он понимает |
| закоммиченный `f04cf1cd` | `l2trans.lm2` | **не завершился за >6 минут** (убит). Согласуется с назначением его WIP: повторный разбор predef-документов на каждое обращение |

Вывод для продолжения: отправная точка — `f04cf1cd` + **исправленный** WIP
(разбор predef один раз) с найденным и починенным крашем; бисекция по
усечённому входу; затем цепочка `l2trans.lm2 → L1 → C → l2trans_next.exe`,
затем `l2trans_next` на том же `l2trans.lm2` до совпадения выходов
(неподвижная точка). См. §6.3.

### 4.6 Runtime-модули на L2 (моя полоса, всё с parity-раннерами)

Готово (на `fable/merge-on-callable`; на codex — все кроме path_storage):

| Модуль | Коммит | Parity | Как связан |
| --- | --- | --- | --- |
| `lmx_msg_storage` | `d1fe4175`/`1307ec42` | 77/0 | splice + `-D` редирект call sites селфтеста (bootstrap-проблема) |
| `lmx_msg_slots` | `cae0097e` | 278/0 | экспорт **реальных** символов (единственный без bootstrap-проблемы) |
| `lmx_msg_path_storage` | `1eb11879` | 541/0, 11 wrapped allocations | `-D` редирект; счётчики аллокатора берутся и отчитываются |
| `lmx_owned_ranges` | `6fb35ee8` | 439/0 | **библиотечная единица** (`7d7ec87c`), имена `ranges_*`, `-D` редирект |
| `lmx_msg_blocks` | `b0fd2276` | 143/0, 147 frees, 2 disposer callbacks | библиотечная единица, `blocks_*`; function-pointer local = disposer |

Bootstrap-проблема (измерена для всех пяти): каждая аллокация арены зовёт
`lmx_msg_storage_move_all` → `lmx_msg_blocks_*` / `lmx_owned_ranges_*`; и
`lmx_msg_assign_path` зовёт `lmx_msg_path_grow` из `lmx_msg_create`. Поэтому
библиотечная единица не может носить экспортируемые ABI-имена этих модулей:
`l2_library_open` строит граф, аллокация зовёт модуль, модуль зовёт
`l2_library_open` — рекурсия. Селфтест редиректится на имена единицы через
`-D`, runtime остаётся на рукописном L1. Только `lmx_msg_slots` свободен.
Это не дефект, это **порядок загрузки**, и при настоящей самосборке его надо
решать явно (первый Message и его массивы строит транслятор — модель §20).

Остаётся рукописным C в `l2src`: `lmx_message_exec.c` (+selftest),
`lmx_message_host.c` (+selftest), `lmx_poll_stub.c`, `lmx_dec.c`,
`indent_parse_probe.c`, `indent_stack_abi.c`, `layout_prefix_abi.c`.
Остаётся L1 без L2-источника: все прочие `lmx_*.lm1` (`lmx_message.lm1`,
`lmx_msg_mail_chain`, `lmx_msg_sched_ready`, `lmx_msg_visit`,
`lmx_msg_liveness`, `lmx_msg_history_owned`, `lmx_msg_roots_stale`,
`lmx_branch_owned`, `lmx_value_owned`, `lmx_chars_owned`, `lmx_array_owned`,
`lmx_array_ref_owned`, `lmx_graph_copy_owned`, `lmx_merge_owned`,
`lmx_message_graph_copy`).

Два тикета Codex для этой полосы пришли, **пока мой ватчер лежал**, и не были
выполнены — теперь это очередь ядра:
- `work_chat/fable/inbox/20260913-111000-msg-mail-chain-l2-port.txt`:
  `lmx_msg_mail_chain` как настоящая библиотечная единица (на месте
  устаревшего program-shaped `.lm2`), контракты из `LMX_MSG_MAIL_CHAIN.txt`,
  раннер `run_port_msg_mail_chain.ps1`;
- `work_chat/fable/inbox/20260913-152000-graph-copy-clean-l2-port.txt`:
  `lmx_graph_copy_owned` на чистом L2 с точной поведенческой парностью
  (одна карта на все корни, копия используемого графа до node=0, aliasing,
  циклы, METHOD/eternal терминалы, атомарность при отказе аллокации).

### 4.7 Парсер

- Новый парсер: `l1src/parser.lm1` (+ копия `stg/l1_baseline/l1src/parser.lm1`);
  перенос из старого `lm2/parser.lm2` @ `620db86`
  (`l1src/README.md` §«What is here»).
- Гейт паритета с эталоном: `tests/p0_tree_contract/run_p0_meta.ps1`
  (собирает пересобранный 620 и текущий, сравнивает P0-метаданные по голденам;
  мутированный голден обязан падать). Отдельно
  `tests/p0_tree_contract/README.txt` описывает случаи `{}`/фенсов/трейлеров.
- На codex-ветке Codex начал **порт парсера на L2**: 25 файлов
  `stg/l1_baseline/l2src/parser_*.lm2` (compact scanner, positions, indent,
  trailer roles, text views, python strings, …) с дифференциальными
  раннерами (`run_candidate_indent.ps1`, `run_candidate_c_scanners.ps1`,
  `*_abi.c` пробы). Это фрагменты, не целый `parser.lm2`.
- Последние правки парсера на codex (13.09, 13:10–14:13): positional
  backslash, span-directed slash parsing, «call and group operands before
  field follow», P0 MIX anchors, C surface assignment targets — все с
  тестами в `tests/l1/*` и с перегенерацией bootstrap-C. Их приёмка через
  `run_p0_meta.ps1` и `gate.ps1` — часть §6.2.

### 4.8 mixa_manager (полоса Claude)

- 52 модуля L1; **24** имеют `.lm2` + `run_mixa_*_l2_parity.ps1` + заметку
  `*_l2_port.txt`: app_controller, app_fmpanel, app_main, app_panel, app_path,
  app_window, button_dispatch, buttons, cmdline, cmdline_dispatch, composite,
  composite_glyphs, console_window, draw, event_fifo, file_win32, fm_remove,
  help, highlight, pointer, process_marker, selection, text_rect, tiles
  (+ две пробы `mixa_entrysig_probe.lm2`, `mixa_selection_l2_probe.lm2`).
- Не портированы (в примерном порядке зависимостей): `mixa_remove`
  (**выдан тикет `20260913-171800-mixa-remove-clean-l2`**), `mixa_remove_confirm`,
  `mixa_selection_walk`, `mixa_dir_win32`, `mixa_fileio_win32`,
  `mixa_file_manager`, `mixa_copy`, `mixa_fm_copy`, `mixa_share*`,
  `mixa_process_win32`, `mixa_pump`, `mixa_backend_*` (headless/win32/table/
  ctors), `mixa_app_win32`, `mixa_audio*`, `mixa_calculator_syntax`.
- Все порты на main упираются в барьер §4.3; после интеграции их раннеры
  должны быть прогнаны заново — это первое, что доказывает пользу слияния.
- Рукописный C в `mixa_manager`: `mixa_console_window.c`, `test.c`, `vendor/`.
- Открытый вопрос Claude из порта app_controller («rebase onto
  origin/codex/core-integration?») — **закрыт** тикетом 171800: не ребейзить,
  отдельной ветки не заводить, продолжать на main; интеграция — дело этого
  чата.
- **Дополнение 17:50:** Claude закрыл тикет 171800 — `mixa_remove` портирован,
  коммит `04bcd56a` на main, oracle-side паритет 41/41 (реальные отказы rmdir,
  read-only, icacls-deny, cancellation, Unicode). Новый **вариант барьера**
  компилятора, точно изолированный: `mixa_remove.lm2:15:25: incompatible
  entry signature` на первом параметре `MixaRemoveNodeFn: f` — формальный
  параметр **типа function-pointer** (`fnptr:` из заголовка). Для
  `lmx_msg_blocks` я реализовал fnptr **как локальную переменную**
  (`b0fd2276`); fnptr **как формал** и как передаваемый аргумент — гэп,
  который надо закрыть в компиляторе после §6.1 (и проверить, что
  `7d7ec87c` его не закрыл сам). Следующий тикет Claude —
  `mixa_remove_confirm` (`20260913-175200-mixa-remove-confirm-clean-l2`).
- **Дополнение 18:40:** Claude закрыл 175200 и продолжил по очереди без
  тикета: `mixa_remove_confirm` — `6c10454e`, 44/44 oracle-side, барьер
  `mixa_remove_confirm.lm2:4:31: unknown foreign type` (формал
  `@: MixaRemoveConfirmCtx ctx`, обычная форма); `mixa_selection_walk` —
  `2e628051`, все проверки oracle-side, барьер
  `mixa_selection_walk.lm2:4:20: unknown foreign type` (формал
  `const: @(MixaSelWalkAllocVTable av)`). Портировано **27 из 52**; путь
  remove → remove_confirm → fm_remove и его walk целиком имеют чистый L2
  источник. Claude сам перешёл к `mixa_dir_win32` (очередь тикета 175200:
  далее `mixa_fileio_win32`, `mixa_file_manager`). После §6.1 перезапускать
  уже **27** раннеров паритета, не 25.

### 4.9 Ватчеры (оба армируются этим чатом)

| Что | Скрипт | Мьютекс | Состояние/heartbeat |
| --- | --- | --- | --- |
| Inbox Fable (сюда пишут Михаил и — раньше — Codex) | `work_chat/fable/scripts/watch_inbox.ps1 -RootPath 'C:/Nyasha_Planet/lingvamyxa'` | `Fable_Inbox_Watcher_Mutex` | `build/fable_watch/heartbeat.txt` |
| **Outbox Claude** (его ответы на мои тикеты) | `work_chat/fable/scripts/watch_peer_outbox.ps1 -RootPath 'C:/Nyasha_Planet/lingvamyxa' -Peer claude` | `Fable_Peer_claude_Outbox_Mutex` | `build/fable_peer_claude_watch/heartbeat.txt` |

Оба — one-shot: FSW на директорию + батч 20 с тишины + fallback 30 мин +
пульс 30 с; печатают один JSON и выходят; **сессия обязана перевзвести**.
`watch_peer_outbox.ps1` дополнительно несёт **часы молчания**: если самый новый
тикет в inbox Claude старше самого нового ответа в его outbox дольше
`SilentSeconds` (1800), он срабатывает с `reason=peer_silent` (один раз на
тикет). Изолированный селфтест: `work_chat/fable/scripts/test_watch_peer_outbox.ps1`
— 15/15 (ответ .tmp→.txt, fallback, touch-only не срабатывает, молчание,
однократность, дубликат = exit 1). Селфтест inbox-ватчера:
`test_watch_inbox.ps1` — 31/31. Ватчер Claude
(`work_chat/claude/scripts/watch_inbox.ps1`, мьютекс `Claude_Inbox_Watcher_Mutex`,
heartbeat `build/claude_watch/heartbeat.txt`) — **его**, не трогать.

---

## 5. Решение об интеграции — нужно подтверждение Михаила; рекомендация

Проблема: `main` — «витрина» с работой Claude, но с компилятором, который не
собирает его порты; `codex/core-integration` — единственная ветка с
работающим ядром, но без трёх последних портов Claude и без моего
path_storage. Codex, который сводил их вручную (черри-пиками), ушёл.

**Рекомендация:** сделать `codex/core-integration` базой `main` одним
явным слиянием, после чего вся работа (ядро, порты Claude) идёт на main, а
ветки — только рабочие срезы с быстрым возвратом в main. Порядок:

1. `git fetch`; создать рабочий worktree `build/fable/integration` на новой
   ветке `integration/main-absorbs-core` от `origin/codex/core-integration`.
2. `git merge origin/main` туда. Ожидаемые конфликты: главные документы
   (взять обе стороны по смыслу — на codex новее core-записи, на main новее
   записи про Claude), `mixa_help.lm2`/`mixa_composite_glyphs.lm2` (сверить с
   Claude — какая версия у него проходит oracle-паритет; при сомнении взять
   его, main-версию, и попросить его перепрогнать), остальные `mixa_manager/*`
   должны слиться без конфликтов (одинаковое содержимое).
3. Принести с `fable/merge-on-callable` коммиты `1eb11879` (path_storage) и
   `868d85db` (заметки) черри-пиком; в `l2trans.lm1` брать сторону codex там,
   где реализовано то же (cast/sizeof), — как в моём `088fce07`.
4. Прогнать: `run_graph_abi.ps1`, `run_l2trans.ps1`, `run_lmx.ps1`, все пять
   `run_port_*.ps1`, `run_mixa.ps1` и **все 24** `run_mixa_*_l2_parity.ps1`
   (они должны впервые пройти L2-сторону, а не только оракул). Записать
   evidence-пути.
5. Только после зелёного: `git checkout main && git merge --ff-only
   integration/main-absorbs-core`, `git push`. **Не force-push main**. Сообщить
   Claude одной строкой: барьер снят, перепрогони раннеры, с этого момента
   `predef:` типов из заголовков — норма.
6. Обновить в модели/плане/этом документе одну запись «интеграция 13.09».

До подтверждения: Claude продолжает на main (oracle-side parity, как сейчас);
работа ядра ведётся на ветке **от `origin/codex/core-integration`** (не от
main), чтобы не плодить третью линию.

Что **не** делать: не сбрасывать чужие dirty-файлы (`git status` в main
показывает незакоммиченный L1-дифф Codex — §4.4); не трогать worktrees
`build/codex/*` кроме чтения; не «чинить» старые тесты под нужную зелень;
не вводить новых лимитов 16/64/128; не менять пин компилятора без §6.2.

---

## 6. План работ ядра (порядок = зависимости)

Каждый шаг: что делать → как проверить → чего не делать. Статусы по
`TICKET_RULES_EN.md`. Каждый проверенный этап — отдельный сфокусированный
коммит + push **своих** путей и запись в `FABLE_GRAPH_ABI.txt` (для frontend)
или в этот документ (для интеграции).

### 6.1 Слить ветки (§5) — первый шаг после ответа Михаила
Готово, когда main содержит ядро и все порты, и все раннеры §8 зелёные на
одном хеше.

### 6.2 Принять правки L1 Codex и решить промоушен пина
- В **своём** worktree (не в `stg/l1_baseline` main-checkout'а, где лежит
  пин) выполнить `stg/l1_baseline/gate.ps1` на слитой ветке: buildCore → seed →
  gen1..gen3 → неподвижная точка → все suites. Отдельно
  `tests/p0_tree_contract/run_p0_meta.ps1` (паритет парсера с 620) и
  `tests/l1/run_*.ps1`, которые Codex добавил.
- Если всё зелёное и gen2==gen3 — это кандидат нового стабильного L1.
  Промоушен: скопировать gen2 в `stg/l1_baseline/build/l1trans/gen2/`, снять
  SHA256, заменить `65D5…` во всех документах (`grep -rl 65D5A5ED`), сообщить
  Claude (он сверяет хеш перед каждым прогоном).
- Если не зелёное — это дефект L1 (или регенерированного bootstrap-C), и он
  блокирует §6.3: транслятор L2 тоже собирается этим L1.
- Не делать: не «подправлять» голдены p0_meta под новый парсер без объяснения
  расхождения с 620 (README требует, чтобы мутированный голден падал).

### 6.3 Самосборка транслятора L2
1. Взять `f04cf1cd` (или слитый main) + WIP `631abc18`; найти краш
   (`631abc18` падает даже на 8-строчном `L2:` unit — начать с него, не с
   14 119 строк): собрать с `-g`, `gdb`/`Dr. Memory`, либо бинарной
   вырезкой WIP-хунков (их 653 строки, разбитых на 4–5 тем).
2. Доказать, что «parse predef once» снимает зависание `f04cf1cd` на большом
   входе (замер времени до/после на усечённых префиксах `l2trans.lm2`: 1k,
   3k, 7k, 14k строк).
3. Прогнать генератор: `powershell -File stg/l1_baseline/l2src/make_l2trans_lm2.ps1`
   (asserts счётчиков должны сойтись; при изменении `l2trans.lm1` менять
   ожидания генератора осознанно, в том же коммите).
4. Цепочка: `l2trans.exe l2trans.lm2 → l2trans_self.lm1` → пин L1 → C → gcc →
   `l2trans_self.exe`; затем `l2trans_self.exe l2trans.lm2 → l2trans_self2.lm1`
   и **байтовое сравнение** `l2trans_self.lm1` == `l2trans_self2.lm1`
   (неподвижная точка L2-уровня); затем полный `run_l2trans.ps1` и
   `run_graph_abi.ps1` с `-TranslatorPath` на self-сборку.
5. Оформить это как раннер `stg/l1_baseline/l2src/run_l2trans_selfhost.ps1`
   (три стадии, хеши, evidence.json) — по образцу моих `run_port_*.ps1`
   (см. `run_port_owned_ranges.ps1`: PENDING/PASS с закреплённым
   «pending»-отклонением, чтобы раннер падал в обе стороны).
6. Каждый недостающий конструкт компилятора — как в портах: минимальный
   fixture в `l2src/tests/unit_*.lm2`, регистрация в `run_graph_abi.ps1`,
   **tripwire** (сначала `throw` в блоке проверок, чтобы доказать, что блок
   исполняется), потом реализация.
Готово, когда неподвижная точка достигнута на пине L1 **и** на кандидате из
§6.2, и все гейты зелёные на self-сборке.

### 6.4 Оставшиеся runtime-модули на L2
Порядок: `lmx_msg_mail_chain` (тикет 111000), `lmx_graph_copy_owned`
(тикет 152000), затем `lmx_branch_owned`/`lmx_value_owned`/`lmx_chars_owned`/
`lmx_array*` (аллокаторы — у них bootstrap-проблема в квадрате: они и есть
аллокация), `lmx_merge_owned`, `lmx_message_graph_copy`, `lmx_msg_*` (sched,
visit, liveness, history, roots_stale), `lmx_message.lm1`; в самом конце —
C-файлы `lmx_message_exec.c`/`lmx_message_host.c` (это Message executor и
host: см. модель §III и `LMX_MSG_EXEC_HOST_V0.txt`). Для каждого — библиотечная
единица (`predef:` заголовка, без `main`), parity-раннер против **неизменного**
L1-оракула, два прогона, tripwire на поведение, запись в `FABLE_GRAPH_ABI.txt`.
Как только самосборка (§6.3) станет реальной, порядок загрузки первого Message
(§4.6) надо решить в компиляторе, а не редиректами в тестах.

### 6.5 Парсер на L2
Из 25 `parser_*.lm2` Codex собрать целый `parser.lm2`, эквивалентный
`l1src/parser.lm1`, с дифференциальным раннером против 620 (тот же
`run_p0_meta.ps1`, но с L2-сборкой парсера). Синтаксические доработки (`{}`,
`c.*`) — только с голденами из 620 и с тестами в `tests/l1`. Полоса
независима от §6.3–6.4 и может идти параллельно (в т.ч. частично — Claude,
если модуль изолирован и не касается транслятора; см. §7.4).

### 6.6 Модель, часть V, шаги 5–10 — что уже есть, чего нет
По документам и коду: typed status/result/throw и merge внутри метода —
есть (`08a6c1e4`, `162faac2`); Message creation через общий copier и
атомарная публикация — есть (`46c11da2`, `e180f719`); ownership transfer /
deliver — есть (`a787198d`, `cae59e50`); nonmoving GC по типизированным
корням — частично (`9673bf2e`: PRIMITIVE/METHOD корни); D7 exec/locks —
инвентарь не закрыт (Grok закрыт); сквозной сценарий §36 — не доказан одним
прогоном. После §6.1–6.3 пройти по §V шагам 5–10 и на каждый механизм иметь
один раннер с evidence. Не считать «есть коммит» = «механизм доказан».

### 6.7 mixa_manager как потребитель ядра
Он не блокирует ядро и не блокируется им, кроме компилятора. После §6.1 его
24 парити-раннера — первый независимый тест библиотечной эмиссии на реальном
коде. Дальше — §7.

---

## 7. Работа с Claude

### 7.1 Протокол (из `PROTOCOL.txt`, `INBOX_WATCHERS.md`)
- Тикет: файл `work_chat/claude/inbox/YYYYMMDD-HHMMSS-slug.txt` (локальное
  время, slug латиницей через дефис — так делал Codex; чистый `HHMMSS` без
  slug использует Михаил для check-in'ов). Весь файл — текст тикета,
  **по-английски**.
- Он претендует на тикет копией в `seen/`, отвечает файлом **с тем же
  basename** в `outbox/`. `seen` = взял, не = сделал. В ответе ищи статус
  (WORKING/BLOCKED/STAGE DONE/DONE), коммит, файлы, команды, exits, счётчики,
  хеши. Он всегда сверяет пин `65D5…` — при промоушене (§6.2) сообщить.
- **Не оставлять его без тикета**: `TICKET_RULES` называет пустой inbox при
  оставшейся авторизованной работе task starvation. Его idle-ответы
  (`…-idle-request-next-ticket`, ответы на check-in'ы) — сигнал выдать
  следующий. Норма его оборота по одному модулю сегодня — 20–60 минут.
- Пробуждение Claude, если outbox молчит при неотвеченном тикете >30 мин:
  `WAKING_CLAUDE.txt` §4 — положить сообщение в inbox, затем **точечно** убить
  его ватчер по heartbeat (`build/claude_watch/heartbeat.txt`: pid + время
  старта + путь скрипта — все три совпали). Никаких широких `Stop-Process`
  по подстроке.

### 7.2 Шаблон тикета (тот, что принимает и выполняет Claude)

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

Для не-портовых задач (стадии приложения) — та же рамка, но требования из
`OPENCODE_HANDOFF_20260911.md` (очереди 1–5), `FIRST_VERSION.txt`,
`PORT_OF_CLEARSHELL.txt`: сохранять дельты владельца, не «упрощать по
аналогии», вся новая логика приложения — L1/L2, не C.

### 7.3 Очередь для Claude
1. Порты в порядке §4.8 (по одному модулю на тикет; связки типа
   `remove` → `remove_confirm` → `selection_walk` можно давать подряд).
2. После §6.1 — «rerun all 24 parity runners on the integrated translator,
   report exits» (один тикет).
3. Затем стадии приложения из `OPENCODE_HANDOFF` очередей 3–4 (файловый
   менеджер: все операции и панели; процесс/консоль) — уже на L2.
4. Возможные изолированные куски ядра, **не** трогающие транслятор и
   runtime: дифференциальные раннеры парсера, голдены p0_tree_contract,
   документация раннеров. Давать их только как изолированные файлы с
   точными путями и критериями; Claude сам предупреждает о пересечениях.

### 7.4 Границы, которые Claude не переходит
Не редактирует `stg/l1_baseline/l2src/l2trans.lm1`, `lmx*.lm1/.h`, `l1src/*`,
главные документы; не ребейзит и не force-push'ит; не заводит веток без
явной просьбы; не пишет C вместо L1/L2 (`CODING_RULES.txt` §0); не трогает
пин компилятора; ничего Grok'у. Если он спрашивает о границе — ответить в
следующем тикете (как в 171800), а не оставлять вопрос висеть.

### 7.5 Как читать его ответ
DONE = коммит на main + файлы + команды + exits + счётчики + хеши + честные
ограничения. «Barrier» с точным line/diagnostic = приемлемый DONE oracle-side,
пока компилятор на main старый; после §6.1 — уже нет. ACK/план = WORKING.
Проверять по `git log origin/main -1 -- mixa_manager/<module>.lm2`, не по
словам. Не просить повторять идентичный прогон без причины.

---

## 8. Инструменты и гейты

| Команда (из `C:\Nyasha_Planet\lingvamyxa` или указанного worktree) | Что доказывает | Когда | Время |
| --- | --- | --- | --- |
| `stg/l1_baseline/l2src/run_graph_abi.ps1` | Мой гейт graph ABI/frontend: selftests 63/70/261, 134 fixtures, 41 негатив, per-fixture assertions | После любой правки `l2trans.lm1`, `lmx*` | ~2 мин |
| `stg/l1_baseline/l2src/run_l2trans.ps1` | Исторический L2 suite (`l2trans gen2 ok`), splice-drives | На границе интеграции, после frontend-правок | ~5 мин |
| `stg/l1_baseline/l2src/run_lmx.ps1` | Message runtime, Exec (`l2 lmx gen2 ok`) | После правок `lmx_message*`, runtime | ~5–10 мин |
| `stg/l1_baseline/l2src/run_port_{msg_storage,msg_slots,msg_path_storage,owned_ranges,msg_blocks}.ps1` | Паритет L2-модуля с L1-оракулом, дважды | После правок модуля или компилятора | ~1 мин каждый |
| `stg/l1_baseline/gate.ps1` | **Полная самосборка L1** (buildCore → gen0..gen3 → suites) — перезаписывает shared-инструмент | Только при изменении **самого L1** (§6.2); никогда «на всякий случай» | ~4+ мин |
| `tests/p0_tree_contract/run_p0_meta.ps1` | Паритет парсера с эталоном 620 | После правок `parser.lm1` | ~1 мин |
| `mixa_manager/run_mixa.ps1` | Полная регрессия приложения (Claude) | Claude, после каждого порта | минуты |
| `mixa_manager/run_mixa_<module>_l2_parity.ps1` | Паритет L1↔L2 одного модуля приложения | Claude; после §6.1 — все 24 | ~1 мин каждый |
| `work_chat/fable/scripts/test_watch_inbox.ps1`, `test_watch_peer_outbox.ps1` | Ватчеры (31/31, 15/15) | При сомнении в ватчере | ~3 мин |

Флаги C во всех раннерах: `-std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types
-Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int`;
gcc `C:/Qt/Tools/mingw1310_64/bin/gcc.exe`. Быстрая пересборка транслятора
для проб — как в моём `/tmp/qb.sh` этой сессии: пин L1 → `l2trans.c` →
gcc с `-I . -I lm1/build … l2src/lmx_poll_stub.c`.

Ловушки инструментов (все стоили времени; подробнее в памяти этого чата):
- PowerShell 5.1 + `$ErrorActionPreference=Stop` + stderr нативной программы =
  падение; вызывать через `cmd /c "… > log 2>&1"` (так делают все раннеры).
- Bash-heredoc, питающий Python, **съедает обратные слэши** (`\node`, `\n`,
  `\x`): скрипты правок `.lm1` писать через Write в scratchpad и вызывать
  `python file.py`; в тексте использовать `chr(92)`/`@B`-плейсхолдер.
- В PowerShell-регексах `'\node'` — это newline+`ode`; `\r?$` при записи
  через Python-heredoc превращается в реальный CR. Проверять байты `od -c`.
- `[ -z "$stderr" ]` — не признак успеха: проверять **exit code**; молчаливый
  `return: 1` в транслятор — реальный класс дефекта (был в `l2_emit_public_sig`).
- Нельзя `cd` в Bash-вызовах: рабочая директория сессии «прилипает».
  Использовать абсолютные пути и `git -C`.

---

## 9. Дисциплина (уроки, которые уже стоили часов)

1. **Ватчер**: армить только через фоновой механизм инструмента, никогда `&`;
   доказательство — свежий heartbeat (`state: waiting`, возраст в секундах,
   живой pid), **не** счёт процессов (ловит чужие каналы); прочитал результат
   — в том же ходе перевзвёл; каждый прочитанный тикет — копия в `seen/`.
   В этой сессии ватчер лежал трижды; последний раз — 6 часов, и за это время
   пришли два тикета Codex (§4.6). После долгого хода **первым делом**
   проверить оба heartbeat'а.
2. **Новая проверка обязана уметь падать**: tripwire (`throw`) в блоке
   assertions, мутация источника — и только потом доверие. Девять fixtures
   когда-то «проходили» мёртвым кодом.
3. **Одно число — один смысл**: значение, кодирующее заодно «отсутствует» или
   второе индексное пространство, дало три дефекта.
4. **Не обобщать с одного наблюдения**; писать «измерено» только про то, что
   запускалось, с командой и exit.
5. **Сначала заподозрить пробу**: grep, как это спеллит соседний код, прежде
   чем объявлять, что язык «не умеет».
6. **Заметки — сразу в коммит и push** (`FABLE_GRAPH_ABI.txt`, этот файл):
   незакоммиченное пропадает — Codex тому пример.
7. **Отказ — не правило**: ограничение, добавленное для удобства эмиттера, —
   гэп; пометить или снять.
8. **Библиотечная единица не может обслуживать runtime, который строит её
   граф** (bootstrap-проблема, §4.6) — измерять до написания раннера.
9. Статусы по `TICKET_RULES`: план/ACK = WORKING; BLOCKED только с точным
   шагом и наименьшим нужным решением; DONE — с evidence; STAGE DONE —
   продолжать без напоминания.
10. Никогда: `git reset/stash/clean` общего дерева, force-push main, замена
    пина, правка чужих in-flight файлов, коммит чужих dirty-хунков как своих.

---

## 10. Открытые вопросы Михаилу (заданы 2026-09-13 в чате)

1. **Интеграция веток** — принять §5 (codex/core-integration становится
   базой main одним слиянием, этот чат делает)? Или иной порядок?
2. **Стабильный L1** — принять процедуру §6.2 (полный gate на слитой ветке →
   промоушен нового пина → замена хеша в документах), или пин 65D5 остаётся
   до явного решения?
3. **Ветка `codex/wip-selfhost-20260913`** — оставить как архив (рекомендую)
   или влить содержательную часть после починки краша?
4. **Два пропущенных тикета Codex** (mail_chain, graph_copy) — считать
   очередью ядра после интеграции (рекомендую) или отложить ради §6.3?

Ответы записать сюда же, ниже, с датой.

Ответы (Михаил, 2026-09-13 ~18:00, в чате: «все 4 вопроса — как
рекомендуешь»):
1. **Да.** `codex/core-integration` становится базой `main` одним слиянием по
   процедуре §5; делает этот чат. Это **первый шаг** для следующей модели.
2. **Да.** Полный `gate.ps1` на слитой ветке; при зелёном — промоушен нового
   пина и замена хеша во всех документах (§6.2). До этого 65D5 read-only.
3. **Архив.** `codex/wip-selfhost-20260913` остаётся как есть; содержательную
   часть (разбор predef один раз, external functions) вливать только после
   починки краша и с собственным evidence (§6.3).
4. **Очередь ядра после интеграции.** Тикеты 111000 (`lmx_msg_mail_chain`) и
   152000 (`lmx_graph_copy_owned`) выполняются после §6.1–6.2, до §6.3 или
   параллельно с ним, если самосборка упрётся в долгую бисекцию.

Английская версия этого документа: `CORE_LEAD_INSTRUCTION_20260913_en.md`
(по просьбе Михаила, чтобы модель лучше понимала). При расхождении верна та,
что обновлена позже; обновлять обе.

---

## 11. Модели и effort для долгой автономной работы по этому ТЗ

Совет из опыта этой сессии (Opus 5 на ядре с середины ночи; Sonnet 5 Medium
на приложении; Fable 5.1 на этом документе):

- **Ведущий чат (ядро, интеграция, самосборка): Opus 5, effort high.**
  Medium хватало на порты модулей с готовым паттерном; но §6.1–6.3 — это
  разрешение конфликтов в 14-тысячестрочном транслятором, бисекция краша и
  доказательство неподвижной точки — там medium начнёт «обобщать с одного
  наблюдения» и чинить тесты под зелень. Xhigh/max — только точечно: на
  краш §6.3.1 и на первый проход слияния, если high упрётся.
- **Claude / mixa_manager: Sonnet 5 Medium — оставить.** Порты по шаблону у
  него идут ровно и честно (oracle-side parity, точные диагностики барьера,
  ни одного обхода типовой модели за 24 модуля). Поднимать до high имеет
  смысл только на стадиях приложения с новой логикой (очереди 3–4), не на
  портах.
- **Fable 5.1 (max)** — для документов/handoff'ов и разбора спорных мест
  модели; для долгого кодинга дороже и не нужно.
- Независимо от модели: включить в стоящие инструкции проверку heartbeat'ов
  в начале каждого хода и запрет `&`-запусков (§9.1) — это стоило больше
  часов, чем любая нехватка «интеллекта».
