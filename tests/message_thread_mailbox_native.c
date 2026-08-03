#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMutex LmMutex;
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
int lm_message_thread_start_mailbox(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
);
int lm_message_thread_join(LmMessageThread *thread, int *result);
int lm_message_thread_bind_route(
    LmMessageThread *thread,
    const char *route
);
int lm_message_thread_send_lmx(
    LmMessageThread *sender,
    const char *endpoint,
    const char *route,
    const char *lmx,
    size_t length
);
int lm_message_thread_current_lmx(
    LmMessageThread *thread,
    const char **out_lmx,
    size_t *out_length
);
size_t lm_message_thread_inbox_count(const LmMessageThread *thread);
size_t lm_message_thread_outbox_count(const LmMessageThread *thread);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
size_t lm_message_thread_collection_count(const LmMessageThread *thread);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);

enum {
    LM_MAILBOX_NATIVE_WORKERS = 4,
    LM_MAILBOX_NATIVE_SENDERS = 8,
    LM_MAILBOX_NATIVE_MESSAGES = 64,
    LM_MAILBOX_NATIVE_ITERATIONS = 20,
    LM_MAILBOX_NATIVE_RECEIVER_STATUS = 777
};

typedef struct LmMailboxNativeState LmMailboxNativeState;

typedef struct LmMailboxNativeSender {
    LmMailboxNativeState *state;
    LmMessageThread *thread;
    size_t identifier;
    size_t queued;
} LmMailboxNativeSender;

struct LmMailboxNativeState {
    LmMutex *mutex;
    LmMessageThread *receiver;
    size_t next_sequence[LM_MAILBOX_NATIVE_SENDERS];
    unsigned char seen[LM_MAILBOX_NATIVE_SENDERS]
        [LM_MAILBOX_NATIVE_MESSAGES];
    size_t received;
    int receiver_active;
    int errors;
};

static void lm_mailbox_native_record_error(
    LmMailboxNativeState *state,
    int error
) {
    if (lm_mutex_lock(state->mutex) != 0) {
        return;
    }
    state->errors |= error;
    (void)lm_mutex_unlock(state->mutex);
}

static void lm_mailbox_native_receive(
    LmMessageThread *thread,
    void *argument
) {
    LmMailboxNativeState *state = (LmMailboxNativeState *)argument;
    const char *lmx = 0;
    size_t length = 0U;
    size_t sender = 0U;
    size_t sequence = 0U;
    int consumed = 0;
    int should_stop = 0;

    if (state == 0 || thread != state->receiver ||
        lm_message_thread_current_lmx(thread, &lmx, &length) != 0 ||
        lmx == 0 ||
        sscanf(lmx, "sender:%zu:%zu%n", &sender, &sequence, &consumed) != 2 ||
        consumed < 0 || (size_t)consumed != length ||
        sender >= LM_MAILBOX_NATIVE_SENDERS ||
        sequence >= LM_MAILBOX_NATIVE_MESSAGES) {
        if (state != 0) {
            lm_mailbox_native_record_error(state, 1);
        }
        lm_message_thread_request_stop(thread, 901);
        return;
    }

    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 902);
        return;
    }
    if (state->receiver_active != 0) {
        state->errors |= 2;
    }
    state->receiver_active += 1;
    if (state->seen[sender][sequence] != 0U) {
        state->errors |= 4;
    }
    if (sequence != state->next_sequence[sender]) {
        state->errors |= 8;
    }
    state->seen[sender][sequence] = 1U;
    state->next_sequence[sender] += 1U;
    state->received += 1U;
    state->receiver_active -= 1;
    should_stop = state->received ==
        (size_t)LM_MAILBOX_NATIVE_SENDERS * LM_MAILBOX_NATIVE_MESSAGES;
    (void)lm_mutex_unlock(state->mutex);

    if (should_stop) {
        lm_message_thread_request_stop(
            thread,
            LM_MAILBOX_NATIVE_RECEIVER_STATUS
        );
    }
}

static void lm_mailbox_native_send(
    LmMessageThread *thread,
    void *argument
) {
    LmMailboxNativeSender *sender = (LmMailboxNativeSender *)argument;
    const char *unexpected = 0;
    size_t unexpected_length = 0U;
    size_t sequence;

    if (sender == 0 || sender->state == 0 || thread != sender->thread) {
        lm_message_thread_request_stop(thread, 903);
        return;
    }
    if (lm_message_thread_current_lmx(
            thread,
            &unexpected,
            &unexpected_length
        ) == 0) {
        lm_mailbox_native_record_error(sender->state, 16);
    }

    for (sequence = 0U;
         sequence < LM_MAILBOX_NATIVE_MESSAGES;
         sequence += 1U) {
        char buffer[64];
        int length = snprintf(
            buffer,
            sizeof(buffer),
            "sender:%zu:%zu",
            sender->identifier,
            sequence
        );

        if (length < 0 || (size_t)length >= sizeof(buffer) ||
            lm_message_thread_send_lmx(
                thread,
                0,
                "/native-mailbox",
                buffer,
                (size_t)length
            ) != 0) {
            lm_mailbox_native_record_error(sender->state, 32);
            break;
        }
        sender->queued += 1U;
        memset(buffer, 'x', (size_t)length);
    }
    if (sender->queued != LM_MAILBOX_NATIVE_MESSAGES ||
        lm_message_thread_outbox_count(thread) != sender->queued) {
        lm_mailbox_native_record_error(sender->state, 64);
    }
    lm_message_thread_request_stop(
        thread,
        200 + (int)sender->identifier
    );
}

