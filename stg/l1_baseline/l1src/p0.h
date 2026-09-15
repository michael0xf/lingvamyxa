#ifndef L1SRC_P0_H
#define L1SRC_P0_H

#include <stddef.h>

#define LM_UNUSED(x) ((void)(x))
#define LM_P0_NODE_STRUCTURE 1
#define LM_P0_NODE_FRAME 2
#define LM_P0_NODE_ATOM 3
#define LM_P0_NODE_DISABLED 4
#define LM_P0_FRAME_COLON 1U
#define LM_P0_FRAME_COMPACT 2U
#define LM_P0_FRAME_INLINE_BODY 4U
#define LM_P0_FRAME_SEPARATOR_CLOSED 8U
#define LM_P0_NODE_INACTIVE 1U
#define LM_P0_NODE_MIX 2U
#define LM_P0_NODE_POSITIONAL_SKIP 4U
#define LM_P0_TRAILER_TAIL_CUTTER 1U
#define LM_P0_TRAILER_COLON 2U

#define LM_P0_STREAM_EVENT_ITEM 1
#define LM_P0_STREAM_EVENT_DELIM 2
#define LM_P0_STREAM_EVENT_BLOCK_STRING 3
#define LM_P0_STREAM_EVENT_DISABLED_BLOCK 4
#define LM_P0_STREAM_EVENT_MIX 5
#define LM_P0_TRAILER_ROLE_NONE 0
#define LM_P0_TRAILER_ROLE_DASH_CUTTER 1
#define LM_P0_TRAILER_ROLE_END 2
#define LM_P0_TRAILER_ROLE_RETURN 3
#define LM_P0_TRAILER_ROLE_UNTIL 4
#define LM_P0_DASH_FENCE_NONE 0
#define LM_P0_DASH_FENCE_VALID 1
#define LM_P0_DASH_FENCE_TOO_LONG 2
#define LM_P0_DASH_FENCE_TRAILING_TEXT 3
#define LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON 1U
#define LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL 2U
#define LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL 4U
#define LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS 8U
#define LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR 16U
#define LM_P0_MAX_FENCE_LENGTH 80U
#define LM_P0_LAYOUT_DELIMITER_STACK_LIMIT 256U
#define LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP 0
#define LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY 1
#define LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE 2
#define LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY 3
#define LM_P0_POSTPROCESS_NODE 1
#define LM_P0_POSTPROCESS_STRUCTURE 2
#define LM_P0_POSTPROCESS_TRAILER 3
#define LM_P0_POSTPROCESS_FRAME_WRAP 4
#define LM_P0_DUMP_NODE 1
#define LM_P0_DUMP_STRUCTURE 2
#define LM_P0_DUMP_TRAILER 3

typedef unsigned char uchar;
typedef int LmP0NodeKind;
typedef unsigned LmP0FrameFlags;
typedef unsigned LmP0NodeFlags;
typedef unsigned LmP0TrailerFlags;
typedef int LmP0StreamEventKind;
typedef int LmP0TrailerRole;
typedef int LmP0DashFenceStatus;
typedef unsigned LmP0FieldParseFlags;
typedef int LmP0FieldParseLoopContinuation;

typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);

typedef struct LmOwnPtrStack {
    void **items;
    size_t count;
    size_t capacity;
    LmOwnDelete delete_item;
} LmOwnPtrStack;

typedef struct LmOwnArena {
    LmOwnPtrStack *allocations;
    int frozen;
} LmOwnArena;

typedef struct LmP0Node LmP0Node;
typedef struct LmP0Field LmP0Field;
typedef struct LmP0Trailer LmP0Trailer;
typedef struct LmP0Document LmP0Document;

typedef struct LmP0Text {
    const char *data;
    size_t length;
} LmP0Text;

typedef struct LmP0Span {
    size_t line;
    size_t column;
    size_t offset;
    size_t length;
} LmP0Span;

typedef struct LmP0Diagnostic {
    int code;
    size_t line;
    size_t column;
    char message[256];
} LmP0Diagnostic;

typedef struct LmP0Structure {
    LmP0Field *first_field;
    LmP0Field *last_field;
    size_t field_count;
    LmP0Trailer *trailer;
} LmP0Structure;

struct LmP0Trailer {
    LmP0Text *spelling;
    unsigned flags;
    LmP0Structure *body;
};

