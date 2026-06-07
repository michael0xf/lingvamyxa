#include <stddef.h>

typedef int LmOwnEdgeKind;


typedef struct LmOwnPtrStack LmOwnPtrStack;
typedef struct LmOwnValueStack LmOwnValueStack;
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
typedef struct LmOwnArena LmOwnArena;


#define LM_OWN_EDGE_BORROWED 1
#define LM_OWN_EDGE_OWNED 2
#define LM_OWN_EDGE_LAZY_OWNED 3
#define LM_OWN_EDGE_EXTERNAL 4


typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);


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


typedef struct AddXClosure AddXClosure;
typedef AddXClosure *AddX;
struct AddXClosure {
    int (*call)(void *env, int);
    void *env;
    void (*destroy)(void *env);
};
static AddX makeAdder(int base);
int main(void);

typedef struct lm_makeAdder_addX_0Env lm_makeAdder_addX_0Env;

struct lm_makeAdder_addX_0Env {
    int base;
    int extra;
};

static int lm_makeAdder_addX_0(lm_makeAdder_addX_0Env *lm_env, int value) {
    return lm_env->base + lm_env->extra + value;
}

static int lm_makeAdder_addX_0_closure_call(void *lm_env, int value) {
    return lm_makeAdder_addX_0((lm_makeAdder_addX_0Env *)lm_env, value);
}

static AddX makeAdder(int base) {
    int extra = 5;
    lm_makeAdder_addX_0Env *lm_makeAdder_addX_0_env;
    lm_makeAdder_addX_0_env = (lm_makeAdder_addX_0Env *)lm_own_new_zero(sizeof(*lm_makeAdder_addX_0_env));
    if (lm_makeAdder_addX_0_env == 0) {
        return 0;
    }
    lm_makeAdder_addX_0_env->base = base;
    lm_makeAdder_addX_0_env->extra = extra;
    (void)lm_makeAdder_addX_0_closure_call;
    {
        AddX lm_return_0;
        lm_return_0 = (AddX)lm_own_new_zero(sizeof(*lm_return_0));
        if (lm_return_0 == 0) {
            return 0;
        }
        lm_return_0->call = lm_makeAdder_addX_0_closure_call;
        lm_return_0->env = lm_makeAdder_addX_0_env;
        lm_return_0->destroy = 0;
        return lm_return_0;
    }
}

int main(void) {
    AddX add = makeAdder(4);
    return add == 0 || add->call(add->env, 3) != 12;
}
