#ifndef L2_TEST_STRUCT_TYPEDEF_H
#define L2_TEST_STRUCT_TYPEDEF_H

/* Two C declaration shapes the manager headers use for aggregate handles.
   A forward typedef of an incomplete struct, and a complete struct whose body
   contains parentheses -- a function pointer -- which sit inside braces and
   must not be mistaken for a function declaration at file scope. */

typedef struct L2TestOpaque L2TestOpaque;

typedef struct L2TestRecord {
    int value;
    int (*callback)(int);
} L2TestRecord;

#endif
