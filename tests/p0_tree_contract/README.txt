P0 tree-contract, initial block/short-form batch
================================================

Provenance
----------
Old parser reference: git 620db8612c32569c8dd507cca135d5d076144e9f
Rebuilt isolated from tracked C (not current lm0, not a whole old selfbuild):

  gcc -std=c99 -I . -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE
    lm1/build/printTree.lm1.c lm1/build/parser.lm1.c lm1/build/own.lm1.c

Example catalogue (not the behavior oracle): spec c999038270ae0d342f912fb95e03f2b26fc6551d
sections 3.4/4/16, especially 4.2-4.5. Fixtures were first authored and dumped
in lingvamyxa_old_worked_version/tests/p0_tree_contract, then copied byte-identical
to this directory.

Goldens are 620db86 printTree stdout. That printer omits frame flags/spans and
skips null field values; a later walker must assert those consumer fields on
the SAME source. Do not regenerate goldens from root/STG on mismatch.

Known prose vs 620db86 disagreements (do not "fix" the old parser):
- C_nested_short (spec ~2070): `. . ShortForm2_arg2` is a two-step increase;
  620db86 rejects P0 parse error 13 at 2:5.
- F_trailer_word (spec ~2500): prose wants 3 root fields (anon, atom, anon).
  620db86 printTree: 2 root fields; trailer_word sits inside the second
  structure with nested block1, not as a root-level atom.
- B_call_int_a_semi_b: prose `call(int(a), (b))`; actual second field is atom
  "b", not an anonymous Structure wrapping b.
- B_repeat_decl_p0: P0 does not repeat head `int` onto `y, 10`; second item is
  a headless Structure. Repeat-head is a consumer profile, not P0.

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
C 2070 nested short             C_nested_short                    REJECT code13 @2:5
D 1936 L1: utf8: fence          D_utf8_inline / D_utf8_vertical   utf8 owns "raw text"
E 2456 dotted pair              E_dotted_anon_pair                {body1} {body2}
E dash pair                     E_dash_anon_pair                  same two anons
F 2500 trailer_word             F_trailer_word                    DISAGREES with 3-field prose
G 2468 nested fn cut            G_fn_nested_cut                   inner return=body; outer return=trailer
G 2478 nested fn end:inner      G_fn_nested_end                   inner trailer end:inner