static int lm_mailbox_native_iteration(void) {
    LmMailboxNativeState state;
    LmMailboxNativeSender senders[LM_MAILBOX_NATIVE_SENDERS];
    int sender_started[LM_MAILBOX_NATIVE_SENDERS] = {0};
    int sender_joined[LM_MAILBOX_NATIVE_SENDERS] = {0};
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    const char *outside_lmx = 0;
    size_t outside_length = 0U;
    size_t index;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = 0;
    int status = 0;

    memset(&state, 0, sizeof(state));
    memset(senders, 0, sizeof(senders));
    state.mutex = lm_mutex_new();
    if (state.mutex == 0) {
        return 2;
    }
    runtime = lm_message_thread_runtime_new();
    pool = lm_message_thread_pool_new(runtime, LM_MAILBOX_NATIVE_WORKERS);
    if (runtime == 0 || pool == 0) {
        status = 3;
        goto cleanup;
    }
    state.receiver = lm_message_thread_new_in(pool);
    if (state.receiver == 0 || lm_message_thread_start_mailbox(
            state.receiver,
            lm_mailbox_native_receive,
            &state
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    receiver_started = 1;
    if (lm_message_thread_bind_route(
            state.receiver,
            "/native-mailbox"
        ) != 0) {
        status = 5;
        goto cleanup;
    }
    if (lm_message_thread_current_lmx(
            state.receiver,
            &outside_lmx,
            &outside_length
        ) == 0) {
        status = 6;
        goto cleanup;
    }

    for (index = 0U; index < LM_MAILBOX_NATIVE_SENDERS; index += 1U) {
        senders[index].state = &state;
        senders[index].identifier = index;
        senders[index].thread = lm_message_thread_new_in(pool);
        if (senders[index].thread == 0 || lm_message_thread_start(
                senders[index].thread,
                lm_mailbox_native_send,
                &senders[index]
            ) != 0) {
            status = 7;
            break;
        }
        sender_started[index] = 1;
    }

    for (index = 0U;
         status == 0 && index < LM_MAILBOX_NATIVE_SENDERS;
         index += 1U) {
        if (lm_message_thread_join(
                senders[index].thread,
                &actor_status
            ) != 0 || actor_status != 200 + (int)index ||
            senders[index].queued != LM_MAILBOX_NATIVE_MESSAGES ||
            lm_message_thread_outbox_count(senders[index].thread) != 0U) {
            status = 8;
            break;
        }
        sender_joined[index] = 1;
    }
    if (status != 0 && receiver_started) {
        lm_message_thread_request_stop(state.receiver, 904);
    }
    if (receiver_started && lm_message_thread_join(
            state.receiver,
            &actor_status
        ) == 0) {
        receiver_joined = 1;
        if (status == 0 && actor_status != LM_MAILBOX_NATIVE_RECEIVER_STATUS) {
            status = 9;
        }
    } else if (status == 0) {
        status = 10;
    }

    if (status == 0 &&
        (state.errors != 0 ||
         state.received !=
            (size_t)LM_MAILBOX_NATIVE_SENDERS * LM_MAILBOX_NATIVE_MESSAGES ||
         lm_message_thread_turn_count(state.receiver) != state.received ||
         lm_message_thread_collection_count(state.receiver) != state.received ||
         lm_message_thread_inbox_count(state.receiver) != 0U ||
         lm_message_thread_outbox_count(state.receiver) != 0U)) {
        status = 11;
    }
    for (index = 0U;
         status == 0 && index < LM_MAILBOX_NATIVE_SENDERS;
         index += 1U) {
        if (state.next_sequence[index] != LM_MAILBOX_NATIVE_MESSAGES) {
            status = 12;
        }
    }

cleanup:
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        for (index = 0U; index < LM_MAILBOX_NATIVE_SENDERS; index += 1U) {
            if (sender_started[index] && !sender_joined[index]) {
                (void)lm_message_thread_join(
                    senders[index].thread,
                    &actor_status
                );
            }
        }
        if (receiver_started && !receiver_joined) {
            (void)lm_message_thread_join(state.receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 13;
        }
    }
    if (runtime != 0 &&
        lm_message_thread_runtime_delete(runtime) != 0 && status == 0) {
        status = 14;
    }
    lm_mutex_delete(state.mutex);
    return status;
}

int main(void) {
    int iteration;

    if (strcmp(lm_thread_provider_name(), "pthread") != 0 &&
        strcmp(lm_thread_provider_name(), "win32") != 0) {
        return 1;
    }
    for (iteration = 0;
         iteration < LM_MAILBOX_NATIVE_ITERATIONS;
         iteration += 1) {
        int status = lm_mailbox_native_iteration();

        if (status != 0) {
            fprintf(
                stderr,
                "native mailbox iteration %d failed: %d\n",
                iteration,
                status
            );
            return status;
        }
    }
    return 0;
}
