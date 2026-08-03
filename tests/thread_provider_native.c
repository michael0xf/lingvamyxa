#include <stddef.h>
#include <string.h>

typedef struct LmHostThread LmHostThread;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef void *(*LmHostThreadEntry)(void *argument);

const char *lm_thread_provider_name(void);
LmHostThread *lm_host_thread_new(void);
void lm_host_thread_delete(LmHostThread *thread);
int lm_host_thread_start(
    LmHostThread *thread,
    LmHostThreadEntry entry,
    void *argument
);
int lm_host_thread_join(LmHostThread *thread, void **result);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition *lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_signal(LmCondition *condition);

typedef struct LmThreadProviderTestState {
    LmMutex *mutex;
    LmCondition *condition;
    int entered;
    int release;
    int value;
    int worker_status;
} LmThreadProviderTestState;

static void *lm_thread_provider_test_entry(void *argument) {
    LmThreadProviderTestState *state = (LmThreadProviderTestState *)argument;

    if (lm_mutex_lock(state->mutex) != 0) {
        state->worker_status = 1;
        return state;
    }
    state->entered = 1;
    if (lm_condition_signal(state->condition) != 0) {
        state->worker_status = 2;
    }
    while (!state->release && state->worker_status == 0) {
        if (lm_condition_wait(state->condition, state->mutex) != 0) {
            state->worker_status = 3;
        }
    }
    if (state->worker_status == 0) {
        state->value = 42;
    }
    if (lm_mutex_unlock(state->mutex) != 0 && state->worker_status == 0) {
        state->worker_status = 4;
    }
    return state;
}

int main(void) {
    LmThreadProviderTestState state = {0};
    LmHostThread *thread = 0;
    void *result = 0;
    int status = 0;

    if (strcmp(lm_thread_provider_name(), "pthread") != 0 &&
        strcmp(lm_thread_provider_name(), "win32") != 0) {
        return 1;
    }

    state.mutex = lm_mutex_new();
    state.condition = lm_condition_new();
    thread = lm_host_thread_new();
    if (state.mutex == 0 || state.condition == 0 || thread == 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_host_thread_start(thread, lm_thread_provider_test_entry, &state) != 0) {
        status = 3;
        goto cleanup;
    }
    if (lm_mutex_lock(state.mutex) != 0) {
        status = 4;
        goto cleanup;
    }
    while (!state.entered) {
        if (lm_condition_wait(state.condition, state.mutex) != 0) {
            status = 5;
            break;
        }
    }
    state.release = 1;
    if (lm_condition_signal(state.condition) != 0 && status == 0) {
        status = 6;
    }
    if (lm_mutex_unlock(state.mutex) != 0 && status == 0) {
        status = 7;
    }
    if (lm_host_thread_join(thread, &result) != 0 && status == 0) {
        status = 8;
    }
    if (status == 0 &&
        (result != &state || state.worker_status != 0 || state.value != 42)) {
        status = 9;
    }

cleanup:
    lm_host_thread_delete(thread);
    lm_condition_delete(state.condition);
    lm_mutex_delete(state.mutex);
    return status;
}
