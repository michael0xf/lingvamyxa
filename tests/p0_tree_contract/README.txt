P0 tree-contract, initial block/short-form batch
================================================

Metadata oracle (p0_meta)
-------------------------
Pretty printTree omits flags, spans and null field slots. The inspection
driver is tests/p0_tree_contract/p0_meta_dump.c. It is compiled twice:

  620:  tracked parser.lm1.c + own.lm1.c from
        C:\Nyasha_Planet\lingvamyxa_old_worked_version at 620db861
        (hashes pinned in run_p0_meta.ps1). That checkout is not modified.
  current: repo lm1/build/parser.lm1.c (own is already inlined).

Goldens are *.meta.txt from the 620 dump (kinds, flags, spans,
spelling bytes/length, null vs empty). run_p0_meta.ps1 rebuilds both
exes, checks 620 dump against those goldens, then compares current
after masking LM_P0_TRAILER_COLON (bit 2) into a separate colon_bit
field. Current must REJECT return_colon_empty_trailer and
return_colon_comment_trailer (P0 error 32). C_nested_short stays the
negative 13@2:5. C_nested_short_ok.lmx is the vertical positive
counterpart. Comparator is ordinal; a mutated golden is required to fail.

Run: powershell -File tests/p0_tree_contract/run_p0_meta.ps1

Harness (153600): child ExitCode is asserted (0 accept vs 1 expected
reject vs crash). Status files persist under build/p0_tree_contract.
TRAILER_COLON is not globally erased: listed fixtures pin current
colon_bit identity as an ordered sequence of colon_bit= fields, not
a count. F_mixed_trailer is 0,1 (bare `return` then `end: b`); a
same-count relocation to 1,0 must fail. Other versioned seqs:
A_header_then_body 1, F_eq_fence 1, F_eq_inner_longer 1,
F_eq_long_opener 1, F_star_long 1, G_fn_nested_cut 1,
G_fn_nested_end 1,1, return_colon_vertical_body 1.
Sensitivity injects kind/flags/span/bytes/order/null/colon-bit faults
without editing committed goldens, including that relocation.
Quote/fence fixtures: Q_triple_* (A+1 is value-layer; P0 keeps
source spelling; exact-3 inline quotes unchanged). F_eq_fence;
F_eq_inner_longer (=== opener, ==== inside as payload);
F_eq_long_opener (==== opener, === inside as payload, nonempty fn).
F_star_fence is a raw *** comment (spec ~3948, 3/4/5-star matching
fences) wrapped in empty `doc:`; 620 field_count=0 because the
comment consumed the body, not because 620 lacks *** comments.
Current P0 empty-colon rejects that empty `doc:`. Keep it as the
empty-colon case. F_star_long is the nonempty long-star comment:
**** opener, shorter *** and longer ***** inside as comment,
int/return survive. invalid_eq_unclosed (20),
invalid_triple_unclosed (4).


Provenance
----------
Old parser reference: git 620db8612c32569c8dd507cca135d5d076144e9f
Isolated checkout: C:\Nyasha_Planet\lingvamyxa_old_worked_version
Rebuilt isolated (not current main build\lm0, not a whole old selfbuild).
Tracked 620 inputs are parser.lm1.c and own.lm1.c only. printTree.lm1.c is
an untracked gitignored driver; do not call the three-file gcc line a
tracked-C rebuild:

  gcc -std=c99 -I . -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE
    lm1/build/printTree.lm1.c lm1/build/parser.lm1.c lm1/build/own.lm1.c

SHA-256 of that isolated rebuild (do not substitute main build\lm0):
  printTree_620db86.exe  CB564AD6FF52F35E918FFBAE6A6E2E166147DADCAAE445F9D5A2526A77801EF3  292976 bytes
  parser.lm1.c          A344B975BD541757AB1CCA89FDC04DD9467C17D041B9A42BC3BE09205E254DDE
  own.lm1.c             8EF21B1AB0561916762829C07B6AD45B7E3A98BC42CA1E1F48D830B6B2C783FA
  printTree.lm1.c       ECC9E9FBFE52E8DEFBACA0F47796167DB36E8B67830D764B4F735F996DB5729D
                        (NOT tracked at 620db86; gitignored 36440-byte Sep6 driver)

