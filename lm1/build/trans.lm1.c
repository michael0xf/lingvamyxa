#include <stddef.h>
#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LM_TRANS_REGISTRY_CLONE_STRUCTURE 1
#define LM_TRANS_REGISTRY_CLONE_TRAILER 2

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
typedef struct LmL4Loader LmL4Loader;
typedef struct LmTransIdentifierRelation LmTransIdentifierRelation;
typedef struct LmTransIdentifierCard LmTransIdentifierCard;
typedef struct LmTransNamespace LmTransNamespace;
typedef struct LmTransRegistryCloneFrame LmTransRegistryCloneFrame;
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
    LmOwnPtrStack *allocations;
    LmOwnPtrStack *allocation_descriptors;
    LmOwnPtrStack *lazy_edges;
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
struct LmL4Column {
    const LmP0Text *name;
    const LmP0Text *descriptors[16U];
    size_t descriptor_count;
};
struct LmL4Loader {
    const char *error_prefix;
    int (*push_row)(void *context, const LmP0Text *table_atom, const LmP0Text *key_atom, const LmP0Node *payload_node);
    int (*push_cell)(void *context, const LmP0Text *table_name, const LmL4Column *column, int split_by_column, const LmP0Text *key_atom, const LmP0Node *payload_node);
    int (*note_key)(void *context, const LmP0Text *table_name, const LmL4Column *column, const LmP0Text *key_atom);
    int (*push_column_metadata)(void *context, const LmP0Text *table_name, LmL4Column **columns, size_t column_count);
    int (*join_table)(void *context, const LmP0Text *source_table, const LmP0Text *target_table);
    int (*dispatch_frame)(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
};
struct LmTransIdentifierRelation {
    const char *name;
    LmOwnPtrStack *symbols;
    LmTransIdentifierRelation *next;
};
struct LmTransIdentifierCard {
    LmP0Text *name;
    char *name_storage;
    unsigned long hash;
    LmOwnPtrStack *symbols;
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
    LmTransIdentifierTable *identifiers;
    LmOwnArena *value_arena;
    LmOwnPtrStack *loaded_paths;
    char *source_path;
    size_t loaded_fact_count;
    int loaded;
    LmL4Loader *l4_loader;
    LmOwnPtrStack *l4_frame_pointer_bindings;
    LmOwnPtrStack *l4_root_frame_pointer_bindings;
    LmOwnPtrStack *l4_atom_pointer_bindings;
    LmOwnPtrStack *l4_payload_pointer_bindings;
    LmOwnPtrStack *binding_pointer_bindings;
} LmTransRegistry;
typedef struct LmTransRegistryFact {
    char *table;
    char *key;
    char *payload;
    const LmP0Node *payload_node;
} LmTransRegistryFact;
struct LmTransRegistryCloneFrame {
    int phase;
    const LmP0Structure *source_structure;
    LmP0Structure *copy_structure;
    const LmP0Trailer *source_trailer;
    LmP0Trailer **copy_trailer_slot;
};
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
    LmP0Text *type_head;
    LmP0Text *name;
    LmP0Text *array_head;
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
    LmP0Text *class_name;
    size_t address_depth;
    int is_const;
} LmTransL4CallableType;
typedef struct LmTransSymbol {
    LmP0Text *name;
    char *name_storage;
    const char *class_name;
    LmP0Text *c_name;
    char *c_name_storage;
    int has_c_name;
    LmP0Text *env_arg;
    char *env_arg_storage;
    int has_env_arg;
    LmP0Text *closure_call_name;
    char *closure_call_name_storage;
    int has_closure_call_name;
    unsigned depth;
    LmOwnPtrStack *param_names;
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
    LmP0Text *name;
    const LmTransSymbol *signature;
    int is_closure;
};
struct LmTransExprLoweredRange {
    LmOwnPtrStack *pieces;
    size_t index;
};
struct LmTransExprStack {
    LmOwnPtrStack *jobs;
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
    LmP0Text *text;
};
struct LmTransExprPiece {
    int (*emit)(FILE *file, LmTransExprStack *stack, LmTransExprLoweredRange *lowered, const LmTransExprPiece *piece, const LmTransNamespace *namespace_, int *out_suspend);
    int leading_space;
    const LmP0Node *node;
    LmTransExprAtomLowering *atom;
    const LmP0Field *first;
    const LmP0Field *stop;
};
struct LmTransExprJob {
    int (*run)(FILE *file, LmTransExprStack *stack, LmTransExprJob *job, const LmTransNamespace *namespace_);
    void (*destroy)(LmTransExprJob *job);
    const char *text;
    LmP0Text *name_text;
    const LmP0Node *node;
    const LmP0Frame *frame;
    LmTransExprRangeJob *range;
    LmTransExprCallArgsJob *call_args;
    LmTransExprLoweredRange *lowered_range;
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
    LmOwnPtrStack *jobs;
};
struct LmTransStatementJob {
    int (*run)(FILE *file, LmTransStatementStack *stack, LmTransStatementJob *job, LmTransNamespace *namespace_);
    void (*destroy)(LmTransStatementJob *job);
    LmTransStatementListJob *list;
    LmTransStatementNodeJob *node;
    LmTransStatementFrameJob *frame;
    LmTransStatementTextJob *text;
    LmTransStatementSyncLeaveJob *sync_leave;
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
    LmOwnPtrStack *previous_cleanups;
    LmOwnPtrStack *previous_loops;
    int has_previous_control_stacks;
} LmTransFunctionState;
struct LmTransFunctionHeader {
    const LmP0Frame *frame;
    LmP0Text *name;
    LmP0Text *c_name;
    LmP0Text *env_type_name;
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
    int is_lazy_descriptor;
};
typedef struct LmTransCapture {
    LmP0Text *name;
    LmP0Text *type_head;
    const LmP0Node *type_node;
    size_t pointer_depth;
    int type_is_head;
} LmTransCapture;
typedef struct LmTransHoistedFunction {
    LmTransFunctionHeader *function;
    LmOwnPtrStack *captures;
    char *c_name_storage;
    char *env_type_storage;
    char *env_var_storage;
    char *closure_call_storage;
    LmP0Text *env_var_name;
    LmP0Text *closure_call_name;
} LmTransHoistedFunction;
typedef struct LmTransBinding {
    int (*call_lowering)(const LmP0Text *head, const LmTransSymbol *symbol, LmTransCallLowering *out);
    int (*expr_frame)(FILE *file, LmTransExprStack *stack, const LmP0Frame *frame, const LmTransNamespace *namespace_);
    int (*expr_emit)(FILE *file, const LmTransExprAtomLowering *lowering, const LmTransNamespace *namespace_);
    int (*expr_state)(const LmTransExprAtomLowering *lowering, const LmP0Node *node, const LmP0Node **previous_operand, int *expect_field_name, int *expect_c_field_name, int *c_dot_path);
    int (*statement_frame)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
    int (*function_receiver)(const LmP0Frame *frame, int is_external, LmTransFunctionHeader *out);
    int (*type_emit)(FILE *file, const LmP0Node *type_node);
    int (*type_structure_value_alloc)(FILE *file, unsigned indent, const LmP0Node *type_node, const LmP0Text *target_name, const LmP0Structure *value, int *out_consumed, int *out_needs_null_check);
    int (*type_structure_value_fill)(FILE *file, unsigned indent, const LmP0Node *type_node, const LmP0Text *target_name, const LmP0Structure *value, const LmTransNamespace *namespace_, int *out_consumed);
    int (*expr_segment_materializer)(FILE *file, LmTransExprStack *stack, const LmTransExprSegment *segment, const LmTransNamespace *namespace_, int *out_consumed);
    int (*atom_statement)(FILE *file, const LmP0Node *node, unsigned indent, LmTransNamespace *namespace_);
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
    int (*atom)(const LmP0Text *atom, int allow_node_cells);
};
struct LmTransTopLevelItem {
    int (*declare)(LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_before_functions)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_function)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int emits_top_level;
    const LmP0Node *node;
    const LmP0Frame *frame;
    LmTransFunctionHeader *function;
    int (*emit_prototype)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
    int (*emit_after_prototypes)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
};
struct LmTransNamespace {
    LmOwnPtrStack *items;
    LmTransIdentifierTable *identifiers;
    const LmTransIdentifierTable *registry_identifiers;
    unsigned depth;
    LmOwnPtrStack *cleanups;
    LmOwnPtrStack *loops;
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
typedef int (*LmL4PushRow)(void *context, const LmP0Text *table_atom, const LmP0Text *key_atom, const LmP0Node *payload_node);
typedef int (*LmL4PushCell)(void *context, const LmP0Text *table_name, const LmL4Column *column, int split_by_column, const LmP0Text *key_atom, const LmP0Node *payload_node);
typedef int (*LmL4NoteKey)(void *context, const LmP0Text *table_name, const LmL4Column *column, const LmP0Text *key_atom);
typedef int (*LmL4PushColumnMetadata)(void *context, const LmP0Text *table_name, LmL4Column **columns, size_t column_count);
typedef int (*LmL4JoinTable)(void *context, const LmP0Text *source_table, const LmP0Text *target_table);
typedef int (*LmL4FrameReceiver)(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
typedef int (*LmTransExprSegmentMaterializer)(FILE *file, LmTransExprStack *stack, const LmTransExprSegment *segment, const LmTransNamespace *namespace_, int *out_consumed);
typedef int (*LmTransCallLoweringHandler)(const LmP0Text *head, const LmTransSymbol *symbol, LmTransCallLowering *out);
typedef int (*LmTransExprFrameHandler)(FILE *file, LmTransExprStack *stack, const LmP0Frame *frame, const LmTransNamespace *namespace_);
typedef int (*LmTransExprAtomEmitHandler)(FILE *file, const LmTransExprAtomLowering *lowering, const LmTransNamespace *namespace_);
typedef int (*LmTransExprAtomStateHandler)(const LmTransExprAtomLowering *lowering, const LmP0Node *node, const LmP0Node **previous_operand, int *expect_field_name, int *expect_c_field_name, int *c_dot_path);
typedef int (*LmTransExprPieceEmitHandler)(FILE *file, LmTransExprStack *stack, LmTransExprLoweredRange *lowered, const LmTransExprPiece *piece, const LmTransNamespace *namespace_, int *out_suspend);
typedef int (*LmTransExprJobHandler)(FILE *file, LmTransExprStack *stack, LmTransExprJob *job, const LmTransNamespace *namespace_);
typedef void (*LmTransExprJobDestroyHandler)(LmTransExprJob *job);
typedef int (*LmTransStatementJobHandler)(FILE *file, LmTransStatementStack *stack, LmTransStatementJob *job, LmTransNamespace *namespace_);
typedef void (*LmTransStatementJobDestroyHandler)(LmTransStatementJob *job);
typedef int (*LmTransStatementFrameHandler)(FILE *file, LmTransStatementStack *stack, const LmP0Frame *frame, unsigned indent, LmTransNamespace *namespace_);
typedef int (*LmTransAtomStatementHandler)(FILE *file, const LmP0Node *node, unsigned indent, LmTransNamespace *namespace_);
typedef int (*LmTransFunctionHeaderReceiver)(const LmP0Frame *frame, int is_external, LmTransFunctionHeader *out);
typedef int (*LmTransTypeEmitReceiver)(FILE *file, const LmP0Node *type_node);
typedef int (*LmTransTypeStructureValueAllocReceiver)(FILE *file, unsigned indent, const LmP0Node *type_node, const LmP0Text *target_name, const LmP0Structure *value, int *out_consumed, int *out_needs_null_check);
typedef int (*LmTransTypeStructureValueFillReceiver)(FILE *file, unsigned indent, const LmP0Node *type_node, const LmP0Text *target_name, const LmP0Structure *value, const LmTransNamespace *namespace_, int *out_consumed);
typedef int (*LmTransTopLevelDeclareHandler)(LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
typedef int (*LmTransTopLevelEmitHandler)(FILE *file, LmTransNamespace *namespace_, const LmTransTopLevelItem *item);
typedef int (*LmTransL4FrameHandler)(const LmP0Frame *frame, int allow_node_cells);
typedef int (*LmTransL4AtomHandler)(const LmP0Text *atom, int allow_node_cells);
typedef int (*LmTransL4PayloadFrameHandler)(FILE *output, const LmP0Frame *frame, LmTransNamespace *namespace_);

void * lm_own_new_zero(size_t size);
void * lm_own_resize(void *object, size_t size);
char * lm_own_copy_bytes(const char *source, size_t length);
void * lm_own_array_new_zero(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_allocation_descriptor(const void *address);
void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_delete_plain(void *object);
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
int lm_own_arena_init(LmOwnArena *arena);
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
LmP0Node * lm_p0_document_mutable_root(LmP0Document *document);
const LmP0Diagnostic * lm_p0_document_diagnostic(const LmP0Document *document);
const LmP0Structure * lm_p0_node_structure(const LmP0Node *node);
const LmP0Frame * lm_p0_node_frame(const LmP0Node *node);
const LmP0Text * lm_p0_node_atom(const LmP0Node *node);
const LmP0Trailer * lm_p0_structure_trailer(const LmP0Structure *structure);
const LmP0Text * lm_p0_frame_head(const LmP0Frame *frame);
const LmP0Structure * lm_p0_frame_body(const LmP0Frame *frame);
const LmP0Trailer * lm_p0_frame_trailer(const LmP0Frame *frame);
const LmP0Text * lm_p0_trailer_spelling(const LmP0Trailer *trailer);
const LmP0Structure * lm_p0_trailer_body(const LmP0Trailer *trailer);
const char * lm_p0_node_kind_class_name(LmP0NodeKind kind);
char * lm_p0_dump_alloc(const LmP0Document *document);
void lm_p0_free(void *ptr);
static LmTransIdentifierCard * lm_trans_identifier_table_find_card(const LmTransIdentifierTable *table, const LmP0Text *name);
static LmTransIdentifierCard * lm_trans_identifier_table_get_card(LmTransIdentifierTable *table, const LmP0Text *name);
static LmTransIdentifierRelation * lm_trans_identifier_card_find_relation(const LmTransIdentifierCard *card, const char *relation_name);
static LmTransIdentifierRelation * lm_trans_identifier_card_get_relation(LmTransIdentifierCard *card, const char *relation_name);
static int lm_trans_binding_resolve(const char *binding, LmTransBinding *out);
static int lm_trans_text_equals(const LmP0Text *text, const char *value);
static int lm_trans_text_same(const LmP0Text *left, const LmP0Text *right);
static int lm_trans_text_starts_with(const LmP0Text *text, const char *prefix);
static int lm_trans_identifier_payload(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_identifier_same(const LmP0Text *left, const LmP0Text *right);
static int lm_trans_registry_identifier_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_registry_literal_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_registry_payload_is_null(const LmP0Text *atom);
static char * lm_trans_text_copy_cstr(const LmP0Text *text);
static char * lm_trans_text_copy_cstr_from_cstr(const char *text);
static char * lm_trans_registry_value_copy_cstr(const LmP0Text *value);
static int lm_trans_registry_clone_text(const LmP0Text *source, LmP0Text *out_text);
static LmP0Text * lm_trans_text_ref_new(const LmP0Text *text);
static LmP0Text * lm_trans_text_ref_new_cstr(const char *text);
static void lm_trans_text_ref_destroy(LmP0Text **text);
static LmP0Text * lm_trans_text_from_cstr(const char *text);
static int lm_trans_text_assign_cstr(LmP0Text *out_text, const char *text);
static int lm_trans_registry_init(void);
static int lm_trans_emit_identifier(FILE *file, const LmP0Text *name);
static int lm_trans_l4_callable_type_from_node(const LmP0Node *node, LmTransL4CallableType *out);
static int lm_trans_collect_abi_params(LmTransAbiParam **params, size_t capacity, size_t *out_count, const LmTransNamespace *namespace_, const char *owner_name, const char *error_name);
static int lm_trans_emit_abi_params(FILE *file, LmTransAbiParam **params, size_t count);
static int lm_trans_emit_abi_return_type(FILE *file, const LmTransNamespace *namespace_, const char *owner_name, const char *error_name);
static int lm_trans_emit_type_name(FILE *file, const LmP0Text *name);
static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node);
static int lm_trans_registry_latest_size_payload(const LmOwnPtrStack *stack, const LmP0Text *key, size_t default_value, size_t *out_value);
static int lm_trans_text_all_char(const LmP0Text *text, char ch);
static int lm_trans_write_all(FILE *file, const char *data, size_t length);
static int lm_trans_put(FILE *file, const char *text);
static FILE * lm_trans_prelude_file(FILE *fallback);
static int lm_trans_write_text(FILE *file, const LmP0Text *text);
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

static LmOwnPtrStack * lm_l4_seen_tables_get(void);
static void lm_l4_seen_tables_set(LmOwnPtrStack *seen_tables);
static int lm_l4_text_equals(const LmP0Text *text, const char *value);
static int lm_l4_text_slice_equals(const char *data, size_t length, const char *value);
static int lm_l4_text_slice_same(const char *left_data, size_t left_length, const char *right_data, size_t right_length);
static int lm_l4_node_is_ignored(const LmP0Node *node);
static const LmP0Structure * lm_l4_node_structure(const LmP0Node *node);
static const LmP0Frame * lm_l4_node_frame(const LmP0Node *node);
static const LmP0Text * lm_l4_node_atom(const LmP0Node *node);
static const LmP0Text * lm_l4_frame_head(const LmP0Frame *frame);
static const LmP0Structure * lm_l4_frame_body(const LmP0Frame *frame);
static const LmP0Text * lm_l4_trailer_spelling(const LmP0Trailer *trailer);
static const LmP0Structure * lm_l4_trailer_body(const LmP0Trailer *trailer);
static const char * lm_l4_error_prefix(const LmL4Loader *loader);
static void lm_l4_error(const LmL4Loader *loader, const char *message);
static const LmP0Field * lm_l4_nth_field(const LmP0Structure *structure, size_t index);
static int lm_l4_trailer_single_atom(const LmP0Trailer *trailer, const LmP0Text **out_text);
static int lm_l4_identifier_payload(const LmP0Text *atom, const char **out_data, size_t *out_length);
static int lm_l4_identifier_equals(const LmP0Text *atom, const char *value);
static int lm_l4_identifier_same(const LmP0Text *left, const LmP0Text *right);
static int lm_l4_frame_single_atom(const LmP0Frame *frame, const char *head, const LmP0Text **out_atom);
static int lm_l4_column_name(const LmP0Field *field, LmL4Column *out_column);
static void lm_l4_columns_destroy(LmL4Column **columns, size_t count);
static int lm_l4_columns_from_frame(const LmL4Loader *loader, const LmP0Frame *frame, LmL4Column **columns, size_t columns_capacity, size_t *out_count);
static int lm_l4_validate_named_trailer(const LmL4Loader *loader, const LmP0Frame *frame, const LmP0Text *expected_name);
static int lm_l4_row_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_rows_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmL4Column **columns, size_t column_count);
static int lm_l4_table_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_seen_table_add(LmOwnPtrStack *seen, const LmP0Text *table_name);
static int lm_l4_check_table_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static const LmP0Field * lm_l4_next_present_field(const LmP0Field *field);
static int lm_l4_join_header(const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body);
static int lm_l4_join_sources_into_target(const LmL4Loader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name);
static int lm_l4_join_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_check_join_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_l4_receiver_table(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_receiver_join(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_receiver_row(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_receiver_ignore(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_dispatch_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame);
static int lm_l4_load_rows(const LmL4Loader *loader, void *context, const LmP0Structure *structure);
static int lm_l4_load_root(const LmL4Loader *loader, void *context, const LmP0Node *root, int implicit_l4);

static LmOwnPtrStack *lm_l4_seen_tables;
static LmOwnPtrStack * lm_l4_seen_tables_get(void) {
    return lm_l4_seen_tables;
}

static void lm_l4_seen_tables_set(LmOwnPtrStack *seen_tables) {
    lm_l4_seen_tables = seen_tables;
}

static int lm_l4_text_equals(const LmP0Text *text, const char *value) {
    size_t length;
    if (text == 0 || value == 0) {
        return 0;
    }
    length = strlen(value);
    return text -> length == length && memcmp(text -> data, value, length) == 0;
}

static int lm_l4_text_slice_equals(const char *data, size_t length, const char *value) {
    size_t value_length;
    if (data == 0 || value == 0) {
        return 0;
    }
    value_length = strlen(value);
    return length == value_length && memcmp(data, value, length) == 0;
}

static int lm_l4_text_slice_same(const char *left_data, size_t left_length, const char *right_data, size_t right_length) {
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

static int lm_l4_node_is_ignored(const LmP0Node *node) {
    return node == 0 || (node -> flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static const LmP0Structure * lm_l4_node_structure(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }
    return node -> as -> structure;
}

static const LmP0Frame * lm_l4_node_frame(const LmP0Node *node) {
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    return node -> as -> frame;
}

static const LmP0Text * lm_l4_node_atom(const LmP0Node *node) {
    if (node == 0 || (node -> kind != LM_P0_NODE_ATOM && node -> kind != LM_P0_NODE_DISABLED)) {
        return 0;
    }
    return node -> as -> atom;
}

static const LmP0Text * lm_l4_frame_head(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> head;
}

static const LmP0Structure * lm_l4_frame_body(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return frame -> body;
}

static const LmP0Text * lm_l4_trailer_spelling(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> spelling;
}

static const LmP0Structure * lm_l4_trailer_body(const LmP0Trailer *trailer) {
    if (trailer == 0) {
        return 0;
    }
    return trailer -> body;
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

static int lm_l4_trailer_single_atom(const LmP0Trailer *trailer, const LmP0Text **out_text) {
    const LmP0Field *field;
    const LmP0Text *atom;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = lm_l4_trailer_body(trailer) -> first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    atom = lm_l4_node_atom(field -> value);
    if (atom == 0) {
        return 0;
    }
    *(out_text) = atom;
    return 1;
}

static int lm_l4_identifier_payload(const LmP0Text *atom, const char **out_data, size_t *out_length) {
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

static int lm_l4_identifier_equals(const LmP0Text *atom, const char *value) {
    const char *data;
    size_t length;
    if (lm_l4_identifier_payload(atom, &data, &length) == 0) {
        return 0;
    }
    return lm_l4_text_slice_equals(data, length, value);
}

static int lm_l4_identifier_same(const LmP0Text *left, const LmP0Text *right) {
    const char *left_data;
    const char *right_data;
    size_t left_length;
    size_t right_length;
    if (lm_l4_identifier_payload(left, &left_data, &left_length) == 0 || lm_l4_identifier_payload(right, &right_data, &right_length) == 0) {
        return 0;
    }
    return lm_l4_text_slice_same(left_data, left_length, right_data, right_length);
}

static int lm_l4_frame_single_atom(const LmP0Frame *frame, const char *head, const LmP0Text **out_atom) {
    const LmP0Field *field;
    const LmP0Text *frame_head;
    const LmP0Text *atom;
    if (frame == 0 || out_atom == 0) {
        return 0;
    }
    frame_head = lm_l4_frame_head(frame);
    if (head != 0 && (frame_head == 0 || lm_l4_text_equals(frame_head, head) == 0)) {
        return 0;
    }
    field = lm_l4_nth_field(lm_l4_frame_body(frame), 0U);
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return - 1;
    }
    atom = lm_l4_node_atom(field -> value);
    if (atom == 0) {
        return - 1;
    }
    *(out_atom) = atom;
    return 1;
}

static int lm_l4_column_name(const LmP0Field *field, LmL4Column *out_column) {
    const LmP0Node *node;
    const LmP0Frame *node_frame;
    const LmP0Structure *node_structure;
    const LmP0Field *body_field;
    const LmP0Text *atom;
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
        atom = lm_l4_node_atom(node);
        if (atom == 0) {
            return - 1;
        }
        out_column->name = atom;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        node_frame = lm_l4_node_frame(node);
        if (lm_l4_frame_single_atom(node_frame, 0, &atom) <= 0) {
            return - 1;
        }
        out_column->name = atom;
        out_column->descriptors[0] = lm_l4_frame_head(node_frame);
        out_column->descriptor_count = 1U;
        return 1;
    }
    if (node -> kind != LM_P0_NODE_STRUCTURE) {
        return - 1;
    }
    node_structure = lm_l4_node_structure(node);
    if (node_structure == 0) {
        return - 1;
    }
    body_field = node_structure -> first_field;
    while (body_field != 0 && lm_l4_node_is_ignored(body_field -> value) != 0) {
        body_field = body_field -> next;
    }
    if (body_field == 0 || body_field -> value == 0 || body_field -> value -> kind != LM_P0_NODE_ATOM) {
        return - 1;
    }
    atom = lm_l4_node_atom(body_field -> value);
    if (atom == 0) {
        return - 1;
    }
    out_column->name = atom;
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
            atom = lm_l4_node_atom(body_field -> value);
            if (atom == 0) {
                return - 1;
            }
            out_column->descriptors[descriptor_count] = atom;
            descriptor_count = descriptor_count + 1U;
        }
        body_field = body_field -> next;
    }
    out_column->descriptor_count = descriptor_count;
    return 1;
}

static void lm_l4_columns_destroy(LmL4Column **columns, size_t count) {
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

static int lm_l4_columns_from_frame(const LmL4Loader *loader, const LmP0Frame *frame, LmL4Column **columns, size_t columns_capacity, size_t *out_count) {
    const LmP0Field *field;
    LmL4Column *column;
    size_t count;
    int status;
    if (frame == 0 || columns == 0 || out_count == 0 || columns_capacity == 0U || lm_l4_text_equals(lm_l4_frame_head(frame), "columns") == 0) {
        return 0;
    }
    *(out_count) = 0U;
    count = 0U;
    field = lm_l4_frame_body(frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_l4_node_is_ignored(field -> value) == 0) {
            if (count >= columns_capacity) {
                lm_l4_error(loader, "table has too many columns");
                return - 1;
            }
            column = lm_own_new_zero(sizeof(LmL4Column));
            if (column == 0) {
                lm_l4_error(loader, "out of memory while reading table columns");
                return - 1;
            }
            status = lm_l4_column_name(field, column);
            if (status <= 0) {
                lm_own_delete(column, 0);
                lm_l4_error(loader, "columns expects atoms or anonymous descriptor structures");
                return - 1;
            }
            columns[count] = column;
            count = count + 1U;
            *(out_count) = count;
        }
        field = field -> next;
    }
    if (count < 2U) {
        lm_l4_error(loader, "table expects at least two columns");
        return - 1;
    }
    if (lm_l4_identifier_equals(columns[0] -> name, "class") == 0) {
        lm_l4_error(loader, "first table column must be class");
        return - 1;
    }
    *(out_count) = count;
    return 1;
}

static int lm_l4_validate_named_trailer(const LmL4Loader *loader, const LmP0Frame *frame, const LmP0Text *expected_name) {
    const LmP0Text *actual;
    if (frame == 0 || frame -> trailer == 0) {
        return 0;
    }
    if (lm_l4_text_equals(lm_l4_trailer_spelling(frame -> trailer), "end") == 0) {
        return 0;
    }
    if (lm_l4_trailer_single_atom(frame -> trailer, &actual) == 0) {
        lm_l4_error(loader, "end trailer expects exactly one target name");
        return 1;
    }
    if (lm_l4_identifier_same(actual, expected_name) == 0 && lm_l4_identifier_same(actual, lm_l4_frame_head(frame)) == 0) {
        lm_l4_error(loader, "end trailer target does not match head/name");
        return 1;
    }
    return 0;
}

static int lm_l4_row_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field *table_field;
    const LmP0Field *key_field;
    const LmP0Field *payload_field;
    const LmP0Text *table_atom;
    const LmP0Text *key_atom;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "row") == 0) {
        return 0;
    }
    table_field = lm_l4_nth_field(lm_l4_frame_body(frame), 0U);
    key_field = lm_l4_nth_field(lm_l4_frame_body(frame), 1U);
    payload_field = lm_l4_nth_field(lm_l4_frame_body(frame), 2U);
    if (table_field == 0 || key_field == 0 || payload_field == 0 || payload_field -> next != 0 || table_field -> value == 0 || key_field -> value == 0 || payload_field -> value == 0 || table_field -> value -> kind != LM_P0_NODE_ATOM || key_field -> value -> kind != LM_P0_NODE_ATOM) {
        lm_l4_error(loader, "row expects table and key atoms plus one payload field");
        return - 1;
    }
    table_atom = lm_l4_node_atom(table_field -> value);
    key_atom = lm_l4_node_atom(key_field -> value);
    if (table_atom == 0 || key_atom == 0) {
        lm_l4_error(loader, "row expects table and key atoms plus one payload field");
        return - 1;
    }
    if (loader == 0 || loader -> push_row == 0) {
        lm_l4_error(loader, "row consumer is not configured");
        return - 1;
    }
    if (loader->push_row(context, table_atom, key_atom, payload_field -> value) != 0) {
        return - 1;
    }
    return 1;
}

static int lm_l4_rows_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmL4Column **columns, size_t column_count) {
    const LmP0Field *field;
    const LmP0Node *key_node;
    const LmP0Node *cell_node;
    const LmP0Text *key_atom;
    size_t field_index;
    size_t column_index;
    int split_by_column;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "rows") == 0) {
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
    key_atom = 0;
    split_by_column = column_count != 2U;
    field = lm_l4_frame_body(frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_l4_node_is_ignored(field -> value) == 0) {
            column_index = field_index % column_count;
            if (column_index == 0U) {
                key_node = field -> value;
                if (key_node -> kind != LM_P0_NODE_ATOM) {
                    lm_l4_error(loader, "table rows currently expect atom cells in the key column");
                    return - 1;
                }
                key_atom = lm_l4_node_atom(key_node);
                if (key_atom == 0) {
                    lm_l4_error(loader, "table rows currently expect atom cells in the key column");
                    return - 1;
                }
                if (loader -> note_key != 0 && loader->note_key(context, table_name, columns[0], key_atom) != 0) {
                    return - 1;
                }
            }
            if (column_index != 0U) {
                cell_node = field -> value;
                if (key_node == 0 || key_atom == 0 || loader->push_cell(context, table_name, columns[column_index], split_by_column, key_atom, cell_node) != 0) {
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
    const LmP0Frame *node_frame;
    LmL4Column **columns;
    const LmP0Text *table_name;
    size_t column_count;
    int have_name;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "table") == 0) {
        return 0;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_l4_error(loader, "out of memory while reading table columns");
        return - 1;
    }
    have_name = 0;
    have_columns = 0;
    have_rows = 0;
    column_count = 0U;
    table_name = 0;
    field = lm_l4_frame_body(frame) -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_l4_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_l4_error(loader, "table body expects name/columns/rows frames");
                lm_l4_columns_destroy(columns, column_count);
                return - 1;
            }
            node_frame = lm_l4_node_frame(node);
            status = lm_l4_frame_single_atom(node_frame, "name", &table_name);
            if (status < 0) {
                lm_l4_error(loader, "table name expects exactly one atom");
                lm_l4_columns_destroy(columns, column_count);
                return - 1;
            }
            if (status > 0) {
                have_name = 1;
                field = field -> next;
                continue;
            }
            if (lm_l4_text_equals(lm_l4_frame_head(node_frame), "columns") != 0) {
                if (have_name == 0) {
                    lm_l4_error(loader, "table columns must appear after name");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                status = lm_l4_columns_from_frame(loader, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && loader->push_column_metadata(context, table_name, columns, column_count) != 0) {
                    lm_l4_error(loader, "cannot store table column metadata");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                have_columns = 1;
                field = field -> next;
                continue;
            }
            if (lm_l4_text_equals(lm_l4_frame_head(node_frame), "rows") != 0) {
                if (have_name == 0 || have_columns == 0) {
                    lm_l4_error(loader, "table rows must appear after name and columns");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                status = lm_l4_rows_from_frame(loader, context, node_frame, table_name, columns, column_count);
                if (status <= 0) {
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                have_rows = 1;
                field = field -> next;
                continue;
            }
            lm_l4_error(loader, "table body expects name/columns/rows frames");
            lm_l4_columns_destroy(columns, column_count);
            return - 1;
        }
        field = field -> next;
    }
    if (have_name == 0 || have_columns == 0 || have_rows == 0) {
        lm_l4_error(loader, "table requires name, columns and rows");
        lm_l4_columns_destroy(columns, column_count);
        return - 1;
    }
    if (lm_l4_validate_named_trailer(loader, frame, table_name) != 0) {
        lm_l4_columns_destroy(columns, column_count);
        return - 1;
    }
    lm_l4_columns_destroy(columns, column_count);
    return 1;
}

static int lm_l4_seen_table_add(LmOwnPtrStack *seen, const LmP0Text *table_name) {
    const char *payload_data;
    char *name;
    size_t i;
    size_t payload_length;
    char *existing;
    if (seen == 0) {
        return - 1;
    }
    if (lm_l4_identifier_payload(table_name, &payload_data, &payload_length) == 0) {
        return - 1;
    }
    i = 0U;
    while (i < seen -> count) {
        existing = lm_own_ptr_stack_at(seen, i);
        if (existing != 0 && lm_l4_text_slice_equals(payload_data, payload_length, existing) != 0) {
            return 1;
        }
        i = i + 1U;
    }
    name = lm_own_new_zero(payload_length + 1U);
    if (name == 0) {
        return - 1;
    }
    if (payload_length > 0U) {
        memcpy(name, payload_data, payload_length);
    }
    name[payload_length] = '\0';
    if (lm_own_ptr_stack_push(seen, name) != 0) {
        lm_own_delete(name, 0);
        return - 1;
    }
    return 0;
}

static int lm_l4_check_table_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *node_frame;
    const LmP0Text *table_name;
    int status;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "table") == 0) {
        return 0;
    }
    status = 0;
    table_name = 0;
    field = lm_l4_frame_body(frame) -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node == 0 || lm_l4_node_is_ignored(node) != 0 || node -> kind != LM_P0_NODE_FRAME) {
            field = field -> next;
            continue;
        }
        node_frame = lm_l4_node_frame(node);
        status = lm_l4_frame_single_atom(node_frame, "name", &table_name);
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

static const LmP0Field * lm_l4_next_present_field(const LmP0Field *field) {
    while (field != 0 && (field -> value == 0 || lm_l4_node_is_ignored(field -> value) != 0)) {
        field = field -> next;
    }
    return field;
}

static int lm_l4_join_header(const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body) {
    const LmP0Field *field;
    const LmP0Structure *sources;
    const LmP0Text *target;
    const LmP0Structure *body;
    if (frame == 0 || out_sources == 0 || out_target == 0 || out_body == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "join") == 0) {
        return 0;
    }
    *(out_sources) = 0;
    *(out_target) = 0;
    *(out_body) = 0;
    field = lm_l4_next_present_field(lm_l4_frame_body(frame) -> first_field);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return - 1;
    }
    sources = lm_l4_node_structure(field -> value);
    if (sources == 0 || lm_l4_next_present_field(sources -> first_field) == 0) {
        return - 1;
    }
    field = lm_l4_next_present_field(field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return - 1;
    }
    target = lm_l4_node_atom(field -> value);
    if (target == 0) {
        return - 1;
    }
    field = lm_l4_next_present_field(field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return - 1;
    }
    body = lm_l4_node_structure(field -> value);
    if (body == 0) {
        return - 1;
    }
    *(out_sources) = sources;
    *(out_target) = target;
    *(out_body) = body;
    return 1;
}

static int lm_l4_join_sources_into_target(const LmL4Loader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Text *source_name;
    if (loader == 0 || loader -> join_table == 0) {
        lm_l4_error(loader, "join consumer is not configured");
        return - 1;
    }
    if (sources == 0 || target_name == 0) {
        return - 1;
    }
    field = sources -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_l4_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_ATOM) {
                lm_l4_error(loader, "join source list currently expects table name atoms");
                return - 1;
            }
            source_name = lm_l4_node_atom(node);
            if (source_name == 0 || loader->join_table(context, source_name, target_name) != 0) {
                return - 1;
            }
        }
        field = field -> next;
    }
    return 0;
}

static int lm_l4_join_from_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Frame *node_frame;
    const LmP0Structure *sources;
    const LmP0Structure *body;
    LmL4Column **columns;
    const LmP0Text *target_name;
    const LmP0Frame *rows_frame;
    size_t column_count;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    field = 0;
    status = lm_l4_join_header(frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status == 0) {
            return 0;
        }
        lm_l4_error(loader, "join expects (sourceTables...) targetName and a table fragment body");
        return - 1;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_l4_error(loader, "out of memory while reading join columns");
        return - 1;
    }
    have_columns = 0;
    have_rows = 0;
    rows_frame = 0;
    column_count = 0U;
    field = lm_l4_next_present_field(body -> first_field);
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_l4_node_is_ignored(node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_l4_error(loader, "join body expects columns/rows frames");
                lm_l4_columns_destroy(columns, column_count);
                return - 1;
            }
            node_frame = lm_l4_node_frame(node);
            if (lm_l4_text_equals(lm_l4_frame_head(node_frame), "columns") != 0) {
                if (have_columns != 0) {
                    lm_l4_error(loader, "join expects one columns frame");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                status = lm_l4_columns_from_frame(loader, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && loader->push_column_metadata(context, target_name, columns, column_count) != 0) {
                    lm_l4_error(loader, "cannot store join target column metadata");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                have_columns = 1;
                field = lm_l4_next_present_field(field -> next);
                continue;
            }
            if (lm_l4_text_equals(lm_l4_frame_head(node_frame), "rows") != 0) {
                if (have_rows != 0) {
                    lm_l4_error(loader, "join expects at most one rows frame");
                    lm_l4_columns_destroy(columns, column_count);
                    return - 1;
                }
                rows_frame = node_frame;
                have_rows = 1;
                field = lm_l4_next_present_field(field -> next);
                continue;
            }
            lm_l4_error(loader, "join body expects columns/rows frames");
            lm_l4_columns_destroy(columns, column_count);
            return - 1;
        }
        field = lm_l4_next_present_field(field -> next);
    }
    if (have_columns == 0) {
        lm_l4_error(loader, "join requires target columns");
        lm_l4_columns_destroy(columns, column_count);
        return - 1;
    }
    if (lm_l4_join_sources_into_target(loader, context, sources, target_name) != 0) {
        lm_l4_columns_destroy(columns, column_count);
        return - 1;
    }
    if (have_rows != 0) {
        status = lm_l4_rows_from_frame(loader, context, rows_frame, target_name, columns, column_count);
        if (status <= 0) {
            lm_l4_columns_destroy(columns, column_count);
            return - 1;
        }
    }
    if (lm_l4_validate_named_trailer(loader, frame, target_name) != 0) {
        lm_l4_columns_destroy(columns, column_count);
        return - 1;
    }
    lm_l4_columns_destroy(columns, column_count);
    return 1;
}

static int lm_l4_check_join_frame_unique(const LmL4Loader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    const LmP0Structure *sources;
    const LmP0Structure *body;
    const LmP0Text *target_name;
    int status;
    if (frame == 0 || lm_l4_text_equals(lm_l4_frame_head(frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    status = lm_l4_join_header(frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status < 0) {
            lm_l4_error(loader, "join expects (sourceTables...) targetName and a table fragment body");
            return - 1;
        }
        return 0;
    }
    status = lm_l4_seen_table_add(seen, target_name);
    if (status < 0) {
        lm_l4_error(loader, "cannot record join target name");
        return - 1;
    }
    if (status > 0) {
        lm_l4_error(loader, "duplicate table in one L4 schema");
        return - 1;
    }
    return 0;
}

static int lm_l4_receiver_table(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    LmOwnPtrStack *seen;
    int status;
    seen = lm_l4_seen_tables_get();
    if (seen != 0 && lm_l4_check_table_frame_unique(loader, frame, seen) != 0) {
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

static int lm_l4_receiver_join(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    LmOwnPtrStack *seen;
    int status;
    seen = lm_l4_seen_tables_get();
    if (seen != 0 && lm_l4_check_join_frame_unique(loader, frame, seen) != 0) {
        return 1;
    }
    status = lm_l4_join_from_frame(loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_l4_error(loader, "join receiver expects join frame");
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

static int lm_l4_receiver_ignore(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    if (loader != 0 || context != 0 || frame != 0) {
        return 0;
    }
    return 0;
}

static int lm_l4_dispatch_frame(const LmL4Loader *loader, void *context, const LmP0Frame *frame) {
    if (frame == 0) {
        return 1;
    }
    if (loader == 0 || loader -> dispatch_frame == 0) {
        lm_l4_error(loader, "registry body expects registered L4 receiver frames");
        return 1;
    }
    return loader->dispatch_frame(loader, context, frame);
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
    seen = 0;
    previous_seen = lm_l4_seen_tables_get();
    owns_seen = previous_seen == 0;
    if (owns_seen != 0) {
        seen = lm_own_new_zero(sizeof(seen[0]));
        if (seen == 0) {
            lm_l4_error(loader, "cannot allocate table duplicate tracker");
            return 1;
        }
        lm_own_ptr_stack_init(seen, lm_own_delete_plain);
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
        if (lm_l4_dispatch_frame(loader, context, lm_l4_node_frame(node)) != 0) {
            status = 1;
        }
        field = field -> next;
    }
    if (owns_seen != 0) {
        lm_l4_seen_tables_set(previous_seen);
        lm_own_ptr_stack_destroy(seen);
        lm_own_delete(seen, 0);
    }
    return status;
}

static int lm_l4_load_root(const LmL4Loader *loader, void *context, const LmP0Node *root, int implicit_l4) {
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
    seen = lm_own_new_zero(sizeof(seen[0]));
    if (seen == 0) {
        lm_l4_error(loader, "cannot allocate table duplicate tracker");
        return 1;
    }
    lm_own_ptr_stack_init(seen, lm_own_delete_plain);
    previous_seen = lm_l4_seen_tables_get();
    lm_l4_seen_tables_set(seen);
    loaded = 0;
    status = 0;
    field = lm_l4_node_structure(root) -> first_field;
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
        if (lm_l4_text_equals(lm_l4_frame_head(lm_l4_node_frame(node)), "L4") != 0 || lm_l4_text_equals(lm_l4_frame_head(lm_l4_node_frame(node)), "registry") != 0) {
            if (lm_l4_load_rows(loader, context, lm_l4_frame_body(lm_l4_node_frame(node))) != 0) {
                status = 1;
            }
            if (status == 0 && lm_l4_validate_named_trailer(loader, lm_l4_node_frame(node), lm_l4_frame_head(lm_l4_node_frame(node))) != 0) {
                status = 1;
            }
            if (status == 0) {
                loaded = 1;
            }
            field = field -> next;
            continue;
        }
        if (implicit_l4 != 0) {
            if (lm_l4_dispatch_frame(loader, context, lm_l4_node_frame(node)) != 0) {
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
    lm_own_delete(seen, 0);
    return status;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void lm_trans_import_document_delete(void *document);
static LmP0Text * lm_trans_registry_new_text(void);
static LmP0Structure * lm_trans_registry_new_structure(void);
static LmP0Frame * lm_trans_registry_new_frame(void);
static LmP0Trailer * lm_trans_registry_new_trailer(void);
static LmTransRegistryCloneFrame * lm_trans_registry_clone_frame_new_structure(const LmP0Structure *source, LmP0Structure *copy);
static LmTransRegistryCloneFrame * lm_trans_registry_clone_frame_new_trailer(const LmP0Trailer *source, LmP0Trailer **copy_slot);
static int lm_trans_registry_clone_push_frame(LmOwnPtrStack *stack, LmTransRegistryCloneFrame *frame);
static int lm_trans_registry_clone_push_structure(LmOwnPtrStack *stack, const LmP0Structure *source, LmP0Structure *copy);
static int lm_trans_registry_clone_push_trailer(LmOwnPtrStack *stack, const LmP0Trailer *source, LmP0Trailer **copy_slot);
static int lm_trans_registry_clone_process_structure(const LmP0Structure *source, LmP0Structure *copy, LmOwnPtrStack *stack);
static int lm_trans_registry_clone_process_trailer(const LmP0Trailer *source, LmP0Trailer **copy_slot, LmOwnPtrStack *stack);
static LmP0Node * lm_trans_registry_clone_node_shallow(const LmP0Node *source, LmOwnPtrStack *stack);
static int lm_trans_registry_clone_run(LmOwnPtrStack *stack);
static LmOwnPtrStack * lm_trans_registry_clone_stack_new(void);
static void lm_trans_registry_clone_stack_delete(LmOwnPtrStack **stack);
static LmP0Trailer * lm_trans_registry_clone_trailer(const LmP0Trailer *source);
static int lm_trans_registry_clone_structure(const LmP0Structure *source, LmP0Structure *out_structure);
static LmP0Node * lm_trans_registry_clone_node(const LmP0Node *source);
static int lm_trans_registry_index_row_relation(const LmP0Text *card_name, const char *relation_name, LmTransRegistryFact *row);
static int lm_trans_registry_index_row_table_view(LmTransRegistryFact *row);
static int lm_trans_registry_index_row(LmTransRegistryFact *row);
static int lm_trans_registry_push_row_values(const LmP0Text *table_value, const LmP0Text *key_value, const LmP0Text *payload_value);
static int lm_trans_registry_push_row_node_values(const LmP0Text *table_value, const LmP0Text *key_value, const LmP0Node *payload_node);
static int lm_trans_registry_join_relation_matches(const char *relation_name, const LmP0Text *source_name, const char **out_suffix);
static char * lm_trans_registry_join_target_relation_new(const LmP0Text *target_name, const char *suffix);
static int lm_trans_registry_join_copy_row(const LmTransRegistryFact *row, const LmP0Text *source_name, const LmP0Text *target_name, size_t *copied);
static int lm_trans_registry_join_table(const LmP0Text *source_table, const LmP0Text *target_table);
static int lm_trans_registry_push_row_atoms(const LmP0Text *table_atom, const LmP0Text *key_atom, const LmP0Text *payload_atom);
static int lm_trans_registry_column_has_descriptor(const LmTransRegistryColumn *column, const char *descriptor);
static int lm_trans_registry_column_is_class_typed(const LmTransRegistryColumn *column);
static int lm_trans_registry_cell_value(const LmP0Text *atom, const LmTransRegistryColumn *column, LmP0Text *out_value);
static int lm_trans_registry_push_table_cell(const LmP0Text *table_name, const LmTransRegistryColumn *column, int split_by_column, const LmP0Text *key_atom, const LmP0Node *payload_node, int allow_node_cells);
static int lm_trans_registry_note_class_kind(const LmP0Text *name, const char *kind);
static int lm_trans_registry_note_class_present(const LmP0Text *name);
static LmTransRegistryFact * lm_trans_registry_lookup_row_in_identifiers(const LmTransIdentifierTable *identifiers, const LmP0Text *key, const char *table);
static const LmOwnPtrStack * lm_trans_registry_relation_stack_in_identifiers(const LmTransIdentifierTable *identifiers, const LmP0Text *key, const char *relation_name);
static const LmOwnPtrStack * lm_trans_registry_relation_stack(const LmP0Text *key, const char *relation_name);
static const LmOwnPtrStack * lm_trans_namespace_registry_relation_stack(const LmTransNamespace *namespace_, const LmP0Text *key, const char *relation_name);
static const LmOwnPtrStack * lm_trans_registry_relation_stack_for_table(const char *table);
static LmTransRegistryFact * lm_trans_registry_relation_stack_latest_row(const LmOwnPtrStack *stack, const LmP0Text *key);
static const char * lm_trans_registry_lookup(const LmP0Text *key, const char *table);
static int lm_trans_registry_has(const LmP0Text *key, const char *table);
static int lm_trans_registry_table_has_rows(const char *table);
static int lm_trans_registry_assert_selected_table_link(const char *source_table, const LmP0Text *key, const char *target_class_table, const char *target_table);
static int lm_trans_registry_assert_payload_table_exists(const char *source_table, const LmP0Text *key, const char *target_table);
static const char * lm_trans_registry_lookup_table_link_checked(const LmP0Text *key, const char *source_table, const char *target_class_table);
static const char * lm_trans_namespace_registry_lookup(const LmTransNamespace *namespace_, const LmP0Text *key, const char *table);
static const char * lm_trans_namespace_registry_lookup_table_link_checked(const LmTransNamespace *namespace_, const LmP0Text *key, const char *source_table, const char *target_class_table);
static int lm_trans_array_head_find_close(const LmP0Text *text, size_t open_index, size_t *out_close_index);
static int lm_trans_array_head_next_dimension(const LmP0Text *head, size_t *index, LmP0Text *out_dimension);
static int lm_trans_text_is_array_receiver_head(const LmP0Text *head);

static FILE *lm_trans_prelude_output;

static const char *lm_trans_current_source_path;

static LmOwnPtrStack *lm_trans_declared_import_paths;

static LmOwnPtrStack *lm_trans_declared_import_documents;

static LmOwnPtrStack *lm_trans_emitted_import_prelude_paths;

static LmOwnPtrStack *lm_trans_emitted_import_function_paths;

static LmOwnPtrStack *lm_trans_emitted_callable_adapters;

static LmOwnPtrStack *lm_trans_emitted_callable_binders;

static LmOwnPtrStack *lm_trans_emitted_array_value_helpers;

static LmOwnPtrStack *lm_trans_emitted_function_return_structs;

static unsigned lm_trans_next_callable_adapter_id;

static unsigned lm_trans_next_array_value_helper_id;

static unsigned lm_trans_next_l2_structure_object_id;

static LmTransRegistry *lm_trans_registry;

static LmP0Text * lm_trans_text_ref_new(const LmP0Text *text);
static LmP0Text * lm_trans_text_ref_new_cstr(const char *text);
static LmP0Text * lm_trans_text_ref_tail_new(const LmP0Text *text, size_t offset);
static void lm_trans_text_ref_destroy(LmP0Text **text);
static void lm_trans_text_ref_delete_any(void *object);
static int lm_trans_function_header_init_fields(LmTransFunctionHeader *function);
static LmTransFunctionHeader * lm_trans_function_header_new(void);
static void lm_trans_function_header_destroy_fields(LmTransFunctionHeader *function);
static void lm_trans_function_header_destroy(LmTransFunctionHeader *function);
static int lm_trans_function_header_copy(LmTransFunctionHeader *target, const LmTransFunctionHeader *source);
static LmOwnPtrStack * lm_trans_ptr_stack_new(LmOwnDelete delete_item);
static void lm_trans_ptr_stack_delete(LmOwnPtrStack **stack);
static LmTransFunctionState * lm_trans_function_state_new(void);
static void lm_trans_function_state_destroy(LmTransFunctionState *state);
static void lm_trans_function_state_destroy_any(void *object);
static void lm_trans_function_state_delete(LmTransFunctionState *state);
static int lm_trans_registry_has_expr_emitter_binding(const LmP0Text *key, const char *table);
static const char * lm_trans_expr_emitter_binding_table(const char *class_name);
static const char * lm_trans_expr_spelling_binding_table(const char *class_name);
static int lm_trans_registry_has_expr_emitter_class(const LmP0Text *key, const char *class_name);
static int lm_trans_registry_has_expr_emitter_binding_class(const LmP0Text *key, const char *class_name);
static int lm_trans_registry_has_any_expr_emitter_binding(const LmP0Text *key);
static int lm_trans_text_is_operator_atom(const LmP0Text *text);
static int lm_trans_copy_stream(FILE *target, FILE *source);
static char * lm_trans_temp_output_path_new(const char *output_path, const char *suffix);
static const LmP0Structure * lm_trans_unwrap_single_anonymous_structure(const LmP0Structure *structure);
static unsigned long lm_trans_hash_bytes(const char *data, size_t length);
static unsigned long lm_trans_hash_cstr(const char *text);
static void lm_trans_ptr_stack_remove_pointer(LmOwnPtrStack *stack, const void *item);
static void lm_trans_identifier_relation_destroy(LmTransIdentifierRelation *relation);
static void lm_trans_identifier_card_destroy(LmTransIdentifierCard *card);
static void lm_trans_identifier_table_init(LmTransIdentifierTable *table);
static int lm_trans_identifier_table_ensure_buckets(LmTransIdentifierTable *table);
static void lm_trans_identifier_table_destroy(LmTransIdentifierTable *table);
static LmTransIdentifierTable * lm_trans_identifier_table_new(void);
static void lm_trans_identifier_table_delete(LmTransIdentifierTable **table);
static LmTransIdentifierCard * lm_trans_identifier_table_find_card(const LmTransIdentifierTable *table, const LmP0Text *name);
static LmTransIdentifierCard * lm_trans_identifier_table_get_card(LmTransIdentifierTable *table, const LmP0Text *name);
static LmTransIdentifierRelation * lm_trans_identifier_card_find_relation(const LmTransIdentifierCard *card, const char *relation_name);
static LmTransIdentifierRelation * lm_trans_identifier_card_get_relation(LmTransIdentifierCard *card, const char *relation_name);
static int lm_trans_identifier_table_add_symbol(LmTransIdentifierTable *table, LmTransSymbol *symbol);
static void lm_trans_identifier_table_remove_symbol(LmTransIdentifierTable *table, const LmTransSymbol *symbol);
static LmTransSymbol * lm_trans_identifier_table_find_latest_symbol(const LmTransIdentifierTable *table, const LmP0Text *name);
static LmTransSymbol * lm_trans_symbol_new(const LmP0Text *name, const char *kind, unsigned depth);
static int lm_trans_symbol_set_owned_name(LmTransSymbol *symbol, const LmP0Text *name);
static int lm_trans_symbol_set_c_name(LmTransSymbol *symbol, const LmP0Text *c_name);
static int lm_trans_symbol_set_env_arg(LmTransSymbol *symbol, const LmP0Text *env_arg);
static int lm_trans_symbol_set_closure_call_name(LmTransSymbol *symbol, const LmP0Text *closure_call_name);
static int lm_trans_capture_init_fields(LmTransCapture *capture);
static int lm_trans_capture_reset(LmTransCapture *capture);
static LmTransCapture * lm_trans_capture_new(const LmP0Text *name, const LmP0Text *type_head, const LmP0Node *type_node, size_t pointer_depth, int type_is_head);
static void lm_trans_capture_destroy(LmTransCapture *capture);
static void lm_trans_capture_delete_any(void *object);
static void lm_trans_hoisted_function_destroy(LmTransHoistedFunction *function);
static void lm_trans_hoisted_function_delete_any(void *object);
static LmTransCleanup * lm_trans_cleanup_new(unsigned id);
static void lm_trans_cleanup_destroy(LmTransCleanup *cleanup);
static void lm_trans_cleanup_delete_any(void *object);
static LmTransLoop * lm_trans_loop_new(size_t cleanup_base);
static void lm_trans_loop_destroy(LmTransLoop *loop);
static void lm_trans_loop_delete_any(void *object);
static LmTransNamespace * lm_trans_namespace_new(void);
static void lm_trans_namespace_destroy(LmTransNamespace *namespace_);
static void lm_trans_namespace_destroy_any(void *object);
static void lm_trans_namespace_delete(LmTransNamespace *namespace_);
static void lm_trans_namespace_enter_scope(LmTransNamespace *namespace_);
static void lm_trans_namespace_leave_scope(LmTransNamespace *namespace_);
static int lm_trans_cleanup_push(LmTransNamespace *namespace_, unsigned id);
static void lm_trans_cleanup_pop(LmTransNamespace *namespace_);
static int lm_trans_loop_push(LmTransNamespace *namespace_);
static void lm_trans_loop_pop(LmTransNamespace *namespace_);
static size_t lm_trans_loop_cleanup_base(const LmTransNamespace *namespace_);
static int lm_trans_emit_sync_name(FILE *file, unsigned id);
static int lm_trans_emit_return_name(FILE *file, unsigned id);
static const LmTransSymbol * lm_trans_namespace_find(const LmTransNamespace *namespace_, const LmP0Text *name);
static LmTransSymbol * lm_trans_namespace_find_mutable(LmTransNamespace *namespace_, const LmP0Text *name);
static int lm_trans_is_c_reference_name(const LmP0Text *name);
static int lm_trans_is_reserved_head_name(const LmP0Text *name);
static int lm_trans_head_binding_resolve(const LmTransNamespace *namespace_, const LmP0Text *head, LmTransHeadBinding *out);
static int lm_trans_namespace_declare_with_c_name(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind, const LmP0Text *c_name);
static int lm_trans_namespace_declare(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind);
static int lm_trans_namespace_declare_generated(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind);
static int lm_trans_namespace_attach_registry(LmTransNamespace *namespace_);
static int lm_trans_namespace_declare_c_name(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind, const LmP0Text *c_name);
static int lm_trans_namespace_set_env_arg(LmTransNamespace *namespace_, const LmP0Text *name, const LmP0Text *env_arg);
static int lm_trans_namespace_set_closure_call_name(LmTransNamespace *namespace_, const LmP0Text *name, const LmP0Text *closure_call_name);
static int lm_trans_symbol_copy_signature(LmTransSymbol *target, const LmTransSymbol *source);
static int lm_trans_namespace_declare_compatible(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind);
static const char * lm_trans_class_c_spelling(const LmP0Text *name);
static int lm_trans_builtin_c_type_tail(const LmP0Text *name);
static int lm_trans_builtin_c_type_name(const LmP0Text *name);
static int lm_trans_emit_name(FILE *file, const LmP0Text *name);
static int lm_trans_emit_type_name(FILE *file, const LmP0Text *name);
static int lm_trans_c_reference_has_path_dot(const LmP0Text *name);
static int lm_trans_node_is_c_reference_atom(const LmP0Node *node);


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
    const LmP0Text *name,
    const LmP0Text *type_head
);
static int lm_trans_head_can_declare_storage(
    const LmP0Text *head,
    const LmTransNamespace *namespace_
);
static LmTransExprAtomLowering *lm_trans_expr_atom_lowering_new(void);
static void lm_trans_expr_atom_lowering_delete(LmTransExprAtomLowering *lowering);
static void lm_trans_expr_callable_type_delete(LmTransL4CallableType *type);
static void lm_trans_expr_call_lowering_delete(LmTransCallLowering *call);

static LmTransBinding *lm_trans_expr_binding_new(void) {
    return (LmTransBinding *)lm_own_new_zero(sizeof(LmTransBinding));
}

static LmTransCallLowering *lm_trans_expr_call_lowering_new(void) {
    LmTransCallLowering *call;

    call = (LmTransCallLowering *)lm_own_new_zero(sizeof(*call));
    if (call != 0) {
        call->name = lm_trans_text_ref_new_cstr("");
        if (call->name == 0) {
            lm_trans_expr_call_lowering_delete(call);
            return 0;
        }
    }
    return call;
}

static void lm_trans_expr_call_lowering_delete(LmTransCallLowering *call) {
    if (call != 0) {
        lm_trans_text_ref_destroy(&call->name);
    }
    lm_own_delete(call, 0);
}

static int lm_trans_callable_type_init_fields(LmTransL4CallableType *type) {
    if (type == 0) {
        return 1;
    }
    if (type->class_name == 0) {
        type->class_name = lm_trans_text_ref_new_cstr("");
    }
    return type->class_name == 0;
}

static int lm_trans_callable_type_reset(LmTransL4CallableType *type) {
    if (lm_trans_callable_type_init_fields(type) != 0) {
        return 1;
    }
    type->class_name->data = "";
    type->class_name->length = 0U;
    type->address_depth = 0U;
    type->is_const = 0;
    return 0;
}

static LmTransL4CallableType *lm_trans_expr_callable_type_new(void) {
    LmTransL4CallableType *type;

    type = (LmTransL4CallableType *)lm_own_new_zero(sizeof(*type));
    if (type != 0 && lm_trans_callable_type_init_fields(type) != 0) {
        lm_trans_expr_callable_type_delete(type);
        return 0;
    }
    return type;
}

static void lm_trans_expr_callable_type_delete(LmTransL4CallableType *type) {
    if (type != 0) {
        lm_trans_text_ref_destroy(&type->class_name);
    }
    lm_own_delete(type, 0);
}

static LmTransAbiParam **lm_trans_expr_abi_params_new(size_t capacity) {
    LmTransAbiParam **params;
    size_t i;

    if (capacity == 0U) {
        return 0;
    }
    params = (LmTransAbiParam **)lm_own_new_zero(sizeof(params[0]) * capacity);
    if (params == 0) {
        return 0;
    }
    for (i = 0U; i < capacity; ++i) {
        params[i] = (LmTransAbiParam *)lm_own_new_zero(sizeof(params[i][0]));
        if (params[i] == 0) {
            while (i != 0U) {
                --i;
                lm_own_delete(params[i], 0);
            }
            lm_own_delete(params, 0);
            return 0;
        }
    }
    return params;
}

static void lm_trans_expr_abi_params_delete(LmTransAbiParam **params, size_t capacity) {
    size_t i;

    if (params == 0) {
        return;
    }
    for (i = 0U; i < capacity; ++i) {
        lm_own_delete(params[i], 0);
    }
    lm_own_delete(params, 0);
}

static LmTransExprJob *lm_trans_expr_job_new(void) {
    return (LmTransExprJob *)lm_own_new_zero(sizeof(LmTransExprJob));
}

static LmTransExprRangeJob *lm_trans_expr_range_job_new(void) {
    return (LmTransExprRangeJob *)lm_own_new_zero(sizeof(LmTransExprRangeJob));
}

static LmTransExprPiece *lm_trans_expr_piece_new(void) {
    LmTransExprPiece *piece;

    piece = (LmTransExprPiece *)lm_own_new_zero(sizeof(LmTransExprPiece));
    if (piece != 0) {
        piece->atom = lm_trans_expr_atom_lowering_new();
        if (piece->atom == 0) {
            lm_own_delete(piece, 0);
            return 0;
        }
    }
    return piece;
}

static void lm_trans_expr_piece_destroy_fields(LmTransExprPiece *piece) {
    if (piece != 0) {
        lm_trans_expr_atom_lowering_delete(piece->atom);
        piece->atom = 0;
    }
}

static void lm_trans_expr_piece_delete(LmTransExprPiece *piece) {
    lm_trans_expr_piece_destroy_fields(piece);
    lm_own_delete(piece, 0);
}

static void lm_trans_expr_piece_delete_any(void *object) {
    lm_trans_expr_piece_delete((LmTransExprPiece *)object);
}

static LmTransExprStack *lm_trans_expr_stack_new(void) {
    return (LmTransExprStack *)lm_own_new_zero(sizeof(LmTransExprStack));
}

static LmTransExprAtomLowering *lm_trans_expr_atom_lowering_new(void) {
    LmTransExprAtomLowering *lowering;

    lowering = (LmTransExprAtomLowering *)lm_own_new_zero(sizeof(*lowering));
    if (lowering != 0) {
        lowering->text = lm_trans_text_ref_new_cstr("");
        if (lowering->text == 0) {
            lm_own_delete(lowering, 0);
            return 0;
        }
    }
    return lowering;
}

static void lm_trans_expr_atom_lowering_destroy_fields(LmTransExprAtomLowering *lowering) {
    if (lowering != 0) {
        lm_trans_text_ref_destroy(&lowering->text);
    }
}

static void lm_trans_expr_atom_lowering_delete(LmTransExprAtomLowering *lowering) {
    lm_trans_expr_atom_lowering_destroy_fields(lowering);
    lm_own_delete(lowering, 0);
}

static int lm_trans_expr_atom_lowering_copy(
    LmTransExprAtomLowering *target,
    const LmTransExprAtomLowering *source
) {
    LmP0Text *text;

    if (target == 0 || source == 0 || target->text == 0) {
        return 1;
    }
    text = target->text;
    *target = *source;
    target->text = text;
    if (source->text != 0) {
        *target->text = *source->text;
    } else {
        target->text->data = "";
        target->text->length = 0U;
    }
    return 0;
}

static int lm_trans_atom_starts_string(const LmP0Text *text) {
    return text != 0 && text->length > 0U && (text->data[0] == '"' || text->data[0] == '\'');
}

static int lm_trans_atom_is_identifier_like(const LmP0Text *text) {
    unsigned char ch;

    if (text == 0 || text->length == 0U || lm_trans_atom_starts_string(text)) {
        return 0;
    }

    if (
        lm_trans_text_is_operator_atom(text) ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\")
    ) {
        return 0;
    }

    ch = (unsigned char)text->data[0];
    return
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') ||
        ch == '_' ||
        ch == '`';
}

static int lm_trans_name_argument_is_valid(const LmP0Text *text) {
    (void)text;
    return 1;
}

static int lm_trans_text_contains_char(const LmP0Text *text, char ch);

static int lm_trans_validate_expr_atom_mode(
    const LmP0Text *atom,
    const LmTransNamespace *namespace_,
    int allow_callable_value
) {
    const LmTransSymbol *symbol;

    if (atom == 0 || lm_trans_is_c_reference_name(atom) || !lm_trans_atom_is_identifier_like(atom)) {
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 expr error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)atom->length,
            atom->data,
            (int)atom->length,
            atom->data
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
            (int)atom->length,
            atom->data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    return 0;
}

static int lm_trans_emit_expr_atom_mode(
    FILE *file,
    const LmP0Text *atom,
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
    const LmP0Text *atom,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_expr_atom_mode(file, atom, namespace_, 0);
}

static int lm_trans_call_lower_value(
    const LmP0Text *head,
    const LmTransSymbol *symbol,
    LmTransCallLowering *out
) {
    if (head == 0) {
        return 1;
    }
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa function \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)head->length,
            head->data,
            (int)head->length,
            head->data
        );
        return 1;
    }

    if (!lm_trans_symbol_is_value_callable(symbol)) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not a value-returning function\n",
            (int)head->length,
            head->data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    out->signature = symbol;
    out->is_closure = lm_trans_symbol_is(symbol, "closure");
    return 0;
}

static int lm_trans_call_lower_statement(
    const LmP0Text *head,
    const LmTransSymbol *symbol,
    LmTransCallLowering *out
) {
    if (head == 0) {
        return 1;
    }
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa callable \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)head->length,
            head->data,
            (int)head->length,
            head->data
        );
        return 1;
    }

    if (!lm_trans_symbol_is_executable_callable(symbol)) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not a callable\n",
            (int)head->length,
            head->data,
            lm_trans_symbol_class_name(symbol->class_name)
        );
        return 1;
    }

    out->signature = symbol;
    out->is_closure = lm_trans_symbol_is(symbol, "closure");
    return 0;
}

static LmTransCallLoweringHandler lm_trans_call_lowering_handler_binding(const char *binding) {
    LmTransBinding *resolved;
    LmTransCallLoweringHandler handler;

    resolved = lm_trans_expr_binding_new();
    if (resolved == 0) {
        return 0;
    }
    handler =
        lm_trans_binding_resolve(binding, resolved) && resolved->call_lowering != 0
            ? resolved->call_lowering
            : 0;
    lm_own_delete(resolved, 0);
    return handler;
}

static int lm_trans_lower_call(
    const LmP0Text *head,
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

    if (head == 0) {
        return 1;
    }
    *out->name = *head;
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
        *out->name = *symbol->c_name;
    }
    return status;
}

static int lm_trans_atom_is_prefix_expr_operator(const LmP0Text *text) {
    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.prefix"
    );
}

static int lm_trans_atom_is_postfix_expr_operator(const LmP0Text *text) {
    return lm_trans_registry_has_expr_emitter_binding_class(
        text,
        "operator.postfix"
    );
}

static int lm_trans_nodes_touch(const LmP0Node *left, const LmP0Node *right) {
    return
        left != 0 &&
        right != 0 &&
        left->span != 0 &&
        right->span != 0 &&
        left->span->offset + left->span->length == right->span->offset;
}

static int lm_trans_atom_is_infix_expr_operator(
    const LmP0Text *text,
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
            !(previous_operand->kind == LM_P0_NODE_ATOM && lm_trans_atom_starts_string(previous_operand->as->atom));
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
        lm_trans_atom_is_prefix_expr_operator(node->as->atom)
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

        if (lm_trans_text_equals(node->as->atom, "[")) {
            bracket_depth = 1;
            field = field->next;
            while (field != 0 && bracket_depth > 0) {
                node = field->value;
                if (node != 0 && node->kind == LM_P0_NODE_ATOM) {
                    if (lm_trans_text_equals(node->as->atom, "[")) {
                        ++bracket_depth;
                    } else if (lm_trans_text_equals(node->as->atom, "]")) {
                        --bracket_depth;
                    }
                }
                previous_operand = node;
                field = field->next;
            }
        } else if (lm_trans_text_equals(node->as->atom, ".")) {
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
        } else if (lm_trans_atom_is_postfix_expr_operator(node->as->atom)) {
            if (previous_operand == 0) {
                break;
            }
            previous_operand = node;
            c_dot_path = 0;
            field = field->next;
        } else if (lm_trans_atom_is_infix_expr_operator(node->as->atom, node, previous_operand)) {
            operand = field->next;
            if (operand == 0) {
                return field;
            }
            while (
                operand->value != 0 &&
                operand->value->kind == LM_P0_NODE_ATOM &&
                lm_trans_atom_is_prefix_expr_operator(operand->value->as->atom)
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
    const LmP0Text *name,
    size_t *out_index
) {
    size_t i;
    const LmP0Text *param_name;

    if (callee == 0 || out_index == 0) {
        return 0;
    }

    for (i = 0U; i < callee->param_names->count; ++i) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(callee->param_names, i);
        if (param_name != 0 && lm_trans_identifier_same(param_name, name)) {
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

    frame = field->value->as->frame;
    if ((frame->flags & LM_P0_FRAME_COLON) == 0U) {
        return 0;
    }

    return lm_trans_signature_param_index(callee, frame->head, out_index);
}

static LmTransExprLoweredRange *lm_trans_expr_lowered_range_new(void) {
    LmTransExprLoweredRange *range;

    range = (LmTransExprLoweredRange *)lm_own_new_zero(sizeof(LmTransExprLoweredRange));
    if (range != 0) {
        range->pieces = lm_trans_ptr_stack_new(lm_trans_expr_piece_delete_any);
        if (range->pieces == 0) {
            lm_own_delete(range, 0);
            return 0;
        }
    }
    return range;
}

static void lm_trans_expr_lowered_range_destroy(LmTransExprLoweredRange *range) {
    if (range != 0) {
        lm_trans_ptr_stack_delete(&range->pieces);
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
        lm_trans_expr_lowered_range_delete(job->lowered_range);
        job->lowered_range = 0;
    }
}

static void lm_trans_expr_job_destroy_name_text(LmTransExprJob *job) {
    if (job != 0) {
        lm_trans_text_ref_destroy(&job->name_text);
    }
}

static void lm_trans_expr_job_destroy_range(LmTransExprJob *job) {
    if (job != 0) {
        lm_own_delete(job->range, 0);
        job->range = 0;
    }
}

static void lm_trans_expr_job_destroy_call_args(LmTransExprJob *job) {
    if (job != 0) {
        lm_own_delete(job->call_args, 0);
        job->call_args = 0;
    }
}

static void lm_trans_expr_job_destroy(LmTransExprJob *job) {
    if (job != 0 && job->destroy != 0) {
        job->destroy(job);
        job->destroy = 0;
    }
}

static void lm_trans_expr_job_delete(LmTransExprJob *job) {
    lm_trans_expr_job_destroy(job);
    lm_own_delete(job, 0);
}

static void lm_trans_expr_job_delete_any(void *object) {
    lm_trans_expr_job_delete((LmTransExprJob *)object);
}

static void lm_trans_expr_stack_pop_delete(LmTransExprStack *stack) {
    LmTransExprJob *job;

    job = stack != 0 && stack->jobs != 0
        ? (LmTransExprJob *)lm_own_ptr_stack_pop(stack->jobs)
        : 0;
    lm_trans_expr_job_delete(job);
}

static void lm_trans_expr_stack_destroy(LmTransExprStack *stack) {
    if (stack != 0) {
        lm_trans_ptr_stack_delete(&stack->jobs);
    }
}

static int lm_trans_expr_stack_push(LmTransExprStack *stack, LmTransExprJob *job) {
    if (stack == 0 || stack->jobs == 0 || job == 0) {
        return 1;
    }

    return lm_own_ptr_stack_push(stack->jobs, job);
}

static int lm_trans_expr_stack_push_text(LmTransExprStack *stack, const char *text) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_text;
    job->destroy = 0;
    job->text = text;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_name_text(LmTransExprStack *stack, const LmP0Text *name) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_name_text;
    job->destroy = lm_trans_expr_job_destroy_name_text;
    job->name_text = lm_trans_text_ref_new(name);
    if (job->name_text == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_node(LmTransExprStack *stack, const LmP0Node *node) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_node;
    job->destroy = 0;
    job->node = node;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_frame(LmTransExprStack *stack, const LmP0Frame *frame) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_frame;
    job->destroy = 0;
    job->frame = frame;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_range_state(
    LmTransExprStack *stack,
    const LmTransExprRangeJob *range
) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0 || range == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    job->run = lm_trans_expr_job_emit_range;
    job->destroy = lm_trans_expr_job_destroy_range;
    job->range = lm_trans_expr_range_job_new();
    if (job->range == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    *job->range = *range;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_range(
    LmTransExprStack *stack,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprRangeJob *range;
    int status;

    range = lm_trans_expr_range_job_new();
    if (range == 0) {
        return 1;
    }
    range->field = first;
    range->stop = stop;
    range->wrote = 0;
    range->previous_operand = 0;
    range->expect_field_name = 0;
    range->expect_c_field_name = 0;
    range->c_dot_path = 0;
    status = lm_trans_expr_stack_push_range_state(stack, range);
    lm_own_delete(range, 0);
    return status;
}

static int lm_trans_expr_stack_push_call_args(
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_schedule_call_args;
    job->destroy = lm_trans_expr_job_destroy_call_args;
    job->call_args = (LmTransExprCallArgsJob *)lm_own_new_zero(sizeof(*job->call_args));
    if (job->call_args == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    job->call_args->body = body;
    job->call_args->callee = callee;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static int lm_trans_expr_stack_push_lowered_range(
    LmTransExprStack *stack,
    LmTransExprLoweredRange *range
) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        lm_trans_expr_lowered_range_delete(range);
        return 1;
    }
    job->run = lm_trans_expr_job_emit_lowered_range;
    job->destroy = lm_trans_expr_job_destroy_lowered_range;
    job->lowered_range = range;
    status = lm_trans_expr_stack_push(stack, job);
    if (status != 0) {
        lm_trans_expr_job_delete(job);
    }
    return status;
}

static LmTransExprSegment *lm_trans_expr_segment_new(void) {
    return (LmTransExprSegment *)lm_own_new_zero(sizeof(LmTransExprSegment));
}

static LmOwnPtrStack *lm_trans_expr_segment_stack_new(void) {
    LmOwnPtrStack *segments;

    segments = (LmOwnPtrStack *)lm_own_new_zero(sizeof(*segments));
    if (segments != 0) {
        lm_own_ptr_stack_init(segments, lm_own_delete_plain);
    }
    return segments;
}

static void lm_trans_expr_segment_stack_delete(LmOwnPtrStack **segments) {
    if (segments != 0 && *segments != 0) {
        lm_own_ptr_stack_destroy(*segments);
        lm_own_delete(*segments, 0);
        *segments = 0;
    }
}

static int lm_trans_expr_segments_resize_blank(
    LmOwnPtrStack *segments,
    size_t count
) {
    LmTransExprSegment *segment;

    if (segments == 0) {
        return 1;
    }
    if (count < segments->count) {
        lm_own_ptr_stack_truncate(segments, count);
        return 0;
    }
    while (segments->count < count) {
        segment = lm_trans_expr_segment_new();
        if (segment == 0) {
            return 1;
        }
        if (lm_own_ptr_stack_push(segments, segment) != 0) {
            lm_own_delete(segment, 0);
            return 1;
        }
    }
    return 0;
}

static int lm_trans_expr_segments_append(
    LmOwnPtrStack *segments,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprSegment *segment;
    int status;

    segment = lm_trans_expr_segment_new();
    if (segment == 0) {
        return 1;
    }
    segment->first = first;
    segment->stop = stop;
    segment->expected_param = 0;
    segment->present = 1;
    status = lm_own_ptr_stack_push(segments, segment);
    if (status != 0) {
        lm_own_delete(segment, 0);
    }
    return status;
}

static int lm_trans_expr_stack_push_name_text(LmTransExprStack *stack, const LmP0Text *name);
static const LmP0Field *lm_trans_call_body_first_field(const LmP0Structure *body);
static int lm_trans_expr_segments_parse_fields(
    LmOwnPtrStack *segments,
    const LmP0Field *first
);
static int lm_trans_expr_stack_push_segments(
    FILE *file,
    LmTransExprStack *stack,
    const LmOwnPtrStack *segments,
    const LmTransNamespace *namespace_
);

static int lm_trans_callable_has_no_params(const LmTransSymbol *symbol) {
    return
        symbol != 0 &&
        symbol->callable_params_node != 0 &&
        symbol->callable_params_node->kind == LM_P0_NODE_STRUCTURE &&
        symbol->callable_params_node->as->structure->first_field == 0;
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

    field = lm_trans_nth_field(symbol->callable_params_node->as->structure, index);
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
    const LmP0Node *current;
    const LmP0Field *field0;
    const LmP0Field *field1;
    const LmP0Field *inner_field;
    const LmP0Node *param_node;
    LmTransL4CallableType *ignored_type;
    int has_callable_type;

    if (out_first == 0) {
        return 0;
    }
    *out_first = 0;

    current = node;
    while (
        current != 0 &&
        current->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(current->as->frame->head, "const")
    ) {
        field0 = current->as->frame->body->first_field;
        param_node = 0;
        if (
            field0 != 0 &&
            field0->next == 0 &&
            field0->value != 0
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as->structure->first_field;
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
            return 0;
        }
        current = param_node;
    }

    if (current == 0 || current->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(current->as->frame->body);
    if (lm_trans_text_is_array_receiver_head(current->as->frame->head)) {
        return lm_trans_array_param_default_fields(current, out_first);
    }

    ignored_type = lm_trans_expr_callable_type_new();
    if (ignored_type == 0) {
        return 0;
    }
    has_callable_type = lm_trans_l4_callable_type_from_node(current, ignored_type);
    lm_trans_expr_callable_type_delete(ignored_type);
    if (has_callable_type) {
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

    if (lm_trans_head_can_declare_storage(current->as->frame->head, 0)) {
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

    if (lm_trans_text_all_char(current->as->frame->head, '@')) {
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
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    LmP0Text *dimension;
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

    current = node;
    while (
        current != 0 &&
        current->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(current->as->frame->head, "const")
    ) {
        frame = current->as->frame;
        field = frame->body->first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        current = field->value;
    }

    if (current == 0 || current->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = current->as->frame;
    if (!lm_trans_text_is_array_receiver_head(frame->head)) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;
    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as->atom, '@')
    ) {
        pointer_depth += field->value->as->atom->length;
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
    dimension = lm_trans_text_ref_new_cstr("");
    if (dimension == 0) {
        return 0;
    }
    while (lm_trans_array_head_next_dimension(frame->head, &dimension_index, dimension)) {
        ++rank;
    }
    if (rank == 0U) {
        lm_trans_text_ref_destroy(&dimension);
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
    lm_trans_text_ref_destroy(&dimension);
    return 1;
}

static int lm_trans_array_param_default_fields(
    const LmP0Node *node,
    const LmP0Field **out_first
) {
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *default_field;
    LmP0Text *dimension;
    size_t dimension_index;

    if (out_first != 0) {
        *out_first = 0;
    }
    if (out_first == 0 || node == 0 || node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    current = node;
    while (
        current != 0 &&
        current->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(current->as->frame->head, "const")
    ) {
        frame = current->as->frame;
        field = frame->body->first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        current = field->value;
    }

    if (current == 0 || current->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = current->as->frame;
    if (!lm_trans_text_is_array_receiver_head(frame->head)) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    field = body != 0 ? body->first_field : 0;
    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as->atom, '@')
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
    dimension = lm_trans_text_ref_new_cstr("");
    if (dimension == 0) {
        return 0;
    }
    while (lm_trans_array_head_next_dimension(frame->head, &dimension_index, dimension)) {
        if (dimension->length == 0U && lm_trans_array_body_dimension_can_consume(default_field)) {
            default_field = default_field->next;
        }
    }

    if (default_field == 0) {
        lm_trans_text_ref_destroy(&dimension);
        return 0;
    }

    *out_first = default_field;
    lm_trans_text_ref_destroy(&dimension);
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
    const LmP0Text *name
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

static int lm_trans_callable_descriptor_is_lazy(
    const LmTransNamespace *namespace_,
    const LmP0Text *name
) {
    return lm_trans_namespace_registry_lookup(namespace_, name, "callable.lazy") != 0;
}

static int lm_trans_callable_descriptor_is_raw_function_reference(
    const LmTransNamespace *namespace_,
    const LmP0Text *name
) {
    return name != 0 && !lm_trans_callable_descriptor_is_lazy(namespace_, name);
}

static int lm_trans_emit_raw_callable_declarator(
    FILE *file,
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name,
    const LmP0Text *name,
    const char *error_name
) {
    LmTransAbiParam **params;
    size_t param_capacity;
    size_t param_count;
    char *descriptor_cstr;
    int status;

    if (file == 0 || descriptor_name == 0 || name == 0) {
        return 1;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 1;
    }
    param_capacity = 256U;
    params = lm_trans_expr_abi_params_new(param_capacity);
    if (params == 0) {
        lm_own_delete(descriptor_cstr, 0);
        return 1;
    }

    status =
        lm_trans_collect_abi_params(
            params,
            param_capacity,
            &param_count,
            namespace_,
            descriptor_cstr,
            error_name != 0 ? error_name : "callable descriptor"
        ) != 0 ||
        lm_trans_emit_abi_return_type(
            file,
            namespace_,
            descriptor_cstr,
            error_name != 0 ? error_name : "callable descriptor"
        ) != 0 ||
        lm_trans_put(file, " (*") != 0 ||
        lm_trans_emit_identifier(file, name) != 0 ||
        lm_trans_put(file, ")(") != 0 ||
        (
            param_count == 0U
                ? lm_trans_put(file, "void")
                : lm_trans_emit_abi_params(file, params, param_count)
        ) != 0 ||
        lm_trans_put(file, ")") != 0;

    lm_trans_expr_abi_params_delete(params, param_capacity);
    lm_own_delete(descriptor_cstr, 0);
    return status;
}

static int lm_trans_node_callable_descriptor_name(
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    LmP0Text *out_name
) {
    const LmP0Node *current;
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

    current = node;
    while (current != 0) {
        if (current->kind == LM_P0_NODE_STRUCTURE) {
            field = current->as->structure->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                return 0;
            }
            current = field->value;
            continue;
        }
        if (
            current->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_equals(current->as->frame->head, "const")
        ) {
            field = current->as->frame->body->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                return 0;
            }
            current = field->value;
            continue;
        }
        break;
    }

    if (current == 0) {
        return 0;
    }

    if (current->kind == LM_P0_NODE_ATOM) {
        symbol = lm_trans_namespace_find(namespace_, current->as->atom);
        if (lm_trans_symbol_is(symbol, "callableDescriptor") && symbol->name != 0) {
            *out_name = *symbol->name;
            return 1;
        }
        if (lm_trans_registry_is_function_pointer_type_name(namespace_, current->as->atom)) {
            *out_name = *current->as->atom;
            return 1;
        }
        return 0;
    }

    if (current->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = current->as->frame;
    symbol = lm_trans_namespace_find(namespace_, frame->head);
    if (lm_trans_symbol_is(symbol, "callableDescriptor") && symbol->name != 0) {
        *out_name = *symbol->name;
        return 1;
    }
    if (lm_trans_registry_is_function_pointer_type_name(namespace_, frame->head)) {
        *out_name = *frame->head;
        return 1;
    }
    return 0;
}

static int lm_trans_expected_param_is_callable_descriptor(
    const LmP0Node *param,
    const LmTransNamespace *namespace_
) {
    LmP0Text *descriptor_name;
    int result;

    descriptor_name = lm_trans_text_ref_new_cstr("");
    if (descriptor_name == 0) {
        return 0;
    }
    result = lm_trans_node_callable_descriptor_name(param, namespace_, descriptor_name);
    lm_trans_text_ref_destroy(&descriptor_name);
    return result;
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
    LmP0Text *void_name;

    if (out == 0) {
        return 0;
    }
    if (lm_trans_callable_type_reset(out) != 0) {
        return 0;
    }
    if (node == 0) {
        void_name = lm_trans_text_from_cstr("void");
        if (void_name == 0) {
            return 0;
        }
        *out->class_name = *void_name;
        lm_trans_text_ref_destroy(&void_name);
        return 1;
    }
    return lm_trans_l4_callable_type_from_node(node, out);
}

static int lm_trans_callable_descriptor_return_type(
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name,
    LmTransL4CallableType *out
) {
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *class_row;
    LmP0Text *return_key;
    size_t const_flag;

    if (out == 0) {
        return 0;
    }
    if (lm_trans_callable_type_reset(out) != 0) {
        return 0;
    }

    return_key = lm_trans_text_ref_new_cstr("return");
    if (return_key == 0) {
        return 0;
    }
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.class");
    class_row = lm_trans_registry_relation_stack_latest_row(class_rows, return_key);
    if (class_row == 0 || class_row->payload == 0) {
        lm_trans_text_ref_destroy(&return_key);
        return 0;
    }

    {
        LmP0Text *class_name;

        class_name = lm_trans_text_from_cstr(class_row->payload);
        if (class_name == 0) {
            lm_trans_text_ref_destroy(&return_key);
            return 0;
        }
        *out->class_name = *class_name;
        lm_trans_text_ref_destroy(&class_name);
    }
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, descriptor_name, "return.const");
    if (
        !lm_trans_registry_latest_size_payload(address_depth_rows, return_key, 0U, &out->address_depth) ||
        !lm_trans_registry_latest_size_payload(const_rows, return_key, 0U, &const_flag)
    ) {
        lm_trans_text_ref_destroy(&return_key);
        return 0;
    }
    out->is_const = const_flag != 0U;
    lm_trans_text_ref_destroy(&return_key);
    return 1;
}

static int lm_trans_callable_signature_matches_descriptor_name(
    const LmP0Text *descriptor_name,
    const LmTransSymbol *candidate,
    const LmTransNamespace *namespace_
) {
    size_t index;
    const LmP0Node *candidate_param;
    LmTransL4CallableType *descriptor_type;
    LmTransL4CallableType *candidate_type;
    LmTransAbiParam **params;
    size_t param_count;
    char *descriptor_cstr;
    int result;
    size_t params_capacity;

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
    params_capacity = 256U;
    params = lm_trans_expr_abi_params_new(params_capacity);
    descriptor_type = lm_trans_expr_callable_type_new();
    candidate_type = lm_trans_expr_callable_type_new();
    if (params == 0 || descriptor_type == 0 || candidate_type == 0) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(candidate_type);
        lm_own_delete(descriptor_cstr, 0);
        return 0;
    }
    if (lm_trans_collect_abi_params(params, params_capacity, &param_count, namespace_, descriptor_cstr, "callable descriptor") != 0) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(candidate_type);
        lm_own_delete(descriptor_cstr, 0);
        return 0;
    }
    lm_own_delete(descriptor_cstr, 0);

    if (param_count != candidate->param_names->count) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(candidate_type);
        return 0;
    }

    for (index = 0U; index < param_count; ++index) {
        candidate_param = lm_trans_callable_param_node_at(candidate, index);
        lm_trans_text_assign_cstr(descriptor_type->class_name, params[index]->class_name);
        descriptor_type->address_depth = params[index]->address_depth;
        descriptor_type->is_const = params[index]->is_const;
        if (
            !lm_trans_l4_callable_type_from_node(candidate_param, candidate_type) ||
            !lm_trans_callable_type_same(descriptor_type, candidate_type)
        ) {
            lm_trans_expr_abi_params_delete(params, params_capacity);
            lm_trans_expr_callable_type_delete(descriptor_type);
            lm_trans_expr_callable_type_delete(candidate_type);
            return 0;
        }
    }

    result =
        lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, descriptor_type) &&
        lm_trans_callable_return_type_from_node(candidate->callable_return_node, candidate_type) &&
        lm_trans_callable_type_same(descriptor_type, candidate_type);
    lm_trans_expr_abi_params_delete(params, params_capacity);
    lm_trans_expr_callable_type_delete(descriptor_type);
    lm_trans_expr_callable_type_delete(candidate_type);
    return result;
}

static int lm_trans_callable_descriptor_has_no_params(
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name
) {
    LmTransAbiParam **params;
    size_t param_count;
    char *descriptor_cstr;
    int result;
    size_t params_capacity;

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }
    params_capacity = 256U;
    params = lm_trans_expr_abi_params_new(params_capacity);
    if (params == 0) {
        lm_own_delete(descriptor_cstr, 0);
        return 0;
    }

    result =
        lm_trans_collect_abi_params(
            params,
            params_capacity,
            &param_count,
            namespace_,
            descriptor_cstr,
            "callable descriptor"
        ) == 0 &&
        param_count == 0U;
    lm_trans_expr_abi_params_delete(params, params_capacity);
    lm_own_delete(descriptor_cstr, 0);
    return result;
}

static int lm_trans_callable_descriptor_param_count(
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name,
    size_t *out_count
) {
    LmTransAbiParam **params;
    char *descriptor_cstr;
    int result;
    size_t params_capacity;

    if (out_count == 0) {
        return 0;
    }
    *out_count = 0U;

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 0;
    }
    params_capacity = 256U;
    params = lm_trans_expr_abi_params_new(params_capacity);
    if (params == 0) {
        lm_own_delete(descriptor_cstr, 0);
        return 0;
    }
    result =
        lm_trans_collect_abi_params(
            params,
            params_capacity,
            out_count,
            namespace_,
            descriptor_cstr,
            "callable descriptor"
        ) == 0;
    lm_trans_expr_abi_params_delete(params, params_capacity);
    lm_own_delete(descriptor_cstr, 0);
    return result;
}

static int lm_trans_callable_descriptor_return_descriptor_name(
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name,
    LmP0Text *out_return_descriptor_name
) {
    LmTransL4CallableType *return_type;
    int result;

    if (out_return_descriptor_name == 0) {
        return 0;
    }
    out_return_descriptor_name->data = "";
    out_return_descriptor_name->length = 0U;

    return_type = lm_trans_expr_callable_type_new();
    if (return_type == 0) {
        return 0;
    }
    if (
        !lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, return_type) ||
        return_type->address_depth != 0U ||
        return_type->is_const ||
        !lm_trans_registry_is_function_pointer_type_name(namespace_, return_type->class_name)
    ) {
        lm_trans_expr_callable_type_delete(return_type);
        return 0;
    }

    *out_return_descriptor_name = *return_type->class_name;
    result = 1;
    lm_trans_expr_callable_type_delete(return_type);
    return result;
}

static int lm_trans_callable_return_chain_depth_to_descriptor(
    const LmTransSymbol *symbol,
    const LmP0Text *descriptor_name,
    const LmTransNamespace *namespace_,
    size_t *out_depth
) {
    LmP0Text *current_descriptor_name;
    size_t depth;

    if (out_depth == 0) {
        return 0;
    }
    *out_depth = 0U;

    current_descriptor_name = lm_trans_text_ref_new_cstr("");
    if (current_descriptor_name == 0) {
        return 0;
    }
    if (
        !lm_trans_symbol_is_value_callable(symbol) ||
        !lm_trans_callable_has_no_params(symbol) ||
        !lm_trans_node_callable_descriptor_name(symbol->callable_return_node, namespace_, current_descriptor_name)
    ) {
        lm_trans_text_ref_destroy(&current_descriptor_name);
        return 0;
    }

    for (depth = 1U; depth < 64U; ++depth) {
        if (lm_trans_identifier_same(current_descriptor_name, descriptor_name)) {
            *out_depth = depth;
            lm_trans_text_ref_destroy(&current_descriptor_name);
            return 1;
        }

        if (
            !lm_trans_callable_descriptor_has_no_params(namespace_, current_descriptor_name) ||
            !lm_trans_callable_descriptor_return_descriptor_name(namespace_, current_descriptor_name, current_descriptor_name)
        ) {
            lm_trans_text_ref_destroy(&current_descriptor_name);
            return 0;
        }
    }

    lm_trans_text_ref_destroy(&current_descriptor_name);
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
    const LmP0Text *source_name,
    const LmP0Text *descriptor_name
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
    if (source_name == 0 || descriptor_name == 0) {
        return 0;
    }
    length = prefix_length + source_name->length + middle_length + descriptor_name->length;
    name = (char *)lm_own_new_zero(length + 1U);
    if (name == 0) {
        return 0;
    }

    memcpy(name, prefix, prefix_length);
    out = prefix_length;
    for (i = 0U; i < source_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(source_name->data[i]) ? source_name->data[i] : '_';
    }
    memcpy(name + out, middle, middle_length);
    out += middle_length;
    for (i = 0U; i < descriptor_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(descriptor_name->data[i]) ? descriptor_name->data[i] : '_';
    }
    name[out] = '\0';
    return name;
}

static int lm_trans_emit_callable_adapter_args(
    FILE *file,
    LmTransAbiParam **params,
    size_t param_count
) {
    size_t i;

    if (file == 0 || (params == 0 && param_count != 0U)) {
        return 1;
    }

    for (i = 0U; i < param_count; ++i) {
        if (
            (i != 0U && lm_trans_put(file, ", ") != 0) ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(params[i]->name)) != 0
        ) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_type_named(
    FILE *file,
    const LmTransL4CallableType *type,
    const LmP0Text *name
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
    if ((type->address_depth != 0U || (name != 0 && name->length != 0U)) && lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < type->address_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    if (name != 0 && name->length != 0U && lm_trans_emit_identifier(file, name) != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_callable_param_node_named(
    FILE *file,
    const LmP0Node *param_node,
    const LmP0Text *name
) {
    LmTransL4CallableType *type;
    int status;

    type = lm_trans_expr_callable_type_new();
    if (type == 0) {
        return 1;
    }
    if (!lm_trans_l4_callable_type_from_node(param_node, type)) {
        lm_trans_expr_callable_type_delete(type);
        return 1;
    }
    status = lm_trans_emit_callable_type_named(file, type, name);
    lm_trans_expr_callable_type_delete(type);
    return status;
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
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(symbol->param_names, first + i);
        if (param_node == 0 || param_name == 0) {
            return 1;
        }
        if (lm_trans_emit_callable_param_node_named(file, param_node, param_name) != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_callable_adapter_source_prototype(
    FILE *file,
    const LmP0Text *source_name,
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
            source_symbol->param_names->count
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
    const LmP0Text *source_name,
    const LmTransSymbol *source_symbol,
    const LmP0Text *descriptor_name,
    size_t chain_depth,
    LmP0Text *out_adapter_name
) {
    FILE *prelude_file;
    LmTransAbiParam **params;
    size_t param_count;
    LmTransL4CallableType *return_type;
    char *descriptor_cstr;
    char *adapter_name;
    const char *stored_name;
    LmP0Text *void_name;
    int returns_void;
    size_t i;
    size_t params_capacity;

    if (out_adapter_name == 0) {
        return 1;
    }
    out_adapter_name->data = "";
    out_adapter_name->length = 0U;

    adapter_name = lm_trans_callable_adapter_name_new(source_name, descriptor_name);
    if (adapter_name == 0) {
        return 1;
    }

    stored_name = lm_trans_string_stack_find(lm_trans_emitted_callable_adapters, adapter_name);
    if (stored_name != 0) {
        lm_own_delete(adapter_name, 0);
        out_adapter_name->data = stored_name;
        out_adapter_name->length = strlen(stored_name);
        return 0;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        lm_own_delete(adapter_name, 0);
        return 1;
    }

    params_capacity = 256U;
    params = lm_trans_expr_abi_params_new(params_capacity);
    return_type = lm_trans_expr_callable_type_new();
    if (params == 0 || return_type == 0) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        lm_own_delete(descriptor_cstr, 0);
        lm_own_delete(adapter_name, 0);
        return 1;
    }

    if (
        lm_trans_collect_abi_params(
            params,
            params_capacity,
            &param_count,
            namespace_,
            descriptor_cstr,
            "callable adapter"
        ) != 0 ||
        !lm_trans_callable_descriptor_return_type(namespace_, descriptor_name, return_type)
    ) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        lm_own_delete(descriptor_cstr, 0);
        lm_own_delete(adapter_name, 0);
        return 1;
    }

    void_name = lm_trans_text_ref_new_cstr("void");
    if (void_name == 0) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        lm_own_delete(descriptor_cstr, 0);
        lm_own_delete(adapter_name, 0);
        return 1;
    }
    returns_void =
        return_type->address_depth == 0U &&
        !return_type->is_const &&
        lm_trans_identifier_same(return_type->class_name, void_name);

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
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        lm_trans_text_ref_destroy(&void_name);
        lm_own_delete(descriptor_cstr, 0);
        lm_own_delete(adapter_name, 0);
        return 1;
    }

    for (i = 0U; i < chain_depth; ++i) {
        if (lm_trans_put(prelude_file, "()") != 0) {
            lm_trans_expr_abi_params_delete(params, params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
            lm_trans_text_ref_destroy(&void_name);
            lm_own_delete(descriptor_cstr, 0);
            lm_own_delete(adapter_name, 0);
            return 1;
        }
    }

    if (
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_callable_adapter_args(prelude_file, params, param_count) != 0 ||
        lm_trans_put(prelude_file, ");\n}\n\n") != 0
    ) {
        lm_trans_expr_abi_params_delete(params, params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        lm_trans_text_ref_destroy(&void_name);
        lm_own_delete(descriptor_cstr, 0);
        lm_own_delete(adapter_name, 0);
        return 1;
    }

    lm_trans_expr_abi_params_delete(params, params_capacity);
    lm_trans_expr_callable_type_delete(return_type);
    lm_trans_text_ref_destroy(&void_name);
    lm_own_delete(descriptor_cstr, 0);
    if (lm_own_ptr_stack_push(lm_trans_emitted_callable_adapters, adapter_name) != 0) {
        lm_own_delete(adapter_name, 0);
        return 1;
    }
    out_adapter_name->data = adapter_name;
    out_adapter_name->length = strlen(adapter_name);
    return 0;
}

static char *lm_trans_callable_binder_name_new(
    const LmP0Text *source_name,
    const LmP0Text *descriptor_name,
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
    if (source_name == 0 || descriptor_name == 0) {
        return 0;
    }
    snprintf(count_buffer, sizeof(count_buffer), "_%lu", (unsigned long)bound_count);
    prefix_length = strlen(prefix);
    middle_length = strlen(middle);
    count_length = strlen(count_buffer);
    length = prefix_length + source_name->length + middle_length + descriptor_name->length + count_length;
    name = (char *)lm_own_new_zero(length + 1U);
    if (name == 0) {
        return 0;
    }

    memcpy(name, prefix, prefix_length);
    out = prefix_length;
    for (i = 0U; i < source_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(source_name->data[i]) ? source_name->data[i] : '_';
    }
    memcpy(name + out, middle, middle_length);
    out += middle_length;
    for (i = 0U; i < descriptor_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(descriptor_name->data[i]) ? descriptor_name->data[i] : '_';
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
    result = (char *)lm_own_new_zero(base_length + suffix_length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, base, base_length);
    memcpy(result + base_length, suffix, suffix_length);
    result[base_length + suffix_length] = '\0';
    return result;
}

static int lm_trans_callable_type_is_void(const LmTransL4CallableType *type) {
    LmP0Text *void_name;
    int result;

    void_name = lm_trans_text_ref_new_cstr("void");
    if (void_name == 0) {
        return 0;
    }
    result =
        type != 0 &&
        type->address_depth == 0U &&
        !type->is_const &&
        lm_trans_identifier_same(type->class_name, void_name);
    lm_trans_text_ref_destroy(&void_name);
    return result;
}

static int lm_trans_callable_descriptor_accepts_source_bind(
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    LmTransAbiParam **descriptor_params,
    size_t descriptor_params_capacity,
    size_t *out_descriptor_param_count,
    LmTransL4CallableType *out_return_type
) {
    size_t index;
    size_t descriptor_param_count;
    size_t source_param_count;
    const LmP0Node *source_param;
    LmTransL4CallableType *descriptor_type;
    LmTransL4CallableType *source_type;
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
    source_param_count = source_symbol->param_names->count;
    if (bound_count > source_param_count) {
        return 0;
    }

    descriptor_type = lm_trans_expr_callable_type_new();
    source_type = lm_trans_expr_callable_type_new();
    if (descriptor_type == 0 || source_type == 0) {
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(source_type);
        return 0;
    }
    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(source_type);
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
    lm_own_delete(descriptor_cstr, 0);
    if (!result) {
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(source_type);
        return 0;
    }

    if (
        !lm_trans_callable_return_type_from_node(source_symbol->callable_return_node, source_type) ||
        !lm_trans_callable_type_same(out_return_type, source_type)
    ) {
        lm_trans_expr_callable_type_delete(descriptor_type);
        lm_trans_expr_callable_type_delete(source_type);
        return 0;
    }

    for (index = 0U; index < descriptor_param_count; ++index) {
        source_param = lm_trans_callable_param_node_at(source_symbol, bound_count + index);
        lm_trans_text_assign_cstr(descriptor_type->class_name, descriptor_params[index]->class_name);
        descriptor_type->address_depth = descriptor_params[index]->address_depth;
        descriptor_type->is_const = descriptor_params[index]->is_const;
        if (
            !lm_trans_l4_callable_type_from_node(source_param, source_type) ||
            !lm_trans_callable_type_same(descriptor_type, source_type)
        ) {
            lm_trans_expr_callable_type_delete(descriptor_type);
            lm_trans_expr_callable_type_delete(source_type);
            return 0;
        }
    }

    *out_descriptor_param_count = descriptor_param_count;
    lm_trans_expr_callable_type_delete(descriptor_type);
    lm_trans_expr_callable_type_delete(source_type);
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
            frame = field->value->as->frame;
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

static char *lm_trans_callable_default_adapter_name_new(
    const LmP0Text *source_name,
    const LmP0Text *descriptor_name
) {
    const char *prefix;
    const char *middle;
    char id_buffer[32];
    size_t prefix_length;
    size_t middle_length;
    size_t id_length;
    size_t length;
    size_t i;
    size_t out;
    char *name;

    prefix = "lm_trans_default_";
    middle = "_as_";
    if (source_name == 0 || descriptor_name == 0) {
        return 0;
    }
    snprintf(id_buffer, sizeof(id_buffer), "_%u", lm_trans_next_callable_adapter_id++);
    prefix_length = strlen(prefix);
    middle_length = strlen(middle);
    id_length = strlen(id_buffer);
    length = prefix_length + source_name->length + middle_length + descriptor_name->length + id_length;
    name = (char *)lm_own_new_zero(length + 1U);
    if (name == 0) {
        return 0;
    }

    memcpy(name, prefix, prefix_length);
    out = prefix_length;
    for (i = 0U; i < source_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(source_name->data[i]) ? source_name->data[i] : '_';
    }
    memcpy(name + out, middle, middle_length);
    out += middle_length;
    for (i = 0U; i < descriptor_name->length; ++i) {
        name[out++] = lm_trans_c_identifier_char(descriptor_name->data[i]) ? descriptor_name->data[i] : '_';
    }
    memcpy(name + out, id_buffer, id_length);
    out += id_length;
    name[out] = '\0';
    return name;
}

static int lm_trans_emit_callable_default_adapter_bound_args(
    FILE *file,
    const LmTransNamespace *namespace_,
    const LmP0Structure *body,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    LmTransAbiParam **descriptor_params,
    size_t descriptor_param_count
) {
    const LmP0Field *field;
    const LmP0Field *next;
    const LmP0Field *default_first;
    const LmP0Node *param_node;
    size_t index;
    int wrote;

    field = lm_trans_call_body_first_field(body);
    wrote = 0;
    for (index = 0U; index < bound_count; ++index) {
        param_node = lm_trans_callable_param_node_at(source_symbol, index);
        if (param_node == 0) {
            return 1;
        }
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (field != 0) {
            next = lm_trans_expr_segment_end(field);
            if (lm_trans_node_is_positional_skip(field->value)) {
                if (!lm_trans_formal_param_default_fields(param_node, &default_first)) {
                    return 1;
                }
                if (lm_trans_emit_expr_range(file, default_first, 0, namespace_) != 0) {
                    return 1;
                }
            } else {
                if (lm_trans_emit_expr_range(file, field, next, namespace_) != 0) {
                    return 1;
                }
            }
            field = next;
        } else {
            if (!lm_trans_formal_param_default_fields(param_node, &default_first)) {
                return 1;
            }
            if (lm_trans_emit_expr_range(file, default_first, 0, namespace_) != 0) {
                return 1;
            }
        }
        wrote = 1;
    }

    for (index = 0U; index < descriptor_param_count; ++index) {
        if (descriptor_params[index] == 0 || descriptor_params[index]->name == 0 || descriptor_params[index]->name[0] == '\0') {
            return 1;
        }
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, lm_trans_text_from_cstr(descriptor_params[index]->name)) != 0) {
            return 1;
        }
        wrote = 1;
    }
    return 0;
}

static int lm_trans_callable_default_adapter_has_bound_values(
    const LmP0Structure *body,
    const LmTransSymbol *source_symbol,
    size_t bound_count
) {
    const LmP0Field *field;
    const LmP0Field *next;
    const LmP0Field *default_first;
    const LmP0Node *param_node;
    size_t index;

    field = lm_trans_call_body_first_field(body);
    for (index = 0U; index < bound_count; ++index) {
        param_node = lm_trans_callable_param_node_at(source_symbol, index);
        if (param_node == 0) {
            return 0;
        }
        if (!lm_trans_formal_param_default_fields(param_node, &default_first)) {
            return 0;
        }
        if (field != 0) {
            next = lm_trans_expr_segment_end(field);
            field = next;
        }
    }
    return field == 0;
}

static int lm_trans_emit_callable_default_adapter(
    FILE *file,
    const LmTransNamespace *namespace_,
    const LmP0Text *source_name,
    const LmTransSymbol *source_symbol,
    const LmP0Text *descriptor_name,
    const LmP0Structure *body,
    size_t bound_count,
    LmP0Text *out_adapter_name
) {
    FILE *prelude_file;
    LmTransAbiParam **descriptor_params;
    size_t descriptor_params_capacity;
    size_t descriptor_param_count;
    size_t explicit_bound_count;
    LmTransL4CallableType *return_type;
    char *adapter_name;
    int returns_void;
    int status;

    if (out_adapter_name == 0) {
        return 1;
    }
    out_adapter_name->data = "";
    out_adapter_name->length = 0U;

    if (
        !lm_trans_call_body_positional_arg_count(body, &explicit_bound_count) ||
        explicit_bound_count > bound_count ||
        !lm_trans_callable_default_adapter_has_bound_values(body, source_symbol, bound_count)
    ) {
        return 0;
    }

    descriptor_params_capacity = 256U;
    descriptor_params = lm_trans_expr_abi_params_new(descriptor_params_capacity);
    return_type = lm_trans_expr_callable_type_new();
    if (descriptor_params == 0 || return_type == 0) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    if (
        !lm_trans_callable_descriptor_accepts_source_bind(
            namespace_,
            descriptor_name,
            source_symbol,
            bound_count,
            descriptor_params,
            descriptor_params_capacity,
            &descriptor_param_count,
            return_type
        )
    ) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 0;
    }

    adapter_name = lm_trans_callable_default_adapter_name_new(source_name, descriptor_name);
    if (adapter_name == 0) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    prelude_file = lm_trans_prelude_file(file);
    returns_void = lm_trans_callable_type_is_void(return_type);
    status =
        lm_trans_emit_callable_adapter_source_prototype(prelude_file, source_name, source_symbol) != 0 ||
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_callable_type_named(prelude_file, return_type, lm_trans_text_from_cstr("")) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(adapter_name)) != 0 ||
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_abi_params(prelude_file, descriptor_params, descriptor_param_count) != 0 ||
        lm_trans_put(prelude_file, ") {\n    ") != 0 ||
        (!returns_void && lm_trans_put(prelude_file, "return ") != 0) ||
        lm_trans_emit_name(prelude_file, source_name) != 0 ||
        lm_trans_put(prelude_file, "(") != 0 ||
        lm_trans_emit_callable_default_adapter_bound_args(
            prelude_file,
            namespace_,
            body,
            source_symbol,
            bound_count,
            descriptor_params,
            descriptor_param_count
        ) != 0 ||
        lm_trans_put(prelude_file, ");\n}\n\n") != 0;
    if (status != 0) {
        lm_own_delete(adapter_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    if (lm_own_ptr_stack_push(lm_trans_emitted_callable_adapters, adapter_name) != 0) {
        lm_own_delete(adapter_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    out_adapter_name->data = adapter_name;
    out_adapter_name->length = strlen(adapter_name);
    lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
    lm_trans_expr_callable_type_delete(return_type);
    return 0;
}

static int lm_trans_emit_callable_binder_descriptor_param_list(
    FILE *file,
    LmTransAbiParam **params,
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
    LmTransAbiParam **descriptor_params,
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
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(source_symbol->param_names, index);
        if (param_name == 0) {
            return 1;
        }
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(env_var_name)) != 0 ||
            lm_trans_put(file, "->") != 0 ||
            lm_trans_emit_identifier(file, param_name) != 0
        ) {
            return 1;
        }
        wrote = 1;
    }

    for (index = 0U; index < descriptor_param_count; ++index) {
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, lm_trans_text_from_cstr(descriptor_params[index]->name)) != 0) {
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
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(source_symbol->param_names, index);
        if (param_node == 0 || param_name == 0) {
            return 1;
        }
        if (
            lm_trans_put(file, "    ") != 0 ||
            lm_trans_emit_callable_param_node_named(file, param_node, param_name) != 0 ||
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
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(source_symbol->param_names, index);
        if (param_name == 0) {
            return 1;
        }
        if (
            lm_trans_put(file, "    ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(env_name)) != 0 ||
            lm_trans_put(file, "->") != 0 ||
            lm_trans_emit_identifier(file, param_name) != 0 ||
            lm_trans_put(file, " = ") != 0 ||
            lm_trans_emit_identifier(file, param_name) != 0 ||
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
    const LmP0Text *source_name,
    const LmTransSymbol *source_symbol,
    const LmP0Text *descriptor_name,
    size_t bound_count,
    LmP0Text *out_binder_name
) {
    FILE *prelude_file;
    LmTransAbiParam **descriptor_params;
    size_t descriptor_param_count;
    LmTransL4CallableType *return_type;
    char *binder_name;
    char *env_type_name;
    char *call_name;
    char *destroy_name;
    const char *stored_name;
    int returns_void;
    size_t descriptor_params_capacity;

    if (out_binder_name == 0) {
        return 1;
    }
    out_binder_name->data = "";
    out_binder_name->length = 0U;

    descriptor_params_capacity = 256U;
    descriptor_params = lm_trans_expr_abi_params_new(descriptor_params_capacity);
    return_type = lm_trans_expr_callable_type_new();
    if (descriptor_params == 0 || return_type == 0) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    if (
        !lm_trans_callable_descriptor_accepts_source_bind(
            namespace_,
            descriptor_name,
            source_symbol,
            bound_count,
            descriptor_params,
            descriptor_params_capacity,
            &descriptor_param_count,
            return_type
        )
    ) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 0;
    }

    binder_name = lm_trans_callable_binder_name_new(source_name, descriptor_name, bound_count);
    if (binder_name == 0) {
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    stored_name = lm_trans_string_stack_find(lm_trans_emitted_callable_binders, binder_name);
    if (stored_name != 0) {
        lm_own_delete(binder_name, 0);
        out_binder_name->data = stored_name;
        out_binder_name->length = strlen(stored_name);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 0;
    }

    env_type_name = lm_trans_cstr_suffix_new(binder_name, "Env");
    call_name = lm_trans_cstr_suffix_new(binder_name, "_call");
    destroy_name = lm_trans_cstr_suffix_new(binder_name, "_destroy");
    if (env_type_name == 0 || call_name == 0 || destroy_name == 0) {
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    prelude_file = lm_trans_prelude_file(file);
    returns_void = lm_trans_callable_type_is_void(return_type);
    if (lm_trans_emit_callable_adapter_source_prototype(prelude_file, source_name, source_symbol) != 0) {
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
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
            lm_own_delete(env_type_name, 0);
            lm_own_delete(call_name, 0);
            lm_own_delete(destroy_name, 0);
            lm_own_delete(binder_name, 0);
            lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
            return 1;
        }
    }

    if (
        lm_trans_put(prelude_file, "static ") != 0 ||
        lm_trans_emit_callable_type_named(prelude_file, return_type, lm_trans_text_from_cstr("")) != 0 ||
        lm_trans_put(prelude_file, " ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(call_name)) != 0 ||
        lm_trans_put(prelude_file, "(void *lm_env") != 0 ||
        (descriptor_param_count != 0U && lm_trans_put(prelude_file, ", ") != 0) ||
        lm_trans_emit_callable_binder_descriptor_param_list(prelude_file, descriptor_params, descriptor_param_count) != 0 ||
        lm_trans_put(prelude_file, ") {\n") != 0
    ) {
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    if (bound_count == 0U) {
        if (lm_trans_put(prelude_file, "    (void)lm_env;\n") != 0) {
            lm_own_delete(env_type_name, 0);
            lm_own_delete(call_name, 0);
            lm_own_delete(destroy_name, 0);
            lm_own_delete(binder_name, 0);
            lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
            return 1;
        }
    } else if (
        lm_trans_put(prelude_file, "    ") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
        lm_trans_put(prelude_file, " *lm_bind_env = (") != 0 ||
        lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
        lm_trans_put(prelude_file, " *)lm_env;\n") != 0
    ) {
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
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
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "static void ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(destroy_name)) != 0 ||
            lm_trans_put(prelude_file, "(void *lm_env) {\n    lm_own_delete(lm_env, 0);\n}\n") != 0
        ) {
            lm_own_delete(env_type_name, 0);
            lm_own_delete(call_name, 0);
            lm_own_delete(destroy_name, 0);
            lm_own_delete(binder_name, 0);
            lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
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
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "    ") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " *lm_env;\n") != 0
        ) {
            lm_own_delete(env_type_name, 0);
            lm_own_delete(call_name, 0);
            lm_own_delete(destroy_name, 0);
            lm_own_delete(binder_name, 0);
            lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
            return 1;
        }
    }
    if (
        lm_trans_put(prelude_file, "    lm_result = (") != 0 ||
        lm_trans_emit_type_name(prelude_file, descriptor_name) != 0 ||
        lm_trans_put(prelude_file, ")lm_own_new_zero(sizeof(*lm_result));\n    if (lm_result == 0) {\n        return 0;\n    }\n") != 0
    ) {
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    if (bound_count != 0U) {
        if (
            lm_trans_put(prelude_file, "    lm_env = (") != 0 ||
            lm_trans_emit_identifier(prelude_file, lm_trans_text_from_cstr(env_type_name)) != 0 ||
            lm_trans_put(prelude_file, " *)lm_own_new_zero(sizeof(*lm_env));\n    if (lm_env == 0) {\n        lm_own_delete(lm_result, 0);\n        return 0;\n    }\n") != 0 ||
            lm_trans_emit_callable_binder_env_assignments(prelude_file, source_symbol, bound_count, "lm_env") != 0
        ) {
            lm_own_delete(env_type_name, 0);
            lm_own_delete(call_name, 0);
            lm_own_delete(destroy_name, 0);
            lm_own_delete(binder_name, 0);
            lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
            lm_trans_expr_callable_type_delete(return_type);
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
        lm_own_delete(env_type_name, 0);
        lm_own_delete(call_name, 0);
        lm_own_delete(destroy_name, 0);
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }

    lm_own_delete(env_type_name, 0);
    lm_own_delete(call_name, 0);
    lm_own_delete(destroy_name, 0);
    if (lm_own_ptr_stack_push(lm_trans_emitted_callable_binders, binder_name) != 0) {
        lm_own_delete(binder_name, 0);
        lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
        lm_trans_expr_callable_type_delete(return_type);
        return 1;
    }
    out_binder_name->data = binder_name;
    out_binder_name->length = strlen(binder_name);
    lm_trans_expr_abi_params_delete(descriptor_params, descriptor_params_capacity);
    lm_trans_expr_callable_type_delete(return_type);
    return 0;
}

static int lm_trans_expr_stack_push_lazy_binder_call(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Text *binder_name,
    const LmP0Structure *body,
    const LmTransSymbol *source_symbol,
    size_t bound_count,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *segments;
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
    segments = lm_trans_expr_segment_stack_new();
    if (segments == 0) {
        return 1;
    }
    status = lm_trans_expr_segments_resize_blank(segments, bound_count);
    field = first;
    index = 0U;
    while (status == 0 && field != 0) {
        if (index >= bound_count) {
            fprintf(stderr, "trans L2 error: too many lazy-bind arguments\n");
            status = 1;
            break;
        }
        segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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
        segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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
        status = lm_trans_expr_stack_push_segments(file, stack, segments, namespace_);
    }
    lm_trans_expr_segment_stack_delete(&segments);
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
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Field *field;

    if (param == 0 || return_node == 0) {
        return 0;
    }

    current = param;
    while (current != 0) {
        if (lm_trans_expected_param_is_callable_descriptor(current, namespace_)) {
            return 0;
        }
        if (current->kind == LM_P0_NODE_STRUCTURE) {
            field = current->as->structure->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                return 0;
            }
            current = field->value;
            continue;
        }
        if (
            current->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_equals(current->as->frame->head, "const")
        ) {
            field = current->as->frame->body->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                return 0;
            }
            current = field->value;
            continue;
        }
        break;
    }

    if (current == 0) {
        return 0;
    }

    if (current->kind == LM_P0_NODE_ATOM) {
        return
            return_node->kind == LM_P0_NODE_ATOM &&
            lm_trans_identifier_same(current->as->atom, return_node->as->atom);
    }

    if (current->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    frame = current->as->frame;
    if (return_node->kind != LM_P0_NODE_ATOM) {
        return 0;
    }

    return lm_trans_identifier_same(frame->head, return_node->as->atom);
}

static int lm_trans_should_force_zero_arg_callable(
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    LmP0Text *out_name,
    const LmTransSymbol **out_symbol
) {
    const LmTransSymbol *symbol;
    LmP0Text *atom;

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

    atom = lm_trans_text_ref_new(segment->first->value->as->atom);
    if (atom == 0) {
        return 0;
    }
    symbol = lm_trans_namespace_find(namespace_, atom);
    if (
        !lm_trans_symbol_is_value_callable(symbol) ||
        !lm_trans_callable_has_no_params(symbol) ||
        !lm_trans_param_descriptor_matches_return(segment->expected_param, symbol->callable_return_node, namespace_)
    ) {
        lm_trans_text_ref_destroy(&atom);
        return 0;
    }

    if (out_name != 0) {
        *out_name = symbol->has_c_name ? *symbol->c_name : *atom;
    }
    if (out_symbol != 0) {
        *out_symbol = symbol;
    }
    lm_trans_text_ref_destroy(&atom);
    return 1;
}

static int lm_trans_expr_stack_push_forced_zero_arg_callable(
    LmTransExprStack *stack,
    const LmP0Text *name,
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
    LmP0Text *forced_name;
    int status;

    (void)file;
    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    forced_name = lm_trans_text_ref_new_cstr("");
    if (forced_name == 0) {
        return 1;
    }
    if (!lm_trans_should_force_zero_arg_callable(segment, namespace_, forced_name, &forced_symbol)) {
        lm_trans_text_ref_destroy(&forced_name);
        return 0;
    }

    status = lm_trans_expr_stack_push_forced_zero_arg_callable(stack, forced_name, forced_symbol);
    lm_trans_text_ref_destroy(&forced_name);
    if (status != 0) {
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
    LmP0Text *descriptor_name;
    LmP0Text *atom;
    LmP0Text *name;
    LmP0Text *adapter_name;
    LmP0Text *binder_name;
    size_t chain_depth;
    size_t explicit_bound_count;
    size_t bound_count;
    size_t descriptor_param_count;
    int descriptor_is_function_pointer;
    int consumed;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 1;
    }

    descriptor_name = lm_trans_text_ref_new_cstr("");
    atom = lm_trans_text_ref_new_cstr("");
    name = lm_trans_text_ref_new_cstr("");
    adapter_name = lm_trans_text_ref_new_cstr("");
    binder_name = lm_trans_text_ref_new_cstr("");
    if (
        descriptor_name == 0 ||
        atom == 0 ||
        name == 0 ||
        adapter_name == 0 ||
        binder_name == 0
    ) {
        lm_trans_text_ref_destroy(&descriptor_name);
        lm_trans_text_ref_destroy(&atom);
        lm_trans_text_ref_destroy(&name);
        lm_trans_text_ref_destroy(&adapter_name);
        lm_trans_text_ref_destroy(&binder_name);
        return 1;
    }

    consumed = 0;
    status = 0;
    if (
        !lm_trans_node_callable_descriptor_name(segment != 0 ? segment->expected_param : 0, namespace_, descriptor_name) ||
        segment == 0 ||
        segment->first == 0 ||
        segment->first->next != segment->stop ||
        segment->first->value == 0
    ) {
        goto cleanup;
    }

    descriptor_is_function_pointer = lm_trans_registry_is_function_pointer_type_name(namespace_, descriptor_name);
    node = segment->first->value;
    if (node->kind == LM_P0_NODE_FRAME) {
        frame = node->as->frame;
        symbol = lm_trans_namespace_find(namespace_, frame->head);
        if (
            symbol != 0 &&
            lm_trans_symbol_is_executable_callable(symbol) &&
            !lm_trans_callable_descriptor_is_lazy(namespace_, descriptor_name) &&
            !descriptor_is_function_pointer &&
            !lm_trans_symbol_is(symbol, "closure") &&
            lm_trans_call_body_positional_arg_count(frame->body, &explicit_bound_count) &&
            lm_trans_callable_descriptor_param_count(namespace_, descriptor_name, &descriptor_param_count) &&
            symbol->param_names->count >= descriptor_param_count
        ) {
            bound_count = symbol->param_names->count - descriptor_param_count;
            if (explicit_bound_count <= bound_count) {
                *name = symbol->has_c_name ? *symbol->c_name : *frame->head;
                if (
                    lm_trans_emit_callable_default_adapter(
                        file,
                        namespace_,
                        name,
                        symbol,
                        descriptor_name,
                        frame->body,
                        bound_count,
                        adapter_name
                    ) != 0
                ) {
                    status = 1;
                    goto cleanup;
                }
                if (adapter_name->length != 0U) {
                    if (lm_trans_expr_stack_push_name_text(stack, adapter_name) != 0) {
                        status = 1;
                        goto cleanup;
                    }
                    consumed = 1;
                    goto cleanup;
                }
            }
        }
        if (
            symbol != 0 &&
            lm_trans_symbol_is_executable_callable(symbol) &&
            !lm_trans_callable_descriptor_is_lazy(namespace_, descriptor_name)
        ) {
            fprintf(
                stderr,
                "trans L2 error: call-shaped callable argument for %.*s is not a raw function reference; defaults could not form an adapter\n",
                (int)descriptor_name->length,
                descriptor_name->data
            );
            status = 1;
            goto cleanup;
        }
        if (
            descriptor_is_function_pointer ||
            symbol == 0 ||
            !lm_trans_symbol_is_executable_callable(symbol) ||
            lm_trans_symbol_is(symbol, "closure") ||
            !lm_trans_call_body_positional_arg_count(frame->body, &explicit_bound_count) ||
            !lm_trans_callable_descriptor_param_count(namespace_, descriptor_name, &descriptor_param_count) ||
            symbol->param_names->count < descriptor_param_count
        ) {
            goto cleanup;
        }
        bound_count = symbol->param_names->count - descriptor_param_count;
        if (explicit_bound_count > bound_count) {
            fprintf(stderr, "trans L2 error: too many lazy-bind arguments for callable descriptor %.*s\n", (int)descriptor_name->length, descriptor_name->data);
            status = 1;
            goto cleanup;
        }

        *name = symbol->has_c_name ? *symbol->c_name : *frame->head;
        if (lm_trans_emit_callable_binder(file, namespace_, name, symbol, descriptor_name, bound_count, binder_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (binder_name->length == 0U) {
            goto cleanup;
        }
        if (lm_trans_expr_stack_push_lazy_binder_call(file, stack, binder_name, frame->body, symbol, bound_count, namespace_) != 0) {
            status = 1;
            goto cleanup;
        }
        consumed = 1;
        goto cleanup;
    }

    if (node->kind != LM_P0_NODE_ATOM) {
        goto cleanup;
    }

    *atom = *node->as->atom;
    if (lm_trans_is_c_reference_name(atom)) {
        if (!descriptor_is_function_pointer) {
            goto cleanup;
        }
        if (lm_trans_expr_stack_push_name_text(stack, atom) != 0) {
            status = 1;
            goto cleanup;
        }
        consumed = 1;
        goto cleanup;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol == 0 || !lm_trans_symbol_is_executable_callable(symbol)) {
        goto cleanup;
    }

    *name = symbol->has_c_name ? *symbol->c_name : *atom;
    if (lm_trans_callable_signature_matches_descriptor_name(descriptor_name, symbol, namespace_)) {
        if (
            lm_trans_callable_descriptor_is_raw_function_reference(namespace_, descriptor_name) &&
            !lm_trans_symbol_is(symbol, "closure")
        ) {
            if (lm_trans_expr_stack_push_name_text(stack, name) != 0) {
                status = 1;
                goto cleanup;
            }
            consumed = 1;
            goto cleanup;
        }
        if (!descriptor_is_function_pointer && !lm_trans_symbol_is(symbol, "closure")) {
            if (lm_trans_emit_callable_binder(file, namespace_, name, symbol, descriptor_name, 0U, binder_name) != 0) {
                status = 1;
                goto cleanup;
            }
            if (binder_name->length == 0U) {
                goto cleanup;
            }
            if (lm_trans_expr_stack_push_lazy_binder_call(file, stack, binder_name, 0, symbol, 0U, namespace_) != 0) {
                status = 1;
                goto cleanup;
            }
            consumed = 1;
            goto cleanup;
        }
        if (lm_trans_expr_stack_push_name_text(stack, name) != 0) {
            status = 1;
            goto cleanup;
        }
        consumed = 1;
        goto cleanup;
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
                adapter_name
            ) != 0 ||
            lm_trans_expr_stack_push_name_text(stack, adapter_name) != 0
        ) {
            status = 1;
            goto cleanup;
        }
        consumed = 1;
        goto cleanup;
    }

    fprintf(
        stderr,
        "trans L2 error: \"%.*s\" neither matches callable descriptor %.*s nor has a no-argument return chain to it\n",
        (int)atom->length,
        atom->data,
        (int)descriptor_name->length,
        descriptor_name->data
    );
    status = 1;

cleanup:
    if (status == 0 && consumed) {
        *out_consumed = 1;
    }
    lm_trans_text_ref_destroy(&descriptor_name);
    lm_trans_text_ref_destroy(&atom);
    lm_trans_text_ref_destroy(&name);
    lm_trans_text_ref_destroy(&adapter_name);
    lm_trans_text_ref_destroy(&binder_name);
    return status;
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
    LmTransBinding *resolved;
    LmTransExprSegmentMaterializer materializer;

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
    resolved = lm_trans_expr_binding_new();
    if (resolved == 0) {
        return 1;
    }
    if (
        !lm_trans_binding_resolve(binding, resolved) ||
        resolved->expr_segment_materializer == 0
    ) {
        fprintf(
            stderr,
            "trans registry inconsistency: materialize.argument has unknown binding %s\n",
            binding
        );
        lm_own_delete(resolved, 0);
        return 1;
    }

    materializer = resolved->expr_segment_materializer;
    lm_own_delete(resolved, 0);
    return materializer(file, stack, segment, namespace_, out_consumed);
}

static int lm_trans_expr_segments_parse_fields(
    LmOwnPtrStack *segments,
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
    const LmOwnPtrStack *segments,
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
        segment = (const LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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
        field = field->value->as->structure->first_field;
    }

    return field;
}

static int lm_trans_call_args_layout_signature(
    LmOwnPtrStack *segments,
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

    if (lm_trans_expr_segments_resize_blank(segments, callee->param_names->count) != 0) {
        return 1;
    }

    index = 0U;
    named_out_of_position = 0;
    while (field != 0) {
        is_named = lm_trans_call_field_is_named_argument(field, callee, &named_index);
        if (is_named) {
            segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, named_index);
            if (segment == 0) {
                return 1;
            }
            if (segment->present) {
                fprintf(stderr, "trans L2 error: duplicate named argument\n");
                return 1;
            }
            named_frame = field->value->as->frame;
            segment->first = named_frame->body->first_field;
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
            if (index >= callee->param_names->count) {
                fprintf(stderr, "trans L2 error: too many arguments\n");
                return 1;
            }
            segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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
        if (index >= callee->param_names->count) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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

    for (index = 0U; index < callee->param_names->count; ++index) {
        segment = (LmTransExprSegment *)lm_own_ptr_stack_at(segments, index);
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
    LmOwnPtrStack *segments,
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

static int lm_trans_call_frame_result_type(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_,
    LmTransL4CallableType *out
) {
    LmTransCallLowering *call;
    LmOwnPtrStack *segments;
    int status;

    if (frame == 0 || out == 0) {
        return 0;
    }

    call = lm_trans_expr_call_lowering_new();
    segments = lm_trans_expr_segment_stack_new();
    if (call == 0 || segments == 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_trans_expr_segment_stack_delete(&segments);
        return -1;
    }

        status = lm_trans_lower_call(frame->head, namespace_, "function", call);
    if (status != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_trans_expr_segment_stack_delete(&segments);
        return -1;
    }
    if (call->signature == 0) {
        fprintf(stderr, "trans L2 type error: C-surface call has no Lingvamyxa result type\n");
        lm_trans_expr_call_lowering_delete(call);
        lm_trans_expr_segment_stack_delete(&segments);
        return -1;
    }
    if (lm_trans_call_args_layout(segments, frame->body, call->signature) != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_trans_expr_segment_stack_delete(&segments);
        return -1;
    }

    status = lm_trans_callable_return_type_from_node(call->signature->callable_return_node, out) ? 1 : 0;
    lm_trans_expr_call_lowering_delete(call);
    lm_trans_expr_segment_stack_delete(&segments);
    return status;
}

static int lm_trans_node_result_type(
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    LmTransL4CallableType *out
) {
    const LmP0Node *current;
    const LmP0Field *field;

    if (out == 0) {
        return -1;
    }
    if (lm_trans_callable_type_reset(out) != 0) {
        return -1;
    }

    current = node;
    while (current != 0 && current->kind == LM_P0_NODE_STRUCTURE) {
        field = current->as->structure->first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        current = field->value;
    }

    if (current != 0 && current->kind == LM_P0_NODE_FRAME) {
        return lm_trans_call_frame_result_type(current->as->frame, namespace_, out);
    }

    return 0;
}

static int lm_trans_expr_stack_schedule_call_args(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *segments;
    int status;

    segments = lm_trans_expr_segment_stack_new();
    if (segments == 0) {
        return 1;
    }
    status = lm_trans_call_args_layout(segments, body, callee);
    if (status == 0) {
        status = lm_trans_expr_stack_push_segments(file, stack, segments, namespace_);
    }
    if (status == 0 && callee != 0 && callee->has_env_arg) {
        if (segments->count > 0U && lm_trans_expr_stack_push_text(stack, ", ") != 0) {
            status = 1;
        } else if (lm_trans_expr_stack_push_text(stack, callee->env_arg->data) != 0) {
            status = 1;
        }
    }
    lm_trans_expr_segment_stack_delete(&segments);
    return status;
}

static void lm_trans_strip_c_prefix(const LmP0Text *text, LmP0Text *out_text) {
    if (out_text == 0) {
        return;
    }
    out_text->data = "";
    out_text->length = 0U;
    if (text == 0) {
        return;
    }
    *out_text = *text;
    if (lm_trans_text_starts_with(out_text, "c.") && out_text->length > 2U) {
        out_text->data += 2U;
        out_text->length -= 2U;
    }
}

static const LmP0Node *lm_trans_single_type_body_node(const LmP0Structure *body) {
    const LmP0Field *field;
    const LmP0Field *inner_field;

    if (body == 0) {
        return 0;
    }

    field = body->first_field;
    if (field == 0 || field->next != 0 || field->value == 0) {
        return 0;
    }

    if (field->value->kind == LM_P0_NODE_STRUCTURE) {
        inner_field = field->value->as->structure->first_field;
        if (inner_field == 0 || inner_field->next != 0) {
            return 0;
        }
        return inner_field->value;
    }

    return field->value;
}

static int lm_trans_cast_type_base_key(const LmP0Node *type_node, LmP0Text *out_key) {
    const LmP0Node *current;
    const LmP0Node *inner;

    if (out_key != 0) {
        out_key->data = "";
        out_key->length = 0U;
    }
    if (type_node == 0 || out_key == 0) {
        return 0;
    }

    current = type_node;
    while (current != 0) {
        if (current->kind == LM_P0_NODE_STRUCTURE) {
            inner = lm_trans_single_type_body_node(current->as->structure);
            if (inner == 0) {
                return 0;
            }
            current = inner;
            continue;
        }

        if (current->kind == LM_P0_NODE_ATOM) {
            lm_trans_strip_c_prefix(current->as->atom, out_key);
            return 1;
        }

        if (
            current->kind == LM_P0_NODE_FRAME &&
            (
                lm_trans_text_equals(current->as->frame->head, "const") ||
                lm_trans_text_all_char(current->as->frame->head, '@')
            )
        ) {
            inner = lm_trans_single_type_body_node(current->as->frame->body);
            if (inner == 0) {
                return 0;
            }
            current = inner;
            continue;
        }

        return 0;
    }

    return 0;
}

static int lm_trans_cast_type_is_allowed(
    const LmP0Node *type_node,
    const LmTransNamespace *namespace_
) {
    LmP0Text *key;

    key = lm_trans_text_ref_new_cstr("");
    if (key == 0) {
        return 0;
    }
    if (!lm_trans_cast_type_base_key(type_node, key)) {
        fprintf(stderr, "trans L2 error: cast target expects a type\n");
        lm_trans_text_ref_destroy(&key);
        return 0;
    }

    if (lm_trans_namespace_registry_lookup(namespace_, key, "cast.target") != 0) {
        lm_trans_text_ref_destroy(&key);
        return 1;
    }

    fprintf(
        stderr,
        "trans L2 error: cast target type \"%.*s\" is not listed in cast.target\n",
        (int)key->length,
        key->data
    );
    lm_trans_text_ref_destroy(&key);
    return 0;
}

static int lm_trans_expr_emit_cast_frame(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *type_field;
    const LmP0Field *expr_first;
    const LmP0Node *type_node;

    if (frame == 0) {
        return 0;
    }

    type_field = lm_trans_call_body_first_field(frame->body);
    if (type_field == 0 || type_field->value == 0) {
        fprintf(stderr, "trans L2 error: cast expects a target type and an expression\n");
        return 1;
    }

    expr_first = type_field->next;
    if (expr_first == 0) {
        fprintf(stderr, "trans L2 error: cast expects an expression after the target type\n");
        return 1;
    }

    type_node = type_field->value;
    if (type_node->kind == LM_P0_NODE_STRUCTURE) {
        type_node = lm_trans_single_type_body_node(type_node->as->structure);
    }
    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: cast target expects a type\n");
        return 1;
    }

    if (!lm_trans_cast_type_is_allowed(type_node, namespace_)) {
        return 1;
    }

    if (
        lm_trans_put(file, "((") != 0 ||
        lm_trans_emit_type_node(file, type_node) != 0 ||
        lm_trans_put(file, ")") != 0 ||
        lm_trans_expr_stack_push_text(stack, ")") != 0 ||
        lm_trans_expr_stack_push_range(stack, expr_first, 0) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_lookup_expr_frame_receiver_binding(
    const LmTransNamespace *namespace_,
    const LmP0Text *head,
    LmTransBinding *out
) {
    const char *binding;

    if (out != 0) {
        memset(out, 0, sizeof(*out));
    }
    if (out == 0) {
        return -1;
    }

    binding = lm_trans_namespace_registry_lookup(namespace_, head, "receiver.expr");
    if (binding == 0) {
        return 0;
    }

    if (!lm_trans_binding_resolve(binding, out) || out->expr_frame == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: receiver.expr[\"%.*s\"] has unknown expression receiver binding %s\n",
            (int)head->length,
            head->data,
            binding
        );
        return -1;
    }

    return 1;
}

static int lm_trans_expr_stack_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    LmTransCallLowering *call;
    LmTransBinding *expr_receiver;
    int expr_receiver_status;
    int has_args;
    int status;

    if (frame == 0) {
        return 0;
    }

    call = lm_trans_expr_call_lowering_new();
    expr_receiver = lm_trans_expr_binding_new();
    if (call == 0 || expr_receiver == 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }

    expr_receiver_status = lm_trans_lookup_expr_frame_receiver_binding(namespace_, frame->head, expr_receiver);
    if (expr_receiver_status < 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }
    if (expr_receiver_status > 0) {
        status = expr_receiver->expr_frame(file, stack, frame, namespace_);
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return status;
    }

    if (lm_trans_lower_call(frame->head, namespace_, "function", call) != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }

    if (call->is_closure) {
        has_args = call->signature != 0 && call->signature->param_names != 0 && call->signature->param_names->count > 0U;
        if (!has_args && lm_trans_call_body_first_field(frame->body) != 0) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (lm_trans_emit_name(file, call->name) != 0) {
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (lm_trans_put(file, "->call(") != 0) {
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (lm_trans_emit_name(file, call->name) != 0) {
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (lm_trans_put(file, "->env") != 0) {
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return 1;
        }
        if (has_args) {
            if (lm_trans_expr_stack_push_call_args(stack, frame->body, call->signature) != 0) {
                lm_trans_expr_call_lowering_delete(call);
                lm_own_delete(expr_receiver, 0);
                return 1;
            }
            status = lm_trans_expr_stack_push_text(stack, ", ");
            lm_trans_expr_call_lowering_delete(call);
            lm_own_delete(expr_receiver, 0);
            return status;
        }
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 0;
    }

    if (lm_trans_emit_name(file, call->name) != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
        lm_trans_expr_call_lowering_delete(call);
        lm_own_delete(expr_receiver, 0);
        return 1;
    }
    status = lm_trans_expr_stack_push_call_args(stack, frame->body, call->signature);
    lm_trans_expr_call_lowering_delete(call);
    lm_own_delete(expr_receiver, 0);
    return status;
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
        return lm_trans_emit_expr_atom(file, node->as->atom, namespace_);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_expr_stack_push_frame(stack, node->as->frame);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
            return 1;
        }
        if (lm_trans_expr_stack_push_range(stack, node->as->structure->first_field, 0) != 0) {
            return 1;
        }
        return lm_trans_expr_stack_push_text(stack, "(");
    }

    return 0;
}

static int lm_trans_atom_is_operand_like(const LmP0Text *text) {
    return
        !lm_trans_text_is_operator_atom(text) &&
        !lm_trans_text_equals(text, "@") &&
        !lm_trans_text_equals(text, "\\");
}

static int lm_trans_atom_is_index_operator(const LmP0Text *text) {
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
    LmTransBinding *resolved;
    int status;

    resolved = lm_trans_expr_binding_new();
    if (resolved == 0) {
        return 1;
    }

    if (
        lowering == 0 ||
        !lm_trans_binding_resolve(binding, resolved) ||
        resolved->expr_emit == 0 ||
        resolved->expr_state == 0
    ) {
        lm_own_delete(resolved, 0);
        return 1;
    }

    lowering->emit = resolved->expr_emit;
    lowering->update = resolved->expr_state;
    status = 0;
    lm_own_delete(resolved, 0);
    return status;
}

static void lm_trans_expr_atom_lowering_set_builtin(
    LmTransExprAtomLowering *lowering,
    const LmP0Text *text,
    const char *binding
) {
    if (lowering != 0 && lowering->text != 0 && text != 0) {
        *lowering->text = *text;
    }
    if (lm_trans_expr_atom_lowering_set_binding(lowering, binding) != 0) {
        lowering->emit = lm_trans_expr_emit_value;
        lowering->update = lm_trans_expr_state_value;
    }
}

static int lm_trans_expr_atom_lowering_set_from_tables(
    LmTransExprAtomLowering *lowering,
    const LmP0Text *key,
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
            key != 0 ? (int)key->length : 0,
            key != 0 ? key->data : "",
            spelling_table,
            emitter_table
        );
        return 1;
    }

    lm_trans_text_assign_cstr(lowering->text, spelling);
    return 0;
}

static int lm_trans_expr_atom_lowering_set_from_class(
    LmTransExprAtomLowering *lowering,
    const LmP0Text *key,
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

    lm_trans_expr_atom_lowering_set_builtin(out, node->as->atom, "lm_trans_expr_emit_value");

    if (expect_field_name) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as->atom, "lm_trans_expr_emit_field_name");
    } else if (expect_c_field_name) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as->atom, "lm_trans_expr_emit_c_field_name");
    } else if (lm_trans_is_c_reference_name(node->as->atom)) {
        lm_trans_expr_atom_lowering_set_builtin(out, node->as->atom, "lm_trans_expr_emit_c_surface");
    } else if (lm_trans_text_equals(node->as->atom, "\\")) {
        if (previous_operand != 0 && lm_trans_nodes_touch(previous_operand, node)) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as->atom,
                "operator.infix"
            );
        } else {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as->atom,
                "operator.prefix"
            );
        }
    } else if (lm_trans_registry_has_expr_emitter_binding(node->as->atom, "expr.atom.emitter")) {
        return lm_trans_expr_atom_lowering_set_from_tables(
            out,
            node->as->atom,
            "expr.atom.ansi_c",
            "expr.atom.emitter"
        );
    } else if (lm_trans_atom_is_index_operator(node->as->atom)) {
        return lm_trans_expr_atom_lowering_set_from_class(
            out,
            node->as->atom,
            "operator.index"
        );
    } else if (lm_trans_atom_is_infix_expr_operator(node->as->atom, node, previous_operand)) {
        return lm_trans_expr_atom_lowering_set_from_class(
            out,
                node->as->atom,
                "operator.infix"
        );
    } else if (lm_trans_text_is_operator_atom(node->as->atom)) {
        if (lm_trans_registry_has_expr_emitter_class(node->as->atom, "operator.prefix")) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as->atom,
                "operator.prefix"
            );
        }
        if (lm_trans_registry_has_expr_emitter_class(node->as->atom, "operator.postfix")) {
            return lm_trans_expr_atom_lowering_set_from_class(
                out,
                node->as->atom,
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
    const LmTransExprAtomLowering *lowering
) {
    LmTransExprPiece *piece;
    int status;

    piece = lm_trans_expr_piece_new();
    if (piece == 0 || lowering == 0) {
        lm_trans_expr_piece_delete(piece);
        return 1;
    }
    piece->emit = lm_trans_expr_piece_emit_atom;
    piece->leading_space = leading_space;
    piece->node = node;
    if (lm_trans_expr_atom_lowering_copy(piece->atom, lowering) != 0) {
        lm_trans_expr_piece_delete(piece);
        return 1;
    }
    piece->first = 0;
    piece->stop = 0;
    status = lm_own_ptr_stack_push(range->pieces, piece);
    if (status != 0) {
        lm_trans_expr_piece_delete(piece);
    }
    return status;
}

static int lm_trans_expr_lowered_range_append_node(
    LmTransExprLoweredRange *range,
    int leading_space,
    const LmP0Node *node
) {
    LmTransExprPiece *piece;
    int status;

    piece = lm_trans_expr_piece_new();
    if (piece == 0) {
        return 1;
    }
    piece->emit = lm_trans_expr_piece_emit_node;
    piece->leading_space = leading_space;
    piece->node = node;
    piece->first = 0;
    piece->stop = 0;
    status = lm_own_ptr_stack_push(range->pieces, piece);
    if (status != 0) {
        lm_trans_expr_piece_delete(piece);
    }
    return status;
}

static int lm_trans_expr_lowered_range_append_index(
    LmTransExprLoweredRange *range,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprPiece *piece;
    int status;

    piece = lm_trans_expr_piece_new();
    if (piece == 0) {
        return 1;
    }
    piece->emit = lm_trans_expr_piece_emit_index;
    piece->leading_space = 0;
    piece->node = 0;
    piece->first = first;
    piece->stop = stop;
    status = lm_own_ptr_stack_push(range->pieces, piece);
    if (status != 0) {
        lm_trans_expr_piece_delete(piece);
    }
    return status;
}

static int lm_trans_field_is_atom(const LmP0Field *field, const char *spelling) {
    return
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as->atom, spelling);
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
    const LmTransExprRangeJob *range
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
    int address_next_operand;
    int piece_leading_space;
    int wrote;
    LmTransExprAtomLowering *atom;

    if (lowered == 0 || range == 0) {
        return 1;
    }

    atom = lm_trans_expr_atom_lowering_new();
    if (atom == 0) {
        return 1;
    }

    field = range->field;
    previous_operand = range->previous_operand;
    expect_field_name = range->expect_field_name;
    expect_c_field_name = range->expect_c_field_name;
    c_dot_path = range->c_dot_path;
    wrote = range->wrote;
    address_next_operand = 0;
    while (field != range->stop) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_field_is_atom(field, "[")) {
                const LmP0Field *close;

                if (previous_operand == 0) {
                    fprintf(stderr, "trans L2 error: index operator expects a target expression\n");
                    return 1;
                }
                close = lm_trans_find_matching_index_close(field, range->stop);
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
                wrote = 1;
                field = close->next;
                continue;
            } else if (node->kind == LM_P0_NODE_ATOM) {
                piece_leading_space = wrote;
                if (lm_trans_lower_expr_atom(
                    node,
                    previous_operand,
                    expect_field_name,
                    expect_c_field_name,
                    c_dot_path,
                    atom
                ) != 0) {
                    lm_trans_expr_atom_lowering_delete(atom);
                    return 1;
                }
                if (
                    address_next_operand &&
                    !expect_field_name &&
                    !expect_c_field_name &&
                    lm_trans_atom_is_operand_like(node->as->atom)
                ) {
                    lm_trans_expr_atom_lowering_set_builtin(
                        atom,
                        node->as->atom,
                        "lm_trans_expr_emit_addressable_value"
                    );
                    piece_leading_space = 0;
                }
                if (
                    lm_trans_update_expr_atom_lowering_state(
                        atom,
                        node,
                        &previous_operand,
                        &expect_field_name,
                        &expect_c_field_name,
                        &c_dot_path
                    ) != 0
                ) {
                    lm_trans_expr_atom_lowering_delete(atom);
                    return 1;
                }
                if (lm_trans_text_equals(node->as->atom, "@")) {
                    address_next_operand = 1;
                } else if (lm_trans_atom_is_operand_like(node->as->atom)) {
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
                    lm_trans_expr_atom_lowering_delete(atom);
                    return 1;
                }
            } else {
                if (
                    lm_trans_expr_lowered_range_append_node(
                        lowered,
                        wrote,
                        node
                    ) != 0
                ) {
                    lm_trans_expr_atom_lowering_delete(atom);
                    return 1;
                }
                previous_operand = node;
                expect_field_name = 0;
                expect_c_field_name = 0;
                c_dot_path = 0;
                address_next_operand = 0;
            }
            wrote = 1;
        }
        field = field->next;
    }

    if (expect_field_name) {
        fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
        lm_trans_expr_atom_lowering_delete(atom);
        return 1;
    }
    if (expect_c_field_name) {
        fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
        lm_trans_expr_atom_lowering_delete(atom);
        return 1;
    }

    lm_trans_expr_atom_lowering_delete(atom);
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
    if (lm_trans_emit_expr_atom_lowering(file, piece->atom, namespace_) != 0) {
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
    (void)file;
    (void)namespace_;
    if (out_suspend != 0) {
        *out_suspend = 1;
    }
    ++lowered->index;
    if (lm_trans_expr_stack_push_lowered_range(stack, lowered) != 0) {
        return 1;
    }
    if (lm_trans_expr_stack_push_node(stack, piece->node) != 0) {
        lm_trans_expr_stack_pop_delete(stack);
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
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, "]") != 0) {
        lm_trans_expr_stack_pop_delete(stack);
        return 1;
    }
    if (lm_trans_expr_stack_push_range(stack, piece->first, piece->stop) != 0) {
        lm_trans_expr_stack_pop_delete(stack);
        lm_trans_expr_stack_pop_delete(stack);
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

    while (lowered->index < lowered->pieces->count) {
        piece = (const LmTransExprPiece *)lm_own_ptr_stack_at(lowered->pieces, lowered->index);
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
    const LmTransExprRangeJob *range,
    const LmTransNamespace *namespace_
) {
    LmTransExprLoweredRange *lowered;

    if (range == 0) {
        return 1;
    }

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
    return lm_trans_put(file, job->text);
}

static int lm_trans_expr_job_emit_name_text(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_emit_name(file, job->name_text);
}

static int lm_trans_expr_job_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_node(file, stack, job->node, namespace_);
}

static int lm_trans_expr_job_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_frame(file, stack, job->frame, namespace_);
}

static int lm_trans_expr_job_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    return lm_trans_expr_stack_emit_range(file, stack, job->range, namespace_);
}

static int lm_trans_expr_job_schedule_call_args(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprJob *job,
    const LmTransNamespace *namespace_
) {
    if (job->call_args == 0) {
        return 1;
    }
    return lm_trans_expr_stack_schedule_call_args(
        file,
        stack,
        job->call_args->body,
        job->call_args->callee,
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

    range = job->lowered_range;
    job->lowered_range = 0;
    job->destroy = 0;
    return lm_trans_expr_stack_emit_lowered_range(file, stack, range, namespace_);
}

static int lm_trans_emit_expr_stack_run(
    FILE *file,
    LmTransExprJob *initial,
    const LmTransNamespace *namespace_
) {
    LmTransExprStack *stack;
    LmTransExprJob *job;
    int status;

    stack = lm_trans_expr_stack_new();
    if (stack == 0 || initial == 0) {
        lm_trans_expr_job_delete(initial);
        lm_own_delete(stack, 0);
        return 1;
    }
    stack->jobs = lm_trans_ptr_stack_new(lm_trans_expr_job_delete_any);
    if (stack->jobs == 0) {
        lm_trans_expr_job_delete(initial);
        lm_trans_expr_stack_destroy(stack);
        lm_own_delete(stack, 0);
        return 1;
    }
    if (lm_trans_expr_stack_push(stack, initial) != 0) {
        lm_trans_expr_job_delete(initial);
        lm_trans_expr_stack_destroy(stack);
        lm_own_delete(stack, 0);
        return 1;
    }
    initial = 0;

    status = 0;
    while (status == 0 && stack->jobs != 0 && stack->jobs->count > 0U) {
        job = (LmTransExprJob *)lm_own_ptr_stack_pop(stack->jobs);
        if (job == 0) {
            status = 1;
            break;
        }
        if (job->run == 0) {
            status = 1;
        } else {
            status = job->run(file, stack, job, namespace_);
        }
        lm_trans_expr_job_delete(job);
    }

    lm_trans_expr_stack_destroy(stack);
    lm_own_delete(stack, 0);
    return status;
}

static int lm_trans_emit_call_args(
    FILE *file,
    const LmP0Structure *body,
    const LmTransNamespace *namespace_,
    const LmTransSymbol *callee
) {
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_schedule_call_args;
    job->destroy = lm_trans_expr_job_destroy_call_args;
    job->call_args = (LmTransExprCallArgsJob *)lm_own_new_zero(sizeof(*job->call_args));
    if (job->call_args == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    job->call_args->body = body;
    job->call_args->callee = callee;
    status = lm_trans_emit_expr_stack_run(file, job, namespace_);
    return status;
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
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_node;
    job->destroy = 0;
    job->node = node;
    status = lm_trans_emit_expr_stack_run(file, job, namespace_);
    return status;
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
    LmTransExprJob *job;
    int status;

    job = lm_trans_expr_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_expr_job_emit_range;
    job->destroy = lm_trans_expr_job_destroy_range;
    job->range = lm_trans_expr_range_job_new();
    if (job->range == 0) {
        lm_trans_expr_job_delete(job);
        return 1;
    }
    job->range->field = first;
    job->range->stop = stop;
    job->range->wrote = 0;
    job->range->previous_operand = 0;
    job->range->expect_field_name = 0;
    job->range->expect_c_field_name = 0;
    job->range->c_dot_path = 0;
    status = lm_trans_emit_expr_stack_run(file, job, namespace_);
    return status;
}

static int lm_trans_array_type_node_info(
    const LmP0Node *type_node,
    const LmP0Node **out_element_type,
    size_t *out_rank
);
static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node);
static int lm_trans_emit_size_literal(FILE *file, size_t value);

static int lm_trans_frame_receiver_key(const LmP0Frame *frame, LmP0Text *out_key) {
    if (frame == 0 || out_key == 0) {
        return 0;
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        lm_trans_text_assign_cstr(out_key, "@");
        return 1;
    }
    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        lm_trans_text_assign_cstr(out_key, "[]");
        return 1;
    }

    *out_key = *frame->head;
    return 1;
}

static int lm_trans_type_receiver_key(
    const LmP0Node *type_node,
    LmP0Text *out_key
) {
    if (out_key == 0) {
        return 0;
    }

    if (type_node != 0 && type_node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_frame_receiver_key(type_node->as->frame, out_key);
    }

    return 0;
}

static int lm_trans_lookup_type_receiver_binding(
    const LmP0Node *type_node,
    LmTransBinding *out
) {
    LmP0Text *key;
    const char *binding;
    LmTransBinding *resolved;
    int found;

    if (out != 0) {
        memset(out, 0, sizeof(*out));
    }
    key = lm_trans_text_ref_new_cstr("");
    if (out == 0 || key == 0 || !lm_trans_type_receiver_key(type_node, key)) {
        lm_trans_text_ref_destroy(&key);
        return 0;
    }
    resolved = lm_trans_expr_binding_new();
    if (resolved == 0) {
        lm_trans_text_ref_destroy(&key);
        return -1;
    }

    found = 0;

    binding = lm_trans_registry_lookup(key, "class.receiver.emit");
    if (binding != 0) {
        memset(resolved, 0, sizeof(*resolved));
        if (!lm_trans_binding_resolve(binding, resolved) || resolved->type_emit == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.emit[\"%.*s\"] has unknown class emit binding %s\n",
                (int)key->length,
                key->data,
                binding
            );
            lm_own_delete(resolved, 0);
            lm_trans_text_ref_destroy(&key);
            return -1;
        }
        out->type_emit = resolved->type_emit;
        found = 1;
    }

    binding = lm_trans_registry_lookup(key, "class.receiver.structure_value_alloc");
    if (binding != 0) {
        memset(resolved, 0, sizeof(*resolved));
        if (!lm_trans_binding_resolve(binding, resolved) || resolved->type_structure_value_alloc == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.structure_value_alloc[\"%.*s\"] has unknown class Structure-value alloc binding %s\n",
                (int)key->length,
                key->data,
                binding
            );
            lm_own_delete(resolved, 0);
            lm_trans_text_ref_destroy(&key);
            return -1;
        }
        out->type_structure_value_alloc = resolved->type_structure_value_alloc;
        found = 1;
    }

    binding = lm_trans_registry_lookup(key, "class.receiver.structure_value_fill");
    if (binding != 0) {
        memset(resolved, 0, sizeof(*resolved));
        if (!lm_trans_binding_resolve(binding, resolved) || resolved->type_structure_value_fill == 0) {
            fprintf(
                stderr,
                "trans registry inconsistency: class.receiver.structure_value_fill[\"%.*s\"] has unknown class Structure-value fill binding %s\n",
                (int)key->length,
                key->data,
                binding
            );
            lm_own_delete(resolved, 0);
            lm_trans_text_ref_destroy(&key);
            return -1;
        }
        out->type_structure_value_fill = resolved->type_structure_value_fill;
        found = 1;
    }

    if (!found) {
        lm_own_delete(resolved, 0);
        lm_trans_text_ref_destroy(&key);
        return 0;
    }

    lm_own_delete(resolved, 0);
    lm_trans_text_ref_destroy(&key);
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

typedef struct LmTransTypePointerSuffix {
    size_t depth;
} LmTransTypePointerSuffix;

static LmTransTypePointerSuffix *lm_trans_type_pointer_suffix_new(size_t depth) {
    LmTransTypePointerSuffix *suffix;

    suffix = (LmTransTypePointerSuffix *)lm_own_new_zero(sizeof(*suffix));
    if (suffix != 0) {
        suffix->depth = depth;
    }
    return suffix;
}

static void lm_trans_type_suffix_stack_delete(LmOwnPtrStack **suffixes) {
    if (suffixes != 0 && *suffixes != 0) {
        lm_own_ptr_stack_destroy(*suffixes);
        lm_own_delete(*suffixes, 0);
        *suffixes = 0;
    }
}

static int lm_trans_type_suffix_stack_push(LmOwnPtrStack **suffixes, size_t depth) {
    LmTransTypePointerSuffix *suffix;

    if (suffixes == 0) {
        return 1;
    }
    if (*suffixes == 0) {
        *suffixes = (LmOwnPtrStack *)lm_own_new_zero(sizeof(**suffixes));
        if (*suffixes == 0) {
            return 1;
        }
        lm_own_ptr_stack_init(*suffixes, lm_own_delete_plain);
    }
    suffix = lm_trans_type_pointer_suffix_new(depth);
    if (suffix == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(*suffixes, suffix) != 0) {
        lm_own_delete(suffix, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_emit_type_pointer_suffix(FILE *file, size_t depth) {
    size_t i;

    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_emit_type_suffixes(FILE *file, LmOwnPtrStack *suffixes) {
    LmTransTypePointerSuffix *suffix;

    while (suffixes != 0 && suffixes->count != 0U) {
        suffix = (LmTransTypePointerSuffix *)lm_own_ptr_stack_pop(suffixes);
        if (suffix != 0) {
            if (lm_trans_emit_type_pointer_suffix(file, suffix->depth) != 0) {
                lm_own_delete(suffix, 0);
                return 1;
            }
            lm_own_delete(suffix, 0);
        }
    }
    return 0;
}

static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node) {
    const LmP0Node *current;
    const LmP0Field *field;
    LmOwnPtrStack *suffixes;
    LmTransBinding *type_receiver;
    int type_receiver_status;
    int status;

    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: type position expects a name\n");
        return 1;
    }

    current = type_node;
    suffixes = 0;
    status = 0;
    while (1) {
        if (current == 0) {
            fprintf(stderr, "trans L2 error: type position expects a name\n");
            status = 1;
            break;
        }

        if (current->kind == LM_P0_NODE_ATOM) {
            status = lm_trans_emit_type_name(file, current->as->atom);
            break;
        }

        if (
            current->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_equals(current->as->frame->head, "const")
        ) {
            field = current->as->frame->body->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                fprintf(stderr, "trans L2 error: const type qualifier expects exactly one type\n");
                status = 1;
                break;
            }
            if (lm_trans_put(file, "const ") != 0) {
                status = 1;
                break;
            }
            current = field->value;
            continue;
        }

        if (
            current->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_all_char(current->as->frame->head, '@')
        ) {
            field = current->as->frame->body->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                fprintf(stderr, "trans L2 error: pointer type expects exactly one type\n");
                status = 1;
                break;
            }
            if (lm_trans_type_suffix_stack_push(&suffixes, current->as->frame->head->length) != 0) {
                status = 1;
                break;
            }
            current = field->value;
            continue;
        }

        type_receiver = lm_trans_expr_binding_new();
        if (type_receiver == 0) {
            status = 1;
            break;
        }
        type_receiver_status = lm_trans_lookup_type_receiver_binding(current, type_receiver);
        if (type_receiver_status < 0) {
            lm_own_delete(type_receiver, 0);
            status = 1;
            break;
        }
        if (type_receiver_status > 0) {
            if (type_receiver->type_emit == 0) {
                fprintf(stderr, "trans registry error: type receiver has no type emitter\n");
                lm_own_delete(type_receiver, 0);
                status = 1;
                break;
            }
            status = type_receiver->type_emit(file, current);
            lm_own_delete(type_receiver, 0);
            break;
        }
        lm_own_delete(type_receiver, 0);
        fprintf(stderr, "trans L2 error: type position expects a name\n");
        status = 1;
        break;
    }
    if (status == 0) {
        status = lm_trans_emit_type_suffixes(file, suffixes);
    }
    lm_trans_type_suffix_stack_delete(&suffixes);
    return status;
}

static int lm_trans_head_can_declare_storage(
    const LmP0Text *head,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_callable_descriptor_param_type(
    FILE *file,
    const LmP0Node *type_node,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_callable_descriptor_param_type(
    FILE *file,
    const LmP0Node *type_node,
    const LmTransNamespace *namespace_
) {
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    LmOwnPtrStack *suffixes;
    int status;

    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: callable descriptor parameter expects a type\n");
        return 1;
    }

    current = type_node;
    suffixes = 0;
    status = 0;
    while (1) {
        if (current == 0) {
            fprintf(stderr, "trans L2 error: callable descriptor parameter expects a type\n");
            status = 1;
            break;
        }

        if (current->kind == LM_P0_NODE_STRUCTURE) {
            field = current->as->structure->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                fprintf(stderr, "trans L2 error: callable descriptor parameter Structure must contain exactly one type\n");
                status = 1;
                break;
            }
            current = field->value;
            continue;
        }

        if (current->kind == LM_P0_NODE_FRAME) {
            frame = current->as->frame;

            if (lm_trans_text_equals(frame->head, "const")) {
                field = frame->body->first_field;
                if (field == 0 || field->next != 0 || field->value == 0) {
                    fprintf(stderr, "trans L2 error: const callable descriptor parameter expects one type\n");
                    status = 1;
                    break;
                }
                if (lm_trans_put(file, "const ") != 0) {
                    status = 1;
                    break;
                }
                current = field->value;
                continue;
            }

            body = lm_trans_unwrap_single_anonymous_structure(frame->body);
            type_field = body != 0 ? body->first_field : 0;

            if (lm_trans_text_all_char(frame->head, '@')) {
                if (type_field == 0 || type_field->value == 0) {
                    fprintf(stderr, "trans L2 error: @ callable descriptor parameter expects a type\n");
                    status = 1;
                    break;
                }
                if (lm_trans_type_suffix_stack_push(&suffixes, frame->head->length) != 0) {
                    status = 1;
                    break;
                }
                current = type_field->value;
                continue;
            }

            if (lm_trans_text_is_array_receiver_head(frame->head)) {
                if (type_field == 0 || type_field->value == 0) {
                    fprintf(stderr, "trans L2 error: [] callable descriptor parameter expects a type\n");
                    status = 1;
                    break;
                }
                if (lm_trans_type_suffix_stack_push(&suffixes, 1U) != 0) {
                    status = 1;
                    break;
                }
                current = type_field->value;
                continue;
            }

            if (lm_trans_head_can_declare_storage(frame->head, namespace_)) {
                status = lm_trans_emit_type_name(file, frame->head);
                break;
            }

            fprintf(stderr, "trans L2 error: unsupported callable descriptor parameter type\n");
            status = 1;
            break;
        }

        status = lm_trans_emit_type_node(file, current);
        break;
    }

    if (status == 0) {
        status = lm_trans_emit_type_suffixes(file, suffixes);
    }
    lm_trans_type_suffix_stack_delete(&suffixes);
    return status;
}

static int lm_trans_emit_function_return_struct_type_name(FILE *file, const LmP0Text *function_name) {
    if (lm_trans_write_text(file, function_name) != 0) {
        return 1;
    }
    return lm_trans_put(file, "Return");
}

static char *lm_trans_function_return_struct_type_name_new(const LmP0Text *function_name) {
    char *name;
    const char *suffix;
    size_t suffix_length;

    if (function_name == 0) {
        return 0;
    }
    suffix = "Return";
    suffix_length = strlen(suffix);
    name = (char *)lm_own_new_zero(function_name->length + suffix_length + 1U);
    if (name == 0) {
        return 0;
    }
    memcpy(name, function_name->data, function_name->length);
    memcpy(name + function_name->length, suffix, suffix_length + 1U);
    return name;
}

static int lm_trans_emit_current_return_type(FILE *file, const LmTransNamespace *namespace_) {
    if (namespace_ != 0 && namespace_->return_type_is_struct) {
        if (namespace_->return_type_name == 0) {
            return 1;
        }
        return lm_trans_emit_function_return_struct_type_name(file, namespace_->return_type_name);
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
    LmP0Text *dimension;
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
        !lm_trans_text_is_array_receiver_head(type_node->as->frame->head)
    ) {
        return 0;
    }

    body = lm_trans_unwrap_single_anonymous_structure(type_node->as->frame->body);
    field = body != 0 ? body->first_field : 0;
    if (field == 0 || field->next != 0 || field->value == 0) {
        fprintf(stderr, "trans L2 error: [] type receiver expects exactly one element type\n");
        return -1;
    }

    dimension_index = 0U;
    rank = 0U;
    dimension = lm_trans_text_ref_new_cstr("");
    if (dimension == 0) {
        return -1;
    }
    while (lm_trans_array_head_next_dimension(type_node->as->frame->head, &dimension_index, dimension)) {
        if (dimension->length != 0U) {
            fprintf(stderr, "trans L2 error: [] type receiver dimensions must be empty in pointer type position\n");
            lm_trans_text_ref_destroy(&dimension);
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
    lm_trans_text_ref_destroy(&dimension);
    return 1;
}

static int lm_trans_emit_type_and_name(
    FILE *file,
    const LmP0Node *type_node,
    const LmP0Text *name,
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
    const LmP0Text *type_head
) {
    if (lm_trans_builtin_c_type_name(type_head)) {
        return lm_trans_emit_type_name(file, type_head);
    }
    return lm_trans_emit_name(file, type_head);
}

static int lm_trans_emit_c_dimension_text(
    FILE *file,
    const LmP0Text *dimension,
    const LmTransNamespace *namespace_,
    const char *error_name
) {
    LmP0Document *dimension_document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *dimension_root;
    int parse_status;

    if (dimension == 0 || dimension->length == 0U) {
        return 0;
    }

    dimension_document = 0;
    parse_status = lm_p0_parse_bytes(dimension->data, dimension->length, &dimension_document);
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
        dimension_root->as->structure->first_field == 0
    ) {
        fprintf(stderr, "trans L2 error: %s must not be empty\n", error_name != 0 ? error_name : "array dimension");
        lm_p0_document_destroy(dimension_document);
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, dimension_root->as->structure->first_field, namespace_) != 0) {
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
    LmP0Text *head_dimension;

    if (file == 0 || declarator == 0) {
        return 1;
    }

    if (declarator->type_is_head) {
        if (declarator->type_head == 0 || lm_trans_emit_type_head_only(file, declarator->type_head) != 0) {
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
    if (declarator->name == 0 || lm_trans_emit_identifier(file, declarator->name) != 0) {
        return 1;
    }

    head_index = 0U;
    head_dimension = lm_trans_text_ref_new_cstr("");
    if (head_dimension == 0) {
        return 1;
    }
    if (declarator->array_head == 0) {
        return 1;
    }
    while (lm_trans_array_head_next_dimension(declarator->array_head, &head_index, head_dimension)) {
        if (lm_trans_put(file, "[") != 0) {
            lm_trans_text_ref_destroy(&head_dimension);
            return 1;
        }
        if (head_dimension->length != 0U) {
            if (lm_trans_emit_c_dimension_text(file, head_dimension, namespace_, dimension_error_name) != 0) {
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
        } else {
            if (declarator->expression_dimensions == 0 || declarator->expression_dimensions->value == 0) {
                fprintf(stderr, "trans L2 error: %s expects a size expression\n", dimension_error_name != 0 ? dimension_error_name : "array dimension");
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            if (lm_trans_emit_expr_node(file, declarator->expression_dimensions->value, namespace_) != 0) {
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            declarator->expression_dimensions = declarator->expression_dimensions->next;
        }
        if (lm_trans_put(file, "]") != 0) {
            lm_trans_text_ref_destroy(&head_dimension);
            return 1;
        }
    }
    lm_trans_text_ref_destroy(&head_dimension);

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
    const LmP0Text *head,
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
    LmP0Text *head_dimension;
    size_t head_index;
    size_t pointer_depth;

    if (frame == 0) {
        return 1;
    }

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;

    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as->atom, '@')
    ) {
        pointer_depth += field->value->as->atom->length;
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
            name_node->as->atom,
            pointer_depth,
            namespace_
        ) != 0
    ) {
        return 1;
    }

    dimension_field = name_field->next;
    head_index = 0U;
    head_dimension = lm_trans_text_ref_new_cstr("");
    if (head_dimension == 0) {
        return 1;
    }
    while (lm_trans_array_head_next_dimension(frame->head, &head_index, head_dimension)) {
        if (lm_trans_put(file, "[") != 0) {
            lm_trans_text_ref_destroy(&head_dimension);
            return 1;
        }
        if (head_dimension->length != 0U) {
            LmP0Document *dimension_document;
            const LmP0Diagnostic *diagnostic;
            const LmP0Node *dimension_root;
            int parse_status;

            dimension_document = 0;
            parse_status = lm_p0_parse_bytes(head_dimension->data, head_dimension->length, &dimension_document);
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
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            dimension_root = lm_p0_document_root(dimension_document);
            if (
                dimension_root == 0 ||
                dimension_root->kind != LM_P0_NODE_STRUCTURE ||
                dimension_root->as->structure->first_field == 0
            ) {
                fprintf(stderr, "trans L2 error: array parameter dimension must not be empty\n");
                lm_p0_document_destroy(dimension_document);
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            if (lm_trans_emit_expr_fields(file, dimension_root->as->structure->first_field, namespace_) != 0) {
                lm_p0_document_destroy(dimension_document);
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            lm_p0_document_destroy(dimension_document);
        } else if (lm_trans_array_body_dimension_can_consume(dimension_field)) {
            if (lm_trans_emit_expr_node(file, dimension_field->value, namespace_) != 0) {
                lm_trans_text_ref_destroy(&head_dimension);
                return 1;
            }
            dimension_field = dimension_field->next;
        }
        if (lm_trans_put(file, "]") != 0) {
            lm_trans_text_ref_destroy(&head_dimension);
            return 1;
        }
    }

    lm_trans_text_ref_destroy(&head_dimension);
    return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
}

static int lm_trans_emit_param(FILE *file, const LmP0Node *node, LmTransNamespace *namespace_) {
    const LmP0Node *current;
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

    current = node;
    while (
        current != 0 &&
        current->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(current->as->frame->head, "const")
    ) {
        field0 = current->as->frame->body->first_field;
        param_node = 0;
        if (
            field0 != 0 &&
            field0->next == 0 &&
            field0->value != 0
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as->structure->first_field;
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
        current = param_node;
    }

    if (current == 0 || current->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: parameter must be a typed frame\n");
        return 1;
    }

    field0 = lm_trans_nth_field(current->as->frame->body, 0U);
    field1 = lm_trans_nth_field(current->as->frame->body, 1U);

    if (lm_trans_head_can_declare_storage(current->as->frame->head, namespace_)) {
        if (field0 == 0 || field0->value == 0 || field0->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: typed parameter expects a name\n");
            return 1;
        }
        name_node = field0->value;
        if (
            lm_trans_symbol_is(lm_trans_namespace_find(namespace_, current->as->frame->head), "callableDescriptor") &&
            lm_trans_callable_descriptor_is_raw_function_reference(namespace_, current->as->frame->head)
        ) {
            if (
                lm_trans_emit_raw_callable_declarator(
                    file,
                    namespace_,
                    current->as->frame->head,
                    name_node->as->atom,
                    "callable descriptor parameter"
                ) != 0
            ) {
                return 1;
            }
            return lm_trans_namespace_declare_storage_binding(
                namespace_,
                name_node->as->atom,
                current->as->frame->head
            );
        }
        if (lm_trans_emit_name(file, current->as->frame->head) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, name_node->as->atom) != 0) {
            return 1;
        }
        return lm_trans_namespace_declare_storage_binding(
            namespace_,
            name_node->as->atom,
            current->as->frame->head
        );
    }

    if (lm_trans_text_all_char(current->as->frame->head, '@')) {
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
                name_node->as->atom,
                current->as->frame->head->length,
                namespace_
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
    }

    if (lm_trans_text_is_array_receiver_head(current->as->frame->head)) {
        return lm_trans_emit_array_param(file, current->as->frame, namespace_);
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

static LmP0Text *lm_trans_statement_text_new(void) {
    return lm_trans_text_ref_new_cstr("");
}

static LmTransBinding *lm_trans_statement_binding_new(void) {
    return (LmTransBinding *)lm_own_new_zero(sizeof(LmTransBinding));
}

static void lm_trans_statement_c_declarator_destroy(LmTransCDeclarator *declarator);

static LmTransCDeclarator *lm_trans_statement_c_declarator_new(void) {
    LmTransCDeclarator *declarator;

    declarator = (LmTransCDeclarator *)lm_own_new_zero(sizeof(LmTransCDeclarator));
    if (declarator != 0) {
        declarator->type_head = lm_trans_statement_text_new();
        declarator->name = lm_trans_statement_text_new();
        declarator->array_head = lm_trans_statement_text_new();
        if (declarator->type_head == 0 || declarator->name == 0 || declarator->array_head == 0) {
            lm_trans_text_ref_destroy(&declarator->type_head);
            lm_trans_text_ref_destroy(&declarator->name);
            lm_trans_text_ref_destroy(&declarator->array_head);
            lm_trans_statement_c_declarator_destroy(declarator);
            return 0;
        }
    }
    return declarator;
}

static void lm_trans_statement_c_declarator_destroy(LmTransCDeclarator *declarator) {
    if (declarator != 0) {
        lm_trans_text_ref_destroy(&declarator->type_head);
        lm_trans_text_ref_destroy(&declarator->name);
        lm_trans_text_ref_destroy(&declarator->array_head);
    }
    lm_own_delete(declarator, 0);
}

static LmTransCallLowering *lm_trans_statement_call_lowering_new(void) {
    return lm_trans_expr_call_lowering_new();
}

static LmTransStatementJob *lm_trans_statement_job_new(void) {
    return (LmTransStatementJob *)lm_own_new_zero(sizeof(LmTransStatementJob));
}

static void lm_trans_statement_job_destroy_list(LmTransStatementJob *job) {
    if (job != 0) {
        lm_own_delete(job->list, 0);
        job->list = 0;
    }
}

static void lm_trans_statement_job_destroy_node(LmTransStatementJob *job) {
    if (job != 0) {
        lm_own_delete(job->node, 0);
        job->node = 0;
    }
}

static void lm_trans_statement_job_destroy_frame(LmTransStatementJob *job) {
    if (job != 0) {
        lm_own_delete(job->frame, 0);
        job->frame = 0;
    }
}

static void lm_trans_statement_job_destroy_text(LmTransStatementJob *job) {
    if (job != 0) {
        lm_own_delete(job->text, 0);
        job->text = 0;
    }
}

static void lm_trans_statement_job_destroy_sync_leave(LmTransStatementJob *job) {
    if (job != 0) {
        lm_own_delete(job->sync_leave, 0);
        job->sync_leave = 0;
    }
}

static void lm_trans_statement_job_destroy(LmTransStatementJob *job) {
    if (job != 0 && job->destroy != 0) {
        job->destroy(job);
        job->destroy = 0;
    }
}

static void lm_trans_statement_job_delete(LmTransStatementJob *job) {
    lm_trans_statement_job_destroy(job);
    lm_own_delete(job, 0);
}

static void lm_trans_statement_job_delete_any(void *object) {
    lm_trans_statement_job_delete((LmTransStatementJob *)object);
}

static LmTransStatementStack *lm_trans_statement_stack_new(void) {
    return (LmTransStatementStack *)lm_own_new_zero(sizeof(LmTransStatementStack));
}

static LmTransHeadBinding *lm_trans_statement_head_binding_new(void) {
    return (LmTransHeadBinding *)lm_own_new_zero(sizeof(LmTransHeadBinding));
}

static LmTransFunctionHeader *lm_trans_statement_function_header_new(void) {
    return lm_trans_function_header_new();
}

static LmTransStatementLowering *lm_trans_statement_lowering_new(void) {
    return (LmTransStatementLowering *)lm_own_new_zero(sizeof(LmTransStatementLowering));
}

static LmTransCapture *lm_trans_statement_capture_new(void) {
    LmTransCapture *capture;

    capture = (LmTransCapture *)lm_own_new_zero(sizeof(*capture));
    if (capture != 0 && lm_trans_capture_init_fields(capture) != 0) {
        lm_trans_capture_destroy(capture);
        return 0;
    }
    return capture;
}

static LmP0Node *lm_trans_statement_node_new(void) {
    return (LmP0Node *)lm_own_new_zero(sizeof(LmP0Node));
}

static LmP0NodeAs *lm_trans_statement_node_as_new(void) {
    return (LmP0NodeAs *)lm_own_new_zero(sizeof(LmP0NodeAs));
}

static LmP0Frame *lm_trans_statement_frame_new(void) {
    return (LmP0Frame *)lm_own_new_zero(sizeof(LmP0Frame));
}

static int lm_trans_params_has_any(const LmP0Node *params) {
    return
        params != 0 &&
        params->kind == LM_P0_NODE_STRUCTURE &&
        params->as->structure->first_field != 0;
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

    if (frame == 0 || frame->body->first_field == 0) {
        return 0;
    }

    end = lm_trans_expr_segment_end(frame->body->first_field);
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

    first = frame->body->first_field;
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
    LmP0Text *receiver_key;
    int result;

    if (frame == 0 || out_index == 0) {
        return 0;
    }

    receiver_key = lm_trans_statement_text_new();
    if (receiver_key == 0) {
        return 0;
    }

    result = 0;
    if (!lm_trans_frame_receiver_key(frame, receiver_key)) {
        goto cleanup;
    }

    index_payload = lm_trans_registry_lookup(receiver_key, "receiver.positional-name.index");
    if (index_payload != 0) {
        if (!lm_trans_parse_size_payload(index_payload, out_index)) {
            fprintf(
                stderr,
                "trans registry error: receiver.positional-name.index for \"%.*s\" must be a non-negative integer\n",
                (int)receiver_key->length,
                receiver_key->data
            );
            exit(2);
        }
        result = 1;
        goto cleanup;
    }

    if (lm_trans_registry_has(receiver_key, "receiver.positional-name.argument")) {
        *out_index = 0U;
        result = 1;
        goto cleanup;
    }

cleanup:
    lm_trans_text_ref_destroy(&receiver_key);
    return result;
}

static int lm_trans_frame_formal_param_unwrap_index(const LmP0Frame *frame, size_t *out_index) {
    const char *index_payload;
    LmP0Text *receiver_key;
    int result;

    if (frame == 0 || out_index == 0) {
        return 0;
    }
    receiver_key = lm_trans_statement_text_new();
    if (receiver_key == 0) {
        return 0;
    }
    result = 0;
    if (!lm_trans_frame_receiver_key(frame, receiver_key)) {
        goto cleanup;
    }

    index_payload = lm_trans_registry_lookup(receiver_key, "receiver.formal-param.unwrap");
    if (index_payload == 0) {
        goto cleanup;
    }
    if (!lm_trans_parse_size_payload(index_payload, out_index)) {
        fprintf(
            stderr,
            "trans registry error: receiver.formal-param.unwrap for \"%.*s\" must be a non-negative integer\n",
            (int)receiver_key->length,
            receiver_key->data
        );
        exit(2);
    }
    result = 1;

cleanup:
    lm_trans_text_ref_destroy(&receiver_key);
    return result;
}

static int lm_trans_name_argument_from_frame(const LmP0Frame *frame, LmP0Text *out_name) {
    const LmP0Field *field;
    const LmP0Field *name_field;
    const LmP0Frame *child_frame;
    size_t name_index;

    if (frame == 0 || out_name == 0) {
        return 0;
    }

    field = frame->body->first_field;
    while (field != 0) {
        if (field->value != 0 && field->value->kind == LM_P0_NODE_FRAME) {
            child_frame = field->value->as->frame;
            if (lm_trans_text_equals(child_frame->head, "name")) {
                name_field = child_frame->body->first_field;
                if (
                    name_field != 0 &&
                    name_field->next == 0 &&
                    name_field->value != 0 &&
                    name_field->value->kind == LM_P0_NODE_ATOM &&
                    lm_trans_name_argument_is_valid(name_field->value->as->atom)
                ) {
                    *out_name = *name_field->value->as->atom;
                    return 1;
                }
            }
        }
        field = field->next;
    }

    if (!lm_trans_frame_positional_name_index(frame, &name_index)) {
        return 0;
    }

    field = lm_trans_nth_field(frame->body, name_index);
    if (field == 0 || field->value == 0) {
        return 0;
    }

    if (
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_name_argument_is_valid(field->value->as->atom)
    ) {
        *out_name = *field->value->as->atom;
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

    *out_target = *frame->head;
    return 1;
}

static int lm_trans_validate_end_trailer(const LmP0Frame *frame) {
    LmP0Text *actual;
    LmP0Text *expected_name;
    LmP0Text *expected_head;
    int status;

    if (frame == 0 || frame->trailer == 0) {
        return 0;
    }

    if (!lm_trans_text_equals(frame->trailer->spelling, "end")) {
        return 0;
    }

    actual = lm_trans_statement_text_new();
    expected_name = lm_trans_statement_text_new();
    expected_head = lm_trans_statement_text_new();
    if (actual == 0 || expected_name == 0 || expected_head == 0) {
        lm_trans_text_ref_destroy(&actual);
        lm_trans_text_ref_destroy(&expected_name);
        lm_trans_text_ref_destroy(&expected_head);
        return 1;
    }
    status = 0;

    if (!lm_trans_trailer_single_atom(frame->trailer, actual)) {
        fprintf(stderr, "trans error: end trailer expects exactly one target name\n");
        status = 1;
        goto cleanup;
    }

    if (!lm_trans_frame_close_target(frame, expected_name)) {
        fprintf(
            stderr,
            "trans error: head \"%.*s\" does not expose a named close target\n",
            (int)frame->head->length,
            frame->head->data
        );
        status = 1;
        goto cleanup;
    }

    if (!lm_trans_frame_receiver_key(frame, expected_head)) {
        fprintf(
            stderr,
            "trans error: head \"%.*s\" does not expose a receiver close target\n",
            (int)frame->head->length,
            frame->head->data
        );
        status = 1;
        goto cleanup;
    }

    if (
        !lm_trans_identifier_same(actual, expected_name) &&
        !lm_trans_identifier_same(actual, expected_head)
    ) {
        fprintf(
            stderr,
            "trans error: end target \"%.*s\" does not match close target \"%.*s\" or receiver head \"%.*s\"\n",
            (int)actual->length,
            actual->data,
            (int)expected_name->length,
            expected_name->data,
            (int)expected_head->length,
            expected_head->data
        );
        status = 1;
        goto cleanup;
    }

cleanup:
    lm_trans_text_ref_destroy(&actual);
    lm_trans_text_ref_destroy(&expected_name);
    lm_trans_text_ref_destroy(&expected_head);
    return status;
}

static int lm_trans_emit_cleanups_until(
    FILE *file,
    unsigned indent,
    const LmTransNamespace *namespace_,
    size_t cleanup_base
) {
    size_t index;
    const LmTransCleanup *cleanup;

    if (namespace_ == 0 || cleanup_base > namespace_->cleanups->count) {
        return 0;
    }

    index = namespace_->cleanups->count;
    while (index > cleanup_base) {
        --index;
        cleanup = (const LmTransCleanup *)lm_own_ptr_stack_at(namespace_->cleanups, index);
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

    *out_atom = *return_fields->value->as->atom;
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

    return fields->value->as->structure;
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
    const LmP0Text *target_name,
    const size_t *indices,
    size_t depth
) {
    size_t i;

    if (target_name == 0 || lm_trans_emit_identifier(file, target_name) != 0) {
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
    const LmP0Text *target_name,
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
    const LmP0Text *target_name,
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

typedef struct LmTransArrayStructureValueFillFrame {
    const LmP0Field *field;
    size_t depth;
    size_t index;
} LmTransArrayStructureValueFillFrame;

static LmTransArrayStructureValueFillFrame *lm_trans_array_structure_value_fill_frame_new(
    const LmP0Field *field,
    size_t depth
) {
    LmTransArrayStructureValueFillFrame *frame;

    frame = (LmTransArrayStructureValueFillFrame *)lm_own_new_zero(sizeof(*frame));
    if (frame != 0) {
        frame->field = field;
        frame->depth = depth;
        frame->index = 0U;
    }
    return frame;
}

static int lm_trans_emit_array_structure_value_fill_iterative(
    FILE *file,
    unsigned indent,
    const LmP0Text *target_name,
    const LmP0Node *element_type,
    size_t rank,
    size_t depth,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    size_t *indices
) {
    LmOwnPtrStack *stack;
    LmTransArrayStructureValueFillFrame *frame;
    const LmP0Field *field;
    const LmP0Structure *child;
    size_t count;
    size_t current_depth;
    size_t index;
    int status;

    if (rank == 0U || depth >= rank) {
        return 1;
    }
    if (value == 0 || value->first_field == 0) {
        return 0;
    }

    stack = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (stack == 0) {
        return 1;
    }
    frame = lm_trans_array_structure_value_fill_frame_new(value != 0 ? value->first_field : 0, depth);
    if (frame == 0) {
        lm_trans_ptr_stack_delete(&stack);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        lm_trans_ptr_stack_delete(&stack);
        return 1;
    }

    status = 0;
    while (status == 0 && stack->count != 0U) {
        frame = (LmTransArrayStructureValueFillFrame *)lm_own_ptr_stack_top(stack);
        if (frame == 0 || frame->field == 0) {
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            continue;
        }

        field = frame->field;
        current_depth = frame->depth;
        index = frame->index;
        frame->field = frame->field->next;
        ++frame->index;
        indices[current_depth] = index;

        if (current_depth + 1U == rank) {
            if (lm_trans_emit_indent(file, indent) != 0) {
                status = 1;
                break;
            }
            if (lm_trans_emit_array_target_path(file, target_name, indices, current_depth + 1U) != 0) {
                status = 1;
                break;
            }
            if (lm_trans_put(file, " = ") != 0) {
                status = 1;
                break;
            }
            if (lm_trans_emit_expr_node(file, field->value, namespace_) != 0) {
                status = 1;
                break;
            }
            if (lm_trans_put(file, ";\n") != 0) {
                status = 1;
                break;
            }
            continue;
        }

        if (field->value == 0 || field->value->kind != LM_P0_NODE_STRUCTURE) {
            fprintf(stderr, "trans L2 error: [] multi-dimensional Structure value expects nested [] Structure fields\n");
            status = 1;
            break;
        }
        child = field->value->as->structure;
        count = lm_trans_structure_field_count(child);
        if (
            lm_trans_emit_array_structure_value_alloc_assignment(
                file,
                indent,
                target_name,
                indices,
                current_depth + 1U,
                element_type,
                rank - current_depth - 1U,
                rank,
                count
            ) != 0
        ) {
            status = 1;
            break;
        }
        if (
            count != 0U &&
            lm_trans_emit_array_null_return_check(
                file,
                indent,
                target_name,
                indices,
                current_depth + 1U,
                namespace_
            ) != 0
        ) {
            status = 1;
            break;
        }
        if (child != 0 && child->first_field != 0) {
            frame = lm_trans_array_structure_value_fill_frame_new(child->first_field, current_depth + 1U);
            if (frame == 0) {
                status = 1;
                break;
            }
            if (lm_own_ptr_stack_push(stack, frame) != 0) {
                lm_own_delete(frame, 0);
                status = 1;
                break;
            }
        }
    }

    lm_trans_ptr_stack_delete(&stack);
    return status;
}

static int lm_trans_emit_array_structure_value_fill(
    FILE *file,
    unsigned indent,
    const LmP0Text *target_name,
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
    indices = (size_t *)lm_own_new_zero(rank * sizeof(*indices));
    if (indices == 0) {
        return 1;
    }
    status = lm_trans_emit_array_structure_value_fill_iterative(
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
    lm_own_delete(indices, 0);
    return status;
}

static int lm_trans_type_receiver_array_structure_value_alloc(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    const LmP0Text *target_name,
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
    if (out_consumed == 0 || out_needs_null_check == 0 || target_name == 0 || value == 0) {
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
    LmTransBinding *type_receiver;
    int status;

    if (out_can_consume != 0) {
        *out_can_consume = 0;
    }
    if (out_can_consume == 0 || value == 0) {
        return 0;
    }

    type_receiver = lm_trans_statement_binding_new();
    if (type_receiver == 0) {
        return 1;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, type_receiver);
    if (status < 0) {
        lm_own_delete(type_receiver, 0);
        return 1;
    }
    if (status == 0) {
        lm_own_delete(type_receiver, 0);
        return 0;
    }
    if (type_receiver->type_structure_value_alloc == 0 || type_receiver->type_structure_value_fill == 0) {
        lm_own_delete(type_receiver, 0);
        return 0;
    }

    *out_can_consume = 1;
    lm_own_delete(type_receiver, 0);
    return 0;
}

static int lm_trans_type_receiver_array_structure_value_fill(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    const LmP0Text *target_name,
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
    if (out_consumed == 0 || target_name == 0 || value == 0) {
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
    const LmP0Text *target_name,
    const LmP0Structure *value,
    int *out_consumed,
    int *out_needs_null_check
) {
    LmTransBinding *type_receiver;
    int status;
    int result;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_needs_null_check != 0) {
        *out_needs_null_check = 0;
    }
    if (out_consumed == 0 || out_needs_null_check == 0 || target_name == 0 || value == 0) {
        return 0;
    }

    type_receiver = lm_trans_statement_binding_new();
    if (type_receiver == 0) {
        return 1;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, type_receiver);
    if (status < 0) {
        lm_own_delete(type_receiver, 0);
        return 1;
    }
    if (status == 0 || type_receiver->type_structure_value_alloc == 0) {
        lm_own_delete(type_receiver, 0);
        return 0;
    }

    result = type_receiver->type_structure_value_alloc(
        file,
        indent,
        type_node,
        target_name,
        value,
        out_consumed,
        out_needs_null_check
    );
    lm_own_delete(type_receiver, 0);
    return result;
}

static int lm_trans_emit_type_receiver_structure_value_fill(
    FILE *file,
    unsigned indent,
    const LmP0Node *type_node,
    const LmP0Text *target_name,
    const LmP0Structure *value,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    LmTransBinding *type_receiver;
    int status;
    int result;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0 || target_name == 0 || value == 0) {
        return 0;
    }

    type_receiver = lm_trans_statement_binding_new();
    if (type_receiver == 0) {
        return 1;
    }

    status = lm_trans_lookup_type_receiver_binding(type_node, type_receiver);
    if (status < 0) {
        lm_own_delete(type_receiver, 0);
        return 1;
    }
    if (status == 0 || type_receiver->type_structure_value_fill == 0) {
        lm_own_delete(type_receiver, 0);
        return 0;
    }

    result = type_receiver->type_structure_value_fill(
        file,
        indent,
        type_node,
        target_name,
        value,
        namespace_,
        out_consumed
    );
    lm_own_delete(type_receiver, 0);
    return result;
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

    return segment->first->value->as->structure;
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

    name = (char *)lm_own_new_zero((size_t)written + 1U);
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
    LmP0Text *helper_name_text;
    LmP0Text *target_name;
    char *helper_name;
    size_t rank;
    size_t pointer_depth;
    size_t count;
    int status;

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

    helper_name_text = lm_trans_text_ref_new_cstr(helper_name);
    target_name = lm_trans_text_ref_new_cstr("lm_value");
    if (helper_name_text == 0 || target_name == 0) {
        lm_own_delete(helper_name, 0);
        lm_trans_text_ref_destroy(&helper_name_text);
        lm_trans_text_ref_destroy(&target_name);
        return 1;
    }
    prelude_file = lm_trans_prelude_file(file);
    count = lm_trans_structure_field_count(value);
    status = 0;

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
        status = 1;
        goto cleanup_error;
    }

    if (
        count != 0U &&
        (
            lm_trans_put(prelude_file, "    if (") != 0 ||
            lm_trans_write_text(prelude_file, target_name) != 0 ||
            lm_trans_put(prelude_file, " == 0) {\n        return 0;\n    }\n") != 0
        )
    ) {
        status = 1;
        goto cleanup_error;
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
        status = 1;
        goto cleanup_error;
    }

    if (
        lm_trans_put(prelude_file, "    return ") != 0 ||
        lm_trans_write_text(prelude_file, target_name) != 0 ||
        lm_trans_put(prelude_file, ";\n}\n\n") != 0
    ) {
        status = 1;
        goto cleanup_error;
    }

    if (lm_own_ptr_stack_push(lm_trans_emitted_array_value_helpers, helper_name) != 0) {
        status = 1;
        goto cleanup_error;
    }

    *out_name = *helper_name_text;
    lm_trans_text_ref_destroy(&helper_name_text);
    lm_trans_text_ref_destroy(&target_name);
    return 0;

cleanup_error:
    if (status != 0) {
        lm_own_delete(helper_name, 0);
    }
    lm_trans_text_ref_destroy(&helper_name_text);
    lm_trans_text_ref_destroy(&target_name);
    return status;
}

static int lm_trans_materialize_array_value(
    FILE *file,
    LmTransExprStack *stack,
    const LmTransExprSegment *segment,
    const LmTransNamespace *namespace_,
    int *out_consumed
) {
    const LmP0Structure *value;
    LmP0Text *helper_name;
    int status;

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

    helper_name = lm_trans_statement_text_new();
    if (helper_name == 0) {
        return 1;
    }
    status = 0;
    if (
        lm_trans_emit_array_value_helper(
            file,
            segment != 0 ? segment->expected_param : 0,
            value,
            namespace_,
            helper_name
        ) != 0 ||
        lm_trans_expr_stack_push_text(stack, ")") != 0 ||
        lm_trans_expr_stack_push_text(stack, "(") != 0 ||
        lm_trans_expr_stack_push_name_text(stack, helper_name) != 0
    ) {
        status = 1;
        goto cleanup;
    }

    *out_consumed = 1;
cleanup:
    lm_trans_text_ref_destroy(&helper_name);
    return status;
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
    LmP0Text *return_name;
    char return_name_storage[64];
    unsigned return_id;
    int can_consume;
    int needs_null_check;
    int filled;
    int status;

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

    return_name = lm_trans_statement_text_new();
    if (return_name == 0) {
        return 1;
    }
    status = 0;
    return_id = namespace_->next_return_id++;
    if (lm_trans_return_name_text(return_id, return_name_storage, sizeof(return_name_storage), return_name) != 0) {
        status = 1;
        goto cleanup;
    }

    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "{\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_current_return_type(file, namespace_) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, " ") != 0 || lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
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
        status = 1;
        goto cleanup;
    }
    if (!*out_consumed) {
        status = lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");
        goto cleanup;
    }
    if (needs_null_check) {
        if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "if (") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, " == 0) {\n") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_cleanups_until(file, indent + 2U, namespace_, 0U) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_indent(file, indent + 2U) != 0 || lm_trans_put(file, "return 0;\n") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "}\n") != 0) {
            status = 1;
            goto cleanup;
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
        status = 1;
        goto cleanup;
    }
    if (!filled) {
        fprintf(stderr, "trans L2 error: return type receiver accepted Structure value allocation but not fill\n");
        status = 1;
        goto cleanup;
    }

    if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "return ") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_write_text(file, return_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    status = lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");

cleanup:
    lm_trans_text_ref_destroy(&return_name);
    return status;
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
    const LmP0Text *name,
    size_t *out_index
) {
    const LmP0Field *field;
    LmP0Text *field_name;
    size_t index;
    int result;

    if (descriptor == 0 || out_index == 0) {
        return 0;
    }

    field_name = lm_trans_statement_text_new();
    if (field_name == 0) {
        return 0;
    }
    result = 0;
    index = 0U;
    field = descriptor->first_field;
    while (field != 0) {
        if (
            field->value != 0 &&
            lm_trans_formal_param_name(field->value, field_name) &&
            lm_trans_identifier_same(field_name, name)
        ) {
            *out_index = index;
            result = 1;
            goto cleanup;
        }
        ++index;
        field = field->next;
    }

cleanup:
    lm_trans_text_ref_destroy(&field_name);
    return result;
}

static LmTransStructReturnFieldValue **lm_trans_struct_return_field_values_new(size_t count) {
    LmTransStructReturnFieldValue **values;
    size_t index;

    values = (LmTransStructReturnFieldValue **)lm_own_new_zero(sizeof(values[0]) * count);
    if (values == 0) {
        return 0;
    }
    for (index = 0U; index < count; ++index) {
        values[index] = (LmTransStructReturnFieldValue *)lm_own_new_zero(sizeof(values[index][0]));
        if (values[index] == 0) {
            while (index != 0U) {
                --index;
                lm_own_delete(values[index], 0);
            }
            lm_own_delete(values, 0);
            return 0;
        }
    }
    return values;
}

static void lm_trans_struct_return_field_values_delete(
    LmTransStructReturnFieldValue **values,
    size_t count
) {
    size_t index;

    if (values == 0) {
        return;
    }
    for (index = 0U; index < count; ++index) {
        lm_own_delete(values[index], 0);
    }
    lm_own_delete(values, 0);
}

static int lm_trans_struct_return_collect_value_fields(
    const LmP0Structure *descriptor,
    const LmP0Structure *value,
    LmTransStructReturnFieldValue **fields,
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
            (field->value->as->frame->flags & LM_P0_FRAME_COLON) != 0U &&
            lm_trans_struct_return_field_index(descriptor, field->value->as->frame->head, &named_index)
        ) {
            frame = field->value->as->frame;
            if (fields[named_index]->present) {
                fprintf(stderr, "trans L2 error: duplicate named fm return field\n");
                return 1;
            }
            if (frame->body->first_field == 0) {
                fprintf(stderr, "trans L2 error: named fm return field expects a value\n");
                return 1;
            }
            fields[named_index]->first = frame->body->first_field;
            fields[named_index]->stop = 0;
            fields[named_index]->present = 1;
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
        if (fields[positional_index]->present) {
            fprintf(stderr, "trans L2 error: duplicate positional fm return field\n");
            return 1;
        }
        fields[positional_index]->first = field;
        fields[positional_index]->stop = field->next;
        fields[positional_index]->present = 1;
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
    LmTransStructReturnFieldValue **values;
    LmP0Text *return_name;
    LmP0Text *field_name;
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

    descriptor = namespace_->return_type_node->as->structure;
    field_count = lm_trans_structure_field_count(descriptor);
    if (field_count == 0U) {
        fprintf(stderr, "trans L2 error: fm return Structure must not be empty\n");
        return 1;
    }

    return_name = lm_trans_statement_text_new();
    field_name = lm_trans_statement_text_new();
    if (return_name == 0 || field_name == 0) {
        lm_trans_text_ref_destroy(&return_name);
        lm_trans_text_ref_destroy(&field_name);
        return 1;
    }
    values = lm_trans_struct_return_field_values_new(field_count);
    if (values == 0) {
        lm_trans_text_ref_destroy(&return_name);
        lm_trans_text_ref_destroy(&field_name);
        return 1;
    }

    status = lm_trans_struct_return_collect_value_fields(descriptor, value, values, field_count);
    if (status == 0) {
        for (index = 0U; index < field_count; ++index) {
            if (!values[index]->present) {
                fprintf(stderr, "trans L2 error: fm Structure return literal does not fill every return field\n");
                status = 1;
                break;
            }
        }
    }
    if (status != 0) {
        goto cleanup;
    }

    if (
        lm_trans_return_name_text(namespace_->next_return_id++, return_name_storage, sizeof(return_name_storage), return_name) != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "{\n") != 0 ||
        lm_trans_emit_indent(file, indent + 1U) != 0 ||
        lm_trans_emit_current_return_type(file, namespace_) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_write_text(file, return_name) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        status = 1;
        goto cleanup;
    }

    for (index = 0U; index < field_count; ++index) {
        if (!lm_trans_struct_return_field_name_at(descriptor, index, field_name)) {
            fprintf(stderr, "trans L2 error: fm return field must expose a binding name\n");
            status = 1;
            goto cleanup;
        }
        if (
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_write_text(file, return_name) != 0 ||
            lm_trans_put(file, ".") != 0 ||
            lm_trans_emit_identifier(file, field_name) != 0 ||
            lm_trans_put(file, " = ") != 0 ||
            lm_trans_emit_expr_range(file, values[index]->first, values[index]->stop, namespace_) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            status = 1;
            goto cleanup;
        }
    }

    if (
        lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0 ||
        lm_trans_emit_indent(file, indent + 1U) != 0 ||
        lm_trans_put(file, "return ") != 0 ||
        lm_trans_write_text(file, return_name) != 0 ||
        lm_trans_put(file, ";\n") != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "}\n") != 0
    ) {
        status = 1;
        goto cleanup;
    }

    *out_consumed = 1;
    status = 0;

cleanup:
    lm_trans_struct_return_field_values_delete(values, field_count);
    lm_trans_text_ref_destroy(&return_name);
    lm_trans_text_ref_destroy(&field_name);
    return status;
}

static int lm_trans_current_return_is_callable_descriptor(
    const LmTransNamespace *namespace_,
    LmP0Text *out_type
) {
    const LmTransSymbol *symbol;
    LmP0Text *type_name;
    int result;

    if (
        namespace_ == 0 ||
        namespace_->return_type_node == 0 ||
        namespace_->return_type_node->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    type_name = lm_trans_statement_text_new();
    if (type_name == 0) {
        return 0;
    }

    *type_name = *namespace_->return_type_node->as->atom;
    symbol = lm_trans_namespace_find(namespace_, type_name);
    result = 0;
    if (!lm_trans_symbol_is(symbol, "callableDescriptor")) {
        goto cleanup;
    }
    if (out_type != 0) {
        *out_type = *type_name;
    }
    result = 1;

cleanup:
    lm_trans_text_ref_destroy(&type_name);
    return result;
}

static int lm_trans_emit_raw_callable_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_,
    int *out_consumed
) {
    LmP0Text *atom;
    LmP0Text *descriptor_type;
    LmP0Text *name;
    LmP0Text *return_name;
    const LmTransSymbol *symbol;
    char return_name_storage[64];
    unsigned return_id;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 0;
    }

    atom = lm_trans_statement_text_new();
    descriptor_type = lm_trans_statement_text_new();
    name = lm_trans_statement_text_new();
    return_name = lm_trans_statement_text_new();
    if (atom == 0 || descriptor_type == 0 || name == 0 || return_name == 0) {
        lm_trans_text_ref_destroy(&atom);
        lm_trans_text_ref_destroy(&descriptor_type);
        lm_trans_text_ref_destroy(&name);
        lm_trans_text_ref_destroy(&return_name);
        return 1;
    }

    status = 0;
    if (
        !lm_trans_return_fields_single_atom(return_fields, atom) ||
        !lm_trans_current_return_is_callable_descriptor(namespace_, descriptor_type) ||
        lm_trans_registry_is_function_pointer_type_name(namespace_, descriptor_type) ||
        !lm_trans_callable_descriptor_is_raw_function_reference(namespace_, descriptor_type)
    ) {
        goto cleanup;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (
        symbol == 0 ||
        !lm_trans_symbol_is_executable_callable(symbol) ||
        !lm_trans_callable_signature_matches_descriptor_name(descriptor_type, symbol, namespace_)
    ) {
        goto cleanup;
    }

    if (symbol->has_env_arg || lm_trans_symbol_is(symbol, "closure")) {
        fprintf(
            stderr,
            "trans L2 error: cannot return capturing callable %.*s as raw descriptor %.*s; use `lazy fn`\n",
            (int)atom->length,
            atom->data,
            (int)descriptor_type->length,
            descriptor_type->data
        );
        status = 1;
        goto cleanup;
    }

    *name = symbol->has_c_name ? *symbol->c_name : *atom;
    if (namespace_ != 0 && namespace_->cleanups->count > 0U) {
        return_id = namespace_->next_return_id++;
        if (lm_trans_return_name_text(return_id, return_name_storage, sizeof(return_name_storage), return_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "{\n") != 0 ||
            lm_trans_emit_indent(file, indent + 1U) != 0
        ) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_raw_callable_declarator(file, namespace_, descriptor_type, return_name, "raw callable return") != 0) {
            status = 1;
            goto cleanup;
        }
        if (
            lm_trans_put(file, " = ") != 0 ||
            lm_trans_write_text(file, name) != 0 ||
            lm_trans_put(file, ";\n") != 0 ||
            lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0 ||
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_put(file, "return ") != 0 ||
            lm_trans_emit_return_name(file, return_id) != 0 ||
            lm_trans_put(file, ";\n") != 0 ||
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "}\n") != 0
        ) {
            status = 1;
            goto cleanup;
        }
    } else {
        if (
            lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0 ||
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "return ") != 0 ||
            lm_trans_write_text(file, name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            status = 1;
            goto cleanup;
        }
    }

    *out_consumed = 1;

cleanup:
    lm_trans_text_ref_destroy(&atom);
    lm_trans_text_ref_destroy(&descriptor_type);
    lm_trans_text_ref_destroy(&name);
    lm_trans_text_ref_destroy(&return_name);
    return status;
}

static int lm_trans_emit_closure_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_,
    int *out_consumed
) {
    LmP0Text *atom;
    LmP0Text *descriptor_type;
    const LmTransSymbol *symbol;
    unsigned return_id;
    int status;

    if (out_consumed != 0) {
        *out_consumed = 0;
    }
    if (out_consumed == 0) {
        return 0;
    }
    atom = lm_trans_statement_text_new();
    descriptor_type = lm_trans_statement_text_new();
    if (atom == 0 || descriptor_type == 0) {
        lm_trans_text_ref_destroy(&atom);
        lm_trans_text_ref_destroy(&descriptor_type);
        return 1;
    }
    status = 0;
    if (
        !lm_trans_return_fields_single_atom(return_fields, atom) ||
        !lm_trans_current_return_is_callable_descriptor(namespace_, descriptor_type)
    ) {
        goto cleanup;
    }
    if (!lm_trans_callable_descriptor_is_lazy(namespace_, descriptor_type)) {
        goto cleanup;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (
        symbol == 0 ||
        !lm_trans_symbol_is(symbol, "function") ||
        !symbol->has_closure_call_name
    ) {
        goto cleanup;
    }

    *out_consumed = 1;
    return_id = namespace_->next_return_id++;

    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "{\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_write_text(file, descriptor_type) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, " ") != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, " = (") != 0 || lm_trans_write_text(file, descriptor_type) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, ")lm_own_new_zero(sizeof(*") != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, "));\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "if (") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, " == 0) {\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_cleanups_until(file, indent + 2U, namespace_, 0U) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 2U) != 0 || lm_trans_put(file, "return 0;\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "}\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, "->call = ") != 0 || lm_trans_write_text(file, symbol->closure_call_name) != 0 || lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, "->env = ") != 0) {
        status = 1;
        goto cleanup;
    }
    if (symbol->has_env_arg) {
        if (lm_trans_write_text(file, symbol->env_arg) != 0) {
            status = 1;
            goto cleanup;
        }
    } else if (lm_trans_put(file, "0") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_emit_return_name(file, return_id) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, "->destroy = 0;\n") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_indent(file, indent + 1U) != 0 || lm_trans_put(file, "return ") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_return_name(file, return_id) != 0 || lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    status = lm_trans_emit_indent(file, indent) || lm_trans_put(file, "}\n");

cleanup:
    lm_trans_text_ref_destroy(&atom);
    lm_trans_text_ref_destroy(&descriptor_type);
    return status;
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

    if (lm_trans_emit_raw_callable_return_statement(file, return_fields, indent, namespace_, &consumed) != 0) {
        return 1;
    }
    if (consumed) {
        return 0;
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

    if (namespace_ != 0 && namespace_->cleanups->count > 0U && namespace_->return_type_node != 0) {
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
        return lm_trans_emit_return_statement(file, trailer->body->first_field, indent, namespace_);
    }

    if (lm_trans_text_equals(trailer->spelling, "end")) {
        return 0;
    }

    fprintf(
        stderr,
        "trans error: unsupported trailer \"%.*s\"\n",
        (int)trailer->spelling->length,
        trailer->spelling->data
    );
    return 1;
}

static int lm_trans_namespace_declare_storage_binding(
    LmTransNamespace *namespace_,
    const LmP0Text *name,
    const LmP0Text *type_head
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

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
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
    if (
        lm_trans_symbol_is(lm_trans_namespace_find(namespace_, frame->head), "callableDescriptor") &&
        lm_trans_callable_descriptor_is_raw_function_reference(namespace_, frame->head)
    ) {
        if (lm_trans_emit_raw_callable_declarator(file, namespace_, frame->head, name_node->as->atom, "callable descriptor") != 0) {
            return 1;
        }
    } else if (lm_trans_builtin_c_type_name(frame->head)) {
        if (lm_trans_emit_type_name(file, frame->head) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, name_node->as->atom) != 0) {
            return 1;
        }
    } else {
        if (lm_trans_emit_name(file, frame->head) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_identifier(file, name_node->as->atom) != 0) {
            return 1;
        }
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
    return lm_trans_namespace_declare_storage_binding(namespace_, name_node->as->atom, frame->head);
}

static int lm_trans_namespace_declare_storage_binding(
    LmTransNamespace *namespace_,
    const LmP0Text *name,
    const LmP0Text *type_head
) {
    LmTransSymbol *symbol;
    const LmTransSymbol *type_symbol;

    type_symbol = lm_trans_namespace_find(namespace_, type_head);
    if (lm_trans_symbol_is(type_symbol, "callableDescriptor")) {
        if (
            lm_trans_namespace_declare(
                namespace_,
                name,
                lm_trans_callable_descriptor_is_raw_function_reference(namespace_, type_head)
                    ? "functionPointer"
                    : "closure"
            ) != 0
        ) {
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

static int lm_trans_atom_can_be_new_binding_name(const LmP0Text *text) {
    return lm_trans_atom_is_identifier_like(text);
}

static int lm_trans_head_can_declare_storage(
    const LmP0Text *head,
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

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    name_field = lm_trans_nth_field(body, 0U);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    return lm_trans_atom_can_be_new_binding_name(name_field->value->as->atom);
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

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
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
            name_node->as->atom,
            frame->head->length,
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
    return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
}

static int lm_trans_emit_pointer_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_pointer_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_array_initializer_node(
    FILE *file,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_array_initializer_fields(
    FILE *file,
    const LmP0Field *first,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    int wrote;

    wrote = 0;
    field = first;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (wrote && lm_trans_put(file, ", ") != 0) {
                return 1;
            }
            if (lm_trans_emit_array_initializer_node(file, field->value, namespace_) != 0) {
                return 1;
            }
            wrote = 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_array_initializer_structure(
    FILE *file,
    const LmP0Structure *structure,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_put(file, "{") != 0) {
        return 1;
    }
    if (lm_trans_emit_array_initializer_fields(file, structure != 0 ? structure->first_field : 0, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, "}");
}

static int lm_trans_emit_array_initializer_node(
    FILE *file,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
) {
    if (node == 0) {
        return 1;
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_emit_array_initializer_structure(file, node->as->structure, namespace_);
    }
    if (
        node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_is_array_receiver_head(node->as->frame->head)
    ) {
        return lm_trans_emit_array_initializer_structure(file, node->as->frame->body, namespace_);
    }
    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_expr_atom_mode(file, node->as->atom, namespace_, 1);
    }
    return lm_trans_emit_expr_node(file, node, namespace_);
}

static int lm_trans_emit_array_initializer(
    FILE *file,
    const LmP0Field *first,
    const LmTransNamespace *namespace_
) {
    if (
        first != 0 &&
        first->next == 0 &&
        first->value != 0 &&
        first->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        return lm_trans_emit_array_initializer_structure(file, first->value->as->structure, namespace_);
    }

    if (lm_trans_put(file, " = {") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_list(file, first, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, "}");
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
    LmTransCDeclarator *declarator;
    LmP0Text *type_head;
    size_t pointer_depth;
    int type_is_head;
    int status;

    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    field = body != 0 ? body->first_field : 0;
    pointer_depth = 0U;
    type_node = 0;
    declarator = 0;
    type_head = lm_trans_statement_text_new();
    if (type_head == 0) {
        return 1;
    }
    type_head->data = 0;
    type_head->length = 0U;
    type_is_head = 0;
    dimension_field = 0;
    status = 0;

    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = field->value->as->frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            name_field = inner->body->first_field;
            if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
                fprintf(stderr, "trans L2 error: array declaration expects a variable name\n");
                status = 1;
                goto cleanup;
            }
            *type_head = *inner->head;
            type_is_head = 1;
            dimension_field = name_field->next;
        } else if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(inner->body, 0U);
            name_field = lm_trans_nth_field(inner->body, 1U);
            if (
                type_field == 0 ||
                name_field == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                fprintf(stderr, "trans L2 error: array pointer declaration expects type and name\n");
                status = 1;
                goto cleanup;
            }
            pointer_depth = inner->head->length;
            type_node = type_field->value;
            dimension_field = name_field->next;
        } else {
            fprintf(stderr, "trans L2 error: array declaration expects a type/name declaration\n");
            status = 1;
            goto cleanup;
        }
    } else {
        while (
            field != 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_ATOM &&
            lm_trans_text_all_char(field->value->as->atom, '@')
        ) {
            pointer_depth += field->value->as->atom->length;
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
            status = 1;
            goto cleanup;
        }
        type_node = type_field->value;
        dimension_field = name_field->next;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        status = 1;
        goto cleanup;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        status = 1;
        goto cleanup;
    }
    declarator = lm_trans_statement_c_declarator_new();
    if (declarator == 0) {
        status = 1;
        goto cleanup;
    }
    declarator->type_node = type_node;
    *declarator->type_head = *type_head;
    *declarator->name = *name_node->as->atom;
    declarator->pointer_depth = pointer_depth;
    declarator->type_is_head = type_is_head;
    *declarator->array_head = *frame->head;
    declarator->expression_dimensions = dimension_field;
    if (lm_trans_emit_c_declarator(file, declarator, namespace_, "[] array dimension") != 0) {
        status = 1;
        goto cleanup;
    }

    initializer_field = declarator->expression_dimensions;
    if (initializer_field != 0) {
        if (lm_trans_emit_array_initializer(file, initializer_field, namespace_) != 0) {
            status = 1;
            goto cleanup;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    status = lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");

cleanup:
    lm_trans_statement_c_declarator_destroy(declarator);
    lm_trans_text_ref_destroy(&type_head);
    return status;
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

    template_body = template_frame != 0 ? lm_trans_unwrap_single_anonymous_structure(template_frame->body) : 0;
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
        lm_trans_head_can_declare_storage(name_field->value->as->atom, namespace_)
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
            name_node->as->atom,
            template_frame->head->length,
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
    return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
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
    LmTransCDeclarator *declarator;
    LmP0Text *type_head;
    LmP0Text *template_type_head;
    size_t pointer_depth;
    int type_is_head;
    int template_type_is_head;
    int status;

    template_body = template_frame != 0 ? lm_trans_unwrap_single_anonymous_structure(template_frame->body) : 0;
    repeat_body = lm_trans_unwrap_single_anonymous_structure(body);
    field = template_body != 0 ? template_body->first_field : 0;
    pointer_depth = 0U;
    type_node = 0;
    template_type_node = 0;
    declarator = 0;
    type_head = lm_trans_statement_text_new();
    template_type_head = lm_trans_statement_text_new();
    if (type_head == 0 || template_type_head == 0) {
        lm_trans_text_ref_destroy(&type_head);
        lm_trans_text_ref_destroy(&template_type_head);
        return 1;
    }
    type_head->data = 0;
    type_head->length = 0U;
    template_type_head->data = 0;
    template_type_head->length = 0U;
    type_is_head = 0;
    template_type_is_head = 0;
    status = 0;

    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = field->value->as->frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            *template_type_head = *inner->head;
            template_type_is_head = 1;
        } else if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(inner->body, 0U);
            if (type_field == 0 || type_field->value == 0) {
                fprintf(stderr, "trans L2 error: repeated array pointer declaration expects a type template\n");
                status = 1;
                goto cleanup;
            }
            pointer_depth = inner->head->length;
            template_type_node = type_field->value;
        } else {
            fprintf(stderr, "trans L2 error: repeated array declaration expects a type/name declaration template\n");
            status = 1;
            goto cleanup;
        }
    } else {
        while (
            field != 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_ATOM &&
            lm_trans_text_all_char(field->value->as->atom, '@')
        ) {
            pointer_depth += field->value->as->atom->length;
            field = field->next;
        }

        type_field = field;
        if (type_field == 0 || type_field->value == 0) {
            fprintf(stderr, "trans L2 error: repeated [] declaration expects a type template\n");
            status = 1;
            goto cleanup;
        }
        template_type_node = type_field->value;
    }

    name_field = repeat_body != 0 ? repeat_body->first_field : 0;
    if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans L2 error: repeated [] declaration expects a variable name\n");
        status = 1;
        goto cleanup;
    }

    type_node = template_type_node;
    *type_head = *template_type_head;
    type_is_head = template_type_is_head;
    if (
        name_field->next != 0 &&
        name_field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_head_can_declare_storage(name_field->value->as->atom, namespace_)
    ) {
        *type_head = *name_field->value->as->atom;
        type_node = 0;
        type_is_head = 1;
        pointer_depth = 0U;
        name_field = name_field->next;
        if (name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: repeated [] declaration override expects a variable name\n");
            status = 1;
            goto cleanup;
        }
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        status = 1;
        goto cleanup;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        status = 1;
        goto cleanup;
    }

    dimension_field = name_field->next;
    declarator = lm_trans_statement_c_declarator_new();
    if (declarator == 0) {
        status = 1;
        goto cleanup;
    }
    declarator->type_node = type_node;
    *declarator->type_head = *type_head;
    *declarator->name = *name_node->as->atom;
    declarator->pointer_depth = pointer_depth;
    declarator->type_is_head = type_is_head;
    *declarator->array_head = *template_frame->head;
    declarator->expression_dimensions = dimension_field;
    if (lm_trans_emit_c_declarator(file, declarator, namespace_, "repeated [] array dimension") != 0) {
        status = 1;
        goto cleanup;
    }

    initializer_field = declarator->expression_dimensions;
    if (initializer_field != 0) {
        if (lm_trans_emit_array_initializer(file, initializer_field, namespace_) != 0) {
            status = 1;
            goto cleanup;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        status = 1;
        goto cleanup;
    }
    status = lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");

cleanup:
    lm_trans_statement_c_declarator_destroy(declarator);
    lm_trans_text_ref_destroy(&type_head);
    lm_trans_text_ref_destroy(&template_type_head);
    return status;
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

    type_field = lm_trans_nth_field(frame->body, 0U);
    name_field = lm_trans_nth_field(frame->body, 1U);
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
    if (lm_trans_emit_name(file, type_node->as->atom) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, name_node->as->atom) != 0) {
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
    return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
}

static int lm_trans_emit_const_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Frame *inner;

    field = frame->body->first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: const receiver expects exactly one declaration frame\n");
        return 1;
    }

    inner = field->value->as->frame;
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
    LmTransCallLowering *call;
    int has_args;
    int status;

    call = lm_trans_statement_call_lowering_new();
    if (call == 0) {
        return 1;
    }

    if (lm_trans_lower_call(frame->head, namespace_, "callable", call) != 0) {
        lm_trans_expr_call_lowering_delete(call);
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        lm_trans_expr_call_lowering_delete(call);
        return 1;
    }
    status = 0;
    if (call->is_closure) {
        has_args = call->signature != 0 && call->signature->param_names != 0 && call->signature->param_names->count > 0U;
        if (!has_args && lm_trans_call_body_first_field(frame->body) != 0) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_name(file, call->name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, "->call(") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_name(file, call->name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, "->env") != 0) {
            status = 1;
            goto cleanup;
        }
        if (has_args) {
            if (lm_trans_put(file, ", ") != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_emit_call_args(file, frame->body, namespace_, call->signature) != 0) {
                status = 1;
                goto cleanup;
            }
        }
        if (lm_trans_put(file, ")") != 0) {
            status = 1;
            goto cleanup;
        }
        status = lm_trans_put(file, ";\n");
        goto cleanup;
    }
    if (lm_trans_emit_name(file, call->name) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, "(") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_emit_call_args(file, frame->body, namespace_, call->signature) != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_trans_put(file, ")") != 0) {
        status = 1;
        goto cleanup;
    }
    status = lm_trans_put(file, ";\n");

cleanup:
    lm_trans_expr_call_lowering_delete(call);
    return status;
}

static int lm_trans_text_contains_char(const LmP0Text *text, char ch) {
    size_t i;

    i = 0U;
    while (text != 0 && i < text->length) {
        if (text->data[i] == ch) {
            return 1;
        }
        ++i;
    }
    return 0;
}

static int lm_trans_head_looks_assignable_target(const LmP0Text *head) {
    return
        lm_trans_text_contains_char(head, '[') ||
        lm_trans_text_contains_char(head, '\\') ||
        lm_trans_c_reference_has_path_dot(head);
}

static char *lm_trans_prefixed_identifier_name_new(
    const char *prefix,
    const LmP0Text *name,
    const char *suffix
) {
    LmP0Text *payload;
    char *result;
    size_t prefix_length;
    size_t suffix_length;
    size_t length;

    if (prefix == 0 || name == 0) {
        return 0;
    }

    payload = lm_trans_statement_text_new();
    if (payload == 0) {
        return 0;
    }
    if (!lm_trans_identifier_payload(name, payload)) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }

    prefix_length = strlen(prefix);
    suffix_length = suffix != 0 ? strlen(suffix) : 0U;
    length = prefix_length + payload->length + suffix_length;
    result = (char *)lm_own_new_zero(length + 1U);
    if (result == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    memcpy(result, prefix, prefix_length);
    if (payload->length != 0U) {
        memcpy(result + prefix_length, payload->data, payload->length);
    }
    if (suffix_length != 0U) {
        memcpy(result + prefix_length + payload->length, suffix, suffix_length);
    }
    result[length] = '\0';
    lm_trans_text_ref_destroy(&payload);
    return result;
}

static const LmP0Structure *lm_trans_named_structure_body(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }
    return lm_trans_unwrap_single_anonymous_structure(frame->body);
}

static int lm_trans_frame_looks_named_structure_declaration(const LmP0Frame *frame) {
    return
        frame != 0 &&
        !lm_trans_is_c_reference_name(frame->head) &&
        !lm_trans_is_reserved_head_name(frame->head) &&
        !lm_trans_text_all_char(frame->head, '@') &&
        !lm_trans_text_is_array_receiver_head(frame->head) &&
        (frame->flags & LM_P0_FRAME_COLON) != 0U &&
        (frame->flags & LM_P0_FRAME_INLINE_BODY) == 0U;
}

static int lm_trans_named_structure_field_info(
    const LmP0Frame *field_frame,
    const LmTransNamespace *namespace_,
    const LmP0Text **out_type_head,
    const LmP0Text **out_name,
    const LmP0Field **out_initializer,
    size_t *out_pointer_depth
) {
    const LmP0Frame *current_frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Frame *inner;
    size_t pointer_depth;

    if (
        field_frame == 0 ||
        out_type_head == 0 ||
        out_name == 0 ||
        out_initializer == 0 ||
        out_pointer_depth == 0
    ) {
        return 0;
    }

    current_frame = field_frame;
    pointer_depth = 0U;

    while (lm_trans_text_all_char(current_frame->head, '@')) {
        body = lm_trans_named_structure_body(current_frame);
        if (body == 0) {
            return 0;
        }

        field = body->first_field;
        if (
            field != 0 &&
            field->next == 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_FRAME
        ) {
            inner = field->value->as->frame;
            pointer_depth += current_frame->head->length;
            current_frame = inner;
            continue;
        }

        type_field = lm_trans_nth_field(body, 0U);
        name_field = lm_trans_nth_field(body, 1U);
        if (
            type_field == 0 ||
            name_field == 0 ||
            type_field->value == 0 ||
            name_field->value == 0 ||
            type_field->value->kind != LM_P0_NODE_ATOM ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            return 0;
        }
        *out_type_head = type_field->value->as->atom;
        *out_name = name_field->value->as->atom;
        *out_initializer = name_field->next;
        *out_pointer_depth = pointer_depth + current_frame->head->length;
        return lm_trans_head_can_declare_storage(*out_type_head, namespace_);
    }

    body = lm_trans_named_structure_body(current_frame);
    if (body == 0) {
        return 0;
    }

    if (!lm_trans_head_can_declare_storage(current_frame->head, namespace_)) {
        return 0;
    }

    name_field = lm_trans_nth_field(body, 0U);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }
    *out_type_head = current_frame->head;
    *out_name = name_field->value->as->atom;
    *out_initializer = name_field->next;
    *out_pointer_depth = pointer_depth;
    return lm_trans_atom_can_be_new_binding_name(*out_name);
}

static int lm_trans_frame_looks_named_structure_layout_declaration(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;
    int saw_field;

    if (!lm_trans_frame_looks_named_structure_declaration(frame)) {
        return 0;
    }

    body = lm_trans_named_structure_body(frame);
    field = body != 0 ? body->first_field : 0;
    saw_field = 0;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (field->value->kind != LM_P0_NODE_FRAME) {
                return 0;
            }
            if (
                !lm_trans_named_structure_field_info(
                    field->value->as->frame,
                    namespace_,
                    &type_head,
                    &name,
                    &initializer,
                    &pointer_depth
                )
            ) {
                return 0;
            }
            saw_field = 1;
        }
        field = field->next;
    }
    return saw_field;
}

static char *lm_trans_named_structure_relation_table_new(
    const LmP0Text *owner,
    const char *suffix
) {
    char *owner_name;
    char *table_name;
    size_t owner_length;
    size_t suffix_length;

    owner_name = lm_trans_prefixed_identifier_name_new("", owner, "");
    if (owner_name == 0 || suffix == 0) {
        lm_own_delete(owner_name, 0);
        return 0;
    }

    owner_length = strlen(owner_name);
    suffix_length = strlen(suffix);
    table_name = (char *)lm_own_new_zero(owner_length + suffix_length + 1U);
    if (table_name == 0) {
        lm_own_delete(owner_name, 0);
        return 0;
    }
    memcpy(table_name, owner_name, owner_length);
    memcpy(table_name + owner_length, suffix, suffix_length + 1U);
    lm_own_delete(owner_name, 0);
    return table_name;
}

static int lm_trans_named_structure_push_relation(
    const LmP0Text *owner,
    const char *suffix,
    const LmP0Text *key,
    const LmP0Text *payload
) {
    char *table_name;
    LmP0Text *table_atom;
    int status;

    table_name = lm_trans_named_structure_relation_table_new(owner, suffix);
    if (table_name == 0) {
        return 1;
    }
    table_atom = lm_trans_text_from_cstr(table_name);
    if (table_atom == 0) {
        lm_own_delete(table_name, 0);
        return 1;
    }
    status = lm_trans_registry_push_row_atoms(table_atom, key, payload) != 0;
    lm_trans_text_ref_destroy(&table_atom);
    lm_own_delete(table_name, 0);
    return status;
}

static int lm_trans_declare_named_structure_owner(
    LmTransNamespace *namespace_,
    const LmP0Text *owner
) {
    LmP0Text *storage_name;
    LmP0Text *layout_backend_table;
    LmP0Text *backend_payload;
    char *storage_name_text;
    int status;

    if (namespace_ == 0 || owner == 0) {
        return 1;
    }

    storage_name_text = lm_trans_prefixed_identifier_name_new("lm_", owner, "");
    storage_name = storage_name_text != 0 ? lm_trans_text_from_cstr(storage_name_text) : 0;
    if (storage_name_text == 0 || storage_name == 0) {
        lm_own_delete(storage_name_text, 0);
        lm_trans_text_ref_destroy(&storage_name);
        return 1;
    }
    layout_backend_table = lm_trans_text_from_cstr("layout.backend");
    backend_payload = lm_trans_text_from_cstr("c.struct");
    if (layout_backend_table == 0 || backend_payload == 0) {
        lm_trans_text_ref_destroy(&layout_backend_table);
        lm_trans_text_ref_destroy(&backend_payload);
        lm_trans_text_ref_destroy(&storage_name);
        lm_own_delete(storage_name_text, 0);
        return 1;
    }

    status =
        lm_trans_registry_note_class_present(owner) != 0 ||
        lm_trans_registry_note_class_kind(owner, "layout") != 0 ||
        lm_trans_registry_push_row_atoms(layout_backend_table, owner, backend_payload) != 0 ||
        lm_trans_namespace_declare_c_name(namespace_, owner, "variable", storage_name) != 0;

    lm_trans_text_ref_destroy(&layout_backend_table);
    lm_trans_text_ref_destroy(&backend_payload);
    lm_trans_text_ref_destroy(&storage_name);
    lm_own_delete(storage_name_text, 0);
    return status != 0;
}

static int lm_trans_named_structure_push_field_relations(
    const LmP0Text *owner,
    const LmP0Text *name,
    const LmP0Text *type_head,
    size_t index,
    size_t pointer_depth
) {
    LmP0Text *index_payload;
    LmP0Text *depth_payload;
    char index_buffer[32];
    char depth_buffer[32];
    int status;

    if (owner == 0 || name == 0 || type_head == 0) {
        return 1;
    }

    snprintf(index_buffer, sizeof(index_buffer), "%lu", (unsigned long)index);
    snprintf(depth_buffer, sizeof(depth_buffer), "%lu", (unsigned long)pointer_depth);
    index_payload = lm_trans_text_from_cstr(index_buffer);
    depth_payload = lm_trans_text_from_cstr(depth_buffer);
    if (index_payload == 0 || depth_payload == 0) {
        lm_trans_text_ref_destroy(&index_payload);
        lm_trans_text_ref_destroy(&depth_payload);
        return 1;
    }

    status =
        lm_trans_named_structure_push_relation(owner, ".field.class", name, type_head) != 0 ||
        lm_trans_named_structure_push_relation(owner, ".field.index", name, index_payload) != 0 ||
        lm_trans_named_structure_push_relation(owner, ".field.address-depth", name, depth_payload) != 0;

    lm_trans_text_ref_destroy(&index_payload);
    lm_trans_text_ref_destroy(&depth_payload);
    return status != 0;
}

static int lm_trans_declare_named_structure_layout(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;
    size_t index;

    if (
        namespace_ == 0 ||
        frame == 0 ||
        !lm_trans_frame_looks_named_structure_layout_declaration(frame, namespace_)
    ) {
        return 1;
    }

    if (lm_trans_declare_named_structure_owner(namespace_, frame->head) != 0) {
        return 1;
    }

    body = lm_trans_named_structure_body(frame);
    field = body != 0 ? body->first_field : 0;
    index = 0U;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                field->value->kind != LM_P0_NODE_FRAME ||
                !lm_trans_named_structure_field_info(
                    field->value->as->frame,
                    namespace_,
                    &type_head,
                    &name,
                    &initializer,
                    &pointer_depth
                )
            ) {
                return 1;
            }
            (void)initializer;
            if (
                lm_trans_named_structure_push_field_relations(
                    frame->head,
                    name,
                    type_head,
                    index,
                    pointer_depth
                ) != 0
            ) {
                return 1;
            }
            ++index;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_named_structure_field_declaration(
    FILE *file,
    const LmP0Frame *field_frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;
    size_t i;

    if (
        !lm_trans_named_structure_field_info(
            field_frame,
            namespace_,
            &type_head,
            &name,
            &initializer,
            &pointer_depth
        )
    ) {
        return 1;
    }
    (void)initializer;

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_type_head_only(file, type_head) != 0) {
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
    if (lm_trans_emit_identifier(file, name) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_named_structure_typedef(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    const LmP0Structure *body;
    const LmP0Field *field;

    if (
        file == 0 ||
        frame == 0 ||
        !lm_trans_frame_looks_named_structure_layout_declaration(frame, namespace_)
    ) {
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " {\n") != 0) {
        return 1;
    }

    body = lm_trans_named_structure_body(frame);
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                field->value->kind != LM_P0_NODE_FRAME ||
                lm_trans_emit_named_structure_field_declaration(
                    file,
                    field->value->as->frame,
                    indent + 1U,
                    namespace_
                ) != 0
            ) {
                return 1;
            }
        }
        field = field->next;
    }

    if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "} ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_named_structure_body_initializer(
    FILE *file,
    const LmP0Structure *body,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_named_structure_initializer(
    FILE *file,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    return lm_trans_emit_named_structure_body_initializer(
        file,
        lm_trans_named_structure_body(frame),
        namespace_
    );
}

static int lm_trans_emit_named_structure_body_initializer(
    FILE *file,
    const LmP0Structure *body,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;
    int wrote;

    if (lm_trans_put(file, " = {") != 0) {
        return 1;
    }

    field = body != 0 ? body->first_field : 0;
    wrote = 0;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                field->value->kind != LM_P0_NODE_FRAME ||
                !lm_trans_named_structure_field_info(
                    field->value->as->frame,
                    namespace_,
                    &type_head,
                    &name,
                    &initializer,
                    &pointer_depth
                )
            ) {
                return 1;
            }
            (void)type_head;
            (void)pointer_depth;
            if (initializer != 0) {
                if (wrote && lm_trans_put(file, ", ") != 0) {
                    return 1;
                }
                if (lm_trans_put(file, ".") != 0 || lm_trans_emit_identifier(file, name) != 0) {
                    return 1;
                }
                if (lm_trans_put(file, " = ") != 0) {
                    return 1;
                }
                if (lm_trans_emit_expr_fields(file, initializer, namespace_) != 0) {
                    return 1;
                }
                wrote = 1;
            }
        }
        field = field->next;
    }

    if (!wrote && lm_trans_put(file, "0") != 0) {
        return 1;
    }
    return lm_trans_put(file, "}");
}

static int lm_trans_emit_named_structure_storage(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    char *storage_name;
    char *pointer_name;

    if (
        file == 0 ||
        namespace_ == 0 ||
        frame == 0 ||
        !lm_trans_frame_looks_named_structure_layout_declaration(frame, namespace_)
    ) {
        return 1;
    }

    storage_name = lm_trans_prefixed_identifier_name_new("lm_", frame->head, "_storage");
    pointer_name = lm_trans_prefixed_identifier_name_new("lm_", frame->head, "");
    if (storage_name == 0 || pointer_name == 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0 || lm_trans_put(file, " ") != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_put(file, storage_name) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_emit_named_structure_initializer(file, frame, namespace_) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_put(file, ";\n") != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_emit_identifier(file, frame->head) != 0 || lm_trans_put(file, " *") != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_put(file, pointer_name) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    if (lm_trans_put(file, " = &") != 0 || lm_trans_put(file, storage_name) != 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }
    lm_own_delete(storage_name, 0);
    lm_own_delete(pointer_name, 0);
    return lm_trans_put(file, ";\n");
}

static int lm_trans_l2_c_identifier_char(char ch) {
    return
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') ||
        (ch >= '0' && ch <= '9') ||
        ch == '_';
}

static char *lm_trans_l2_c_identifier_new(
    const char *prefix,
    const LmP0Text *text,
    const char *suffix
) {
    LmP0Text *payload;
    char *result;
    size_t prefix_length;
    size_t suffix_length;
    size_t payload_length;
    size_t length;
    size_t i;
    size_t out;
    char ch;

    if (prefix == 0 || text == 0) {
        return 0;
    }

    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 0;
    }
    if (!lm_trans_registry_literal_value(text, payload)) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }

    prefix_length = strlen(prefix);
    suffix_length = suffix != 0 ? strlen(suffix) : 0U;
    payload_length = payload->length != 0U ? payload->length : 1U;
    length = prefix_length + payload_length + suffix_length;
    result = (char *)lm_own_new_zero(length + 1U);
    if (result == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }

    memcpy(result, prefix, prefix_length);
    out = prefix_length;
    if (payload->length == 0U) {
        result[out++] = 'x';
    } else {
        for (i = 0U; i < payload->length; ++i) {
            ch = payload->data[i];
            result[out++] = lm_trans_l2_c_identifier_char(ch) ? ch : '_';
        }
    }
    if (suffix_length != 0U) {
        memcpy(result + out, suffix, suffix_length);
        out += suffix_length;
    }
    result[out] = '\0';
    lm_trans_text_ref_destroy(&payload);
    return result;
}

static int lm_trans_emit_c_string_literal_text(FILE *file, const LmP0Text *text) {
    size_t i;
    unsigned char ch;

    if (file == 0 || text == 0) {
        return 1;
    }
    if (lm_trans_put(file, "\"") != 0) {
        return 1;
    }
    for (i = 0U; i < text->length; ++i) {
        ch = (unsigned char)text->data[i];
        if (ch == '\\' || ch == '"') {
            if (fputc('\\', file) == EOF || fputc((int)ch, file) == EOF) {
                return 1;
            }
        } else if (ch == '\n') {
            if (lm_trans_put(file, "\\n") != 0) {
                return 1;
            }
        } else if (ch == '\r') {
            if (lm_trans_put(file, "\\r") != 0) {
                return 1;
            }
        } else if (ch == '\t') {
            if (lm_trans_put(file, "\\t") != 0) {
                return 1;
            }
        } else if (ch < 32U || ch > 126U) {
            if (fprintf(file, "\\%03o", (unsigned)ch) < 0) {
                return 1;
            }
        } else if (fputc((int)ch, file) == EOF) {
            return 1;
        }
    }
    return lm_trans_put(file, "\"");
}

static int lm_trans_emit_c_string_literal_atom(FILE *file, const LmP0Text *atom) {
    LmP0Text *payload;
    int status;

    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 1;
    }
    if (!lm_trans_registry_literal_value(atom, payload)) {
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }
    status = lm_trans_emit_c_string_literal_text(file, payload);
    lm_trans_text_ref_destroy(&payload);
    return status;
}

static const LmP0Field *lm_trans_l2_structure_first_present_field(const LmP0Structure *structure) {
    const LmP0Field *field;

    field = structure != 0 ? structure->first_field : 0;
    while (field != 0 && lm_trans_node_is_ignored(field->value)) {
        field = field->next;
    }
    return field;
}

static const LmP0Field *lm_trans_l2_structure_next_present_field(const LmP0Field *field) {
    field = field != 0 ? field->next : 0;
    while (field != 0 && lm_trans_node_is_ignored(field->value)) {
        field = field->next;
    }
    return field;
}

static int lm_trans_l2_structure_has_present_fields(const LmP0Structure *structure) {
    return lm_trans_l2_structure_first_present_field(structure) != 0;
}

static LmP0Node *lm_trans_l2_frame_wrapper_node_new(const LmP0Frame *frame) {
    LmP0Node *node;

    if (frame == 0) {
        return 0;
    }
    node = (LmP0Node *)lm_own_new_zero(sizeof(*node));
    if (node == 0) {
        return 0;
    }
    node->as = (LmP0NodeAs *)lm_own_new_zero(sizeof(*node->as));
    if (node->as == 0) {
        lm_own_delete(node, 0);
        return 0;
    }
    node->kind = LM_P0_NODE_FRAME;
    node->as->frame = (LmP0Frame *)frame;
    return node;
}

static void lm_trans_l2_frame_wrapper_node_delete(LmP0Node **node) {
    if (node == 0 || *node == 0) {
        return;
    }
    lm_own_delete((*node)->as, 0);
    lm_own_delete(*node, 0);
    *node = 0;
}

static char *lm_trans_l2_structure_member_name_new(const LmP0Node *node, size_t index) {
    const LmP0Text *base;
    LmP0Text *generated_base;
    char suffix[32];
    char *result;

    generated_base = 0;
    if (node != 0 && node->kind == LM_P0_NODE_FRAME) {
        base = node->as->frame->head;
    } else if (node != 0 && node->kind == LM_P0_NODE_STRUCTURE) {
        generated_base = lm_trans_text_ref_new_cstr("structure");
        if (generated_base == 0) {
            return 0;
        }
        base = generated_base;
    } else {
        generated_base = lm_trans_text_ref_new_cstr("atom");
        if (generated_base == 0) {
            return 0;
        }
        base = generated_base;
    }

    snprintf(suffix, sizeof(suffix), "_%lu", (unsigned long)index);
    result = lm_trans_l2_c_identifier_new("field_", base, suffix);
    lm_trans_text_ref_destroy(&generated_base);
    return result;
}

static char *lm_trans_l2_object_suffix_name_new(
    const char *object_name,
    const char *suffix
) {
    size_t object_length;
    size_t suffix_length;
    char *name;

    if (object_name == 0 || suffix == 0) {
        return 0;
    }

    object_length = strlen(object_name);
    suffix_length = strlen(suffix);
    name = (char *)lm_own_new_zero(object_length + suffix_length + 1U);
    if (name == 0) {
        return 0;
    }
    memcpy(name, object_name, object_length);
    memcpy(name + object_length, suffix, suffix_length + 1U);
    return name;
}

static char *lm_trans_l2_child_object_name_new(
    const char *object_name,
    const char *member_name
) {
    size_t object_length;
    size_t member_length;
    char *name;

    if (object_name == 0 || member_name == 0) {
        return 0;
    }

    object_length = strlen(object_name);
    member_length = strlen(member_name);
    name = (char *)lm_own_new_zero(object_length + member_length + 2U);
    if (name == 0) {
        return 0;
    }
    memcpy(name, object_name, object_length);
    name[object_length] = '_';
    memcpy(name + object_length + 1U, member_name, member_length + 1U);
    return name;
}

static char *lm_trans_l2_object_type_name_new(const char *object_name) {
    return lm_trans_l2_object_suffix_name_new(object_name, "_type");
}

static char *lm_trans_l2_object_storage_name_new(const char *object_name) {
    return lm_trans_l2_object_suffix_name_new(object_name, "_storage");
}

static int lm_trans_emit_l2_structure_node_definition(
    FILE *file,
    const LmP0Node *node,
    const char *object_name,
    unsigned indent
);

static int lm_trans_emit_l2_structure_node_initializer(
    FILE *file,
    const LmP0Node *node,
    const char *object_name
);

static int lm_trans_emit_l2_structure_fields_children(
    FILE *file,
    const LmP0Structure *structure,
    const char *object_name,
    unsigned indent
) {
    const LmP0Field *field;
    char *member_name;
    char *child_object_name;
    size_t index;

    field = lm_trans_l2_structure_first_present_field(structure);
    index = 0U;
    while (field != 0) {
        if (
            field->value != 0 &&
            (
                field->value->kind == LM_P0_NODE_STRUCTURE ||
                field->value->kind == LM_P0_NODE_FRAME
            )
        ) {
            member_name = lm_trans_l2_structure_member_name_new(field->value, index);
            if (member_name == 0) {
                return 1;
            }
            child_object_name = lm_trans_l2_child_object_name_new(object_name, member_name);
            if (child_object_name == 0) {
                lm_own_delete(member_name, 0);
                return 1;
            }
            if (
                lm_trans_emit_l2_structure_node_definition(
                    file,
                    field->value,
                    child_object_name,
                    indent
                ) != 0
            ) {
                lm_own_delete(child_object_name, 0);
                lm_own_delete(member_name, 0);
                return 1;
            }
            lm_own_delete(child_object_name, 0);
            lm_own_delete(member_name, 0);
        }
        ++index;
        field = lm_trans_l2_structure_next_present_field(field);
    }
    return 0;
}

static int lm_trans_emit_l2_structure_node_type(
    FILE *file,
    const LmP0Node *node,
    const char *object_name,
    const char *member_name,
    unsigned indent
) {
    char *child_object_name;
    char *child_type_name;
    int status;

    if (file == 0 || node == 0 || object_name == 0 || member_name == 0) {
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "const char *") != 0 ||
            lm_trans_put(file, member_name) != 0 ||
            lm_trans_put(file, ";\n") != 0;
    }

    if (node->kind == LM_P0_NODE_STRUCTURE || node->kind == LM_P0_NODE_FRAME) {
        child_object_name = lm_trans_l2_child_object_name_new(object_name, member_name);
        if (child_object_name == 0) {
            return 1;
        }
        child_type_name = lm_trans_l2_object_type_name_new(child_object_name);
        if (child_type_name == 0) {
            lm_own_delete(child_object_name, 0);
            return 1;
        }
        status =
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "const ") != 0 ||
            lm_trans_put(file, child_type_name) != 0 ||
            lm_trans_put(file, " *") != 0 ||
            lm_trans_put(file, member_name) != 0 ||
            lm_trans_put(file, ";\n") != 0;
        lm_own_delete(child_type_name, 0);
        lm_own_delete(child_object_name, 0);
        return status;
    }

    return 1;
}

static int lm_trans_emit_l2_structure_fields_type(
    FILE *file,
    const LmP0Structure *structure,
    const char *object_name,
    unsigned indent
) {
    const LmP0Field *field;
    char *member_name;
    size_t index;

    field = lm_trans_l2_structure_first_present_field(structure);
    if (field == 0) {
        return
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "const char *empty;\n") != 0;
    }

    index = 0U;
    while (field != 0) {
        member_name = lm_trans_l2_structure_member_name_new(field->value, index);
        if (member_name == 0) {
            return 1;
        }
        if (
            lm_trans_emit_l2_structure_node_type(
                file,
                field->value,
                object_name,
                member_name,
                indent
            ) != 0
        ) {
            lm_own_delete(member_name, 0);
            return 1;
        }
        lm_own_delete(member_name, 0);
        ++index;
        field = lm_trans_l2_structure_next_present_field(field);
    }
    return 0;
}

static int lm_trans_emit_l2_structure_field_initializer(
    FILE *file,
    const LmP0Node *node,
    const char *object_name,
    const char *member_name
) {
    char *child_object_name;
    char *child_storage_name;
    int status;

    if (file == 0 || node == 0 || object_name == 0 || member_name == 0) {
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_c_string_literal_atom(file, node->as->atom);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE || node->kind == LM_P0_NODE_FRAME) {
        child_object_name = lm_trans_l2_child_object_name_new(object_name, member_name);
        if (child_object_name == 0) {
            return 1;
        }
        child_storage_name = lm_trans_l2_object_storage_name_new(child_object_name);
        if (child_storage_name == 0) {
            lm_own_delete(child_object_name, 0);
            return 1;
        }
        status =
            lm_trans_put(file, "&") != 0 ||
            lm_trans_put(file, child_storage_name) != 0;
        lm_own_delete(child_storage_name, 0);
        lm_own_delete(child_object_name, 0);
        return status;
    }

    return 1;
}

static int lm_trans_emit_l2_structure_fields_initializer(
    FILE *file,
    const LmP0Structure *structure,
    const char *object_name
) {
    const LmP0Field *field;
    char *member_name;
    size_t index;
    int wrote;

    field = lm_trans_l2_structure_first_present_field(structure);
    if (field == 0) {
        return lm_trans_put(file, "0");
    }

    wrote = 0;
    index = 0U;
    while (field != 0) {
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        member_name = lm_trans_l2_structure_member_name_new(field->value, index);
        if (member_name == 0) {
            return 1;
        }
        if (
            lm_trans_emit_l2_structure_field_initializer(
                file,
                field->value,
                object_name,
                member_name
            ) != 0
        ) {
            lm_own_delete(member_name, 0);
            return 1;
        }
        lm_own_delete(member_name, 0);
        wrote = 1;
        ++index;
        field = lm_trans_l2_structure_next_present_field(field);
    }
    return 0;
}

static int lm_trans_emit_l2_structure_node_initializer(
    FILE *file,
    const LmP0Node *node,
    const char *object_name
) {
    if (file == 0 || node == 0 || object_name == 0) {
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_c_string_literal_atom(file, node->as->atom);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return
            lm_trans_put(file, "{") != 0 ||
            lm_trans_emit_l2_structure_fields_initializer(file, node->as->structure, object_name) != 0 ||
            lm_trans_put(file, "}") != 0;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if (
            lm_trans_put(file, "{") != 0 ||
            lm_trans_emit_c_string_literal_atom(file, node->as->frame->head) != 0
        ) {
            return 1;
        }
        if (
            lm_trans_l2_structure_has_present_fields(node->as->frame->body) &&
            (
                lm_trans_put(file, ", ") != 0 ||
                lm_trans_emit_l2_structure_fields_initializer(file, node->as->frame->body, object_name) != 0
            )
        ) {
            return 1;
        }
        return lm_trans_put(file, "}") != 0;
    }

    return 1;
}

static int lm_trans_emit_l2_structure_node_definition(
    FILE *file,
    const LmP0Node *node,
    const char *object_name,
    unsigned indent
) {
    char *type_name;
    char *storage_name;

    if (file == 0 || node == 0 || object_name == 0) {
        return 1;
    }
    if (node->kind != LM_P0_NODE_STRUCTURE && node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    if (
        node->kind == LM_P0_NODE_FRAME &&
        lm_trans_emit_l2_structure_fields_children(file, node->as->frame->body, object_name, indent) != 0
    ) {
        return 1;
    }
    if (
        node->kind == LM_P0_NODE_STRUCTURE &&
        lm_trans_emit_l2_structure_fields_children(file, node->as->structure, object_name, indent) != 0
    ) {
        return 1;
    }

    type_name = lm_trans_l2_object_type_name_new(object_name);
    storage_name = lm_trans_l2_object_storage_name_new(object_name);
    if (type_name == 0 || storage_name == 0) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        return 1;
    }

    if (
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "typedef struct ") != 0 ||
        lm_trans_put(file, type_name) != 0 ||
        lm_trans_put(file, " {\n") != 0
    ) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        return 1;
    }
    if (
        node->kind == LM_P0_NODE_FRAME &&
        (
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_put(file, "const char *head;\n") != 0 ||
            lm_trans_emit_l2_structure_fields_type(file, node->as->frame->body, object_name, indent + 1U) != 0
        )
    ) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        return 1;
    }
    if (
        node->kind == LM_P0_NODE_STRUCTURE &&
        lm_trans_emit_l2_structure_fields_type(file, node->as->structure, object_name, indent + 1U) != 0
    ) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        return 1;
    }
    if (
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "} ") != 0 ||
        lm_trans_put(file, type_name) != 0 ||
        lm_trans_put(file, ";\n") != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "static const ") != 0 ||
        lm_trans_put(file, type_name) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_put(file, storage_name) != 0 ||
        lm_trans_put(file, " = ") != 0 ||
        lm_trans_emit_l2_structure_node_initializer(file, node, object_name) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        return 1;
    }

    lm_own_delete(type_name, 0);
    lm_own_delete(storage_name, 0);
    return 0;
}

static const LmP0Text *lm_trans_l2_table_name_atom(const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Frame *child_frame;
    const LmP0Field *name_field;

    field = frame != 0 ? lm_trans_l2_structure_first_present_field(frame->body) : 0;
    while (field != 0) {
        if (field->value != 0 && field->value->kind == LM_P0_NODE_FRAME) {
            child_frame = field->value->as->frame;
            if (lm_trans_text_equals(child_frame->head, "name")) {
                name_field = lm_trans_l2_structure_first_present_field(child_frame->body);
                if (name_field != 0 && name_field->value != 0 && name_field->value->kind == LM_P0_NODE_ATOM) {
                    return name_field->value->as->atom;
                }
            }
        }
        field = lm_trans_l2_structure_next_present_field(field);
    }
    return frame != 0 ? frame->head : 0;
}

static const LmP0Text *lm_trans_l2_join_target_atom(const LmP0Frame *frame) {
    const LmP0Field *field;

    field = frame != 0 ? lm_trans_l2_structure_first_present_field(frame->body) : 0;
    if (field != 0) {
        field = lm_trans_l2_structure_next_present_field(field);
    }
    if (field != 0 && field->value != 0 && field->value->kind == LM_P0_NODE_ATOM) {
        return field->value->as->atom;
    }
    return frame != 0 ? frame->head : 0;
}

static int lm_trans_emit_l2_structure_frame_storage(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const char *prefix,
    const LmP0Text *name_hint
) {
    char id_suffix[32];
    char *object_name;
    char *type_name;
    char *storage_name;
    LmP0Node *node;

    if (file == 0 || frame == 0 || prefix == 0 || name_hint == 0) {
        return 1;
    }

    snprintf(
        id_suffix,
        sizeof(id_suffix),
        "_%lu",
        (unsigned long)++lm_trans_next_l2_structure_object_id
    );
    object_name = lm_trans_l2_c_identifier_new(prefix, name_hint, id_suffix);
    if (object_name == 0) {
        return 1;
    }
    type_name = lm_trans_l2_object_type_name_new(object_name);
    storage_name = lm_trans_l2_object_storage_name_new(object_name);
    if (type_name == 0 || storage_name == 0) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        lm_own_delete(object_name, 0);
        return 1;
    }

    node = lm_trans_l2_frame_wrapper_node_new(frame);
    if (node == 0) {
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        lm_own_delete(object_name, 0);
        return 1;
    }

    if (
        lm_trans_emit_l2_structure_node_definition(file, node, object_name, indent) != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "static const ") != 0 ||
        lm_trans_put(file, type_name) != 0 ||
        lm_trans_put(file, " *const ") != 0 ||
        lm_trans_put(file, object_name) != 0 ||
        lm_trans_put(file, " = &") != 0 ||
        lm_trans_put(file, storage_name) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        lm_trans_l2_frame_wrapper_node_delete(&node);
        lm_own_delete(type_name, 0);
        lm_own_delete(storage_name, 0);
        lm_own_delete(object_name, 0);
        return 1;
    }

    if (indent == 0U) {
        if (
            lm_trans_put(file, "enum { ") != 0 ||
            lm_trans_put(file, object_name) != 0 ||
            lm_trans_put(file, "_anchor = sizeof(") != 0 ||
            lm_trans_put(file, object_name) != 0 ||
            lm_trans_put(file, ") };\n") != 0
        ) {
            lm_trans_l2_frame_wrapper_node_delete(&node);
            lm_own_delete(type_name, 0);
            lm_own_delete(storage_name, 0);
            lm_own_delete(object_name, 0);
            return 1;
        }
    } else {
        if (
            lm_trans_emit_indent(file, indent) != 0 ||
            lm_trans_put(file, "(void)&") != 0 ||
            lm_trans_put(file, object_name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            lm_trans_l2_frame_wrapper_node_delete(&node);
            lm_own_delete(type_name, 0);
            lm_own_delete(storage_name, 0);
            lm_own_delete(object_name, 0);
            return 1;
        }
    }

    lm_trans_l2_frame_wrapper_node_delete(&node);
    lm_own_delete(type_name, 0);
    lm_own_delete(storage_name, 0);
    lm_own_delete(object_name, 0);
    return 0;
}

static int lm_trans_emit_l2_table_structure(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent
) {
    return lm_trans_emit_l2_structure_frame_storage(
        file,
        frame,
        indent,
        "lm_l2_table_",
        lm_trans_l2_table_name_atom(frame)
    );
}

static int lm_trans_emit_l2_join_structure(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent
) {
    return lm_trans_emit_l2_structure_frame_storage(
        file,
        frame,
        indent,
        "lm_l2_join_",
        lm_trans_l2_join_target_atom(frame)
    );
}

static int lm_trans_layout_backend_is_supported(const char *backend);
static LmTransLayoutField **lm_trans_layout_fields_new(size_t capacity);
static void lm_trans_layout_fields_delete(LmTransLayoutField **fields, size_t capacity);
static int lm_trans_collect_layout_fields(
    const char *layout_name,
    const LmTransNamespace *namespace_,
    LmTransLayoutField **fields,
    size_t capacity,
    size_t *out_count
);
static int lm_trans_sort_layout_fields(LmTransLayoutField **fields, size_t field_count);
static int lm_trans_emit_layout_fields(
    FILE *file,
    const char *layout_name,
    const LmTransNamespace *namespace_,
    unsigned indent
);
static int lm_trans_node_result_type(
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    LmTransL4CallableType *out
);

static const LmP0Structure *lm_trans_merge_source_list(const LmP0Frame *frame) {
    const LmP0Field *field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "merge")) {
        return 0;
    }
    field = lm_trans_nth_field(frame->body, 0U);
    if (
        field == 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }
    return field->value->as->structure;
}

static const LmP0Text *lm_trans_merge_target_name(const LmP0Frame *frame) {
    const LmP0Field *field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "merge")) {
        return 0;
    }
    field = lm_trans_nth_field(frame->body, 1U);
    if (
        field == 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }
    return field->value->as->atom;
}

static const LmP0Structure *lm_trans_merge_local_body(const LmP0Frame *frame) {
    const LmP0Field *field;

    if (frame == 0) {
        return 0;
    }
    field = lm_trans_nth_field(frame->body, 2U);
    if (field == 0) {
        return 0;
    }
    if (
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        return 0;
    }
    return field->value->as->structure;
}

static int lm_trans_frame_looks_merge_named_structure_declaration(
    const LmP0Frame *frame
) {
    const LmP0Structure *sources;
    const LmP0Text *target;

    sources = lm_trans_merge_source_list(frame);
    target = lm_trans_merge_target_name(frame);
    return
        sources != 0 &&
        target != 0 &&
        sources->first_field != 0 &&
        lm_trans_atom_can_be_new_binding_name(target);
}

static int lm_trans_merge_push_size_relation(
    const LmP0Text *owner,
    const char *suffix,
    const LmP0Text *name,
    size_t value
) {
    LmP0Text *payload;
    char buffer[32];
    int status;

    snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)value);
    payload = lm_trans_text_from_cstr(buffer);
    if (payload == 0) {
        return 1;
    }
    status = lm_trans_named_structure_push_relation(owner, suffix, name, payload);
    lm_trans_text_ref_destroy(&payload);
    return status;
}

static int lm_trans_merge_push_layout_field_relations(
    const LmP0Text *owner,
    const LmTransLayoutField *field,
    size_t index
) {
    LmP0Text *name;
    LmP0Text *payload;
    int status;

    if (owner == 0 || field == 0 || field->name == 0) {
        return 1;
    }

    name = lm_trans_text_from_cstr(field->name);
    payload = lm_trans_text_from_cstr(
        field->is_union ? field->union_layout_name : field->class_name
    );
    if (name == 0 || payload == 0) {
        lm_trans_text_ref_destroy(&name);
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }

    status =
        lm_trans_named_structure_push_relation(
            owner,
            field->is_union ? ".field.union" : ".field.class",
            name,
            payload
        ) != 0 ||
        lm_trans_merge_push_size_relation(owner, ".field.index", name, index) != 0 ||
        lm_trans_merge_push_size_relation(owner, ".field.address-depth", name, field->address_depth) != 0 ||
        (field->is_const && lm_trans_merge_push_size_relation(owner, ".field.const", name, 1U) != 0) ||
        (field->has_array_count && lm_trans_merge_push_size_relation(owner, ".field.array-count", name, field->array_count) != 0);

    lm_trans_text_ref_destroy(&name);
    lm_trans_text_ref_destroy(&payload);
    return status != 0;
}

static int lm_trans_merge_source_layout_name_from_atom_new(
    const LmP0Text *atom,
    const LmTransNamespace *namespace_,
    char **out_name
) {
    LmP0Text *payload;
    const char *backend;

    if (out_name != 0) {
        *out_name = 0;
    }
    if (atom == 0 || out_name == 0) {
        return 0;
    }

    payload = lm_trans_statement_text_new();
    if (payload == 0) {
        return -1;
    }
    if (!lm_trans_identifier_payload(atom, payload)) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }

    backend = lm_trans_namespace_registry_lookup(namespace_, payload, "layout.backend");
    if (backend == 0 || !lm_trans_layout_backend_is_supported(backend)) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }

    *out_name = lm_trans_text_copy_cstr(payload);
    lm_trans_text_ref_destroy(&payload);
    return *out_name != 0 ? 1 : -1;
}

static int lm_trans_merge_source_layout_name_from_call_new(
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    char **out_name
) {
    LmTransL4CallableType *type;
    const char *backend;
    int status;

    if (out_name != 0) {
        *out_name = 0;
    }
    if (node == 0 || out_name == 0) {
        return 0;
    }

    type = lm_trans_expr_callable_type_new();
    if (type == 0) {
        return -1;
    }
    status = lm_trans_node_result_type(node, namespace_, type);
    if (status <= 0) {
        lm_trans_expr_callable_type_delete(type);
        return status;
    }
    if (type->address_depth != 0U || type->is_const) {
        lm_trans_expr_callable_type_delete(type);
        return 0;
    }
    backend = lm_trans_namespace_registry_lookup(namespace_, type->class_name, "layout.backend");
    if (backend == 0 || !lm_trans_layout_backend_is_supported(backend)) {
        lm_trans_expr_callable_type_delete(type);
        return 0;
    }
    *out_name = lm_trans_text_copy_cstr(type->class_name);
    lm_trans_expr_callable_type_delete(type);
    return *out_name != 0 ? 1 : -1;
}

static int lm_trans_merge_push_source_layout_fields(
    const LmP0Text *owner,
    const char *layout_name,
    const LmTransNamespace *namespace_,
    size_t *index
) {
    LmTransLayoutField **fields;
    size_t field_capacity;
    size_t field_count;
    size_t i;

    if (owner == 0 || layout_name == 0 || index == 0) {
        return 1;
    }

    field_capacity = 256U;
    fields = lm_trans_layout_fields_new(field_capacity);
    if (fields == 0) {
        return 1;
    }
    if (
        lm_trans_collect_layout_fields(
            layout_name,
            namespace_,
            fields,
            field_capacity,
            &field_count
        ) != 0 ||
        lm_trans_sort_layout_fields(fields, field_count) != 0
    ) {
        lm_trans_layout_fields_delete(fields, field_capacity);
        return 1;
    }

    for (i = 0U; i < field_count; ++i) {
        if (lm_trans_merge_push_layout_field_relations(owner, fields[i], *index) != 0) {
            lm_trans_layout_fields_delete(fields, field_capacity);
            return 1;
        }
        ++*index;
    }

    lm_trans_layout_fields_delete(fields, field_capacity);
    return 0;
}

static int lm_trans_merge_push_inline_source_fields(
    const LmP0Text *owner,
    const LmP0Frame *source,
    const LmTransNamespace *namespace_,
    size_t *index
) {
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;

    if (
        owner == 0 ||
        source == 0 ||
        index == 0 ||
        !lm_trans_frame_looks_named_structure_layout_declaration(source, namespace_)
    ) {
        return 1;
    }

    body = lm_trans_named_structure_body(source);
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                field->value->kind != LM_P0_NODE_FRAME ||
                !lm_trans_named_structure_field_info(
                    field->value->as->frame,
                    namespace_,
                    &type_head,
                    &name,
                    &initializer,
                    &pointer_depth
                )
            ) {
                return 1;
            }
            (void)initializer;
            if (
                lm_trans_named_structure_push_field_relations(
                    owner,
                    name,
                    type_head,
                    *index,
                    pointer_depth
                ) != 0
            ) {
                return 1;
            }
            ++*index;
        }
        field = field->next;
    }
    return 0;
}

static int lm_trans_merge_push_source_node_fields(
    const LmP0Text *owner,
    const LmP0Node *node,
    const LmTransNamespace *namespace_,
    size_t *index
) {
    char *layout_name;
    int status;

    if (owner == 0 || node == 0 || index == 0) {
        return 1;
    }

    layout_name = 0;
    if (node->kind == LM_P0_NODE_ATOM) {
        status = lm_trans_merge_source_layout_name_from_atom_new(
            node->as->atom,
            namespace_,
            &layout_name
        );
    } else if (node->kind == LM_P0_NODE_FRAME) {
        if (lm_trans_frame_looks_named_structure_layout_declaration(node->as->frame, namespace_)) {
            return lm_trans_merge_push_inline_source_fields(
                owner,
                node->as->frame,
                namespace_,
                index
            );
        }
        status = lm_trans_merge_source_layout_name_from_call_new(node, namespace_, &layout_name);
    } else {
        status = lm_trans_merge_source_layout_name_from_call_new(node, namespace_, &layout_name);
    }

    if (status < 0) {
        return 1;
    }
    if (status == 0 || layout_name == 0) {
        fprintf(stderr, "trans L2 merge error: source must be a layout atom, inline named structure, or call returning a layout\n");
        return 1;
    }

    status = lm_trans_merge_push_source_layout_fields(owner, layout_name, namespace_, index);
    lm_own_delete(layout_name, 0);
    return status;
}

static int lm_trans_declare_merge_named_structure_layout(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    const LmP0Structure *sources;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Text *target;
    const LmP0Text *type_head;
    const LmP0Text *name;
    const LmP0Field *initializer;
    size_t pointer_depth;
    size_t index;

    if (
        namespace_ == 0 ||
        frame == 0 ||
        !lm_trans_frame_looks_merge_named_structure_declaration(frame)
    ) {
        fprintf(stderr, "trans L2 merge error: merge expects (sources...) targetName and an optional body\n");
        return 1;
    }

    target = lm_trans_merge_target_name(frame);
    if (lm_trans_declare_named_structure_owner(namespace_, target) != 0) {
        return 1;
    }

    index = 0U;
    sources = lm_trans_merge_source_list(frame);
    field = sources->first_field;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (lm_trans_merge_push_source_node_fields(target, field->value, namespace_, &index) != 0) {
                return 1;
            }
        }
        field = field->next;
    }

    body = lm_trans_merge_local_body(frame);
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                field->value->kind != LM_P0_NODE_FRAME ||
                !lm_trans_named_structure_field_info(
                    field->value->as->frame,
                    namespace_,
                    &type_head,
                    &name,
                    &initializer,
                    &pointer_depth
                )
            ) {
                return 1;
            }
            (void)initializer;
            if (
                lm_trans_named_structure_push_field_relations(
                    target,
                    name,
                    type_head,
                    index,
                    pointer_depth
                ) != 0
            ) {
                return 1;
            }
            ++index;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_merge_named_structure_typedef(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    const LmP0Text *target;
    char *layout_name;

    target = lm_trans_merge_target_name(frame);
    layout_name = target != 0 ? lm_trans_prefixed_identifier_name_new("", target, "") : 0;
    if (file == 0 || target == 0 || layout_name == 0) {
        lm_own_delete(layout_name, 0);
        return 1;
    }

    if (
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "typedef struct ") != 0 ||
        lm_trans_emit_identifier(file, target) != 0 ||
        lm_trans_put(file, " {\n") != 0 ||
        lm_trans_emit_layout_fields(file, layout_name, namespace_, indent + 1U) != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        lm_trans_put(file, "} ") != 0 ||
        lm_trans_emit_identifier(file, target) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        lm_own_delete(layout_name, 0);
        return 1;
    }

    lm_own_delete(layout_name, 0);
    return 0;
}

static int lm_trans_emit_merge_named_structure_storage(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Text *target;
    char *storage_name;
    char *pointer_name;

    target = lm_trans_merge_target_name(frame);
    if (file == 0 || namespace_ == 0 || target == 0) {
        return 1;
    }

    storage_name = lm_trans_prefixed_identifier_name_new("lm_", target, "_storage");
    pointer_name = lm_trans_prefixed_identifier_name_new("lm_", target, "");
    if (storage_name == 0 || pointer_name == 0) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }

    if (
        lm_trans_emit_indent(file, indent) != 0 ||
        (qualifier != 0 && lm_trans_put(file, qualifier) != 0) ||
        lm_trans_emit_identifier(file, target) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_put(file, storage_name) != 0 ||
        lm_trans_emit_named_structure_body_initializer(file, lm_trans_merge_local_body(frame), namespace_) != 0 ||
        lm_trans_put(file, ";\n") != 0 ||
        lm_trans_emit_indent(file, indent) != 0 ||
        (qualifier != 0 && lm_trans_put(file, qualifier) != 0) ||
        lm_trans_emit_identifier(file, target) != 0 ||
        lm_trans_put(file, " *") != 0 ||
        lm_trans_put(file, pointer_name) != 0 ||
        lm_trans_put(file, " = &") != 0 ||
        lm_trans_put(file, storage_name) != 0 ||
        lm_trans_put(file, ";\n") != 0
    ) {
        lm_own_delete(storage_name, 0);
        lm_own_delete(pointer_name, 0);
        return 1;
    }

    lm_own_delete(storage_name, 0);
    lm_own_delete(pointer_name, 0);
    return 0;
}

static int lm_trans_emit_assignment_target(FILE *file, const LmP0Text *target) {
    size_t i;
    size_t deref_depth;
    LmP0Text *target_payload;
    LmP0Text *target_path;
    int is_c_reference_path;
    int status;

    target_payload = lm_trans_statement_text_new();
    target_path = lm_trans_statement_text_new();
    if (target_payload == 0 || target_path == 0) {
        lm_trans_text_ref_destroy(&target_payload);
        lm_trans_text_ref_destroy(&target_path);
        return 1;
    }
    status = 0;

    if (!lm_trans_identifier_payload(target, target_payload)) {
        status = 1;
        goto cleanup;
    }

    deref_depth = 0U;
    while (
        deref_depth < target_payload->length &&
        target_payload->data[deref_depth] == '\\'
    ) {
        ++deref_depth;
    }

    target_path->data = target_payload->data + deref_depth;
    target_path->length = target_payload->length - deref_depth;
    if (target_path->length == 0U) {
        fprintf(stderr, "trans L2 error: dereferenced assignment target expects a binding or field-follow path\n");
        status = 1;
        goto cleanup;
    }

    is_c_reference_path = lm_trans_is_c_reference_name(target_path);
    if (!is_c_reference_path && lm_trans_text_contains_char(target_path, '.')) {
        fprintf(stderr, "trans L2 error: dot belongs only to c.* ANSI C surface names; use \\ for Lingvamyxa structures\n");
        status = 1;
        goto cleanup;
    }

    while (deref_depth > 0U) {
        if (lm_trans_put(file, "*") != 0) {
            status = 1;
            goto cleanup;
        }
        --deref_depth;
    }
    if (
        target_payload->length != target_path->length &&
        lm_trans_put(file, "(") != 0
    ) {
        status = 1;
        goto cleanup;
    }

    i = 0U;
    if (is_c_reference_path) {
        i = 2U;
    }
    while (i < target_path->length) {
        if (target_path->data[i] == '\\') {
            if (lm_trans_put(file, "->") != 0) {
                status = 1;
                goto cleanup;
            }
        } else if (lm_trans_write_all(file, target_path->data + i, 1U) != 0) {
            status = 1;
            goto cleanup;
        }
        ++i;
    }
    if (
        target_payload->length != target_path->length &&
        lm_trans_put(file, ")") != 0
    ) {
        status = 1;
        goto cleanup;
    }

cleanup:
    lm_trans_text_ref_destroy(&target_payload);
    lm_trans_text_ref_destroy(&target_path);
    return status;
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
    if (lm_trans_emit_expr_fields(file, frame->body->first_field, namespace_) != 0) {
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
    if (lm_trans_emit_expr_fields(file, frame->body->first_field, namespace_) != 0) {
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
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;
    const char *current_qualifier;

    current = node;
    current_qualifier = qualifier;
    while (1) {
        if (lm_trans_node_is_ignored(current)) {
            return 0;
        }

        if (current == 0 || current->kind != LM_P0_NODE_FRAME) {
            fprintf(stderr, "trans L2 error: structure field must be a named field\n");
            return 1;
        }

        frame = current->as->frame;
        if (!lm_trans_text_equals(frame->head, "const")) {
            break;
        }
        name_field = frame->body->first_field;
        if (
            name_field == 0 ||
            name_field->next != 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_FRAME
        ) {
            fprintf(stderr, "trans L2 error: const structure field expects exactly one field declaration\n");
            return 1;
        }
        current_qualifier = "const ";
        current = name_field->value;
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        type_field = lm_trans_nth_field(frame->body, 0U);
        name_field = lm_trans_nth_field(frame->body, 1U);
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
        if (current_qualifier != 0 && lm_trans_put(file, current_qualifier) != 0) {
            return 1;
        }
        if (
            lm_trans_emit_type_and_name(
                file,
                type_field->value,
                name_node->as->atom,
                frame->head->length,
                0
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    name_field = lm_trans_nth_field(frame->body, 0U);
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
    if (current_qualifier != 0 && lm_trans_put(file, current_qualifier) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_identifier(file, name_field->value->as->atom) != 0) {
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
    const LmP0Text *function_name,
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

    field = return_node->as->structure->first_field;
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
    const LmP0Text *function_name,
    const LmP0Node *return_node
) {
    char *type_name;
    const char *stored_name;

    type_name = lm_trans_function_return_struct_type_name_new(function_name);
    if (type_name == 0) {
        return 1;
    }

    stored_name = lm_trans_string_stack_find(lm_trans_emitted_function_return_structs, type_name);
    if (stored_name != 0) {
        lm_own_delete(type_name, 0);
        return 0;
    }

    if (lm_trans_emit_function_return_structure(file, function_name, return_node) != 0) {
        lm_own_delete(type_name, 0);
        return 1;
    }

    if (lm_own_ptr_stack_push(lm_trans_emitted_function_return_structs, type_name) != 0) {
        lm_own_delete(type_name, 0);
        return 1;
    }

    return 0;
}

static void lm_trans_statement_stack_destroy(LmTransStatementStack *stack) {
    if (stack != 0) {
        lm_trans_ptr_stack_delete(&stack->jobs);
    }
}

static int lm_trans_statement_stack_push(LmTransStatementStack *stack, LmTransStatementJob *job) {
    if (stack == 0 || stack->jobs == 0 || job == 0) {
        return 1;
    }

    return lm_own_ptr_stack_push(stack->jobs, job);
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
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_statement_job_emit_list;
    job->destroy = lm_trans_statement_job_destroy_list;
    job->list = (LmTransStatementListJob *)lm_own_new_zero(sizeof(*job->list));
    if (job->list == 0) {
        lm_trans_statement_job_delete(job);
        return 1;
    }
    job->list->field = field;
    job->list->indent = indent;
    job->list->unwrap_single_structure = unwrap_single_structure;
    job->list->repeat_frame = repeat_frame;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
}

static int lm_trans_statement_stack_push_node(
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent,
    const LmP0Frame *repeat_frame
) {
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_statement_job_emit_node;
    job->destroy = lm_trans_statement_job_destroy_node;
    job->node = (LmTransStatementNodeJob *)lm_own_new_zero(sizeof(*job->node));
    if (job->node == 0) {
        lm_trans_statement_job_delete(job);
        return 1;
    }
    job->node->node = node;
    job->node->indent = indent;
    job->node->repeat_frame = repeat_frame;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
}

static int lm_trans_statement_stack_push_frame_job(
    LmTransStatementStack *stack,
    LmTransStatementJobHandler handler,
    const LmP0Frame *frame,
    unsigned indent
) {
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = handler;
    job->destroy = lm_trans_statement_job_destroy_frame;
    job->frame = (LmTransStatementFrameJob *)lm_own_new_zero(sizeof(*job->frame));
    if (job->frame == 0) {
        lm_trans_statement_job_delete(job);
        return 1;
    }
    job->frame->frame = frame;
    job->frame->indent = indent;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
}

static int lm_trans_statement_stack_push_simple(
    LmTransStatementStack *stack,
    LmTransStatementJobHandler handler
) {
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = handler;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
}

static int lm_trans_statement_stack_push_indent_text(
    LmTransStatementStack *stack,
    unsigned indent,
    const char *text
) {
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_statement_job_emit_indent_text;
    job->destroy = lm_trans_statement_job_destroy_text;
    job->text = (LmTransStatementTextJob *)lm_own_new_zero(sizeof(*job->text));
    if (job->text == 0) {
        lm_trans_statement_job_delete(job);
        return 1;
    }
    job->text->indent = indent;
    job->text->text = text;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
}

static int lm_trans_statement_stack_push_sync_leave(
    LmTransStatementStack *stack,
    unsigned indent,
    unsigned cleanup_id
) {
    LmTransStatementJob *job;
    int status;

    job = lm_trans_statement_job_new();
    if (job == 0) {
        return 1;
    }
    job->run = lm_trans_statement_job_emit_sync_leave;
    job->destroy = lm_trans_statement_job_destroy_sync_leave;
    job->sync_leave = (LmTransStatementSyncLeaveJob *)lm_own_new_zero(sizeof(*job->sync_leave));
    if (job->sync_leave == 0) {
        lm_trans_statement_job_delete(job);
        return 1;
    }
    job->sync_leave->indent = indent;
    job->sync_leave->cleanup_id = cleanup_id;
    status = lm_trans_statement_stack_push(stack, job);
    if (status != 0) {
        lm_trans_statement_job_delete(job);
    }
    return status;
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
        return first->value->as->structure->first_field;
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
        frame->body->first_field != 0
    ) {
        return 1;
    }
    if (
        lm_trans_text_is_array_receiver_head(frame->head) &&
        frame->body->first_field != 0
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

    if (frame == 0 || frame->body->first_field == 0) {
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
        lm_trans_statement_stack_push_list(stack, frame->body->first_field, indent + 1U, 1, 0);
    if (status != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_named_structure(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;

    if (lm_trans_declare_named_structure_layout(namespace_, frame) != 0) {
        return 1;
    }
    if (lm_trans_emit_named_structure_typedef(file, frame, indent, namespace_) != 0) {
        return 1;
    }
    return lm_trans_emit_named_structure_storage(file, frame, indent, namespace_, "");
}

static int lm_trans_statement_stack_schedule_merge_named_structure(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;

    if (lm_trans_declare_merge_named_structure_layout(namespace_, frame) != 0) {
        return 1;
    }
    if (lm_trans_emit_merge_named_structure_typedef(file, frame, indent, namespace_) != 0) {
        return 1;
    }
    return lm_trans_emit_merge_named_structure_storage(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_loop_jump_statement(
    FILE *file,
    const LmP0Text *spelling,
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
static int lm_trans_emit_l4_define_table(
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
static int lm_trans_l4_root_head_binding_resolve(const LmP0Text *head, LmTransL4HeadBinding *out);
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
static int lm_trans_emit_l2_os_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_define_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_ifndef_default_frame(
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
    return lm_trans_emit_return_statement(file, frame->body->first_field, indent, namespace_);
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

static int lm_trans_statement_emit_define_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_define_frame(lm_trans_prelude_file(file), frame, namespace_);
}

static int lm_trans_statement_emit_ifndef_default_prelude(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)indent;
    return lm_trans_emit_l2_ifndef_default_frame(lm_trans_prelude_file(file), frame, namespace_);
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

static int lm_trans_statement_emit_table(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_emit_l2_table_structure(file, frame, indent);
}

static int lm_trans_statement_emit_join(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    return lm_trans_emit_l2_join_structure(file, frame, indent);
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

static int lm_trans_statement_emit_named_structure(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_named_structure(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_emit_merge_named_structure(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_statement_stack_schedule_merge_named_structure(file, stack, frame, indent, namespace_);
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
    const LmP0Text *name,
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
static char *lm_trans_env_arg_new(const LmP0Text *env_var_name);

static int lm_trans_statement_lowering_from_head(
    const LmP0Text *head,
    LmTransStatementLowering *out
) {
    LmTransHeadBinding *binding;
    int result;

    if (out == 0) {
        return 0;
    }

    binding = lm_trans_statement_head_binding_new();
    if (binding == 0) {
        return 0;
    }
    result = 0;
    if (lm_trans_head_binding_resolve(0, head, binding) != 0) {
        lm_own_delete(binding, 0);
        return 0;
    }
    if (binding->statement_frame == 0) {
        goto cleanup;
    }

    out->emit = binding->statement_frame;
    result = 1;

cleanup:
    lm_own_delete(binding, 0);
    return result;
}

static int lm_trans_lower_statement_frame(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_,
    LmTransStatementLowering *out
) {
    const LmTransSymbol *symbol;
    LmTransFunctionHeader *function;
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

    function = lm_trans_statement_function_header_new();
    if (function == 0) {
        return 1;
    }
    function_status = lm_trans_function_header_from_frame(frame, 0, function);
    if (function_status < 0) {
        lm_own_delete(function, 0);
        return 1;
    }
    if (function_status > 0) {
        out->emit = lm_trans_statement_emit_nested_function;
        lm_own_delete(function, 0);
        return 0;
    }
    lm_own_delete(function, 0);

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
    if (lm_trans_frame_looks_named_structure_layout_declaration(frame, namespace_)) {
        out->emit = lm_trans_statement_emit_named_structure;
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
            (int)frame->head->length,
            frame->head->data,
            (int)frame->head->length,
            frame->head->data
        );
        return 1;
    }

    fprintf(
        stderr,
        "trans L2 error: \"%.*s\" is %s, not a statement head\n",
        (int)frame->head->length,
        frame->head->data,
        lm_trans_symbol_class_name(symbol->class_name)
    );
    return 1;
}

static int lm_trans_emit_loop_jump_statement(
    FILE *file,
    const LmP0Text *spelling,
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
    const LmTransStatementLowering *lowering
) {
    if (lowering == 0 || lowering->emit == 0) {
        return 1;
    }
    return lowering->emit(file, stack, frame, indent, namespace_);
}

static int lm_trans_statement_stack_emit_frame(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;
    LmTransStatementLowering *lowering;

    status =
        lm_trans_statement_stack_push_frame_job(stack, lm_trans_statement_job_emit_trailer, frame, indent) ||
        lm_trans_statement_stack_push_frame_job(stack, lm_trans_statement_job_validate_end, frame, indent);
    if (status != 0) {
        return 1;
    }

    lowering = lm_trans_statement_lowering_new();
    if (lowering == 0) {
        return 1;
    }
    if (lm_trans_lower_statement_frame(frame, namespace_, lowering) != 0) {
        lm_own_delete(lowering, 0);
        return 1;
    }
    status = lm_trans_statement_stack_emit_frame_lowering(file, stack, frame, indent, namespace_, lowering);
    lm_own_delete(lowering, 0);
    return status;
}

static int lm_trans_emit_atom_loop_jump_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_loop_jump_statement(file, node->as->atom, indent, namespace_);
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

static int lm_trans_atom_statement_emit_define_prelude(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    (void)node;
    (void)indent;
    return lm_trans_emit_l4_define_table(lm_trans_prelude_file(file), namespace_);
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
    const LmP0Text *sequence_name,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmOwnPtrStack *rows;
    LmTransRegistryFact *row;
    LmTransAtomStatementHandler handler;
    LmP0Node *node;
    LmP0NodeAs *node_as;
    LmP0Text *item_name;
    const char *binding;
    size_t emitted;
    size_t expected_index;
    size_t index;
    size_t i;
    int found;
    int status;

    rows = lm_trans_namespace_registry_relation_stack(namespace_, sequence_name, "item");
    if (rows == 0) {
        fprintf(
            stderr,
            "trans registry error: atom statement sequence \"%.*s\" has no item rows\n",
            (int)sequence_name->length,
            sequence_name->data
        );
        return 1;
    }

    node = lm_trans_statement_node_new();
    node_as = lm_trans_statement_node_as_new();
    item_name = lm_trans_statement_text_new();
    if (node == 0 || node_as == 0 || item_name == 0) {
        lm_own_delete(node, 0);
        lm_own_delete(node_as, 0);
        lm_trans_text_ref_destroy(&item_name);
        return 1;
    }

    status = 0;
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
                        (int)sequence_name->length,
                        sequence_name->data,
                        (unsigned long)expected_index
                    );
                    status = 1;
                    goto cleanup;
                }
                found = 1;
                lm_trans_text_assign_cstr(item_name, row->key);
                binding = lm_trans_registry_lookup(item_name, "receiver.atom.statement");
                handler = lm_trans_atom_statement_binding_handler(binding);
                if (handler == 0 || handler == lm_trans_atom_statement_emit_sequence_prelude) {
                    fprintf(
                        stderr,
                        "trans registry error: atom statement sequence \"%.*s\" item %s has no concrete atom receiver\n",
                        (int)sequence_name->length,
                        sequence_name->data,
                        row->key
                    );
                    status = 1;
                    goto cleanup;
                }
                memset(node, 0, sizeof(*node));
                memset(node_as, 0, sizeof(*node_as));
                node->kind = LM_P0_NODE_ATOM;
                node->as = node_as;
                node->as->atom = item_name;
                if (handler(file, node, indent, namespace_) != 0) {
                    status = 1;
                    goto cleanup;
                }
                ++emitted;
            }
        }
        if (!found) {
            fprintf(
                stderr,
                "trans registry error: atom statement sequence \"%.*s\" is missing index %lu\n",
                (int)sequence_name->length,
                sequence_name->data,
                (unsigned long)expected_index
            );
            status = 1;
            goto cleanup;
        }
        ++expected_index;
    }

cleanup:
    lm_own_delete(node, 0);
    lm_own_delete(node_as, 0);
    lm_trans_text_ref_destroy(&item_name);
    return status;
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
    return lm_trans_emit_atom_statement_sequence(file, node->as->atom, indent, namespace_);
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
    LmTransBinding *resolved;
    LmTransAtomStatementHandler handler;

    if (binding == 0) {
        return 0;
    }

    resolved = lm_trans_statement_binding_new();
    if (resolved == 0) {
        return 0;
    }

    handler = 0;
    if (lm_trans_binding_resolve(binding, resolved) && resolved->atom_statement != 0) {
        handler = resolved->atom_statement;
    }
    lm_own_delete(resolved, 0);
    return handler;
}

static LmTransAtomStatementHandler lm_trans_lower_atom_statement(const LmP0Text *atom) {
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
    LmTransFunctionHeader *function;
    const LmTransHoistedFunction *hoisted;
    const LmTransCapture *capture;
    char *env_arg;
    LmP0Text *env_arg_text;
    size_t capture_index;
    int status;

    (void)stack;

    function = lm_trans_statement_function_header_new();
    env_arg = 0;
    env_arg_text = 0;
    if (function == 0) {
        return 1;
    }

    status = lm_trans_function_header_from_frame(frame, 0, function);
    if (status <= 0) {
        status = 1;
        goto cleanup;
    }

    hoisted = lm_trans_namespace_find_hoisted_function(namespace_, frame);
    if (hoisted == 0) {
        fprintf(stderr, "trans L2 internal error: nested function has no hoisted C binding\n");
        status = 1;
        goto cleanup;
    }

    if (hoisted->function->has_env) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->function->env_type_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, " *") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, " = (") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->function->env_type_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, " *)lm_own_new_zero(sizeof(*") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, "));\n") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, "if (") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, " == 0) {\n") != 0) {
            status = 1;
            goto cleanup;
        }
        if (namespace_ != 0 && namespace_->return_type_node != 0) {
            if (
                lm_trans_emit_indent(file, indent + 1U) != 0 ||
                lm_trans_put(file, "return 0;\n") != 0
            ) {
                status = 1;
                goto cleanup;
            }
        } else if (
            lm_trans_emit_indent(file, indent + 1U) != 0 ||
            lm_trans_put(file, "return;\n") != 0
        ) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_indent(file, indent) != 0 || lm_trans_put(file, "}\n") != 0) {
            status = 1;
            goto cleanup;
        }
        for (capture_index = 0U; capture_index < hoisted->captures->count; ++capture_index) {
            capture = (const LmTransCapture *)lm_own_ptr_stack_at(hoisted->captures, capture_index);
            if (capture == 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_emit_indent(file, indent) != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_write_text(file, hoisted->env_var_name) != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_put(file, "->") != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_emit_identifier(file, capture->name) != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_put(file, " = ") != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_emit_identifier(file, capture->name) != 0) {
                status = 1;
                goto cleanup;
            }
            if (lm_trans_put(file, ";\n") != 0) {
                status = 1;
                goto cleanup;
            }
        }
    }

    if (
        lm_trans_namespace_declare_c_name(
            namespace_,
            function->name,
            function->symbol_class,
            hoisted->function->c_name
        ) != 0
    ) {
        status = 1;
        goto cleanup;
    }

    if (hoisted->function->has_env) {
        env_arg = lm_trans_env_arg_new(hoisted->env_var_name);
        if (env_arg == 0) {
            status = 1;
            goto cleanup;
        }
        env_arg_text = lm_trans_text_ref_new_cstr(env_arg);
        if (env_arg_text == 0) {
            status = 1;
            goto cleanup;
        }
        status = lm_trans_namespace_set_env_arg(namespace_, function->name, env_arg_text);
        if (status != 0) {
            goto cleanup;
        }
    }

    if (lm_trans_namespace_set_closure_call_name(namespace_, function->name, hoisted->closure_call_name) != 0) {
        status = 1;
        goto cleanup;
    }

    if (
        !hoisted->function->is_sub &&
        !hoisted->function->is_struct_return &&
        hoisted->function->return_node != 0
    ) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, "(void)") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_write_text(file, hoisted->closure_call_name) != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            status = 1;
            goto cleanup;
        }
    }

    status = lm_trans_namespace_set_callable_shape(namespace_, function);

cleanup:
    lm_own_delete(env_arg, 0);
    lm_trans_text_ref_destroy(&env_arg_text);
    lm_own_delete(function, 0);
    return status;
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
    LmP0Frame *repeated_frame;
    int status;

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (lm_trans_node_is_positional_skip(node)) {
        fprintf(stderr, "trans L2 error: skipped positional statement has no default in this profile\n");
        return 1;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        handler = lm_trans_lower_atom_statement(node->as->atom);
        if (handler == 0) {
            return 1;
        }
        return handler(file, node, indent, namespace_);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_statement_stack_emit_frame(file, stack, node->as->frame, indent, namespace_);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE && repeat_frame != 0) {
        if (lm_trans_frame_looks_storage_declaration(repeat_frame, namespace_)) {
            repeated_frame = lm_trans_statement_frame_new();
            if (repeated_frame == 0) {
                return 1;
            }
            repeated_frame->head = repeat_frame->head;
            repeated_frame->flags = LM_P0_FRAME_COMPACT;
            repeated_frame->body = node->as->structure;
            status = lm_trans_statement_stack_emit_frame(file, stack, repeated_frame, indent, namespace_);
            lm_own_delete(repeated_frame, 0);
            return status;
        }
        if (lm_trans_text_all_char(repeat_frame->head, '@')) {
            return lm_trans_emit_pointer_declaration_repeat_with_qualifier(
                file,
                repeat_frame,
                node->as->structure,
                indent,
                namespace_,
                ""
            );
        }
        if (lm_trans_text_is_array_receiver_head(repeat_frame->head)) {
            return lm_trans_emit_array_declaration_repeat_with_qualifier(
                file,
                repeat_frame,
                node->as->structure,
                indent,
                namespace_,
                ""
            );
        }
        repeated_frame = lm_trans_statement_frame_new();
        if (repeated_frame == 0) {
            return 1;
        }
        repeated_frame->head = repeat_frame->head;
        repeated_frame->flags = LM_P0_FRAME_COMPACT;
        repeated_frame->body = node->as->structure;
        status = lm_trans_statement_stack_emit_frame(file, stack, repeated_frame, indent, namespace_);
        lm_own_delete(repeated_frame, 0);
        return status;
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
    if (job->list == 0) {
        return 1;
    }
    field = lm_trans_statement_list_first_field(
        job->list->field,
        job->list->unwrap_single_structure
    );
    if (field == 0) {
        return 0;
    }

    node = field->value;
    current_repeat_frame =
        job->list->repeat_frame != 0 &&
        node != 0 &&
        node->kind == LM_P0_NODE_STRUCTURE
        ? job->list->repeat_frame
        : 0;

    next_repeat_frame = 0;
    if (
        node != 0 &&
        node->kind == LM_P0_NODE_FRAME &&
        lm_trans_statement_frame_is_repeatable(node->as->frame, namespace_)
    ) {
        next_repeat_frame = node->as->frame;
    } else if (current_repeat_frame != 0) {
        next_repeat_frame = current_repeat_frame;
    }

    status = 0;
    if (field->next != 0) {
        status = lm_trans_statement_stack_push_list(
            stack,
            field->next,
            job->list->indent,
            0,
            next_repeat_frame
        );
    }
    if (status == 0) {
        status = lm_trans_statement_stack_push_node(
            stack,
            field->value,
            job->list->indent,
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
    if (job->node == 0) {
        return 1;
    }
    return lm_trans_statement_stack_emit_node(
        file,
        stack,
        job->node->node,
        job->node->indent,
        job->node->repeat_frame,
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
    if (job->frame == 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(job->frame->frame);
}

static int lm_trans_statement_job_emit_trailer(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)stack;
    if (job->frame == 0 || job->frame->frame == 0) {
        return 1;
    }
    return lm_trans_emit_trailer_statement(
        file,
        job->frame->frame->trailer,
        job->frame->indent,
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
    if (job->text == 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, job->text->indent) != 0) {
        return 1;
    }
    return lm_trans_put(file, job->text->text);
}

static int lm_trans_statement_job_emit_sync_leave(
    FILE *file,
    LmTransStatementStack *stack,
    LmTransStatementJob *job,
    LmTransNamespace *namespace_
) {
    (void)stack;
    (void)namespace_;
    if (job->sync_leave == 0) {
        return 1;
    }
    return lm_trans_statement_stack_emit_sync_leave(
        file,
        job->sync_leave->indent,
        job->sync_leave->cleanup_id
    );
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    LmTransStatementStack *stack;
    LmTransStatementJob *job;
    int status;

    stack = lm_trans_statement_stack_new();
    if (stack == 0) {
        lm_own_delete(stack, 0);
        return 1;
    }
    stack->jobs = lm_trans_ptr_stack_new(lm_trans_statement_job_delete_any);
    if (stack->jobs == 0) {
        lm_own_delete(stack, 0);
        return 1;
    }
    status = lm_trans_statement_stack_push_list(stack, first, indent, 1, 0);

    while (status == 0 && stack->jobs != 0 && stack->jobs->count > 0U) {
        job = (LmTransStatementJob *)lm_own_ptr_stack_pop(stack->jobs);
        if (job == 0) {
            status = 1;
            break;
        }
        if (job->run == 0) {
            status = 1;
        } else {
            status = job->run(file, stack, job, namespace_);
        }
        lm_trans_statement_job_delete(job);
    }

    lm_trans_statement_stack_destroy(stack);
    lm_own_delete(stack, 0);
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
    field = params->as->structure->first_field;
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
    LmP0Text *name;
    int first;
    int status;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    name = lm_trans_statement_text_new();
    if (name == 0) {
        return 1;
    }
    status = 0;
    first = 1;
    field = params->as->structure->first_field;
    while (field != 0) {
        if (!lm_trans_formal_param_name(field->value, name)) {
            status = 1;
            goto cleanup;
        }
        if (!first && lm_trans_put(file, ", ") != 0) {
            status = 1;
            goto cleanup;
        }
        if (lm_trans_emit_identifier(file, name) != 0) {
            status = 1;
            goto cleanup;
        }
        first = 0;
        field = field->next;
    }

cleanup:
    lm_trans_text_ref_destroy(&name);
    return status;
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
    function = hoisted->function;
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
        *out_frame = node->as->frame;
        return 1;
    }

    if (node->kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }

    field = node->as->structure->first_field;
    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        *out_frame = field->value->as->frame;
        return 1;
    }

    return 0;
}

static int lm_trans_formal_param_name(const LmP0Node *node, LmP0Text *out_name) {
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Field *field;
    const LmP0Field *name_field;
    size_t name_index;
    size_t unwrap_index;

    if (out_name == 0) {
        return 0;
    }

    current = node;
    while (lm_trans_single_frame_node(current, &frame)) {
        if (!lm_trans_frame_formal_param_unwrap_index(frame, &unwrap_index)) {
            break;
        }
        field = lm_trans_nth_field(frame->body, unwrap_index);
        if (field == 0 || field->value == 0) {
            return 0;
        }
        current = field->value;
    }

    if (!lm_trans_single_frame_node(current, &frame)) {
        return 0;
    }

    if (lm_trans_frame_positional_name_index(frame, &name_index)) {
        name_field = lm_trans_nth_field(frame->body, name_index);
    } else {
        name_field = lm_trans_nth_field(frame->body, 0U);
    }

    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_name = *name_field->value->as->atom;
    return 1;
}

static int lm_trans_namespace_set_signature(
    LmTransNamespace *namespace_,
    const LmP0Text *name,
    const LmP0Frame *function_frame
) {
    LmTransSymbol *symbol;
    const LmP0Field *params_field;
    const LmP0Field *field;
    LmP0Text *param_name;
    LmOwnPtrStack *param_names;
    LmP0Text *param_name_ref;
    LmP0Text *existing_param_name_ref;
    size_t index;
    size_t i;
    int allow_unnamed_params;

    if (namespace_ == 0 || name == 0 || function_frame == 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }

    params_field = lm_trans_nth_field(function_frame->body, 1U);
    if (
        params_field == 0 ||
        params_field->value == 0 ||
        params_field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects a parameter Structure\n");
        return 1;
    }

    param_name = lm_trans_statement_text_new();
    if (param_name == 0) {
        return 1;
    }
    param_names = lm_trans_ptr_stack_new(lm_trans_text_ref_delete_any);
    if (param_names == 0) {
        lm_trans_text_ref_destroy(&param_name);
        return 1;
    }

    allow_unnamed_params = lm_trans_symbol_is(symbol, "callableDescriptor");
    index = 0U;
    field = params_field->value->as->structure->first_field;
    while (field != 0) {
        if (!lm_trans_formal_param_name(field->value, param_name)) {
            if (!allow_unnamed_params) {
                fprintf(
                    stderr,
                    "trans L2 error: function \"%.*s\" parameter %lu must expose a binding name\n",
                    (int)name->length,
                    name->data,
                    (unsigned long)index
                );
                lm_trans_ptr_stack_delete(&param_names);
                lm_trans_text_ref_destroy(&param_name);
                return 1;
            }
            lm_trans_text_assign_cstr(param_name, "");
        }
        for (i = 0U; i < index; ++i) {
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(param_names, i);
            if (
                param_name->length != 0U &&
                param_name_ref != 0 &&
                lm_trans_identifier_same(param_name_ref, param_name)
            ) {
                fprintf(stderr, "trans L2 error: duplicate function parameter name\n");
                lm_trans_ptr_stack_delete(&param_names);
                lm_trans_text_ref_destroy(&param_name);
                return 1;
            }
        }
        param_name_ref = lm_trans_text_ref_new(param_name);
        if (param_name_ref == 0) {
            lm_trans_ptr_stack_delete(&param_names);
            lm_trans_text_ref_destroy(&param_name);
            return 1;
        }
        if (lm_own_ptr_stack_push(param_names, param_name_ref) != 0) {
            lm_trans_text_ref_delete_any(param_name_ref);
            lm_trans_ptr_stack_delete(&param_names);
            lm_trans_text_ref_destroy(&param_name);
            return 1;
        }
        ++index;
        field = field->next;
    }

    if (symbol->has_signature) {
        if (symbol->param_names == 0 || symbol->param_names->count != param_names->count) {
            fprintf(stderr, "trans L2 error: os-imported function signatures must match\n");
            lm_trans_ptr_stack_delete(&param_names);
            lm_trans_text_ref_destroy(&param_name);
            return 1;
        }
        for (i = 0U; i < param_names->count; ++i) {
            existing_param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(symbol->param_names, i);
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(param_names, i);
            if (
                existing_param_name_ref == 0 ||
                param_name_ref == 0 ||
                !lm_trans_identifier_same(existing_param_name_ref, param_name_ref)
            ) {
                fprintf(stderr, "trans L2 error: os-imported function signatures must match\n");
                lm_trans_ptr_stack_delete(&param_names);
                lm_trans_text_ref_destroy(&param_name);
                return 1;
            }
        }
        lm_trans_ptr_stack_delete(&param_names);
        lm_trans_text_ref_destroy(&param_name);
        return 0;
    }

    lm_trans_ptr_stack_delete(&symbol->param_names);
    symbol->param_names = param_names;
    symbol->has_signature = 1;
    lm_trans_text_ref_destroy(&param_name);
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
    int is_struct_return,
    int is_lazy_descriptor
) {
    const LmP0Field *name_field;
    const LmP0Field *params_field;
    const LmP0Field *return_field;
    const LmP0Field *body_field;
    LmP0Text *name;
    LmP0Text *c_name;
    LmP0Text *env_type_name;

    if (frame == 0 || out == 0) {
        return 0;
    }

    name_field = lm_trans_nth_field(frame->body, 0U);
    params_field = lm_trans_nth_field(frame->body, 1U);

    if (
        name_field == 0 ||
        params_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects name and parameters\n");
        return -1;
    }

    if (lm_trans_function_header_init_fields(out) != 0) {
        return -1;
    }
    name = out->name;
    c_name = out->c_name;
    env_type_name = out->env_type_name;
    memset(out, 0, sizeof(*out));
    out->name = name;
    out->c_name = c_name;
    out->env_type_name = env_type_name;
    out->frame = frame;
    *out->name = *name_field->value->as->atom;
    if (!lm_trans_identifier_payload(out->name, out->c_name)) {
        return -1;
    }
    out->params_node = params_field->value;
    out->symbol_class = is_sub ? "procedure" : "function";
    out->is_sub = is_sub;
    out->is_struct_return = is_struct_return;
    out->is_external = is_external;
    out->is_lazy_descriptor = is_lazy_descriptor;

    if (is_sub) {
        body_field = lm_trans_nth_field(frame->body, 2U);
        if (
            body_field != 0 &&
            body_field->next == 0 &&
            body_field->value != 0 &&
            body_field->value->kind == LM_P0_NODE_STRUCTURE
        ) {
            out->body_start = body_field->value->as->structure->first_field;
        } else {
            out->body_start = params_field->next;
        }
        if (out->body_start == 0) {
            out->is_descriptor_only = 1;
        }
        return 1;
    }

    return_field = lm_trans_nth_field(frame->body, 2U);
    if (return_field == 0 || return_field->value == 0) {
        fprintf(stderr, "trans L2 error: fn expects return type\n");
        return -1;
    }

    out->return_node = return_field->value;
    body_field = lm_trans_nth_field(frame->body, 3U);
    if (
        body_field != 0 &&
        body_field->next == 0 &&
        body_field->value != 0 &&
        body_field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        out->body_start = body_field->value->as->structure->first_field;
    } else {
        out->body_start = return_field->next;
    }

    if (
        !is_struct_return &&
        out->body_start == 0 &&
        (
            frame->trailer == 0 ||
            !lm_trans_text_equals(frame->trailer->spelling, "return")
        )
    ) {
        out->symbol_class = "callableDescriptor";
        out->is_descriptor_only = 1;
    }
    if (is_lazy_descriptor && !out->is_descriptor_only) {
        fprintf(stderr, "trans L2 error: lazy fn is supported only for descriptor-only fn declarations in this profile\n");
        return -1;
    }

    return 1;
}

static int lm_trans_receiver_fn(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 0, 0, 0);
}

static int lm_trans_receiver_lazy_fn(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 0, 0, 1);
}

static int lm_trans_receiver_fm(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 0, 1, 0);
}

static int lm_trans_receiver_sub(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    return lm_trans_function_header_common(frame, is_external, out, 1, 0, 0);
}

typedef struct LmTransPointerBinding {
    const char *name;
    LmTransBinding *value;
} LmTransPointerBinding;

static void lm_trans_pointer_binding_delete(void *object) {
    LmTransPointerBinding *binding;

    binding = (LmTransPointerBinding *)object;
    if (binding == 0) {
        return;
    }
    lm_own_delete(binding->value, 0);
    lm_own_delete(binding, 0);
}

static int lm_trans_pointer_binding_push(const char *name, const LmTransBinding *value) {
    LmTransPointerBinding *binding;

    if (lm_trans_registry == 0 || name == 0 || value == 0) {
        return 1;
    }
    if (lm_trans_registry->binding_pointer_bindings == 0) {
        lm_trans_registry->binding_pointer_bindings = lm_trans_ptr_stack_new(lm_trans_pointer_binding_delete);
        if (lm_trans_registry->binding_pointer_bindings == 0) {
            return 1;
        }
    }
    binding = (LmTransPointerBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    binding->name = name;
    binding->value = (LmTransBinding *)lm_own_new_zero(sizeof(*binding->value));
    if (binding->value == 0) {
        lm_trans_pointer_binding_delete(binding);
        return 1;
    }
    *binding->value = *value;
    if (lm_own_ptr_stack_push(lm_trans_registry->binding_pointer_bindings, binding) != 0) {
        lm_trans_pointer_binding_delete(binding);
        return 1;
    }
    return 0;
}

static int lm_trans_pointer_binding_push_call_lowering(
    const char *name,
    LmTransCallLoweringHandler handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->call_lowering = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_expr_emit(
    const char *name,
    LmTransExprAtomEmitHandler emit,
    LmTransExprAtomStateHandler state
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->expr_emit = emit;
    value->expr_state = state;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_expr_frame(
    const char *name,
    LmTransExprFrameHandler handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->expr_frame = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_statement(
    const char *name,
    LmTransStatementFrameHandler handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->statement_frame = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_function_receiver(
    const char *name,
    LmTransFunctionHeaderReceiver handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->function_receiver = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_type_emit(
    const char *name,
    LmTransTypeEmitReceiver handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->type_emit = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_type_alloc(
    const char *name,
    LmTransTypeStructureValueAllocReceiver handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->type_structure_value_alloc = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_type_fill(
    const char *name,
    LmTransTypeStructureValueFillReceiver handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->type_structure_value_fill = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_materializer(
    const char *name,
    LmTransExprSegmentMaterializer handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->expr_segment_materializer = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_binding_push_atom_statement(
    const char *name,
    LmTransAtomStatementHandler handler
) {
    LmTransBinding *value;
    int status;

    value = lm_trans_statement_binding_new();
    if (value == 0) {
        return 1;
    }
    value->atom_statement = handler;
    status = lm_trans_pointer_binding_push(name, value);
    lm_own_delete(value, 0);
    return status;
}

static int lm_trans_pointer_bindings_init(void) {
    int status;

    if (lm_trans_registry == 0) {
        return 1;
    }
    if (lm_trans_registry->binding_pointer_bindings != 0) {
        return 0;
    }

    status =
        lm_trans_pointer_binding_push_call_lowering("lm_trans_call_lower_value", lm_trans_call_lower_value) != 0 ||
        lm_trans_pointer_binding_push_call_lowering("lm_trans_call_lower_statement", lm_trans_call_lower_statement) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_value", lm_trans_expr_emit_value, lm_trans_expr_state_value) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_addressable_value", lm_trans_expr_emit_addressable_value, lm_trans_expr_state_value) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_name", lm_trans_expr_emit_name, lm_trans_expr_state_value) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_field_name", lm_trans_expr_emit_name, lm_trans_expr_state_field_name) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_c_field_name", lm_trans_expr_emit_name, lm_trans_expr_state_c_field_name) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_c_surface", lm_trans_expr_emit_name, lm_trans_expr_state_c_surface) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_operator", lm_trans_expr_emit_raw, lm_trans_expr_state_operator) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_deref", lm_trans_expr_emit_raw, lm_trans_expr_state_deref) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_pointer_follow", lm_trans_expr_emit_raw, lm_trans_expr_state_pointer_follow) != 0 ||
        lm_trans_pointer_binding_push_expr_emit("lm_trans_expr_emit_c_dot", lm_trans_expr_emit_raw, lm_trans_expr_state_c_dot) != 0 ||
        lm_trans_pointer_binding_push_expr_frame("lm_trans_expr_emit_cast_frame", lm_trans_expr_emit_cast_frame) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_return", lm_trans_statement_emit_return) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_if", lm_trans_statement_emit_if) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_while", lm_trans_statement_emit_while) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_else", lm_trans_statement_emit_else) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_synchronized", lm_trans_statement_emit_synchronized) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_loop_jump", lm_trans_statement_emit_loop_jump) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_include_prelude", lm_trans_statement_emit_include_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_define_prelude", lm_trans_statement_emit_define_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_ifndef_default_prelude", lm_trans_statement_emit_ifndef_default_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_os_prelude", lm_trans_statement_emit_os_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_ifdef_prelude", lm_trans_statement_emit_ifdef_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_guard_prelude", lm_trans_statement_emit_guard_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_extern_c_prelude", lm_trans_statement_emit_extern_c_prelude) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_import", lm_trans_statement_emit_import) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_table", lm_trans_statement_emit_table) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_join", lm_trans_statement_emit_join) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_merge_named_structure", lm_trans_statement_emit_merge_named_structure) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_const_declaration", lm_trans_statement_emit_const_declaration) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_array_declaration", lm_trans_statement_emit_array_declaration) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_structure_declaration", lm_trans_statement_emit_structure_declaration) != 0 ||
        lm_trans_pointer_binding_push_statement("lm_trans_statement_emit_c_tagged_declaration", lm_trans_statement_emit_c_tagged_declaration) != 0 ||
        lm_trans_pointer_binding_push_function_receiver("lm_trans_receiver_fn", lm_trans_receiver_fn) != 0 ||
        lm_trans_pointer_binding_push_function_receiver("lm_trans_receiver_lazy_fn", lm_trans_receiver_lazy_fn) != 0 ||
        lm_trans_pointer_binding_push_function_receiver("lm_trans_receiver_fm", lm_trans_receiver_fm) != 0 ||
        lm_trans_pointer_binding_push_function_receiver("lm_trans_receiver_sub", lm_trans_receiver_sub) != 0 ||
        lm_trans_pointer_binding_push_type_emit("lm_trans_type_receiver_array_emit", lm_trans_type_receiver_array_emit) != 0 ||
        lm_trans_pointer_binding_push_type_alloc("lm_trans_type_receiver_array_structure_value_alloc", lm_trans_type_receiver_array_structure_value_alloc) != 0 ||
        lm_trans_pointer_binding_push_type_fill("lm_trans_type_receiver_array_structure_value_fill", lm_trans_type_receiver_array_structure_value_fill) != 0 ||
        lm_trans_pointer_binding_push_materializer("lm_trans_materialize_zero_arg_callable", lm_trans_materialize_zero_arg_callable) != 0 ||
        lm_trans_pointer_binding_push_materializer("lm_trans_materialize_callable_descriptor_value", lm_trans_materialize_callable_descriptor_value) != 0 ||
        lm_trans_pointer_binding_push_materializer("lm_trans_materialize_array_value", lm_trans_materialize_array_value) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_include_prelude", lm_trans_atom_statement_emit_include_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_define_prelude", lm_trans_atom_statement_emit_define_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_os_prelude", lm_trans_atom_statement_emit_os_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_layout_prelude", lm_trans_atom_statement_emit_layout_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_constant_prelude", lm_trans_atom_statement_emit_constant_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_alias_prelude", lm_trans_atom_statement_emit_alias_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_forward_prelude", lm_trans_atom_statement_emit_forward_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_prototype_prelude", lm_trans_atom_statement_emit_prototype_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_fn_prelude", lm_trans_atom_statement_emit_fn_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_guard_prelude", lm_trans_atom_statement_emit_guard_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_extern_c_prelude", lm_trans_atom_statement_emit_extern_c_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_unit_prelude", lm_trans_atom_statement_emit_unit_prelude) != 0 ||
        lm_trans_pointer_binding_push_atom_statement("lm_trans_atom_statement_emit_sequence_prelude", lm_trans_atom_statement_emit_sequence_prelude) != 0;
    if (status != 0) {
        lm_trans_ptr_stack_delete(&lm_trans_registry->binding_pointer_bindings);
    }
    return status;
}

static const LmTransPointerBinding *lm_trans_pointer_binding_find(const char *name) {
    size_t i;
    const LmTransPointerBinding *binding;
    const LmOwnPtrStack *bindings;

    if (lm_trans_registry == 0 || name == 0) {
        return 0;
    }
    bindings = lm_trans_registry->binding_pointer_bindings;
    if (bindings == 0) {
        return 0;
    }

    for (i = 0U; i < bindings->count; ++i) {
        binding = (const LmTransPointerBinding *)lm_own_ptr_stack_at(bindings, i);
        if (binding != 0 && binding->name != 0 && strcmp(binding->name, name) == 0) {
            return binding;
        }
    }
    return 0;
}

static int lm_trans_binding_resolve(const char *binding, LmTransBinding *out) {
    const LmTransPointerBinding *resolved;

    if (out == 0) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    if (binding == 0 || lm_trans_pointer_bindings_init() != 0) {
        return 0;
    }

    resolved = lm_trans_pointer_binding_find(binding);
    if (resolved == 0) {
        return 0;
    }
    if (resolved->value == 0) {
        return 0;
    }
    *out = *resolved->value;
    return 1;
}

static int lm_trans_function_header_from_frame(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    LmTransHeadBinding *binding;
    int result;

    if (frame == 0 || out == 0) {
        return 0;
    }

    binding = lm_trans_statement_head_binding_new();
    if (binding == 0) {
        return -1;
    }
    if (lm_trans_head_binding_resolve(0, frame->head, binding) != 0) {
        lm_own_delete(binding, 0);
        return -1;
    }
    if (binding->function_receiver == 0) {
        lm_own_delete(binding, 0);
        return 0;
    }

    result = binding->function_receiver(frame, is_external, out);
    lm_own_delete(binding, 0);
    return result;
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

    field = frame->body->first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: external receiver expects exactly one fn/sub frame\n");
        return -1;
    }

    inner = field->value->as->frame;
    status = lm_trans_function_header_from_frame(inner, 1, out);
    if (status != 0) {
        return status;
    }

    fprintf(stderr, "trans L2 error: external receiver expects fn/sub frame\n");
    return -1;
}

static char *lm_trans_hoisted_c_name_new(
    const LmP0Text *parent_name,
    const LmP0Text *child_name,
    size_t index
) {
    char *name;
    int needed;

    if (parent_name == 0 || child_name == 0) {
        return 0;
    }
    needed = snprintf(
        0,
        0,
        "lm_%.*s_%.*s_%zu",
        (int)parent_name->length,
        parent_name->data,
        (int)child_name->length,
        child_name->data,
        index
    );
    if (needed < 0) {
        return 0;
    }

    name = (char *)lm_own_new_zero((size_t)needed + 1U);
    if (name == 0) {
        return 0;
    }

    if (
        snprintf(
            name,
            (size_t)needed + 1U,
            "lm_%.*s_%.*s_%zu",
            (int)parent_name->length,
            parent_name->data,
            (int)child_name->length,
            child_name->data,
            index
        ) != needed
    ) {
        lm_own_delete(name, 0);
        return 0;
    }

    return name;
}

static char *lm_trans_hoisted_suffix_name_new(const LmP0Text *base_name, const char *suffix) {
    char *name;
    int needed;

    needed = snprintf(
        0,
        0,
        "%.*s%s",
        (int)base_name->length,
        base_name->data,
        suffix != 0 ? suffix : ""
    );
    if (needed < 0) {
        return 0;
    }

    name = (char *)lm_own_new_zero((size_t)needed + 1U);
    if (name == 0) {
        return 0;
    }

    if (
        snprintf(
            name,
            (size_t)needed + 1U,
            "%.*s%s",
            (int)base_name->length,
            base_name->data,
            suffix != 0 ? suffix : ""
        ) != needed
    ) {
        lm_own_delete(name, 0);
        return 0;
    }

    return name;
}

static char *lm_trans_captured_expr_new(const LmP0Text *name) {
    char *text;
    int needed;

    if (name == 0) {
        return 0;
    }
    needed = snprintf(
        0,
        0,
        "lm_env->%.*s",
        (int)name->length,
        name->data
    );
    if (needed < 0) {
        return 0;
    }

    text = (char *)lm_own_new_zero((size_t)needed + 1U);
    if (text == 0) {
        return 0;
    }
    if (
        snprintf(
            text,
            (size_t)needed + 1U,
            "lm_env->%.*s",
            (int)name->length,
            name->data
        ) != needed
    ) {
        lm_own_delete(text, 0);
        return 0;
    }
    return text;
}

static char *lm_trans_env_arg_new(const LmP0Text *env_var_name) {
    char *text;
    int needed;

    if (env_var_name == 0) {
        return 0;
    }
    needed = snprintf(
        0,
        0,
        "%.*s",
        (int)env_var_name->length,
        env_var_name->data
    );
    if (needed < 0) {
        return 0;
    }

    text = (char *)lm_own_new_zero((size_t)needed + 1U);
    if (text == 0) {
        return 0;
    }
    if (
        snprintf(
            text,
            (size_t)needed + 1U,
            "%.*s",
            (int)env_var_name->length,
            env_var_name->data
        ) != needed
    ) {
        lm_own_delete(text, 0);
        return 0;
    }
    return text;
}

static const LmTransCapture *lm_trans_capture_stack_find(
    const LmOwnPtrStack *captures,
    const LmP0Text *name
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

static int lm_trans_local_name_stack_has(const LmOwnPtrStack *names, const LmP0Text *name) {
    size_t i;
    const LmP0Text *stored;

    if (names == 0) {
        return 0;
    }

    for (i = 0U; i < names->count; ++i) {
        stored = (const LmP0Text *)lm_own_ptr_stack_at(names, i);
        if (stored != 0 && lm_trans_identifier_same(stored, name)) {
            return 1;
        }
    }
    return 0;
}

static int lm_trans_local_name_stack_add(LmOwnPtrStack *names, const LmP0Text *name) {
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
    const LmP0Frame *current;
    const LmP0Frame *inner;

    if (frame == 0 || out == 0) {
        return 0;
    }

    current = frame;
    while (lm_trans_text_equals(current->head, "const")) {
        field = current->body->first_field;
        if (
            field == 0 ||
            field->next != 0 ||
            field->value == 0 ||
            field->value->kind != LM_P0_NODE_FRAME
        ) {
            return 0;
        }
        current = field->value->as->frame;
    }

    body = lm_trans_unwrap_single_anonymous_structure(current->body);

    if (lm_trans_head_can_declare_storage(current->head, namespace_)) {
        name_field = lm_trans_nth_field(body, 0U);
        if (
            name_field == 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            return 0;
        }
        if (lm_trans_capture_reset(out) != 0) {
            return 0;
        }
        *out->name = *name_field->value->as->atom;
        *out->type_head = *current->head;
        out->type_is_head = 1;
        return 1;
    }

    if (lm_trans_text_all_char(current->head, '@')) {
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
        if (lm_trans_capture_reset(out) != 0) {
            return 0;
        }
        *out->name = *name_field->value->as->atom;
        out->type_node = type_field->value;
        out->pointer_depth = current->head->length;
        return 1;
    }

    if (
        lm_trans_text_is_array_receiver_head(current->head) &&
        body != 0 &&
        body->first_field != 0 &&
        body->first_field->next == 0 &&
        body->first_field->value != 0 &&
        body->first_field->value->kind == LM_P0_NODE_FRAME
    ) {
        inner = body->first_field->value->as->frame;
        if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
            name_field = inner->body->first_field;
            if (
                name_field == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                return 0;
            }
            if (lm_trans_capture_reset(out) != 0) {
                return 0;
            }
            *out->name = *name_field->value->as->atom;
            *out->type_head = *inner->head;
            out->type_is_head = 1;
            out->pointer_depth = 1U;
            return 1;
        }
        if (lm_trans_text_all_char(inner->head, '@')) {
            type_field = lm_trans_nth_field(inner->body, 0U);
            name_field = lm_trans_nth_field(inner->body, 1U);
            if (
                type_field == 0 ||
                name_field == 0 ||
                type_field->value == 0 ||
                name_field->value == 0 ||
                name_field->value->kind != LM_P0_NODE_ATOM
            ) {
                return 0;
            }
            if (lm_trans_capture_reset(out) != 0) {
                return 0;
            }
            *out->name = *name_field->value->as->atom;
            out->type_node = type_field->value;
            out->pointer_depth = inner->head->length + 1U;
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
    LmTransCapture *capture;
    LmTransCapture *stored;
    int status;

    capture = lm_trans_statement_capture_new();
    if (capture == 0) {
        return 1;
    }
    status = 0;
    if (!lm_trans_capture_from_decl_frame(frame, namespace_, capture)) {
        lm_trans_capture_destroy(capture);
        return 0;
    }
    if (lm_trans_capture_stack_find(candidates, capture->name) != 0) {
        goto cleanup;
    }

    stored = lm_trans_capture_new(
        capture->name,
        capture->type_head,
        capture->type_node,
        capture->pointer_depth,
        capture->type_is_head
    );
    if (stored == 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_own_ptr_stack_push(candidates, stored) != 0) {
        lm_trans_capture_destroy(stored);
        status = 1;
        goto cleanup;
    }

cleanup:
    lm_trans_capture_destroy(capture);
    return status;
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

    field = params->as->structure->first_field;
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
    LmTransFunctionHeader *nested;
    LmTransCapture *capture;
    int status;

    nested = lm_trans_statement_function_header_new();
    capture = lm_trans_statement_capture_new();
    if (nested == 0 || capture == 0) {
        lm_own_delete(nested, 0);
        lm_trans_capture_destroy(capture);
        return 1;
    }

    if (frame != 0 && !lm_trans_local_name_stack_has(names, frame->head)) {
        status = lm_trans_function_header_from_frame(frame, 0, nested);
        if (status < 0) {
            lm_own_delete(nested, 0);
            lm_trans_capture_destroy(capture);
            return 1;
        }
        if (status > 0) {
            lm_own_delete(nested, 0);
            lm_trans_capture_destroy(capture);
            return 0;
        }
    }

    if (lm_trans_capture_from_decl_frame(frame, namespace_, capture)) {
        status = lm_trans_local_name_stack_add(names, capture->name);
        lm_own_delete(nested, 0);
        lm_trans_capture_destroy(capture);
        return status;
    }

    status = lm_trans_collect_local_names_from_fields(
        names,
        frame != 0 ? frame->body->first_field : 0,
        namespace_
    );
    lm_own_delete(nested, 0);
    lm_trans_capture_destroy(capture);
    return status;
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
        return lm_trans_collect_local_names_from_frame(names, node->as->frame, namespace_);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_collect_local_names_from_fields(
            names,
            node->as->structure->first_field,
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
    LmTransFunctionHeader *nested;
    int status;

    nested = lm_trans_statement_function_header_new();
    if (nested == 0) {
        return 1;
    }

    if (frame != 0 && !lm_trans_local_name_stack_has(local_names, frame->head)) {
        status = lm_trans_function_header_from_frame(frame, 0, nested);
        if (status < 0) {
            lm_own_delete(nested, 0);
            return 1;
        }
        if (status > 0) {
            lm_own_delete(nested, 0);
            return 0;
        }
    }

    status = lm_trans_collect_capture_uses_from_fields(
        captures,
        candidates,
        local_names,
        frame != 0 ? frame->body->first_field : 0
    );
    lm_own_delete(nested, 0);
    return status;
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
        candidate = lm_trans_capture_stack_find(candidates, node->as->atom);
        if (
            candidate != 0 &&
            !lm_trans_local_name_stack_has(local_names, node->as->atom)
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
            node->as->frame
        );
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_collect_capture_uses_from_fields(
            captures,
            candidates,
            local_names,
            node->as->structure->first_field
        );
    }

    return 0;
}

static int lm_trans_analyze_function_captures(
    LmTransHoistedFunction *hoisted,
    const LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *local_names;
    const LmP0Field *field;
    LmP0Text *param_name;
    int status;

    if (hoisted == 0) {
        return 1;
    }

    local_names = lm_trans_ptr_stack_new(lm_trans_text_ref_delete_any);
    param_name = lm_trans_statement_text_new();
    if (local_names == 0 || param_name == 0) {
        lm_trans_ptr_stack_delete(&local_names);
        lm_trans_text_ref_destroy(&param_name);
        return 1;
    }
    status = 0;

    if (
        hoisted->function->params_node != 0 &&
        hoisted->function->params_node->kind == LM_P0_NODE_STRUCTURE
    ) {
        field = hoisted->function->params_node->as->structure->first_field;
        while (status == 0 && field != 0) {
            if (lm_trans_formal_param_name(field->value, param_name)) {
                status = lm_trans_local_name_stack_add(local_names, param_name);
            }
            field = field->next;
        }
    }

    if (status == 0) {
        status = lm_trans_collect_local_names_from_fields(
            local_names,
            hoisted->function->body_start,
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_collect_capture_uses_from_fields(
            hoisted->captures,
            candidates,
            local_names,
            hoisted->function->body_start
        );
    }

    lm_trans_ptr_stack_delete(&local_names);
    lm_trans_text_ref_destroy(&param_name);
    return status;
}

static int lm_trans_collect_hoisted_functions_from_node(
    LmOwnPtrStack *hoisted_functions,
    const LmP0Text *parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Node *node
);

static int lm_trans_collect_hoisted_functions_from_fields(
    LmOwnPtrStack *hoisted_functions,
    const LmP0Text *parent_c_name,
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
    const LmP0Text *parent_c_name,
    const LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame,
    const LmTransFunctionHeader *function
) {
    LmTransHoistedFunction *hoisted;
    char *c_name;
    char *env_type_name;
    char *env_var_name;
    char *closure_call_name;
    size_t index;

    if (function == 0 || function->is_descriptor_only) {
        fprintf(stderr, "trans L2 error: nested descriptor-only fn is not supported yet\n");
        return 1;
    }

    index = hoisted_functions != 0 ? hoisted_functions->count : 0U;
    c_name = lm_trans_hoisted_c_name_new(parent_c_name, function->name, index);
    if (c_name == 0) {
        return 1;
    }
    closure_call_name = lm_trans_hoisted_suffix_name_new(lm_trans_text_from_cstr(c_name), "_closure_call");
    if (closure_call_name == 0) {
        lm_own_delete(c_name, 0);
        return 1;
    }

    hoisted = (LmTransHoistedFunction *)lm_own_new_zero(sizeof(*hoisted));
    if (hoisted == 0) {
        lm_own_delete(c_name, 0);
        lm_own_delete(closure_call_name, 0);
        return 1;
    }
    hoisted->env_var_name = lm_trans_text_ref_new_cstr("");
    hoisted->closure_call_name = lm_trans_text_ref_new_cstr("");
    if (hoisted->env_var_name == 0 || hoisted->closure_call_name == 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        lm_own_delete(c_name, 0);
        lm_own_delete(closure_call_name, 0);
        return 1;
    }
    hoisted->c_name_storage = c_name;
    hoisted->closure_call_storage = closure_call_name;

    hoisted->function = lm_trans_function_header_new();
    if (hoisted->function == 0 || lm_trans_function_header_copy(hoisted->function, function) != 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }
    hoisted->function->frame = frame;
    lm_trans_text_assign_cstr(hoisted->function->c_name, c_name);
    hoisted->function->is_external = 0;
    hoisted->function->declare_self_alias = 1;
    hoisted->captures = lm_trans_ptr_stack_new(lm_trans_capture_delete_any);
    if (hoisted->captures == 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }
    lm_trans_text_assign_cstr(hoisted->closure_call_name, closure_call_name);

    if (lm_trans_analyze_function_captures(hoisted, candidates, namespace_) != 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }

    if (hoisted->captures->count > 0U) {
        env_type_name = lm_trans_hoisted_suffix_name_new(hoisted->function->c_name, "Env");
        env_var_name = lm_trans_hoisted_suffix_name_new(hoisted->function->c_name, "_env");
        if (env_type_name == 0 || env_var_name == 0) {
            lm_own_delete(env_type_name, 0);
            lm_own_delete(env_var_name, 0);
            lm_trans_hoisted_function_destroy(hoisted);
            return 1;
        }
        hoisted->env_type_storage = env_type_name;
        hoisted->env_var_storage = env_var_name;
        lm_trans_text_assign_cstr(hoisted->env_var_name, env_var_name);
        lm_trans_text_assign_cstr(hoisted->function->env_type_name, env_type_name);
        hoisted->function->captures = hoisted->captures;
        hoisted->function->has_env = 1;
    }

    if (lm_own_ptr_stack_push(hoisted_functions, hoisted) != 0) {
        lm_trans_hoisted_function_destroy(hoisted);
        return 1;
    }

    return 0;
}

static int lm_trans_collect_hoisted_functions_from_frame(
    LmOwnPtrStack *hoisted_functions,
    const LmP0Text *parent_c_name,
    LmOwnPtrStack *candidates,
    const LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    LmTransFunctionHeader *function;
    LmTransCapture *ignored_capture;
    size_t scope_base;
    int status;

    function = lm_trans_statement_function_header_new();
    ignored_capture = lm_trans_statement_capture_new();
    if (function == 0 || ignored_capture == 0) {
        lm_own_delete(function, 0);
        lm_trans_capture_destroy(ignored_capture);
        return 1;
    }

    if (frame != 0 && lm_trans_capture_stack_find(candidates, frame->head) == 0) {
        status = lm_trans_function_header_from_frame(frame, 0, function);
        if (status < 0) {
            lm_own_delete(function, 0);
            lm_trans_capture_destroy(ignored_capture);
            return 1;
        }
        if (status > 0) {
            status = lm_trans_collect_hoisted_function(
                hoisted_functions,
                parent_c_name,
                candidates,
                namespace_,
                frame,
                function
            );
            lm_own_delete(function, 0);
            lm_trans_capture_destroy(ignored_capture);
            return status;
        }
    }

    if (lm_trans_capture_from_decl_frame(frame, namespace_, ignored_capture)) {
        status = lm_trans_capture_candidates_add_decl(candidates, frame, namespace_);
        lm_own_delete(function, 0);
        lm_trans_capture_destroy(ignored_capture);
        return status;
    }

    scope_base = candidates != 0 ? candidates->count : 0U;
    status = lm_trans_collect_hoisted_functions_from_fields(
        hoisted_functions,
        parent_c_name,
        candidates,
        namespace_,
        frame != 0 ? frame->body->first_field : 0
    );
    lm_own_ptr_stack_truncate(candidates, scope_base);
    lm_own_delete(function, 0);
    lm_trans_capture_destroy(ignored_capture);
    return status;
}

static int lm_trans_collect_hoisted_functions_from_node(
    LmOwnPtrStack *hoisted_functions,
    const LmP0Text *parent_c_name,
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
            node->as->frame
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
                node->as->structure->first_field
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
        if (hoisted != 0 && hoisted->function->frame == frame) {
            return hoisted;
        }
    }

    return 0;
}


static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1);
static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2);
static int lm_trans_emit_l1_os_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_emit_l1_ifdef_frame(FILE *output, const LmP0Frame *frame);
static int lm_trans_emit_l2_os_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_define_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_ifndef_default_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l2_ifdef_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
);
static int lm_trans_emit_l4_define_table(
    FILE *file,
    const LmTransNamespace *namespace_
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
static int lm_trans_top_level_declare_merge_named_structure(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
);
static int lm_trans_top_level_emit_merge_named_structure_typedef(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
);
static int lm_trans_top_level_emit_merge_named_structure_storage(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
);
static int lm_trans_emit_function(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
);
static const LmP0Field *lm_trans_first_active_field(const LmP0Structure *structure);

static void lm_trans_top_level_item_destroy(LmTransTopLevelItem *item) {
    if (item != 0) {
        lm_trans_function_header_destroy(item->function);
        item->function = 0;
    }
}

static LmTransTopLevelItem *lm_trans_top_level_item_new(void) {
    return (LmTransTopLevelItem *)lm_own_new_zero(sizeof(LmTransTopLevelItem));
}

static void lm_trans_top_level_item_delete(LmTransTopLevelItem *item) {
    lm_trans_top_level_item_destroy(item);
    lm_own_delete(item, 0);
}

static LmTransHeadBinding *lm_trans_head_binding_new(void) {
    return (LmTransHeadBinding *)lm_own_new_zero(sizeof(LmTransHeadBinding));
}

static LmP0Structure *lm_trans_p0_structure_view_new(void) {
    return (LmP0Structure *)lm_own_new_zero(sizeof(LmP0Structure));
}

static int lm_trans_top_level_declare_function(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    char *return_type_name_storage;
    LmP0Text *return_type_name;

    if (item == 0 || item->function == 0) {
        return 1;
    }

    if (lm_trans_registry_note_class_kind(item->function->name, item->function->symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_declare(namespace_, item->function->name, item->function->symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_set_callable_shape(namespace_, item->function) != 0) {
        return 1;
    }
    if (item->function->is_struct_return) {
        return_type_name_storage = lm_trans_function_return_struct_type_name_new(item->function->c_name);
        if (return_type_name_storage == 0) {
            return 1;
        }
        return_type_name = lm_trans_text_from_cstr(return_type_name_storage);
        if (return_type_name == 0) {
            lm_own_delete(return_type_name_storage, 0);
            return 1;
        }
        if (lm_trans_namespace_declare_generated(namespace_, return_type_name, "class") != 0) {
            lm_trans_text_ref_destroy(&return_type_name);
            lm_own_delete(return_type_name_storage, 0);
            return 1;
        }
        lm_trans_text_ref_destroy(&return_type_name);
        lm_own_delete(return_type_name_storage, 0);
    }
    return 0;
}

static int lm_trans_top_level_declare_function_compatible(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    char *return_type_name_storage;
    LmP0Text *return_type_name;

    if (item == 0 || item->function == 0) {
        return 1;
    }

    if (lm_trans_registry_note_class_kind(item->function->name, item->function->symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_declare_compatible(namespace_, item->function->name, item->function->symbol_class) != 0) {
        return 1;
    }
    if (lm_trans_namespace_set_callable_shape(namespace_, item->function) != 0) {
        return 1;
    }
    if (item->function->is_struct_return) {
        return_type_name_storage = lm_trans_function_return_struct_type_name_new(item->function->c_name);
        if (return_type_name_storage == 0) {
            return 1;
        }
        return_type_name = lm_trans_text_from_cstr(return_type_name_storage);
        if (return_type_name == 0) {
            lm_own_delete(return_type_name_storage, 0);
            return 1;
        }
        if (lm_trans_namespace_declare_generated(namespace_, return_type_name, "class") != 0) {
            lm_trans_text_ref_destroy(&return_type_name);
            lm_own_delete(return_type_name_storage, 0);
            return 1;
        }
        lm_trans_text_ref_destroy(&return_type_name);
        lm_own_delete(return_type_name_storage, 0);
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

static int lm_trans_top_level_emit_l2(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)namespace_;
    return lm_trans_emit_l2_frame(file, item->frame);
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

static int lm_trans_top_level_emit_table(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_statement_emit_table(file, 0, item != 0 ? item->frame : 0, 0U, namespace_);
}

static int lm_trans_top_level_emit_join(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_statement_emit_join(file, 0, item != 0 ? item->frame : 0, 0U, namespace_);
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

static int lm_trans_top_level_emit_define(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_define_frame(file, item != 0 ? item->frame : 0, namespace_);
}

static int lm_trans_top_level_emit_ifndef_default(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    return lm_trans_emit_l2_ifndef_default_frame(file, item != 0 ? item->frame : 0, namespace_);
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

static int lm_trans_top_level_emit_atom_define(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    (void)item;
    return lm_trans_emit_l4_define_table(file, namespace_);
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
    return lm_trans_emit_atom_statement_sequence(file, item->node->as->atom, 0U, namespace_);
}

static int lm_trans_top_level_atom_binding(
    const LmP0Text *atom,
    LmTransTopLevelItem *out
) {
    const char *binding;
    LmTransBinding *resolved;
    LmTransAtomStatementHandler handler;

    if (out == 0) {
        return -1;
    }

    binding = lm_trans_registry_lookup(atom, "receiver.atom.statement");
    if (binding == 0) {
        return 0;
    }

    resolved = lm_trans_statement_binding_new();
    if (resolved == 0) {
        return -1;
    }
    if (!lm_trans_binding_resolve(binding, resolved) || resolved->atom_statement == 0) {
        fprintf(stderr, "trans registry error: unknown top-level atom binding %s\n", binding);
        lm_own_delete(resolved, 0);
        return -1;
    }
    handler = resolved->atom_statement;
    lm_own_delete(resolved, 0);

    if (handler == lm_trans_atom_statement_emit_include_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_include;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_os_prelude) {
        out->declare = lm_trans_top_level_declare_atom_os;
        out->emit_before_functions = lm_trans_top_level_emit_atom_os;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_layout_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_layout;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_constant_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_constant;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_define_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_define;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_alias_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_alias;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_forward_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_forward;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_prototype_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_prototype;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_fn_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_fn;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_guard_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_guard;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_extern_c_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_extern_c;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_unit_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_atom_unit;
        out->emits_top_level = 1;
        return 1;
    }
    if (handler == lm_trans_atom_statement_emit_sequence_prelude) {
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
    LmTransStatementFrameHandler receiver;

    if (binding == 0 || out == 0 || binding->statement_frame == 0) {
        return 0;
    }

    receiver = binding->statement_frame;
    if (receiver == 0) {
        return 0;
    }

    if (receiver == lm_trans_statement_emit_include_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_include;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_define_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_define;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_ifndef_default_prelude) {
        out->emit_before_functions = lm_trans_top_level_emit_ifndef_default;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_os_prelude) {
        out->declare = lm_trans_top_level_declare_os;
        out->emit_before_functions = lm_trans_top_level_emit_os;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_ifdef_prelude) {
        out->declare = lm_trans_top_level_declare_ifdef;
        out->emit_before_functions = lm_trans_top_level_emit_ifdef;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_guard_prelude) {
        out->declare = lm_trans_top_level_declare_guard;
        out->emit_before_functions = lm_trans_top_level_emit_guard;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_extern_c_prelude) {
        out->declare = lm_trans_top_level_declare_extern_c;
        out->emit_before_functions = lm_trans_top_level_emit_extern_c;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_import) {
        out->declare = lm_trans_top_level_declare_import;
        if (lm_trans_import_frame_has_code_path(out->frame)) {
            out->emit_before_functions = lm_trans_top_level_emit_import_prelude;
            out->emit_function = lm_trans_top_level_emit_import_functions;
        }
        return 1;
    }
    if (receiver == lm_trans_statement_emit_table) {
        out->emit_after_prototypes = lm_trans_top_level_emit_table;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_join) {
        out->emit_after_prototypes = lm_trans_top_level_emit_join;
        out->emits_top_level = 1;
        return 1;
    }
    if (receiver == lm_trans_statement_emit_merge_named_structure) {
        out->declare = lm_trans_top_level_declare_merge_named_structure;
        out->emit_before_functions = lm_trans_top_level_emit_merge_named_structure_typedef;
        out->emit_after_prototypes = lm_trans_top_level_emit_merge_named_structure_storage;
        out->emits_top_level = 1;
        return 1;
    }

    return 0;
}

static int lm_trans_frame_is_l1_level_receiver(const LmP0Frame *frame);
static int lm_trans_frame_is_l2_level_receiver(const LmP0Frame *frame);
static const char *lm_trans_level_receiver_binding_from_frame(const LmP0Frame *frame);

static int lm_trans_top_level_level_binding(
    const LmTransHeadBinding *binding,
    LmTransTopLevelItem *out
) {
    const char *receiver;

    if (
        binding == 0 ||
        out == 0 ||
        binding->receiver_type == 0 ||
        strcmp(binding->receiver_type, "receiver.level") != 0
    ) {
        return 0;
    }

    receiver = lm_trans_level_receiver_binding_from_frame(out->frame);
    if (receiver == 0) {
        fprintf(stderr, "trans registry error: receiver.level has no binding for top-level frame\n");
        return -1;
    }
    if (lm_trans_frame_is_l1_level_receiver(out->frame)) {
        out->emit_after_prototypes = lm_trans_top_level_emit_l1;
        out->emits_top_level = 1;
        return 1;
    }
    if (lm_trans_frame_is_l2_level_receiver(out->frame)) {
        out->emit_after_prototypes = lm_trans_top_level_emit_l2;
        out->emits_top_level = 1;
        return 1;
    }

    fprintf(stderr, "trans registry error: unknown receiver.level binding %s\n", receiver);
    return -1;
}

static const char *lm_trans_level_receiver_binding_from_head(const LmP0Text *head) {
    const char *receiver_type;

    if (head == 0) {
        return 0;
    }

    receiver_type = lm_trans_registry_lookup(head, "namespace");
    if (receiver_type == 0 || strcmp(receiver_type, "receiver.level") != 0) {
        return 0;
    }
    return lm_trans_registry_lookup(head, "receiver.level");
}

static const char *lm_trans_level_receiver_binding_from_frame(const LmP0Frame *frame) {
    return frame != 0 ? lm_trans_level_receiver_binding_from_head(frame->head) : 0;
}

static int lm_trans_frame_has_level_receiver_binding(
    const LmP0Frame *frame,
    const char *binding
) {
    const char *actual;

    if (binding == 0) {
        return 0;
    }

    actual = lm_trans_level_receiver_binding_from_frame(frame);
    return actual != 0 && strcmp(actual, binding) == 0;
}

static int lm_trans_frame_is_l1_level_receiver(const LmP0Frame *frame) {
    return lm_trans_frame_has_level_receiver_binding(frame, "lm_trans_emit_l1_frame");
}

static int lm_trans_frame_is_l2_level_receiver(const LmP0Frame *frame) {
    return lm_trans_frame_has_level_receiver_binding(frame, "lm_trans_emit_l2_frame");
}

static const LmP0Text *lm_trans_end_frame_target(const LmP0Frame *frame) {
    const char *receiver_type;
    const LmP0Field *field;

    if (frame == 0 || frame->head == 0 || frame->body == 0) {
        return 0;
    }

    receiver_type = lm_trans_registry_lookup(frame->head, "namespace");
    if (
        receiver_type == 0 ||
        strcmp(receiver_type, "receiver.trailer") != 0 ||
        lm_trans_registry_lookup(frame->head, "receiver.trailer") == 0
    ) {
        return 0;
    }

    field = frame->body->first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    return field->value->as->atom;
}

static int lm_trans_end_frame_targets_level_receiver_binding(
    const LmP0Frame *frame,
    const char *binding
) {
    const LmP0Text *target;
    const char *actual;

    if (binding == 0) {
        return 0;
    }

    target = lm_trans_end_frame_target(frame);
    actual = lm_trans_level_receiver_binding_from_head(target);
    return actual != 0 && strcmp(actual, binding) == 0;
}

static int lm_trans_end_frame_targets_l1_level_receiver(const LmP0Frame *frame) {
    return lm_trans_end_frame_targets_level_receiver_binding(frame, "lm_trans_emit_l1_frame");
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
    field = params->as->structure->first_field;
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
    LmP0Text *descriptor_name;
    char *descriptor_name_storage;

    descriptor_name = lm_trans_text_from_cstr("");
    if (
        item == 0 ||
        item->frame == 0 ||
        item->function == 0 ||
        descriptor_name == 0 ||
        !lm_trans_registry_identifier_value(item->function->name, descriptor_name)
    ) {
        lm_trans_text_ref_destroy(&descriptor_name);
        return 1;
    }

    descriptor_name_storage = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_name_storage == 0) {
        lm_trans_text_ref_destroy(&descriptor_name);
        return 1;
    }

    if (lm_trans_registry_materialize_fn_descriptor_frame(item->frame) != 0) {
        lm_trans_text_ref_destroy(&descriptor_name);
        lm_own_delete(descriptor_name_storage, 0);
        return 1;
    }

    if (lm_trans_callable_descriptor_is_lazy(namespace_, descriptor_name)) {
        status = lm_trans_emit_callable_descriptor(file, item->function, namespace_);
    } else if (lm_trans_l4_is_function_pointer_type(namespace_, descriptor_name_storage)) {
        status = lm_trans_emit_l4_function_pointer_type_name(
            file,
            namespace_,
            descriptor_name_storage,
            "functionPointerType"
        );
    } else {
        status = 0;
    }
    lm_trans_text_ref_destroy(&descriptor_name);
    lm_own_delete(descriptor_name_storage, 0);
    if (status != 0) {
        return 1;
    }
    if (
        item->function->is_external &&
        lm_trans_validate_end_trailer(item->frame) != 0
    ) {
        return 1;
    }
    return 0;
}

static int lm_trans_function_raw_return_descriptor_name(
    const LmTransFunctionHeader *function,
    const LmTransNamespace *namespace_,
    LmP0Text *out_descriptor_name
) {
    if (
        function == 0 ||
        function->is_sub ||
        function->is_struct_return ||
        function->return_node == 0 ||
        function->return_node->kind == LM_P0_NODE_STRUCTURE ||
        out_descriptor_name == 0 ||
        !lm_trans_node_callable_descriptor_name(function->return_node, namespace_, out_descriptor_name) ||
        lm_trans_registry_is_function_pointer_type_name(namespace_, out_descriptor_name) ||
        !lm_trans_callable_descriptor_is_raw_function_reference(namespace_, out_descriptor_name)
    ) {
        return 0;
    }
    return 1;
}

static int lm_trans_emit_raw_callable_return_prefix(
    FILE *file,
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name
) {
    char *descriptor_cstr;
    int status;

    if (file == 0 || descriptor_name == 0) {
        return 1;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 1;
    }
    status =
        lm_trans_emit_abi_return_type(
            file,
            namespace_,
            descriptor_cstr,
            "callable return descriptor"
        ) != 0 ||
        lm_trans_put(file, " (*") != 0;
    lm_own_delete(descriptor_cstr, 0);
    return status;
}

static int lm_trans_emit_raw_callable_return_suffix(
    FILE *file,
    const LmTransNamespace *namespace_,
    const LmP0Text *descriptor_name
) {
    LmTransAbiParam **params;
    size_t param_capacity;
    size_t param_count;
    char *descriptor_cstr;
    int status;

    if (file == 0 || descriptor_name == 0) {
        return 1;
    }

    descriptor_cstr = lm_trans_text_copy_cstr(descriptor_name);
    if (descriptor_cstr == 0) {
        return 1;
    }
    param_capacity = 256U;
    params = lm_trans_expr_abi_params_new(param_capacity);
    if (params == 0) {
        lm_own_delete(descriptor_cstr, 0);
        return 1;
    }

    status =
        lm_trans_collect_abi_params(
            params,
            param_capacity,
            &param_count,
            namespace_,
            descriptor_cstr,
            "callable return descriptor"
        ) != 0 ||
        lm_trans_put(file, "))(") != 0 ||
        (
            param_count == 0U
                ? lm_trans_put(file, "void")
                : lm_trans_emit_abi_params(file, params, param_count)
        ) != 0 ||
        lm_trans_put(file, ")") != 0;

    lm_trans_expr_abi_params_delete(params, param_capacity);
    lm_own_delete(descriptor_cstr, 0);
    return status;
}

static int lm_trans_emit_function_prototype(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    int status;
    int raw_return;
    LmP0Text *raw_return_descriptor_name;

    if (function == 0 || function->is_descriptor_only) {
        return 0;
    }

    raw_return_descriptor_name = lm_trans_text_ref_new_cstr("");
    if (raw_return_descriptor_name == 0) {
        return 1;
    }
    raw_return = lm_trans_function_raw_return_descriptor_name(
        function,
        namespace_,
        raw_return_descriptor_name
    );

    if (function->is_sub) {
        if (!function->is_external && lm_trans_put(file, "static ") != 0) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            return 1;
        }
        if (lm_trans_put(file, "void ") != 0) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            return 1;
        }
    } else {
        if (function->return_node == 0) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            return 0;
        }
        if (
            function->is_struct_return &&
            lm_trans_emit_function_return_structure_once(file, function->c_name, function->return_node) != 0
        ) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            return 1;
        }
        if (!function->is_external && lm_trans_put(file, "static ") != 0) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            return 1;
        }
        if (function->is_struct_return) {
            if (
                lm_trans_emit_function_return_struct_type_name(file, function->c_name) != 0 ||
                lm_trans_put(file, " ") != 0
            ) {
                lm_trans_text_ref_destroy(&raw_return_descriptor_name);
                return 1;
            }
        } else if (raw_return) {
            if (lm_trans_emit_raw_callable_return_prefix(file, namespace_, raw_return_descriptor_name) != 0) {
                lm_trans_text_ref_destroy(&raw_return_descriptor_name);
                return 1;
            }
        } else {
            if (function->return_node->kind == LM_P0_NODE_STRUCTURE) {
                lm_trans_text_ref_destroy(&raw_return_descriptor_name);
                return 0;
            }
            if (
                lm_trans_emit_type_node(file, function->return_node) != 0 ||
                lm_trans_put(file, " ") != 0
            ) {
                lm_trans_text_ref_destroy(&raw_return_descriptor_name);
                return 1;
            }
        }
    }

    if (
        lm_trans_write_text(file, function->c_name) != 0 ||
        lm_trans_put(file, "(") != 0
    ) {
        lm_trans_text_ref_destroy(&raw_return_descriptor_name);
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status = lm_trans_emit_function_params(file, function, namespace_);
    lm_trans_namespace_leave_scope(namespace_);
    if (status != 0) {
        lm_trans_text_ref_destroy(&raw_return_descriptor_name);
        return 1;
    }

    if (raw_return && lm_trans_emit_raw_callable_return_suffix(file, namespace_, raw_return_descriptor_name) != 0) {
        lm_trans_text_ref_destroy(&raw_return_descriptor_name);
        return 1;
    }
    lm_trans_text_ref_destroy(&raw_return_descriptor_name);
    return lm_trans_put(file, raw_return ? ";\n" : ");\n");
}

static int lm_trans_top_level_emit_function_prototype(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->function == 0) {
        return 1;
    }
    return lm_trans_emit_function_prototype(file, item->function, namespace_);
}

static int lm_trans_top_level_emit_function(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    int status;

    if (item == 0 || item->function == 0) {
        return 1;
    }

    status = lm_trans_emit_function(file, item->function, namespace_);
    if (status != 0) {
        return 1;
    }
    if (
        item->function->is_external &&
        lm_trans_validate_end_trailer(item->frame) != 0
    ) {
        return 1;
    }
    return 0;
}

static const LmP0Frame *lm_trans_const_storage_inner_frame(
    const LmP0Frame *frame
) {
    const LmP0Field *field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "const")) {
        return 0;
    }

    field = frame->body != 0 ? frame->body->first_field : 0;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        return 0;
    }

    return field->value->as->frame;
}

static int lm_trans_frame_looks_top_level_storage_declaration(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Frame *inner;

    if (
        lm_trans_frame_looks_storage_declaration(frame, namespace_) ||
        (frame != 0 && lm_trans_text_all_char(frame->head, '@')) ||
        (frame != 0 && lm_trans_text_is_array_receiver_head(frame->head))
    ) {
        return 1;
    }

    inner = lm_trans_const_storage_inner_frame(frame);
    return
        lm_trans_frame_looks_storage_declaration(inner, namespace_) ||
        (inner != 0 && lm_trans_text_all_char(inner->head, '@')) ||
        (inner != 0 && lm_trans_text_is_array_receiver_head(inner->head));
}

static int lm_trans_top_level_declare_storage_declaration(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    const LmP0Frame *frame;
    const LmP0Structure *body;
    const LmP0Field *field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;
    const LmP0Frame *inner;

    if (item == 0 || item->frame == 0) {
        return 1;
    }

    frame = item->frame;
    inner = lm_trans_const_storage_inner_frame(frame);
    if (inner != 0) {
        frame = inner;
    }
    body = lm_trans_unwrap_single_anonymous_structure(frame->body);
    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        name_field = lm_trans_nth_field(body, 0U);
        name_node = name_field != 0 ? name_field->value : 0;
        if (name_node == 0 || name_node->kind != LM_P0_NODE_ATOM) {
            return 1;
        }
        return lm_trans_namespace_declare_storage_binding(namespace_, name_node->as->atom, frame->head);
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        name_field = lm_trans_nth_field(body, 1U);
        name_node = name_field != 0 ? name_field->value : 0;
        if (name_node == 0 || name_node->kind != LM_P0_NODE_ATOM) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
    }

    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        field = body != 0 ? body->first_field : 0;
        if (
            field != 0 &&
            field->next == 0 &&
            field->value != 0 &&
            field->value->kind == LM_P0_NODE_FRAME
        ) {
            inner = field->value->as->frame;
            if (lm_trans_head_can_declare_storage(inner->head, namespace_)) {
                name_field = inner->body->first_field;
            } else if (lm_trans_text_all_char(inner->head, '@')) {
                name_field = lm_trans_nth_field(inner->body, 1U);
            } else {
                return 1;
            }
        } else {
            while (
                field != 0 &&
                field->value != 0 &&
                field->value->kind == LM_P0_NODE_ATOM &&
                lm_trans_text_all_char(field->value->as->atom, '@')
            ) {
                field = field->next;
            }
            name_field = field != 0 ? field->next : 0;
        }

        name_node = name_field != 0 ? name_field->value : 0;
        if (name_node == 0 || name_node->kind != LM_P0_NODE_ATOM) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as->atom, "variable");
    }

    return 1;
}

static int lm_trans_top_level_emit_static_declaration(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    const LmP0Frame *frame;
    const LmP0Frame *inner;
    const char *qualifier;

    if (item == 0 || item->frame == 0) {
        return 1;
    }

    frame = item->frame;
    qualifier = "static ";
    inner = lm_trans_const_storage_inner_frame(frame);
    if (inner != 0) {
        frame = inner;
        qualifier = "static const ";
    }
    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        return lm_trans_emit_declaration_with_qualifier(file, frame, 0U, namespace_, qualifier);
    }
    if (lm_trans_text_all_char(frame->head, '@')) {
        return lm_trans_emit_pointer_declaration_with_qualifier(file, frame, 0U, namespace_, qualifier);
    }
    if (lm_trans_text_is_array_receiver_head(frame->head)) {
        return lm_trans_emit_array_declaration_with_qualifier(file, frame, 0U, namespace_, qualifier);
    }

    return 1;
}

static int lm_trans_top_level_declare_named_structure(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_declare_named_structure_layout(namespace_, item->frame);
}

static int lm_trans_top_level_emit_named_structure_typedef(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_emit_named_structure_typedef(file, item->frame, 0U, namespace_);
}

static int lm_trans_top_level_emit_named_structure_storage(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_emit_named_structure_storage(file, item->frame, 0U, namespace_, "static ");
}

static int lm_trans_top_level_declare_merge_named_structure(
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_declare_merge_named_structure_layout(namespace_, item->frame);
}

static int lm_trans_top_level_emit_merge_named_structure_typedef(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_emit_merge_named_structure_typedef(file, item->frame, 0U, namespace_);
}

static int lm_trans_top_level_emit_merge_named_structure_storage(
    FILE *file,
    LmTransNamespace *namespace_,
    const LmTransTopLevelItem *item
) {
    if (item == 0 || item->frame == 0) {
        return 1;
    }
    return lm_trans_emit_merge_named_structure_storage(file, item->frame, 0U, namespace_, "static ");
}

static int lm_trans_lower_top_level_item(
    const LmP0Node *node,
    LmTransTopLevelItem *out
) {
    LmTransHeadBinding *binding;
    LmTransL4HeadBinding *l4_root_binding;
    int function_status;
    int statement_status;
    int level_status;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    out->node = node;

    if (node->kind == LM_P0_NODE_ATOM) {
        function_status = lm_trans_top_level_atom_binding(node->as->atom, out);
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

    out->frame = node->as->frame;

    out->function = lm_trans_function_header_new();
    if (out->function == 0) {
        return 1;
    }
    function_status = lm_trans_top_level_function_header(out->frame, out->function);
    if (function_status < 0) {
        lm_trans_function_header_destroy(out->function);
        out->function = 0;
        return 1;
    }
    if (function_status > 0) {
        out->declare = lm_trans_top_level_declare_function;
        if (out->function->is_descriptor_only) {
            if (!out->function->is_sub) {
                out->emit_before_functions = lm_trans_top_level_emit_callable_descriptor;
                out->emits_top_level = 1;
            }
        } else {
            out->emit_prototype = lm_trans_top_level_emit_function_prototype;
            out->emit_function = lm_trans_top_level_emit_function;
        }
        return 0;
    }
    lm_trans_function_header_destroy(out->function);
    out->function = 0;

    binding = lm_trans_head_binding_new();
    if (binding == 0) {
        return 1;
    }
    if (lm_trans_head_binding_resolve(0, out->frame->head, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    statement_status = lm_trans_top_level_statement_binding(binding, out);
    if (statement_status == 0) {
        level_status = lm_trans_top_level_level_binding(binding, out);
    } else {
        level_status = 0;
    }
    lm_own_delete(binding, 0);
    if (statement_status < 0 || level_status < 0) {
        return 1;
    }
    if (statement_status != 0 || level_status != 0) {
        return 0;
    }

    l4_root_binding = (LmTransL4HeadBinding *)lm_own_new_zero(sizeof(*l4_root_binding));
    if (l4_root_binding == 0) {
        return 1;
    }
    if (lm_trans_l4_root_head_binding_resolve(out->frame->head, l4_root_binding) != 0) {
        lm_own_delete(l4_root_binding, 0);
        return 1;
    }
    if (l4_root_binding->frame != 0) {
        lm_own_delete(l4_root_binding, 0);
        out->emit_before_functions = lm_trans_top_level_emit_registry;
        return 0;
    }
    lm_own_delete(l4_root_binding, 0);

    if (lm_trans_frame_looks_top_level_storage_declaration(out->frame, 0)) {
        out->declare = lm_trans_top_level_declare_storage_declaration;
        out->emit_after_prototypes = lm_trans_top_level_emit_static_declaration;
        out->emits_top_level = 1;
        return 0;
    }

    if (lm_trans_frame_looks_named_structure_layout_declaration(out->frame, 0)) {
        out->declare = lm_trans_top_level_declare_named_structure;
        out->emit_before_functions = lm_trans_top_level_emit_named_structure_typedef;
        out->emit_after_prototypes = lm_trans_top_level_emit_named_structure_storage;
        out->emits_top_level = 1;
        return 0;
    }

    fprintf(stderr, "trans L2 error: top-level L2 field must be fn, sub, external fn/sub, registered top-level statement frame, named structure, registered level receiver, or registered L4 root receiver data\n");
    return 1;
}

static LmTransTopLevelItem *lm_trans_top_level_item_lowered_new(
    const LmP0Node *node
) {
    LmTransTopLevelItem *item;

    item = lm_trans_top_level_item_new();
    if (item == 0) {
        return 0;
    }
    if (lm_trans_lower_top_level_item(node, item) != 0) {
        lm_trans_top_level_item_delete(item);
        return 0;
    }
    return item;
}

static int lm_trans_emit_function_with_hoisted(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_,
    LmOwnPtrStack *hoisted_functions
) {
    const LmP0Frame *frame;
    LmTransFunctionState *state;
    int status;
    LmP0Text *env_text;
    int raw_return;
    LmP0Text *raw_return_descriptor_name;

    if (function == 0 || function->frame == 0 || hoisted_functions == 0) {
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

    frame = function->frame;
    state = 0;
    raw_return = 0;
    raw_return_descriptor_name = 0;

    if (function->is_descriptor_only) {
        fprintf(stderr, "trans L2 error: descriptor-only fn does not have an executable body\n");
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

    if (lm_trans_emit_env_type(file, function, namespace_) != 0) {
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

    if (function->is_sub) {
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
        }
        if (lm_trans_put(file, "void ") != 0) {
            lm_trans_ptr_stack_delete(&hoisted_functions);
            return 1;
        }
    } else {
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_structure_once(file, function->c_name, function->return_node) != 0) {
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
        } else {
            if (function->return_node->kind == LM_P0_NODE_STRUCTURE) {
                fprintf(stderr, "trans L2 error: fn expects a single-value return type; use fm for Structure return\n");
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
        }
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
        }
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_struct_type_name(file, function->c_name) != 0) {
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
        } else {
            raw_return_descriptor_name = lm_trans_text_ref_new_cstr("");
            if (raw_return_descriptor_name == 0) {
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            raw_return = lm_trans_function_raw_return_descriptor_name(
                function,
                namespace_,
                raw_return_descriptor_name
            );
            if (
                raw_return
                    ? lm_trans_emit_raw_callable_return_prefix(file, namespace_, raw_return_descriptor_name)
                    : lm_trans_emit_type_node(file, function->return_node)
            ) {
                lm_trans_text_ref_destroy(&raw_return_descriptor_name);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
        }
        if (!raw_return && lm_trans_put(file, " ") != 0) {
            lm_trans_ptr_stack_delete(&hoisted_functions);
            return 1;
        }
    }

    if (lm_trans_write_text(file, function->c_name) != 0) {
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        lm_trans_ptr_stack_delete(&hoisted_functions);
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
            lm_trans_ptr_stack_delete(&hoisted_functions);
            return 1;
        }
        if (function->has_env) {
            env_text = lm_trans_text_from_cstr("lm_env");
            if (env_text == 0) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            if (
                lm_trans_namespace_set_env_arg(
                    namespace_,
                    function->name,
                    env_text
                ) != 0
            ) {
                lm_trans_text_ref_destroy(&env_text);
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            lm_trans_text_ref_destroy(&env_text);
        }
    }
    if (function->has_env && function->captures != 0) {
        size_t capture_index;
        const LmTransCapture *capture;
        char *capture_expr;
        LmP0Text *capture_text;

        for (capture_index = 0U; capture_index < function->captures->count; ++capture_index) {
            capture = (const LmTransCapture *)lm_own_ptr_stack_at(function->captures, capture_index);
            if (capture == 0) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            capture_expr = lm_trans_captured_expr_new(capture->name);
            if (capture_expr == 0) {
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            capture_text = lm_trans_text_from_cstr(capture_expr);
            if (capture_text == 0) {
                lm_own_delete(capture_expr, 0);
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            if (
                lm_trans_namespace_declare_c_name(
                    namespace_,
                    capture->name,
                    "variable",
                    capture_text
                ) != 0
            ) {
                lm_trans_text_ref_destroy(&capture_text);
                lm_own_delete(capture_expr, 0);
                lm_trans_namespace_leave_scope(namespace_);
                lm_trans_ptr_stack_delete(&hoisted_functions);
                return 1;
            }
            lm_trans_text_ref_destroy(&capture_text);
            lm_own_delete(capture_expr, 0);
        }
    }
    if (lm_trans_emit_function_params(file, function, namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }
    if (raw_return) {
        raw_return_descriptor_name = lm_trans_text_ref_new_cstr("");
        if (raw_return_descriptor_name == 0) {
            lm_trans_namespace_leave_scope(namespace_);
            lm_trans_ptr_stack_delete(&hoisted_functions);
            return 1;
        }
        if (
            !lm_trans_function_raw_return_descriptor_name(
                function,
                namespace_,
                raw_return_descriptor_name
            ) ||
            lm_trans_emit_raw_callable_return_suffix(file, namespace_, raw_return_descriptor_name) != 0
        ) {
            lm_trans_text_ref_destroy(&raw_return_descriptor_name);
            lm_trans_namespace_leave_scope(namespace_);
            lm_trans_ptr_stack_delete(&hoisted_functions);
            return 1;
        }
        lm_trans_text_ref_destroy(&raw_return_descriptor_name);
    }
    if (lm_trans_put(file, raw_return ? " {\n" : ") {\n") != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

    state = lm_trans_function_state_new();
    if (state == 0) {
        lm_trans_namespace_leave_scope(namespace_);
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

    if (function->is_struct_return) {
        state->current_return_type_name = lm_trans_text_ref_new(function->c_name);
        if (state->current_return_type_name == 0) {
            lm_trans_function_state_delete(state);
            lm_trans_namespace_leave_scope(namespace_);
            lm_trans_ptr_stack_delete(&hoisted_functions);
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
    namespace_->hoisted_functions = hoisted_functions;
    namespace_->cleanups = lm_trans_ptr_stack_new(lm_trans_cleanup_delete_any);
    namespace_->loops = lm_trans_ptr_stack_new(lm_trans_loop_delete_any);
    if (namespace_->cleanups == 0 || namespace_->loops == 0) {
        lm_trans_ptr_stack_delete(&namespace_->cleanups);
        lm_trans_ptr_stack_delete(&namespace_->loops);
        namespace_->cleanups = state->previous_cleanups;
        namespace_->loops = state->previous_loops;
        state->previous_cleanups = 0;
        state->previous_loops = 0;
        state->has_previous_control_stacks = 0;
        lm_trans_function_state_delete(state);
        lm_trans_namespace_leave_scope(namespace_);
        lm_trans_ptr_stack_delete(&hoisted_functions);
        return 1;
    }

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
    lm_trans_ptr_stack_delete(&namespace_->cleanups);
    lm_trans_ptr_stack_delete(&namespace_->loops);
    namespace_->cleanups = state->previous_cleanups;
    namespace_->loops = state->previous_loops;
    state->previous_cleanups = 0;
    state->previous_loops = 0;
    state->has_previous_control_stacks = 0;
    lm_trans_function_state_delete(state);
    lm_trans_namespace_leave_scope(namespace_);
    lm_trans_ptr_stack_delete(&hoisted_functions);
    return status;
}

typedef struct LmTransFunctionEmitFrame {
    const LmTransFunctionHeader *function;
    LmOwnPtrStack *hoisted_functions;
    size_t hoisted_index;
    int phase;
} LmTransFunctionEmitFrame;

static void lm_trans_function_emit_frame_delete_any(void *item) {
    LmTransFunctionEmitFrame *frame;

    frame = (LmTransFunctionEmitFrame *)item;
    if (frame != 0) {
        lm_trans_ptr_stack_delete(&frame->hoisted_functions);
    }
    lm_own_delete(frame, 0);
}

static LmTransFunctionEmitFrame *lm_trans_function_emit_frame_new(
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    LmTransFunctionEmitFrame *frame;
    LmOwnPtrStack *capture_candidates;

    if (function == 0 || function->frame == 0) {
        return 0;
    }
    if (function->is_descriptor_only) {
        fprintf(stderr, "trans L2 error: descriptor-only fn does not have an executable body\n");
        return 0;
    }

    frame = (LmTransFunctionEmitFrame *)lm_own_new_zero(sizeof(*frame));
    if (frame == 0) {
        return 0;
    }
    frame->function = function;
    frame->phase = 0;

    frame->hoisted_functions = lm_trans_ptr_stack_new(lm_trans_hoisted_function_delete_any);
    if (frame->hoisted_functions == 0) {
        lm_trans_function_emit_frame_delete_any(frame);
        return 0;
    }
    capture_candidates = lm_trans_ptr_stack_new(lm_trans_capture_delete_any);
    if (capture_candidates == 0) {
        lm_trans_function_emit_frame_delete_any(frame);
        return 0;
    }
    if (
        lm_trans_capture_candidates_add_captures(capture_candidates, function->captures) != 0 ||
        lm_trans_capture_candidates_add_params(capture_candidates, function->params_node, namespace_) != 0
    ) {
        lm_trans_ptr_stack_delete(&capture_candidates);
        lm_trans_function_emit_frame_delete_any(frame);
        return 0;
    }
    if (
        lm_trans_collect_hoisted_functions_from_fields(
            frame->hoisted_functions,
            function->c_name,
            capture_candidates,
            namespace_,
            function->body_start
        ) != 0
    ) {
        lm_trans_ptr_stack_delete(&capture_candidates);
        lm_trans_function_emit_frame_delete_any(frame);
        return 0;
    }
    lm_trans_ptr_stack_delete(&capture_candidates);
    return frame;
}

static int lm_trans_emit_function(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    LmOwnPtrStack *stack;
    LmTransFunctionEmitFrame *frame;
    LmTransFunctionEmitFrame *child;
    LmTransHoistedFunction *hoisted;
    LmOwnPtrStack *hoisted_functions;
    int status;

    stack = lm_trans_ptr_stack_new(lm_trans_function_emit_frame_delete_any);
    if (stack == 0) {
        return 1;
    }
    frame = lm_trans_function_emit_frame_new(function, namespace_);
    if (frame == 0) {
        lm_trans_ptr_stack_delete(&stack);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_trans_function_emit_frame_delete_any(frame);
        lm_trans_ptr_stack_delete(&stack);
        return 1;
    }

    status = 0;
    while (status == 0 && stack->count != 0U) {
        frame = (LmTransFunctionEmitFrame *)lm_own_ptr_stack_top(stack);
        if (frame == 0) {
            status = 1;
            break;
        }

        if (frame->phase == 0) {
            if (frame->hoisted_index < frame->hoisted_functions->count) {
                hoisted = (LmTransHoistedFunction *)lm_own_ptr_stack_at(frame->hoisted_functions, frame->hoisted_index);
                if (hoisted == 0 || hoisted->function == 0) {
                    status = 1;
                    break;
                }
                child = lm_trans_function_emit_frame_new(hoisted->function, namespace_);
                if (child == 0) {
                    status = 1;
                    break;
                }
                frame->phase = 1;
                if (lm_own_ptr_stack_push(stack, child) != 0) {
                    lm_trans_function_emit_frame_delete_any(child);
                    status = 1;
                    break;
                }
                continue;
            }
            frame->phase = 2;
        }

        if (frame->phase == 1) {
            hoisted = (LmTransHoistedFunction *)lm_own_ptr_stack_at(frame->hoisted_functions, frame->hoisted_index);
            if (hoisted == 0) {
                status = 1;
                break;
            }
            if (
                lm_trans_put(file, "\n") != 0 ||
                lm_trans_emit_closure_call_wrapper(file, hoisted, namespace_) != 0 ||
                lm_trans_put(file, "\n") != 0
            ) {
                status = 1;
                break;
            }
            ++frame->hoisted_index;
            frame->phase = 0;
            continue;
        }

        if (frame->phase == 2) {
            hoisted_functions = frame->hoisted_functions;
            frame->hoisted_functions = 0;
            status = lm_trans_emit_function_with_hoisted(file, frame->function, namespace_, hoisted_functions);
            lm_trans_function_emit_frame_delete_any(lm_own_ptr_stack_pop(stack));
            continue;
        }

        status = 1;
    }

    lm_trans_ptr_stack_delete(&stack);
    return status;
}

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
static int lm_trans_emit_l4_payload_import_document(
    FILE *output,
    const char *payload_path,
    const LmP0Node *root,
    LmTransNamespace *namespace_
);
static int lm_trans_root_has_explicit_l2_frame(const LmP0Structure *root);
static int lm_trans_string_stack_has(const LmOwnPtrStack *stack, const char *value);
static int lm_trans_declare_l2_structure_import(
    LmTransNamespace *namespace_,
    const LmP0Structure *body
);
static int lm_trans_declare_l4_fn_descriptors(
    LmTransNamespace *namespace_
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
    LmTransTopLevelItem *item;
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
            item = lm_trans_top_level_item_lowered_new(node);
            if (item == 0) {
                return 1;
            }

            if (item->declare != 0 && item->declare(namespace_, item) != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            lm_trans_top_level_item_delete(item);

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
    LmTransTopLevelItem *item;
    int status;
    int emitted_prelude;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prelude = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        item = lm_trans_top_level_item_lowered_new(node);
        if (item == 0) {
            return 1;
        }

        if (item->emit_before_functions != 0) {
            if (emitted_prelude && lm_trans_put(file, "\n") != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            status = item->emit_before_functions(file, namespace_, item);
        } else {
            status = 0;
        }

        if (status != 0) {
            lm_trans_top_level_item_delete(item);
            return 1;
        }
        if (item->emits_top_level) {
            emitted_prelude = 1;
        }
        lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int status;
    int emitted_prototype;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prototype = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        item = lm_trans_top_level_item_lowered_new(node);
        if (item == 0) {
            return 1;
        }

        if (item->emit_prototype != 0) {
            status = item->emit_prototype(file, namespace_, item);
            if (status != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            emitted_prototype = 1;
        }
        lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int status;
    int emitted_prelude;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_prelude = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        item = lm_trans_top_level_item_lowered_new(node);
        if (item == 0) {
            return 1;
        }

        if (item->emit_after_prototypes != 0) {
            if (emitted_prelude && lm_trans_put(file, "\n") != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            status = item->emit_after_prototypes(file, namespace_, item);
        } else {
            status = 0;
        }

        if (status != 0) {
            lm_trans_top_level_item_delete(item);
            return 1;
        }
        if (item->emits_top_level && item->emit_after_prototypes != 0) {
            emitted_prelude = 1;
        }
        lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int status;
    int emitted_function;

    if (namespace_ == 0) {
        return 1;
    }

    emitted_function = 0;
    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        item = lm_trans_top_level_item_lowered_new(node);
        if (item == 0) {
            return 1;
        }

        if (item->emit_function != 0) {
            if (emitted_function && lm_trans_put(file, "\n") != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            status = item->emit_function(file, namespace_, item);
            if (status != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
            emitted_function = 1;
        }
        lm_trans_top_level_item_delete(item);
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

    return lm_trans_emit_l2_structure(file, l2->body, l2);
}

static const char *lm_trans_import_source_path(void) {
    if (lm_trans_current_source_path != 0) {
        return lm_trans_current_source_path;
    }
    return lm_trans_registry != 0 ? lm_trans_registry->source_path : 0;
}

static int lm_trans_import_stack_note(LmOwnPtrStack *stack, const char *path) {
    char *copy;

    if (stack == 0 || path == 0) {
        return 1;
    }
    if (lm_trans_string_stack_has(stack, path)) {
        return 0;
    }

    copy = lm_trans_text_copy_cstr_from_cstr(path);
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, copy) != 0) {
        lm_own_delete(copy, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_import_text_has_extension(const LmP0Text *path, const char *extension) {
    size_t extension_length;

    if (path == 0 || extension == 0) {
        return 0;
    }
    extension_length = strlen(extension);
    if (path->length < extension_length) {
        return 0;
    }
    return memcmp(path->data + path->length - extension_length, extension, extension_length) == 0;
}

static int lm_trans_import_frame_has_code_path(const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text *path_value;
    int result;

    path_value = lm_trans_text_from_cstr("");
    if (path_value == 0) {
        return 1;
    }
    result = 0;
    field = frame != 0 ? frame->body->first_field : 0;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as->atom, path_value)
            ) {
                result = 1;
                break;
            }
            if (!lm_trans_import_text_has_extension(path_value, ".lm4")) {
                result = 1;
                break;
            }
        }
        field = field->next;
    }
    lm_trans_text_ref_destroy(&path_value);
    return result;
}

static int lm_trans_import_resolve_node(
    const LmP0Node *node,
    char *path,
    size_t path_size,
    const char *error_name
) {
    LmP0Text *path_value;
    char *candidate_name;
    int status;

    path_value = lm_trans_text_from_cstr("");
    if (path_value == 0) {
        return 1;
    }
    if (
        node == 0 ||
        node->kind != LM_P0_NODE_ATOM ||
        !lm_trans_registry_literal_value(node->as->atom, path_value)
    ) {
        fprintf(stderr, "trans L2 import error: import path must be an atom/string\n");
        lm_trans_text_ref_destroy(&path_value);
        return 1;
    }

    candidate_name = lm_trans_text_copy_cstr(path_value);
    lm_trans_text_ref_destroy(&path_value);
    if (candidate_name == 0) {
        return 1;
    }
    status = lm_trans_registry_candidate_path(
        lm_trans_import_source_path(),
        candidate_name,
        path,
        path_size
    );
    lm_own_delete(candidate_name, 0);
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
    LmTransTopLevelItem *item;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        return lm_trans_declare_l2_structure_import(namespace_, root->as->structure);
    }

    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame)
            ) {
                if (lm_trans_declare_l2_structure_import(namespace_, node->as->frame->body) != 0) {
                    return 1;
                }
            } else {
                item = lm_trans_top_level_item_lowered_new(node);
                if (item == 0) {
                    return 1;
                }
                if (item->declare != 0 && item->declare(namespace_, item) != 0) {
                    lm_trans_top_level_item_delete(item);
                    return 1;
                }
                lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        return lm_trans_emit_l2_structure_prelude(file, root->as->structure, namespace_);
    }

    emitted = 0;
    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame)
            ) {
                status = lm_trans_emit_l2_structure_prelude(file, node->as->frame->body, namespace_);
            } else {
                item = lm_trans_top_level_item_lowered_new(node);
                if (item == 0) {
                    return 1;
                }
                if (item->emit_before_functions != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        lm_trans_top_level_item_delete(item);
                        return 1;
                    }
                    status = item->emit_before_functions(file, namespace_, item);
                } else {
                    status = 0;
                }
                if (item->emits_top_level) {
                    emitted = 1;
                }
                lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        return lm_trans_emit_l2_structure_prototypes(file, root->as->structure, namespace_);
    }

    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame)
            ) {
                status = lm_trans_emit_l2_structure_prototypes(file, node->as->frame->body, namespace_);
            } else {
                item = lm_trans_top_level_item_lowered_new(node);
                if (item == 0) {
                    return 1;
                }
                status = item->emit_prototype != 0
                    ? item->emit_prototype(file, namespace_, item)
                    : 0;
                lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        return lm_trans_emit_l2_structure_after_prototypes(file, root->as->structure, namespace_);
    }

    emitted = 0;
    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame)
            ) {
                status = lm_trans_emit_l2_structure_after_prototypes(file, node->as->frame->body, namespace_);
            } else {
                item = lm_trans_top_level_item_lowered_new(node);
                if (item == 0) {
                    return 1;
                }
                if (item->emit_after_prototypes != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        lm_trans_top_level_item_delete(item);
                        return 1;
                    }
                    status = item->emit_after_prototypes(file, namespace_, item);
                    emitted = 1;
                } else {
                    status = 0;
                }
                lm_trans_top_level_item_delete(item);
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
    LmTransTopLevelItem *item;
    int emitted;
    int status;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        return lm_trans_emit_l2_structure_functions(file, root->as->structure, namespace_);
    }

    emitted = 0;
    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame)
            ) {
                if (emitted && lm_trans_put(file, "\n") != 0) {
                    return 1;
                }
                status = lm_trans_emit_l2_structure_functions(file, node->as->frame->body, namespace_);
                if (status == 0) {
                    status = lm_trans_validate_end_trailer(node->as->frame);
                }
                emitted = 1;
            } else {
                item = lm_trans_top_level_item_lowered_new(node);
                if (item == 0) {
                    return 1;
                }
                if (item->emit_function != 0) {
                    if (emitted && lm_trans_put(file, "\n") != 0) {
                        lm_trans_top_level_item_delete(item);
                        return 1;
                    }
                    status = item->emit_function(file, namespace_, item);
                    emitted = 1;
                } else {
                    status = 0;
                }
                lm_trans_top_level_item_delete(item);
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
        if (lm_trans_string_stack_has(lm_trans_declared_import_paths, path)) {
            return 0;
        }
        if (lm_trans_import_stack_note(lm_trans_declared_import_paths, path) != 0) {
            return 1;
        }
        if (lm_trans_registry_load_file_path(path, 1, &loaded) != 0) {
            return 1;
        }
        return lm_trans_declare_l4_fn_descriptors(namespace_);
    }
    if (lm_trans_string_stack_has(lm_trans_declared_import_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(lm_trans_declared_import_paths, path) != 0) {
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
    if (lm_own_ptr_stack_push(lm_trans_declared_import_documents, document) != 0) {
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
    if (lm_trans_string_stack_has(lm_trans_emitted_import_prelude_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(lm_trans_emitted_import_prelude_paths, path) != 0) {
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
    if (lm_trans_string_stack_has(lm_trans_emitted_import_function_paths, path)) {
        return 0;
    }
    if (lm_trans_import_stack_note(lm_trans_emitted_import_function_paths, path) != 0) {
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
    field = frame->body->first_field;
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
    field = frame->body->first_field;
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
    field = frame->body->first_field;
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

static int lm_trans_inline_string_payload(const LmP0Text *text, LmP0Text *out_payload, size_t *out_run) {
    char quote;
    size_t open_run;
    size_t close_run;

    if (text == 0 || out_payload == 0 || out_run == 0 || text->length < 2U) {
        return 0;
    }

    quote = text->data[0];
    if (quote != '\'' && quote != '"') {
        return 0;
    }

    open_run = 0U;
    while (open_run < text->length && text->data[open_run] == quote) {
        ++open_run;
    }
    if (text->length < open_run * 2U) {
        return 0;
    }

    close_run = 0U;
    while (close_run < text->length && text->data[text->length - close_run - 1U] == quote) {
        ++close_run;
    }
    if (close_run < open_run) {
        return 0;
    }

    out_payload->data = text->data + open_run;
    out_payload->length = text->length - open_run * 2U;
    *out_run = open_run;
    return 1;
}

static int lm_trans_emit_l1_payload(FILE *output, const LmP0Text *text) {
    LmP0Text *payload;
    size_t delimiter_run;
    size_t i;
    char ch;
    int status;

    payload = lm_trans_text_from_cstr("");
    if (payload == 0) {
        return 1;
    }
    if (lm_trans_inline_string_payload(text, payload, &delimiter_run)) {
        if (delimiter_run == 1U) {
            status = 0;
            i = 0U;
            while (i < payload->length) {
                ch = payload->data[i++];
                if (ch == '\\' && i < payload->length) {
                    ch = payload->data[i++];
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
                    status = 1;
                    break;
                }
            }
            lm_trans_text_ref_destroy(&payload);
            return status;
        }

        status = lm_trans_write_all(output, payload->data, payload->length);
        lm_trans_text_ref_destroy(&payload);
        return status;
    }

    lm_trans_text_ref_destroy(&payload);
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

static const char *lm_trans_os_condition(const LmP0Text *name) {
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
    const LmP0Text *name,
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

static int lm_trans_emit_preprocessor_atom_token(
    FILE *output,
    const LmP0Node *node,
    const char *receiver_name
) {
    LmP0Text *payload;
    int status;

    if (node == 0 || node->kind != LM_P0_NODE_ATOM) {
        fprintf(
            stderr,
            "trans preprocessor error: %s receiver expects atom tokens\n",
            receiver_name != 0 ? receiver_name : "preprocessor"
        );
        return 1;
    }
    payload = lm_trans_text_from_cstr("");
    if (payload == 0) {
        return 1;
    }
    if (!lm_trans_identifier_payload(node->as->atom, payload)) {
        fprintf(
            stderr,
            "trans preprocessor error: %s receiver cannot read atom token\n",
            receiver_name != 0 ? receiver_name : "preprocessor"
        );
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }
    status = lm_trans_write_text(output, payload);
    lm_trans_text_ref_destroy(&payload);
    return status;
}

static int lm_trans_emit_preprocessor_define_frame(
    FILE *output,
    const LmP0Frame *frame,
    int ifndef_default,
    const char *receiver_name
) {
    const LmP0Field *field;
    LmP0Text *name;
    int status;

    if (output == 0 || frame == 0 || frame->body == 0) {
        return 1;
    }

    field = lm_trans_first_active_field(frame->body);
    if (field == 0 || field->value == 0 || field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(
            stderr,
            "trans preprocessor error: %s receiver expects macro name as first atom\n",
            receiver_name != 0 ? receiver_name : "define"
        );
        return 1;
    }
    name = lm_trans_text_from_cstr("");
    if (name == 0) {
        return 1;
    }
    if (!lm_trans_registry_identifier_value(field->value->as->atom, name) || name->length == 0U) {
        fprintf(
            stderr,
            "trans preprocessor error: %s receiver expects macro name as identifier atom\n",
            receiver_name != 0 ? receiver_name : "define"
        );
        status = 1;
        goto cleanup;
    }

    if (ifndef_default) {
        if (
            lm_trans_put(output, "#ifndef ") != 0 ||
            lm_trans_write_text(output, name) != 0 ||
            lm_trans_put(output, "\n") != 0
        ) {
            status = 1;
            goto cleanup;
        }
    }

    if (
        lm_trans_put(output, "#define ") != 0 ||
        lm_trans_write_text(output, name) != 0
    ) {
        status = 1;
        goto cleanup;
    }

    field = field->next;
    while (field != 0) {
        if (!lm_trans_node_is_ignored(field->value)) {
            if (
                lm_trans_put(output, " ") != 0 ||
                lm_trans_emit_preprocessor_atom_token(output, field->value, receiver_name) != 0
            ) {
                status = 1;
                goto cleanup;
            }
        }
        field = field->next;
    }

    if (lm_trans_put(output, "\n") != 0) {
        status = 1;
        goto cleanup;
    }

    if (ifndef_default && lm_trans_put(output, "#endif\n") != 0) {
        status = 1;
        goto cleanup;
    }

    status = lm_trans_validate_end_trailer(frame);

cleanup:
    lm_trans_text_ref_destroy(&name);
    return status;
}

static int lm_trans_emit_l2_define_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    (void)namespace_;
    return lm_trans_emit_preprocessor_define_frame(output, frame, 0, "define");
}

static int lm_trans_emit_l2_ifndef_default_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    (void)namespace_;
    return lm_trans_emit_preprocessor_define_frame(output, frame, 1, "ifndef-default");
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
                    !lm_trans_frame_is_l1_level_receiver(node->as->frame) &&
                    !lm_trans_text_equals(node->as->frame->head, "include") &&
                    !lm_trans_text_equals(node->as->frame->head, "os") &&
                    !lm_trans_text_equals(node->as->frame->head, "ifdef") &&
                    !lm_trans_end_frame_targets_l1_level_receiver(node->as->frame)
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
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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

            if (lm_trans_os_branch_looks_l2(branch->body)) {
                if (lm_trans_declare_l2_structure_import(namespace_, branch->body) != 0) {
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
    LmP0Text *condition;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    emitted_default = 0;
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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
                condition = lm_trans_text_from_cstr("");
                if (condition == 0) {
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, condition) != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                lm_trans_text_ref_destroy(&condition);
            }

            if (lm_trans_os_branch_looks_l2(branch->body)) {
                if (lm_trans_declare_l2_structure_import(namespace_, branch->body) != 0) {
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
    LmTransTopLevelItem *item;

    field = body != 0 ? body->first_field : 0;
    while (field != 0) {
        node = field->value;
        item = lm_trans_top_level_item_lowered_new(node);
        if (item == 0) {
            return 1;
        }

        if (item->function != 0 && item->function->frame != 0) {
            if (lm_trans_top_level_declare_function_compatible(namespace_, item) != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
        } else if (item->declare != 0) {
            if (item->declare(namespace_, item) != 0) {
                lm_trans_top_level_item_delete(item);
                return 1;
            }
        }
        lm_trans_top_level_item_delete(item);

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
    if (lm_trans_os_branch_looks_l2(branch->body)) {
        return lm_trans_emit_l2_structure_with_namespace(output, branch->body, 0, namespace_, 0);
    }
    return lm_trans_emit_l1_structure(output, branch->body);
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
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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
    LmP0Text *condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L2 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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
                condition = lm_trans_text_from_cstr("");
                if (condition == 0) {
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, condition) != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                if (lm_trans_write_text(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                lm_trans_text_ref_destroy(&condition);
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

    name_field = lm_trans_first_active_field(frame->body);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: guard receiver expects guard name as first atom\n");
        return 1;
    }

    *out_name = *name_field->value->as->atom;
    *out_body = *frame->body;
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
            *out_body = *body_field->value->as->structure;
        }
    }
    return 0;
}

static int lm_trans_declare_l2_guard_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    LmP0Text *name;
    LmP0Structure *body;

    name = lm_trans_text_from_cstr("");
    body = lm_trans_p0_structure_view_new();
    if (name == 0 || body == 0) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    if (lm_trans_guard_body_from_frame(frame, name, body) != 0) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    (void)name;
    if (lm_trans_declare_l2_wrapper_body(namespace_, body) != 0) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    lm_trans_text_ref_destroy(&name);
    lm_own_delete(body, 0);
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l2_guard_frame(
    FILE *output,
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    LmP0Text *name;
    LmP0Structure *body;
    int status;

    name = lm_trans_text_from_cstr("");
    body = lm_trans_p0_structure_view_new();
    if (name == 0 || body == 0) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    if (lm_trans_guard_body_from_frame(frame, name, body) != 0) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    if (
        lm_trans_put(output, "#ifndef ") != 0 ||
        lm_trans_emit_identifier(output, name) != 0 ||
        lm_trans_put(output, "\n#define ") != 0 ||
        lm_trans_emit_identifier(output, name) != 0 ||
        lm_trans_put(output, "\n") != 0
    ) {
        lm_trans_text_ref_destroy(&name);
        lm_own_delete(body, 0);
        return 1;
    }
    status = 0;
    if (lm_trans_emit_l2_wrapper_body(output, body, namespace_) != 0) {
        status = 1;
    } else if (lm_trans_put(output, "#endif\n") != 0) {
        status = 1;
    }
    if (status == 0 && lm_trans_validate_end_trailer(frame) != 0) {
        status = 1;
    }
    lm_trans_text_ref_destroy(&name);
    lm_own_delete(body, 0);
    return status;
}

static int lm_trans_declare_l2_extern_c_frame(
    LmTransNamespace *namespace_,
    const LmP0Frame *frame
) {
    if (frame == 0) {
        return 1;
    }
    if (lm_trans_declare_l2_wrapper_body(namespace_, frame->body) != 0) {
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
    if (lm_trans_emit_l2_wrapper_body(output, frame->body, namespace_) != 0) {
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
    if (node->kind == LM_P0_NODE_FRAME && lm_trans_frame_is_l2_level_receiver(node->as->frame)) {
        if (lm_trans_declare_l2_structure_import(namespace_, node->as->frame->body) != 0) {
            return 1;
        }
        return lm_trans_validate_end_trailer(node->as->frame);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_os_branch_looks_l2(node->as->structure)) {
        return lm_trans_declare_l2_structure_import(namespace_, node->as->structure);
    }
    return 0;
}

static int lm_trans_declare_l2_registry_os_table(LmTransNamespace *namespace_) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    LmP0Text *branch_name;
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
    branch_name = lm_trans_text_from_cstr("");
    if (branch_name == 0) {
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload_node != 0
        ) {
            lm_trans_text_assign_cstr(branch_name, row->key);
            if (lm_trans_text_equals(branch_name, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os table has duplicate default branch\n");
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                emitted_default = 1;
            } else {
                condition = lm_trans_os_condition(branch_name);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: registry os table has unknown branch %s\n", row->key);
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os default branch must be last\n");
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
            }
            if (lm_trans_declare_l2_registry_os_node(namespace_, row->payload_node) != 0) {
                lm_trans_text_ref_destroy(&branch_name);
                return 1;
            }
            emitted = 1;
        }
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        lm_trans_text_ref_destroy(&branch_name);
        return 1;
    }
    lm_trans_text_ref_destroy(&branch_name);
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
    if (node->kind == LM_P0_NODE_FRAME && lm_trans_frame_is_l2_level_receiver(node->as->frame)) {
        return lm_trans_emit_l2_structure_with_namespace(output, node->as->frame->body, node->as->frame, namespace_, 0);
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_os_branch_looks_l2(node->as->structure)) {
        return lm_trans_emit_l2_structure_with_namespace(output, node->as->structure, 0, namespace_, 0);
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
                    lm_trans_frame_is_l1_level_receiver(node->as->frame) ||
                    lm_trans_frame_is_l2_level_receiver(node->as->frame) ||
                    lm_trans_text_equals(node->as->frame->head, "include") ||
                    lm_trans_text_equals(node->as->frame->head, "os") ||
                    lm_trans_text_equals(node->as->frame->head, "ifdef") ||
                    lm_trans_end_frame_targets_l1_level_receiver(node->as->frame)
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
    LmP0Text *path_value;
    char *candidate_name;
    char payload_path[4096];
    int emitted;
    int status;

    if (frame == 0) {
        return 1;
    }

    path_value = lm_trans_text_from_cstr("");
    if (path_value == 0) {
        return 1;
    }
    field = frame->body->first_field;
    emitted = 0;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node == 0 ||
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as->atom, path_value)
            ) {
                fprintf(stderr, "trans L4 unit error: unit.payload import path must be an atom/string\n");
                lm_trans_text_ref_destroy(&path_value);
                return 1;
            }

            candidate_name = lm_trans_text_copy_cstr(path_value);
            if (candidate_name == 0) {
                lm_trans_text_ref_destroy(&path_value);
                return 1;
            }
            if (
                lm_trans_registry_candidate_path(
                    lm_trans_registry->source_path,
                    candidate_name,
                    payload_path,
                    sizeof(payload_path)
            ) != 0
            ) {
                lm_own_delete(candidate_name, 0);
                fprintf(stderr, "trans L4 unit error: unit.payload import path is too long\n");
                lm_trans_text_ref_destroy(&path_value);
                return 1;
            }
            lm_own_delete(candidate_name, 0);

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
                lm_trans_text_ref_destroy(&path_value);
                return 1;
            }

            if (
                lm_trans_emit_l4_payload_import_document(
                    output,
                    payload_path,
                    lm_p0_document_root(document),
                    namespace_
                ) != 0
            ) {
                lm_p0_document_destroy(document);
                lm_trans_text_ref_destroy(&path_value);
                return 1;
            }
            lm_p0_document_destroy(document);
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L4 unit error: unit.payload import receiver expects at least one path\n");
        lm_trans_text_ref_destroy(&path_value);
        return 1;
    }
    lm_trans_text_ref_destroy(&path_value);
    return 0;
}

static int lm_trans_emit_l4_payload_import_document(
    FILE *output,
    const char *payload_path,
    const LmP0Node *root,
    LmTransNamespace *namespace_
) {
    const char *previous_source_path;
    FILE *prelude_file;
    int loaded;
    int status;

    if (output == 0 || payload_path == 0 || root == 0 || namespace_ == 0) {
        return 1;
    }

    if (!lm_trans_path_has_extension(payload_path, ".lm2")) {
        return lm_trans_emit_l4_payload_node(output, root, namespace_);
    }

    if (lm_trans_registry_load_file_path(payload_path, 1, &loaded) != 0) {
        return 1;
    }

    previous_source_path = lm_trans_current_source_path;
    lm_trans_current_source_path = payload_path;

    status = lm_trans_declare_l2_import_root(
        namespace_,
        root,
        1
    );
    prelude_file = lm_trans_prelude_file(output);
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_prelude(
            prelude_file,
            root,
            1,
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_prototypes(
            prelude_file,
            root,
            1,
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_after_prototypes(
            prelude_file,
            root,
            1,
            namespace_
        );
    }
    if (status == 0) {
        status = lm_trans_emit_l2_import_root_functions(
            output,
            root,
            1,
            namespace_
        );
    }

    lm_trans_current_source_path = previous_source_path;
    return status;
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
        ? lm_trans_emit_l2_structure_with_namespace(output, frame->body, frame, namespace_, 0)
        : 1;
}

typedef struct LmTransL4PayloadPointerBinding {
    const char *head;
    const char *receiver_type;
    LmTransL4PayloadFrameHandler handler;
} LmTransL4PayloadPointerBinding;

static int lm_trans_l4_payload_pointer_binding_push(
    LmOwnPtrStack *bindings,
    const char *head,
    const char *receiver_type,
    LmTransL4PayloadFrameHandler handler
) {
    LmTransL4PayloadPointerBinding *binding;

    if (bindings == 0 || head == 0 || receiver_type == 0 || handler == 0) {
        return 1;
    }
    binding = (LmTransL4PayloadPointerBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    binding->head = head;
    binding->receiver_type = receiver_type;
    binding->handler = handler;
    if (lm_own_ptr_stack_push(bindings, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_l4_payload_pointer_bindings_init(void) {
    LmOwnPtrStack *bindings;

    if (lm_trans_registry == 0) {
        return 1;
    }
    if (lm_trans_registry->l4_payload_pointer_bindings != 0) {
        return 0;
    }

    bindings = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (bindings == 0) {
        return 1;
    }
    if (
        lm_trans_l4_payload_pointer_binding_push(bindings, "import", "l4.payload", lm_trans_l4_payload_receiver_import) != 0 ||
        lm_trans_l4_payload_pointer_binding_push(bindings, "L2", "l4.payload", lm_trans_l4_payload_receiver_l2) != 0
    ) {
        lm_trans_ptr_stack_delete(&bindings);
        return 1;
    }
    lm_trans_registry->l4_payload_pointer_bindings = bindings;
    return 0;
}

static void lm_trans_l4_payload_pointer_bindings_destroy(void) {
    if (lm_trans_registry != 0) {
        lm_trans_ptr_stack_delete(&lm_trans_registry->l4_payload_pointer_bindings);
    }
}

static const LmTransL4PayloadPointerBinding *lm_trans_l4_payload_pointer_binding_find(
    const LmP0Text *head
) {
    size_t i;
    const LmOwnPtrStack *bindings;
    const LmTransL4PayloadPointerBinding *binding;

    if (head == 0 || lm_trans_registry == 0 || lm_trans_registry->l4_payload_pointer_bindings == 0) {
        return 0;
    }

    bindings = lm_trans_registry->l4_payload_pointer_bindings;
    for (i = 0U; i < bindings->count; ++i) {
        binding = (const LmTransL4PayloadPointerBinding *)lm_own_ptr_stack_at(bindings, i);
        if (binding != 0 && lm_trans_text_equals(head, binding->head)) {
            return binding;
        }
    }
    return 0;
}

static int lm_trans_l4_payload_frame_handler_resolve(
    const LmP0Text *head,
    LmTransL4PayloadFrameHandler *out
) {
    const char *receiver_type;
    const LmTransL4PayloadPointerBinding *binding;

    if (head == 0 || out == 0) {
        return 1;
    }
    *out = 0;

    receiver_type = lm_trans_registry_lookup(head, "namespace.l4.payload");
    if (receiver_type == 0) {
        return 0;
    }

    binding = lm_trans_l4_payload_pointer_binding_find(head);
    if (binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: namespace.l4.payload[\"%.*s\"] has no direct L4 payload pointer binding\n",
            (int)head->length,
            head->data
        );
        return 1;
    }

    if (strcmp(receiver_type, binding->receiver_type) != 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: namespace.l4.payload[\"%.*s\"] has receiver marker %s, expected %s\n",
            (int)head->length,
            head->data,
            receiver_type,
            binding->receiver_type
        );
        return 1;
    }

    *out = binding->handler;
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
        if (lm_trans_l4_payload_frame_handler_resolve(node->as->frame->head, &frame_handler) != 0) {
            return 1;
        }
        if (frame_handler != 0) {
            return frame_handler(output, node->as->frame, namespace_);
        }
    }
    if (node->kind == LM_P0_NODE_STRUCTURE && lm_trans_l4_payload_structure_looks_bare_l2(node->as->structure)) {
        return lm_trans_emit_l2_structure_with_namespace(output, node->as->structure, 0, namespace_, 0);
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
    LmP0Text *branch_name;
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
    branch_name = lm_trans_text_from_cstr("");
    if (branch_name == 0) {
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (
            row != 0 &&
            row->key != 0 &&
            row->payload_node != 0
        ) {
            lm_trans_text_assign_cstr(branch_name, row->key);
            if (lm_trans_text_equals(branch_name, "default")) {
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os table has duplicate default branch\n");
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                emitted_default = 1;
                if (opened && lm_trans_put(output, "#else\n") != 0) {
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
            } else {
                condition = lm_trans_os_condition(branch_name);
                if (condition == 0) {
                    fprintf(stderr, "trans L2 error: registry os table has unknown branch %s\n", row->key);
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                if (emitted_default) {
                    fprintf(stderr, "trans L2 error: registry os default branch must be last\n");
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                if (lm_trans_put(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    lm_trans_text_ref_destroy(&branch_name);
                    return 1;
                }
                opened = 1;
            }

            if (lm_trans_emit_l2_registry_os_node(output, row->payload_node, namespace_) != 0) {
                lm_trans_text_ref_destroy(&branch_name);
                return 1;
            }
            emitted = 1;
        }
    }

    if (!emitted) {
        fprintf(stderr, "trans L2 error: atom os receiver expects table os with node body cells\n");
        lm_trans_text_ref_destroy(&branch_name);
        return 1;
    }
    if (opened && lm_trans_put(output, "#endif\n") != 0) {
        lm_trans_text_ref_destroy(&branch_name);
        return 1;
    }
    lm_trans_text_ref_destroy(&branch_name);
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
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L1 error: os receiver expects win/mac/unix/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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

            if (lm_trans_emit_l1_structure(output, branch->body) != 0) {
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
    LmP0Text *condition;
    int opened;
    int emitted_default;

    if (frame == 0) {
        return 1;
    }

    opened = 0;
    emitted_default = 0;
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L1 error: ifdef receiver expects condition/default frames\n");
                return 1;
            }

            branch = node->as->frame;
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
                condition = lm_trans_text_from_cstr("");
                if (condition == 0) {
                    return 1;
                }
                if (lm_trans_ifdef_condition_payload(branch->head, condition) != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                if (lm_trans_put(output, opened ? "#elif " : "#if ") != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                if (lm_trans_write_text(output, condition) != 0 || lm_trans_put(output, "\n") != 0) {
                    lm_trans_text_ref_destroy(&condition);
                    return 1;
                }
                lm_trans_text_ref_destroy(&condition);
                opened = 1;
            }

            if (lm_trans_emit_l1_structure(output, branch->body) != 0) {
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
    LmP0Text *payload;
    size_t delimiter_run;
    int emitted;

    if (frame == 0) {
        return 1;
    }

    payload = lm_trans_text_from_cstr("");
    if (payload == 0) {
        return 1;
    }
    emitted = 0;
    field = frame->body->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_inline_string_payload(node->as->atom, payload, &delimiter_run) ||
                payload->length == 0U
            ) {
                fprintf(stderr, "trans L1 error: include receiver expects string header atoms\n");
                lm_trans_text_ref_destroy(&payload);
                return 1;
            }
            (void)delimiter_run;
            if (lm_trans_emit_l1_include_target_text(output, payload->data, payload->length) != 0) {
                lm_trans_text_ref_destroy(&payload);
                return 1;
            }
            emitted = 1;
        }
        field = field->next;
    }

    if (!emitted) {
        fprintf(stderr, "trans L1 error: include receiver expects at least one header\n");
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }

    lm_trans_text_ref_destroy(&payload);
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node) {
    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        if (lm_trans_emit_l1_payload(output, node->as->atom) != 0) {
            return 1;
        }
        return lm_trans_write_all(output, "\n", 1U);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_emit_l1_structure(output, node->as->structure);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if (lm_trans_frame_is_l1_level_receiver(node->as->frame)) {
            return lm_trans_emit_l1_frame(output, node->as->frame);
        }
        if (lm_trans_frame_is_l2_level_receiver(node->as->frame)) {
            return lm_trans_emit_l2_frame(output, node->as->frame);
        }
        if (lm_trans_text_equals(node->as->frame->head, "os")) {
            return lm_trans_emit_l1_os_frame(output, node->as->frame);
        }
        if (lm_trans_text_equals(node->as->frame->head, "ifdef")) {
            return lm_trans_emit_l1_ifdef_frame(output, node->as->frame);
        }
        if (lm_trans_text_equals(node->as->frame->head, "include")) {
            return lm_trans_emit_l1_include_frame(output, node->as->frame);
        }
        if (lm_trans_end_frame_targets_l1_level_receiver(node->as->frame)) {
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

    if (lm_trans_emit_l1_structure(output, l1->body) != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(l1);
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
            lm_trans_frame_is_l2_level_receiver(node->as->frame)
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
    LmTransL4HeadBinding *l4_root_binding;
    int has_l4_root;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }
    prelude_file = lm_trans_prelude_file(output);

    if (implicit_l2 && !lm_trans_root_has_explicit_l2_frame(root->as->structure)) {
        if (lm_trans_emit_l2_structure(output, root->as->structure, 0) != 0) {
            return 1;
        }
        if (emitted != 0) {
            *emitted = 1;
        }
        return 0;
    }

    field = root->as->structure->first_field;
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
            l4_root_binding = (LmTransL4HeadBinding *)lm_own_new_zero(sizeof(*l4_root_binding));
            if (l4_root_binding == 0) {
                return 1;
            }
            if (lm_trans_l4_root_head_binding_resolve(node->as->frame->head, l4_root_binding) != 0) {
                lm_own_delete(l4_root_binding, 0);
                return 1;
            }
            has_l4_root = l4_root_binding->frame != 0;
            lm_own_delete(l4_root_binding, 0);
            if (lm_trans_frame_is_l1_level_receiver(node->as->frame)) {
                if (lm_trans_emit_l1_body(prelude_file, node->as->frame, emitted) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_frame_is_l2_level_receiver(node->as->frame)) {
                if (lm_trans_emit_l2_frame(output, node->as->frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (has_l4_root) {
            } else if (lm_trans_text_equals(node->as->frame->head, "os")) {
                if (lm_trans_emit_l1_os_frame(prelude_file, node->as->frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_text_equals(node->as->frame->head, "ifdef")) {
                if (lm_trans_emit_l1_ifdef_frame(prelude_file, node->as->frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_text_equals(node->as->frame->head, "include")) {
                if (lm_trans_emit_l1_include_frame(prelude_file, node->as->frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (lm_trans_end_frame_targets_l1_level_receiver(node->as->frame)) {
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

static LmP0Text *lm_trans_l4_text_view_new(const char *text) {
    return lm_trans_text_from_cstr(text != 0 ? text : "");
}

static void lm_trans_l4_text_view_delete(LmP0Text **view) {
    lm_trans_text_ref_destroy(view);
}

static int lm_trans_layout_class_requires_pointer_field(
    const LmTransNamespace *namespace_,
    const char *class_name
) {
    LmP0Text *class_text;
    const char *backend;
    int result;

    if (namespace_ == 0 || class_name == 0) {
        return 0;
    }

    class_text = lm_trans_l4_text_view_new(class_name);
    if (class_text == 0) {
        return 0;
    }
    backend = lm_trans_namespace_registry_lookup(namespace_, class_text, "layout.backend");
    result = lm_trans_layout_backend_is_supported(backend);
    lm_trans_l4_text_view_delete(&class_text);
    return result;
}

static size_t lm_trans_layout_field_effective_address_depth(
    const LmTransLayoutField *field,
    const LmTransNamespace *namespace_
) {
    if (field == 0 || field->is_union || field->address_depth != 0U) {
        return field != 0 ? field->address_depth : 0U;
    }
    if (lm_trans_layout_class_requires_pointer_field(namespace_, field->class_name)) {
        return 1U;
    }
    return 0U;
}

static LmTransAbiParam **lm_trans_l4_abi_params_new(size_t capacity) {
    LmTransAbiParam **params;
    size_t i;

    if (capacity == 0U) {
        return 0;
    }
    params = (LmTransAbiParam **)lm_own_new_zero(sizeof(params[0]) * capacity);
    if (params == 0) {
        return 0;
    }
    for (i = 0U; i < capacity; ++i) {
        params[i] = (LmTransAbiParam *)lm_own_new_zero(sizeof(params[i][0]));
        if (params[i] == 0) {
            while (i != 0U) {
                --i;
                lm_own_delete(params[i], 0);
            }
            lm_own_delete(params, 0);
            return 0;
        }
    }
    return params;
}

static void lm_trans_l4_abi_params_delete(LmTransAbiParam **params, size_t capacity) {
    size_t i;

    if (params == 0) {
        return;
    }
    for (i = 0U; i < capacity; ++i) {
        lm_own_delete(params[i], 0);
    }
    lm_own_delete(params, 0);
}

static LmTransLayoutField **lm_trans_layout_fields_new(size_t capacity) {
    LmTransLayoutField **fields;
    size_t i;

    if (capacity == 0U) {
        return 0;
    }
    fields = (LmTransLayoutField **)lm_own_new_zero(sizeof(fields[0]) * capacity);
    if (fields == 0) {
        return 0;
    }
    for (i = 0U; i < capacity; ++i) {
        fields[i] = (LmTransLayoutField *)lm_own_new_zero(sizeof(fields[i][0]));
        if (fields[i] == 0) {
            while (i != 0U) {
                --i;
                lm_own_delete(fields[i], 0);
            }
            lm_own_delete(fields, 0);
            return 0;
        }
    }
    return fields;
}

static void lm_trans_layout_fields_delete(LmTransLayoutField **fields, size_t capacity) {
    size_t i;

    if (fields == 0) {
        return;
    }
    for (i = 0U; i < capacity; ++i) {
        lm_own_delete(fields[i], 0);
    }
    lm_own_delete(fields, 0);
}

static int lm_trans_registry_collect_layout_names(
    LmOwnPtrStack *names,
    const LmTransNamespace *namespace_
) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    char *name;
    LmP0Text *key_text;

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
    key_text = lm_trans_l4_text_view_new("");
    if (key_text == 0) {
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
            lm_trans_text_assign_cstr(key_text, row->key);
            if (lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0) {
                fprintf(stderr, "trans L4 layout error: layout.backend key %s is not a class\n", row->key);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                lm_own_delete(name, 0);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                lm_own_delete(name, 0);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
        }
    }
    lm_trans_l4_text_view_delete(&key_text);
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
    LmP0Text *field_text;

    if (layout_name == 0 || field_name == 0 || field == 0) {
        return 1;
    }

    field_text = lm_trans_l4_text_view_new(field_name);
    if (field_text == 0) {
        return 1;
    }
    index_row = lm_trans_registry_relation_stack_latest_row(index_rows, field_text);
    address_depth_row = lm_trans_registry_relation_stack_latest_row(address_depth_rows, field_text);
    const_row = lm_trans_registry_relation_stack_latest_row(const_rows, field_text);
    array_count_row = lm_trans_registry_relation_stack_latest_row(array_count_rows, field_text);

    if (index_row == 0 || !lm_trans_parse_size_payload(index_row->payload, &index)) {
        fprintf(stderr, "trans L4 layout error: field %s.%s requires field.index\n", layout_name, field_name);
        lm_trans_l4_text_view_delete(&field_text);
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
        lm_trans_l4_text_view_delete(&field_text);
        return 1;
    }
    if (const_row != 0) {
        if (!lm_trans_parse_size_payload(const_row->payload, &index)) {
            fprintf(stderr, "trans L4 layout error: field %s.%s has invalid field.const\n", layout_name, field_name);
            lm_trans_l4_text_view_delete(&field_text);
            return 1;
        }
        field->is_const = index != 0U;
    }
    if (array_count_row != 0) {
        if (!lm_trans_parse_size_payload(array_count_row->payload, &field->array_count)) {
            fprintf(stderr, "trans L4 layout error: field %s.%s has invalid field.array-count\n", layout_name, field_name);
            lm_trans_l4_text_view_delete(&field_text);
            return 1;
        }
        field->has_array_count = 1;
    }
    lm_trans_l4_text_view_delete(&field_text);
    return 0;
}

static int lm_trans_collect_layout_fields(
    const char *layout_name,
    const LmTransNamespace *namespace_,
    LmTransLayoutField **fields,
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
    LmP0Text *layout_text;

    if (layout_name == 0 || fields == 0 || out_count == 0) {
        return 1;
    }

    layout_text = lm_trans_l4_text_view_new(layout_name);
    if (layout_text == 0) {
        return 1;
    }
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.class");
    union_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.union");
    index_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.index");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.const");
    array_count_rows = lm_trans_namespace_registry_relation_stack(namespace_, layout_text, "field.array-count");

    if (class_rows == 0 && union_rows == 0) {
        fprintf(stderr, "trans L4 layout error: layout %s requires field.class or field.union facts\n", layout_name);
        lm_trans_l4_text_view_delete(&layout_text);
        return 1;
    }

    field_count = 0U;
    if (class_rows != 0) {
        for (i = 0U; i < class_rows->count; ++i) {
            row = (LmTransRegistryFact *)lm_own_ptr_stack_at(class_rows, i);
            if (row != 0 && row->key != 0 && row->payload != 0) {
                if (field_count >= capacity) {
                    fprintf(stderr, "trans L4 layout error: too many fields in %s\n", layout_name);
                    lm_trans_l4_text_view_delete(&layout_text);
                    return 1;
                }
                field = fields[field_count];
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
                    lm_trans_l4_text_view_delete(&layout_text);
                    return 1;
                }
                field->class_name = row->payload;
                if (
                    field->is_const &&
                    lm_trans_layout_field_effective_address_depth(field, namespace_) == 0U &&
                    !field->has_array_count
                ) {
                    fprintf(stderr, "trans L4 layout error: field %s.%s const value fields are not supported yet\n", layout_name, row->key);
                    lm_trans_l4_text_view_delete(&layout_text);
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
                    lm_trans_l4_text_view_delete(&layout_text);
                    return 1;
                }
                field = fields[field_count];
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
                    lm_trans_l4_text_view_delete(&layout_text);
                    return 1;
                }
                field->is_union = 1;
                field->union_layout_name = row->payload;
                ++field_count;
            }
        }
    }

    *out_count = field_count;
    lm_trans_l4_text_view_delete(&layout_text);
    return 0;
}

static int lm_trans_sort_layout_fields(
    LmTransLayoutField **fields,
    size_t field_count
) {
    size_t i;
    size_t j;
    LmTransLayoutField *swap;

    if (fields == 0) {
        return 1;
    }

    for (i = 0U; i < field_count; ++i) {
        for (j = i + 1U; j < field_count; ++j) {
            if (fields[j]->index < fields[i]->index) {
                swap = fields[i];
                fields[i] = fields[j];
                fields[j] = swap;
            }
        }
    }
    return 0;
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
    LmTransCDeclarator *declarator;
    size_t effective_address_depth;

    if (file == 0 || field == 0) {
        return 1;
    }
    declarator = 0;
    effective_address_depth = lm_trans_layout_field_effective_address_depth(field, namespace_);

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
        if (field->is_const || effective_address_depth != 0U || field->has_array_count) {
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
        declarator = lm_trans_statement_c_declarator_new();
        if (declarator == 0) {
            return 1;
        }
        lm_trans_text_assign_cstr(declarator->type_head, field->class_name);
        lm_trans_text_assign_cstr(declarator->name, field->name);
        declarator->pointer_depth = effective_address_depth;
        declarator->type_is_head = 1;
        if (field->has_array_count) {
            declarator->literal_dimensions[0] = field->array_count;
            declarator->literal_dimension_count = 1U;
        }
        if (
            lm_trans_emit_indent(file, indent) != 0 ||
            (field->is_const && lm_trans_put(file, "const ") != 0) ||
            lm_trans_emit_c_declarator(file, declarator, namespace_, "layout array dimension") != 0
        ) {
            lm_trans_statement_c_declarator_destroy(declarator);
            return 1;
        }
        lm_trans_statement_c_declarator_destroy(declarator);
        declarator = 0;
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
    LmTransLayoutField **fields;
    size_t field_capacity;
    size_t field_count;
    size_t i;

    field_capacity = 256U;
    fields = lm_trans_layout_fields_new(field_capacity);
    if (fields == 0) {
        return 1;
    }
    if (
        lm_trans_collect_layout_fields(
            layout_name,
            namespace_,
            fields,
            field_capacity,
            &field_count
        ) != 0
    ) {
        lm_trans_layout_fields_delete(fields, field_capacity);
        return 1;
    }
    if (lm_trans_sort_layout_fields(fields, field_count) != 0) {
        lm_trans_layout_fields_delete(fields, field_capacity);
        return 1;
    }

    for (i = 0U; i < field_count; ++i) {
        if (lm_trans_emit_layout_field(file, fields[i], namespace_, indent) != 0) {
            lm_trans_layout_fields_delete(fields, field_capacity);
            return 1;
        }
    }

    lm_trans_layout_fields_delete(fields, field_capacity);
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
    LmP0Text *layout_text;
    int status;

    if (file == 0 || layout_name == 0) {
        return 1;
    }

    layout_text = lm_trans_l4_text_view_new(layout_name);
    if (layout_text == 0) {
        return 1;
    }
    backend_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("layout"), "backend");
    backend_row = lm_trans_registry_relation_stack_latest_row(backend_rows, layout_text);
    backend = backend_row != 0 ? backend_row->payload : 0;
    if (!lm_trans_layout_backend_is_supported(backend)) {
        fprintf(stderr, "trans L4 layout error: layout %s has unsupported layout.backend\n", layout_name);
        lm_trans_l4_text_view_delete(&layout_text);
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
        lm_trans_l4_text_view_delete(&layout_text);
        return 1;
    }

    if (lm_trans_emit_layout_fields(file, layout_name, namespace_, 1U) != 0) {
        lm_trans_l4_text_view_delete(&layout_text);
        return 1;
    }

    if (has_forward && (strcmp(backend, "c.struct") == 0 || strcmp(backend, "c.union") == 0)) {
        status = lm_trans_put(file, "};\n");
        lm_trans_l4_text_view_delete(&layout_text);
        return status;
    }
    if (strcmp(backend, "c.struct") == 0 || strcmp(backend, "c.union") == 0) {
        status =
            lm_trans_put(file, "} ") ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(layout_name)) ||
            lm_trans_put(file, ";\n");
        lm_trans_l4_text_view_delete(&layout_text);
        return status;
    }
    status = lm_trans_put(file, "};\n");
    lm_trans_l4_text_view_delete(&layout_text);
    return status;
}

static int lm_trans_emit_l4_layout_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    size_t i;
    const char *name;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_layout_names(names, namespace_) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    for (i = 0U; i < names->count; ++i) {
        if (i == 0U && lm_trans_put(file, "#include <stddef.h>\n\n") != 0) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        name = (const char *)lm_own_ptr_stack_at(names, i);
        if (name != 0 && lm_trans_emit_layout_definition(file, name, namespace_) != 0) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    lm_trans_ptr_stack_delete(&names);
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
    LmP0Text *key_text;

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
    key_text = lm_trans_l4_text_view_new("");
    if (key_text == 0) {
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
            lm_trans_text_assign_cstr(key_text, row->key);
            if (lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0) {
                fprintf(stderr, "trans L4 constant error: constant.backend key %s is not a class\n", row->key);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                lm_own_delete(name, 0);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                lm_own_delete(name, 0);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
        }
    }
    lm_trans_l4_text_view_delete(&key_text);
    return 0;
}

static int lm_trans_emit_l4_constant_defines(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    size_t i;
    const char *name;
    LmTransRegistryFact *value_row;
    const LmOwnPtrStack *value_rows;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_constant_define_names(names, namespace_) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    value_rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("constant"),
        "value"
    );

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        value_row = lm_trans_registry_relation_stack_latest_row(
            value_rows,
            lm_trans_text_from_cstr(name)
        );
        if (value_row == 0 || value_row->payload == 0) {
            fprintf(stderr, "trans L4 constant error: %s requires constant.value\n", name);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        if (
            lm_trans_put(file, "#define ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, " ") != 0 ||
            lm_trans_put(file, value_row->payload) != 0 ||
            lm_trans_put(file, "\n") != 0
        ) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_l4_define_table(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    const LmOwnPtrStack *rows;
    LmTransRegistryFact *row;
    size_t i;
    int emitted;

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("define"),
        "value"
    );
    if (rows == 0) {
        return 0;
    }

    emitted = 0;
    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (row == 0 || row->key == 0) {
            continue;
        }
        if (
            lm_trans_put(file, "#define ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(row->key)) != 0
        ) {
            return 1;
        }
        if (
            row->payload != 0 &&
            row->payload[0] != '\0' &&
            (
                lm_trans_put(file, " ") != 0 ||
                lm_trans_put(file, row->payload) != 0
            )
        ) {
            return 1;
        }
        if (lm_trans_put(file, "\n") != 0) {
            return 1;
        }
        emitted = 1;
    }

    if (emitted && lm_trans_put(file, "\n") != 0) {
        return 1;
    }
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
    LmP0Text *key_text;

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
    key_text = lm_trans_l4_text_view_new("");
    if (key_text == 0) {
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
            lm_trans_text_assign_cstr(key_text, row->key);
            if (
                require_class &&
                lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0
            ) {
                fprintf(stderr, "trans L4 %s error: %s.backend key %s is not a class\n", error_name, receiver_name, row->key);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            name = lm_trans_text_copy_cstr(key_text);
            if (name == 0) {
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
            if (lm_trans_string_stack_has(names, name)) {
                lm_own_delete(name, 0);
            } else if (lm_own_ptr_stack_push(names, name) != 0) {
                lm_own_delete(name, 0);
                lm_trans_l4_text_view_delete(&key_text);
                return 1;
            }
        }
    }

    lm_trans_l4_text_view_delete(&key_text);
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
    LmP0Text *key_text;

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
    key_text = lm_trans_l4_text_view_new("");
    if (key_text == 0) {
        return 1;
    }

    for (i = 0U; i < rows->count; ++i) {
        row = (LmTransRegistryFact *)lm_own_ptr_stack_at(rows, i);
        if (row == 0 || row->key == 0 || row->payload == 0) {
            continue;
        }
        lm_trans_text_assign_cstr(key_text, row->key);
        if (
            require_class &&
            lm_trans_namespace_registry_lookup(namespace_, key_text, "class.present") == 0
        ) {
            fprintf(stderr, "trans L4 %s error: %s.%s key %s is not a class\n", error_name, owner_name, relation_name, row->key);
            lm_trans_l4_text_view_delete(&key_text);
            return 1;
        }
        name = lm_trans_text_copy_cstr(key_text);
        if (name == 0) {
            lm_trans_l4_text_view_delete(&key_text);
            return 1;
        }
        if (lm_trans_string_stack_has(names, name)) {
            lm_own_delete(name, 0);
        } else if (lm_own_ptr_stack_push(names, name) != 0) {
            lm_own_delete(name, 0);
            lm_trans_l4_text_view_delete(&key_text);
            return 1;
        }
    }

    lm_trans_l4_text_view_delete(&key_text);
    return 0;
}

static int lm_trans_l4_is_function_pointer_type(
    const LmTransNamespace *namespace_,
    const char *name
) {
    LmP0Text *name_text;
    const LmOwnPtrStack *rows;
    int found;

    if (name == 0) {
        return 0;
    }

    name_text = lm_trans_l4_text_view_new(name);
    if (name_text == 0) {
        return 0;
    }
    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "value"
    );
    if (lm_trans_registry_relation_stack_latest_row(rows, name_text) != 0) {
        lm_trans_l4_text_view_delete(&name_text);
        return 1;
    }

    rows = lm_trans_namespace_registry_relation_stack(
        namespace_,
        lm_trans_text_from_cstr("functionPointerType"),
        "row"
    );
    found = lm_trans_registry_relation_stack_latest_row(rows, name_text) != 0;
    lm_trans_l4_text_view_delete(&name_text);
    return found;
}

static int lm_trans_emit_abi_typed_name(
    FILE *file,
    const char *class_name,
    size_t address_depth,
    int is_const,
    const char *name
) {
    size_t i;
    LmP0Text *class_text;

    if (file == 0 || class_name == 0) {
        return 1;
    }

    if (is_const && lm_trans_put(file, "const ") != 0) {
        return 1;
    }
    class_text = lm_trans_l4_text_view_new(class_name);
    if (class_text == 0) {
        return 1;
    }
    if (lm_trans_emit_type_name(file, class_text) != 0) {
        lm_trans_l4_text_view_delete(&class_text);
        return 1;
    }
    if ((address_depth != 0U || name != 0) && lm_trans_put(file, " ") != 0) {
        lm_trans_l4_text_view_delete(&class_text);
        return 1;
    }
    for (i = 0U; i < address_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            lm_trans_l4_text_view_delete(&class_text);
            return 1;
        }
    }
    if (name != 0) {
        if (lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0) {
            lm_trans_l4_text_view_delete(&class_text);
            return 1;
        }
    }
    lm_trans_l4_text_view_delete(&class_text);
    return 0;
}

static int lm_trans_registry_latest_size_payload(
    const LmOwnPtrStack *rows,
    const LmP0Text *key,
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
    LmTransAbiParam **params,
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
    LmP0Text *owner_text;
    LmP0Text *param_text;
    LmTransRegistryFact *row;
    LmTransRegistryFact *index_row;
    size_t i;
    size_t j;
    size_t index;
    size_t const_flag;
    LmTransAbiParam *swap;
    int status;

    if (params == 0 || out_count == 0 || owner_name == 0 || error_name == 0) {
        return 1;
    }

    owner_text = lm_trans_l4_text_view_new(owner_name);
    param_text = lm_trans_l4_text_view_new("");
    if (owner_text == 0 || param_text == 0) {
        lm_trans_l4_text_view_delete(&owner_text);
        lm_trans_l4_text_view_delete(&param_text);
        return 1;
    }

    status = 0;
    *out_count = 0U;
    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "param.class");
    if (class_rows == 0) {
        goto cleanup;
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
            status = 1;
            goto cleanup;
        }

        lm_trans_text_assign_cstr(param_text, row->key);
        index_row = lm_trans_registry_relation_stack_latest_row(index_rows, param_text);
        if (index_row == 0 || !lm_trans_parse_size_payload(index_row->payload, &index)) {
            fprintf(stderr, "trans L4 %s error: parameter %s.%s requires param.index\n", error_name, owner_name, row->key);
            status = 1;
            goto cleanup;
        }

        params[*out_count]->name = row->key;
        params[*out_count]->class_name = row->payload;
        params[*out_count]->index = index;
        if (
            !lm_trans_registry_latest_size_payload(
                address_depth_rows,
                param_text,
                0U,
                &params[*out_count]->address_depth
            )
        ) {
            fprintf(stderr, "trans L4 %s error: parameter %s.%s has invalid param.address-depth\n", error_name, owner_name, row->key);
            status = 1;
            goto cleanup;
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
            status = 1;
            goto cleanup;
        }
        params[*out_count]->is_const = const_flag != 0U;
        params[*out_count]->index = index;
        ++*out_count;
    }

    for (i = 0U; i < *out_count; ++i) {
        for (j = i + 1U; j < *out_count; ++j) {
            if (params[j]->index < params[i]->index) {
                swap = params[i];
                params[i] = params[j];
                params[j] = swap;
            }
        }
    }

cleanup:
    lm_trans_l4_text_view_delete(&owner_text);
    lm_trans_l4_text_view_delete(&param_text);
    return status;
}

static int lm_trans_emit_abi_params(
    FILE *file,
    LmTransAbiParam **params,
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
                params[i]->class_name,
                params[i]->address_depth,
                params[i]->is_const,
                params[i]->name
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
    LmOwnPtrStack *names;
    const LmOwnPtrStack *target_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *target_row;
    const char *name;
    LmP0Text *name_text;
    size_t i;
    size_t address_depth;
    size_t const_flag;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    name_text = lm_trans_l4_text_view_new("");
    if (name_text == 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "alias", "c.typedef", "alias", 1) != 0) {
        lm_trans_l4_text_view_delete(&name_text);
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    target_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("alias"), "target.const");

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        lm_trans_text_assign_cstr(name_text, name);
        target_row = lm_trans_registry_relation_stack_latest_row(target_rows, name_text);
        if (target_row == 0 || target_row->payload == 0) {
            fprintf(stderr, "trans L4 alias error: %s requires alias.target\n", name);
            lm_trans_l4_text_view_delete(&name_text);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        if (
            !lm_trans_registry_latest_size_payload(address_depth_rows, name_text, 0U, &address_depth) ||
            !lm_trans_registry_latest_size_payload(const_rows, name_text, 0U, &const_flag)
        ) {
            fprintf(stderr, "trans L4 alias error: %s has invalid alias target flags\n", name);
            lm_trans_l4_text_view_delete(&name_text);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        if (
            lm_trans_put(file, "typedef ") != 0 ||
            lm_trans_emit_abi_typed_name(file, target_row->payload, address_depth, const_flag != 0U, name) != 0 ||
            lm_trans_put(file, ";\n") != 0
        ) {
            lm_trans_l4_text_view_delete(&name_text);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_l4_text_view_delete(&name_text);
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_l4_text_view_delete(&name_text);
    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_l4_forward_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    const LmOwnPtrStack *backend_rows;
    LmTransRegistryFact *backend_row;
    const char *name;
    const char *tag;
    size_t i;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "forward", "c.struct", "forward", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "forward", "c.union", "forward", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    backend_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("forward"), "backend");
    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        backend_row = lm_trans_registry_relation_stack_latest_row(backend_rows, lm_trans_text_from_cstr(name));
        if (backend_row == 0 || backend_row->payload == 0) {
            fprintf(stderr, "trans L4 forward error: %s requires forward.backend\n", name);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        if (strcmp(backend_row->payload, "c.struct") == 0) {
            tag = "struct";
        } else if (strcmp(backend_row->payload, "c.union") == 0) {
            tag = "union";
        } else {
            fprintf(stderr, "trans L4 forward error: unsupported backend %s for %s\n", backend_row->payload, name);
            lm_trans_ptr_stack_delete(&names);
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
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_abi_return_type(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *owner_name,
    const char *error_name
) {
    LmP0Text *owner_text;
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *class_row;
    LmP0Text *return_key;
    size_t address_depth;
    size_t const_flag;
    int status;

    if (file == 0 || owner_name == 0 || error_name == 0) {
        return 1;
    }

    owner_text = lm_trans_l4_text_view_new(owner_name);
    return_key = lm_trans_l4_text_view_new("");
    if (owner_text == 0 || return_key == 0) {
        lm_trans_l4_text_view_delete(&owner_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 1;
    }

    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.class");
    class_row = lm_trans_registry_relation_stack_latest_return_row(class_rows);
    if (class_row == 0 || class_row->key == 0 || class_row->payload == 0) {
        fprintf(stderr, "trans L4 %s error: %s requires return.class\n", error_name, owner_name);
        lm_trans_l4_text_view_delete(&owner_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 1;
    }

    lm_trans_text_assign_cstr(return_key, class_row->key);
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, owner_text, "return.const");
    if (
        !lm_trans_registry_latest_size_payload(address_depth_rows, return_key, 0U, &address_depth) ||
        !lm_trans_registry_latest_size_payload(const_rows, return_key, 0U, &const_flag)
    ) {
        fprintf(stderr, "trans L4 %s error: %s has invalid return flags\n", error_name, owner_name);
        lm_trans_l4_text_view_delete(&owner_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 1;
    }

    status = lm_trans_emit_abi_typed_name(file, class_row->payload, address_depth, const_flag != 0U, 0);
    lm_trans_l4_text_view_delete(&owner_text);
    lm_trans_l4_text_view_delete(&return_key);
    return status;
}

static int lm_trans_emit_l4_prototype_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
) {
    LmTransAbiParam **params;
    size_t param_capacity;
    size_t param_count;
    const char *class_kind;
    int is_sub;
    int status;

    if (name == 0 || error_name == 0) {
        return 1;
    }

    class_kind = lm_trans_namespace_registry_lookup(namespace_, lm_trans_text_from_cstr(name), "class.kind");
    is_sub = class_kind != 0 && strcmp(class_kind, "procedure") == 0;

    param_capacity = 256U;
    params = lm_trans_l4_abi_params_new(param_capacity);
    if (params == 0) {
        return 1;
    }
    status =
        lm_trans_collect_abi_params(params, param_capacity, &param_count, namespace_, name, error_name) != 0 ||
        (
            lm_trans_namespace_registry_lookup(namespace_, lm_trans_text_from_cstr(name), "fn.external") == 0 &&
            lm_trans_put(file, "static ") != 0
        ) ||
        (
            is_sub
                ? lm_trans_put(file, "void")
                : lm_trans_emit_abi_return_type(file, namespace_, name, error_name)
        ) != 0 ||
        lm_trans_put(file, " ") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
        lm_trans_put(file, "(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ");\n") != 0;
    lm_trans_l4_abi_params_delete(params, param_capacity);
    return status;
}

static int lm_trans_emit_l4_prototypes(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    const char *name;
    size_t i;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_relation_names(names, namespace_, "fn", "descriptor", "fn", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        if (lm_trans_l4_is_function_pointer_type(namespace_, name)) {
            continue;
        }
        if (lm_trans_emit_l4_prototype_name(file, namespace_, name, "fn") != 0) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static LmP0Node *lm_trans_registry_synthetic_node(LmP0NodeKind kind) {
    LmP0Node *node;

    node = (LmP0Node *)lm_own_arena_new_zero(
        lm_trans_registry->value_arena,
        sizeof(*node)
    );
    if (node == 0) {
        return 0;
    }
    node->as = (LmP0NodeAs *)lm_own_arena_new_zero(
        lm_trans_registry->value_arena,
        sizeof(*node->as)
    );
    if (node->as == 0) {
        return 0;
    }
    node->kind = kind;
    if (kind == LM_P0_NODE_ATOM || kind == LM_P0_NODE_DISABLED) {
        node->as->atom = lm_trans_registry_new_text();
        if (node->as->atom == 0) {
            return 0;
        }
    } else if (kind == LM_P0_NODE_FRAME) {
        node->as->frame = lm_trans_registry_new_frame();
        if (node->as->frame == 0) {
            return 0;
        }
    } else if (kind == LM_P0_NODE_STRUCTURE) {
        node->as->structure = lm_trans_registry_new_structure();
        if (node->as->structure == 0) {
            return 0;
        }
    }
    return node;
}

static LmP0Node *lm_trans_registry_synthetic_atom_node(const LmP0Text *text) {
    LmP0Node *node;

    node = lm_trans_registry_synthetic_node(LM_P0_NODE_ATOM);
    if (node == 0) {
        return 0;
    }
    if (lm_trans_registry_clone_text(text, node->as->atom) != 0) {
        return 0;
    }
    return node;
}

static LmP0Node *lm_trans_registry_synthetic_atom_node_cstr(const char *text) {
    return lm_trans_registry_synthetic_atom_node(lm_trans_text_from_cstr(text));
}

static LmP0Field *lm_trans_registry_synthetic_append_node(
    LmP0Structure *structure,
    LmP0Node *value
) {
    LmP0Field *field;

    if (structure == 0 || value == 0) {
        return 0;
    }

    field = (LmP0Field *)lm_own_arena_new_zero(
        lm_trans_registry->value_arena,
        sizeof(*field)
    );
    if (field == 0) {
        return 0;
    }
    field->value = value;

    if (structure->last_field != 0) {
        structure->last_field->next = field;
    } else {
        structure->first_field = field;
    }
    structure->last_field = field;
    ++structure->field_count;
    return field;
}

static LmP0Node *lm_trans_registry_synthetic_frame_node(const LmP0Text *head) {
    LmP0Node *node;

    node = lm_trans_registry_synthetic_node(LM_P0_NODE_FRAME);
    if (node == 0) {
        return 0;
    }
    if (lm_trans_registry_clone_text(head, node->as->frame->head) != 0) {
        return 0;
    }
    return node;
}

static LmP0Node *lm_trans_registry_synthetic_frame_node_cstr(const char *head) {
    return lm_trans_registry_synthetic_frame_node(lm_trans_text_from_cstr(head));
}

static LmP0Node *lm_trans_registry_synthetic_structure_node(void) {
    return lm_trans_registry_synthetic_node(LM_P0_NODE_STRUCTURE);
}

static char *lm_trans_registry_synthetic_at_head(size_t address_depth) {
    char *head;

    if (address_depth == 0U) {
        return 0;
    }

    head = (char *)lm_own_arena_new_zero(
        lm_trans_registry->value_arena,
        address_depth + 1U
    );
    if (head == 0) {
        return 0;
    }
    memset(head, '@', address_depth);
    head[address_depth] = '\0';
    return head;
}

static LmP0Node *lm_trans_registry_synthetic_callable_type_node(
    const char *class_name,
    size_t address_depth,
    int is_const,
    const char *name
) {
    LmP0Node *node;
    LmP0Node *wrapper;
    char *at_head;

    if (class_name == 0) {
        return 0;
    }

    if (address_depth != 0U) {
        at_head = lm_trans_registry_synthetic_at_head(address_depth);
        if (at_head == 0) {
            return 0;
        }
        node = lm_trans_registry_synthetic_frame_node_cstr(at_head);
        if (
            node == 0 ||
            lm_trans_registry_synthetic_append_node(
                node->as->frame->body,
                lm_trans_registry_synthetic_atom_node_cstr(class_name)
            ) == 0
        ) {
            return 0;
        }
        if (
            name != 0 &&
            lm_trans_registry_synthetic_append_node(
                node->as->frame->body,
                lm_trans_registry_synthetic_atom_node_cstr(name)
            ) == 0
        ) {
            return 0;
        }
    } else if (name != 0) {
        node = lm_trans_registry_synthetic_frame_node_cstr(class_name);
        if (
            node == 0 ||
            lm_trans_registry_synthetic_append_node(
                node->as->frame->body,
                lm_trans_registry_synthetic_atom_node_cstr(name)
            ) == 0
        ) {
            return 0;
        }
    } else {
        node = lm_trans_registry_synthetic_atom_node_cstr(class_name);
        if (node == 0) {
            return 0;
        }
    }

    if (!is_const) {
        return node;
    }

    wrapper = lm_trans_registry_synthetic_frame_node_cstr("const");
    if (
        wrapper == 0 ||
        lm_trans_registry_synthetic_append_node(wrapper->as->frame->body, node) == 0
    ) {
        return 0;
    }
    return wrapper;
}

static LmP0Node *lm_trans_registry_synthetic_return_node(
    const LmTransNamespace *namespace_,
    const char *name
) {
    LmP0Text *name_text;
    LmP0Text *return_key;
    const LmOwnPtrStack *class_rows;
    const LmOwnPtrStack *address_depth_rows;
    const LmOwnPtrStack *const_rows;
    LmTransRegistryFact *class_row;
    LmP0Node *return_node;
    size_t address_depth;
    size_t const_flag;

    if (name == 0) {
        return 0;
    }

    name_text = lm_trans_l4_text_view_new(name);
    return_key = lm_trans_l4_text_view_new("");
    if (name_text == 0 || return_key == 0) {
        lm_trans_l4_text_view_delete(&name_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 0;
    }

    class_rows = lm_trans_namespace_registry_relation_stack(namespace_, name_text, "return.class");
    class_row = lm_trans_registry_relation_stack_latest_return_row(class_rows);
    if (class_row == 0 || class_row->payload == 0) {
        fprintf(stderr, "trans L4 fn import error: %s requires return.class\n", name);
        lm_trans_l4_text_view_delete(&name_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 0;
    }

    lm_trans_text_assign_cstr(return_key, class_row->key != 0 ? class_row->key : "return");
    address_depth_rows = lm_trans_namespace_registry_relation_stack(namespace_, name_text, "return.address-depth");
    const_rows = lm_trans_namespace_registry_relation_stack(namespace_, name_text, "return.const");
    if (
        !lm_trans_registry_latest_size_payload(address_depth_rows, return_key, 0U, &address_depth) ||
        !lm_trans_registry_latest_size_payload(const_rows, return_key, 0U, &const_flag)
    ) {
        fprintf(stderr, "trans L4 fn import error: %s has invalid return flags\n", name);
        lm_trans_l4_text_view_delete(&name_text);
        lm_trans_l4_text_view_delete(&return_key);
        return 0;
    }

    return_node = lm_trans_registry_synthetic_callable_type_node(
        class_row->payload,
        address_depth,
        const_flag != 0U,
        0
    );
    lm_trans_l4_text_view_delete(&name_text);
    lm_trans_l4_text_view_delete(&return_key);
    return return_node;
}

static LmP0Node *lm_trans_registry_synthetic_params_node(
    const LmTransNamespace *namespace_,
    const char *name
) {
    LmTransAbiParam **params;
    LmP0Node *params_node;
    LmP0Node *param_node;
    size_t param_capacity;
    size_t param_count;
    size_t i;

    if (name == 0) {
        return 0;
    }

    param_capacity = 256U;
    params = lm_trans_l4_abi_params_new(param_capacity);
    if (params == 0) {
        return 0;
    }
    if (lm_trans_collect_abi_params(
        params,
        param_capacity,
        &param_count,
        namespace_,
        name,
        "fn import"
    ) != 0) {
        lm_trans_l4_abi_params_delete(params, param_capacity);
        return 0;
    }

    params_node = lm_trans_registry_synthetic_structure_node();
    if (params_node == 0) {
        lm_trans_l4_abi_params_delete(params, param_capacity);
        return 0;
    }

    for (i = 0U; i < param_count; ++i) {
        param_node = lm_trans_registry_synthetic_callable_type_node(
            params[i]->class_name,
            params[i]->address_depth,
            params[i]->is_const,
            params[i]->name
        );
        if (
            param_node == 0 ||
            lm_trans_registry_synthetic_append_node(
                params_node->as->structure,
                param_node
            ) == 0
        ) {
            lm_trans_l4_abi_params_delete(params, param_capacity);
            return 0;
        }
    }

    lm_trans_l4_abi_params_delete(params, param_capacity);
    return params_node;
}

static LmP0Node *lm_trans_registry_synthetic_fn_frame_node(
    const char *name,
    LmP0Node *params_node,
    LmP0Node *return_node
) {
    LmP0Node *frame_node;

    if (name == 0 || params_node == 0 || return_node == 0) {
        return 0;
    }

    frame_node = lm_trans_registry_synthetic_frame_node_cstr("fn");
    if (
        frame_node == 0 ||
        lm_trans_registry_synthetic_append_node(
            frame_node->as->frame->body,
            lm_trans_registry_synthetic_atom_node_cstr(name)
        ) == 0 ||
        lm_trans_registry_synthetic_append_node(
            frame_node->as->frame->body,
            params_node
        ) == 0 ||
        lm_trans_registry_synthetic_append_node(
            frame_node->as->frame->body,
            return_node
        ) == 0
    ) {
        return 0;
    }

    return frame_node;
}

static LmP0Node *lm_trans_registry_synthetic_sub_frame_node(
    const char *name,
    LmP0Node *params_node
) {
    LmP0Node *frame_node;

    if (name == 0 || params_node == 0) {
        return 0;
    }

    frame_node = lm_trans_registry_synthetic_frame_node_cstr("sub");
    if (
        frame_node == 0 ||
        lm_trans_registry_synthetic_append_node(
            frame_node->as->frame->body,
            lm_trans_registry_synthetic_atom_node_cstr(name)
        ) == 0 ||
        lm_trans_registry_synthetic_append_node(
            frame_node->as->frame->body,
            params_node
        ) == 0
    ) {
        return 0;
    }

    return frame_node;
}

static int lm_trans_declare_l4_sub_descriptor(
    LmTransNamespace *namespace_,
    const char *name
) {
    LmP0Node *params_node;
    LmP0Node *frame_node;
    LmTransFunctionHeader *function;
    LmP0Text *name_text;
    int status;

    if (namespace_ == 0 || name == 0) {
        return 1;
    }

    function = lm_trans_function_header_new();
    name_text = lm_trans_l4_text_view_new(name);
    if (function == 0 || name_text == 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 1;
    }
    if (lm_trans_namespace_find(namespace_, name_text) != 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 0;
    }

    params_node = lm_trans_registry_synthetic_params_node(namespace_, name);
    frame_node = lm_trans_registry_synthetic_sub_frame_node(name, params_node);
    if (params_node == 0 || frame_node == 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 1;
    }

    function->frame = frame_node->as->frame;
    *function->name = *frame_node->as->frame->body->first_field->value->as->atom;
    *function->c_name = *function->name;
    function->params_node = params_node;
    function->return_node = 0;
    function->symbol_class = "procedure";
    function->is_sub = 1;
    function->is_descriptor_only = 1;
    function->is_external = lm_trans_namespace_registry_lookup(namespace_, name_text, "fn.external") != 0;

    status = (
        lm_trans_namespace_declare_c_name(
            namespace_,
            function->name,
            function->symbol_class,
            function->c_name
        ) != 0 ||
        lm_trans_namespace_set_callable_shape(namespace_, function) != 0
    );

    lm_trans_function_header_destroy(function);
    lm_trans_l4_text_view_delete(&name_text);
    return status;
}

static int lm_trans_declare_l4_fn_descriptor(
    LmTransNamespace *namespace_,
    const char *name
) {
    LmP0Node *params_node;
    LmP0Node *return_node;
    LmP0Node *frame_node;
    LmTransFunctionHeader *function;
    LmP0Text *name_text;
    const char *receiver_name;
    int status;

    if (namespace_ == 0 || name == 0) {
        return 1;
    }
    if (lm_trans_l4_is_function_pointer_type(namespace_, name)) {
        return 0;
    }
    function = lm_trans_function_header_new();
    name_text = lm_trans_l4_text_view_new(name);
    if (function == 0 || name_text == 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 1;
    }
    if (lm_trans_namespace_find(namespace_, name_text) != 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 0;
    }
    receiver_name = lm_trans_namespace_registry_lookup(namespace_, name_text, "fn.receiver");
    if (receiver_name != 0 && strcmp(receiver_name, "sub") == 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return lm_trans_declare_l4_sub_descriptor(namespace_, name);
    }

    params_node = lm_trans_registry_synthetic_params_node(namespace_, name);
    return_node = lm_trans_registry_synthetic_return_node(namespace_, name);
    frame_node = lm_trans_registry_synthetic_fn_frame_node(name, params_node, return_node);
    if (params_node == 0 || return_node == 0 || frame_node == 0) {
        lm_trans_function_header_destroy(function);
        lm_trans_l4_text_view_delete(&name_text);
        return 1;
    }

    function->frame = frame_node->as->frame;
    *function->name = *frame_node->as->frame->body->first_field->value->as->atom;
    *function->c_name = *function->name;
    function->params_node = params_node;
    function->return_node = return_node;
    function->symbol_class = "function";
    function->is_external = lm_trans_namespace_registry_lookup(namespace_, name_text, "fn.external") != 0;

    status = (
        lm_trans_namespace_declare_c_name(
            namespace_,
            function->name,
            function->symbol_class,
            function->c_name
        ) != 0 ||
        lm_trans_namespace_set_callable_shape(namespace_, function) != 0
    );

    lm_trans_function_header_destroy(function);
    lm_trans_l4_text_view_delete(&name_text);
    return status;
}

static int lm_trans_declare_l4_fn_descriptors(
    LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    const char *name;
    size_t i;

    if (namespace_ == 0) {
        return 1;
    }

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_relation_names(names, namespace_, "fn", "descriptor", "fn", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        if (lm_trans_declare_l4_fn_descriptor(namespace_, name) != 0) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_l4_function_pointer_type_name(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *name,
    const char *error_name
) {
    LmTransAbiParam **params;
    size_t param_capacity;
    size_t param_count;
    int status;

    if (name == 0 || error_name == 0) {
        return 1;
    }

    param_capacity = 256U;
    params = lm_trans_l4_abi_params_new(param_capacity);
    if (params == 0) {
        return 1;
    }
    status =
        lm_trans_collect_abi_params(params, param_capacity, &param_count, namespace_, name, error_name) != 0 ||
        lm_trans_put(file, "typedef ") != 0 ||
        lm_trans_emit_abi_return_type(file, namespace_, name, error_name) != 0 ||
        lm_trans_put(file, " (*") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
        lm_trans_put(file, ")(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ");\n") != 0;
    lm_trans_l4_abi_params_delete(params, param_capacity);
    return status;
}

static int lm_trans_emit_l4_function_pointer_type_field(
    FILE *file,
    const LmTransNamespace *namespace_,
    const char *type_name,
    const char *field_name,
    const char *error_name
) {
    LmTransAbiParam **params;
    size_t param_capacity;
    size_t param_count;
    int status;

    if (type_name == 0 || field_name == 0 || error_name == 0) {
        return 1;
    }

    param_capacity = 256U;
    params = lm_trans_l4_abi_params_new(param_capacity);
    if (params == 0) {
        return 1;
    }
    status =
        lm_trans_collect_abi_params(params, param_capacity, &param_count, namespace_, type_name, error_name) != 0 ||
        lm_trans_emit_abi_return_type(file, namespace_, type_name, error_name) != 0 ||
        lm_trans_put(file, " (*") != 0 ||
        lm_trans_emit_identifier(file, lm_trans_text_from_cstr(field_name)) != 0 ||
        lm_trans_put(file, ")(") != 0 ||
        lm_trans_emit_abi_params(file, params, param_count) != 0 ||
        lm_trans_put(file, ")") != 0;
    lm_trans_l4_abi_params_delete(params, param_capacity);
    return status;
}

static int lm_trans_emit_l4_function_pointer_type_typedefs(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
    const char *name;
    size_t i;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (
        lm_trans_registry_collect_relation_names(names, namespace_, "functionPointerType", "value", "functionPointerType", 1) != 0 ||
        lm_trans_registry_collect_relation_names(names, namespace_, "functionPointerType", "row", "functionPointerType", 1) != 0
    ) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        if (lm_trans_emit_l4_function_pointer_type_name(file, namespace_, name, "functionPointerType") != 0) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_ptr_stack_delete(&names);
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
    LmOwnPtrStack *names;
    const char *name;
    size_t i;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "guard", "c.header-guard", "guard", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        if (
            lm_trans_put(file, "#ifndef ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, "\n#define ") != 0 ||
            lm_trans_emit_identifier(file, lm_trans_text_from_cstr(name)) != 0 ||
            lm_trans_put(file, "\n#endif\n") != 0
        ) {
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_l4_extern_c_markers(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "extern_c", "c.wrapper", "extern_c", 0) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    if (
        names->count != 0U &&
        (
            lm_trans_put(file, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n") != 0 ||
            lm_trans_put(file, "#ifdef __cplusplus\n}\n#endif\n\n") != 0
        )
    ) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_emit_l4_units(
    FILE *file,
    const LmTransNamespace *namespace_
) {
    LmOwnPtrStack *names;
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

    names = lm_trans_ptr_stack_new(lm_trans_free_any);
    if (names == 0) {
        return 1;
    }
    if (lm_trans_registry_collect_backend_names(names, namespace_, "unit", "c.static-unit", "unit", 1) != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }

    payload_rows = lm_trans_namespace_registry_relation_stack(namespace_, lm_trans_text_from_cstr("unit"), "payload");
    for (i = 0U; i < names->count; ++i) {
        name = (const char *)lm_own_ptr_stack_at(names, i);
        payload_row = lm_trans_registry_relation_stack_latest_row(payload_rows, lm_trans_text_from_cstr(name));
        if (payload_row == 0 || (payload_row->payload == 0 && payload_row->payload_node == 0)) {
            fprintf(stderr, "trans L4 unit error: %s requires unit.payload\n", name);
            lm_trans_ptr_stack_delete(&names);
            return 1;
        }
        if (payload_row->payload_node != 0) {
            if (
                lm_trans_emit_l4_payload_node(file, payload_row->payload_node, (LmTransNamespace *)namespace_) != 0 ||
                lm_trans_put(file, "\n") != 0
            ) {
                lm_trans_ptr_stack_delete(&names);
                return 1;
            }
        } else {
            if (
                lm_trans_put(file, payload_row->payload) != 0 ||
                lm_trans_put(file, "\n") != 0
            ) {
                lm_trans_ptr_stack_delete(&names);
                return 1;
            }
        }
    }

    if (names->count != 0U && lm_trans_put(file, "\n") != 0) {
        lm_trans_ptr_stack_delete(&names);
        return 1;
    }
    lm_trans_ptr_stack_delete(&names);
    return 0;
}

static int lm_trans_registry_init(void) {
    if (lm_trans_registry != 0) {
        return 0;
    }
    lm_trans_registry = (LmTransRegistry *)lm_own_new_zero(sizeof(*lm_trans_registry));
    return lm_trans_registry == 0 ? 1 : 0;
}

static void lm_trans_registry_destroy(void) {
    if (lm_trans_registry == 0) {
        return;
    }
    lm_trans_l4_payload_pointer_bindings_destroy();
    lm_trans_ptr_stack_delete(&lm_trans_registry->binding_pointer_bindings);
    lm_trans_ptr_stack_delete(&lm_trans_registry->l4_atom_pointer_bindings);
    lm_trans_ptr_stack_delete(&lm_trans_registry->l4_root_frame_pointer_bindings);
    lm_trans_ptr_stack_delete(&lm_trans_registry->l4_frame_pointer_bindings);
    lm_own_delete(lm_trans_registry->l4_loader, 0);
    lm_trans_registry->l4_loader = 0;
    if (lm_trans_registry->loaded) {
        lm_trans_identifier_table_delete(&lm_trans_registry->identifiers);
        lm_own_arena_destroy(lm_trans_registry->value_arena);
        lm_own_delete(lm_trans_registry->value_arena, 0);
        lm_trans_ptr_stack_delete(&lm_trans_registry->loaded_paths);
        lm_own_delete(lm_trans_registry->source_path, 0);
        lm_trans_registry->value_arena = 0;
        lm_trans_registry->source_path = 0;
        lm_trans_registry->loaded_fact_count = 0U;
        lm_trans_registry->loaded = 0;
    }
    lm_own_delete(lm_trans_registry, 0);
    lm_trans_registry = 0;
}

static char *lm_trans_registry_join_text3(const LmP0Text *first, const char *separator, const LmP0Text *second) {
    size_t separator_length;
    size_t length;
    char *result;

    if (first == 0 || second == 0 || separator == 0) {
        return 0;
    }
    separator_length = strlen(separator);
    length = first->length + separator_length + second->length;
    result = (char *)lm_own_new_zero(length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, first->data, first->length);
    memcpy(result + first->length, separator, separator_length);
    memcpy(result + first->length + separator_length, second->data, second->length);
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
    const LmP0Text *payload
) {
    return lm_trans_registry_push_row_atoms(
        lm_trans_text_from_cstr(table),
        lm_trans_text_from_cstr(key),
        payload
    );
}

static int lm_trans_registry_push_column_metadata(
    const LmP0Text *table_name,
    LmTransRegistryColumn **columns,
    size_t column_count
) {
    LmP0Text *table_payload;
    LmP0Text *column_payload;
    size_t index;
    size_t descriptor_index;
    char *column_key;
    char *descriptor_key;
    char index_buffer[32];
    char count_buffer[32];
    int status;

    if (columns == 0) {
        return -1;
    }

    table_payload = lm_trans_l4_text_view_new("");
    column_payload = lm_trans_l4_text_view_new("");
    if (table_payload == 0 || column_payload == 0) {
        status = -1;
        goto cleanup;
    }
    if (!lm_trans_registry_identifier_value(table_name, table_payload)) {
        status = -1;
        goto cleanup;
    }
    if (lm_trans_registry_note_class_present(table_name) != 0) {
        status = -1;
        goto cleanup;
    }

    for (index = 0U; index < column_count; ++index) {
        if (columns[index] == 0 || columns[index]->name == 0 || !lm_trans_registry_identifier_value(columns[index]->name, column_payload)) {
            status = -1;
            goto cleanup;
        }
        if (lm_trans_registry_note_class_present(columns[index]->name) != 0) {
            status = -1;
            goto cleanup;
        }
        column_key = lm_trans_registry_join_text3(table_payload, ".", column_payload);
        if (column_key == 0) {
            status = -1;
            goto cleanup;
        }

        snprintf(index_buffer, sizeof(index_buffer), "%lu", (unsigned long)index);
        snprintf(count_buffer, sizeof(count_buffer), "%lu", (unsigned long)columns[index]->descriptor_count);

        if (
            lm_trans_registry_push_generated_row_text("column.table", column_key, table_payload) != 0 ||
            lm_trans_registry_push_generated_row_text("column.name", column_key, column_payload) != 0 ||
            lm_trans_registry_push_generated_row_cstr("column.index", column_key, index_buffer) != 0 ||
            lm_trans_registry_push_generated_row_cstr("column.descriptor.count", column_key, count_buffer) != 0
        ) {
            lm_own_delete(column_key, 0);
            status = -1;
            goto cleanup;
        }

        for (descriptor_index = 0U; descriptor_index < columns[index]->descriptor_count; ++descriptor_index) {
            snprintf(index_buffer, sizeof(index_buffer), "%lu", (unsigned long)descriptor_index);
            descriptor_key = lm_trans_registry_join_text3(lm_trans_text_from_cstr(column_key), ".", lm_trans_text_from_cstr(index_buffer));
            if (descriptor_key == 0) {
                lm_own_delete(column_key, 0);
                status = -1;
                goto cleanup;
            }
            if (
                columns[index]->descriptors[descriptor_index] == 0 ||
                lm_trans_registry_push_generated_row_text("column.descriptor", descriptor_key, columns[index]->descriptors[descriptor_index]) != 0
            ) {
                lm_own_delete(descriptor_key, 0);
                lm_own_delete(column_key, 0);
                status = -1;
                goto cleanup;
            }
            lm_own_delete(descriptor_key, 0);
        }

        lm_own_delete(column_key, 0);
    }

    status = 0;

cleanup:
    lm_trans_l4_text_view_delete(&table_payload);
    lm_trans_l4_text_view_delete(&column_payload);
    return status;
}

static char *lm_trans_registry_relation_name_new(const LmP0Text *owner, const char *suffix) {
    size_t suffix_length;
    size_t length;
    char *result;

    if (suffix == 0) {
        return 0;
    }

    suffix_length = strlen(suffix);
    length = owner->length + suffix_length;
    result = (char *)lm_own_new_zero(length + 1U);
    if (result == 0) {
        return 0;
    }
    if (owner->length != 0U) {
        memcpy(result, owner->data, owner->length);
    }
    memcpy(result + owner->length, suffix, suffix_length);
    result[length] = '\0';
    return result;
}

static int lm_trans_registry_push_owner_relation_text(
    const LmP0Text *owner,
    const char *suffix,
    const LmP0Text *key,
    const LmP0Text *payload
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
    lm_own_delete(table_name, 0);
    return status;
}

static int lm_trans_registry_push_owner_relation_size(
    const LmP0Text *owner,
    const char *suffix,
    const LmP0Text *key,
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
    const LmP0Node *current;
    const LmP0Frame *frame;
    const LmP0Field *field;

    if (node == 0 || out == 0) {
        return 0;
    }

    if (lm_trans_callable_type_reset(out) != 0) {
        return 0;
    }

    current = node;
    while (current != 0) {
        if (current->kind == LM_P0_NODE_ATOM) {
            if (!lm_trans_registry_identifier_value(current->as->atom, out->class_name)) {
                return 0;
            }
            return 1;
        }

        if (current->kind != LM_P0_NODE_FRAME) {
            return 0;
        }

        frame = current->as->frame;
        if (lm_trans_text_equals(frame->head, "const")) {
            field = frame->body->first_field;
            if (field == 0 || field->next != 0 || field->value == 0) {
                return 0;
            }
            out->is_const = 1;
            current = field->value;
            continue;
        }

        if (lm_trans_text_all_char(frame->head, '@')) {
            field = frame->body->first_field;
            if (field == 0 || field->value == 0) {
                return 0;
            }
            out->address_depth += frame->head->length;
            current = field->value;
            continue;
        }

        if (lm_trans_text_is_array_receiver_head(frame->head)) {
            field = frame->body->first_field;
            if (field == 0 || field->value == 0) {
                return 0;
            }
            ++out->address_depth;
            current = field->value;
            continue;
        }

        if (!lm_trans_registry_identifier_value(frame->head, out->class_name)) {
            return 0;
        }
        return 1;
    }

    return 0;
}

static int lm_trans_registry_materialize_fn_descriptor_param(
    const LmP0Text *function_name,
    const LmP0Node *param_node,
    size_t index
) {
    LmTransL4CallableType *param_type;
    LmP0Text *param_name;
    char fallback_name[32];
    int status;

    param_type = lm_trans_expr_callable_type_new();
    param_name = lm_trans_l4_text_view_new("");
    if (param_type == 0 || param_name == 0) {
        lm_trans_expr_callable_type_delete(param_type);
        lm_trans_l4_text_view_delete(&param_name);
        return 1;
    }

    if (!lm_trans_l4_callable_type_from_node(param_node, param_type)) {
        fprintf(stderr, "trans fn descriptor error: parameter descriptor expects a type\n");
        lm_trans_expr_callable_type_delete(param_type);
        lm_trans_l4_text_view_delete(&param_name);
        return 1;
    }
    if (!lm_trans_formal_param_name(param_node, param_name)) {
        snprintf(fallback_name, sizeof(fallback_name), "arg%lu", (unsigned long)index);
        lm_trans_text_assign_cstr(param_name, fallback_name);
    }

    status =
        lm_trans_registry_push_owner_relation_text(function_name, ".param.class", param_name, param_type->class_name) != 0 ||
        lm_trans_registry_push_owner_relation_size(function_name, ".param.index", param_name, index) != 0 ||
        (
            param_type->address_depth != 0U &&
            lm_trans_registry_push_owner_relation_size(function_name, ".param.address-depth", param_name, param_type->address_depth) != 0
        ) ||
        (
            param_type->is_const &&
            lm_trans_registry_push_owner_relation_size(function_name, ".param.const", param_name, 1U) != 0
        ) ||
        lm_trans_registry_note_class_present(param_type->class_name) != 0;
    lm_trans_expr_callable_type_delete(param_type);
    lm_trans_l4_text_view_delete(&param_name);
    return status;
}

static int lm_trans_registry_materialize_fn_descriptor_frame(const LmP0Frame *frame) {
    LmTransFunctionHeader *function;
    LmTransL4CallableType *return_type;
    const LmP0Field *field;
    size_t index;
    LmP0Text *function_name;
    LmP0Text *receiver_name;
    int status;

    if (frame == 0) {
        return 1;
    }

    function = lm_trans_function_header_new();
    return_type = lm_trans_expr_callable_type_new();
    function_name = lm_trans_l4_text_view_new("");
    receiver_name = lm_trans_l4_text_view_new("");
    if (function == 0 || return_type == 0 || function_name == 0 || receiver_name == 0) {
        status = 1;
        goto cleanup;
    }
    if (!lm_trans_identifier_payload(frame->head, receiver_name)) {
        status = 1;
        goto cleanup;
    }

    if (
        (
            lm_trans_text_equals(receiver_name, "lazy fn")
                ? lm_trans_receiver_lazy_fn(frame, 0, function)
                : lm_trans_receiver_fn(frame, 0, function)
        ) <= 0
    ) {
        fprintf(stderr, "trans fn descriptor error: malformed descriptor-only fn\n");
        status = 1;
        goto cleanup;
    }
    if (!function->is_descriptor_only) {
        fprintf(stderr, "trans fn descriptor error: descriptor-only fn must not have a body\n");
        status = 1;
        goto cleanup;
    }
    if (!lm_trans_registry_identifier_value(function->name, function_name)) {
        status = 1;
        goto cleanup;
    }
    if (!lm_trans_l4_callable_type_from_node(function->return_node, return_type)) {
        fprintf(stderr, "trans fn descriptor error: return descriptor expects a type\n");
        status = 1;
        goto cleanup;
    }

    if (
        lm_trans_registry_note_class_present(function_name) != 0 ||
        lm_trans_registry_note_class_present(return_type->class_name) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("fn.descriptor"),
            function_name,
            lm_trans_text_from_cstr("callableDescriptor")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("fn.receiver"),
            function_name,
            receiver_name
        ) != 0 ||
        lm_trans_registry_push_owner_relation_text(function_name, ".return.class", lm_trans_text_from_cstr("return"), return_type->class_name) != 0
    ) {
        status = 1;
        goto cleanup;
    }
    if (
        return_type->address_depth != 0U &&
        lm_trans_registry_push_owner_relation_size(function_name, ".return.address-depth", lm_trans_text_from_cstr("return"), return_type->address_depth) != 0
    ) {
        status = 1;
        goto cleanup;
    }
    if (
        return_type->is_const &&
        lm_trans_registry_push_owner_relation_size(function_name, ".return.const", lm_trans_text_from_cstr("return"), 1U) != 0
    ) {
        status = 1;
        goto cleanup;
    }
    if (
        function->is_lazy_descriptor &&
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("callable.lazy"),
            function_name,
            lm_trans_text_from_cstr("1")
        ) != 0
    ) {
        status = 1;
        goto cleanup;
    }

    if (function->params_node == 0 || function->params_node->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans fn descriptor error: parameters must be a Structure\n");
        status = 1;
        goto cleanup;
    }

    index = 0U;
    field = function->params_node->as->structure->first_field;
    while (field != 0) {
        if (
            field->value != 0 &&
            !lm_trans_node_is_ignored(field->value) &&
            lm_trans_registry_materialize_fn_descriptor_param(function_name, field->value, index) != 0
        ) {
            status = 1;
            goto cleanup;
        }
        if (field->value != 0 && !lm_trans_node_is_ignored(field->value)) {
            ++index;
        }
        field = field->next;
    }

    status = 0;

cleanup:
    lm_trans_function_header_destroy(function);
    lm_trans_expr_callable_type_delete(return_type);
    lm_trans_l4_text_view_delete(&function_name);
    lm_trans_l4_text_view_delete(&receiver_name);
    return status;
}

static int lm_trans_registry_materialize_sub_descriptor_frame(const LmP0Frame *frame) {
    LmTransFunctionHeader *function;
    const LmP0Field *field;
    size_t index;
    LmP0Text *function_name;
    LmP0Text *receiver_name;
    int status;

    if (frame == 0) {
        return 1;
    }

    function = lm_trans_function_header_new();
    function_name = lm_trans_l4_text_view_new("");
    receiver_name = lm_trans_l4_text_view_new("");
    if (function == 0 || function_name == 0 || receiver_name == 0) {
        status = 1;
        goto cleanup;
    }
    if (!lm_trans_identifier_payload(frame->head, receiver_name)) {
        status = 1;
        goto cleanup;
    }

    if (lm_trans_receiver_sub(frame, 0, function) <= 0) {
        fprintf(stderr, "trans sub descriptor error: malformed descriptor-only sub\n");
        status = 1;
        goto cleanup;
    }
    if (!function->is_descriptor_only) {
        fprintf(stderr, "trans sub descriptor error: descriptor-only sub must not have a body\n");
        status = 1;
        goto cleanup;
    }
    if (!lm_trans_registry_identifier_value(function->name, function_name)) {
        status = 1;
        goto cleanup;
    }

    if (
        lm_trans_registry_note_class_present(function_name) != 0 ||
        lm_trans_registry_note_class_kind(function_name, "procedure") != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("fn.descriptor"),
            function_name,
            lm_trans_text_from_cstr("procedure")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("fn.receiver"),
            function_name,
            receiver_name
        ) != 0
    ) {
        status = 1;
        goto cleanup;
    }

    if (function->params_node == 0 || function->params_node->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans sub descriptor error: parameters must be a Structure\n");
        status = 1;
        goto cleanup;
    }

    index = 0U;
    field = function->params_node->as->structure->first_field;
    while (field != 0) {
        if (
            field->value != 0 &&
            !lm_trans_node_is_ignored(field->value) &&
            lm_trans_registry_materialize_fn_descriptor_param(function_name, field->value, index) != 0
        ) {
            status = 1;
            goto cleanup;
        }
        if (field->value != 0 && !lm_trans_node_is_ignored(field->value)) {
            ++index;
        }
        field = field->next;
    }

    status = 0;

cleanup:
    lm_trans_function_header_destroy(function);
    lm_trans_l4_text_view_delete(&function_name);
    lm_trans_l4_text_view_delete(&receiver_name);
    return status;
}

static const LmL4Loader *lm_trans_registry_l4_loader_get(void) {
    return lm_trans_registry != 0 ? lm_trans_registry->l4_loader : 0;
}

static int lm_trans_registry_load_table_frame_common(
    const LmP0Frame *frame,
    int allow_node_cells,
    const char *error_context
) {
    LmTransL4LoadContext *context;
    int status;

    context = (LmTransL4LoadContext *)lm_own_new_zero(sizeof(*context));
    if (context == 0) {
        return 1;
    }
    context->allow_node_cells = allow_node_cells;
    status = lm_l4_table_from_frame(lm_trans_registry_l4_loader_get(), context, frame);
    if (status <= 0) {
        if (status == 0) {
            fprintf(
                stderr,
                "%s error: table receiver expects a table frame\n",
                error_context != 0 ? error_context : "trans registry"
            );
        }
        lm_own_delete(context, 0);
        return 1;
    }
    lm_own_delete(context, 0);
    return 0;
}

static int lm_trans_l4_receiver_table(const LmP0Frame *frame, int allow_node_cells) {
    return lm_trans_registry_load_table_frame_common(frame, allow_node_cells, "trans L4");
}

static int lm_trans_l4_receiver_join(const LmP0Frame *frame, int allow_node_cells) {
    LmTransL4LoadContext *context;
    int status;

    context = (LmTransL4LoadContext *)lm_own_new_zero(sizeof(*context));
    if (context == 0) {
        return 1;
    }
    context->allow_node_cells = allow_node_cells;
    status = lm_l4_join_from_frame(lm_trans_registry_l4_loader_get(), context, frame);
    if (status <= 0) {
        if (status == 0) {
            fprintf(stderr, "trans L4 error: join receiver expects a join frame\n");
        }
        lm_own_delete(context, 0);
        return 1;
    }
    lm_own_delete(context, 0);
    return 0;
}

static int lm_trans_l4_receiver_row(const LmP0Frame *frame, int allow_node_cells) {
    LmTransL4LoadContext *context;
    int status;

    context = (LmTransL4LoadContext *)lm_own_new_zero(sizeof(*context));
    if (context == 0) {
        return 1;
    }
    context->allow_node_cells = allow_node_cells;
    status = lm_l4_row_from_frame(lm_trans_registry_l4_loader_get(), context, frame);
    if (status <= 0) {
        if (status == 0) {
            fprintf(stderr, "trans L4 error: row receiver expects a row frame\n");
        }
        lm_own_delete(context, 0);
        return 1;
    }
    lm_own_delete(context, 0);
    return 0;
}

static int lm_trans_l4_receiver_fn_descriptor(const LmP0Frame *frame, int allow_node_cells) {
    (void)allow_node_cells;
    return lm_trans_registry_materialize_fn_descriptor_frame(frame);
}

static int lm_trans_l4_receiver_sub_descriptor(const LmP0Frame *frame, int allow_node_cells) {
    (void)allow_node_cells;
    return lm_trans_registry_materialize_sub_descriptor_frame(frame);
}

static int lm_trans_l4_atom_receiver_prelude_sequence(const LmP0Text *atom, int allow_node_cells) {
    (void)allow_node_cells;
    if (atom == 0) {
        return 1;
    }
    if (lm_trans_registry_relation_stack(atom, "item") == 0) {
        fprintf(
            stderr,
            "trans L4 error: prelude sequence \"%.*s\" has no item rows\n",
            (int)atom->length,
            atom->data
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

typedef struct LmTransL4FramePointerBinding {
    const char *head;
    const char *receiver_type;
    LmTransL4FrameHandler handler;
} LmTransL4FramePointerBinding;

typedef struct LmTransL4AtomPointerBinding {
    const char *atom;
    const char *receiver_type;
    LmTransL4AtomHandler handler;
} LmTransL4AtomPointerBinding;

static int lm_trans_l4_root_receiver_registry(const LmP0Frame *frame, int allow_node_cells);

static int lm_trans_l4_frame_pointer_binding_push(
    LmOwnPtrStack *bindings,
    const char *head,
    const char *receiver_type,
    LmTransL4FrameHandler handler
) {
    LmTransL4FramePointerBinding *binding;

    if (bindings == 0 || head == 0 || receiver_type == 0 || handler == 0) {
        return 1;
    }
    binding = (LmTransL4FramePointerBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    binding->head = head;
    binding->receiver_type = receiver_type;
    binding->handler = handler;
    if (lm_own_ptr_stack_push(bindings, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_l4_atom_pointer_binding_push(
    LmOwnPtrStack *bindings,
    const char *atom,
    const char *receiver_type,
    LmTransL4AtomHandler handler
) {
    LmTransL4AtomPointerBinding *binding;

    if (bindings == 0 || atom == 0 || receiver_type == 0 || handler == 0) {
        return 1;
    }
    binding = (LmTransL4AtomPointerBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    binding->atom = atom;
    binding->receiver_type = receiver_type;
    binding->handler = handler;
    if (lm_own_ptr_stack_push(bindings, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_l4_pointer_bindings_init(void) {
    LmOwnPtrStack *frame_bindings;
    LmOwnPtrStack *root_frame_bindings;
    LmOwnPtrStack *atom_bindings;

    if (lm_trans_registry == 0) {
        return 1;
    }
    if (
        lm_trans_registry->l4_frame_pointer_bindings != 0 ||
        lm_trans_registry->l4_root_frame_pointer_bindings != 0 ||
        lm_trans_registry->l4_atom_pointer_bindings != 0
    ) {
        return
            lm_trans_registry->l4_frame_pointer_bindings != 0 &&
            lm_trans_registry->l4_root_frame_pointer_bindings != 0 &&
            lm_trans_registry->l4_atom_pointer_bindings != 0
                ? 0
                : 1;
    }

    frame_bindings = lm_trans_ptr_stack_new(lm_own_delete_plain);
    root_frame_bindings = lm_trans_ptr_stack_new(lm_own_delete_plain);
    atom_bindings = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (frame_bindings == 0 || root_frame_bindings == 0 || atom_bindings == 0) {
        lm_trans_ptr_stack_delete(&frame_bindings);
        lm_trans_ptr_stack_delete(&root_frame_bindings);
        lm_trans_ptr_stack_delete(&atom_bindings);
        return 1;
    }
    if (
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "table", "l4.frame", lm_trans_l4_receiver_table) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "join", "l4.frame", lm_trans_l4_receiver_join) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "row", "l4.frame", lm_trans_l4_receiver_row) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "fn", "l4.frame", lm_trans_l4_receiver_fn_descriptor) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "lazy fn", "l4.frame", lm_trans_l4_receiver_fn_descriptor) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(frame_bindings, "sub", "l4.frame", lm_trans_l4_receiver_sub_descriptor) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(root_frame_bindings, "L4", "l4.root", lm_trans_l4_root_receiver_registry) != 0 ||
        lm_trans_l4_frame_pointer_binding_push(root_frame_bindings, "registry", "l4.root", lm_trans_l4_root_receiver_registry) != 0 ||
        lm_trans_l4_atom_pointer_binding_push(atom_bindings, "loadHeaders", "l4.atom", lm_trans_l4_atom_receiver_prelude_sequence) != 0
    ) {
        lm_trans_ptr_stack_delete(&frame_bindings);
        lm_trans_ptr_stack_delete(&root_frame_bindings);
        lm_trans_ptr_stack_delete(&atom_bindings);
        return 1;
    }

    lm_trans_registry->l4_frame_pointer_bindings = frame_bindings;
    lm_trans_registry->l4_root_frame_pointer_bindings = root_frame_bindings;
    lm_trans_registry->l4_atom_pointer_bindings = atom_bindings;
    return 0;
}

static const LmTransL4FramePointerBinding *lm_trans_l4_frame_pointer_binding_find(
    const LmP0Text *head,
    const LmOwnPtrStack *bindings
) {
    size_t i;
    const LmTransL4FramePointerBinding *binding;

    if (head == 0 || bindings == 0) {
        return 0;
    }

    for (i = 0U; i < bindings->count; ++i) {
        binding = (const LmTransL4FramePointerBinding *)lm_own_ptr_stack_at(bindings, i);
        if (binding != 0 && lm_trans_text_equals(head, binding->head)) {
            return binding;
        }
    }
    return 0;
}

static const LmTransL4AtomPointerBinding *lm_trans_l4_atom_pointer_binding_find(
    const LmP0Text *atom,
    const LmOwnPtrStack *bindings
) {
    size_t i;
    const LmTransL4AtomPointerBinding *binding;

    if (atom == 0 || bindings == 0) {
        return 0;
    }

    for (i = 0U; i < bindings->count; ++i) {
        binding = (const LmTransL4AtomPointerBinding *)lm_own_ptr_stack_at(bindings, i);
        if (binding != 0 && lm_trans_text_equals(atom, binding->atom)) {
            return binding;
        }
    }
    return 0;
}

static int lm_trans_l4_head_binding_resolve_from_pointer_table(
    const LmP0Text *head,
    const char *namespace_table,
    const LmOwnPtrStack *bindings,
    LmTransL4HeadBinding *out
) {
    const LmTransL4FramePointerBinding *binding;
    const char *receiver_type;

    if (head == 0 || out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));
    receiver_type = lm_trans_registry_lookup(head, namespace_table);
    if (receiver_type == 0) {
        return 0;
    }

    binding = lm_trans_l4_frame_pointer_binding_find(head, bindings);
    if (binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has no direct L4 pointer binding\n",
            namespace_table,
            (int)head->length,
            head->data
        );
        return 1;
    }

    if (strcmp(receiver_type, binding->receiver_type) != 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has receiver marker %s, expected %s\n",
            namespace_table,
            (int)head->length,
            head->data,
            receiver_type,
            binding->receiver_type
        );
        return 1;
    }

    out->receiver_type = receiver_type;
    out->receiver_binding = 0;
    out->frame = binding->handler;
    return 0;
}

static int lm_trans_l4_head_binding_resolve(
    const LmP0Text *head,
    LmTransL4HeadBinding *out
) {
    return lm_trans_l4_head_binding_resolve_from_pointer_table(
        head,
        "namespace.l4",
        lm_trans_registry != 0 ? lm_trans_registry->l4_frame_pointer_bindings : 0,
        out
    );
}

static int lm_trans_l4_root_head_binding_resolve(
    const LmP0Text *head,
    LmTransL4HeadBinding *out
) {
    return lm_trans_l4_head_binding_resolve_from_pointer_table(
        head,
        "namespace.l4.root",
        lm_trans_registry != 0 ? lm_trans_registry->l4_root_frame_pointer_bindings : 0,
        out
    );
}

static int lm_trans_l4_atom_binding_resolve_from_pointer_table(
    const LmP0Text *atom,
    const char *namespace_table,
    const LmOwnPtrStack *bindings,
    LmTransL4AtomBinding *out
) {
    const LmTransL4AtomPointerBinding *binding;
    const char *receiver_type;

    if (atom == 0 || out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));
    receiver_type = lm_trans_registry_lookup(atom, namespace_table);
    if (receiver_type == 0) {
        return 0;
    }

    binding = lm_trans_l4_atom_pointer_binding_find(atom, bindings);
    if (binding == 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has no direct L4 atom pointer binding\n",
            namespace_table,
            (int)atom->length,
            atom->data
        );
        return 1;
    }

    if (strcmp(receiver_type, binding->receiver_type) != 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: %s[\"%.*s\"] has receiver marker %s, expected %s\n",
            namespace_table,
            (int)atom->length,
            atom->data,
            receiver_type,
            binding->receiver_type
        );
        return 1;
    }

    out->receiver_type = receiver_type;
    out->receiver_binding = 0;
    out->atom = binding->handler;
    return 0;
}

static int lm_trans_l4_atom_binding_resolve(
    const LmP0Text *atom,
    LmTransL4AtomBinding *out
) {
    return lm_trans_l4_atom_binding_resolve_from_pointer_table(
        atom,
        "namespace.l4.atom",
        lm_trans_registry != 0 ? lm_trans_registry->l4_atom_pointer_bindings : 0,
        out
    );
}

static int lm_trans_registry_load_l4_frame(
    const LmP0Frame *frame,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4HeadBinding *binding;
    int status;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (frame == 0) {
        return 1;
    }
    binding = (LmTransL4HeadBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    if (lm_trans_l4_head_binding_resolve(frame->head, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    if (binding->frame == 0) {
        lm_own_delete(binding, 0);
        return 0;
    }
    status = binding->frame(frame, allow_node_cells);
    lm_own_delete(binding, 0);
    if (status != 0) {
        return 1;
    }
    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_load_l4_atom(
    const LmP0Text *atom,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4AtomBinding *binding;
    int status;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    binding = (LmTransL4AtomBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    if (lm_trans_l4_atom_binding_resolve(atom, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    if (binding->atom == 0) {
        lm_own_delete(binding, 0);
        return 0;
    }
    status = binding->atom(atom, allow_node_cells);
    lm_own_delete(binding, 0);
    if (status != 0) {
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
                if (lm_trans_registry_load_l4_atom(node->as->atom, allow_node_cells, &loaded) != 0) {
                    return 1;
                }
                if (!loaded) {
                    fprintf(stderr, "trans L4 error: unknown L4 atom receiver\n");
                    return 1;
                }
            } else if (node->kind == LM_P0_NODE_FRAME) {
                if (lm_trans_registry_load_l4_frame(node->as->frame, allow_node_cells, &loaded) != 0) {
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
    if (lm_trans_registry_load_l4_structure(frame->body, allow_node_cells) != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_registry_load_l4_root_frame(
    const LmP0Frame *frame,
    int allow_node_cells,
    int *out_loaded
) {
    LmTransL4HeadBinding *binding;
    int status;

    if (out_loaded != 0) {
        *out_loaded = 0;
    }
    if (frame == 0) {
        return 1;
    }
    binding = (LmTransL4HeadBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    if (lm_trans_l4_root_head_binding_resolve(frame->head, binding) != 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    if (binding->frame == 0) {
        lm_own_delete(binding, 0);
        return 0;
    }
    status = binding->frame(frame, allow_node_cells);
    lm_own_delete(binding, 0);
    if (status != 0) {
        return 1;
    }
    if (out_loaded != 0) {
        *out_loaded = 1;
    }
    return 0;
}

static int lm_trans_registry_l4_push_row(
    void *context,
    const LmP0Text *table_atom,
    const LmP0Text *key_atom,
    const LmP0Node *payload_node
) {
    LmTransL4LoadContext *load_context;
    LmP0Text *table_value;
    LmP0Text *key_value;
    int status;

    load_context = (LmTransL4LoadContext *)context;
    if (table_atom == 0 || key_atom == 0 || payload_node == 0) {
        return -1;
    }

    if (payload_node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_registry_push_row_atoms(table_atom, key_atom, payload_node->as->atom);
    }

    if (load_context == 0 || !load_context->allow_node_cells) {
        fprintf(stderr, "trans registry error: node row payloads are not allowed in this registry load\n");
        return -1;
    }
    table_value = lm_trans_l4_text_view_new("");
    key_value = lm_trans_l4_text_view_new("");
    if (table_value == 0 || key_value == 0) {
        lm_trans_l4_text_view_delete(&table_value);
        lm_trans_l4_text_view_delete(&key_value);
        return -1;
    }
    if (
        !lm_trans_registry_identifier_value(table_atom, table_value) ||
        !lm_trans_registry_identifier_value(key_atom, key_value)
    ) {
        lm_trans_l4_text_view_delete(&table_value);
        lm_trans_l4_text_view_delete(&key_value);
        return -1;
    }
    status = lm_trans_registry_push_row_node_values(table_value, key_value, payload_node);
    lm_trans_l4_text_view_delete(&table_value);
    lm_trans_l4_text_view_delete(&key_value);
    return status;
}

static int lm_trans_registry_l4_note_key(
    void *context,
    const LmP0Text *table_name,
    const LmL4Column *column,
    const LmP0Text *key_atom
) {
    (void)context;
    (void)table_name;
    if (key_atom == 0) {
        return -1;
    }
    if (lm_trans_registry_column_is_class_typed(column)) {
        return lm_trans_registry_note_class_present(key_atom);
    }
    return 0;
}

static int lm_trans_registry_l4_push_cell(
    void *context,
    const LmP0Text *table_name,
    const LmL4Column *column,
    int split_by_column,
    const LmP0Text *key_atom,
    const LmP0Node *payload_node
) {
    LmTransL4LoadContext *load_context;

    load_context = (LmTransL4LoadContext *)context;
    if (table_name == 0 || key_atom == 0) {
        return -1;
    }
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
        lm_trans_registry_payload_is_null(payload_node->as->atom) == 0
    ) {
        return lm_trans_registry_note_class_present(payload_node->as->atom);
    }
    return 0;
}

static int lm_trans_registry_l4_push_column_metadata(
    void *context,
    const LmP0Text *table_name,
    LmL4Column **columns,
    size_t column_count
) {
    (void)context;
    if (table_name == 0) {
        return -1;
    }
    return lm_trans_registry_push_column_metadata(table_name, columns, column_count);
}

static int lm_trans_registry_l4_join_table(
    void *context,
    const LmP0Text *source_table,
    const LmP0Text *target_table
) {
    (void)context;
    return lm_trans_registry_join_table(source_table, target_table) != 0 ? -1 : 0;
}

static LmL4FrameReceiver lm_trans_registry_l4_resolve_frame(
    const LmL4Loader *loader,
    const LmP0Text *head
) {
    const char *receiver_type;

    (void)loader;
    receiver_type = lm_trans_registry_lookup(head, "namespace.l4");
    if (receiver_type == 0) {
        return 0;
    }
    if (strcmp(receiver_type, "l4.frame") != 0) {
        fprintf(
            stderr,
            "trans registry inconsistency: namespace.l4[\"%.*s\"] has receiver marker %s, expected l4.frame\n",
            head != 0 ? (int)head->length : 0,
            head != 0 ? head->data : "",
            receiver_type
        );
        return 0;
    }
    if (lm_trans_text_equals(head, "table")) {
        return lm_l4_receiver_table;
    }
    if (lm_trans_text_equals(head, "join")) {
        return lm_l4_receiver_join;
    }
    if (lm_trans_text_equals(head, "row")) {
        return lm_l4_receiver_row;
    }
    if (lm_trans_text_equals(head, "fn") || lm_trans_text_equals(head, "lazy fn")) {
        return lm_l4_receiver_ignore;
    }
    fprintf(
        stderr,
        "trans registry inconsistency: namespace.l4[\"%.*s\"] has no direct frame receiver binding\n",
        head != 0 ? (int)head->length : 0,
        head != 0 ? head->data : ""
    );
    return 0;
}

static int lm_trans_registry_l4_dispatch_frame(
    const LmL4Loader *loader,
    void *context,
    const LmP0Frame *frame
) {
    LmL4FrameReceiver receiver;

    if (frame == 0) {
        return 1;
    }
    receiver = lm_trans_registry_l4_resolve_frame(loader, frame->head);
    if (receiver == 0) {
        fprintf(
            stderr,
            "trans registry error: registry body expects registered L4 receiver frames, got \"%.*s\"\n",
            frame->head != 0 ? (int)frame->head->length : 0,
            frame->head != 0 ? frame->head->data : ""
        );
        return 1;
    }
    return receiver(loader, context, frame);
}

static LmL4Loader *lm_trans_registry_l4_loader_new(void) {
    LmL4Loader *loader;

    loader = (LmL4Loader *)lm_own_new_zero(sizeof(*loader));
    if (loader == 0) {
        return 0;
    }
    loader->error_prefix = "trans";
    loader->push_row = lm_trans_registry_l4_push_row;
    loader->push_cell = lm_trans_registry_l4_push_cell;
    loader->note_key = lm_trans_registry_l4_note_key;
    loader->push_column_metadata = lm_trans_registry_l4_push_column_metadata;
    loader->join_table = lm_trans_registry_l4_join_table;
    loader->dispatch_frame = lm_trans_registry_l4_dispatch_frame;
    return loader;
}

static int lm_trans_registry_l4_runtime_init(void) {
    if (lm_trans_registry == 0) {
        return 1;
    }
    if (lm_trans_registry->l4_loader == 0) {
        lm_trans_registry->l4_loader = lm_trans_registry_l4_loader_new();
        if (lm_trans_registry->l4_loader == 0) {
            return 1;
        }
    }
    if (
        lm_trans_l4_pointer_bindings_init() != 0 ||
        lm_trans_l4_payload_pointer_bindings_init() != 0
    ) {
        return 1;
    }
    return 0;
}

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
    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans L4 error: root fields must be registered L4 root receiver frames, or registered L4 receiver frames in .lm4 files\n");
                return 1;
            }
            if (lm_trans_registry_load_l4_root_frame(node->as->frame, allow_node_cells, &item_loaded) != 0) {
                return 1;
            }
            if (item_loaded) {
                loaded = 1;
            } else if (implicit_l4) {
                if (lm_trans_registry_load_l4_frame(node->as->frame, allow_node_cells, &item_loaded) != 0) {
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

    for (i = 0U; i < lm_trans_registry->loaded_paths->count; ++i) {
        loaded_path = (const char *)lm_own_ptr_stack_at(lm_trans_registry->loaded_paths, i);
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

    copy = lm_trans_text_copy_cstr_from_cstr(registry_path);
    if (copy == 0) {
        return 1;
    }

    if (lm_own_ptr_stack_push(lm_trans_registry->loaded_paths, copy) != 0) {
        lm_own_delete(copy, 0);
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
    LmP0Text *path_value;
    char *candidate_name;
    char registry_path[4096];
    int loaded;

    if (frame == 0) {
        return 1;
    }

    field = frame->body->first_field;
    if (field == 0) {
        fprintf(stderr, "trans registry import error: import receiver expects at least one path\n");
        return 1;
    }

    path_value = lm_trans_l4_text_view_new("");
    if (path_value == 0) {
        return 1;
    }
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (
                node == 0 ||
                node->kind != LM_P0_NODE_ATOM ||
                !lm_trans_registry_literal_value(node->as->atom, path_value)
            ) {
                fprintf(stderr, "trans registry import error: import path must be an atom/string\n");
                lm_trans_l4_text_view_delete(&path_value);
                return 1;
            }

            candidate_name = lm_trans_text_copy_cstr(path_value);
            if (candidate_name == 0) {
                lm_trans_l4_text_view_delete(&path_value);
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
                lm_own_delete(candidate_name, 0);
                fprintf(stderr, "trans registry import error: import path is too long\n");
                lm_trans_l4_text_view_delete(&path_value);
                return 1;
            }
            lm_own_delete(candidate_name, 0);

            if (!lm_trans_registry_path_already_loaded(registry_path)) {
                if (lm_trans_registry_load_file_path(registry_path, 1, &loaded) != 0) {
                    lm_trans_l4_text_view_delete(&path_value);
                    return 1;
                }
                if (!loaded) {
                    fprintf(stderr, "trans registry import error: cannot read %s\n", registry_path);
                    lm_trans_l4_text_view_delete(&path_value);
                    return 1;
                }
            }
        }
        field = field->next;
    }

    lm_trans_l4_text_view_delete(&path_value);
    return 0;
}

typedef struct LmTransRegistryImportFrame {
    const LmP0Field *field;
    int descend_l2;
} LmTransRegistryImportFrame;

static LmTransRegistryImportFrame *lm_trans_registry_import_frame_new(
    const LmP0Field *field,
    int descend_l2
) {
    LmTransRegistryImportFrame *frame;

    frame = (LmTransRegistryImportFrame *)lm_own_new_zero(sizeof(*frame));
    if (frame != 0) {
        frame->field = field;
        frame->descend_l2 = descend_l2;
    }
    return frame;
}

static int lm_trans_registry_load_imports_from_structure(
    const LmP0Structure *structure,
    const char *source_path,
    int descend_l2
) {
    LmOwnPtrStack *stack;
    LmTransRegistryImportFrame *frame;
    const LmP0Node *node;
    LmTransHeadBinding *binding;

    binding = (LmTransHeadBinding *)lm_own_new_zero(sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    if (structure == 0 || structure->first_field == 0) {
        lm_own_delete(binding, 0);
        return 0;
    }

    stack = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (stack == 0) {
        lm_own_delete(binding, 0);
        return 1;
    }
    frame = lm_trans_registry_import_frame_new(structure->first_field, descend_l2);
    if (frame == 0) {
        lm_trans_ptr_stack_delete(&stack);
        lm_own_delete(binding, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        lm_trans_ptr_stack_delete(&stack);
        lm_own_delete(binding, 0);
        return 1;
    }

    while (stack->count != 0U) {
        frame = (LmTransRegistryImportFrame *)lm_own_ptr_stack_top(stack);
        if (frame == 0 || frame->field == 0) {
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            continue;
        }

        node = frame->field->value;
        frame->field = frame->field->next;
        if (
            node != 0 &&
            !lm_trans_node_is_ignored(node) &&
            node->kind == LM_P0_NODE_FRAME
        ) {
            if (lm_trans_head_binding_resolve(0, node->as->frame->head, binding) != 0) {
                lm_trans_ptr_stack_delete(&stack);
                lm_own_delete(binding, 0);
                return 1;
            }
            if (binding->statement_frame == lm_trans_statement_emit_import) {
                if (lm_trans_registry_load_import_frame(node->as->frame, source_path) != 0) {
                    lm_trans_ptr_stack_delete(&stack);
                    lm_own_delete(binding, 0);
                    return 1;
                }
            } else if (
                frame->descend_l2 &&
                lm_trans_frame_is_l2_level_receiver(node->as->frame) &&
                node->as->frame->body != 0 &&
                node->as->frame->body->first_field != 0
            ) {
                frame = lm_trans_registry_import_frame_new(node->as->frame->body->first_field, 0);
                if (frame == 0) {
                    lm_trans_ptr_stack_delete(&stack);
                    lm_own_delete(binding, 0);
                    return 1;
                }
                if (lm_own_ptr_stack_push(stack, frame) != 0) {
                    lm_own_delete(frame, 0);
                    lm_trans_ptr_stack_delete(&stack);
                    lm_own_delete(binding, 0);
                    return 1;
                }
            }
        }
    }

    lm_trans_ptr_stack_delete(&stack);
    lm_own_delete(binding, 0);
    return 0;
}

static int lm_trans_registry_load_inline_root(const LmP0Node *root, const char *source_path) {
    const LmP0Field *field;
    const LmP0Node *node;
    int loaded;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    if (lm_trans_registry_load_imports_from_structure(root->as->structure, source_path, 1) != 0) {
        return 1;
    }

    field = root->as->structure->first_field;
    while (field != 0) {
        node = field->value;
        if (
            node != 0 &&
            !lm_trans_node_is_ignored(node) &&
            node->kind == LM_P0_NODE_FRAME
        ) {
            if (lm_trans_registry_load_l4_root_frame(node->as->frame, 1, &loaded) != 0) {
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
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("table"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("join"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("row"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("fn"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("lazy fn"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4"),
            lm_trans_text_from_cstr("sub"),
            lm_trans_text_from_cstr("l4.frame")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4.root"),
            lm_trans_text_from_cstr("L4"),
            lm_trans_text_from_cstr("l4.root")
        ) != 0 ||
        lm_trans_registry_push_row_values(
            lm_trans_text_from_cstr("namespace.l4.root"),
            lm_trans_text_from_cstr("registry"),
            lm_trans_text_from_cstr("l4.root")
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
    if (lm_trans_registry_init() != 0) {
        return 1;
    }
    lm_trans_registry->loaded = 1;
    lm_trans_registry->identifiers = lm_trans_identifier_table_new();
    lm_trans_registry->value_arena = (LmOwnArena *)lm_own_new_zero(sizeof(*lm_trans_registry->value_arena));
    lm_trans_registry->loaded_paths = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (
        lm_trans_registry->identifiers == 0 ||
        lm_trans_registry->value_arena == 0 ||
        lm_own_arena_init(lm_trans_registry->value_arena) != 0 ||
        lm_trans_registry->loaded_paths == 0
    ) {
        lm_trans_registry_destroy();
        return 1;
    }
    lm_trans_registry->source_path = lm_trans_text_copy_cstr_from_cstr(source_path);
    if (lm_trans_registry->source_path == 0) {
        lm_trans_registry_destroy();
        return 1;
    }
    lm_trans_registry->loaded_fact_count = 0U;

    if (lm_trans_registry_l4_runtime_init() != 0) {
        lm_trans_registry_destroy();
        return 1;
    }

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
        lm_own_delete(prelude_path, 0);
        lm_own_delete(body_path, 0);
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
        lm_own_delete(prelude_path, 0);
        lm_own_delete(body_path, 0);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    lm_trans_prelude_output = prelude_output;
    lm_trans_current_source_path = source_path;
    lm_trans_next_array_value_helper_id = 0U;
    emitted = 0;
    lm_trans_declared_import_paths = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_declared_import_documents = lm_trans_ptr_stack_new(lm_trans_import_document_delete);
    lm_trans_emitted_import_prelude_paths = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_emitted_import_function_paths = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_emitted_callable_adapters = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_emitted_callable_binders = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_emitted_array_value_helpers = lm_trans_ptr_stack_new(lm_own_delete_plain);
    lm_trans_emitted_function_return_structs = lm_trans_ptr_stack_new(lm_own_delete_plain);
    if (
        lm_trans_declared_import_paths == 0 ||
        lm_trans_declared_import_documents == 0 ||
        lm_trans_emitted_import_prelude_paths == 0 ||
        lm_trans_emitted_import_function_paths == 0 ||
        lm_trans_emitted_callable_adapters == 0 ||
        lm_trans_emitted_callable_binders == 0 ||
        lm_trans_emitted_array_value_helpers == 0 ||
        lm_trans_emitted_function_return_structs == 0
    ) {
        fprintf(stderr, "trans error: out of memory while creating module state stacks\n");
        status = 1;
    } else {
        status = lm_trans_emit_module_prelude_plan(prelude_output);
    }
    if (status == 0) {
        status = lm_trans_emit_root_sequence(
            body_output,
            root,
            lm_trans_path_has_extension(source_path, ".lm2"),
            &emitted
        );
    }
    lm_trans_ptr_stack_delete(&lm_trans_emitted_function_return_structs);
    lm_trans_ptr_stack_delete(&lm_trans_emitted_array_value_helpers);
    lm_trans_ptr_stack_delete(&lm_trans_emitted_callable_binders);
    lm_trans_ptr_stack_delete(&lm_trans_emitted_callable_adapters);
    lm_trans_ptr_stack_delete(&lm_trans_emitted_import_function_paths);
    lm_trans_ptr_stack_delete(&lm_trans_emitted_import_prelude_paths);
    lm_trans_ptr_stack_delete(&lm_trans_declared_import_documents);
    lm_trans_ptr_stack_delete(&lm_trans_declared_import_paths);
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
    lm_own_delete(prelude_path, 0);
    lm_own_delete(body_path, 0);
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


static int lm_trans_text_equals(const LmP0Text *text, const char *value);
static int lm_trans_text_same(const LmP0Text *left, const LmP0Text *right);
static int lm_trans_text_starts_with(const LmP0Text *text, const char *prefix);
static int lm_trans_identifier_payload(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_identifier_same(const LmP0Text *left, const LmP0Text *right);
static int lm_trans_emit_identifier(FILE *file, const LmP0Text *name);
static int lm_trans_registry_identifier_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_registry_literal_value(const LmP0Text *atom, LmP0Text *out_payload);
static int lm_trans_registry_payload_is_null(const LmP0Text *atom);
static char * lm_trans_text_copy_cstr(const LmP0Text *text);
static char * lm_trans_text_copy_cstr_from_cstr(const char *text);
static char * lm_trans_registry_value_copy_cstr(const LmP0Text *value);
static int lm_trans_registry_clone_text(const LmP0Text *source, LmP0Text *out_text);
static LmP0Text * lm_trans_text_from_cstr(const char *text);
static int lm_trans_text_assign_cstr(LmP0Text *out_text, const char *text);
static int lm_trans_text_all_char(const LmP0Text *text, char ch);
static int lm_trans_write_all(FILE *file, const char *data, size_t length);
static int lm_trans_put(FILE *file, const char *text);
static FILE * lm_trans_prelude_file(FILE *fallback);
static int lm_trans_write_text(FILE *file, const LmP0Text *text);
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


static void lm_trans_import_document_delete(void *document) {
    lm_p0_document_destroy(((LmP0Document *)document));
}

static LmP0Text * lm_trans_registry_new_text(void) {
    LmP0Text *text;
    text = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(text[0]));
    if (text != 0) {
        text->data = "";
        text->length = 0U;
    }
    return text;
}

static LmP0Structure * lm_trans_registry_new_structure(void) {
    return lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(LmP0Structure));
}

static LmP0Frame * lm_trans_registry_new_frame(void) {
    LmP0Frame *frame;
    frame = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(frame[0]));
    if (frame == 0) {
        return 0;
    }
    frame->head = lm_trans_registry_new_text();
    frame->body = lm_trans_registry_new_structure();
    if (frame -> head == 0 || frame -> body == 0) {
        return 0;
    }
    return frame;
}

static LmP0Trailer * lm_trans_registry_new_trailer(void) {
    LmP0Trailer *trailer;
    trailer = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(trailer[0]));
    if (trailer == 0) {
        return 0;
    }
    trailer->spelling = lm_trans_registry_new_text();
    trailer->body = lm_trans_registry_new_structure();
    if (trailer -> spelling == 0 || trailer -> body == 0) {
        return 0;
    }
    return trailer;
}

static LmTransRegistryCloneFrame * lm_trans_registry_clone_frame_new_structure(const LmP0Structure *source, LmP0Structure *copy) {
    LmTransRegistryCloneFrame *frame;
    frame = lm_own_new_zero(sizeof(frame[0]));
    if (frame == 0) {
        return 0;
    }
    frame->phase = LM_TRANS_REGISTRY_CLONE_STRUCTURE;
    frame->source_structure = source;
    frame->copy_structure = copy;
    return frame;
}

static LmTransRegistryCloneFrame * lm_trans_registry_clone_frame_new_trailer(const LmP0Trailer *source, LmP0Trailer **copy_slot) {
    LmTransRegistryCloneFrame *frame;
    frame = lm_own_new_zero(sizeof(frame[0]));
    if (frame == 0) {
        return 0;
    }
    frame->phase = LM_TRANS_REGISTRY_CLONE_TRAILER;
    frame->source_trailer = source;
    frame->copy_trailer_slot = copy_slot;
    return frame;
}

static int lm_trans_registry_clone_push_frame(LmOwnPtrStack *stack, LmTransRegistryCloneFrame *frame) {
    if (stack == 0 || frame == 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    return 0;
}

static int lm_trans_registry_clone_push_structure(LmOwnPtrStack *stack, const LmP0Structure *source, LmP0Structure *copy) {
    return lm_trans_registry_clone_push_frame(stack, lm_trans_registry_clone_frame_new_structure(source, copy));
}

static int lm_trans_registry_clone_push_trailer(LmOwnPtrStack *stack, const LmP0Trailer *source, LmP0Trailer **copy_slot) {
    if (copy_slot == 0) {
        return 1;
    }
    *(copy_slot) = 0;
    if (source == 0) {
        return 0;
    }
    return lm_trans_registry_clone_push_frame(stack, lm_trans_registry_clone_frame_new_trailer(source, copy_slot));
}

static int lm_trans_registry_clone_process_structure(const LmP0Structure *source, LmP0Structure *copy, LmOwnPtrStack *stack) {
    const LmP0Field *source_field;
    LmP0Field *copy_field;
    LmP0Field *previous_field;
    if (copy == 0) {
        return 1;
    }
    memset(copy, 0, sizeof(copy[0]));
    if (source == 0) {
        return 0;
    }
    previous_field = 0;
    source_field = source -> first_field;
    while (source_field != 0) {
        copy_field = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(copy_field[0]));
        if (copy_field == 0) {
            return 1;
        }
        copy_field->value = lm_trans_registry_clone_node_shallow(source_field -> value, stack);
        if (source_field -> value != 0 && copy_field -> value == 0) {
            return 1;
        }
        if (previous_field == 0) {
            copy->first_field = copy_field;
        }
        else {
            previous_field->next = copy_field;
        }
        previous_field = copy_field;
        copy->last_field = copy_field;
        copy->field_count = copy -> field_count + 1U;
        source_field = source_field -> next;
    }
    if (lm_trans_registry_clone_push_trailer(stack, source -> trailer, &copy -> trailer) != 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_registry_clone_process_trailer(const LmP0Trailer *source, LmP0Trailer **copy_slot, LmOwnPtrStack *stack) {
    LmP0Trailer *copy;
    if (copy_slot == 0) {
        return 1;
    }
    *(copy_slot) = 0;
    if (source == 0) {
        return 0;
    }
    copy = lm_trans_registry_new_trailer();
    if (copy == 0) {
        return 1;
    }
    *(copy_slot) = copy;
    copy->flags = source -> flags;
    if (lm_trans_registry_clone_text(source -> spelling, copy -> spelling) != 0) {
        return 1;
    }
    return lm_trans_registry_clone_push_structure(stack, source -> body, copy -> body);
}

static LmP0Node * lm_trans_registry_clone_node_shallow(const LmP0Node *source, LmOwnPtrStack *stack) {
    LmP0Node *copy;
    if (source == 0) {
        return 0;
    }
    copy = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(copy[0]));
    if (copy == 0) {
        return 0;
    }
    copy->kind = source -> kind;
    copy->flags = source -> flags;
    copy->span = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(copy -> span[0]));
    if (copy -> span == 0) {
        return 0;
    }
    if (source -> span != 0) {
        memcpy(copy -> span, source -> span, sizeof(copy -> span[0]));
    }
    copy->as = lm_own_arena_new_zero(lm_trans_registry->value_arena, sizeof(copy -> as[0]));
    if (copy -> as == 0) {
        return 0;
    }
    if (source -> kind == LM_P0_NODE_ATOM || source -> kind == LM_P0_NODE_DISABLED) {
        copy->as->atom = lm_trans_registry_new_text();
        if (copy -> as -> atom == 0 || lm_trans_registry_clone_text(source -> as -> atom, copy -> as -> atom) != 0) {
            return 0;
        }
        return copy;
    }
    if (source -> kind == LM_P0_NODE_FRAME) {
        copy->as->frame = lm_trans_registry_new_frame();
        if (copy -> as -> frame == 0) {
            return 0;
        }
        copy->as->frame->flags = source -> as -> frame -> flags;
        if (lm_trans_registry_clone_text(source -> as -> frame -> head, copy -> as -> frame -> head) != 0 || lm_trans_registry_clone_push_structure(stack, source -> as -> frame -> body, copy -> as -> frame -> body) != 0 || lm_trans_registry_clone_push_trailer(stack, source -> as -> frame -> trailer, &copy -> as -> frame -> trailer) != 0) {
            return 0;
        }
        return copy;
    }
    if (source -> kind == LM_P0_NODE_STRUCTURE) {
        copy->as->structure = lm_trans_registry_new_structure();
        if (copy -> as -> structure == 0) {
            return 0;
        }
        if (lm_trans_registry_clone_push_structure(stack, source -> as -> structure, copy -> as -> structure) != 0) {
            return 0;
        }
        return copy;
    }
    return 0;
}

static int lm_trans_registry_clone_run(LmOwnPtrStack *stack) {
    LmTransRegistryCloneFrame *frame;
    int status;
    int handled;
    status = 0;
    while (status == 0 && stack != 0 && stack -> count != 0U) {
        frame = lm_own_ptr_stack_pop(stack);
        if (frame == 0) {
            status = 1;
            break;
        }
        handled = 0;
        if (frame -> phase == LM_TRANS_REGISTRY_CLONE_STRUCTURE) {
            status = lm_trans_registry_clone_process_structure(frame -> source_structure, frame -> copy_structure, stack);
            handled = 1;
        }
        if (handled == 0 && frame -> phase == LM_TRANS_REGISTRY_CLONE_TRAILER) {
            status = lm_trans_registry_clone_process_trailer(frame -> source_trailer, frame -> copy_trailer_slot, stack);
            handled = 1;
        }
        if (handled == 0) {
            status = 1;
        }
        lm_own_delete(frame, 0);
    }
    return status;
}

static LmOwnPtrStack * lm_trans_registry_clone_stack_new(void) {
    LmOwnPtrStack *stack;
    stack = lm_own_new_zero(sizeof(stack[0]));
    if (stack == 0) {
        return 0;
    }
    lm_own_ptr_stack_init(stack, lm_own_delete_plain);
    return stack;
}

static void lm_trans_registry_clone_stack_delete(LmOwnPtrStack **stack) {
    if (stack != 0 && * stack != 0) {
        lm_own_ptr_stack_destroy(* stack);
        lm_own_delete(* stack, 0);
        *(stack) = 0;
    }
}

static LmP0Trailer * lm_trans_registry_clone_trailer(const LmP0Trailer *source) {
    LmOwnPtrStack *stack;
    LmP0Trailer *copy;
    if (source == 0) {
        return 0;
    }
    stack = lm_trans_registry_clone_stack_new();
    if (stack == 0) {
        return 0;
    }
    copy = 0;
    if (lm_trans_registry_clone_push_trailer(stack, source, &copy) != 0 || lm_trans_registry_clone_run(stack) != 0) {
        lm_trans_registry_clone_stack_delete(&stack);
        return 0;
    }
    lm_trans_registry_clone_stack_delete(&stack);
    return copy;
}

static int lm_trans_registry_clone_structure(const LmP0Structure *source, LmP0Structure *out_structure) {
    LmOwnPtrStack *stack;
    int status;
    stack = lm_trans_registry_clone_stack_new();
    if (stack == 0) {
        return 1;
    }
    status = lm_trans_registry_clone_push_structure(stack, source, out_structure) || lm_trans_registry_clone_run(stack);
    lm_trans_registry_clone_stack_delete(&stack);
    return status;
}

static LmP0Node * lm_trans_registry_clone_node(const LmP0Node *source) {
    LmOwnPtrStack *stack;
    LmP0Node *copy;
    if (source == 0) {
        return 0;
    }
    stack = lm_trans_registry_clone_stack_new();
    if (stack == 0) {
        return 0;
    }
    copy = lm_trans_registry_clone_node_shallow(source, stack);
    if (copy == 0 || lm_trans_registry_clone_run(stack) != 0) {
        lm_trans_registry_clone_stack_delete(&stack);
        return 0;
    }
    lm_trans_registry_clone_stack_delete(&stack);
    return copy;
}

static int lm_trans_registry_index_row_relation(const LmP0Text *card_name, const char *relation_name, LmTransRegistryFact *row) {
    LmTransRegistry *registry;
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;
    if (row == 0 || relation_name == 0) {
        return 1;
    }
    registry = lm_trans_registry;
    card = lm_trans_identifier_table_get_card(registry -> identifiers, card_name);
    if (card == 0) {
        return 1;
    }
    relation = lm_trans_identifier_card_get_relation(card, relation_name);
    if (relation == 0) {
        return 1;
    }
    return lm_own_ptr_stack_push(relation -> symbols, row);
}

static int lm_trans_registry_index_row_table_view(LmTransRegistryFact *row) {
    const char *dot;
    LmP0Text *table_name;
    const char *relation_name;
    int status;
    if (row == 0 || row -> table == 0) {
        return 1;
    }
    table_name = lm_trans_text_ref_new_cstr("");
    if (table_name == 0) {
        return 1;
    }
    dot = strchr(row -> table, '.');
    if (dot != 0 && dot != row -> table && dot[1] != '\0') {
        table_name->data = row -> table;
        table_name->length = dot - row -> table;
        relation_name = dot + 1U;
    }
    else {
        table_name->data = row -> table;
        table_name->length = strlen(row -> table);
        relation_name = "row";
    }
    status = lm_trans_registry_index_row_relation(table_name, relation_name, row);
    lm_trans_text_ref_destroy(&table_name);
    return status;
}

static int lm_trans_registry_index_row(LmTransRegistryFact *row) {
    LmTransRegistry *registry;
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;
    LmP0Text *key_name;
    int status;
    if (row == 0 || row -> table == 0 || row -> key == 0) {
        return 1;
    }
    key_name = lm_trans_text_from_cstr(row -> key);
    if (key_name == 0) {
        return 1;
    }
    registry = lm_trans_registry;
    card = lm_trans_identifier_table_get_card(registry -> identifiers, key_name);
    if (card == 0) {
        lm_trans_text_ref_destroy(&key_name);
        return 1;
    }
    relation = lm_trans_identifier_card_get_relation(card, row -> table);
    if (relation == 0) {
        lm_trans_text_ref_destroy(&key_name);
        return 1;
    }
    if (lm_own_ptr_stack_push(relation -> symbols, row) != 0) {
        lm_trans_text_ref_destroy(&key_name);
        return 1;
    }
    status = lm_trans_registry_index_row_table_view(row);
    lm_trans_text_ref_destroy(&key_name);
    return status;
}

static int lm_trans_registry_push_row_values(const LmP0Text *table_value, const LmP0Text *key_value, const LmP0Text *payload_value) {
    LmTransRegistry *registry;
    LmTransRegistryFact *row;
    registry = lm_trans_registry;
    row = lm_own_arena_new_zero(registry -> value_arena, sizeof(row[0]));
    if (row == 0) {
        return - 1;
    }
    row->table = lm_trans_registry_value_copy_cstr(table_value);
    row->key = lm_trans_registry_value_copy_cstr(key_value);
    row->payload = lm_trans_registry_value_copy_cstr(payload_value);
    if (row -> table == 0 || row -> key == 0 || row -> payload == 0) {
        return - 1;
    }
    if (lm_trans_registry_index_row(row) != 0) {
        return - 1;
    }
    registry->loaded_fact_count = registry -> loaded_fact_count + 1U;
    return 0;
}

static int lm_trans_registry_push_row_node_values(const LmP0Text *table_value, const LmP0Text *key_value, const LmP0Node *payload_node) {
    LmTransRegistry *registry;
    LmTransRegistryFact *row;
    LmP0Node *payload_copy;
    if (payload_node == 0) {
        return - 1;
    }
    registry = lm_trans_registry;
    row = lm_own_arena_new_zero(registry -> value_arena, sizeof(row[0]));
    if (row == 0) {
        return - 1;
    }
    payload_copy = lm_trans_registry_clone_node(payload_node);
    if (payload_copy == 0) {
        return - 1;
    }
    row->table = lm_trans_registry_value_copy_cstr(table_value);
    row->key = lm_trans_registry_value_copy_cstr(key_value);
    row->payload_node = payload_copy;
    if (row -> table == 0 || row -> key == 0) {
        return - 1;
    }
    if (lm_trans_registry_index_row(row) != 0) {
        return - 1;
    }
    registry->loaded_fact_count = registry -> loaded_fact_count + 1U;
    return 0;
}

static int lm_trans_registry_join_relation_matches(const char *relation_name, const LmP0Text *source_name, const char **out_suffix) {
    size_t relation_length;
    if (relation_name == 0 || source_name == 0 || source_name -> data == 0 || out_suffix == 0) {
        return 0;
    }
    relation_length = strlen(relation_name);
    if (relation_length == source_name -> length && memcmp(relation_name, source_name -> data, source_name -> length) == 0) {
        *(out_suffix) = "";
        return 1;
    }
    if (relation_length > source_name -> length + 1U && memcmp(relation_name, source_name -> data, source_name -> length) == 0 && relation_name[source_name -> length] == '.') {
        *(out_suffix) = relation_name + source_name -> length;
        return 1;
    }
    return 0;
}

static char * lm_trans_registry_join_target_relation_new(const LmP0Text *target_name, const char *suffix) {
    size_t suffix_length;
    size_t length;
    char *relation_name;
    if (target_name == 0 || target_name -> data == 0 || suffix == 0) {
        return 0;
    }
    suffix_length = strlen(suffix);
    length = target_name -> length + suffix_length;
    relation_name = lm_own_new_zero(length + 1U);
    if (relation_name == 0) {
        return 0;
    }
    if (target_name -> length != 0U) {
        memcpy(relation_name, target_name -> data, target_name -> length);
    }
    if (suffix_length != 0U) {
        memcpy(relation_name + target_name -> length, suffix, suffix_length);
    }
    relation_name[length] = '\0';
    return relation_name;
}

static int lm_trans_registry_join_copy_row(const LmTransRegistryFact *row, const LmP0Text *source_name, const LmP0Text *target_name, size_t *copied) {
    const char *suffix;
    char *target_relation;
    LmP0Text *target_relation_text;
    LmP0Text *key_text;
    LmP0Text *payload_text;
    int status;
    if (row == 0 || row -> table == 0 || row -> key == 0 || (row -> payload_node == 0 && row -> payload == 0)) {
        return 0;
    }
    if (copied == 0 || lm_trans_registry_join_relation_matches(row -> table, source_name, &suffix) == 0) {
        return 0;
    }
    target_relation = lm_trans_registry_join_target_relation_new(target_name, suffix);
    if (target_relation == 0) {
        return 1;
    }
    target_relation_text = lm_trans_text_from_cstr(target_relation);
    key_text = lm_trans_text_from_cstr(row -> key);
    payload_text = 0;
    if (row -> payload != 0) {
        payload_text = lm_trans_text_from_cstr(row -> payload);
    }
    status = 1;
    if (target_relation_text != 0 && key_text != 0) {
        if (row -> payload_node != 0) {
            status = lm_trans_registry_push_row_node_values(target_relation_text, key_text, row -> payload_node) != 0;
        }
        else {
            if (payload_text != 0) {
                status = lm_trans_registry_push_row_values(target_relation_text, key_text, payload_text) != 0;
            }
        }
    }
    lm_trans_text_ref_destroy(&payload_text);
    lm_trans_text_ref_destroy(&key_text);
    lm_trans_text_ref_destroy(&target_relation_text);
    lm_own_delete(target_relation, 0);
    if (status != 0) {
        return 1;
    }
    copied[0] = copied[0] + 1U;
    return 0;
}

static int lm_trans_registry_join_table(const LmP0Text *source_table, const LmP0Text *target_table) {
    LmP0Text *source_name;
    LmP0Text *target_name;
    LmTransIdentifierTable *identifiers;
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;
    LmTransRegistryFact *row;
    const char *relation_suffix;
    size_t bucket_index;
    size_t relation_bucket_index;
    size_t symbol_index;
    size_t symbol_count;
    size_t copied;
    int status;
    if (lm_trans_registry == 0 || source_table == 0 || target_table == 0) {
        return 1;
    }
    source_name = lm_trans_text_ref_new_cstr("");
    target_name = lm_trans_text_ref_new_cstr("");
    if (source_name == 0 || target_name == 0) {
        lm_trans_text_ref_destroy(&source_name);
        lm_trans_text_ref_destroy(&target_name);
        return 1;
    }
    if (lm_trans_registry_identifier_value(source_table, source_name) == 0 || lm_trans_registry_identifier_value(target_table, target_name) == 0) {
        lm_trans_text_ref_destroy(&source_name);
        lm_trans_text_ref_destroy(&target_name);
        return 1;
    }
    identifiers = lm_trans_registry->identifiers;
    copied = 0U;
    status = 0;
    if (identifiers != 0 && identifiers -> buckets != 0) {
        bucket_index = 0U;
        while (status == 0 && bucket_index < identifiers -> bucket_count) {
            card = identifiers -> buckets[bucket_index];
            while (status == 0 && card != 0) {
                if (card -> relation_buckets != 0) {
                    relation_bucket_index = 0U;
                    while (status == 0 && relation_bucket_index < card -> relation_bucket_count) {
                        relation = card -> relation_buckets[relation_bucket_index];
                        while (status == 0 && relation != 0) {
                            relation_suffix = 0;
                            if (relation -> symbols != 0 && lm_trans_registry_join_relation_matches(relation -> name, source_name, &relation_suffix) != 0) {
                                symbol_count = relation -> symbols -> count;
                                symbol_index = 0U;
                                while (status == 0 && symbol_index < symbol_count) {
                                    row = lm_own_ptr_stack_at(relation -> symbols, symbol_index);
                                    status = lm_trans_registry_join_copy_row(row, source_name, target_name, &copied);
                                    symbol_index = symbol_index + 1U;
                                }
                            }
                            relation = relation -> next;
                        }
                        relation_bucket_index = relation_bucket_index + 1U;
                    }
                }
            }
            bucket_index = bucket_index + 1U;
        }
    }
    if (status == 0 && copied == 0U) {
        fprintf(stderr, "trans registry error: join source table \"%.*s\" has no rows\n", (((int)source_name -> length)), source_name -> data);
        status = 1;
    }
    lm_trans_text_ref_destroy(&source_name);
    lm_trans_text_ref_destroy(&target_name);
    return status;
}

static int lm_trans_registry_push_row_atoms(const LmP0Text *table_atom, const LmP0Text *key_atom, const LmP0Text *payload_atom) {
    LmP0Text *table_value;
    LmP0Text *key_value;
    LmP0Text *payload_value;
    int status;
    if (lm_trans_registry_payload_is_null(payload_atom) != 0) {
        return 0;
    }
    table_value = lm_trans_text_ref_new_cstr("");
    key_value = lm_trans_text_ref_new_cstr("");
    payload_value = lm_trans_text_ref_new_cstr("");
    if (table_value == 0 || key_value == 0 || payload_value == 0) {
        lm_trans_text_ref_destroy(&table_value);
        lm_trans_text_ref_destroy(&key_value);
        lm_trans_text_ref_destroy(&payload_value);
        return - 1;
    }
    status = - 1;
    if (lm_trans_registry_identifier_value(table_atom, table_value) != 0 && lm_trans_registry_identifier_value(key_atom, key_value) != 0 && lm_trans_registry_literal_value(payload_atom, payload_value) != 0) {
        status = lm_trans_registry_push_row_values(table_value, key_value, payload_value);
    }
    lm_trans_text_ref_destroy(&table_value);
    lm_trans_text_ref_destroy(&key_value);
    lm_trans_text_ref_destroy(&payload_value);
    return status;
}

static int lm_trans_registry_column_has_descriptor(const LmTransRegistryColumn *column, const char *descriptor) {
    size_t i;
    LmP0Text *payload;
    if (column == 0 || descriptor == 0) {
        return 0;
    }
    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 0;
    }
    i = 0U;
    while (i < column -> descriptor_count) {
        if (column -> descriptors[i] != 0 && lm_trans_registry_identifier_value(column -> descriptors[i], payload) != 0 && lm_trans_text_equals(payload, descriptor) != 0) {
            lm_trans_text_ref_destroy(&payload);
            return 1;
        }
        i = i + 1U;
    }
    lm_trans_text_ref_destroy(&payload);
    return 0;
}

static int lm_trans_registry_column_is_class_typed(const LmTransRegistryColumn *column) {
    LmP0Text *payload;
    int is_class;
    if (column == 0) {
        return 0;
    }
    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 0;
    }
    is_class = 0;
    if (column -> name != 0 && lm_trans_registry_identifier_value(column -> name, payload) != 0 && lm_trans_text_equals(payload, "class") != 0) {
        is_class = 1;
    }
    lm_trans_text_ref_destroy(&payload);
    return is_class || lm_trans_registry_column_has_descriptor(column, "class");
}

static int lm_trans_registry_cell_value(const LmP0Text *atom, const LmTransRegistryColumn *column, LmP0Text *out_value) {
    if (lm_trans_registry_payload_is_null(atom) != 0) {
        return 0;
    }
    if (lm_trans_registry_column_has_descriptor(column, "char") != 0) {
        if (lm_trans_registry_literal_value(atom, out_value) != 0) {
            return 1;
        }
        return - 1;
    }
    if (lm_trans_registry_identifier_value(atom, out_value) != 0) {
        return 1;
    }
    return - 1;
}

static int lm_trans_registry_push_table_cell(const LmP0Text *table_name, const LmTransRegistryColumn *column, int split_by_column, const LmP0Text *key_atom, const LmP0Node *payload_node, int allow_node_cells) {
    LmP0Text *table_payload;
    LmP0Text *column_payload;
    LmP0Text *key_payload;
    LmP0Text *payload_value;
    LmP0Text *relation_table;
    char *relation_name;
    size_t relation_length;
    int status;
    int cell_status;
    int wants_node;
    if (payload_node == 0 || column == 0) {
        return - 1;
    }
    table_payload = lm_trans_text_ref_new_cstr("");
    column_payload = lm_trans_text_ref_new_cstr("");
    key_payload = lm_trans_text_ref_new_cstr("");
    payload_value = lm_trans_text_ref_new_cstr("");
    relation_table = 0;
    relation_name = 0;
    status = - 1;
    if (table_payload == 0 || column_payload == 0 || key_payload == 0 || payload_value == 0) {
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    wants_node = lm_trans_registry_column_has_descriptor(column, "node");
    if (wants_node == 0) {
        if (payload_node -> kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans registry error: non-atom table cell requires a node-typed column\n");
            lm_own_delete(relation_name, 0);
            lm_trans_text_ref_destroy(&relation_table);
            lm_trans_text_ref_destroy(&table_payload);
            lm_trans_text_ref_destroy(&column_payload);
            lm_trans_text_ref_destroy(&key_payload);
            lm_trans_text_ref_destroy(&payload_value);
            return status;
        }
        cell_status = lm_trans_registry_cell_value(payload_node -> as -> atom, column, payload_value);
        if (cell_status == 0) {
            status = 0;
            lm_own_delete(relation_name, 0);
            lm_trans_text_ref_destroy(&relation_table);
            lm_trans_text_ref_destroy(&table_payload);
            lm_trans_text_ref_destroy(&column_payload);
            lm_trans_text_ref_destroy(&key_payload);
            lm_trans_text_ref_destroy(&payload_value);
            return status;
        }
        if (cell_status < 0) {
            lm_own_delete(relation_name, 0);
            lm_trans_text_ref_destroy(&relation_table);
            lm_trans_text_ref_destroy(&table_payload);
            lm_trans_text_ref_destroy(&column_payload);
            lm_trans_text_ref_destroy(&key_payload);
            lm_trans_text_ref_destroy(&payload_value);
            return status;
        }
    }
    else {
        if (allow_node_cells == 0) {
            fprintf(stderr, "trans registry error: node table cells are allowed only in inline L4 for now\n");
            lm_own_delete(relation_name, 0);
            lm_trans_text_ref_destroy(&relation_table);
            lm_trans_text_ref_destroy(&table_payload);
            lm_trans_text_ref_destroy(&column_payload);
            lm_trans_text_ref_destroy(&key_payload);
            lm_trans_text_ref_destroy(&payload_value);
            return status;
        }
        if (payload_node -> kind == LM_P0_NODE_ATOM && lm_trans_registry_payload_is_null(payload_node -> as -> atom) != 0) {
            status = 0;
            lm_own_delete(relation_name, 0);
            lm_trans_text_ref_destroy(&relation_table);
            lm_trans_text_ref_destroy(&table_payload);
            lm_trans_text_ref_destroy(&column_payload);
            lm_trans_text_ref_destroy(&key_payload);
            lm_trans_text_ref_destroy(&payload_value);
            return status;
        }
    }
    if (lm_trans_registry_identifier_value(table_name, table_payload) == 0 || lm_trans_registry_identifier_value(key_atom, key_payload) == 0) {
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    if (split_by_column == 0) {
        if (wants_node != 0) {
            status = lm_trans_registry_push_row_node_values(table_payload, key_payload, payload_node);
        }
        else {
            status = lm_trans_registry_push_row_values(table_payload, key_payload, payload_value);
        }
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    if (column -> name == 0 || lm_trans_registry_identifier_value(column -> name, column_payload) == 0) {
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    relation_length = table_payload -> length + 1U + column_payload -> length;
    relation_name = lm_own_new_zero(relation_length + 1U);
    if (relation_name == 0) {
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    memcpy(relation_name, table_payload -> data, table_payload -> length);
    relation_name[table_payload->length] = '.';
    memcpy(relation_name + table_payload -> length + 1U, column_payload -> data, column_payload -> length);
    relation_name[relation_length] = '\0';
    relation_table = lm_trans_text_from_cstr(relation_name);
    if (relation_table == 0) {
        lm_own_delete(relation_name, 0);
        lm_trans_text_ref_destroy(&relation_table);
        lm_trans_text_ref_destroy(&table_payload);
        lm_trans_text_ref_destroy(&column_payload);
        lm_trans_text_ref_destroy(&key_payload);
        lm_trans_text_ref_destroy(&payload_value);
        return status;
    }
    if (wants_node != 0) {
        status = lm_trans_registry_push_row_node_values(relation_table, key_payload, payload_node);
    }
    else {
        status = lm_trans_registry_push_row_values(relation_table, key_payload, payload_value);
    }
    lm_own_delete(relation_name, 0);
    lm_trans_text_ref_destroy(&relation_table);
    lm_trans_text_ref_destroy(&table_payload);
    lm_trans_text_ref_destroy(&column_payload);
    lm_trans_text_ref_destroy(&key_payload);
    lm_trans_text_ref_destroy(&payload_value);
    return status;
}

static int lm_trans_registry_note_class_kind(const LmP0Text *name, const char *kind) {
    LmP0Text *table;
    LmP0Text *payload;
    int status;
    if (kind == 0) {
        return 0;
    }
    table = lm_trans_text_from_cstr("class.kind");
    payload = lm_trans_text_from_cstr(kind);
    if (table == 0 || payload == 0) {
        lm_trans_text_ref_destroy(&table);
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }
    status = lm_trans_registry_push_row_atoms(table, name, payload) != 0;
    lm_trans_text_ref_destroy(&table);
    lm_trans_text_ref_destroy(&payload);
    return status;
}

static int lm_trans_registry_note_class_present(const LmP0Text *name) {
    LmP0Text *table;
    LmP0Text *payload;
    LmP0Text *class_text;
    int status;
    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 1;
    }
    if (lm_trans_registry_identifier_value(name, payload) == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    table = lm_trans_text_from_cstr("class.present");
    class_text = lm_trans_text_from_cstr("class");
    if (table == 0 || class_text == 0) {
        lm_trans_text_ref_destroy(&payload);
        lm_trans_text_ref_destroy(&table);
        lm_trans_text_ref_destroy(&class_text);
        return 1;
    }
    status = lm_trans_registry_push_row_values(table, payload, class_text) != 0;
    lm_trans_text_ref_destroy(&payload);
    lm_trans_text_ref_destroy(&table);
    lm_trans_text_ref_destroy(&class_text);
    return status;
}

static LmTransRegistryFact * lm_trans_registry_lookup_row_in_identifiers(const LmTransIdentifierTable *identifiers, const LmP0Text *key, const char *table) {
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
    return lm_own_ptr_stack_top(relation -> symbols);
}

static const LmOwnPtrStack * lm_trans_registry_relation_stack_in_identifiers(const LmTransIdentifierTable *identifiers, const LmP0Text *key, const char *relation_name) {
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
    if (relation != 0) {
        return relation -> symbols;
    }
    return 0;
}

static const LmOwnPtrStack * lm_trans_registry_relation_stack(const LmP0Text *key, const char *relation_name) {
    return lm_trans_registry_relation_stack_in_identifiers(lm_trans_registry->identifiers, key, relation_name);
}

static const LmOwnPtrStack * lm_trans_namespace_registry_relation_stack(const LmTransNamespace *namespace_, const LmP0Text *key, const char *relation_name) {
    const LmOwnPtrStack *stack;
    if (namespace_ != 0 && namespace_ -> registry_identifiers != 0) {
        stack = lm_trans_registry_relation_stack_in_identifiers(namespace_ -> registry_identifiers, key, relation_name);
        if (stack != 0) {
            return stack;
        }
    }
    return lm_trans_registry_relation_stack(key, relation_name);
}

static const LmOwnPtrStack * lm_trans_registry_relation_stack_for_table(const char *table) {
    const char *dot;
    LmP0Text *table_name;
    const char *relation_name;
    const LmOwnPtrStack *stack;
    if (table == 0) {
        return 0;
    }
    table_name = lm_trans_text_ref_new_cstr("");
    if (table_name == 0) {
        return 0;
    }
    dot = strchr(table, '.');
    if (dot != 0 && dot != table && dot[1] != '\0') {
        table_name->data = table;
        table_name->length = (((size_t)(dot - table)));
        relation_name = dot + 1;
    }
    else {
        table_name->data = table;
        table_name->length = strlen(table);
        relation_name = "row";
    }
    stack = lm_trans_registry_relation_stack(table_name, relation_name);
    lm_trans_text_ref_destroy(&table_name);
    return stack;
}

static LmTransRegistryFact * lm_trans_registry_relation_stack_latest_row(const LmOwnPtrStack *stack, const LmP0Text *key) {
    size_t i;
    LmTransRegistryFact *row;
    LmP0Text *key_payload;
    key_payload = lm_trans_text_ref_new_cstr("");
    if (stack == 0 || key_payload == 0 || lm_trans_identifier_payload(key, key_payload) == 0) {
        lm_trans_text_ref_destroy(&key_payload);
        return 0;
    }
    i = stack -> count;
    while (i > 0U) {
        i = i - 1U;
        row = lm_own_ptr_stack_at(stack, i);
        if (row != 0 && row -> key != 0 && lm_trans_text_equals(key_payload, row -> key) != 0) {
            lm_trans_text_ref_destroy(&key_payload);
            return row;
        }
    }
    lm_trans_text_ref_destroy(&key_payload);
    return 0;
}

static const char * lm_trans_registry_lookup(const LmP0Text *key, const char *table) {
    LmTransRegistryFact *row;
    row = lm_trans_registry_lookup_row_in_identifiers(lm_trans_registry->identifiers, key, table);
    if (row != 0) {
        return row -> payload;
    }
    return 0;
}

static int lm_trans_registry_has(const LmP0Text *key, const char *table) {
    return lm_trans_registry_lookup(key, table) != 0;
}

static int lm_trans_registry_table_has_rows(const char *table) {
    const LmOwnPtrStack *stack;
    if (table == 0) {
        return 0;
    }
    stack = lm_trans_registry_relation_stack_for_table(table);
    return stack != 0 && stack -> count != 0U;
}

static int lm_trans_registry_assert_selected_table_link(const char *source_table, const LmP0Text *key, const char *target_class_table, const char *target_table) {
    LmP0Text *target_key;
    int target_has_class_row;
    if (source_table == 0 || target_table == 0) {
        return 1;
    }
    target_has_class_row = 1;
    target_key = 0;
    if (target_class_table != 0) {
        target_key = lm_trans_text_from_cstr(target_table);
        target_has_class_row = target_key != 0 && lm_trans_registry_has(target_key, target_class_table) != 0;
        lm_trans_text_ref_destroy(&target_key);
    }
    if (target_has_class_row == 0) {
        fprintf(stderr, "trans registry inconsistency: %s[\"%.*s\"] points to %s, but %s has no class row for that target\n", source_table, (((int)key -> length)), key -> data, target_table, target_class_table);
        return 1;
    }
    if (lm_trans_registry_has(key, target_table) == 0) {
        fprintf(stderr, "trans registry inconsistency: %s[\"%.*s\"] points to %s, but %s has no matching row for that key\n", source_table, (((int)key -> length)), key -> data, target_table, target_table);
        return 1;
    }
    return 0;
}

static int lm_trans_registry_assert_payload_table_exists(const char *source_table, const LmP0Text *key, const char *target_table) {
    if (source_table == 0 || target_table == 0) {
        return 1;
    }
    if (lm_trans_registry_table_has_rows(target_table) == 0) {
        fprintf(stderr, "trans registry inconsistency: %s[\"%.*s\"] points to %s, but that table has no rows\n", source_table, (((int)key -> length)), key -> data, target_table);
        return 1;
    }
    return 0;
}

static const char * lm_trans_registry_lookup_table_link_checked(const LmP0Text *key, const char *source_table, const char *target_class_table) {
    const char *target_table;
    target_table = lm_trans_registry_lookup(key, source_table);
    if (target_table != 0 && lm_trans_registry_assert_selected_table_link(source_table, key, target_class_table, target_table) != 0) {
        exit(2);
    }
    return target_table;
}

static const char * lm_trans_namespace_registry_lookup(const LmTransNamespace *namespace_, const LmP0Text *key, const char *table) {
    LmTransRegistryFact *row;
    if (namespace_ != 0 && namespace_ -> registry_identifiers != 0) {
        row = lm_trans_registry_lookup_row_in_identifiers(namespace_ -> registry_identifiers, key, table);
        if (row != 0) {
            return row -> payload;
        }
    }
    return lm_trans_registry_lookup(key, table);
}

static const char * lm_trans_namespace_registry_lookup_table_link_checked(const LmTransNamespace *namespace_, const LmP0Text *key, const char *source_table, const char *target_class_table) {
    const char *target_table;
    target_table = lm_trans_namespace_registry_lookup(namespace_, key, source_table);
    if (target_table != 0 && lm_trans_registry_assert_selected_table_link(source_table, key, target_class_table, target_table) != 0) {
        exit(2);
    }
    return target_table;
}

static int lm_trans_array_head_find_close(const LmP0Text *text, size_t open_index, size_t *out_close_index) {
    size_t i;
    size_t depth;
    char quote;
    if (text == 0 || out_close_index == 0 || open_index >= text -> length || text -> data[open_index] != '[') {
        return 0;
    }
    i = open_index;
    depth = 0U;
    while (i < text -> length) {
        if (text -> data[i] == '"' || text -> data[i] == '`') {
            quote = text -> data[i];
            i = i + 1U;
            while (i < text -> length) {
                if (quote == '"' && text -> data[i] == '\\') {
                    i = i + 2U;
                    continue;
                }
                if (quote == '`' && text -> data[i] == '`' && i + 1U < text -> length && text -> data[i + 1U] == '`') {
                    i = i + 2U;
                    continue;
                }
                if (text -> data[i] == quote) {
                    i = i + 1U;
                    break;
                }
                i = i + 1U;
            }
            continue;
        }
        if (text -> data[i] == '\'') {
            i = i + 1U;
            while (i < text -> length) {
                if (text -> data[i] == '\\') {
                    i = i + 2U;
                    continue;
                }
                if (text -> data[i] == '\'') {
                    i = i + 1U;
                    break;
                }
                i = i + 1U;
            }
            continue;
        }
        if (text -> data[i] == '[') {
            depth = depth + 1U;
        }
        else {
            if (text -> data[i] == ']') {
                depth = depth - 1U;
                if (depth == 0U) {
                    out_close_index[0] = i;
                    return 1;
                }
            }
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_trans_array_head_next_dimension(const LmP0Text *head, size_t *index, LmP0Text *out_dimension) {
    size_t close_index;
    if (head == 0 || index == 0 || out_dimension == 0 || index[0] >= head -> length || head -> data[index[0]] != '[') {
        return 0;
    }
    if (lm_trans_array_head_find_close(head, index[0], &close_index) == 0) {
        return 0;
    }
    out_dimension->data = head -> data + index[0] + 1U;
    out_dimension->length = close_index - index[0] - 1U;
    index[0] = close_index + 1U;
    return 1;
}

static int lm_trans_text_is_array_receiver_head(const LmP0Text *head) {
    size_t index;
    size_t count;
    LmP0Text *dimension;
    index = 0U;
    count = 0U;
    if (head == 0) {
        return 0;
    }
    dimension = lm_trans_text_ref_new_cstr("");
    if (dimension == 0) {
        return 0;
    }
    while (index < head -> length) {
        if (lm_trans_array_head_next_dimension(head, &index, dimension) == 0) {
            lm_trans_text_ref_destroy(&dimension);
            return 0;
        }
        count = count + 1U;
    }
    lm_trans_text_ref_destroy(&dimension);
    return count > 0U;
}

static LmP0Text * lm_trans_text_ref_new(const LmP0Text *text) {
    LmP0Text *copy;
    copy = lm_own_new_zero(sizeof(copy[0]));
    if (copy != 0 && text != 0) {
        copy[0] = text[0];
    }
    return copy;
}

static LmP0Text * lm_trans_text_ref_new_cstr(const char *text) {
    LmP0Text *copy;
    copy = lm_own_new_zero(sizeof(copy[0]));
    if (copy != 0 && text != 0) {
        copy->data = text;
        copy->length = strlen(text);
    }
    return copy;
}

static LmP0Text * lm_trans_text_ref_tail_new(const LmP0Text *text, size_t offset) {
    LmP0Text *tail;
    if (text == 0 || offset > text -> length) {
        return 0;
    }
    tail = lm_own_new_zero(sizeof(tail[0]));
    if (tail != 0) {
        tail->data = text -> data + offset;
        tail->length = text -> length - offset;
    }
    return tail;
}

static void lm_trans_text_ref_destroy(LmP0Text **text) {
    if (text != 0 && text[0] != 0) {
        lm_own_delete(text[0], 0);
        text[0] = 0;
    }
}

static void lm_trans_text_ref_delete_any(void *object) {
    lm_own_delete(object, 0);
}

static int lm_trans_function_header_init_fields(LmTransFunctionHeader *function) {
    if (function == 0) {
        return 1;
    }
    if (function -> name == 0) {
        function->name = lm_trans_text_ref_new_cstr("");
    }
    if (function -> c_name == 0) {
        function->c_name = lm_trans_text_ref_new_cstr("");
    }
    if (function -> env_type_name == 0) {
        function->env_type_name = lm_trans_text_ref_new_cstr("");
    }
    return function -> name == 0 || function -> c_name == 0 || function -> env_type_name == 0;
}

static LmTransFunctionHeader * lm_trans_function_header_new(void) {
    LmTransFunctionHeader *function;
    function = lm_own_new_zero(sizeof(function[0]));
    if (function != 0 && lm_trans_function_header_init_fields(function) != 0) {
        lm_trans_text_ref_destroy(&function -> name);
        lm_trans_text_ref_destroy(&function -> c_name);
        lm_trans_text_ref_destroy(&function -> env_type_name);
        lm_own_delete(function, 0);
        return 0;
    }
    return function;
}

static void lm_trans_function_header_destroy_fields(LmTransFunctionHeader *function) {
    if (function != 0) {
        lm_trans_text_ref_destroy(&function -> name);
        lm_trans_text_ref_destroy(&function -> c_name);
        lm_trans_text_ref_destroy(&function -> env_type_name);
    }
}

static void lm_trans_function_header_destroy(LmTransFunctionHeader *function) {
    lm_trans_function_header_destroy_fields(function);
    lm_own_delete(function, 0);
}

static int lm_trans_function_header_copy(LmTransFunctionHeader *target, const LmTransFunctionHeader *source) {
    LmP0Text *name;
    LmP0Text *c_name;
    LmP0Text *env_type_name;
    if (target == 0 || source == 0 || lm_trans_function_header_init_fields(target) != 0) {
        return 1;
    }
    name = target -> name;
    c_name = target -> c_name;
    env_type_name = target -> env_type_name;
    target[0] = source[0];
    target->name = name;
    target->c_name = c_name;
    target->env_type_name = env_type_name;
    if (source -> name != 0) {
        target->name[0] = source -> name[0];
    }
    else {
        target->name->data = "";
        target->name->length = 0U;
    }
    if (source -> c_name != 0) {
        target->c_name[0] = source -> c_name[0];
    }
    else {
        target->c_name->data = "";
        target->c_name->length = 0U;
    }
    if (source -> env_type_name != 0) {
        target->env_type_name[0] = source -> env_type_name[0];
    }
    else {
        target->env_type_name->data = "";
        target->env_type_name->length = 0U;
    }
    return 0;
}

static LmOwnPtrStack * lm_trans_ptr_stack_new(LmOwnDelete delete_item) {
    LmOwnPtrStack *stack;
    stack = lm_own_new_zero(sizeof(stack[0]));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, delete_item);
    }
    return stack;
}

static void lm_trans_ptr_stack_delete(LmOwnPtrStack **stack) {
    if (stack != 0 && stack[0] != 0) {
        lm_own_ptr_stack_destroy(stack[0]);
        lm_own_delete(stack[0], 0);
        stack[0] = 0;
    }
}

static LmTransFunctionState * lm_trans_function_state_new(void) {
    return lm_own_new_zero(sizeof(LmTransFunctionState));
}

static void lm_trans_function_state_destroy(LmTransFunctionState *state) {
    if (state != 0) {
        lm_trans_text_ref_destroy(&state -> current_return_type_name);
        if (state -> has_previous_control_stacks) {
            lm_trans_ptr_stack_delete(&state -> previous_cleanups);
            lm_trans_ptr_stack_delete(&state -> previous_loops);
            state->has_previous_control_stacks = 0;
        }
    }
}

static void lm_trans_function_state_destroy_any(void *object) {
    lm_trans_function_state_destroy(((LmTransFunctionState *)object));
}

static void lm_trans_function_state_delete(LmTransFunctionState *state) {
    lm_own_delete(state, lm_trans_function_state_destroy_any);
}

static int lm_trans_registry_has_expr_emitter_binding(const LmP0Text *key, const char *table) {
    const char *binding;
    LmTransBinding *resolved;
    const char *table_name;
    if (key == 0) {
        return 0;
    }
    binding = lm_trans_registry_lookup(key, table);
    if (binding == 0) {
        return 0;
    }
    resolved = lm_own_new_zero(sizeof(resolved[0]));
    if (resolved == 0) {
        exit(2);
    }
    if (lm_trans_binding_resolve(binding, resolved) == 0 || resolved -> expr_emit == 0 || resolved -> expr_state == 0) {
        table_name = table;
        if (table_name == 0) {
            table_name = "<none>";
        }
        fprintf(stderr, "trans registry inconsistency: %s[\"%.*s\"] has unknown expression emitter binding %s\n", table_name, (((int)key -> length)), key -> data, binding);
        lm_own_delete(resolved, 0);
        exit(2);
    }
    lm_own_delete(resolved, 0);
    return 1;
}

static const char * lm_trans_expr_emitter_binding_table(const char *class_name) {
    LmP0Text *class_text;
    const char *table;
    class_text = lm_trans_text_ref_new_cstr(class_name);
    if (class_text == 0) {
        exit(2);
    }
    table = lm_trans_registry_lookup(class_text, "expr_emitter_binding");
    if (table != 0 && lm_trans_registry_assert_payload_table_exists("expr_emitter_binding", class_text, table) != 0) {
        lm_trans_text_ref_destroy(&class_text);
        exit(2);
    }
    lm_trans_text_ref_destroy(&class_text);
    return table;
}

static const char * lm_trans_expr_spelling_binding_table(const char *class_name) {
    LmP0Text *class_text;
    const char *table;
    class_text = lm_trans_text_ref_new_cstr(class_name);
    if (class_text == 0) {
        exit(2);
    }
    table = lm_trans_registry_lookup(class_text, "expr_spelling_binding");
    if (table != 0 && lm_trans_registry_assert_payload_table_exists("expr_spelling_binding", class_text, table) != 0) {
        lm_trans_text_ref_destroy(&class_text);
        exit(2);
    }
    lm_trans_text_ref_destroy(&class_text);
    if (table != 0) {
        return table;
    }
    return class_name;
}

static int lm_trans_registry_has_expr_emitter_class(const LmP0Text *key, const char *class_name) {
    const char *table;
    table = lm_trans_expr_emitter_binding_table(class_name);
    return table != 0 && lm_trans_registry_has_expr_emitter_binding(key, table);
}

static int lm_trans_registry_has_expr_emitter_binding_class(const LmP0Text *key, const char *class_name) {
    return lm_trans_registry_has_expr_emitter_class(key, class_name);
}

static int lm_trans_registry_has_any_expr_emitter_binding(const LmP0Text *key) {
    size_t i;
    LmTransRegistryFact *row;
    const LmOwnPtrStack *rows;
    rows = lm_trans_registry_relation_stack(lm_trans_text_from_cstr("expr_emitter_binding"), "row");
    if (rows == 0) {
        return 0;
    }
    i = 0U;
    while (i < rows -> count) {
        row = lm_own_ptr_stack_at(rows, i);
        if (row != 0 && row -> payload != 0 && row -> key != 0) {
            if (lm_trans_registry_assert_payload_table_exists("expr_emitter_binding", lm_trans_text_from_cstr(row -> key), row -> payload) != 0) {
                exit(2);
            }
            if (lm_trans_registry_has_expr_emitter_binding(key, row -> payload)) {
                return 1;
            }
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_trans_text_is_operator_atom(const LmP0Text *text) {
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
    count = fread(buffer, 1U, sizeof(buffer), source);
    while (count != 0U) {
        if (fwrite(buffer, 1U, count, target) != count) {
            return 1;
        }
        count = fread(buffer, 1U, sizeof(buffer), source);
    }
    if (ferror(source)) {
        return 1;
    }
    return 0;
}

static char * lm_trans_temp_output_path_new(const char *output_path, const char *suffix) {
    size_t output_length;
    size_t suffix_length;
    char *result;
    if (output_path == 0 || suffix == 0) {
        return 0;
    }
    output_length = strlen(output_path);
    suffix_length = strlen(suffix);
    result = lm_own_new_zero(output_length + suffix_length + 1U);
    if (result == 0) {
        return 0;
    }
    memcpy(result, output_path, output_length);
    memcpy(result + output_length, suffix, suffix_length + 1U);
    return result;
}

static const LmP0Structure * lm_trans_unwrap_single_anonymous_structure(const LmP0Structure *structure) {
    const LmP0Field *field;
    const LmP0Structure *current;
    current = structure;
    while (current != 0) {
        field = current -> first_field;
        if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
            break;
        }
        current = field -> value -> as -> structure;
    }
    return current;
}

static unsigned long lm_trans_hash_bytes(const char *data, size_t length) {
    unsigned long hash;
    size_t i;
    hash = 2166136261UL;
    i = 0U;
    while (i < length) {
        hash = hash ^ (((unsigned char)data[i]));
        hash = hash * 16777619UL;
        i = i + 1U;
    }
    return hash;
}

static unsigned long lm_trans_hash_cstr(const char *text) {
    if (text != 0) {
        return lm_trans_hash_bytes(text, strlen(text));
    }
    return 0UL;
}

static void lm_trans_ptr_stack_remove_pointer(LmOwnPtrStack *stack, const void *item) {
    size_t i;
    if (stack == 0 || item == 0) {
        return;
    }
    i = stack -> count;
    while (i > 0U) {
        i = i - 1U;
        if (stack -> items[i] == item) {
            while (i + 1U < stack -> count) {
                stack->items[i] = stack -> items[i + 1U];
                i = i + 1U;
            }
            stack->count = stack -> count - 1U;
            stack->items[stack->count] = 0;
            return;
        }
    }
}

static void lm_trans_identifier_relation_destroy(LmTransIdentifierRelation *relation) {
    if (relation != 0) {
        lm_trans_ptr_stack_delete(&relation -> symbols);
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
    lm_trans_ptr_stack_delete(&card -> symbols);
    if (card -> relation_buckets != 0) {
        i = 0U;
        while (i < card -> relation_bucket_count) {
            relation = card -> relation_buckets[i];
            while (relation != 0) {
                next = relation -> next;
                lm_trans_identifier_relation_destroy(relation);
                relation = next;
            }
            i = i + 1U;
        }
    }
    lm_own_delete(card -> relation_buckets, 0);
    card->relation_buckets = 0;
    card->relation_bucket_count = 0U;
    lm_trans_text_ref_destroy(&card -> name);
    lm_own_delete(card -> name_storage, 0);
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

static int lm_trans_identifier_table_ensure_buckets(LmTransIdentifierTable *table) {
    size_t bucket_count;
    if (table == 0) {
        return 1;
    }
    if (table -> buckets != 0) {
        return 0;
    }
    bucket_count = 257U;
    table->buckets = lm_own_new_zero(bucket_count * sizeof(table -> buckets[0]));
    if (table -> buckets == 0) {
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
    if (table -> buckets != 0) {
        i = 0U;
        while (i < table -> bucket_count) {
            card = table -> buckets[i];
            while (card != 0) {
                next = card -> next;
                lm_trans_identifier_card_destroy(card);
                card = next;
            }
            i = i + 1U;
        }
    }
    lm_own_delete(table -> buckets, 0);
    table->buckets = 0;
    table->bucket_count = 0U;
    table->count = 0U;
}

static LmTransIdentifierTable * lm_trans_identifier_table_new(void) {
    LmTransIdentifierTable *table;
    table = lm_own_new_zero(sizeof(table[0]));
    if (table != 0) {
        lm_trans_identifier_table_init(table);
    }
    return table;
}

static void lm_trans_identifier_table_delete(LmTransIdentifierTable **table) {
    if (table != 0 && table[0] != 0) {
        lm_trans_identifier_table_destroy(table[0]);
        lm_own_delete(table[0], 0);
        table[0] = 0;
    }
}

static LmTransIdentifierCard * lm_trans_identifier_table_find_card(const LmTransIdentifierTable *table, const LmP0Text *name) {
    LmP0Text *payload;
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierCard *card;
    payload = lm_trans_text_ref_new(0);
    if (payload == 0) {
        return 0;
    }
    if (table == 0 || table -> buckets == 0 || table -> bucket_count == 0U || lm_trans_identifier_payload(name, payload) == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    hash = lm_trans_hash_bytes(payload -> data, payload -> length);
    bucket = hash % table -> bucket_count;
    card = table -> buckets[bucket];
    while (card != 0) {
        if (card -> hash == hash && card -> name != 0 && lm_trans_text_same(card -> name, payload)) {
            lm_trans_text_ref_destroy(&payload);
            return card;
        }
        card = card -> next;
    }
    lm_trans_text_ref_destroy(&payload);
    return 0;
}

static LmTransIdentifierCard * lm_trans_identifier_table_get_card(LmTransIdentifierTable *table, const LmP0Text *name) {
    LmP0Text *payload;
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierCard *card;
    char *copy;
    payload = lm_trans_text_ref_new(0);
    if (payload == 0) {
        return 0;
    }
    if (table == 0 || lm_trans_identifier_payload(name, payload) == 0 || lm_trans_identifier_table_ensure_buckets(table) != 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    card = lm_trans_identifier_table_find_card(table, name);
    if (card != 0) {
        lm_trans_text_ref_destroy(&payload);
        return card;
    }
    hash = lm_trans_hash_bytes(payload -> data, payload -> length);
    bucket = hash % table -> bucket_count;
    card = lm_own_new_zero(sizeof(card[0]));
    if (card == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    copy = lm_trans_text_copy_cstr(payload);
    if (copy == 0) {
        lm_trans_text_ref_destroy(&payload);
        lm_trans_identifier_card_destroy(card);
        return 0;
    }
    card->name_storage = copy;
    card->name = lm_trans_text_ref_new_cstr(copy);
    if (card -> name == 0) {
        lm_trans_text_ref_destroy(&payload);
        lm_trans_identifier_card_destroy(card);
        return 0;
    }
    card->symbols = lm_trans_ptr_stack_new(0);
    if (card -> symbols == 0) {
        lm_trans_text_ref_destroy(&payload);
        lm_trans_identifier_card_destroy(card);
        return 0;
    }
    card->relation_bucket_count = 31U;
    card->relation_buckets = lm_own_new_zero(card -> relation_bucket_count * sizeof(card -> relation_buckets[0]));
    if (card -> relation_buckets == 0) {
        lm_trans_text_ref_destroy(&payload);
        lm_trans_identifier_card_destroy(card);
        return 0;
    }
    card->hash = hash;
    card->next = table -> buckets[bucket];
    table->buckets[bucket] = card;
    table->count = table -> count + 1U;
    lm_trans_text_ref_destroy(&payload);
    return card;
}

static LmTransIdentifierRelation * lm_trans_identifier_card_find_relation(const LmTransIdentifierCard *card, const char *relation_name) {
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierRelation *relation;
    if (card == 0 || relation_name == 0 || card -> relation_buckets == 0 || card -> relation_bucket_count == 0U) {
        return 0;
    }
    hash = lm_trans_hash_cstr(relation_name);
    bucket = hash % card -> relation_bucket_count;
    relation = card -> relation_buckets[bucket];
    while (relation != 0) {
        if (relation -> name != 0 && strcmp(relation -> name, relation_name) == 0) {
            return relation;
        }
        relation = relation -> next;
    }
    return 0;
}

static LmTransIdentifierRelation * lm_trans_identifier_card_get_relation(LmTransIdentifierCard *card, const char *relation_name) {
    unsigned long hash;
    size_t bucket;
    LmTransIdentifierRelation *relation;
    if (card == 0 || relation_name == 0 || card -> relation_buckets == 0 || card -> relation_bucket_count == 0U) {
        return 0;
    }
    hash = lm_trans_hash_cstr(relation_name);
    bucket = hash % card -> relation_bucket_count;
    relation = card -> relation_buckets[bucket];
    while (relation != 0) {
        if (relation -> name != 0 && strcmp(relation -> name, relation_name) == 0) {
            return relation;
        }
        relation = relation -> next;
    }
    relation = lm_own_new_zero(sizeof(relation[0]));
    if (relation == 0) {
        return 0;
    }
    relation->name = relation_name;
    relation->symbols = lm_trans_ptr_stack_new(0);
    if (relation -> symbols == 0) {
        lm_own_delete(relation, 0);
        return 0;
    }
    relation->next = card -> relation_buckets[bucket];
    card->relation_buckets[bucket] = relation;
    return relation;
}

static int lm_trans_identifier_table_add_symbol(LmTransIdentifierTable *table, LmTransSymbol *symbol) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;
    if (table == 0 || symbol == 0 || symbol -> name == 0) {
        return 1;
    }
    card = lm_trans_identifier_table_get_card(table, symbol -> name);
    if (card == 0) {
        return 1;
    }
    relation = lm_trans_identifier_card_get_relation(card, symbol -> class_name);
    if (relation == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(card -> symbols, symbol) != 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(relation -> symbols, symbol) != 0) {
        lm_trans_ptr_stack_remove_pointer(card -> symbols, symbol);
        return 1;
    }
    return 0;
}

static void lm_trans_identifier_table_remove_symbol(LmTransIdentifierTable *table, const LmTransSymbol *symbol) {
    LmTransIdentifierCard *card;
    LmTransIdentifierRelation *relation;
    if (table == 0 || symbol == 0 || symbol -> name == 0) {
        return;
    }
    card = lm_trans_identifier_table_find_card(table, symbol -> name);
    if (card == 0) {
        return;
    }
    lm_trans_ptr_stack_remove_pointer(card -> symbols, symbol);
    relation = lm_trans_identifier_card_get_relation(card, symbol -> class_name);
    if (relation != 0) {
        lm_trans_ptr_stack_remove_pointer(relation -> symbols, symbol);
    }
}

static LmTransSymbol * lm_trans_identifier_table_find_latest_symbol(const LmTransIdentifierTable *table, const LmP0Text *name) {
    LmTransIdentifierCard *card;
    card = lm_trans_identifier_table_find_card(table, name);
    if (card == 0) {
        return 0;
    }
    return lm_own_ptr_stack_top(card -> symbols);
}

static LmTransSymbol * lm_trans_symbol_new(const LmP0Text *name, const char *kind, unsigned depth) {
    LmTransSymbol *symbol;
    symbol = lm_own_new_zero(sizeof(symbol[0]));
    if (symbol == 0) {
        return 0;
    }
    symbol->name = lm_trans_text_ref_new(name);
    if (symbol -> name == 0) {
        lm_trans_symbol_destroy(symbol);
        return 0;
    }
    symbol->c_name = lm_trans_text_ref_new_cstr("");
    symbol->env_arg = lm_trans_text_ref_new_cstr("");
    symbol->closure_call_name = lm_trans_text_ref_new_cstr("");
    if (symbol -> c_name == 0 || symbol -> env_arg == 0 || symbol -> closure_call_name == 0) {
        lm_trans_symbol_destroy(symbol);
        return 0;
    }
    symbol->param_names = lm_trans_ptr_stack_new(lm_trans_text_ref_delete_any);
    if (symbol -> param_names == 0) {
        lm_trans_symbol_destroy(symbol);
        return 0;
    }
    symbol->class_name = kind;
    symbol->depth = depth;
    return symbol;
}

static int lm_trans_symbol_set_owned_name(LmTransSymbol *symbol, const LmP0Text *name) {
    char *copy;
    if (symbol == 0 || symbol -> name == 0 || name == 0) {
        return 1;
    }
    copy = lm_own_new_zero(name -> length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, name -> data, name -> length);
    copy[name->length] = '\0';
    lm_own_delete(symbol -> name_storage, 0);
    symbol->name_storage = copy;
    symbol->name->data = copy;
    symbol->name->length = name -> length;
    return 0;
}

static int lm_trans_symbol_set_c_name(LmTransSymbol *symbol, const LmP0Text *c_name) {
    char *copy;
    if (symbol == 0 || symbol -> c_name == 0 || c_name == 0) {
        return 1;
    }
    copy = lm_own_new_zero(c_name -> length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, c_name -> data, c_name -> length);
    copy[c_name->length] = '\0';
    lm_own_delete(symbol -> c_name_storage, 0);
    symbol->c_name_storage = copy;
    symbol->c_name->data = copy;
    symbol->c_name->length = c_name -> length;
    symbol->has_c_name = 1;
    return 0;
}

static int lm_trans_symbol_set_env_arg(LmTransSymbol *symbol, const LmP0Text *env_arg) {
    char *copy;
    if (symbol == 0 || symbol -> env_arg == 0 || env_arg == 0) {
        return 1;
    }
    copy = lm_own_new_zero(env_arg -> length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, env_arg -> data, env_arg -> length);
    copy[env_arg->length] = '\0';
    lm_own_delete(symbol -> env_arg_storage, 0);
    symbol->env_arg_storage = copy;
    symbol->env_arg->data = copy;
    symbol->env_arg->length = env_arg -> length;
    symbol->has_env_arg = 1;
    return 0;
}

static int lm_trans_symbol_set_closure_call_name(LmTransSymbol *symbol, const LmP0Text *closure_call_name) {
    char *copy;
    if (symbol == 0 || symbol -> closure_call_name == 0 || closure_call_name == 0) {
        return 1;
    }
    copy = lm_own_new_zero(closure_call_name -> length + 1U);
    if (copy == 0) {
        return 1;
    }
    memcpy(copy, closure_call_name -> data, closure_call_name -> length);
    copy[closure_call_name->length] = '\0';
    lm_own_delete(symbol -> closure_call_name_storage, 0);
    symbol->closure_call_name_storage = copy;
    symbol->closure_call_name->data = copy;
    symbol->closure_call_name->length = closure_call_name -> length;
    symbol->has_closure_call_name = 1;
    return 0;
}

static int lm_trans_capture_init_fields(LmTransCapture *capture) {
    if (capture == 0) {
        return 1;
    }
    if (capture -> name == 0) {
        capture->name = lm_trans_text_ref_new_cstr("");
    }
    if (capture -> type_head == 0) {
        capture->type_head = lm_trans_text_ref_new_cstr("");
    }
    return capture -> name == 0 || capture -> type_head == 0;
}

static int lm_trans_capture_reset(LmTransCapture *capture) {
    if (lm_trans_capture_init_fields(capture) != 0) {
        return 1;
    }
    capture->name->data = "";
    capture->name->length = 0U;
    capture->type_head->data = "";
    capture->type_head->length = 0U;
    capture->type_node = 0;
    capture->pointer_depth = 0U;
    capture->type_is_head = 0;
    return 0;
}

static LmTransCapture * lm_trans_capture_new(const LmP0Text *name, const LmP0Text *type_head, const LmP0Node *type_node, size_t pointer_depth, int type_is_head) {
    LmTransCapture *capture;
    if (name == 0 || type_head == 0) {
        return 0;
    }
    capture = lm_own_new_zero(sizeof(capture[0]));
    if (capture != 0) {
        if (lm_trans_capture_init_fields(capture) != 0) {
            lm_trans_text_ref_destroy(&capture -> name);
            lm_trans_text_ref_destroy(&capture -> type_head);
            lm_own_delete(capture, 0);
            return 0;
        }
        capture->name[0] = name[0];
        capture->type_head[0] = type_head[0];
        capture->type_node = type_node;
        capture->pointer_depth = pointer_depth;
        capture->type_is_head = type_is_head;
    }
    return capture;
}

static void lm_trans_capture_destroy(LmTransCapture *capture) {
    if (capture != 0) {
        lm_trans_text_ref_destroy(&capture -> name);
        lm_trans_text_ref_destroy(&capture -> type_head);
    }
    lm_own_delete(capture, 0);
}

static void lm_trans_capture_delete_any(void *object) {
    lm_trans_capture_destroy(((LmTransCapture *)object));
}

static void lm_trans_hoisted_function_destroy(LmTransHoistedFunction *function) {
    if (function != 0) {
        lm_trans_function_header_destroy(function -> function);
        function->function = 0;
        lm_trans_ptr_stack_delete(&function -> captures);
        lm_own_delete(function -> c_name_storage, 0);
        function->c_name_storage = 0;
        lm_own_delete(function -> env_type_storage, 0);
        function->env_type_storage = 0;
        lm_own_delete(function -> env_var_storage, 0);
        function->env_var_storage = 0;
        lm_own_delete(function -> closure_call_storage, 0);
        function->closure_call_storage = 0;
        lm_trans_text_ref_destroy(&function -> env_var_name);
        lm_trans_text_ref_destroy(&function -> closure_call_name);
    }
    lm_own_delete(function, 0);
}

static void lm_trans_hoisted_function_delete_any(void *object) {
    lm_trans_hoisted_function_destroy(((LmTransHoistedFunction *)object));
}

static LmTransCleanup * lm_trans_cleanup_new(unsigned id) {
    LmTransCleanup *cleanup;
    cleanup = lm_own_new_zero(sizeof(cleanup[0]));
    if (cleanup != 0) {
        cleanup->id = id;
    }
    return cleanup;
}

static void lm_trans_cleanup_destroy(LmTransCleanup *cleanup) {
    lm_own_delete(cleanup, 0);
}

static void lm_trans_cleanup_delete_any(void *object) {
    lm_trans_cleanup_destroy(((LmTransCleanup *)object));
}

static LmTransLoop * lm_trans_loop_new(size_t cleanup_base) {
    LmTransLoop *loop;
    loop = lm_own_new_zero(sizeof(loop[0]));
    if (loop != 0) {
        loop->cleanup_base = cleanup_base;
    }
    return loop;
}

static void lm_trans_loop_destroy(LmTransLoop *loop) {
    lm_own_delete(loop, 0);
}

static void lm_trans_loop_delete_any(void *object) {
    lm_trans_loop_destroy(((LmTransLoop *)object));
}

static LmTransNamespace * lm_trans_namespace_new(void) {
    LmTransNamespace *namespace_;
    namespace_ = lm_own_new_zero(sizeof(namespace_[0]));
    if (namespace_ != 0) {
        namespace_->items = lm_trans_ptr_stack_new(lm_trans_symbol_delete_any);
        namespace_->identifiers = lm_trans_identifier_table_new();
        namespace_->registry_identifiers = 0;
        if (lm_trans_registry != 0) {
            namespace_->registry_identifiers = lm_trans_registry -> identifiers;
        }
        namespace_->cleanups = lm_trans_ptr_stack_new(lm_trans_cleanup_delete_any);
        namespace_->loops = lm_trans_ptr_stack_new(lm_trans_loop_delete_any);
        if (namespace_ -> items == 0 || namespace_ -> identifiers == 0 || namespace_ -> cleanups == 0 || namespace_ -> loops == 0) {
            lm_trans_identifier_table_delete(&namespace_ -> identifiers);
            lm_trans_ptr_stack_delete(&namespace_ -> items);
            lm_trans_ptr_stack_delete(&namespace_ -> cleanups);
            lm_trans_ptr_stack_delete(&namespace_ -> loops);
            lm_own_delete(namespace_, 0);
            return 0;
        }
    }
    return namespace_;
}

static void lm_trans_namespace_destroy(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        lm_trans_identifier_table_delete(&namespace_ -> identifiers);
        namespace_->registry_identifiers = 0;
        lm_trans_ptr_stack_delete(&namespace_ -> items);
        lm_trans_ptr_stack_delete(&namespace_ -> cleanups);
        lm_trans_ptr_stack_delete(&namespace_ -> loops);
        namespace_->depth = 0U;
        namespace_->next_cleanup_id = 0U;
        namespace_->return_type_node = 0;
        namespace_->return_type_is_struct = 0;
        lm_trans_text_ref_destroy(&namespace_ -> return_type_name);
        namespace_->next_return_id = 0U;
        namespace_->hoisted_functions = 0;
    }
}

static void lm_trans_namespace_destroy_any(void *object) {
    lm_trans_namespace_destroy(((LmTransNamespace *)object));
}

static void lm_trans_namespace_delete(LmTransNamespace *namespace_) {
    lm_own_delete(namespace_, lm_trans_namespace_destroy_any);
}

static void lm_trans_namespace_enter_scope(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        namespace_->depth = namespace_ -> depth + 1U;
    }
}

static void lm_trans_namespace_leave_scope(LmTransNamespace *namespace_) {
    LmTransSymbol *symbol;
    if (namespace_ == 0) {
        return;
    }
    while (namespace_ -> items != 0 && namespace_ -> items -> count > 0U) {
        symbol = lm_own_ptr_stack_top(namespace_ -> items);
        if (symbol == 0 || symbol -> depth != namespace_ -> depth) {
            break;
        }
        symbol = lm_own_ptr_stack_pop(namespace_ -> items);
        lm_trans_identifier_table_remove_symbol(namespace_ -> identifiers, symbol);
        lm_trans_symbol_destroy(symbol);
    }
    if (namespace_ -> depth > 0U) {
        namespace_->depth = namespace_ -> depth - 1U;
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
    if (namespace_ -> cleanups == 0 || lm_own_ptr_stack_push(namespace_ -> cleanups, cleanup) != 0) {
        lm_trans_cleanup_destroy(cleanup);
        return 1;
    }
    return 0;
}

static void lm_trans_cleanup_pop(LmTransNamespace *namespace_) {
    LmTransCleanup *cleanup;
    if (namespace_ != 0) {
        cleanup = lm_own_ptr_stack_pop(namespace_ -> cleanups);
        lm_trans_cleanup_destroy(cleanup);
    }
}

static int lm_trans_loop_push(LmTransNamespace *namespace_) {
    LmTransLoop *loop;
    if (namespace_ == 0) {
        return 1;
    }
    if (namespace_ -> cleanups == 0 || namespace_ -> loops == 0) {
        return 1;
    }
    loop = lm_trans_loop_new(namespace_ -> cleanups -> count);
    if (loop == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(namespace_ -> loops, loop) != 0) {
        lm_trans_loop_destroy(loop);
        return 1;
    }
    return 0;
}

static void lm_trans_loop_pop(LmTransNamespace *namespace_) {
    LmTransLoop *loop;
    if (namespace_ != 0) {
        loop = lm_own_ptr_stack_pop(namespace_ -> loops);
        lm_trans_loop_destroy(loop);
    }
}

static size_t lm_trans_loop_cleanup_base(const LmTransNamespace *namespace_) {
    const LmTransLoop *loop;
    if (namespace_ == 0 || namespace_ -> loops == 0 || namespace_ -> loops -> count == 0U) {
        if (namespace_ != 0 && namespace_ -> cleanups != 0) {
            return namespace_ -> cleanups -> count;
        }
        return 0U;
    }
    loop = lm_own_ptr_stack_top(namespace_ -> loops);
    if (loop != 0) {
        return loop -> cleanup_base;
    }
    return namespace_ -> cleanups -> count;
}

static int lm_trans_emit_sync_name(FILE *file, unsigned id) {
    if (fprintf(file, "lm_sync_%u", id) < 0) {
        return 1;
    }
    return 0;
}

static int lm_trans_emit_return_name(FILE *file, unsigned id) {
    if (fprintf(file, "lm_return_%u", id) < 0) {
        return 1;
    }
    return 0;
}

static const LmTransSymbol * lm_trans_namespace_find(const LmTransNamespace *namespace_, const LmP0Text *name) {
    if (namespace_ == 0) {
        return 0;
    }
    return lm_trans_identifier_table_find_latest_symbol(namespace_ -> identifiers, name);
}

static LmTransSymbol * lm_trans_namespace_find_mutable(LmTransNamespace *namespace_, const LmP0Text *name) {
    if (namespace_ == 0) {
        return 0;
    }
    return lm_trans_identifier_table_find_latest_symbol(namespace_ -> identifiers, name);
}

static int lm_trans_is_c_reference_name(const LmP0Text *name) {
    return name != 0 && lm_trans_text_starts_with(name, "c.") && name -> length > 2U;
}

static int lm_trans_is_reserved_head_name(const LmP0Text *name) {
    const char *namespace_class;
    if (name == 0) {
        return 0;
    }
    namespace_class = lm_trans_registry_lookup_table_link_checked(name, "namespace", "receiver.type");
    return namespace_class != 0;
}

static int lm_trans_head_binding_resolve(const LmTransNamespace *namespace_, const LmP0Text *head, LmTransHeadBinding *out) {
    LmTransBinding *resolved;
    int receiver_status;
    if (head == 0 || out == 0) {
        return 1;
    }
    resolved = lm_own_new_zero(sizeof(resolved[0]));
    if (resolved == 0) {
        return 1;
    }
    memset(out, 0, sizeof(out[0]));
    out->symbol = lm_trans_namespace_find(namespace_, head);
    out->receiver_type = lm_trans_namespace_registry_lookup_table_link_checked(namespace_, head, "namespace", "receiver.type");
    out->function_receiver_binding = lm_trans_namespace_registry_lookup(namespace_, head, "receiver.function");
    if (out -> function_receiver_binding != 0) {
        receiver_status = lm_trans_binding_resolve(out -> function_receiver_binding, resolved);
        if (receiver_status == 0 || resolved -> function_receiver == 0) {
            fprintf(stderr, "trans registry inconsistency: receiver.function[\"%.*s\"] has unknown function binding %s\n", (((int)head -> length)), head -> data, out -> function_receiver_binding);
            lm_own_delete(resolved, 0);
            return 1;
        }
        out->function_receiver = resolved -> function_receiver;
    }
    out->statement_receiver_binding = lm_trans_namespace_registry_lookup(namespace_, head, "receiver.statement");
    if (out -> statement_receiver_binding != 0) {
        if (lm_trans_binding_resolve(out -> statement_receiver_binding, resolved) == 0 || resolved -> statement_frame == 0) {
            fprintf(stderr, "trans registry inconsistency: receiver.statement[\"%.*s\"] has unknown statement binding %s\n", (((int)head -> length)), head -> data, out -> statement_receiver_binding);
            lm_own_delete(resolved, 0);
            return 1;
        }
        out->statement_frame = resolved -> statement_frame;
    }
    lm_own_delete(resolved, 0);
    return 0;
}

static int lm_trans_namespace_declare_with_c_name(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind, const LmP0Text *c_name) {
    LmTransSymbol *symbol;
    if (namespace_ == 0 || name == 0) {
        return 1;
    }
    if (lm_trans_is_c_reference_name(name)) {
        fprintf(stderr, "trans L2 error: C-surface spelling \"%.*s\" cannot be declared as a Lingvamyxa name\n", (((int)name -> length)), name -> data);
        return 1;
    }
    symbol = lm_trans_symbol_new(name, kind, namespace_ -> depth);
    if (symbol == 0) {
        return 1;
    }
    if (c_name != 0 && lm_trans_symbol_set_c_name(symbol, c_name) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (namespace_ -> items == 0 || lm_own_ptr_stack_push(namespace_ -> items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_trans_identifier_table_add_symbol(namespace_ -> identifiers, symbol) != 0) {
        symbol = lm_own_ptr_stack_pop(namespace_ -> items);
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static int lm_trans_namespace_declare(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind) {
    return lm_trans_namespace_declare_with_c_name(namespace_, name, kind, 0);
}

static int lm_trans_namespace_declare_generated(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind) {
    const LmTransSymbol *existing;
    LmTransSymbol *symbol;
    if (namespace_ == 0 || name == 0) {
        return 1;
    }
    existing = lm_trans_namespace_find(namespace_, name);
    if (existing != 0) {
        if (lm_trans_symbol_is(existing, kind)) {
            return 0;
        }
        fprintf(stderr, "trans L2 error: generated name \"%.*s\" is already visible as %s\n", (((int)name -> length)), name -> data, lm_trans_symbol_class_name(existing -> class_name));
        return 1;
    }
    symbol = lm_trans_symbol_new(name, kind, namespace_ -> depth);
    if (symbol == 0) {
        return 1;
    }
    if (lm_trans_symbol_set_owned_name(symbol, name) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (namespace_ -> items == 0 || lm_own_ptr_stack_push(namespace_ -> items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    if (lm_trans_identifier_table_add_symbol(namespace_ -> identifiers, symbol) != 0) {
        symbol = lm_own_ptr_stack_pop(namespace_ -> items);
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static int lm_trans_namespace_attach_registry(LmTransNamespace *namespace_) {
    if (namespace_ == 0 || lm_trans_registry == 0) {
        return 1;
    }
    namespace_->registry_identifiers = lm_trans_registry -> identifiers;
    return 0;
}

static int lm_trans_namespace_declare_c_name(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind, const LmP0Text *c_name) {
    return lm_trans_namespace_declare_with_c_name(namespace_, name, kind, c_name);
}

static int lm_trans_namespace_set_env_arg(LmTransNamespace *namespace_, const LmP0Text *name, const LmP0Text *env_arg) {
    LmTransSymbol *symbol;
    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }
    return lm_trans_symbol_set_env_arg(symbol, env_arg);
}

static int lm_trans_namespace_set_closure_call_name(LmTransNamespace *namespace_, const LmP0Text *name, const LmP0Text *closure_call_name) {
    LmTransSymbol *symbol;
    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }
    return lm_trans_symbol_set_closure_call_name(symbol, closure_call_name);
}

static int lm_trans_symbol_copy_signature(LmTransSymbol *target, const LmTransSymbol *source) {
    size_t i;
    const LmP0Text *source_name;
    LmP0Text *copy;
    if (target == 0 || source == 0) {
        return 1;
    }
    if (target -> param_names == 0) {
        target->param_names = lm_trans_ptr_stack_new(lm_trans_text_ref_delete_any);
        if (target -> param_names == 0) {
            return 1;
        }
    }
    else {
        lm_own_ptr_stack_destroy(target -> param_names);
        lm_own_ptr_stack_init(target -> param_names, lm_trans_text_ref_delete_any);
    }
    if (source -> param_names == 0) {
        return 0;
    }
    i = 0U;
    while (i < source -> param_names -> count) {
        source_name = lm_own_ptr_stack_at(source -> param_names, i);
        if (source_name == 0) {
            return 1;
        }
        copy = lm_trans_text_ref_new(source_name);
        if (copy == 0) {
            return 1;
        }
        if (lm_own_ptr_stack_push(target -> param_names, copy) != 0) {
            lm_trans_text_ref_delete_any(copy);
            return 1;
        }
        i = i + 1U;
    }
    target->has_signature = source -> has_signature;
    target->callable_params_node = source -> callable_params_node;
    target->callable_return_node = source -> callable_return_node;
    target->has_callable_shape = source -> has_callable_shape;
    target->callable_returns_value = source -> callable_returns_value;
    target->callable_is_struct_return = source -> callable_is_struct_return;
    target->callable_is_external = source -> callable_is_external;
    return 0;
}

static int lm_trans_namespace_declare_compatible(LmTransNamespace *namespace_, const LmP0Text *name, const char *kind) {
    const LmTransSymbol *existing;
    existing = lm_trans_namespace_find(namespace_, name);
    if (existing != 0) {
        if (lm_trans_symbol_is(existing, kind)) {
            return 0;
        }
    }
    return lm_trans_namespace_declare(namespace_, name, kind);
}

static const char * lm_trans_class_c_spelling(const LmP0Text *name) {
    if (name == 0) {
        return 0;
    }
    return lm_trans_registry_lookup(name, "class.spelling");
}

static int lm_trans_builtin_c_type_tail(const LmP0Text *name) {
    if (name == 0) {
        return 0;
    }
    return lm_trans_class_c_spelling(name) != 0 || lm_trans_registry_has(name, "class.cTail");
}

static int lm_trans_builtin_c_type_name(const LmP0Text *name) {
    LmP0Text *tail;
    int result;
    if (name == 0) {
        return 0;
    }
    if (lm_trans_text_starts_with(name, "c.") && name -> length > 2U) {
        tail = lm_trans_text_ref_tail_new(name, 2U);
        if (tail == 0) {
            return 0;
        }
        result = lm_trans_builtin_c_type_tail(tail);
        lm_trans_text_ref_destroy(&tail);
        return result;
    }
    return lm_trans_builtin_c_type_tail(name);
}

static int lm_trans_emit_name(FILE *file, const LmP0Text *name) {
    const char *spelling;
    LmP0Text *tail;
    int status;
    if (name == 0) {
        return 1;
    }
    if (lm_trans_text_starts_with(name, "c.")) {
        tail = lm_trans_text_ref_tail_new(name, 2U);
        if (tail == 0) {
            return 1;
        }
        spelling = lm_trans_class_c_spelling(tail);
        if (spelling != 0) {
            status = lm_trans_put(file, spelling);
            lm_trans_text_ref_destroy(&tail);
            return status;
        }
        status = lm_trans_write_text(file, tail);
        lm_trans_text_ref_destroy(&tail);
        return status;
    }
    return lm_trans_emit_identifier(file, name);
}

static int lm_trans_emit_type_name(FILE *file, const LmP0Text *name) {
    const char *spelling;
    LmP0Text *tail;
    int status;
    if (name == 0) {
        return 1;
    }
    if (lm_trans_text_starts_with(name, "c.") && name -> length > 2U) {
        tail = lm_trans_text_ref_tail_new(name, 2U);
        if (tail == 0) {
            return 1;
        }
        spelling = lm_trans_class_c_spelling(tail);
        if (spelling != 0) {
            status = lm_trans_put(file, spelling);
            lm_trans_text_ref_destroy(&tail);
            return status;
        }
        status = lm_trans_write_text(file, tail);
        lm_trans_text_ref_destroy(&tail);
        return status;
    }
    spelling = lm_trans_class_c_spelling(name);
    if (spelling != 0) {
        return lm_trans_put(file, spelling);
    }
    return lm_trans_emit_identifier(file, name);
}

static int lm_trans_c_reference_has_path_dot(const LmP0Text *name) {
    size_t i;
    if (lm_trans_is_c_reference_name(name) == 0) {
        return 0;
    }
    i = 2U;
    while (i < name -> length) {
        if (name -> data[i] == '.') {
            return 1;
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_trans_node_is_c_reference_atom(const LmP0Node *node) {
    return node != 0 && node -> kind == LM_P0_NODE_ATOM && lm_trans_is_c_reference_name(node -> as -> atom);
}





static int lm_trans_text_equals(const LmP0Text *text, const char *value) {
    size_t length;
    if (text == 0 || value == 0) {
        return 0;
    }
    length = strlen(value);
    return text -> length == length && memcmp(text -> data, value, length) == 0;
}

static int lm_trans_text_same(const LmP0Text *left, const LmP0Text *right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    if (left -> length != right -> length) {
        return 0;
    }
    if (left -> length == 0U) {
        return 1;
    }
    return memcmp(left -> data, right -> data, left -> length) == 0;
}

static int lm_trans_text_starts_with(const LmP0Text *text, const char *prefix) {
    size_t length;
    if (text == 0 || prefix == 0) {
        return 0;
    }
    length = strlen(prefix);
    return text -> length >= length && memcmp(text -> data, prefix, length) == 0;
}

static int lm_trans_identifier_payload(const LmP0Text *atom, LmP0Text *out_payload) {
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

static int lm_trans_identifier_same(const LmP0Text *left, const LmP0Text *right) {
    LmP0Text *left_payload;
    LmP0Text *right_payload;
    int same;
    left_payload = lm_trans_text_ref_new_cstr("");
    right_payload = lm_trans_text_ref_new_cstr("");
    if (left_payload == 0 || right_payload == 0) {
        lm_trans_text_ref_destroy(&left_payload);
        lm_trans_text_ref_destroy(&right_payload);
        return 0;
    }
    if (lm_trans_identifier_payload(left, left_payload) == 0 || lm_trans_identifier_payload(right, right_payload) == 0) {
        lm_trans_text_ref_destroy(&left_payload);
        lm_trans_text_ref_destroy(&right_payload);
        return 0;
    }
    same = lm_trans_text_same(left_payload, right_payload);
    lm_trans_text_ref_destroy(&left_payload);
    lm_trans_text_ref_destroy(&right_payload);
    return same;
}

static int lm_trans_emit_identifier(FILE *file, const LmP0Text *name) {
    LmP0Text *payload;
    int status;
    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 1;
    }
    if (lm_trans_identifier_payload(name, payload) == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 1;
    }
    status = lm_trans_write_text(file, payload);
    lm_trans_text_ref_destroy(&payload);
    return status;
}

static int lm_trans_registry_identifier_value(const LmP0Text *atom, LmP0Text *out_payload) {
    if (atom == 0 || out_payload == 0) {
        return 0;
    }
    if (atom -> length > 0U && (atom -> data[0] == '"' || atom -> data[0] == '\'')) {
        return 0;
    }
    return lm_trans_identifier_payload(atom, out_payload);
}

static int lm_trans_registry_literal_value(const LmP0Text *atom, LmP0Text *out_payload) {
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
    return lm_trans_identifier_payload(atom, out_payload);
}

static int lm_trans_registry_payload_is_null(const LmP0Text *atom) {
    LmP0Text *payload;
    int is_null;
    payload = lm_trans_text_ref_new_cstr("");
    if (payload == 0) {
        return 0;
    }
    if (lm_trans_registry_identifier_value(atom, payload) == 0) {
        lm_trans_text_ref_destroy(&payload);
        return 0;
    }
    is_null = payload -> length == 4U && memcmp(payload -> data, "NULL", 4U) == 0;
    lm_trans_text_ref_destroy(&payload);
    return is_null;
}

static char * lm_trans_text_copy_cstr(const LmP0Text *text) {
    if (text == 0) {
        return 0;
    }
    return lm_own_copy_bytes(text -> data, text -> length);
}

static char * lm_trans_text_copy_cstr_from_cstr(const char *text) {
    LmP0Text *view;
    char *copy;
    view = lm_trans_text_from_cstr(text);
    if (view == 0) {
        return 0;
    }
    copy = lm_trans_text_copy_cstr(view);
    lm_trans_text_ref_destroy(&view);
    return copy;
}

static char * lm_trans_registry_value_copy_cstr(const LmP0Text *value) {
    char *copy;
    size_t length;
    if (value == 0) {
        return 0;
    }
    length = value -> length;
    copy = lm_own_arena_new_zero(lm_trans_registry->value_arena, length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (length != 0U) {
        memcpy(copy, value -> data, length);
    }
    copy[length] = '\0';
    return copy;
}

static int lm_trans_registry_clone_text(const LmP0Text *source, LmP0Text *out_text) {
    char *copy;
    if (out_text == 0) {
        return 1;
    }
    out_text->data = "";
    out_text->length = 0U;
    if (source == 0) {
        return 1;
    }
    if (source -> length == 0U) {
        return 0;
    }
    if (source -> data == 0) {
        return 1;
    }
    copy = lm_own_arena_copy_bytes(lm_trans_registry->value_arena, source -> data, source -> length);
    if (copy == 0) {
        return 1;
    }
    out_text->data = copy;
    out_text->length = source -> length;
    return 0;
}

static LmP0Text * lm_trans_text_from_cstr(const char *text) {
    return lm_trans_text_ref_new_cstr(text);
}

static int lm_trans_text_assign_cstr(LmP0Text *out_text, const char *text) {
    if (out_text == 0) {
        return 0;
    }
    if (text == 0) {
        text = "";
    }
    out_text->data = text;
    out_text->length = strlen(text);
    return 1;
}

static int lm_trans_text_all_char(const LmP0Text *text, char ch) {
    size_t i;
    size_t length;
    if (text == 0) {
        return 0;
    }
    length = text -> length;
    if (length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < length) {
        if (text -> data[i] != ch) {
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

static int lm_trans_write_text(FILE *file, const LmP0Text *text) {
    if (text == 0) {
        return 1;
    }
    return lm_trans_write_all(file, text -> data, text -> length);
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
    const LmP0Text *atom;
    const LmP0Field *field;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = trailer -> body -> first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    atom = field -> value -> as -> atom;
    out_text[0] = *atom;
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
    return symbol != 0 && symbol -> has_callable_shape != 0 && (lm_trans_symbol_is(symbol, "function") != 0 || lm_trans_symbol_is(symbol, "procedure") != 0 || lm_trans_symbol_is(symbol, "closure") != 0 || lm_trans_symbol_is(symbol, "functionPointer") != 0);
}

static int lm_trans_symbol_is_value_callable(const LmTransSymbol *symbol) {
    return lm_trans_symbol_is_executable_callable(symbol) != 0 && symbol -> callable_returns_value != 0;
}

static void lm_trans_symbol_destroy_fields(LmTransSymbol *symbol) {
    if (symbol != 0) {
        lm_trans_text_ref_destroy(&symbol->name);
        lm_own_delete(symbol -> name_storage, 0);
        symbol->name_storage = 0;
        lm_trans_text_ref_destroy(&symbol->c_name);
        lm_own_delete(symbol -> c_name_storage, 0);
        symbol->c_name_storage = 0;
        symbol->has_c_name = 0;
        lm_trans_text_ref_destroy(&symbol->env_arg);
        lm_own_delete(symbol -> env_arg_storage, 0);
        symbol->env_arg_storage = 0;
        symbol->has_env_arg = 0;
        lm_trans_text_ref_destroy(&symbol->closure_call_name);
        lm_own_delete(symbol -> closure_call_name_storage, 0);
        symbol->closure_call_name_storage = 0;
        symbol->has_closure_call_name = 0;
        lm_trans_ptr_stack_delete(&symbol->param_names);
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
    lm_own_delete(object, 0);
}
