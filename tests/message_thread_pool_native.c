#include <stddef.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef void (*LmMessageThreadEntry)(LmMessageThread *thread, void *argument);

const char *lm_thread_provider_name(void);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool);
int lm_message_thread_start(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
);
int lm_message_thread_join(LmMessageThread *thread, int *result);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
int lm_message_thread_is_running(const LmMessageThread *thread);
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
size_t lm_message_thread_collection_count(const LmMessageThread *thread);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition *lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_broadcast(LmCondition *condition);

enum {
    LM_POOL_TEST_WORKERS = 4,
    LM_POOL_TEST_ACTORS = 8,
    LM_POOL_TEST_TURNS = 12,
    LM_POOL_TEST_STOP_ACTORS = 8
};

typedef struct LmPoolTestState LmPoolTestState;

typedef struct LmPoolTestActor {
    LmPoolTestState *state;
    LmMessageThread *thread;
    size_t turns;
    int identifier;
    int active;
} LmPoolTestActor;

struct LmPoolTestState {
    LmMutex *mutex;
    LmCondition *barrier;
    int first_turn_arrived;
    int barrier_failed;
    int active_total;
    int maximum_active;
    int errors;
    int callback_join_result;
    int callback_runtime_delete_result;
    LmMessageThreadRuntime *runtime;
    LmMessageThread *peer_thread;
    LmMessageThread *foreign_thread;
};

static void lm_pool_test_foreign_entry(
    LmMessageThread *thread,
    void *argument
) {
    (void)argument;
    lm_message_thread_request_stop(thread, 177);
}

static void lm_pool_test_actor_entry(
    LmMessageThread *thread,
    void *argument
) {
    LmPoolTestActor *actor = (LmPoolTestActor *)argument;
    LmPoolTestState *state = actor->state;
    int first_turn;
    int should_stop;

    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 199);
        return;
    }
    if (thread != actor->thread) {
        state->errors |= 1;
    }
    if (actor->active != 0) {
        state->errors |= 2;
    }
    actor->active += 1;
    state->active_total += 1;
    if (state->active_total > state->maximum_active) {
        state->maximum_active = state->active_total;
    }

    actor->turns += 1U;
    first_turn = actor->turns == 1U;
    if (first_turn) {
        if (actor->identifier == 0) {
            int ignored_status = 0;
            int foreign_join_result;

            state->callback_join_result = lm_message_thread_join(
                thread,
                &ignored_status
            );
            if (state->callback_join_result != 1) {
                state->errors |= 64;
            }
            state->callback_runtime_delete_result =
                lm_message_thread_runtime_delete(state->runtime);
            if (state->callback_runtime_delete_result != 1) {
                state->errors |= 256;
            }
            lm_message_thread_request_stop(state->peer_thread, 188);
            foreign_join_result = lm_message_thread_join(
                state->foreign_thread,
                &ignored_status
            );
            if (foreign_join_result != 1) {
                state->errors |= 128;
            }
            (void)lm_condition_broadcast(state->barrier);
        }
        state->first_turn_arrived += 1;
        if (state->first_turn_arrived >= LM_POOL_TEST_WORKERS) {
            if (lm_condition_broadcast(state->barrier) != 0) {
                state->errors |= 4;
            }
        }
        while (state->first_turn_arrived < LM_POOL_TEST_WORKERS) {
            if (lm_condition_wait(state->barrier, state->mutex) != 0) {
                state->barrier_failed = 1;
                state->errors |= 8;
                (void)lm_condition_broadcast(state->barrier);
                break;
            }
        }
    }

    actor->active -= 1;
    state->active_total -= 1;
    should_stop = actor->turns >= LM_POOL_TEST_TURNS;
    (void)lm_mutex_unlock(state->mutex);

    if (should_stop) {
        lm_message_thread_request_stop(thread, 100 + actor->identifier);
    }
}