Example catalogue (not the behavior oracle): spec c999038270ae0d342f912fb95e03f2b26fc6551d
sections 3.4/4/16, especially 4.2-4.5. Fixtures were first authored and dumped
in lingvamyxa_old_worked_version/tests/p0_tree_contract, then copied byte-identical
to this directory.

Goldens are 620db86 printTree stdout. That printer omits frame flags/spans and
skips null field values; a later walker must assert those consumer fields on
the SAME source. Do not regenerate goldens from root/STG on mismatch.

Known prose vs 620db86 disagreements (do not "fix" the old parser):
- F_trailer_word (spec ~2500): prose wants 3 root fields (anon, atom, anon).
  620db86 printTree: 2 root fields; trailer_word sits inside the second
  structure with nested block1, not as a root-level atom.
- B_call_int_a_semi_b: prose `call(int(a), (b))`; actual second field is atom
  "b", not an anonymous Structure wrapping b.

Resolved historical prose error, not a parser/spec discrepancy:
- C_nested_short: the two-dot line after inline `ShortForm1: 1 ShortForm2: 2`
  is INVALID. 620db86 and current P0 reject `P0 parse error 13 at 2:5`.
  Keep this negative fixture and diagnostic. Correct spelling is vertical
  `ShortForm1: 1` then `. ShortForm2: 2` (physical levels 0 -> 1 -> 2 -> 1).
  Repeat-head on `B_repeat_decl_p0` is a consumer profile, not P0, and is
  not itself a prose disagreement.

Case table (spec c999038 -> fixture -> 620db86)
----------------------------------------------
user printf inline+vertical     printf_inline_then_vertical.lmx   5 fields: a b c d + {e f g}
user printf vertical-only       printf_vertical_only.lmx          2 fields: a + {b c d}
A 1770/1900 inline vs compact   A_inline_f_abc / A_compact_f_paren  same tree f(a,b,c)
A 1904 vertical no inline       A_vertical_no_inline              3 direct args
A 1914 header then body         A_header_then_body                fn 4 fields + end:test
B 1953 comma                    B_comma                           head(arg0,arg1)
B 1961 semicolon                B_semicolon                       head(arg0) then {arg1}
B 2030 call int a b; ret        B_call_int_ab_ret                 call(int(a,b), ret(int))
B 2038 call int a; b            B_call_int_a_semi_b               call(int(a), b)  [atom b]
B 2048 print a b; c             B_print_ab_semi_c                 print(a,b) then {c}
B 1973 int x,5; y,10            B_repeat_decl_p0                  int(x,5) then {y,10}
C nested short (invalid)        C_nested_short                    REJECT code13 @2:5 (resolved prose error)
return bare trailer             return_bare_trailer               620 accepts; dump spelling=return fields=0
return: empty trailer           return_colon_empty_trailer        620 accepts same dump; current P0 rejects 32
return # comment trailer        return_bare_comment_trailer       620 accepts; same dump as bare
return: # comment trailer       return_colon_comment_trailer      620 accepts same dump; current P0 rejects 32
return: vertical 5 + 5          return_colon_vertical_body        620 and current P0 accept; trailer fields=3

620 printTree dumps of the four return-trailer fixtures are byte-identical
(tree SHA-256 1FC5520614309842BBC5C2309A653E09CC8619E55BE6BFDB91EFD10BCFCD9D09).
The dump cannot show colon vs bare. New parser stores LM_P0_TRAILER_COLON (2U)
in existing trailer flags; that bit is versioned new metadata and must be
asserted separately. Do not mask all flags in the shared walker. Main
build\lm0 is additional evidence, not the 620 oracle.
D 1936 L1: utf8: fence          D_utf8_inline / D_utf8_vertical   utf8 owns "raw text"
E 2456 dotted pair              E_dotted_anon_pair                {body1} {body2}
E dash pair                     E_dash_anon_pair                  same two anons
F 2500 trailer_word             F_trailer_word                    DISAGREES with 3-field prose
G 2468 nested fn cut            G_fn_nested_cut                   inner return=body; outer return=trailer
G 2478 nested fn end:inner      G_fn_nested_end                   inner trailer end:inner
