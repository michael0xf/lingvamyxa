#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


#include <stddef.h>
#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




typedef struct LmOwnPtrStack LmOwnPtrStack;
typedef struct LmOwnValueStack LmOwnValueStack;
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
typedef struct LmOwnArena LmOwnArena;
typedef struct LmP0Node LmP0Node;
typedef struct LmP0Field LmP0Field;
typedef struct LmP0Trailer LmP0Trailer;
typedef struct LmP0Document LmP0Document;
typedef struct LmRegistrySourceColumn LmRegistrySourceColumn;
typedef struct LmRegistrySourceLoader LmRegistrySourceLoader;
typedef struct LmTableColumnDescriptor LmTableColumnDescriptor;
typedef struct LmTableCell LmTableCell;
typedef struct LmTableRow LmTableRow;
typedef struct LmRegistryViewRow LmRegistryViewRow;
typedef struct LmTableDescriptor LmTableDescriptor;
typedef struct LmRegistryView LmRegistryView;


typedef int LmOwnEdgeKind;
typedef int LmP0NodeKind;
typedef unsigned LmP0FrameFlags;
typedef unsigned LmP0NodeFlags;
typedef unsigned LmP0TrailerFlags;
typedef int LmP0StreamEventKind;
typedef int LmP0TrailerRole;
typedef int LmP0DashFenceStatus;
typedef unsigned LmP0FieldParseFlags;
typedef int LmP0FieldParseLoopContinuation;


#define LM_OWN_EDGE_BORROWED 1
#define LM_OWN_EDGE_OWNED 2
#define LM_OWN_EDGE_LAZY_OWNED 3
#define LM_OWN_EDGE_EXTERNAL 4
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


#include <stddef.h>

typedef struct LmSlice {
    void *ptr;
    size_t length;
} LmSlice;
struct LmOwnPtrStack {
    void **items;
    size_t count;
    size_t capacity;
    void (*delete_item)(void *object);
};
struct LmOwnValueStack {
    void *items;
    size_t count;
    size_t capacity;
    size_t item_size;
};
struct LmOwnAllocationDescriptor {
    void *address;
    LmOwnArena * owner;
    size_t bytes;
    size_t element_size;
    size_t count;
    size_t rank;
    size_t level;
};
struct LmOwnLazyEdge {
    LmOwnEdgeKind kind;
    LmOwnArena * source_owner;
    LmOwnArena * target_owner;
    const void *source;
    size_t size;
    const void **patch_slot;
};
struct LmOwnArena {
    LmOwnPtrStack * allocations;
    LmOwnPtrStack * allocation_descriptors;
    LmOwnPtrStack * lazy_edges;
    int frozen;
};
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
    char message[256U];
} LmP0Diagnostic;
typedef struct LmP0Structure {
    LmP0Field * first_field;
    LmP0Field * last_field;
    size_t field_count;
    LmP0Trailer * trailer;
} LmP0Structure;
struct LmP0Trailer {
    LmP0Text * spelling;
    unsigned flags;
    LmP0Structure * body;
};
typedef struct LmP0Frame {
    LmP0Text * head;
    unsigned flags;
    LmP0Structure * body;
    LmP0Trailer * trailer;
} LmP0Frame;
typedef struct LmP0NodeAs {
    LmP0Structure * structure;
    LmP0Frame * frame;
    LmP0Text * atom;
} LmP0NodeAs;
struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span * span;
    LmP0NodeAs * as;
};
struct LmP0Field {
    LmP0Node * value;
    LmP0Field * next;
};
struct LmRegistrySourceColumn {
    const LmP0Text * name;
    const LmP0Text * descriptors[16U];
    size_t descriptor_count;
    const LmP0Text * type_name;
    size_t address_depth;
    size_t array_rank;
    int is_const;
};
struct LmRegistrySourceLoader {
    const char *error_prefix;
    int (*push_column_metadata)(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
    int (*push_table_row)(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
    int (*join_table)(void *context, const LmP0Text *source_table, const LmP0Text *target_table);
    int (*formal_param_unwrap_index)(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
    int (*positional_name_index)(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
};
struct LmTableColumnDescriptor {
    char *name;
    char *descriptor;
    LmOwnPtrStack * descriptors;
    char *type_name;
    size_t index;
    size_t address_depth;
    size_t array_rank;
    int is_const;
};
struct LmTableCell {
    char *atom;
    char *value;
    const void *node;
    const void *source;
    int explicit_none;
};
struct LmTableRow {
    LmOwnPtrStack * cells;
    const LmTableRow * source;
    size_t local_sequence;
    int source_native;
};
struct LmRegistryViewRow {
    char *table;
    char *key;
    char *payload;
    const void *payload_node;
    const void *source;
    size_t local_sequence;
};
struct LmTableDescriptor {
    char *name;
    LmOwnPtrStack * columns;
    LmOwnPtrStack * rows;
    LmOwnPtrStack * source_rows;
    LmOwnPtrStack * materialized_rows;
};
struct LmRegistryView {
    const LmRegistryView * parent;
    LmOwnPtrStack * tables;
    LmOwnPtrStack * facts;
    size_t local_fact_count;
    LmOwnPtrStack * source_tables;
    LmOwnPtrStack * class_names;
    size_t mutation_generation;
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
    LmP0Node * root;
    LmP0Diagnostic * diagnostic;
    LmOwnArena * source_owner;
    LmOwnArena * token_arena;
    LmOwnArena * tree_arena;
    LmOwnArena * diagnostic_arena;
    int owners_initialized;
    int frozen;
};
typedef struct LmP0PendingDelimiter {
    int active;
    LmP0StreamEvent * event;
} LmP0PendingDelimiter;
typedef struct LmP0PendingMix {
    LmP0StreamEvent * events;
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
    LmP0Structure * *parents;
    LmP0Node * *owners;
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
    const LmP0Node * node;
    const LmP0Structure * structure;
    const LmP0Trailer * trailer;
    const LmP0Field * field;
    size_t indent;
} LmP0DumpFrame;
typedef struct LmP0FieldParseLoopFrame {
    LmP0IndentStack * indent_stack;
    LmP0Structure * structure;
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
    LmP0Node * node;
    size_t start;
    size_t close_index;
} LmP0FieldParseLoopFrame;
typedef struct LmP0Registry {
    LmRegistryView * view;
    int loaded;
    int loading;
} LmP0Registry;
typedef struct LmP0PostprocessFrame {
    int phase;
    LmP0Node * node;
    LmP0Structure * structure;
    LmP0Trailer * trailer;
    LmP0Field * field;
} LmP0PostprocessFrame;


#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields 1
typedef void (*LmOwnDestroyFields)(void *object);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDelete
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDelete 1
typedef void (*LmOwnDelete)(void *object);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRegistrySourcePushTableRow
#define LM_LMX_TYPEDEF_DEFINED_LmRegistrySourcePushTableRow 1
typedef int (*LmRegistrySourcePushTableRow)(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRegistrySourcePushColumnMetadata
#define LM_LMX_TYPEDEF_DEFINED_LmRegistrySourcePushColumnMetadata 1
typedef int (*LmRegistrySourcePushColumnMetadata)(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRegistrySourceJoinTable
#define LM_LMX_TYPEDEF_DEFINED_LmRegistrySourceJoinTable 1
typedef int (*LmRegistrySourceJoinTable)(void *context, const LmP0Text *source_table, const LmP0Text *target_table);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRegistrySourceFrameIndexRule
#define LM_LMX_TYPEDEF_DEFINED_LmRegistrySourceFrameIndexRule 1
typedef int (*LmRegistrySourceFrameIndexRule)(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
#endif


void * (lm_own_new_zero)(size_t size);
void * (lm_own_resize)(void *object, size_t size);
char * (lm_own_copy_bytes)(const char *source, size_t length);
void * (lm_own_array_new_zero)(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * (lm_own_allocation_descriptor)(const void *address);
void (lm_own_delete)(void *object, LmOwnDestroyFields destroy_fields);
void (lm_own_delete_plain)(void *object);
void (lm_own_pointer_array_delete)(void **items, size_t count, LmOwnDelete delete_item);
void (lm_own_ptr_stack_init)(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void (lm_own_ptr_stack_destroy)(LmOwnPtrStack *stack);
int (lm_own_ptr_stack_push)(LmOwnPtrStack *stack, void *item);
void * (lm_own_ptr_stack_pop)(LmOwnPtrStack *stack);
void * (lm_own_ptr_stack_at)(const LmOwnPtrStack *stack, size_t index);
void * (lm_own_ptr_stack_top)(const LmOwnPtrStack *stack);
void (lm_own_ptr_stack_truncate)(LmOwnPtrStack *stack, size_t count);
void (lm_own_value_stack_init)(LmOwnValueStack *stack, size_t item_size);
void (lm_own_value_stack_destroy)(LmOwnValueStack *stack);
int (lm_own_value_stack_push)(LmOwnValueStack *stack, const void *item);
int (lm_own_value_stack_resize_zero)(LmOwnValueStack *stack, size_t count);
int (lm_own_value_stack_pop)(LmOwnValueStack *stack, void *out_item);
void * (lm_own_value_stack_at)(const LmOwnValueStack *stack, size_t index);
void * (lm_own_value_stack_top)(const LmOwnValueStack *stack);
void (lm_own_value_stack_truncate)(LmOwnValueStack *stack, size_t count);
int (lm_own_arena_init)(LmOwnArena *arena);
void (lm_own_arena_destroy)(LmOwnArena *arena);
void * (lm_own_arena_new_zero)(LmOwnArena *arena, size_t size);
void * (lm_own_arena_array_new_zero)(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * (lm_own_arena_allocation_descriptor)(const LmOwnArena *arena, const void *address);
char * (lm_own_arena_copy_bytes)(LmOwnArena *arena, const char *source, size_t length);
int (lm_own_arena_add_lazy_edge)(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int (lm_own_arena_promote_lazy_edges)(LmOwnArena *arena);
int (lm_own_arena_absorb)(LmOwnArena *target, LmOwnArena *source);
void (lm_own_arena_freeze)(LmOwnArena *arena);
int (lm_own_arena_is_frozen)(const LmOwnArena *arena);
int (lm_own_tree_cut)(LmOwnArena *arena);
int (lm_own_tree_cut_promote_lazy_edges)(LmOwnArena *arena);
int (lm_p0_parse_string)(const char *source, LmP0Document **out_document);
int (lm_p0_parse_bytes)(const char *source, size_t source_length, LmP0Document **out_document);
int (lm_p0_parse_file)(const char *path, LmP0Document **out_document);
void (lm_p0_document_destroy)(LmP0Document *document);
const LmP0Node * (lm_p0_document_root)(const LmP0Document *document);
LmP0Node * (lm_p0_document_mutable_root)(LmP0Document *document);
const LmP0Diagnostic * (lm_p0_document_diagnostic)(const LmP0Document *document);
const LmP0Structure * (lm_p0_node_structure)(const LmP0Node *node);
const LmP0Frame * (lm_p0_node_frame)(const LmP0Node *node);
const LmP0Text * (lm_p0_node_atom)(const LmP0Node *node);
const LmP0Trailer * (lm_p0_structure_trailer)(const LmP0Structure *structure);
const LmP0Text * (lm_p0_frame_head)(const LmP0Frame *frame);
const LmP0Structure * (lm_p0_frame_body)(const LmP0Frame *frame);
const LmP0Trailer * (lm_p0_frame_trailer)(const LmP0Frame *frame);
const LmP0Text * (lm_p0_trailer_spelling)(const LmP0Trailer *trailer);
const LmP0Structure * (lm_p0_trailer_body)(const LmP0Trailer *trailer);
const char * (lm_p0_node_kind_class_name)(LmP0NodeKind kind);
char * (lm_p0_dump_alloc)(const LmP0Document *document);
void (lm_p0_free)(void *ptr);
static int lm_registry_source_load_root(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root);





































































static int lm_registry_source_text_equals(const LmP0Text *text, const char *value);
static int lm_registry_source_text_slice_equals(const char *data, size_t length, const char *value);
static int lm_registry_source_text_slice_same(const char *left_data, size_t left_length, const char *right_data, size_t right_length);
static int lm_registry_source_node_is_ignored(const LmP0Node *node);
static const LmP0Structure * lm_registry_source_node_structure(const LmP0Node *node);
static const LmP0Frame * lm_registry_source_node_frame(const LmP0Node *node);
static const LmP0Text * lm_registry_source_node_atom(const LmP0Node *node);
static const LmP0Text * lm_registry_source_frame_head(const LmP0Frame *frame);
static const LmP0Structure * lm_registry_source_frame_body(const LmP0Frame *frame);
static const LmP0Text * lm_registry_source_trailer_spelling(const LmP0Trailer *trailer);
static const LmP0Structure * lm_registry_source_trailer_body(const LmP0Trailer *trailer);
static const char * lm_registry_source_error_prefix(const LmRegistrySourceLoader *loader);
static void lm_registry_source_error(const LmRegistrySourceLoader *loader, const char *message);
static const LmP0Field * lm_registry_source_nth_field(const LmP0Structure *structure, size_t index);
static int lm_registry_source_trailer_single_atom(const LmP0Trailer *trailer, const LmP0Text **out_text);
static int lm_registry_source_identifier_payload(const LmP0Text *atom, const char **out_data, size_t *out_length);
static int lm_registry_source_identifier_same(const LmP0Text *left, const LmP0Text *right);
static int lm_registry_source_text_all_char(const LmP0Text *text, char ch);
static int lm_registry_source_text_is_array_receiver_head(const LmP0Text *head);
static int lm_registry_source_structure_single_visible_node(const LmP0Structure *structure, const LmP0Node **out_node);
static int lm_registry_source_single_frame_node(const LmP0Node *node, const LmP0Frame **out_frame);
static int lm_registry_source_frame_formal_param_unwrap_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_registry_source_frame_positional_name_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_registry_source_formal_param_name(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_name);
static int lm_registry_source_column_descriptor_from_param_node(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_descriptor);
static size_t lm_registry_source_array_receiver_rank(const LmP0Text *head);
static int lm_registry_source_column_type_shape(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column);
static int lm_registry_source_column_name_from_param_node(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column);
static int lm_registry_source_structure_single_visible_frame(const LmP0Structure *structure, const LmP0Frame **out_frame);
static int lm_registry_source_frame_single_atom(const LmP0Frame *frame, const char *head, const LmP0Text **out_atom);
static int lm_registry_source_column_name(const LmRegistrySourceLoader *loader, void *context, const LmP0Field *field, LmRegistrySourceColumn *out_column);
static void lm_registry_source_columns_destroy(LmRegistrySourceColumn **columns, size_t count);
static int lm_registry_source_columns_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmRegistrySourceColumn **columns, size_t columns_capacity, size_t *out_count);
static int lm_registry_source_validate_named_trailer(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, const LmP0Text *expected_name);
static int lm_registry_source_rows_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_registry_source_table_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame);
static int lm_registry_source_seen_table_add(LmOwnPtrStack *seen, const LmP0Text *table_name);
static int lm_registry_source_check_table_frame_unique(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static const LmP0Field * lm_registry_source_next_present_field(const LmP0Field *field);
static int lm_registry_source_join_header(const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body);
static int lm_registry_source_join_sources_into_target(const LmRegistrySourceLoader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name);
static int lm_registry_source_join_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame);
static int lm_registry_source_check_join_frame_unique(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_table_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_join_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_root(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root);


static int lm_registry_source_text_equals(const LmP0Text *text, const char *value) {
    size_t length;
    if (text == 0 || value == 0) {
        return 0;
    }
    length = strlen(value);
    return text -> length == length && memcmp(text -> data, value, length) == 0;
}

static int lm_registry_source_text_slice_equals(const char *data, size_t length, const char *value) {
    size_t value_length;
    if (data == 0 || value == 0) {
        return 0;
    }
    value_length = strlen(value);
    return length == value_length && memcmp(data, value, length) == 0;
}

static int lm_registry_source_text_slice_same(const char *left_data, size_t left_length, const char *right_data, size_t right_length) {
    if (left_data == 0 || right_data == 0) {
        return 0;
    }
    if (left_length != right_length) {
        return 0;
    }
    if (left_length == 0U) {
        return 1;
    }
    return memcmp(left_data, right_data, left_length) == 0;
}

static int lm_registry_source_node_is_ignored(const LmP0Node *node) {
    return node == 0 || (node -> flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static const LmP0Structure * lm_registry_source_node_structure(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }
    return node -> as -> structure;
}

static const LmP0Frame * lm_registry_source_node_frame(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    return node -> as -> frame;
}

static const LmP0Text * lm_registry_source_node_atom(const LmP0Node *node) {
    if (node == 0 || (node -> kind != LM_P0_NODE_ATOM && node -> kind != LM_P0_NODE_DISABLED)) {
        return 0;
    }
    return node -> as -> atom;
}

static const LmP0Text * lm_registry_source_frame_head(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> head;
}

static const LmP0Structure * lm_registry_source_frame_body(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> body;
}

static const LmP0Text * lm_registry_source_trailer_spelling(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> spelling;
}

static const LmP0Structure * lm_registry_source_trailer_body(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> body;
}

static const char * lm_registry_source_error_prefix(const LmRegistrySourceLoader *loader) {
    if (loader != 0 && loader -> error_prefix != 0) {
        return loader -> error_prefix;
    }
    return "registry source";
}

static void lm_registry_source_error(const LmRegistrySourceLoader *loader, const char *message) {
    fprintf(stderr, "%s registry error: %s\n", lm_registry_source_error_prefix(loader), message);
}

static const LmP0Field * lm_registry_source_nth_field(const LmP0Structure *structure, size_t index) {
    const LmP0Field * field;
    size_t i;
    if (structure == 0) {
        return 0;
    }
    field = structure -> first_field;
    i = 0U;
    while (field != 0 && i < index) {
        field = field -> next;
        i = i + 1U;
    }
    return field;
}

static int lm_registry_source_trailer_single_atom(const LmP0Trailer *trailer, const LmP0Text **out_text) {
    const LmP0Field * field;
    const LmP0Text * atom;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = lm_registry_source_trailer_body(trailer) -> first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    atom = lm_registry_source_node_atom(field -> value);
    if (atom == 0) {
        return 0;
    }
    *(out_text) = atom;
    return 1;
}

static int lm_registry_source_identifier_payload(const LmP0Text *atom, const char **out_data, size_t *out_length) {
    if (atom == 0 || out_data == 0 || out_length == 0 || atom -> data == 0) {
        return 0;
    }
    if (atom -> length >= 2U && atom -> data[0] == '`' && atom -> data[atom -> length - 1U] == '`') {
        *(out_data) = atom -> data + 1U;
        *(out_length) = atom -> length - 2U;
        return 1;
    }
    *(out_data) = atom -> data;
    *(out_length) = atom -> length;
    return 1;
}

static int lm_registry_source_identifier_same(const LmP0Text *left, const LmP0Text *right) {
    const char *left_data;
    const char *right_data;
    size_t left_length;
    size_t right_length;
    if (lm_registry_source_identifier_payload(left, &left_data, &left_length) == 0 || lm_registry_source_identifier_payload(right, &right_data, &right_length) == 0) {
        return 0;
    }
    return lm_registry_source_text_slice_same(left_data, left_length, right_data, right_length);
}

static int lm_registry_source_text_all_char(const LmP0Text *text, char ch) {
    size_t i;
    if (text == 0 || text -> length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < text -> length) {
        if (text -> data[i] != ch) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_registry_source_text_is_array_receiver_head(const LmP0Text *head) {
    size_t i;
    size_t count;
    if (head == 0 || head -> length == 0U) {
        return 0;
    }
    i = 0U;
    count = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0;
        }
        i = i + 1U;
        count = count + 1U;
    }
    return count > 0U;
}

static int lm_registry_source_structure_single_visible_node(const LmP0Structure *structure, const LmP0Node **out_node) {
    const LmP0Field * field;
    const LmP0Field * next_field;
    if (structure == 0 || out_node == 0) {
        return 0;
    }
    field = structure -> first_field;
    while (field != 0 && lm_registry_source_node_is_ignored(field -> value) != 0) {
        field = field -> next;
    }
    if (field == 0 || field -> value == 0) {
        return 0;
    }
    next_field = field -> next;
    while (next_field != 0 && lm_registry_source_node_is_ignored(next_field -> value) != 0) {
        next_field = next_field -> next;
    }
    if (next_field != 0) {
        return 0;
    }
    *(out_node) = field -> value;
    return 1;
}

static int lm_registry_source_single_frame_node(const LmP0Node *node, const LmP0Frame **out_frame) {
    const LmP0Node * inner;
    if (node == 0 || out_frame == 0) {
        return 0;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        *(out_frame) = node -> as -> frame;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        if (lm_registry_source_structure_single_visible_node(node -> as -> structure, &inner) == 0 || inner -> kind != LM_P0_NODE_FRAME) {
            return 0;
        }
        *(out_frame) = inner -> as -> frame;
        return 1;
    }
    return 0;
}

static int lm_registry_source_frame_formal_param_unwrap_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    if (frame == 0 || out_index == 0) {
        return 0;
    }
    if (loader != 0 && loader -> formal_param_unwrap_index != 0 && loader->formal_param_unwrap_index(loader, context, frame, out_index) != 0) {
        return 1;
    }
    if (lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "const")) {
        *(out_index) = 0U;
        return 1;
    }
    return 0;
}

static int lm_registry_source_frame_positional_name_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    if (frame == 0 || out_index == 0) {
        return 0;
    }
    if (loader != 0 && loader -> positional_name_index != 0 && loader->positional_name_index(loader, context, frame, out_index) != 0) {
        return 1;
    }
    if (lm_registry_source_text_all_char(lm_registry_source_frame_head(frame), '@') != 0 || lm_registry_source_text_is_array_receiver_head(lm_registry_source_frame_head(frame)) != 0) {
        *(out_index) = 1U;
        return 1;
    }
    return 0;
}

static int lm_registry_source_formal_param_name(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_name) {
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * field;
    const LmP0Field * name_field;
    size_t name_index;
    size_t unwrap_index;
    if (out_name == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(current -> as -> structure, &current) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_registry_source_single_frame_node(current, &frame) == 0) {
            break;
        }
        if (lm_registry_source_frame_formal_param_unwrap_index(loader, context, frame, &unwrap_index) == 0) {
            break;
        }
        field = lm_registry_source_nth_field(frame -> body, unwrap_index);
        if (field == 0 || field -> value == 0) {
            return 0;
        }
        current = field -> value;
    }
    if (lm_registry_source_single_frame_node(current, &frame) == 0) {
        return 0;
    }
    if (lm_registry_source_frame_positional_name_index(loader, context, frame, &name_index) == 0) {
        name_index = 0U;
    }
    name_field = lm_registry_source_nth_field(frame -> body, name_index);
    if (name_field == 0 || name_field -> value == 0 || name_field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    *(out_name) = name_field -> value -> as -> atom;
    return 1;
}

static int lm_registry_source_column_descriptor_from_param_node(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_descriptor) {
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * type_field;
    size_t name_index;
    size_t unwrap_index;
    if (out_descriptor == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(current -> as -> structure, &current) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_registry_source_single_frame_node(current, &frame) == 0) {
            break;
        }
        if (lm_registry_source_frame_formal_param_unwrap_index(loader, context, frame, &unwrap_index) == 0) {
            break;
        }
        type_field = lm_registry_source_nth_field(frame -> body, unwrap_index);
        if (type_field == 0 || type_field -> value == 0) {
            return 0;
        }
        current = type_field -> value;
    }
    if (lm_registry_source_single_frame_node(current, &frame) == 0) {
        return 0;
    }
    if (lm_registry_source_frame_positional_name_index(loader, context, frame, &name_index) == 0 || name_index == 0U) {
        *(out_descriptor) = lm_registry_source_frame_head(frame);
        return 1;
    }
    type_field = lm_registry_source_nth_field(frame -> body, 0U);
    if (type_field != 0 && type_field -> value != 0) {
        if (type_field -> value -> kind == LM_P0_NODE_ATOM) {
            *(out_descriptor) = type_field -> value -> as -> atom;
            return 1;
        }
        if (type_field -> value -> kind == LM_P0_NODE_FRAME) {
            *(out_descriptor) = type_field -> value -> as -> frame -> head;
            return 1;
        }
        if (type_field -> value -> kind == LM_P0_NODE_STRUCTURE && lm_registry_source_structure_single_visible_frame(type_field -> value -> as -> structure, &frame) != 0) {
            *(out_descriptor) = frame -> head;
            return 1;
        }
    }
    return 0;
}

static size_t lm_registry_source_array_receiver_rank(const LmP0Text *head) {
    size_t i;
    size_t rank;
    if (head == 0) {
        return 0U;
    }
    i = 0U;
    rank = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0U;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0U;
        }
        i = i + 1U;
        rank = rank + 1U;
    }
    return rank;
}

static int lm_registry_source_column_type_shape(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column) {
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * field;
    size_t index;
    size_t rank;
    if (node == 0 || out_column == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(current -> as -> structure, &current) == 0) {
                return 1;
            }
            continue;
        }
        if (current -> kind == LM_P0_NODE_ATOM) {
            out_column->type_name = current -> as -> atom;
            return 1;
        }
        if (current -> kind != LM_P0_NODE_FRAME) {
            return 1;
        }
        frame = current -> as -> frame;
        if (lm_registry_source_text_equals(frame -> head, "const") != 0) {
            out_column->is_const = 1;
            if (lm_registry_source_frame_formal_param_unwrap_index(loader, context, frame, &index) == 0) {
                index = 0U;
            }
            field = lm_registry_source_nth_field(frame -> body, index);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        if (lm_registry_source_text_all_char(frame -> head, '@') != 0) {
            out_column->address_depth = out_column -> address_depth + frame -> head -> length;
            field = lm_registry_source_nth_field(frame -> body, 0U);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        rank = lm_registry_source_array_receiver_rank(frame -> head);
        if (rank != 0U) {
            out_column->array_rank = out_column -> array_rank + rank;
            out_column->address_depth = out_column -> address_depth + 1U;
            field = lm_registry_source_nth_field(frame -> body, 0U);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        out_column->type_name = frame -> head;
        return 1;
    }
    return 1;
}

static int lm_registry_source_column_name_from_param_node(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column) {
    const LmP0Text * atom;
    if (node == 0 || out_column == 0) {
        return 0;
    }
    if (lm_registry_source_formal_param_name(loader, context, node, &atom) == 0) {
        return -1;
    }
    out_column->name = atom;
    if (lm_registry_source_column_descriptor_from_param_node(loader, context, node, &atom) != 0) {
        out_column->descriptors[0] = atom;
        out_column->descriptor_count = 1U;
    }
    if (lm_registry_source_column_type_shape(loader, context, node, out_column) == 0) {
        return -1;
    }
    return 1;
}

static int lm_registry_source_structure_single_visible_frame(const LmP0Structure *structure, const LmP0Frame **out_frame) {
    const LmP0Field * field;
    const LmP0Field * next_field;
    if (structure == 0 || out_frame == 0) {
        return 0;
    }
    field = structure -> first_field;
    while (field != 0 && lm_registry_source_node_is_ignored(field -> value) != 0) {
        field = field -> next;
    }
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    next_field = field -> next;
    while (next_field != 0 && lm_registry_source_node_is_ignored(next_field -> value) != 0) {
        next_field = next_field -> next;
    }
    if (next_field != 0) {
        return 0;
    }
    *(out_frame) = lm_registry_source_node_frame(field -> value);
    return 1;
}

static int lm_registry_source_frame_single_atom(const LmP0Frame *frame, const char *head, const LmP0Text **out_atom) {
    const LmP0Field * field;
    const LmP0Text * frame_head;
    const LmP0Text * atom;
    if (frame == 0 || out_atom == 0) {
        return 0;
    }
    frame_head = lm_registry_source_frame_head(frame);
    if (head != 0 && (frame_head == 0 || lm_registry_source_text_equals(frame_head, head) == 0)) {
        return 0;
    }
    field = lm_registry_source_nth_field(lm_registry_source_frame_body(frame), 0U);
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    atom = lm_registry_source_node_atom(field -> value);
    if (atom == 0) {
        return -1;
    }
    *(out_atom) = atom;
    return 1;
}

static int lm_registry_source_column_name(const LmRegistrySourceLoader *loader, void *context, const LmP0Field *field, LmRegistrySourceColumn *out_column) {
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Structure * node_structure;
    const LmP0Field * body_field;
    const LmP0Text * atom;
    size_t descriptor_count;
    int status;
    if (field == 0 || out_column == 0) {
        return 0;
    }
    memset(out_column, 0, sizeof(out_column[0]));
    node = field -> value;
    if (node == 0) {
        return -1;
    }
    if (node -> kind == LM_P0_NODE_ATOM) {
        atom = lm_registry_source_node_atom(node);
        if (atom == 0) {
            return -1;
        }
        out_column->name = atom;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        return lm_registry_source_column_name_from_param_node(loader, context, node, out_column);
    }
    if (node -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    node_structure = lm_registry_source_node_structure(node);
    if (node_structure == 0) {
        return -1;
    }
    status = lm_registry_source_structure_single_visible_frame(node_structure, &node_frame);
    if (status != 0) {
        return lm_registry_source_column_name_from_param_node(loader, context, node, out_column);
    }
    body_field = node_structure -> first_field;
    while (body_field != 0 && lm_registry_source_node_is_ignored(body_field -> value) != 0) {
        body_field = body_field -> next;
    }
    if (body_field == 0 || body_field -> value == 0 || body_field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    atom = lm_registry_source_node_atom(body_field -> value);
    if (atom == 0) {
        return -1;
    }
    out_column->name = atom;
    descriptor_count = 0U;
    body_field = body_field -> next;
    while (body_field != 0) {
        if (body_field -> value != 0 && lm_registry_source_node_is_ignored(body_field -> value) == 0) {
            if (body_field -> value -> kind != LM_P0_NODE_ATOM) {
                return -1;
            }
            if (descriptor_count >= sizeof(out_column -> descriptors) / sizeof(out_column -> descriptors[0])) {
                return -1;
            }
            atom = lm_registry_source_node_atom(body_field -> value);
            if (atom == 0) {
                return -1;
            }
            out_column->descriptors[descriptor_count] = atom;
            descriptor_count = descriptor_count + 1U;
        }
        body_field = body_field -> next;
    }
    out_column->descriptor_count = descriptor_count;
    return 1;
}

static void lm_registry_source_columns_destroy(LmRegistrySourceColumn **columns, size_t count) {
    size_t index;
    if (columns == 0) {
        return;
    }
    index = 0U;
    while (index < count) {
        lm_own_delete(columns[index], 0);
        index = index + 1U;
    }
    lm_own_delete(columns, 0);
}

static int lm_registry_source_columns_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmRegistrySourceColumn **columns, size_t columns_capacity, size_t *out_count) {
    const LmP0Field * field;
    LmRegistrySourceColumn * column;
    size_t count;
    int status;
    if (frame == 0 || columns == 0 || out_count == 0 || columns_capacity == 0U || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "columns") == 0) {
        return 0;
    }
    *(out_count) = 0U;
    count = 0U;
    field = lm_registry_source_frame_body(frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_registry_source_node_is_ignored(field -> value) == 0) {
            if (count >= columns_capacity) {
                lm_registry_source_error(loader, "table has too many columns");
                return -1;
            }
            column = lm_own_new_zero(sizeof(LmRegistrySourceColumn));
            if (column == 0) {
                lm_registry_source_error(loader, "out of memory while reading table columns");
                return -1;
            }
            status = lm_registry_source_column_name(loader, context, field, column);
            if (status <= 0) {
                lm_own_delete(column, 0);
                lm_registry_source_error(loader, "columns expects atoms or anonymous descriptor structures");
                return -1;
            }
            columns[count] = column;
            count = count + 1U;
            *(out_count) = count;
        }
        field = field -> next;
    }
    *(out_count) = count;
    return 1;
}

static int lm_registry_source_validate_named_trailer(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, const LmP0Text *expected_name) {
    const LmP0Text * actual;
    if (frame == 0 || frame -> trailer == 0) {
        return 0;
    }
    if (lm_registry_source_text_equals(lm_registry_source_trailer_spelling(frame -> trailer), "end") == 0) {
        return 0;
    }
    if (lm_registry_source_trailer_single_atom(frame -> trailer, &actual) == 0) {
        lm_registry_source_error(loader, "end trailer expects exactly one target name");
        return 1;
    }
    if (lm_registry_source_identifier_same(actual, expected_name) == 0 && lm_registry_source_identifier_same(actual, lm_registry_source_frame_head(frame)) == 0) {
        lm_registry_source_error(loader, "end trailer target does not match head/name");
        return 1;
    }
    return 0;
}

static int lm_registry_source_rows_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    const LmP0Field * field;
    const LmP0Node * *row_cells;
    size_t field_index;
    size_t column_index;
    if (frame == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "rows") == 0) {
        return 0;
    }
    if (loader == 0 || loader -> push_table_row == 0) {
        lm_registry_source_error(loader, "table row consumer is not configured");
        return -1;
    }
    if (columns == 0 || column_count == 0U) {
        field = lm_registry_source_frame_body(frame) -> first_field;
        while (field != 0) {
            if (field -> value != 0 && lm_registry_source_node_is_ignored(field -> value) == 0) {
                lm_registry_source_error(loader, "rows require columns before cells");
                return -1;
            }
            field = field -> next;
        }
        return 1;
    }
    row_cells = lm_own_new_zero(column_count * sizeof(row_cells[0]));
    if (row_cells == 0) {
        lm_registry_source_error(loader, "out of memory while reading table rows");
        return -1;
    }
    field_index = 0U;
    field = lm_registry_source_frame_body(frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_registry_source_node_is_ignored(field -> value) == 0) {
            column_index = field_index % column_count;
            row_cells[column_index] = field -> value;
            if (column_index == 0U) {
                if (field -> value -> kind != LM_P0_NODE_ATOM || lm_registry_source_node_atom(field -> value) == 0) {
                    lm_registry_source_error(loader, "table rows currently expect atom cells in the key column");
                    lm_own_delete(row_cells, 0);
                    return -1;
                }
            }
            if (column_index + 1U == column_count && loader->push_table_row(context, table_name, columns, column_count, row_cells) != 0) {
                lm_own_delete(row_cells, 0);
                return -1;
            }
            field_index = field_index + 1U;
        }
        field = field -> next;
    }
    if ((field_index % column_count) != 0U) {
        lm_registry_source_error(loader, "rows field count is not divisible by column count; use explicit None for empty cells");
        lm_own_delete(row_cells, 0);
        return -1;
    }
    lm_own_delete(row_cells, 0);
    return 1;
}

static int lm_registry_source_table_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    LmRegistrySourceColumn * *columns;
    const LmP0Text * table_name;
    size_t column_count;
    int have_name;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "table") == 0) {
        return 0;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_registry_source_error(loader, "out of memory while reading table columns");
        return -1;
    }
    have_name = 0;
    have_columns = 0;
    have_rows = 0;
    column_count = 0U;
    table_name = 0;
    field = lm_registry_source_frame_body(frame) -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_registry_source_error(loader, "table body expects name/columns/rows frames");
                lm_registry_source_columns_destroy(columns, column_count);
                return -1;
            }
            node_frame = lm_registry_source_node_frame(node);
            status = lm_registry_source_frame_single_atom(node_frame, "name", &table_name);
            if (status < 0) {
                lm_registry_source_error(loader, "table name expects exactly one atom");
                lm_registry_source_columns_destroy(columns, column_count);
                return -1;
            }
            if (status > 0) {
                have_name = 1;
                field = field -> next;
                continue;
            }
            if (lm_registry_source_text_equals(lm_registry_source_frame_head(node_frame), "columns") != 0) {
                if (have_name == 0) {
                    lm_registry_source_error(loader, "table columns must appear after name");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                status = lm_registry_source_columns_from_frame(loader, context, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && loader->push_column_metadata(context, table_name, columns, column_count) != 0) {
                    lm_registry_source_error(loader, "cannot store table column metadata");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                have_columns = 1;
                field = field -> next;
                continue;
            }
            if (lm_registry_source_text_equals(lm_registry_source_frame_head(node_frame), "rows") != 0) {
                if (have_name == 0 || have_columns == 0) {
                    lm_registry_source_error(loader, "table rows must appear after name and columns");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                status = lm_registry_source_rows_from_frame(loader, context, node_frame, table_name, columns, column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                have_rows = 1;
                field = field -> next;
                continue;
            }
            lm_registry_source_error(loader, "table body expects name/columns/rows frames");
            lm_registry_source_columns_destroy(columns, column_count);
            return -1;
        }
        field = field -> next;
    }
    if (have_name == 0 || have_columns == 0 || have_rows == 0) {
        lm_registry_source_error(loader, "table requires name, columns and rows");
        lm_registry_source_columns_destroy(columns, column_count);
        return -1;
    }
    if (lm_registry_source_validate_named_trailer(loader, frame, table_name) != 0) {
        lm_registry_source_columns_destroy(columns, column_count);
        return -1;
    }
    lm_registry_source_columns_destroy(columns, column_count);
    return 1;
}

static int lm_registry_source_seen_table_add(LmOwnPtrStack *seen, const LmP0Text *table_name) {
    const char *payload_data;
    char *name;
    size_t i;
    size_t payload_length;
    char *existing;
    if (seen == 0) {
        return -1;
    }
    if (lm_registry_source_identifier_payload(table_name, &payload_data, &payload_length) == 0) {
        return -1;
    }
    i = 0U;
    while (i < seen -> count) {
        existing = lm_own_ptr_stack_at(seen, i);
        if (existing != 0 && lm_registry_source_text_slice_equals(payload_data, payload_length, existing) != 0) {
            return 1;
        }
        i = i + 1U;
    }
    name = lm_own_new_zero(payload_length + 1U);
    if (name == 0) {
        return -1;
    }
    if (payload_length > 0U) {
        memcpy(name, payload_data, payload_length);
    }
    name[payload_length] = '\0';
    if (lm_own_ptr_stack_push(seen, name) != 0) {
        lm_own_delete(name, 0);
        return -1;
    }
    return 0;
}

static int lm_registry_source_check_table_frame_unique(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Text * table_name;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "table") == 0) {
        return 0;
    }
    status = 0;
    table_name = 0;
    field = lm_registry_source_frame_body(frame) -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node == 0 || lm_registry_source_node_is_ignored(node) != 0 || node -> kind != LM_P0_NODE_FRAME) {
            field = field -> next;
            continue;
        }
        node_frame = lm_registry_source_node_frame(node);
        status = lm_registry_source_frame_single_atom(node_frame, "name", &table_name);
        if (status < 0) {
            lm_registry_source_error(loader, "table name expects exactly one atom");
            return -1;
        }
        if (status > 0) {
            break;
        }
        field = field -> next;
    }
    if (status == 0) {
        return 0;
    }
    status = lm_registry_source_seen_table_add(seen, table_name);
    if (status < 0) {
        lm_registry_source_error(loader, "cannot record table name");
        return -1;
    }
    if (status > 0) {
        lm_registry_source_error(loader, "duplicate table in one registry source module");
        return -1;
    }
    return 0;
}

static const LmP0Field * lm_registry_source_next_present_field(const LmP0Field *field) {
    while (field != 0 && (field -> value == 0 || lm_registry_source_node_is_ignored(field -> value) != 0)) {
        field = field -> next;
    }
    return field;
}

static int lm_registry_source_join_header(const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body) {
    const LmP0Field * field;
    const LmP0Structure * frame_body;
    const LmP0Structure * sources;
    const LmP0Text * target;
    const LmP0Structure * body;
    if (frame == 0 || out_sources == 0 || out_target == 0 || out_body == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "join") == 0) {
        return 0;
    }
    *(out_sources) = 0;
    *(out_target) = 0;
    *(out_body) = 0;
    frame_body = lm_registry_source_frame_body(frame);
    if (frame_body == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(frame_body -> first_field);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    sources = lm_registry_source_node_structure(field -> value);
    if (sources == 0 || lm_registry_source_next_present_field(sources -> first_field) == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    target = lm_registry_source_node_atom(field -> value);
    if (target == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    body = lm_registry_source_node_structure(field -> value);
    if (body == 0) {
        return -1;
    }
    *(out_sources) = sources;
    *(out_target) = target;
    *(out_body) = body;
    return 1;
}

static int lm_registry_source_join_sources_into_target(const LmRegistrySourceLoader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name) {
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Text * source_name;
    if (loader == 0 || loader -> join_table == 0) {
        lm_registry_source_error(loader, "join consumer is not configured");
        return -1;
    }
    if (sources == 0 || target_name == 0) {
        return -1;
    }
    field = sources -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_ATOM) {
                lm_registry_source_error(loader, "join source list currently expects table name atoms");
                return -1;
            }
            source_name = lm_registry_source_node_atom(node);
            if (source_name == 0 || loader->join_table(context, source_name, target_name) != 0) {
                return -1;
            }
        }
        field = field -> next;
    }
    return 0;
}

static int lm_registry_source_join_from_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Structure * sources;
    const LmP0Structure * body;
    LmRegistrySourceColumn * *columns;
    const LmP0Text * target_name;
    const LmP0Frame * rows_frame;
    size_t column_count;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    field = 0;
    status = lm_registry_source_join_header(frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status == 0) {
            return 0;
        }
        lm_registry_source_error(loader, "join expects (sourceTables...) targetName and a table fragment body");
        return -1;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_registry_source_error(loader, "out of memory while reading join columns");
        return -1;
    }
    have_columns = 0;
    have_rows = 0;
    rows_frame = 0;
    column_count = 0U;
    field = lm_registry_source_next_present_field(body -> first_field);
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_registry_source_error(loader, "join body expects columns/rows frames");
                lm_registry_source_columns_destroy(columns, column_count);
                return -1;
            }
            node_frame = lm_registry_source_node_frame(node);
            if (lm_registry_source_text_equals(lm_registry_source_frame_head(node_frame), "columns") != 0) {
                if (have_columns != 0) {
                    lm_registry_source_error(loader, "join expects one columns frame");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                status = lm_registry_source_columns_from_frame(loader, context, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && loader->push_column_metadata(context, target_name, columns, column_count) != 0) {
                    lm_registry_source_error(loader, "cannot store join target column metadata");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                have_columns = 1;
                field = lm_registry_source_next_present_field(field -> next);
                continue;
            }
            if (lm_registry_source_text_equals(lm_registry_source_frame_head(node_frame), "rows") != 0) {
                if (have_rows != 0) {
                    lm_registry_source_error(loader, "join expects at most one rows frame");
                    lm_registry_source_columns_destroy(columns, column_count);
                    return -1;
                }
                rows_frame = node_frame;
                have_rows = 1;
                field = lm_registry_source_next_present_field(field -> next);
                continue;
            }
            lm_registry_source_error(loader, "join body expects columns/rows frames");
            lm_registry_source_columns_destroy(columns, column_count);
            return -1;
        }
        field = lm_registry_source_next_present_field(field -> next);
    }
    if (have_columns == 0) {
        lm_registry_source_error(loader, "join requires target columns");
        lm_registry_source_columns_destroy(columns, column_count);
        return -1;
    }
    if (lm_registry_source_join_sources_into_target(loader, context, sources, target_name) != 0) {
        lm_registry_source_columns_destroy(columns, column_count);
        return -1;
    }
    if (have_rows != 0) {
        status = lm_registry_source_rows_from_frame(loader, context, rows_frame, target_name, columns, column_count);
        if (status <= 0) {
            lm_registry_source_columns_destroy(columns, column_count);
            return -1;
        }
    }
    if (lm_registry_source_validate_named_trailer(loader, frame, target_name) != 0) {
        lm_registry_source_columns_destroy(columns, column_count);
        return -1;
    }
    lm_registry_source_columns_destroy(columns, column_count);
    return 1;
}

static int lm_registry_source_check_join_frame_unique(const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    const LmP0Structure * sources;
    const LmP0Structure * body;
    const LmP0Text * target_name;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    status = lm_registry_source_join_header(frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status < 0) {
            lm_registry_source_error(loader, "join expects (sourceTables...) targetName and a table fragment body");
            return -1;
        }
        return 0;
    }
    status = lm_registry_source_seen_table_add(seen, target_name);
    if (status < 0) {
        lm_registry_source_error(loader, "cannot record join target name");
        return -1;
    }
    if (status > 0) {
        lm_registry_source_error(loader, "duplicate table in one registry source module");
        return -1;
    }
    return 0;
}

static int lm_registry_source_load_table_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    int status;
    if (lm_registry_source_check_table_frame_unique(loader, frame, seen) != 0) {
        return 1;
    }
    status = lm_registry_source_table_from_frame(loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_registry_source_error(loader, "table receiver expects table frame");
        }
        return 1;
    }
    return 0;
}

static int lm_registry_source_load_join_frame(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    int status;
    if (lm_registry_source_check_join_frame_unique(loader, frame, seen) != 0) {
        return 1;
    }
    status = lm_registry_source_join_from_frame(loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_registry_source_error(loader, "join receiver expects join frame");
        }
        return 1;
    }
    return 0;
}

static int lm_registry_source_load_root(const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root) {
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * frame;
    LmOwnPtrStack * seen;
    int loaded;
    int status;
    if (root == 0 || root -> kind != LM_P0_NODE_STRUCTURE) {
        lm_registry_source_error(loader, "root must be a Structure");
        return 1;
    }
    seen = lm_own_new_zero(sizeof(seen[0]));
    if (seen == 0) {
        lm_registry_source_error(loader, "cannot allocate table duplicate tracker");
        return 1;
    }
    lm_own_ptr_stack_init(seen, lm_own_delete_plain);
    loaded = 0;
    status = 0;
    field = lm_registry_source_node_structure(root) -> first_field;
    while (field != 0 && status == 0) {
        node = field -> value;
        if (lm_registry_source_node_is_ignored(node) != 0) {
            field = field -> next;
            continue;
        }
        if (node -> kind != LM_P0_NODE_FRAME) {
            lm_registry_source_error(loader, "root fields must be table or join frames");
            status = 1;
            field = field -> next;
            continue;
        }
        frame = lm_registry_source_node_frame(node);
        if (lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "table") != 0) {
            status = lm_registry_source_load_table_frame(loader, context, frame, seen);
        }
        else {
            if (lm_registry_source_text_equals(lm_registry_source_frame_head(frame), "join") != 0) {
                status = lm_registry_source_load_join_frame(loader, context, frame, seen);
            }
            else {
                lm_registry_source_error(loader, "root fields must be table or join frames");
                status = 1;
            }
        }
        if (status == 0) {
            loaded = 1;
        }
        field = field -> next;
    }
    if (status == 0 && loaded == 0) {
        lm_registry_source_error(loader, "no rows loaded");
        status = 1;
    }
    lm_own_ptr_stack_destroy(seen);
    lm_own_delete(seen, 0);
    return status;
}








































































































#include <string.h>
static char * lm_table_descriptor_copy_slice(const char *data, size_t length);
static char * lm_table_descriptor_copy_cstr(const char *value);
static int lm_table_descriptor_slice_equals(const char *left, size_t left_length, const char *right, size_t right_length);
static int lm_table_descriptor_cstr_equals_slice(const char *left, const char *right, size_t right_length);
static LmOwnPtrStack * lm_table_descriptor_stack_new(LmOwnDelete delete_item);
static void lm_table_column_descriptor_delete_any(void *object);
static void lm_table_cell_delete_any(void *object);
static void lm_table_row_delete_any(void *object);
static void lm_registry_view_row_delete_any(void *object);
static void lm_table_descriptor_delete_any(void *object);
static LmTableColumnDescriptor * lm_table_column_descriptor_new_slice(const char *name, size_t name_length, const char *type_name, size_t type_name_length, size_t address_depth, size_t array_rank, int is_const);
static int lm_table_column_descriptor_add_descriptor_slice(LmTableColumnDescriptor *column, const char *descriptor, size_t descriptor_length);
static size_t lm_table_column_descriptor_descriptor_count(const LmTableColumnDescriptor *column);
static const char * lm_table_column_descriptor_descriptor_at(const LmTableColumnDescriptor *column, size_t index);
static int lm_table_column_descriptor_has_descriptor(const LmTableColumnDescriptor *column, const char *descriptor);
static int lm_table_descriptor_take_column(LmTableDescriptor *table, LmTableColumnDescriptor *column);
static int lm_table_descriptor_add_column_slices(LmTableDescriptor *table, const char *name, size_t name_length, const char *type_name, size_t type_name_length, const char **descriptor_data, const size_t *descriptor_lengths, size_t descriptor_count, size_t address_depth, size_t array_rank, int is_const);
static int lm_table_descriptor_add_column(LmTableDescriptor *table, const char *name, const char *descriptor);
static LmTableDescriptor * lm_table_descriptor_new_empty_slice(const char *name, size_t name_length);
static LmTableDescriptor * lm_table_descriptor_new_slice(const char *name, size_t name_length);
static size_t lm_table_descriptor_column_count(const LmTableDescriptor *table);
static const LmTableColumnDescriptor * lm_table_descriptor_column_at(const LmTableDescriptor *table, size_t index);
static size_t lm_table_descriptor_row_count(const LmTableDescriptor *table);
static const LmRegistryViewRow * lm_table_descriptor_row_at(const LmTableDescriptor *table, size_t index);
static LmTableCell * lm_table_cell_new_slice(const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none);
static LmTableCell * lm_table_cell_new_cstr(const char *atom, const char *value, const void *node, const void *source, int explicit_none);
static LmTableRow * lm_table_row_new(const LmTableRow *source);
static int lm_table_row_take_cell(LmTableRow *row, LmTableCell *cell);
static int lm_table_row_take_cell_slice(LmTableRow *row, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none);
static size_t lm_table_row_cell_count(const LmTableRow *row);
static const LmTableCell * lm_table_row_cell_at(const LmTableRow *row, size_t index);
static int lm_table_descriptor_take_materialized_row(LmTableDescriptor *table, LmTableRow *row);
static size_t lm_table_descriptor_materialized_row_count(const LmTableDescriptor *table);
static const LmTableRow * lm_table_descriptor_materialized_row_at(const LmTableDescriptor *table, size_t index);
static int lm_table_descriptor_append_materialized_row_copy(LmTableDescriptor *table, const LmTableRow *source_row);
static int lm_table_descriptor_schema_same(const LmTableDescriptor *left, const LmTableDescriptor *right);
static int lm_table_descriptor_join_schema_compatible(const LmTableDescriptor *source, const LmTableDescriptor *target);
static LmRegistryView * lm_registry_view_new(const LmRegistryView *parent);
static void lm_registry_view_note_mutation(LmRegistryView *view);
static void lm_registry_view_delete(LmRegistryView *view);
static int lm_registry_view_class_has_slice(const LmRegistryView *view, const char *name, size_t name_length);
static int lm_registry_view_class_has(const LmRegistryView *view, const char *name);
static int lm_registry_view_class_add(LmRegistryView *view, const char *name);
static size_t lm_registry_view_class_count(const LmRegistryView *view);
static const char * lm_registry_view_class_at(const LmRegistryView *view, size_t index);
static LmTableDescriptor * lm_registry_view_find_local_table_slice(const LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_ensure_local_table_slice(LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_find_local_source_table_slice(const LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_find_source_table_slice(const LmRegistryView *view, const char *table, size_t table_length);
static int lm_registry_view_take_local_source_table(LmRegistryView *view, LmTableDescriptor *descriptor);
static size_t lm_registry_view_source_table_count(const LmRegistryView *view);
static const LmTableDescriptor * lm_registry_view_source_table_at(const LmRegistryView *view, size_t index);
static int lm_table_descriptor_source_path_column_matches(const LmTableDescriptor *descriptor, size_t column_index, const char *path, size_t path_length);
static int lm_registry_view_source_path_has_rows_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, int projected_only, int *out_covered);
static int lm_registry_view_source_path_has_rows_slice(const LmRegistryView *view, const char *path, size_t path_length, int *out_covered);
static int lm_registry_view_source_path_has_rows(const LmRegistryView *view, const char *path, int *out_covered);
static int lm_registry_view_source_path_has_key_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int projected_only, int *out_covered);
static int lm_registry_view_source_path_has_key_slice(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int *out_covered);
static const LmTableCell * lm_registry_view_source_path_cell_at_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered);
static const LmTableCell * lm_registry_view_source_path_cell_at_slice(const LmRegistryView *view, const char *path, size_t path_length, size_t index, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered);
static const char * lm_registry_view_source_path_key_at_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, int *out_covered);
static const char * lm_registry_view_source_path_key_at_slice(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int *out_covered);
static int lm_registry_view_append_materialized_rows_mode(LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length, int note_mutation, int require_compatible_source, size_t snapshot_source_count, const size_t *snapshot_row_counts);
static int lm_registry_view_append_materialized_rows(LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length);
static size_t lm_registry_view_fact_count(const LmRegistryView *view);
static const LmRegistryViewRow * lm_registry_view_fact_at(const LmRegistryView *view, size_t index);
static int lm_registry_view_push_relation(LmRegistryView *view, const char *table, const char *key, const char *payload, const void *payload_node, const void *source);
static const LmRegistryViewRow * lm_registry_view_lookup_local_exact_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_exact_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_exact(const LmRegistryView *view, const char *key, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_default_slice(const LmRegistryView *view, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_default(const LmRegistryView *view, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup(const LmRegistryView *view, const char *key, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_text_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_text(const LmRegistryView *view, const char *key, const char *table);
static size_t lm_registry_view_local_table_row_count_slice(const LmRegistryView *view, const char *table, size_t table_length);
static size_t lm_registry_view_table_row_count_slice(const LmRegistryView *view, const char *table, size_t table_length);
static size_t lm_registry_view_table_row_count(const LmRegistryView *view, const char *table);
static const LmRegistryViewRow * lm_registry_view_table_row_at_slice(const LmRegistryView *view, const char *table, size_t table_length, size_t index);
static const LmRegistryViewRow * lm_registry_view_table_row_at(const LmRegistryView *view, const char *table, size_t index);
static size_t lm_table_descriptor_matching_key_count_slice(const LmTableDescriptor *descriptor, const char *key, size_t key_length);
static const LmRegistryViewRow * lm_table_descriptor_matching_key_at_slice(const LmTableDescriptor *descriptor, const char *key, size_t key_length, size_t match_index);
static size_t lm_registry_view_matching_key_count_slice(const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length);
static size_t lm_registry_view_matching_key_count(const LmRegistryView *view, const char *table, const char *key);
static const LmRegistryViewRow * lm_registry_view_matching_key_at_slice(const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length, size_t match_index);
static const LmRegistryViewRow * lm_registry_view_matching_key_at(const LmRegistryView *view, const char *table, const char *key, size_t match_index);
static const LmOwnPtrStack * lm_registry_view_local_source_rows_slice(const LmRegistryView *view, const char *table, size_t table_length);
static int lm_registry_view_table_has_rows(const LmRegistryView *view, const char *table);


static int lm_p0_text_equals(const LmP0Text *text, const char *value);
static int lm_p0_identifier_payload(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_identifier_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_literal_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_payload_is_null(const LmP0Text *atom);
static int lm_p0_is_horizontal_space(char value);
static int lm_p0_is_line_break(char value);
static size_t lm_p0_line_break_width_at(const char *source, size_t length, size_t index);
static int lm_p0_is_field_space(char value);
static int lm_p0_is_field_separator(char value);
static int lm_p0_is_short_form_separator(char value);
static int lm_p0_is_quoted_token_boundary(char value);
static int lm_p0_starts_python_string(const char *text, size_t length, size_t index);
static int lm_p0_is_decimal_digit(char value);
static char * lm_p0_copy_bytes(const char *source, size_t length);
static LmP0Text * lm_p0_text_view_new_cstr(const char *text);
static void lm_p0_text_view_delete(LmP0Text *text);
static LmP0Text * lm_p0_text_from_cstr(const char *text);


#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef LM_P0_ENABLE_REGISTRY_COMPARE
#define LM_P0_ENABLE_REGISTRY_COMPARE 1
#endif

static int lm_p0_registry_selftest_expect_path(const char *path, size_t index, const char *expected_key, const char *expected_value);
static int lm_p0_registry_selftest_expect_lookup(const char *path, const char *key, const char *expected_value);
static int lm_p0_registry_source_tables_selftest(void);

#ifdef LM_P0_REGISTRY_SELFTEST
static int lm_p0_registry_source_tables_selftest(void);
int main(void) {
    return lm_p0_registry_source_tables_selftest();
}
#endif
static int lm_p0_document_init_owners(LmP0Document *document);
static void lm_p0_document_destroy_owners(LmP0Document *document);
static void lm_p0_document_freeze_tree(LmP0Document *document);
static void lm_p0_indent_stack_free(LmP0IndentStack *stack);
static void lm_p0_indent_stack_free_any(void *object);
static int lm_p0_indent_stack_push(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column);
static int lm_p0_indent_stack_init(LmP0Document *document, LmP0IndentStack *stack);
static LmP0IndentStack * lm_p0_indent_stack_new_empty(void);
static LmP0IndentStack * lm_p0_indent_stack_new(LmP0Document *document);
static void lm_p0_indent_stack_delete(LmP0IndentStack *stack);
static int lm_p0_indent_stack_copy(LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column);
static LmP0IndentStack * lm_p0_indent_stack_clone(LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column);
static size_t lm_p0_indent_tab_column(size_t column);
static void lm_p0_scan_indent_column(const char *source, size_t start, size_t end, size_t *out_offset, size_t *out_column);
static size_t lm_p0_visual_column_between(const char *source, size_t start, size_t end);
static const LmTableCell * lm_p0_registry_source_path_cell_at(const char *path, size_t index, const LmTableCell **out_key_cell);
static const LmTableCell * lm_p0_registry_source_path_lookup_cell_slice(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, const LmTableCell **out_key_cell);
static const char * lm_p0_registry_lookup(const LmP0Text *key, const char *table);
static int lm_p0_registry_source_path_has_rows(const char *table);
static int lm_p0_registry_table_has_rows(const char *table);
static int lm_p0_registry_table_has_rows_loaded_or_loading(const char *table);
static size_t lm_p0_count_line_breaks(const char *source, size_t start, size_t end);
static void lm_p0_position_in_slice(const char *text, size_t length, size_t index, size_t base_line, size_t base_column, size_t *out_line, size_t *out_column);
static void lm_p0_advance_layout_line(const char *source, size_t length, size_t line_start, size_t line_end, size_t *offset, size_t *line);
static int lm_p0_index_is_line_start(const char *text, size_t index);
static int lm_p0_line_rest_is_horizontal_space(const char *source, size_t start, size_t end);
static size_t lm_p0_find_physical_line_end(const char *source, size_t length, size_t start);
static int lm_p0_text_has_prefix_name(const char *text, size_t length, const char *name, int allow_bare);
static LmP0TrailerRole lm_p0_legacy_trailer_role(const char *text, size_t length);
static LmP0TrailerRole lm_p0_trailer_role_from_payload(const char *payload);
static const char * lm_p0_trailer_role_payload(LmP0TrailerRole role);
static int lm_p0_registry_trailer_allows_bare(const char *class_name);
static int lm_p0_trailer_role_is_tail_cutter(LmP0TrailerRole role);
static int lm_p0_node_head_is(const LmP0Node *node, const char *name);
static int lm_p0_trailer_role_accepts_target(LmP0TrailerRole role, const LmP0Node *target);
const LmP0Structure * lm_p0_node_structure(const LmP0Node *node);
const LmP0Frame * lm_p0_node_frame(const LmP0Node *node);
const LmP0Text * lm_p0_node_atom(const LmP0Node *node);
const LmP0Trailer * lm_p0_structure_trailer(const LmP0Structure *structure);
const LmP0Text * lm_p0_frame_head(const LmP0Frame *frame);
const LmP0Structure * lm_p0_frame_body(const LmP0Frame *frame);
const LmP0Trailer * lm_p0_frame_trailer(const LmP0Frame *frame);
const LmP0Text * lm_p0_trailer_spelling(const LmP0Trailer *trailer);
const LmP0Structure * lm_p0_trailer_body(const LmP0Trailer *trailer);
static int lm_p0_stream_event_is_tail_cutter(const LmP0StreamEvent *event);
static int lm_p0_find_python_string_end(const char *text, size_t length, size_t start, size_t *out_end);
static size_t lm_p0_skip_python_string_unchecked(const char *text, size_t length, size_t start);
static void lm_p0_scan_layout_prefix(const char *source, size_t length, size_t start, size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level);
static int lm_p0_layout_prefix_is_deeper(size_t indent_column, size_t dot_level, size_t base_indent_column, size_t base_dot_level);
static LmP0DashFenceStatus lm_p0_dash_fence_status(const char *text, size_t length, size_t *out_dash_count);
static int lm_p0_validate_dash_fence_line(LmP0Document *document, const char *text, size_t length, size_t line, size_t column);
static int lm_p0_match_block_string_fence_line(const char *source, size_t line_start, size_t line_end, size_t eq_count);
static int lm_p0_match_raw_comment_fence_line(const char *source, size_t line_start, size_t line_end, size_t star_count);
static void lm_p0_dump_append_size(LmP0Dump *dump, size_t value);
static void lm_p0_dump_append_field_count_line(LmP0Dump *dump, size_t field_count);
static void lm_p0_registry_private_api_anchor(void);
static int lm_p0_registry_init(void);
static void lm_p0_registry_destroy(void);
static LmP0Text * lm_p0_text_ref_new_empty(void);
static void lm_p0_text_ref_delete(LmP0Text *text);
static int lm_p0_document_register_lazy_text(LmP0Document *document, const char *source, size_t length, const char **patch_slot, size_t line, size_t column);
static LmP0Text * lm_p0_new_text(LmP0Document *document, const char *source, size_t length, size_t line, size_t column);
static LmP0Structure * lm_p0_new_structure(LmP0Document *document, size_t line, size_t column);
static LmP0Frame * lm_p0_new_frame(LmP0Document *document, size_t line, size_t column);
static LmP0Trailer * lm_p0_new_trailer(LmP0Document *document, const char *spelling, size_t spelling_length, size_t line, size_t column);
static void lm_p0_set_diagnostic(LmP0Document *document, int code, size_t line, size_t column, const char *message);
static int lm_p0_registry_column_has_descriptor(const LmRegistrySourceColumn *column, const char *descriptor);
static int lm_p0_registry_cell_none_cell_matches(const LmP0Text *payload, const LmP0Text *class_atom);
static int lm_p0_registry_cell_is_null(const LmP0Text *atom, const LmRegistrySourceColumn *column);
static int lm_p0_registry_cell_value(const LmP0Text *atom, const LmRegistrySourceColumn *column, LmP0Text *out_value);
static LmTableDescriptor * lm_p0_registry_source_descriptor_new(const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_p0_registry_materialize_source_row(const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
static int lm_p0_registry_compare_enabled(void);
static void lm_p0_registry_compare_fail(const char *table, const char *key, const char *registry_payload, const char *legacy_payload);
static void lm_p0_trim_right(const char **text, size_t *length);
static void lm_p0_trim_trailing_line_comment(const char **text, size_t *length);
static int lm_p0_indent_level_from_column(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level);
static size_t lm_p0_skip_fence_block_unchecked(const char *text, size_t length, size_t start, char fence_char);
static size_t lm_p0_find_layout_line_end(const char *source, size_t length, size_t start);
static size_t lm_p0_scan_brace_mark_unchecked(const char *text, size_t length, size_t start, int *closed);
static int lm_p0_skip_brace_mark_ex(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, int record);
static int lm_p0_skip_brace_mark(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t column);
static int lm_p0_consume_brace_mark(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column);
static int lm_p0_skip_leading_brace_marks_ex(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index, int record);
static int lm_p0_skip_leading_brace_marks(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index);
static int lm_p0_line_is_standalone_mix_run(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, int *out_is_mix);
static int lm_p0_scan_leading_mix_prefix(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *out_prefix_start, size_t *out_prefix_end, size_t *out_anchor);
static LmP0DashFenceStatus lm_p0_dash_fence_status_after_comment_trim(const char *text, size_t length, size_t *out_dash_count);
static int lm_p0_scan_raw_comment_block(LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, size_t *next_offset, size_t *next_line);
static int lm_p0_scan_block_string_event(LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, LmP0StreamEvent *event, size_t *next_offset, size_t *next_line);
static LmP0Node * lm_p0_new_node(LmP0Document *document, LmP0NodeKind kind);
static int lm_p0_append_field(LmP0Document *document, LmP0Structure *structure, LmP0Node *node);
static int lm_p0_pointer_source_offset(const LmP0Document *document, const char *pointer, size_t *out_offset);
static size_t lm_p0_offset_from_line_column(const char *text, size_t length, size_t line, size_t column);
static void lm_p0_copy_payload_diagnostic(LmP0Document *document, const LmP0Document *payload_document, size_t payload_offset);
static void lm_p0_adjust_node_span_to_document(LmP0Document *document, LmP0Node *node, size_t base_offset);
static void lm_p0_adjust_structure_spans_to_document(LmP0Document *document, LmP0Structure *structure, size_t base_offset);
static int lm_p0_record_mix_mark(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t start, size_t end);
static int lm_p0_is_bare_identifier_start(char c);
static int lm_p0_is_bare_identifier_rest(char c);
static int lm_p0_is_hex_digit(char c);
static int lm_p0_scan_number_token(const char *text, size_t end_index, size_t start, size_t *out_end);
static size_t lm_p0_scan_c_quoted_token(const char *text, size_t end_index, size_t quote_index);
static int lm_p0_starts_c_prefixed_quote(const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_char_token(const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_prefixed_quote_token(const char *text, size_t end_index, size_t start);
static int lm_p0_starts_c_surface_atom(const char *text, size_t end_index, size_t start);
static int lm_p0_is_c_surface_top_boundary(char value);
static size_t lm_p0_scan_c_sizeof_surface_atom(const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_surface_atom(const char *text, size_t end_index, size_t start);
static int lm_p0_scan_c_char(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static int lm_p0_scan_c_prefixed_quote(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static size_t lm_p0_scan_builtin_compact_atom_piece(const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_registry_compact_atom_piece(const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_compact_atom_piece(const char *text, size_t end_index, size_t start, int allow_signed_number);
static int lm_p0_append_atom_slice(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index);
static int lm_p0_append_positional_skip(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t index);
static int lm_p0_append_compact_atom_pieces(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index);
static void lm_p0_free_node(LmP0Node *node);
static int lm_p0_relaxed_level_from_column(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t base_level, size_t line, size_t source_column, size_t *out_level);
static int lm_p0_source_level_after_line_break(LmP0Document *document, LmP0IndentStack *indent_stack, const char *text, size_t length, size_t index, size_t line, size_t column, size_t base_level, size_t *content_index, size_t *out_level);
static int lm_p0_skip_field_space(LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t *current_source_level, int *stopped_by_source_level);
static int lm_p0_scan_python_string(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static int lm_p0_scan_quoted(LmP0Document *document, const char *text, size_t length, size_t *index, char quote, size_t line, size_t base_column);
static int lm_p0_require_quoted_token_boundary(LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column);
static int lm_p0_find_matching_paren(LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index);
static int lm_p0_find_matching_bracket(LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index);
static int lm_p0_find_colon(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *colon_index);
static int lm_p0_field_start_looks_explicit_frame(LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column);
static void lm_p0_field_parse_loop_frame_delete_any(void *object);
static LmOwnPtrStack * lm_p0_field_parse_loop_stack_new(LmP0Document *document, size_t line, size_t column);
static void lm_p0_field_parse_loop_stack_delete(LmOwnPtrStack *stack);
static int lm_p0_field_parse_loop_push(LmP0Document *document, LmOwnPtrStack *stack, LmP0IndentStack *indent_stack, int indent_stack_owned, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t i, size_t current_source_level, int allow_empty_fields, int expect_field, int headless_group_after_separator, LmP0FieldParseLoopContinuation continuation, LmP0Node *node, size_t start, size_t close_index);
static int lm_p0_field_parse_fail(LmOwnPtrStack **parse_stack, LmP0IndentStack **indent_stack, int *indent_stack_owned);
static int lm_p0_parse_append_node_and_update(LmP0Document *document, LmP0Structure *structure, LmP0Node *node, unsigned flags, int allow_empty_fields, int *expect_field, int *headless_group_after_separator);
static int lm_p0_parse_fields_until_with_layout(LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t layout_base_level, size_t *index);
static int lm_p0_parse_fields_until(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t *index);
static int lm_p0_parse_fields_into(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset);
static int lm_p0_stack_ensure(LmP0Document *document, LmP0Stack *stack, size_t level);
static void lm_p0_stack_truncate_deeper(LmP0Stack *stack, size_t level);
static size_t lm_p0_stack_top_level(const LmP0Stack *stack);
static int lm_p0_stack_level_is_trailer_body(const LmP0Stack *stack, size_t level);
static size_t lm_p0_stack_collapse_soft_to_event(LmP0Stack *stack, size_t event_level);
static LmP0TrailerRole lm_p0_registry_trailer_role(const char *text, size_t length);
static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length);
static void lm_p0_stack_free(LmP0Stack *stack);
static void lm_p0_stack_free_any(void *object);
static LmP0Stack * lm_p0_stack_new(void);
static void lm_p0_stack_delete(LmP0Stack *stack);
static LmP0PendingDelimiter * lm_p0_pending_delimiter_new(void);
static LmP0StreamEvent * lm_p0_stream_event_new(void);
static LmP0StreamEvent * lm_p0_stream_event_new_copy(const LmP0StreamEvent *event);
static void lm_p0_stream_event_delete(LmP0StreamEvent *event);
static void lm_p0_pending_delimiter_clear(LmP0PendingDelimiter *pending);
static int lm_p0_pending_delimiter_set(LmP0Document *document, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event);
static void lm_p0_pending_delimiter_delete(LmP0PendingDelimiter *pending);
static LmP0PendingMix * lm_p0_pending_mix_new(void);
static void lm_p0_pending_mix_free(LmP0PendingMix *pending);
static void lm_p0_pending_mix_free_any(void *object);
static void lm_p0_pending_mix_delete(LmP0PendingMix *pending);
static int lm_p0_pending_mix_push(LmP0Document *document, LmP0PendingMix *pending, const LmP0StreamEvent *event);
static LmP0DisabledState * lm_p0_disabled_state_new(size_t base_level);
static void lm_p0_disabled_state_delete(LmP0DisabledState *state);
static int lm_p0_node_keeps_source_child_level(LmP0Node *node);
static LmP0Structure * lm_p0_node_child_structure(LmP0Node *node);
static LmP0Node * lm_p0_structure_last_colon_frame(LmP0Structure *structure);
static int lm_p0_stack_install_node_lineage(LmP0Document *document, LmP0Stack *stack, size_t base_level, LmP0Node *node);
static int lm_p0_stack_ensure_root_level_alias(LmP0Document *document, LmP0Stack *stack, size_t level);
static int lm_p0_stack_open_implicit_anonymous(LmP0Document *document, LmP0Stack *stack, size_t parent_level, size_t line, size_t column, size_t offset);
static LmP0Trailer * * lm_p0_node_trailer_slot(LmP0Node *node);
static LmP0Trailer * lm_p0_attach_trailer(LmP0Document *document, LmP0Node *node, const char *spelling, size_t spelling_length, unsigned flags, size_t line, size_t column);
static int lm_p0_parse_trailer_item(LmP0Document *document, LmP0Node *target, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, LmP0Structure **out_body);
static int lm_p0_stream_resolve_pending_delimiter(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, size_t next_level);
static int lm_p0_stream_apply_item_event(LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event);
static int lm_p0_stream_apply_mix_event(LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event);
static int lm_p0_stream_apply_event(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event);
static int lm_p0_pending_mix_flush(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending_delimiter, LmP0PendingMix *pending_mix, size_t level);
static size_t lm_p0_stream_block_string_level(const LmP0Stack *stack, const LmP0PendingDelimiter *pending);
static int lm_p0_validate_disabled_item_text(LmP0Document *document, const char *text, size_t length, size_t line, size_t column);
static int lm_p0_disabled_scan_next_event(LmP0Document *document, LmP0IndentStack *indent_stack, size_t *offset, size_t *line, LmP0StreamEvent *event, int *has_event);
static int lm_p0_disabled_event_is_tail_cutter(const LmP0StreamEvent *event);
static int lm_p0_disabled_state_accept_event(LmP0Document *document, LmP0DisabledState *state, LmP0StreamEvent *event, int *done_after_event, int *done_before_event);
static int lm_p0_validate_disabled_block(LmP0Document *document, const LmP0IndentStack *indent_stack, size_t first_next_offset, size_t first_next_line, size_t base_level, const char *header_text, size_t header_length, size_t header_line, size_t header_column, size_t *out_offset, size_t *out_line);
static int lm_p0_parse_stream(LmP0Document *document);
static void lm_p0_structure_recount(LmP0Structure *structure);
static LmP0PostprocessFrame * lm_p0_postprocess_frame_new(int phase);
static int lm_p0_postprocess_push_frame(LmOwnPtrStack *stack, LmP0PostprocessFrame *frame);
static int lm_p0_postprocess_push_node(LmOwnPtrStack *stack, LmP0Node *node);
static int lm_p0_postprocess_push_structure(LmOwnPtrStack *stack, LmP0Structure *structure);
static int lm_p0_postprocess_push_trailer(LmOwnPtrStack *stack, LmP0Trailer *trailer);
static int lm_p0_postprocess_push_frame_wrap(LmOwnPtrStack *stack, LmP0Node *node);
static LmOwnPtrStack * lm_p0_postprocess_stack_new(void);
static void lm_p0_postprocess_stack_delete(LmOwnPtrStack **stack);
static int lm_p0_postprocess_run(LmP0Document *document, LmOwnPtrStack *stack);
static int lm_p0_postprocess_node(LmP0Document *document, LmP0Node *node);
static int lm_p0_wrap_fields_from_line(LmP0Document *document, LmP0Structure *structure, size_t head_line, size_t inline_event_end_offset);
static int lm_p0_validate_nonempty_colon_frames_in_trailer(LmP0Document *document, const LmP0Trailer *trailer);
static int lm_p0_validate_nonempty_colon_frames_in_node(LmP0Document *document, const LmP0Node *node);
static int lm_p0_validate_nonempty_colon_frames_in_structure(LmP0Document *document, const LmP0Structure *structure);
int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document);
int lm_p0_parse_string(const char *source, LmP0Document **out_document);
int lm_p0_parse_file(const char *path, LmP0Document **out_document);
static int lm_p0_registry_source_push_column_metadata(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_p0_registry_source_push_table_row(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
static int lm_p0_registry_source_join_table(void *context, const LmP0Text *source_table, const LmP0Text *target_table);
static int lm_p0_registry_source_text_all_char(const LmP0Text *text, char ch);
static int lm_p0_registry_source_text_is_array_receiver_head(const LmP0Text *head);
static const LmP0Text * lm_p0_registry_source_frame_receiver_key(const LmP0Frame *frame);
static int lm_p0_registry_source_parse_size_payload(const char *payload, size_t *out_value);
static int lm_p0_registry_source_formal_param_unwrap_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_p0_registry_source_positional_name_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static LmRegistrySourceLoader * lm_p0_registry_source_loader_new(void);
static int lm_p0_path_has_extension(const char *path, const char *extension);
static int lm_p0_registry_require_source_only(const char *phase);
static int lm_p0_registry_parse_unsigned_payload(const char *payload, unsigned *out_value);
static const char * lm_p0_registry_lookup_key_by_unsigned_payload(const char *table, unsigned value);
static int lm_p0_registry_validate_abi_constant(const char *table, const char *key, unsigned expected);
static int lm_p0_registry_validate_abi_constants(void);
const char * lm_p0_node_kind_class_name(LmP0NodeKind kind);
static int lm_p0_registry_load_default(void);
void lm_p0_document_destroy(LmP0Document *document);
const LmP0Node * lm_p0_document_root(const LmP0Document *document);
LmP0Node * lm_p0_document_mutable_root(LmP0Document *document);
const LmP0Diagnostic * lm_p0_document_diagnostic(const LmP0Document *document);
void lm_p0_free(void *ptr);
static int lm_p0_dump_reserve(LmP0Dump *dump, size_t extra);
static void lm_p0_dump_append(LmP0Dump *dump, const char *text, size_t length);
static void lm_p0_dump_append_cstr(LmP0Dump *dump, const char *text);
static void lm_p0_dump_indent(LmP0Dump *dump, size_t indent);
static void lm_p0_dump_text(LmP0Dump *dump, const LmP0Text *text);
static LmP0DumpFrame * lm_p0_dump_frame_new(int phase, size_t indent);
static int lm_p0_dump_push_frame(LmOwnPtrStack *stack, LmP0DumpFrame *frame);
static int lm_p0_dump_push_node(LmOwnPtrStack *stack, const LmP0Node *node, size_t indent);
static int lm_p0_dump_push_structure(LmOwnPtrStack *stack, const LmP0Structure *structure, size_t indent);
static int lm_p0_dump_push_trailer(LmOwnPtrStack *stack, const LmP0Trailer *trailer, size_t indent);
static LmOwnPtrStack * lm_p0_dump_stack_new(void);
static void lm_p0_dump_stack_delete(LmOwnPtrStack **stack);
static void lm_p0_dump_run(LmP0Dump *dump, LmOwnPtrStack *stack);
static void lm_p0_dump_node(LmP0Dump *dump, const LmP0Node *node, size_t indent);
static LmP0Dump * lm_p0_dump_new(void);
static char * lm_p0_dump_take_data(LmP0Dump *dump);
static void lm_p0_dump_delete(LmP0Dump *dump);
char * lm_p0_dump_alloc(const LmP0Document *document);

#ifndef LM_UNUSED
#define LM_UNUSED(value) ((void)(value))
#endif













































static LmP0Registry * lm_p0_registry;





static char * lm_table_descriptor_copy_slice(const char *data, size_t length) {
    if (data == 0 && length != 0U) {
        return 0;
    }
    return lm_own_copy_bytes(data, length);
}

static char * lm_table_descriptor_copy_cstr(const char *value) {
    if (value == 0) {
        return 0;
    }
    return lm_table_descriptor_copy_slice(value, strlen(value));
}

static int lm_table_descriptor_slice_equals(const char *left, size_t left_length, const char *right, size_t right_length) {
    if (left_length != right_length) {
        return 0;
    }
    if (left_length == 0U) {
        return 1;
    }
    if (left == 0 || right == 0) {
        return 0;
    }
    return memcmp(left, right, left_length) == 0;
}

static int lm_table_descriptor_cstr_equals_slice(const char *left, const char *right, size_t right_length) {
    if (left == 0) {
        return 0;
    }
    return lm_table_descriptor_slice_equals(left, strlen(left), right, right_length);
}

static LmOwnPtrStack * lm_table_descriptor_stack_new(LmOwnDelete delete_item) {
    LmOwnPtrStack * stack;
    stack = lm_own_new_zero(sizeof(stack[0]));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, delete_item);
    }
    return stack;
}

static void lm_table_column_descriptor_delete_any(void *object) {
    LmTableColumnDescriptor * column;
    column = object;
    if (column == 0) {
        return;
    }
    lm_own_delete(column -> name, 0);
    lm_own_delete(column -> descriptor, 0);
    if (column -> descriptors != 0) {
        lm_own_ptr_stack_destroy(column -> descriptors);
        lm_own_delete(column -> descriptors, 0);
        column->descriptors = 0;
    }
    lm_own_delete(column -> type_name, 0);
    column->name = 0;
    column->descriptor = 0;
    column->type_name = 0;
    lm_own_delete(column, 0);
}

static void lm_table_cell_delete_any(void *object) {
    LmTableCell * cell;
    cell = object;
    if (cell == 0) {
        return;
    }
    lm_own_delete(cell -> atom, 0);
    lm_own_delete(cell -> value, 0);
    cell->atom = 0;
    cell->value = 0;
    cell->node = 0;
    cell->source = 0;
    lm_own_delete(cell, 0);
}

static void lm_table_row_delete_any(void *object) {
    LmTableRow * row;
    row = object;
    if (row == 0) {
        return;
    }
    if (row -> cells != 0) {
        lm_own_ptr_stack_destroy(row -> cells);
        lm_own_delete(row -> cells, 0);
        row->cells = 0;
    }
    row->source = 0;
    row->source_native = 0;
    lm_own_delete(row, 0);
}

static void lm_registry_view_row_delete_any(void *object) {
    LmRegistryViewRow * row;
    row = object;
    if (row == 0) {
        return;
    }
    lm_own_delete(row -> table, 0);
    lm_own_delete(row -> key, 0);
    lm_own_delete(row -> payload, 0);
    row->table = 0;
    row->key = 0;
    row->payload = 0;
    row->payload_node = 0;
    row->source = 0;
    lm_own_delete(row, 0);
}

static void lm_table_descriptor_delete_any(void *object) {
    LmTableDescriptor * table;
    table = object;
    if (table == 0) {
        return;
    }
    if (table -> columns != 0) {
        lm_own_ptr_stack_destroy(table -> columns);
        lm_own_delete(table -> columns, 0);
        table->columns = 0;
    }
    if (table -> rows != 0) {
        lm_own_ptr_stack_destroy(table -> rows);
        lm_own_delete(table -> rows, 0);
        table->rows = 0;
    }
    if (table -> materialized_rows != 0) {
        lm_own_ptr_stack_destroy(table -> materialized_rows);
        lm_own_delete(table -> materialized_rows, 0);
        table->materialized_rows = 0;
    }
    if (table -> source_rows != 0) {
        lm_own_ptr_stack_destroy(table -> source_rows);
        lm_own_delete(table -> source_rows, 0);
        table->source_rows = 0;
    }
    lm_own_delete(table -> name, 0);
    table->name = 0;
    lm_own_delete(table, 0);
}

static LmTableColumnDescriptor * lm_table_column_descriptor_new_slice(const char *name, size_t name_length, const char *type_name, size_t type_name_length, size_t address_depth, size_t array_rank, int is_const) {
    LmTableColumnDescriptor * column;
    if (name == 0) {
        return 0;
    }
    column = lm_own_new_zero(sizeof(column[0]));
    if (column == 0) {
        return 0;
    }
    column->name = lm_table_descriptor_copy_slice(name, name_length);
    column->descriptors = lm_table_descriptor_stack_new(lm_own_delete_plain);
    if (type_name != 0) {
        column->type_name = lm_table_descriptor_copy_slice(type_name, type_name_length);
    }
    column->address_depth = address_depth;
    column->array_rank = array_rank;
    column->is_const = is_const;
    if (column -> name == 0 || column -> descriptors == 0 || (type_name != 0 && column -> type_name == 0)) {
        lm_table_column_descriptor_delete_any(column);
        return 0;
    }
    return column;
}

static int lm_table_column_descriptor_add_descriptor_slice(LmTableColumnDescriptor *column, const char *descriptor, size_t descriptor_length) {
    char *copy;
    if (column == 0 || column -> descriptors == 0 || descriptor == 0) {
        return 1;
    }
    copy = lm_table_descriptor_copy_slice(descriptor, descriptor_length);
    if (copy == 0) {
        return 1;
    }
    if (column -> descriptor == 0) {
        column->descriptor = lm_table_descriptor_copy_slice(descriptor, descriptor_length);
        if (column -> descriptor == 0) {
            lm_own_delete(copy, 0);
            return 1;
        }
    }
    if (lm_own_ptr_stack_push(column -> descriptors, copy) != 0) {
        lm_own_delete(copy, 0);
        return 1;
    }
    return 0;
}

static size_t lm_table_column_descriptor_descriptor_count(const LmTableColumnDescriptor *column) {
    if (column == 0 || column -> descriptors == 0) {
        return 0U;
    }
    return column -> descriptors -> count;
}

static const char * lm_table_column_descriptor_descriptor_at(const LmTableColumnDescriptor *column, size_t index) {
    if (column == 0 || column -> descriptors == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(column -> descriptors, index);
}

static int lm_table_column_descriptor_has_descriptor(const LmTableColumnDescriptor *column, const char *descriptor) {
    const char *candidate;
    size_t index;
    if (column == 0 || descriptor == 0) {
        return 0;
    }
    index = 0U;
    while (index < lm_table_column_descriptor_descriptor_count(column)) {
        candidate = lm_table_column_descriptor_descriptor_at(column, index);
        if (candidate != 0 && strcmp(candidate, descriptor) == 0) {
            return 1;
        }
        index = index + 1U;
    }
    return 0;
}

static int lm_table_descriptor_take_column(LmTableDescriptor *table, LmTableColumnDescriptor *column) {
    if (table == 0 || table -> columns == 0 || column == 0 || column -> name == 0) {
        return 1;
    }
    column->index = table -> columns -> count;
    if (lm_own_ptr_stack_push(table -> columns, column) != 0) {
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_add_column_slices(LmTableDescriptor *table, const char *name, size_t name_length, const char *type_name, size_t type_name_length, const char **descriptor_data, const size_t *descriptor_lengths, size_t descriptor_count, size_t address_depth, size_t array_rank, int is_const) {
    LmTableColumnDescriptor * column;
    size_t descriptor_index;
    if (table == 0 || name == 0 || (type_name == 0 && type_name_length != 0U) || (descriptor_count != 0U && (descriptor_data == 0 || descriptor_lengths == 0))) {
        return 1;
    }
    column = lm_table_column_descriptor_new_slice(name, name_length, type_name, type_name_length, address_depth, array_rank, is_const);
    if (column == 0) {
        return 1;
    }
    descriptor_index = 0U;
    while (descriptor_index < descriptor_count) {
        if (descriptor_data[descriptor_index] == 0 || lm_table_column_descriptor_add_descriptor_slice(column, descriptor_data[descriptor_index], descriptor_lengths[descriptor_index]) != 0) {
            lm_table_column_descriptor_delete_any(column);
            return 1;
        }
        descriptor_index = descriptor_index + 1U;
    }
    if (lm_table_descriptor_take_column(table, column) != 0) {
        lm_table_column_descriptor_delete_any(column);
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_add_column(LmTableDescriptor *table, const char *name, const char *descriptor) {
    const char *descriptor_data[1];
    size_t descriptor_lengths[1];
    if (name == 0) {
        return 1;
    }
    if (descriptor == 0) {
        return lm_table_descriptor_add_column_slices(table, name, strlen(name), 0, 0U, 0, 0, 0U, 0U, 0U, 0);
    }
    descriptor_data[0] = descriptor;
    descriptor_lengths[0] = strlen(descriptor);
    return lm_table_descriptor_add_column_slices(table, name, strlen(name), 0, 0U, (((const char **)descriptor_data)), descriptor_lengths, 1U, 0U, 0U, 0);
}

static LmTableDescriptor * lm_table_descriptor_new_empty_slice(const char *name, size_t name_length) {
    LmTableDescriptor * table;
    if (name == 0) {
        return 0;
    }
    table = lm_own_new_zero(sizeof(table[0]));
    if (table == 0) {
        return 0;
    }
    table->name = lm_table_descriptor_copy_slice(name, name_length);
    table->columns = lm_table_descriptor_stack_new(lm_table_column_descriptor_delete_any);
    table->rows = lm_table_descriptor_stack_new(lm_registry_view_row_delete_any);
    table->materialized_rows = lm_table_descriptor_stack_new(lm_table_row_delete_any);
    table->source_rows = lm_table_descriptor_stack_new(0);
    if (table -> name == 0 || table -> columns == 0 || table -> rows == 0 || table -> materialized_rows == 0 || table -> source_rows == 0) {
        lm_table_descriptor_delete_any(table);
        return 0;
    }
    return table;
}

static LmTableDescriptor * lm_table_descriptor_new_slice(const char *name, size_t name_length) {
    LmTableDescriptor * table;
    table = lm_table_descriptor_new_empty_slice(name, name_length);
    if (table == 0) {
        return 0;
    }
    if (lm_table_descriptor_add_column(table, "class", "key") != 0 || lm_table_descriptor_add_column(table, "value", "payload") != 0) {
        lm_table_descriptor_delete_any(table);
        return 0;
    }
    return table;
}

static size_t lm_table_descriptor_column_count(const LmTableDescriptor *table) {
    if (table == 0 || table -> columns == 0) {
        return 0U;
    }
    return table -> columns -> count;
}

static const LmTableColumnDescriptor * lm_table_descriptor_column_at(const LmTableDescriptor *table, size_t index) {
    if (table == 0 || table -> columns == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> columns, index);
}

static size_t lm_table_descriptor_row_count(const LmTableDescriptor *table) {
    if (table == 0 || table -> rows == 0) {
        return 0U;
    }
    return table -> rows -> count;
}

static const LmRegistryViewRow * lm_table_descriptor_row_at(const LmTableDescriptor *table, size_t index) {
    if (table == 0 || table -> rows == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> rows, index);
}

static LmTableCell * lm_table_cell_new_slice(const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none) {
    LmTableCell * cell;
    if (atom == 0 && value == 0 && node == 0 && explicit_none == 0) {
        return 0;
    }
    cell = lm_own_new_zero(sizeof(cell[0]));
    if (cell == 0) {
        return 0;
    }
    if (atom != 0) {
        cell->atom = lm_table_descriptor_copy_slice(atom, atom_length);
    }
    if (value != 0) {
        cell->value = lm_table_descriptor_copy_slice(value, value_length);
    }
    cell->node = node;
    cell->source = source;
    cell->explicit_none = explicit_none;
    if ((atom != 0 && cell -> atom == 0) || (value != 0 && cell -> value == 0)) {
        lm_table_cell_delete_any(cell);
        return 0;
    }
    return cell;
}

static LmTableCell * lm_table_cell_new_cstr(const char *atom, const char *value, const void *node, const void *source, int explicit_none) {
    size_t atom_length;
    size_t value_length;
    atom_length = 0U;
    value_length = 0U;
    if (atom != 0) {
        atom_length = strlen(atom);
    }
    if (value != 0) {
        value_length = strlen(value);
    }
    return lm_table_cell_new_slice(atom, atom_length, value, value_length, node, source, explicit_none);
}

static LmTableRow * lm_table_row_new(const LmTableRow *source) {
    LmTableRow * row;
    row = lm_own_new_zero(sizeof(row[0]));
    if (row == 0) {
        return 0;
    }
    row->cells = lm_table_descriptor_stack_new(lm_table_cell_delete_any);
    row->source = source;
    if (source != 0) {
        row->source_native = source -> source_native;
    }
    if (row -> cells == 0) {
        lm_table_row_delete_any(row);
        return 0;
    }
    return row;
}

static int lm_table_row_take_cell(LmTableRow *row, LmTableCell *cell) {
    if (row == 0 || row -> cells == 0 || cell == 0) {
        return 1;
    }
    return lm_own_ptr_stack_push(row -> cells, cell);
}

static int lm_table_row_take_cell_slice(LmTableRow *row, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none) {
    LmTableCell * cell;
    cell = lm_table_cell_new_slice(atom, atom_length, value, value_length, node, source, explicit_none);
    if (cell == 0) {
        return 1;
    }
    if (lm_table_row_take_cell(row, cell) != 0) {
        lm_table_cell_delete_any(cell);
        return 1;
    }
    return 0;
}

static size_t lm_table_row_cell_count(const LmTableRow *row) {
    if (row == 0 || row -> cells == 0) {
        return 0U;
    }
    return row -> cells -> count;
}

static const LmTableCell * lm_table_row_cell_at(const LmTableRow *row, size_t index) {
    if (row == 0 || row -> cells == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(row -> cells, index);
}

static int lm_table_descriptor_take_materialized_row(LmTableDescriptor *table, LmTableRow *row) {
    if (table == 0 || table -> materialized_rows == 0 || row == 0 || row -> cells == 0) {
        return 1;
    }
    if (row -> cells -> count != lm_table_descriptor_column_count(table)) {
        return 1;
    }
    row->local_sequence = table -> materialized_rows -> count;
    return lm_own_ptr_stack_push(table -> materialized_rows, row);
}

static size_t lm_table_descriptor_materialized_row_count(const LmTableDescriptor *table) {
    if (table == 0 || table -> materialized_rows == 0) {
        return 0U;
    }
    return table -> materialized_rows -> count;
}

static const LmTableRow * lm_table_descriptor_materialized_row_at(const LmTableDescriptor *table, size_t index) {
    if (table == 0 || table -> materialized_rows == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> materialized_rows, index);
}

static int lm_table_descriptor_append_materialized_row_copy(LmTableDescriptor *table, const LmTableRow *source_row) {
    LmTableRow * row;
    const LmTableCell * source_cell;
    LmTableCell * cell;
    size_t atom_length;
    size_t value_length;
    size_t index;
    if (table == 0 || source_row == 0 || lm_table_row_cell_count(source_row) != lm_table_descriptor_column_count(table)) {
        return 1;
    }
    row = lm_table_row_new(source_row);
    if (row == 0) {
        return 1;
    }
    index = 0U;
    while (index < lm_table_row_cell_count(source_row)) {
        source_cell = lm_table_row_cell_at(source_row, index);
        if (source_cell == 0) {
            lm_table_row_delete_any(row);
            return 1;
        }
        if (source_cell -> atom == 0 && source_cell -> value == 0) {
            cell = lm_table_cell_new_cstr(0, 0, source_cell -> node, source_cell -> source, source_cell -> explicit_none);
            if (cell == 0 || lm_table_row_take_cell(row, cell) != 0) {
                lm_table_cell_delete_any(cell);
                lm_table_row_delete_any(row);
                return 1;
            }
        }
        else {
            atom_length = 0U;
            value_length = 0U;
            if (source_cell -> atom != 0) {
                atom_length = strlen(source_cell -> atom);
            }
            if (source_cell -> value != 0) {
                value_length = strlen(source_cell -> value);
            }
            if (lm_table_row_take_cell_slice(row, source_cell -> atom, atom_length, source_cell -> value, value_length, source_cell -> node, source_cell -> source, source_cell -> explicit_none) != 0) {
                lm_table_row_delete_any(row);
                return 1;
            }
        }
        index = index + 1U;
    }
    if (lm_table_descriptor_take_materialized_row(table, row) != 0) {
        lm_table_row_delete_any(row);
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_schema_same(const LmTableDescriptor *left, const LmTableDescriptor *right) {
    const LmTableColumnDescriptor * left_column;
    const LmTableColumnDescriptor * right_column;
    const char *left_descriptor;
    const char *right_descriptor;
    size_t column_index;
    size_t descriptor_index;
    if (left == 0 || right == 0 || lm_table_descriptor_column_count(left) != lm_table_descriptor_column_count(right)) {
        return 0;
    }
    column_index = 0U;
    while (column_index < lm_table_descriptor_column_count(left)) {
        left_column = lm_table_descriptor_column_at(left, column_index);
        right_column = lm_table_descriptor_column_at(right, column_index);
        if (left_column == 0 || right_column == 0 || left_column -> name == 0 || right_column -> name == 0 || strcmp(left_column -> name, right_column -> name) != 0 || left_column -> address_depth != right_column -> address_depth || left_column -> array_rank != right_column -> array_rank || left_column -> is_const != right_column -> is_const || lm_table_column_descriptor_descriptor_count(left_column) != lm_table_column_descriptor_descriptor_count(right_column)) {
            return 0;
        }
        if ((left_column -> type_name == 0) != (right_column -> type_name == 0)) {
            return 0;
        }
        if (left_column -> type_name != 0 && strcmp(left_column -> type_name, right_column -> type_name) != 0) {
            return 0;
        }
        descriptor_index = 0U;
        while (descriptor_index < lm_table_column_descriptor_descriptor_count(left_column)) {
            left_descriptor = lm_table_column_descriptor_descriptor_at(left_column, descriptor_index);
            right_descriptor = lm_table_column_descriptor_descriptor_at(right_column, descriptor_index);
            if (left_descriptor == 0 || right_descriptor == 0 || lm_table_column_descriptor_has_descriptor(right_column, left_descriptor) == 0 || strcmp(left_descriptor, right_descriptor) != 0) {
                return 0;
            }
            descriptor_index = descriptor_index + 1U;
        }
        column_index = column_index + 1U;
    }
    return 1;
}

static int lm_table_descriptor_join_schema_compatible(const LmTableDescriptor *source, const LmTableDescriptor *target) {
    if (lm_table_descriptor_schema_same(source, target) != 0) {
        return 1;
    }
    return lm_table_descriptor_column_count(source) == 2U && lm_table_descriptor_column_count(target) == 2U;
}

static LmRegistryView * lm_registry_view_new(const LmRegistryView *parent) {
    LmRegistryView * view;
    view = lm_own_new_zero(sizeof(view[0]));
    if (view == 0) {
        return 0;
    }
    view->parent = parent;
    view->tables = lm_table_descriptor_stack_new(lm_table_descriptor_delete_any);
    view->source_tables = lm_table_descriptor_stack_new(lm_table_descriptor_delete_any);
    view->facts = lm_table_descriptor_stack_new(0);
    view->class_names = lm_table_descriptor_stack_new(lm_own_delete_plain);
    if (view -> tables == 0 || view -> source_tables == 0 || view -> facts == 0 || view -> class_names == 0) {
        if (view -> tables != 0) {
            lm_own_ptr_stack_destroy(view -> tables);
            lm_own_delete(view -> tables, 0);
        }
        if (view -> facts != 0) {
            lm_own_ptr_stack_destroy(view -> facts);
            lm_own_delete(view -> facts, 0);
        }
        if (view -> source_tables != 0) {
            lm_own_ptr_stack_destroy(view -> source_tables);
            lm_own_delete(view -> source_tables, 0);
        }
        if (view -> class_names != 0) {
            lm_own_ptr_stack_destroy(view -> class_names);
            lm_own_delete(view -> class_names, 0);
        }
        lm_own_delete(view, 0);
        return 0;
    }
    return view;
}

static void lm_registry_view_note_mutation(LmRegistryView *view) {
    if (view != 0) {
        view->mutation_generation = view -> mutation_generation + 1U;
    }
}

static void lm_registry_view_delete(LmRegistryView *view) {
    if (view == 0) {
        return;
    }
    if (view -> facts != 0) {
        lm_own_ptr_stack_destroy(view -> facts);
        lm_own_delete(view -> facts, 0);
        view->facts = 0;
    }
    if (view -> tables != 0) {
        lm_own_ptr_stack_destroy(view -> tables);
        lm_own_delete(view -> tables, 0);
        view->tables = 0;
    }
    if (view -> source_tables != 0) {
        lm_own_ptr_stack_destroy(view -> source_tables);
        lm_own_delete(view -> source_tables, 0);
        view->source_tables = 0;
    }
    if (view -> class_names != 0) {
        lm_own_ptr_stack_destroy(view -> class_names);
        lm_own_delete(view -> class_names, 0);
        view->class_names = 0;
    }
    view->parent = 0;
    view->local_fact_count = 0U;
    lm_own_delete(view, 0);
}

static int lm_registry_view_class_has_slice(const LmRegistryView *view, const char *name, size_t name_length) {
    size_t index;
    const char *candidate;
    if (view == 0 || name == 0) {
        return 0;
    }
    if (view -> class_names != 0) {
        index = 0U;
        while (index < view -> class_names -> count) {
            candidate = lm_own_ptr_stack_at(view -> class_names, index);
            if (candidate != 0 && lm_table_descriptor_cstr_equals_slice(candidate, name, name_length) != 0) {
                return 1;
            }
            index = index + 1U;
        }
    }
    return lm_registry_view_class_has_slice(view -> parent, name, name_length);
}

static int lm_registry_view_class_has(const LmRegistryView *view, const char *name) {
    if (name == 0) {
        return 0;
    }
    return lm_registry_view_class_has_slice(view, name, strlen(name));
}

static int lm_registry_view_class_add(LmRegistryView *view, const char *name) {
    char *copy;
    if (view == 0 || view -> class_names == 0 || name == 0) {
        return 1;
    }
    if (lm_registry_view_class_has(view, name) != 0) {
        return 0;
    }
    copy = lm_table_descriptor_copy_cstr(name);
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> class_names, copy) != 0) {
        lm_own_delete(copy, 0);
        return 1;
    }
    lm_registry_view_note_mutation(view);
    return 0;
}

static size_t lm_registry_view_class_count(const LmRegistryView *view) {
    size_t count;
    size_t index;
    const LmRegistryView * current;
    const char *candidate;
    count = 0U;
    current = view;
    while (current != 0) {
        if (current -> class_names != 0) {
            index = 0U;
            while (index < current -> class_names -> count) {
                candidate = lm_own_ptr_stack_at(current -> class_names, index);
                if (candidate != 0 && lm_registry_view_class_has(current -> parent, candidate) == 0) {
                    count = count + 1U;
                }
                index = index + 1U;
            }
        }
        current = current -> parent;
    }
    return count;
}

static const char * lm_registry_view_class_at(const LmRegistryView *view, size_t index) {
    size_t parent_count;
    size_t local_index;
    const LmRegistryView * current;
    const char *candidate;
    current = view;
    while (current != 0) {
        parent_count = lm_registry_view_class_count(current -> parent);
        if (index < parent_count) {
            current = current -> parent;
            continue;
        }
        index = index - parent_count;
        if (current -> class_names == 0) {
            return 0;
        }
        local_index = 0U;
        while (local_index < current -> class_names -> count) {
            candidate = lm_own_ptr_stack_at(current -> class_names, local_index);
            if (candidate != 0 && lm_registry_view_class_has(current -> parent, candidate) == 0) {
                if (index == 0U) {
                    return candidate;
                }
                index = index - 1U;
            }
            local_index = local_index + 1U;
        }
        return 0;
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_find_local_table_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    size_t index;
    LmTableDescriptor * descriptor;
    if (view == 0 || view -> tables == 0 || table == 0) {
        return 0;
    }
    index = 0U;
    while (index < view -> tables -> count) {
        descriptor = lm_own_ptr_stack_at(view -> tables, index);
        if (descriptor != 0 && descriptor -> name != 0 && lm_table_descriptor_cstr_equals_slice(descriptor -> name, table, table_length) != 0) {
            return descriptor;
        }
        index = index + 1U;
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_ensure_local_table_slice(LmRegistryView *view, const char *table, size_t table_length) {
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    if (descriptor != 0) {
        return descriptor;
    }
    if (view == 0 || view -> tables == 0) {
        return 0;
    }
    descriptor = lm_table_descriptor_new_slice(table, table_length);
    if (descriptor == 0) {
        return 0;
    }
    if (lm_own_ptr_stack_push(view -> tables, descriptor) != 0) {
        lm_table_descriptor_delete_any(descriptor);
        return 0;
    }
    return descriptor;
}

static LmTableDescriptor * lm_registry_view_find_local_source_table_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    size_t index;
    LmTableDescriptor * descriptor;
    if (view == 0 || view -> source_tables == 0 || table == 0) {
        return 0;
    }
    index = view -> source_tables -> count;
    while (index > 0U) {
        index = index - 1U;
        descriptor = lm_own_ptr_stack_at(view -> source_tables, index);
        if (descriptor != 0 && descriptor -> name != 0 && lm_table_descriptor_cstr_equals_slice(descriptor -> name, table, table_length) != 0) {
            return descriptor;
        }
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_find_source_table_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0;
    }
    descriptor = lm_registry_view_find_local_source_table_slice(view, table, table_length);
    if (descriptor != 0) {
        return descriptor;
    }
    return lm_registry_view_find_source_table_slice(view -> parent, table, table_length);
}

static int lm_registry_view_take_local_source_table(LmRegistryView *view, LmTableDescriptor *descriptor) {
    if (view == 0 || view -> source_tables == 0 || descriptor == 0 || descriptor -> name == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> source_tables, descriptor) != 0) {
        return 1;
    }
    lm_registry_view_note_mutation(view);
    return 0;
}

static size_t lm_registry_view_source_table_count(const LmRegistryView *view) {
    size_t count;
    if (view == 0) {
        return 0U;
    }
    count = 0U;
    if (view -> source_tables != 0) {
        count = view -> source_tables -> count;
    }
    if (view -> parent != 0) {
        count = count + lm_registry_view_source_table_count(view -> parent);
    }
    return count;
}

static const LmTableDescriptor * lm_registry_view_source_table_at(const LmRegistryView *view, size_t index) {
    size_t parent_count;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_source_table_count(view -> parent);
    if (index < parent_count) {
        return lm_registry_view_source_table_at(view -> parent, index);
    }
    if (view -> source_tables == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(view -> source_tables, index - parent_count);
}

static int lm_table_descriptor_source_path_column_matches(const LmTableDescriptor *descriptor, size_t column_index, const char *path, size_t path_length) {
    const LmTableColumnDescriptor * column;
    size_t table_name_length;
    size_t column_count;
    size_t column_name_length;
    if (descriptor == 0 || descriptor -> name == 0 || path == 0) {
        return 0;
    }
    column_count = lm_table_descriptor_column_count(descriptor);
    if (column_index == 0U || column_index >= column_count) {
        return 0;
    }
    column = lm_table_descriptor_column_at(descriptor, column_index);
    if (column == 0 || column -> name == 0) {
        return 0;
    }
    if (column_count == 2U) {
        return lm_table_descriptor_cstr_equals_slice(descriptor -> name, path, path_length);
    }
    table_name_length = strlen(descriptor -> name);
    column_name_length = strlen(column -> name);
    if (path_length != table_name_length + 1U + column_name_length) {
        return 0;
    }
    if (memcmp(path, descriptor -> name, table_name_length) != 0 || path[table_name_length] != '.') {
        return 0;
    }
    return memcmp(path + table_name_length + 1U, column -> name, column_name_length) == 0;
}

static int lm_registry_view_source_path_has_rows_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, int projected_only, int *out_covered) {
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || out_covered == 0) {
        return 0;
    }
    table_count = lm_registry_view_source_table_count(view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(view, table_index);
        if (descriptor != 0 && descriptor -> name != 0) {
            column_count = lm_table_descriptor_column_count(descriptor);
            column_index = 1U;
            while (column_index < column_count) {
                if (lm_table_descriptor_source_path_column_matches(descriptor, column_index, path, path_length) != 0) {
                    out_covered[0] = 1;
                    row_count = lm_table_descriptor_materialized_row_count(descriptor);
                    row_index = 0U;
                    while (row_index < row_count) {
                        row = lm_table_descriptor_materialized_row_at(descriptor, row_index);
                        if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                            cell = lm_table_row_cell_at(row, column_index);
                            if (cell != 0 && (cell -> value != 0 || cell -> node != 0)) {
                                return 1;
                            }
                        }
                        row_index = row_index + 1U;
                    }
                }
                column_index = column_index + 1U;
            }
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static int lm_registry_view_source_path_has_rows_slice(const LmRegistryView *view, const char *path, size_t path_length, int *out_covered) {
    return lm_registry_view_source_path_has_rows_slice_mode(view, path, path_length, 0, out_covered);
}

static int lm_registry_view_source_path_has_rows(const LmRegistryView *view, const char *path, int *out_covered) {
    if (path == 0) {
        if (out_covered != 0) {
            out_covered[0] = 0;
        }
        return 0;
    }
    return lm_registry_view_source_path_has_rows_slice(view, path, strlen(path), out_covered);
}

static int lm_registry_view_source_path_has_key_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int projected_only, int *out_covered) {
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || key == 0 || out_covered == 0) {
        return 0;
    }
    table_count = lm_registry_view_source_table_count(view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(view, table_index);
        column_count = lm_table_descriptor_column_count(descriptor);
        column_index = 1U;
        while (column_index < column_count) {
            if (lm_table_descriptor_source_path_column_matches(descriptor, column_index, path, path_length) != 0) {
                out_covered[0] = 1;
                row_count = lm_table_descriptor_materialized_row_count(descriptor);
                row_index = 0U;
                while (row_index < row_count) {
                    row = lm_table_descriptor_materialized_row_at(descriptor, row_index);
                    if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                        key_cell = lm_table_row_cell_at(row, 0U);
                        payload_cell = lm_table_row_cell_at(row, column_index);
                        if (key_cell != 0 && key_cell -> value != 0 && lm_table_descriptor_cstr_equals_slice(key_cell -> value, key, key_length) != 0 && payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                            return 1;
                        }
                    }
                    row_index = row_index + 1U;
                }
            }
            column_index = column_index + 1U;
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static int lm_registry_view_source_path_has_key_slice(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int *out_covered) {
    return lm_registry_view_source_path_has_key_slice_mode(view, path, path_length, key, key_length, 0, out_covered);
}

static const LmTableCell * lm_registry_view_source_path_cell_at_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered) {
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    size_t fact_index;
    int descriptor_covered;
    if (out_descriptor != 0) {
        out_descriptor[0] = 0;
    }
    if (out_column_index != 0) {
        out_column_index[0] = 0U;
    }
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || out_covered == 0) {
        return 0;
    }
    fact_index = 0U;
    table_count = lm_registry_view_source_table_count(view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(view, table_index);
        column_count = lm_table_descriptor_column_count(descriptor);
        descriptor_covered = 0;
        column_index = 1U;
        while (column_index < column_count) {
            if (lm_table_descriptor_source_path_column_matches(descriptor, column_index, path, path_length) != 0) {
                descriptor_covered = 1;
                out_covered[0] = 1;
            }
            column_index = column_index + 1U;
        }
        if (descriptor_covered != 0) {
            row_count = lm_table_descriptor_materialized_row_count(descriptor);
            row_index = 0U;
            while (row_index < row_count) {
                row = lm_table_descriptor_materialized_row_at(descriptor, row_index);
                if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                    key_cell = lm_table_row_cell_at(row, 0U);
                    column_index = 1U;
                    while (column_index < column_count) {
                        if (lm_table_descriptor_source_path_column_matches(descriptor, column_index, path, path_length) != 0) {
                            payload_cell = lm_table_row_cell_at(row, column_index);
                            if (key_cell != 0 && key_cell -> value != 0 && payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                                if (fact_index == index) {
                                    if (out_descriptor != 0) {
                                        out_descriptor[0] = descriptor;
                                    }
                                    if (out_column_index != 0) {
                                        out_column_index[0] = column_index;
                                    }
                                    if (out_key_cell != 0) {
                                        out_key_cell[0] = key_cell;
                                    }
                                    return payload_cell;
                                }
                                fact_index = fact_index + 1U;
                            }
                        }
                        column_index = column_index + 1U;
                    }
                }
                row_index = row_index + 1U;
            }
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static const LmTableCell * lm_registry_view_source_path_cell_at_slice(const LmRegistryView *view, const char *path, size_t path_length, size_t index, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered) {
    return lm_registry_view_source_path_cell_at_slice_mode(view, path, path_length, index, 0, out_descriptor, out_column_index, out_key_cell, out_covered);
}

static const char * lm_registry_view_source_path_key_at_slice_mode(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, int *out_covered) {
    const LmTableCell * key_cell;
    key_cell = 0;
    if (projected_only == 0) {
        if (lm_registry_view_source_path_cell_at_slice(view, path, path_length, index, 0, 0, &key_cell, out_covered) == 0 || key_cell == 0) {
            return 0;
        }
    }
    else {
        if (lm_registry_view_source_path_cell_at_slice_mode(view, path, path_length, index, 1, 0, 0, &key_cell, out_covered) == 0 || key_cell == 0) {
            return 0;
        }
    }
    return key_cell -> value;
}

static const char * lm_registry_view_source_path_key_at_slice(const LmRegistryView *view, const char *path, size_t path_length, size_t index, int *out_covered) {
    return lm_registry_view_source_path_key_at_slice_mode(view, path, path_length, index, 0, out_covered);
}

static int lm_registry_view_append_materialized_rows_mode(LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length, int note_mutation, int require_compatible_source, size_t snapshot_source_count, const size_t *snapshot_row_counts) {
    const LmTableDescriptor * source;
    const LmTableRow * row;
    size_t source_count;
    size_t row_count;
    size_t original_row_count;
    size_t source_index;
    size_t index;
    int matched;
    int incompatible;
    if (view == 0 || target == 0 || source_table == 0) {
        return 1;
    }
    source = lm_registry_view_find_source_table_slice(view, source_table, source_table_length);
    if (source == 0) {
        return require_compatible_source != 0;
    }
    original_row_count = lm_table_descriptor_materialized_row_count(target);
    source_count = lm_registry_view_source_table_count(view);
    if (snapshot_row_counts != 0) {
        if (snapshot_source_count > source_count) {
            return 1;
        }
        source_count = snapshot_source_count;
    }
    source_index = 0U;
    matched = 0;
    incompatible = 0;
    while (source_index < source_count) {
        source = lm_registry_view_source_table_at(view, source_index);
        if (source != 0 && source -> name != 0 && lm_table_descriptor_cstr_equals_slice(source -> name, source_table, source_table_length) != 0) {
            if (lm_table_descriptor_join_schema_compatible(source, target) == 0) {
                incompatible = 1;
            }
            else {
                matched = 1;
                row_count = lm_table_descriptor_materialized_row_count(source);
                if (snapshot_row_counts != 0) {
                    if (snapshot_row_counts[source_index] > row_count) {
                        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
                        return 1;
                    }
                    row_count = snapshot_row_counts[source_index];
                }
                else {
                    if (source == target) {
                        row_count = original_row_count;
                    }
                }
                index = 0U;
                while (index < row_count) {
                    row = lm_table_descriptor_materialized_row_at(source, index);
                    if (row == 0 || lm_table_descriptor_append_materialized_row_copy(target, row) != 0) {
                        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
                        return 1;
                    }
                    index = index + 1U;
                }
            }
        }
        source_index = source_index + 1U;
    }
    if (require_compatible_source != 0 && (matched == 0 || incompatible != 0)) {
        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
        return 1;
    }
    if (note_mutation != 0 && lm_table_descriptor_materialized_row_count(target) != original_row_count) {
        lm_registry_view_note_mutation(view);
    }
    return 0;
}

static int lm_registry_view_append_materialized_rows(LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length) {
    return lm_registry_view_append_materialized_rows_mode(view, target, source_table, source_table_length, 1, 0, 0U, 0);
}

static size_t lm_registry_view_fact_count(const LmRegistryView *view) {
    size_t count;
    if (view == 0) {
        return 0U;
    }
    count = view -> local_fact_count;
    if (view -> parent != 0) {
        count = count + lm_registry_view_fact_count(view -> parent);
    }
    return count;
}

static const LmRegistryViewRow * lm_registry_view_fact_at(const LmRegistryView *view, size_t index) {
    size_t parent_count;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_fact_count(view -> parent);
    if (index < parent_count) {
        return lm_registry_view_fact_at(view -> parent, index);
    }
    if (view -> facts == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(view -> facts, index - parent_count);
}

static int lm_registry_view_push_relation(LmRegistryView *view, const char *table, const char *key, const char *payload, const void *payload_node, const void *source) {
    LmTableDescriptor * descriptor;
    LmRegistryViewRow * row;
    void *popped;
    if (view == 0 || table == 0 || key == 0 || (payload == 0 && payload_node == 0)) {
        return 1;
    }
    descriptor = lm_registry_view_ensure_local_table_slice(view, table, strlen(table));
    if (descriptor == 0 || descriptor -> rows == 0 || view -> facts == 0) {
        return 1;
    }
    row = lm_own_new_zero(sizeof(row[0]));
    if (row == 0) {
        return 1;
    }
    row->table = lm_table_descriptor_copy_cstr(table);
    row->key = lm_table_descriptor_copy_cstr(key);
    if (payload != 0) {
        row->payload = lm_table_descriptor_copy_cstr(payload);
    }
    row->payload_node = payload_node;
    row->source = source;
    row->local_sequence = view -> local_fact_count;
    if (row -> table == 0 || row -> key == 0 || (payload != 0 && row -> payload == 0)) {
        lm_registry_view_row_delete_any(row);
        return 1;
    }
    if (lm_own_ptr_stack_push(descriptor -> rows, row) != 0) {
        lm_registry_view_row_delete_any(row);
        return 1;
    }
    if (lm_own_ptr_stack_push(descriptor -> source_rows, (((void *)source))) != 0) {
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> facts, row) != 0) {
        popped = lm_own_ptr_stack_pop(descriptor -> source_rows);
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    if (payload != 0 && strcmp(table, "class.present") == 0 && lm_registry_view_class_add(view, key) != 0) {
        popped = lm_own_ptr_stack_pop(view -> facts);
        popped = lm_own_ptr_stack_pop(descriptor -> source_rows);
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    view->local_fact_count = view -> local_fact_count + 1U;
    lm_registry_view_note_mutation(view);
    return 0;
}

static const LmRegistryViewRow * lm_registry_view_lookup_local_exact_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    LmTableDescriptor * descriptor;
    LmRegistryViewRow * row;
    size_t index;
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    if (descriptor == 0 || descriptor -> rows == 0) {
        return 0;
    }
    index = descriptor -> rows -> count;
    while (index > 0U) {
        index = index - 1U;
        row = lm_own_ptr_stack_at(descriptor -> rows, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(row -> key, key, key_length) != 0) {
            return row;
        }
    }
    return 0;
}

static const LmRegistryViewRow * lm_registry_view_lookup_exact_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    const LmRegistryViewRow * row;
    if (view == 0 || key == 0 || table == 0) {
        return 0;
    }
    row = lm_registry_view_lookup_local_exact_slice(view, key, key_length, table, table_length);
    if (row != 0) {
        return row;
    }
    return lm_registry_view_lookup_exact_slice(view -> parent, key, key_length, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_exact(const LmRegistryView *view, const char *key, const char *table) {
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_exact_slice(view, key, strlen(key), table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_default_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    return lm_registry_view_lookup_exact_slice(view, "default", 7U, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_default(const LmRegistryView *view, const char *table) {
    if (table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_default_slice(view, table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    const LmRegistryViewRow * row;
    row = lm_registry_view_lookup_exact_slice(view, key, key_length, table, table_length);
    if (row != 0) {
        return row;
    }
    return lm_registry_view_lookup_default_slice(view, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup(const LmRegistryView *view, const char *key, const char *table) {
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_slice(view, key, strlen(key), table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_text_slice(const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    const LmRegistryViewRow * row;
    row = lm_registry_view_lookup_exact_slice(view, key, key_length, table, table_length);
    if (row != 0 && row -> payload != 0) {
        return row;
    }
    return lm_registry_view_lookup_default_slice(view, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_text(const LmRegistryView *view, const char *key, const char *table) {
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_text_slice(view, key, strlen(key), table, strlen(table));
}

static size_t lm_registry_view_local_table_row_count_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    return lm_table_descriptor_row_count(descriptor);
}

static size_t lm_registry_view_table_row_count_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    size_t count;
    if (view == 0 || table == 0) {
        return 0U;
    }
    count = lm_registry_view_local_table_row_count_slice(view, table, table_length);
    if (view -> parent != 0) {
        count = count + lm_registry_view_table_row_count_slice(view -> parent, table, table_length);
    }
    return count;
}

static size_t lm_registry_view_table_row_count(const LmRegistryView *view, const char *table) {
    if (table == 0) {
        return 0U;
    }
    return lm_registry_view_table_row_count_slice(view, table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_table_row_at_slice(const LmRegistryView *view, const char *table, size_t table_length, size_t index) {
    size_t parent_count;
    LmTableDescriptor * descriptor;
    if (view == 0 || table == 0) {
        return 0;
    }
    parent_count = lm_registry_view_table_row_count_slice(view -> parent, table, table_length);
    if (index < parent_count) {
        return lm_registry_view_table_row_at_slice(view -> parent, table, table_length, index);
    }
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    return lm_table_descriptor_row_at(descriptor, index - parent_count);
}

static const LmRegistryViewRow * lm_registry_view_table_row_at(const LmRegistryView *view, const char *table, size_t index) {
    if (table == 0) {
        return 0;
    }
    return lm_registry_view_table_row_at_slice(view, table, strlen(table), index);
}

static size_t lm_table_descriptor_matching_key_count_slice(const LmTableDescriptor *descriptor, const char *key, size_t key_length) {
    size_t count;
    size_t index;
    const LmRegistryViewRow * row;
    count = 0U;
    index = 0U;
    while (index < lm_table_descriptor_row_count(descriptor)) {
        row = lm_table_descriptor_row_at(descriptor, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(row -> key, key, key_length) != 0) {
            count = count + 1U;
        }
        index = index + 1U;
    }
    return count;
}

static const LmRegistryViewRow * lm_table_descriptor_matching_key_at_slice(const LmTableDescriptor *descriptor, const char *key, size_t key_length, size_t match_index) {
    size_t index;
    const LmRegistryViewRow * row;
    index = 0U;
    while (index < lm_table_descriptor_row_count(descriptor)) {
        row = lm_table_descriptor_row_at(descriptor, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(row -> key, key, key_length) != 0) {
            if (match_index == 0U) {
                return row;
            }
            match_index = match_index - 1U;
        }
        index = index + 1U;
    }
    return 0;
}

static size_t lm_registry_view_matching_key_count_slice(const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length) {
    size_t count;
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0U;
    }
    count = lm_registry_view_matching_key_count_slice(view -> parent, table, table_length, key, key_length);
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    return count + lm_table_descriptor_matching_key_count_slice(descriptor, key, key_length);
}

static size_t lm_registry_view_matching_key_count(const LmRegistryView *view, const char *table, const char *key) {
    if (table == 0 || key == 0) {
        return 0U;
    }
    return lm_registry_view_matching_key_count_slice(view, table, strlen(table), key, strlen(key));
}

static const LmRegistryViewRow * lm_registry_view_matching_key_at_slice(const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length, size_t match_index) {
    size_t parent_count;
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_matching_key_count_slice(view -> parent, table, table_length, key, key_length);
    if (match_index < parent_count) {
        return lm_registry_view_matching_key_at_slice(view -> parent, table, table_length, key, key_length, match_index);
    }
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    return lm_table_descriptor_matching_key_at_slice(descriptor, key, key_length, match_index - parent_count);
}

static const LmRegistryViewRow * lm_registry_view_matching_key_at(const LmRegistryView *view, const char *table, const char *key, size_t match_index) {
    if (table == 0 || key == 0) {
        return 0;
    }
    return lm_registry_view_matching_key_at_slice(view, table, strlen(table), key, strlen(key), match_index);
}

static const LmOwnPtrStack * lm_registry_view_local_source_rows_slice(const LmRegistryView *view, const char *table, size_t table_length) {
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(view, table, table_length);
    if (descriptor == 0) {
        return 0;
    }
    return descriptor -> source_rows;
}

static int lm_registry_view_table_has_rows(const LmRegistryView *view, const char *table) {
    return lm_registry_view_table_row_count(view, table) != 0U;
}


static int lm_p0_text_equals(const LmP0Text *text, const char *value) {
    size_t value_length;
    if (text == 0 || value == 0) {
        return 0;
    }
    value_length = strlen(value);
    return text -> length == value_length && memcmp(text -> data, value, value_length) == 0;
}

static int lm_p0_identifier_payload(const LmP0Text *atom, LmP0Text *out_payload) {
    if (atom == 0 || out_payload == 0 || atom -> data == 0) {
        return 0;
    }
    out_payload->data = atom -> data;
    out_payload->length = atom -> length;
    if (atom -> length >= 2U && atom -> data[0] == '`' && atom -> data[atom -> length - 1U] == '`') {
        out_payload->data = atom -> data + 1U;
        out_payload->length = atom -> length - 2U;
    }
    return 1;
}

static int lm_p0_registry_identifier_value(const LmP0Text *atom, LmP0Text *out_payload) {
    if (atom == 0 || out_payload == 0) {
        return 0;
    }
    if (atom -> length > 0U && (atom -> data[0] == '"' || atom -> data[0] == '\'')) {
        return 0;
    }
    return lm_p0_identifier_payload(atom, out_payload);
}

static int lm_p0_registry_literal_value(const LmP0Text *atom, LmP0Text *out_payload) {
    char quote;
    if (atom == 0 || out_payload == 0 || atom -> data == 0) {
        return 0;
    }
    out_payload->data = atom -> data;
    out_payload->length = atom -> length;
    if (atom -> length < 2U) {
        return 1;
    }
    quote = atom -> data[0];
    if ((quote == '"' || quote == '\'') && atom -> data[atom -> length - 1U] == quote) {
        out_payload->data = atom -> data + 1U;
        out_payload->length = atom -> length - 2U;
        return 1;
    }
    return lm_p0_identifier_payload(atom, out_payload);
}

static int lm_p0_registry_payload_is_null(const LmP0Text *atom) {
    LmP0Text * payload;
    int is_null;
    payload = lm_p0_text_view_new_cstr("");
    if (payload == 0) {
        return 0;
    }
    if (lm_p0_registry_identifier_value(atom, payload) == 0) {
        lm_p0_text_view_delete(payload);
        return 0;
    }
    is_null = payload -> length == 4U && memcmp(payload -> data, "None", 4U) == 0;
    lm_p0_text_view_delete(payload);
    return is_null;
}

static int lm_p0_is_horizontal_space(char value) {
    return value == ' ' || value == '\t';
}

static int lm_p0_is_line_break(char value) {
    return value == '\n' || value == '\r';
}

static size_t lm_p0_line_break_width_at(const char *source, size_t length, size_t index) {
    if (index >= length) {
        return 0U;
    }
    if (source[index] == '\r') {
        if (index + 1U < length && source[index + 1U] == '\n') {
            return 2U;
        }
        return 1U;
    }
    if (source[index] == '\n') {
        return 1U;
    }
    return 0U;
}

static int lm_p0_is_field_space(char value) {
    return lm_p0_is_horizontal_space(value) || lm_p0_is_line_break(value);
}

static int lm_p0_is_field_separator(char value) {
    return value == ',' || value == ';';
}

static int lm_p0_is_short_form_separator(char value) {
    return value == ';';
}

static int lm_p0_is_quoted_token_boundary(char value) {
    return lm_p0_is_field_space(value) || lm_p0_is_field_separator(value) || value == '(' || value == ')' || value == '#';
}

static int lm_p0_starts_python_string(const char *text, size_t length, size_t index) {
    char quote;
    if (index + 2U >= length) {
        return 0;
    }
    quote = text[index];
    if (quote != '"' && quote != '\'') {
        return 0;
    }
    return text[index + 1U] == quote && text[index + 2U] == quote;
}

static int lm_p0_is_decimal_digit(char value) {
    return value >= '0' && value <= '9';
}

static char * lm_p0_copy_bytes(const char *source, size_t length) {
    return lm_own_copy_bytes(source, length);
}

static LmP0Text * lm_p0_text_view_new_cstr(const char *text) {
    LmP0Text * result;
    result = lm_own_new_zero(1U * sizeof(LmP0Text));
    if (result == 0) {
        return 0;
    }
    if (text != 0) {
        result->data = text;
    }
    if (text == 0) {
        result->data = "";
    }
    result->length = strlen(result -> data);
    return result;
}

static void lm_p0_text_view_delete(LmP0Text *text) {
    lm_own_delete(text, 0);
}

static LmP0Text * lm_p0_text_from_cstr(const char *text) {
    return lm_p0_text_view_new_cstr(text);
}

static int lm_p0_document_init_owners(LmP0Document *document) {
    if (document == 0) {
        return 1;
    }
    if (document -> owners_initialized != 0) {
        return 0;
    }
    document->source_owner = lm_own_new_zero(sizeof(LmOwnArena));
    document->token_arena = lm_own_new_zero(sizeof(LmOwnArena));
    document->tree_arena = lm_own_new_zero(sizeof(LmOwnArena));
    document->diagnostic_arena = lm_own_new_zero(sizeof(LmOwnArena));
    if (document -> source_owner == 0 || document -> token_arena == 0 || document -> tree_arena == 0 || document -> diagnostic_arena == 0) {
        lm_own_delete(document -> diagnostic_arena, 0);
        lm_own_delete(document -> tree_arena, 0);
        lm_own_delete(document -> token_arena, 0);
        lm_own_delete(document -> source_owner, 0);
        document->diagnostic_arena = 0;
        document->tree_arena = 0;
        document->token_arena = 0;
        document->source_owner = 0;
        return 1;
    }
    document->owners_initialized = 1;
    if (lm_own_arena_init(document -> source_owner) != 0 || lm_own_arena_init(document -> token_arena) != 0 || lm_own_arena_init(document -> tree_arena) != 0 || lm_own_arena_init(document -> diagnostic_arena) != 0) {
        lm_p0_document_destroy_owners(document);
        return 1;
    }
    document->diagnostic = lm_own_arena_new_zero(document -> diagnostic_arena, sizeof(document -> diagnostic[0]));
    if (document -> diagnostic == 0) {
        lm_p0_document_destroy_owners(document);
        return 1;
    }
    return 0;
}

static void lm_p0_document_destroy_owners(LmP0Document *document) {
    if (document != 0 && document -> owners_initialized != 0) {
        lm_own_arena_destroy(document -> diagnostic_arena);
        lm_own_delete(document -> diagnostic_arena, 0);
        lm_own_arena_destroy(document -> tree_arena);
        lm_own_delete(document -> tree_arena, 0);
        lm_own_arena_destroy(document -> token_arena);
        lm_own_delete(document -> token_arena, 0);
        lm_own_arena_destroy(document -> source_owner);
        lm_own_delete(document -> source_owner, 0);
        document->diagnostic_arena = 0;
        document->tree_arena = 0;
        document->token_arena = 0;
        document->source_owner = 0;
        document->diagnostic = 0;
        document->owners_initialized = 0;
        document->frozen = 0;
    }
}

static void lm_p0_document_freeze_tree(LmP0Document *document) {
    if (document != 0 && document -> owners_initialized != 0) {
        lm_own_arena_freeze(document -> tree_arena);
        lm_own_arena_freeze(document -> source_owner);
        document->frozen = 1;
    }
}

static void lm_p0_indent_stack_free(LmP0IndentStack *stack) {
    lm_own_delete(stack -> columns, 0);
    stack->columns = 0;
    stack->count = 0U;
    stack->capacity = 0U;
}

static void lm_p0_indent_stack_free_any(void *object) {
    LmP0IndentStack * stack;
    stack = object;
    lm_p0_indent_stack_free(stack);
}

static int lm_p0_indent_stack_push(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column) {
    size_t new_capacity;
    size_t *columns;
    if (stack -> count == stack -> capacity) {
        if (stack -> capacity == 0U) {
            new_capacity = 8U;
        }
        if (stack -> capacity != 0U) {
            new_capacity = stack -> capacity * 2U;
        }
        columns = lm_own_resize(stack -> columns, new_capacity * sizeof(columns[0]));
        if (columns == 0) {
            lm_p0_set_diagnostic(document, 1, line, source_column, "out of memory while storing indentation levels");
            return 0;
        }
        stack->columns = columns;
        stack->capacity = new_capacity;
    }
    stack->columns[stack->count] = column;
    stack->count = stack -> count + 1U;
    return 1;
}

static int lm_p0_indent_stack_init(LmP0Document *document, LmP0IndentStack *stack) {
    memset(stack, 0, sizeof(stack[0]));
    return lm_p0_indent_stack_push(document, stack, 0U, 0U, 0U);
}

static LmP0IndentStack * lm_p0_indent_stack_new_empty(void) {
    return lm_own_new_zero(1U * sizeof(LmP0IndentStack));
}

static LmP0IndentStack * lm_p0_indent_stack_new(LmP0Document *document) {
    LmP0IndentStack * stack;
    stack = lm_p0_indent_stack_new_empty();
    if (stack == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating indentation stack");
        return 0;
    }
    if (lm_p0_indent_stack_init(document, stack) == 0) {
        lm_own_delete(stack, lm_p0_indent_stack_free_any);
        return 0;
    }
    return stack;
}

static void lm_p0_indent_stack_delete(LmP0IndentStack *stack) {
    lm_own_delete(stack, lm_p0_indent_stack_free_any);
}

static int lm_p0_indent_stack_copy(LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column) {
    size_t capacity;
    memset(target, 0, sizeof(target[0]));
    if (source -> capacity > source -> count) {
        capacity = source -> capacity;
    }
    if (source -> capacity <= source -> count) {
        capacity = source -> count;
    }
    if (capacity == 0U) {
        return 1;
    }
    target->columns = lm_own_new_zero(capacity * sizeof(target->columns[0]));
    if (target -> columns == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return 0;
    }
    memcpy(target -> columns, source -> columns, source -> count * sizeof(target->columns[0]));
    target->count = source -> count;
    target->capacity = capacity;
    return 1;
}

static LmP0IndentStack * lm_p0_indent_stack_clone(LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column) {
    LmP0IndentStack * target;
    target = lm_p0_indent_stack_new_empty();
    if (target == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return 0;
    }
    if (lm_p0_indent_stack_copy(document, target, source, line, column) == 0) {
        lm_p0_indent_stack_delete(target);
        return 0;
    }
    return target;
}

static size_t lm_p0_indent_tab_column(size_t column) {
    return ((column / 8U) + 1U) * 8U;
}

static void lm_p0_scan_indent_column(const char *source, size_t start, size_t end, size_t *out_offset, size_t *out_column) {
    size_t p;
    size_t column;
    p = start;
    column = 0U;
    while (p < end && lm_p0_is_horizontal_space(source[p])) {
        if (source[p] == '\t') {
            column = lm_p0_indent_tab_column(column);
        }
        if (source[p] != '\t') {
            column = column + 1U;
        }
        p = p + 1U;
    }
    out_offset[0] = p;
    out_column[0] = column;
}

static size_t lm_p0_visual_column_between(const char *source, size_t start, size_t end) {
    size_t p;
    size_t column;
    p = start;
    column = 0U;
    while (p < end) {
        if (source[p] == '\r' && p + 1U < end && source[p + 1U] == '\n') {
            p = p + 1U;
        }
        if (source[p] == '\r') {
            column = 0U;
            p = p + 1U;
            continue;
        }
        if (source[p] == '\n') {
            column = 0U;
            p = p + 1U;
            continue;
        }
        if (source[p] == '\t') {
            column = lm_p0_indent_tab_column(column);
            p = p + 1U;
            continue;
        }
        column = column + 1U;
        p = p + 1U;
    }
    return column;
}

static const LmTableCell * lm_p0_registry_source_path_cell_at(const char *path, size_t index, const LmTableCell **out_key_cell) {
    int covered;
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    if (path == 0 || lm_p0_registry == 0 || lm_p0_registry->view == 0) {
        return 0;
    }
    covered = 0;
    return lm_registry_view_source_path_cell_at_slice(lm_p0_registry->view, path, strlen(path), index, 0, 0, out_key_cell, &covered);
}

static const LmTableCell * lm_p0_registry_source_path_lookup_cell_slice(const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, const LmTableCell **out_key_cell) {
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_index;
    size_t row_index;
    size_t column_index;
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    if (view == 0 || path == 0 || key == 0) {
        return 0;
    }
    table_index = lm_registry_view_source_table_count(view);
    while (table_index > 0U) {
        table_index = table_index - 1U;
        descriptor = lm_registry_view_source_table_at(view, table_index);
        if (descriptor != 0) {
            row_index = lm_table_descriptor_materialized_row_count(descriptor);
            while (row_index > 0U) {
                row_index = row_index - 1U;
                row = lm_table_descriptor_materialized_row_at(descriptor, row_index);
                key_cell = lm_table_row_cell_at(row, 0U);
                if (key_cell != 0 && key_cell -> value != 0 && lm_table_descriptor_cstr_equals_slice(key_cell -> value, key, key_length) != 0) {
                    column_index = lm_table_descriptor_column_count(descriptor);
                    while (column_index > 1U) {
                        column_index = column_index - 1U;
                        if (lm_table_descriptor_source_path_column_matches(descriptor, column_index, path, path_length) != 0) {
                            payload_cell = lm_table_row_cell_at(row, column_index);
                            if (payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                                if (out_key_cell != 0) {
                                    out_key_cell[0] = key_cell;
                                }
                                return payload_cell;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static const char * lm_p0_registry_lookup(const LmP0Text *key, const char *table) {
    LmP0Text * key_payload;
    const LmTableCell * payload_cell;
    if (table == 0 || lm_p0_registry == 0 || lm_p0_registry->view == 0 || (lm_p0_registry->loaded == 0 && lm_p0_registry->loading == 0)) {
        return 0;
    }
    key_payload = lm_p0_text_from_cstr("");
    if (key_payload == 0) {
        return 0;
    }
    if (lm_p0_identifier_payload(key, key_payload) == 0) {
        lm_p0_text_view_delete(key_payload);
        return 0;
    }
    payload_cell = lm_p0_registry_source_path_lookup_cell_slice(lm_p0_registry->view, table, strlen(table), key_payload -> data, key_payload -> length, 0);
    lm_p0_text_view_delete(key_payload);
    if (payload_cell == 0) {
        return 0;
    }
    return payload_cell -> value;
}

static int lm_p0_registry_source_path_has_rows(const char *table) {
    int covered;
    if (table == 0 || lm_p0_registry == 0 || lm_p0_registry->view == 0) {
        return 0;
    }
    covered = 0;
    return lm_registry_view_source_path_has_rows(lm_p0_registry->view, table, &covered);
}

static int lm_p0_registry_table_has_rows(const char *table) {
    if (lm_p0_registry == 0 || lm_p0_registry->loaded == 0) {
        return 0;
    }
    return lm_p0_registry_source_path_has_rows(table);
}

static int lm_p0_registry_table_has_rows_loaded_or_loading(const char *table) {
    return lm_p0_registry_source_path_has_rows(table);
}

static size_t lm_p0_count_line_breaks(const char *source, size_t start, size_t end) {
    size_t count;
    size_t i;
    size_t width;
    count = 0U;
    i = start;
    while (i < end) {
        width = lm_p0_line_break_width_at(source, end, i);
        if (width != 0U) {
            count = count + 1U;
            i = i + width;
            continue;
        }
        i = i + 1U;
    }
    return count;
}

static void lm_p0_position_in_slice(const char *text, size_t length, size_t index, size_t base_line, size_t base_column, size_t *out_line, size_t *out_column) {
    size_t i;
    size_t line;
    size_t column;
    size_t width;
    line = base_line;
    column = base_column;
    if (index > length) {
        index = length;
    }
    i = 0U;
    while (i < index) {
        width = lm_p0_line_break_width_at(text, index, i);
        if (width != 0U) {
            line = line + 1U;
            column = 1U;
            i = i + width;
            continue;
        }
        column = column + 1U;
        i = i + 1U;
    }
    out_line[0] = line;
    out_column[0] = column;
}

static void lm_p0_advance_layout_line(const char *source, size_t length, size_t line_start, size_t line_end, size_t *offset, size_t *line) {
    size_t next_offset;
    size_t line_breaks;
    size_t width;
    next_offset = line_end;
    line_breaks = lm_p0_count_line_breaks(source, line_start, line_end);
    width = lm_p0_line_break_width_at(source, length, next_offset);
    if (width != 0U) {
        line_breaks = line_breaks + 1U;
        next_offset = next_offset + width;
    }
    offset[0] = next_offset;
    line[0] = line[0] + line_breaks;
}

static int lm_p0_index_is_line_start(const char *text, size_t index) {
    if (index == 0U) {
        return 1;
    }
    return text[index - 1U] == '\n' || text[index - 1U] == '\r';
}

static int lm_p0_line_rest_is_horizontal_space(const char *source, size_t start, size_t end) {
    size_t i;
    i = start;
    while (i < end) {
        if (lm_p0_is_horizontal_space(source[i]) == 0) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static size_t lm_p0_find_physical_line_end(const char *source, size_t length, size_t start) {
    size_t i;
    i = start;
    while (i < length && lm_p0_is_line_break(source[i]) == 0) {
        i = i + 1U;
    }
    return i;
}

static int lm_p0_text_has_prefix_name(const char *text, size_t length, const char *name, int allow_bare) {
    size_t name_length;
    name_length = strlen(name);
    if (length < name_length) {
        return 0;
    }
    if (memcmp(text, name, name_length) != 0) {
        return 0;
    }
    if (length == name_length) {
        return allow_bare;
    }
    if (allow_bare != 0 && (lm_p0_is_horizontal_space(text[name_length]) != 0 || text[name_length] == '#')) {
        return 1;
    }
    return text[name_length] == ':';
}

static LmP0TrailerRole lm_p0_legacy_trailer_role(const char *text, size_t length) {
    if (lm_p0_text_has_prefix_name(text, length, "end", 0) != 0) {
        return LM_P0_TRAILER_ROLE_END;
    }
    if (lm_p0_text_has_prefix_name(text, length, "return", 1) != 0) {
        return LM_P0_TRAILER_ROLE_RETURN;
    }
    if (lm_p0_text_has_prefix_name(text, length, "until", 0) != 0) {
        return LM_P0_TRAILER_ROLE_UNTIL;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static LmP0TrailerRole lm_p0_trailer_role_from_payload(const char *payload) {
    if (payload == 0) {
        return LM_P0_TRAILER_ROLE_NONE;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_END") == 0 || strcmp(payload, "trailer.end") == 0) {
        return LM_P0_TRAILER_ROLE_END;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_RETURN") == 0 || strcmp(payload, "trailer.return") == 0) {
        return LM_P0_TRAILER_ROLE_RETURN;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_UNTIL") == 0 || strcmp(payload, "trailer.until") == 0) {
        return LM_P0_TRAILER_ROLE_UNTIL;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_DASH_CUTTER") == 0 || strcmp(payload, "trailer.dash-cutter") == 0) {
        return LM_P0_TRAILER_ROLE_DASH_CUTTER;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static const char * lm_p0_trailer_role_payload(LmP0TrailerRole role) {
    if (role == LM_P0_TRAILER_ROLE_END) {
        return "trailer.end";
    }
    if (role == LM_P0_TRAILER_ROLE_RETURN) {
        return "trailer.return";
    }
    if (role == LM_P0_TRAILER_ROLE_UNTIL) {
        return "trailer.until";
    }
    if (role == LM_P0_TRAILER_ROLE_DASH_CUTTER) {
        return "trailer.dash-cutter";
    }
    return 0;
}

static int lm_p0_registry_trailer_allows_bare(const char *class_name) {
    const char *payload;
    payload = lm_p0_registry_lookup(lm_p0_text_from_cstr(class_name), "trailer.allow-bare");
    return payload != 0 && (strcmp(payload, "1") == 0 || strcmp(payload, "true") == 0);
}

static int lm_p0_trailer_role_is_tail_cutter(LmP0TrailerRole role) {
    return role != LM_P0_TRAILER_ROLE_NONE;
}

static int lm_p0_node_head_is(const LmP0Node *node, const char *name) {
    size_t name_length;
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    name_length = strlen(name);
    return node -> as -> frame -> head -> length == name_length && memcmp(node -> as -> frame -> head -> data, name, name_length) == 0;
}

static int lm_p0_trailer_role_accepts_target(LmP0TrailerRole role, const LmP0Node *target) {
    const char *role_payload;
    const char *target_head;
    role_payload = lm_p0_trailer_role_payload(role);
    target_head = 0;
    if (role_payload != 0) {
        target_head = lm_p0_registry_lookup(lm_p0_text_from_cstr(role_payload), "trailer.target");
    }
    if (target_head != 0) {
        return lm_p0_node_head_is(target, target_head);
    }
    if (lm_p0_registry_table_has_rows("trailer.target") == 0 && role == LM_P0_TRAILER_ROLE_RETURN) {
        return lm_p0_node_head_is(target, "fn");
    }
    return lm_p0_trailer_role_is_tail_cutter(role);
}

const LmP0Structure * lm_p0_node_structure(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }
    return node -> as -> structure;
}

const LmP0Frame * lm_p0_node_frame(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    return node -> as -> frame;
}

const LmP0Text * lm_p0_node_atom(const LmP0Node *node) {
    if (node == 0 || (node -> kind != LM_P0_NODE_ATOM && node -> kind != LM_P0_NODE_DISABLED)) {
        return 0;
    }
    return node -> as -> atom;
}

const LmP0Trailer * lm_p0_structure_trailer(const LmP0Structure *structure) {
    if (structure == 0) {
        return 0;
    }
    return structure -> trailer;
}

const LmP0Text * lm_p0_frame_head(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> head;
}

const LmP0Structure * lm_p0_frame_body(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> body;
}

const LmP0Trailer * lm_p0_frame_trailer(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> trailer;
}

const LmP0Text * lm_p0_trailer_spelling(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> spelling;
}

const LmP0Structure * lm_p0_trailer_body(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> body;
}

static int lm_p0_stream_event_is_tail_cutter(const LmP0StreamEvent *event) {
    return event -> kind == LM_P0_STREAM_EVENT_ITEM && lm_p0_trailer_role_is_tail_cutter(lm_p0_trailer_role(event -> text, event -> text_length)) != 0;
}

static int lm_p0_find_python_string_end(const char *text, size_t length, size_t start, size_t *out_end) {
    char quote;
    size_t i;
    size_t run_length;
    if (lm_p0_starts_python_string(text, length, start) == 0) {
        out_end[0] = start;
        return 0;
    }
    quote = text[start];
    i = start + 3U;
    while (i < length) {
        if (text[i] == quote) {
            run_length = 1U;
            while (i + run_length < length && text[i + run_length] == quote) {
                run_length = run_length + 1U;
            }
            if (run_length == 3U) {
                out_end[0] = i + 3U;
                return 1;
            }
            i = i + run_length;
            continue;
        }
        i = i + 1U;
    }
    out_end[0] = length;
    return 0;
}

static size_t lm_p0_skip_python_string_unchecked(const char *text, size_t length, size_t start) {
    size_t end;
    if (lm_p0_find_python_string_end(text, length, start, &end) == 0) {
        return length;
    }
    return end;
}

static void lm_p0_scan_layout_prefix(const char *source, size_t length, size_t start, size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level) {
    size_t p;
    size_t indent_column;
    size_t dot_level;
    lm_p0_scan_indent_column(source, start, length, &p, &indent_column);
    dot_level = 0U;
    while (p < length && source[p] == '.') {
        dot_level = dot_level + 1U;
        p = p + 1U;
        while (p < length && lm_p0_is_horizontal_space(source[p]) != 0) {
            p = p + 1U;
        }
    }
    out_offset[0] = p;
    out_indent_column[0] = indent_column;
    out_dot_level[0] = dot_level;
}

static int lm_p0_layout_prefix_is_deeper(size_t indent_column, size_t dot_level, size_t base_indent_column, size_t base_dot_level) {
    if (dot_level > 0U || base_dot_level > 0U) {
        return dot_level > base_dot_level;
    }
    return indent_column > base_indent_column;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status(const char *text, size_t length, size_t *out_dash_count) {
    size_t dash_count;
    size_t i;
    int closed;
    size_t brace_end;
    dash_count = 0U;
    while (dash_count < length && text[dash_count] == '-') {
        dash_count = dash_count + 1U;
    }
    if (out_dash_count != 0) {
        out_dash_count[0] = dash_count;
    }
    if (dash_count < 3U) {
        return LM_P0_DASH_FENCE_NONE;
    }
    if (dash_count > LM_P0_MAX_FENCE_LENGTH) {
        return LM_P0_DASH_FENCE_TOO_LONG;
    }
    i = dash_count;
    while (i < length) {
        if (lm_p0_is_horizontal_space(text[i]) != 0) {
            i = i + 1U;
            continue;
        }
        if (text[i] == '#') {
            return LM_P0_DASH_FENCE_VALID;
        }
        if (text[i] == '{') {
            brace_end = lm_p0_scan_brace_mark_unchecked(text, length, i, &closed);
            if (closed != 0) {
                i = brace_end;
                continue;
            }
        }
        return LM_P0_DASH_FENCE_TRAILING_TEXT;
    }
    return LM_P0_DASH_FENCE_VALID;
}

static int lm_p0_validate_dash_fence_line(LmP0Document *document, const char *text, size_t length, size_t line, size_t column) {
    LmP0DashFenceStatus status;
    status = lm_p0_dash_fence_status_after_comment_trim(text, length, 0);
    if (status == LM_P0_DASH_FENCE_TOO_LONG) {
        lm_p0_set_diagnostic(document, 25, line, column, "dash delimiter fence length exceeds 80 characters");
        return 0;
    }
    if (status == LM_P0_DASH_FENCE_TRAILING_TEXT) {
        lm_p0_set_diagnostic(document, 26, line, column, "dash delimiter line must contain only the dash fence, whitespace, or a line comment");
        return 0;
    }
    return 1;
}

static int lm_p0_match_block_string_fence_line(const char *source, size_t line_start, size_t line_end, size_t eq_count) {
    size_t i;
    if (eq_count < 3U || eq_count > LM_P0_MAX_FENCE_LENGTH || line_start + eq_count > line_end) {
        return 0;
    }
    i = 0U;
    while (i < eq_count) {
        if (source[line_start + i] != '=') {
            return 0;
        }
        i = i + 1U;
    }
    if (line_start + eq_count < line_end && source[line_start + eq_count] == '=') {
        return 0;
    }
    i = line_start + eq_count;
    while (i < line_end) {
        if (lm_p0_is_horizontal_space(source[i]) == 0) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_match_raw_comment_fence_line(const char *source, size_t line_start, size_t line_end, size_t star_count) {
    size_t i;
    if (star_count < 3U || star_count > LM_P0_MAX_FENCE_LENGTH || line_start + star_count > line_end) {
        return 0;
    }
    i = 0U;
    while (i < star_count) {
        if (source[line_start + i] != '*') {
            return 0;
        }
        i = i + 1U;
    }
    if (line_start + star_count < line_end && source[line_start + star_count] == '*') {
        return 0;
    }
    return lm_p0_line_rest_is_horizontal_space(source, line_start + star_count, line_end);
}

static void lm_p0_dump_append_size(LmP0Dump *dump, size_t value) {
    char buffer[32];
    size_t index;
    if (value == 0U) {
        lm_p0_dump_append_cstr(dump, "0");
        return;
    }
    index = sizeof(buffer);
    while (value > 0U) {
        index = index - 1U;
        buffer[index] = '0' + value % 10U;
        value = value / 10U;
    }
    lm_p0_dump_append(dump, buffer + index, sizeof(buffer) - index);
}

static void lm_p0_dump_append_field_count_line(LmP0Dump *dump, size_t field_count) {
    lm_p0_dump_append_cstr(dump, " fields=");
    lm_p0_dump_append_size(dump, field_count);
    lm_p0_dump_append_cstr(dump, "\n");
}

static void lm_p0_registry_private_api_anchor(void) {
    LM_UNUSED(&lm_registry_view_class_at);
    LM_UNUSED(&lm_registry_view_source_path_key_at_slice);
    LM_UNUSED(&lm_registry_view_append_materialized_rows);
    LM_UNUSED(&lm_registry_view_fact_at);
    LM_UNUSED(&lm_registry_view_push_relation);
    LM_UNUSED(&lm_registry_view_lookup_exact);
    LM_UNUSED(&lm_registry_view_lookup_default);
    LM_UNUSED(&lm_registry_view_lookup);
    LM_UNUSED(&lm_registry_view_lookup_text);
    LM_UNUSED(&lm_registry_view_table_row_at);
    LM_UNUSED(&lm_registry_view_matching_key_count);
    LM_UNUSED(&lm_registry_view_matching_key_at);
    LM_UNUSED(&lm_registry_view_local_source_rows_slice);
    LM_UNUSED(&lm_registry_view_table_has_rows);
    LM_UNUSED(&lm_p0_registry_source_tables_selftest);
}

static int lm_p0_registry_init(void) {
    lm_p0_registry_private_api_anchor();
    if (lm_p0_registry != 0) {
        return 0;
    }
    lm_p0_registry = (((LmP0Registry *)lm_own_new_zero(sizeof(lm_p0_registry[0]))));
    if (lm_p0_registry == 0) {
        return 1;
    }
    lm_p0_registry->view = lm_registry_view_new(0);
    if (lm_p0_registry -> view == 0) {
        lm_registry_view_delete(lm_p0_registry -> view);
        lm_own_delete(lm_p0_registry, 0);
        lm_p0_registry = 0;
        return 1;
    }
    return 0;
}

static void lm_p0_registry_destroy(void) {
    if (lm_p0_registry == 0) {
        return;
    }
    lm_registry_view_delete(lm_p0_registry -> view);
    lm_p0_registry->view = 0;
    lm_p0_registry->loaded = 0;
    lm_p0_registry->loading = 0;
    lm_own_delete(lm_p0_registry, 0);
    lm_p0_registry = 0;
}

static LmP0Text * lm_p0_text_ref_new_empty(void) {
    return (((LmP0Text *)lm_own_new_zero(sizeof(LmP0Text))));
}

static void lm_p0_text_ref_delete(LmP0Text *text) {
    lm_own_delete(text, 0);
}

static int lm_p0_document_register_lazy_text(LmP0Document *document, const char *source, size_t length, const char **patch_slot, size_t line, size_t column) {
    if (patch_slot == 0) {
        return 0;
    }
    if (length == 0U) {
        patch_slot[0] = "";
        return 1;
    }
    if (((document == 0) || (document -> owners_initialized == 0)) || (lm_own_arena_add_lazy_edge(document -> tree_arena, document -> source_owner, source, length, (((const void **)patch_slot))) != 0)) {
        if (document != 0) {
            lm_p0_set_diagnostic(document, 1, line, column, "out of memory while registering parser lazy text edge");
        }
        return 0;
    }
    return 1;
}

static LmP0Text * lm_p0_new_text(LmP0Document *document, const char *source, size_t length, size_t line, size_t column) {
    LmP0Text * text;
    text = (((LmP0Text *)lm_own_arena_new_zero(document -> tree_arena, sizeof(text[0]))));
    if (text == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser text");
        return 0;
    }
    if (source != 0) {
        text->data = source;
    }
    else {
        text->data = "";
    }
    if (source != 0) {
        text->length = length;
    }
    else {
        text->length = 0U;
    }
    if (lm_p0_document_register_lazy_text(document, text -> data, text -> length, &text -> data, line, column) == 0) {
        return 0;
    }
    return text;
}

static LmP0Structure * lm_p0_new_structure(LmP0Document *document, size_t line, size_t column) {
    LmP0Structure * structure;
    structure = (((LmP0Structure *)lm_own_arena_new_zero(document -> tree_arena, sizeof(structure[0]))));
    if (structure == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser structure");
        return 0;
    }
    return structure;
}

static LmP0Frame * lm_p0_new_frame(LmP0Document *document, size_t line, size_t column) {
    LmP0Frame * frame;
    frame = (((LmP0Frame *)lm_own_arena_new_zero(document -> tree_arena, sizeof(frame[0]))));
    if (frame == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser frame");
        return 0;
    }
    frame->head = lm_p0_new_text(document, "", 0U, line, column);
    if (frame -> head == 0) {
        return 0;
    }
    frame->body = lm_p0_new_structure(document, line, column);
    if (frame -> body == 0) {
        return 0;
    }
    return frame;
}

static LmP0Trailer * lm_p0_new_trailer(LmP0Document *document, const char *spelling, size_t spelling_length, size_t line, size_t column) {
    LmP0Trailer * trailer;
    trailer = (((LmP0Trailer *)lm_own_arena_new_zero(document -> tree_arena, sizeof(trailer[0]))));
    if (trailer == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser trailer");
        return 0;
    }
    trailer->spelling = lm_p0_new_text(document, spelling, spelling_length, line, column);
    if (trailer -> spelling == 0) {
        return 0;
    }
    trailer->body = lm_p0_new_structure(document, line, column);
    if (trailer -> body == 0) {
        return 0;
    }
    return trailer;
}

static void lm_p0_set_diagnostic(LmP0Document *document, int code, size_t line, size_t column, const char *message) {
    if (document == 0 || document -> diagnostic == 0 || document -> diagnostic -> code != 0) {
        return;
    }
    if (message == 0) {
        message = "";
    }
    document->diagnostic->code = code;
    document->diagnostic->line = line;
    document->diagnostic->column = column;
    snprintf(document -> diagnostic -> message, sizeof(document -> diagnostic -> message), "%s", message);
}

static int lm_p0_registry_column_has_descriptor(const LmRegistrySourceColumn *column, const char *descriptor) {
    size_t i;
    LmP0Text * payload;
    int result;
    if ((column == 0) || (descriptor == 0)) {
        return 0;
    }
    payload = lm_p0_text_ref_new_empty();
    if (payload == 0) {
        return 0;
    }
    result = 0;
    i = 0U;
    while (i < column -> descriptor_count) {
        if (((column -> descriptors[i] != 0) && lm_p0_registry_identifier_value(column -> descriptors[i], payload)) && lm_p0_text_equals(payload, descriptor)) {
            result = 1;
            break;
        }
        i = i + 1U;
    }
    lm_p0_text_ref_delete(payload);
    return result;
}

static int lm_p0_registry_cell_none_cell_matches(const LmP0Text *payload, const LmP0Text *class_atom) {
    LmP0Text * class_name;
    const char *none_value;
    int result;
    if (payload == 0 || class_atom == 0) {
        return 0;
    }
    class_name = lm_p0_text_ref_new_empty();
    if (class_name == 0) {
        return 0;
    }
    result = 0;
    if (lm_p0_registry_identifier_value(class_atom, class_name) != 0) {
        none_value = lm_p0_registry_lookup(class_name, "None.cell");
        if (none_value != 0 && lm_p0_text_equals(payload, none_value)) {
            result = 1;
        }
    }
    lm_p0_text_ref_delete(class_name);
    return result;
}

static int lm_p0_registry_cell_is_null(const LmP0Text *atom, const LmRegistrySourceColumn *column) {
    LmP0Text * payload;
    size_t i;
    int result;
    if (lm_p0_registry_payload_is_null(atom)) {
        return 1;
    }
    if (atom == 0 || column == 0) {
        return 0;
    }
    payload = lm_p0_text_ref_new_empty();
    if (payload == 0) {
        return 0;
    }
    if (lm_p0_registry_identifier_value(atom, payload) == 0) {
        lm_p0_text_ref_delete(payload);
        return 0;
    }
    result = 0;
    if (column -> name != 0 && lm_p0_registry_cell_none_cell_matches(payload, column -> name)) {
        result = 1;
    }
    i = 0U;
    while (result == 0 && i < column -> descriptor_count) {
        if (column -> descriptors[i] != 0 && lm_p0_registry_cell_none_cell_matches(payload, column -> descriptors[i])) {
            result = 1;
        }
        i = i + 1U;
    }
    lm_p0_text_ref_delete(payload);
    return result;
}

static int lm_p0_registry_cell_value(const LmP0Text *atom, const LmRegistrySourceColumn *column, LmP0Text *out_value) {
    if (lm_p0_registry_cell_is_null(atom, column)) {
        return 0;
    }
    if (lm_p0_registry_column_has_descriptor(column, "char")) {
        if (lm_p0_registry_literal_value(atom, out_value)) {
            return 1;
        }
        return -1;
    }
    if (lm_p0_registry_identifier_value(atom, out_value)) {
        return 1;
    }
    return -1;
}

static LmTableDescriptor * lm_p0_registry_source_descriptor_new(const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    LmP0Text * table_value;
    LmP0Text * column_value;
    LmP0Text * descriptor_value;
    LmP0Text * type_value;
    LmTableDescriptor * descriptor;
    const char *descriptor_data[16];
    size_t descriptor_lengths[16];
    const char *type_data;
    size_t type_length;
    size_t column_index;
    size_t descriptor_index;
    if (table_name == 0 || columns == 0 || column_count == 0U) {
        return 0;
    }
    table_value = lm_p0_text_ref_new_empty();
    column_value = lm_p0_text_ref_new_empty();
    descriptor_value = lm_p0_text_ref_new_empty();
    type_value = lm_p0_text_ref_new_empty();
    descriptor = 0;
    if (table_value == 0 || column_value == 0 || descriptor_value == 0 || type_value == 0 || lm_p0_registry_identifier_value(table_name, table_value) == 0) {
        lm_p0_text_ref_delete(table_value);
        lm_p0_text_ref_delete(column_value);
        lm_p0_text_ref_delete(descriptor_value);
        lm_p0_text_ref_delete(type_value);
        return 0;
    }
    descriptor = lm_table_descriptor_new_empty_slice(table_value -> data, table_value -> length);
    column_index = 0U;
    while (descriptor != 0 && column_index < column_count) {
        if (columns[column_index] == 0 || columns[column_index] -> name == 0 || lm_p0_registry_identifier_value(columns[column_index] -> name, column_value) == 0) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        type_data = 0;
        type_length = 0U;
        if (columns[column_index] -> type_name != 0) {
            if (lm_p0_registry_identifier_value(columns[column_index] -> type_name, type_value) == 0) {
                lm_table_descriptor_delete_any(descriptor);
                descriptor = 0;
                break;
            }
            type_data = type_value -> data;
            type_length = type_value -> length;
        }
        if (columns[column_index] -> descriptor_count > sizeof(descriptor_data) / sizeof(descriptor_data[0])) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        descriptor_index = 0U;
        while (descriptor_index < columns[column_index] -> descriptor_count) {
            if (columns[column_index] -> descriptors[descriptor_index] == 0 || lm_p0_registry_identifier_value(columns[column_index] -> descriptors[descriptor_index], descriptor_value) == 0) {
                lm_table_descriptor_delete_any(descriptor);
                descriptor = 0;
                break;
            }
            descriptor_data[descriptor_index] = descriptor_value -> data;
            descriptor_lengths[descriptor_index] = descriptor_value -> length;
            descriptor_index = descriptor_index + 1U;
        }
        if (descriptor == 0) {
            break;
        }
        if (lm_table_descriptor_add_column_slices(descriptor, column_value -> data, column_value -> length, type_data, type_length, (((const char **)descriptor_data)), descriptor_lengths, columns[column_index] -> descriptor_count, columns[column_index] -> address_depth, columns[column_index] -> array_rank, columns[column_index] -> is_const) != 0) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        column_index = column_index + 1U;
    }
    lm_p0_text_ref_delete(table_value);
    lm_p0_text_ref_delete(column_value);
    lm_p0_text_ref_delete(descriptor_value);
    lm_p0_text_ref_delete(type_value);
    return descriptor;
}

static int lm_p0_registry_materialize_source_row(const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells) {
    LmP0Text * table_value;
    LmP0Text * decoded;
    LmTableDescriptor * descriptor;
    LmTableRow * row;
    const LmP0Node * node;
    const LmP0Text * atom;
    size_t column_index;
    int cell_status;
    int status;
    if (table_name == 0 || columns == 0 || column_count == 0U || cells == 0 || lm_p0_registry == 0 || lm_p0_registry -> view == 0) {
        return -1;
    }
    table_value = lm_p0_text_ref_new_empty();
    decoded = lm_p0_text_ref_new_empty();
    if (table_value == 0 || decoded == 0 || lm_p0_registry_identifier_value(table_name, table_value) == 0) {
        lm_p0_text_ref_delete(table_value);
        lm_p0_text_ref_delete(decoded);
        return -1;
    }
    descriptor = lm_registry_view_find_local_source_table_slice(lm_p0_registry -> view, table_value -> data, table_value -> length);
    if (descriptor == 0 || lm_table_descriptor_column_count(descriptor) != column_count) {
        lm_p0_text_ref_delete(table_value);
        lm_p0_text_ref_delete(decoded);
        return -1;
    }
    row = lm_table_row_new(0);
    status = 0;
    column_index = 0U;
    while (status == 0 && column_index < column_count) {
        node = cells[column_index];
        if (columns[column_index] == 0 || node == 0 || node -> kind != LM_P0_NODE_ATOM) {
            status = -1;
        }
        else {
            atom = node -> as -> atom;
            if (atom == 0) {
                status = -1;
            }
            else {
                if (column_index == 0U) {
                    cell_status = lm_p0_registry_identifier_value(atom, decoded);
                }
                else {
                    cell_status = lm_p0_registry_cell_value(atom, columns[column_index], decoded);
                }
                if (cell_status < 0 || (column_index == 0U && cell_status == 0)) {
                    status = -1;
                }
                else {
                    if (cell_status == 0) {
                        if (lm_table_row_take_cell_slice(row, atom -> data, atom -> length, 0, 0U, 0, 0, 1) != 0) {
                            status = -1;
                        }
                    }
                    else {
                        if (lm_table_row_take_cell_slice(row, atom -> data, atom -> length, decoded -> data, decoded -> length, 0, 0, 0) != 0) {
                            status = -1;
                        }
                    }
                }
            }
        }
        column_index = column_index + 1U;
    }
    lm_p0_text_ref_delete(table_value);
    lm_p0_text_ref_delete(decoded);
    if (status != 0 || row == 0) {
        lm_table_row_delete_any(row);
        return -1;
    }
    if (lm_table_descriptor_take_materialized_row(descriptor, row) != 0) {
        lm_table_row_delete_any(row);
        return -1;
    }
    lm_registry_view_note_mutation(lm_p0_registry -> view);
    return 0;
}

static int lm_p0_registry_compare_enabled(void) {
    const char *value;
    if (LM_P0_ENABLE_REGISTRY_COMPARE == 0) {
        return 0;
    }
    value = getenv("LM_P0_COMPARE_REGISTRY");
    return value == 0 || strcmp(value, "0") != 0;
}

static void lm_p0_registry_compare_fail(const char *table, const char *key, const char *registry_payload, const char *legacy_payload) {
    const char *lm_p0_tmp_1;
    if (table != 0) {
        lm_p0_tmp_1 = table;
    }
    else {
        lm_p0_tmp_1 = "<none>";
    }
    const char *lm_p0_tmp_2;
    if (key != 0) {
        lm_p0_tmp_2 = key;
    }
    else {
        lm_p0_tmp_2 = "<none>";
    }
    const char *lm_p0_tmp_3;
    if (registry_payload != 0) {
        lm_p0_tmp_3 = registry_payload;
    }
    else {
        lm_p0_tmp_3 = "<none>";
    }
    const char *lm_p0_tmp_4;
    if (legacy_payload != 0) {
        lm_p0_tmp_4 = legacy_payload;
    }
    else {
        lm_p0_tmp_4 = "<none>";
    }
    fprintf(stderr, "parser registry mismatch: table=%s key=\"%s\" registry=%s legacy=%s\n", lm_p0_tmp_1, lm_p0_tmp_2, lm_p0_tmp_3, lm_p0_tmp_4);
    exit(2);
}

static void lm_p0_trim_right(const char **text, size_t *length) {
    while ((length[0] > 0U) && (lm_p0_is_horizontal_space(text[0][(length[0] - 1U)]) || (text[0][(length[0] - 1U)] == '\r'))) {
        length[0] = length[0] - 1U;
    }
}

static void lm_p0_trim_trailing_line_comment(const char **text, size_t *length) {
    size_t i;
    i = 0U;
    while (i < length[0]) {
        size_t prefixed_end;
        if (lm_p0_starts_python_string(text[0], length[0], i)) {
            i = lm_p0_skip_python_string_unchecked(text[0], length[0], i);
            continue;
        }
        prefixed_end = lm_p0_scan_c_prefixed_quote_token(text[0], length[0], i);
        if (prefixed_end > i) {
            i = prefixed_end;
            continue;
        }
        if ((text[0][i] == '\'') && (lm_p0_starts_python_string(text[0], length[0], i) == 0)) {
            i = lm_p0_scan_c_char_token(text[0], length[0], i);
            continue;
        }
        if ((text[0][i] == '"') || (text[0][i] == '`')) {
            char quote;
            char lm_p0_tmp_5;
            lm_p0_tmp_5 = text[0][i];
            i = i + 1U;
            quote = lm_p0_tmp_5;
            while (i < length[0]) {
                if ((quote == '"') && (text[0][i] == '\\')) {
                    if ((i + 1U) < length[0]) {
                        i = i + 2U;
                    }
                    else {
                        i = i + 1U;
                    }
                    continue;
                }
                if ((((quote == '`') && (text[0][i] == '`')) && ((i + 1U) < length[0])) && (text[0][(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                char lm_p0_tmp_6;
                lm_p0_tmp_6 = text[0][i];
                i = i + 1U;
                if (lm_p0_tmp_6 == quote) {
                    break;
                }
            }
            continue;
        }
        if (text[0][i] == '#') {
            size_t end_index;
            end_index = (i + 1U);
            while ((end_index < length[0]) && (lm_p0_is_line_break(text[0][end_index]) == 0)) {
                end_index = end_index + 1U;
            }
            if (end_index == length[0]) {
                length[0] = i;
                lm_p0_trim_right(text, length);
            }
            return;
        }
        i = i + 1U;
    }
}

static int lm_p0_indent_level_from_column(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level) {
    size_t top;
    top = stack -> columns[(stack -> count - 1U)];
    if (column == top) {
        out_level[0] = (stack -> count - 1U);
        return 1;
    }
    if (column > top) {
        if (lm_p0_indent_stack_push(document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = (stack -> count - 1U);
        return 1;
    }
    while ((stack -> count > 0U) && (stack -> columns[(stack -> count - 1U)] > column)) {
        stack->count = stack -> count - 1U;
    }
    if ((stack -> count == 0U) || (stack -> columns[(stack -> count - 1U)] != column)) {
        lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
        return 0;
    }
    out_level[0] = (stack -> count - 1U);
    return 1;
}

static size_t lm_p0_skip_fence_block_unchecked(const char *text, size_t length, size_t start, char fence_char) {
    size_t fence_count;
    size_t line_end;
    size_t scan_start;
    line_end = lm_p0_find_physical_line_end(text, length, start);
    fence_count = 0U;
    while (((start + fence_count) < line_end) && (text[(start + fence_count)] == fence_char)) {
        fence_count = fence_count + 1U;
    }
    if ((fence_count < 3U) || (fence_count > LM_P0_MAX_FENCE_LENGTH)) {
        return start;
    }
    if (fence_char == '=') {
        if (lm_p0_match_block_string_fence_line(text, start, line_end, fence_count) == 0) {
            return start;
        }
    }
    else {
        if (fence_char == '*') {
            if (lm_p0_match_raw_comment_fence_line(text, start, line_end, fence_count) == 0) {
                return start;
            }
        }
        else {
            return start;
        }
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(text, length, scan_start);
    }
    while (scan_start <= length) {
        size_t current_end;
        current_end = lm_p0_find_physical_line_end(text, length, scan_start);
        if ((fence_char == '=') && lm_p0_match_block_string_fence_line(text, scan_start, current_end, fence_count)) {
            size_t lm_p0_tmp_7;
            if (current_end < length) {
                lm_p0_tmp_7 = lm_p0_line_break_width_at(text, length, current_end);
            }
            else {
                lm_p0_tmp_7 = 0U;
            }
            return (current_end + lm_p0_tmp_7);
        }
        if ((fence_char == '*') && lm_p0_match_raw_comment_fence_line(text, scan_start, current_end, fence_count)) {
            size_t lm_p0_tmp_8;
            if (current_end < length) {
                lm_p0_tmp_8 = lm_p0_line_break_width_at(text, length, current_end);
            }
            else {
                lm_p0_tmp_8 = 0U;
            }
            return (current_end + lm_p0_tmp_8);
        }
        if (current_end == length) {
            break;
        }
        scan_start = (current_end + lm_p0_line_break_width_at(text, length, current_end));
    }
    return length;
}

static size_t lm_p0_find_layout_line_end(const char *source, size_t length, size_t start) {
    size_t i;
    size_t depth;
    size_t current_line_indent;
    size_t current_line_dot_level;
    size_t ignored_content_offset;
    size_t delimiter_indent_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    size_t delimiter_dot_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    char delimiter_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    char quote;
    i = start;
    depth = 0U;
    quote = '\0';
    lm_p0_scan_layout_prefix(source, length, start, &ignored_content_offset, &current_line_indent, &current_line_dot_level);
    while (i < length) {
        if (quote != '\0') {
            if ((quote == '"') && (source[i] == '\\')) {
                size_t line_break_width;
                if ((i + 1U) < length) {
                    line_break_width = lm_p0_line_break_width_at(source, length, (i + 1U));
                }
                else {
                    line_break_width = 0U;
                }
                if (line_break_width > 0U) {
                    i = i + (1U + line_break_width);
                }
                else {
                    if ((i + 1U) < length) {
                        i = i + 2U;
                    }
                    else {
                        i = i + 1U;
                    }
                }
                continue;
            }
            if ((((quote == '`') && (source[i] == '`')) && ((i + 1U) < length)) && (source[(i + 1U)] == '`')) {
                i = i + 2U;
                continue;
            }
            if (source[i] == quote) {
                quote = '\0';
            }
            i = i + 1U;
            continue;
        }
        if (lm_p0_starts_python_string(source, length, i)) {
            i = lm_p0_skip_python_string_unchecked(source, length, i);
            continue;
        }
        if (lm_p0_scan_c_prefixed_quote_token(source, length, i) > i) {
            i = lm_p0_scan_c_prefixed_quote_token(source, length, i);
            continue;
        }
        if ((source[i] == '\'') && (lm_p0_starts_python_string(source, length, i) == 0)) {
            i = lm_p0_scan_c_char_token(source, length, i);
            continue;
        }
        if ((source[i] == '"') || (source[i] == '`')) {
            char lm_p0_tmp_9;
            lm_p0_tmp_9 = source[i];
            i = i + 1U;
            quote = lm_p0_tmp_9;
            continue;
        }
        if (source[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(source[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (source[i] == '{') {
            int closed;
            size_t brace_end;
            brace_end = lm_p0_scan_brace_mark_unchecked(source, length, i, &closed);
            if (closed) {
                i = brace_end;
                continue;
            }
        }
        if (lm_p0_is_line_break(source[i])) {
            size_t line_break_width;
            size_t next_line_start;
            size_t next_content_offset;
            size_t next_line_indent;
            size_t next_line_dot_level;
            size_t base_indent;
            size_t base_dot_level;
            int next_line_starts_with_matching_close;
            if (depth == 0U) {
                break;
            }
            line_break_width = lm_p0_line_break_width_at(source, length, i);
            next_line_start = (i + line_break_width);
            lm_p0_scan_layout_prefix(source, length, next_line_start, &next_content_offset, &next_line_indent, &next_line_dot_level);
            next_line_starts_with_matching_close = 0;
            if ((depth <= LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) && (next_content_offset < length)) {
                char top_delimiter;
                top_delimiter = delimiter_stack[(depth - 1U)];
                next_line_starts_with_matching_close = (((top_delimiter == '(') && (source[next_content_offset] == ')')) || ((top_delimiter == '[') && (source[next_content_offset] == ']')));
            }
            if (depth <= LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) {
                base_indent = delimiter_indent_stack[(depth - 1U)];
                base_dot_level = delimiter_dot_stack[(depth - 1U)];
                if ((next_line_starts_with_matching_close == 0) && (lm_p0_layout_prefix_is_deeper(next_line_indent, next_line_dot_level, base_indent, base_dot_level) == 0)) {
                    break;
                }
            }
            current_line_indent = next_line_indent;
            current_line_dot_level = next_line_dot_level;
            i = next_line_start;
            continue;
        }
        if ((source[i] == '(') || (source[i] == '[')) {
            if (depth < LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) {
                delimiter_indent_stack[depth] = current_line_indent;
                delimiter_dot_stack[depth] = current_line_dot_level;
                delimiter_stack[depth] = source[i];
            }
            depth = depth + 1U;
        }
        else {
            if (((source[i] == ')') || (source[i] == ']')) && (depth > 0U)) {
                depth = depth - 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static size_t lm_p0_scan_brace_mark_unchecked(const char *text, size_t length, size_t start, int *closed) {
    size_t i;
    size_t depth;
    i = start;
    depth = 0U;
    closed[0] = 0;
    while (i < length) {
        size_t prefixed_end;
        if (lm_p0_index_is_line_start(text, i) && ((text[i] == '=') || (text[i] == '*'))) {
            size_t fence_end;
            fence_end = lm_p0_skip_fence_block_unchecked(text, length, i, text[i]);
            if (fence_end > i) {
                i = fence_end;
                continue;
            }
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            i = lm_p0_skip_python_string_unchecked(text, length, i);
            continue;
        }
        prefixed_end = lm_p0_scan_c_prefixed_quote_token(text, length, i);
        if (prefixed_end > i) {
            i = prefixed_end;
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(text, length, i) == 0)) {
            i = lm_p0_scan_c_char_token(text, length, i);
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            char quote;
            char lm_p0_tmp_10;
            lm_p0_tmp_10 = text[i];
            i = i + 1U;
            quote = lm_p0_tmp_10;
            while (i < length) {
                if ((quote == '"') && (text[i] == '\\')) {
                    size_t line_break_width;
                    if ((i + 1U) < length) {
                        line_break_width = lm_p0_line_break_width_at(text, length, (i + 1U));
                    }
                    else {
                        line_break_width = 0U;
                    }
                    if (line_break_width > 0U) {
                        i = i + (1U + line_break_width);
                    }
                    else {
                        if ((i + 1U) < length) {
                            i = i + 2U;
                        }
                        else {
                            i = i + 1U;
                        }
                    }
                    continue;
                }
                if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                char lm_p0_tmp_11;
                lm_p0_tmp_11 = text[i];
                i = i + 1U;
                if (lm_p0_tmp_11 == quote) {
                    break;
                }
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (text[i] == '{') {
            depth = depth + 1U;
            i = i + 1U;
            continue;
        }
        if (text[i] == '}') {
            i = i + 1U;
            if (depth > 0U) {
                depth = depth - 1U;
                if (depth == 0U) {
                    closed[0] = 1;
                    return i;
                }
            }
            continue;
        }
        i = i + 1U;
    }
    return i;
}

static int lm_p0_skip_brace_mark_ex(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, int record) {
    size_t start;
    size_t end_index;
    int closed;
    if ((index[0] >= length) || (text[index[0]] != '{')) {
        return 1;
    }
    start = index[0];
    end_index = lm_p0_scan_brace_mark_unchecked(text, length, start, &closed);
    index[0] = end_index;
    if (closed == 0) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(text, length, start, line, column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 31, diagnostic_line, diagnostic_column, "unterminated brace mark");
        return 0;
    }
    if (record && (lm_p0_record_mix_mark(document, structure, text, length, line, column, start, end_index) == 0)) {
        return 0;
    }
    return 1;
}

static int lm_p0_skip_brace_mark(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t column) {
    return lm_p0_skip_brace_mark_ex(document, 0, text, length, index, line, column, 0);
}

static int lm_p0_consume_brace_mark(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column) {
    return lm_p0_skip_brace_mark_ex(document, structure, text, length, index, line, column, 1);
}

static int lm_p0_skip_leading_brace_marks_ex(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index, int record) {
    while (index[0] < length) {
        while ((index[0] < length) && lm_p0_is_horizontal_space(text[index[0]])) {
            index[0] = index[0] + 1U;
        }
        if ((index[0] >= length) || (text[index[0]] != '{')) {
            return 1;
        }
        if (lm_p0_skip_brace_mark_ex(document, 0, text, length, index, line, column, record) == 0) {
            return 0;
        }
    }
    return 1;
}

static int lm_p0_skip_leading_brace_marks(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index) {
    return lm_p0_skip_leading_brace_marks_ex(document, text, length, line, column, index, 0);
}

static int lm_p0_line_is_standalone_mix_run(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, int *out_is_mix) {
    size_t i;
    int saw_mix;
    i = 0U;
    saw_mix = 0;
    out_is_mix[0] = 0;
    while (i < length) {
        while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
            i = i + 1U;
        }
        if (i >= length) {
            break;
        }
        if (text[i] == '#') {
            break;
        }
        if (text[i] != '{') {
            return 1;
        }
        if (lm_p0_skip_brace_mark(document, text, length, &i, line, column) == 0) {
            return 0;
        }
        saw_mix = 1;
    }
    out_is_mix[0] = saw_mix;
    return 1;
}

static int lm_p0_scan_leading_mix_prefix(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *out_prefix_start, size_t *out_prefix_end, size_t *out_anchor) {
    size_t i;
    size_t prefix_start;
    size_t prefix_end;
    int saw_mix;
    i = 0U;
    while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
        i = i + 1U;
    }
    if ((i >= length) || (text[i] != '{')) {
        return 1;
    }
    prefix_start = i;
    prefix_end = i;
    saw_mix = 0;
    while (i < length) {
        while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
            i = i + 1U;
        }
        if ((i >= length) || (text[i] != '{')) {
            break;
        }
        if (lm_p0_skip_brace_mark(document, text, length, &i, line, column) == 0) {
            return 0;
        }
        prefix_end = i;
        saw_mix = 1;
    }
    while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
        i = i + 1U;
    }
    if (((saw_mix == 0) || (i >= length)) || (text[i] == '#')) {
        return 1;
    }
    out_prefix_start[0] = prefix_start;
    out_prefix_end[0] = prefix_end;
    out_anchor[0] = i;
    return 1;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status_after_comment_trim(const char *text, size_t length, size_t *out_dash_count) {
    const char *trimmed_text;
    size_t trimmed_length;
    trimmed_text = text;
    trimmed_length = length;
    lm_p0_trim_trailing_line_comment(&trimmed_text, &trimmed_length);
    return lm_p0_dash_fence_status(trimmed_text, trimmed_length, out_dash_count);
}

static int lm_p0_scan_raw_comment_block(LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, size_t *next_offset, size_t *next_line) {
    size_t line_end;
    size_t star_count;
    size_t scan_start;
    size_t scan_line;
    if ((line_start >= length) || (source[line_start] != '*')) {
        return 0;
    }
    line_end = lm_p0_find_physical_line_end(source, length, line_start);
    star_count = 0U;
    while (((line_start + star_count) < line_end) && (source[(line_start + star_count)] == '*')) {
        star_count = star_count + 1U;
    }
    if (star_count < 3U) {
        return 0;
    }
    if (star_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "raw comment fence length exceeds 80 characters");
        return 0;
    }
    if (lm_p0_match_raw_comment_fence_line(source, line_start, line_end, star_count) == 0) {
        lm_p0_set_diagnostic(document, 27, line, 1U, "raw comment fence line must contain only the star fence and whitespace");
        return 0;
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(source, length, scan_start);
    }
    scan_line = (line + 1U);
    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;
        current_end = lm_p0_find_physical_line_end(source, length, scan_start);
        if (lm_p0_match_raw_comment_fence_line(source, scan_start, current_end, star_count)) {
            if (current_end < length) {
                break_width = lm_p0_line_break_width_at(source, length, current_end);
            }
            else {
                break_width = 0U;
            }
            next_offset[0] = (current_end + break_width);
            size_t lm_p0_tmp_12;
            if (break_width > 0U) {
                lm_p0_tmp_12 = 1U;
            }
            else {
                lm_p0_tmp_12 = 0U;
            }
            next_line[0] = (scan_line + lm_p0_tmp_12);
            return 1;
        }
        if (current_end == length) {
            break;
        }
        scan_start = (current_end + lm_p0_line_break_width_at(source, length, current_end));
        scan_line = scan_line + 1U;
    }
    lm_p0_set_diagnostic(document, 24, line, 1U, "unterminated raw comment block");
    return 0;
}

static int lm_p0_scan_block_string_event(LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, LmP0StreamEvent *event, size_t *next_offset, size_t *next_line) {
    size_t line_end;
    size_t eq_count;
    size_t content_start;
    size_t content_end;
    size_t scan_start;
    size_t scan_line;
    if ((line_start >= length) || (source[line_start] != '=')) {
        return 0;
    }
    line_end = lm_p0_find_physical_line_end(source, length, line_start);
    eq_count = 0U;
    while (((line_start + eq_count) < line_end) && (source[(line_start + eq_count)] == '=')) {
        eq_count = eq_count + 1U;
    }
    if (eq_count < 3U) {
        return 0;
    }
    if (eq_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "block string fence length exceeds 80 characters");
        return 0;
    }
    if (lm_p0_match_block_string_fence_line(source, line_start, line_end, eq_count) == 0) {
        lm_p0_set_diagnostic(document, 29, line, 1U, "block string fence line must contain only the equals fence and whitespace");
        return 0;
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(source, length, scan_start);
    }
    content_start = scan_start;
    content_end = content_start;
    scan_line = (line + 1U);
    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;
        current_end = lm_p0_find_physical_line_end(source, length, scan_start);
        if (lm_p0_match_block_string_fence_line(source, scan_start, current_end, eq_count)) {
            if (current_end < length) {
                break_width = lm_p0_line_break_width_at(source, length, current_end);
            }
            else {
                break_width = 0U;
            }
            memset(event, 0, sizeof(event[0]));
            event->kind = LM_P0_STREAM_EVENT_BLOCK_STRING;
            event->text = (source + content_start);
            event->text_length = (content_end - content_start);
            event->line = line;
            event->column = 1U;
            event->offset = line_start;
            next_offset[0] = (current_end + break_width);
            size_t lm_p0_tmp_13;
            if (break_width > 0U) {
                lm_p0_tmp_13 = 1U;
            }
            else {
                lm_p0_tmp_13 = 0U;
            }
            next_line[0] = (scan_line + lm_p0_tmp_13);
            return 1;
        }
        if (current_end == length) {
            break;
        }
        content_end = current_end;
        scan_start = (current_end + lm_p0_line_break_width_at(source, length, current_end));
        scan_line = scan_line + 1U;
    }
    lm_p0_set_diagnostic(document, 20, line, 1U, "unterminated block string literal");
    return 0;
}

static LmP0Node * lm_p0_new_node(LmP0Document *document, LmP0NodeKind kind) {
    LmP0Node * node;
    node = (((LmP0Node *)lm_own_arena_new_zero(document -> tree_arena, sizeof(node[0]))));
    if (node == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node");
        return 0;
    }
    node->span = (((LmP0Span *)lm_own_arena_new_zero(document -> tree_arena, sizeof(node -> span[0]))));
    if (node -> span == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node span");
        return 0;
    }
    node->as = (((LmP0NodeAs *)lm_own_arena_new_zero(document -> tree_arena, sizeof(node -> as[0]))));
    if (node -> as == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node payload");
        return 0;
    }
    node->kind = kind;
    if (kind == LM_P0_NODE_STRUCTURE) {
        node->as->structure = lm_p0_new_structure(document, 0U, 0U);
        if (node -> as -> structure == 0) {
            return 0;
        }
    }
    else {
        if (kind == LM_P0_NODE_FRAME) {
            node->as->frame = lm_p0_new_frame(document, 0U, 0U);
            if (node -> as -> frame == 0) {
                return 0;
            }
        }
        else {
            if ((kind == LM_P0_NODE_ATOM) || (kind == LM_P0_NODE_DISABLED)) {
                node->as->atom = lm_p0_new_text(document, "", 0U, 0U, 0U);
                if (node -> as -> atom == 0) {
                    return 0;
                }
            }
        }
    }
    return node;
}

static int lm_p0_append_field(LmP0Document *document, LmP0Structure *structure, LmP0Node *node) {
    LmP0Field * field;
    field = (((LmP0Field *)lm_own_arena_new_zero(document -> tree_arena, sizeof(field[0]))));
    if (field == 0) {
        size_t lm_p0_tmp_14;
        if (node != 0) {
            lm_p0_tmp_14 = node -> span -> line;
        }
        else {
            lm_p0_tmp_14 = 0U;
        }
        size_t lm_p0_tmp_15;
        if (node != 0) {
            lm_p0_tmp_15 = node -> span -> column;
        }
        else {
            lm_p0_tmp_15 = 0U;
        }
        lm_p0_set_diagnostic(document, 1, lm_p0_tmp_14, lm_p0_tmp_15, "out of memory while allocating parser field");
        return 0;
    }
    field->value = node;
    if (structure -> last_field == 0) {
        structure->first_field = field;
    }
    else {
        structure->last_field->next = field;
    }
    structure->last_field = field;
    structure->field_count = structure -> field_count + 1U;
    return 1;
}

static int lm_p0_pointer_source_offset(const LmP0Document *document, const char *pointer, size_t *out_offset) {
    const char *begin;
    const char *end_index;
    if (((document == 0) || (document -> source == 0)) || (pointer == 0)) {
        return 0;
    }
    begin = document -> source;
    end_index = (document -> source + document -> source_length);
    if ((pointer < begin) || (pointer > end_index)) {
        return 0;
    }
    out_offset[0] = (((size_t)(pointer - begin)));
    return 1;
}

static size_t lm_p0_offset_from_line_column(const char *text, size_t length, size_t line, size_t column) {
    size_t i;
    size_t current_line;
    size_t current_column;
    i = 0U;
    current_line = 1U;
    current_column = 1U;
    while (i < length) {
        if ((current_line == line) && (current_column == column)) {
            return i;
        }
        if (text[i] == '\r') {
            current_line = current_line + 1U;
            current_column = 1U;
            if (((i + 1U) < length) && (text[(i + 1U)] == '\n')) {
                i = i + 1U;
            }
        }
        else {
            if (text[i] == '\n') {
                current_line = current_line + 1U;
                current_column = 1U;
            }
            else {
                current_column = current_column + 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static void lm_p0_copy_payload_diagnostic(LmP0Document *document, const LmP0Document *payload_document, size_t payload_offset) {
    size_t local_offset;
    if (((((document == 0) || (document -> diagnostic == 0)) || (payload_document == 0)) || (payload_document -> diagnostic == 0)) || (payload_document -> diagnostic -> code == 0)) {
        return;
    }
    document->diagnostic[0] = payload_document -> diagnostic[0];
    local_offset = lm_p0_offset_from_line_column(payload_document -> source, payload_document -> source_length, payload_document -> diagnostic -> line, payload_document -> diagnostic -> column);
    lm_p0_position_in_slice(document -> source, document -> source_length, (payload_offset + local_offset), 1U, 1U, &document -> diagnostic -> line, &document -> diagnostic -> column);
}

static void lm_p0_adjust_node_span_to_document(LmP0Document *document, LmP0Node *node, size_t base_offset) {
    if (node == 0) {
        return;
    }
    node->span->offset = node -> span -> offset + base_offset;
    lm_p0_position_in_slice(document -> source, document -> source_length, node -> span -> offset, 1U, 1U, &node -> span -> line, &node -> span -> column);
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        lm_p0_adjust_structure_spans_to_document(document, node -> as -> structure, base_offset);
        if (node -> as -> structure -> trailer != 0) {
            lm_p0_adjust_structure_spans_to_document(document, node -> as -> structure -> trailer -> body, base_offset);
        }
    }
    else {
        if (node -> kind == LM_P0_NODE_FRAME) {
            lm_p0_adjust_structure_spans_to_document(document, node -> as -> frame -> body, base_offset);
            if (node -> as -> frame -> trailer != 0) {
                lm_p0_adjust_structure_spans_to_document(document, node -> as -> frame -> trailer -> body, base_offset);
            }
        }
    }
}

static void lm_p0_adjust_structure_spans_to_document(LmP0Document *document, LmP0Structure *structure, size_t base_offset) {
    LmP0Field * field;
    field = structure -> first_field;
    while (field != 0) {
        lm_p0_adjust_node_span_to_document(document, field -> value, base_offset);
        field = field -> next;
    }
}

static int lm_p0_record_mix_mark(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t start, size_t end) {
    LmP0Node * node;
    size_t span_line;
    size_t span_column;
    size_t absolute_offset;
    size_t payload_offset;
    size_t payload_length;
    LmP0Document * payload_document;
    int status;
    node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (node == 0) {
        return 0;
    }
    node->flags = node -> flags | LM_P0_NODE_MIX;
    lm_p0_position_in_slice(text, length, start, line, column, &span_line, &span_column);
    node->span->line = span_line;
    node->span->column = span_column;
    node->span->length = end - start;
    if (lm_p0_pointer_source_offset(document, text + start, &absolute_offset)) {
        node->span->offset = absolute_offset;
    }
    if (lm_p0_append_field(document, structure, node) == 0) {
        lm_p0_free_node(node);
        return 0;
    }
    payload_offset = node -> span -> offset + 1U;
    if (end > start + 2U) {
        payload_length = end - start - 2U;
    }
    else {
        payload_length = 0U;
    }
    if (payload_length > 0U) {
        payload_document = lm_own_new_zero(sizeof(payload_document[0]));
        if (payload_document == 0) {
            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while creating MIX payload document");
            return 0;
        }
        status = 0;
        if (lm_p0_document_init_owners(payload_document) != 0) {
            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while creating MIX payload owners");
        }
        else {
            payload_document->source = (((char *)(text + start + 1U)));
            payload_document->source_length = payload_length;
            if (lm_p0_parse_stream(payload_document) == 0) {
                lm_p0_copy_payload_diagnostic(document, payload_document, payload_offset);
            }
            else {
                if (lm_p0_postprocess_node(payload_document, payload_document -> root) == 0) {
                    lm_p0_copy_payload_diagnostic(document, payload_document, payload_offset);
                }
                else {
                    if (lm_p0_validate_nonempty_colon_frames_in_node(payload_document, payload_document -> root) == 0) {
                        lm_p0_copy_payload_diagnostic(document, payload_document, payload_offset);
                    }
                    else {
                        if (lm_own_tree_cut(payload_document -> tree_arena) != 0) {
                            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while promoting MIX lazy text edges");
                        }
                        else {
                            node->as->structure = payload_document -> root -> as -> structure;
                            payload_document->root->as->structure = 0;
                            if (lm_own_arena_absorb(document -> tree_arena, payload_document -> tree_arena) != 0) {
                                lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while moving MIX tree into parser arena");
                            }
                            else {
                                lm_p0_adjust_structure_spans_to_document(document, node -> as -> structure, payload_offset);
                                status = 1;
                            }
                        }
                    }
                }
            }
        }
        lm_p0_document_destroy_owners(payload_document);
        lm_own_delete(payload_document, 0);
        if (status == 0) {
            return 0;
        }
    }
    return 1;
}

static int lm_p0_is_bare_identifier_start(char c) {
    unsigned char value;
    value = (((unsigned char)c));
    return (isalpha(value) || (c == '_'));
}

static int lm_p0_is_bare_identifier_rest(char c) {
    unsigned char value;
    value = (((unsigned char)c));
    return (isalnum(value) || (c == '_'));
}

static int lm_p0_is_hex_digit(char c) {
    unsigned char value;
    value = (((unsigned char)c));
    return (isxdigit(value) != 0);
}

static int lm_p0_scan_number_token(const char *text, size_t end_index, size_t start, size_t *out_end) {
    size_t i;
    int saw_digit;
    i = start;
    saw_digit = 0;
    if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
        if (((i + 1U) < end_index) && lm_p0_is_decimal_digit(text[(i + 1U)])) {
            i = i + 1U;
        }
        else {
            out_end[0] = start;
            return 0;
        }
    }
    if ((((i + 1U) < end_index) && (text[i] == '0')) && ((text[(i + 1U)] == 'x') || (text[(i + 1U)] == 'X'))) {
        i = i + 2U;
        while ((i < end_index) && lm_p0_is_hex_digit(text[i])) {
            saw_digit = 1;
            i = i + 1U;
        }
        if ((i < end_index) && (text[i] == '.')) {
            i = i + 1U;
            while ((i < end_index) && lm_p0_is_hex_digit(text[i])) {
                saw_digit = 1;
                i = i + 1U;
            }
        }
        if ((saw_digit && (i < end_index)) && ((text[i] == 'p') || (text[i] == 'P'))) {
            size_t exponent_start;
            exponent_start = i;
            i = i + 1U;
            if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
                i = i + 1U;
            }
            if ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
                while ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
                    i = i + 1U;
                }
            }
            else {
                i = exponent_start;
            }
        }
        while ((i < end_index) && lm_p0_is_bare_identifier_rest(text[i])) {
            i = i + 1U;
        }
        if (saw_digit) {
            out_end[0] = i;
        }
        else {
            out_end[0] = (start + 1U);
        }
        return 1;
    }
    while ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
        saw_digit = 1;
        i = i + 1U;
    }
    if ((i < end_index) && (text[i] == '.')) {
        i = i + 1U;
        while ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
            saw_digit = 1;
            i = i + 1U;
        }
    }
    if ((saw_digit && (i < end_index)) && ((text[i] == 'e') || (text[i] == 'E'))) {
        size_t exponent_start;
        exponent_start = i;
        i = i + 1U;
        if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
            i = i + 1U;
        }
        if ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
            while ((i < end_index) && lm_p0_is_decimal_digit(text[i])) {
                i = i + 1U;
            }
        }
        else {
            i = exponent_start;
        }
    }
    while ((i < end_index) && lm_p0_is_bare_identifier_rest(text[i])) {
        i = i + 1U;
    }
    if (saw_digit) {
        out_end[0] = i;
    }
    else {
        out_end[0] = start;
    }
    return saw_digit;
}

static size_t lm_p0_scan_c_quoted_token(const char *text, size_t end_index, size_t quote_index) {
    size_t i;
    char quote;
    if ((quote_index >= end_index) || ((text[quote_index] != '\'') && (text[quote_index] != '"'))) {
        return quote_index;
    }
    quote = text[quote_index];
    i = (quote_index + 1U);
    while (i < end_index) {
        if ((text[i] == '\\') && ((i + 1U) < end_index)) {
            size_t line_break_width;
            line_break_width = lm_p0_line_break_width_at(text, end_index, (i + 1U));
            if (line_break_width > 0U) {
                i = i + (1U + line_break_width);
            }
            else {
                i = i + 2U;
            }
            continue;
        }
        if (text[i] == quote) {
            return (i + 1U);
        }
        if (lm_p0_is_line_break(text[i])) {
            return (quote_index + 1U);
        }
        i = i + 1U;
    }
    return (quote_index + 1U);
}

static int lm_p0_starts_c_prefixed_quote(const char *text, size_t end_index, size_t start) {
    return ((((start + 1U) < end_index) && (text[start] == 'L')) && ((text[(start + 1U)] == '\'') || (text[(start + 1U)] == '"')));
}

static size_t lm_p0_scan_c_char_token(const char *text, size_t end_index, size_t start) {
    if ((start >= end_index) || (text[start] != '\'')) {
        return start;
    }
    return lm_p0_scan_c_quoted_token(text, end_index, start);
}

static size_t lm_p0_scan_c_prefixed_quote_token(const char *text, size_t end_index, size_t start) {
    size_t quoted_end;
    if (lm_p0_starts_c_prefixed_quote(text, end_index, start) == 0) {
        return start;
    }
    quoted_end = lm_p0_scan_c_quoted_token(text, end_index, (start + 1U));
    if (quoted_end > (start + 2U)) {
        return quoted_end;
    }
    return (start + 1U);
}

static int lm_p0_starts_c_surface_atom(const char *text, size_t end_index, size_t start) {
    return ((((start + 1U) < end_index) && (text[start] == 'c')) && (text[(start + 1U)] == '.'));
}

static int lm_p0_is_c_surface_top_boundary(char value) {
    return ((((((lm_p0_is_field_space(value) || lm_p0_is_field_separator(value)) || (value == '(')) || (value == ')')) || (value == ':')) || (value == '#')) || (value == '{'));
}

static size_t lm_p0_scan_c_sizeof_surface_atom(const char *text, size_t end_index, size_t start) {
    size_t i;
    size_t close_index;
    size_t depth;
    int raw_safe;
    if ((((start + 8U) >= end_index) || (memcmp((text + start), "c.sizeof", 8U) != 0)) || (text[(start + 8U)] != '(')) {
        return start;
    }
    i = (start + 9U);
    depth = 1U;
    while (i < end_index) {
        if (lm_p0_is_line_break(text[i])) {
            return start;
        }
        if (lm_p0_starts_c_prefixed_quote(text, end_index, i)) {
            size_t quoted_end;
            quoted_end = lm_p0_scan_c_prefixed_quote_token(text, end_index, i);
            if (quoted_end > (i + 1U)) {
                i = quoted_end;
                continue;
            }
        }
        if ((text[i] == '\'') || (text[i] == '"')) {
            size_t quoted_end;
            quoted_end = lm_p0_scan_c_quoted_token(text, end_index, i);
            if (quoted_end > (i + 1U)) {
                i = quoted_end;
                continue;
            }
        }
        if (text[i] == '(') {
            depth = depth + 1U;
        }
        else {
            if (text[i] == ')') {
                depth = depth - 1U;
                if (depth == 0U) {
                    break;
                }
            }
        }
        i = i + 1U;
    }
    if ((i >= end_index) || (text[i] != ')')) {
        return start;
    }
    close_index = i;
    raw_safe = 1;
    i = (start + 9U);
    while (i < close_index) {
        if (((text[i] == '\\') || (text[i] == '@')) || ((((i + 1U) < close_index) && (text[i] == 'c')) && (text[(i + 1U)] == '.'))) {
            raw_safe = 0;
            break;
        }
        i = i + 1U;
    }
    if (raw_safe) {
        return (close_index + 1U);
    }
    return start;
}

static size_t lm_p0_scan_c_surface_atom(const char *text, size_t end_index, size_t start) {
    size_t i;
    size_t sizeof_end;
    size_t bracket_depth;
    if (lm_p0_starts_c_surface_atom(text, end_index, start) == 0) {
        return start;
    }
    sizeof_end = lm_p0_scan_c_sizeof_surface_atom(text, end_index, start);
    if (sizeof_end > start) {
        return sizeof_end;
    }
    i = (start + 2U);
    bracket_depth = 0U;
    while (i < end_index) {
        if (lm_p0_is_line_break(text[i])) {
            break;
        }
        if ((bracket_depth == 0U) && lm_p0_is_c_surface_top_boundary(text[i])) {
            break;
        }
        if (text[i] == '[') {
            bracket_depth = bracket_depth + 1U;
        }
        else {
            if ((text[i] == ']') && (bracket_depth > 0U)) {
                bracket_depth = bracket_depth - 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static int lm_p0_scan_c_char(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    size_t end_index;
    end_index = lm_p0_scan_c_char_token(text, length, index[0]);
    if (end_index <= (index[0] + 1U)) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated C character literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static int lm_p0_scan_c_prefixed_quote(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    size_t end_index;
    end_index = lm_p0_scan_c_prefixed_quote_token(text, length, index[0]);
    if (end_index <= (index[0] + 1U)) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated C prefixed literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static size_t lm_p0_scan_builtin_compact_atom_piece(const char *text, size_t end_index, size_t start) {
    if ((start + 1U) < end_index) {
        char first;
        char second;
        first = text[start];
        second = text[(start + 1U)];
        if (((((((((first == '!') && (second == '=')) || ((first == '<') && (second == '='))) || ((first == '>') && (second == '='))) || ((first == '&') && (second == '&'))) || ((first == '|') && (second == '|'))) || ((first == '+') && (second == '+'))) || ((first == '-') && (second == '-'))) || ((first == '[') && (second == ']'))) {
            return (start + 2U);
        }
    }
    return (start + 1U);
}

static size_t lm_p0_scan_registry_compact_atom_piece(const char *text, size_t end_index, size_t start) {
    size_t i;
    size_t best_length;
    size_t token_length;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    if (lm_p0_registry_table_has_rows_loaded_or_loading("p0.compact-token") == 0) {
        return lm_p0_scan_builtin_compact_atom_piece(text, end_index, start);
    }
    best_length = 0U;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at("p0.compact-token", i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0) {
            token_length = strlen(key_cell -> value);
            if (((token_length > best_length) && ((start + token_length) <= end_index)) && (memcmp((text + start), key_cell -> value, token_length) == 0)) {
                best_length = token_length;
            }
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at("p0.compact-token", i, &key_cell);
    }
    if (best_length > 0U) {
        return (start + best_length);
    }
    return (start + 1U);
}

static size_t lm_p0_scan_compact_atom_piece(const char *text, size_t end_index, size_t start, int allow_signed_number) {
    size_t i;
    if (start >= end_index) {
        return start;
    }
    if (lm_p0_starts_c_prefixed_quote(text, end_index, start)) {
        return lm_p0_scan_c_prefixed_quote_token(text, end_index, start);
    }
    if (lm_p0_starts_c_surface_atom(text, end_index, start)) {
        return lm_p0_scan_c_surface_atom(text, end_index, start);
    }
    if (lm_p0_is_bare_identifier_start(text[start])) {
        i = (start + 1U);
        while ((i < end_index) && lm_p0_is_bare_identifier_rest(text[i])) {
            i = i + 1U;
        }
        if (((((i == (start + 1U)) && (text[start] == 'c')) && ((i + 1U) < end_index)) && (text[i] == '.')) && lm_p0_is_bare_identifier_start(text[(i + 1U)])) {
            i = i + 2U;
            while ((i < end_index) && lm_p0_is_bare_identifier_rest(text[i])) {
                i = i + 1U;
            }
        }
        return i;
    }
    if (lm_p0_is_decimal_digit(text[start]) || (allow_signed_number && (((start + 1U) < end_index) && ((text[start] == '+') || (text[start] == '-')) && lm_p0_is_decimal_digit(text[(start + 1U)])))) {
        size_t number_end;
        if (lm_p0_scan_number_token(text, end_index, start, &number_end)) {
            return number_end;
        }
    }
    if ((text[start] == '\'') && (lm_p0_starts_python_string(text, end_index, start) == 0)) {
        return lm_p0_scan_c_char_token(text, end_index, start);
    }
    if ((start + 1U) < end_index) {
        char first;
        char second;
        first = text[start];
        second = text[(start + 1U)];
        if ((first == '@') && (second == '@')) {
            size_t at_end;
            at_end = (start + 2U);
            while ((at_end < end_index) && (text[at_end] == '@')) {
                at_end = at_end + 1U;
            }
            return at_end;
        }
    }
    return lm_p0_scan_registry_compact_atom_piece(text, end_index, start);
}

static int lm_p0_append_atom_slice(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index) {
    LmP0Node * node;
    node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
    if (node == 0) {
        return 0;
    }
    node->as->atom->data = (text + start);
    node->as->atom->length = (end_index - start);
    node->span->line = line;
    lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
    node->span->offset = (offset + start);
    node->span->length = (end_index - start);
    if (lm_p0_document_register_lazy_text(document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
        return 0;
    }
    return lm_p0_append_field(document, structure, node);
}

static int lm_p0_append_positional_skip(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t index) {
    LmP0Node * node;
    node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
    if (node == 0) {
        return 0;
    }
    node->flags = node -> flags | LM_P0_NODE_POSITIONAL_SKIP;
    node->as->atom->data = "";
    node->as->atom->length = 0U;
    node->span->line = line;
    lm_p0_position_in_slice(text, length, index, line, column, &node -> span -> line, &node -> span -> column);
    node->span->offset = (offset + index);
    node->span->length = 0U;
    return lm_p0_append_field(document, structure, node);
}

static int lm_p0_append_compact_atom_pieces(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index) {
    size_t i;
    int allow_signed_number;
    i = start;
    allow_signed_number = 1;
    while (i < end_index) {
        size_t piece_end;
        if (lm_p0_is_field_space(text[i])) {
            i = i + 1U;
            continue;
        }
        if (text[i] == '[') {
            size_t close_index;
            size_t inner_index;
            if (lm_p0_append_atom_slice(document, structure, text, length, line, column, offset, i, (i + 1U)) == 0) {
                return 0;
            }
            if (lm_p0_find_matching_bracket(document, text, end_index, i, line, column, &close_index) == 0) {
                return 0;
            }
            inner_index = 0U;
            if (lm_p0_parse_fields_until(document, structure, ((text + i) + 1U), ((close_index - i) - 1U), line, ((column + i) + 1U), ((offset + i) + 1U), 0U, 0U, 0U, &inner_index) == 0) {
                return 0;
            }
            if (lm_p0_append_atom_slice(document, structure, text, length, line, column, offset, close_index, (close_index + 1U)) == 0) {
                return 0;
            }
            i = (close_index + 1U);
            allow_signed_number = 0;
            continue;
        }
        piece_end = lm_p0_scan_compact_atom_piece(text, end_index, i, allow_signed_number);
        if (piece_end <= i) {
            piece_end = (i + 1U);
        }
        if (lm_p0_append_atom_slice(document, structure, text, length, line, column, offset, i, piece_end) == 0) {
            return 0;
        }
        i = piece_end;
        allow_signed_number = 0;
    }
    return 1;
}

static void lm_p0_free_node(LmP0Node *node) {
    LM_UNUSED(node);
}

static int lm_p0_relaxed_level_from_column(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t base_level, size_t line, size_t source_column, size_t *out_level) {
    size_t i;
    size_t parent_level;
    if (column == 0U) {
        if (base_level == 0U) {
            out_level[0] = 0U;
        }
        else {
            out_level[0] = 1U;
        }
        return 1;
    }
    if (base_level == 0U) {
        parent_level = 0U;
    }
    else {
        parent_level = (base_level - 1U);
    }
    if (stack -> count == 0U) {
        if (lm_p0_indent_stack_push(document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = base_level;
        return 1;
    }
    if (column > stack -> columns[(stack -> count - 1U)]) {
        if (lm_p0_indent_stack_push(document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = ((base_level + stack -> count) - 1U);
        return 1;
    }
    if (column < stack -> columns[0]) {
        out_level[0] = parent_level;
        return 1;
    }
    while ((stack -> count > 0U) && (stack -> columns[(stack -> count - 1U)] > column)) {
        stack->count = stack -> count - 1U;
    }
    if (stack -> count == 0U) {
        out_level[0] = parent_level;
        return 1;
    }
    i = 0U;
    while (i < stack -> count) {
        if (stack -> columns[i] == column) {
            out_level[0] = (base_level + i);
            return 1;
        }
        i = i + 1U;
    }
    lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
    return 0;
}

static int lm_p0_source_level_after_line_break(LmP0Document *document, LmP0IndentStack *indent_stack, const char *text, size_t length, size_t index, size_t line, size_t column, size_t base_level, size_t *content_index, size_t *out_level) {
    size_t p;
    size_t indent_column;
    size_t dot_level;
    p = index;
    if ((((p < length) && (text[p] == '\r')) && ((p + 1U) < length)) && (text[(p + 1U)] == '\n')) {
        p = p + 2U;
    }
    else {
        if ((p < length) && lm_p0_is_line_break(text[p])) {
            p = p + 1U;
        }
    }
    indent_column = 0U;
    while ((p < length) && lm_p0_is_horizontal_space(text[p])) {
        if (text[p] == '\t') {
            indent_column = lm_p0_indent_tab_column(indent_column);
        }
        else {
            indent_column = indent_column + 1U;
        }
        p = p + 1U;
    }
    dot_level = 0U;
    while ((p < length) && (text[p] == '.')) {
        dot_level = dot_level + 1U;
        p = p + 1U;
        while ((p < length) && lm_p0_is_horizontal_space(text[p])) {
            p = p + 1U;
        }
    }
    content_index[0] = p;
    if (dot_level > 0U) {
        out_level[0] = dot_level;
        return 1;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(text, length, p, line, column, &diagnostic_line, &diagnostic_column);
    return lm_p0_relaxed_level_from_column(document, indent_stack, indent_column, base_level, diagnostic_line, diagnostic_column, out_level);
}

static int lm_p0_skip_field_space(LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t *current_source_level, int *stopped_by_source_level) {
    stopped_by_source_level[0] = 0;
    while (index[0] < length) {
        if (lm_p0_is_horizontal_space(text[index[0]])) {
            index[0] = index[0] + 1U;
            continue;
        }
        if (text[index[0]] == '#') {
            while ((index[0] < length) && (lm_p0_is_line_break(text[index[0]]) == 0)) {
                index[0] = index[0] + 1U;
            }
            continue;
        }
        if (text[index[0]] == '{') {
            if (lm_p0_consume_brace_mark(document, structure, text, length, index, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_is_line_break(text[index[0]])) {
            size_t content_index;
            size_t next_level;
            if (lm_p0_source_level_after_line_break(document, indent_stack, text, length, index[0], line, column, layout_base_level, &content_index, &next_level) == 0) {
                return 0;
            }
            if ((((((flags & LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL) != 0U) && (next_level < short_source_level)) && (content_index < length)) && (text[content_index] != ')')) && (text[content_index] != ']')) {
                size_t diagnostic_line;
                size_t diagnostic_column;
                lm_p0_position_in_slice(text, length, content_index, line, column, &diagnostic_line, &diagnostic_column);
                lm_p0_set_diagnostic(document, 13, diagnostic_line, diagnostic_column, "bounded form continuation must stay inside the form");
                return 0;
            }
            if (((flags & LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) != 0U) && (next_level <= short_source_level)) {
                stopped_by_source_level[0] = 1;
                return 1;
            }
            current_source_level[0] = next_level;
            index[0] = content_index;
            continue;
        }
        break;
    }
    return 1;
}

static int lm_p0_scan_python_string(LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    size_t end_index;
    if (lm_p0_find_python_string_end(text, length, index[0], &end_index) == 0) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated python-like string literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static int lm_p0_scan_quoted(LmP0Document *document, const char *text, size_t length, size_t *index, char quote, size_t line, size_t base_column) {
    size_t i;
    if (lm_p0_starts_python_string(text, length, index[0])) {
        return lm_p0_scan_python_string(document, text, length, index, line, base_column);
    }
    i = (index[0] + 1U);
    while (i < length) {
        if (quote == '"') {
            if (text[i] == '\\') {
                size_t line_break_width;
                if ((i + 1U) < length) {
                    line_break_width = lm_p0_line_break_width_at(text, length, (i + 1U));
                }
                else {
                    line_break_width = 0U;
                }
                if (line_break_width > 0U) {
                    i = i + (1U + line_break_width);
                    continue;
                }
                if ((i + 1U) < length) {
                    i = i + 2U;
                }
                else {
                    i = i + 1U;
                }
                continue;
            }
            if (lm_p0_is_line_break(text[i])) {
                size_t diagnostic_line;
                size_t diagnostic_column;
                lm_p0_position_in_slice(text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
                lm_p0_set_diagnostic(document, 19, diagnostic_line, diagnostic_column, "unescaped newline in string literal");
                return 0;
            }
        }
        if ((quote == '`') && (text[i] == '\0')) {
            size_t diagnostic_line;
            size_t diagnostic_column;
            lm_p0_position_in_slice(text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(document, 30, diagnostic_line, diagnostic_column, "NUL byte in exact quoted identifier");
            return 0;
        }
        if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
            i = i + 2U;
            continue;
        }
        if (text[i] == quote) {
            index[0] = (i + 1U);
            return 1;
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated quoted token");
    return 0;
}

static int lm_p0_require_quoted_token_boundary(LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column) {
    size_t diagnostic_line;
    size_t diagnostic_column;
    if ((index >= length) || lm_p0_is_quoted_token_boundary(text[index])) {
        return 1;
    }
    lm_p0_position_in_slice(text, length, index, line, column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 18, diagnostic_line, diagnostic_column, "missing separator after quoted token");
    return 0;
}

static int lm_p0_find_matching_paren(LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index) {
    size_t i;
    size_t depth;
    i = open_index;
    depth = 0U;
    while (i < length) {
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            if (lm_p0_scan_python_string(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(text, length, i) == 0)) {
            if (lm_p0_scan_c_char(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(document, text, length, &i, text[i], line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            depth = depth + 1U;
        }
        else {
            if (text[i] == ')') {
                depth = depth - 1U;
                if (depth == 0U) {
                    close_index[0] = i;
                    return 1;
                }
            }
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(text, length, open_index, line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 5, diagnostic_line, diagnostic_column, "unclosed parenthesized form");
    return 0;
}

static int lm_p0_find_matching_bracket(LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index) {
    size_t i;
    size_t bracket_depth;
    size_t paren_depth;
    i = open_index;
    bracket_depth = 0U;
    paren_depth = 0U;
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            if (lm_p0_scan_python_string(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(text, length, i) == 0)) {
            if (lm_p0_scan_c_char(document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(document, text, length, &i, text[i], line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            paren_depth = paren_depth + 1U;
        }
        else {
            if ((text[i] == ')') && (paren_depth > 0U)) {
                paren_depth = paren_depth - 1U;
            }
            else {
                if (text[i] == '[') {
                    bracket_depth = bracket_depth + 1U;
                }
                else {
                    if (text[i] == ']') {
                        if (bracket_depth == 0U) {
                            break;
                        }
                        bracket_depth = bracket_depth - 1U;
                        if ((bracket_depth == 0U) && (paren_depth == 0U)) {
                            close_index[0] = i;
                            return 1;
                        }
                    }
                }
            }
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(text, length, open_index, line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 5, diagnostic_line, diagnostic_column, "unclosed index bracket form");
    return 0;
}

static int lm_p0_find_colon(LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *colon_index) {
    size_t i;
    size_t depth;
    depth = 0U;
    i = 0U;
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_c_surface_atom(text, length, i)) {
            i = lm_p0_scan_c_surface_atom(text, length, i);
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            i = lm_p0_skip_python_string_unchecked(text, length, i);
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(text, length, i) == 0)) {
            if (lm_p0_scan_c_char(document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            char quote;
            char lm_p0_tmp_16;
            lm_p0_tmp_16 = text[i];
            i = i + 1U;
            quote = lm_p0_tmp_16;
            while (i < length) {
                if ((quote == '"') && (text[i] == '\\')) {
                    i = i + 2U;
                    continue;
                }
                if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                if ((quote == '`') && (text[i] == '\0')) {
                    size_t diagnostic_line;
                    size_t diagnostic_column;
                    lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                    lm_p0_set_diagnostic(document, 30, diagnostic_line, diagnostic_column, "NUL byte in exact quoted identifier");
                    return -1;
                }
                char lm_p0_tmp_17;
                lm_p0_tmp_17 = text[i];
                i = i + 1U;
                if (lm_p0_tmp_17 == quote) {
                    break;
                }
            }
            continue;
        }
        if ((text[i] == '(') || (text[i] == '[')) {
            depth = depth + 1U;
        }
        else {
            if (((text[i] == ')') || (text[i] == ']')) && (depth > 0U)) {
                depth = depth - 1U;
            }
            else {
                if ((text[i] == ':') && (depth == 0U)) {
                    colon_index[0] = i;
                    return 1;
                }
            }
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_p0_field_start_looks_explicit_frame(LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column) {
    size_t i;
    size_t head_end;
    size_t close_index;
    if (index >= length) {
        return 0;
    }
    i = index;
    if (text[i] == '(') {
        if (lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index) == 0) {
            return 0;
        }
        return (((close_index + 1U) < length) && (text[(close_index + 1U)] == ':'));
    }
    if (lm_p0_starts_c_surface_atom(text, length, i)) {
        i = lm_p0_scan_c_surface_atom(text, length, i);
        head_end = i;
    }
    else {
        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, column) == 0) {
                return 0;
            }
            head_end = i;
        }
        else {
            if ((lm_p0_starts_python_string(text, length, i) || (text[i] == '"')) || (text[i] == '`')) {
                if (lm_p0_scan_quoted(document, text, length, &i, text[i], line, column) == 0) {
                    return 0;
                }
                head_end = i;
            }
            else {
                if (text[i] == '\'') {
                    if (lm_p0_scan_c_char(document, text, length, &i, line, column) == 0) {
                        return 0;
                    }
                    head_end = i;
                }
                else {
                    while ((((((((i < length) && (lm_p0_is_field_space(text[i]) == 0)) && (lm_p0_is_field_separator(text[i]) == 0)) && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (text[i] != '{')) && (text[i] != ':')) {
                        if (text[i] == '[') {
                            size_t bracket_close_index;
                            if (lm_p0_find_matching_bracket(document, text, length, i, line, column, &bracket_close_index) == 0) {
                                return 0;
                            }
                            i = (bracket_close_index + 1U);
                            continue;
                        }
                        if (text[i] == ']') {
                            break;
                        }
                        i = i + 1U;
                    }
                    head_end = i;
                }
            }
        }
    }
    return (((head_end > index) && (i < length)) && ((text[i] == ':') || (text[i] == '(')));
}

static void lm_p0_field_parse_loop_frame_delete_any(void *object) {
    LmP0FieldParseLoopFrame * frame;
    frame = (((LmP0FieldParseLoopFrame *)object));
    if (frame == 0) {
        return;
    }
    if (frame -> indent_stack_owned) {
        lm_p0_indent_stack_delete(frame -> indent_stack);
    }
    lm_own_delete(frame, 0);
}

static LmOwnPtrStack * lm_p0_field_parse_loop_stack_new(LmP0Document *document, size_t line, size_t column) {
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating parser field stack");
        return 0;
    }
    lm_own_ptr_stack_init(stack, lm_p0_field_parse_loop_frame_delete_any);
    return stack;
}

static void lm_p0_field_parse_loop_stack_delete(LmOwnPtrStack *stack) {
    if (stack == 0) {
        return;
    }
    lm_own_ptr_stack_destroy(stack);
    lm_own_delete(stack, 0);
}

static int lm_p0_field_parse_loop_push(LmP0Document *document, LmOwnPtrStack *stack, LmP0IndentStack *indent_stack, int indent_stack_owned, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t i, size_t current_source_level, int allow_empty_fields, int expect_field, int headless_group_after_separator, LmP0FieldParseLoopContinuation continuation, LmP0Node *node, size_t start, size_t close_index) {
    LmP0FieldParseLoopFrame * frame;
    frame = (((LmP0FieldParseLoopFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while growing parser field stack");
        return 0;
    }
    frame->indent_stack = indent_stack;
    frame->structure = structure;
    frame->text = text;
    frame->length = length;
    frame->line = line;
    frame->column = column;
    frame->offset = offset;
    frame->flags = flags;
    frame->short_source_level = short_source_level;
    frame->layout_base_level = layout_base_level;
    frame->i = i;
    frame->current_source_level = current_source_level;
    frame->allow_empty_fields = allow_empty_fields;
    frame->expect_field = expect_field;
    frame->headless_group_after_separator = headless_group_after_separator;
    frame->indent_stack_owned = indent_stack_owned;
    frame->continuation = continuation;
    frame->node = node;
    frame->start = start;
    frame->close_index = close_index;
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_p0_field_parse_loop_frame_delete_any(frame);
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while growing parser field stack");
        return 0;
    }
    return 1;
}

static int lm_p0_field_parse_fail(LmOwnPtrStack **parse_stack, LmP0IndentStack **indent_stack, int *indent_stack_owned) {
    if ((((indent_stack_owned != 0) && indent_stack_owned[0]) && (indent_stack != 0)) && (indent_stack[0] != 0)) {
        lm_p0_indent_stack_delete(indent_stack[0]);
        indent_stack[0] = 0;
        indent_stack_owned[0] = 0;
    }
    if (parse_stack != 0) {
        lm_p0_field_parse_loop_stack_delete(parse_stack[0]);
        parse_stack[0] = 0;
    }
    return 0;
}

static int lm_p0_parse_append_node_and_update(LmP0Document *document, LmP0Structure *structure, LmP0Node *node, unsigned flags, int allow_empty_fields, int *expect_field, int *headless_group_after_separator) {
    if (lm_p0_append_field(document, structure, node) == 0) {
        return 0;
    }
    expect_field[0] = 0;
    if ((allow_empty_fields && (node -> kind == LM_P0_NODE_FRAME)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) != 0U)) {
        expect_field[0] = 1;
    }
    if ((((flags & LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR) != 0U) && (node -> kind == LM_P0_NODE_FRAME)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) != 0U)) {
        headless_group_after_separator[0] = 1;
    }
    return 1;
}

static int lm_p0_parse_fields_until_with_layout(LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t layout_base_level, size_t *index) {
    size_t i;
    size_t start;
    size_t head_end;
    size_t close_index;
    size_t current_source_level;
    size_t child_index;
    LmOwnPtrStack * parse_stack;
    LmP0FieldParseLoopFrame * frame;
    LmP0IndentStack * child_indent_stack;
    LmP0Node * node;
    int allow_empty_fields;
    int expect_field;
    int headless_group_after_separator;
    int indent_stack_owned;
    int restart_parse_context;
    parse_stack = lm_p0_field_parse_loop_stack_new(document, line, column);
    if (parse_stack == 0) {
        return 0;
    }
    i = index[0];
    current_source_level = initial_source_level;
    allow_empty_fields = ((flags & LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS) != 0U);
    expect_field = allow_empty_fields;
    headless_group_after_separator = 0;
    indent_stack_owned = 0;
    restart_parse_context = 1;
    while (restart_parse_context) {
        restart_parse_context = 0;
        while (i < length) {
            int stopped_by_source_level;
            if (lm_p0_skip_field_space(document, indent_stack, structure, text, length, &i, line, column, flags, short_source_level, layout_base_level, &current_source_level, &stopped_by_source_level) == 0) {
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            if (stopped_by_source_level) {
                break;
            }
            if (i >= length) {
                break;
            }
            if (((((headless_group_after_separator && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (lm_p0_is_field_separator(text[i]) == 0)) && (lm_p0_field_start_looks_explicit_frame(document, text, length, i, line, column) == 0)) {
                LmP0Node * group_node;
                group_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
                if (group_node == 0) {
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP, group_node, i, 0U) == 0) {
                    lm_p0_free_node(group_node);
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                structure = group_node -> as -> structure;
                flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                short_source_level = current_source_level;
                current_source_level = (current_source_level + 1U);
                allow_empty_fields = 1;
                expect_field = 1;
                headless_group_after_separator = 0;
                indent_stack_owned = 0;
                restart_parse_context = 1;
                break;
            }
            else {
                if (headless_group_after_separator && (lm_p0_is_field_separator(text[i]) == 0)) {
                    headless_group_after_separator = 0;
                }
            }
            if (lm_p0_index_is_line_start(text, i)) {
                size_t field_line;
                size_t field_column;
                size_t next_offset;
                size_t next_line;
                LmP0StreamEvent * block_event;
                lm_p0_position_in_slice(text, length, i, line, column, &field_line, &field_column);
                if (lm_p0_scan_raw_comment_block(document, text, length, i, field_line, &next_offset, &next_line)) {
                    i = next_offset;
                    continue;
                }
                if (document -> diagnostic -> code != 0) {
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                block_event = lm_p0_stream_event_new();
                if (block_event == 0) {
                    lm_p0_set_diagnostic(document, 1, field_line, field_column, "out of memory while creating block string event");
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (lm_p0_scan_block_string_event(document, text, length, i, field_line, block_event, &next_offset, &next_line)) {
                    node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
                    if (node == 0) {
                        lm_p0_stream_event_delete(block_event);
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node->as->atom->data = block_event -> text;
                    node->as->atom->length = block_event -> text_length;
                    node->span->line = field_line;
                    node->span->column = field_column;
                    node->span->offset = (offset + i);
                    node->span->length = (next_offset - i);
                    if (lm_p0_document_register_lazy_text(document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
                        lm_p0_stream_event_delete(block_event);
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_append_field(document, structure, node) == 0) {
                        lm_p0_free_node(node);
                        lm_p0_stream_event_delete(block_event);
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    lm_p0_stream_event_delete(block_event);
                    expect_field = 0;
                    i = next_offset;
                    continue;
                }
                lm_p0_stream_event_delete(block_event);
                if (document -> diagnostic -> code != 0) {
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
            }
            if (lm_p0_is_field_separator(text[i])) {
                char separator;
                size_t separator_index;
                separator_index = i;
                char lm_p0_tmp_18;
                lm_p0_tmp_18 = text[i];
                i = i + 1U;
                separator = lm_p0_tmp_18;
                if (lm_p0_is_short_form_separator(separator) && ((flags & LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON) != 0U)) {
                    break;
                }
                if (allow_empty_fields && expect_field) {
                    if (lm_p0_append_positional_skip(document, structure, text, length, line, column, offset, separator_index) == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                }
                expect_field = allow_empty_fields;
                continue;
            }
            if (text[i] == ')') {
                break;
            }
            start = i;
            head_end = i;
            node = 0;
            if (text[i] == '(') {
                if (lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index) == 0) {
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (((close_index == (i + 1U)) && ((close_index + 1U) < length)) && (text[(close_index + 1U)] == ':')) {
                    size_t body_index;
                    unsigned body_flags;
                    int has_inline_body;
                    node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node->as->frame->head->data = (text + start);
                    node->as->frame->head->length = ((close_index - start) + 1U);
                    node->as->frame->flags = LM_P0_FRAME_COLON;
                    i = (close_index + 2U);
                    while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
                        i = i + 1U;
                    }
                    body_index = i;
                    has_inline_body = (((((i < length) && (lm_p0_is_line_break(text[i]) == 0)) && (lm_p0_is_field_separator(text[i]) == 0)) && (text[i] != ')')) && (text[i] != '#'));
                    if (has_inline_body) {
                        node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_INLINE_BODY;
                    }
                    body_flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY, node, start, close_index) == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> frame -> body;
                    flags = body_flags;
                    short_source_level = current_source_level;
                    current_source_level = (current_source_level + 1U);
                    i = body_index;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack_owned = 0;
                    restart_parse_context = 1;
                    break;
                }
                else {
                    node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    child_indent_stack = lm_p0_indent_stack_new_empty();
                    if (child_indent_stack == 0) {
                        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE, node, start, close_index) == 0) {
                        lm_p0_indent_stack_delete(child_indent_stack);
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> structure;
                    text = ((text + i) + 1U);
                    length = ((close_index - i) - 1U);
                    column = ((column + i) + 1U);
                    offset = ((offset + i) + 1U);
                    flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    short_source_level = (current_source_level + 1U);
                    layout_base_level = (current_source_level + 1U);
                    current_source_level = (current_source_level + 1U);
                    i = 0U;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack = child_indent_stack;
                    indent_stack_owned = 1;
                    restart_parse_context = 1;
                    break;
                }
            }
            else {
                if ((text[i] == '[') && (lm_p0_field_start_looks_explicit_frame(document, text, length, i, line, column) == 0)) {
                    if (lm_p0_find_matching_bracket(document, text, length, i, line, column, &close_index) == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    child_indent_stack = lm_p0_indent_stack_new_empty();
                    if (child_indent_stack == 0) {
                        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE, node, start, close_index) == 0) {
                        lm_p0_indent_stack_delete(child_indent_stack);
                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> structure;
                    text = ((text + i) + 1U);
                    length = ((close_index - i) - 1U);
                    column = ((column + i) + 1U);
                    offset = ((offset + i) + 1U);
                    flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    short_source_level = (current_source_level + 1U);
                    layout_base_level = (current_source_level + 1U);
                    current_source_level = (current_source_level + 1U);
                    i = 0U;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack = child_indent_stack;
                    indent_stack_owned = 1;
                    restart_parse_context = 1;
                    break;
                }
                else {
                    int quoted_head;
                    quoted_head = 0;
                    if (lm_p0_starts_c_surface_atom(text, length, i)) {
                        i = lm_p0_scan_c_surface_atom(text, length, i);
                        head_end = i;
                    }
                    else {
                        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
                            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, column) == 0) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            head_end = i;
                            quoted_head = 1;
                        }
                        else {
                            if ((lm_p0_starts_python_string(text, length, i) || (text[i] == '"')) || (text[i] == '`')) {
                                if (lm_p0_scan_quoted(document, text, length, &i, text[i], line, column) == 0) {
                                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                                }
                                head_end = i;
                                quoted_head = 1;
                            }
                            else {
                                if (text[i] == '\'') {
                                    if (lm_p0_scan_c_char(document, text, length, &i, line, column) == 0) {
                                        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                                    }
                                    head_end = i;
                                    quoted_head = 1;
                                }
                                else {
                                    while ((((((((i < length) && (lm_p0_is_field_space(text[i]) == 0)) && (lm_p0_is_field_separator(text[i]) == 0)) && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (text[i] != '{')) && (text[i] != ':')) {
                                        if (text[i] == '[') {
                                            size_t bracket_close_index;
                                            if (lm_p0_find_matching_bracket(document, text, length, i, line, column, &bracket_close_index) == 0) {
                                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                                            }
                                            i = (bracket_close_index + 1U);
                                            continue;
                                        }
                                        if (text[i] == ']') {
                                            break;
                                        }
                                        if ((((lm_p0_starts_c_prefixed_quote(text, length, i) || lm_p0_starts_python_string(text, length, i)) || (text[i] == '"')) || (text[i] == '`')) || (text[i] == '\'')) {
                                            size_t diagnostic_line;
                                            size_t diagnostic_column;
                                            lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                                            lm_p0_set_diagnostic(document, 18, diagnostic_line, diagnostic_column, "missing separator before quoted token");
                                            return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                                        }
                                        i = i + 1U;
                                    }
                                    head_end = i;
                                }
                            }
                        }
                    }
                    if (((i < length) && (text[i] == ':')) && (head_end > start)) {
                        size_t body_index;
                        unsigned body_flags;
                        int has_inline_body;
                        node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
                        if (node == 0) {
                            return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                        }
                        node->as->frame->head->data = (text + start);
                        node->as->frame->head->length = (head_end - start);
                        node->as->frame->flags = LM_P0_FRAME_COLON;
                        i = i + 1U;
                        while ((i < length) && lm_p0_is_horizontal_space(text[i])) {
                            i = i + 1U;
                        }
                        body_index = i;
                        has_inline_body = (((((i < length) && (lm_p0_is_line_break(text[i]) == 0)) && (lm_p0_is_field_separator(text[i]) == 0)) && (text[i] != ')')) && (text[i] != '#'));
                        if (has_inline_body) {
                            node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_INLINE_BODY;
                        }
                        body_flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                        if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY, node, start, 0U) == 0) {
                            return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                        }
                        structure = node -> as -> frame -> body;
                        flags = body_flags;
                        short_source_level = current_source_level;
                        current_source_level = (current_source_level + 1U);
                        i = body_index;
                        allow_empty_fields = 1;
                        expect_field = 1;
                        headless_group_after_separator = 0;
                        indent_stack_owned = 0;
                        restart_parse_context = 1;
                        break;
                    }
                    else {
                        if (((i < length) && (text[i] == '(')) && (head_end > start)) {
                            node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
                            if (node == 0) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            node->as->frame->head->data = (text + start);
                            node->as->frame->head->length = (head_end - start);
                            node->as->frame->flags = LM_P0_FRAME_COMPACT;
                            if (lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index) == 0) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            child_indent_stack = lm_p0_indent_stack_new_empty();
                            if (child_indent_stack == 0) {
                                lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (lm_p0_field_parse_loop_push(document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY, node, start, close_index) == 0) {
                                lm_p0_indent_stack_delete(child_indent_stack);
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            structure = node -> as -> frame -> body;
                            text = ((text + i) + 1U);
                            length = ((close_index - i) - 1U);
                            column = ((column + i) + 1U);
                            offset = ((offset + i) + 1U);
                            flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                            short_source_level = (current_source_level + 1U);
                            layout_base_level = (current_source_level + 1U);
                            current_source_level = (current_source_level + 1U);
                            i = 0U;
                            allow_empty_fields = 1;
                            expect_field = 1;
                            headless_group_after_separator = 0;
                            indent_stack = child_indent_stack;
                            indent_stack_owned = 1;
                            restart_parse_context = 1;
                            break;
                        }
                        else {
                            if (head_end == start) {
                                size_t diagnostic_line;
                                size_t diagnostic_column;
                                lm_p0_position_in_slice(text, length, start, line, column, &diagnostic_line, &diagnostic_column);
                                lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unexpected character in field list");
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (quoted_head && (lm_p0_require_quoted_token_boundary(document, text, length, head_end, line, column) == 0)) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (quoted_head == 0) {
                                i = head_end;
                                if (lm_p0_append_compact_atom_pieces(document, structure, text, length, line, column, offset, start, head_end) == 0) {
                                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                                }
                                expect_field = 0;
                                continue;
                            }
                            node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
                            if (node == 0) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            node->as->atom->data = (text + start);
                            node->as->atom->length = (head_end - start);
                            node->span->line = line;
                            lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
                            node->span->offset = (offset + start);
                            node->span->length = (head_end - start);
                            if (lm_p0_document_register_lazy_text(document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
                                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            i = head_end;
                        }
                    }
                }
            }
            if (lm_p0_parse_append_node_and_update(document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
        }
        if (restart_parse_context) {
            continue;
        }
        child_index = i;
        if (indent_stack_owned) {
            lm_p0_indent_stack_delete(indent_stack);
            indent_stack = 0;
            indent_stack_owned = 0;
        }
        frame = (((LmP0FieldParseLoopFrame *)lm_own_ptr_stack_pop(parse_stack)));
        if (frame == 0) {
            index[0] = child_index;
            lm_p0_field_parse_loop_stack_delete(parse_stack);
            return 1;
        }
        indent_stack = frame -> indent_stack;
        structure = frame -> structure;
        text = frame -> text;
        length = frame -> length;
        line = frame -> line;
        column = frame -> column;
        offset = frame -> offset;
        flags = frame -> flags;
        short_source_level = frame -> short_source_level;
        layout_base_level = frame -> layout_base_level;
        i = frame -> i;
        current_source_level = frame -> current_source_level;
        allow_empty_fields = frame -> allow_empty_fields;
        expect_field = frame -> expect_field;
        headless_group_after_separator = frame -> headless_group_after_separator;
        indent_stack_owned = frame -> indent_stack_owned;
        node = frame -> node;
        start = frame -> start;
        close_index = frame -> close_index;
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP) {
            if (child_index <= start) {
                lm_p0_free_node(node);
                headless_group_after_separator = 0;
            }
            else {
                node->span->line = line;
                lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
                node->span->offset = (offset + start);
                node->span->length = (child_index - start);
                if (lm_p0_append_field(document, structure, node) == 0) {
                    lm_p0_free_node(node);
                    lm_own_delete(frame, 0);
                    return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
                }
                expect_field = 0;
                headless_group_after_separator = ((((flags & LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR) != 0U) && (child_index > 0U)) && lm_p0_is_short_form_separator(text[(child_index - 1U)]));
                i = child_index;
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY) {
            if ((child_index > 0U) && lm_p0_is_short_form_separator(text[(child_index - 1U)])) {
                node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_SEPARATOR_CLOSED;
            }
            node->span->line = line;
            lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = (child_index - start);
            if (lm_p0_document_register_lazy_text(document, node -> as -> frame -> head -> data, node -> as -> frame -> head -> length, &node -> as -> frame -> head -> data, node -> span -> line, node -> span -> column) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            i = child_index;
            if (lm_p0_parse_append_node_and_update(document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE) {
            node->span->line = line;
            lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = ((close_index - start) + 1U);
            i = (close_index + 1U);
            if (lm_p0_parse_append_node_and_update(document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY) {
            node->span->line = line;
            lm_p0_position_in_slice(text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = ((close_index - start) + 1U);
            if (lm_p0_document_register_lazy_text(document, node -> as -> frame -> head -> data, node -> as -> frame -> head -> length, &node -> as -> frame -> head -> data, node -> span -> line, node -> span -> column) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            i = (close_index + 1U);
            if (lm_p0_parse_append_node_and_update(document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        lm_own_delete(frame, 0);
        lm_p0_set_diagnostic(document, 1, line, column, "internal parser field stack continuation error");
        return lm_p0_field_parse_fail(&parse_stack, &indent_stack, &indent_stack_owned);
    }
    return 0;
}

static int lm_p0_parse_fields_until(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t *index) {
    LmP0IndentStack * indent_stack;
    int status;
    indent_stack = lm_p0_indent_stack_new_empty();
    if (indent_stack == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
        return 0;
    }
    status = lm_p0_parse_fields_until_with_layout(document, indent_stack, structure, text, length, line, column, offset, flags, short_source_level, initial_source_level, initial_source_level, index);
    lm_p0_indent_stack_delete(indent_stack);
    return status;
}

static int lm_p0_parse_fields_into(LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset) {
    size_t index;
    index = 0U;
    return lm_p0_parse_fields_until(document, structure, text, length, line, column, offset, LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR, 0U, 0U, &index);
}

static int lm_p0_stack_ensure(LmP0Document *document, LmP0Stack *stack, size_t level) {
    LmP0Structure * *parents;
    LmP0Node * *owners;
    unsigned char *hard;
    size_t old_capacity;
    size_t new_capacity;
    size_t i;
    if (level < stack -> capacity) {
        return 1;
    }
    old_capacity = stack -> capacity;
    if (old_capacity == 0U) {
        new_capacity = 8U;
    }
    else {
        new_capacity = old_capacity;
    }
    while (new_capacity <= level) {
        new_capacity = new_capacity * 2U;
    }
    parents = (((LmP0Structure * *)lm_own_resize(stack -> parents, (new_capacity * sizeof(parents[0])))));
    if (parents == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        return 0;
    }
    owners = (((LmP0Node * *)lm_own_resize(stack -> owners, (new_capacity * sizeof(owners[0])))));
    if (owners == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        return 0;
    }
    hard = (((unsigned char *)lm_own_resize(stack -> hard, (new_capacity * sizeof(hard[0])))));
    if (hard == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        stack->owners = owners;
        return 0;
    }
    stack->parents = parents;
    stack->owners = owners;
    stack->hard = hard;
    stack->capacity = new_capacity;
    i = old_capacity;
    while (i < new_capacity) {
        stack->parents[i] = 0;
        stack->owners[i] = 0;
        stack->hard[i] = 0U;
        i = i + 1U;
    }
    return 1;
}

static void lm_p0_stack_truncate_deeper(LmP0Stack *stack, size_t level) {
    size_t i;
    if (stack -> capacity <= (level + 1U)) {
        return;
    }
    i = (level + 1U);
    while (i < stack -> capacity) {
        stack->parents[i] = 0;
        stack->owners[i] = 0;
        stack->hard[i] = 0U;
        i = i + 1U;
    }
}

static size_t lm_p0_stack_top_level(const LmP0Stack *stack) {
    size_t i;
    i = stack -> capacity;
    while (i > 0U) {
        i = i - 1U;
        if (stack -> parents[i] != 0) {
            return i;
        }
    }
    return 0U;
}

static int lm_p0_stack_level_is_trailer_body(const LmP0Stack *stack, size_t level) {
    LmP0Node * owner;
    if ((level >= stack -> capacity) || (stack -> parents[level] == 0)) {
        return 0;
    }
    owner = stack -> owners[level];
    if (owner == 0) {
        return 0;
    }
    if (((owner -> kind == LM_P0_NODE_FRAME) && (owner -> as -> frame -> trailer != 0)) && (stack -> parents[level] == owner -> as -> frame -> trailer -> body)) {
        return 1;
    }
    if (((owner -> kind == LM_P0_NODE_STRUCTURE) && (owner -> as -> structure -> trailer != 0)) && (stack -> parents[level] == owner -> as -> structure -> trailer -> body)) {
        return 1;
    }
    return 0;
}

static size_t lm_p0_stack_collapse_soft_to_event(LmP0Stack *stack, size_t event_level) {
    size_t top_level;
    top_level = lm_p0_stack_top_level(stack);
    while (((top_level > 0U) && (event_level < top_level)) && (stack -> hard[top_level] == 0U)) {
        lm_p0_stack_truncate_deeper(stack, (top_level - 1U));
        top_level = lm_p0_stack_top_level(stack);
    }
    return top_level;
}

static LmP0TrailerRole lm_p0_registry_trailer_role(const char *text, size_t length) {
    size_t i;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    const char *role_payload;
    LmP0TrailerRole result;
    if (lm_p0_registry_table_has_rows("trailer.role") == 0) {
        return LM_P0_TRAILER_ROLE_NONE;
    }
    result = LM_P0_TRAILER_ROLE_NONE;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at("namespace", i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0 && payload_cell -> value != 0) {
            role_payload = lm_p0_registry_lookup(lm_p0_text_from_cstr(payload_cell -> value), "trailer.role");
            if (role_payload != 0 && lm_p0_text_has_prefix_name(text, length, key_cell -> value, lm_p0_registry_trailer_allows_bare(payload_cell -> value))) {
                result = lm_p0_trailer_role_from_payload(role_payload);
            }
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at("namespace", i, &key_cell);
    }
    return result;
}

static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length) {
    LmP0TrailerRole legacy_role;
    LmP0TrailerRole registry_role;
    if (lm_p0_dash_fence_status_after_comment_trim(text, length, 0) == LM_P0_DASH_FENCE_VALID) {
        return LM_P0_TRAILER_ROLE_DASH_CUTTER;
    }
    legacy_role = lm_p0_legacy_trailer_role(text, length);
    if (lm_p0_registry_table_has_rows("trailer.role") == 0) {
        return legacy_role;
    }
    registry_role = lm_p0_registry_trailer_role(text, length);
    if (lm_p0_registry_compare_enabled() && (registry_role != legacy_role)) {
        char *key;
        key = lm_p0_copy_bytes(text, length);
        lm_p0_registry_compare_fail("trailer.role", key, lm_p0_trailer_role_payload(registry_role), lm_p0_trailer_role_payload(legacy_role));
    }
    return registry_role;
}

static void lm_p0_stack_free(LmP0Stack *stack) {
    lm_own_delete(stack -> parents, 0);
    lm_own_delete(stack -> owners, 0);
    lm_own_delete(stack -> hard, 0);
    stack->parents = 0;
    stack->owners = 0;
    stack->hard = 0;
    stack->capacity = 0U;
}

static void lm_p0_stack_free_any(void *object) {
    lm_p0_stack_free(((LmP0Stack *)object));
}

static LmP0Stack * lm_p0_stack_new(void) {
    return (((LmP0Stack *)lm_own_new_zero(1U * sizeof(LmP0Stack))));
}

static void lm_p0_stack_delete(LmP0Stack *stack) {
    lm_own_delete(stack, lm_p0_stack_free_any);
}

static LmP0PendingDelimiter * lm_p0_pending_delimiter_new(void) {
    return (((LmP0PendingDelimiter *)lm_own_new_zero(1U * sizeof(LmP0PendingDelimiter))));
}

static LmP0StreamEvent * lm_p0_stream_event_new(void) {
    return (((LmP0StreamEvent *)lm_own_new_zero(1U * sizeof(LmP0StreamEvent))));
}

static LmP0StreamEvent * lm_p0_stream_event_new_copy(const LmP0StreamEvent *event) {
    LmP0StreamEvent * copy;
    if (event == 0) {
        return 0;
    }
    copy = (((LmP0StreamEvent *)lm_own_new_zero(1U * sizeof(copy[0]))));
    if (copy != 0) {
        copy[0] = event[0];
    }
    return copy;
}

static void lm_p0_stream_event_delete(LmP0StreamEvent *event) {
    lm_own_delete(event, 0);
}

static void lm_p0_pending_delimiter_clear(LmP0PendingDelimiter *pending) {
    if (pending != 0) {
        lm_p0_stream_event_delete(pending -> event);
        pending->event = 0;
        pending->active = 0;
    }
}

static int lm_p0_pending_delimiter_set(LmP0Document *document, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event) {
    if (pending == 0) {
        return 0;
    }
    lm_p0_pending_delimiter_clear(pending);
    pending->event = lm_p0_stream_event_new_copy(event);
    if (pending -> event == 0) {
        size_t lm_p0_tmp_19;
        if (event != 0) {
            lm_p0_tmp_19 = event -> line;
        }
        else {
            lm_p0_tmp_19 = 0U;
        }
        size_t lm_p0_tmp_20;
        if (event != 0) {
            lm_p0_tmp_20 = event -> column;
        }
        else {
            lm_p0_tmp_20 = 0U;
        }
        lm_p0_set_diagnostic(document, 1, lm_p0_tmp_19, lm_p0_tmp_20, "out of memory while storing pending delimiter");
        return 0;
    }
    pending->active = 1;
    return 1;
}

static void lm_p0_pending_delimiter_delete(LmP0PendingDelimiter *pending) {
    if (pending != 0) {
        lm_p0_pending_delimiter_clear(pending);
        lm_own_delete(pending, 0);
    }
}

static LmP0PendingMix * lm_p0_pending_mix_new(void) {
    return (((LmP0PendingMix *)lm_own_new_zero(1U * sizeof(LmP0PendingMix))));
}

static void lm_p0_pending_mix_free(LmP0PendingMix *pending) {
    if (pending != 0) {
        lm_own_delete(pending -> events, 0);
        pending->events = 0;
        pending->count = 0U;
        pending->capacity = 0U;
    }
}

static void lm_p0_pending_mix_free_any(void *object) {
    lm_p0_pending_mix_free(((LmP0PendingMix *)object));
}

static void lm_p0_pending_mix_delete(LmP0PendingMix *pending) {
    lm_own_delete(pending, lm_p0_pending_mix_free_any);
}

static int lm_p0_pending_mix_push(LmP0Document *document, LmP0PendingMix *pending, const LmP0StreamEvent *event) {
    size_t new_capacity;
    LmP0StreamEvent * events;
    if ((pending == 0) || (event == 0)) {
        return 0;
    }
    if (pending -> count == pending -> capacity) {
        if (pending -> capacity == 0U) {
            new_capacity = 4U;
        }
        else {
            new_capacity = (pending -> capacity * 2U);
        }
        events = (((LmP0StreamEvent *)lm_own_resize(pending -> events, (new_capacity * sizeof(events[0])))));
        if (events == 0) {
            lm_p0_set_diagnostic(document, 1, event -> line, event -> column, "out of memory while storing pending MIX marks");
            return 0;
        }
        pending->events = events;
        pending->capacity = new_capacity;
    }
    pending->events[pending->count] = event[0];
    pending->count = pending -> count + 1U;
    return 1;
}

static LmP0DisabledState * lm_p0_disabled_state_new(size_t base_level) {
    LmP0DisabledState * state;
    state = (((LmP0DisabledState *)lm_own_new_zero(1U * sizeof(state[0]))));
    if (state != 0) {
        state->base_level = base_level;
        state->top_level = base_level;
        state->pending_item = 1;
        state->pending_level = base_level;
    }
    return state;
}

static void lm_p0_disabled_state_delete(LmP0DisabledState *state) {
    lm_own_delete(state, 0);
}

static int lm_p0_node_keeps_source_child_level(LmP0Node *node) {
    if (node -> kind == LM_P0_NODE_FRAME) {
        return (((node -> as -> frame -> flags & LM_P0_FRAME_COLON) != 0U) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) == 0U));
    }
    return 0;
}

static LmP0Structure * lm_p0_node_child_structure(LmP0Node *node) {
    if (node -> kind == LM_P0_NODE_FRAME) {
        return node -> as -> frame -> body;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        return node -> as -> structure;
    }
    return 0;
}

static LmP0Node * lm_p0_structure_last_colon_frame(LmP0Structure *structure) {
    LmP0Node * node;
    if ((structure -> last_field == 0) || (structure -> last_field -> value == 0)) {
        return 0;
    }
    node = structure -> last_field -> value;
    if (((node -> kind == LM_P0_NODE_FRAME) && ((node -> as -> frame -> flags & LM_P0_FRAME_COLON) != 0U)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) == 0U)) {
        return node;
    }
    return 0;
}

static int lm_p0_stack_install_node_lineage(LmP0Document *document, LmP0Stack *stack, size_t base_level, LmP0Node *node) {
    LmP0Node * owner;
    size_t level;
    if (lm_p0_node_keeps_source_child_level(node) == 0) {
        if (lm_p0_stack_ensure(document, stack, (base_level + 1U)) == 0) {
            return 0;
        }
        stack->parents[(base_level + 1U)] = 0;
        stack->owners[(base_level + 1U)] = 0;
        stack->hard[(base_level + 1U)] = 0U;
        lm_p0_stack_truncate_deeper(stack, (base_level + 1U));
        return 1;
    }
    owner = node;
    level = (base_level + 1U);
    while (lm_p0_node_keeps_source_child_level(owner)) {
        LmP0Structure * body;
        LmP0Node * next_owner;
        if (lm_p0_stack_ensure(document, stack, level) == 0) {
            return 0;
        }
        body = lm_p0_node_child_structure(owner);
        stack->parents[level] = body;
        stack->owners[level] = owner;
        if (body -> field_count == 0U) {
            stack->hard[level] = 1U;
        }
        else {
            stack->hard[level] = 0U;
        }
        next_owner = lm_p0_structure_last_colon_frame(body);
        if (next_owner == 0) {
            break;
        }
        owner = next_owner;
        level = level + 1U;
    }
    lm_p0_stack_truncate_deeper(stack, level);
    return 1;
}

static int lm_p0_stack_ensure_root_level_alias(LmP0Document *document, LmP0Stack *stack, size_t level) {
    if ((level != 1U) || (stack -> parents[1] != 0)) {
        return 1;
    }
    if (lm_p0_stack_ensure(document, stack, 1U) == 0) {
        return 0;
    }
    stack->parents[1] = stack -> parents[0];
    stack->owners[1] = stack -> owners[0];
    stack->hard[1] = 1U;
    return 1;
}

static int lm_p0_stack_open_implicit_anonymous(LmP0Document *document, LmP0Stack *stack, size_t parent_level, size_t line, size_t column, size_t offset) {
    LmP0Node * anonymous_node;
    LmP0Structure * parent;
    parent = stack -> parents[parent_level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, line, column, "source level has no open parent structure");
        return 0;
    }
    anonymous_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (anonymous_node == 0) {
        return 0;
    }
    anonymous_node->span->line = line;
    anonymous_node->span->column = column;
    anonymous_node->span->offset = offset;
    anonymous_node->span->length = 0U;
    if (lm_p0_append_field(document, parent, anonymous_node) == 0) {
        lm_p0_free_node(anonymous_node);
        return 0;
    }
    if (lm_p0_stack_ensure(document, stack, (parent_level + 1U)) == 0) {
        return 0;
    }
    stack->parents[(parent_level + 1U)] = anonymous_node -> as -> structure;
    stack->owners[(parent_level + 1U)] = anonymous_node;
    stack->hard[(parent_level + 1U)] = 1U;
    lm_p0_stack_truncate_deeper(stack, (parent_level + 1U));
    return 1;
}

static LmP0Trailer * * lm_p0_node_trailer_slot(LmP0Node *node) {
    if (node -> kind == LM_P0_NODE_FRAME) {
        return &node -> as -> frame -> trailer;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        return &node -> as -> structure -> trailer;
    }
    return 0;
}

static LmP0Trailer * lm_p0_attach_trailer(LmP0Document *document, LmP0Node *node, const char *spelling, size_t spelling_length, unsigned flags, size_t line, size_t column) {
    LmP0Trailer * *slot;
    LmP0Trailer * trailer;
    slot = lm_p0_node_trailer_slot(node);
    if (slot == 0) {
        lm_p0_set_diagnostic(document, 10, line, column, "this parser node cannot receive a trailer");
        return 0;
    }
    if (slot[0] != 0) {
        lm_p0_set_diagnostic(document, 11, line, column, "parser node already has a trailer");
        return 0;
    }
    trailer = lm_p0_new_trailer(document, spelling, spelling_length, line, column);
    if (trailer == 0) {
        return 0;
    }
    trailer->flags = flags;
    slot[0] = trailer;
    return trailer;
}

static int lm_p0_parse_trailer_item(LmP0Document *document, LmP0Node *target, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, LmP0Structure **out_body) {
    size_t colon_index;
    size_t spelling_length;
    size_t body_start;
    LmP0Trailer * trailer;
    out_body[0] = 0;
    int colon_status;
    colon_status = lm_p0_find_colon(document, text, length, line, column, &colon_index);
    if (colon_status < 0) {
        return 0;
    }
    if (colon_status == 0) {
        colon_index = length;
    }
    if (colon_index < length) {
        spelling_length = colon_index;
        while ((spelling_length > 0U) && lm_p0_is_horizontal_space(text[(spelling_length - 1U)])) {
            spelling_length = spelling_length - 1U;
        }
        body_start = (colon_index + 1U);
        while ((body_start < length) && lm_p0_is_horizontal_space(text[body_start])) {
            body_start = body_start + 1U;
        }
    }
    else {
        body_start = 0U;
        if ((body_start < length) && lm_p0_starts_c_prefixed_quote(text, length, body_start)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &body_start, line, column) == 0) {
                return 0;
            }
            spelling_length = body_start;
        }
        else {
            if ((body_start < length) && (text[body_start] == '\'')) {
                if (lm_p0_scan_c_char(document, text, length, &body_start, line, column) == 0) {
                    return 0;
                }
                spelling_length = body_start;
            }
            else {
                if ((body_start < length) && ((lm_p0_starts_python_string(text, length, body_start) || (text[body_start] == '"')) || (text[body_start] == '`'))) {
                    if (lm_p0_scan_quoted(document, text, length, &body_start, text[body_start], line, column) == 0) {
                        return 0;
                    }
                    spelling_length = body_start;
                }
                else {
                    while ((((body_start < length) && (lm_p0_is_horizontal_space(text[body_start]) == 0)) && (lm_p0_is_field_separator(text[body_start]) == 0)) && (text[body_start] != '#')) {
                        body_start = body_start + 1U;
                    }
                    spelling_length = body_start;
                }
            }
        }
        while ((body_start < length) && lm_p0_is_horizontal_space(text[body_start])) {
            body_start = body_start + 1U;
        }
    }
    if (spelling_length == 0U) {
        lm_p0_set_diagnostic(document, 12, line, column, "trailer spelling is empty");
        return 0;
    }
    trailer = lm_p0_attach_trailer(document, target, text, spelling_length, flags, line, column);
    if (trailer == 0) {
        return 0;
    }
    if (lm_p0_parse_fields_into(document, trailer -> body, (text + body_start), (length - body_start), line, (column + body_start), (offset + body_start)) == 0) {
        return 0;
    }
    out_body[0] = trailer -> body;
    return 1;
}

static int lm_p0_stream_resolve_pending_delimiter(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, size_t next_level) {
    const LmP0StreamEvent * event;
    size_t top_level;
    LmP0Structure * parent;
    if ((pending -> active == 0) || (pending -> event == 0)) {
        return 1;
    }
    event = pending -> event;
    if (lm_p0_stack_ensure(document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    top_level = lm_p0_stack_collapse_soft_to_event(stack, event -> level);
    if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
        stack->hard[top_level] = 1U;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(stack, event -> level);
        top_level = lm_p0_stack_top_level(stack);
    }
    parent = stack -> parents[event -> level];
    if ((parent == 0) && (event -> level == 1U)) {
        parent = stack -> parents[0];
    }
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    if (next_level == (event -> level + 1U)) {
        LmP0Node * anonymous_node;
        anonymous_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
        if (anonymous_node == 0) {
            return 0;
        }
        anonymous_node->span->line = event -> line;
        anonymous_node->span->column = event -> column;
        anonymous_node->span->offset = event -> offset;
        anonymous_node->span->length = event -> text_length;
        if (lm_p0_append_field(document, parent, anonymous_node) == 0) {
            lm_p0_free_node(anonymous_node);
            return 0;
        }
        stack->parents[(event->level + 1U)] = anonymous_node -> as -> structure;
        stack->owners[(event->level + 1U)] = anonymous_node;
        stack->hard[(event->level + 1U)] = 1U;
        lm_p0_stack_truncate_deeper(stack, (event -> level + 1U));
    }
    else {
        lm_p0_stack_truncate_deeper(stack, event -> level);
    }
    lm_p0_pending_delimiter_clear(pending);
    return 1;
}

static int lm_p0_stream_apply_item_event(LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event) {
    LmP0TrailerRole trailer_role;
    size_t top_level;
    LmP0Structure * parent;
    LmP0Node * node;
    int trailer_target_available;
    int trailer_target_accepted;
    if (lm_p0_stack_ensure(document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_validate_dash_fence_line(document, event -> text, event -> text_length, event -> line, event -> column) == 0)) {
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_ITEM) {
        trailer_role = lm_p0_trailer_role(event -> text, event -> text_length);
    }
    else {
        trailer_role = LM_P0_TRAILER_ROLE_NONE;
    }
    top_level = lm_p0_stack_top_level(stack);
    trailer_target_available = ((lm_p0_trailer_role_is_tail_cutter(trailer_role) && ((event -> level + 1U) <= top_level)) && (stack -> owners[(event -> level + 1U)] != 0));
    trailer_target_accepted = (trailer_target_available && lm_p0_trailer_role_accepts_target(trailer_role, stack -> owners[(event -> level + 1U)]));
    if ((lm_p0_trailer_role_is_tail_cutter(trailer_role) == 0) || (trailer_target_accepted == 0)) {
        top_level = lm_p0_stack_collapse_soft_to_event(stack, event -> level);
        if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
            stack->hard[top_level] = 1U;
        }
        trailer_target_available = ((lm_p0_trailer_role_is_tail_cutter(trailer_role) && ((event -> level + 1U) <= top_level)) && (stack -> owners[(event -> level + 1U)] != 0));
        trailer_target_accepted = (trailer_target_available && lm_p0_trailer_role_accepts_target(trailer_role, stack -> owners[(event -> level + 1U)]));
    }
    if ((lm_p0_trailer_role_is_tail_cutter(trailer_role) && ((event -> level + 1U) < top_level)) && (trailer_target_accepted == 0)) {
        lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "tail-cutter target is not valid for this receiver");
        return 0;
    }
    if (lm_p0_trailer_role_is_tail_cutter(trailer_role) && trailer_target_accepted) {
        LmP0Structure * trailer_body;
        LmP0Node * target;
        size_t target_level;
        target_level = (event -> level + 1U);
        target = stack -> owners[target_level];
        lm_p0_stack_truncate_deeper(stack, target_level);
        if (lm_p0_parse_trailer_item(document, target, event -> text, event -> text_length, event -> line, event -> column, event -> offset, LM_P0_TRAILER_TAIL_CUTTER, &trailer_body) == 0) {
            return 0;
        }
        stack->parents[target_level] = trailer_body;
        stack->owners[target_level] = target;
        stack->hard[target_level] = 0U;
        if (target_level > 0U) {
            stack->hard[(target_level - 1U)] = 0U;
        }
        lm_p0_stack_truncate_deeper(stack, target_level);
        return 1;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(stack, event -> level);
        top_level = lm_p0_stack_top_level(stack);
    }
    if ((stack -> parents[event -> level] == 0) && (event -> level > top_level)) {
        if (event -> level != (top_level + 1U)) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "source level jumps too deep");
            return 0;
        }
        if (lm_p0_stack_open_implicit_anonymous(document, stack, top_level, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
    }
    if (lm_p0_stack_ensure_root_level_alias(document, stack, event -> level) == 0) {
        return 0;
    }
    parent = stack -> parents[event -> level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_ITEM) {
        LmP0Field * previous_last;
        LmP0Field * field;
        previous_last = parent -> last_field;
        if (lm_p0_parse_fields_into(document, parent, event -> text, event -> text_length, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
        if (previous_last != 0) {
            field = previous_last -> next;
        }
        else {
            field = parent -> first_field;
        }
        while (field != 0) {
            if (field -> value != 0) {
                field->value->flags = field -> value -> flags | event -> node_flags;
            }
            field = field -> next;
        }
        if ((parent -> last_field == previous_last) || (parent -> last_field == 0)) {
            return 1;
        }
        return lm_p0_stack_install_node_lineage(document, stack, event -> level, parent -> last_field -> value);
    }
    if (event -> kind == LM_P0_STREAM_EVENT_DISABLED_BLOCK) {
        node = lm_p0_new_node(document, LM_P0_NODE_DISABLED);
        if (node == 0) {
            return 0;
        }
        node->as->atom->data = event -> text;
        node->as->atom->length = event -> text_length;
        node->span->line = event -> line;
        node->span->column = event -> column;
        node->span->offset = event -> offset;
        node->span->length = event -> text_length;
        if (lm_p0_document_register_lazy_text(document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
            return 0;
        }
    }
    else {
        if (event -> kind != LM_P0_STREAM_EVENT_BLOCK_STRING) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "unexpected stream item event kind");
            return 0;
        }
        node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
        if (node == 0) {
            return 0;
        }
        node->as->atom->data = event -> text;
        node->as->atom->length = event -> text_length;
        node->span->line = event -> line;
        node->span->column = event -> column;
        node->span->offset = event -> offset;
        node->span->length = event -> text_length;
        if (lm_p0_document_register_lazy_text(document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
            return 0;
        }
    }
    node->flags = node -> flags | event -> node_flags;
    if (lm_p0_append_field(document, parent, node) == 0) {
        lm_p0_free_node(node);
        return 0;
    }
    if (lm_p0_stack_install_node_lineage(document, stack, event -> level, node) == 0) {
        return 0;
    }
    return 1;
}

static int lm_p0_stream_apply_mix_event(LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event) {
    size_t top_level;
    LmP0Structure * parent;
    LmP0Field * previous_last;
    LmP0Field * field;
    if (lm_p0_stack_ensure(document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    top_level = lm_p0_stack_collapse_soft_to_event(stack, event -> level);
    if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
        stack->hard[top_level] = 1U;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(stack, event -> level);
        top_level = lm_p0_stack_top_level(stack);
    }
    if ((stack -> parents[event -> level] == 0) && (event -> level > top_level)) {
        if (event -> level != (top_level + 1U)) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "source level jumps too deep");
            return 0;
        }
        if (lm_p0_stack_open_implicit_anonymous(document, stack, top_level, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
    }
    if (lm_p0_stack_ensure_root_level_alias(document, stack, event -> level) == 0) {
        return 0;
    }
    parent = stack -> parents[event -> level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    previous_last = parent -> last_field;
    if (lm_p0_parse_fields_into(document, parent, event -> text, event -> text_length, event -> line, event -> column, event -> offset) == 0) {
        return 0;
    }
    if (previous_last != 0) {
        field = previous_last -> next;
    }
    else {
        field = parent -> first_field;
    }
    while (field != 0) {
        if (field -> value != 0) {
            field->value->flags = field -> value -> flags | event -> node_flags;
        }
        field = field -> next;
    }
    return 1;
}

static int lm_p0_stream_apply_event(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event) {
    if (event -> kind == LM_P0_STREAM_EVENT_DELIM) {
        if (lm_p0_stream_resolve_pending_delimiter(document, stack, pending, event -> level) == 0) {
            return 0;
        }
        return lm_p0_pending_delimiter_set(document, pending, event);
    }
    if (lm_p0_stream_resolve_pending_delimiter(document, stack, pending, event -> level) == 0) {
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_MIX) {
        return lm_p0_stream_apply_mix_event(document, stack, event);
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_dash_fence_status_after_comment_trim(event -> text, event -> text_length, 0) == LM_P0_DASH_FENCE_VALID)) {
        if (lm_p0_pending_delimiter_set(document, pending, event) == 0) {
            return 0;
        }
        pending->event->kind = LM_P0_STREAM_EVENT_DELIM;
        return 1;
    }
    return lm_p0_stream_apply_item_event(document, stack, event);
}

static int lm_p0_pending_mix_flush(LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending_delimiter, LmP0PendingMix *pending_mix, size_t level) {
    size_t i;
    if ((pending_mix == 0) || (pending_mix -> count == 0U)) {
        return 1;
    }
    i = 0U;
    while (i < pending_mix -> count) {
        LmP0StreamEvent * event;
        event = lm_p0_stream_event_new();
        if (event == 0) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while flushing pending MIX event");
            return 0;
        }
        event[0] = pending_mix -> events[i];
        event->level = level;
        if (lm_p0_stream_apply_event(document, stack, pending_delimiter, event) == 0) {
            lm_p0_stream_event_delete(event);
            return 0;
        }
        lm_p0_stream_event_delete(event);
        i = i + 1U;
    }
    pending_mix->count = 0U;
    return 1;
}

static size_t lm_p0_stream_block_string_level(const LmP0Stack *stack, const LmP0PendingDelimiter *pending) {
    size_t level;
    if (pending -> active && (pending -> event != 0)) {
        return (pending -> event -> level + 1U);
    }
    level = lm_p0_stack_top_level(stack);
    while (level > 0U) {
        if ((stack -> parents[level] != 0) && (lm_p0_stack_level_is_trailer_body(stack, level) == 0)) {
            return level;
        }
        level = level - 1U;
    }
    return 0U;
}

static int lm_p0_validate_disabled_item_text(LmP0Document *document, const char *text, size_t length, size_t line, size_t column) {
    size_t i;
    i = 0U;
    if (lm_p0_validate_dash_fence_line(document, text, length, line, column) == 0) {
        return 0;
    }
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            if (lm_p0_scan_python_string(document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(text, length, i) == 0)) {
            if (lm_p0_scan_c_char(document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(document, text, length, &i, text[i], line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            size_t close_index;
            if (lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index) == 0) {
                return 0;
            }
            i = (close_index + 1U);
            continue;
        }
        if (text[i] == ')') {
            size_t diagnostic_line;
            size_t diagnostic_column;
            lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unmatched closing parenthesis");
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_disabled_scan_next_event(LmP0Document *document, LmP0IndentStack *indent_stack, size_t *offset, size_t *line, LmP0StreamEvent *event, int *has_event) {
    const char *source;
    size_t length;
    source = document -> source;
    length = document -> source_length;
    has_event[0] = 0;
    while (offset[0] <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;
        line_start = offset[0];
        if (line_start >= length) {
            return 1;
        }
        if (lm_p0_scan_raw_comment_block(document, source, length, line_start, line[0], offset, line)) {
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            return 0;
        }
        if (lm_p0_scan_block_string_event(document, source, length, line_start, line[0], event, offset, line)) {
            event->level = (((size_t)-1));
            has_event[0] = 1;
            return 1;
        }
        if (document -> diagnostic -> code != 0) {
            return 0;
        }
        line_end = lm_p0_find_layout_line_end(source, length, line_start);
        raw_length = (line_end - line_start);
        if ((raw_length > 0U) && (source[((line_start + raw_length) - 1U)] == '\r')) {
            raw_length = raw_length - 1U;
        }
        if (raw_length == 0U) {
            lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
            continue;
        }
        p = line_start;
        level = 0U;
        dotted_level = 0;
        if (((((line[0] == 1U) && (raw_length >= 3U)) && ((((unsigned char)source[p])) == 0xEFU)) && ((((unsigned char)source[(p + 1U)])) == 0xBBU)) && ((((unsigned char)source[(p + 2U)])) == 0xBFU)) {
            p = p + 3U;
        }
        if ((p < (line_start + raw_length)) && (source[p] == '.')) {
            dotted_level = 1;
            while ((p < (line_start + raw_length)) && (source[p] == '.')) {
                level = level + 1U;
                p = p + 1U;
                while ((p < (line_start + raw_length)) && lm_p0_is_horizontal_space(source[p])) {
                    p = p + 1U;
                }
            }
        }
        else {
            size_t indent_column;
            size_t mark_skip;
            lm_p0_scan_indent_column(source, p, (line_start + raw_length), &p, &indent_column);
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(&text, &text_length);
            mark_skip = 0U;
            if (lm_p0_skip_leading_brace_marks(document, text, text_length, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &mark_skip) == 0) {
                return 0;
            }
            p = p + mark_skip;
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(&text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
                continue;
            }
            if (lm_p0_indent_level_from_column(document, indent_stack, indent_column, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &level) == 0) {
                return 0;
            }
        }
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(&text, &text_length);
        size_t mark_skip;
        mark_skip = 0U;
        if (lm_p0_skip_leading_brace_marks(document, text, text_length, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &mark_skip) == 0) {
            return 0;
        }
        p = p + mark_skip;
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(&text, &text_length);
        if ((text_length == 0U) || (text[0] == '#')) {
            if (dotted_level && (level > 0U)) {
                text_length = 0U;
            }
            else {
                lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
                continue;
            }
        }
        memset(event, 0, sizeof(event[0]));
        event->level = level;
        event->text = text;
        event->text_length = text_length;
        event->line = line[0];
        event->column = ((((size_t)(text - (source + line_start)))) + 1U);
        event->offset = (((size_t)(text - source)));
        if ((text_length == 0U) && (level > 0U)) {
            event->kind = LM_P0_STREAM_EVENT_DELIM;
        }
        else {
            event->kind = LM_P0_STREAM_EVENT_ITEM;
        }
        lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
        has_event[0] = 1;
        return 1;
    }
    return 1;
}

static int lm_p0_disabled_event_is_tail_cutter(const LmP0StreamEvent *event) {
    if (event -> kind == LM_P0_STREAM_EVENT_DELIM) {
        return 1;
    }
    return lm_p0_stream_event_is_tail_cutter(event);
}

static int lm_p0_disabled_state_accept_event(LmP0Document *document, LmP0DisabledState *state, LmP0StreamEvent *event, int *done_after_event, int *done_before_event) {
    int is_tail_cutter;
    done_after_event[0] = 0;
    done_before_event[0] = 0;
    if (event -> level == (((size_t)-1))) {
        if (state -> pending_item) {
            event->level = (state -> pending_level + 1U);
        }
        else {
            event->level = state -> top_level;
        }
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_validate_disabled_item_text(document, event -> text, event -> text_length, event -> line, event -> column) == 0)) {
        return 0;
    }
    if (state -> pending_item) {
        if (event -> level == (state -> pending_level + 1U)) {
            state->body_started = 1;
            state->top_level = event -> level;
            state->pending_item = 0;
        }
        else {
            if (event -> level > (state -> pending_level + 1U)) {
                lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "disabled block source level jumps too deep");
                return 0;
            }
            else {
                state->pending_item = 0;
            }
        }
    }
    is_tail_cutter = lm_p0_disabled_event_is_tail_cutter(event);
    if (state -> body_started == 0) {
        if ((event -> level == state -> base_level) && is_tail_cutter) {
            done_after_event[0] = 1;
            return 1;
        }
        if (event -> level <= state -> base_level) {
            done_before_event[0] = 1;
            return 1;
        }
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "disabled block source level has no open parent");
        return 0;
    }
    if ((is_tail_cutter && (event -> level >= state -> base_level)) && ((event -> level + 1U) <= state -> top_level)) {
        state->top_level = event -> level;
        state->pending_item = 0;
        if (event -> level == state -> base_level) {
            done_after_event[0] = 1;
        }
        return 1;
    }
    if ((event -> level + 1U) < state -> top_level) {
        lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "disabled block source level decrease must be one step unless a tail cutter is used");
        return 0;
    }
    if (event -> level <= state -> base_level) {
        done_before_event[0] = 1;
        return 1;
    }
    if (event -> level < state -> top_level) {
        state->top_level = event -> level;
    }
    if (event -> level > (state -> top_level + 1U)) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "disabled block source level has no open parent");
        return 0;
    }
    if (event -> level == (state -> top_level + 1U)) {
        state->top_level = event -> level;
    }
    state->pending_item = 1;
    state->pending_level = event -> level;
    return 1;
}

static int lm_p0_validate_disabled_block(LmP0Document *document, const LmP0IndentStack *indent_stack, size_t first_next_offset, size_t first_next_line, size_t base_level, const char *header_text, size_t header_length, size_t header_line, size_t header_column, size_t *out_offset, size_t *out_line) {
    LmP0IndentStack * local_indent;
    LmP0DisabledState * state;
    LmP0StreamEvent * event;
    size_t offset;
    size_t line;
    int status;
    if (lm_p0_validate_disabled_item_text(document, header_text, header_length, header_line, header_column) == 0) {
        return 0;
    }
    local_indent = lm_p0_indent_stack_clone(document, indent_stack, header_line, header_column);
    if (local_indent == 0) {
        return 0;
    }
    state = lm_p0_disabled_state_new(base_level);
    if (state == 0) {
        lm_p0_set_diagnostic(document, 1, header_line, header_column, "out of memory while creating disabled block state");
        lm_p0_indent_stack_delete(local_indent);
        return 0;
    }
    event = lm_p0_stream_event_new();
    if (event == 0) {
        lm_p0_set_diagnostic(document, 1, header_line, header_column, "out of memory while creating disabled block event");
        lm_p0_disabled_state_delete(state);
        lm_p0_indent_stack_delete(local_indent);
        return 0;
    }
    offset = first_next_offset;
    line = first_next_line;
    status = 1;
    while (status && (offset <= document -> source_length)) {
        size_t event_offset;
        size_t event_line;
        int has_event;
        int done_after_event;
        int done_before_event;
        event_offset = offset;
        event_line = line;
        memset(event, 0, sizeof(event[0]));
        if (lm_p0_disabled_scan_next_event(document, local_indent, &offset, &line, event, &has_event) == 0) {
            status = 0;
            break;
        }
        if (has_event == 0) {
            if (state -> body_started && (state -> top_level > state -> base_level)) {
                lm_p0_set_diagnostic(document, 22, header_line, header_column, "unterminated disabled block");
                status = 0;
            }
            break;
        }
        if (lm_p0_disabled_state_accept_event(document, state, event, &done_after_event, &done_before_event) == 0) {
            status = 0;
            break;
        }
        if (done_before_event) {
            offset = event_offset;
            line = event_line;
            break;
        }
        if (done_after_event) {
            break;
        }
    }
    lm_p0_stream_event_delete(event);
    lm_p0_disabled_state_delete(state);
    lm_p0_indent_stack_delete(local_indent);
    out_offset[0] = offset;
    out_line[0] = line;
    return (status && (document -> diagnostic -> code == 0));
}

static int lm_p0_parse_stream(LmP0Document *document) {
    const char *source;
    size_t length;
    size_t offset;
    size_t line;
    LmP0IndentStack * indent_stack;
    LmP0Stack * stack;
    LmP0PendingDelimiter * pending;
    LmP0PendingMix * pending_mix;
    LmP0StreamEvent * event;
    int status;
    int has_last_physical_level;
    size_t last_physical_level;
    source = document -> source;
    length = document -> source_length;
    offset = 0U;
    line = 1U;
    status = 1;
    has_last_physical_level = 0;
    last_physical_level = 0U;
    indent_stack = 0;
    stack = 0;
    pending = 0;
    pending_mix = 0;
    event = 0;
    document->root = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (document -> root == 0) {
        return 0;
    }
    document->root->span->offset = 0U;
    document->root->span->length = document -> source_length;
    indent_stack = lm_p0_indent_stack_new(document);
    stack = lm_p0_stack_new();
    pending = lm_p0_pending_delimiter_new();
    pending_mix = lm_p0_pending_mix_new();
    if (((((indent_stack == 0) || (stack == 0)) || (pending == 0)) || (pending_mix == 0)) || (lm_p0_stack_ensure(document, stack, 0U) == 0)) {
        if ((pending == 0) || (pending_mix == 0)) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating pending stream state");
        }
        lm_p0_indent_stack_delete(indent_stack);
        lm_p0_stack_delete(stack);
        lm_p0_pending_delimiter_delete(pending);
        lm_p0_pending_mix_delete(pending_mix);
        return 0;
    }
    stack->parents[0] = document -> root -> as -> structure;
    stack->owners[0] = document -> root;
    event = lm_p0_stream_event_new();
    if (event == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating stream event");
        lm_p0_indent_stack_delete(indent_stack);
        lm_p0_stack_delete(stack);
        lm_p0_pending_delimiter_delete(pending);
        lm_p0_pending_mix_delete(pending_mix);
        return 0;
    }
    while (offset <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;
        unsigned node_flags;
        int has_leading_mix_prefix;
        size_t mix_prefix_start;
        size_t mix_prefix_end;
        line_start = offset;
        if (lm_p0_scan_raw_comment_block(document, source, length, line_start, line, &offset, &line)) {
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            status = 0;
            break;
        }
        if (lm_p0_scan_block_string_event(document, source, length, line_start, line, event, &offset, &line)) {
            event->level = lm_p0_stream_block_string_level(stack, pending);
            if (lm_p0_pending_mix_flush(document, stack, pending, pending_mix, event -> level) == 0) {
                status = 0;
                break;
            }
            if (lm_p0_stream_apply_event(document, stack, pending, event) == 0) {
                status = 0;
                break;
            }
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            status = 0;
            break;
        }
        line_end = lm_p0_find_layout_line_end(source, length, offset);
        raw_length = (line_end - line_start);
        if ((raw_length > 0U) && (source[((line_start + raw_length) - 1U)] == '\r')) {
            raw_length = raw_length - 1U;
        }
        if (raw_length == 0U) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
            continue;
        }
        p = line_start;
        level = 0U;
        dotted_level = 0;
        has_leading_mix_prefix = 0;
        mix_prefix_start = 0U;
        mix_prefix_end = 0U;
        if (((((line == 1U) && (raw_length >= 3U)) && ((((unsigned char)source[p])) == 0xEFU)) && ((((unsigned char)source[(p + 1U)])) == 0xBBU)) && ((((unsigned char)source[(p + 2U)])) == 0xBFU)) {
            p = p + 3U;
        }
        if (((p == line_start) && (raw_length > 0U)) && (source[p] == '{')) {
            int standalone_mix;
            standalone_mix = 0;
            if (lm_p0_line_is_standalone_mix_run(document, (source + p), ((line_start + raw_length) - p), line, ((((size_t)(p - line_start))) + 1U), &standalone_mix) == 0) {
                status = 0;
                break;
            }
            if (standalone_mix) {
                memset(event, 0, sizeof(event[0]));
                event->kind = LM_P0_STREAM_EVENT_MIX;
                event->text = (source + p);
                event->text_length = ((line_start + raw_length) - p);
                event->line = line;
                event->column = ((((size_t)(p - line_start))) + 1U);
                event->offset = p;
                if (lm_p0_pending_mix_push(document, pending_mix, event) == 0) {
                    status = 0;
                    break;
                }
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
                continue;
            }
        }
        size_t prefix_start;
        size_t prefix_end;
        size_t prefix_anchor;
        prefix_start = 0U;
        prefix_end = 0U;
        prefix_anchor = 0U;
        if (lm_p0_scan_leading_mix_prefix(document, (source + p), ((line_start + raw_length) - p), line, ((((size_t)(p - line_start))) + 1U), &prefix_start, &prefix_end, &prefix_anchor) == 0) {
            status = 0;
            break;
        }
        if (prefix_end > prefix_start) {
            has_leading_mix_prefix = 1;
            mix_prefix_start = (p + prefix_start);
            mix_prefix_end = (p + prefix_end);
            p = p + prefix_anchor;
        }
        if ((p < (line_start + raw_length)) && (source[p] == '.')) {
            dotted_level = 1;
            while ((p < (line_start + raw_length)) && (source[p] == '.')) {
                level = level + 1U;
                p = p + 1U;
                while ((p < (line_start + raw_length)) && lm_p0_is_horizontal_space(source[p])) {
                    p = p + 1U;
                }
            }
        }
        else {
            size_t indent_column;
            if (has_leading_mix_prefix) {
                indent_column = lm_p0_visual_column_between(source, line_start, p);
            }
            else {
                lm_p0_scan_indent_column(source, p, (line_start + raw_length), &p, &indent_column);
            }
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(&text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
                continue;
            }
            if (lm_p0_indent_level_from_column(document, indent_stack, indent_column, line, ((((size_t)(text - (source + line_start)))) + 1U), &level) == 0) {
                status = 0;
                break;
            }
        }
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(&text, &text_length);
        node_flags = 0U;
        if ((text_length == 0U) && (level == 0U)) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
            continue;
        }
        if ((text_length > 0U) && (text[0] == '#')) {
            if (dotted_level && (level > 0U)) {
                text_length = 0U;
            }
            else {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
                continue;
            }
        }
        if ((text_length > 0U) && (text[0] == '%')) {
            size_t marker_column;
            size_t header_column;
            size_t next_offset;
            size_t next_line;
            size_t disabled_next_offset;
            size_t disabled_next_line;
            size_t skip;
            node_flags = node_flags | LM_P0_NODE_INACTIVE;
            marker_column = ((((size_t)(text - (source + line_start)))) + 1U);
            skip = 1U;
            while ((skip < text_length) && lm_p0_is_horizontal_space(text[skip])) {
                skip = skip + 1U;
            }
            text = text + skip;
            text_length = text_length - skip;
            lm_p0_trim_trailing_line_comment(&text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                lm_p0_set_diagnostic(document, 21, line, marker_column, "disabled marker must be followed by a source item");
                status = 0;
                break;
            }
            header_column = ((((size_t)(text - (source + line_start)))) + 1U);
            next_offset = line_start;
            next_line = line;
            lm_p0_advance_layout_line(source, length, line_start, line_end, &next_offset, &next_line);
            if (lm_p0_validate_disabled_block(document, indent_stack, next_offset, next_line, level, text, text_length, line, header_column, &disabled_next_offset, &disabled_next_line) == 0) {
                status = 0;
                break;
            }
            memset(event, 0, sizeof(event[0]));
            event->kind = LM_P0_STREAM_EVENT_DISABLED_BLOCK;
            event->level = level;
            event->node_flags = node_flags;
            event->text = text;
            event->text_length = text_length;
            event->line = line;
            event->column = header_column;
            event->offset = (((size_t)(text - source)));
            if (lm_p0_stream_apply_event(document, stack, pending, event) == 0) {
                status = 0;
                break;
            }
            offset = disabled_next_offset;
            line = disabled_next_line;
            continue;
        }
        memset(event, 0, sizeof(event[0]));
        event->level = level;
        event->node_flags = node_flags;
        event->text = text;
        event->text_length = text_length;
        event->line = line;
        event->column = ((((size_t)(text - (source + line_start)))) + 1U);
        event->offset = (((size_t)(text - source)));
        if ((text_length == 0U) && (level > 0U)) {
            event->kind = LM_P0_STREAM_EVENT_DELIM;
        }
        else {
            event->kind = LM_P0_STREAM_EVENT_ITEM;
        }
        if (has_last_physical_level) {
            if (level > (last_physical_level + 1U)) {
                lm_p0_set_diagnostic(document, 13, line, event -> column, "source level increase must be one step");
                status = 0;
                break;
            }
            if (((level + 1U) < last_physical_level) && (lm_p0_stream_event_is_tail_cutter(event) == 0)) {
                lm_p0_set_diagnostic(document, 13, line, event -> column, "source level decrease must be one step unless a tail cutter is used");
                status = 0;
                break;
            }
        }
        if (lm_p0_pending_mix_flush(document, stack, pending, pending_mix, level) == 0) {
            status = 0;
            break;
        }
        if (has_leading_mix_prefix) {
            LmP0StreamEvent * mix_event;
            mix_event = lm_p0_stream_event_new();
            if (mix_event == 0) {
                lm_p0_set_diagnostic(document, 1, line, ((((size_t)(mix_prefix_start - line_start))) + 1U), "out of memory while creating leading MIX event");
                status = 0;
                break;
            }
            mix_event->kind = LM_P0_STREAM_EVENT_MIX;
            mix_event->level = level;
            mix_event->text = (source + mix_prefix_start);
            mix_event->text_length = (mix_prefix_end - mix_prefix_start);
            mix_event->line = line;
            mix_event->column = ((((size_t)(mix_prefix_start - line_start))) + 1U);
            mix_event->offset = mix_prefix_start;
            if (lm_p0_stream_apply_event(document, stack, pending, mix_event) == 0) {
                lm_p0_stream_event_delete(mix_event);
                status = 0;
                break;
            }
            lm_p0_stream_event_delete(mix_event);
        }
        if (lm_p0_stream_apply_event(document, stack, pending, event) == 0) {
            status = 0;
            break;
        }
        has_last_physical_level = 1;
        last_physical_level = level;
        if (line_end == length) {
            break;
        }
        lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
    }
    if (status && (lm_p0_pending_mix_flush(document, stack, pending, pending_mix, lm_p0_stream_block_string_level(stack, pending)) == 0)) {
        status = 0;
    }
    if (status && (lm_p0_stream_resolve_pending_delimiter(document, stack, pending, (((size_t)-1))) == 0)) {
        status = 0;
    }
    lm_p0_stream_event_delete(event);
    lm_p0_indent_stack_delete(indent_stack);
    lm_p0_stack_delete(stack);
    lm_p0_pending_delimiter_delete(pending);
    lm_p0_pending_mix_delete(pending_mix);
    return (status && (document -> diagnostic -> code == 0));
}

static void lm_p0_structure_recount(LmP0Structure *structure) {
    LmP0Field * field;
    structure->field_count = 0U;
    structure->last_field = 0;
    field = structure -> first_field;
    while (field != 0) {
        structure->field_count = structure -> field_count + 1U;
        structure->last_field = field;
        field = field -> next;
    }
}

static LmP0PostprocessFrame * lm_p0_postprocess_frame_new(int phase) {
    LmP0PostprocessFrame * frame;
    frame = (((LmP0PostprocessFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame != 0) {
        frame->phase = phase;
    }
    return frame;
}

static int lm_p0_postprocess_push_frame(LmOwnPtrStack *stack, LmP0PostprocessFrame *frame) {
    if ((stack == 0) || (frame == 0)) {
        lm_own_delete(frame, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    return 0;
}

static int lm_p0_postprocess_push_node(LmOwnPtrStack *stack, LmP0Node *node) {
    LmP0PostprocessFrame * frame;
    if (node == 0) {
        return 0;
    }
    frame = lm_p0_postprocess_frame_new(LM_P0_POSTPROCESS_NODE);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_postprocess_push_frame(stack, frame);
}

static int lm_p0_postprocess_push_structure(LmOwnPtrStack *stack, LmP0Structure *structure) {
    LmP0PostprocessFrame * frame;
    if (structure == 0) {
        return 1;
    }
    frame = lm_p0_postprocess_frame_new(LM_P0_POSTPROCESS_STRUCTURE);
    if (frame != 0) {
        frame->structure = structure;
        frame->field = structure -> first_field;
    }
    return lm_p0_postprocess_push_frame(stack, frame);
}

static int lm_p0_postprocess_push_trailer(LmOwnPtrStack *stack, LmP0Trailer *trailer) {
    LmP0PostprocessFrame * frame;
    if (trailer == 0) {
        return 0;
    }
    frame = lm_p0_postprocess_frame_new(LM_P0_POSTPROCESS_TRAILER);
    if (frame != 0) {
        frame->trailer = trailer;
    }
    return lm_p0_postprocess_push_frame(stack, frame);
}

static int lm_p0_postprocess_push_frame_wrap(LmOwnPtrStack *stack, LmP0Node *node) {
    LmP0PostprocessFrame * frame;
    if (node == 0) {
        return 1;
    }
    frame = lm_p0_postprocess_frame_new(LM_P0_POSTPROCESS_FRAME_WRAP);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_postprocess_push_frame(stack, frame);
}

static LmOwnPtrStack * lm_p0_postprocess_stack_new(void) {
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, lm_own_delete_plain);
    }
    return stack;
}

static void lm_p0_postprocess_stack_delete(LmOwnPtrStack **stack) {
    if ((stack != 0) && (stack[0] != 0)) {
        lm_own_ptr_stack_destroy(stack[0]);
        lm_own_delete(stack[0], 0);
        stack[0] = 0;
    }
}

static int lm_p0_postprocess_run(LmP0Document *document, LmOwnPtrStack *stack) {
    LmP0PostprocessFrame * frame;
    LmP0Node * node;
    LmP0Structure * structure;
    int status;
    status = 1;
    while (((status != 0) && (stack != 0)) && (stack -> count != 0U)) {
        frame = (((LmP0PostprocessFrame *)lm_own_ptr_stack_top(stack)));
        if (frame == 0) {
            status = 0;
            break;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_NODE) {
            node = frame -> node;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (node == 0) {
                continue;
            }
            if (node -> kind == LM_P0_NODE_FRAME) {
                status = (((lm_p0_postprocess_push_trailer(stack, node -> as -> frame -> trailer) == 0) && (lm_p0_postprocess_push_frame_wrap(stack, node) == 0)) && (lm_p0_postprocess_push_structure(stack, node -> as -> frame -> body) == 0));
            }
            else {
                if (node -> kind == LM_P0_NODE_STRUCTURE) {
                    status = ((lm_p0_postprocess_push_trailer(stack, node -> as -> structure -> trailer) == 0) && (lm_p0_postprocess_push_structure(stack, node -> as -> structure) == 0));
                }
            }
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_STRUCTURE) {
            while ((frame -> field != 0) && (frame -> field -> value == 0)) {
                frame->field = frame -> field -> next;
            }
            if (frame -> field == 0) {
                lm_p0_structure_recount(frame -> structure);
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            node = frame -> field -> value;
            frame->field = frame -> field -> next;
            status = (lm_p0_postprocess_push_node(stack, node) == 0);
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_TRAILER) {
            if (frame -> trailer == 0) {
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            structure = frame -> trailer -> body;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            status = (lm_p0_postprocess_push_structure(stack, structure) == 0);
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_FRAME_WRAP) {
            node = frame -> node;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (((node != 0) && ((node -> as -> frame -> flags & LM_P0_FRAME_INLINE_BODY) != 0U)) && (lm_p0_wrap_fields_from_line(document, node -> as -> frame -> body, node -> span -> line, (node -> span -> offset + node -> span -> length)) == 0)) {
                status = 0;
            }
            continue;
        }
        status = 0;
    }
    return status;
}

static int lm_p0_postprocess_node(LmP0Document *document, LmP0Node *node) {
    LmOwnPtrStack * stack;
    int status;
    if (node == 0) {
        return 1;
    }
    stack = lm_p0_postprocess_stack_new();
    if (stack == 0) {
        return 0;
    }
    status = ((lm_p0_postprocess_push_node(stack, node) == 0) && lm_p0_postprocess_run(document, stack));
    lm_p0_postprocess_stack_delete(&stack);
    return status;
}

static int lm_p0_wrap_fields_from_line(LmP0Document *document, LmP0Structure *structure, size_t head_line, size_t inline_event_end_offset) {
    LmP0Field * field;
    LmP0Field * previous;
    LmP0Field * group_first;
    LmP0Node * group_node;
    LmP0Field * move;
    previous = 0;
    field = structure -> first_field;
    while (field != 0) {
        if (((field -> value != 0) && (field -> value -> span -> line != head_line)) && (field -> value -> span -> offset >= inline_event_end_offset)) {
            break;
        }
        previous = field;
        field = field -> next;
    }
    if (field == 0) {
        return 1;
    }
    group_first = field;
    group_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (group_node == 0) {
        return 0;
    }
    group_node->span[0] = group_first -> value -> span[0];
    move = group_first;
    while (move != 0) {
        LmP0Field * next_move;
        LmP0Node * value;
        next_move = move -> next;
        value = move -> value;
        move->value = 0;
        if (lm_p0_append_field(document, group_node -> as -> structure, value) == 0) {
            lm_p0_free_node(group_node);
            return 0;
        }
        move = next_move;
    }
    group_first->value = group_node;
    group_first->next = 0;
    if (previous == 0) {
        structure->first_field = group_first;
    }
    else {
        previous->next = group_first;
    }
    lm_p0_structure_recount(structure);
    return 1;
}

static int lm_p0_validate_nonempty_colon_frames_in_trailer(LmP0Document *document, const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 1;
    }
    return lm_p0_validate_nonempty_colon_frames_in_structure(document, trailer -> body);
}

static int lm_p0_validate_nonempty_colon_frames_in_node(LmP0Document *document, const LmP0Node *node) {
    if (node == 0) {
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        if (lm_p0_validate_nonempty_colon_frames_in_structure(document, node -> as -> frame -> body) == 0) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(document, node -> as -> frame -> trailer);
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        if (lm_p0_validate_nonempty_colon_frames_in_structure(document, node -> as -> structure) == 0) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(document, node -> as -> structure -> trailer);
    }
    return 1;
}

static int lm_p0_validate_nonempty_colon_frames_in_structure(LmP0Document *document, const LmP0Structure *structure) {
    const LmP0Field * field;
    field = structure -> first_field;
    while (field != 0) {
        if (lm_p0_validate_nonempty_colon_frames_in_node(document, field -> value) == 0) {
            return 0;
        }
        field = field -> next;
    }
    return 1;
}

int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document) {
    LmP0Document * document;
    if (out_document == 0) {
        return 1;
    }
    out_document[0] = 0;
    document = (((LmP0Document *)lm_own_new_zero(sizeof(document[0]))));
    if (document == 0) {
        return 1;
    }
    if (lm_p0_document_init_owners(document) != 0) {
        lm_own_delete(document, 0);
        return 1;
    }
    if (source == 0) {
        source = "";
        source_length = 0U;
    }
    if (lm_p0_registry_load_default() != 0) {
        lm_p0_document_destroy_owners(document);
        lm_own_delete(document, 0);
        return 1;
    }
    document->source_length = source_length;
    document->source = lm_own_arena_copy_bytes(document -> source_owner, source, document -> source_length);
    if (document -> source == 0) {
        lm_p0_document_destroy_owners(document);
        lm_own_delete(document, 0);
        return 1;
    }
    if (lm_p0_parse_stream(document)) {
        if (lm_p0_postprocess_node(document, document -> root)) {
            lm_p0_validate_nonempty_colon_frames_in_node(document, document -> root);
        }
    }
    if (document -> diagnostic -> code == 0) {
        if (lm_own_tree_cut(document -> tree_arena) != 0) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while promoting parser lazy text edges");
        }
        else {
            lm_p0_document_freeze_tree(document);
        }
    }
    out_document[0] = document;
    if (document -> diagnostic -> code == 0) {
        return 0;
    }
    return document -> diagnostic -> code;
}

int lm_p0_parse_string(const char *source, LmP0Document **out_document) {
    if (source == 0) {
        source = "";
    }
    return lm_p0_parse_bytes(source, strlen(source), out_document);
}

int lm_p0_parse_file(const char *path, LmP0Document **out_document) {
    FILE * file;
    long size;
    char *buffer;
    size_t read_size;
    int status;
    if (out_document == 0) {
        return 1;
    }
    out_document[0] = 0;
    file = fopen(path, "rb");
    if (file == 0) {
        return 1;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return 1;
    }
    size = ftell(file);
    if (size < 0L) {
        fclose(file);
        return 1;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 1;
    }
    buffer = (((char *)lm_own_new_zero((((size_t)size)) + 1U)));
    if (buffer == 0) {
        fclose(file);
        return 1;
    }
    read_size = fread(buffer, 1U, (((size_t)size)), file);
    fclose(file);
    if (read_size != (((size_t)size))) {
        lm_own_delete(buffer, 0);
        return 1;
    }
    buffer[read_size] = '\0';
    status = lm_p0_parse_bytes(buffer, read_size, out_document);
    lm_own_delete(buffer, 0);
    return status;
}

static int lm_p0_registry_source_push_column_metadata(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    LmTableDescriptor * descriptor;
    LM_UNUSED(context);
    if (table_name == 0 || lm_p0_registry == 0 || lm_p0_registry -> view == 0) {
        return -1;
    }
    descriptor = lm_p0_registry_source_descriptor_new(table_name, columns, column_count);
    if (descriptor == 0) {
        return -1;
    }
    if (lm_registry_view_take_local_source_table(lm_p0_registry -> view, descriptor) != 0) {
        lm_table_descriptor_delete_any(descriptor);
        return -1;
    }
    return 0;
}

static int lm_p0_registry_source_push_table_row(void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells) {
    LM_UNUSED(context);
    return lm_p0_registry_materialize_source_row(table_name, columns, column_count, cells);
}

static int lm_p0_registry_source_join_table(void *context, const LmP0Text *source_table, const LmP0Text *target_table) {
    LmP0Text * source_name;
    LmP0Text * target_name;
    LmTableDescriptor * target;
    int status;
    LM_UNUSED(context);
    if (lm_p0_registry == 0 || lm_p0_registry -> view == 0 || source_table == 0 || target_table == 0) {
        return -1;
    }
    source_name = lm_p0_text_ref_new_empty();
    target_name = lm_p0_text_ref_new_empty();
    if ((source_name == 0) || (target_name == 0)) {
        lm_p0_text_ref_delete(source_name);
        lm_p0_text_ref_delete(target_name);
        return -1;
    }
    if ((lm_p0_registry_identifier_value(source_table, source_name) == 0) || (lm_p0_registry_identifier_value(target_table, target_name) == 0)) {
        lm_p0_text_ref_delete(source_name);
        lm_p0_text_ref_delete(target_name);
        return -1;
    }
    target = lm_registry_view_find_local_source_table_slice(lm_p0_registry -> view, target_name -> data, target_name -> length);
    status = -1;
    if (target != 0) {
        status = lm_registry_view_append_materialized_rows_mode(lm_p0_registry -> view, target, source_name -> data, source_name -> length, 1, 1, 0U, 0);
    }
    lm_p0_text_ref_delete(source_name);
    lm_p0_text_ref_delete(target_name);
    return status;
}

static int lm_p0_registry_source_text_all_char(const LmP0Text *text, char ch) {
    size_t i;
    if (text == 0 || text -> length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < text -> length) {
        if (text -> data[i] != ch) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_registry_source_text_is_array_receiver_head(const LmP0Text *head) {
    size_t i;
    size_t count;
    if (head == 0 || head -> length == 0U) {
        return 0;
    }
    i = 0U;
    count = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0;
        }
        i = i + 1U;
        count = count + 1U;
    }
    return count > 0U;
}

static const LmP0Text * lm_p0_registry_source_frame_receiver_key(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    if (lm_p0_registry_source_text_all_char(frame -> head, '@') != 0) {
        return lm_p0_text_from_cstr("@");
    }
    if (lm_p0_registry_source_text_is_array_receiver_head(frame -> head) != 0) {
        return lm_p0_text_from_cstr("[]");
    }
    return frame -> head;
}

static int lm_p0_registry_source_parse_size_payload(const char *payload, size_t *out_value) {
    char *end_index;
    unsigned long value;
    if (payload == 0 || payload[0] == '\0' || out_value == 0) {
        return 0;
    }
    end_index = 0;
    value = strtoul(payload, &end_index, 10);
    if (end_index == payload || end_index == 0 || end_index[0] != '\0') {
        return 0;
    }
    out_value[0] = ((size_t)value);
    return 1;
}

static int lm_p0_registry_source_formal_param_unwrap_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    const char *payload;
    const LmP0Text * key;
    LM_UNUSED(loader);
    LM_UNUSED(context);
    key = lm_p0_registry_source_frame_receiver_key(frame);
    payload = lm_p0_registry_lookup(key, "receiver.formal-param.unwrap");
    if (payload == 0) {
        return 0;
    }
    return lm_p0_registry_source_parse_size_payload(payload, out_index);
}

static int lm_p0_registry_source_positional_name_index(const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    const char *payload;
    const LmP0Text * key;
    LM_UNUSED(loader);
    LM_UNUSED(context);
    key = lm_p0_registry_source_frame_receiver_key(frame);
    payload = lm_p0_registry_lookup(key, "receiver.positional-name.index");
    if (payload == 0) {
        payload = lm_p0_registry_lookup(key, "receiver.positional-name.argument");
        if (payload == 0) {
            return 0;
        }
        out_index[0] = 0U;
        return 1;
    }
    return lm_p0_registry_source_parse_size_payload(payload, out_index);
}

static LmRegistrySourceLoader * lm_p0_registry_source_loader_new(void) {
    LmRegistrySourceLoader * loader;
    loader = (((LmRegistrySourceLoader *)lm_own_new_zero(sizeof(loader[0]))));
    if (loader == 0) {
        return 0;
    }
    loader->error_prefix = "parser";
    loader->push_column_metadata = &lm_p0_registry_source_push_column_metadata;
    loader->push_table_row = &lm_p0_registry_source_push_table_row;
    loader->join_table = &lm_p0_registry_source_join_table;
    loader->formal_param_unwrap_index = &lm_p0_registry_source_formal_param_unwrap_index;
    loader->positional_name_index = &lm_p0_registry_source_positional_name_index;
    return loader;
}

static int lm_p0_path_has_extension(const char *path, const char *extension) {
    size_t path_length;
    size_t extension_length;
    size_t i;
    char left;
    char right;
    if ((path == 0) || (extension == 0)) {
        return 0;
    }
    path_length = strlen(path);
    extension_length = strlen(extension);
    if (path_length < extension_length) {
        return 0;
    }
    i = 0U;
    while (i < extension_length) {
        left = path[((path_length - extension_length) + i)];
        right = extension[i];
        if ((left >= 'A') && (left <= 'Z')) {
            left = (((char)((left - 'A') + 'a')));
        }
        if ((right >= 'A') && (right <= 'Z')) {
            right = (((char)((right - 'A') + 'a')));
        }
        if (left != right) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_registry_require_source_only(const char *phase) {
    if (lm_p0_registry == 0 || lm_p0_registry -> view == 0) {
        return 1;
    }
    if (lm_p0_registry -> view -> parent != 0) {
        fprintf(stderr, "parser registry error: parser view is not isolated %s source-table load\n", phase);
        return 1;
    }
    if (lm_registry_view_fact_count(lm_p0_registry -> view) != 0U) {
        fprintf(stderr, "parser registry error: legacy facts appeared %s source-table load\n", phase);
        return 1;
    }
    return 0;
}

static int lm_p0_registry_parse_unsigned_payload(const char *payload, unsigned *out_value) {
    char *end_index;
    unsigned long value;
    if ((payload == 0) || (out_value == 0)) {
        return 1;
    }
    end_index = 0;
    value = strtoul(payload, &end_index, 10);
    if ((((end_index == payload) || (end_index == 0)) || (end_index[0] != '\0')) || (value > (((unsigned long)UINT_MAX)))) {
        return 1;
    }
    out_value[0] = (((unsigned)value));
    return 0;
}

static const char * lm_p0_registry_lookup_key_by_unsigned_payload(const char *table, unsigned value) {
    size_t i;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    const char *result;
    unsigned actual;
    if (((table == 0) || (lm_p0_registry == 0)) || (lm_p0_registry -> loaded == 0)) {
        return 0;
    }
    result = 0;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at(table, i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0 && payload_cell -> value != 0 && lm_p0_registry_parse_unsigned_payload(payload_cell -> value, &actual) == 0 && actual == value) {
            result = key_cell -> value;
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at(table, i, &key_cell);
    }
    return result;
}

static int lm_p0_registry_validate_abi_constant(const char *table, const char *key, unsigned expected) {
    const char *payload;
    unsigned actual;
    if (lm_p0_registry_table_has_rows(table) == 0) {
        return 0;
    }
    payload = lm_p0_registry_lookup(lm_p0_text_from_cstr(key), table);
    if (payload == 0) {
        fprintf(stderr, "parser registry error: missing ABI constant %s[%s]\n", table, key);
        return 1;
    }
    if (lm_p0_registry_parse_unsigned_payload(payload, &actual) != 0) {
        fprintf(stderr, "parser registry error: ABI constant %s[%s] expects unsigned integer payload\n", table, key);
        return 1;
    }
    if (actual != expected) {
        fprintf(stderr, "parser registry mismatch: ABI constant %s[%s] registry=%lu C=%lu\n", table, key, (((unsigned long)actual)), (((unsigned long)expected)));
        return 1;
    }
    return 0;
}

static int lm_p0_registry_validate_abi_constants(void) {
    return (((((((((((lm_p0_registry_validate_abi_constant("node.kind", "structure", LM_P0_NODE_STRUCTURE) || lm_p0_registry_validate_abi_constant("node.kind", "frame", LM_P0_NODE_FRAME)) || lm_p0_registry_validate_abi_constant("node.kind", "atom", LM_P0_NODE_ATOM)) || lm_p0_registry_validate_abi_constant("node.kind", "disabled", LM_P0_NODE_DISABLED)) || lm_p0_registry_validate_abi_constant("frame.flag", "colon", LM_P0_FRAME_COLON)) || lm_p0_registry_validate_abi_constant("frame.flag", "compact", LM_P0_FRAME_COMPACT)) || lm_p0_registry_validate_abi_constant("frame.flag", "inline-body", LM_P0_FRAME_INLINE_BODY)) || lm_p0_registry_validate_abi_constant("frame.flag", "separator-closed", LM_P0_FRAME_SEPARATOR_CLOSED)) || lm_p0_registry_validate_abi_constant("node.flag", "inactive", LM_P0_NODE_INACTIVE)) || lm_p0_registry_validate_abi_constant("node.flag", "mix", LM_P0_NODE_MIX)) || lm_p0_registry_validate_abi_constant("node.flag", "positional-skip", LM_P0_NODE_POSITIONAL_SKIP)) || lm_p0_registry_validate_abi_constant("trailer.flag", "tail-cutter", LM_P0_TRAILER_TAIL_CUTTER));
}

const char * lm_p0_node_kind_class_name(LmP0NodeKind kind) {
    const char *registry_name;
    registry_name = 0;
    if ((kind >= 0) && (lm_p0_registry_load_default() == 0)) {
        registry_name = lm_p0_registry_lookup_key_by_unsigned_payload("node.kind", (((unsigned)kind)));
    }
    if (registry_name != 0) {
        return registry_name;
    }
    if (kind == LM_P0_NODE_STRUCTURE) {
        return "structure";
    }
    if (kind == LM_P0_NODE_FRAME) {
        return "frame";
    }
    if (kind == LM_P0_NODE_ATOM) {
        return "atom";
    }
    if (kind == LM_P0_NODE_DISABLED) {
        return "disabled";
    }
    return "unknown";
}

static int lm_p0_registry_load_default(void) {
    const char *override_path;
    const char *candidates[4];
    const char *registry_path;
    LmP0Document * document;
    const LmP0Diagnostic * diagnostic;
    LmRegistrySourceLoader * loader;
    int override_enabled;
    int status;
    size_t i;
    if ((lm_p0_registry != 0) && (lm_p0_registry -> loaded || lm_p0_registry -> loading)) {
        return 0;
    }
    if (lm_p0_registry_init() != 0) {
        return 1;
    }
    if (lm_p0_registry_require_source_only("before") != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    lm_p0_registry->loading = 1;
    if (lm_p0_registry_require_source_only("during") != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    override_path = getenv("LM_P0_REGISTRY");
    override_enabled = ((override_path != 0) && (override_path[0] != '\0'));
    if (override_enabled && lm_p0_path_has_extension(override_path, ".lm2") == 0) {
        fprintf(stderr, "parser registry error: LM_P0_REGISTRY accepts .lm2 source-table modules only: %s\n", override_path);
        lm_p0_registry_destroy();
        return 1;
    }
    if (override_enabled) {
        candidates[0] = override_path;
    }
    else {
        candidates[0] = "lm2/parser_registry.lm2";
    }
    candidates[1] = "../lm2/parser_registry.lm2";
    candidates[2] = "../../lm2/parser_registry.lm2";
    candidates[3] = 0;
    document = 0;
    registry_path = 0;
    i = 0U;
    while (candidates[i] != 0) {
        registry_path = candidates[i];
        status = lm_p0_parse_file(registry_path, &document);
        if (status == 0) {
            break;
        }
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != 0) {
            fprintf(stderr, "parser registry parse error %d at %zu:%zu in %s: %s\n", diagnostic -> code, diagnostic -> line, diagnostic -> column, registry_path, diagnostic -> message);
            lm_p0_document_destroy(document);
            lm_p0_registry_destroy();
            return 1;
        }
        lm_p0_document_destroy(document);
        document = 0;
        if (override_enabled) {
            fprintf(stderr, "parser registry error: cannot read %s\n", registry_path);
            lm_p0_registry_destroy();
            return 1;
        }
        i = i + 1U;
    }
    if (document == 0) {
        if (lm_p0_registry_require_source_only("after") != 0) {
            lm_p0_registry_destroy();
            return 1;
        }
        lm_p0_registry->loaded = 1;
        lm_p0_registry->loading = 0;
        return 0;
    }
    loader = lm_p0_registry_source_loader_new();
    if (loader == 0) {
        lm_p0_document_destroy(document);
        lm_p0_registry_destroy();
        return 1;
    }
    status = lm_registry_source_load_root(loader, 0, lm_p0_document_root(document));
    lm_own_delete(loader, 0);
    lm_p0_document_destroy(document);
    if (status != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    if (lm_p0_registry_require_source_only("after") != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    lm_p0_registry->loaded = 1;
    lm_p0_registry->loading = 0;
    if (lm_p0_registry_validate_abi_constants() != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    if (lm_p0_registry_require_source_only("after validation") != 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    return 0;
}

static int lm_p0_registry_selftest_expect_path(const char *path, size_t index, const char *expected_key, const char *expected_value) {
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    payload_cell = lm_p0_registry_source_path_cell_at(path, index, &key_cell);
    if (payload_cell == 0 || key_cell == 0 || key_cell -> value == 0 || payload_cell -> value == 0) {
        return 1;
    }
    if (strcmp(key_cell -> value, expected_key) != 0 || strcmp(payload_cell -> value, expected_value) != 0) {
        return 1;
    }
    return 0;
}

static int lm_p0_registry_selftest_expect_lookup(const char *path, const char *key, const char *expected_value) {
    LmP0Text * key_text;
    const char *actual;
    int status;
    key_text = lm_p0_text_from_cstr(key);
    if (key_text == 0) {
        return 1;
    }
    actual = lm_p0_registry_lookup(key_text, path);
    status = actual == 0 || strcmp(actual, expected_value) != 0;
    lm_p0_text_view_delete(key_text);
    return status;
}

static int lm_p0_registry_source_tables_selftest(void) {
    const LmTableDescriptor * alpha;
    const LmTableDescriptor * gamma;
    const LmTableDescriptor * synthetic;
    const LmTableColumnDescriptor * column;
    const LmTableRow * alpha_row;
    const LmTableRow * gamma_row;
    const LmTableRow * none_row;
    const LmTableRow * joined_none_row;
    const LmTableCell * alpha_cell;
    const LmTableCell * gamma_cell;
    const LmTableCell * key_cell;
    const LmTableCell * none_cell;
    LmP0Text * bare_key;
    LmP0Text * incompatible_source;
    LmP0Text * incompatible_target;
    size_t index;
    size_t synthetic_row_count;
    int covered;
    int status;
    lm_p0_registry_destroy();
    if (lm_p0_registry_load_default() != 0 || lm_p0_registry == 0 || lm_p0_registry -> view == 0) {
        lm_p0_registry_destroy();
        return 1;
    }
    status = 0;
    if (lm_registry_view_fact_count(lm_p0_registry -> view) != 0U || lm_registry_view_source_table_count(lm_p0_registry -> view) != 5U) {
        status = 1;
    }
    alpha = lm_registry_view_find_local_source_table_slice(lm_p0_registry -> view, "alpha", 5U);
    gamma = lm_registry_view_find_local_source_table_slice(lm_p0_registry -> view, "gamma", 5U);
    synthetic = lm_registry_view_find_local_source_table_slice(lm_p0_registry -> view, "synthetic", 9U);
    if (status == 0 && (alpha == 0 || gamma == 0 || synthetic == 0 || lm_table_descriptor_column_count(alpha) != 4U || lm_table_descriptor_materialized_row_count(alpha) != 4U || lm_table_descriptor_column_count(gamma) != 4U || lm_table_descriptor_materialized_row_count(gamma) != 6U || lm_table_descriptor_column_count(synthetic) != 2U || lm_table_descriptor_materialized_row_count(synthetic) != 1U)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(alpha, 0U);
    if (status == 0 && (column == 0 || column -> index != 0U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "class") != 0 || strcmp(column -> type_name, "class") != 0 || lm_table_column_descriptor_descriptor_count(column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(column, 0U), "class") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(alpha, 1U);
    if (status == 0 && (column == 0 || column -> index != 1U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "left") != 0 || strcmp(column -> type_name, "char") != 0 || column -> address_depth != 0U || column -> array_rank != 0U || column -> is_const != 0 || lm_table_column_descriptor_descriptor_count(column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(column, 0U), "char") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(alpha, 2U);
    if (status == 0 && (column == 0 || column -> index != 2U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "number") != 0 || strcmp(column -> type_name, "int") != 0 || lm_table_column_descriptor_descriptor_count(column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(column, 0U), "int") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(alpha, 3U);
    if (status == 0 && (column == 0 || column -> index != 3U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "enabled") != 0 || strcmp(column -> type_name, "boolean") != 0 || lm_table_column_descriptor_descriptor_count(column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(column, 0U), "boolean") != 0)) {
        status = 1;
    }
    covered = 1;
    if (status == 0 && (lm_registry_view_source_path_has_rows(lm_p0_registry -> view, "alpha", &covered) != 0 || covered != 0)) {
        status = 1;
    }
    covered = 0;
    if (status == 0 && (lm_registry_view_source_path_has_rows(lm_p0_registry -> view, "alpha.left", &covered) == 0 || covered == 0)) {
        status = 1;
    }
    covered = 0;
    if (status == 0 && (lm_registry_view_source_path_has_key_slice(lm_p0_registry -> view, "alpha.left", 10U, "noneKey", 7U, &covered) != 0 || covered == 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_path("alpha.left", 0U, "first", "left-old") != 0 || lm_p0_registry_selftest_expect_path("alpha.left", 1U, "duplicate", "left-first") != 0 || lm_p0_registry_selftest_expect_path("alpha.left", 2U, "duplicate", "left-last") != 0 || lm_p0_registry_source_path_cell_at("alpha.left", 3U, &key_cell) != 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_path("gamma.left", 0U, "first", "left-old") != 0 || lm_p0_registry_selftest_expect_path("gamma.left", 1U, "duplicate", "left-first") != 0 || lm_p0_registry_selftest_expect_path("gamma.left", 2U, "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_path("gamma.left", 3U, "betaKey", "beta-left") != 0 || lm_p0_registry_selftest_expect_path("gamma.left", 4U, "directKey", "direct-left") != 0 || lm_p0_registry_source_path_cell_at("gamma.left", 5U, &key_cell) != 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_lookup("alpha.left", "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_lookup("alpha.number", "duplicate", "3") != 0 || lm_p0_registry_selftest_expect_lookup("gamma.left", "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_lookup("columnOrder.value", "duplicateColumn", "last-column") != 0 || lm_p0_registry_selftest_expect_lookup("synthetic", "explicitKey", "explicit-value") != 0)) {
        status = 1;
    }
    bare_key = lm_p0_text_from_cstr("duplicate");
    if (status == 0 && (bare_key == 0 || lm_p0_registry_lookup(bare_key, "alpha") != 0)) {
        status = 1;
    }
    lm_p0_text_view_delete(bare_key);
    incompatible_source = lm_p0_text_from_cstr("alpha");
    incompatible_target = lm_p0_text_from_cstr("synthetic");
    synthetic_row_count = lm_table_descriptor_materialized_row_count(synthetic);
    if (status == 0 && (incompatible_source == 0 || incompatible_target == 0 || lm_p0_registry_source_join_table(0, incompatible_source, incompatible_target) == 0 || lm_table_descriptor_materialized_row_count(synthetic) != synthetic_row_count)) {
        status = 1;
    }
    lm_p0_text_view_delete(incompatible_source);
    lm_p0_text_view_delete(incompatible_target);
    none_row = lm_table_descriptor_materialized_row_at(alpha, 2U);
    joined_none_row = lm_table_descriptor_materialized_row_at(gamma, 2U);
    index = 1U;
    while (status == 0 && index < 4U) {
        none_cell = lm_table_row_cell_at(none_row, index);
        if (none_cell == 0 || none_cell -> atom == 0 || strcmp(none_cell -> atom, "None") != 0 || none_cell -> value != 0 || none_cell -> node != 0 || none_cell -> source != 0 || none_cell -> explicit_none == 0) {
            status = 1;
        }
        none_cell = lm_table_row_cell_at(joined_none_row, index);
        if (none_cell == 0 || none_cell -> value != 0 || none_cell -> node != 0 || none_cell -> source != 0 || none_cell -> explicit_none == 0) {
            status = 1;
        }
        index = index + 1U;
    }
    alpha_row = lm_table_descriptor_materialized_row_at(alpha, 0U);
    gamma_row = lm_table_descriptor_materialized_row_at(gamma, 0U);
    alpha_cell = lm_table_row_cell_at(alpha_row, 1U);
    gamma_cell = lm_table_row_cell_at(gamma_row, 1U);
    if (status == 0 && (alpha_row == 0 || gamma_row == 0 || gamma_row == alpha_row || gamma_row -> source != alpha_row || alpha_cell == 0 || gamma_cell == 0 || alpha_cell -> atom == 0 || gamma_cell -> atom == 0 || alpha_cell -> value == 0 || gamma_cell -> value == 0 || gamma_cell == alpha_cell || alpha_cell -> atom == gamma_cell -> atom || alpha_cell -> value == gamma_cell -> value || gamma_cell -> source != 0 || strcmp(alpha_cell -> value, gamma_cell -> value) != 0)) {
        status = 1;
    }
    if (lm_registry_view_fact_count(lm_p0_registry -> view) != 0U || lm_p0_registry_require_source_only("after selftest lookups") != 0) {
        status = 1;
    }
    lm_p0_registry_destroy();
    return status;
}

void lm_p0_document_destroy(LmP0Document *document) {
    if (document == 0) {
        return;
    }
    lm_p0_document_destroy_owners(document);
    lm_own_delete(document, 0);
}

const LmP0Node * lm_p0_document_root(const LmP0Document *document) {
    if (document != 0) {
        return document -> root;
    }
    return 0;
}

LmP0Node * lm_p0_document_mutable_root(LmP0Document *document) {
    if (document != 0) {
        return document -> root;
    }
    return 0;
}

const LmP0Diagnostic * lm_p0_document_diagnostic(const LmP0Document *document) {
    if (((document == 0) || (document -> diagnostic == 0)) || (document -> diagnostic -> code == 0)) {
        return 0;
    }
    return document -> diagnostic;
}

void lm_p0_free(void *ptr) {
    lm_own_delete(ptr, 0);
}

static int lm_p0_dump_reserve(LmP0Dump *dump, size_t extra) {
    char *data;
    size_t new_capacity;
    if (dump -> failed) {
        return 0;
    }
    if (((dump -> length + extra) + 1U) <= dump -> capacity) {
        return 1;
    }
    if (dump -> capacity == 0U) {
        new_capacity = 256U;
    }
    else {
        new_capacity = dump -> capacity;
    }
    while (new_capacity < ((dump -> length + extra) + 1U)) {
        new_capacity = new_capacity * 2U;
    }
    data = (((char *)lm_own_resize(dump -> data, new_capacity)));
    if (data == 0) {
        dump->failed = 1;
        return 0;
    }
    dump->data = data;
    dump->capacity = new_capacity;
    return 1;
}

static void lm_p0_dump_append(LmP0Dump *dump, const char *text, size_t length) {
    if (lm_p0_dump_reserve(dump, length) == 0) {
        return;
    }
    memcpy((dump -> data + dump -> length), text, length);
    dump->length = dump -> length + length;
    dump->data[dump->length] = '\0';
}

static void lm_p0_dump_append_cstr(LmP0Dump *dump, const char *text) {
    lm_p0_dump_append(dump, text, strlen(text));
}

static void lm_p0_dump_indent(LmP0Dump *dump, size_t indent) {
    size_t i;
    i = 0U;
    while (i < indent) {
        lm_p0_dump_append_cstr(dump, "  ");
        i = i + 1U;
    }
}

static void lm_p0_dump_text(LmP0Dump *dump, const LmP0Text *text) {
    lm_p0_dump_append_cstr(dump, "\"");
    if (text != 0) {
        lm_p0_dump_append(dump, text -> data, text -> length);
    }
    lm_p0_dump_append_cstr(dump, "\"");
}

static LmP0DumpFrame * lm_p0_dump_frame_new(int phase, size_t indent) {
    LmP0DumpFrame * frame;
    frame = (((LmP0DumpFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame != 0) {
        frame->phase = phase;
        frame->indent = indent;
    }
    return frame;
}

static int lm_p0_dump_push_frame(LmOwnPtrStack *stack, LmP0DumpFrame *frame) {
    if ((stack == 0) || (frame == 0)) {
        lm_own_delete(frame, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    return 0;
}

static int lm_p0_dump_push_node(LmOwnPtrStack *stack, const LmP0Node *node, size_t indent) {
    LmP0DumpFrame * frame;
    if (node == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(LM_P0_DUMP_NODE, indent);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_dump_push_frame(stack, frame);
}

static int lm_p0_dump_push_structure(LmOwnPtrStack *stack, const LmP0Structure *structure, size_t indent) {
    LmP0DumpFrame * frame;
    if (structure == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(LM_P0_DUMP_STRUCTURE, indent);
    if (frame != 0) {
        frame->structure = structure;
        frame->field = structure -> first_field;
    }
    return lm_p0_dump_push_frame(stack, frame);
}

static int lm_p0_dump_push_trailer(LmOwnPtrStack *stack, const LmP0Trailer *trailer, size_t indent) {
    LmP0DumpFrame * frame;
    if (trailer == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(LM_P0_DUMP_TRAILER, indent);
    if (frame != 0) {
        frame->trailer = trailer;
    }
    return lm_p0_dump_push_frame(stack, frame);
}

static LmOwnPtrStack * lm_p0_dump_stack_new(void) {
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, lm_own_delete_plain);
    }
    return stack;
}

static void lm_p0_dump_stack_delete(LmOwnPtrStack **stack) {
    if ((stack != 0) && (stack[0] != 0)) {
        lm_own_ptr_stack_destroy(stack[0]);
        lm_own_delete(stack[0], 0);
        stack[0] = 0;
    }
}

static void lm_p0_dump_run(LmP0Dump *dump, LmOwnPtrStack *stack) {
    LmP0DumpFrame * frame;
    const LmP0Node * node;
    const LmP0Structure * structure;
    const LmP0Trailer * trailer;
    const char *structure_name;
    size_t indent;
    while ((((dump != 0) && (dump -> failed == 0)) && (stack != 0)) && (stack -> count != 0U)) {
        frame = (((LmP0DumpFrame *)lm_own_ptr_stack_top(stack)));
        if (frame == 0) {
            dump->failed = 1;
            return;
        }
        if (frame -> phase == LM_P0_DUMP_NODE) {
            node = frame -> node;
            indent = frame -> indent;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (node == 0) {
                continue;
            }
            structure_name = lm_p0_node_kind_class_name(LM_P0_NODE_STRUCTURE);
            lm_p0_dump_indent(dump, indent);
            if ((node -> flags & LM_P0_NODE_INACTIVE) != 0U) {
                lm_p0_dump_append_cstr(dump, "Inactive ");
            }
            if ((node -> flags & LM_P0_NODE_MIX) != 0U) {
                lm_p0_dump_append_cstr(dump, "MIX ");
            }
            if ((node -> flags & LM_P0_NODE_POSITIONAL_SKIP) != 0U) {
                lm_p0_dump_append_cstr(dump, "PositionalSkip ");
            }
            if (node -> kind == LM_P0_NODE_STRUCTURE) {
                lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(node -> kind));
                lm_p0_dump_append_field_count_line(dump, node -> as -> structure -> field_count);
                if ((lm_p0_dump_push_trailer(stack, node -> as -> structure -> trailer, (indent + 1U)) != 0) || (lm_p0_dump_push_structure(stack, node -> as -> structure, (indent + 1U)) != 0)) {
                    dump->failed = 1;
                }
            }
            else {
                if (node -> kind == LM_P0_NODE_FRAME) {
                    lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(node -> kind));
                    lm_p0_dump_append_cstr(dump, " head=");
                    lm_p0_dump_text(dump, node -> as -> frame -> head);
                    lm_p0_dump_append_cstr(dump, " body=");
                    lm_p0_dump_append_cstr(dump, structure_name);
                    lm_p0_dump_append_field_count_line(dump, node -> as -> frame -> body -> field_count);
                    if ((lm_p0_dump_push_trailer(stack, node -> as -> frame -> trailer, (indent + 1U)) != 0) || (lm_p0_dump_push_structure(stack, node -> as -> frame -> body, (indent + 1U)) != 0)) {
                        dump->failed = 1;
                    }
                }
                else {
                    if (node -> kind == LM_P0_NODE_ATOM) {
                        lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(node -> kind));
                        lm_p0_dump_append_cstr(dump, " ");
                        lm_p0_dump_text(dump, node -> as -> atom);
                        lm_p0_dump_append_cstr(dump, "\n");
                    }
                    else {
                        if (node -> kind == LM_P0_NODE_DISABLED) {
                            lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(node -> kind));
                            lm_p0_dump_append_cstr(dump, " ");
                            lm_p0_dump_text(dump, node -> as -> atom);
                            lm_p0_dump_append_cstr(dump, "\n");
                        }
                        else {
                            lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(node -> kind));
                            lm_p0_dump_append_cstr(dump, " kind=");
                            lm_p0_dump_append_size(dump, (((size_t)node -> kind)));
                            lm_p0_dump_append_cstr(dump, "\n");
                        }
                    }
                }
            }
            continue;
        }
        if (frame -> phase == LM_P0_DUMP_STRUCTURE) {
            while ((frame -> field != 0) && (frame -> field -> value == 0)) {
                frame->field = frame -> field -> next;
            }
            if (frame -> field == 0) {
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            node = frame -> field -> value;
            frame->field = frame -> field -> next;
            if (lm_p0_dump_push_node(stack, node, frame -> indent) != 0) {
                dump->failed = 1;
            }
            continue;
        }
        if (frame -> phase == LM_P0_DUMP_TRAILER) {
            trailer = frame -> trailer;
            indent = frame -> indent;
            if (trailer != 0) {
                structure = trailer -> body;
            }
            else {
                structure = 0;
            }
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (trailer == 0) {
                continue;
            }
            lm_p0_dump_indent(dump, indent);
            if ((trailer -> flags & LM_P0_TRAILER_TAIL_CUTTER) != 0U) {
                lm_p0_dump_append_cstr(dump, "Tail cutter trailer spelling=");
            }
            else {
                lm_p0_dump_append_cstr(dump, "Trailer spelling=");
            }
            lm_p0_dump_text(dump, trailer -> spelling);
            lm_p0_dump_append_field_count_line(dump, trailer -> body -> field_count);
            if (lm_p0_dump_push_structure(stack, structure, (indent + 1U)) != 0) {
                dump->failed = 1;
            }
            continue;
        }
        dump->failed = 1;
    }
}

static void lm_p0_dump_node(LmP0Dump *dump, const LmP0Node *node, size_t indent) {
    LmOwnPtrStack * stack;
    if (node == 0) {
        return;
    }
    stack = lm_p0_dump_stack_new();
    if (stack == 0) {
        dump->failed = 1;
        return;
    }
    if (lm_p0_dump_push_node(stack, node, indent) != 0) {
        dump->failed = 1;
    }
    else {
        lm_p0_dump_run(dump, stack);
    }
    lm_p0_dump_stack_delete(&stack);
}

static LmP0Dump * lm_p0_dump_new(void) {
    return (((LmP0Dump *)lm_own_new_zero(1U * sizeof(LmP0Dump))));
}

static char * lm_p0_dump_take_data(LmP0Dump *dump) {
    char *data;
    if (dump == 0) {
        return 0;
    }
    data = dump -> data;
    dump->data = 0;
    dump->length = 0U;
    dump->capacity = 0U;
    return data;
}

static void lm_p0_dump_delete(LmP0Dump *dump) {
    if (dump != 0) {
        lm_own_delete(dump -> data, 0);
        lm_own_delete(dump, 0);
    }
}

char * lm_p0_dump_alloc(const LmP0Document *document) {
    LmP0Dump * dump;
    char *data;
    dump = lm_p0_dump_new();
    if (dump == 0) {
        return 0;
    }
    if ((document == 0) || (document -> root == 0)) {
        lm_p0_dump_append_cstr(dump, "");
        data = lm_p0_dump_take_data(dump);
        lm_p0_dump_delete(dump);
        return data;
    }
    lm_p0_dump_node(dump, document -> root, 0U);
    if (dump -> failed) {
        lm_p0_dump_delete(dump);
        return 0;
    }
    data = lm_p0_dump_take_data(dump);
    lm_p0_dump_delete(dump);
    return data;
}