static void lm_pool_test_stop_entry(
    LmMessageThread *thread,
    void *argument
) {
    LmPoolTestActor *actor = (LmPoolTestActor *)argument;
    LmPoolTestState *state = actor->state;

    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 198);
        return;
    }
    if (thread != actor->thread) {
        state->errors |= 16;
    }
    if (actor->active != 0) {
        state->errors |= 32;
    }
    actor->active += 1;
    actor->turns += 1U;
    actor->active -= 1;
    (void)lm_mutex_unlock(state->mutex);
}

int main(void) {
    LmPoolTestState state;
    LmPoolTestActor actors[LM_POOL_TEST_ACTORS] = {{0}};
    LmPoolTestActor stop_actors[LM_POOL_TEST_STOP_ACTORS] = {{0}};
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    LmMessageThreadPool *foreign_pool = 0;
    LmMessageThread *foreign_thread = 0;
    size_t started = 0U;
    size_t stop_started = 0U;
    size_t index;
    int status = 0;
    int delete_status = 0;

    if (strcmp(lm_thread_provider_name(), "pthread") != 0 &&
        strcmp(lm_thread_provider_name(), "win32") != 0) {
        return 1;
    }
    state.mutex = lm_mutex_new();
    if (state.mutex == 0) {
        return 2;
    }
    state.barrier = lm_condition_new();
    if (state.barrier == 0) {
        lm_mutex_delete(state.mutex);
        return 3;
    }
    state.first_turn_arrived = 0;
    state.barrier_failed = 0;
    state.active_total = 0;
    state.maximum_active = 0;
    state.errors = 0;
    state.callback_join_result = -1;
    state.callback_runtime_delete_result = -1;
    state.runtime = 0;
    state.peer_thread = 0;
    state.foreign_thread = 0;

    runtime = lm_message_thread_runtime_new();
    if (runtime == 0) {
        status = 24;
        goto cleanup;
    }
    state.runtime = runtime;
    foreign_pool = lm_message_thread_pool_new(runtime, 1U);
    if (foreign_pool == 0) {
        status = 21;
        goto cleanup;
    }
    foreign_thread = lm_message_thread_new_in(foreign_pool);
    if (foreign_thread == 0 || lm_message_thread_start(
            foreign_thread,
            lm_pool_test_foreign_entry,
            0
        ) != 0) {
        status = 22;
        goto cleanup;
    }
    state.foreign_thread = foreign_thread;

    pool = lm_message_thread_pool_new(runtime, LM_POOL_TEST_WORKERS);
    if (pool == 0) {
        status = 4;
        goto cleanup;
    }

    for (index = 0U; index < LM_POOL_TEST_ACTORS; index += 1U) {
        actors[index].state = &state;
        actors[index].identifier = (int)index;
        actors[index].thread = lm_message_thread_new_in(pool);
        if (actors[index].thread == 0) {
            status = 5;
            goto cleanup;
        }
    }
    state.peer_thread = actors[1].thread;

    if (lm_message_thread_start(
            actors[0].thread,
            lm_pool_test_actor_entry,
            &actors[0]
        ) != 0) {
        status = 6;
        goto cleanup;
    }
    started = 1U;
    if (lm_message_thread_start(
            actors[0].thread,
            lm_pool_test_actor_entry,
            &actors[0]
        ) == 0) {
        status = 7;
        goto cleanup;
    }
    for (index = 1U; index < LM_POOL_TEST_ACTORS; index += 1U) {
        if (lm_message_thread_start(
                actors[index].thread,
                lm_pool_test_actor_entry,
                &actors[index]
            ) != 0) {
            status = 8;
            goto cleanup;
        }
        started += 1U;
    }

    if (lm_mutex_lock(state.mutex) != 0) {
        status = 19;
        goto cleanup;
    }
    while (state.callback_join_result < 0) {
        if (lm_condition_wait(state.barrier, state.mutex) != 0) {
            status = 20;
            break;
        }
    }
    (void)lm_mutex_unlock(state.mutex);
    if (status != 0) {
        goto cleanup;
    }

    for (index = 0U; index < LM_POOL_TEST_ACTORS; index += 1U) {
        int actor_status = 0;

        if (lm_message_thread_join(actors[index].thread, &actor_status) != 0) {
            status = 9;
            goto cleanup;
        }
        if (actor_status != 100 + (int)index) {
            status = 10;
            goto cleanup;
        }
        if (actors[index].turns != LM_POOL_TEST_TURNS ||
            lm_message_thread_turn_count(actors[index].thread) !=
                LM_POOL_TEST_TURNS ||
            lm_message_thread_collection_count(actors[index].thread) !=
                LM_POOL_TEST_TURNS) {
            status = 11;
            goto cleanup;
        }
    }
    if (state.errors != 0 ||
        state.maximum_active < LM_POOL_TEST_WORKERS ||
        state.first_turn_arrived < LM_POOL_TEST_WORKERS) {
        status = 12;
        goto cleanup;
    }
    {
        int ignored_status = 0;

        if (lm_message_thread_join(actors[0].thread, &ignored_status) == 0 ||
            lm_message_thread_start(
                actors[0].thread,
                lm_pool_test_actor_entry,
                &actors[0]
            ) == 0) {
            status = 13;
            goto cleanup;
        }
    }

    for (index = 0U; index < LM_POOL_TEST_STOP_ACTORS; index += 1U) {
        stop_actors[index].state = &state;
        stop_actors[index].identifier = (int)index;
        stop_actors[index].thread = lm_message_thread_new_in(pool);
        if (stop_actors[index].thread == 0 ||
            lm_message_thread_start(
                stop_actors[index].thread,
                lm_pool_test_stop_entry,
                &stop_actors[index]
            ) != 0) {
            status = 14;
            goto cleanup;
        }
        stop_started += 1U;
    }

    lm_message_thread_pool_request_stop(pool);
    for (index = 0U; index < LM_POOL_TEST_STOP_ACTORS; index += 1U) {
        int actor_status = -1;
        size_t turns;
        size_t collections;

        if (lm_message_thread_join(stop_actors[index].thread, &actor_status) != 0) {
            status = 15;
            goto cleanup;
        }
        turns = lm_message_thread_turn_count(stop_actors[index].thread);
        collections = lm_message_thread_collection_count(
            stop_actors[index].thread
        );
        if (actor_status != 0 ||
            lm_message_thread_is_running(stop_actors[index].thread) != 0 ||
            stop_actors[index].turns != turns ||
            (collections != turns && collections != turns + 1U)) {
            status = 16;
            goto cleanup;
        }
    }
    if (state.errors != 0 || lm_message_thread_new_in(pool) != 0) {
        status = 17;
    }

cleanup:
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        for (index = 0U; index < started; index += 1U) {
            int ignored_status = 0;

            (void)lm_message_thread_join(
                actors[index].thread,
                &ignored_status
            );
        }
        for (index = 0U; index < stop_started; index += 1U) {
            int ignored_status = 0;

            (void)lm_message_thread_join(
                stop_actors[index].thread,
                &ignored_status
            );
        }
        delete_status = lm_message_thread_pool_delete(pool);
        if (delete_status != 0 && status == 0) {
            status = 18;
        }
    }
    if (foreign_pool != 0) {
        int foreign_status = 0;
        int foreign_delete_status;

        lm_message_thread_pool_request_stop(foreign_pool);
        if (foreign_thread != 0) {
            (void)lm_message_thread_join(foreign_thread, &foreign_status);
        }
        foreign_delete_status = lm_message_thread_pool_delete(foreign_pool);
        if (foreign_delete_status != 0) {
            delete_status = foreign_delete_status;
            if (status == 0) {
                status = 23;
            }
        }
    }
    if (runtime != 0) {
        int runtime_delete_status =
            lm_message_thread_runtime_delete(runtime);

        if (runtime_delete_status != 0 && status == 0) {
            status = 25;
        }
    }
    if (delete_status == 0) {
        lm_condition_delete(state.barrier);
        lm_mutex_delete(state.mutex);
    }
    return status;
}