typedef struct LmP0Frame {
    LmP0Text *head;
    unsigned flags;
    LmP0Structure *body;
    LmP0Trailer *trailer;
} LmP0Frame;

typedef struct LmP0NodeAs {
    LmP0Structure *structure;
    LmP0Frame *frame;
    LmP0Text *atom;
} LmP0NodeAs;

struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span *span;
    LmP0NodeAs *as;
};

struct LmP0Field {
    LmP0Node *value;
    LmP0Field *next;
};

typedef struct LmP0StreamEvent {
    LmP0StreamEventKind kind;
    unsigned node_flags;
    size_t level;
    const char *text;
    size_t text_length;
    size_t line;
    size_t column;
    size_t offset;
} LmP0StreamEvent;

struct LmP0Document {
    char *source;
    size_t source_length;
    LmP0Node *root;
    LmP0Diagnostic *diagnostic;
    LmOwnArena *source_owner;
    LmOwnArena *token_arena;
    LmOwnArena *tree_arena;
    LmOwnArena *diagnostic_arena;
    int owners_initialized;
    int frozen;
};

typedef struct LmP0PendingDelimiter {
    int active;
    LmP0StreamEvent *event;
} LmP0PendingDelimiter;

typedef struct LmP0PendingMix {
    LmP0StreamEvent *events;
    size_t count;
    size_t capacity;
} LmP0PendingMix;

typedef struct LmP0IndentStack {
    size_t *columns;
    size_t count;
    size_t capacity;
} LmP0IndentStack;

typedef struct LmP0DisabledState {
    int body_started;
    int pending_item;
    size_t base_level;
    size_t top_level;
    size_t pending_level;
} LmP0DisabledState;

typedef struct LmP0Stack {
    LmP0Structure **parents;
    LmP0Node **owners;
    unsigned char *hard;
    size_t capacity;
} LmP0Stack;

typedef struct LmP0Dump {
    char *data;
    size_t length;
    size_t capacity;
    int failed;
} LmP0Dump;

typedef struct LmP0DumpFrame {
    int phase;
    const LmP0Node *node;
    const LmP0Structure *structure;
    const LmP0Trailer *trailer;
    const LmP0Field *field;
    size_t indent;
} LmP0DumpFrame;

typedef struct LmP0FieldParseLoopFrame {
    LmP0IndentStack *indent_stack;
    LmP0Structure *structure;
    const char *text;
    size_t length;
    size_t line;
    size_t column;
    size_t offset;
    unsigned flags;
    size_t short_source_level;
    size_t layout_base_level;
    size_t i;
    size_t current_source_level;
    int allow_empty_fields;
    int expect_field;
    int headless_group_after_separator;
    int indent_stack_owned;
    LmP0FieldParseLoopContinuation continuation;
    LmP0Node *node;
    size_t start;
    size_t close_index;
} LmP0FieldParseLoopFrame;

typedef struct LmP0PostprocessFrame {
    int phase;
    LmP0Node *node;
    LmP0Structure *structure;
    LmP0Trailer *trailer;
    LmP0Field *field;
} LmP0PostprocessFrame;

int lm_p0_parse_string(const char *source, LmP0Document **out_document);
int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document);
int lm_p0_parse_file(const char *path, LmP0Document **out_document);
void lm_p0_document_destroy(LmP0Document *document);
const LmP0Node *lm_p0_document_root(const LmP0Document *document);
LmP0Node *lm_p0_document_mutable_root(LmP0Document *document);
const LmP0Diagnostic *lm_p0_document_diagnostic(const LmP0Document *document);
const LmP0Structure *lm_p0_node_structure(const LmP0Node *node);
const LmP0Frame *lm_p0_node_frame(const LmP0Node *node);
const LmP0Text *lm_p0_node_atom(const LmP0Node *node);
const LmP0Trailer *lm_p0_structure_trailer(const LmP0Structure *structure);
const LmP0Text *lm_p0_frame_head(const LmP0Frame *frame);
const LmP0Structure *lm_p0_frame_body(const LmP0Frame *frame);
const LmP0Trailer *lm_p0_frame_trailer(const LmP0Frame *frame);
const LmP0Text *lm_p0_trailer_spelling(const LmP0Trailer *trailer);
const LmP0Structure *lm_p0_trailer_body(const LmP0Trailer *trailer);
const char *lm_p0_node_kind_class_name(LmP0NodeKind kind);
char *lm_p0_dump_alloc(const LmP0Document *document);
void lm_p0_free(void *ptr);

#endif
