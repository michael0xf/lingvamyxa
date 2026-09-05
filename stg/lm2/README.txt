stg/lm2 — staging primitive descriptions and converters
=======================================================

Where primitives are described today
------------------------------------

Spec (Lingvamyxa_spec.txt):

    §2.2          substrate, primitive leaves, conversion keys
    §2.2.2        a primitive is a memory cell + a description Structure
    §2.2.2.2      contract lattice: flag ⊂ count ⊂ integer ⊂ ratio ⊂ float ⊂ complex
    §2.2.2.6      void is semantic:none (no cell); unit is empty Structure
    §2.2.3        primitive leaves of implements(varA varB Consumer)
    §2.2.4        converters are explicit receivers selected by merge key
    §6.6          C99 / GMP / MPFR / decNumber spellings
    §19.21.3      used-tree algorithm

Live bootstrap now loads these tables (trans reads convert=keyed/forbidden
at primitive leaves of implements; converter insertion on substitution
edges is still later):

    lm2/primitive.lm2     loaded at trans start; convert column keyed/forbidden
    lm2/convert.lm2       merge-keyed converter rows
    lm2/core.lm2          class kinds for void, int, FILE, …

This directory is a *maximal* profile: as many merge-keyed converters as we
can name, plus stubs of the receivers trans will insert on substitution
edges. It is not yet wired into trans.

EN: implements never calls a converter. At a primitive leaf it only asks:
does Consumer follow description fields (width, range, signed, opaque)? If
not, the leaf is a moved word — true when conversion is not forbidden and a
key exists (or A and B are the same description). Empty uses(Consumer varB)
is true. Converter failure is a later edge diagnostic, not implements false.

RU: implements конвертер не вызывает. На листе-примитиве смотрит, ходит ли
Consumer по полям описания (width, range, signed, opaque). Если нет — это
перенос слова: true, когда конвертация не запрещена и ключ есть (или A и B
одно описание). Пустой uses(Consumer varB) — true. Сбой конвертера —
диагностика ребра, не false у implements.

Therefore for two primitives and a *word-moving* Consumer Structure
(WordMover below), implements is almost always true in this profile: every
cell-bearing pair has a key (widen, narrow, text, flag, …). Exceptions:

    void / None     no cell; no key to int/FILE/Text (bug0)
    Consumer that follows width/range/opaque on the required primitive
                    then those fields must exist on varA (not a converter job)
    File\close      File is a Structure; a bare u32 does not have close

Files
-----

    primitive.lm2       description rows; convert=keyed means look in convert.lm2
    convert.lm2         keys: from, to, forbidden, receiver, impl
    convert_impl.lm2    stub receivers (C99 now; GMP/MPFR/decNumber later)
    consumers.lm2       WordMover vs DescriptionWalker
    File.lm2            spec example: u32 → File.handle, not File\close

When trans grows merge-keyed insertion (§2.2.4), point it at these tables
via predef/import, then fill convert_impl.lm2 for real.

Где описаны примитивы сейчас
----------------------------

Спека §2.2–2.2.4, §6.6, §19.21.3. Живой bootstrap: lm2/primitive.lm2
(колонка convert пока None). Здесь — максимальный набор ключей, чтобы
implements(int, u32, WordMover), implements(DecimalText, int, WordMover)
и т.п. были true за счёт ключа, а trans на ребре вставлял приёмник.
