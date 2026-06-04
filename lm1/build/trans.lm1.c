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
typedef struct LmL4Column LmL4Column;
typedef struct LmL4Receiver LmL4Receiver;
typedef struct LmL4Loader LmL4Loader;
typedef struct LmTransIdentifierRelation LmTransIdentifierRelation;
typedef struct LmTransIdentifierCard LmTransIdentifierCard;
typedef struct LmTransNamespace LmTransNamespace;
typedef struct LmTransExprSegment LmTransExprSegment;
typedef struct LmTransCallLowering LmTransCallLowering;
typedef struct LmTransExprAtomLowering LmTransExprAtomLowering;
typedef struct LmTransExprLoweredRange LmTransExprLoweredRange;
typedef struct LmTransExprPiece LmTransExprPiece;
typedef struct LmTransExprJob LmTransExprJob;
typedef struct LmTransExprStack LmTransExprStack;
typedef struct LmTransStatementStack LmTransStatementStack;
typedef struct LmTransStatementJob LmTransStatementJob;
typedef struct LmTransFunctionHeader LmTransFunctionHeader;
typedef struct LmTransL4HeadBinding LmTransL4HeadBinding;
typedef struct LmTransL4AtomBinding LmTransL4AtomBinding;
typedef struct LmTransTopLevelItem LmTransTopLevelItem;

typedef int LmOwnEdgeKind;
typedef int LmP0NodeKind;
typedef unsigned LmP0FrameFlags;
typedef unsigned LmP0NodeFlags;
typedef unsigned LmP0TrailerFlags;
typedef LmL4Column LmTransRegistryColumn;

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

#include <stddef.h>

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
    LmOwnArena *owner;
    size_t bytes;
    size_t element_size;
    size_t count;
    size_t rank;
    size_t level;
};
struct LmOwnLazyEdge {
    LmOwnEdgeKind kind;
    LmOwnArena *source_owner;
    LmOwnArena *target_owner;
    const void *source;
    size_t size;
    const void **patch_slot;
};
struct LmOwnArena {
    LmOwnPtrStack allocations;
    LmOwnValueStack allocation_descriptors;
    LmOwnValueStack lazy_edges;
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
    LmP0Field *first_field;
    LmP0Field *last_field;
    size_t field_count;
    LmP0Trailer *trailer;
} LmP0Structure;
struct LmP0Trailer {
    LmP0Text spelling;
    unsigned flags;
    LmP0Structure body;
};
typedef struct LmP0Frame {
    LmP0Text head;
    unsigned flags;
    LmP0Structure body;
    LmP0Trailer *trailer;
} LmP0Frame;
struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span span;
    union {
        LmP0Structure structure;
        LmP0Frame frame;
        LmP0Text atom;
    } as;
};
struct LmP0Field {
    LmP0Node *value;
    LmP0Field *next;
};
struct LmL4Column {
    LmP0Text name;
    LmP0Text descriptors[16U];
    size_t descriptor_count;
};
struct LmL4Receiver {
    const char *name;
    int (*frame)(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
};
struct LmL4Loader {
    const char *error_prefix;
    int (*push_row)(void *context, LmP0Text table_atom, LmP0Text key_atom, const LmP0Node *payload_node);
    int (*push_cell)(void *context, LmP0Text table_name, const LmL4Column *column, int split_by_column, LmP0Text key_atom, const LmP0Node *payload_node);
    int (*note_key)(void *context, LmP0Text table_name, const LmL4Column *column, LmP0Text key_atom);
    int (*push_column_metadata)(void *context, LmP0Text table_name, const LmL4Column *columns, size_t column_count);
    const LmL4Receiver *receivers;
    size_t receiver_count;
};
struct LmTransIdentifierRelation {
    const char *name;
    LmOwnPtrStack symbols;
    LmTransIdentifierRelation *next;
};
struct LmTransIdentifierCard {
    LmP0Text *name;
    char *name_storage;
    unsigned long hash;
    LmOwnPtrStack symbols;
    LmTransIdentifierRelation **relation_buckets;
    size_t relation_bucket_count;
    LmTransIdentifierCard *next;
};
typedef struct LmTransIdentifierTable {
    LmTransIdentifierCard **buckets;
    size_t bucket_count;
    size_t count;
} LmTransIdentifierTable;
typedef struct LmTransRegistry {
    LmTransIdentifierTable identifiers;
    LmOwnArena value_arena;
    LmOwnPtrStack loaded_paths;
    char *source_path;
    size_t loaded_fact_count;
    int loaded;
} LmTransRegistry;
typedef struct LmTransRegistryFact {
    char *table;
    char *key;
    char *payload;
    const LmP0Node *payload_node;
} LmTransRegistryFact;
typedef struct LmTransLayoutField {
    const char *name;
    const char *class_name;
    const char *union_layout_name;
    size_t index;
    size_t address_depth;
    size_t array_count;
    int is_const;
    int has_array_count;
    int is_union;
} LmTransLayoutField;
typedef struct LmTransCDeclarator {
    const LmP0Node *type_node;
    const LmP0Field *expression_dimensions;
    LmP0Text type_head;
    LmP0Text name;
    LmP0Text array_head;
    size_t pointer_depth;
    size_t literal_dimensions[8U];
    size_t literal_dimension_count;
    int type_is_head;
} LmTransCDeclarator;
typedef struct LmTransAbiParam {
    const char *name;
    const char *class_name;
    size_t index;
    size_t address_depth;
    int is_const;
} LmTransAbiParam;
typedef struct LmTransL4CallableType {
    LmP0Text class_name;
    size_t address_depth;
    int is_const;
} LmTransL4CallableType;
typedef struct LmTransSymbol {
    LmP0Text *name;
    char *name_storage;
    const char *class_name;
    LmP0Text c_name;
    char *c_name_storage;
    int has_c_name;
    LmP0Text env_arg;
    char *env_arg_storage;
    int has_env_arg;
    LmP0Text closure_call_name;
    char *closure_call_name_storage;
    int has_closure_call_name;
    unsigned depth;
    LmOwnPtrStack param_names;
    int has_signature;
    const LmP0Node *callable_params_node;
    const LmP0Node *callable_return_node;
    int has_callable_shape;
    int callable_returns_value;
    int callable_is_struct_return;
    int callable_is_external;
} LmTransSymbol;
typedef struct LmTransCleanup {
    unsigned id;
} LmTransCleanup;
typedef struct LmTransLoop {
    size_t cleanup_base;
} LmTransLoop;
struct LmTransExprSegment {
    const LmP0Field *first;
    const LmP0Field *stop;
    const LmP0Node *expected_param;
    int present;
};
typedef struct LmTransStructReturnFieldValue {
    const LmP0Field *first;
    const LmP0Field *stop;
    int present;
} LmTransStructReturnFieldValue;
struct LmTransCallLowering {
    LmP0Text name;
    const LmTransSymbol *signature;
    int is_closure;
};
struct LmTransExprLoweredRange {
    LmOwnValueStack pieces;
    size_t index;
};
struct LmTransExprStack {
    LmOwnValueStack jobs;
};
typedef struct LmTransExprRangeJob {
    const LmP0Field *field;
    const LmP0Field *stop;
    int wrote;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
} LmTransExprRangeJob;
typedef struct LmTransExprCallArgsJob {
    const LmP0Structure *body;
    const LmTransSymbol *callee;
} LmTransExprCallArgsJob;
struct LmTransExprAtomLowering {
    int (*emit)(FILE *file, const LmTransExprAtomLowering *lowering, const LmTransNamespace *namespace_);
    int (*update)(const LmTransExprAtomLowering *lowering, const LmP0Node *node, const LmP0Node **previous_operand, int *expect_field_name, int *expect_c_field_name, int *c_dot_path);
    LmP0Text text;
};
struct LmTransExprPiece {
    int (*emit)(FILE *file, LmTransExprStack *stack, LmTransExprLoweredRange *lowered, const LmTransExprPiece *piece, const LmTransNamespace *namespace_, int *out_suspend);
    int leading_space;
    const LmP0Node *node;
    LmTransExprAtomLowering atom;
    const LmP0Field *first;
    const LmP0Field *stop;
};
struct LmTransExprJob {
    int (*run)(FILE *file, LmTransExprStack *stack, LmTransExprJob *job, const LmTransNamespace *namespace_);
    void (*destroy)(LmTransExprJob *job);
    union {
        const char *text;
        LmP0Text name_text;
        const LmP0Node *node;
        const LmP0Frame *frame;
        LmTransExprRangeJob range;
        LmTransExprCallArgsJob call_args;
        LmTransExprLoweredRange *lowered_range;
    } as;
};
typedef struct LmTransStatementListJob {
    const LmP0Field *field;
    unsigned indent;
    int unwrap_single_structure;
    const LmP0Frame *repeat_frame;
} LmTransStatementListJob;
typedef struct LmTransStatementNodeJob {
    const LmP0Node *node;
    unsigned indent;
    const LmP0Frame *repeat_frame;
} LmTransStatementNodeJob;
typedef struct LmTransStatementFrameJob {
    const LmP0Frame *frame;
    unsigned indent;
} LmTransStatementFrameJob;
typedef struct LmTransStatementTextJob {
    unsigned indent;
    const char *text;
} LmTransStatementTextJob;
typedef struct LmTransStatementSyncLeaveJob {
    unsigned indent;
    unsigned cleanup_id;
} LmTransStatementSyncLeaveJob;
struct LmTransStatementStack {
    LmOwnValueStack jobs;
};
struct LmTransStatementJob {
    int (*run)(FILE *file, LmTransStatementStack *stack, LmTransStatementJob *job, LmTransNamespace *namespace_);
    union {
        LmTransStatementListJob list;
        LmTransStatementNodeJob node;
        LmTransStatementFrameJob frame;
        LmTransStatementTextJob text;
        LmTransStatementSyncLeaveJob sync_leave;
    } as;
};
typedef struct LmTransStatementLowering {
    int (*emit)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
} LmTransStatementLowering;
typedef struct LmTransFunctionState {
    const LmP0Node *previous_return_type_node;
    int previous_return_type_is_struct;
    LmP0Text *previous_return_type_name;
    LmP0Text *current_return_type_name;
    unsigned previous_next_return_id;
    const LmOwnPtrStack *previous_hoisted_functions;
    LmOwnPtrStack previous_cleanups;
    LmOwnPtrStack previous_loops;
    int has_previous_control_stacks;
} LmTransFunctionState;
struct LmTransFunctionHeader {
    const LmP0Frame *frame;
    LmP0Text name;
    LmP0Text c_name;
    LmP0Text env_type_name;
    const LmP0Node *params_node;
    const LmP0Node *return_node;
    const LmP0Field *body_start;
    const LmOwnPtrStack *captures;
    const char *symbol_class;
    int is_sub;
    int is_struct_return;
    int is_external;
    int is_descriptor_only;
    int declare_self_alias;
    int has_env;
};
typedef struct LmTransCapture {
    LmP0Text name;
    LmP0Text type_head;
    const LmP0Node *type_node;
    size_t pointer_depth;
    int type_is_head;
} LmTransCapture;
typedef struct LmTransHoistedFunction {
    LmTransFunctionHeader function;
    LmOwnPtrStack captures;
    char *c_name_storage;
    char *env_type_storage;
    char *env_var_storage;
    char *closure_call_storage;
    LmP0Text env_var_name;
    LmP0Text closure_call_name;
} LmTransHoistedFunction;
typedef struct LmTransBinding {
    int (*call_lowering)(LmP0Text head, const LmTransSymbol *symbol, LmTransCallLowering *out);
    int (*expr_emit)(FILE *file, const LmTransExprAtomLowering *lowering, const LmTransNamespace *namespace_);
    int (*expr_state)(const LmTransExprAtomLowering *lowering, const LmP0Node *node, const LmP0Node **previous_operand, int *expect_field_name, int *expect_c_field_name, int *c_dot_path);
    int (*statement_frame)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
    int (*function_receiver)(const LmP0Frame *frame, int is_external, LmTransFunctionHeader *out);
    int (*type_emit)(FILE *file, const LmP0Node *type_node);
    int (*type_structure_value_alloc)(FILE *file, unsigned indent, const LmP0Node *type_node, LmP0Text target_name, const LmP0Structure *value, int *out_consumed, int *out_needs_null_check);
    int (*type_structure_value_fill)(FILE *file, unsigned indent, const LmP0Node *type_node, LmP0Text target_name, const LmP0Structure *value, const LmTransNamespace *namespace_, int *out_consumed);
    int (*expr_segment_materializer)(FILE *file, LmTransExprStack *stack, const LmTransExprSegment *segment, const LmTransNamespace *namespace_, int *out_consumed);
    int (*l4_frame)(const LmP0Frame *frame, int allow_node_cells);
    int (*l4_atom)(LmP0Text atom, int allow_node_cells);
    int (*l4_payload_frame)(FILE *output, const LmP0Frame *frame, LmTransNamespace *namespace_);
} LmTransBinding;
typedef struct LmTransHeadBinding {
    const LmTransSymbol *symbol;
    const char *receiver_type;
    const char *function_receiver_binding;
    const char *statement_receiver_binding;
    int (*function_receiver)(const LmP0Frame *frame, int is_external, LmTransFunctionHeader *out);
    int (*statement_frame)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
} LmTransHeadBinding;
struct LmTransL4HeadBinding {
    const char *receiver_type;
    const char *receiver_binding;
    int (*frame)(const LmP0Frame *frame, int allow_node_cells);
};
struct LmTransL4AtomBinding {
    const char *receiver_type;
    const char *receiver_binding;
    int (*atom)(LmP0Text atom, int allow_node_cells);
};
struct LmTransTopLevelItem {
    int (*declare)(LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_before_functions)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_function)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int emits_top_level;
    const LmP0Node *node;
    const LmP0Frame *frame;
    LmTransFunctionHeader function;
    int (*emit_prototype)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_after_prototypes)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
};
struct LmTransNamespace {
    LmOwnPtrStack items;
    LmTransIdentifierTable identifiers;
    const LmTransIdentifierTable *registry_identifiers;
    unsigned depth;
    LmOwnPtrStack cleanups;
    LmOwnPtrStack loops;
    unsigned next_cleanup_id;
    const LmP0Node *return_type_node;
    int return_type_is_struct;
    LmP0Text *return_type_name;
    unsigned next_return_id;
    const LmOwnPtrStack *hoisted_functions;
};
typedef struct LmTransL4LoadContext {
    int allow_node_cells;
} LmTransL4LoadContext;

typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);
typedef int (*LmL4PushRow)(void *context, LmP0Text table_atom, LmP0Text key_atom, const LmP0Node *payload_node);
typedef int (*LmL4PushCell)(void *context, LmP0Text table_name, const LmL4Column *column, int split_by_column, LmP0Text key_atom, const LmP0Node *payload_node);
typedef int (*LmL4NoteKey)(void *context, LmP0Text table_name, const LmL4Column *column, LmP0Text key_atom);
typedef int (*LmL4PushColumnMetadata)(void *context, LmP0Text table_name, const LmL4Column *columns, size_t column_count);
typedef int (*LmL4FrameReceiver)(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
typedef int (*LmTransExprSegmentMaterializer)(FILE *file, LmTransExprStack *stack, const LmTransExprSegment *segment, const LmTransNamespace *namespace_, int *out_consumed);
typedef int (*LmTransCallLoweringHandler)(LmP0Text head, const LmTransSymbol *symbol, LmTransCallLowering *out);
typedef int (*LmTransExprAtomEmitHandler)(FILE *file, const LmTransExprAtomLowering *lowering, const LmTransNamespace *namespace_);
typedef int (*LmTransExprAtomStateHandler)(const LmTransExprAtomLowering *lowering, const LmP0Node *node, const LmP0Node **previous_operand, int *expect_field_name, int *expect_c_field_name, int *c_dot_path);
typedef int (*LmTransExprPieceEmitHandler)(FILE *file, LmTransExprStack *stack, LmTransExprLoweredRange *lowered, const LmTransExprPiece *piece, const LmTransNamespace *namespace_, int *out_suspend);
typedef int (*LmTransExprJobHandler)(FILE *file, LmTransExprStack *stack, LmTransExprJob *job, const LmTransNamespace *namespace_);
typedef void (*LmTransExprJobDestroyHandler)(LmTransExprJob *job);
typedef int (*LmTransStatementJobHandler)(FILE *file, LmTransStatementStack *stack, LmTransStatementJob *job, LmTransNamespace *namespace_);
typedef int (*LmTransStatementFrameHandler)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
typedef int (*LmTransAtomStatementHandler)(FILE *file, const LmP0Node *node, unsigned indent, LmTransNamespace *namespace_);
typedef int (*LmTransFunctionHeaderReceiver)(const LmP0Frame *frame, int is_external, LmTransFunctionHeader *out);
typedef int (*LmTransTypeEmitReceiver)(FILE *file, const LmP0Node *type_node);
typedef int (*LmTransTypeStructureValueAllocReceiver)(FILE *file, unsigned indent, const LmP0Node *type_node, LmP0Text target_name, const LmP0Structure *value, int *out_consumed, int *out_needs_null_check);
typedef int (*LmTransTypeStructureValueFillReceiver)(FILE *file, unsigned indent, const LmP0Node *type_node, LmP0Text target_name, const LmP0Structure *value, const LmTransNamespace *namespace_, int *out_consumed);
typedef int (*LmTransTopLevelDeclareHandler)(LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
typedef int (*LmTransTopLevelEmitHandler)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
typedef int (*LmTransL4FrameHandler)(const LmP0Frame *frame, int allow_node_cells);
typedef int (*LmTransL4AtomHandler)(LmP0Text atom, int allow_node_cells);
typedef int (*LmTransL4PayloadFrameHandler)(FILE *output, const LmP0Frame *frame, LmTransNamespace *namespace_);

void * lm_own_new_zero(size_t size);
void * lm_own_array_new_zero(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_allocation_descriptor(const void *address);
void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);
void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack);
int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item);
void * lm_own_ptr_stack_pop(LmOwnPtrStack *stack);
void * lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index);
void * lm_own_ptr_stack_top(const LmOwnPtrStack *stack);
void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count);
void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size);
void lm_own_value_stack_destroy(LmOwnValueStack *stack);
int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item);
int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count);
int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item);
void * lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index);
void * lm_own_value_stack_top(const LmOwnValueStack *stack);
void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count);
void lm_own_arena_init(LmOwnArena *arena);
void lm_own_arena_destroy(LmOwnArena *arena);
void * lm_own_arena_new_zero(LmOwnArena *arena, size_t size);
void * lm_own_arena_array_new_zero(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(const LmOwnArena *arena, const void *address);
char * lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length);
int lm_own_arena_add_lazy_edge(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int lm_own_arena_promote_lazy_edges(LmOwnArena *arena);
int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source);
void lm_own_arena_freeze(LmOwnArena *arena);
int lm_own_arena_is_frozen(const LmOwnArena *arena);
int lm_own_tree_cut(LmOwnArena *arena);
int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena);
int lm_p0_parse_string(const char *source, LmP0Document **out_document);
int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document);
int lm_p0_parse_file(const char *path, LmP0Document **out_document);
void lm_p0_document_destroy(LmP0Document *document);
const LmP0Node * lm_p0_document_root(const LmP0Document *document);
const LmP0Diagnostic * lm_p0_document_diagnostic(const LmP0Document *document);
const char * lm_p0_node_kind_class_name(LmP0NodeKind kind);
char * lm_p0_dump_alloc(const LmP0Document *document);
void lm_p0_free(void *ptr);

static LmOwnPtrStack *lm_l4_seen_tables;
static LmOwnPtrStack *lm_l4_seen_tables_get(void) {
    return lm_l4_seen_tables;
}
static void lm_l4_seen_tables_set(LmOwnPtrStack *seen_tables) {
    lm_l4_seen_tables = seen_tables;
}
static int lm_l4_text_equals(LmP0Text text, const char *value);
static int lm_l4_text_same(LmP0Text left, LmP0Text right);
static int lm_l4_node_is_ignored(const LmP0Node *node);
static const char * lm_l4_error_prefix(const LmL4Loader *loader);
static void lm_l4_error(const LmL4Loader *loader, const char *message);
static const LmP0Field * lm_l4_nth_field(const LmP0Structure *structure, size_t index);
static int lm_l4_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text);
static int lm_l4_identifier_value(LmP0Text atom, LmP0Text *out_payload);
static int lm_l4_frame_single_atom(const LmP0Frame *frame, const char *head, LmP0Text *out_atom);
static int lm_l4_column_name(const LmP0Field *field, LmL4Column *out_column);
static int lm_l4_columns_from_frame(const LmL4Loader *loader, const LmP0Frame *frame, LmL4Column *columns, size_t columns_capacity, size_t *out_count);
static int lm_l4_validate_named_trailer(const LmL4Loader *loader, const LmP0Frame *frame, LmP0Text expected_name);
static int lm_l4_row_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_rows_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame, LmP0Text table_name, const LmL4Column *columns, size_t column_count);
static int lm_l4_table_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_seen_table_add(LmOwnPtrStack *seen, LmP0Text table_name);
static int lm_l4_check_table_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_l4_receiver_table(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_receiver_row(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static const LmL4Receiver * lm_l4_find_receiver(const LmL4Loader *loader, LmP0Text head);
static int lm_l4_dispatch_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_load_rows(const LmL4Loader *loader, void *context, const LmP0Structure *structure);
static int lm_l4_load_root(const LmL4Loader *loader, void *context, const LmP0Node *root, int implicit_l4, size_t *out_row_count);

static int lm_l4_text_equals(LmP0Text text, const char *value) {
    size_t length;
    if (value == 0) {
        return 0;
    }
    length = strlen(value);
    return text.length == length && memcmp(text.data, value, length) == 0;
}

static int lm_l4_text_same(LmP0Text left, LmP0Text right) {
    if (left.length != right.length) {
        return 0;
    }
    if (left.length == 0U) {
        return 1;
    }
    return memcmp(left.data, right.data, left.length) == 0;
}

static int lm_l4_node_is_ignored(const LmP0Node *node) {
    return node == 0 || (node -> flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static const char * lm_l4_error_prefix(const LmL4Loader *loader) {
    if (loader != 0 && loader -> error_prefix != 0) {
        return loader -> error_prefix;
    }
    return "L4";
}

static void lm_l4_error(const LmL4Loader *loader, const char *message) {
    fprintf(stderr, "%s registry error: %s\n", lm_l4_error_prefix(loader), message);
}

static const LmP0Field * lm_l4_nth_field(const LmP0Structure *structure, size_t index) {
    const LmP0Field *field;
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

static int lm_l4_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text) {
    const LmP0Field *field;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = trailer->body.first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    out_text[0] = field->value->as.atom;
    return 1;
}

static int lm_l4_identifier_value(LmP0Text atom, LmP0Text *out_payload) {
    if (out_payload == 0 || atom.data == 0) {
        return 0;
    }
    if (atom.length >= 2U && atom.data[0] == '`' && atom.data[atom.length - 1U] == '`') {
        out_payload[0] = atom;
        out_payload->data = atom.data + 1U;
        out_payload->length = atom.length - 2U;
        return 1;
    }
    out_payload[0] = atom;
    return 1;
}

static int lm_l4_frame_single_atom(const LmP0Frame *frame, const char *head, LmP0Text *out_atom) {
    const LmP0Field *field;
    if (frame == 0 || out_atom == 0) {
        return 0;
    }
    if (head != 0 && lm_l4_text_equals(frame -> head, head) == 0) {
        return 0;
    }
    field = lm_l4_nth_field(&frame->body, 0U);
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return - 1;
    }
    out_atom[0] = field->value->as.atom;
    return 1;
}

static int lm_l4_column_name(const LmP0Field *field, LmL4Column *out_column) {
    const LmP0Node *node;
    const LmP0Field *body_field;
    size_t descriptor_count;
    if (field == 0 || out_column == 0) {
        return 0;
    }
    memset(out_column, 0, sizeof(out_column[0]));
    node = field -> value;
    if (node == 0) {
        return - 1;
    }
    if (node -> kind == LM_P0_NODE_ATOM) {
        out_column->name = node->as.atom;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        if (lm_l4_frame_single_atom(&node->as.frame, 0, &out_column->name) <= 0) {
            return - 1;
        }
        out_column->descriptors[0] = node->as.frame.head;
        out_column->descriptor_count = 1U;
        return 1;
    }
    if (node -> kind != LM_P0_NODE_STRUCTURE) {
        return - 1;
    }
    body_field = node->as.structure.first_field;
    while (body_field != 0 && lm_l4_node_is_ignored(body_field -> value) != 0) {
        body_field = body_field -> next;
    }
    if (body_field == 0 || body_field -> value == 0 || body_field -> value -> kind != LM_P0_NODE_ATOM) {
        return - 1;
    }
    out_column->name = body_field->value->as.atom;
    descriptor_count = 0U;
    body_field = body_field -> next;
    while (body_field != 0) {
        if (body_field -> value != 0 && lm_l4_node_is_ignored(body_field -> value) == 0) {
            if (body_field -> value -> kind != LM_P0_NODE_ATOM) {
                return - 1;
            }
            if (descriptor_count >= sizeof(out_column -> descriptors) / sizeof(out_column -> descriptors[0])) {
                return - 1;
            }
            out_column->descriptors[descriptor_count] = body_field->value->as.atom;
            descriptor_count = descriptor_count + 1U;
        }
        body_field = body_field -> next;
    }
    out_column->descriptor_count = descriptor_count;
    return 1;
}

static int lm_l4_columns_from_frame(const LmL4Loader *loader, const LmP0Frame *frame, LmL4Column *columns, size_t columns_capacity, size_t *out_count) {
    const LmP0Field *field;
    LmL4Column column;
    LmP0Text first_column;
    size_t count;
    int status;
    if (frame == 0 || columns == 0 || out_count == 0 || columns_capacity == 0U || lm_l4_text_equals(frame -> head, "columns") == 0) {
        return 0;
    }
    count = 0U;
    field = frame->body.first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_l4_node_is_ignored(field -> value) == 0) {
            status = lm_l4_column_name(field, &column);
            if (status <= 0) {
                lm_l4_error(loader, "columns expects atoms or anonymous descriptor structures");
                return - 1;
            }
            if (count >= columns_capacity) {
                lm_l4_error(loader, "table has too many columns");
                return - 1;
            }
            columns[count] = column;
            count = count + 1U;
        }
        field = field -> next;
    }
    if (count < 2U) {
        lm_l4_error(loader, "table expects at least two columns");
        return - 1;
    }
    if (lm_l4_identifier_value(columns[0].name, &first_column) == 0 || lm_l4_text_equals(first_column, "class") == 0) {
        lm_l4_error(loader, "first table column must be class");
        return - 1;
    }
    out_count[0] = count;
    return 1;
}

static int lm_l4_validate_named_trailer(const LmL4Loader *loader, const LmP0Frame *frame, LmP0Text expected_name) {
    LmP0Text actual;
    LmP0Text actual_payload;
    LmP0Text expected_payload;
    if (frame == 0 || frame -> trailer == 0) {
        return 0;
    }
    if (lm_l4_text_equals(frame->trailer->spelling, "end") == 0) {
        return 0;
    }
    if (lm_l4_trailer_single_atom(frame -> trailer, &actual) == 0) {
        lm_l4_error(loader, "end trailer expects exactly one target name");
        return 1;
    }
    if (lm_l4_identifier_value(actual, &actual_payload) == 0 || lm_l4_identifier_value(expected_name, &expected_payload) == 0 || lm_l4_text_same(actual_payload, expected_payload) == 0) {
        lm_l4_error(loader, "end trailer target does not match head/name");
        return 1;
    }
    return 0;
}

static int lm_l4_row_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field *table_field;
    const LmP0Field *key_field;
    const LmP0Field *payload_field;
    if (frame == 0 || lm_l4_text_equals(frame -> head, "row") == 0) {
        return 0;
    }
    table_field = lm_l4_nth_field(&frame->body, 0U);
    key_field = lm_l4_nth_field(&frame->body, 1U);
    payload_field = lm_l4_nth_field(&frame->body, 2U);
    if (table_field == 0 || key_field == 0 || payload_field == 0 || payload_field -> next != 0 || table_field -> value == 0 || key_field -> value == 0 || payload_field -> value == 0 || table_field -> value -> kind != LM_P0_NODE_ATOM || key_field -> value -> kind != LM_P0_NODE_ATOM) {
        lm_l4_error(loader, "row expects table and key atoms plus one payload field");
        return - 1;
    }
    if (loader == 0 || loader -> push_row == 0) {
        lm_l4_error(loader, "row consumer is not configured");
        return - 1;
    }
    if (loader->push_row(context, table_field->value->as.atom, key_field->value->as.atom, payload_field -> value) != 0) {
        return - 1;
    }
    return 1;
}

static int lm_l4_rows_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame, LmP0Text table_name, const LmL4Column *columns, size_t column_count) {
    const LmP0Field *field;
    const LmP0Node *key_node;
    const LmP0Node *cell_node;
    size_t field_index;
    size_t column_index;
    int split_by_column;
    if (frame == 0 || lm_l4_text_equals(frame -> head, "rows") == 0) {
        return 0;
    }
    if (loader == 0 || loader -> push_cell == 0) {
        lm_l4_error(loader, "cell consumer is not configured");
        return - 1;
    }
    if (columns == 0 || column_count < 2U) {
        lm_l4_error(loader, "rows require at least two columns");
        return - 1;
    }
    field_index = 0U;
    key_node = 0;
    split_by_column = column_count != 2U;
    field = frame->body.first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_l4_node_is_ignored(field -> value) == 0) {
            column_index = field_index % column_count;
            if (column_index == 0U) {
                key_node = field -> value;
                if (key_node -> kind != LM_P0_NODE_ATOM) {
                    lm_l4_error(loader, "table rows currently expect atom cells in the key column");
                    return - 1;
                }
                if (loader -> note_key != 0 && loader->note_key(context, table_name, &columns[0], key_node->as.atom) != 0) {
                    return - 1;
                }
            }
            if (column_index != 0U) {
                cell_node = field -> value;
                if (key_node == 0 || loader->push_cell(context, table_name, &columns[column_index], split_by_column, key_node->as.atom, cell_node) != 0) {
                    return - 1;
                }
            }
            field_index = field_index + 1U;
        }
        field = field -> next;
    }
    if ((field_index % column_count) != 0U) {
        lm_l4_error(loader, "rows field count is not divisible by column count; use explicit NULL for empty cells");
        return - 1;
    }
    if (field_index == 0U) {
        lm_l4_error(loader, "table rows must not be empty");
        return - 1;
    }
    return 1;
}

static int lm_l4_table_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmL4Column columns[128];
    LmP0Text table_name;
    size_t column_count;
    int have_name;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_l4_text_equals(frame -> head, "table") == 0) {
        return 0;
    }
    have_name = 0;
    have_columns = 0;
    have_rows = 0;
    column_count = 0U;
    field = frame->body.first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_l4_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_l4_error(loader, "table body expects name/columns/rows frames");
                return - 1;
            }
            status = lm_l4_frame_single_atom(&node->as.frame, "name", &table_name);
            if (status < 0) {
                lm_l4_error(loader, "table name expects exactly one atom");
                return - 1;
            }
            if (status > 0) {
                have_name = 1;
                field = field -> next;
                continue;
            }
            if (lm_l4_text_equals(node->as.frame.head, "columns") != 0) {
                if (have_name == 0) {
                    lm_l4_error(loader, "table columns must appear after name");
                    return - 1;
                }
                status = lm_l4_columns_from_frame(loader, &node->as.frame, columns, sizeof(columns) / sizeof(columns[0]), &column_count);
                if (status <= 0) {
                    return - 1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && loader->push_column_metadata(context, table_name, columns, column_count) != 0) {
                    lm_l4_error(loader, "cannot store table column metadata");
                    return - 1;
                }
                have_columns = 1;
                field = field -> next;
                continue;
            }
            if (lm_l4_text_equals(node->as.frame.head, "rows") != 0) {
                if (have_name == 0 || have_columns == 0) {
                    lm_l4_error(loader, "table rows must appear after name and columns");
                    return - 1;
                }
                status = lm_l4_rows_from_frame(loader, context, &node->as.frame, table_name, columns, column_count);
                if (status <= 0) {
                    return - 1;
                }
                have_rows = 1;
                field = field -> next;
                continue;
            }
            lm_l4_error(loader, "table body expects name/columns/rows frames");
            return - 1;
        }
        field = field -> next;
    }
    if (have_name == 0 || have_columns == 0 || have_rows == 0) {
        lm_l4_error(loader, "table requires name, columns and rows");
        return - 1;
    }
    if (lm_l4_validate_named_trailer(loader, frame, table_name) != 0) {
        return - 1;
    }
    return 1;
}

static int lm_l4_seen_table_add(LmOwnPtrStack *seen, LmP0Text table_name) {
    LmP0Text payload;
    char *name;
    size_t i;
    size_t payload_length;
    char *existing;
    if (seen == 0) {
        return - 1;
    }
    if (lm_l4_identifier_value(table_name, &payload) == 0) {
        return - 1;
    }
    payload_length = payload.length;
    i = 0U;
    while (i < seen -> count) {
        existing = lm_own_ptr_stack_at(seen, i);
        if (existing != 0 && lm_l4_text_equals(payload, existing) != 0) {
            return 1;
        }
        i = i + 1U;
    }
    name = malloc(payload_length + 1U);
    if (name == 0) {
        return - 1;
    }
    if (payload_length > 0U) {
        memcpy(name, payload.data, payload_length);
    }
    name[payload_length] = '\0';
    if (lm_own_ptr_stack_push(seen, name) != 0) {
        free(name);
        return - 1;
    }
    return 0;
}

static int lm_l4_check_table_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text table_name;
    int status;
    if (frame == 0 || lm_l4_text_equals(frame -> head, "table") == 0) {
        return 0;
    }
    status = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field -> value;
        if (node == 0 || lm_l4_node_is_ignored(node) != 0 || node -> kind != LM_P0_NODE_FRAME) {
            field = field -> next;
            continue;
        }
        status = lm_l4_frame_single_atom(&node->as.frame, "name", &table_name);
        if (status < 0) {
            lm_l4_error(loader, "table name expects exactly one atom");
            return - 1;
        }
        if (status > 0) {
            break;
        }
        field = field -> next;
    }
    if (status == 0) {
        return 0;
    }
    status = lm_l4_seen_table_add(seen, table_name);
    if (status < 0) {
        lm_l4_error(loader, "cannot record table name");
        return - 1;
    }
    if (status > 0) {
        lm_l4_error(loader, "duplicate table in one L4 schema");
        return - 1;
    }
    return 0;
}

static int lm_l4_receiver_table(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    int status;
    if (lm_l4_seen_tables_get() != 0 && lm_l4_check_table_frame_unique(loader, frame, lm_l4_seen_tables_get()) != 0) {
        return 1;
    }
    status = lm_l4_table_from_frame(loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_l4_error(loader, "table receiver expects table frame");
        }
        return 1;
    }
    return 0;
}

static int lm_l4_receiver_row(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    int status;
    status = lm_l4_row_from_frame(loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_l4_error(loader, "row receiver expects row frame");
        }
        return 1;
    }
    return 0;
}

static const LmL4Receiver * lm_l4_find_receiver(const LmL4Loader *loader, LmP0Text head) {
    size_t i;
    if (loader == 0 || loader -> receivers == 0) {
        return 0;
    }
    i = 0U;
    while (i < loader -> receiver_count) {
        if (loader->receivers[i].name != 0 && lm_l4_text_equals(head, loader->receivers[i].name) != 0) {
            return &loader->receivers[i];
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_l4_dispatch_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    const LmL4Receiver *receiver;
    if (frame == 0) {
        return 1;
    }
    receiver = lm_l4_find_receiver(loader, frame -> head);
    if (receiver == 0 || receiver -> frame == 0) {
        lm_l4_error(loader, "registry body expects registered L4 receiver frames");
        return 1;
    }
    return receiver->frame(loader, context, frame);
}

static int lm_l4_load_rows(const LmL4Loader *loader, void *context, const LmP0Structure *structure) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmOwnPtrStack *seen;
    LmOwnPtrStack *previous_seen;
    int owns_seen;
    int status;
    if (structure == 0) {
        return 0;
    }
    owns_seen = lm_l4_seen_tables_get() == 0;
    seen = 0;
    previous_seen = 0;
    if (owns_seen != 0) {
        seen = malloc(sizeof(seen[0]));
        if (seen == 0) {
            lm_l4_error(loader, "cannot allocate table duplicate tracker");
            return 1;
        }
        lm_own_ptr_stack_init(seen, free);
        previous_seen = lm_l4_seen_tables_get();
        lm_l4_seen_tables_set(seen);
    }
    status = 0;
    field = structure -> first_field;
    while (field != 0 && status == 0) {
        node = field -> value;
        if (lm_l4_node_is_ignored(node) != 0) {
            field = field -> next;
            continue;
        }
        if (node -> kind != LM_P0_NODE_FRAME) {
            lm_l4_error(loader, "registry body expects registered L4 receiver frames");
            status = 1;
            field = field -> next;
            continue;
        }
        if (lm_l4_dispatch_frame(loader, context, &node->as.frame) != 0) {
            status = 1;
        }
        field = field -> next;
    }
    if (owns_seen != 0) {
        lm_l4_seen_tables_set(previous_seen);
        lm_own_ptr_stack_destroy(seen);
        free(seen);
    }
    return status;
}

static int lm_l4_load_root(const LmL4Loader *loader, void *context, const LmP0Node *root, int implicit_l4, size_t *out_row_count) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmOwnPtrStack *seen;
    LmOwnPtrStack *previous_seen;
    int loaded;
    int status;
    if (root == 0 || root -> kind != LM_P0_NODE_STRUCTURE) {
        lm_l4_error(loader, "root must be a Structure");
        return 1;
    }
    if (out_row_count != 0) {
        out_row_count[0] = 0U;
    }
    seen = malloc(sizeof(seen[0]));
    if (seen == 0) {
        lm_l4_error(loader, "cannot allocate table duplicate tracker");
        return 1;
    }
    lm_own_ptr_stack_init(seen, free);
    previous_seen = lm_l4_seen_tables_get();
    lm_l4_seen_tables_set(seen);
    loaded = 0;
    status = 0;
    field = root->as.structure.first_field;
    while (field != 0 && status == 0) {
        node = field -> value;
        if (lm_l4_node_is_ignored(node) != 0) {
            field = field -> next;
            continue;
        }
        if (node -> kind != LM_P0_NODE_FRAME) {
            lm_l4_error(loader, "root fields must be L4/registry frames, or registered L4 receiver frames in .lm4 files");
            status = 1;
            field = field -> next;
            continue;
        }
        if (lm_l4_text_equals(node->as.frame.head, "L4") != 0 || lm_l4_text_equals(node->as.frame.head, "registry") != 0) {
            if (lm_l4_load_rows(loader, context, &node->as.frame.body) != 0) {
                status = 1;
            }
            if (status == 0 && lm_l4_validate_named_trailer(loader, &node->as.frame, node->as.frame.head) != 0) {
                status = 1;
            }
            if (status == 0) {
                loaded = 1;
            }
            field = field -> next;
            continue;
        }
        if (implicit_l4 != 0) {
            if (lm_l4_dispatch_frame(loader, context, &node->as.frame) != 0) {
                status = 1;
            }
            if (status == 0) {
                loaded = 1;
            }
            field = field -> next;
            continue;
        }
        lm_l4_error(loader, "root fields must be L4/registry frames");
        status = 1;
        field = field -> next;
    }
    if (status == 0 && loaded == 0) {
        lm_l4_error(loader, "no rows loaded");
        status = 1;
    }
    lm_l4_seen_tables_set(previous_seen);
    lm_own_ptr_stack_destroy(seen);
    free(seen);
    return status;
}
static int lm_l4_receiver_ignore(
    const LmL4Loader *loader,
    void *context,
    const LmP0Frame *frame
) {
    (void)loader;
    (void)context;
    (void)frame;
    return 0;
}

static const LmL4Receiver lm_l4_default_receivers[] = {
    { "table", lm_l4_receiver_table },
    { "row", lm_l4_receiver_row },
    { "fn", lm_l4_receiver_ignore }
};




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int lm_trans_text_equals(LmP0Text text, const char *value);
static int lm_trans_text_same(LmP0Text left, LmP0Text right);
static int lm_trans_text_starts_with(LmP0Text text, const char *prefix);
static int lm_trans_identifier_payload(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_identifier_same(LmP0Text left, LmP0Text right);
static int lm_trans_emit_identifier(FILE *file, LmP0Text name);
static int lm_trans_registry_identifier_value(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_registry_literal_value(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_registry_payload_is_null(LmP0Text atom);
static char * lm_trans_text_copy_cstr(LmP0Text text);
static char * lm_trans_registry_value_copy_cstr(LmP0Text value);
static int lm_trans_registry_clone_text(LmP0Text source, LmP0Text *out_text);
static LmP0Text lm_trans_text_from_cstr(const char *text);
static int lm_trans_text_all_char(LmP0Text text, char ch);
static int lm_trans_write_all(FILE *file, const char *data, size_t length);
static int lm_trans_put(FILE *file, const char *text);
static FILE * lm_trans_prelude_file(FILE *fallback);
static int lm_trans_write_text(FILE *file, LmP0Text text);
static int lm_trans_emit_indent(FILE *file, unsigned indent);
static const LmP0Field * lm_trans_nth_field(const LmP0Structure *structure, size_t index);
static int lm_trans_node_is_ignored(const LmP0Node *node);
static int lm_trans_node_is_positional_skip(const LmP0Node *node);
static int lm_trans_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text);
static const char * lm_trans_symbol_class_name(const char *class_name);
static int lm_trans_symbol_class_is(const char *class_name, const char *expected);
static int lm_trans_symbol_is(const LmTransSymbol *symbol, const char *class_name);
static int lm_trans_symbol_is_executable_callable(const LmTransSymbol *symbol);
static int lm_trans_symbol_is_value_callable(const LmTransSymbol *symbol);
static void lm_trans_symbol_destroy_fields(LmTransSymbol *symbol);
static void lm_trans_symbol_destroy_fields_any(void *object);
static void lm_trans_symbol_destroy(LmTransSymbol *symbol);
static void lm_trans_symbol_delete_any(void *object);
static void lm_trans_free_any(void *object);

static FILE *lm_trans_prelude_output;
static const char *lm_trans_current_source_path;
static LmOwnPtrStack lm_trans_declared_import_paths;
static LmOwnPtrStack lm_trans_declared_import_documents;
static LmOwnPtrStack lm_trans_emitted_import_prelude_paths;
static LmOwnPtrStack lm_trans_emitted_import_function_paths;
static LmOwnPtrStack lm_trans_emitted_callable_adapters;
static LmOwnPtrStack lm_trans_emitted_callable_binders;
static LmOwnPtrStack lm_trans_emitted_array_value_helpers;
static LmOwnPtrStack lm_trans_emitted_function_return_structs;
static unsigned lm_trans_next_array_value_helper_id;

static LmTransRegistry lm_trans_registry;

static int lm_trans_text_equals(LmP0Text text, const char *value);
static int lm_trans_text_same(LmP0Text left, LmP0Text right);
static int lm_trans_text_starts_with(LmP0Text text, const char *prefix);
static int lm_trans_identifier_payload(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_identifier_same(LmP0Text left, LmP0Text right);
static int lm_trans_emit_identifier(FILE *file, LmP0Text name);
static int lm_trans_registry_identifier_value(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_registry_literal_value(LmP0Text atom, LmP0Text *out_payload);
static int lm_trans_registry_payload_is_null(LmP0Text atom);
static char *lm_trans_text_copy_cstr(LmP0Text text);
static char *lm_trans_registry_value_copy_cstr(LmP0Text value);
static int lm_trans_registry_clone_text(LmP0Text source, LmP0Text *out_text);
static LmP0Text lm_trans_text_from_cstr(const char *text);
static int lm_trans_l4_callable_type_from_node(const LmP0Node *node, LmTransL4CallableType *out);
static int lm_trans_collect_abi_params(
    LmTransAbiParam *params,
    size_t capacity,
    size_t *out_count,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *error_name
);
static int lm_trans_emit_abi_params(
    FILE *file,
    const LmTransAbiParam *params,
    size_t count
);
static int lm_trans_emit_abi_return_type(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *error_name
);
static int lm_trans_emit_type_name(FILE *file, LmP0Text name);
static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node);
static int lm_trans_registry_latest_size_payload(
    const LmOwnPtrStack *stack,
    LmP0Text key,
    size_t default_value,
    size_t *out_value
);
static int lm_trans_text_all_char(LmP0Text text, char ch);
static int lm_trans_write_all(FILE *file, const char *data, size_t length);
static int lm_trans_put(FILE *file, const char *text);
static FILE *lm_trans_prelude_file(FILE *fallback);
static int lm_trans_write_text(FILE *file, LmP0Text text);
static int lm_trans_emit_indent(FILE *file, unsigned indent);
static const LmP0Field *lm_trans_nth_field(const LmP0Structure *structure, size_t index);
static int lm_trans_node_is_ignored(const LmP0Node *node);
static int lm_trans_node_is_positional_skip(const LmP0Node *node);
static int lm_trans_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text);
static const char *lm_trans_symbol_class_name(const char *class_name);
static int lm_trans_symbol_class_is(const char *class_name, const char *expected);
static int lm_trans_symbol_is(const LmTransSymbol *symbol, const char *class_name);
static int lm_trans_symbol_is_executable_callable(const LmTransSymbol *symbol);
static int lm_trans_symbol_is_value_callable(const LmTransSymbol *symbol);
static void lm_trans_symbol_destroy_fields(LmTransSymbol *symbol);
static void lm_trans_symbol_destroy_fields_any(void *object);
static void lm_trans_symbol_destroy(LmTransSymbol *symbol);
static void lm_trans_symbol_delete_any(void *object);
static void lm_trans_free_any(void *object);

static void lm_trans_import_document_delete(void *document) {
    lm_p0_document_destroy((LmP0Document *)document);
}

static LmP0Node *lm_trans_registry_clone_node(const LmP0Node *source);

static int lm_trans_registry_clone_structure(
    const LmP0Structure *source,
    LmP0Structure *out_structure
);

static LmP0Trailer *lm_trans_registry_clone_trailer(
    const LmP0Trailer *source
) {
    LmP0Trailer *copy;

    if (source == 0) {
        return 0;
    }

    copy = (LmP0Trailer *)lm_own_arena_new_zero(
        &lm_trans_registry.value_arena,
        sizeof(*copy)
    );
    if (copy == 0) {
        return 0;
    }

    copy->flags = source->flags;
    if (
        lm_trans_registry_clone_text(source->spelling, &copy->spelling) != 0 ||
        lm_trans_registry_clone_structure(&source->body, &copy->body) != 0
    ) {
        return 0;
    }

    return copy;
}

static int lm_trans_registry_clone_structure(
    const LmP0Structure *source,
    LmP0Structure *out_structure
) {
    const LmP0Field *source_field;
    LmP0Field *copy_field;
    LmP0Field *previous_field;

    if (out_structure == 0) {
        return 1;
    }

    memset(out_structure, 0, sizeof(*out_structure));
    if (source == 0) {
        return 0;
    }

    previous_field = 0;
    source_field = source->first_field;
    while (source_field != 0) {
        copy_field = (LmP0Field *)lm_own_arena_new_zero(
            &lm_trans_registry.value_arena,
            sizeof(*copy_field)
        );
        if (copy_field == 0) {
            return 1;
        }
        copy_field->value = lm_trans_registry_clone_node(source_field->value);
        if (source_field->value != 0 && copy_field->value == 0) {
            return 1;
        }

        if (previous_field == 0) {
            out_structure->first_field = copy_field;
        } else {
            previous_field->next = copy_field;
        }
        previous_field = copy_field;
        out_structure->last_field = copy_field;
        ++out_structure->field_count;

        source_field = source_field->next;
    }

    out_structure->trailer = lm_trans_registry_clone_trailer(source->trailer);
    if (source->trailer != 0 && out_structure->trailer == 0) {
        return 1;
    }

    return 0;
}

static LmP0Node *lm_trans_registry_clone_node(const LmP0Node *source) {
    LmP0Node *copy;

    if (source == 0) {
        return 0;
    }

    copy = (LmP0Node *)lm_own_arena_new_zero(
        &lm_trans_registry.value_arena,
        sizeof(*copy)
    );
    if (copy == 0) {
        return 0;
    }

    copy->kind = source->kind;
    copy->flags = source->flags;
    copy->span = source->span;

    if (source->kind == LM_P0_NODE_ATOM || source->kind == LM_P0_NODE_DISABLED) {
        if (lm_trans_registry_clone_text(source->as.atom, &copy->as.atom) != 0) {
            return 0;
        }
    } else if (source->kind == LM_P0_NODE_FRAME) {
        copy->as.frame.flags = source->as.frame.flags;
        if (
            lm_trans_registry_clone_text(source->as.frame.head, &copy->as.frame.head) != 0 ||
            lm_trans_registry_clone_structure(&source->as.frame.body, &copy->as.frame.body) != 0
        ) {
            return 0;
        }
        copy->as.frame.trailer = lm_trans_registry_clone_trailer(source->as.frame.trailer);
        if (source->as.frame.trailer != 0 && copy->as.frame.trailer == 0) {
            return 0;
        }
    } else if (source->kind == LM_P0_NODE_STRUCTURE) {
        if (lm_trans_registry_clone_structure(&source->as.structure, &copy->as.structure) != 0) {
            return 0;
        }
    } else {
        return 0;
    }

    return copy;
}

static LmTransIdentifierCard *lm_trans_identifier_table_find_card(
    const LmTransIdentifierTable *table,
    LmP0Text name
);
static LmTransIdentifierCard *lm_trans_identifier_table_get_card(
    LmTransIdentifierTable *table,
    LmP0Text name
);
static LmTransIdentifierRelation *lm_trans_identifier_card_find_relation(
    const LmTransIdentifierCard *card,
    const char *relation_name
);
static LmTransIdentifierRelation *lm_trans_identifier_card_get_relation(
    LmTransIdentifierCard *card,
    const char *relation_name
);

static int lm_trans_registry_index_row_relation(
    LmP0Text card_name,
    const char *relation_name,
    LmTransRegistryFact *row
) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (row == 0 || relation_name == 0) {
        return 1;
    }

    card = lm_trans_identifier_table_get_card(&lm_trans_registry.identifiers, card_name);
    if (card == 0) {
        return 1;
    }

    relation = lm_trans_identifier_card_get_relation(card, relation_name);
    if (relation == 0) {
        return 1;
    }

    return lm_own_ptr_stack_push(&relation->symbols, row);
}

static int lm_trans_registry_index_row_table_view(LmTransRegistryFact *row) {
    const char *dot;
    LmP0Text table_name;
    const char *relation_name;

    if (row == 0 || row->table == 0) {
        return 1;
    }

    dot = strchr(row->table, '.');
    if (dot != 0 && dot != row->table && dot[1] != '\0') {
        table_name.data = row->table;
        table_name.length = (size_t)(dot - row->table);
        relation_name = dot + 1;
    } else {
        table_name = lm_trans_text_from_cstr(row->table);
        relation_name = "row";
    }

    return lm_trans_registry_index_row_relation(table_name, relation_name, row);
}

static int lm_trans_registry_index_row(LmTransRegistryFact *row) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (row == 0 || row->table == 0 || row->key == 0) {
        return 1;
    }

    card = lm_trans_identifier_table_get_card(
        &lm_trans_registry.identifiers,
        lm_trans_text_from_cstr(row->key)
    );
    if (card == 0) {
        return 1;
    }

    relation = lm_trans_identifier_card_get_relation(card, row->table);
    if (relation == 0) {
        return 1;
    }

    if (lm_own_ptr_stack_push(&relation->symbols, row) != 0) {
        return 1;
    }

    return lm_trans_registry_index_row_table_view(row);
}

static int lm_trans_registry_push_row_values(
    LmP0Text table_value,
    LmP0Text key_value,
    LmP0Text payload_value
) {
    LmTransRegistryFact *row;

    row = (LmTransRegistryFact *)lm_own_arena_new_zero(
        &lm_trans_registry.value_arena,
        sizeof(*row)
    );
    if (row == 0) {
        return -1;
    }

    row->table = lm_trans_registry_value_copy_cstr(table_value);
    row->key = lm_trans_registry_value_copy_cstr(key_value);
    row->payload = lm_trans_registry_value_copy_cstr(payload_value);
    if (row->table == 0 || row->key == 0 || row->payload == 0) {
        return -1;
    }

    if (lm_trans_registry_index_row(row) != 0) {
        return -1;
    }

    ++lm_trans_registry.loaded_fact_count;
    return 0;
}

static int lm_trans_registry_push_row_node_values(
    LmP0Text table_value,
    LmP0Text key_value,
    const LmP0Node *payload_node
) {
    LmTransRegistryFact *row;
    LmP0Node *payload_copy;

    if (payload_node == 0) {
        return -1;
    }

    row = (LmTransRegistryFact *)lm_own_arena_new_zero(
        &lm_trans_registry.value_arena,
        sizeof(*row)
    );
    if (row == 0) {
        return -1;
    }

    payload_copy = lm_trans_registry_clone_node(payload_node);
    if (payload_copy == 0) {
        return -1;
    }

    row->table = lm_trans_registry_value_copy_cstr(table_value);
    row->key = lm_trans_registry_value_copy_cstr(key_value);
    row->payload_node = payload_copy;
    if (row->table == 0 || row->key == 0) {
        return -1;
    }

    if (lm_trans_registry_index_row(row) != 0) {
        return -1;
    }

    ++lm_trans_registry.loaded_fact_count;
    return 0;
}

static int lm_trans_registry_push_row_atoms(
    LmP0Text table_atom,
    LmP0Text key_atom,
    LmP0Text payload_atom
) {
    LmP0Text table_value;
    LmP0Text key_value;
    LmP0Text payload_value;

    if (lm_trans_registry_payload_is_null(payload_atom)) {
        return 0;
    }
    if (
        !lm_trans_registry_identifier_value(table_atom, &table_value) ||
        !lm_trans_registry_identifier_value(key_atom, &key_value) ||
        !lm_trans_registry_literal_value(payload_atom, &payload_value)
    ) {
        return -1;
    }
    return lm_trans_registry_push_row_values(table_value, key_value, payload_value);
}

static int lm_trans_registry_column_has_descriptor(
    const LmTransRegistryColumn *column,
    const char *descriptor
) {
    size_t i;
    LmP0Text payload;

    if (column == 0 || descriptor == 0) {
        return 0;
    }

    for (i = 0U; i < column->descriptor_count; ++i) {
        if (
            lm_trans_registry_identifier_value(column->descriptors[i], &payload) &&
            lm_trans_text_equals(payload, descriptor)
        ) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_registry_column_is_class_typed(
    const LmTransRegistryColumn *column
) {
    LmP0Text payload;

    if (column == 0) {
        return 0;
    }
    if (
        lm_trans_registry_identifier_value(column->name, &payload) &&
        lm_trans_text_equals(payload, "class")
    ) {
        return 1;
    }
    return lm_trans_registry_column_has_descriptor(column, "class");
}

static int lm_trans_registry_cell_value(
    LmP0Text atom,
    const LmTransRegistryColumn *column,
    LmP0Text *out_value
) {
    if (lm_trans_registry_payload_is_null(atom)) {
        return 0;
    }
    if (lm_trans_registry_column_has_descriptor(column, "char")) {
        return lm_trans_registry_literal_value(atom, out_value) ? 1 : -1;
    }
    return lm_trans_registry_identifier_value(atom, out_value) ? 1 : -1;
}

static int lm_trans_registry_push_table_cell(
    LmP0Text table_name,
    const LmTransRegistryColumn *column,
    int split_by_column,
    LmP0Text key_atom,
    const LmP0Node *payload_node,
    int allow_node_cells
) {
    LmP0Text table_payload;
    LmP0Text column_payload;
    LmP0Text key_payload;
    LmP0Text payload_value;
    LmP0Text relation_table;
    char *relation_name;
    size_t relation_length;
    int status;
    int cell_status;
    int wants_node;

    if (payload_node == 0 || column == 0) {
        return -1;
    }

    wants_node = lm_trans_registry_column_has_descriptor(column, "node");

    if (!wants_node) {
        if (payload_node->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans registry error: non-atom table cell requires a node-typed column\n");
            return -1;
        }
        cell_status = lm_trans_registry_cell_value(payload_node->as.atom, column, &payload_value);
        if (cell_status == 0) {
            return 0;
        }
        if (cell_status < 0) {
            return -1;
        }
    } else if (!allow_node_cells) {
        fprintf(stderr, "trans registry error: node table cells are allowed only in inline L4 for now\n");
        return -1;
    } else if (
        payload_node->kind == LM_P0_NODE_ATOM &&
        lm_trans_registry_payload_is_null(payload_node->as.atom)
    ) {
        return 0;
    }

    if (!lm_trans_registry_identifier_value(table_name, &table_payload)) {
        return -1;
    }

    if (!lm_trans_registry_identifier_value(key_atom, &key_payload)) {
        return -1;
    }

    if (!split_by_column) {
        if (wants_node) {
            return lm_trans_registry_push_row_node_values(table_payload, key_payload, payload_node);
        }
        return lm_trans_registry_push_row_values(table_payload, key_payload, payload_value);
    }

    if (!lm_trans_registry_identifier_value(column->name, &column_payload)) {
        return -1;
    }

    relation_length = table_payload.length + 1U + column_payload.length;
    relation_name = (char *)malloc(relation_length + 1U);
    if (relation_name == 0) {
        return -1;
    }
    memcpy(relation_name, table_payload.data, table_payload.length);
    relation_name[table_payload.length] = '.';
    memcpy(relation_name + table_payload.length + 1U, column_payload.data, column_payload.length);
    relation_name[relation_length] = '\0';

    relation_table = lm_trans_text_from_cstr(relation_name);
    if (wants_node) {
        status = lm_trans_registry_push_row_node_values(relation_table, key_payload, payload_node);
    } else {
        status = lm_trans_registry_push_row_values(relation_table, key_payload, payload_value);
    }
    free(relation_name);
    return status;
}

static int lm_trans_registry_note_class_kind(LmP0Text name, const char *kind) {
    if (kind == 0) {
        return 0;
    }

    return lm_trans_registry_push_row_atoms(
        lm_trans_text_from_cstr("class.kind"),
        name,
        lm_trans_text_from_cstr(kind)
    ) != 0;
}

static int lm_trans_registry_note_class_present(LmP0Text name) {
    LmP0Text payload;

    if (!lm_trans_registry_identifier_value(name, &payload)) {
        return 0;
    }

    return lm_trans_registry_push_row_values(
        lm_trans_text_from_cstr("class.present"),
        payload,
        lm_trans_text_from_cstr("class")
    ) != 0;
}

static LmTransRegistryFact *lm_trans_registry_lookup_row_in_identifiers(
    const LmTransIdentifierTable *identifiers,
    LmP0Text key,
    const char *table
) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (table == 0) {
        return 0;
    }

    card = lm_trans_identifier_table_find_card(identifiers, key);
    if (card == 0) {
        return 0;
    }
    relation = lm_trans_identifier_card_find_relation(card, table);
    if (relation == 0) {
        return 0;
    }
    return (LmTransRegistryFact *)lm_own_ptr_stack_top(&relation->symbols);
}

static const LmOwnPtrStack *lm_trans_registry_relation_stack_in_identifiers(
    const LmTransIdentifierTable *identifiers,
    LmP0Text key,
    const char *relation_name
) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (relation_name == 0) {
        return 0;
    }

    card = lm_trans_identifier_table_find_card(identifiers, key);
    if (card == 0) {
        return 0;
    }

    relation = lm_trans_identifier_card_find_relation(card, relation_name);
    return relation != 0 ? &relation->symbols : 0;
}

static const LmOwnPtrStack *lm_trans_registry_relation_stack(
    LmP0Text key,
    const char *relation_name
) {
    return lm_trans_registry_relation_stack_in_identifiers(
        &lm_trans_registry.identifiers,
        key,
        relation_name
    );
}

static const LmOwnPtrStack *lm_trans_namespace_registry_relation_stack(
    const LmTransNamespace *namespace_,
    LmP0Text key,
    const char *relation_name
) {
    const LmOwnPtrStack *stack;

    if (namespace_ != 0 && namespace_->registry_identifiers != 0) {
        stack = lm_trans_registry_relation_stack_in_identifiers(
            namespace_->registry_identifiers,
            key,
            relation_name
        );
        if (stack != 0) {
            return stack;
        }
    }

    return lm_trans_registry_relation_stack(key, relation_name);
}

static const LmOwnPtrStack *lm_trans_registry_relation_stack_for_table(
    const char *table
) {
    const char *dot;
    LmP0Text table_name;
    const char *relation_name;

    if (table == 0) {
        return 0;
    }

    dot = strchr(table, '.');
    if (dot != 0 && dot != table && dot[1] != '\0') {
        table_name.data = table;
        table_name.length = (size_t)(dot - table);
        relation_name = dot + 1;
    } else {
        table_name = lm_trans_text_from_cstr(table);
        relation_name = "row";
    }

    return lm_trans_registry_relation_stack(table_name, relation_name);
}

static LmTransRegistryFact *lm_trans_registry_relation_stack_latest_row(
    const LmOwnPtrStack *stack,
    LmP0Text key
) {
    size_t i;
    LmTransRegistryFact *row;
    LmP0Text key_payload;

    if (stack == 0 || !lm_trans_identifier_payload(key, &key_payload)) {
        return 0;
    }

    i = stack->count;
    while (i > 0U) {
        --i;
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(stack, i);
        if (row != 0 && row->key != 0 && lm_trans_text_equals(key_payload, row->key)) {
            return row;
        }
    }

    return 0;
}

static const char *lm_trans_registry_lookup(LmP0Text key, const char *table) {
    LmTransRegistryFact *row;

    row = lm_trans_registry_lookup_row_in_identifiers(
        &lm_trans_registry.identifiers,
        key,
        table
    );
    if (row != 0) {
        return row->payload;
    }

    return 0;
}

static int lm_trans_registry_has(LmP0Text key, const char *table) {
    return lm_trans_registry_lookup(key, table) != 0;
}

static int lm_trans_registry_table_has_rows(const char *table) {
    const LmOwnPtrStack *stack;

    if (table == 0) {
        return 0;
    }

    stack = lm_trans_registry_relation_stack_for_table(table);
    return stack != 0 && stack->count != 0U;
}

static int lm_trans_registry_assert_selected_table_link(
    const char *source_table,
    LmP0Text key,
    const char *target_class_table,
    const char *target_table
) {
    if (source_table == 0 || target_table == 0) {
        return 1;
    }
    if (
        target_class_table != 0 &&
        !lm_trans_registry_has(lm_trans_text_from_cstr(target_table), target_class_table)
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] points to %s, but %s has no class row for that target\n",
            source_table,
            (int)key.length,
            key.data,
            target_table,
            target_class_table
        );
        return 1;
    }
    if (!lm_trans_registry_has(key, target_table)) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] points to %s, but %s has no matching row for that key\n",
            source_table,
            (int)key.length,
            key.data,
            target_table,
            target_table
        );
        return 1;
    }
    return 0;
}

static int lm_trans_registry_assert_payload_table_exists(
    const char *source_table,
    LmP0Text key,
    const char *target_table
) {
    if (source_table == 0 || target_table == 0) {
        return 1;
    }
    if (!lm_trans_registry_table_has_rows(target_table)) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] points to %s, but that table has no rows\n",
            source_table,
            (int)key.length,
            key.data,
            target_table
        );
        return 1;
    }
    return 0;
}

static const char *lm_trans_registry_lookup_table_link_checked(
    LmP0Text key,
    const char *source_table,
    const char *target_class_table
) {
    const char *target_table;

    target_table = lm_trans_registry_lookup(key, source_table);
    if (
        target_table != 0 &&
        lm_trans_registry_assert_selected_table_link(source_table, key, target_class_table, target_table) != 0
    ) {
        exit(2);
    }
    return target_table;
}

static const char *lm_trans_namespace_registry_lookup(
    const LmTransNamespace *namespace_,
    LmP0Text key,
    const char *table
) {
    LmTransRegistryFact *row;

    if (namespace_ != 0 && namespace_->registry_identifiers != 0) {
        row = lm_trans_registry_lookup_row_in_identifiers(
            namespace_->registry_identifiers,
            key,
            table
        );
        if (row != 0) {
            return row->payload;
        }
    }

    return lm_trans_registry_lookup(key, table);
}

static const char *lm_trans_namespace_registry_lookup_table_link_checked(
    const LmTransNamespace *namespace_,
    LmP0Text key,
    const char *source_table,
    const char *target_class_table
) {
    const char *target_table;

    target_table = lm_trans_namespace_registry_lookup(namespace_, key, source_table);
    if (
        target_table != 0 &&
        lm_trans_registry_assert_selected_table_link(source_table, key, target_class_table, target_table) != 0
    ) {
        exit(2);
    }
    return target_table;
}

static int lm_trans_binding_resolve(const char *binding, LmTransBinding *out);

static int lm_trans_array_head_find_close(LmP0Text text, size_t open_index, size_t *out_close_index) {
    size_t i;
    size_t depth;

    if (out_close_index == 0 || open_index >= text.length || text.data[open_index] != '[') {
        return 0;
    }

    i = open_index;
    depth = 0U;
    while (i < text.length) {
        if (text.data[i] == '"' || text.data[i] == '`') {
            char quote;

            quote = text.data[i++];
            while (i < text.length) {
                if (quote == '"' && text.data[i] == '\\') {
                    i += 2U;
                    continue;
                }
                if (quote == '`' && text.data[i] == '`' && i + 1U < text.length && text.data[i + 1U] == '`') {
                    i += 2U;
                    continue;
                }
                if (text.data[i++] == quote) {
                    break;
                }
            }
            continue;
        }
        if (text.data[i] == '\'') {
            ++i;
            while (i < text.length) {
                if (text.data[i] == '\\') {
                    i += 2U;
                    continue;
                }
                if (text.data[i++] == '\'') {
                    break;
                }
            }
            continue;
        }
        if (text.data[i] == '[') {
            ++depth;
        } else if (text.data[i] == ']') {
            --depth;
            if (depth == 0U) {
                *out_close_index = i;
                return 1;
            }
        }
        ++i;
    }

    return 0;
}

static int lm_trans_array_head_next_dimension(
    LmP0Text head,
    size_t *index,
    LmP0Text *out_dimension
) {
    size_t close_index;

    if (index == 0 || out_dimension == 0 || *index >= head.length || head.data[*index] != '[') {
        return 0;
    }
    if (!lm_trans_array_head_find_close(head, *index, &close_index)) {
        return 0;
    }

    out_dimension->data = head.data + *index + 1U;
    out_dimension->length = close_index - *index - 1U;
    *index = close_index + 1U;
    return 1;
}

static int lm_trans_text_is_array_receiver_head(LmP0Text head) {
    size_t index;
    size_t count;
    LmP0Text dimension;

    index = 0U;
    count = 0U;
    while (index < head.length) {
        if (!lm_trans_array_head_next_dimension(head, &index, &dimension)) {
            return 0;
        }
        ++count;
    }

    return count > 0U;
}

static LmP0Text *lm_trans_text_ref_new(LmP0Text text) {
    LmP0Text *copy;

    copy = (LmP0Text *)lm_own_new_zero(sizeof(*copy));
    if (copy != 0) {
        *copy = text;
    }
    return copy;
}

static void lm_trans_text_ref_destroy(LmP0Text **text) {
    if (text != 0 && *text != 0) {
        lm_own_delete(*text, 0);
        *text = 0;
    }
}

static void lm_trans_text_ref_delete_any(void *object) {
    lm_own_delete(object, 0);
}

static LmTransFunctionState *lm_trans_function_state_new(void) {
    return (LmTransFunctionState *)lm_own_new_zero(sizeof(LmTransFunctionState));
}

static void lm_trans_function_state_destroy(LmTransFunctionState *state) {
    if (state != 0) {
        lm_trans_text_ref_destroy(&state->current_return_type_name);
        if (state->has_previous_control_stacks) {
            lm_own_ptr_stack_destroy(&state->previous_cleanups);
            lm_own_ptr_stack_destroy(&state->previous_loops);
            state->has_previous_control_stacks = 0;
        }
    }
}

static void lm_trans_function_state_destroy_any(void *object) {
    lm_trans_function_state_destroy((LmTransFunctionState *)object);
}

static void lm_trans_function_state_delete(LmTransFunctionState *state) {
    lm_own_delete(state, lm_trans_function_state_destroy_any);
}

static int lm_trans_registry_has_expr_emitter_binding(LmP0Text key, const char *table) {
    const char *binding;
    LmTransBinding resolved;

    binding = lm_trans_registry_lookup(key, table);
    if (binding == 0) {
        return 0;
    }
    if (
        !lm_trans_binding_resolve(binding, &resolved) ||
        resolved.expr_emit == 0 ||
        resolved.expr_state == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has unknown expression emitter binding %s\n",
            table != 0 ? table : "<none>",
            (int)key.length,
            key.data,
            binding
        );
        exit(2);
    }
    return 1;
}

static const char *lm_trans_expr_emitter_binding_table(const char *class_name) {
    LmP0Text class_text;
    const char *table;

    class_text = lm_trans_text_from_cstr(class_name);
    table = lm_trans_registry_lookup(class_text, "expr_emitter_binding");
    if (
        table != 0 &&
        lm_trans_registry_assert_payload_table_exists("expr_emitter_binding", class_text, table) != 0
    ) {
        exit(2);
    }
    return table;
}

static const char *lm_trans_expr_spelling_binding_table(const char *class_name) {
    LmP0Text class_text;
    const char *table;

    class_text = lm_trans_text_from_cstr(class_name);
    table = lm_trans_registry_lookup(class_text, "expr_spelling_binding");
    if (
        table != 0 &&
        lm_trans_registry_assert_payload_table_exists("expr_spelling_binding", class_text, table) != 0
    ) {
        exit(2);
    }
    return table != 0 ? table : class_name;
}

static int lm_trans_registry_has_expr_emitter_class(LmP0Text key, const char *class_name) {
    const char *table;

    table = lm_trans_expr_emitter_binding_table(class_name);
    return table != 0 && lm_trans_registry_has_expr_emitter_binding(key, table);
}

static int lm_trans_registry_has_expr_emitter_binding_class(
    LmP0Text key,
    const char *class_name
) {
    return lm_trans_registry_has_expr_emitter_class(key, class_name);
}

static int lm_trans_registry_has_any_expr_emitter_binding(LmP0Text key) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;

    rows = lm_trans_registry_relation_stack(
        lm_trans_text_from_cstr("expr_emitter_binding"),
        "row"
    );
    if (rows == 0) {
        return 0;
    }

    i = 0U;
    while (i < rows->count) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->payload != 0 &&
            row->key != 0
        ) {
            if (
                lm_trans_registry_assert_payload_table_exists("expr_emitter_binding", lm_trans_text_from_cstr(row->key), row->payload) != 0
            ) {
                exit(2);
            }
            if (lm_trans_registry_has_expr_emitter_binding(key, row->payload)) {
                return 1;
            }
        }
        ++i;
    }
    return 0;
}

static int lm_trans_text_is_operator_atom(LmP0Text text) {
    return lm_trans_registry_has_any_expr_emitter_binding(text);
}

static int lm_trans_copy_stream(FILE *target, FILE *source) {
    char buffer[4096];
    size_t count;

    if (target == 0 || source == 0) {
        return 1;
    }
    if (fflush(source) != 0 || fseek(source, 0L, SEEK_SET) != 0) {
        return 1;
    }
    while ((count = fread(buffer, 1U, sizeof(buffer), source)) != 0U) {
        if (fwrite(buffer, 1U, count, target) != count) {
            return 1;
        }
    }

    return ferror(source) ? 1 : 0;
}

static char *lm_trans_temp_output_path_new(const char *output_path, const char *suffix) {
    size_t output_length;
    size_t suffix_length;
    char *result;

    if (output_path == 0 || suffix == 0) {
        return 0;
    }

    output_length = strlen(output_path);
    suffix_length = strlen(suffix);
    result = (char *)malloc(output_length + suffix_length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, output_path, output_length);
    memcpy(result + output_length, suffix, suffix_length + 1U);
    return result;
}

static const LmP0Structure *lm_trans_unwrap_single_anonymous_structure(
    const LmP0Structure *structure
) {
    const LmP0Field *field;

    while (structure != 0) {
        field = structure->first_field;
        if (
            field == 0 ||
            field->next != 0 ||
            field->value == 0 ||
            field->value->kind != LM_P0_NODE_STRUCTURE
        ) {
            break;
        }
        structure = &field->value->as.structure;
    }

    return structure;
}

static unsigned long lm_trans_hash_bytes(const char *data, size_t length) {
    unsigned long hash;
    size_t i;

    hash = 2166136261UL;
    for (i = 0U; i < length; ++i) {
        hash ^= (unsigned char)data[i];
        hash *= 16777619UL;
    }
    return hash;
}

static unsigned long lm_trans_hash_cstr(const char *text) {
    return text != 0 ? lm_trans_hash_bytes(text, strlen(text)) : 0UL;
}

static void lm_trans_ptr_stack_remove_pointer(
    LmOwnPtrStack *stack,
    const void *item
) {
    size_t i;

    if (stack == 0 || item == 0) {
        return;
    }

    i = stack->count;
    while (i > 0U) {
        --i;
        if (stack->items[i] == item) {
            while (i + 1U < stack->count) {
                stack->items[i] = stack->items[i + 1U];
                ++i;
            }
            --stack->count;
            stack->items[stack->count] = 0;
            return;
        }
    }
}

static void lm_trans_identifier_relation_destroy(
    LmTransIdentifierRelation *relation
) {
    if (relation != 0) {
        lm_own_ptr_stack_destroy(&relation->symbols);
    }
    lm_own_delete(relation, 0);
}

static void lm_trans_identifier_card_destroy(LmTransIdentifierCard *card) {
    size_t i;
    LmTransIdentifierRelation *relation;
    LmTransIdentifierRelation *next;

    if (card == 0) {
        return;
    }

    lm_own_ptr_stack_destroy(&card->symbols);
    if (card->relation_buckets != 0) {
        for (i = 0U; i < card->relation_bucket_count; ++i) {
            relation = card->relation_buckets[i];
            while (relation != 0) {
                next = relation->next;
                lm_trans_identifier_relation_destroy(relation);
                relation = next;
            }
        }
    }
    free(card->relation_buckets);
    card->relation_buckets = 0;
    card->relation_bucket_count = 0U;
    lm_trans_text_ref_destroy(&card->name);
    free(card->name_storage);
    card->name_storage = 0;
    lm_own_delete(card, 0);
}

static void lm_trans_identifier_table_init(LmTransIdentifierTable *table) {
    if (table != 0) {
        table->buckets = 0;
        table->bucket_count = 0U;
        table->count = 0U;
    }
}

static int lm_trans_identifier_table_ensure_buckets(
    LmTransIdentifierTable *table
) {
    size_t bucket_count;

    if (table == 0) {
        return 1;
    }
    if (table->buckets != 0) {
        return 0;
    }

    bucket_count = 257U;
    table->buckets = (LmTransIdentifierCard **)calloc(
        bucket_count,
        sizeof(*table->buckets)
    );
    if (table->buckets == 0) {
        return 1;
    }
    table->bucket_count = bucket_count;
    return 0;
}

static void lm_trans_identifier_table_destroy(LmTransIdentifierTable *table) {
    size_t i;
    LmTransIdentifierCard *card;
    LmTransIdentifierCard *next;

    if (table == 0) {
        return;
    }

    if (table->buckets != 0) {
        for (i = 0U; i < table->bucket_count; ++i) {
            card = table->buckets[i];
            while (card != 0) {
                next = card->next;
                lm_trans_identifier_card_destroy(card);
                card = next;
            }
        }
    }
    free(table->buckets);
    table->buckets = 0;
    table->bucket_count = 0U;
    table->count = 0U;
}

static LmTransIdentifierCard *lm_trans_identifier_table_find_card(
    const LmTransIdentifierTable *table,
    LmP0Text name
) {
    LmP0Text payload;
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierCard *card;

    if (
        table == 0 ||
        table->buckets == 0 ||
        table->bucket_count == 0U ||
        !lm_trans_identifier_payload(name, &payload)
    ) {
        return 0;
    }

    hash = lm_trans_hash_bytes(payload.data, payload.length);
    bucket = (size_t)(hash % table->bucket_count);
    card = table->buckets[bucket];
    while (card != 0) {
        if (
            card->hash == hash &&
            card->name != 0 &&
            lm_trans_text_same(*card->name, payload)
        ) {
            return card;
        }
        card = card->next;
    }
    return 0;
}

static LmTransIdentifierCard *lm_trans_identifier_table_get_card(
    LmTransIdentifierTable *table,
    LmP0Text name
) {
    LmP0Text payload;
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierCard *card;
    char *copy;

    if (
        table == 0 ||
        !lm_trans_identifier_payload(name, &payload) ||
        lm_trans_identifier_table_ensure_buckets(table) != 0
    ) {
        return 0;
    }

    card = lm_trans_identifier_table_find_card(table, name);
    if (card != 0) {
        return card;
    }

    hash = lm_trans_hash_bytes(payload.data, payload.length);
    bucket = (size_t)(hash % table->bucket_count);
    card = (LmTransIdentifierCard *)lm_own_new_zero(sizeof(*card));
    if (card == 0) {
        return 0;
    }

    copy = lm_trans_text_copy_cstr(payload);
    if (copy == 0) {
        lm_trans_identifier_card_destroy(card);
        return 0;
    }
    card->name_storage = copy;
    card->name = lm_trans_text_ref_new(lm_trans_text_from_cstr(copy));
    if (card->name == 0) {
        lm_trans_identifier_card_destroy(card);
        return 0;
    }

    lm_own_ptr_stack_init(&card->symbols, 0);
    card->relation_bucket_count = 31U;
    card->relation_buckets = (LmTransIdentifierRelation **)calloc(
        card->relation_bucket_count,
        sizeof(*card->relation_buckets)
    );
    if (card->relation_buckets == 0) {
        lm_trans_identifier_card_destroy(card);
        return 0;
    }

    card->hash = hash;
    card->next = table->buckets[bucket];
    table->buckets[bucket] = card;
    ++table->count;
    return card;
}

static LmTransIdentifierRelation *lm_trans_identifier_card_find_relation(
    const LmTransIdentifierCard *card,
    const char *relation_name
) {
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierRelation *relation;

    if (
        card == 0 ||
        relation_name == 0 ||
        card->relation_buckets == 0 ||
        card->relation_bucket_count == 0U
    ) {
        return 0;
    }

    hash = lm_trans_hash_cstr(relation_name);
    bucket = (size_t)(hash % card->relation_bucket_count);
    relation = card->relation_buckets[bucket];
    while (relation != 0) {
        if (relation->name != 0 && strcmp(relation->name, relation_name) == 0) {
            return relation;
        }
        relation = relation->next;
    }
    return 0;
}

static LmTransIdentifierRelation *lm_trans_identifier_card_get_relation(
    LmTransIdentifierCard *card,
    const char *relation_name
) {
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierRelation *relation;

    if (
        card == 0 ||
        relation_name == 0 ||
        card->relation_buckets == 0 ||
        card->relation_bucket_count == 0U
    ) {
        return 0;
    }

    hash = lm_trans_hash_cstr(relation_name);
    bucket = (size_t)(hash % card->relation_bucket_count);
    relation = card->relation_buckets[bucket];
    while (relation != 0) {
        if (relation->name != 0 && strcmp(relation->name, relation_name) == 0) {
            return relation;
        }
        relation = relation->next;
    }

    relation = (LmTransIdentifierRelation *)lm_own_new_zero(sizeof(*relation));
    if (relation == 0) {
        return 0;
    }
    relation->name = relation_name;
    lm_own_ptr_stack_init(&relation->symbols, 0);
    relation->next = card->relation_buckets[bucket];
    card->relation_buckets[bucket] = relation;
    return relation;
}

static int lm_trans_identifier_table_add_symbol(
    LmTransIdentifierTable *table,
    LmTransSymbol *symbol
) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (table == 0 || symbol == 0 || symbol->name == 0) {
        return 1;
    }

    card = lm_trans_identifier_table_get_card(table, *symbol->name);
    if (card == 0) {
        return 1;
    }
    relation = lm_trans_identifier_card_get_relation(card, symbol->class_name);
    if (relation == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&card->symbols, symbol) != 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&relation->symbols, symbol) != 0) {
        lm_trans_ptr_stack_remove_pointer(&card->symbols, symbol);
        return 1;
    }
    return 0;
}

static void lm_trans_identifier_table_remove_symbol(
    LmTransIdentifierTable *table,
    const LmTransSymbol *symbol
) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;

    if (table == 0 || symbol == 0 || symbol->name == 0) {
        return;
    }

    card = lm_trans_identifier_table_find_card(table, *symbol->name);
    if (card == 0) {
        return;
    }
    lm_trans_ptr_stack_remove_pointer(&card->symbols, symbol);
    relation = lm_trans_identifier_card_get_relation(card, symbol->class_name);
    if (relation != 0) {
        lm_trans_ptr_stack_remove_pointer(&relation->symbols, symbol);
    }
}

static LmTransSymbol *lm_trans_identifier_table_find_latest_symbol(
    const LmTransIdentifierTable *table,
    LmP0Text name
) {
    LmTransIdentifierCard *card;

    card = lm_trans_identifier_table_find_card(table, name);
    if (card == 0) {
        return 0;
    }
    return (LmTransSymbol *)lm_own_ptr_stack_top(&card->symbols);
}

static LmTransSymbol *lm_trans_symbol_new(
    LmP0Text name,
    const char *kind,
    unsigned depth
) {
    LmTransSymbol *symbol;

    symbol = (LmTransSymbol *)lm_own_new_zero(sizeof(*symbol));
    if (symbol == 0) {
        return 0;
    }

    symbol->name = lm_trans_text_ref_new(name);
    if (symbol->name == 0) {
        lm_trans_symbol_destroy(symbol);
        return 0;
    }

    lm_own_ptr_stack_init(&symbol->param_names, lm_trans_text_ref_delete_any);
    symbol->class_name = kind;
    symbol->depth = depth;
    return symbol;
}

static int lm_trans_symbol_set_owned_name(LmTransSymbol *symbol, LmP0Text name) {
    char *copy;

    if (symbol == 0 || symbol->name == 0) {
        return 1;
    }

    copy = (char *)malloc(name.length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, name.data, name.length);
    copy[name.length] = '\0';

    free(symbol->name_storage);
    symbol->name_storage = copy;
    symbol->name->data = copy;
    symbol->name->length = name.length;
    return 0;
}

static int lm_trans_symbol_set_c_name(LmTransSymbol *symbol, LmP0Text c_name) {
    char *copy;

    if (symbol == 0) {
        return 1;
    }

    copy = (char *)malloc(c_name.length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, c_name.data, c_name.length);
    copy[c_name.length] = '\0';

    free(symbol->c_name_storage);
    symbol->c_name_storage = copy;
    symbol->c_name.data = copy;
    symbol->c_name.length = c_name.length;
    symbol->has_c_name = 1;
    return 0;
}

static int lm_trans_symbol_set_env_arg(LmTransSymbol *symbol, LmP0Text env_arg) {
    char *copy;

    if (symbol == 0) {
        return 1;
    }

    copy = (char *)malloc(env_arg.length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, env_arg.data, env_arg.length);
    copy[env_arg.length] = '\0';

    free(symbol->env_arg_storage);
    symbol->env_arg_storage = copy;
    symbol->env_arg.data = copy;
    symbol->env_arg.length = env_arg.length;
    symbol->has_env_arg = 1;
    return 0;
}

static int lm_trans_symbol_set_closure_call_name(LmTransSymbol *symbol, LmP0Text closure_call_name) {
    char *copy;

    if (symbol == 0) {
        return 1;
    }

    copy = (char *)malloc(closure_call_name.length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, closure_call_name.data, closure_call_name.length);
    copy[closure_call_name.length] = '\0';

    free(symbol->closure_call_name_storage);
    symbol->closure_call_name_storage = copy;
    symbol->closure_call_name.data = copy;
    symbol->closure_call_name.length = closure_call_name.length;
    symbol->has_closure_call_name = 1;
    return 0;
}

static LmTransCapture *lm_trans_capture_new(
    LmP0Text name,
    LmP0Text type_head,
    const LmP0Node *type_node,
    size_t pointer_depth,
    int type_is_head
) {
    LmTransCapture *capture;

    capture = (LmTransCapture *)lm_own_new_zero(sizeof(*capture));
    if (capture != 0) {
        capture->name = name;
        capture->type_head = type_head;
        capture->type_node = type_node;
        capture->pointer_depth = pointer_depth;
        capture->type_is_head = type_is_head;
    }
    return capture;
}

static void lm_trans_capture_destroy(LmTransCapture *capture) {
    lm_own_delete(capture, 0);
}

static void lm_trans_capture_delete_any(void *object) {
    lm_trans_capture_destroy((LmTransCapture *)object);
}

static void lm_trans_hoisted_function_destroy(LmTransHoistedFunction *function) {
    if (function != 0) {
        lm_own_ptr_stack_destroy(&function->captures);
        free(function->c_name_storage);
        function->c_name_storage = 0;
        free(function->env_type_storage);
        function->env_type_storage = 0;
        free(function->env_var_storage);
        function->env_var_storage = 0;
        free(function->closure_call_storage);
        function->closure_call_storage = 0;
    }
    lm_own_delete(function, 0);
}

static void lm_trans_hoisted_function_delete_any(void *object) {
    lm_trans_hoisted_function_destroy((LmTransHoistedFunction *)object);
}

static LmTransCleanup *lm_trans_cleanup_new(unsigned id) {
    LmTransCleanup *cleanup;

    cleanup = (LmTransCleanup *)lm_own_new_zero(sizeof(*cleanup));
    if (cleanup != 0) {
        cleanup->id = id;
    }
    return cleanup;
}

static void lm_trans_cleanup_destroy(LmTransCleanup *cleanup) {
    lm_own_delete(cleanup, 0);
}

static void lm_trans_cleanup_delete_any(void *object) {
    lm_trans_cleanup_destroy((LmTransCleanup *)object);
}

static LmTransLoop *lm_trans_loop_new(size_t cleanup_base) {
    LmTransLoop *loop;

    loop = (LmTransLoop *)lm_own_new_zero(sizeof(*loop));
    if (loop != 0) {
        loop->cleanup_base = cleanup_base;
    }
    return loop;
}

static void lm_trans_loop_destroy(LmTransLoop *loop) {
    lm_own_delete(loop, 0);
}

static void lm_trans_loop_delete_any(void *object) {
    lm_trans_loop_destroy((LmTransLoop *)object);
}

static LmTransNamespace *lm_trans_namespace_new(void) {
    LmTransNamespace *namespace_;

    namespace_ = (LmTransNamespace *)lm_own_new_zero(sizeof(LmTransNamespace));
    if (namespace_ != 0) {
        lm_own_ptr_stack_init(&namespace_->items, lm_trans_symbol_delete_any);
        lm_trans_identifier_table_init(&namespace_->identifiers);
        namespace_->registry_identifiers = &lm_trans_registry.identifiers;
        lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
        lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);
    }
    return namespace_;
}

static void lm_trans_namespace_destroy(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        lm_trans_identifier_table_destroy(&namespace_->identifiers);
        namespace_->registry_identifiers = 0;
        lm_own_ptr_stack_destroy(&namespace_->items);
        lm_own_ptr_stack_destroy(&namespace_->cleanups);
        lm_own_ptr_stack_destroy(&namespace_->loops);
        namespace_->depth = 0U;
        namespace_->next_cleanup_id = 0U;
        namespace_->return_type_node = 0;
        namespace_->return_type_is_struct = 0;
        lm_trans_text_ref_destroy(&namespace_->return_type_name);
        namespace_->next_return_id = 0U;
        namespace_->hoisted_functions = 0;
    }
}

static void lm_trans_namespace_destroy_any(void *object) {
    lm_trans_namespace_destroy((LmTransNamespace *)object);
}

static void lm_trans_namespace_delete(LmTransNamespace *namespace_) {
    lm_own_delete(namespace_, lm_trans_namespace_destroy_any);
}

static void lm_trans_namespace_enter_scope(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        ++namespace_->depth;
    }
}

static void lm_trans_namespace_leave_scope(LmTransNamespace *namespace_) {
    LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return;
    }

    while (namespace_->items.count > 0U) {
        symbol = (LmTransSymbol *)lm_own_ptr_stack_top(&namespace_->items);
        if (symbol == 0 || symbol->depth != namespace_->depth) {
            break;
        }
        symbol = (LmTransSymbol *)lm_own_ptr_stack_pop(&namespace_->items);
        lm_trans_identifier_table_remove_symbol(&namespace_->identifiers, symbol);
        lm_trans_symbol_destroy(symbol);
    }

    if (namespace_->depth > 0U) {
        --namespace_->depth;
    }
}

static int lm_trans_cleanup_push(LmTransNamespace *namespace_, unsigned id) {
    LmTransCleanup *cleanup;

    if (namespace_ == 0) {
        return 1;
    }

    cleanup = lm_trans_cleanup_new(id);
    if (cleanup == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->cleanups, cleanup) != 0) {
        lm_trans_cleanup_destroy(cleanup);
        return 1;
    }
    return 0;
}

static void lm_trans_cleanup_pop(LmTransNamespace *namespace_) {
    LmTransCleanup *cleanup;

    if (namespace_ != 0) {
        cleanup = (LmTransCleanup *)lm_own_ptr_stack_pop(&namespace_->cleanups);
        lm_trans_cleanup_destroy(cleanup);
    }
}

static int lm_trans_loop_push(LmTransNamespace *namespace_) {
    LmTransLoop *loop;

    if (namespace_ == 0) {
        return 1;
    }

    loop = lm_trans_loop_new(namespace_->cleanups.count);
    if (loop == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->loops, loop) != 0) {
        lm_trans_loop_destroy(loop);
        return 1;
    }
    return 0;
}

static void lm_trans_loop_pop(LmTransNamespace *namespace_) {
    LmTransLoop *loop;

    if (namespace_ != 0) {
        loop = (LmTransLoop *)lm_own_ptr_stack_pop(&namespace_->loops);
        lm_trans_loop_destroy(loop);
    }
}

static size_t lm_trans_loop_cleanup_base(const LmTransNamespace *namespace_) {
    const LmTransLoop *loop;

    if (namespace_ == 0 || namespace_->loops.count == 0U) {
        return namespace_ != 0 ? namespace_->cleanups.count : 0U;
    }

    loop = (const LmTransLoop *)lm_own_ptr_stack_top(&namespace_->loops);
    return loop != 0 ? loop->cleanup_base : namespace_->cleanups.count;
}

static int lm_trans_emit_sync_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_sync_%u", id) < 0 ? 1 : 0;
}

static int lm_trans_emit_return_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_return_%u", id) < 0 ? 1 : 0;
}

static const LmTransSymbol *lm_trans_namespace_find(
    const LmTransNamespace *namespace_,
    LmP0Text name
) {
    if (namespace_ == 0) {
        return 0;
    }

    return lm_trans_identifier_table_find_latest_symbol(&namespace_->identifiers, name);
}

static LmTransSymbol *lm_trans_namespace_find_mutable(
    LmTransNamespace *namespace_,
    LmP0Text name
) {
    if (namespace_ == 0) {
        return 0;
    }

    return lm_trans_identifier_table_find_latest_symbol(&namespace_->identifiers, name);
}

static int lm_trans_is_c_reference_name(LmP0Text name);

static int lm_trans_is_reserved_head_name(LmP0Text name) {
    const char *namespace_class;

    namespace_class = lm_trans_registry_lookup_table_link_checked(name, "namespace", "receiver.type");
    return namespace_class != 0;
}

static int lm_trans_head_binding_resolve(
    const LmTransNamespace *namespace_,
    LmP0Text head,
    LmTransHeadBinding *out
) {
    LmTransBinding resolved;
    int receiver_status;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));
    out->symbol = lm_trans_namespace_find(namespace_, head);
    out->receiver_type = lm_trans_namespace_registry_lookup_table_link_checked(
        namespace_,
        head,
        "namespace",
        "receiver.type"
    );

    out->function_receiver_binding = lm_trans_namespace_registry_lookup(
        namespace_,
        head,
        "receiver.function"
    );
    if (out->function_receiver_binding != 0) {
        receiver_status = lm_trans_binding_resolve(out->function_receiver_binding, &resolved);
        if (!receiver_status || resolved.function_receiver == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: receiver.function[\"%.*s\"] has unknown function binding %s\n",
                (int)head.length,
                head.data,
                out->function_receiver_binding
            );
            return 1;
        }
        out->function_receiver = resolved.function_receiver;
    }

    out->statement_receiver_binding = lm_trans_namespace_registry_lookup(
        namespace_,
        head,
        "receiver.statement"
    );
    if (out->statement_receiver_binding != 0) {
        if (
            !lm_trans_binding_resolve(out->statement_receiver_binding, &resolved) ||
            resolved.statement_frame == 0
        ) {
            fprintf(
                stderr,
                "trans registry inconsistency: receiver.statement[\"%.*s\"] has unknown statement binding %s\n",
                (int)head.length,
                head.data,
                out->statement_receiver_binding
            );
            return 1;
        }
        out->statement_frame = resolved.statement_frame;
    }

    return 0;
}

static int lm_trans_namespace_declare_with_c_name(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const char *kind,
    const LmP0Text *c_name
) {
    LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return 1;
    }

    if (lm_trans_is_c_reference_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: C-surface spelling \"%.*s\" cannot be declared as a Lingvamyxa name\n",
            (int)name.length,
            name.data
        );
        return 1;
    }

    symbol = lm_trans_symbol_new(name, kind, namespace_->depth);
    if (symbol == 0) {
        return 1;
    }
    if (c_name != 0 && lm_trans_symbol_set_c_name(symbol, *c_name) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_trans_identifier_table_add_symbol(&namespace_->identifiers, symbol) != 0) {
        symbol = (LmTransSymbol *)lm_own_ptr_stack_pop(&namespace_->items);
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static int lm_trans_namespace_declare(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const char *kind
) {
    return lm_trans_namespace_declare_with_c_name(namespace_, name, kind, 0);
}

static int lm_trans_namespace_declare_generated(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const char *kind
) {
    const LmTransSymbol *existing;
    LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return 1;
    }

    existing = lm_trans_namespace_find(namespace_, name);
    if (existing != 0) {
        if (lm_trans_symbol_is(existing, kind)) {
            return 0;
        }
        fprintf(
            stderr,
            "trans L2 error: generated name \"%.*s\" is already visible as %s\n",
            (int)name.length,
            name.data,
            lm_trans_symbol_class_name(existing->class_name)
        );
        return 1;
    }

    symbol = lm_trans_symbol_new(name, kind, namespace_->depth);
    if (symbol == 0) {
        return 1;
    }
    if (lm_trans_symbol_set_owned_name(symbol, name) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_trans_identifier_table_add_symbol(&namespace_->identifiers, symbol) != 0) {
        symbol = (LmTransSymbol *)lm_own_ptr_stack_pop(&namespace_->items);
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static int lm_trans_namespace_attach_registry(
    LmTransNamespace *namespace_
) {
    if (namespace_ == 0) {
        return 1;
    }
    namespace_->registry_identifiers = &lm_trans_registry.identifiers;
    return 0;
}

static int lm_trans_namespace_declare_c_name(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const char *kind,
    LmP0Text c_name
) {
    return lm_trans_namespace_declare_with_c_name(namespace_, name, kind, &c_name);
}

static int lm_trans_namespace_set_env_arg(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text env_arg
) {
    LmTransSymbol *symbol;

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }
    return lm_trans_symbol_set_env_arg(symbol, env_arg);
}

static int lm_trans_namespace_set_closure_call_name(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text closure_call_name
) {
    LmTransSymbol *symbol;

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }
    return lm_trans_symbol_set_closure_call_name(symbol, closure_call_name);
}

static int lm_trans_symbol_copy_signature(
    LmTransSymbol *target,
    const LmTransSymbol *source
) {
    size_t i;
    const LmP0Text *source_name;
    LmP0Text *copy;

    if (target == 0 || source == 0) {
        return 1;
    }

    lm_own_ptr_stack_destroy(&target->param_names);
    lm_own_ptr_stack_init(&target->param_names, lm_trans_text_ref_delete_any);
    for (i = 0U; i < source->param_names.count; ++i) {
        source_name = (const LmP0Text *)lm_own_ptr_stack_at(&source->param_names, i);
        if (source_name == 0) {
            return 1;
        }
        copy = lm_trans_text_ref_new(*source_name);
        if (copy == 0) {
            return 1;
        }
        if (lm_own_ptr_stack_push(&target->param_names, copy) != 0) {
            lm_trans_text_ref_delete_any(copy);
            return 1;
        }
    }
    target->has_signature = source->has_signature;
    target->callable_params_node = source->callable_params_node;
    target->callable_return_node = source->callable_return_node;
    target->has_callable_shape = source->has_callable_shape;
    target->callable_returns_value = source->callable_returns_value;
    target->callable_is_struct_return = source->callable_is_struct_return;
    target->callable_is_external = source->callable_is_external;
    return 0;
}

static int lm_trans_namespace_declare_compatible(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const char *kind
) {
    const LmTransSymbol *existing;

    existing = lm_trans_namespace_find(namespace_, name);
    if (existing != 0) {
        if (lm_trans_symbol_is(existing, kind)) {
            return 0;
        }
    }

    return lm_trans_namespace_declare(namespace_, name, kind);
}

static const char *lm_trans_class_c_spelling(LmP0Text name) {
    return lm_trans_registry_lookup(name, "class.spelling");
}

static int lm_trans_builtin_c_type_tail(LmP0Text name) {
    return
        lm_trans_class_c_spelling(name) != 0 ||
        lm_trans_registry_has(name, "class.cTail");
}

static int lm_trans_builtin_c_type_name(LmP0Text name) {
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.") && name.length > 2U) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        return lm_trans_builtin_c_type_tail(tail);
    }

    return lm_trans_builtin_c_type_tail(name);
}

static int lm_trans_emit_name(FILE *file, LmP0Text name) {
    const char *spelling;
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.")) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        spelling = lm_trans_class_c_spelling(tail);
        if (spelling != 0) {
            return lm_trans_put(file, spelling);
        }
        return lm_trans_write_text(file, tail);
    }

    return lm_trans_emit_identifier(file, name);
}

static int lm_trans_emit_type_name(FILE *file, LmP0Text name) {
    const char *spelling;
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.") && name.length > 2U) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        spelling = lm_trans_class_c_spelling(tail);
        if (spelling != 0) {
            return lm_trans_put(file, spelling);
        }
        return lm_trans_write_text(file, tail);
    }

    spelling = lm_trans_class_c_spelling(name);
    if (spelling != 0) {
        return lm_trans_put(file, spelling);
    }
    return lm_trans_emit_identifier(file, name);
}

static int lm_trans_is_c_reference_name(LmP0Text name) {
    return lm_trans_text_starts_with(name, "c.") && name.length > 2U;
}

static int lm_trans_c_reference_has_path_dot(LmP0Text name) {
    size_t i;

    if (!lm_trans_is_c_reference_name(name)) {
        return 0;
    }

    i = 2U;
    while (i < name.length) {
        if (name.data[i] == '.') {
            return 1;
        }
        ++i;
    }

    return 0;
}

static int lm_trans_node_is_c_reference_atom(const LmP0Node *node) {
    return
        node != 0 &&
        node->kind == LM_P0_NODE_ATOM &&
        lm_trans_is_c_reference_name(node->as.atom);
}

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node, const LmTransNamespace *namespace_);
static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first, const LmTransNamespace *namespace_);
static int lm_trans_emit_expr_range(
    FILE *file,
    const LmP0Field *first,
    const LmP0Field *stop,
    const LmTransNamespace *namespace_
);
static int lm_trans_formal_param_name(const LmP0Node *node, LmP0Text *out_name);
static int lm_trans_namespace_declare_storage_binding(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text type_head
);

static int lm_trans_atom_starts_string(LmP0Text text) {
    return text.length > 0U && (text.data[0] == '"' || text.data[0] == '\'');
}

static int lm_trans_atom_is_identifier_like(LmP0Text text) {
    unsigned char ch;

    if (text.length == 0U || lm_trans_atom_starts_string(text)) {
        return 0;
    }

    if (
        lm_trans_text_is_operator_atom(text) ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\")
    ) {
        return 0;
    }

    ch = (unsigned char)text.data[0];
    return
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') ||
        ch == '_' ||
        ch == '`';
}

static int lm_trans_name_argument_is_valid(LmP0Text text) {
    (void)text;
    return 1;
}

static int lm_trans_text_contains_char(LmP0Text text, char ch);

static int lm_trans_validate_expr_atom_mode(
    LmP0Text atom,
    const LmTransNamespace *namespace_,
    int allow_callable_value
) {
    const LmTransSymbol *symbol;

    if (lm_trans_is_c_reference_name(atom) || !lm_trans_atom_is_identifier_like(atom)) {
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 expr error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)atom.length,
            atom.data,
            (int)atom.length,
            atom.data
        );
        return 1;
    }

    if (allow_callable_value && lm_trans_symbol_is_executable_callable(symbol)) {
        return 0;
    }

    if (
        lm_trans_symbol_is(symbol, "procedure") ||
        lm_trans_symbol_is(symbol, "function") ||
        lm_trans_symbol_is(symbol, "label") ||
        lm_trans_symbol_is(symbol, "class") ||
        lm_trans_symbol_is(symbol, "callableDescriptor")
    ) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not an expression value\n",
            (int)atom.length,
            atom.data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    return 0;
}

static int lm_trans_emit_expr_atom_mode(
    FILE *file,
    LmP0Text atom,
    const LmTransNamespace *namespace_,
    int allow_callable_value
) {
    const LmTransSymbol *symbol;

    if (lm_trans_validate_expr_atom_mode(atom, namespace_, allow_callable_value) != 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol != 0 && symbol->has_c_name) {
        return lm_trans_write_text(file, symbol->c_name);
    }

    return lm_trans_emit_name(file, atom);
}

static int lm_trans_emit_expr_atom(
    FILE *file,
    LmP0Text atom,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_expr_atom_mode(file, atom, namespace_, 0);
}

static int lm_trans_call_lower_value(
    LmP0Text head,
    const LmTransSymbol *symbol,
    LmTransCallLowering *out
) {
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa function \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)head.length,
            head.data,
            (int)head.length,
            head.data
        );
        return 1;
    }

    if (!lm_trans_symbol_is_value_callable(symbol)) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not a value-returning function\n",
            (int)head.length,
            head.data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    out->signature = symbol;
    out->is_closure = lm_trans_symbol_is(symbol, "closure");
    return 0;
}

static int lm_trans_call_lower_statement(
    LmP0Text head,
    const LmTransSymbol *symbol,
    LmTransCallLowering *out
) {
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa callable \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)head.length,
            head.data,
            (int)head.length,
            head.data
        );
        return 1;
    }

    if (!lm_trans_symbol_is_executable_callable(symbol)) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not a callable\n",
            (int)head.length,
            head.data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    out->signature = symbol;
    out->is_closure = lm_trans_symbol_is(symbol, "closure");
    return 0;
}

static LmTransCallLoweringHandler lm_trans_call_lowering_handler_binding(const char *binding) {
    LmTransBinding resolved;

    if (lm_trans_binding_resolve(binding, &resolved) && resolved.call_lowering != 0) {
        return resolved.call_lowering;
    }
    return 0;
}

static int lm_trans_lower_call(
    LmP0Text head,
    const LmTransNamespace *namespace_,
    const char *lowering_class,
    LmTransCallLowering *out
) {
    const LmTransSymbol *symbol;
    const char *binding;
    LmTransCallLoweringHandler handler;
    int status;

    if (out == 0) {
        return 1;
    }

    out->name = head;
    out->signature = 0;
    out->is_closure = 0;
    if (lm_trans_is_c_reference_name(head)) {
        return 0;
    }

    binding = lm_trans_registry_lookup(lm_trans_text_from_cstr(lowering_class), "call.lowering");
    handler = lm_trans_call_lowering_handler_binding(binding);
    if (handler == 0) {
        fprintf(
            stderr,
            "trans L2 internal error: call lowering class \"%s\" has no registered handler\n",
            lowering_class != 0 ? lowering_class : ""
        );
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, head);
    status = handler(head, symbol, out);
    if (status == 0 && symbol != 0 && symbol->has_c_name) {
        out->name = symbol->c_name;
    }
    return status;
}

static int lm_trans_atom_is_prefix_expr_operator(LmP0Text text) {
    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.prefix"
    );
}

static int lm_trans_atom_is_postfix_expr_operator(LmP0Text text) {
    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.postfix"
    );
}

static int lm_trans_nodes_touch(const LmP0Node *left, const LmP0Node *right) {
    return
        left != 0 &&
        right != 0 &&
        left->span.offset + left->span.length == right->span.offset;
}

static int lm_trans_atom_is_infix_expr_operator(
    LmP0Text text,
    const LmP0Node *operator_node,
    const LmP0Node *previous_operand
) {
    int registry_result;

    if (lm_trans_text_equals(text, "\\")) {
        if (!lm_trans_registry_has_expr_emitter_class(text, "operator.infix")) {
            return 0;
        }
        registry_result =
            previous_operand != 0 &&
            lm_trans_nodes_touch(previous_operand, operator_node) &&
            !(previous_operand->kind == LM_P0_NODE_ATOM && lm_trans_atom_starts_string(previous_operand->as.atom));
        return registry_result;
    }

    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.infix"
    );
}

static const LmP0Field *lm_trans_expr_segment_end(const LmP0Field *first) {
    const LmP0Field *field;
    const LmP0Field *operand;
    const LmP0Node *previous_operand;
    const LmP0Node *node;
    int bracket_depth;
    int c_dot_path;

    if (first == 0) {
        return 0;
    }

    field = first;
    previous_operand = 0;
    c_dot_path = 0;

    node = field->value;
    while (
        node != 0 &&
        node->kind == LM_P0_NODE_ATOM &&
        lm_trans_atom_is_prefix_expr_operator(node->as.atom)
    ) {
        field = field->next;
        if (field == 0) {
            return 0;
        }
        node = field->value;
    }
    previous_operand = node;
    c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
    field = field->next;

    while (field != 0) {
        node = field->value;
        if (node == 0 || node->kind != LM_P0_NODE_ATOM) {
            break;
        }

        if (lm_trans_text_equals(node->as.atom, "[")) {
            bracket_depth = 1;
            field = field->next;
            while (field != 0 && bracket_depth > 0) {
                node = field->value;
                if (node != 0 && node->kind == LM_P0_NODE_ATOM) {
                    if (lm_trans_text_equals(node->as.atom, "[")) {
                        ++bracket_depth;
                    } else if (lm_trans_text_equals(node->as.atom, "]")) {
                        --bracket_depth;
                    }
                }
                previous_operand = node;
                field = field->next;
            }
        } else if (lm_trans_text_equals(node->as.atom, ".")) {
            if (
                !c_dot_path ||
                previous_operand == 0 ||
                !lm_trans_nodes_touch(previous_operand, node)
            ) {
                break;
            }
            operand = field->next;
            if (operand == 0) {
                return field;
            }
            previous_operand = operand->value;
            c_dot_path = 1;
            field = operand->next;
        } else if (lm_trans_atom_is_postfix_expr_operator(node->as.atom)) {
            if (previous_operand == 0) {
                break;
            }
            previous_operand = node;
            c_dot_path = 0;
            field = field->next;
        } else if (lm_trans_atom_is_infix_expr_operator(node->as.atom, node, previous_operand)) {
            operand = field->next;
            if (operand == 0) {
                return field;
            }
            while (
                operand->value != 0 &&
                operand->value->kind == LM_P0_NODE_ATOM &&
                lm_trans_atom_is_prefix_expr_operator(operand->value->as.atom)
            ) {
                operand = operand->next;
                if (operand == 0) {
                    return field;
                }
            }
            previous_operand = operand->value;
            c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
            field = operand->next;
        } else {
            break;
        }
    }

    return field;
}

static int lm_trans_signature_param_index(
    const LmTransSymbol *callee,
    LmP0Text name,
    size_t *out_index
) {
    size_t i;
    const LmP0Text *param_name;

    if (callee == 0 || out_index == 0) {
        return 0;
    }

    for (i = 0U; i < callee->param_names.count; ++i) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&callee->param_names, i);
        if (param_name != 0 && lm_trans_identifier_same(*param_name, name)) {
            *out_index = i;
            return 1;
        }
    }

    return 0;
}

static int lm_trans_call_field_is_named_argument(
    const LmP0Field *field,
    const LmTransSymbol *callee,
    size_t *out_index
) {
    const LmP0Frame *frame;

    if (
        field == 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME ||
        callee == 0 ||
        out_index == 0
    ) {
        return 0;
    }

    frame = &field->value->as.frame;
    if ((frame->flags & LM_P0_FRAME_COLON) == 0U) {
        return 0;
    }

    return lm_trans_signature_param_index(callee, frame->head, out_index);
}

static LmTransExprLoweredRange *lm_trans_expr_lowered_range_new(void) {
    LmTransExprLoweredRange *range;

    range = (LmTransExprLoweredRange *)lm_own_new_zero(sizeof(LmTransExprLoweredRange));
    if (range != 0) {
        lm_own_value_stack_init(&range->pieces, sizeof(LmTransExprPiece));
    }
    return range;
}

static void lm_trans_expr_lowered_range_destroy(LmTransExprLoweredRange *range) {
    if (range != 0) {
        lm_own_value_stack_destroy(&range->pieces);
        range->index = 0U;
    }
}

static void lm_trans_expr_lowered_range_destroy_any(void *object) {
    lm_trans_expr_lowered_range_destroy((LmTransExprLoweredRange *)object);
}

static void lm_trans_expr_lowered_range_delete(LmTransExprLoweredRange *range) {
    lm_own_delete(range, lm_trans_expr_lowered_range_destroy_any);
}

static int lm_trans_expr_job_emit_text(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_emit_name_text(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_schedule_call_args(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);
static int lm_trans_expr_job_emit_lowered_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
);

static void lm_trans_expr_job_destroy_lowered_range(LmTransExprJob *job) {
    if (job != 0) {
        lm_trans_expr_lowered_range_delete(job->as.lowered_range);
        job->as.lowered_range = 0;
    }
}

static void lm_trans_expr_job_destroy(LmTransExprJob *job) {
    if (job != 0 && job->destroy != 0) {
        job->destroy(job);
        job->destroy = 0;
    }
}

static void lm_trans_expr_stack_destroy(LmTransExprStack *stack) {
    LmTransExprJob job;

    if (stack != 0) {
        while (lm_own_value_stack_pop(&stack->jobs, &job) == 0) {
            lm_trans_expr_job_destroy(&job);
        }
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_expr_stack_push(LmTransExprStack *stack, LmTransExprJob job) {
    if (stack == 0) {
        return 1;
    }

    return lm_own_value_stack_push(&stack->jobs, &job);
}

static int lm_trans_expr_stack_push_text(LmTransExprStack *stack, const char *text) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_text;
    job.destroy = 0;
    job.as.text = text;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_name_text(LmTransExprStack *stack, LmP0Text name) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_name_text;
    job.destroy = 0;
    job.as.name_text = name;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_node(LmTransExprStack *stack, const LmP0Node *node) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_node;
    job.destroy = 0;
    job.as.node = node;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_frame(LmTransExprStack *stack, const LmP0Frame *frame) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_frame;
    job.destroy = 0;
    job.as.frame = frame;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_range_state(
    LmTransExprStack *stack,
    LmTransExprRangeJob range
) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_range;
    job.destroy = 0;
    job.as.range = range;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_range(
    LmTransExprStack *stack,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprRangeJob range;

    range.field = first;
    range.stop = stop;
    range.wrote = 0;
    range.previous_operand = 0;
    range.expect_field_name = 0;
    range.expect_c_field_name = 0;
    range.c_dot_path = 0;
    return lm_trans_expr_stack_push_range_state(stack, range);
}

static int lm_trans_expr_stack_push_call_args(
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_schedule_call_args;
    job.destroy = 0;
    job.as.call_args.body = body;
    job.as.call_args.callee = callee;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_lowered_range(
    LmTransExprStack *stack,
    LmTransExprLoweredRange *range
) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_lowered_range;
    job.destroy = lm_trans_expr_job_destroy_lowered_range;
    job.as.lowered_range = range;
    return lm_trans_expr_stack_push(stack, job);
}

static void lm_trans_expr_segments_init(LmOwnValueStack *segments) {
    lm_own_value_stack_init(segments, sizeof(LmTransExprSegment));
}

static int lm_trans_expr_segments_append(
    LmOwnValueStack *segments,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprSegment segment;

    segment.first = first;
    segment.stop = stop;
    segment.expected_param = 0;
    segment.present = 1;
    return lm_own_value_stack_push(segments, &segment);
}

static int lm_trans_expr_stack_push_name_text(LmTransExprStack *stack, LmP0Text name);
static const LmP0Field *lm_trans_call_body_first_field(const LmP0Structure *body);
static int lm_trans_expr_segments_parse_fields(
    LmOwnValueStack *segments,
    const LmP0Field *first
);
static int lm_trans_expr_stack_push_segments(
    FILE *file,
    LmTransExprStack *stack,
    const LmOwnValueStack *segments,
    const LmTransNamespace *namespace_
);

static int lm_trans_callable_has_no_params(const LmTransSymbol *symbol) {
    return
        symbol != 0 &&
        symbol->callable_params_node != 0 &&
        symbol->callable_params_node->kind == LM_P0_NODE_STRUCTURE &&
        symbol->callable_params_node->as.structure.first_field == 0;
}

static const LmP0Node *lm_trans_callable_param_node_at(
    const LmTransSymbol *symbol,
    size_t index
) {
    const LmP0Field *field;

    if (
        symbol == 0 ||
        symbol->callable_params_node == 0 ||
        symbol->callable_params_node->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }

    field = lm_trans_nth_field(&symbol->callable_params_node->as.structure, index);
    return field != 0 ? field->value : 0;
}

static int lm_trans_array_param_default_fields(
    const LmP0Node *node,
    const LmP0Field **out_first
);

static int lm_trans_formal_param_default_fields(
    const LmP0Node *node,
    const LmP0Field **out_first
) {
    const LmP0Structure *body;
    const LmP0Field *field0;
    const LmP0Field *field1;
    const LmP0Field *inner_field;
    const LmP0Node *param_node;
    LmTransL4CallableType ignored_type;

    if (out_first == 0) {
        return 0;
    }
    *out_first = 0;

    if (node == 0 || node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    if (lm_trans_text_equals(node->as.frame.head, "const")) {
        field0 = node->as.frame.body.first_field;
        param_node = 0;
        if (
            field0 != 0 &&
            field0->next == 0 &&
            field0->value != 0
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as.structure.first_field;
                if (
                    inner_field != 0 &&
                    inner_field->next == 0 &&
                    inner_field->value != 0 &&
                    inner_field->value->kind == LM_P0_NODE_FRAME
                ) {
                    param_node = inner_field->value;
                }
            }
        }
        return lm_trans_formal_param_default_fields(param_node, out_first);
    }

    body = lm_trans_unwrap_single_anonymous_structure(&node->as.frame.body);
    if (lm_trans_text_is_array_receiver_head(node->as.frame.head)) {
        return lm_trans_array_param_default_fields(node, out_first);
    }

    if (lm_trans_l4_callable_type_from_node(node, &ignored_type)) {
        field0 = lm_trans_nth_field(body, 0U);
        if (
            field0 == 0 ||
            field0->value == 0 ||
            field0->value->kind != LM_P0_NODE_ATOM ||
            field0->next == 0
        ) {
            return 0;
        }
        *out_first = field0->next;
        return 1;
    }

    if (lm_trans_text_all_char(node->as.frame.head, '@')) {
        field1 = lm_trans_nth_field(body, 1U);
        if (
            field1 == 0 ||
            field1->value == 0 ||
            field1->value->kind != LM_P0_NODE_ATOM ||
            field1->next == 0
        ) {
            return 0;
        }
        *out_first = field1->next;
        return 1;
    }

    return 0;
}

static int lm_trans_array_body_dimension_can_consume(const LmP0Field *field) {
    return
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM;
}

static int lm_trans_array_param_type_info(
    const LmP0Node *node,
    const LmP0Node **out_element_type,
    size_t *out_rank,
    size_t *out_pointer_depth
) {
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    LmP0Text dimension;
    size_t dimension_index;
    size_t pointer_depth;
    size_t rank;

    if (out_element_type != 0) {
        *out_element_type = 0;
    }
    if (out_rank != 0) {
        *out_rank = 0U;
    }
    if (out_pointer_depth != 0) {
        *out_pointer_depth = 0U;
    }

    if (node == 0 || node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field != 0 && field->next == 0 && field->value != 0) {
            return lm_trans_array_param_type_info(field->value, out_element_type, out_rank, out_pointer_depth);
        }
        return 0;
    }

    if (!lm_trans_text_is_array_receiver_head(frame->head)) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;
    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as.atom, '@')
    ) {
        pointer_depth += field->value->as.atom.length;
        field = field->next;
    }

    type_field = field;
    name_field = type_field != 0 ? type_field->next : 0;
    if (
        type_field == 0 ||
        type_field->value == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    dimension_index = 0U;
    rank = 0U;
    while (lm_trans_array_head_next_dimension(frame->head, &dimension_index, &dimension)) {
        ++rank;
    }
    if (rank == 0U) {
        return 0;
    }

    if (out_element_type != 0) {
        *out_element_type = type_field->value;
    }
    if (out_rank != 0) {
        *out_rank = rank;
    }
    if (out_pointer_depth != 0) {
        *out_pointer_depth = pointer_depth;
    }
    return 1;
}

static int lm_trans_array_param_default_fields(
    const LmP0Node *node,
    const LmP0Field **out_first
) {
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *default_field;
    LmP0Text dimension;
    size_t dimension_index;

    if (out_first != 0) {
        *out_first = 0;
    }
    if (out_first == 0 || node == 0 || node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field != 0 && field->next == 0 && field->value != 0) {
            return lm_trans_array_param_default_fields(field->value, out_first);
        }
        return 0;
    }

    if (!lm_trans_text_is_array_receiver_head(frame->head)) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    field = body != 0 ? body->first_field : 0;
    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as.atom, '@')
    ) {
        field = field->next;
    }

    type_field = field;
    name_field = type_field != 0 ? type_field->next : 0;
    if (
        type_field == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    default_field = name_field->next;
    dimension_index = 0U;
    while (lm_trans_array_head_next_dimension(frame->head, &dimension_index, &dimension)) {
        if (dimension.length == 0U && lm_trans_array_body_dimension_can_consume(default_field)) {
            default_field = default_field->next;
        }
    }

    if (default_field == 0) {
        return 0;
    }

    *out_first = default_field;
    return 1;
}

static int lm_trans_expr_segment_set_default(
    LmTransExprSegment *segment,
    const LmTransSymbol *callee,
    size_t index
) {
    const LmP0Node *param_node;
    const LmP0Field *default_first;

    if (segment == 0 || callee == 0) {
        return 0;
    }

    param_node = lm_trans_callable_param_node_at(callee, index);
    if (!lm_trans_formal_param_default_fields(param_node, &default_first)) {
        return 0;
    }

    segment->first = default_first;
    segment->stop = 0;
    segment->expected_param = param_node;
    segment->present = 1;
    return 1;
}

static int lm_trans_expected_param_is_callable_descriptor(
    const LmP0Node *param,
    const LmTransNamespace *namespace_
);

static int lm_trans_registry_is_function_pointer_type_name(
    const LmTransNamespace *namespace_,
    LmP0Text name
) {
    const LmOwnPtrStack *rows;

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "value"
    );
    if (lm_trans_registry_relation_stack_latest_row(rows, name) != 0) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "row"
    );
    return lm_trans_registry_relation_stack_latest_row(rows, name) != 0;
}

static int lm_trans_node_callable_descriptor_name(
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    LmP0Text *out_name
) {
    const LmTransSymbol *symbol;
    const LmP0Frame *frame;
    const LmP0Field *field;

    if (out_name == 0) {
        return 0;
    }
    out_name->data = "";
    out_name->length = 0U;

    if (node == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        field = node->as.structure.first_field;
        if (field != 0 && field->next == 0) {
            return lm_trans_node_callable_descriptor_name(field->value, namespace_, out_name);
        }
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        symbol = lm_trans_namespace_find(namespace_, node->as.atom);
        if (lm_trans_symbol_is(symbol, "callableDescriptor") && symbol->name != 0) {
            *out_name = *symbol->name;
            return 1;
        }
        if (lm_trans_registry_is_function_pointer_type_name(namespace_, node->as.atom)) {
            *out_name = node->as.atom;
            return 1;
        }
        return 0;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field != 0 && field->next == 0) {
            return lm_trans_node_callable_descriptor_name(field->value, namespace_, out_name);
        }
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, frame->head);
    if (lm_trans_symbol_is(symbol, "callableDescriptor") && symbol->name != 0) {
        *out_name = *symbol->name;
        return 1;
    }
    if (lm_trans_registry_is_function_pointer_type_name(namespace_, frame->head)) {
        *out_name = frame->head;
        return 1;
    }
    return 0;
}

static int lm_trans_expected_param_is_callable_descriptor(
    const LmP0Node *param,
    const LmTransNamespace *namespace_
) {
    LmP0Text descriptor_name;

    return lm_trans_node_callable_descriptor_name(param, namespace_, &descriptor_name);
}

static int lm_trans_callable_type_same(
    const LmTransL4CallableType *left,
    const LmTransL4CallableType *right
) {
    return
        left != 0 &&
        right != 0 &&
        left->address_depth == right->address_depth &&
        left->is_const == right->is_const &&
        lm_trans_identifier_same(left->class_name, right->class_name);
}

static int lm_trans_callable_return_type_from_node(
    const LmP0Node *node,
    LmTransL4CallableType *out
) {
    if (out == 0) {
        return 0;
    }
    if (node == 0) {
        memset(out, 0, sizeof(*out));
        out->class_name = lm_trans_text_from_cstr("void");
        return 1;
    }
    return lm_trans_l4_callable_type_from_node(node, out);
}

static int lm_trans_callable_descriptor_return_type(
    const LmTransNamespace *namespace_,
    LmP0Text descriptor_name,
    LmTransL4CallableType *out
) {
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *class_row;
    LmP0Text return_key;
    size_t const_flag;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    return_key = lm_trans_text_from_cstr("return");
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.class");
    class_row = lm_trans_registry_relation_stack_latest_row(class_rows, return_key);
    if (class_row == 0 || class_row->payload == 0) {
        return 0;
    }

    out->class_name = lm_trans_text_from_cstr(class_row->payload);
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.const");
    if (
        !lm_trans_registry_latest_size_payload(address_depth_rows, return_key, 0U, &out->address_depth) ||
        !lm_trans_registry_latest_size_payload(const_rows, return_key, 0U, &const_flag)
    ) {
        return 0;
    }
    out->is_const = const_flag != 0U;
    return 1;
}

static int lm_trans_callable_signature_matches_descriptor_name(
    LmP0Text descriptor_name,
    const LmTransSymbol *candidate,
    const LmTransNamespace *namespace_
) {
    size_t index;
    const LmP0Node *candidate_param;
    LmTransL4CallableType descriptor_type;
    LmTransL4CallableType candidate_type;
    LmTransAbiParam params[256];
    size_t param_count;
    char *descriptor_cstr;
    int result;

    if (
        candidate == 0 ||
        !lm_trans_symbol_is_executable_callable(candidate)
    ) {
        return 0;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }
    if (lm_trans_collect_abi_params(params, sizeof(params) / sizeof(params[0]), &param_count, namespace_, descriptor_cstr, "callable descriptor") != 0) {
        free(descriptor_cstr);
        return 0;
    }
    free(descriptor_cstr);

    if (param_count != candidate->param_names.count) {
        return 0;
    }

    for (index = 0U; index < param_count; ++index) {
        candidate_param = lm_trans_callable_param_node_at(candidate, index);
        descriptor_type.class_name = lm_trans_text_from_cstr(params[index].class_name);
        descriptor_type.address_depth = params[index].address_depth;
        descriptor_type.is_const = params[index].is_const;
        if (
            !lm_trans_l4_callable_type_from_node(candidate_param, &candidate_type) ||
            !lm_trans_callable_type_same(&descriptor_type, &candidate_type)
        ) {
            return 0;
        }
    }

    result =
        lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, &descriptor_type) &&
        lm_trans_callable_return_type_from_node(candidate->callable_return_node, &candidate_type) &&
        lm_trans_callable_type_same(&descriptor_type, &candidate_type);
    return result;
}

static int lm_trans_callable_descriptor_has_no_params(
    const LmTransNamespace *namespace_,
    LmP0Text descriptor_name
) {
    LmTransAbiParam params[256];
    size_t param_count;
    char *descriptor_cstr;
    int result;

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }

    result =
        lm_trans_collect_abi_params(
            params,
            sizeof(params) / sizeof(params[0]),
            &param_count,
            namespace_,
            descriptor_cstr,
            "callable descriptor"
        ) == 0 &&
        param_count == 0U;
    free(descriptor_cstr);
    return result;
}

static int lm_trans_callable_descriptor_param_count(
    const LmTransNamespace *namespace_,
    LmP0Text descriptor_name,
    size_t *out_count
) {
    LmTransAbiParam params[256];
    char *descriptor_cstr;
    int result;

    if (out_count == 0) {
        return 0;
    }
    *out_count = 0U;

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }
    result =
        lm_trans_collect_abi_params(
            params,
            sizeof(params) / sizeof(params[0]),
            out_count,
            namespace_,
            descriptor_cstr,
            "callable descriptor"
        ) == 0;
    free(descriptor_cstr);
    return result;
}

static int lm_trans_callable_descriptor_return_descriptor_name(
    const LmTransNamespace *namespace_,
    LmP0Text descriptor_name,
    LmP0Text *out_return_descriptor_name
) {
    LmTransL4CallableType return_type;

    if (out_return_descriptor_name == 0) {
        return 0;
    }
    out_return_descriptor_name->data = "";
    out_return_descriptor_name->length = 0U;

    if (
        !lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, &return_type) ||
        return_type.address_depth != 0U ||
        return_type.is_const ||
        !lm_trans_registry_is_function_pointer_type_name(namespace_, return_type.class_name)
    ) {
        return 0;
    }

    *out_return_descriptor_name = return_type.class_name;
    return 1;
}

static int lm_trans_callable_return_chain_depth_to_descriptor(
    const LmTransSymbol *symbol,
    LmP0Text descriptor_name,
    const LmTransNamespace *namespace_,
    size_t *out_depth
) {
    LmP0Text current_descriptor_name;
    size_t depth;

    if (out_depth == 0) {
        return 0;
    }
    *out_depth = 0U;

    if (
        !lm_trans_symbol_is_value_callable(symbol) ||
        !lm_trans_callable_has_no_params(symbol) ||
        !lm_trans_node_callable_descriptor_name(symbol->callable_return_node, namespace_, &current_descriptor_name)
    ) {
        return 0;
    }

    for (depth = 1U; depth < 64U; ++depth) {
        if (lm_trans_identifier_same(current_descriptor_name, descriptor_name)) {
            *out_depth = depth;
            return 1;
        }

        if (
            !lm_trans_callable_descriptor_has_no_params(namespace_, current_descriptor_name) ||
            !lm_trans_callable_descriptor_return_descriptor_name(namespace_, current_descriptor_name, &current_descriptor_name)
        ) {
            return 0;
        }
    }

    return 0;
}

static const char *lm_trans_string_stack_find(
    const LmOwnPtrStack *stack,
    const char *value
) {
    size_t i;
    const char *item;

    if (stack == 0 || value == 0) {
        return 0;
    }

    for (i = 0U; i < stack->count; ++i) {
        item = (const char *)lm_own_ptr_stack_at(stack, i);
        if (item != 0 && strcmp(item, value) == 0) {
            return item;
        }
    }
    return 0;
}

static int lm_trans_c_identifier_char(char ch) {
    return
        (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') ||
        ch == '_';
}

static char *lm_trans_callable_adapter_name_new(
    LmP0Text source_name,
    LmP0Text descriptor_name
) {
    const char *prefix;
    const char *middle;
    size_t prefix_length;
    size_t middle_length;
    size_t length;
    size_t i;
    size_t out;
    char *name;

    prefix = "lm_trans_adapter_";
    middle = "_as_";
    prefix_length = strlen(prefix);
    middle_length = strlen(middle);
    length = prefix_length + source_name.length + middle_length + descriptor_name.length;
    name = (char *)malloc(length + 1U);
    if (name == 0) {
        return 0;
    }

    memcpy(name, prefix, prefix_length);
    out = prefix_length;
    for (i = 0U; i < source_name.length; ++i) {
        name[out++] = lm_trans_c_identifier_char(source_name.data[i]) ? source_name.data[i] : '_';
    }
    memcpy(name + out, middle, middle_length);
    out += middle_length;
    for (i = 0U; i < descriptor_name.length; ++i) {
        name[out++] = lm_trans_c_identifier_char(descriptor_name.data[i]) ? descriptor_name.data[i] : '_';
    }
    name[out] = '\0';
    return name;
}

static int lm_trans_emit_callable_adapter_args(
    FILE *file,
    const LmTransAbiParam *params,
    size_t param_count
) {
    size_t i;

    if (file == 0 || (params == 0 && param_count != 0U)) {
        return 1;
    }

    for (i = 0U; i < param_count; ++i) {
        if (
            (i != 0U && lm_trans_put(file, ", ") != 0) ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(params[i].name)) != 0
        ) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_type_named(
    FILE *file,
    const LmTransL4CallableType *type,
    LmP0Text name
) {
    size_t i;

    if (file == 0 || type == 0) {
        return 1;
    }
    if (type->is_const && lm_trans_put(file, "const ") != 0) {
        return 1;
    }
    if (lm_trans_emit_type_name(file, type->class_name) != 0) {
        return 1;
    }
    if ((type->address_depth != 0U || name.length != 0U) && lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < type->address_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    if (name.length != 0U && lm_trans_emit_identifier(file, name) != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_callable_param_node_named(
    FILE *file,
    const LmP0Node *param_node,
    LmP0Text name
) {
    LmTransL4CallableType type;

    if (!lm_trans_l4_callable_type_from_node(param_node, &type)) {
        return 1;
    }
    return lm_trans_emit_callable_type_named(file, &type, name);
}

static int lm_trans_emit_callable_symbol_param_slice(
    FILE *file,
    const LmTransSymbol *symbol,
    size_t first,
    size_t count
) {
    size_t i;
    const LmP0Node *param_node;
    const LmP0Text *param_name;

    if (file == 0 || symbol == 0) {
        return 1;
    }

    if (count == 0U) {
        return lm_trans_put(file, "void");
    }

    for (i = 0U; i < count; ++i) {
        if (i != 0U && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        param_node = lm_trans_callable_param_node_at(symbol, first + i);
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&symbol->param_names, first + i);
        if (param_node == 0 || param_name == 0) {
            return 1;
        }
        if (lm_trans_emit_callable_param_node_named(file, param_node, *param_name) != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_adapter_source_prototype(
    FILE *file,
    LmP0Text source_name,
    const LmTransSymbol *source_symbol
) {
    if (
        file == 0 ||
        source_symbol == 0 ||
        !lm_trans_symbol_is_executable_callable(source_symbol) ||
        source_symbol->callable_is_struct_return
    ) {
        return 1;
    }

    if (
        !source_symbol->callable_is_external &&
        lm_trans_put(file, "static ") != 0
    ) {
        return 1;
    }

    if (
        (
            source_symbol->callable_returns_value
                ? lm_trans_emit_type_node(file, source_symbol->callable_return_node)
                : lm_trans_put(file, "void")
        ) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_emit_name(file, source_name) != 0 ||
        lm_trans_put(file, "(") != 0 ||
        lm_trans_emit_callable_symbol_param_slice(
            file,
            source_symbol,
            0U,
            source_symbol->param_names.count
        ) != 0 ||
        lm_trans_put(file, ");\n") != 0
    ) {
        return 1;
    }

    return 0;
}

static int lm_trans_emit_callable_return_chain_adapter(
    FILE *file,
    const LmTransNamespace *namespace_,
    LmP0Text source_name,
    const LmTransSymbol *source_symbol,
    LmP0Text descriptor_name,
    size_t chain_depth,
    LmP0Text *out_adapter_name
) {
    FILE *prelude_file;
    LmTransAbiParam params[256];
    size_t param_count;
    LmTransL4CallableType return_type;
    char *descriptor_cstr;
    char *adapter_name;
    const char *stored_name;
    int returns_void;
    size_t i;

    if (out_adapter_name == 0) {
        return 1;
    }
    out_adapter_name->data = "";
    out_adapter_name->length = 0U;

    adapter_name = lm_trans_callable_adapter_name_new(source_name, descriptor_name);
    if (adapter_name == 0) {
        return 1;
    }

    stored_name = lm_trans_string_stack_find(&lm_trans_emitted_callable_adapters, adapter_name);
    if (stored_name != 0) {
        free(adapter_name);
        out_adapter_name->data = stored_name;
        out_adapter_name->length = strlen(stored_name);
        return 0;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        free(adapter_name);
        return 1;
    }

    if (
        lm_trans_collect_abi_params(
            params,
            sizeof(params) / sizeof(params[0]),
            &param_count,
            namespace_,
            descriptor_cstr,
            "callable adapter"
        ) != 0 ||
        !lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, &return_type)
    ) {
        free(descriptor_cstr);
        free(adapter_name);
        return 1;
    }

    returns_void =
        return_type.address_depth == 0U &&
        !return_type.is_const &&
        lm_trans_identifier_same(return_type.class_name, lm_trans_text_from_cstr("void"));

    prelude_file = lm_trans_prelude_file(file);
    if (
        lm_trans_emit_callable_adapter_source_prototype(prelude_file, source_name, source_symbol) != 0 ||
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_abi_return_type(prelude_file, namespace_, descriptor_cstr, "callable adapter") != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(adapter_name)) != 0 ||
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_abi_params(prelude_file, params, param_count) != 0 ||
        lm_trans_put(prelude_file, ") {\n    ") != 0 ||
        (!returns_void && lm_trans_put(prelude_file, "return ") != 0) ||
        lm_trans_emit_name(prelude_file, source_name) != 0
    ) {
        free(descriptor_cstr);
        free(adapter_name);
        return 1;
    }

    for (i = 0U; i < chain_depth; ++i) {
        if (lm_trans_put(prelude_file, "()") != 0) {
            free(descriptor_cstr);
            free(adapter_name);
            return 1;
        }
    }

    if (
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_callable_adapter_args(prelude_file, params, param_count) != 0 ||
        lm_trans_put(prelude_file, ");\n}\n\n") != 0
    ) {
        free(descriptor_cstr);
        free(adapter_name);
        return 1;
    }

    free(descriptor_cstr);
    if (lm_own_ptr_stack_push(&lm_trans_emitted_callable_adapters, adapter_name) != 0) {
        free(adapter_name);
        return 1;
    }
    out_adapter_name->data = adapter_name;
    out_adapter_name->length = strlen(adapter_name);
    return 0;
}

static char *lm_trans_callable_binder_name_new(
    LmP0Text source_name,
    LmP0Text descriptor_name,
    size_t bound_count
) {
    const char *prefix;
    const char *middle;
    char count_buffer[32];
    size_t prefix_length;
    size_t middle_length;
    size_t count_length;
    size_t length;
    size_t i;
    size_t out;
    char *name;

    prefix = "lm_trans_bind_";
    middle = "_as_";
    snprintf(count_buffer, sizeof(count_buffer), "_%lu", (unsigned long)bound_count);
    prefix_length = strlen(prefix);
    middle_length = strlen(middle);
    count_length = strlen(count_buffer);
    length = prefix_length + source_name.length + middle_length + descriptor_name.length + count_length;
    name = (char *)malloc(length + 1U);
    if (name == 0) {
        return 0;
    }

    memcpy(name, prefix, prefix_length);
    out = prefix_length;
    for (i = 0U; i < source_name.length; ++i) {
        name[out++] = lm_trans_c_identifier_char(source_name.data[i]) ? source_name.data[i] : '_';
    }
    memcpy(name + out, middle, middle_length);
    out += middle_length;
    for (i = 0U; i < descriptor_name.length; ++i) {
        name[out++] = lm_trans_c_identifier_char(descriptor_name.data[i]) ? descriptor_name.data[i] : '_';
    }
    memcpy(name + out, count_buffer, count_length);
    out += count_length;
    name[out] = '\0';
    return name;
}

static char *lm_trans_cstr_suffix_new(const char *base, const char *suffix) {
    size_t base_length;
    size_t suffix_length;
    char *result;

    if (base == 0 || suffix == 0) {
        return 0;
    }
    base_length = strlen(base);
    suffix_length = strlen(suffix);
    result = (char *)malloc(base_length + suffix_length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, base, base_length);
    memcpy(result + base_length, suffix, suffix_length);
    result[base_length + suffix_length] = '\0';
    return result;
}

static int lm_trans_callable_type_is_void(const LmTransL4CallableType *type) {
    return
        type != 0 &&
        type->address_depth == 0U &&
        !type->is_const &&
        lm_trans_identifier_same(type->class_name, lm_trans_text_from_cstr("void"));
}

static int lm_trans_callable_descriptor_accepts_source_bind(
    const LmTransNamespace *namespace_,
    LmP0Text descriptor_name,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    LmTransAbiParam *descriptor_params,
    size_t descriptor_params_capacity,
    size_t *out_descriptor_param_count,
    LmTransL4CallableType *out_return_type
) {
    size_t index;
    size_t descriptor_param_count;
    size_t source_param_count;
    const LmP0Node *source_param;
    LmTransL4CallableType descriptor_type;
    LmTransL4CallableType source_type;
    char *descriptor_cstr;
    int result;

    if (
        namespace_ == 0 ||
        source_symbol == 0 ||
        descriptor_params == 0 ||
        out_descriptor_param_count == 0 ||
        out_return_type == 0 ||
        source_symbol->callable_is_struct_return
    ) {
        return 0;
    }

    *out_descriptor_param_count = 0U;
    memset(out_return_type, 0, sizeof(*out_return_type));
    source_param_count = source_symbol->param_names.count;
    if (bound_count > source_param_count) {
        return 0;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }
    result =
        lm_trans_collect_abi_params(
            descriptor_params,
            descriptor_params_capacity,
            &descriptor_param_count,
            namespace_,
            descriptor_cstr,
            "callable binder"
        ) == 0 &&
        lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, out_return_type) &&
        descriptor_param_count + bound_count == source_param_count;
    free(descriptor_cstr);
    if (!result) {
        return 0;
    }

    if (
        !lm_trans_callable_return_type_from_node(source_symbol->callable_return_node, &source_type) ||
        !lm_trans_callable_type_same(out_return_type, &source_type)
    ) {
        return 0;
    }

    for (index = 0U; index < descriptor_param_count; ++index) {
        source_param = lm_trans_callable_param_node_at(source_symbol, bound_count + index);
        descriptor_type.class_name = lm_trans_text_from_cstr(descriptor_params[index].class_name);
        descriptor_type.address_depth = descriptor_params[index].address_depth;
        descriptor_type.is_const = descriptor_params[index].is_const;
        if (
            !lm_trans_l4_callable_type_from_node(source_param, &source_type) ||
            !lm_trans_callable_type_same(&descriptor_type, &source_type)
        ) {
            return 0;
        }
    }

    *out_descriptor_param_count = descriptor_param_count;
    return 1;
}

static int lm_trans_call_body_positional_arg_count(
    const LmP0Structure *body,
    size_t *out_count
) {
    const LmP0Field *field;
    const LmP0Field *next;
    const LmP0Frame *frame;

    if (out_count == 0) {
        return 0;
    }

    *out_count = 0U;
    field = lm_trans_call_body_first_field(body);
    while (field != 0) {
        if (
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_FRAME
        ) {
            frame = &field->value->as.frame;
            if ((frame->flags & LM_P0_FRAME_COLON) != 0U) {
                return 0;
            }
        }
        next = lm_trans_expr_segment_end(field);
        ++*out_count;
        field = next;
    }
    return 1;
}

static int lm_trans_emit_callable_binder_descriptor_param_list(
    FILE *file,
    const LmTransAbiParam *params,
    size_t param_count
) {
    if (param_count == 0U) {
        (void)file;
        (void)params;
        return 0;
    }
    return lm_trans_emit_abi_params(file, params, param_count);
}

static int lm_trans_emit_callable_binder_source_call_args(
    FILE *file,
    const LmTransSymbol *source_symbol,
    const LmTransAbiParam *descriptor_params,
    size_t descriptor_param_count,
    size_t bound_count,
    const char *env_var_name
) {
    size_t index;
    const LmP0Text *param_name;
    int wrote;

    if (file == 0 || source_symbol == 0 || env_var_name == 0) {
        return 1;
    }

    wrote = 0;
    for (index = 0U; index < bound_count; ++index) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&source_symbol->param_names, index);
        if (param_name == 0) {
            return 1;
        }
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(env_var_name)) != 0 ||
            lm_trans_put(file, "->") != 0 ||
            lm_trans_emit_identifier(file, *param_name) != 0
        ) {
            return 1;
        }
        wrote = 1;
    }

    for (index = 0U; index < descriptor_param_count; ++index) {
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, lm_trans_text_from_cstr(descriptor_params[index].name)) != 0) {
            return 1;
        }
        wrote = 1;
    }
    return 0;
}

static int lm_trans_emit_callable_binder_env_fields(
    FILE *file,
    const LmTransSymbol *source_symbol,
    size_t bound_count
) {
    size_t index;
    const LmP0Node *param_node;
    const LmP0Text *param_name;

    for (index = 0U; index < bound_count; ++index) {
        param_node = lm_trans_callable_param_node_at(source_symbol, index);
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&source_symbol->param_names, index);
        if (param_node == 0 || param_name == 0) {
            return 1;
        }
        if (
            lm_trans_put(file, "    ") != 0 ||
            lm_trans_emit_callable_param_node_named(file, param_node, *param_name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_binder_env_assignments(
    FILE *file,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    const char *env_name
) {
    size_t index;
    const LmP0Text *param_name;

    for (index = 0U; index < bound_count; ++index) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&source_symbol->param_names, index);
        if (param_name == 0) {
            return 1;
        }
        if (
            lm_trans_put(file, "    ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(env_name)) != 0 ||
            lm_trans_put(file, "->") != 0 ||
            lm_trans_emit_identifier(file, *param_name) != 0 ||
            lm_trans_put(file, " = ") != 0 ||
            lm_trans_emit_identifier(file, *param_name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_binder(
    FILE *file,
    const LmTransNamespace *namespace_,
    LmP0Text source_name,
    const LmTransSymbol *source_symbol,
    LmP0Text descriptor_name,
    size_t bound_count,
    LmP0Text *out_binder_name
) {
    FILE *prelude_file;
    LmTransAbiParam descriptor_params[256];
    size_t descriptor_param_count;
    LmTransL4CallableType return_type;
    char *binder_name;
    char *env_type_name;
    char *call_name;
    char *destroy_name;
    const char *stored_name;
    int returns_void;

    if (out_binder_name == 0) {
        return 1;
    }
    out_binder_name->data = "";
    out_binder_name->length = 0U;

    if (
        !lm_trans_callable_descriptor_accepts_source_bind(
            namespace_,
            descriptor_name,
            source_symbol,
            bound_count,
            descriptor_params,
            sizeof(descriptor_params) / sizeof(descriptor_params[0]),
            &descriptor_param_count,
            &return_type
        )
    ) {
        return 0;
    }

    binder_name = lm_trans_callable_binder_name_new(source_name, descriptor_name, bound_count);
    if (binder_name == 0) {
        return 1;
    }
    stored_name = lm_trans_string_stack_find(&lm_trans_emitted_callable_binders, binder_name);
    if (stored_name != 0) {
        free(binder_name);
        out_binder_name->data = stored_name;
        out_binder_name->length = strlen(stored_name);
        return 0;
    }

    env_type_name = lm_trans_cstr_suffix_new(binder_name, "Env");
    call_name = lm_trans_cstr_suffix_new(binder_name, "_call");
    destroy_name = lm_trans_cstr_suffix_new(binder_name, "_destroy");
    if (env_type_name == 0 || call_name == 0 || destroy_name == 0) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }

    prelude_file = lm_trans_prelude_file(file);
    returns_void = lm_trans_callable_type_is_void(&return_type);
    if (lm_trans_emit_callable_adapter_source_prototype(prelude_file, source_name, source_symbol) != 0) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }

    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "typedef struct ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, ";\nstruct ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " {\n") != 0 ||
            lm_trans_emit_callable_binder_env_fields(prelude_file, source_symbol, bound_count) != 0 ||
            lm_trans_put(prelude_file, "};\n") != 0
        ) {
            free(env_type_name);
            free(call_name);
            free(destroy_name);
            free(binder_name);
            return 1;
        }
    }

    if (
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_callable_type_named(prelude_file, &return_type, lm_trans_text_from_cstr("")) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(call_name)) != 0 ||
        lm_trans_put(prelude_file, "(void *lm_env") != 0 ||
        (descriptor_param_count != 0U && lm_trans_put(prelude_file, ", ") != 0) ||
        lm_trans_emit_callable_binder_descriptor_param_list(prelude_file, descriptor_params, descriptor_param_count) != 0 ||
        lm_trans_put(prelude_file, ") {\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }
    if (bound_count == 0U) {
        if (lm_trans_put(prelude_file, "    (void)lm_env;\n") != 0) {
            free(env_type_name);
            free(call_name);
            free(destroy_name);
            free(binder_name);
            return 1;
        }
    } else if (
        lm_trans_put(prelude_file, "    ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
        lm_trans_put(prelude_file, " *lm_bind_env = (") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
        lm_trans_put(prelude_file, " *)lm_env;\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }
    if (
        lm_trans_put(prelude_file, "    ") != 0 ||
        (!returns_void && lm_trans_put(prelude_file, "return ") != 0) ||
        lm_trans_emit_name(prelude_file, source_name) != 0 ||
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_callable_binder_source_call_args(
            prelude_file,
            source_symbol,
            descriptor_params,
            descriptor_param_count,
            bound_count,
            bound_count != 0U ? "lm_bind_env" : "lm_env"
        ) != 0 ||
        lm_trans_put(prelude_file, ");\n}\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }

    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "static void ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(destroy_name)) != 0 ||
            lm_trans_put(prelude_file, "(void *lm_env) {\n    lm_own_delete(lm_env, 0);\n}\n") != 0
        ) {
            free(env_type_name);
            free(call_name);
            free(destroy_name);
            free(binder_name);
            return 1;
        }
    }

    if (
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_type_name(prelude_file, descriptor_name) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(binder_name)) != 0 ||
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_callable_symbol_param_slice(prelude_file, source_symbol, 0U, bound_count) != 0 ||
        lm_trans_put(prelude_file, ") {\n    ") != 0 ||
        lm_trans_emit_type_name(prelude_file, descriptor_name) != 0 ||
        lm_trans_put(prelude_file, " lm_result;\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }
    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "    ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " *lm_env;\n") != 0
        ) {
            free(env_type_name);
            free(call_name);
            free(destroy_name);
            free(binder_name);
            return 1;
        }
    }
    if (
        lm_trans_put(prelude_file, "    lm_result = (") != 0 ||
        lm_trans_emit_type_name(prelude_file, descriptor_name) != 0 ||
        lm_trans_put(prelude_file, ")lm_own_new_zero(sizeof(*lm_result));\n    if (lm_result == 0) {\n        return 0;\n    }\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }
    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "    lm_env = (") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " *)lm_own_new_zero(sizeof(*lm_env));\n    if (lm_env == 0) {\n        lm_own_delete(lm_result, 0);\n        return 0;\n    }\n") != 0 ||
            lm_trans_emit_callable_binder_env_assignments(prelude_file, source_symbol, bound_count, "lm_env") != 0
        ) {
            free(env_type_name);
            free(call_name);
            free(destroy_name);
            free(binder_name);
            return 1;
        }
    }
    if (
        lm_trans_put(prelude_file, "    lm_result->call = ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(call_name)) != 0 ||
        lm_trans_put(prelude_file, ";\n    lm_result->env = ") != 0 ||
        (
            bound_count != 0U
                ? lm_trans_put(prelude_file, "lm_env")
                : lm_trans_put(prelude_file, "0")
        ) != 0 ||
        lm_trans_put(prelude_file, ";\n    lm_result->destroy = ") != 0 ||
        (
            bound_count != 0U
                ? lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(destroy_name))
                : lm_trans_put(prelude_file, "0")
        ) != 0 ||
        lm_trans_put(prelude_file, ";\n    return lm_result;\n}\n\n") != 0
    ) {
        free(env_type_name);
        free(call_name);
        free(destroy_name);
        free(binder_name);
        return 1;
    }

    free(env_type_name);
    free(call_name);
    free(destroy_name);
    if (lm_own_ptr_stack_push(&lm_trans_emitted_callable_binders, binder_name) != 0) {
        free(binder_name);
        return 1;
    }
    out_binder_name->data = binder_name;
    out_binder_name->length = strlen(binder_name);
    return 0;
}

static int lm_trans_expr_stack_push_lazy_binder_call(
    FILE *file,
    LmTransExprStack *stack,
    LmP0Text binder_name,
    const LmP0Structure *body,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    const LmTransNamespace *namespace_
) {
    LmOwnValueStack segments;
    const LmP0Field *first;
    const LmP0Field *field;
    const LmP0Field *next;
    LmTransExprSegment *segment;
    size_t index;
    int status;

    if (
        lm_trans_expr_stack_push_text(stack, ")") != 0
    ) {
        return 1;
    }

    first = lm_trans_call_body_first_field(body);
    lm_trans_expr_segments_init(&segments);
    status = lm_own_value_stack_resize_zero(&segments, bound_count);
    field = first;
    index = 0U;
    while (status == 0 && field != 0) {
        if (index >= bound_count) {
            fprintf(stderr, "trans L2 error: too many lazy-bind arguments\n");
            status = 1;
            break;
        }
        segment = (LmTransExprSegment *)lm_own_value_stack_at(&segments, index);
        if (segment == 0) {
            status = 1;
            break;
        }
        if (lm_trans_node_is_positional_skip(field->value)) {
            if (!lm_trans_expr_segment_set_default(segment, source_symbol, index)) {
                fprintf(stderr, "trans L2 error: skipped lazy-bind argument has no default\n");
                status = 1;
                break;
            }
            ++index;
            field = field->next;
            continue;
        }
        next = lm_trans_expr_segment_end(field);
        segment->first = field;
        segment->stop = next;
        segment->expected_param = lm_trans_callable_param_node_at(source_symbol, index);
        segment->present = 1;
        ++index;
        field = next;
    }
    while (status == 0 && index < bound_count) {
        segment = (LmTransExprSegment *)lm_own_value_stack_at(&segments, index);
        if (segment == 0) {
            status = 1;
            break;
        }
        if (!lm_trans_expr_segment_set_default(segment, source_symbol, index)) {
            fprintf(stderr, "trans L2 error: missing lazy-bind argument has no default\n");
            status = 1;
            break;
        }
        ++index;
    }
    if (status == 0) {
        status = lm_trans_expr_stack_push_segments(file, stack, &segments, namespace_);
    }
    lm_own_value_stack_destroy(&segments);
    if (status != 0) {
        return 1;
    }

    if (
        lm_trans_expr_stack_push_text(stack, "(") != 0 ||
        lm_trans_expr_stack_push_name_text(stack, binder_name) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_param_descriptor_matches_return(
    const LmP0Node *param,
    const LmP0Node *return_node,
    const LmTransNamespace *namespace_
) {
    const LmP0Frame *frame;
    const LmP0Field *field;

    if (param == 0 || return_node == 0) {
        return 0;
    }

    if (lm_trans_expected_param_is_callable_descriptor(param, namespace_)) {
        return 0;
    }

    if (param->kind == LM_P0_NODE_STRUCTURE) {
        field = param->as.structure.first_field;
        if (field != 0 && field->next == 0) {
            return lm_trans_param_descriptor_matches_return(field->value, return_node, namespace_);
        }
        return 0;
    }

    if (param->kind == LM_P0_NODE_ATOM) {
        return
            return_node->kind == LM_P0_NODE_ATOM &&
            lm_trans_identifier_same(param->as.atom, return_node->as.atom);
    }

    if (param->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = &param->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field != 0 && field->next == 0) {
            return lm_trans_param_descriptor_matches_return(field->value, return_node, namespace_);
        }
        return 0;
    }

    if (return_node->kind != LM_P0_NODE_ATOM) {
        return 0;
    }

    return lm_trans_identifier_same(frame->head, return_node->as.atom);
}

static int lm_trans_should_force_zero_arg_callable(
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    LmP0Text *out_name,
    const LmTransSymbol **out_symbol
) {
    const LmTransSymbol *symbol;
    LmP0Text atom;

    if (out_name != 0) {
        out_name->data = "";
        out_name->length = 0U;
    }
    if (out_symbol != 0) {
        *out_symbol = 0;
    }
    if (
        segment == 0 ||
        segment->expected_param == 0 ||
        segment->first == 0 ||
        segment->first->next != segment->stop ||
        segment->first->value == 0 ||
        segment->first->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    atom = segment->first->value->as.atom;
    symbol = lm_trans_namespace_find(namespace_, atom);
    if (
        !lm_trans_symbol_is_value_callable(symbol) ||
        !lm_trans_callable_has_no_params(symbol) ||
        !lm_trans_param_descriptor_matches_return(segment->expected_param, symbol->callable_return_node, namespace_)
    ) {
        return 0;
    }

    if (out_name != 0) {
        *out_name = symbol->has_c_name ? symbol->c_name : atom;
    }
    if (out_symbol != 0) {
        *out_symbol = symbol;
    }
    return 1;
}

static int lm_trans_expr_stack_push_forced_zero_arg_callable(
    LmTransExprStack *stack,
    LmP0Text name,
    const LmTransSymbol *symbol
) {
    if (stack == 0 || symbol == 0) {
        return 1;
    }

    if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
        return 1;
    }

    if (lm_trans_symbol_is(symbol, "closure")) {
        if (
            lm_trans_expr_stack_push_text(stack, "->env") != 0 ||
            lm_trans_expr_stack_push_name_text(stack, name) != 0 ||
            lm_trans_expr_stack_push_text(stack, "->call(") != 0 ||
            lm_trans_expr_stack_push_name_text(stack, name) != 0
        ) {
            return 1;
        }
        return 0;
    }

    if (
        lm_trans_expr_stack_push_text(stack, "(") != 0 ||
        lm_trans_expr_stack_push_name_text(stack, name) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_materialize_zero_arg_callable(
    FILE *file,
    LmTransExprStack *stack,
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmTransSymbol *forced_symbol;
    LmP0Text forced_name;

    (void)file;
    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    if (!lm_trans_should_force_zero_arg_callable(segment, namespace_, &forced_name, &forced_symbol)) {
        return 0;
    }

    if (lm_trans_expr_stack_push_forced_zero_arg_callable(stack, forced_name, forced_symbol) != 0) {
        return 1;
    }
    *out_consumed = 1;
    return 0;
}

static int lm_trans_materialize_callable_descriptor_value(
    FILE *file,
    LmTransExprStack *stack,
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmTransSymbol *symbol;
    const LmP0Node *node;
    const LmP0Frame *frame;
    LmP0Text descriptor_name;
    LmP0Text atom;
    LmP0Text name;
    LmP0Text adapter_name;
    LmP0Text binder_name;
    size_t chain_depth;
    size_t explicit_bound_count;
    size_t bound_count;
    size_t descriptor_param_count;
    int descriptor_is_function_pointer;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    if (
        !lm_trans_node_callable_descriptor_name(segment != 0 ? segment->expected_param : 0, namespace_, &descriptor_name) ||
        segment == 0 ||
        segment->first == 0 ||
        segment->first->next != segment->stop ||
        segment->first->value == 0
    ) {
        return 0;
    }

    descriptor_is_function_pointer = lm_trans_registry_is_function_pointer_type_name(namespace_, descriptor_name);
    node = segment->first->value;
    if (node->kind == LM_P0_NODE_FRAME) {
        frame = &node->as.frame;
        symbol = lm_trans_namespace_find(namespace_, frame->head);
        if (
            descriptor_is_function_pointer ||
            symbol == 0 ||
            !lm_trans_symbol_is_executable_callable(symbol) ||
            lm_trans_symbol_is(symbol, "closure") ||
            !lm_trans_call_body_positional_arg_count(&frame->body, &explicit_bound_count) ||
            !lm_trans_callable_descriptor_param_count(namespace_, descriptor_name, &descriptor_param_count) ||
            symbol->param_names.count < descriptor_param_count
        ) {
            return 0;
        }
        bound_count = symbol->param_names.count - descriptor_param_count;
        if (explicit_bound_count > bound_count) {
            fprintf(stderr, "trans L2 error: too many lazy-bind arguments for callable descriptor %.*s\n", (int)descriptor_name.length, descriptor_name.data);
            return 1;
        }

        name = symbol->has_c_name ? symbol->c_name : frame->head;
        if (lm_trans_emit_callable_binder(file, namespace_, name, symbol, descriptor_name, bound_count, &binder_name) != 0) {
            return 1;
        }
        if (binder_name.length == 0U) {
            return 0;
        }
        if (lm_trans_expr_stack_push_lazy_binder_call(file, stack, binder_name, &frame->body, symbol, bound_count, namespace_) != 0) {
            return 1;
        }
        *out_consumed = 1;
        return 0;
    }

    if (node->kind != LM_P0_NODE_ATOM) {
        return 0;
    }

    atom = node->as.atom;
    if (lm_trans_is_c_reference_name(atom)) {
        if (!descriptor_is_function_pointer) {
            return 0;
        }
        if (lm_trans_expr_stack_push_name_text(stack, atom) != 0) {
            return 1;
        }
        *out_consumed = 1;
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol == 0 || !lm_trans_symbol_is_executable_callable(symbol)) {
        return 0;
    }

    name = symbol->has_c_name ? symbol->c_name : atom;
    if (lm_trans_callable_signature_matches_descriptor_name(descriptor_name, symbol, namespace_)) {
        if (!descriptor_is_function_pointer && !lm_trans_symbol_is(symbol, "closure")) {
            if (lm_trans_emit_callable_binder(file, namespace_, name, symbol, descriptor_name, 0U, &binder_name) != 0) {
                return 1;
            }
            if (binder_name.length == 0U) {
                return 0;
            }
            if (lm_trans_expr_stack_push_lazy_binder_call(file, stack, binder_name, 0, symbol, 0U, namespace_) != 0) {
                return 1;
            }
            *out_consumed = 1;
            return 0;
        }
        if (lm_trans_expr_stack_push_name_text(stack, name) != 0) {
            return 1;
        }
        *out_consumed = 1;
        return 0;
    }

    if (
        descriptor_is_function_pointer &&
        !lm_trans_symbol_is(symbol, "closure") &&
        lm_trans_callable_return_chain_depth_to_descriptor(symbol, descriptor_name, namespace_, &chain_depth)
    ) {
        if (
            lm_trans_emit_callable_return_chain_adapter(
                file,
                namespace_,
                name,
                symbol,
                descriptor_name,
                chain_depth,
                &adapter_name
            ) != 0 ||
            lm_trans_expr_stack_push_name_text(stack, adapter_name) != 0
        ) {
            return 1;
        }
        *out_consumed = 1;
        return 0;
    }

    fprintf(
        stderr,
        "trans L2 error: \"%.*s\" neither matches callable descriptor %.*s nor has a no-argument return chain to it\n",
        (int)atom.length,
        atom.data,
        (int)descriptor_name.length,
        descriptor_name.data
    );
    return 1;
}

static int lm_trans_expr_stack_try_materialize_segment(
    FILE *file,
    LmTransExprStack *stack,
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const char *binding;
    const char *class_name;
    LmTransBinding resolved;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    if (lm_trans_array_param_type_info(segment != 0 ? segment->expected_param : 0, 0, 0, 0)) {
        class_name = "array.value";
    } else if (lm_trans_expected_param_is_callable_descriptor(segment != 0 ? segment->expected_param : 0, namespace_)) {
        class_name = "callable.descriptor.value";
    } else {
        class_name = "callable.zero-arg.value";
    }

    binding = lm_trans_registry_lookup(
        lm_trans_text_from_cstr(class_name),
        "materialize.argument"
    );
    if (binding == 0) {
        return 0;
    }
    if (
        !lm_trans_binding_resolve(binding, &resolved) ||
        resolved.expr_segment_materializer == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: materialize.argument has unknown binding %s\n",
            binding
        );
        return 1;
    }

    return resolved.expr_segment_materializer(file, stack, segment, namespace_, out_consumed);
}

static int lm_trans_expr_segments_parse_fields(
    LmOwnValueStack *segments,
    const LmP0Field *first
) {
    const LmP0Field *field;
    const LmP0Field *next;

    field = first;
    while (field != 0) {
        if (lm_trans_node_is_positional_skip(field->value)) {
            fprintf(stderr, "trans L2 error: skipped positional argument has no default in this profile\n");
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_expr_segments_append(segments, field, next) != 0) {
            return 1;
        }
        field = next;
    }

    return 0;
}

static int lm_trans_expr_stack_push_segments(
    FILE *file,
    LmTransExprStack *stack,
    const LmOwnValueStack *segments,
    const LmTransNamespace *namespace_
) {
    size_t index;
    const LmTransExprSegment *segment;
    int materialized;

    if (segments == 0) {
        return 0;
    }

    index = segments->count;
    while (index > 0U) {
        --index;
        segment = (const LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        if (lm_trans_expr_stack_try_materialize_segment(file, stack, segment, namespace_, &materialized) != 0) {
            return 1;
        }
        if (!materialized) {
            if (lm_trans_expr_stack_push_range(stack, segment->first, segment->stop) != 0) {
                return 1;
            }
        }
        if (index > 0U && lm_trans_expr_stack_push_text(stack, ", ") != 0) {
            return 1;
        }
    }

    return 0;
}

static const LmP0Field *lm_trans_call_body_first_field(const LmP0Structure *body) {
    const LmP0Field *field;

    if (body == 0) {
        return 0;
    }

    field = body->first_field;
    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        field = field->value->as.structure.first_field;
    }

    return field;
}

static int lm_trans_call_args_layout_signature(
    LmOwnValueStack *segments,
    const LmP0Field *field,
    const LmTransSymbol *callee
) {
    const LmP0Field *next;
    LmTransExprSegment *segment;
    const LmP0Frame *named_frame;
    size_t index;
    size_t named_index;
    int named_out_of_position;
    int is_named;

    if (callee == 0) {
        return 1;
    }

    if (lm_own_value_stack_resize_zero(segments, callee->param_names.count) != 0) {
        return 1;
    }

    index = 0U;
    named_out_of_position = 0;
    while (field != 0) {
        is_named = lm_trans_call_field_is_named_argument(field, callee, &named_index);
        if (is_named) {
            segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, named_index);
            if (segment == 0) {
                return 1;
            }
            if (segment->present) {
                fprintf(stderr, "trans L2 error: duplicate named argument\n");
                return 1;
            }
            named_frame = &field->value->as.frame;
            segment->first = named_frame->body.first_field;
            segment->stop = 0;
            segment->expected_param = lm_trans_callable_param_node_at(callee, named_index);
            segment->present = 1;
            if (named_index == index) {
                ++index;
            } else {
                named_out_of_position = 1;
            }
            field = field->next;
            continue;
        }

        if (named_out_of_position) {
            fprintf(stderr, "trans L2 error: positional argument after out-of-position named argument\n");
            return 1;
        }
        if (lm_trans_node_is_positional_skip(field->value)) {
            if (index >= callee->param_names.count) {
                fprintf(stderr, "trans L2 error: too many arguments\n");
                return 1;
            }
            segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, index);
            if (segment == 0) {
                return 1;
            }
            if (!lm_trans_expr_segment_set_default(segment, callee, index)) {
                fprintf(stderr, "trans L2 error: skipped positional argument has no default\n");
                return 1;
            }
            ++index;
            field = field->next;
            continue;
        }
        if (index >= callee->param_names.count) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        segment->first = field;
        segment->stop = next;
        segment->expected_param = lm_trans_callable_param_node_at(callee, index);
        segment->present = 1;
        ++index;
        field = next;
    }

    for (index = 0U; index < callee->param_names.count; ++index) {
        segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        if (!segment->present) {
            if (!lm_trans_expr_segment_set_default(segment, callee, index)) {
                fprintf(stderr, "trans L2 error: missing function argument\n");
                return 1;
            }
        }
    }

    return 0;
}

static int lm_trans_call_args_layout(
    LmOwnValueStack *segments,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    const LmP0Field *field;

    field = lm_trans_call_body_first_field(body);
    if (callee != 0 && callee->has_signature) {
        return lm_trans_call_args_layout_signature(segments, field, callee);
    }

    return lm_trans_expr_segments_parse_fields(segments, field);
}

static int lm_trans_expr_stack_schedule_call_args(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee,
    const LmTransNamespace *namespace_
) {
    LmOwnValueStack segments;
    int status;

    lm_trans_expr_segments_init(&segments);
    status = lm_trans_call_args_layout(&segments, body, callee);
    if (status == 0) {
        status = lm_trans_expr_stack_push_segments(file, stack, &segments, namespace_);
    }
    if (status == 0 && callee != 0 && callee->has_env_arg) {
        if (segments.count > 0U && lm_trans_expr_stack_push_text(stack, ", ") != 0) {
            status = 1;
        } else if (lm_trans_expr_stack_push_text(stack, callee->env_arg.data) != 0) {
            status = 1;
        }
    }
    lm_own_value_stack_destroy(&segments);
    return status;
}

static int lm_trans_expr_stack_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    LmTransCallLowering call;
    int has_args;

    if (frame == 0) {
        return 0;
    }

    if (lm_trans_lower_call(frame->head, namespace_, "function", &call) != 0) {
        return 1;
    }

    if (call.is_closure) {
        has_args = call.signature != 0 && call.signature->param_names.count > 0U;
        if (!has_args && lm_trans_call_body_first_field(&frame->body) != 0) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            return 1;
        }
        if (lm_trans_emit_name(file, call.name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "->call(") != 0) {
            return 1;
        }
        if (lm_trans_emit_name(file, call.name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "->env") != 0) {
            return 1;
        }
        if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
            return 1;
        }
        if (has_args) {
            if (lm_trans_expr_stack_push_call_args(stack, &frame->body, call.signature) != 0) {
                return 1;
            }
            return lm_trans_expr_stack_push_text(stack, ", ");
        }
        return 0;
    }

    if (lm_trans_emit_name(file, call.name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
        return 1;
    }
    return lm_trans_expr_stack_push_call_args(stack, &frame->body, call.signature);
}

static int lm_trans_expr_stack_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
) {
    if (node == 0) {
        return 0;
    }

    if (lm_trans_node_is_positional_skip(node)) {
        fprintf(stderr, "trans L2 error: skipped positional argument has no default in this profile\n");
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_expr_atom(file, node->as.atom, namespace_);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_expr_stack_push_frame(stack, &node->as.frame);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
            return 1;
        }
        if (lm_trans_expr_stack_push_range(stack, node->as.structure.first_field, 0) != 0) {
            return 1;
        }
        return lm_trans_expr_stack_push_text(stack, "(");
    }

    return 0;
}

static int lm_trans_atom_is_operand_like(LmP0Text text) {
    return
        !lm_trans_text_is_operator_atom(text) &&
        !lm_trans_text_equals(text, "@") &&
        !lm_trans_text_equals(text, "\\");
}

static int lm_trans_atom_is_index_operator(LmP0Text text) {
    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.index"
    );
}

static int lm_trans_expr_emit_value(
    FILE *file,
    const LmTransExprAtomLowering *lowering,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_expr_atom(file, lowering->text, namespace_);
}

static int lm_trans_expr_emit_addressable_value(
    FILE *file,
    const LmTransExprAtomLowering *lowering,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_expr_atom_mode(file, lowering->text, namespace_, 1);
}

static int lm_trans_expr_emit_name(
    FILE *file,
    const LmTransExprAtomLowering *lowering,
    const LmTransNamespace *namespace_
) {
    (void)namespace_;
    return lm_trans_emit_name(file, lowering->text);
}

static int lm_trans_expr_emit_raw(
    FILE *file,
    const LmTransExprAtomLowering *lowering,
    const LmTransNamespace *namespace_
) {
    (void)namespace_;
    return lm_trans_write_text(file, lowering->text);
}

static int lm_trans_expr_state_value(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)expect_field_name;
    (void)expect_c_field_name;
    *previous_operand = lm_trans_atom_is_operand_like(lowering->text) ? node : 0;
    *c_dot_path = lm_trans_node_is_c_reference_atom(*previous_operand);
    return 0;
}

static int lm_trans_expr_state_field_name(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)expect_c_field_name;
    if (!lm_trans_atom_is_identifier_like(lowering->text)) {
        fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
        return 1;
    }
    *previous_operand = node;
    *expect_field_name = 0;
    *c_dot_path = 0;
    return 0;
}

static int lm_trans_expr_state_c_field_name(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)expect_field_name;
    if (!lm_trans_atom_is_identifier_like(lowering->text)) {
        fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
        return 1;
    }
    *previous_operand = node;
    *expect_c_field_name = 0;
    *c_dot_path = 1;
    return 0;
}

static int lm_trans_expr_state_c_dot(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)lowering;
    (void)expect_field_name;
    if (
        !*c_dot_path ||
        *previous_operand == 0 ||
        !lm_trans_nodes_touch(*previous_operand, node)
    ) {
        fprintf(stderr, "trans L2 error: C value-field dot must follow a c.name path\n");
        return 1;
    }
    *previous_operand = 0;
    *expect_c_field_name = 1;
    return 0;
}

static int lm_trans_expr_state_pointer_follow(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)lowering;
    (void)node;
    (void)expect_c_field_name;
    *previous_operand = 0;
    *expect_field_name = 1;
    *c_dot_path = 0;
    return 0;
}

static int lm_trans_expr_state_deref(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)lowering;
    (void)node;
    (void)expect_field_name;
    (void)expect_c_field_name;
    *previous_operand = 0;
    *c_dot_path = 0;
    return 0;
}

static int lm_trans_expr_state_operator(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)lowering;
    (void)node;
    (void)expect_field_name;
    (void)expect_c_field_name;
    *previous_operand = 0;
    *c_dot_path = 0;
    return 0;
}

static int lm_trans_expr_state_c_surface(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    (void)lowering;
    (void)expect_field_name;
    (void)expect_c_field_name;
    *previous_operand = node;
    *c_dot_path = 1;
    return 0;
}

static int lm_trans_expr_atom_lowering_set_binding(
    LmTransExprAtomLowering *lowering,
    const char *binding
) {
    LmTransBinding resolved;

    if (
        lowering == 0 ||
        !lm_trans_binding_resolve(binding, &resolved) ||
        resolved.expr_emit == 0 ||
        resolved.expr_state == 0
    ) {
        return 1;
    }

    lowering->emit = resolved.expr_emit;
    lowering->update = resolved.expr_state;
    return 0;
}

static void lm_trans_expr_atom_lowering_set_builtin(
    LmTransExprAtomLowering *lowering,
    LmP0Text text,
    const char *binding
) {
    lowering->text = text;
    if (lm_trans_expr_atom_lowering_set_binding(lowering, binding) != 0) {
        lowering->emit = lm_trans_expr_emit_value;
        lowering->update = lm_trans_expr_state_value;
    }
}

static int lm_trans_expr_atom_lowering_set_from_tables(
    LmTransExprAtomLowering *lowering,
    LmP0Text key,
    const char *spelling_table,
    const char *emitter_table
) {
    const char *spelling;
    const char *binding;

    spelling = lm_trans_registry_lookup(key, spelling_table);
    binding = lm_trans_registry_lookup(key, emitter_table);
    if (
        spelling == 0 ||
        binding == 0 ||
        lm_trans_expr_atom_lowering_set_binding(lowering, binding) != 0
    ) {
        fprintf(
            stderr,
            "trans L2 internal error: expression atom \"%.*s\" has no registered emitter in %s/%s\n",
            (int)key.length,
            key.data,
            spelling_table,
            emitter_table
        );
        return 1;
    }

    lowering->text = lm_trans_text_from_cstr(spelling);
    return 0;
}

static int lm_trans_expr_atom_lowering_set_from_class(
    LmTransExprAtomLowering *lowering,
    LmP0Text key,
    const char *class_name
) {
    const char *spelling_table;
    const char *emitter_table;

    spelling_table = lm_trans_expr_spelling_binding_table(class_name);
    emitter_table = lm_trans_expr_emitter_binding_table(class_name);
    if (emitter_table == 0) {
        fprintf(
            stderr,
            "trans L2 internal error: expression class %s has no emitter binding table\n",
            class_name != 0 ? class_name : ""
        );
        return 1;
    }
    return lm_trans_expr_atom_lowering_set_from_tables(
        lowering,
        key,
        spelling_table,
        emitter_table
    );
}

static int lm_trans_lower_expr_atom(
    const LmP0Node *node,
    const LmP0Node *previous_operand,
    int expect_field_name,
    int expect_c_field_name,
    int c_dot_path,
    LmTransExprAtomLowering *out
) {
    if (out == 0 || node == 0 || node->kind != LM_P0_NODE_ATOM) {
        return 1;
    }

    lm_trans_expr_atom_lowering_set_builtin(out, node->as.atom, "lm_trans_expr_emit_value");

    if (expect_field_name) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as.atom, "lm_trans_expr_emit_field_name");
    } else if (expect_c_field_name) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as.atom, "lm_trans_expr_emit_c_field_name");
    } else if (lm_trans_is_c_reference_name(node->as.atom)) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as.atom, "lm_trans_expr_emit_c_surface");
    } else if (lm_trans_text_equals(node->as.atom, "\\")) {
        if (previous_operand != 0 && lm_trans_nodes_touch(previous_operand, node)) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as.atom,
                "operator.infix"
            );
        } else {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as.atom,
                "operator.prefix"
            );
        }
    } else if (lm_trans_registry_has_expr_emitter_binding(node->as.atom, "expr.atom.emitter")) {
        return lm_trans_expr_atom_lowering_set_from_tables(
            out,
            node->as.atom,
            "expr.atom.ansi_c",
            "expr.atom.emitter"
        );
    } else if (lm_trans_atom_is_index_operator(node->as.atom)) {
        return lm_trans_expr_atom_lowering_set_from_class(
            out,
            node->as.atom,
            "operator.index"
        );
    } else if (lm_trans_atom_is_infix_expr_operator(node->as.atom, node, previous_operand)) {
        return lm_trans_expr_atom_lowering_set_from_class(
            out,
            node->as.atom,
            "operator.infix"
        );
    } else if (lm_trans_text_is_operator_atom(node->as.atom)) {
        if (lm_trans_registry_has_expr_emitter_class(node->as.atom, "operator.prefix")) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as.atom,
                "operator.prefix"
            );
        }
        if (lm_trans_registry_has_expr_emitter_class(node->as.atom, "operator.postfix")) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as.atom,
                "operator.postfix"
            );
        }
    }

    (void)c_dot_path;
    return 0;
}

static int lm_trans_update_expr_atom_lowering_state(
    const LmTransExprAtomLowering *lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    if (lowering == 0 || lowering->update == 0) {
        return 1;
    }

    return lowering->update(
        lowering,
        node,
        previous_operand,
        expect_field_name,
        expect_c_field_name,
        c_dot_path
    );
}

static int lm_trans_emit_expr_atom_lowering(
    FILE *file,
    const LmTransExprAtomLowering *lowering,
    const LmTransNamespace *namespace_
) {
    if (lowering == 0 || lowering->emit == 0) {
        return 1;
    }
    return lowering->emit(file, lowering, namespace_);
}

static int lm_trans_expr_piece_emit_atom(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
);
static int lm_trans_expr_piece_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
);
static int lm_trans_expr_piece_emit_index(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
);

static int lm_trans_expr_lowered_range_append_atom(
    LmTransExprLoweredRange *range,
    int leading_space,
    const LmP0Node *node,
    LmTransExprAtomLowering lowering
) {
    LmTransExprPiece piece;

    piece.emit = lm_trans_expr_piece_emit_atom;
    piece.leading_space = leading_space;
    piece.node = node;
    piece.atom = lowering;
    piece.first = 0;
    piece.stop = 0;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_expr_lowered_range_append_node(
    LmTransExprLoweredRange *range,
    int leading_space,
    const LmP0Node *node
) {
    LmTransExprPiece piece;

    piece.emit = lm_trans_expr_piece_emit_node;
    piece.leading_space = leading_space;
    piece.node = node;
    piece.atom.emit = 0;
    piece.atom.update = 0;
    piece.atom.text.data = 0;
    piece.atom.text.length = 0U;
    piece.first = 0;
    piece.stop = 0;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_expr_lowered_range_append_index(
    LmTransExprLoweredRange *range,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprPiece piece;

    piece.emit = lm_trans_expr_piece_emit_index;
    piece.leading_space = 0;
    piece.node = 0;
    piece.atom.emit = 0;
    piece.atom.update = 0;
    piece.atom.text.data = 0;
    piece.atom.text.length = 0U;
    piece.first = first;
    piece.stop = stop;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_field_is_atom(const LmP0Field *field, const char *spelling) {
    return
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as.atom, spelling);
}

static const LmP0Field *lm_trans_find_matching_index_close(
    const LmP0Field *open,
    const LmP0Field *stop
) {
    const LmP0Field *field;
    int depth;

    if (!lm_trans_field_is_atom(open, "[")) {
        return 0;
    }

    depth = 1;
    field = open->next;
    while (field != stop) {
        if (lm_trans_field_is_atom(field, "[")) {
            ++depth;
        } else if (lm_trans_field_is_atom(field, "]")) {
            --depth;
            if (depth == 0) {
                return field;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_expr_lowered_range_build(
    LmTransExprLoweredRange *lowered,
    LmTransExprRangeJob range
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
    int address_next_operand;
    int piece_leading_space;
    LmTransExprAtomLowering atom;

    if (lowered == 0) {
        return 1;
    }

    field = range.field;
    previous_operand = range.previous_operand;
    expect_field_name = range.expect_field_name;
    expect_c_field_name = range.expect_c_field_name;
    c_dot_path = range.c_dot_path;
    address_next_operand = 0;
    while (field != range.stop) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_field_is_atom(field, "[")) {
                const LmP0Field *close;

                if (previous_operand == 0) {
                    fprintf(stderr, "trans L2 error: index operator expects a target expression\n");
                    return 1;
                }
                close = lm_trans_find_matching_index_close(field, range.stop);
                if (close == 0) {
                    fprintf(stderr, "trans L2 error: unclosed index operator\n");
                    return 1;
                }
                if (lm_trans_expr_lowered_range_append_index(lowered, field->next, close) != 0) {
                    return 1;
                }
                previous_operand = close->value;
                expect_field_name = 0;
                expect_c_field_name = 0;
                c_dot_path = 0;
                range.wrote = 1;
                field = close->next;
                continue;
            } else if (node->kind == LM_P0_NODE_ATOM) {
                piece_leading_space = range.wrote;
                if (lm_trans_lower_expr_atom(
                    node,
                    previous_operand,
                    expect_field_name,
                    expect_c_field_name,
                    c_dot_path,
                    &atom
                ) != 0) {
                    return 1;
                }
                if (
                    address_next_operand &&
                    !expect_field_name &&
                    !expect_c_field_name &&
                    lm_trans_atom_is_operand_like(node->as.atom)
                ) {
                    lm_trans_expr_atom_lowering_set_builtin(
                        &atom,
                        node->as.atom,
                        "lm_trans_expr_emit_addressable_value"
                    );
                    piece_leading_space = 0;
                }
                if (
                    lm_trans_update_expr_atom_lowering_state(
                        &atom,
                        node,
                        &previous_operand,
                        &expect_field_name,
                        &expect_c_field_name,
                        &c_dot_path
                    ) != 0
                ) {
                    return 1;
                }
                if (lm_trans_text_equals(node->as.atom, "@")) {
                    address_next_operand = 1;
                } else if (lm_trans_atom_is_operand_like(node->as.atom)) {
                    address_next_operand = 0;
                }
                if (
                    lm_trans_expr_lowered_range_append_atom(
                        lowered,
                        piece_leading_space,
                        node,
                        atom
                    ) != 0
                ) {
                    return 1;
                }
            } else {
                if (
                    lm_trans_expr_lowered_range_append_node(
                        lowered,
                        range.wrote,
                        node
                    ) != 0
                ) {
                    return 1;
                }
                previous_operand = node;
                expect_field_name = 0;
                expect_c_field_name = 0;
                c_dot_path = 0;
                address_next_operand = 0;
            }
            range.wrote = 1;
        }
        field = field->next;
    }

    if (expect_field_name) {
        fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
        return 1;
    }
    if (expect_c_field_name) {
        fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
        return 1;
    }

    return 0;
}

static int lm_trans_expr_piece_emit_atom(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
) {
    (void)stack;
    if (out_suspend != 0) {
        *out_suspend = 0;
    }
    if (lm_trans_emit_expr_atom_lowering(file, &piece->atom, namespace_) != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    ++lowered->index;
    return 0;
}

static int lm_trans_expr_piece_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
) {
    LmTransExprJob discard;

    (void)file;
    (void)namespace_;
    if (out_suspend != 0) {
        *out_suspend = 1;
    }
    ++lowered->index;
    if (lm_trans_expr_stack_push_lowered_range(stack, lowered) != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    if (lm_trans_expr_stack_push_node(stack, piece->node) != 0) {
        (void)lm_own_value_stack_pop(&stack->jobs, &discard);
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    return 0;
}

static int lm_trans_expr_piece_emit_index(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransExprPiece *piece,
    const LmTransNamespace *namespace_,
    int *out_suspend
) {
    LmTransExprJob discard;

    (void)namespace_;
    if (out_suspend != 0) {
        *out_suspend = 1;
    }
    if (lm_trans_put(file, "[") != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    ++lowered->index;
    if (lm_trans_expr_stack_push_lowered_range(stack, lowered) != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, "]") != 0) {
        (void)lm_own_value_stack_pop(&stack->jobs, &discard);
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    if (lm_trans_expr_stack_push_range(stack, piece->first, piece->stop) != 0) {
        (void)lm_own_value_stack_pop(&stack->jobs, &discard);
        (void)lm_own_value_stack_pop(&stack->jobs, &discard);
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    return 0;
}

static int lm_trans_expr_stack_emit_lowered_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransNamespace *namespace_
) {
    const LmTransExprPiece *piece;
    int suspend;

    if (lowered == 0) {
        return 0;
    }

    while (lowered->index < lowered->pieces.count) {
        piece = (const LmTransExprPiece *)lm_own_value_stack_at(&lowered->pieces, lowered->index);
        if (piece == 0) {
            lm_trans_expr_lowered_range_delete(lowered);
            return 1;
        }
        if (piece->leading_space && lm_trans_put(file, " ") != 0) {
            lm_trans_expr_lowered_range_delete(lowered);
            return 1;
        }
        if (piece->emit == 0) {
            lm_trans_expr_lowered_range_delete(lowered);
            return 1;
        }
        suspend = 0;
        if (piece->emit(file, stack, lowered, piece, namespace_, &suspend) != 0) {
            return 1;
        }
        if (suspend) {
            return 0;
        }
    }

    lm_trans_expr_lowered_range_delete(lowered);
    return 0;
}

static int lm_trans_expr_stack_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprRangeJob range,
    const LmTransNamespace *namespace_
) {
    LmTransExprLoweredRange *lowered;

    lowered = lm_trans_expr_lowered_range_new();
    if (lowered == 0) {
        return 1;
    }
    if (lm_trans_expr_lowered_range_build(lowered, range) != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    return lm_trans_expr_stack_emit_lowered_range(file, stack, lowered, namespace_);
}

static int lm_trans_expr_job_emit_text(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_put(file, job->as.text);
}

static int lm_trans_expr_job_emit_name_text(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_emit_name(file, job->as.name_text);
}

static int lm_trans_expr_job_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_node(file, stack, job->as.node, namespace_);
}

static int lm_trans_expr_job_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_frame(file, stack, job->as.frame, namespace_);
}

static int lm_trans_expr_job_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_range(file, stack, job->as.range, namespace_);
}

static int lm_trans_expr_job_schedule_call_args(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_schedule_call_args(
        file,
        stack,
        job->as.call_args.body,
        job->as.call_args.callee,
        namespace_
    );
}

static int lm_trans_expr_job_emit_lowered_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    LmTransExprLoweredRange *range;

    range = job->as.lowered_range;
    job->as.lowered_range = 0;
    job->destroy = 0;
    return lm_trans_expr_stack_emit_lowered_range(file, stack, range, namespace_);
}

static int lm_trans_emit_expr_stack_run(
    FILE *file,
    LmTransExprJob initial,
    const LmTransNamespace *namespace_
) {
    LmTransExprStack stack;
    LmTransExprJob job;
    int status;

    lm_own_value_stack_init(&stack.jobs, sizeof(LmTransExprJob));
    if (lm_trans_expr_stack_push(&stack, initial) != 0) {
        return 1;
    }

    status = 0;
    while (status == 0 && stack.jobs.count > 0U) {
        if (lm_own_value_stack_pop(&stack.jobs, &job) != 0) {
            status = 1;
            break;
        }
        if (job.run == 0) {
            status = 1;
        } else {
            status = job.run(file, &stack, &job, namespace_);
        }
        lm_trans_expr_job_destroy(&job);
    }

    lm_trans_expr_stack_destroy(&stack);
    return status;
}

static int lm_trans_emit_call_args(
    FILE *file,
    const LmP0Structure *body,
    const LmTransNamespace *namespace_,
    const LmTransSymbol *callee
) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_schedule_call_args;
    job.destroy = 0;
    job.as.call_args.body = body;
    job.as.call_args.callee = callee;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_expr_list(
    FILE *file,
    const LmP0Field *first,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Field *next;
    int wrote;

    wrote = 0;
    field = first;
    while (field != 0) {
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_emit_expr_range(file, field, next, namespace_) != 0) {
            return 1;
        }
        wrote = 1;
        field = next;
    }

    return 0;
}

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node, const LmTransNamespace *namespace_) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_node;
    job.destroy = 0;
    job.as.node = node;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first, const LmTransNamespace *namespace_) {
    return lm_trans_emit_expr_range(file, first, 0, namespace_);
}

static int lm_trans_emit_expr_range(
    FILE *file,
    const LmP0Field *first,
    const LmP0Field *stop,
    const LmTransNamespace *namespace_
) {
    LmTransExprJob job;

    job.run = lm_trans_expr_job_emit_range;
    job.destroy = 0;
    job.as.range.field = first;
    job.as.range.stop = stop;
    job.as.range.wrote = 0;
    job.as.range.previous_operand = 0;
    job.as.range.expect_field_name = 0;
    job.as.range.expect_c_field_name = 0;
    job.as.range.c_dot_path = 0;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_array_type_node_info(
    const LmP0Node *type_node,
    const LmP0Node **out_element_type,
    size_t *out_rank
);
static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node);
static int lm_trans_emit_size_literal(FILE *file, size_t value);

static int lm_trans_type_receiver_key(
    const LmP0Node *type_node,
    LmP0Text *out_key
) {
    if (out_key == 0) {
        return 0;
    }

    if (
        type_node != 0 &&
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_is_array_receiver_head(type_node->as.frame.head)
    ) {
        *out_key = lm_trans_text_from_cstr("[]");
        return 1;
    }

    if (type_node != 0 && type_node->kind == LM_P0_NODE_FRAME) {
        *out_key = type_node->as.frame.head;
        return 1;
    }

    return 0;
}

static int lm_trans_lookup_type_receiver_binding(
    const LmP0Node *type_node,
    LmTransBinding *out
) {
    LmP0Text key;
    const char *binding;
    LmTransBinding resolved;
    int found;

    if (out != 0) {
        memset(out, 0, sizeof(*out));
    }
    if (out == 0 || !lm_trans_type_receiver_key(type_node, &key)) {
        return 0;
    }

    found = 0;

    binding = lm_trans_registry_lookup(key, "class.receiver.emit");
    if (binding != 0) {
        if (!lm_trans_binding_resolve(binding, &resolved) || resolved.type_emit == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.emit[\"%.*s\"] has unknown class emit binding %s\n",
                (int)key.length,
                key.data,
                binding
            );
            return -1;
        }
        out->type_emit = resolved.type_emit;
        found = 1;
    }

    binding = lm_trans_registry_lookup(key, "class.receiver.structure_value_alloc");
    if (binding != 0) {
        if (!lm_trans_binding_resolve(binding, &resolved) || resolved.type_structure_value_alloc == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.structure_value_alloc[\"%.*s\"] has unknown class Structure-value alloc binding %s\n",
                (int)key.length,
                key.data,
                binding
            );
            return -1;
        }
        out->type_structure_value_alloc = resolved.type_structure_value_alloc;
        found = 1;
    }

    binding = lm_trans_registry_lookup(key, "class.receiver.structure_value_fill");
    if (binding != 0) {
        if (!lm_trans_binding_resolve(binding, &resolved) || resolved.type_structure_value_fill == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.structure_value_fill[\"%.*s\"] has unknown class Structure-value fill binding %s\n",
                (int)key.length,
                key.data,
                binding
            );
            return -1;
        }
        out->type_structure_value_fill = resolved.type_structure_value_fill;
        found = 1;
    }

    if (!found) {
        return 0;
    }

    return 1;
}

static int lm_trans_type_receiver_array_emit(FILE *file, const LmP0Node *type_node) {
    const LmP0Node *element_type;
    size_t rank;
    size_t i;
    int status;

    status = lm_trans_array_type_node_info(type_node, &element_type, &rank);
    if (status <= 0) {
        return status < 0 ? 1 : 0;
    }
    if (lm_trans_emit_type_node(file, element_type) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < rank; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node) {
    const LmP0Field *field;
    LmTransBinding type_receiver;
    int type_receiver_status;
    size_t i;

    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: type position expects a name\n");
        return 1;
    }

    if (type_node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_type_name(file, type_node->as.atom);
    }

    if (
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(type_node->as.frame.head, "const")
    ) {
        field = type_node->as.frame.body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: const type qualifier expects exactly one type\n");
            return 1;
        }
        if (lm_trans_put(file, "const ") != 0) {
            return 1;
        }
        return lm_trans_emit_type_node(file, field->value);
    }

    if (
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_all_char(type_node->as.frame.head, '@')
    ) {
        field = type_node->as.frame.body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: pointer type expects exactly one type\n");
            return 1;
        }
        if (lm_trans_emit_type_node(file, field->value) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        for (i = 0U; i < type_node->as.frame.head.length; ++i) {
            if (lm_trans_put(file, "*") != 0) {
                return 1;
            }
        }
        return 0;
    }

    type_receiver_status = lm_trans_lookup_type_receiver_binding(type_node, &type_receiver);
    if (type_receiver_status < 0) {
        return 1;
    }
    if (type_receiver_status > 0) {
        if (type_receiver.type_emit == 0) {
            fprintf(stderr, "trans registry error: type receiver has no type emitter\n");
            return 1;
        }
        return type_receiver.type_emit(file, type_node);
    }

    fprintf(stderr, "trans L2 error: type position expects a name\n");
    return 1;
}

static int lm_trans_head_can_declare_storage(
    LmP0Text head,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_callable_descriptor_param_type(
    FILE *file,
    const LmP0Node *type_node,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_callable_descriptor_param_frame(
    FILE *file,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    size_t i;

    if (frame == 0) {
        return 1;
    }

    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: const callable descriptor parameter expects one type\n");
            return 1;
        }
        if (lm_trans_put(file, "const ") != 0) {
            return 1;
        }
        return lm_trans_emit_callable_descriptor_param_type(file, field->value, namespace_);
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    type_field = body != 0 ? body->first_field : 0;

    if (lm_trans_text_all_char(frame->head, '@')) {
        if (type_field == 0 || type_field->value == 0) {
            fprintf(stderr, "trans L2 error: @ callable descriptor parameter expects a type\n");
            return 1;
        }
        if (lm_trans_emit_callable_descriptor_param_type(file, type_field->value, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        for (i = 0U; i < frame->head.length; ++i) {
            if (lm_trans_put(file, "*") != 0) {
                return 1;
            }
        }
        return 0;
    }

    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        if (type_field == 0 || type_field->value == 0) {
            fprintf(stderr, "trans L2 error: [] callable descriptor parameter expects a type\n");
            return 1;
        }
        if (lm_trans_emit_callable_descriptor_param_type(file, type_field->value, namespace_) != 0) {
            return 1;
        }
        return lm_trans_put(file, " *");
    }

    if (lm_trans_head_can_declare_storage(frame->head, namespace_)) {
        return lm_trans_emit_type_name(file, frame->head);
    }

    fprintf(stderr, "trans L2 error: unsupported callable descriptor parameter type\n");
    return 1;
}

static int lm_trans_emit_callable_descriptor_param_type(
    FILE *file,
    const LmP0Node *type_node,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;

    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: callable descriptor parameter expects a type\n");
        return 1;
    }

    if (type_node->kind == LM_P0_NODE_STRUCTURE) {
        field = type_node->as.structure.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: callable descriptor parameter Structure must contain exactly one type\n");
            return 1;
        }
        return lm_trans_emit_callable_descriptor_param_type(file, field->value, namespace_);
    }

    if (type_node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_emit_callable_descriptor_param_frame(file, &type_node->as.frame, namespace_);
    }

    return lm_trans_emit_type_node(file, type_node);
}

static int lm_trans_emit_function_return_struct_type_name(FILE *file, LmP0Text function_name) {
    if (lm_trans_write_text(file, function_name) != 0) {
        return 1;
    }
    return lm_trans_put(file, "Return");
}

static char *lm_trans_function_return_struct_type_name_new(LmP0Text function_name) {
    char *name;
    const char *suffix;
    size_t suffix_length;

    suffix = "Return";
    suffix_length = strlen(suffix);
    name = (char *)malloc(function_name.length + suffix_length + 1U);
    if (name == 0) {
        return 0;
    }
    memcpy(name, function_name.data, function_name.length);
    memcpy(name + function_name.length, suffix, suffix_length + 1U);
    return name;
}

static int lm_trans_emit_current_return_type(FILE *file, const LmTransNamespace *namespace_) {
    if (namespace_ != 0 && namespace_->return_type_is_struct) {
        if (namespace_->return_type_name == 0) {
            return 1;
        }
        return lm_trans_emit_function_return_struct_type_name(file, *namespace_->return_type_name);
    }
    return lm_trans_emit_type_node(file, namespace_ != 0 ? namespace_->return_type_node : 0);
}

static int lm_trans_array_type_node_info(
    const LmP0Node *type_node,
    const LmP0Node **out_element_type,
    size_t *out_rank
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    LmP0Text dimension;
    size_t dimension_index;
    size_t rank;

    if (out_element_type != 0) {
        *out_element_type = 0;
    }
    if (out_rank != 0) {
        *out_rank = 0U;
    }
    if (
        type_node == 0 ||
        type_node->kind != LM_P0_NODE_FRAME ||
        !lm_trans_text_is_array_receiver_head(type_node->as.frame.head)
    ) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&type_node->as.frame.body);
    field = body != 0 ? body->first_field : 0;
    if (field == 0 || field->next != 0 || field->value == 0) {
        fprintf(stderr, "trans L2 error: [] type receiver expects exactly one element type\n");
        return -1;
    }

    dimension_index = 0U;
    rank = 0U;
    while (lm_trans_array_head_next_dimension(type_node->as.frame.head, &dimension_index, &dimension)) {
        if (dimension.length != 0U) {
            fprintf(stderr, "trans L2 error: [] type receiver dimensions must be empty in pointer type position\n");
            return -1;
        }
        ++rank;
    }

    if (out_element_type != 0) {
        *out_element_type = field->value;
    }
    if (out_rank != 0) {
        *out_rank = rank;
    }
    return 1;
}

static int lm_trans_emit_type_and_name(
    FILE *file,
    const LmP0Node *type_node,
    LmP0Text name,
    size_t pointer_depth,
    const LmTransNamespace *namespace_
) {
    size_t i;

    if (type_node == 0) {
        return 1;
    }

    (void)namespace_;

    if (lm_trans_emit_type_node(file, type_node) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < pointer_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    return lm_trans_emit_identifier(file, name);
}

static int lm_trans_emit_type_head_only(
    FILE *file,
    LmP0Text type_head
) {
    if (lm_trans_builtin_c_type_name(type_head)) {
        return lm_trans_emit_type_name(file, type_head);
    }
    return lm_trans_emit_name(file, type_head);
}

static int lm_trans_emit_c_dimension_text(
    FILE *file,
    LmP0Text dimension,
    const LmTransNamespace *namespace_,
    const char *error_name
) {
    LmP0Document *dimension_document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *dimension_root;
    int parse_status;

    if (dimension.length == 0U) {
        return 0;
    }

    dimension_document = 0;
    parse_status = lm_p0_parse_bytes(dimension.data, dimension.length, &dimension_document);
    if (parse_status != 0) {
        diagnostic = dimension_document != 0 ? lm_p0_document_diagnostic(dimension_document) : 0;
        if (diagnostic != 0) {
            fprintf(
                stderr,
                "trans L2 error: %s parse error %d at %zu:%zu: %s\n",
                error_name != 0 ? error_name : "array dimension",
                diagnostic->code,
                diagnostic->line,
                diagnostic->column,
                diagnostic->message
            );
        } else {
            fprintf(stderr, "trans L2 error: %s parse failed\n", error_name != 0 ? error_name : "array dimension");
        }
        if (dimension_document != 0) {
            lm_p0_document_destroy(dimension_document);
        }
        return 1;
    }
    dimension_root = lm_p0_document_root(dimension_document);
    if (
        dimension_root == 0 ||
        dimension_root->kind != LM_P0_NODE_STRUCTURE ||
        dimension_root->as.structure.first_field == 0
    ) {
        fprintf(stderr, "trans L2 error: %s must not be empty\n", error_name != 0 ? error_name : "array dimension");
        lm_p0_document_destroy(dimension_document);
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, dimension_root->as.structure.first_field, namespace_) != 0) {
        lm_p0_document_destroy(dimension_document);
        return 1;
    }
    lm_p0_document_destroy(dimension_document);
    return 0;
}

static int lm_trans_emit_c_declarator(
    FILE *file,
    LmTransCDeclarator *declarator,
    const LmTransNamespace *namespace_,
    const char *dimension_error_name
) {
    size_t i;
    size_t head_index;
    LmP0Text head_dimension;

    if (file == 0 || declarator == 0) {
        return 1;
    }

    if (declarator->type_is_head) {
        if (lm_trans_emit_type_head_only(file, declarator->type_head) != 0) {
            return 1;
        }
    } else {
        if (declarator->type_node == 0 || lm_trans_emit_type_node(file, declarator->type_node) != 0) {
            return 1;
        }
    }

    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < declarator->pointer_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    if (lm_trans_emit_identifier(file, declarator->name) != 0) {
        return 1;
    }

    head_index = 0U;
    while (lm_trans_array_head_next_dimension(declarator->array_head, &head_index, &head_dimension)) {
        if (lm_trans_put(file, "[") != 0) {
            return 1;
        }
        if (head_dimension.length != 0U) {
            if (lm_trans_emit_c_dimension_text(file, head_dimension, namespace_, dimension_error_name) != 0) {
                return 1;
            }
        } else {
            if (declarator->expression_dimensions == 0 || declarator->expression_dimensions->value == 0) {
                fprintf(stderr, "trans L2 error: %s expects a size expression\n", dimension_error_name != 0 ? dimension_error_name : "array dimension");
                return 1;
            }
            if (lm_trans_emit_expr_node(file, declarator->expression_dimensions->value, namespace_) != 0) {
                return 1;
            }
            declarator->expression_dimensions = declarator->expression_dimensions->next;
        }
        if (lm_trans_put(file, "]") != 0) {
            return 1;
        }
    }

    for (i = 0U; i < declarator->literal_dimension_count; ++i) {
        if (
            lm_trans_put(file, "[") != 0 ||
            lm_trans_emit_size_literal(file, declarator->literal_dimensions[i]) != 0 ||
            lm_trans_put(file, "]") != 0
        ) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_head_can_declare_storage(
    LmP0Text head,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_array_param(FILE *file, const LmP0Frame *frame, LmTransNamespace *namespace_) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *dimension_field;
    const LmP0Node *name_node;
    const LmP0Node *type_node;
    LmP0Text head_dimension;
    size_t head_index;
    size_t pointer_depth;

    if (frame == 0) {
        return 1;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;

    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as.atom, '@')
    ) {
        pointer_depth += field->value->as.atom.length;
        field = field->next;
    }

    type_field = field;
    name_field = type_field != 0 ? type_field->next : 0;
    if (
        type_field == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: [] parameter expects [@...] type name [dimensions...]\n");
        return 1;
    }

    type_node = type_field->value;
    name_node = name_field->value;
    if (
        lm_trans_emit_type_and_name(
            file,
            type_node,
            name_node->as.atom,
            pointer_depth,
            namespace_
        ) != 0
    ) {
        return 1;
    }

    dimension_field = name_field->next;
    head_index = 0U;
    while (lm_trans_array_head_next_dimension(frame->head, &head_index, &head_dimension)) {
        if (lm_trans_put(file, "[") != 0) {
            return 1;
        }
        if (head_dimension.length != 0U) {
            LmP0Document *dimension_document;
            const LmP0Diagnostic *diagnostic;
            const LmP0Node *dimension_root;
            int parse_status;

            dimension_document = 0;
            parse_status = lm_p0_parse_bytes(head_dimension.data, head_dimension.length, &dimension_document);
            if (parse_status != 0) {
                diagnostic = dimension_document != 0 ? lm_p0_document_diagnostic(dimension_document) : 0;
                if (diagnostic != 0) {
                    fprintf(
                        stderr,
                        "trans L2 error: array parameter dimension parse error %d at %zu:%zu: %s\n",
                        diagnostic->code,
                        diagnostic->line,
                        diagnostic->column,
                        diagnostic->message
                    );
                } else {
                    fprintf(stderr, "trans L2 error: array parameter dimension parse failed\n");
                }
                if (dimension_document != 0) {
                    lm_p0_document_destroy(dimension_document);
                }
                return 1;
            }
            dimension_root = lm_p0_document_root(dimension_document);
            if (
                dimension_root == 0 ||
                dimension_root->kind != LM_P0_NODE_STRUCTURE ||
                dimension_root->as.structure.first_field == 0
            ) {
                fprintf(stderr, "trans L2 error: array parameter dimension must not be empty\n");
                lm_p0_document_destroy(dimension_document);
                return 1;
            }
            if (lm_trans_emit_expr_fields(file, dimension_root->as.structure.first_field, namespace_) != 0) {
                lm_p0_document_destroy(dimension_document);
                return 1;
            }
            lm_p0_document_destroy(dimension_document);
        } else if (lm_trans_array_body_dimension_can_consume(dimension_field)) {
            if (lm_trans_emit_expr_node(file, dimension_field->value, namespace_) != 0) {
                return 1;
            }
            dimension_field = dimension_field->next;
        }
        if (lm_trans_put(file, "]") != 0) {
            return 1;
        }
    }

    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_param(FILE *file, const LmP0Node *node, LmTransNamespace *namespace_) {
    const LmP0Field *field0;
    const LmP0Field *field1;
    const LmP0Field *inner_field;
    const LmP0Node *param_node;
    const LmP0Node *name_node;

    if (node == 0 || node->kind != LM_P0_NODE_FRAME) {
        if (lm_trans_node_is_positional_skip(node)) {
            fprintf(stderr, "trans L2 error: skipped function parameter has no default declaration in this profile\n");
            return 1;
        }
        fprintf(stderr, "trans L2 error: parameter must be a typed frame\n");
        return 1;
    }

    if (lm_trans_text_equals(node->as.frame.head, "const")) {
        field0 = node->as.frame.body.first_field;
        param_node = 0;
        if (
            field0 != 0 &&
            field0->next == 0 &&
            field0->value != 0
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as.structure.first_field;
                if (
                    inner_field != 0 &&
                    inner_field->next == 0 &&
                    inner_field->value != 0 &&
                    inner_field->value->kind == LM_P0_NODE_FRAME
                ) {
                    param_node = inner_field->value;
                }
            }
        }
        if (param_node == 0) {
            fprintf(stderr, "trans L2 error: const parameter expects exactly one parameter declaration\n");
            return 1;
        }
        if (lm_trans_put(file, "const ") != 0) {
            return 1;
        }
        return lm_trans_emit_param(file, param_node, namespace_);
    }

    field0 = lm_trans_nth_field(&node->as.frame.body, 0U);
    field1 = lm_trans_nth_field(&node->as.frame.body, 1U);

    if (lm_trans_head_can_declare_storage(node->as.frame.head, namespace_)) {
        if (field0 == 0 || field0->value == 0 || field0->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: typed parameter expects a name\n");
            return 1;
        }
        name_node = field0->value;
        if (lm_trans_emit_name(file, node->as.frame.head) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, name_node->as.atom) != 0) {
            return 1;
        }
        return lm_trans_namespace_declare_storage_binding(
            namespace_,
            name_node->as.atom,
            node->as.frame.head
        );
    }

    if (lm_trans_text_all_char(node->as.frame.head, '@')) {
        if (
            field0 == 0 ||
            field1 == 0 ||
            field1->value == 0 ||
            field1->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: @ parameter expects type and name\n");
            return 1;
        }
        name_node = field1->value;
        if (
            lm_trans_emit_type_and_name(
                file,
                field0->value,
                name_node->as.atom,
                node->as.frame.head.length,
                namespace_
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
    }

    if (lm_trans_text_is_array_receiver_head(node->as.frame.head)) {
        return lm_trans_emit_array_param(file, &node->as.frame, namespace_);
    }

    fprintf(stderr, "trans L2 error: unsupported parameter head\n");
    return 1;
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNamespace *namespace_
);

static int lm_trans_params_has_any(const LmP0Node *params) {
    return
        params != 0 &&
        params->kind == LM_P0_NODE_STRUCTURE &&
        params->as.structure.first_field != 0;
}

static int lm_trans_emit_capture_field(
    FILE *file,
    const LmTransCapture *capture,
    unsigned indent,
    size_t extra_pointer_depth,
    const LmTransNamespace *namespace_
) {
    size_t i;

    if (capture == 0) {
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (capture->type_is_head) {
        if (lm_trans_builtin_c_type_name(capture->type_head)) {
            if (lm_trans_emit_type_name(file, capture->type_head) != 0) {
                return 1;
            }
        } else if (lm_trans_emit_name(file, capture->type_head) != 0) {
            return 1;
        }
    } else if (lm_trans_emit_type_node(file, capture->type_node) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < capture->pointer_depth + extra_pointer_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    if (lm_trans_emit_identifier(file, capture->name) != 0) {
        return 1;
    }
    (void)namespace_;
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_env_type(
    FILE *file,
    const LmTransFunctionHeader *function,
    const LmTransNamespace *namespace_
) {
    size_t i;
    const LmTransCapture *capture;

    if (function == 0 || !function->has_env || function->captures == 0) {
        return 0;
    }

    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->env_type_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->env_type_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\n\nstruct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->env_type_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " {\n") != 0) {
        return 1;
    }

    for (i = 0U; i < function->captures->count; ++i) {
        capture = (const LmTransCapture *)lm_own_ptr_stack_at(function->captures, i);
        if (lm_trans_emit_capture_field(file, capture, 1U, 0U, namespace_) != 0) {
            return 1;
        }
    }

    return lm_trans_put(file, "};\n\n");
}

static const LmP0Field *lm_trans_control_body_start(const LmP0Frame *frame) {
    const LmP0Field *end;

    if (frame == 0 || frame->body.first_field == 0) {
        return 0;
    }

    end = lm_trans_expr_segment_end(frame->body.first_field);
    if (end != 0 && end->value != 0 && end->value->kind == LM_P0_NODE_STRUCTURE) {
        return end;
    }

    return 0;
}

static int lm_trans_emit_control_condition(
    FILE *file,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *body_start;
    const LmP0Field *first;

    first = frame->body.first_field;
    if (first == 0) {
        return 0;
    }

    body_start = lm_trans_control_body_start(frame);
    return lm_trans_emit_expr_range(file, first, body_start, namespace_);
}

static int lm_trans_parse_size_payload(const char *payload, size_t *out_value) {
    char *end;
    unsigned long value;

    if (payload == 0 || payload[0] == '\0' || out_value == 0) {
        return 0;
    }

    end = 0;
    value = strtoul(payload, &end, 10);
    if (end == payload || end == 0 || *end != '\0') {
        return 0;
    }

    *out_value = (size_t)value;
    return 1;
}

static int lm_trans_frame_positional_name_index(const LmP0Frame *frame, size_t *out_index) {
    const char *index_payload;

    if (frame == 0 || out_index == 0) {
        return 0;
    }

    if (lm_trans_text_all_char(frame->head, '@') || lm_trans_text_is_array_receiver_head(frame->head)) {
        *out_index = 1U;
        return 1;
    }

    index_payload = lm_trans_registry_lookup(frame->head, "receiver.positional-name.index");
    if (index_payload != 0) {
        if (!lm_trans_parse_size_payload(index_payload, out_index)) {
            fprintf(
                stderr,
                "trans registry error: receiver.positional-name.index for \"%.*s\" must be a non-negative integer\n",
                (int)frame->head.length,
                frame->head.data
            );
            exit(2);
        }
        return 1;
    }

    if (lm_trans_registry_has(frame->head, "receiver.positional-name.argument")) {
        *out_index = 0U;
        return 1;
    }

    return 0;
}

static int lm_trans_name_argument_from_frame(const LmP0Frame *frame, LmP0Text *out_name) {
    const LmP0Field *field;
    const LmP0Field *name_field;
    const LmP0Frame *child_frame;
    size_t name_index;

    if (frame == 0 || out_name == 0) {
        return 0;
    }

    field = frame->body.first_field;
    while (field != 0) {
        if (field->value != 0 && field->value->kind == LM_P0_NODE_FRAME) {
            child_frame = &field->value->as.frame;
            if (lm_trans_text_equals(child_frame->head, "name")) {
                name_field = child_frame->body.first_field;
                if (
                    name_field != 0 &&
                    name_field->next == 0 &&
                    name_field->value != 0 &&
                    name_field->value->kind == LM_P0_NODE_ATOM &&
                    lm_trans_name_argument_is_valid(name_field->value->as.atom)
                ) {
                    *out_name = name_field->value->as.atom;
                    return 1;
                }
            }
        }
        field = field->next;
    }

    if (!lm_trans_frame_positional_name_index(frame, &name_index)) {
        return 0;
    }

    field = lm_trans_nth_field(&frame->body, name_index);
    if (field == 0 || field->value == 0) {
        return 0;
    }

    if (
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_name_argument_is_valid(field->value->as.atom)
    ) {
        *out_name = field->value->as.atom;
        return 1;
    }

    return 0;
}

static int lm_trans_frame_close_target(const LmP0Frame *frame, LmP0Text *out_target) {
    if (frame == 0 || out_target == 0) {
        return 0;
    }

    if (lm_trans_name_argument_from_frame(frame, out_target)) {
        return 1;
    }

    *out_target = frame->head;
    return 1;
}

static int lm_trans_validate_end_trailer(const LmP0Frame *frame) {
    LmP0Text actual;
    LmP0Text expected;

    if (frame == 0 || frame->trailer == 0) {
        return 0;
    }

    if (!lm_trans_text_equals(frame->trailer->spelling, "end")) {
        return 0;
    }

    if (!lm_trans_trailer_single_atom(frame->trailer, &actual)) {
        fprintf(stderr, "trans error: end trailer expects exactly one target name\n");
        return 1;
    }

    if (!lm_trans_frame_close_target(frame, &expected)) {
        fprintf(
            stderr,
            "trans error: head \"%.*s\" does not expose a named close target\n",
            (int)frame->head.length,
            frame->head.data
        );
        return 1;
    }

    if (!lm_trans_identifier_same(actual, expected)) {
        fprintf(
            stderr,
            "trans error: end target \"%.*s\" does not match close target \"%.*s\"\n",
            (int)actual.length,
            actual.data,
            (int)expected.length,
            expected.data
        );
        return 1;
    }

    return 0;
}

static int lm_trans_emit_cleanups_until(
    FILE *file,
    unsigned indent,
    const LmTransNamespace *namespace_,
    size_t cleanup_base
) {
    size_t index;
    const LmTransCleanup *cleanup;

    if (namespace_ == 0 || cleanup_base > namespace_->cleanups.count) {
        return 0;
    }

    index = namespace_->cleanups.count;
    while (index > cleanup_base) {
        --index;
        cleanup = (const LmTransCleanup *)lm_own_ptr_stack_at(&namespace_->cleanups, index);
        if (cleanup == 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "lm_synchronized_leave(") != 0) {
            return 1;
        }
        if (lm_trans_emit_sync_name(file, cleanup->id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ");\n") != 0) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_return_fields_single_atom(
    const LmP0Field *return_fields,
    LmP0Text *out_atom
) {
    if (
        out_atom == 0 ||
        return_fields == 0 ||
        return_fields->next != 0 ||
        return_fields->value == 0 ||
        return_fields->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_atom = return_fields->value->as.atom;
    return 1;
}

static const LmP0Structure *lm_trans_fields_single_structure_value(
    const LmP0Field *fields
) {
    if (
        fields == 0 ||
        fields->next != 0 ||
        fields->value == 0 ||
        fields->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }

    return &fields->value->as.structure;
}

static size_t lm_trans_structure_field_count(const LmP0Structure *structure) {
    const LmP0Field *field;
    size_t count;

    count = 0U;
    field = structure != 0 ? structure->first_field : 0;
    while (field != 0) {
        ++count;
        field = field->next;
    }

    return count;
}

static int lm_trans_emit_size_literal(FILE *file, size_t value) {
    return fprintf(file, "%zuU", value) < 0 ? 1 : 0;
}

static int lm_trans_emit_array_pointer_type(
    FILE *file,
    const LmP0Node *element_type,
    size_t pointer_depth
) {
    size_t i;

    if (lm_trans_emit_type_node(file, element_type) != 0) {
        return 1;
    }
    if (pointer_depth == 0U) {
        return 0;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < pointer_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_array_target_path(
    FILE *file,
    LmP0Text target_name,
    const size_t *indices,
    size_t depth
) {
    size_t i;

    if (lm_trans_emit_identifier(file, target_name) != 0) {
        return 1;
    }
    for (i = 0U; i < depth; ++i) {
        if (lm_trans_put(file, "[") != 0) {
            return 1;
        }
        if (lm_trans_emit_size_literal(file, indices != 0 ? indices[i] : 0U) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "]") != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_array_structure_value_alloc_assignment(
    FILE *file,
    unsigned indent,
    LmP0Text target_name,
    const size_t *indices,
    size_t depth,
    const LmP0Node *element_type,
    size_t pointer_depth,
    size_t rank,
    size_t count
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_array_target_path(file, target_name, indices, depth) != 0) {
        return 1;
    }
    if (count == 0U) {
        return lm_trans_put(file, " = 0;\n");
    }

    if (lm_trans_put(file, " = (") != 0) {
        return 1;
    }
    if (lm_trans_emit_array_pointer_type(file, element_type, pointer_depth) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ")lm_own_array_new_zero(sizeof(*") != 0) {
        return 1;
    }
    if (lm_trans_emit_array_target_path(file, target_name, indices, depth) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "), ") != 0) {
        return 1;
    }
    if (lm_trans_emit_size_literal(file, count) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ", ") != 0) {
        return 1;
    }
    if (lm_trans_emit_size_literal(file, rank) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ", ") != 0) {
        return 1;
    }
    if (lm_trans_emit_size_literal(file, depth) != 0) {
        return 1;
    }
    return lm_trans_put(file, ");\n");
}

static int lm_trans_emit_array_null_return_check(
    FILE *file,
    unsigned indent,
    LmP0Text target_name,
    const size_t *indices,
    size_t depth,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "if (") != 0) {
        return 1;
    }
    if (lm_trans_emit_array_target_path(file, target_name, indices, depth) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " == 0) {\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "return 0;\n") != 0) {
        return 1;
    }
    return lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");
}

static int lm_trans_emit_array_structure_value_fill_recursive(
    FILE *file,
    unsigned indent,
    LmP0Text target_name,
    const LmP0Node *element_type,
    size_t rank,
    size_t depth,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    size_t *indices
) {
    const LmP0Field *field;
    const LmP0Structure *child;
    size_t count;
    size_t index;

    if (rank == 0U || depth >= rank) {
        return 1;
    }

    index = 0U;
    field = value != 0 ? value->first_field : 0;
    while (field != 0) {
        indices[depth] = index;
        if (depth + 1U == rank) {
            if (lm_trans_emit_indent(file, indent) != 0) {
                return 1;
            }
            if (lm_trans_emit_array_target_path(file, target_name, indices, depth + 1U) != 0) {
                return 1;
            }
            if (lm_trans_put(file, " = ") != 0) {
                return 1;
            }
            if (lm_trans_emit_expr_node(file, field->value, namespace_) != 0) {
                return 1;
            }
            if (lm_trans_put(file, ";\n") != 0) {
                return 1;
            }
        } else {
            if (field->value == 0 || field->value->kind != LM_P0_NODE_STRUCTURE) {
                fprintf(stderr, "trans L2 error: [] multi-dimensional Structure value expects nested [] Structure fields\n");
                return 1;
            }
            child = &field->value->as.structure;
            count = lm_trans_structure_field_count(child);
            if (
                lm_trans_emit_array_structure_value_alloc_assignment(
                    file,
                    indent,
                    target_name,
                    indices,
                    depth + 1U,
                    element_type,
                    rank - depth - 1U,
                    rank,
                    count
                ) != 0
            ) {
                return 1;
            }
            if (
                count != 0U &&
                lm_trans_emit_array_null_return_check(
                    file,
                    indent,
                    target_name,
                    indices,
                    depth + 1U,
                    namespace_
                ) != 0
            ) {
                return 1;
            }
            if (
                lm_trans_emit_array_structure_value_fill_recursive(
                    file,
                    indent,
                    target_name,
                    element_type,
                    rank,
                    depth + 1U,
                    child,
                    namespace_,
                    indices
                ) != 0
            ) {
                return 1;
            }
        }
        ++index;
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_array_structure_value_fill(
    FILE *file,
    unsigned indent,
    LmP0Text target_name,
    const LmP0Node *element_type,
    size_t rank,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_
) {
    size_t *indices;
    int status;

    if (rank == 0U) {
        return 1;
    }
    indices = (size_t *)calloc(rank, sizeof(*indices));
    if (indices == 0) {
        return 1;
    }
    status = lm_trans_emit_array_structure_value_fill_recursive(
        file,
        indent,
        target_name,
        element_type,
        rank,
        0U,
        value,
        namespace_,
        indices
    );
    free(indices);
    return status;
}

static int lm_trans_type_receiver_array_structure_value_alloc(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    LmP0Text target_name,
    const LmP0Structure *value,
    int *out_consumed,
    int *out_needs_null_check
) {
    const LmP0Node *element_type;
    size_t rank;
    size_t count;
    int type_status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_needs_null_check != 0) {
        *out_needs_null_check = 0;
    }
    if (out_consumed == 0 || out_needs_null_check == 0 || value == 0) {
        return 0;
    }

    type_status = lm_trans_array_type_node_info(type_node, &element_type, &rank);
    if (type_status < 0) {
        return 1;
    }
    if (type_status == 0) {
        return 0;
    }

    *out_consumed = 1;
    count = lm_trans_structure_field_count(value);
    *out_needs_null_check = count != 0U;
    return lm_trans_emit_array_structure_value_alloc_assignment(
        file,
        indent,
        target_name,
        0,
        0U,
        element_type,
        rank,
        rank,
        count
    );
}

static int lm_trans_type_receiver_structure_value_can_consume(
    const LmP0Node *type_node,
    const LmP0Structure *value,
    int *out_can_consume
) {
    LmTransBinding type_receiver;
    int status;

    if (out_can_consume != 0) {
        *out_can_consume = 0;
    }
    if (out_can_consume == 0 || value == 0) {
        return 0;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, &type_receiver);
    if (status < 0) {
        return 1;
    }
    if (status == 0) {
        return 0;
    }
    if (type_receiver.type_structure_value_alloc == 0 || type_receiver.type_structure_value_fill == 0) {
        return 0;
    }

    *out_can_consume = 1;
    return 0;
}

static int lm_trans_type_receiver_array_structure_value_fill(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    LmP0Text target_name,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmP0Node *element_type;
    size_t rank;
    int type_status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0 || value == 0) {
        return 0;
    }

    type_status = lm_trans_array_type_node_info(type_node, &element_type, &rank);
    if (type_status < 0) {
        return 1;
    }
    if (type_status == 0) {
        return 0;
    }

    *out_consumed = 1;
    return lm_trans_emit_array_structure_value_fill(file, indent, target_name, element_type, rank, value, namespace_);
}

static int lm_trans_emit_type_receiver_structure_value_alloc(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    LmP0Text target_name,
    const LmP0Structure *value,
    int *out_consumed,
    int *out_needs_null_check
) {
    LmTransBinding type_receiver;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_needs_null_check != 0) {
        *out_needs_null_check = 0;
    }
    if (out_consumed == 0 || out_needs_null_check == 0 || value == 0) {
        return 0;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, &type_receiver);
    if (status < 0) {
        return 1;
    }
    if (status == 0 || type_receiver.type_structure_value_alloc == 0) {
        return 0;
    }

    return type_receiver.type_structure_value_alloc(
        file,
        indent,
        type_node,
        target_name,
        value,
        out_consumed,
        out_needs_null_check
    );
}

static int lm_trans_emit_type_receiver_structure_value_fill(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    LmP0Text target_name,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    LmTransBinding type_receiver;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0 || value == 0) {
        return 0;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, &type_receiver);
    if (status < 0) {
        return 1;
    }
    if (status == 0 || type_receiver.type_structure_value_fill == 0) {
        return 0;
    }

    return type_receiver.type_structure_value_fill(
        file,
        indent,
        type_node,
        target_name,
        value,
        namespace_,
        out_consumed
    );
}

static const LmP0Structure *lm_trans_expr_segment_single_structure_value(
    const LmTransExprSegment *segment
) {
    if (
        segment == 0 ||
        segment->first == 0 ||
        segment->first->next != segment->stop ||
        segment->first->value == 0 ||
        segment->first->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }

    return &segment->first->value->as.structure;
}

static char *lm_trans_array_value_helper_name_new(void) {
    char buffer[64];
    int written;
    char *name;

    written = snprintf(
        buffer,
        sizeof(buffer),
        "lm_trans_array_value_%u",
        lm_trans_next_array_value_helper_id++
    );
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        return 0;
    }

    name = (char *)malloc((size_t)written + 1U);
    if (name == 0) {
        return 0;
    }
    memcpy(name, buffer, (size_t)written + 1U);
    return name;
}

static int lm_trans_emit_array_value_helper(
    FILE *file,
    const LmP0Node *param_node,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    LmP0Text *out_name
) {
    FILE *prelude_file;
    const LmP0Node *element_type;
    LmP0Text helper_name_text;
    LmP0Text target_name;
    char *helper_name;
    size_t rank;
    size_t pointer_depth;
    size_t count;

    if (out_name != 0) {
        out_name->data = "";
        out_name->length = 0U;
    }
    if (
        out_name == 0 ||
        value == 0 ||
        !lm_trans_array_param_type_info(param_node, &element_type, &rank, &pointer_depth)
    ) {
        return 1;
    }

    helper_name = lm_trans_array_value_helper_name_new();
    if (helper_name == 0) {
        return 1;
    }

    helper_name_text = lm_trans_text_from_cstr(helper_name);
    target_name = lm_trans_text_from_cstr("lm_value");
    prelude_file = lm_trans_prelude_file(file);
    count = lm_trans_structure_field_count(value);

    if (
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_array_pointer_type(prelude_file, element_type, pointer_depth + rank) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, helper_name_text) != 0 ||
        lm_trans_put(prelude_file, "(void) {\n    ") != 0 ||
        lm_trans_emit_array_pointer_type(prelude_file, element_type, pointer_depth + rank) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_write_text(prelude_file, target_name) != 0 ||
        lm_trans_put(prelude_file, ";\n") != 0 ||
        lm_trans_emit_array_structure_value_alloc_assignment(
            prelude_file,
            1U,
            target_name,
            0,
            0U,
            element_type,
            pointer_depth + rank,
            rank,
            count
        ) != 0
    ) {
        free(helper_name);
        return 1;
    }

    if (
        count != 0U &&
        (
            lm_trans_put(prelude_file, "    if (") != 0 ||
            lm_trans_write_text(prelude_file, target_name) != 0 ||
            lm_trans_put(prelude_file, " == 0) {\n        return 0;\n    }\n") != 0
        )
    ) {
        free(helper_name);
        return 1;
    }

    if (
        count != 0U &&
        lm_trans_emit_array_structure_value_fill(
            prelude_file,
            1U,
            target_name,
            element_type,
            rank,
            value,
            namespace_
        ) != 0
    ) {
        free(helper_name);
        return 1;
    }

    if (
        lm_trans_put(prelude_file, "    return ") != 0 ||
        lm_trans_write_text(prelude_file, target_name) != 0 ||
        lm_trans_put(prelude_file, ";\n}\n\n") != 0
    ) {
        free(helper_name);
        return 1;
    }

    if (lm_own_ptr_stack_push(&lm_trans_emitted_array_value_helpers, helper_name) != 0) {
        free(helper_name);
        return 1;
    }

    *out_name = helper_name_text;
    return 0;
}

static int lm_trans_materialize_array_value(
    FILE *file,
    LmTransExprStack *stack,
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmP0Structure *value;
    LmP0Text helper_name;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    value = lm_trans_expr_segment_single_structure_value(segment);
    if (value == 0) {
        return 0;
    }

    if (
        lm_trans_emit_array_value_helper(
            file,
            segment != 0 ? segment->expected_param : 0,
            value,
            namespace_,
            &helper_name
        ) != 0 ||
        lm_trans_expr_stack_push_text(stack, ")") != 0 ||
        lm_trans_expr_stack_push_text(stack, "(") != 0 ||
        lm_trans_expr_stack_push_name_text(stack, helper_name) != 0
    ) {
        return 1;
    }

    *out_consumed = 1;
    return 0;
}

static int lm_trans_return_name_text(unsigned id, char *buffer, size_t buffer_size, LmP0Text *out_name) {
    int written;

    if (buffer == 0 || buffer_size == 0U || out_name == 0) {
        return 1;
    }
    written = snprintf(buffer, buffer_size, "lm_return_%u", id);
    if (written < 0 || (size_t)written >= buffer_size) {
        return 1;
    }
    out_name->data = buffer;
    out_name->length = (size_t)written;
    return 0;
}

static int lm_trans_emit_typed_value_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmP0Structure *structure_value;
    LmP0Text return_name;
    char return_name_storage[64];
    unsigned return_id;
    int can_consume;
    int needs_null_check;
    int filled;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (
        out_consumed == 0 ||
        namespace_ == 0 ||
        namespace_->return_type_node == 0 ||
        namespace_->return_type_is_struct
    ) {
        return 0;
    }

    structure_value = lm_trans_fields_single_structure_value(return_fields);
    if (structure_value == 0) {
        return 0;
    }
    if (lm_trans_type_receiver_structure_value_can_consume(namespace_->return_type_node, structure_value, &can_consume) != 0) {
        return 1;
    }
    if (!can_consume) {
        return 0;
    }

    return_id = namespace_->next_return_id++;
    if (lm_trans_return_name_text(return_id, return_name_storage, sizeof(return_name_storage), &return_name) != 0) {
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "{\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0) {
        return 1;
    }
    if (lm_trans_emit_current_return_type(file, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0 || lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    if (
        lm_trans_emit_type_receiver_structure_value_alloc(
            file,
            indent + 1U,
            namespace_->return_type_node,
            return_name,
            structure_value,
            out_consumed,
            &needs_null_check
        ) != 0
    ) {
        return 1;
    }
    if (!*out_consumed) {
        return lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");
    }
    if (needs_null_check) {
        if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "if (") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, " == 0) {\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_cleanups_until(file, indent + 2U, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 2U) != 0 || lm_trans_put(file, "return 0;\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "}\n") != 0) {
            return 1;
        }
    }

    if (
        lm_trans_emit_type_receiver_structure_value_fill(
            file,
            indent + 1U,
            namespace_->return_type_node,
            return_name,
            structure_value,
            namespace_,
            &filled
        ) != 0
    ) {
        return 1;
    }
    if (!filled) {
        fprintf(stderr, "trans L2 error: return type receiver accepted Structure value allocation but not fill\n");
        return 1;
    }

    if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "return ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");
}

static int lm_trans_struct_return_field_name_at(
    const LmP0Structure *descriptor,
    size_t index,
    LmP0Text *out_name
) {
    const LmP0Field *field;

    if (descriptor == 0 || out_name == 0) {
        return 0;
    }

    field = lm_trans_nth_field(descriptor, index);
    if (field == 0 || field->value == 0) {
        return 0;
    }

    return lm_trans_formal_param_name(field->value, out_name);
}

static int lm_trans_struct_return_field_index(
    const LmP0Structure *descriptor,
    LmP0Text name,
    size_t *out_index
) {
    const LmP0Field *field;
    LmP0Text field_name;
    size_t index;

    if (descriptor == 0 || out_index == 0) {
        return 0;
    }

    index = 0U;
    field = descriptor->first_field;
    while (field != 0) {
        if (
            field->value != 0 &&
            lm_trans_formal_param_name(field->value, &field_name) &&
            lm_trans_identifier_same(field_name, name)
        ) {
            *out_index = index;
            return 1;
        }
        ++index;
        field = field->next;
    }

    return 0;
}

static int lm_trans_struct_return_collect_value_fields(
    const LmP0Structure *descriptor,
    const LmP0Structure *value,
    LmTransStructReturnFieldValue *fields,
    size_t field_count
) {
    const LmP0Field *field;
    const LmP0Frame *frame;
    size_t positional_index;
    size_t named_index;
    int positional_disabled;

    if (descriptor == 0 || value == 0 || fields == 0) {
        return 1;
    }

    positional_index = 0U;
    positional_disabled = 0;
    field = value->first_field;
    while (field != 0) {
        if (field->value == 0) {
            field = field->next;
            continue;
        }
        if (lm_trans_node_is_positional_skip(field->value)) {
            if (positional_disabled) {
                fprintf(stderr, "trans L2 error: positional skip after out-of-position named fm return field\n");
                return 1;
            }
            ++positional_index;
            field = field->next;
            continue;
        }

        if (
            field->value->kind == LM_P0_NODE_FRAME &&
            (field->value->as.frame.flags & LM_P0_FRAME_COLON) != 0U &&
            lm_trans_struct_return_field_index(descriptor, field->value->as.frame.head, &named_index)
        ) {
            frame = &field->value->as.frame;
            if (fields[named_index].present) {
                fprintf(stderr, "trans L2 error: duplicate named fm return field\n");
                return 1;
            }
            if (frame->body.first_field == 0) {
                fprintf(stderr, "trans L2 error: named fm return field expects a value\n");
                return 1;
            }
            fields[named_index].first = frame->body.first_field;
            fields[named_index].stop = 0;
            fields[named_index].present = 1;
            if (named_index == positional_index) {
                ++positional_index;
            } else {
                positional_disabled = 1;
            }
            field = field->next;
            continue;
        }

        if (positional_disabled) {
            fprintf(stderr, "trans L2 error: positional fm return field after out-of-position named field\n");
            return 1;
        }
        if (positional_index >= field_count) {
            fprintf(stderr, "trans L2 error: too many positional fm return fields\n");
            return 1;
        }
        if (fields[positional_index].present) {
            fprintf(stderr, "trans L2 error: duplicate positional fm return field\n");
            return 1;
        }
        fields[positional_index].first = field;
        fields[positional_index].stop = field->next;
        fields[positional_index].present = 1;
        ++positional_index;
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_struct_value_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmP0Structure *descriptor;
    const LmP0Structure *value;
    LmTransStructReturnFieldValue *values;
    LmP0Text return_name;
    LmP0Text field_name;
    char return_name_storage[64];
    size_t field_count;
    size_t index;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (
        out_consumed == 0 ||
        namespace_ == 0 ||
        !namespace_->return_type_is_struct ||
        namespace_->return_type_node == 0 ||
        namespace_->return_type_node->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }

    value = lm_trans_fields_single_structure_value(return_fields);
    if (value == 0) {
        return 0;
    }

    descriptor = &namespace_->return_type_node->as.structure;
    field_count = lm_trans_structure_field_count(descriptor);
    if (field_count == 0U) {
        fprintf(stderr, "trans L2 error: fm return Structure must not be empty\n");
        return 1;
    }

    values = (LmTransStructReturnFieldValue *)lm_own_new_zero(sizeof(*values) * field_count);
    if (values == 0) {
        return 1;
    }

    status = lm_trans_struct_return_collect_value_fields(descriptor, value, values, field_count);
    if (status == 0) {
        for (index = 0U; index < field_count; ++index) {
            if (!values[index].present) {
                fprintf(stderr, "trans L2 error: fm Structure return literal does not fill every return field\n");
                status = 1;
                break;
            }
        }
    }
    if (status != 0) {
        lm_own_delete(values, 0);
        return 1;
    }

    if (
        lm_trans_return_name_text(namespace_->next_return_id++, return_name_storage, sizeof(return_name_storage), &return_name) != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "{\n") != 0 ||
        lm_trans_emit_indent(file, indent + 1U) != 0 ||
        lm_trans_emit_current_return_type(file, namespace_) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_write_text(file, return_name) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        lm_own_delete(values, 0);
        return 1;
    }

    for (index = 0U; index < field_count; ++index) {
        if (!lm_trans_struct_return_field_name_at(descriptor, index, &field_name)) {
            fprintf(stderr, "trans L2 error: fm return field must expose a binding name\n");
            lm_own_delete(values, 0);
            return 1;
        }
        if (
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_write_text(file, return_name) != 0 ||
            lm_trans_put(file, ".") != 0 ||
            lm_trans_emit_identifier(file, field_name) != 0 ||
            lm_trans_put(file, " = ") != 0 ||
            lm_trans_emit_expr_range(file, values[index].first, values[index].stop, namespace_) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            lm_own_delete(values, 0);
            return 1;
        }
    }

    lm_own_delete(values, 0);

    if (
        lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0 ||
        lm_trans_emit_indent(file, indent + 1U) != 0 ||
        lm_trans_put(file, "return ") != 0 ||
        lm_trans_write_text(file, return_name) != 0 ||
        lm_trans_put(file, ";\n") != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "}\n") != 0
    ) {
        return 1;
    }

    *out_consumed = 1;
    return 0;
}

static int lm_trans_current_return_is_callable_descriptor(
    const LmTransNamespace *namespace_,
    LmP0Text *out_type
) {
    const LmTransSymbol *symbol;
    LmP0Text type_name;

    if (
        namespace_ == 0 ||
        namespace_->return_type_node == 0 ||
        namespace_->return_type_node->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    type_name = namespace_->return_type_node->as.atom;
    symbol = lm_trans_namespace_find(namespace_, type_name);
    if (!lm_trans_symbol_is(symbol, "callableDescriptor")) {
        return 0;
    }
    if (out_type != 0) {
        *out_type = type_name;
    }
    return 1;
}

static int lm_trans_emit_closure_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_,
    int *out_consumed
) {
    LmP0Text atom;
    LmP0Text descriptor_type;
    const LmTransSymbol *symbol;
    unsigned return_id;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (
        out_consumed == 0 ||
        !lm_trans_return_fields_single_atom(return_fields, &atom) ||
        !lm_trans_current_return_is_callable_descriptor(namespace_, &descriptor_type)
    ) {
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (
        symbol == 0 ||
        !lm_trans_symbol_is(symbol, "function") ||
        !symbol->has_closure_call_name
    ) {
        return 0;
    }

    *out_consumed = 1;
    return_id = namespace_->next_return_id++;

    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "{\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_write_text(file, descriptor_type) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = (") != 0 || lm_trans_write_text(file, descriptor_type) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ")lm_own_new_zero(sizeof(*") != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, "));\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "if (") != 0) {
        return 1;
    }
    if (lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, " == 0) {\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_cleanups_until(file, indent + 2U, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 2U) != 0 || lm_trans_put(file, "return 0;\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "}\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "->call = ") != 0 || lm_trans_write_text(file, symbol->closure_call_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, "->env = ") != 0) {
        return 1;
    }
    if (symbol->has_env_arg) {
        if (lm_trans_write_text(file, symbol->env_arg) != 0) {
            return 1;
        }
    } else if (lm_trans_put(file, "0") != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "->destroy = 0;\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "return ") != 0) {
        return 1;
    }
    if (lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");
}

static int lm_trans_emit_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    unsigned return_id;
    int consumed;

    if (return_fields == 0) {
        if (lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "return;\n");
    }

    if (lm_trans_emit_closure_return_statement(file, return_fields, indent, namespace_, &consumed) != 0) {
        return 1;
    }
    if (consumed) {
        return 0;
    }

    if (lm_trans_emit_struct_value_return_statement(file, return_fields, indent, namespace_, &consumed) != 0) {
        return 1;
    }
    if (consumed) {
        return 0;
    }

    if (lm_trans_emit_typed_value_return_statement(file, return_fields, indent, namespace_, &consumed) != 0) {
        return 1;
    }
    if (consumed) {
        return 0;
    }

    if (namespace_ != 0 && namespace_->cleanups.count > 0U && namespace_->return_type_node != 0) {
        return_id = namespace_->next_return_id++;
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "{\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0) {
            return 1;
        }
        if (lm_trans_emit_current_return_type(file, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_return_name(file, return_id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, return_fields, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "return ") != 0) {
            return 1;
        }
        if (lm_trans_emit_return_name(file, return_id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "}\n");
    }

    if (lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "return ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, return_fields, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_trailer_statement(
    FILE *file,
    const LmP0Trailer *trailer,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (trailer == 0) {
        return 0;
    }

    if (lm_trans_text_equals(trailer->spelling, "return")) {
        return lm_trans_emit_return_statement(file, trailer->body.first_field, indent, namespace_);
    }

    if (lm_trans_text_equals(trailer->spelling, "end")) {
        return 0;
    }

    fprintf(
        stderr,
        "trans error: unsupported trailer \"%.*s\"\n",
        (int)trailer->spelling.length,
        trailer->spelling.data
    );
    return 1;
}

static int lm_trans_namespace_declare_storage_binding(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text type_head
);

static int lm_trans_emit_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Structure *body;
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    name_field = lm_trans_nth_field(body, 0U);
    if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans L2 error: declaration expects a name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (lm_trans_builtin_c_type_name(frame->head)) {
        if (lm_trans_emit_type_name(file, frame->head) != 0) {
            return 1;
        }
    } else {
        if (lm_trans_emit_name(file, frame->head) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, name_node->as.atom) != 0) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare_storage_binding(namespace_, name_node->as.atom, frame->head);
}

static int lm_trans_namespace_declare_storage_binding(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text type_head
) {
    LmTransSymbol *symbol;
    const LmTransSymbol *type_symbol;

    type_symbol = lm_trans_namespace_find(namespace_, type_head);
    if (lm_trans_symbol_is(type_symbol, "callableDescriptor")) {
        if (lm_trans_namespace_declare(namespace_, name, "closure") != 0) {
            return 1;
        }
        symbol = lm_trans_namespace_find_mutable(namespace_, name);
        return lm_trans_symbol_copy_signature(symbol, type_symbol);
    }

    return lm_trans_namespace_declare(namespace_, name, "variable");
}

static int lm_trans_emit_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_atom_can_be_new_binding_name(LmP0Text text) {
    return lm_trans_atom_is_identifier_like(text);
}

static int lm_trans_head_can_declare_storage(
    LmP0Text head,
    const LmTransNamespace *namespace_
) {
    const LmTransSymbol *symbol;

    if (lm_trans_builtin_c_type_name(head)) {
        return 1;
    }

    if (
        lm_trans_text_all_char(head, '@') ||
        lm_trans_text_is_array_receiver_head(head)
    ) {
        return 0;
    }

    if (lm_trans_is_reserved_head_name(head)) {
        return 0;
    }

    if (lm_trans_registry_has(head, "class.present")) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, head);
    return
        lm_trans_symbol_is(symbol, "class") ||
        lm_trans_symbol_is(symbol, "callableDescriptor");
}

static int lm_trans_frame_looks_storage_declaration(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Structure *body;
    const LmP0Field *name_field;

    if (
        frame == 0 ||
        !lm_trans_head_can_declare_storage(frame->head, namespace_)
    ) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    name_field = lm_trans_nth_field(body, 0U);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    return lm_trans_atom_can_be_new_binding_name(name_field->value->as.atom);
}

static int lm_trans_emit_pointer_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Structure *body;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    type_field = lm_trans_nth_field(body, 0U);
    name_field = lm_trans_nth_field(body, 1U);
    if (
        type_field == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: @ declaration expects type and name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (
        lm_trans_emit_type_and_name(
            file,
            type_field->value,
            name_node->as.atom,
            frame->head.length,
            namespace_
        ) != 0
    ) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_pointer_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_pointer_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_array_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *dimension_field;
    const LmP0Field *initializer_field;
    const LmP0Node *name_node;
    const LmP0Node *type_node;
    const LmP0Frame *inner;
    LmTransCDeclarator declarator;
    LmP0Text type_head;
    size_t pointer_depth;
    int type_is_head;

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;
    type_node = 0;
    type_head.data = 0;
    type_head.length = 0U;
    type_is_head = 0;
    dimension_field = 0;

    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = &field->value->as.frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            name_field = inner->body.first_field;
            if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
                fprintf(stderr, "trans L2 error: array declaration expects a variable name\n");
                return 1;
            }
            type_head = inner->head;
            type_is_head = 1;
            dimension_field = name_field->next;
        } else if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(&inner->body, 0U);
            name_field = lm_trans_nth_field(&inner->body, 1U);
            if (
                type_field == 0 ||
                name_field == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                fprintf(stderr, "trans L2 error: array pointer declaration expects type and name\n");
                return 1;
            }
            pointer_depth = inner->head.length;
            type_node = type_field->value;
            dimension_field = name_field->next;
        } else {
            fprintf(stderr, "trans L2 error: array declaration expects a type/name declaration\n");
            return 1;
        }
    } else {
        while (
            field != 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_ATOM &&
            lm_trans_text_all_char(field->value->as.atom, '@')
        ) {
            pointer_depth += field->value->as.atom.length;
            field = field->next;
        }

        type_field = field;
        name_field = type_field != 0 ? type_field->next : 0;
        if (
            type_field == 0 ||
            name_field == 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: [] declaration expects [@...] type name size...\n");
            return 1;
        }
        type_node = type_field->value;
        dimension_field = name_field->next;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    memset(&declarator, 0, sizeof(declarator));
    declarator.type_node = type_node;
    declarator.type_head = type_head;
    declarator.name = name_node->as.atom;
    declarator.pointer_depth = pointer_depth;
    declarator.type_is_head = type_is_head;
    declarator.array_head = frame->head;
    declarator.expression_dimensions = dimension_field;
    if (lm_trans_emit_c_declarator(file, &declarator, namespace_, "[] array dimension") != 0) {
        return 1;
    }

    initializer_field = declarator.expression_dimensions;
    if (initializer_field != 0) {
        if (lm_trans_put(file, " = {") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_list(file, initializer_field, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "}") != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_array_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_array_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_pointer_declaration_repeat_with_qualifier(
    FILE *file,
    const LmP0Frame *template_frame,
    const LmP0Structure *body,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Structure *template_body;
    const LmP0Structure *repeat_body;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *type_node;
    const LmP0Node *name_node;

    template_body = template_frame != 0 ? lm_trans_unwrap_single_anonymous_structure(&template_frame->body) : 0;
    repeat_body = lm_trans_unwrap_single_anonymous_structure(body);
    type_field = template_body != 0 ? template_body->first_field : 0;
    name_field = repeat_body != 0 ? repeat_body->first_field : 0;
    if (
        template_frame == 0 ||
        type_field == 0 ||
        type_field->value == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: repeated @ declaration expects type template and name\n");
        return 1;
    }

    type_node = type_field->value;
    if (
        name_field->next != 0 &&
        name_field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_head_can_declare_storage(name_field->value->as.atom, namespace_)
    ) {
        type_node = name_field->value;
        name_field = name_field->next;
        if (name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: repeated @ declaration override expects a variable name\n");
            return 1;
        }
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (
        lm_trans_emit_type_and_name(
            file,
            type_node,
            name_node->as.atom,
            template_frame->head.length,
            namespace_
        ) != 0
    ) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_array_declaration_repeat_with_qualifier(
    FILE *file,
    const LmP0Frame *template_frame,
    const LmP0Structure *body,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Structure *template_body;
    const LmP0Structure *repeat_body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *dimension_field;
    const LmP0Field *initializer_field;
    const LmP0Node *name_node;
    const LmP0Node *type_node;
    const LmP0Node *template_type_node;
    const LmP0Frame *inner;
    LmTransCDeclarator declarator;
    LmP0Text type_head;
    LmP0Text template_type_head;
    size_t pointer_depth;
    int type_is_head;
    int template_type_is_head;

    template_body = template_frame != 0 ? lm_trans_unwrap_single_anonymous_structure(&template_frame->body) : 0;
    repeat_body = lm_trans_unwrap_single_anonymous_structure(body);
    field = template_body != 0 ? template_body->first_field : 0;
    pointer_depth = 0U;
    type_node = 0;
    template_type_node = 0;
    type_head.data = 0;
    type_head.length = 0U;
    template_type_head.data = 0;
    template_type_head.length = 0U;
    type_is_head = 0;
    template_type_is_head = 0;

    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = &field->value->as.frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            template_type_head = inner->head;
            template_type_is_head = 1;
        } else if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(&inner->body, 0U);
            if (type_field == 0 || type_field->value == 0) {
                fprintf(stderr, "trans L2 error: repeated array pointer declaration expects a type template\n");
                return 1;
            }
            pointer_depth = inner->head.length;
            template_type_node = type_field->value;
        } else {
            fprintf(stderr, "trans L2 error: repeated array declaration expects a type/name declaration template\n");
            return 1;
        }
    } else {
        while (
            field != 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_ATOM &&
            lm_trans_text_all_char(field->value->as.atom, '@')
        ) {
            pointer_depth += field->value->as.atom.length;
            field = field->next;
        }

        type_field = field;
        if (type_field == 0 || type_field->value == 0) {
            fprintf(stderr, "trans L2 error: repeated [] declaration expects a type template\n");
            return 1;
        }
        template_type_node = type_field->value;
    }

    name_field = repeat_body != 0 ? repeat_body->first_field : 0;
    if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans L2 error: repeated [] declaration expects a variable name\n");
        return 1;
    }

    type_node = template_type_node;
    type_head = template_type_head;
    type_is_head = template_type_is_head;
    if (
        name_field->next != 0 &&
        name_field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_head_can_declare_storage(name_field->value->as.atom, namespace_)
    ) {
        type_head = name_field->value->as.atom;
        type_node = 0;
        type_is_head = 1;
        pointer_depth = 0U;
        name_field = name_field->next;
        if (name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: repeated [] declaration override expects a variable name\n");
            return 1;
        }
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }

    dimension_field = name_field->next;
    memset(&declarator, 0, sizeof(declarator));
    declarator.type_node = type_node;
    declarator.type_head = type_head;
    declarator.name = name_node->as.atom;
    declarator.pointer_depth = pointer_depth;
    declarator.type_is_head = type_is_head;
    declarator.array_head = template_frame->head;
    declarator.expression_dimensions = dimension_field;
    if (lm_trans_emit_c_declarator(file, &declarator, namespace_, "repeated [] array dimension") != 0) {
        return 1;
    }

    initializer_field = declarator.expression_dimensions;
    if (initializer_field != 0) {
        if (lm_trans_put(file, " = {") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_list(file, initializer_field, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "}") != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_c_tagged_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *type_node;
    const LmP0Node *name_node;

    type_field = lm_trans_nth_field(&frame->body, 0U);
    name_field = lm_trans_nth_field(&frame->body, 1U);
    if (
        type_field == 0 ||
        name_field == 0 ||
        type_field->value == 0 ||
        name_field->value == 0 ||
        type_field->value->kind != LM_P0_NODE_ATOM ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: C tagged declaration expects C tag name and variable name\n");
        return 1;
    }

    type_node = type_field->value;
    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, type_node->as.atom) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, name_node->as.atom) != 0) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, "variable");
}

static int lm_trans_emit_const_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Frame *inner;

    field = frame->body.first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: const receiver expects exactly one declaration frame\n");
        return 1;
    }

    inner = &field->value->as.frame;
    if (lm_trans_text_is_array_receiver_head(inner->head)) {
        return lm_trans_emit_array_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }
    if (lm_trans_frame_looks_storage_declaration(inner, namespace_)) {
        return lm_trans_emit_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }
    if (lm_trans_text_all_char(inner->head, '@')) {
        return lm_trans_emit_pointer_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }

    fprintf(stderr, "trans L2 error: const receiver expects storage, pointer, or array declaration\n");
    return 1;
}

static int lm_trans_emit_call_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    LmTransCallLowering call;
    int has_args;

    if (lm_trans_lower_call(frame->head, namespace_, "callable", &call) != 0) {
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (call.is_closure) {
        has_args = call.signature != 0 && call.signature->param_names.count > 0U;
        if (!has_args && lm_trans_call_body_first_field(&frame->body) != 0) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            return 1;
        }
        if (lm_trans_emit_name(file, call.name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "->call(") != 0) {
            return 1;
        }
        if (lm_trans_emit_name(file, call.name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "->env") != 0) {
            return 1;
        }
        if (has_args) {
            if (lm_trans_put(file, ", ") != 0) {
                return 1;
            }
            if (lm_trans_emit_call_args(file, &frame->body, namespace_, call.signature) != 0) {
                return 1;
            }
        }
        if (lm_trans_put(file, ")") != 0) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }
    if (lm_trans_emit_name(file, call.name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_emit_call_args(file, &frame->body, namespace_, call.signature) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ")") != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_text_contains_char(LmP0Text text, char ch) {
    size_t i;

    i = 0U;
    while (i < text.length) {
        if (text.data[i] == ch) {
            return 1;
        }
        ++i;
    }
    return 0;
}

static int lm_trans_head_looks_assignable_target(LmP0Text head) {
    return
        lm_trans_text_contains_char(head, '[') ||
        lm_trans_text_contains_char(head, '\\') ||
        lm_trans_c_reference_has_path_dot(head);
}

static int lm_trans_emit_assignment_target(FILE *file, LmP0Text target) {
    size_t i;
    size_t deref_depth;
    LmP0Text target_payload;
    LmP0Text target_path;
    int is_c_reference_path;

    if (!lm_trans_identifier_payload(target, &target_payload)) {
        return 1;
    }

    deref_depth = 0U;
    while (
        deref_depth < target_payload.length &&
        target_payload.data[deref_depth] == '\\'
    ) {
        ++deref_depth;
    }

    target_path.data = target_payload.data + deref_depth;
    target_path.length = target_payload.length - deref_depth;
    if (target_path.length == 0U) {
        fprintf(stderr, "trans L2 error: dereferenced assignment target expects a binding or field-follow path\n");
        return 1;
    }

    is_c_reference_path = lm_trans_is_c_reference_name(target_path);
    if (!is_c_reference_path && lm_trans_text_contains_char(target_path, '.')) {
        fprintf(stderr, "trans L2 error: dot belongs only to c.* ANSI C surface names; use \\ for Lingvamyxa structures\n");
        return 1;
    }

    while (deref_depth > 0U) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
        --deref_depth;
    }
    if (
        target_payload.length != target_path.length &&
        lm_trans_put(file, "(") != 0
    ) {
        return 1;
    }

    i = 0U;
    if (is_c_reference_path) {
        i = 2U;
    }
    while (i < target_path.length) {
        if (target_path.data[i] == '\\') {
            if (lm_trans_put(file, "->") != 0) {
                return 1;
            }
        } else if (lm_trans_write_all(file, target_path.data + i, 1U) != 0) {
            return 1;
        }
        ++i;
    }
    if (
        target_payload.length != target_path.length &&
        lm_trans_put(file, ")") != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_target_assignment(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_assignment_target(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, frame->body.first_field, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_assignment(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, frame->body.first_field, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_struct_field_with_qualifier(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    const char *qualifier
) {
    const LmP0Frame *frame;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: structure field must be a named field\n");
        return 1;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        name_field = frame->body.first_field;
        if (
            name_field == 0 ||
            name_field->next != 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_FRAME
        ) {
            fprintf(stderr, "trans L2 error: const structure field expects exactly one field declaration\n");
            return 1;
        }
        (void)qualifier;
        return lm_trans_emit_struct_field_with_qualifier(file, name_field->value, indent, "const ");
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        type_field = lm_trans_nth_field(&frame->body, 0U);
        name_field = lm_trans_nth_field(&frame->body, 1U);
        if (
            type_field == 0 ||
            name_field == 0 ||
            name_field->next != 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: pointer structure field expects type and name\n");
            return 1;
        }
        name_node = name_field->value;
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
            return 1;
        }
        if (
            lm_trans_emit_type_and_name(
                file,
                type_field->value,
                name_node->as.atom,
                frame->head.length,
                0
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == 0 ||
        name_field->next != 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: structure field expects Type: name\n");
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, name_field->value->as.atom) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_struct_field(
    FILE *file,
    const LmP0Node *node,
    unsigned indent
) {
    return lm_trans_emit_struct_field_with_qualifier(file, node, indent, "");
}

static int lm_trans_emit_function_return_structure(
    FILE *file,
    LmP0Text function_name,
    const LmP0Node *return_node
) {
    const LmP0Field *field;

    if (return_node == 0 || return_node->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: fm expects a return Structure\n");
        return 1;
    }

    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\n\nstruct ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " {\n") != 0) {
        return 1;
    }

    field = return_node->as.structure.first_field;
    if (field == 0) {
        fprintf(stderr, "trans L2 error: fm return Structure must not be empty\n");
        return 1;
    }
    while (field != 0) {
        if (lm_trans_emit_struct_field(file, field->value, 1U) != 0) {
            return 1;
        }
        field = field->next;
    }

    return lm_trans_put(file, "};\n\n");
}

static int lm_trans_emit_function_return_structure_once(
    FILE *file,
    LmP0Text function_name,
    const LmP0Node *return_node
) {
    char *type_name;
    const char *stored_name;

    type_name = lm_trans_function_return_struct_type_name_new(function_name);
    if (type_name == 0) {
        return 1;
    }

    stored_name = lm_trans_string_stack_find(&lm_trans_emitted_function_return_structs, type_name);
    if (stored_name != 0) {
        free(type_name);
        return 0;
    }

    if (lm_trans_emit_function_return_structure(file, function_name, return_node) != 0) {
        free(type_name);
        return 1;
    }

    if (lm_own_ptr_stack_push(&lm_trans_emitted_function_return_structs, type_name) != 0) {
        free(type_name);
        return 1;
    }

    return 0;
}

static int lm_trans_frame_looks_label_declaration(const LmP0Frame *frame) {
    return
        frame != 0 &&
        !lm_trans_is_c_reference_name(frame->head) &&
        !lm_trans_is_reserved_head_name(frame->head) &&
        (frame->flags & LM_P0_FRAME_COLON) != 0U &&
        (frame->flags & LM_P0_FRAME_INLINE_BODY) == 0U;
}

static void lm_trans_statement_stack_destroy(LmTransStatementStack *stack) {
    if (stack != 0) {
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_statement_stack_push(LmTransStatementStack *stack, LmTransStatementJob job) {
    if (stack == 0) {
        return 1;
    }

    return lm_own_value_stack_push(&stack->jobs, &job);
}

static int lm_trans_statement_job_emit_list(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_emit_node(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_validate_end(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_emit_trailer(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_leave_scope(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_loop_pop(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_cleanup_pop(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_emit_indent_text(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);
static int lm_trans_statement_job_emit_sync_leave(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
);

static int lm_trans_statement_stack_push_list(
    LmTransStatementStack *stack,
    const LmP0Field *field,
    unsigned indent,
    int unwrap_single_structure,
    const LmP0Frame *repeat_frame
) {
    LmTransStatementJob job;

    job.run = lm_trans_statement_job_emit_list;
    job.as.list.field = field;
    job.as.list.indent = indent;
    job.as.list.unwrap_single_structure = unwrap_single_structure;
    job.as.list.repeat_frame = repeat_frame;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_node(
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent,
    const LmP0Frame *repeat_frame
) {
    LmTransStatementJob job;

    job.run = lm_trans_statement_job_emit_node;
    job.as.node.node = node;
    job.as.node.indent = indent;
    job.as.node.repeat_frame = repeat_frame;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_frame_job(
    LmTransStatementStack *stack,
    LmTransStatementJobHandler handler,
    const LmP0Frame *frame,
    unsigned indent
) {
    LmTransStatementJob job;

    job.run = handler;
    job.as.frame.frame = frame;
    job.as.frame.indent = indent;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_simple(
    LmTransStatementStack *stack,
    LmTransStatementJobHandler handler
) {
    LmTransStatementJob job;

    job.run = handler;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_indent_text(
    LmTransStatementStack *stack,
    unsigned indent,
    const char *text
) {
    LmTransStatementJob job;

    job.run = lm_trans_statement_job_emit_indent_text;
    job.as.text.indent = indent;
    job.as.text.text = text;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_sync_leave(
    LmTransStatementStack *stack,
    unsigned indent,
    unsigned cleanup_id
) {
    LmTransStatementJob job;

    job.run = lm_trans_statement_job_emit_sync_leave;
    job.as.sync_leave.indent = indent;
    job.as.sync_leave.cleanup_id = cleanup_id;
    return lm_trans_statement_stack_push(stack, job);
}

static const LmP0Field *lm_trans_statement_list_first_field(
    const LmP0Field *first,
    int unwrap_single_structure
) {
    if (
        unwrap_single_structure &&
        first != 0 &&
        first->next == 0 &&
        first->value != 0 &&
        first->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        return first->value->as.structure.first_field;
    }

    return first;
}

static int lm_trans_statement_frame_is_repeatable(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    if (frame == 0) {
        return 0;
    }
    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        return 1;
    }
    if (
        lm_trans_text_all_char(frame->head, '@') &&
        frame->body.first_field != 0
    ) {
        return 1;
    }
    if (
        lm_trans_text_is_array_receiver_head(frame->head) &&
        frame->body.first_field != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_statement_stack_schedule_control(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *keyword
) {
    const LmP0Field *body_start;
    int is_loop;
    int status;

    is_loop = strcmp(keyword, "while") == 0;
    body_start = lm_trans_control_body_start(frame);
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, keyword) != 0 || lm_trans_put(file, " (") != 0) {
        return 1;
    }
    if (lm_trans_emit_control_condition(file, frame, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    if (is_loop && lm_trans_loop_push(namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_leave_scope) ||
        (is_loop && lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_loop_pop)) ||
        lm_trans_statement_stack_push_list(stack, body_start, indent + 1U, 1, 0);
    if (status != 0) {
        if (is_loop) {
            lm_trans_loop_pop(namespace_);
        }
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_synchronized(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *body_start;
    unsigned cleanup_id;
    int status;

    if (frame == 0 || frame->body.first_field == 0) {
        fprintf(stderr, "trans L2 error: synchronized expects an object expression\n");
        return 1;
    }

    cleanup_id = namespace_->next_cleanup_id++;
    body_start = lm_trans_control_body_start(frame);
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "void *") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = (void *)(") != 0) {
        return 1;
    }
    if (lm_trans_emit_control_condition(file, frame, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ");\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "lm_synchronized_enter(") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ");\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "{\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    if (lm_trans_cleanup_push(namespace_, cleanup_id) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    status =
        lm_trans_statement_stack_push_sync_leave(stack, indent, cleanup_id) ||
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_leave_scope) ||
        lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_cleanup_pop) ||
        lm_trans_statement_stack_push_list(stack, body_start, indent + 1U, 1, 0);
    if (status != 0) {
        lm_trans_cleanup_pop(namespace_);
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_else(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "else {\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_leave_scope) ||
        lm_trans_statement_stack_push_list(stack, frame->body.first_field, indent + 1U, 1, 0);
    if (status != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_label(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;

    if (lm_trans_namespace_declare(namespace_, frame->head, "label") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ":\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "{\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, lm_trans_statement_job_leave_scope) ||
        lm_trans_statement_stack_push_list(stack, frame->body.first_field, indent + 1U, 1, 0);
    if (status != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_emit_loop_jump_statement(
    FILE *file,
    LmP0Text spelling,
    unsigned indent,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l1_include_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_emit_l1_include_target_text(
    FILE *output,
    const char *data,
    size_t length
);
static int lm_trans_emit_registry_include_table(
    FILE *output,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_layout_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_constant_defines(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_alias_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_forward_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_prototypes(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_prototype_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
);
static int lm_trans_emit_l4_function_pointer_type_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_function_pointer_type_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
);
static int lm_trans_emit_l4_fn_descriptors(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_l4_is_function_pointer_type(
    const LmTransNamespace *namespace_,
    const char *name
);
static int lm_trans_registry_materialize_fn_descriptor_frame(const LmP0Frame *frame);
static int lm_trans_l4_receiver_table(const LmP0Frame *frame, int allow_node_cells);
static int lm_trans_l4_receiver_row(const LmP0Frame *frame, int allow_node_cells);
static int lm_trans_l4_receiver_fn_descriptor(const LmP0Frame *frame, int allow_node_cells);
static int lm_trans_l4_root_receiver_registry(const LmP0Frame *frame, int allow_node_cells);
static int lm_trans_l4_root_head_binding_resolve(LmP0Text head, LmTransL4HeadBinding *out);
static int lm_trans_l4_atom_receiver_prelude_sequence(LmP0Text atom, int allow_node_cells);
static int lm_trans_l4_atom_binding_resolve(LmP0Text atom, LmTransL4AtomBinding *out);
static int lm_trans_emit_l4_guard_markers(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_extern_c_markers(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_units(
    FILE *file,
    const LmTransNamespace *namespace_
);
static int lm_trans_declare_l2_registry_os_table(LmTransNamespace *namespace_);
static int lm_trans_emit_l2_registry_os_table(
    FILE *output,
    LmTransNamespace *namespace_
);
static int lm_trans_l4_payload_receiver_import(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_l4_payload_receiver_l2(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_os_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_ifdef_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_guard_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_extern_c_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_declare_l2_guard_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
);
static int lm_trans_declare_l2_extern_c_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
);
static int lm_trans_declare_l2_import_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
);
static int lm_trans_import_frame_has_code_path(const LmP0Frame *frame);
static int lm_trans_emit_l2_import_prelude(
    FILE *file,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_import_functions(
    FILE *file,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);

static int lm_trans_statement_emit_return(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_return_statement(file, frame->body.first_field, indent, namespace_);
}

static int lm_trans_statement_emit_if(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "if");
}

static int lm_trans_statement_emit_while(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "while");
}

static int lm_trans_statement_emit_else(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_else(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_emit_synchronized(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_synchronized(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_emit_loop_jump(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_loop_jump_statement(file, frame->head, indent, namespace_);
}

static int lm_trans_statement_emit_include_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    (void)namespace_;
    return lm_trans_emit_l1_include_frame(lm_trans_prelude_file(file), frame);
}

static int lm_trans_statement_emit_os_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_os_frame(lm_trans_prelude_file(file), frame, namespace_);
}

static int lm_trans_statement_emit_ifdef_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_ifdef_frame(lm_trans_prelude_file(file), frame, namespace_);
}

static int lm_trans_statement_emit_guard_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_guard_frame(lm_trans_prelude_file(file), frame, namespace_);
}

static int lm_trans_statement_emit_extern_c_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_extern_c_frame(lm_trans_prelude_file(file), frame, namespace_);
}

static int lm_trans_statement_emit_import(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)frame;
    (void)namespace_;
    if (indent == 0U) {
        return 0;
    }
    fprintf(stderr, "trans L2 error: import receiver is top-level only\n");
    return 1;
}

static int lm_trans_statement_emit_const_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_const_statement(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_array_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_array_declaration(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_structure_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)frame;
    (void)indent;
    (void)namespace_;
    fprintf(stderr, "trans L2 error: (): receiver is reserved for Structure-like variable declarations\n");
    return 0;
}

static int lm_trans_statement_emit_c_tagged_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_c_tagged_declaration(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_storage_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_declaration(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_pointer_declaration(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_pointer_declaration(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_target_assignment(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_target_assignment(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_label(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_label(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_emit_call(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_call_statement(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_assignment(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_assignment(file, frame, indent, namespace_);
}

static int lm_trans_statement_emit_nested_function(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
);
static int lm_trans_function_header_from_frame(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
);
static int lm_trans_namespace_set_signature(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const LmP0Frame *function_frame
);
static int lm_trans_namespace_set_callable_shape(
    LmTransNamespace *namespace_,
    const LmTransFunctionHeader *function
);
static const LmTransHoistedFunction *lm_trans_namespace_find_hoisted_function(
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame
);
static char *lm_trans_env_arg_new(LmP0Text env_var_name);

static int lm_trans_statement_lowering_from_head(
    LmP0Text head,
    LmTransStatementLowering *out
) {
    LmTransHeadBinding binding;

    if (out == 0) {
        return 0;
    }

    if (lm_trans_head_binding_resolve(0, head, &binding) != 0) {
        return 0;
    }
    if (binding.statement_frame == 0) {
        return 0;
    }

    out->emit = binding.statement_frame;
    return 1;
}

static int lm_trans_lower_statement_frame(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_,
    LmTransStatementLowering *out
) {
    const LmTransSymbol *symbol;
    LmTransFunctionHeader function;
    int function_status;

    if (frame == 0 || out == 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, frame->head);
    if (symbol != 0) {
        if (lm_trans_symbol_is(symbol, "variable")) {
            out->emit = lm_trans_statement_emit_assignment;
            return 0;
        }
        if (
            lm_trans_symbol_is(symbol, "function") ||
            lm_trans_symbol_is(symbol, "procedure")
        ) {
            out->emit = lm_trans_statement_emit_call;
            return 0;
        }
    }

    function_status = lm_trans_function_header_from_frame(frame, 0, &function);
    if (function_status < 0) {
        return 1;
    }
    if (function_status > 0) {
        out->emit = lm_trans_statement_emit_nested_function;
        return 0;
    }

    if (lm_trans_statement_lowering_from_head(frame->head, out)) {
        return 0;
    }
    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        out->emit = lm_trans_statement_emit_array_declaration;
        return 0;
    }

    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        out->emit = lm_trans_statement_emit_storage_declaration;
        return 0;
    }
    if (lm_trans_text_all_char(frame->head, '@')) {
        out->emit = lm_trans_statement_emit_pointer_declaration;
        return 0;
    }
    if (lm_trans_head_looks_assignable_target(frame->head)) {
        out->emit = lm_trans_statement_emit_target_assignment;
        return 0;
    }
    if (lm_trans_frame_looks_label_declaration(frame)) {
        out->emit = lm_trans_statement_emit_label;
        return 0;
    }
    if (lm_trans_is_c_reference_name(frame->head)) {
        out->emit = lm_trans_statement_emit_call;
        return 0;
    }

    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 statement error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)frame->head.length,
            frame->head.data,
            (int)frame->head.length,
            frame->head.data
        );
        return 1;
    }

    fprintf(
        stderr,
        "trans L2 error: \"%.*s\" is %s, not a statement head\n",
        (int)frame->head.length,
        frame->head.data,
        lm_trans_symbol_class_name(symbol->class_name)
    );
    return 1;
}

static int lm_trans_emit_loop_jump_statement(
    FILE *file,
    LmP0Text spelling,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (lm_trans_emit_cleanups_until(file, indent, namespace_, lm_trans_loop_cleanup_base(namespace_)) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, spelling) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_statement_stack_emit_frame_lowering(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    LmTransStatementLowering lowering
) {
    if (lowering.emit == 0) {
        return 1;
    }
    return lowering.emit(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_stack_emit_frame(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;
    LmTransStatementLowering lowering;

    status =
        lm_trans_statement_stack_push_frame_job(stack, lm_trans_statement_job_emit_trailer, frame, indent) ||
        lm_trans_statement_stack_push_frame_job(stack, lm_trans_statement_job_validate_end, frame, indent);
    if (status != 0) {
        return 1;
    }

    if (lm_trans_lower_statement_frame(frame, namespace_, &lowering) != 0) {
        return 1;
    }
    return lm_trans_statement_stack_emit_frame_lowering(file, stack, frame, indent, namespace_, lowering);
}

static int lm_trans_emit_atom_loop_jump_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_loop_jump_statement(file, node->as.atom, indent, namespace_);
}

static int lm_trans_emit_atom_return_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    return lm_trans_emit_return_statement(file, 0, indent, namespace_);
}

static int lm_trans_emit_atom_string_error_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)node;
    (void)indent;
    (void)namespace_;
    fprintf(stderr, "trans L2 error: standalone string field is not consumed by any L2 receiver\n");
    return 1;
}

static int lm_trans_emit_atom_expr_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_node(file, node, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_atom_statement_emit_include_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_registry_include_table(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_os_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l2_registry_os_table(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_layout_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_layout_typedefs(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_constant_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_constant_defines(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_alias_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_alias_typedefs(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_forward_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_forward_typedefs(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_prototype_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_prototypes(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_fn_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_fn_descriptors(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_guard_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_guard_markers(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_extern_c_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_extern_c_markers(lm_trans_prelude_file(file), namespace_);
}

static int lm_trans_atom_statement_emit_unit_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_units(lm_trans_prelude_file(file), namespace_);
}

static LmTransAtomStatementHandler lm_trans_atom_statement_binding_handler(
    const char *binding
);
static int lm_trans_atom_statement_emit_sequence_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
);

static int lm_trans_sequence_item_index(const char *payload, size_t *out_index) {
    char *end;
    unsigned long value;

    if (payload == 0 || payload[0] == '\0' || out_index == 0) {
        return 0;
    }

    value = strtoul(payload, &end, 10);
    if (end == payload || *end != '\0') {
        return 0;
    }
    *out_index = (size_t)value;
    return 1;
}

static int lm_trans_emit_atom_statement_sequence(
    FILE *file,
    LmP0Text sequence_name,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmOwnPtrStack *rows;
    LmTransRegistryFact *row;
    LmTransAtomStatementHandler handler;
    LmP0Node node;
    LmP0Text item_name;
    const char *binding;
    size_t emitted;
    size_t expected_index;
    size_t index;
    size_t i;
    int found;

    rows = lm_trans_namespace_registry_relation_stack(namespace_, sequence_name, "item");
    if (rows == 0) {
        fprintf(
            stderr,
            "trans registry error: atom statement sequence \"%.*s\" has no item rows\n",
            (int)sequence_name.length,
            sequence_name.data
        );
        return 1;
    }

    emitted = 0U;
    expected_index = 0U;
    while (emitted < rows->count) {
        found = 0;
        for (i = 0U; i < rows->count; ++i) {
            row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
            if (
                row != 0 &&
                row->key != 0 &&
                lm_trans_sequence_item_index(row->payload, &index) &&
                index == expected_index
            ) {
                if (found) {
                    fprintf(
                        stderr,
                        "trans registry error: atom statement sequence \"%.*s\" has duplicate index %lu\n",
                        (int)sequence_name.length,
                        sequence_name.data,
                        (unsigned long)expected_index
                    );
                    return 1;
                }
                found = 1;
                item_name = lm_trans_text_from_cstr(row->key);
                binding = lm_trans_registry_lookup(item_name, "receiver.atom.statement");
                handler = lm_trans_atom_statement_binding_handler(binding);
                if (handler == 0 || handler == lm_trans_atom_statement_emit_sequence_prelude) {
                    fprintf(
                        stderr,
                        "trans registry error: atom statement sequence \"%.*s\" item %s has no concrete atom receiver\n",
                        (int)sequence_name.length,
                        sequence_name.data,
                        row->key
                    );
                    return 1;
                }
                memset(&node, 0, sizeof(node));
                node.kind = LM_P0_NODE_ATOM;
                node.as.atom = item_name;
                if (handler(file, &node, indent, namespace_) != 0) {
                    return 1;
                }
                ++emitted;
            }
        }
        if (!found) {
            fprintf(
                stderr,
                "trans registry error: atom statement sequence \"%.*s\" is missing index %lu\n",
                (int)sequence_name.length,
                sequence_name.data,
                (unsigned long)expected_index
            );
            return 1;
        }
        ++expected_index;
    }

    return 0;
}

static int lm_trans_atom_statement_emit_sequence_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (node == 0 || node->kind != LM_P0_NODE_ATOM) {
        return 1;
    }
    return lm_trans_emit_atom_statement_sequence(file, node->as.atom, indent, namespace_);
}

static int lm_trans_emit_configured_prelude_sequences(
    FILE *file,
    LmTransNamespace *namespace_,
    int *out_emitted
) {
    const LmOwnPtrStack *rows;
    LmTransRegistryFact *row;
    size_t i;

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("prelude"),
        "sequence"
    );
    if (rows == 0) {
        return 0;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (row != 0 && row->key != 0) {
            if (out_emitted != 0 && *out_emitted && lm_trans_put(file, "\n") != 0) {
                return 1;
            }
            if (
                lm_trans_emit_atom_statement_sequence(
                    file,
                    lm_trans_text_from_cstr(row->key),
                    0U,
                    namespace_
                ) != 0
            ) {
                return 1;
            }
            if (out_emitted != 0) {
                *out_emitted = 1;
            }
        }
    }

    return 0;
}

static LmTransAtomStatementHandler lm_trans_atom_statement_binding_handler(
    const char *binding
) {
    if (binding == 0) {
        return 0;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_include_prelude") == 0) {
        return lm_trans_atom_statement_emit_include_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_os_prelude") == 0) {
        return lm_trans_atom_statement_emit_os_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_layout_prelude") == 0) {
        return lm_trans_atom_statement_emit_layout_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_constant_prelude") == 0) {
        return lm_trans_atom_statement_emit_constant_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_alias_prelude") == 0) {
        return lm_trans_atom_statement_emit_alias_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_forward_prelude") == 0) {
        return lm_trans_atom_statement_emit_forward_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_prototype_prelude") == 0) {
        return lm_trans_atom_statement_emit_prototype_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_fn_prelude") == 0) {
        return lm_trans_atom_statement_emit_fn_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_guard_prelude") == 0) {
        return lm_trans_atom_statement_emit_guard_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_extern_c_prelude") == 0) {
        return lm_trans_atom_statement_emit_extern_c_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_unit_prelude") == 0) {
        return lm_trans_atom_statement_emit_unit_prelude;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_sequence_prelude") == 0) {
        return lm_trans_atom_statement_emit_sequence_prelude;
    }
    return 0;
}

static LmTransAtomStatementHandler lm_trans_lower_atom_statement(LmP0Text atom) {
    const char *binding;
    LmTransAtomStatementHandler handler;

    if (lm_trans_text_equals(atom, "break")) {
        return lm_trans_emit_atom_loop_jump_statement;
    }
    if (lm_trans_text_equals(atom, "continue")) {
        return lm_trans_emit_atom_loop_jump_statement;
    }
    if (lm_trans_text_equals(atom, "return")) {
        return lm_trans_emit_atom_return_statement;
    }
    if (lm_trans_atom_starts_string(atom)) {
        return lm_trans_emit_atom_string_error_statement;
    }
    binding = lm_trans_registry_lookup(atom, "receiver.atom.statement");
    if (binding != 0) {
        handler = lm_trans_atom_statement_binding_handler(binding);
        if (handler != 0) {
            return handler;
        }
        fprintf(stderr, "trans registry error: unknown atom statement binding %s\n", binding);
        return 0;
    }
    return lm_trans_emit_atom_expr_statement;
}

static int lm_trans_statement_emit_nested_function(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    LmTransFunctionHeader function;
    const LmTransHoistedFunction *hoisted;
    const LmTransCapture *capture;
    char *env_arg;
    LmP0Text env_arg_text;
    size_t capture_index;
    int status;

    (void)stack;

    status = lm_trans_function_header_from_frame(frame, 0, &function);
    if (status <= 0) {
        return 1;
    }

    hoisted = lm_trans_namespace_find_hoisted_function(namespace_, frame);
    if (hoisted == 0) {
        fprintf(stderr, "trans L2 internal error: nested function has no hoisted C binding\n");
        return 1;
    }

    if (hoisted->function.has_env) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->function.env_type_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " *") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " = (") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->function.env_type_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " *)lm_own_new_zero(sizeof(*") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "));\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "if (") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " == 0) {\n") != 0) {
            return 1;
        }
        if (namespace_ != 0 && namespace_->return_type_node != 0) {
            if (
                lm_trans_emit_indent(file, indent + 1U) != 0 ||
                lm_trans_put(file, "return 0;\n") != 0
            ) {
                return 1;
            }
        } else if (
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_put(file, "return;\n") != 0
        ) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "}\n") != 0) {
            return 1;
        }
        for (capture_index = 0U; capture_index < hoisted->captures.count; ++capture_index) {
            capture = (const LmTransCapture *)lm_own_ptr_stack_at(&hoisted->captures, capture_index);
            if (capture == 0) {
                return 1;
            }
            if (lm_trans_emit_indent(file, indent) != 0) {
                return 1;
            }
            if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
                return 1;
            }
            if (lm_trans_put(file, "->") != 0) {
                return 1;
            }
            if (lm_trans_emit_identifier(file, capture->name) != 0) {
                return 1;
            }
            if (lm_trans_put(file, " = ") != 0) {
                return 1;
            }
            if (lm_trans_emit_identifier(file, capture->name) != 0) {
                return 1;
            }
            if (lm_trans_put(file, ";\n") != 0) {
                return 1;
            }
        }
    }

    if (
        lm_trans_namespace_declare_c_name(
            namespace_,
            function.name,
            function.symbol_class,
            hoisted->function.c_name
        ) != 0
    ) {
        return 1;
    }

    if (hoisted->function.has_env) {
        env_arg = lm_trans_env_arg_new(hoisted->env_var_name);
        if (env_arg == 0) {
            return 1;
        }
        env_arg_text = lm_trans_text_from_cstr(env_arg);
        status = lm_trans_namespace_set_env_arg(namespace_, function.name, env_arg_text);
        free(env_arg);
        if (status != 0) {
            return 1;
        }
    }

    if (lm_trans_namespace_set_closure_call_name(namespace_, function.name, hoisted->closure_call_name) != 0) {
        return 1;
    }

    if (
        !hoisted->function.is_sub &&
        !hoisted->function.is_struct_return &&
        hoisted->function.return_node != 0
    ) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "(void)") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, hoisted->closure_call_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
    }

    return lm_trans_namespace_set_callable_shape(namespace_, &function);
}

static int lm_trans_statement_stack_emit_node(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent,
    const LmP0Frame *repeat_frame,
    LmTransNamespace *namespace_
) {
    LmTransAtomStatementHandler handler;
    LmP0Frame repeated_frame;

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (lm_trans_node_is_positional_skip(node)) {
        fprintf(stderr, "trans L2 error: skipped positional statement has no default in this profile\n");
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        handler = lm_trans_lower_atom_statement(node->as.atom);
        if (handler == 0) {
            return 1;
        }
        return handler(file, node, indent, namespace_);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_statement_stack_emit_frame(file, stack, &node->as.frame, indent, namespace_);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE && repeat_frame != 0) {
        if (lm_trans_frame_looks_storage_declaration(repeat_frame, namespace_)) {
            memset(&repeated_frame, 0, sizeof(repeated_frame));
            repeated_frame.head = repeat_frame->head;
            repeated_frame.flags = LM_P0_FRAME_COMPACT;
            repeated_frame.body = node->as.structure;
            return lm_trans_statement_stack_emit_frame(file, stack, &repeated_frame, indent, namespace_);
        }
        if (lm_trans_text_all_char(repeat_frame->head, '@')) {
            return lm_trans_emit_pointer_declaration_repeat_with_qualifier(
                file,
                repeat_frame,
                &node->as.structure,
                indent,
                namespace_,
                ""
            );
        }
        if (lm_trans_text_is_array_receiver_head(repeat_frame->head)) {
            return lm_trans_emit_array_declaration_repeat_with_qualifier(
                file,
                repeat_frame,
                &node->as.structure,
                indent,
                namespace_,
                ""
            );
        }
        memset(&repeated_frame, 0, sizeof(repeated_frame));
        repeated_frame.head = repeat_frame->head;
        repeated_frame.flags = LM_P0_FRAME_COMPACT;
        repeated_frame.body = node->as.structure;
        return lm_trans_statement_stack_emit_frame(file, stack, &repeated_frame, indent, namespace_);
    }

    return 0;
}

static int lm_trans_statement_stack_emit_sync_leave(
    FILE *file,
    unsigned indent,
    unsigned cleanup_id
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "lm_synchronized_leave(") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    return lm_trans_put(file, ");\n");
}

static int lm_trans_statement_job_emit_list(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *current_repeat_frame;
    const LmP0Frame *next_repeat_frame;
    int status;

    (void)file;
    field = lm_trans_statement_list_first_field(
        job->as.list.field,
        job->as.list.unwrap_single_structure
    );
    if (field == 0) {
        return 0;
    }

    node = field->value;
    current_repeat_frame =
        job->as.list.repeat_frame != 0 &&
        node != 0 &&
        node->kind == LM_P0_NODE_STRUCTURE
        ? job->as.list.repeat_frame
        : 0;

    next_repeat_frame = 0;
    if (
        node != 0 &&
        node->kind == LM_P0_NODE_FRAME &&
        lm_trans_statement_frame_is_repeatable(&node->as.frame, namespace_)
    ) {
        next_repeat_frame = &node->as.frame;
    } else if (current_repeat_frame != 0) {
        next_repeat_frame = current_repeat_frame;
    }

    status = 0;
    if (field->next != 0) {
        status = lm_trans_statement_stack_push_list(
            stack,
            field->next,
            job->as.list.indent,
            0,
            next_repeat_frame
        );
    }
    if (status == 0) {
        status = lm_trans_statement_stack_push_node(
            stack,
            field->value,
            job->as.list.indent,
            current_repeat_frame
        );
    }
    return status;
}

static int lm_trans_statement_job_emit_node(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_emit_node(
        file,
        stack,
        job->as.node.node,
        job->as.node.indent,
        job->as.node.repeat_frame,
        namespace_
    );
}

static int lm_trans_statement_job_validate_end(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)namespace_;
    return lm_trans_validate_end_trailer(job->as.frame.frame);
}

static int lm_trans_statement_job_emit_trailer(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)stack;
    return lm_trans_emit_trailer_statement(
        file,
        job->as.frame.frame->trailer,
        job->as.frame.indent,
        namespace_
    );
}

static int lm_trans_statement_job_leave_scope(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)job;
    lm_trans_namespace_leave_scope(namespace_);
    return 0;
}

static int lm_trans_statement_job_loop_pop(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)job;
    lm_trans_loop_pop(namespace_);
    return 0;
}

static int lm_trans_statement_job_cleanup_pop(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)file;
    (void)stack;
    (void)job;
    lm_trans_cleanup_pop(namespace_);
    return 0;
}

static int lm_trans_statement_job_emit_indent_text(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    if (lm_trans_emit_indent(file, job->as.text.indent) != 0) {
        return 1;
    }
    return lm_trans_put(file, job->as.text.text);
}

static int lm_trans_statement_job_emit_sync_leave(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_statement_stack_emit_sync_leave(
        file,
        job->as.sync_leave.indent,
        job->as.sync_leave.cleanup_id
    );
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    LmTransStatementStack stack;
    LmTransStatementJob job;
    int status;

    lm_own_value_stack_init(&stack.jobs, sizeof(LmTransStatementJob));
    status = lm_trans_statement_stack_push_list(&stack, first, indent, 1, 0);

    while (status == 0 && stack.jobs.count > 0U) {
        if (lm_own_value_stack_pop(&stack.jobs, &job) != 0) {
            status = 1;
            break;
        }
        if (job.run == 0) {
            status = 1;
        } else {
            status = job.run(file, &stack, &job, namespace_);
        }
    }

    lm_trans_statement_stack_destroy(&stack);
    return status;
}

static int lm_trans_emit_params_body(
    FILE *file,
    const LmP0Node *params,
    LmTransNamespace *namespace_,
    int emit_void_when_empty
) {
    const LmP0Field *field;
    int first;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: function parameters must be a Structure\n");
        return 1;
    }

    first = 1;
    field = params->as.structure.first_field;
    while (field != 0) {
        if (!first && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_param(file, field->value, namespace_) != 0) {
            return 1;
        }
        first = 0;
        field = field->next;
    }

    if (first && emit_void_when_empty) {
        return lm_trans_put(file, "void");
    }

    return 0;
}

static int lm_trans_emit_params(
    FILE *file,
    const LmP0Node *params,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_params_body(file, params, namespace_, 1);
}

static int lm_trans_emit_function_params(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    int has_user_params;

    if (function == 0) {
        return 1;
    }

    has_user_params = lm_trans_params_has_any(function->params_node);
    if (function->has_env) {
        if (lm_trans_write_text(file, function->env_type_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " *lm_env") != 0) {
            return 1;
        }
        if (has_user_params && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        return lm_trans_emit_params_body(file, function->params_node, namespace_, 0);
    }

    return lm_trans_emit_params(file, function->params_node, namespace_);
}

static int lm_trans_formal_param_name(const LmP0Node *node, LmP0Text *out_name);

static int lm_trans_emit_param_name_list(
    FILE *file,
    const LmP0Node *params
) {
    const LmP0Field *field;
    LmP0Text name;
    int first;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    first = 1;
    field = params->as.structure.first_field;
    while (field != 0) {
        if (!lm_trans_formal_param_name(field->value, &name)) {
            return 1;
        }
        if (!first && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, name) != 0) {
            return 1;
        }
        first = 0;
        field = field->next;
    }
    return 0;
}

static int lm_trans_emit_closure_call_wrapper(
    FILE *file,
    const LmTransHoistedFunction *hoisted,
    LmTransNamespace *namespace_
) {
    const LmTransFunctionHeader *function;
    int has_params;

    if (hoisted == 0) {
        return 1;
    }
    function = &hoisted->function;
    if (function->is_sub || function->is_struct_return || function->return_node == 0) {
        return 0;
    }

    if (lm_trans_put(file, "static ") != 0) {
        return 1;
    }
    if (lm_trans_emit_type_node(file, function->return_node) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, hoisted->closure_call_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(void *lm_env") != 0) {
        return 1;
    }
    has_params = lm_trans_params_has_any(function->params_node);
    if (has_params && lm_trans_put(file, ", ") != 0) {
        return 1;
    }
    if (lm_trans_emit_params_body(file, function->params_node, namespace_, 0) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        return 1;
    }
    if (!function->has_env) {
        if (lm_trans_put(file, "    (void)lm_env;\n") != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, "    return ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (function->has_env) {
        if (lm_trans_put(file, "(") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, function->env_type_name) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " *)lm_env") != 0) {
            return 1;
        }
        if (has_params && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
    }
    if (lm_trans_emit_param_name_list(file, function->params_node) != 0) {
        return 1;
    }
    return lm_trans_put(file, ");\n}\n");
}

static int lm_trans_single_frame_node(const LmP0Node *node, const LmP0Frame **out_frame) {
    const LmP0Field *field;

    if (node == 0 || out_frame == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        *out_frame = &node->as.frame;
        return 1;
    }

    if (node->kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }

    field = node->as.structure.first_field;
    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        *out_frame = &field->value->as.frame;
        return 1;
    }

    return 0;
}

static int lm_trans_formal_param_name(const LmP0Node *node, LmP0Text *out_name) {
    const LmP0Frame *frame;
    const LmP0Field *field;
    const LmP0Field *name_field;
    size_t name_index;

    if (out_name == 0 || !lm_trans_single_frame_node(node, &frame)) {
        return 0;
    }

    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        return lm_trans_formal_param_name(field->value, out_name);
    }

    if (lm_trans_frame_positional_name_index(frame, &name_index)) {
        name_field = lm_trans_nth_field(&frame->body, name_index);
    } else {
        name_field = lm_trans_nth_field(&frame->body, 0U);
    }

    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_name = name_field->value->as.atom;
    return 1;
}

static int lm_trans_namespace_set_signature(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const LmP0Frame *function_frame
) {
    LmTransSymbol *symbol;
    const LmP0Field *params_field;
    const LmP0Field *field;
    LmP0Text param_name;
    LmOwnPtrStack param_names;
    LmP0Text *param_name_ref;
    LmP0Text *existing_param_name_ref;
    size_t index;
    size_t i;
    int allow_unnamed_params;

    if (namespace_ == 0 || function_frame == 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }

    params_field = lm_trans_nth_field(&function_frame->body, 1U);
    if (
        params_field == 0 ||
        params_field->value == 0 ||
        params_field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects a parameter Structure\n");
        return 1;
    }

    lm_own_ptr_stack_init(&param_names, lm_trans_text_ref_delete_any);

    allow_unnamed_params = lm_trans_symbol_is(symbol, "callableDescriptor");
    index = 0U;
    field = params_field->value->as.structure.first_field;
    while (field != 0) {
        if (!lm_trans_formal_param_name(field->value, &param_name)) {
            if (!allow_unnamed_params) {
                fprintf(stderr, "trans L2 error: function parameter must expose a binding name\n");
                lm_own_ptr_stack_destroy(&param_names);
                return 1;
            }
            param_name = lm_trans_text_from_cstr("");
        }
        for (i = 0U; i < index; ++i) {
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(&param_names, i);
            if (
                param_name.length != 0U &&
                param_name_ref != 0 &&
                lm_trans_identifier_same(*param_name_ref, param_name)
            ) {
                fprintf(stderr, "trans L2 error: duplicate function parameter name\n");
                lm_own_ptr_stack_destroy(&param_names);
                return 1;
            }
        }
        param_name_ref = lm_trans_text_ref_new(param_name);
        if (param_name_ref == 0) {
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        if (lm_own_ptr_stack_push(&param_names, param_name_ref) != 0) {
            lm_trans_text_ref_delete_any(param_name_ref);
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        ++index;
        field = field->next;
    }

    if (symbol->has_signature) {
        if (symbol->param_names.count != param_names.count) {
            fprintf(stderr, "trans L2 error: os-imported function signatures must match\n");
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        for (i = 0U; i < param_names.count; ++i) {
            existing_param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(&symbol->param_names, i);
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(&param_names, i);
            if (
                existing_param_name_ref == 0 ||
                param_name_ref == 0 ||
                !lm_trans_identifier_same(*existing_param_name_ref, *param_name_ref)
            ) {
                fprintf(stderr, "trans L2 error: os-imported function signatures must match\n");
                lm_own_ptr_stack_destroy(&param_names);
                return 1;
            }
        }
        lm_own_ptr_stack_destroy(&param_names);
        return 0;
    }

    lm_own_ptr_stack_destroy(&symbol->param_names);
    symbol->param_names = param_names;
    symbol->has_signature = 1;
    return 0;
}

static int lm_trans_namespace_set_callable_shape(
    LmTransNamespace *namespace_,
    const LmTransFunctionHeader *function
) {
    LmTransSymbol *symbol;

    if (namespace_ == 0 || function == 0) {
        return 1;
    }

    if (lm_trans_namespace_set_signature(namespace_, function->name, function->frame) != 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, function->name);
    if (symbol == 0) {
        return 1;
    }

    symbol->callable_params_node = function->params_node;
    symbol->callable_return_node = function->return_node;
    symbol->has_callable_shape = 1;
    symbol->callable_returns_value = !function->is_sub;
    symbol->callable_is_struct_return = function->is_struct_return;
    symbol->callable_is_external = function->is_external;
    return 0;
}

static int lm_trans_function_header_common(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out,
    int is_sub,
    int is_struct_return
) {
    const LmP0Field *name_field;
    const LmP0Field *params_field;
    const LmP0Field *return_field;
    const LmP0Field *body_field;

    if (frame == 0 || out == 0) {
        return 0;
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    params_field = lm_trans_nth_field(&frame->body, 1U);

    if (
        name_field == 0 ||
        params_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects name and parameters\n");
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->frame = frame;
    out->name = name_field->value->as.atom;
    if (!lm_trans_identifier_payload(out->name, &out->c_name)) {
        return -1;
    }
    out->params_node = params_field->value;
    out->symbol_class = is_sub ? "procedure" : "function";
    out->is_sub = is_sub;
    out->is_struct_return = is_struct_return;
    out->is_external = is_external;

    if (is_sub) {
        body_field = lm_trans_nth_field(&frame->body, 2U);
        if (
            body_field != 0 &&
            body_field->next == 0 &&
            body_field->value != 0 &&
            body_field->value->kind == LM_P0_NODE_STRUCTURE
        ) {
            out->body_start = body_field->value->as.structure.first_field;
        } else {
            out->body_start = params_field->next;
        }
        return 1;
    }

    return_field = lm_trans_nth_field(&frame->body, 2U);
    if (return_field == 0 || return_field->value == 0) {
        fprintf(stderr, "trans L2 error: fn expects return type\n");
        return -1;
    }

    out->return_node = return_field->value;
    body_field = lm_trans_nth_field(&frame->body, 3U);
    if (
        body_field != 0 &&
        body_field->next == 0 &&
        body_field->value != 0 &&
        body_field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        out->body_start = body_field->value->as.structure.first_field;
    } else {
        out->body_start = return_field->next;
    }

    if (!is_struct_return && out->body_start == 0) {
        out->symbol_class = "callableDescriptor";
        out->is_descriptor_only = 1;
    }

    return 1;
}

static int lm_trans_receiver_fn(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 0, 0);
}

static int lm_trans_receiver_fm(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 0, 1);
}

static int lm_trans_receiver_sub(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 1, 0);
}

static int lm_trans_binding_resolve(const char *binding, LmTransBinding *out) {
    if (out == 0) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    if (binding == 0) {
        return 0;
    }

    if (strcmp(binding, "lm_trans_call_lower_value") == 0) {
        out->call_lowering = lm_trans_call_lower_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_call_lower_statement") == 0) {
        out->call_lowering = lm_trans_call_lower_statement;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_value") == 0) {
        out->expr_emit = lm_trans_expr_emit_value;
        out->expr_state = lm_trans_expr_state_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_addressable_value") == 0) {
        out->expr_emit = lm_trans_expr_emit_addressable_value;
        out->expr_state = lm_trans_expr_state_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_name") == 0) {
        out->expr_emit = lm_trans_expr_emit_name;
        out->expr_state = lm_trans_expr_state_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_field_name") == 0) {
        out->expr_emit = lm_trans_expr_emit_name;
        out->expr_state = lm_trans_expr_state_field_name;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_c_field_name") == 0) {
        out->expr_emit = lm_trans_expr_emit_name;
        out->expr_state = lm_trans_expr_state_c_field_name;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_c_surface") == 0) {
        out->expr_emit = lm_trans_expr_emit_name;
        out->expr_state = lm_trans_expr_state_c_surface;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_operator") == 0) {
        out->expr_emit = lm_trans_expr_emit_raw;
        out->expr_state = lm_trans_expr_state_operator;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_deref") == 0) {
        out->expr_emit = lm_trans_expr_emit_raw;
        out->expr_state = lm_trans_expr_state_deref;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_pointer_follow") == 0) {
        out->expr_emit = lm_trans_expr_emit_raw;
        out->expr_state = lm_trans_expr_state_pointer_follow;
        return 1;
    }
    if (strcmp(binding, "lm_trans_expr_emit_c_dot") == 0) {
        out->expr_emit = lm_trans_expr_emit_raw;
        out->expr_state = lm_trans_expr_state_c_dot;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_return") == 0) {
        out->statement_frame = lm_trans_statement_emit_return;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_if") == 0) {
        out->statement_frame = lm_trans_statement_emit_if;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_while") == 0) {
        out->statement_frame = lm_trans_statement_emit_while;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_else") == 0) {
        out->statement_frame = lm_trans_statement_emit_else;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_synchronized") == 0) {
        out->statement_frame = lm_trans_statement_emit_synchronized;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_loop_jump") == 0) {
        out->statement_frame = lm_trans_statement_emit_loop_jump;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_include_prelude") == 0) {
        out->statement_frame = lm_trans_statement_emit_include_prelude;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_os_prelude") == 0) {
        out->statement_frame = lm_trans_statement_emit_os_prelude;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_ifdef_prelude") == 0) {
        out->statement_frame = lm_trans_statement_emit_ifdef_prelude;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_guard_prelude") == 0) {
        out->statement_frame = lm_trans_statement_emit_guard_prelude;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_extern_c_prelude") == 0) {
        out->statement_frame = lm_trans_statement_emit_extern_c_prelude;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_import") == 0) {
        out->statement_frame = lm_trans_statement_emit_import;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_const_declaration") == 0) {
        out->statement_frame = lm_trans_statement_emit_const_declaration;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_array_declaration") == 0) {
        out->statement_frame = lm_trans_statement_emit_array_declaration;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_structure_declaration") == 0) {
        out->statement_frame = lm_trans_statement_emit_structure_declaration;
        return 1;
    }
    if (strcmp(binding, "lm_trans_statement_emit_c_tagged_declaration") == 0) {
        out->statement_frame = lm_trans_statement_emit_c_tagged_declaration;
        return 1;
    }
    if (strcmp(binding, "lm_trans_receiver_fn") == 0) {
        out->function_receiver = lm_trans_receiver_fn;
        return 1;
    }
    if (strcmp(binding, "lm_trans_receiver_fm") == 0) {
        out->function_receiver = lm_trans_receiver_fm;
        return 1;
    }
    if (strcmp(binding, "lm_trans_receiver_sub") == 0) {
        out->function_receiver = lm_trans_receiver_sub;
        return 1;
    }
    if (strcmp(binding, "lm_trans_type_receiver_array_emit") == 0) {
        out->type_emit = lm_trans_type_receiver_array_emit;
        return 1;
    }
    if (strcmp(binding, "lm_trans_type_receiver_array_structure_value_alloc") == 0) {
        out->type_structure_value_alloc = lm_trans_type_receiver_array_structure_value_alloc;
        return 1;
    }
    if (strcmp(binding, "lm_trans_type_receiver_array_structure_value_fill") == 0) {
        out->type_structure_value_fill = lm_trans_type_receiver_array_structure_value_fill;
        return 1;
    }
    if (strcmp(binding, "lm_trans_materialize_zero_arg_callable") == 0) {
        out->expr_segment_materializer = lm_trans_materialize_zero_arg_callable;
        return 1;
    }
    if (strcmp(binding, "lm_trans_materialize_callable_descriptor_value") == 0) {
        out->expr_segment_materializer = lm_trans_materialize_callable_descriptor_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_materialize_array_value") == 0) {
        out->expr_segment_materializer = lm_trans_materialize_array_value;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_receiver_table") == 0) {
        out->l4_frame = lm_trans_l4_receiver_table;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_receiver_row") == 0) {
        out->l4_frame = lm_trans_l4_receiver_row;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_receiver_fn_descriptor") == 0) {
        out->l4_frame = lm_trans_l4_receiver_fn_descriptor;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_root_receiver_registry") == 0) {
        out->l4_frame = lm_trans_l4_root_receiver_registry;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_atom_receiver_prelude_sequence") == 0) {
        out->l4_atom = lm_trans_l4_atom_receiver_prelude_sequence;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_payload_receiver_import") == 0) {
        out->l4_payload_frame = lm_trans_l4_payload_receiver_import;
        return 1;
    }
    if (strcmp(binding, "lm_trans_l4_payload_receiver_l2") == 0) {
        out->l4_payload_frame = lm_trans_l4_payload_receiver_l2;
        return 1;
    }

    return 0;
}

static int lm_trans_function_header_from_frame(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    LmTransHeadBinding binding;

    if (frame == 0 || out == 0) {
        return 0;
    }

    if (lm_trans_head_binding_resolve(0, frame->head, &binding) != 0) {
        return -1;
    }
    if (binding.function_receiver == 0) {
        return 0;
    }

    return binding.function_receiver(frame, is_external, out);
}

static int lm_trans_top_level_function_header(
    const LmP0Frame *frame,
    LmTransFunctionHeader *out
) {
    const LmP0Field *field;
    const LmP0Frame *inner;
    int status;

    if (frame == 0 || out == 0) {
        return 0;
    }

    status = lm_trans_function_header_from_frame(frame, 0, out);
    if (status != 0) {
        return status;
    }

    if (!lm_trans_text_equals(frame->head, "external")) {
        return 0;
    }

    field = frame->body.first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: external receiver expects exactly one fn/sub frame\n");
        return -1;
    }

    inner = &field->value->as.frame;
    status = lm_trans_function_header_from_frame(inner, 1, out);
    if (status != 0) {
        return status;
    }

    fprintf(stderr, "trans L2 error: external receiver expects fn/sub frame\n");
    return -1;
}

static char *lm_trans_hoisted_c_name_new(
    LmP0Text parent_name,
    LmP0Text child_name,
    size_t index
) {
    char *name;
    int needed;

    needed = snprintf(
        0,
        0,
        "lm_%.*s_%.*s_%zu",
        (int)parent_name.length,
        parent_name.data,
        (int)child_name.length,
        child_name.data,
        index
    );
    if (needed < 0) {
        return 0;
    }

    name = (char *)malloc((size_t)needed + 1U);
    if (name == 0) {
        return 0;
    }

    if (
        snprintf(
            name,
            (size_t)needed + 1U,
            "lm_%.*s_%.*s_%zu",
            (int)parent_name.length,
            parent_name.data,
            (int)child_name.length,
            child_name.data,
            index
        ) != needed
    ) {
        free(name);
        return 0;
    }

    return name;
}

static char *lm_trans_hoisted_suffix_name_new(LmP0Text base_name, const char *suffix) {
    char *name;
    int needed;

    needed = snprintf(
        0,
        0,
        "%.*s%s",
        (int)base_name.length,
        base_name.data,
        suffix != 0 ? suffix : ""
    );
    if (needed < 0) {
        return 0;
    }

    name = (char *)malloc((size_t)needed + 1U);
    if (name == 0) {
        return 0;
    }

    if (
        snprintf(
            name,
            (size_t)needed + 1U,
            "%.*s%s",
            (int)base_name.length,
            base_name.data,
            suffix != 0 ? suffix : ""
        ) != needed
    ) {
        free(name);
        return 0;
    }

    return name;
}

static char *lm_trans_captured_expr_new(LmP0Text name) {
    char *text;
    int needed;

    needed = snprintf(
        0,
        0,
        "lm_env->%.*s",
        (int)name.length,
        name.data
    );
    if (needed < 0) {
        return 0;
    }

    text = (char *)malloc((size_t)needed + 1U);
    if (text == 0) {
        return 0;
    }
    if (
        snprintf(
            text,
            (size_t)needed + 1U,
            "lm_env->%.*s",
            (int)name.length,
            name.data
        ) != needed
    ) {
        free(text);
        return 0;
    }
    return text;
}

static char *lm_trans_env_arg_new(LmP0Text env_var_name) {
    char *text;
    int needed;

    needed = snprintf(
        0,
        0,
        "%.*s",
        (int)env_var_name.length,
        env_var_name.data
    );
    if (needed < 0) {
        return 0;
    }

    text = (char *)malloc((size_t)needed + 1U);
    if (text == 0) {
        return 0;
    }
    if (
        snprintf(
            text,
            (size_t)needed + 1U,
            "%.*s",
            (int)env_var_name.length,
            env_var_name.data
        ) != needed
    ) {
        free(text);
        return 0;
    }
    return text;
}

static const LmTransCapture *lm_trans_capture_stack_find(
    const LmOwnPtrStack *captures,
    LmP0Text name
) {
    size_t i;
    const LmTransCapture *capture;

    if (captures == 0) {
        return 0;
    }

    for (i = 0U; i < captures->count; ++i) {
        capture = (const LmTransCapture *)lm_own_ptr_stack_at(captures, i);
        if (capture != 0 && lm_trans_identifier_same(capture->name, name)) {
            return capture;
        }
    }
    return 0;
}

static int lm_trans_capture_stack_add_copy(
    LmOwnPtrStack *captures,
    const LmTransCapture *source
) {
    LmTransCapture *copy;

    if (captures == 0 || source == 0) {
        return 1;
    }

    if (lm_trans_capture_stack_find(captures, source->name) != 0) {
        return 0;
    }

    copy = lm_trans_capture_new(
        source->name,
        source->type_head,
        source->type_node,
        source->pointer_depth,
        source->type_is_head
    );
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(captures, copy) != 0) {
        lm_trans_capture_destroy(copy);
        return 1;
    }
    return 0;
}

static int lm_trans_local_name_stack_has(const LmOwnPtrStack *names, LmP0Text name) {
    size_t i;
    const LmP0Text *stored;

    if (names == 0) {
        return 0;
    }

    for (i = 0U; i < names->count; ++i) {
        stored = (const LmP0Text *)lm_own_ptr_stack_at(names, i);
        if (stored != 0 && lm_trans_identifier_same(*stored, name)) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_local_name_stack_add(LmOwnPtrStack *names, LmP0Text name) {
    LmP0Text *copy;

    if (names == 0) {
        return 1;
    }
    if (lm_trans_local_name_stack_has(names, name)) {
        return 0;
    }
    copy = lm_trans_text_ref_new(name);
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(names, copy) != 0) {
        lm_trans_text_ref_delete_any(copy);
        return 1;
    }
    return 0;
}

static int lm_trans_capture_from_decl_frame(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_,
    LmTransCapture *out
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Frame *inner;

    if (frame == 0 || out == 0) {
        return 0;
    }

    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (
            field != 0 &&
            field->next == 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_FRAME
        ) {
            return lm_trans_capture_from_decl_frame(&field->value->as.frame, namespace_, out);
        }
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(&frame->body);

    if (lm_trans_head_can_declare_storage(frame->head, namespace_)) {
        name_field = lm_trans_nth_field(body, 0U);
        if (
            name_field == 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            return 0;
        }
        memset(out, 0, sizeof(*out));
        out->name = name_field->value->as.atom;
        out->type_head = frame->head;
        out->type_is_head = 1;
        return 1;
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        type_field = lm_trans_nth_field(body, 0U);
        name_field = lm_trans_nth_field(body, 1U);
        if (
            type_field == 0 ||
            name_field == 0 ||
            type_field->value == 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            return 0;
        }
        memset(out, 0, sizeof(*out));
        out->name = name_field->value->as.atom;
        out->type_node = type_field->value;
        out->pointer_depth = frame->head.length;
        return 1;
    }

    if (
        lm_trans_text_is_array_receiver_head(frame->head) &&
        body != 0 &&
        body->first_field != 0 &&
        body->first_field->next == 0 &&
        body->first_field->value != 0 &&
        body->first_field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = &body->first_field->value->as.frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            name_field = inner->body.first_field;
            if (
                name_field == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                return 0;
            }
            memset(out, 0, sizeof(*out));
            out->name = name_field->value->as.atom;
            out->type_head = inner->head;
            out->type_is_head = 1;
            out->pointer_depth = 1U;
            return 1;
        }
        if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(&inner->body, 0U);
            name_field = lm_trans_nth_field(&inner->body, 1U);
            if (
                type_field == 0 ||
                name_field == 0 ||
                type_field->value == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                return 0;
            }
            memset(out, 0, sizeof(*out));
            out->name = name_field->value->as.atom;
            out->type_node = type_field->value;
            out->pointer_depth = inner->head.length + 1U;
            return 1;
        }
    }

    return 0;
}

static int lm_trans_capture_candidates_add_decl(
    LmOwnPtrStack *candidates,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    LmTransCapture capture;
    LmTransCapture *stored;

    if (!lm_trans_capture_from_decl_frame(frame, namespace_, &capture)) {
        return 0;
    }
    if (lm_trans_capture_stack_find(candidates, capture.name) != 0) {
        return 0;
    }

    stored = lm_trans_capture_new(
        capture.name,
        capture.type_head,
        capture.type_node,
        capture.pointer_depth,
        capture.type_is_head
    );
    if (stored == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(candidates, stored) != 0) {
        lm_trans_capture_destroy(stored);
        return 1;
    }
    return 0;
}

static int lm_trans_capture_candidates_add_params(
    LmOwnPtrStack *candidates,
    const LmP0Node *params,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Frame *frame;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }

    field = params->as.structure.first_field;
    while (field != 0) {
        if (lm_trans_single_frame_node(field->value, &frame)) {
            if (lm_trans_capture_candidates_add_decl(candidates, frame, namespace_) != 0) {
                return 1;
            }
        }
        field = field->next;
    }
    return 0;
}

static int lm_trans_capture_candidates_add_captures(
    LmOwnPtrStack *candidates,
    const LmOwnPtrStack *captures
) {
    size_t i;
    const LmTransCapture *capture;

    if (captures == 0) {
        return 0;
    }

    for (i = 0U; i < captures->count; ++i) {
        capture = (const LmTransCapture *)lm_own_ptr_stack_at(captures, i);
        if (capture != 0 && lm_trans_capture_stack_add_copy(candidates, capture) != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_collect_local_names_from_node(
    LmOwnPtrStack *names,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
);

static int lm_trans_collect_local_names_from_fields(
    LmOwnPtrStack *names,
    const LmP0Field *field,
    const LmTransNamespace *namespace_
) {
    while (field != 0) {
        if (lm_trans_collect_local_names_from_node(names, field->value, namespace_) != 0) {
            return 1;
        }
        field = field->next;
    }
    return 0;
}

static int lm_trans_collect_local_names_from_frame(
    LmOwnPtrStack *names,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    LmTransFunctionHeader nested;
    LmTransCapture capture;
    int status;

    if (frame != 0 && !lm_trans_local_name_stack_has(names, frame->head)) {
        status = lm_trans_function_header_from_frame(frame, 0, &nested);
        if (status < 0) {
            return 1;
        }
        if (status > 0) {
            return 0;
        }
    }

    if (lm_trans_capture_from_decl_frame(frame, namespace_, &capture)) {
        return lm_trans_local_name_stack_add(names, capture.name);
    }

    return lm_trans_collect_local_names_from_fields(
        names,
        frame != 0 ? frame->body.first_field : 0,
        namespace_
    );
}

static int lm_trans_collect_local_names_from_node(
    LmOwnPtrStack *names,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_node_is_ignored(node) || node == 0) {
        return 0;
    }
    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_collect_local_names_from_frame(names, &node->as.frame, namespace_);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_collect_local_names_from_fields(
            names,
            node->as.structure.first_field,
            namespace_
        );
    }
    return 0;
}

static int lm_trans_collect_capture_uses_from_node(
    LmOwnPtrStack *captures,
    const LmOwnPtrStack *candidates,
    const LmOwnPtrStack *local_names,
    const LmP0Node *node
);

static int lm_trans_collect_capture_uses_from_fields(
    LmOwnPtrStack *captures,
    const LmOwnPtrStack *candidates,
    const LmOwnPtrStack *local_names,
    const LmP0Field *field
) {
    while (field != 0) {
        if (
            lm_trans_collect_capture_uses_from_node(
                captures,
                candidates,
                local_names,
                field->value
            ) != 0
        ) {
            return 1;
        }
        field = field->next;
    }
    return 0;
}

static int lm_trans_collect_capture_uses_from_frame(
    LmOwnPtrStack *captures,
    const LmOwnPtrStack *candidates,
    const LmOwnPtrStack *local_names,
    const LmP0Frame *frame
) {
    LmTransFunctionHeader nested;
    int status;

    if (frame != 0 && !lm_trans_local_name_stack_has(local_names, frame->head)) {
        status = lm_trans_function_header_from_frame(frame, 0, &nested);
        if (status < 0) {
            return 1;
        }
        if (status > 0) {
            return 0;
        }
    }

    return lm_trans_collect_capture_uses_from_fields(
        captures,
        candidates,
        local_names,
        frame != 0 ? frame->body.first_field : 0
    );
}

static int lm_trans_collect_capture_uses_from_node(
    LmOwnPtrStack *captures,
    const LmOwnPtrStack *candidates,
    const LmOwnPtrStack *local_names,
    const LmP0Node *node
) {
    const LmTransCapture *candidate;

    if (lm_trans_node_is_ignored(node) || node == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        candidate = lm_trans_capture_stack_find(candidates, node->as.atom);
        if (
            candidate != 0 &&
            !lm_trans_local_name_stack_has(local_names, node->as.atom)
        ) {
            return lm_trans_capture_stack_add_copy(captures, candidate);
        }
        return 0;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_collect_capture_uses_from_frame(
            captures,
            candidates,
            local_names,
            &node->as.frame
        );
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_collect_capture_uses_from_fields(
            captures,
            candidates,
            local_names,
            node->as.structure.first_field
        );
    }

    return 0;
}

static int lm_trans_analyze_function_captures(
    LmTransHoistedFunction *hoisted,
    const LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack local_names;
    const LmP0Field *field;
    LmP0Text param_name;
    int status;

    if (hoisted == 0) {
        return 1;
    }

    lm_own_ptr_stack_init(&local_names, lm_trans_text_ref_delete_any);
    status = 0;

    if (
        hoisted->function.params_node != 0 &&
        hoisted->function.params_node->kind == LM_P0_NODE_STRUCTURE
    ) {
        field = hoisted->function.params_node->as.structure.first_field;
        while (status == 0 && field != 0) {
            if (lm_trans_formal_param_name(field->value, &param_name)) {
                status = lm_trans_local_name_stack_add(&local_names, param_name);
            }
            field = field->next;
        }
    }

    if (status == 0) {
        status = lm_trans_collect_local_names_from_fields(
            &local_names,
            hoisted->function.body_start,
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_collect_capture_uses_from_fields(
            &hoisted->captures,
            candidates,
            &local_names,
            hoisted->function.body_start
        );
    }

    lm_own_ptr_stack_destroy(&local_names);
    return status;
}

static int lm_trans_collect_hoisted_functions_from_node(
    LmOwnPtrStack *hoisted_functions,
    LmP0Text parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Node *node
);

static int lm_trans_collect_hoisted_functions_from_fields(
    LmOwnPtrStack *hoisted_functions,
    LmP0Text parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Field *field
) {
    while (field != 0) {
        if (lm_trans_collect_hoisted_functions_from_node(
            hoisted_functions,
            parent_c_name,
            candidates,
            namespace_,
            field->value
        ) != 0) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_collect_hoisted_function(
    LmOwnPtrStack *hoisted_functions,
    LmP0Text parent_c_name,
    const LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame,
    LmTransFunctionHeader function
) {
    LmTransHoistedFunction *hoisted;
    char *c_name;
    char *env_type_name;
    char *env_var_name;
    char *closure_call_name;
    size_t index;

    if (function.is_descriptor_only) {
        fprintf(stderr, "trans L2 error: nested descriptor-only fn is not supported yet\n");
        return 1;
    }

    index = hoisted_functions != 0 ? hoisted_functions->count : 0U;
    c_name = lm_trans_hoisted_c_name_new(parent_c_name, function.name, index);
    if (c_name == 0) {
        return 1;
    }
    closure_call_name = lm_trans_hoisted_suffix_name_new(lm_trans_text_from_cstr(c_name), "_closure_call");
    if (closure_call_name == 0) {
        free(c_name);
        return 1;
    }

    hoisted = (LmTransHoistedFunction *)lm_own_new_zero(sizeof(*hoisted));
    if (hoisted == 0) {
        free(c_name);
        free(closure_call_name);
        return 1;
    }

    hoisted->function = function;
    hoisted->function.frame = frame;
    hoisted->function.c_name = lm_trans_text_from_cstr(c_name);
    hoisted->function.is_external = 0;
    hoisted->function.declare_self_alias = 1;
    lm_own_ptr_stack_init(&hoisted->captures, lm_trans_capture_delete_any);
    hoisted->c_name_storage = c_name;
    hoisted->closure_call_storage = closure_call_name;
    hoisted->closure_call_name = lm_trans_text_from_cstr(closure_call_name);

    if (lm_trans_analyze_function_captures(hoisted, candidates, namespace_) != 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }

    if (hoisted->captures.count > 0U) {
        env_type_name = lm_trans_hoisted_suffix_name_new(hoisted->function.c_name, "Env");
        env_var_name = lm_trans_hoisted_suffix_name_new(hoisted->function.c_name, "_env");
        if (env_type_name == 0 || env_var_name == 0) {
            free(env_type_name);
            free(env_var_name);
            lm_trans_hoisted_function_destroy(hoisted);
            return 1;
        }
        hoisted->env_type_storage = env_type_name;
        hoisted->env_var_storage = env_var_name;
        hoisted->env_var_name = lm_trans_text_from_cstr(env_var_name);
        hoisted->function.env_type_name = lm_trans_text_from_cstr(env_type_name);
        hoisted->function.captures = &hoisted->captures;
        hoisted->function.has_env = 1;
    }

    if (lm_own_ptr_stack_push(hoisted_functions, hoisted) != 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }

    return 0;
}

static int lm_trans_collect_hoisted_functions_from_frame(
    LmOwnPtrStack *hoisted_functions,
    LmP0Text parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    LmTransFunctionHeader function;
    LmTransCapture ignored_capture;
    size_t scope_base;
    int status;

    if (frame != 0 && lm_trans_capture_stack_find(candidates, frame->head) == 0) {
        status = lm_trans_function_header_from_frame(frame, 0, &function);
        if (status < 0) {
            return 1;
        }
        if (status > 0) {
            return lm_trans_collect_hoisted_function(
                hoisted_functions,
                parent_c_name,
                candidates,
                namespace_,
                frame,
                function
            );
        }
    }

    if (lm_trans_capture_from_decl_frame(frame, namespace_, &ignored_capture)) {
        return lm_trans_capture_candidates_add_decl(candidates, frame, namespace_);
    }

    scope_base = candidates != 0 ? candidates->count : 0U;
    status = lm_trans_collect_hoisted_functions_from_fields(
        hoisted_functions,
        parent_c_name,
        candidates,
        namespace_,
        frame != 0 ? frame->body.first_field : 0
    );
    lm_own_ptr_stack_truncate(candidates, scope_base);
    return status;
}

static int lm_trans_collect_hoisted_functions_from_node(
    LmOwnPtrStack *hoisted_functions,
    LmP0Text parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Node *node
) {
    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_collect_hoisted_functions_from_frame(
            hoisted_functions,
            parent_c_name,
            candidates,
            namespace_,
            &node->as.frame
        );
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        size_t scope_base;
        int status;

        scope_base = candidates != 0 ? candidates->count : 0U;
        status =
            lm_trans_collect_hoisted_functions_from_fields(
                hoisted_functions,
                parent_c_name,
                candidates,
                namespace_,
                node->as.structure.first_field
            );
        lm_own_ptr_stack_truncate(candidates, scope_base);
        return status;
    }

    return 0;
}

static const LmTransHoistedFunction *lm_trans_namespace_find_hoisted_function(
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    size_t i;
    const LmTransHoistedFunction *hoisted;

    if (namespace_ == 0 || namespace_->hoisted_functions == 0 || frame == 0) {
        return 0;
    }

    for (i = 0U; i < namespace_->hoisted_functions->count; ++i) {
        hoisted = (const LmTransHoistedFunction *)lm_own_ptr_stack_at(
            namespace_->hoisted_functions,
            i
        );
        if (hoisted != 0 && hoisted->function.frame == frame) {
            return hoisted;
        }
    }

    return 0;
}

static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1);
static int lm_trans_emit_l1_os_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_emit_l1_ifdef_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_emit_l2_os_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_ifdef_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l1_include_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_top_level_declare_os(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
);
static int lm_trans_top_level_declare_ifdef(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
);
static int lm_trans_emit_function(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
);

static int lm_trans_top_level_declare_function(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    char *return_type_name_storage;
    LmP0Text return_type_name;

    if (lm_trans_registry_note_class_kind(item->function.name, item->function.symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_declare(namespace_, item->function.name, item->function.symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_set_callable_shape(namespace_, &item->function) != 0) {
        return 1;
    }
    if (item->function.is_struct_return) {
        return_type_name_storage = lm_trans_function_return_struct_type_name_new(item->function.c_name);
        if (return_type_name_storage == 0) {
            return 1;
        }
        return_type_name = lm_trans_text_from_cstr(return_type_name_storage);
        if (lm_trans_namespace_declare_generated(namespace_, return_type_name, "class") != 0) {
            free(return_type_name_storage);
            return 1;
        }
        free(return_type_name_storage);
    }
    return 0;
}

static int lm_trans_top_level_declare_function_compatible(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    char *return_type_name_storage;
    LmP0Text return_type_name;

    if (lm_trans_registry_note_class_kind(item->function.name, item->function.symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_declare_compatible(namespace_, item->function.name, item->function.symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_set_callable_shape(namespace_, &item->function) != 0) {
        return 1;
    }
    if (item->function.is_struct_return) {
        return_type_name_storage = lm_trans_function_return_struct_type_name_new(item->function.c_name);
        if (return_type_name_storage == 0) {
            return 1;
        }
        return_type_name = lm_trans_text_from_cstr(return_type_name_storage);
        if (lm_trans_namespace_declare_generated(namespace_, return_type_name, "class") != 0) {
            free(return_type_name_storage);
            return 1;
        }
        free(return_type_name_storage);
    }
    return 0;
}

static int lm_trans_top_level_emit_l1(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)namespace_;
    return lm_trans_emit_l1_frame(file, item->frame);
}

static int lm_trans_top_level_emit_os(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_os_frame(file, item->frame, namespace_);
}

static int lm_trans_top_level_emit_ifdef(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_ifdef_frame(file, item->frame, namespace_);
}

static int lm_trans_top_level_declare_guard(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_declare_l2_guard_frame(namespace_, item != 0 ? item->frame : 0);
}

static int lm_trans_top_level_emit_guard(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_guard_frame(file, item->frame, namespace_);
}

static int lm_trans_top_level_declare_extern_c(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_declare_l2_extern_c_frame(namespace_, item != 0 ? item->frame : 0);
}

static int lm_trans_top_level_emit_extern_c(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_extern_c_frame(file, item->frame, namespace_);
}

static int lm_trans_top_level_declare_import(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_declare_l2_import_frame(namespace_, item != 0 ? item->frame : 0);
}

static int lm_trans_top_level_emit_import_prelude(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_import_prelude(file, item != 0 ? item->frame : 0, namespace_);
}

static int lm_trans_top_level_emit_import_functions(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_import_functions(file, item != 0 ? item->frame : 0, namespace_);
}

static int lm_trans_top_level_emit_include(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)namespace_;
    return lm_trans_emit_l1_include_frame(file, item->frame);
}

static int lm_trans_top_level_declare_atom_os(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_declare_l2_registry_os_table(namespace_);
}

static int lm_trans_top_level_emit_atom_include(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_registry_include_table(file, namespace_);
}

static int lm_trans_top_level_emit_atom_os(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l2_registry_os_table(file, namespace_);
}

static int lm_trans_top_level_emit_atom_layout(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_layout_typedefs(file, namespace_);
}

static int lm_trans_top_level_emit_atom_constant(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_constant_defines(file, namespace_);
}

static int lm_trans_top_level_emit_atom_alias(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_alias_typedefs(file, namespace_);
}

static int lm_trans_top_level_emit_atom_forward(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_forward_typedefs(file, namespace_);
}

static int lm_trans_top_level_emit_atom_prototype(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_prototypes(file, namespace_);
}

static int lm_trans_top_level_emit_atom_fn(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_fn_descriptors(file, namespace_);
}

static int lm_trans_top_level_emit_atom_guard(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_guard_markers(file, namespace_);
}

static int lm_trans_top_level_emit_atom_extern_c(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_extern_c_markers(file, namespace_);
}

static int lm_trans_top_level_emit_atom_unit(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_units(file, namespace_);
}

static int lm_trans_top_level_emit_atom_sequence(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->node == 0 || item->node->kind != LM_P0_NODE_ATOM) {
        return 1;
    }
    return lm_trans_emit_atom_statement_sequence(file, item->node->as.atom, 0U, namespace_);
}

static int lm_trans_top_level_atom_binding(
    LmP0Text atom,
    LmTransTopLevelItem *out
) {
    const char *binding;

    if (out == 0) {
        return -1;
    }

    binding = lm_trans_registry_lookup(atom, "receiver.atom.statement");
    if (binding == 0) {
        return 0;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_include_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_include;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_os_prelude") == 0) {
        out->declare = lm_trans_top_level_declare_atom_os;
        out->emit_before_functions = lm_trans_top_level_emit_atom_os;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_layout_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_layout;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_constant_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_constant;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_alias_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_alias;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_forward_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_forward;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_prototype_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_prototype;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_fn_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_fn;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_guard_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_guard;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_extern_c_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_extern_c;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_unit_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_unit;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(binding, "lm_trans_atom_statement_emit_sequence_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_sequence;
        out->emits_top_level = 1;
        return 1;
    }

    fprintf(stderr, "trans registry error: unknown top-level atom binding %s\n", binding);
    return -1;
}

static int lm_trans_top_level_statement_binding(
    const LmTransHeadBinding *binding,
    LmTransTopLevelItem *out
) {
    const char *receiver;

    if (binding == 0 || out == 0 || binding->statement_frame == 0) {
        return 0;
    }

    receiver = binding->statement_receiver_binding;
    if (receiver == 0) {
        return 0;
    }

    if (strcmp(receiver, "lm_trans_statement_emit_include_prelude") == 0) {
        out->emit_before_functions = lm_trans_top_level_emit_include;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(receiver, "lm_trans_statement_emit_os_prelude") == 0) {
        out->declare = lm_trans_top_level_declare_os;
        out->emit_before_functions = lm_trans_top_level_emit_os;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(receiver, "lm_trans_statement_emit_ifdef_prelude") == 0) {
        out->declare = lm_trans_top_level_declare_ifdef;
        out->emit_before_functions = lm_trans_top_level_emit_ifdef;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(receiver, "lm_trans_statement_emit_guard_prelude") == 0) {
        out->declare = lm_trans_top_level_declare_guard;
        out->emit_before_functions = lm_trans_top_level_emit_guard;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(receiver, "lm_trans_statement_emit_extern_c_prelude") == 0) {
        out->declare = lm_trans_top_level_declare_extern_c;
        out->emit_before_functions = lm_trans_top_level_emit_extern_c;
        out->emits_top_level = 1;
        return 1;
    }
    if (strcmp(receiver, "lm_trans_statement_emit_import") == 0) {
        out->declare = lm_trans_top_level_declare_import;
        if (lm_trans_import_frame_has_code_path(out->frame)) {
            out->emit_before_functions = lm_trans_top_level_emit_import_prelude;
            out->emit_function = lm_trans_top_level_emit_import_functions;
        }
        return 1;
    }

    return 0;
}

static int lm_trans_top_level_emit_registry(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0) {
        return 1;
    }
    (void)file;
    (void)namespace_;
    return 0;
}

static int lm_trans_emit_callable_descriptor_params_body(
    FILE *file,
    const LmP0Node *params,
    const LmTransNamespace *namespace_,
    int emit_void_when_empty
) {
    const LmP0Field *field;
    int first;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: callable descriptor parameters must be a Structure\n");
        return 1;
    }

    first = 1;
    field = params->as.structure.first_field;
    while (field != 0) {
        if (!first && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_callable_descriptor_param_type(file, field->value, namespace_) != 0) {
            return 1;
        }
        first = 0;
        field = field->next;
    }

    if (first && emit_void_when_empty) {
        return lm_trans_put(file, "void");
    }

    return 0;
}

static int lm_trans_emit_callable_descriptor(
    FILE *file,
    const LmTransFunctionHeader *function,
    const LmTransNamespace *namespace_
) {
    if (function == 0 || !function->is_descriptor_only) {
        return 1;
    }

    if (function->return_node == 0 || function->return_node->kind == LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: descriptor-only fn expects a single return type\n");
        return 1;
    }

    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "Closure ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "Closure;\ntypedef ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "Closure *") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\nstruct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, function->c_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "Closure {\n    ") != 0) {
        return 1;
    }
    if (lm_trans_emit_type_node(file, function->return_node) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " (*call)(void *env") != 0) {
        return 1;
    }
    if (lm_trans_params_has_any(function->params_node) && lm_trans_put(file, ", ") != 0) {
        return 1;
    }
    if (lm_trans_emit_callable_descriptor_params_body(file, function->params_node, namespace_, 0) != 0) {
        return 1;
    }
    return lm_trans_put(file, ");\n    void *env;\n    void (*destroy)(void *env);\n};\n");
}

static int lm_trans_top_level_emit_callable_descriptor(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    int status;
    LmP0Text descriptor_name;
    char *descriptor_name_storage;

    if (
        item == 0 ||
        item->frame == 0 ||
        !lm_trans_registry_identifier_value(item->function.name, &descriptor_name)
    ) {
        return 1;
    }

    descriptor_name_storage = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_name_storage == 0) {
        return 1;
    }

    if (lm_trans_registry_materialize_fn_descriptor_frame(item->frame) != 0) {
        free(descriptor_name_storage);
        return 1;
    }

    if (lm_trans_l4_is_function_pointer_type(namespace_, descriptor_name_storage)) {
        status = lm_trans_emit_l4_function_pointer_type_name(
            file,
            namespace_,
            descriptor_name_storage,
            "functionPointerType"
        );
    } else {
        status = lm_trans_emit_callable_descriptor(file, &item->function, namespace_);
    }
    free(descriptor_name_storage);
    if (status != 0) {
        return 1;
    }
    if (
        item->function.is_external &&
        lm_trans_validate_end_trailer(item->frame) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_function_prototype(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    int status;

    if (function == 0 || function->is_descriptor_only) {
        return 0;
    }

    if (function->is_sub) {
        if (!function->is_external && lm_trans_put(file, "static ") != 0) {
            return 1;
        }
        if (lm_trans_put(file, "void ") != 0) {
            return 1;
        }
    } else {
        if (function->return_node == 0) {
            return 0;
        }
        if (
            function->is_struct_return &&
            lm_trans_emit_function_return_structure_once(file, function->c_name, function->return_node) != 0
        ) {
            return 1;
        }
        if (!function->is_external && lm_trans_put(file, "static ") != 0) {
            return 1;
        }
        if (function->is_struct_return) {
            if (
                lm_trans_emit_function_return_struct_type_name(file, function->c_name) != 0 ||
                lm_trans_put(file, " ") != 0
            ) {
                return 1;
            }
        } else {
            if (function->return_node->kind == LM_P0_NODE_STRUCTURE) {
                return 0;
            }
            if (
                lm_trans_emit_type_node(file, function->return_node) != 0 ||
                lm_trans_put(file, " ") != 0
            ) {
                return 1;
            }
        }
    }

    if (
        lm_trans_write_text(file, function->c_name) != 0 ||
        lm_trans_put(file, "(") != 0
    ) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status = lm_trans_emit_function_params(file, function, namespace_);
    lm_trans_namespace_leave_scope(namespace_);
    if (status != 0) {
        return 1;
    }

    return lm_trans_put(file, ");\n");
}

static int lm_trans_top_level_emit_function_prototype(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0) {
        return 1;
    }
    return lm_trans_emit_function_prototype(file, &item->function, namespace_);
}

static int lm_trans_top_level_emit_function(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    int status;

    status = lm_trans_emit_function(file, &item->function, namespace_);
    if (status != 0) {
        return 1;
    }
    if (
        item->function.is_external &&
        lm_trans_validate_end_trailer(item->frame) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_lower_top_level_item(
    const LmP0Node *node,
    LmTransTopLevelItem *out
) {
    LmTransHeadBinding binding;
    LmTransL4HeadBinding l4_root_binding;
    int function_status;
    int statement_status;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    out->node = node;

    if (node->kind == LM_P0_NODE_ATOM) {
        function_status = lm_trans_top_level_atom_binding(node->as.atom, out);
        if (function_status < 0) {
            return 1;
        }
        if (function_status > 0) {
            return 0;
        }
        fprintf(stderr, "trans L2 error: top-level L2 atom field must be consumed by an atom receiver\n");
        return 1;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: top-level L2 field must be consumed by an L2 receiver\n");
        return 1;
    }

    out->frame = &node->as.frame;

    function_status = lm_trans_top_level_function_header(out->frame, &out->function);
    if (function_status < 0) {
        return 1;
    }
    if (function_status > 0) {
        out->declare = lm_trans_top_level_declare_function;
        if (out->function.is_descriptor_only) {
            out->emit_before_functions = lm_trans_top_level_emit_callable_descriptor;
            out->emits_top_level = 1;
        } else {
            out->emit_prototype = lm_trans_top_level_emit_function_prototype;
            out->emit_function = lm_trans_top_level_emit_function;
        }
        return 0;
    }

    if (lm_trans_head_binding_resolve(0, out->frame->head, &binding) != 0) {
        return 1;
    }
    statement_status = lm_trans_top_level_statement_binding(&binding, out);
    if (statement_status != 0) {
        return 0;
    }

    if (lm_trans_text_equals(out->frame->head, "L1")) {
        out->emit_after_prototypes = lm_trans_top_level_emit_l1;
        out->emits_top_level = 1;
        return 0;
    }

    if (lm_trans_l4_root_head_binding_resolve(out->frame->head, &l4_root_binding) != 0) {
        return 1;
    }
    if (l4_root_binding.frame != 0) {
        out->emit_before_functions = lm_trans_top_level_emit_registry;
        return 0;
    }

    fprintf(stderr, "trans L2 error: top-level L2 field must be fn, sub, external fn/sub, registered top-level statement frame, L1, or registered L4 root receiver data\n");
    return 1;
}

static int lm_trans_emit_function(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    const LmP0Frame *frame;
    LmTransFunctionState *state;
    LmOwnPtrStack hoisted_functions;
    LmOwnPtrStack capture_candidates;
    LmTransHoistedFunction *hoisted;
    size_t hoisted_index;
    int status;

    if (function == 0 || function->frame == 0) {
        return 1;
    }

    frame = function->frame;
    state = 0;

    if (function->is_descriptor_only) {
        fprintf(stderr, "trans L2 error: descriptor-only fn does not have an executable body\n");
        return 1;
    }

    lm_own_ptr_stack_init(&hoisted_functions, lm_trans_hoisted_function_delete_any);
    lm_own_ptr_stack_init(&capture_candidates, lm_trans_capture_delete_any);
    if (
        lm_trans_capture_candidates_add_captures(&capture_candidates, function->captures) != 0 ||
        lm_trans_capture_candidates_add_params(&capture_candidates, function->params_node, namespace_) != 0
    ) {
        lm_own_ptr_stack_destroy(&capture_candidates);
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }
    if (
        lm_trans_collect_hoisted_functions_from_fields(
            &hoisted_functions,
            function->c_name,
            &capture_candidates,
            namespace_,
            function->body_start
        ) != 0
    ) {
        lm_own_ptr_stack_destroy(&capture_candidates);
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }
    lm_own_ptr_stack_destroy(&capture_candidates);

    for (hoisted_index = 0U; hoisted_index < hoisted_functions.count; ++hoisted_index) {
        hoisted = (LmTransHoistedFunction *)lm_own_ptr_stack_at(&hoisted_functions, hoisted_index);
        if (hoisted == 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
        if (lm_trans_emit_function(file, &hoisted->function, namespace_) != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
        if (lm_trans_put(file, "\n") != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
        if (lm_trans_emit_closure_call_wrapper(file, hoisted, namespace_) != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
        if (lm_trans_put(file, "\n") != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
    }

    if (lm_trans_emit_env_type(file, function, namespace_) != 0) {
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }

    if (function->is_sub) {
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        }
        if (lm_trans_put(file, "void ") != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
    } else {
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_structure_once(file, function->c_name, function->return_node) != 0) {
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        } else {
            if (function->return_node->kind == LM_P0_NODE_STRUCTURE) {
                fprintf(stderr, "trans L2 error: fn expects a single-value return type; use fm for Structure return\n");
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        }
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        }
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_struct_type_name(file, function->c_name) != 0) {
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        } else {
            if (lm_trans_emit_type_node(file, function->return_node) != 0) {
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        }
        if (lm_trans_put(file, " ") != 0) {
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
    }

    if (lm_trans_write_text(file, function->c_name) != 0) {
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }
    lm_trans_namespace_enter_scope(namespace_);
    if (function->declare_self_alias) {
        if (
            lm_trans_namespace_declare_c_name(
                namespace_,
                function->name,
                function->symbol_class,
                function->c_name
            ) != 0 ||
            lm_trans_namespace_set_callable_shape(namespace_, function) != 0
        ) {
            lm_trans_namespace_leave_scope(namespace_);
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
        if (function->has_env) {
            if (
                lm_trans_namespace_set_env_arg(
                    namespace_,
                    function->name,
                    lm_trans_text_from_cstr("lm_env")
                ) != 0
            ) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
        }
    }
    if (function->has_env && function->captures != 0) {
        size_t capture_index;
        const LmTransCapture *capture;
        char *capture_expr;
        LmP0Text capture_text;

        for (capture_index = 0U; capture_index < function->captures->count; ++capture_index) {
            capture = (const LmTransCapture *)lm_own_ptr_stack_at(function->captures, capture_index);
            if (capture == 0) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
            capture_expr = lm_trans_captured_expr_new(capture->name);
            if (capture_expr == 0) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
            capture_text = lm_trans_text_from_cstr(capture_expr);
            if (
                lm_trans_namespace_declare_c_name(
                    namespace_,
                    capture->name,
                    "variable",
                    capture_text
                ) != 0
            ) {
                free(capture_expr);
                lm_trans_namespace_leave_scope(namespace_);
                lm_own_ptr_stack_destroy(&hoisted_functions);
                return 1;
            }
            free(capture_expr);
        }
    }
    if (lm_trans_emit_function_params(file, function, namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }

    state = lm_trans_function_state_new();
    if (state == 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_own_ptr_stack_destroy(&hoisted_functions);
        return 1;
    }

    if (function->is_struct_return) {
        state->current_return_type_name = lm_trans_text_ref_new(function->c_name);
        if (state->current_return_type_name == 0) {
            lm_trans_function_state_delete(state);
            lm_trans_namespace_leave_scope(namespace_);
            lm_own_ptr_stack_destroy(&hoisted_functions);
            return 1;
        }
    }

    state->previous_return_type_node = namespace_->return_type_node;
    state->previous_return_type_is_struct = namespace_->return_type_is_struct;
    state->previous_return_type_name = namespace_->return_type_name;
    state->previous_next_return_id = namespace_->next_return_id;
    state->previous_hoisted_functions = namespace_->hoisted_functions;
    state->previous_cleanups = namespace_->cleanups;
    state->previous_loops = namespace_->loops;
    state->has_previous_control_stacks = 1;
    namespace_->return_type_node = function->is_sub ? 0 : function->return_node;
    namespace_->return_type_is_struct = function->is_struct_return;
    namespace_->return_type_name = function->is_struct_return ? state->current_return_type_name : state->previous_return_type_name;
    namespace_->next_return_id = 0U;
    namespace_->hoisted_functions = &hoisted_functions;
    lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
    lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);

    status = lm_trans_emit_statement_list(file, function->body_start, 1U, namespace_);
    if (status == 0) {
        status = lm_trans_validate_end_trailer(frame);
    }
    if (status == 0) {
        status = lm_trans_emit_trailer_statement(file, frame->trailer, 1U, namespace_);
    }
    if (status == 0) {
        status = lm_trans_put(file, "}\n");
    }

    namespace_->return_type_node = state->previous_return_type_node;
    namespace_->return_type_is_struct = state->previous_return_type_is_struct;
    namespace_->return_type_name = state->previous_return_type_name;
    namespace_->next_return_id = state->previous_next_return_id;
    namespace_->hoisted_functions = state->previous_hoisted_functions;
    lm_own_ptr_stack_destroy(&namespace_->cleanups);
    lm_own_ptr_stack_destroy(&namespace_->loops);
    namespace_->cleanups = state->previous_cleanups;
    namespace_->loops = state->previous_loops;
    lm_own_ptr_stack_init(&state->previous_cleanups, 0);
    lm_own_ptr_stack_init(&state->previous_loops, 0);
    state->has_previous_control_stacks = 0;
    lm_trans_function_state_delete(state);
    lm_trans_namespace_leave_scope(namespace_);
    lm_own_ptr_stack_destroy(&hoisted_functions);
    return status;
}

static int lm_trans_is_end_target(const LmP0Frame *frame, const char *target);
static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1);
static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2);
static int lm_trans_path_has_extension(const char *path, const char *extension);
static int lm_trans_registry_load_file_path(
    const char *registry_path,
    int required,
    int *out_loaded
);
static int lm_trans_registry_candidate_path(
    const char *source_path,
    const char *candidate_name,
    char *registry_path,
    size_t registry_path_size
);
static int lm_trans_root_has_explicit_l2_frame(const LmP0Structure *root);
static int lm_trans_string_stack_has(const LmOwnPtrStack *stack, const char *value);
static int lm_trans_declare_l2_structure_import(
    LmTransNamespace *namespace_,
    const LmP0Structure *body
);
static int lm_trans_emit_l2_structure_prelude(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_structure_prototypes(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_structure_after_prototypes(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_structure_functions(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
);

static int lm_trans_emit_l2_structure_with_namespace(
    FILE *file,
    const LmP0Structure *body,
    const LmP0Frame *explicit_l2,
    LmTransNamespace *namespace_,
    int declare_items
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    FILE *prelude_file;

    if (namespace_ == 0) {
        return 1;
    }

    prelude_file = lm_trans_prelude_file(file);
    if (declare_items) {
        if (lm_trans_namespace_attach_registry(namespace_) != 0) {
            return 1;
        }

        field = body != 0 ? body->first_field : 0;
        while (field != 0) {
            node = field->value;
            if (lm_trans_lower_top_level_item(node, &item) != 0) {
                return 1;
            }

            if (item.declare != 0 && item.declare(namespace_, &item) != 0) {
                return 1;
            }

            field = field->next;
        }
    }

    if (
        lm_trans_emit_l2_structure_prelude(prelude_file, body, namespace_) != 0 ||
        lm_trans_emit_l2_structure_prototypes(prelude_file, body, namespace_) != 0 ||
        lm_trans_emit_l2_structure_after_prototypes(prelude_file, body, namespace_) != 0 ||
        lm_trans_emit_l2_structure_functions(file, body, namespace_) != 0
    ) {
        return 1;
    }

    return explicit_l2 != 0 ? lm_trans_validate_end_trailer(explicit_l2) : 0;
}

static int lm_trans_emit_l2_structure_prelude(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int status;
    int emitted_prelude;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prelude = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            return 1;
        }

        if (item.emit_before_functions != 0) {
            if (emitted_prelude && lm_trans_put(file, "\n") != 0) {
                return 1;
            }
            status = item.emit_before_functions(file, namespace_, &item);
        } else {
            status = 0;
        }

        if (status != 0) {
            return 1;
        }
        if (item.emits_top_level) {
            emitted_prelude = 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_structure_prototypes(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int status;
    int emitted_prototype;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prototype = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            return 1;
        }

        if (item.emit_prototype != 0) {
            status = item.emit_prototype(file, namespace_, &item);
            if (status != 0) {
                return 1;
            }
            emitted_prototype = 1;
        }
        field = field->next;
    }

    return emitted_prototype ? lm_trans_put(file, "\n") : 0;
}

static int lm_trans_emit_l2_structure_after_prototypes(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int status;
    int emitted_prelude;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prelude = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            return 1;
        }

        if (item.emit_after_prototypes != 0) {
            if (emitted_prelude && lm_trans_put(file, "\n") != 0) {
                return 1;
            }
            status = item.emit_after_prototypes(file, namespace_, &item);
        } else {
            status = 0;
        }

        if (status != 0) {
            return 1;
        }
        if (item.emits_top_level && item.emit_after_prototypes != 0) {
            emitted_prelude = 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_structure_functions(
    FILE *file,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int status;
    int emitted_function;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_function = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            return 1;
        }

        if (item.emit_function != 0) {
            if (emitted_function && lm_trans_put(file, "\n") != 0) {
                return 1;
            }
            status = item.emit_function(file, namespace_, &item);
            if (status != 0) {
                return 1;
            }
            emitted_function = 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_structure(FILE *file, const LmP0Structure *body, const LmP0Frame *explicit_l2) {
    LmTransNamespace *namespace_;
    int status;

    namespace_ = lm_trans_namespace_new();
    if (namespace_ == 0) {
        return 1;
    }

    status = lm_trans_emit_l2_structure_with_namespace(file, body, explicit_l2, namespace_, 1);
    lm_trans_namespace_delete(namespace_);
    return status;
}

static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2) {
    if (l2 == 0) {
        return 1;
    }

    return lm_trans_emit_l2_structure(file, &l2->body, l2);
}

static const char *lm_trans_import_source_path(void) {
    if (lm_trans_current_source_path != 0) {
        return lm_trans_current_source_path;
    }
    return lm_trans_registry.source_path;
}

static int lm_trans_import_stack_note(LmOwnPtrStack *stack, const char *path) {
    char *copy;

    if (stack == 0 || path == 0) {
        return 1;
    }
    if (lm_trans_string_stack_has(stack, path)) {
        return 0;
    }

    copy = lm_trans_text_copy_cstr(lm_trans_text_from_cstr(path));
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, copy) != 0) {
        free(copy);
        return 1;
    }
    return 0;
}

static int lm_trans_import_text_has_extension(LmP0Text path, const char *extension) {
    size_t extension_length;

    if (extension == 0) {
        return 0;
    }
    extension_length = strlen(extension);
    if (path.length < extension_length) {
        return 0;
    }
    return memcmp(path.data + path.length - extension_length, extension, extension_length) == 0;
}

static int lm_trans_import_frame_has_code_path(const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text path_value;

    field = frame != 0 ? frame->body.first_field : 0;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as.atom, &path_value)
            ) {
                return 1;
            }
            if (!lm_trans_import_text_has_extension(path_value, ".lm4")) {
                return 1;
            }
        }
        field = field->next;
    }
    return 0;
}

static int lm_trans_import_resolve_node(
    const LmP0Node *node,
    char *path,
    size_t path_size,
    const char *error_name
) {
    LmP0Text path_value;
    char *candidate_name;
    int status;

    if (
        node == 0 ||
        node->kind != LM_P0_NODE_ATOM ||
        !lm_trans_registry_literal_value(node->as.atom, &path_value)
    ) {
        fprintf(stderr, "trans L2 import error: import path must be an atom/string\n");
        return 1;
    }

    candidate_name = lm_trans_text_copy_cstr(path_value);
    if (candidate_name == 0) {
        return 1;
    }
    status = lm_trans_registry_candidate_path(
        lm_trans_import_source_path(),
        candidate_name,
        path,
        path_size
    );
    free(candidate_name);
    if (status != 0) {
        fprintf(
            stderr,
            "trans L2 import error: %s path is too long\n",
            error_name != 0 ? error_name : "import"
        );
        return 1;
    }
    return 0;
}

static int lm_trans_import_parse_document(
    const char *path,
    LmP0Document **out_document,
    const char *phase
) {
    const LmP0Diagnostic *diagnostic;
    int status;

    if (out_document == 0) {
        return 1;
    }

    *out_document = 0;
    status = lm_p0_parse_file(path, out_document);
    if (status == 0) {
        return 0;
    }

    diagnostic = lm_p0_document_diagnostic(*out_document);
    if (diagnostic != 0) {
        fprintf(
            stderr,
            "trans L2 import %s parse error %d at %lu:%lu in %s: %s\n",
            phase != 0 ? phase : "phase",
            diagnostic->code,
            (unsigned long)diagnostic->line,
            (unsigned long)diagnostic->column,
            path,
            diagnostic->message
        );
    } else {
        fprintf(stderr, "trans L2 import error: cannot read %s\n", path);
    }
    lm_p0_document_destroy(*out_document);
    *out_document = 0;
    return 1;
}

static int lm_trans_declare_l2_import_root(
    LmTransNamespace *namespace_,
    const LmP0Node *root,
    int implicit_l2
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        return lm_trans_declare_l2_structure_import(namespace_, &root->as.structure);
    }

    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                if (lm_trans_declare_l2_structure_import(namespace_, &node->as.frame.body) != 0) {
                    return 1;
                }
            } else {
                if (lm_trans_lower_top_level_item(node, &item) != 0) {
                    return 1;
                }
                if (item.declare != 0 && item.declare(namespace_, &item) != 0) {
                    return 1;
                }
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_import_root_prelude(
    FILE *file,
    const LmP0Node *root,
    int implicit_l2,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        return lm_trans_emit_l2_structure_prelude(file, &root->as.structure, namespace_);
    }

    emitted = 0;
    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                status = lm_trans_emit_l2_structure_prelude(file, &node->as.frame.body, namespace_);
            } else {
                if (lm_trans_lower_top_level_item(node, &item) != 0) {
                    return 1;
                }
                if (item.emit_before_functions != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        return 1;
                    }
                    status = item.emit_before_functions(file, namespace_, &item);
                } else {
                    status = 0;
                }
                if (item.emits_top_level) {
                    emitted = 1;
                }
            }
            if (status != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_import_root_prototypes(
    FILE *file,
    const LmP0Node *root,
    int implicit_l2,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        return lm_trans_emit_l2_structure_prototypes(file, &root->as.structure, namespace_);
    }

    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                status = lm_trans_emit_l2_structure_prototypes(file, &node->as.frame.body, namespace_);
            } else {
                if (lm_trans_lower_top_level_item(node, &item) != 0) {
                    return 1;
                }
                status = item.emit_prototype != 0
                    ? item.emit_prototype(file, namespace_, &item)
                    : 0;
            }
            if (status != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_import_root_after_prototypes(
    FILE *file,
    const LmP0Node *root,
    int implicit_l2,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        return lm_trans_emit_l2_structure_after_prototypes(file, &root->as.structure, namespace_);
    }

    emitted = 0;
    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                status = lm_trans_emit_l2_structure_after_prototypes(file, &node->as.frame.body, namespace_);
            } else {
                if (lm_trans_lower_top_level_item(node, &item) != 0) {
                    return 1;
                }
                if (item.emit_after_prototypes != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        return 1;
                    }
                    status = item.emit_after_prototypes(file, namespace_, &item);
                    emitted = 1;
                } else {
                    status = 0;
                }
            }
            if (status != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l2_import_root_functions(
    FILE *file,
    const LmP0Node *root,
    int implicit_l2,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        return lm_trans_emit_l2_structure_functions(file, &root->as.structure, namespace_);
    }

    emitted = 0;
    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                if (emitted && lm_trans_put(file, "\n") != 0) {
                    return 1;
                }
                status = lm_trans_emit_l2_structure_functions(file, &node->as.frame.body, namespace_);
                if (status == 0) {
                    status = lm_trans_validate_end_trailer(&node->as.frame);
                }
                emitted = 1;
            } else {
                if (lm_trans_lower_top_level_item(node, &item) != 0) {
                    return 1;
                }
                if (item.emit_function != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        return 1;
                    }
                    status = item.emit_function(file, namespace_, &item);
                    emitted = 1;
                } else {
                    status = 0;
                }
            }
            if (status != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_declare_l2_import_path(
    LmTransNamespace *namespace_,
    const char *path
) {
    LmP0Document *document;
    const char *previous_source_path;
    int loaded;
    int status;

    if (path == 0) {
        return 1;
    }
    if (lm_trans_path_has_extension(path, ".lm4")) {
        return lm_trans_registry_load_file_path(path, 1, &loaded);
    }
    if (lm_trans_string_stack_has(&lm_trans_declared_import_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(&lm_trans_declared_import_paths, path) != 0) {
        return 1;
    }
    if (lm_trans_registry_load_file_path(path, 1, &loaded) != 0) {
        return 1;
    }

    document = 0;
    if (lm_trans_import_parse_document(path, &document, "declare") != 0) {
        return 1;
    }

    previous_source_path = lm_trans_current_source_path;
    lm_trans_current_source_path = path;
    status = lm_trans_declare_l2_import_root(
        namespace_,
        lm_p0_document_root(document),
        lm_trans_path_has_extension(path, ".lm2")
    );
    lm_trans_current_source_path = previous_source_path;
    if (status != 0) {
        lm_p0_document_destroy(document);
        return status;
    }
    if (lm_own_ptr_stack_push(&lm_trans_declared_import_documents, document) != 0) {
        lm_p0_document_destroy(document);
        return 1;
    }
    return 0;
}

static int lm_trans_emit_l2_import_path_prelude(
    FILE *file,
    LmTransNamespace *namespace_,
    const char *path
) {
    LmP0Document *document;
    const char *previous_source_path;
    int loaded;
    int status;

    if (path == 0) {
        return 1;
    }
    if (lm_trans_path_has_extension(path, ".lm4")) {
        return lm_trans_registry_load_file_path(path, 1, &loaded);
    }
    if (lm_trans_string_stack_has(&lm_trans_emitted_import_prelude_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(&lm_trans_emitted_import_prelude_paths, path) != 0) {
        return 1;
    }
    if (lm_trans_registry_load_file_path(path, 1, &loaded) != 0) {
        return 1;
    }

    document = 0;
    if (lm_trans_import_parse_document(path, &document, "prelude") != 0) {
        return 1;
    }

    previous_source_path = lm_trans_current_source_path;
    lm_trans_current_source_path = path;
    status = lm_trans_emit_l2_import_root_prelude(
        file,
        lm_p0_document_root(document),
        lm_trans_path_has_extension(path, ".lm2"),
        namespace_
    );
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_prototypes(
            file,
            lm_p0_document_root(document),
            lm_trans_path_has_extension(path, ".lm2"),
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_after_prototypes(
            file,
            lm_p0_document_root(document),
            lm_trans_path_has_extension(path, ".lm2"),
            namespace_
        );
    }
    lm_trans_current_source_path = previous_source_path;
    lm_p0_document_destroy(document);
    return status;
}

static int lm_trans_emit_l2_import_path_functions(
    FILE *file,
    LmTransNamespace *namespace_,
    const char *path
) {
    LmP0Document *document;
    const char *previous_source_path;
    int loaded;
    int status;

    if (path == 0) {
        return 1;
    }
    if (lm_trans_path_has_extension(path, ".lm4")) {
        return lm_trans_registry_load_file_path(path, 1, &loaded);
    }
    if (lm_trans_string_stack_has(&lm_trans_emitted_import_function_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(&lm_trans_emitted_import_function_paths, path) != 0) {
        return 1;
    }
    if (lm_trans_registry_load_file_path(path, 1, &loaded) != 0) {
        return 1;
    }

    document = 0;
    if (lm_trans_import_parse_document(path, &document, "function") != 0) {
        return 1;
    }

    previous_source_path = lm_trans_current_source_path;
    lm_trans_current_source_path = path;
    status = lm_trans_emit_l2_import_root_functions(
        file,
        lm_p0_document_root(document),
        lm_trans_path_has_extension(path, ".lm2"),
        namespace_
    );
    lm_trans_current_source_path = previous_source_path;
    lm_p0_document_destroy(document);
    return status;
}

static int lm_trans_declare_l2_import_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    const LmP0Field *field;
    const LmP0Node *node;
    char path[4096];
    int declared;

    if (frame == 0) {
        return 1;
    }

    declared = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_import_resolve_node(node, path, sizeof(path), "import") != 0) {
                return 1;
            }
            if (lm_trans_declare_l2_import_path(namespace_, path) != 0) {
                return 1;
            }
            declared = 1;
        }
        field = field->next;
    }

    if (!declared) {
        fprintf(stderr, "trans L2 import error: import receiver expects at least one path\n");
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l2_import_prelude(
    FILE *file,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    char path[4096];
    int emitted;

    if (frame == 0) {
        return 1;
    }

    emitted = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_import_resolve_node(node, path, sizeof(path), "import") != 0) {
                return 1;
            }
            if (lm_trans_emit_l2_import_path_prelude(file, namespace_, path) != 0) {
                return 1;
            }
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 import error: import receiver expects at least one path\n");
        return 1;
    }
    return 0;
}

static int lm_trans_emit_l2_import_functions(
    FILE *file,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    char path[4096];
    int emitted;

    if (frame == 0) {
        return 1;
    }

    emitted = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_import_resolve_node(node, path, sizeof(path), "import") != 0) {
                return 1;
            }
            if (lm_trans_emit_l2_import_path_functions(file, namespace_, path) != 0) {
                return 1;
            }
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 import error: import receiver expects at least one path\n");
        return 1;
    }
    return 0;
}

static int lm_trans_inline_string_payload(LmP0Text text, LmP0Text *out_payload, size_t *out_run) {
    char quote;
    size_t open_run;
    size_t close_run;

    if (out_payload == 0 || out_run == 0 || text.length < 2U) {
        return 0;
    }

    quote = text.data[0];
    if (quote != '\'' && quote != '"') {
        return 0;
    }

    open_run = 0U;
    while (open_run < text.length && text.data[open_run] == quote) {
        ++open_run;
    }
    if (text.length < open_run * 2U) {
        return 0;
    }

    close_run = 0U;
    while (close_run < text.length && text.data[text.length - close_run - 1U] == quote) {
        ++close_run;
    }
    if (close_run < open_run) {
        return 0;
    }

    out_payload->data = text.data + open_run;
    out_payload->length = text.length - open_run * 2U;
    *out_run = open_run;
    return 1;
}

static int lm_trans_emit_l1_payload(FILE *output, LmP0Text text) {
    LmP0Text payload;
    size_t delimiter_run;
    size_t i;
    char ch;

    if (lm_trans_inline_string_payload(text, &payload, &delimiter_run)) {
        if (delimiter_run == 1U) {
            i = 0U;
            while (i < payload.length) {
                ch = payload.data[i++];
                if (ch == '\\' && i < payload.length) {
                    ch = payload.data[i++];
                    if (ch == 'n') {
                        ch = '\n';
                    } else if (ch == 'r') {
                        ch = '\r';
                    } else if (ch == 't') {
                        ch = '\t';
                    } else if (ch == '0') {
                        ch = '\0';
                    }
                }
                if (lm_trans_write_all(output, &ch, 1U) != 0) {
                    return 1;
                }
            }
            return 0;
        }

        return lm_trans_write_all(output, payload.data, payload.length);
    }

    return lm_trans_write_text(output, text);
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node);

static int lm_trans_emit_l1_structure(FILE *output, const LmP0Structure *structure) {
    const LmP0Field *field;

    if (structure == 0) {
        return 0;
    }

    field = structure->first_field;
    while (field != 0) {
        if (lm_trans_emit_l1_node(output, field->value) != 0) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static const char *lm_trans_os_condition(LmP0Text name) {
    if (lm_trans_text_equals(name, "win")) {
        return "defined(_WIN32)";
    }
    if (lm_trans_text_equals(name, "mac")) {
        return "defined(__APPLE__)";
    }
    if (lm_trans_text_equals(name, "unix")) {
        return "defined(__unix__)";
    }
    return 0;
}

static int lm_trans_ifdef_condition_payload(
    LmP0Text name,
    LmP0Text *out_condition
) {
    if (
        out_condition == 0 ||
        lm_trans_identifier_payload(name, out_condition) == 0 ||
        out_condition->length == 0U
    ) {
        fprintf(stderr, "trans preprocessor error: ifdef branch expects a non-empty condition\n");
        return 1;
    }
    return 0;
}

static int lm_trans_os_branch_looks_l2(const LmP0Structure *body) {
    const LmP0Field *field;
    const LmP0Node *node;

    if (body == 0) {
        return 0;
    }

    field = body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind == LM_P0_NODE_FRAME) {
                if (
                    !lm_trans_text_equals(node->as.frame.head, "L1") &&
                    !lm_trans_text_equals(node->as.frame.head, "include") &&
                    !lm_trans_text_equals(node->as.frame.head, "os") &&
                    !lm_trans_text_equals(node->as.frame.head, "ifdef") &&
                    !lm_trans_is_end_target(&node->as.frame, "L1")
                ) {
                    return 1;
                }
            } else if (node->kind == LM_P0_NODE_STRUCTURE) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_declare_l2_structure_import(
    LmTransNamespace *namespace_,
    const LmP0Structure *body
);

static int lm_trans_declare_l2_os_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    const char *condition;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: os receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
            } else {
                condition = lm_trans_os_condition(branch->head);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: unknown os branch\n");
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: os default branch must be last\n");
                    return 1;
                }
            }

            if (lm_trans_os_branch_looks_l2(&branch->body)) {
                if (lm_trans_declare_l2_structure_import(namespace_, &branch->body) != 0) {
                    return 1;
                }
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_declare_l2_ifdef_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    LmP0Text condition;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: ifdef receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
            } else {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: ifdef default branch must be last\n");
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, &condition) != 0) {
                    return 1;
                }
            }

            if (lm_trans_os_branch_looks_l2(&branch->body)) {
                if (lm_trans_declare_l2_structure_import(namespace_, &branch->body) != 0) {
                    return 1;
                }
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_top_level_declare_ifdef(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_declare_l2_ifdef_frame(namespace_, item != 0 ? item->frame : 0);
}

static int lm_trans_declare_l2_structure_import(
    LmTransNamespace *namespace_,
    const LmP0Structure *body
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransTopLevelItem item;

    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            return 1;
        }

        if (item.function.frame != 0) {
            if (lm_trans_top_level_declare_function_compatible(namespace_, &item) != 0) {
                return 1;
            }
        } else if (item.declare != 0) {
            if (item.declare(namespace_, &item) != 0) {
                return 1;
            }
        }

        field = field->next;
    }

    return 0;
}

static int lm_trans_top_level_declare_os(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_declare_l2_os_frame(namespace_, item != 0 ? item->frame : 0);
}

static int lm_trans_emit_l2_os_branch(
    FILE *output,
    const LmP0Frame *branch,
    LmTransNamespace *namespace_
) {
    if (lm_trans_os_branch_looks_l2(&branch->body)) {
        return lm_trans_emit_l2_structure_with_namespace(output, &branch->body, 0, namespace_, 0);
    }
    return lm_trans_emit_l1_structure(output, &branch->body);
}

static int lm_trans_emit_l2_os_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    const char *condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: os receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
                if (opened) {
                    if (lm_trans_put(output, "#else\n") != 0) {
                        return 1;
                    }
                }
            } else {
                condition = lm_trans_os_condition(branch->head);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: unknown os branch\n");
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: os default branch must be last\n");
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    return 1;
                }
                if (lm_trans_put(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l2_os_branch(output, branch, namespace_) != 0) {
                return 1;
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l2_ifdef_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    LmP0Text condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: ifdef receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
                if (opened) {
                    if (lm_trans_put(output, "#else\n") != 0) {
                        return 1;
                    }
                }
            } else {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: ifdef default branch must be last\n");
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, &condition) != 0) {
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    return 1;
                }
                if (lm_trans_write_text(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l2_os_branch(output, branch, namespace_) != 0) {
                return 1;
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(frame);
}

static const LmP0Field *lm_trans_first_active_field(
    const LmP0Structure *body
) {
    const LmP0Field *field;

    field = body != 0 ? body->first_field : 0;
    while (field != 0 && lm_trans_node_is_ignored(field->value)) {
        field = field->next;
    }
    return field;
}

static int lm_trans_emit_l2_wrapper_body(
    FILE *output,
    const LmP0Structure *body,
    LmTransNamespace *namespace_
) {
    if (lm_trans_os_branch_looks_l2(body)) {
        return lm_trans_emit_l2_structure_with_namespace(output, body, 0, namespace_, 0);
    }
    return lm_trans_emit_l1_structure(output, body);
}

static int lm_trans_declare_l2_wrapper_body(
    LmTransNamespace *namespace_,
    const LmP0Structure *body
) {
    if (lm_trans_os_branch_looks_l2(body)) {
        return lm_trans_declare_l2_structure_import(namespace_, body);
    }
    return 0;
}

static int lm_trans_guard_body_from_frame(
    const LmP0Frame *frame,
    LmP0Text *out_name,
    LmP0Structure *out_body
) {
    const LmP0Field *name_field;
    const LmP0Field *body_field;
    const LmP0Field *extra_field;

    if (frame == 0 || out_name == 0 || out_body == 0) {
        return 1;
    }

    name_field = lm_trans_first_active_field(&frame->body);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: guard receiver expects guard name as first atom\n");
        return 1;
    }

    *out_name = name_field->value->as.atom;
    *out_body = frame->body;
    out_body->first_field = name_field->next;
    body_field = lm_trans_first_active_field(out_body);
    if (
        body_field != 0 &&
        body_field->value != 0 &&
        body_field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        extra_field = body_field->next;
        while (extra_field != 0 && lm_trans_node_is_ignored(extra_field->value)) {
            extra_field = extra_field->next;
        }
        if (extra_field == 0) {
            *out_body = body_field->value->as.structure;
        }
    }
    return 0;
}

static int lm_trans_declare_l2_guard_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    LmP0Text name;
    LmP0Structure body;

    if (lm_trans_guard_body_from_frame(frame, &name, &body) != 0) {
        return 1;
    }
    (void)name;
    if (lm_trans_declare_l2_wrapper_body(namespace_, &body) != 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l2_guard_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    LmP0Text name;
    LmP0Structure body;

    if (lm_trans_guard_body_from_frame(frame, &name, &body) != 0) {
        return 1;
    }
    if (
        lm_trans_put(output, "#ifndef ") != 0 ||
        lm_trans_emit_identifier(output, name) != 0 ||
        lm_trans_put(output, "\n#define ") != 0 ||
        lm_trans_emit_identifier(output, name) != 0 ||
        lm_trans_put(output, "\n") != 0
    ) {
        return 1;
    }
    if (lm_trans_emit_l2_wrapper_body(output, &body, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_declare_l2_extern_c_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_declare_l2_wrapper_body(namespace_, &frame->body) != 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l2_extern_c_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_put(output, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_l2_wrapper_body(output, &frame->body, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(output, "#ifdef __cplusplus\n}\n#endif\n") != 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_declare_l2_registry_os_node(
    LmTransNamespace *namespace_,
    const LmP0Node *node
) {
    if (node == 0) {
        return 0;
    }
    if (node->kind == LM_P0_NODE_FRAME && lm_trans_text_equals(node->as.frame.head, "L2")) {
        if (lm_trans_declare_l2_structure_import(namespace_, &node->as.frame.body) != 0) {
            return 1;
        }
        return lm_trans_validate_end_trailer(&node->as.frame);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_os_branch_looks_l2(&node->as.structure)) {
        return lm_trans_declare_l2_structure_import(namespace_, &node->as.structure);
    }
    return 0;
}

static int lm_trans_declare_l2_registry_os_table(LmTransNamespace *namespace_) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    LmP0Text branch_name;
    const char *condition;
    int emitted_default;
    int emitted;

    emitted_default = 0;
    emitted = 0;
    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("os"),
        "row"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload_node != 0
        ) {
            branch_name = lm_trans_text_from_cstr(row->key);
            if (lm_trans_text_equals(branch_name, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os table has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
            } else {
                condition = lm_trans_os_condition(branch_name);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: registry os table has unknown branch %s\n", row->key);
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os default branch must be last\n");
                    return 1;
                }
            }
            if (lm_trans_declare_l2_registry_os_node(namespace_, row->payload_node) != 0) {
                return 1;
            }
            emitted = 1;
        }
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        return 1;
    }
    return 0;
}

static int lm_trans_emit_l2_registry_os_node(
    FILE *output,
    const LmP0Node *node,
    LmTransNamespace *namespace_
) {
    if (node == 0) {
        return 0;
    }
    if (node->kind == LM_P0_NODE_FRAME && lm_trans_text_equals(node->as.frame.head, "L2")) {
        return lm_trans_emit_l2_structure_with_namespace(output, &node->as.frame.body, &node->as.frame, namespace_, 0);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_os_branch_looks_l2(&node->as.structure)) {
        return lm_trans_emit_l2_structure_with_namespace(output, &node->as.structure, 0, namespace_, 0);
    }
    return lm_trans_emit_l1_node(output, node);
}

static int lm_trans_l4_payload_structure_looks_bare_l2(const LmP0Structure *body) {
    const LmP0Field *field;
    const LmP0Node *node;
    int found;

    if (body == 0) {
        return 0;
    }

    found = 0;
    field = body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind == LM_P0_NODE_ATOM) {
                return 0;
            }
            if (node->kind == LM_P0_NODE_FRAME) {
                if (
                    lm_trans_text_equals(node->as.frame.head, "L1") ||
                    lm_trans_text_equals(node->as.frame.head, "L2") ||
                    lm_trans_text_equals(node->as.frame.head, "include") ||
                    lm_trans_text_equals(node->as.frame.head, "os") ||
                    lm_trans_text_equals(node->as.frame.head, "ifdef") ||
                    lm_trans_is_end_target(&node->as.frame, "L1")
                ) {
                    return 0;
                }
                found = 1;
            } else if (node->kind == LM_P0_NODE_STRUCTURE) {
                found = 1;
            } else {
                return 0;
            }
        }
        field = field->next;
    }

    return found;
}

static int lm_trans_emit_l4_payload_node(
    FILE *output,
    const LmP0Node *node,
    LmTransNamespace *namespace_
);
static int lm_trans_registry_candidate_path(
    const char *source_path,
    const char *candidate_name,
    char *registry_path,
    size_t registry_path_size
);

static int lm_trans_emit_l4_payload_import_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Diagnostic *diagnostic;
    LmP0Document *document;
    LmP0Text path_value;
    char *candidate_name;
    char payload_path[4096];
    int emitted;
    int status;

    if (frame == 0) {
        return 1;
    }

    field = frame->body.first_field;
    emitted = 0;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node == 0 ||
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as.atom, &path_value)
            ) {
                fprintf(stderr, "trans L4 unit error: unit.payload import path must be an atom/string\n");
                return 1;
            }

            candidate_name = lm_trans_text_copy_cstr(path_value);
            if (candidate_name == 0) {
                return 1;
            }
            if (
                lm_trans_registry_candidate_path(
                    lm_trans_registry.source_path,
                    candidate_name,
                    payload_path,
                    sizeof(payload_path)
                ) != 0
            ) {
                free(candidate_name);
                fprintf(stderr, "trans L4 unit error: unit.payload import path is too long\n");
                return 1;
            }
            free(candidate_name);

            document = 0;
            status = lm_p0_parse_file(payload_path, &document);
            if (status != 0) {
                diagnostic = lm_p0_document_diagnostic(document);
                if (diagnostic != 0) {
                    fprintf(
                        stderr,
                        "trans L4 unit import parse error %d at %lu:%lu in %s: %s\n",
                        diagnostic->code,
                        (unsigned long)diagnostic->line,
                        (unsigned long)diagnostic->column,
                        payload_path,
                        diagnostic->message
                    );
                } else {
                    fprintf(stderr, "trans L4 unit error: cannot read unit.payload import %s\n", payload_path);
                }
                lm_p0_document_destroy(document);
                return 1;
            }

            if (lm_trans_emit_l4_payload_node(output, lm_p0_document_root(document), namespace_) != 0) {
                lm_p0_document_destroy(document);
                return 1;
            }
            lm_p0_document_destroy(document);
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L4 unit error: unit.payload import receiver expects at least one path\n");
        return 1;
    }
    return 0;
}

static int lm_trans_l4_payload_receiver_import(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_l4_payload_import_frame(output, frame, namespace_);
}

static int lm_trans_l4_payload_receiver_l2(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    return frame != 0
        ? lm_trans_emit_l2_structure_with_namespace(output, &frame->body, frame, namespace_, 0)
        : 1;
}

static int lm_trans_l4_payload_frame_handler_resolve(
    LmP0Text head,
    LmTransL4PayloadFrameHandler *out
) {
    LmTransBinding resolved;
    const char *receiver_type;
    const char *receiver_binding;

    if (out == 0) {
        return 1;
    }
    *out = 0;

    receiver_type = lm_trans_registry_lookup_table_link_checked(
        head,
        "namespace.l4.payload",
        "receiver.type"
    );
    if (receiver_type == 0) {
        return 0;
    }

    receiver_binding = lm_trans_registry_lookup(head, "receiver.l4.payload");
    if (receiver_binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: namespace.l4.payload[\"%.*s\"] has no receiver.l4.payload binding\n",
            (int)head.length,
            head.data
        );
        return 1;
    }
    if (
        !lm_trans_binding_resolve(receiver_binding, &resolved) ||
        resolved.l4_payload_frame == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: receiver.l4.payload[\"%.*s\"] has unknown payload binding %s\n",
            (int)head.length,
            head.data,
            receiver_binding
        );
        return 1;
    }

    *out = resolved.l4_payload_frame;
    return 0;
}

static int lm_trans_emit_l4_payload_node(
    FILE *output,
    const LmP0Node *node,
    LmTransNamespace *namespace_
) {
    LmTransL4PayloadFrameHandler frame_handler;

    if (node == 0) {
        return 0;
    }
    if (node->kind == LM_P0_NODE_FRAME) {
        if (lm_trans_l4_payload_frame_handler_resolve(node->as.frame.head, &frame_handler) != 0) {
            return 1;
        }
        if (frame_handler != 0) {
            return frame_handler(output, &node->as.frame, namespace_);
        }
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_l4_payload_structure_looks_bare_l2(&node->as.structure)) {
        return lm_trans_emit_l2_structure_with_namespace(output, &node->as.structure, 0, namespace_, 0);
    }
    return lm_trans_emit_l1_node(output, node);
}

static int lm_trans_emit_l2_registry_os_table(
    FILE *output,
    LmTransNamespace *namespace_
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    LmP0Text branch_name;
    const char *condition;
    int opened;
    int emitted_default;
    int emitted;

    opened = 0;
    emitted_default = 0;
    emitted = 0;
    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("os"),
        "row"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload_node != 0
        ) {
            branch_name = lm_trans_text_from_cstr(row->key);
            if (lm_trans_text_equals(branch_name, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os table has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
                if (opened && lm_trans_put(output, "#else\n") != 0) {
                    return 1;
                }
            } else {
                condition = lm_trans_os_condition(branch_name);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: registry os table has unknown branch %s\n", row->key);
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os default branch must be last\n");
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    return 1;
                }
                if (lm_trans_put(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l2_registry_os_node(output, row->payload_node, namespace_) != 0) {
                return 1;
            }
            emitted = 1;
        }
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        return 1;
    }
    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_l1_os_frame(FILE *output, const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    const char *condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L1 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L1 error: os receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
                if (opened) {
                    if (lm_trans_put(output, "#else\n") != 0) {
                        return 1;
                    }
                }
            } else {
                condition = lm_trans_os_condition(branch->head);
                if (condition == 0) {
                    fprintf(stderr, "trans L1 error: unknown os branch\n");
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L1 error: os default branch must be last\n");
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    return 1;
                }
                if (lm_trans_put(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l1_structure(output, &branch->body) != 0) {
                return 1;
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l1_ifdef_frame(FILE *output, const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *branch;
    LmP0Text condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L1 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = &node->as.frame;
            if (lm_trans_text_equals(branch->head, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L1 error: ifdef receiver has duplicate default branch\n");
                    return 1;
                }
                emitted_default = 1;
                if (opened) {
                    if (lm_trans_put(output, "#else\n") != 0) {
                        return 1;
                    }
                }
            } else {
                if (emitted_default) {
                    fprintf(stderr, "trans L1 error: ifdef default branch must be last\n");
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, &condition) != 0) {
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    return 1;
                }
                if (lm_trans_write_text(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l1_structure(output, &branch->body) != 0) {
                return 1;
            }
            if (lm_trans_validate_end_trailer(branch) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l1_include_target_text(
    FILE *output,
    const char *data,
    size_t length
) {
    int is_direct_target;

    if (output == 0 || data == 0 || length == 0U) {
        return 1;
    }

    is_direct_target =
        length >= 2U &&
        (
            (data[0] == '<' && data[length - 1U] == '>') ||
            (data[0] == '"' && data[length - 1U] == '"')
        );

    if (lm_trans_put(output, "#include ") != 0) {
        return 1;
    }
    if (is_direct_target) {
        if (lm_trans_write_all(output, data, length) != 0) {
            return 1;
        }
    } else {
        if (
            lm_trans_put(output, "\"") != 0 ||
            lm_trans_write_all(output, data, length) != 0 ||
            lm_trans_put(output, "\"") != 0
        ) {
            return 1;
        }
    }
    return lm_trans_put(output, "\n");
}

static int lm_trans_emit_registry_include_table(
    FILE *output,
    const LmTransNamespace *namespace_
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    int emitted;

    emitted = 0;
    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("include"),
        "row"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L1 error: atom include receiver expects table include\n");
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->payload != 0
        ) {
            if (lm_trans_emit_l1_include_target_text(output, row->payload, strlen(row->payload)) != 0) {
                return 1;
            }
            emitted = 1;
        }
    }

    return emitted ? 0 : 1;
}

static int lm_trans_emit_l1_include_frame(FILE *output, const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text payload;
    size_t delimiter_run;
    int emitted;

    if (frame == 0) {
        return 1;
    }

    emitted = 0;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_inline_string_payload(node->as.atom, &payload, &delimiter_run) ||
                payload.length == 0U
            ) {
                fprintf(stderr, "trans L1 error: include receiver expects string header atoms\n");
                return 1;
            }
            (void)delimiter_run;
            if (lm_trans_emit_l1_include_target_text(output, payload.data, payload.length) != 0) {
                return 1;
            }
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L1 error: include receiver expects at least one header\n");
        return 1;
    }

    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node) {
    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        if (lm_trans_emit_l1_payload(output, node->as.atom) != 0) {
            return 1;
        }
        return lm_trans_write_all(output, "\n", 1U);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_emit_l1_structure(output, &node->as.structure);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if (lm_trans_text_equals(node->as.frame.head, "L1")) {
            return lm_trans_emit_l1_frame(output, &node->as.frame);
        }
        if (lm_trans_text_equals(node->as.frame.head, "L2")) {
            return lm_trans_emit_l2_frame(output, &node->as.frame);
        }
        if (lm_trans_text_equals(node->as.frame.head, "os")) {
            return lm_trans_emit_l1_os_frame(output, &node->as.frame);
        }
        if (lm_trans_text_equals(node->as.frame.head, "ifdef")) {
            return lm_trans_emit_l1_ifdef_frame(output, &node->as.frame);
        }
        if (lm_trans_text_equals(node->as.frame.head, "include")) {
            return lm_trans_emit_l1_include_frame(output, &node->as.frame);
        }
        if (lm_trans_is_end_target(&node->as.frame, "L1")) {
            return 0;
        }

        fprintf(stderr, "trans error: unknown translator receiver inside L1\n");
        return 1;
    }

    return 0;
}

static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1) {
    if (l1 == 0) {
        return 1;
    }

    if (lm_trans_emit_l1_structure(output, &l1->body) != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(l1);
}

static int lm_trans_is_end_target(const LmP0Frame *frame, const char *target) {
    const LmP0Field *field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "end")) {
        return 0;
    }

    field = frame->body.first_field;
    return
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as.atom, target);
}

static int lm_trans_emit_l1_body(FILE *output, const LmP0Frame *l1, int *emitted) {
    if (lm_trans_emit_l1_frame(output, l1) != 0) {
        return 1;
    }

    if (emitted != 0) {
        *emitted = 1;
    }
    return 0;
}

static int lm_trans_root_has_explicit_l2_frame(const LmP0Structure *root) {
    const LmP0Field *field;
    const LmP0Node *node;

    if (root == 0) {
        return 0;
    }

    field = root->first_field;
    while (field != 0) {
        node = field->value;
        if (
            node != 0 &&
            node->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_equals(node->as.frame.head, "L2")
        ) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_root_sequence(FILE *output, const LmP0Node *root, int implicit_l2, int *emitted) {
    const LmP0Field *field;
    const LmP0Node *node;
    FILE *prelude_file;
    LmTransL4HeadBinding l4_root_binding;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }
    prelude_file = lm_trans_prelude_file(output);

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(&root->as.structure)) {
        if (lm_trans_emit_l2_structure(output, &root->as.structure, 0) != 0) {
            return 1;
        }
        if (emitted != 0) {
            *emitted = 1;
        }
        return 0;
    }

    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind == LM_P0_NODE_ATOM) {
                fprintf(stderr, "trans error: root raw text must be inside L1\n");
                return 1;
            }
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans error: root field must be L1, L2, registered L4 root receiver, os, ifdef, include, raw L1 text, or end: L1\n");
                return 1;
            }
            if (lm_trans_l4_root_head_binding_resolve(node->as.frame.head, &l4_root_binding) != 0) {
                return 1;
            }
            if (lm_trans_text_equals(node->as.frame.head, "L1")) {
                if (lm_trans_emit_l1_body(prelude_file, &node->as.frame, emitted) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "L2")) {
                if (lm_trans_emit_l2_frame(output, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (l4_root_binding.frame != 0) {
                (void)l4_root_binding;
            } else if (lm_trans_text_equals(node->as.frame.head, "os")) {
                if (lm_trans_emit_l1_os_frame(prelude_file, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "ifdef")) {
                if (lm_trans_emit_l1_ifdef_frame(prelude_file, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "include")) {
                if (lm_trans_emit_l1_include_frame(prelude_file, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_is_end_target(&node->as.frame, "L1")) {
                *emitted = 1;
            } else {
                fprintf(stderr, "trans error: root field must be L1, L2, registered L4 root receiver, os, ifdef, include, raw L1 text, or end: L1\n");
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_string_stack_has(const LmOwnPtrStack *stack, const char *value) {
    size_t i;
    const char *item;

    if (stack == 0 || value == 0) {
        return 0;
    }

    for (i = 0U; i < stack->count; ++i) {
        item = (const char *)lm_own_ptr_stack_at(stack, i);
        if (item != 0 && strcmp(item, value) == 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_layout_backend_is_supported(const char *backend) {
    return
        backend != 0 &&
        (
            strcmp(backend, "c.struct") == 0 ||
            strcmp(backend, "c.named-struct") == 0 ||
            strcmp(backend, "c.union") == 0 ||
            strcmp(backend, "c.named-union") == 0
        );
}

static int lm_trans_registry_collect_layout_names(
    LmOwnPtrStack *names,
    const LmTransNamespace *namespace_
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    char *name;
    LmP0Text key_text;

    if (names == 0) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("layout"),
        "backend"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L4 layout error: layout receiver expects layout.backend facts\n");
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload != 0 &&
            lm_trans_layout_backend_is_supported(row->payload)
        ) {
            key_text = lm_trans_text_from_cstr(row->key);
            if (lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0) {
                fprintf(stderr, "trans L4 layout error: layout.backend key %s is not a class\n", row->key);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                free(name);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                free(name);
                return 1;
            }
        }
    }
    return 0;
}

static int lm_trans_collect_layout_field_common(
    const char *layout_name,
    const char *field_name,
    const LmOwnPtrStack *index_rows,
    const LmOwnPtrStack *address_depth_rows,
    const LmOwnPtrStack *const_rows,
    const LmOwnPtrStack *array_count_rows,
    LmTransLayoutField *field
) {
    size_t index;
    LmTransRegistryFact *index_row;
    LmTransRegistryFact *address_depth_row;
    LmTransRegistryFact *const_row;
    LmTransRegistryFact *array_count_row;
    LmP0Text field_text;

    if (layout_name == 0 || field_name == 0 || field == 0) {
        return 1;
    }

    field_text = lm_trans_text_from_cstr(field_name);
    index_row = lm_trans_registry_relation_stack_latest_row(index_rows, field_text);
    address_depth_row = lm_trans_registry_relation_stack_latest_row(address_depth_rows, field_text);
    const_row = lm_trans_registry_relation_stack_latest_row(const_rows, field_text);
    array_count_row = lm_trans_registry_relation_stack_latest_row(array_count_rows, field_text);

    if (index_row == 0 || !lm_trans_parse_size_payload(index_row->payload, &index)) {
        fprintf(stderr, "trans L4 layout error: field %s.%s requires field.index\n", layout_name, field_name);
        return 1;
    }

    memset(field, 0, sizeof(*field));
    field->name = field_name;
    field->index = index;

    if (
        address_depth_row != 0 &&
        !lm_trans_parse_size_payload(address_depth_row->payload, &field->address_depth)
    ) {
        fprintf(stderr, "trans L4 layout error: field %s.%s has invalid field.address-depth\n", layout_name, field_name);
        return 1;
    }
    if (const_row != 0) {
        if (!lm_trans_parse_size_payload(const_row->payload, &index)) {
            fprintf(stderr, "trans L4 layout error: field %s.%s has invalid field.const\n", layout_name, field_name);
            return 1;
        }
        field->is_const = index != 0U;
    }
    if (array_count_row != 0) {
        if (!lm_trans_parse_size_payload(array_count_row->payload, &field->array_count)) {
            fprintf(stderr, "trans L4 layout error: field %s.%s has invalid field.array-count\n", layout_name, field_name);
            return 1;
        }
        field->has_array_count = 1;
    }
    return 0;
}

static int lm_trans_collect_layout_fields(
    const char *layout_name,
    const LmTransNamespace *namespace_,
    LmTransLayoutField *fields,
    size_t capacity,
    size_t *out_count
) {
    size_t i;
    size_t field_count;
    LmTransRegistryFact *row;
    LmTransLayoutField *field;
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *union_rows;
    const LmOwnPtrStack *index_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    const LmOwnPtrStack *array_count_rows;
    LmP0Text layout_text;

    if (layout_name == 0 || fields == 0 || out_count == 0) {
        return 1;
    }

    layout_text = lm_trans_text_from_cstr(layout_name);
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.class");
    union_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.union");
    index_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.index");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.const");
    array_count_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.array-count");

    if (class_rows == 0 && union_rows == 0) {
        fprintf(stderr, "trans L4 layout error: layout %s requires field.class or field.union facts\n", layout_name);
        return 1;
    }

    field_count = 0U;
    if (class_rows != 0) {
        for (i = 0U; i < class_rows->count; ++i) {
            row = (LmTransRegistryFact *)lm_own_ptr_stack_at(class_rows, i);
            if (row != 0 && row->key != 0 && row->payload != 0) {
                if (field_count >= capacity) {
                    fprintf(stderr, "trans L4 layout error: too many fields in %s\n", layout_name);
                    return 1;
                }
                field = &fields[field_count];
                if (
                    lm_trans_collect_layout_field_common(
                        layout_name,
                        row->key,
                        index_rows,
                        address_depth_rows,
                        const_rows,
                        array_count_rows,
                        field
                    ) != 0
                ) {
                    return 1;
                }
                field->class_name = row->payload;
                if (field->is_const && field->address_depth == 0U && !field->has_array_count) {
                    fprintf(stderr, "trans L4 layout error: field %s.%s const value fields are not supported yet\n", layout_name, row->key);
                    return 1;
                }
                ++field_count;
            }
        }
    }

    if (union_rows != 0) {
        for (i = 0U; i < union_rows->count; ++i) {
            row = (LmTransRegistryFact *)lm_own_ptr_stack_at(union_rows, i);
            if (row != 0 && row->key != 0 && row->payload != 0) {
                if (field_count >= capacity) {
                    fprintf(stderr, "trans L4 layout error: too many fields in %s\n", layout_name);
                    return 1;
                }
                field = &fields[field_count];
                if (
                    lm_trans_collect_layout_field_common(
                        layout_name,
                        row->key,
                        index_rows,
                        address_depth_rows,
                        const_rows,
                        array_count_rows,
                        field
                    ) != 0
                ) {
                    return 1;
                }
                field->is_union = 1;
                field->union_layout_name = row->payload;
                ++field_count;
            }
        }
    }

    *out_count = field_count;
    return 0;
}

static void lm_trans_sort_layout_fields(
    LmTransLayoutField *fields,
    size_t field_count
) {
    size_t i;
    size_t j;
    LmTransLayoutField swap;

    if (fields == 0) {
        return;
    }

    for (i = 0U; i < field_count; ++i) {
        for (j = i + 1U; j < field_count; ++j) {
            if (fields[j].index < fields[i].index) {
                swap = fields[i];
                fields[i] = fields[j];
                fields[j] = swap;
            }
        }
    }
}

static int lm_trans_emit_layout_fields(
    FILE *file,
    const char *layout_name,
    const LmTransNamespace *namespace_,
    unsigned indent
);
static int lm_trans_l4_is_function_pointer_type(
    const LmTransNamespace *namespace_,
    const char *name
);
static int lm_trans_emit_l4_function_pointer_type_field(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *type_name,
    const char *field_name,
    const char *error_name
);

static int lm_trans_emit_layout_field(
    FILE *file,
    const LmTransLayoutField *field,
    const LmTransNamespace *namespace_,
    unsigned indent
) {
    LmTransCDeclarator declarator;

    if (file == 0 || field == 0) {
        return 1;
    }

    if (field->is_union) {
        if (
            field->union_layout_name == 0 ||
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "union {\n") != 0 ||
            lm_trans_emit_layout_fields(file, field->union_layout_name, namespace_, indent + 1U) != 0 ||
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "} ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(field->name)) != 0
        ) {
            return 1;
        }
    } else if (lm_trans_l4_is_function_pointer_type(namespace_, field->class_name)) {
        if (field->is_const || field->address_depth != 0U || field->has_array_count) {
            fprintf(stderr, "trans L4 layout error: function pointer field %s cannot use const, pointer-depth, or array-count\n", field->name);
            return 1;
        }
        if (
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_emit_l4_function_pointer_type_field(file, namespace_, field->class_name, field->name, "layout") != 0
        ) {
            return 1;
        }
    } else {
        memset(&declarator, 0, sizeof(declarator));
        declarator.type_head = lm_trans_text_from_cstr(field->class_name);
        declarator.name = lm_trans_text_from_cstr(field->name);
        declarator.pointer_depth = field->address_depth;
        declarator.type_is_head = 1;
        if (field->has_array_count) {
            declarator.literal_dimensions[0] = field->array_count;
            declarator.literal_dimension_count = 1U;
        }
        if (
            lm_trans_emit_indent(file, indent) != 0 ||
            (field->is_const && lm_trans_put(file, "const ") != 0) ||
            lm_trans_emit_c_declarator(file, &declarator, namespace_, "layout array dimension") != 0
        ) {
            return 1;
        }
    }

    if (field->is_union && field->has_array_count) {
        if (
            lm_trans_put(file, "[") != 0 ||
            lm_trans_emit_size_literal(file, field->array_count) != 0 ||
            lm_trans_put(file, "]") != 0
        ) {
            return 1;
        }
    }

    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_layout_fields(
    FILE *file,
    const char *layout_name,
    const LmTransNamespace *namespace_,
    unsigned indent
) {
    LmTransLayoutField fields[256];
    size_t field_count;
    size_t i;

    if (
        lm_trans_collect_layout_fields(
            layout_name,
            namespace_,
            fields,
            sizeof(fields) / sizeof(fields[0]),
            &field_count
        ) != 0
    ) {
        return 1;
    }
    lm_trans_sort_layout_fields(fields, field_count);

    for (i = 0U; i < field_count; ++i) {
        if (lm_trans_emit_layout_field(file, &fields[i], namespace_, indent) != 0) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_emit_layout_definition(
    FILE *file,
    const char *layout_name,
    const LmTransNamespace *namespace_
) {
    LmTransRegistryFact *backend_row;
    LmTransRegistryFact *forward_row;
    const LmOwnPtrStack *backend_rows;
    const LmOwnPtrStack *forward_rows;
    const char *backend;
    int has_forward;
    LmP0Text layout_text;

    if (file == 0 || layout_name == 0) {
        return 1;
    }

    layout_text = lm_trans_text_from_cstr(layout_name);
    backend_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("layout"), "backend");
    backend_row = lm_trans_registry_relation_stack_latest_row(backend_rows, layout_text);
    backend = backend_row != 0 ? backend_row->payload : 0;
    if (!lm_trans_layout_backend_is_supported(backend)) {
        fprintf(stderr, "trans L4 layout error: layout %s has unsupported layout.backend\n", layout_name);
        return 1;
    }
    forward_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("forward"), "backend");
    forward_row = lm_trans_registry_relation_stack_latest_row(forward_rows, layout_text);
    has_forward =
        forward_row != 0 &&
        forward_row->payload != 0 &&
        (
            strcmp(forward_row->payload, "c.struct") == 0 ||
            strcmp(forward_row->payload, "c.union") == 0
        );

    if (
        (
            strcmp(backend, "c.struct") == 0 &&
            lm_trans_put(file, has_forward ? "struct " : "typedef struct ") != 0
        ) ||
        (
            strcmp(backend, "c.named-struct") == 0 &&
            lm_trans_put(file, "struct ") != 0
        ) ||
        (
            strcmp(backend, "c.union") == 0 &&
            lm_trans_put(file, has_forward ? "union " : "typedef union ") != 0
        ) ||
        (
            strcmp(backend, "c.named-union") == 0 &&
            lm_trans_put(file, "union ") != 0
        ) ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(layout_name)) != 0 ||
        lm_trans_put(file, " {\n") != 0
    ) {
        return 1;
    }

    if (lm_trans_emit_layout_fields(file, layout_name, namespace_, 1U) != 0) {
        return 1;
    }

    if (has_forward && (strcmp(backend, "c.struct") == 0 || strcmp(backend, "c.union") == 0)) {
        return lm_trans_put(file, "};\n");
    }
    if (strcmp(backend, "c.struct") == 0 || strcmp(backend, "c.union") == 0) {
        return
            lm_trans_put(file, "} ") ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(layout_name)) ||
            lm_trans_put(file, ";\n");
    }
    return lm_trans_put(file, "};\n");
}

static int lm_trans_emit_l4_layout_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    size_t i;
    const char *name;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_layout_names(&names, namespace_) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    for (i = 0U; i < names.count; ++i) {
        if (i == 0U && lm_trans_put(file, "#include <stddef.h>\n\n") != 0) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        if (name != 0 && lm_trans_emit_layout_definition(file, name, namespace_) != 0) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_registry_collect_constant_define_names(
    LmOwnPtrStack *names,
    const LmTransNamespace *namespace_
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    char *name;
    LmP0Text key_text;

    if (names == 0) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("constant"),
        "backend"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L4 constant error: constant receiver expects constant.backend facts\n");
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload != 0 &&
            strcmp(row->payload, "c.define") == 0
        ) {
            key_text = lm_trans_text_from_cstr(row->key);
            if (lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0) {
                fprintf(stderr, "trans L4 constant error: constant.backend key %s is not a class\n", row->key);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                free(name);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                free(name);
                return 1;
            }
        }
    }
    return 0;
}

static int lm_trans_emit_l4_constant_defines(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    size_t i;
    const char *name;
    LmTransRegistryFact *value_row;
    const LmOwnPtrStack *value_rows;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_constant_define_names(&names, namespace_) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    value_rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("constant"),
        "value"
    );

    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        value_row = lm_trans_registry_relation_stack_latest_row(
            value_rows,
            lm_trans_text_from_cstr(name)
        );
        if (value_row == 0 || value_row->payload == 0) {
            fprintf(stderr, "trans L4 constant error: %s requires constant.value\n", name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (
            lm_trans_put(file, "#define ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, " ") != 0 ||
            lm_trans_put(file, value_row->payload) != 0 ||
            lm_trans_put(file, "\n") != 0
        ) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_registry_collect_backend_names(
    LmOwnPtrStack *names,
    const LmTransNamespace *namespace_,
    const char *receiver_name,
    const char *backend_payload,
    const char *error_name,
    int require_class
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    char *name;
    LmP0Text key_text;

    if (
        names == 0 ||
        receiver_name == 0 ||
        backend_payload == 0 ||
        error_name == 0
    ) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr(receiver_name),
        "backend"
    );
    if (rows == 0) {
        fprintf(stderr, "trans L4 %s error: %s receiver expects %s.backend facts\n", error_name, receiver_name, receiver_name);
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload != 0 &&
            strcmp(row->payload, backend_payload) == 0
        ) {
            key_text = lm_trans_text_from_cstr(row->key);
            if (
                require_class &&
                lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0
            ) {
                fprintf(stderr, "trans L4 %s error: %s.backend key %s is not a class\n", error_name, receiver_name, row->key);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                free(name);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                free(name);
                return 1;
            }
        }
    }

    return 0;
}

static int lm_trans_registry_collect_relation_names(
    LmOwnPtrStack *names,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *relation_name,
    const char *error_name,
    int require_class
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    char *name;
    LmP0Text key_text;

    if (
        names == 0 ||
        owner_name == 0 ||
        relation_name == 0 ||
        error_name == 0
    ) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr(owner_name),
        relation_name
    );
    if (rows == 0) {
        return 0;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (row == 0 || row->key == 0 || row->payload == 0) {
            continue;
        }
        key_text = lm_trans_text_from_cstr(row->key);
        if (
            require_class &&
            lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0
        ) {
            fprintf(stderr, "trans L4 %s error: %s.%s key %s is not a class\n", error_name, owner_name, relation_name, row->key);
            return 1;
        }
        name = lm_trans_text_copy_cstr(key_text);
        if (name == 0) {
            return 1;
        }
        if (lm_trans_string_stack_has(names, name)) {
            free(name);
        } else if (lm_own_ptr_stack_push(names, name) != 0) {
            free(name);
            return 1;
        }
    }

    return 0;
}

static int lm_trans_l4_is_function_pointer_type(
    const LmTransNamespace *namespace_,
    const char *name
) {
    LmP0Text name_text;
    const LmOwnPtrStack *rows;

    if (name == 0) {
        return 0;
    }

    name_text = lm_trans_text_from_cstr(name);
    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "value"
    );
    if (lm_trans_registry_relation_stack_latest_row(rows, name_text) != 0) {
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "row"
    );
    return lm_trans_registry_relation_stack_latest_row(rows, name_text) != 0;
}

static int lm_trans_emit_abi_typed_name(
    FILE *file,
    const char *class_name,
    size_t address_depth,
    int is_const,
    const char *name
) {
    size_t i;

    if (file == 0 || class_name == 0) {
        return 1;
    }

    if (is_const && lm_trans_put(file, "const ") != 0) {
        return 1;
    }
    if (lm_trans_emit_type_name(file, lm_trans_text_from_cstr(class_name)) != 0) {
        return 1;
    }
    if ((address_depth != 0U || name != 0) && lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < address_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    if (name != 0) {
        if (lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_registry_latest_size_payload(
    const LmOwnPtrStack *rows,
    LmP0Text key,
    size_t default_value,
    size_t *out_value
) {
    LmTransRegistryFact *row;

    if (out_value == 0) {
        return 0;
    }

    *out_value = default_value;
    row = lm_trans_registry_relation_stack_latest_row(rows, key);
    if (row == 0) {
        return 1;
    }
    return lm_trans_parse_size_payload(row->payload, out_value);
}

static LmTransRegistryFact *lm_trans_registry_relation_stack_latest_any(
    const LmOwnPtrStack *stack
) {
    if (stack == 0 || stack->count == 0U) {
        return 0;
    }
    return (LmTransRegistryFact *)lm_own_ptr_stack_at(stack, stack->count - 1U);
}

static LmTransRegistryFact *lm_trans_registry_relation_stack_latest_return_row(
    const LmOwnPtrStack *stack
) {
    LmTransRegistryFact *row;

    row = lm_trans_registry_relation_stack_latest_row(
        stack,
        lm_trans_text_from_cstr("return")
    );
    return row != 0 ? row : lm_trans_registry_relation_stack_latest_any(stack);
}

static int lm_trans_collect_abi_params(
    LmTransAbiParam *params,
    size_t capacity,
    size_t *out_count,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *error_name
) {
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *index_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmP0Text owner_text;
    LmP0Text param_text;
    LmTransRegistryFact *row;
    LmTransRegistryFact *index_row;
    size_t i;
    size_t j;
    size_t index;
    size_t const_flag;
    LmTransAbiParam swap;

    if (params == 0 || out_count == 0 || owner_name == 0 || error_name == 0) {
        return 1;
    }

    *out_count = 0U;
    owner_text = lm_trans_text_from_cstr(owner_name);
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "param.class");
    if (class_rows == 0) {
        return 0;
    }
    index_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "param.index");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "param.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "param.const");

    for (i = 0U; i < class_rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(class_rows, i);
        if (row == 0 || row->key == 0 || row->payload == 0) {
            continue;
        }
        if (*out_count >= capacity) {
            fprintf(stderr, "trans L4 %s error: too many parameters in %s\n", error_name, owner_name);
            return 1;
        }

        param_text = lm_trans_text_from_cstr(row->key);
        index_row = lm_trans_registry_relation_stack_latest_row(index_rows, param_text);
        if (index_row == 0 || !lm_trans_parse_size_payload(index_row->payload, &index)) {
            fprintf(stderr, "trans L4 %s error: parameter %s.%s requires param.index\n", error_name, owner_name, row->key);
            return 1;
        }

        params[*out_count].name = row->key;
        params[*out_count].class_name = row->payload;
        params[*out_count].index = index;
        if (
            !lm_trans_registry_latest_size_payload(
                address_depth_rows,
                param_text,
                0U,
                &params[*out_count].address_depth
            )
        ) {
            fprintf(stderr, "trans L4 %s error: parameter %s.%s has invalid param.address-depth\n", error_name, owner_name, row->key);
            return 1;
        }
        if (
            !lm_trans_registry_latest_size_payload(
                const_rows,
                param_text,
                0U,
                &const_flag
            )
        ) {
            fprintf(stderr, "trans L4 %s error: parameter %s.%s has invalid param.const\n", error_name, owner_name, row->key);
            return 1;
        }
        params[*out_count].is_const = const_flag != 0U;
        params[*out_count].index = index;
        ++*out_count;
    }

    for (i = 0U; i < *out_count; ++i) {
        for (j = i + 1U; j < *out_count; ++j) {
            if (params[j].index < params[i].index) {
                swap = params[i];
                params[i] = params[j];
                params[j] = swap;
            }
        }
    }

    return 0;
}

static int lm_trans_emit_abi_params(
    FILE *file,
    const LmTransAbiParam *params,
    size_t count
) {
    size_t i;

    if (file == 0 || (params == 0 && count != 0U)) {
        return 1;
    }

    if (count == 0U) {
        return lm_trans_put(file, "void");
    }

    for (i = 0U; i < count; ++i) {
        if (i != 0U && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (
            lm_trans_emit_abi_typed_name(
                file,
                params[i].class_name,
                params[i].address_depth,
                params[i].is_const,
                params[i].name
            ) != 0
        ) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_emit_l4_alias_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const LmOwnPtrStack *target_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *target_row;
    const char *name;
    LmP0Text name_text;
    size_t i;
    size_t address_depth;
    size_t const_flag;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "alias", "c.typedef", "alias", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    target_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target.const");

    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        name_text = lm_trans_text_from_cstr(name);
        target_row = lm_trans_registry_relation_stack_latest_row(target_rows, name_text);
        if (target_row == 0 || target_row->payload == 0) {
            fprintf(stderr, "trans L4 alias error: %s requires alias.target\n", name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (
            !lm_trans_registry_latest_size_payload(address_depth_rows, name_text, 0U, &address_depth) ||
            !lm_trans_registry_latest_size_payload(const_rows, name_text, 0U, &const_flag)
        ) {
            fprintf(stderr, "trans L4 alias error: %s has invalid alias target flags\n", name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (
            lm_trans_put(file, "typedef ") != 0 ||
            lm_trans_emit_abi_typed_name(file, target_row->payload, address_depth, const_flag != 0U, name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_l4_forward_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const LmOwnPtrStack *backend_rows;
    LmTransRegistryFact *backend_row;
    const char *name;
    const char *tag;
    size_t i;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "forward", "c.struct", "forward", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "forward", "c.union", "forward", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    backend_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("forward"), "backend");
    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        backend_row = lm_trans_registry_relation_stack_latest_row(backend_rows, lm_trans_text_from_cstr(name));
        if (backend_row == 0 || backend_row->payload == 0) {
            fprintf(stderr, "trans L4 forward error: %s requires forward.backend\n", name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (strcmp(backend_row->payload, "c.struct") == 0) {
            tag = "struct";
        } else if (strcmp(backend_row->payload, "c.union") == 0) {
            tag = "union";
        } else {
            fprintf(stderr, "trans L4 forward error: unsupported backend %s for %s\n", backend_row->payload, name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (
            lm_trans_put(file, "typedef ") != 0 ||
            lm_trans_put(file, tag) != 0 ||
            lm_trans_put(file, " ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, " ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_abi_return_type(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *error_name
) {
    LmP0Text owner_text;
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *class_row;
    LmP0Text return_key;
    size_t address_depth;
    size_t const_flag;

    if (file == 0 || owner_name == 0 || error_name == 0) {
        return 1;
    }

    owner_text = lm_trans_text_from_cstr(owner_name);
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.class");
    class_row = lm_trans_registry_relation_stack_latest_return_row(class_rows);
    if (class_row == 0 || class_row->key == 0 || class_row->payload == 0) {
        fprintf(stderr, "trans L4 %s error: %s requires return.class\n", error_name, owner_name);
        return 1;
    }

    return_key = lm_trans_text_from_cstr(class_row->key);
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.const");
    if (
        !lm_trans_registry_latest_size_payload(address_depth_rows, return_key, 0U, &address_depth) ||
        !lm_trans_registry_latest_size_payload(const_rows, return_key, 0U, &const_flag)
    ) {
        fprintf(stderr, "trans L4 %s error: %s has invalid return flags\n", error_name, owner_name);
        return 1;
    }

    return lm_trans_emit_abi_typed_name(file, class_row->payload, address_depth, const_flag != 0U, 0);
}

static int lm_trans_emit_l4_prototype_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
) {
    LmTransAbiParam params[256];
    size_t param_count;

    if (name == 0 || error_name == 0) {
        return 1;
    }

    return
        lm_trans_collect_abi_params(params, sizeof(params) / sizeof(params[0]), &param_count, namespace_, name, error_name) != 0 ||
        lm_trans_emit_abi_return_type(file, namespace_, name, error_name) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
        lm_trans_put(file, "(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ");\n") != 0;
}

static int lm_trans_emit_l4_prototypes(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const char *name;
    size_t i;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_relation_names(&names, namespace_, "fn", "descriptor", "fn", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        if (lm_trans_l4_is_function_pointer_type(namespace_, name)) {
            continue;
        }
        if (lm_trans_emit_l4_prototype_name(file, namespace_, name, "fn") != 0) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_l4_function_pointer_type_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
) {
    LmTransAbiParam params[256];
    size_t param_count;

    if (name == 0 || error_name == 0) {
        return 1;
    }

    return
        lm_trans_collect_abi_params(params, sizeof(params) / sizeof(params[0]), &param_count, namespace_, name, error_name) != 0 ||
        lm_trans_put(file, "typedef ") != 0 ||
        lm_trans_emit_abi_return_type(file, namespace_, name, error_name) != 0 ||
        lm_trans_put(file, " (*") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
        lm_trans_put(file, ")(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ");\n") != 0;
}

static int lm_trans_emit_l4_function_pointer_type_field(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *type_name,
    const char *field_name,
    const char *error_name
) {
    LmTransAbiParam params[256];
    size_t param_count;

    if (type_name == 0 || field_name == 0 || error_name == 0) {
        return 1;
    }

    return
        lm_trans_collect_abi_params(params, sizeof(params) / sizeof(params[0]), &param_count, namespace_, type_name, error_name) != 0 ||
        lm_trans_emit_abi_return_type(file, namespace_, type_name, error_name) != 0 ||
        lm_trans_put(file, " (*") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(field_name)) != 0 ||
        lm_trans_put(file, ")(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ")") != 0;
}

static int lm_trans_emit_l4_function_pointer_type_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const char *name;
    size_t i;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (
        lm_trans_registry_collect_relation_names(&names, namespace_, "functionPointerType", "value", "functionPointerType", 1) != 0 ||
        lm_trans_registry_collect_relation_names(&names, namespace_, "functionPointerType", "row", "functionPointerType", 1) != 0
    ) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        if (lm_trans_emit_l4_function_pointer_type_name(file, namespace_, name, "functionPointerType") != 0) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_l4_fn_descriptors(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_l4_function_pointer_type_typedefs(file, namespace_);
}

static int lm_trans_emit_l4_guard_markers(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const char *name;
    size_t i;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "guard", "c.header-guard", "guard", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        if (
            lm_trans_put(file, "#ifndef ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, "\n#define ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, "\n#endif\n") != 0
        ) {
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_l4_extern_c_markers(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "extern_c", "c.wrapper", "extern_c", 0) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    if (
        names.count != 0U &&
        (
            lm_trans_put(file, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n") != 0 ||
            lm_trans_put(file, "#ifdef __cplusplus\n}\n#endif\n\n") != 0
        )
    ) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static int lm_trans_emit_l4_units(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack names;
    const LmOwnPtrStack *backend_rows;
    const LmOwnPtrStack *payload_rows;
    LmTransRegistryFact *payload_row;
    const char *name;
    size_t i;

    backend_rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("unit"),
        "backend"
    );
    if (backend_rows == 0) {
        return 0;
    }

    lm_own_ptr_stack_init(&names, lm_trans_free_any);
    if (lm_trans_registry_collect_backend_names(&names, namespace_, "unit", "c.static-unit", "unit", 1) != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }

    payload_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("unit"), "payload");
    for (i = 0U; i < names.count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(&names, i);
        payload_row = lm_trans_registry_relation_stack_latest_row(payload_rows, lm_trans_text_from_cstr(name));
        if (payload_row == 0 || (payload_row->payload == 0 && payload_row->payload_node == 0)) {
            fprintf(stderr, "trans L4 unit error: %s requires unit.payload\n", name);
            lm_own_ptr_stack_destroy(&names);
            return 1;
        }
        if (payload_row->payload_node != 0) {
            if (
                lm_trans_emit_l4_payload_node(file, payload_row->payload_node, (LmTransNamespace *)namespace_) != 0 ||
                lm_trans_put(file, "\n") != 0
            ) {
                lm_own_ptr_stack_destroy(&names);
                return 1;
            }
        } else {
            if (
                lm_trans_put(file, payload_row->payload) != 0 ||
                lm_trans_put(file, "\n") != 0
            ) {
                lm_own_ptr_stack_destroy(&names);
                return 1;
            }
        }
    }

    if (names.count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_own_ptr_stack_destroy(&names);
        return 1;
    }
    lm_own_ptr_stack_destroy(&names);
    return 0;
}

static void lm_trans_registry_destroy(void) {
    if (lm_trans_registry.loaded) {
        lm_trans_identifier_table_destroy(&lm_trans_registry.identifiers);
        lm_own_arena_destroy(&lm_trans_registry.value_arena);
        lm_own_ptr_stack_destroy(&lm_trans_registry.loaded_paths);
        free(lm_trans_registry.source_path);
        lm_trans_registry.source_path = 0;
        lm_trans_registry.loaded_fact_count = 0U;
        lm_trans_registry.loaded = 0;
    }
}

static char *lm_trans_registry_join_text3(LmP0Text first, const char *separator, LmP0Text second) {
    size_t separator_length;
    size_t length;
    char *result;

    separator_length = strlen(separator);
    length = first.length + separator_length + second.length;
    result = (char *)malloc(length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, first.data, first.length);
    memcpy(result + first.length, separator, separator_length);
    memcpy(result + first.length + separator_length, second.data, second.length);
    result[length] = '\0';
    return result;
}

static int lm_trans_registry_push_generated_row_cstr(
    const char *table,
    const char *key,
    const char *payload
) {
    return lm_trans_registry_push_row_atoms(
        lm_trans_text_from_cstr(table),
        lm_trans_text_from_cstr(key),
        lm_trans_text_from_cstr(payload)
    );
}

static int lm_trans_registry_push_generated_row_text(
    const char *table,
    const char *key,
    LmP0Text payload
) {
    return lm_trans_registry_push_row_atoms(
        lm_trans_text_from_cstr(table),
        lm_trans_text_from_cstr(key),
        payload
    );
}

static int lm_trans_registry_push_column_metadata(
    LmP0Text table_name,
    const LmTransRegistryColumn *columns,
    size_t column_count
) {
    LmP0Text table_payload;
    LmP0Text column_payload;
    size_t index;
    size_t descriptor_index;
    char *column_key;
    char *descriptor_key;
    char index_buffer[32];
    char count_buffer[32];

    if (columns == 0) {
        return -1;
    }
    if (!lm_trans_registry_identifier_value(table_name, &table_payload)) {
        return -1;
    }
    if (lm_trans_registry_note_class_present(table_name) != 0) {
        return -1;
    }

    for (index = 0U; index < column_count; ++index) {
        if (!lm_trans_registry_identifier_value(columns[index].name, &column_payload)) {
            return -1;
        }
        if (lm_trans_registry_note_class_present(columns[index].name) != 0) {
            return -1;
        }
        column_key = lm_trans_registry_join_text3(table_payload, ".", column_payload);
        if (column_key == 0) {
            return -1;
        }

        snprintf(index_buffer, sizeof(index_buffer), "%lu", (unsigned long)index);
        snprintf(count_buffer, sizeof(count_buffer), "%lu", (unsigned long)columns[index].descriptor_count);

        if (
            lm_trans_registry_push_generated_row_text("column.table", column_key, table_payload) != 0 ||
            lm_trans_registry_push_generated_row_text("column.name", column_key, column_payload) != 0 ||
            lm_trans_registry_push_generated_row_cstr("column.index", column_key, index_buffer) != 0 ||
            lm_trans_registry_push_generated_row_cstr("column.descriptor.count", column_key, count_buffer) != 0
        ) {
            free(column_key);
            return -1;
        }

        for (descriptor_index = 0U; descriptor_index < columns[index].descriptor_count; ++descriptor_index) {
            snprintf(index_buffer, sizeof(index_buffer), "%lu", (unsigned long)descriptor_index);
            descriptor_key = lm_trans_registry_join_text3(lm_trans_text_from_cstr(column_key), ".", lm_trans_text_from_cstr(index_buffer));
            if (descriptor_key == 0) {
                free(column_key);
                return -1;
            }
            if (lm_trans_registry_push_generated_row_text("column.descriptor", descriptor_key, columns[index].descriptors[descriptor_index]) != 0) {
                free(descriptor_key);
                free(column_key);
                return -1;
            }
            free(descriptor_key);
        }

        free(column_key);
    }

    return 0;
}

static char *lm_trans_registry_relation_name_new(LmP0Text owner, const char *suffix) {
    size_t suffix_length;
    size_t length;
    char *result;

    if (suffix == 0) {
        return 0;
    }

    suffix_length = strlen(suffix);
    length = owner.length + suffix_length;
    result = (char *)malloc(length + 1U);
    if (result == 0) {
        return 0;
    }
    if (owner.length != 0U) {
        memcpy(result, owner.data, owner.length);
    }
    memcpy(result + owner.length, suffix, suffix_length);
    result[length] = '\0';
    return result;
}

static int lm_trans_registry_push_owner_relation_text(
    LmP0Text owner,
    const char *suffix,
    LmP0Text key,
    LmP0Text payload
) {
    char *table_name;
    int status;

    table_name = lm_trans_registry_relation_name_new(owner, suffix);
    if (table_name == 0) {
        return 1;
    }
    status = lm_trans_registry_push_row_values(
        lm_trans_text_from_cstr(table_name),
        key,
        payload
    ) != 0;
    free(table_name);
    return status;
}

static int lm_trans_registry_push_owner_relation_size(
    LmP0Text owner,
    const char *suffix,
    LmP0Text key,
    size_t value
) {
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)value);
    return lm_trans_registry_push_owner_relation_text(
        owner,
        suffix,
        key,
        lm_trans_text_from_cstr(buffer)
    );
}

static int lm_trans_l4_callable_type_from_node(
    const LmP0Node *node,
    LmTransL4CallableType *out
) {
    const LmP0Frame *frame;
    const LmP0Field *field;
    LmTransL4CallableType inner;

    if (node == 0 || out == 0) {
        return 0;
    }

    memset(out, 0, sizeof(*out));

    if (node->kind == LM_P0_NODE_ATOM) {
        if (!lm_trans_registry_identifier_value(node->as.atom, &out->class_name)) {
            return 0;
        }
        return 1;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        if (!lm_trans_l4_callable_type_from_node(field->value, &inner)) {
            return 0;
        }
        *out = inner;
        out->is_const = 1;
        return 1;
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        field = frame->body.first_field;
        if (field == 0 || field->value == 0) {
            return 0;
        }
        if (!lm_trans_l4_callable_type_from_node(field->value, &inner)) {
            return 0;
        }
        *out = inner;
        out->address_depth += frame->head.length;
        return 1;
    }

    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        field = frame->body.first_field;
        if (field == 0 || field->value == 0) {
            return 0;
        }
        if (!lm_trans_l4_callable_type_from_node(field->value, &inner)) {
            return 0;
        }
        *out = inner;
        ++out->address_depth;
        return 1;
    }

    if (!lm_trans_registry_identifier_value(frame->head, &out->class_name)) {
        return 0;
    }
    return 1;
}

static int lm_trans_registry_materialize_fn_descriptor_param(
    LmP0Text function_name,
    const LmP0Node *param_node,
    size_t index
) {
    LmTransL4CallableType param_type;
    LmP0Text param_name;
    char fallback_name[32];

    if (!lm_trans_l4_callable_type_from_node(param_node, &param_type)) {
        fprintf(stderr, "trans fn descriptor error: parameter descriptor expects a type\n");
        return 1;
    }
    if (!lm_trans_formal_param_name(param_node, &param_name)) {
        snprintf(fallback_name, sizeof(fallback_name), "arg%lu", (unsigned long)index);
        param_name = lm_trans_text_from_cstr(fallback_name);
    }

    if (
        lm_trans_registry_push_owner_relation_text(function_name, ".param.class", param_name, param_type.class_name) != 0 ||
        lm_trans_registry_push_owner_relation_size(function_name, ".param.index", param_name, index) != 0
    ) {
        return 1;
    }
    if (
        param_type.address_depth != 0U &&
        lm_trans_registry_push_owner_relation_size(function_name, ".param.address-depth", param_name, param_type.address_depth) != 0
    ) {
        return 1;
    }
    if (
        param_type.is_const &&
        lm_trans_registry_push_owner_relation_size(function_name, ".param.const", param_name, 1U) != 0
    ) {
        return 1;
    }
    return lm_trans_registry_note_class_present(param_type.class_name);
}

static int lm_trans_registry_materialize_fn_descriptor_frame(const LmP0Frame *frame) {
    LmTransFunctionHeader function;
    LmTransL4CallableType return_type;
    const LmP0Field *field;
    size_t index;
    LmP0Text function_name;

    if (frame == 0) {
        return 1;
    }

    if (lm_trans_receiver_fn(frame, 0, &function) <= 0) {
        fprintf(stderr, "trans fn descriptor error: malformed descriptor-only fn\n");
        return 1;
    }
    if (!function.is_descriptor_only) {
        fprintf(stderr, "trans fn descriptor error: descriptor-only fn must not have a body\n");
        return 1;
    }
    if (!lm_trans_registry_identifier_value(function.name, &function_name)) {
        return 1;
    }
    if (!lm_trans_l4_callable_type_from_node(function.return_node, &return_type)) {
        fprintf(stderr, "trans fn descriptor error: return descriptor expects a type\n");
        return 1;
    }

    if (
        lm_trans_registry_note_class_present(function_name) != 0 ||
        lm_trans_registry_note_class_present(return_type.class_name) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("fn.descriptor"),
            function_name,
            lm_trans_text_from_cstr("callableDescriptor")
        ) != 0 ||
        lm_trans_registry_push_owner_relation_text(function_name, ".return.class", lm_trans_text_from_cstr("return"), return_type.class_name) != 0
    ) {
        return 1;
    }
    if (
        return_type.address_depth != 0U &&
        lm_trans_registry_push_owner_relation_size(function_name, ".return.address-depth", lm_trans_text_from_cstr("return"), return_type.address_depth) != 0
    ) {
        return 1;
    }
    if (
        return_type.is_const &&
        lm_trans_registry_push_owner_relation_size(function_name, ".return.const", lm_trans_text_from_cstr("return"), 1U) != 0
    ) {
        return 1;
    }

    if (function.params_node == 0 || function.params_node->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans fn descriptor error: parameters must be a Structure\n");
        return 1;
    }

    index = 0U;
    field = function.params_node->as.structure.first_field;
    while (field != 0) {
        if (
            field->value != 0 &&
            !lm_trans_node_is_ignored(field->value) &&
            lm_trans_registry_materialize_fn_descriptor_param(function_name, field->value, index) != 0
        ) {
            return 1;
        }
        if (field->value != 0 && !lm_trans_node_is_ignored(field->value)) {
            ++index;
        }
        field = field->next;
    }

    return 0;
}

static const LmL4Loader lm_trans_registry_l4_loader;

static int lm_trans_l4_receiver_table(const LmP0Frame *frame, int allow_node_cells) {
    LmTransL4LoadContext context;
    int status;

    context.allow_node_cells = allow_node_cells;
    status = lm_l4_table_from_frame(&lm_trans_registry_l4_loader, &context, frame);
    if (status <= 0) {
        if (status == 0) {
            fprintf(stderr, "trans L4 error: table receiver expects a table frame\n");
        }
        return 1;
    }
    return 0;
}

static int lm_trans_l4_receiver_row(const LmP0Frame *frame, int allow_node_cells) {
    LmTransL4LoadContext context;
    int status;

    context.allow_node_cells = allow_node_cells;
    status = lm_l4_row_from_frame(&lm_trans_registry_l4_loader, &context, frame);
    if (status <= 0) {
        if (status == 0) {
            fprintf(stderr, "trans L4 error: row receiver expects a row frame\n");
        }
        return 1;
    }
    return 0;
}

static int lm_trans_l4_receiver_fn_descriptor(const LmP0Frame *frame, int allow_node_cells) {
    (void)allow_node_cells;
    return lm_trans_registry_materialize_fn_descriptor_frame(frame);
}

static int lm_trans_l4_atom_receiver_prelude_sequence(LmP0Text atom, int allow_node_cells) {
    (void)allow_node_cells;
    if (lm_trans_registry_relation_stack(atom, "item") == 0) {
        fprintf(
            stderr,
            "trans L4 error: prelude sequence \"%.*s\" has no item rows\n",
            (int)atom.length,
            atom.data
        );
        return 1;
    }
    if (lm_trans_registry_has(atom, "prelude.sequence")) {
        return 0;
    }
    return lm_trans_registry_push_row_values(
        lm_trans_text_from_cstr("prelude.sequence"),
        atom,
        lm_trans_text_from_cstr("1")
    ) != 0;
}

static int lm_trans_l4_head_binding_resolve_from_tables(
    LmP0Text head,
    const char *namespace_table,
    const char *receiver_table,
    LmTransL4HeadBinding *out
) {
    LmTransBinding resolved;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));
    out->receiver_type = lm_trans_registry_lookup_table_link_checked(
        head,
        namespace_table,
        "receiver.type"
    );
    if (out->receiver_type == 0) {
        return 0;
    }

    out->receiver_binding = lm_trans_registry_lookup(head, receiver_table);
    if (out->receiver_binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has no %s binding\n",
            namespace_table,
            (int)head.length,
            head.data,
            receiver_table
        );
        return 1;
    }
    if (
        !lm_trans_binding_resolve(out->receiver_binding, &resolved) ||
        resolved.l4_frame == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has unknown L4 binding %s\n",
            receiver_table,
            (int)head.length,
            head.data,
            out->receiver_binding
        );
        return 1;
    }
    out->frame = resolved.l4_frame;
    return 0;
}

static int lm_trans_l4_head_binding_resolve(
    LmP0Text head,
    LmTransL4HeadBinding *out
) {
    return lm_trans_l4_head_binding_resolve_from_tables(
        head,
        "namespace.l4",
        "receiver.l4",
        out
    );
}

static int lm_trans_l4_root_head_binding_resolve(
    LmP0Text head,
    LmTransL4HeadBinding *out
) {
    return lm_trans_l4_head_binding_resolve_from_tables(
        head,
        "namespace.l4.root",
        "receiver.l4.root",
        out
    );
}

static int lm_trans_l4_atom_binding_resolve_from_tables(
    LmP0Text atom,
    const char *namespace_table,
    const char *receiver_table,
    LmTransL4AtomBinding *out
) {
    LmTransBinding resolved;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));
    out->receiver_type = lm_trans_registry_lookup_table_link_checked(
        atom,
        namespace_table,
        "receiver.type"
    );
    if (out->receiver_type == 0) {
        return 0;
    }

    out->receiver_binding = lm_trans_registry_lookup(atom, receiver_table);
    if (out->receiver_binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has no %s binding\n",
            namespace_table,
            (int)atom.length,
            atom.data,
            receiver_table
        );
        return 1;
    }
    if (
        !lm_trans_binding_resolve(out->receiver_binding, &resolved) ||
        resolved.l4_atom == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has unknown L4 atom binding %s\n",
            receiver_table,
            (int)atom.length,
            atom.data,
            out->receiver_binding
        );
        return 1;
    }
    out->atom = resolved.l4_atom;
    return 0;
}

static int lm_trans_l4_atom_binding_resolve(
    LmP0Text atom,
    LmTransL4AtomBinding *out
) {
    return lm_trans_l4_atom_binding_resolve_from_tables(
        atom,
        "namespace.l4.atom",
        "receiver.l4.atom",
        out
    );
}

static int lm_trans_registry_load_l4_frame(
    const LmP0Frame *frame,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4HeadBinding binding;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_l4_head_binding_resolve(frame->head, &binding) != 0) {
        return 1;
    }
    if (binding.frame == 0) {
        return 0;
    }
    if (binding.frame(frame, allow_node_cells) != 0) {
        return 1;
    }
    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_load_l4_atom(
    LmP0Text atom,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4AtomBinding binding;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (lm_trans_l4_atom_binding_resolve(atom, &binding) != 0) {
        return 1;
    }
    if (binding.atom == 0) {
        return 0;
    }
    if (binding.atom(atom, allow_node_cells) != 0) {
        return 1;
    }
    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_load_l4_structure(
    const LmP0Structure *structure,
    int allow_node_cells
) {
    const LmP0Field *field;
    const LmP0Node *node;
    int loaded;

    field = structure != 0 ? structure->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind == LM_P0_NODE_ATOM) {
                if (lm_trans_registry_load_l4_atom(node->as.atom, allow_node_cells, &loaded) != 0) {
                    return 1;
                }
                if (!loaded) {
                    fprintf(stderr, "trans L4 error: unknown L4 atom receiver\n");
                    return 1;
                }
            } else if (node->kind == LM_P0_NODE_FRAME) {
                if (lm_trans_registry_load_l4_frame(&node->as.frame, allow_node_cells, &loaded) != 0) {
                    return 1;
                }
                if (!loaded) {
                    fprintf(stderr, "trans L4 error: unknown L4 receiver\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "trans L4 error: L4 body expects registered L4 atom or frame receivers\n");
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_l4_root_receiver_registry(const LmP0Frame *frame, int allow_node_cells) {
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_registry_load_l4_structure(&frame->body, allow_node_cells) != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_registry_load_l4_root_frame(
    const LmP0Frame *frame,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4HeadBinding binding;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_l4_root_head_binding_resolve(frame->head, &binding) != 0) {
        return 1;
    }
    if (binding.frame == 0) {
        return 0;
    }
    if (binding.frame(frame, allow_node_cells) != 0) {
        return 1;
    }
    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_l4_push_row(
    void *context,
    LmP0Text table_atom,
    LmP0Text key_atom,
    const LmP0Node *payload_node
) {
    LmTransL4LoadContext *load_context;
    LmP0Text table_value;
    LmP0Text key_value;

    load_context = (LmTransL4LoadContext *)context;
    if (payload_node == 0) {
        return -1;
    }

    if (payload_node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_registry_push_row_atoms(table_atom, key_atom, payload_node->as.atom);
    }

    if (load_context == 0 || !load_context->allow_node_cells) {
        fprintf(stderr, "trans registry error: node row payloads are not allowed in this registry load\n");
        return -1;
    }
    if (
        !lm_trans_registry_identifier_value(table_atom, &table_value) ||
        !lm_trans_registry_identifier_value(key_atom, &key_value)
    ) {
        return -1;
    }
    return lm_trans_registry_push_row_node_values(table_value, key_value, payload_node);
}

static int lm_trans_registry_l4_note_key(
    void *context,
    LmP0Text table_name,
    const LmL4Column *column,
    LmP0Text key_atom
) {
    (void)context;
    (void)table_name;
    if (lm_trans_registry_column_is_class_typed(column)) {
        return lm_trans_registry_note_class_present(key_atom);
    }
    return 0;
}

static int lm_trans_registry_l4_push_cell(
    void *context,
    LmP0Text table_name,
    const LmL4Column *column,
    int split_by_column,
    LmP0Text key_atom,
    const LmP0Node *payload_node
) {
    LmTransL4LoadContext *load_context;

    load_context = (LmTransL4LoadContext *)context;
    if (
        lm_trans_registry_push_table_cell(
            table_name,
            column,
            split_by_column,
            key_atom,
            payload_node,
            load_context != 0 && load_context->allow_node_cells
        ) != 0
    ) {
        return -1;
    }
    if (
        lm_trans_registry_column_is_class_typed(column) &&
        payload_node != 0 &&
        payload_node->kind == LM_P0_NODE_ATOM &&
        lm_trans_registry_payload_is_null(payload_node->as.atom) == 0
    ) {
        return lm_trans_registry_note_class_present(payload_node->as.atom);
    }
    return 0;
}

static int lm_trans_registry_l4_push_column_metadata(
    void *context,
    LmP0Text table_name,
    const LmL4Column *columns,
    size_t column_count
) {
    (void)context;
    return lm_trans_registry_push_column_metadata(table_name, columns, column_count);
}

static const LmL4Loader lm_trans_registry_l4_loader = {
    "trans",
    lm_trans_registry_l4_push_row,
    lm_trans_registry_l4_push_cell,
    lm_trans_registry_l4_note_key,
    lm_trans_registry_l4_push_column_metadata,
    lm_l4_default_receivers,
    sizeof(lm_l4_default_receivers) / sizeof(lm_l4_default_receivers[0])
};

static int lm_trans_registry_load_root(const LmP0Node *root, int implicit_l4, int allow_node_cells) {
    const LmP0Field *field;
    const LmP0Node *node;
    int loaded;
    int item_loaded;

    (void)lm_l4_load_root;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L4 error: root must be a Structure\n");
        return 1;
    }

    loaded = 0;
    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L4 error: root fields must be registered L4 root receiver frames, or registered L4 receiver frames in .lm4 files\n");
                return 1;
            }
            if (lm_trans_registry_load_l4_root_frame(&node->as.frame, allow_node_cells, &item_loaded) != 0) {
                return 1;
            }
            if (item_loaded) {
                loaded = 1;
            } else if (implicit_l4) {
                if (lm_trans_registry_load_l4_frame(&node->as.frame, allow_node_cells, &item_loaded) != 0) {
                    return 1;
                }
                if (!item_loaded) {
                    fprintf(stderr, "trans L4 error: root fields must be registered L4 root receiver frames, or registered L4 receiver frames in .lm4 files\n");
                    return 1;
                }
                loaded = 1;
            } else {
                fprintf(stderr, "trans L4 error: root fields must be registered L4 root receiver frames\n");
                return 1;
            }
        }
        field = field->next;
    }

    if (!loaded) {
        fprintf(stderr, "trans L4 error: no rows loaded\n");
        return 1;
    }
    return 0;
}

static int lm_trans_path_has_extension(const char *path, const char *extension) {
    size_t path_length;
    size_t extension_length;
    size_t i;
    char left;
    char right;

    if (path == 0 || extension == 0) {
        return 0;
    }

    path_length = strlen(path);
    extension_length = strlen(extension);
    if (path_length < extension_length) {
        return 0;
    }

    i = 0U;
    while (i < extension_length) {
        left = path[path_length - extension_length + i];
        right = extension[i];
        if (left >= 'A' && left <= 'Z') {
            left = (char)(left - 'A' + 'a');
        }
        if (right >= 'A' && right <= 'Z') {
            right = (char)(right - 'A' + 'a');
        }
        if (left != right) {
            return 0;
        }
        ++i;
    }

    return 1;
}

static int lm_trans_registry_load_file_path(
    const char *registry_path,
    int required,
    int *out_loaded
);
static int lm_trans_registry_candidate_path(
    const char *source_path,
    const char *candidate_name,
    char *registry_path,
    size_t registry_path_size
);

static int lm_trans_registry_path_already_loaded(const char *registry_path) {
    size_t i;
    const char *loaded_path;

    if (registry_path == 0) {
        return 0;
    }

    for (i = 0U; i < lm_trans_registry.loaded_paths.count; ++i) {
        loaded_path = (const char *)lm_own_ptr_stack_at(&lm_trans_registry.loaded_paths, i);
        if (loaded_path != 0 && strcmp(loaded_path, registry_path) == 0) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_registry_note_loaded_path(const char *registry_path) {
    char *copy;

    if (registry_path == 0) {
        return 1;
    }
    if (lm_trans_registry_path_already_loaded(registry_path)) {
        return 0;
    }

    copy = lm_trans_text_copy_cstr(lm_trans_text_from_cstr(registry_path));
    if (copy == 0) {
        return 1;
    }

    if (lm_own_ptr_stack_push(&lm_trans_registry.loaded_paths, copy) != 0) {
        free(copy);
        return 1;
    }

    return 0;
}

static int lm_trans_registry_load_import_frame(
    const LmP0Frame *frame,
    const char *source_path
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text path_value;
    char *candidate_name;
    char registry_path[4096];
    int loaded;

    if (frame == 0) {
        return 1;
    }

    field = frame->body.first_field;
    if (field == 0) {
        fprintf(stderr, "trans registry import error: import receiver expects at least one path\n");
        return 1;
    }

    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node == 0 ||
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as.atom, &path_value)
            ) {
                fprintf(stderr, "trans registry import error: import path must be an atom/string\n");
                return 1;
            }

            candidate_name = lm_trans_text_copy_cstr(path_value);
            if (candidate_name == 0) {
                return 1;
            }
            if (
                lm_trans_registry_candidate_path(
                    source_path,
                    candidate_name,
                    registry_path,
                    sizeof(registry_path)
                ) != 0
            ) {
                free(candidate_name);
                fprintf(stderr, "trans registry import error: import path is too long\n");
                return 1;
            }
            free(candidate_name);

            if (!lm_trans_registry_path_already_loaded(registry_path)) {
                if (lm_trans_registry_load_file_path(registry_path, 1, &loaded) != 0) {
                    return 1;
                }
                if (!loaded) {
                    fprintf(stderr, "trans registry import error: cannot read %s\n", registry_path);
                    return 1;
                }
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_registry_load_imports_from_structure(
    const LmP0Structure *structure,
    const char *source_path,
    int descend_l2
) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransHeadBinding binding;

    field = structure != 0 ? structure->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (
            node != 0 &&
            !lm_trans_node_is_ignored(node) &&
            node->kind == LM_P0_NODE_FRAME
        ) {
            if (lm_trans_head_binding_resolve(0, node->as.frame.head, &binding) != 0) {
                return 1;
            }
            if (binding.statement_frame == lm_trans_statement_emit_import) {
                if (lm_trans_registry_load_import_frame(&node->as.frame, source_path) != 0) {
                    return 1;
                }
            } else if (descend_l2 && lm_trans_text_equals(node->as.frame.head, "L2")) {
                if (lm_trans_registry_load_imports_from_structure(&node->as.frame.body, source_path, 0) != 0) {
                    return 1;
                }
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_registry_load_inline_root(const LmP0Node *root, const char *source_path) {
    const LmP0Field *field;
    const LmP0Node *node;
    int loaded;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (lm_trans_registry_load_imports_from_structure(&root->as.structure, source_path, 1) != 0) {
        return 1;
    }

    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (
            node != 0 &&
            !lm_trans_node_is_ignored(node) &&
            node->kind == LM_P0_NODE_FRAME
        ) {
            if (lm_trans_registry_load_l4_root_frame(&node->as.frame, 1, &loaded) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_path_separator(char ch) {
    return ch == '/' || ch == '\\';
}

static int lm_trans_path_is_absolute(const char *path) {
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_trans_path_separator(path[0])) {
        return 1;
    }
    return (
        ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) &&
        path[1] == ':' &&
        lm_trans_path_separator(path[2])
    );
}

static int lm_trans_path_segment_equals(
    const char *path,
    size_t offset,
    const char *segment
) {
    size_t i;

    if (path == 0 || segment == 0) {
        return 0;
    }
    i = 0U;
    while (segment[i] != '\0') {
        if (path[offset + i] != segment[i]) {
            return 0;
        }
        ++i;
    }
    return path[offset + i] == '\0' || lm_trans_path_separator(path[offset + i]);
}

static size_t lm_trans_project_root_length_from_source(const char *source_path) {
    size_t length;
    size_t i;

    if (source_path == 0 || !lm_trans_path_is_absolute(source_path)) {
        return 0U;
    }

    length = strlen(source_path);
    for (i = 0U; i < length; ++i) {
        if (
            (i == 0U || lm_trans_path_separator(source_path[i - 1U])) &&
            (
                lm_trans_path_segment_equals(source_path, i, "lm2") ||
                lm_trans_path_segment_equals(source_path, i, "lm4") ||
                lm_trans_path_segment_equals(source_path, i, "tests")
            )
        ) {
            return i;
        }
    }

    return 0U;
}

static int lm_trans_registry_project_path_for_source(
    const char *source_path,
    const char *file_name,
    char *buffer,
    size_t size
) {
    size_t root_length;
    size_t file_length;

    if (buffer == 0 || file_name == 0 || size == 0U) {
        return 1;
    }

    root_length = lm_trans_project_root_length_from_source(source_path);
    file_length = strlen(file_name);
    if (root_length == 0U) {
        if (file_length >= size) {
            return 1;
        }
        strcpy(buffer, file_name);
        return 0;
    }

    if (root_length + file_length >= size) {
        fprintf(stderr, "trans error: registry path is too long\n");
        return 1;
    }
    memcpy(buffer, source_path, root_length);
    strcpy(buffer + root_length, file_name);
    return 0;
}

static int lm_trans_registry_path_for_source(
    const char *source_path,
    const char *file_name,
    char *buffer,
    size_t size
) {
    size_t length;
    size_t i;
    size_t slash;

    if (buffer == 0 || file_name == 0 || size == 0U) {
        return 1;
    }

    if (source_path == 0 || source_path[0] == '\0') {
        if (strlen(file_name) >= size) {
            return 1;
        }
        strcpy(buffer, file_name);
        return 0;
    }

    length = strlen(source_path);
    slash = 0U;
    for (i = 0U; i < length; ++i) {
        if (lm_trans_path_separator(source_path[i])) {
            slash = i + 1U;
        }
    }

    if (slash == 0U) {
        if (strlen(file_name) >= size) {
            return 1;
        }
        strcpy(buffer, file_name);
        return 0;
    }

    if (slash + strlen(file_name) >= size) {
        fprintf(stderr, "trans error: registry path is too long\n");
        return 1;
    }
    memcpy(buffer, source_path, slash);
    strcpy(buffer + slash, file_name);
    return 0;
}

static int lm_trans_registry_load_file_path(
    const char *registry_path,
    int required,
    int *out_loaded
) {
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    int status;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (registry_path == 0 || registry_path[0] == '\0') {
        if (required) {
            fprintf(stderr, "trans registry error: empty registry path\n");
            return 1;
        }
        return 0;
    }
    if (lm_trans_registry_path_already_loaded(registry_path)) {
        if (out_loaded != 0) {
            *out_loaded = 1;
        }
        return 0;
    }

    document = 0;
    status = lm_p0_parse_file(registry_path, &document);
    if (status != 0) {
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != 0) {
            fprintf(
                stderr,
                "trans registry parse error %d at %lu:%lu in %s: %s\n",
                diagnostic->code,
                (unsigned long)diagnostic->line,
                (unsigned long)diagnostic->column,
                registry_path,
                diagnostic->message
            );
            lm_p0_document_destroy(document);
            return 1;
        }

        lm_p0_document_destroy(document);
        if (required) {
            fprintf(stderr, "trans registry error: cannot read %s\n", registry_path);
            return 1;
        }
        return 0;
    }

    if (lm_trans_path_has_extension(registry_path, ".lm4")) {
        status = lm_trans_registry_load_root(
            lm_p0_document_root(document),
            1,
            1
        );
    } else {
        status = lm_trans_registry_load_inline_root(
            lm_p0_document_root(document),
            registry_path
        );
    }
    if (status != 0) {
        lm_p0_document_destroy(document);
        return 1;
    }

    lm_p0_document_destroy(document);
    if (lm_trans_registry_note_loaded_path(registry_path) != 0) {
        return 1;
    }

    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_candidate_path(
    const char *source_path,
    const char *candidate_name,
    char *registry_path,
    size_t registry_path_size
) {
    if (candidate_name == 0 || registry_path == 0 || registry_path_size == 0U) {
        return 1;
    }
    if (strncmp(candidate_name, "lm2/", 4U) == 0 || strncmp(candidate_name, "lm4/", 4U) == 0) {
        return lm_trans_registry_project_path_for_source(
            source_path,
            candidate_name,
            registry_path,
            registry_path_size
        );
    }
    return lm_trans_registry_path_for_source(
        source_path,
        candidate_name,
        registry_path,
        registry_path_size
    );
}

static int lm_trans_registry_seed_l4_receivers(void) {
    return
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.type"),
            lm_trans_text_from_cstr("receiver.l4"),
            lm_trans_text_from_cstr("lm_trans_dispatch_l4_receiver")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.type"),
            lm_trans_text_from_cstr("receiver.l4.root"),
            lm_trans_text_from_cstr("lm_trans_dispatch_l4_root_receiver")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("table"),
            lm_trans_text_from_cstr("receiver.l4")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("row"),
            lm_trans_text_from_cstr("receiver.l4")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("fn"),
            lm_trans_text_from_cstr("receiver.l4")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.l4"),
            lm_trans_text_from_cstr("table"),
            lm_trans_text_from_cstr("lm_trans_l4_receiver_table")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.l4"),
            lm_trans_text_from_cstr("row"),
            lm_trans_text_from_cstr("lm_trans_l4_receiver_row")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.l4"),
            lm_trans_text_from_cstr("fn"),
            lm_trans_text_from_cstr("lm_trans_l4_receiver_fn_descriptor")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4.root"),
            lm_trans_text_from_cstr("L4"),
            lm_trans_text_from_cstr("receiver.l4.root")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4.root"),
            lm_trans_text_from_cstr("registry"),
            lm_trans_text_from_cstr("receiver.l4.root")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.l4.root"),
            lm_trans_text_from_cstr("L4"),
            lm_trans_text_from_cstr("lm_trans_l4_root_receiver_registry")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("receiver.l4.root"),
            lm_trans_text_from_cstr("registry"),
            lm_trans_text_from_cstr("lm_trans_l4_root_receiver_registry")
        ) != 0;
}

static int lm_trans_registry_load_for_source(const char *source_path) {
    char registry_path[4096];
    const char *core_candidate_names[4];
    const char *candidate_names[6];
    const char *override_path;
    int override_enabled;
    int loaded;
    int registry_loaded;
    size_t candidate_index;

    lm_trans_registry_destroy();
    lm_trans_identifier_table_init(&lm_trans_registry.identifiers);
    lm_own_arena_init(&lm_trans_registry.value_arena);
    lm_own_ptr_stack_init(&lm_trans_registry.loaded_paths, free);
    lm_trans_registry.source_path = lm_trans_text_copy_cstr(lm_trans_text_from_cstr(source_path));
    if (lm_trans_registry.source_path == 0) {
        lm_trans_registry.loaded = 1;
        lm_trans_registry_destroy();
        return 1;
    }
    lm_trans_registry.loaded_fact_count = 0U;
    lm_trans_registry.loaded = 1;

    if (lm_trans_registry_seed_l4_receivers() != 0) {
        lm_trans_registry_destroy();
        return 1;
    }

    core_candidate_names[0] = "lm4/core.lm4";
    core_candidate_names[1] = "core.lm4";
    core_candidate_names[2] = "lm2/core.lm4";
    core_candidate_names[3] = 0;

    candidate_index = 0U;
    while (core_candidate_names[candidate_index] != 0) {
        if (
            lm_trans_registry_candidate_path(
                source_path,
                core_candidate_names[candidate_index],
                registry_path,
                sizeof(registry_path)
            ) != 0
        ) {
            lm_trans_registry_destroy();
            return 1;
        }
        if (lm_trans_registry_load_file_path(registry_path, 0, &loaded) != 0) {
            lm_trans_registry_destroy();
            return 1;
        }
        if (loaded) {
            break;
        }
        ++candidate_index;
    }

    override_path = getenv("LM_TRANS_REGISTRY");
    override_enabled = override_path != 0 && override_path[0] != '\0';
    candidate_names[0] = override_enabled ? "" : "lm4/trans_registry.lm4";
    candidate_names[1] = "trans_registry.lm4";
    candidate_names[2] = "trans_registry.lmx";
    candidate_names[3] = "lm2/trans_registry.lm4";
    candidate_names[4] = "lm2/trans_registry.lmx";
    candidate_names[5] = 0;

    registry_loaded = 0;
    candidate_index = 0U;
    while (override_enabled || candidate_names[candidate_index] != 0) {
        if (override_enabled) {
            if (strlen(override_path) >= sizeof(registry_path)) {
                fprintf(stderr, "trans error: LM_TRANS_REGISTRY path is too long\n");
                lm_trans_registry_destroy();
                return 1;
            }
            strcpy(registry_path, override_path);
        } else if (
            lm_trans_registry_candidate_path(
                source_path,
                candidate_names[candidate_index],
                registry_path,
                sizeof(registry_path)
            ) != 0
        ) {
            lm_trans_registry_destroy();
            return 1;
        }

        if (lm_trans_registry_load_file_path(registry_path, override_enabled, &loaded) != 0) {
            lm_trans_registry_destroy();
            return 1;
        }
        if (loaded) {
            registry_loaded = 1;
            break;
        }

        if (override_enabled) {
            break;
        }
        ++candidate_index;
    }

    if (!registry_loaded) {
        fprintf(stderr, "trans registry error: cannot read trans_registry.lm4\n");
        lm_trans_registry_destroy();
        return 1;
    }

    return 0;
}

static int lm_trans_emit_module_prelude_plan(FILE *file) {
    LmTransNamespace *namespace_;
    int emitted;
    int status;

    namespace_ = lm_trans_namespace_new();
    if (namespace_ == 0) {
        return 1;
    }

    emitted = 0;
    status = lm_trans_namespace_attach_registry(namespace_);
    if (status == 0) {
        status = lm_trans_emit_configured_prelude_sequences(file, namespace_, &emitted);
    }
    if (status == 0 && emitted) {
        status = lm_trans_put(file, "\n");
    }

    lm_trans_namespace_delete(namespace_);
    return status;
}

static int lm_trans_emit_document(const char *source_path, const char *output_path) {
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *root;
    FILE *output;
    FILE *prelude_output;
    FILE *body_output;
    FILE *previous_prelude_output;
    char *prelude_path;
    char *body_path;
    int status;
    int close_status;
    int emitted;

    document = 0;
    output = 0;
    prelude_output = 0;
    body_output = 0;
    previous_prelude_output = lm_trans_prelude_output;
    prelude_path = 0;
    body_path = 0;
    if (lm_trans_registry_load_for_source(source_path) != 0) {
        return 1;
    }

    status = lm_p0_parse_file(source_path, &document);
    if (status != 0) {
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != 0) {
            fprintf(
                stderr,
                "trans parse error %d at %lu:%lu: %s\n",
                diagnostic->code,
                (unsigned long)diagnostic->line,
                (unsigned long)diagnostic->column,
                diagnostic->message
            );
        } else {
            fprintf(stderr, "trans parse error while reading %s\n", source_path);
        }
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    root = lm_p0_document_root(document);
    if (lm_trans_registry_load_inline_root(root, source_path) != 0) {
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    prelude_path = lm_trans_temp_output_path_new(output_path, ".prelude.tmp");
    body_path = lm_trans_temp_output_path_new(output_path, ".body.tmp");
    if (prelude_path == 0 || body_path == 0) {
        fprintf(stderr, "trans error: cannot allocate temporary output paths\n");
        free(prelude_path);
        free(body_path);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    prelude_output = fopen(prelude_path, "w+b");
    body_output = fopen(body_path, "w+b");
    if (prelude_output == 0 || body_output == 0) {
        fprintf(stderr, "trans error: cannot create temporary output streams\n");
        if (prelude_output != 0) {
            fclose(prelude_output);
        }
        if (body_output != 0) {
            fclose(body_output);
        }
        remove(prelude_path);
        remove(body_path);
        free(prelude_path);
        free(body_path);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    lm_trans_prelude_output = prelude_output;
    lm_trans_current_source_path = source_path;
    lm_own_ptr_stack_init(&lm_trans_declared_import_paths, free);
    lm_own_ptr_stack_init(&lm_trans_declared_import_documents, lm_trans_import_document_delete);
    lm_own_ptr_stack_init(&lm_trans_emitted_import_prelude_paths, free);
    lm_own_ptr_stack_init(&lm_trans_emitted_import_function_paths, free);
    lm_own_ptr_stack_init(&lm_trans_emitted_callable_adapters, free);
    lm_own_ptr_stack_init(&lm_trans_emitted_callable_binders, free);
    lm_own_ptr_stack_init(&lm_trans_emitted_array_value_helpers, free);
    lm_own_ptr_stack_init(&lm_trans_emitted_function_return_structs, free);
    lm_trans_next_array_value_helper_id = 0U;
    emitted = 0;
    status = lm_trans_emit_module_prelude_plan(prelude_output);
    if (status == 0) {
        status = lm_trans_emit_root_sequence(
            body_output,
            root,
            lm_trans_path_has_extension(source_path, ".lm2"),
            &emitted
        );
    }
    lm_own_ptr_stack_destroy(&lm_trans_emitted_function_return_structs);
    lm_own_ptr_stack_destroy(&lm_trans_emitted_array_value_helpers);
    lm_own_ptr_stack_destroy(&lm_trans_emitted_callable_binders);
    lm_own_ptr_stack_destroy(&lm_trans_emitted_callable_adapters);
    lm_own_ptr_stack_destroy(&lm_trans_emitted_import_function_paths);
    lm_own_ptr_stack_destroy(&lm_trans_emitted_import_prelude_paths);
    lm_own_ptr_stack_destroy(&lm_trans_declared_import_documents);
    lm_own_ptr_stack_destroy(&lm_trans_declared_import_paths);
    lm_trans_current_source_path = 0;
    lm_trans_prelude_output = previous_prelude_output;
    if (status == 0 && !emitted) {
        fprintf(stderr, "trans error: root L1 or L2 frame was not found\n");
        status = 1;
    }

    if (status == 0) {
        output = fopen(output_path, "wb");
        if (output == 0) {
            fprintf(stderr, "trans error: cannot open output file %s\n", output_path);
            status = 1;
        }
    }
    if (
        status == 0 &&
        (
            lm_trans_copy_stream(output, prelude_output) != 0 ||
            lm_trans_copy_stream(output, body_output) != 0
        )
    ) {
        fprintf(stderr, "trans error: cannot write output file %s\n", output_path);
        status = 1;
    }

    close_status = output != 0 ? fclose(output) : 0;
    fclose(prelude_output);
    fclose(body_output);
    remove(prelude_path);
    remove(body_path);
    free(prelude_path);
    free(body_path);
    if (status != 0) {
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    if (close_status != 0) {
        fprintf(stderr, "trans error: cannot write output file %s\n", output_path);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    lm_p0_document_destroy(document);
    lm_trans_registry_destroy();
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    if (argc != 3) {
        fprintf(stderr, "usage: trans.lm0[.exe] <source.lm2> <output.lm1.c>\n");
        return 1;
    }

    return lm_trans_emit_document(argv[1], argv[2]);
}

static int lm_trans_text_equals(LmP0Text text, const char *value) {
    size_t length;
    length = strlen(value);
    return text.length == length && memcmp(text.data, value, length) == 0;
}

static int lm_trans_text_same(LmP0Text left, LmP0Text right) {
    if (left.length != right.length) {
        return 0;
    }
    if (left.length == 0U) {
        return 1;
    }
    return memcmp(left.data, right.data, left.length) == 0;
}

static int lm_trans_text_starts_with(LmP0Text text, const char *prefix) {
    size_t length;
    length = strlen(prefix);
    return text.length >= length && memcmp(text.data, prefix, length) == 0;
}

static int lm_trans_identifier_payload(LmP0Text atom, LmP0Text *out_payload) {
    if (out_payload == 0) {
        return 0;
    }
    out_payload[0] = atom;
    if (atom.length >= 2U && atom.data[0] == '`' && atom.data[atom.length - 1U] == '`') {
        out_payload->data = atom.data + 1U;
        out_payload->length = atom.length - 2U;
    }
    return 1;
}

static int lm_trans_identifier_same(LmP0Text left, LmP0Text right) {
    LmP0Text left_payload;
    LmP0Text right_payload;
    if (lm_trans_identifier_payload(left, &left_payload) == 0 || lm_trans_identifier_payload(right, &right_payload) == 0) {
        return 0;
    }
    return lm_trans_text_same(left_payload, right_payload);
}

static int lm_trans_emit_identifier(FILE *file, LmP0Text name) {
    LmP0Text payload;
    if (lm_trans_identifier_payload(name, &payload) == 0) {
        return 1;
    }
    return lm_trans_write_text(file, payload);
}

static int lm_trans_registry_identifier_value(LmP0Text atom, LmP0Text *out_payload) {
    if (out_payload == 0) {
        return 0;
    }
    if (atom.length > 0U && (atom.data[0] == '"' || atom.data[0] == '\'')) {
        return 0;
    }
    return lm_trans_identifier_payload(atom, out_payload);
}

static int lm_trans_registry_literal_value(LmP0Text atom, LmP0Text *out_payload) {
    char quote;
    if (out_payload == 0) {
        return 0;
    }
    out_payload[0] = atom;
    if (atom.length < 2U) {
        return 1;
    }
    quote = atom.data[0];
    if ((quote == '"' || quote == '\'') && atom.data[atom.length - 1U] == quote) {
        out_payload->data = atom.data + 1U;
        out_payload->length = atom.length - 2U;
        return 1;
    }
    return lm_trans_identifier_payload(atom, out_payload);
}

static int lm_trans_registry_payload_is_null(LmP0Text atom) {
    LmP0Text payload;
    if (lm_trans_registry_identifier_value(atom, &payload) == 0) {
        return 0;
    }
    return payload.length == 4U && memcmp(payload.data, "NULL", 4U) == 0;
}

static char * lm_trans_text_copy_cstr(LmP0Text text) {
    char *copy;
    size_t length;
    length = text.length;
    copy = malloc(length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (length > 0U) {
        memcpy(copy, text.data, length);
    }
    copy[length] = '\0';
    return copy;
}

static char * lm_trans_registry_value_copy_cstr(LmP0Text value) {
    char *copy;
    size_t length;
    length = value.length;
    copy = lm_own_arena_new_zero(&lm_trans_registry.value_arena, length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (length != 0U) {
        memcpy(copy, value.data, length);
    }
    copy[length] = '\0';
    return copy;
}

static int lm_trans_registry_clone_text(LmP0Text source, LmP0Text *out_text) {
    char *copy;
    if (out_text == 0) {
        return 1;
    }
    out_text->data = "";
    out_text->length = 0U;
    if (source.length == 0U) {
        return 0;
    }
    if (source.data == 0) {
        return 1;
    }
    copy = lm_own_arena_copy_bytes(&lm_trans_registry.value_arena, source.data, source.length);
    if (copy == 0) {
        return 1;
    }
    out_text->data = copy;
    out_text->length = source.length;
    return 0;
}

static LmP0Text lm_trans_text_from_cstr(const char *text) {
    LmP0Text result;
    if (text == 0) {
        result.data = "";
        result.length = 0U;
    }
    if (text != 0) {
        result.data = text;
        result.length = strlen(text);
    }
    return result;
}

static int lm_trans_text_all_char(LmP0Text text, char ch) {
    size_t i;
    size_t length;
    length = text.length;
    if (length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < length) {
        if (text.data[i] != ch) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_trans_write_all(FILE *file, const char *data, size_t length) {
    if (fwrite(data, 1U, length, file) == length) {
        return 0;
    }
    return 1;
}

static int lm_trans_put(FILE *file, const char *text) {
    if (fputs(text, file) < 0) {
        return 1;
    }
    return 0;
}

static FILE * lm_trans_prelude_file(FILE *fallback) {
    if (lm_trans_prelude_output != 0) {
        return lm_trans_prelude_output;
    }
    return fallback;
}

static int lm_trans_write_text(FILE *file, LmP0Text text) {
    return lm_trans_write_all(file, text.data, text.length);
}

static int lm_trans_emit_indent(FILE *file, unsigned indent) {
    unsigned i;
    i = 0U;
    while (i < indent) {
        if (lm_trans_put(file, "    ") != 0) {
            return 1;
        }
        i = i + 1U;
    }
    return 0;
}

static const LmP0Field * lm_trans_nth_field(const LmP0Structure *structure, size_t index) {
    const LmP0Field *field;
    size_t i;
    field = structure->first_field;
    i = 0U;
    while (field != 0 && i < index) {
        field = field -> next;
        i = i + 1U;
    }
    return field;
}

static int lm_trans_node_is_ignored(const LmP0Node *node) {
    return node == 0 || (node -> flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static int lm_trans_node_is_positional_skip(const LmP0Node *node) {
    return node != 0 && (node -> flags & LM_P0_NODE_POSITIONAL_SKIP) != 0U;
}

static int lm_trans_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text) {
    const LmP0Field *field;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = trailer->body.first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    out_text[0] = field->value->as.atom;
    return 1;
}

static const char * lm_trans_symbol_class_name(const char *class_name) {
    if (class_name != 0) {
        return class_name;
    }
    return "symbol";
}

static int lm_trans_symbol_class_is(const char *class_name, const char *expected) {
    return class_name != 0 && expected != 0 && strcmp(class_name, expected) == 0;
}

static int lm_trans_symbol_is(const LmTransSymbol *symbol, const char *class_name) {
    return symbol != 0 && lm_trans_symbol_class_is(symbol -> class_name, class_name) != 0;
}

static int lm_trans_symbol_is_executable_callable(const LmTransSymbol *symbol) {
    return symbol != 0 && symbol -> has_callable_shape != 0 && (lm_trans_symbol_is(symbol, "function") != 0 || lm_trans_symbol_is(symbol, "procedure") != 0 || lm_trans_symbol_is(symbol, "closure") != 0);
}

static int lm_trans_symbol_is_value_callable(const LmTransSymbol *symbol) {
    return lm_trans_symbol_is_executable_callable(symbol) != 0 && symbol -> callable_returns_value != 0;
}

static void lm_trans_symbol_destroy_fields(LmTransSymbol *symbol) {
    if (symbol != 0) {
        lm_trans_text_ref_destroy(&symbol->name);
        free(symbol -> name_storage);
        symbol->name_storage = 0;
        free(symbol -> c_name_storage);
        symbol->c_name_storage = 0;
        symbol->c_name.data = "";
        symbol->c_name.length = 0U;
        symbol->has_c_name = 0;
        free(symbol -> env_arg_storage);
        symbol->env_arg_storage = 0;
        symbol->env_arg.data = "";
        symbol->env_arg.length = 0U;
        symbol->has_env_arg = 0;
        free(symbol -> closure_call_name_storage);
        symbol->closure_call_name_storage = 0;
        symbol->closure_call_name.data = "";
        symbol->closure_call_name.length = 0U;
        symbol->has_closure_call_name = 0;
        lm_own_ptr_stack_destroy(&symbol->param_names);
        symbol->has_signature = 0;
        symbol->callable_params_node = 0;
        symbol->callable_return_node = 0;
        symbol->has_callable_shape = 0;
        symbol->callable_returns_value = 0;
        symbol->callable_is_struct_return = 0;
    }
}

static void lm_trans_symbol_destroy_fields_any(void *object) {
    LmTransSymbol *symbol;
    symbol = object;
    lm_trans_symbol_destroy_fields(symbol);
}

static void lm_trans_symbol_destroy(LmTransSymbol *symbol) {
    lm_own_delete(symbol, lm_trans_symbol_destroy_fields_any);
}

static void lm_trans_symbol_delete_any(void *object) {
    LmTransSymbol *symbol;
    symbol = object;
    lm_trans_symbol_destroy(symbol);
}

static void lm_trans_free_any(void *object) {
    free(object);
}
