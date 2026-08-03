#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;
typedef void (*LmMessageThreadEntry)(LmMessageThread *thread, void *argument);

typedef struct LmRestLmxHttpServerOptionsV1 {
    size_t abi_size;
    const char *bind_address;
    unsigned port;
    const char *base_path;
    size_t max_body_bytes;
    unsigned worker_count;
    unsigned request_timeout_ms;
} LmRestLmxHttpServerOptionsV1;

const char *lm_rest_lmx_http_server_provider_name(void);
int lm_rest_lmx_http_server_start(
    LmMessageThreadRuntime *runtime,
    const LmRestLmxHttpServerOptionsV1 *options,
    LmRestLmxHttpServer **out_server
);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
int lm_rest_lmx_http_server_port(
    const LmRestLmxHttpServer *server,
    unsigned *out_port
);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
int lm_message_thread_runtime_attach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
);
int lm_message_thread_runtime_detach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
);
LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
LmMessageThread *lm_message_thread_new(void);
LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool);
void lm_message_thread_delete(LmMessageThread *thread);
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
int lm_message_thread_current_lmx(
    LmMessageThread *thread,
    const char **out_lmx,
    size_t *out_length
);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
int lm_message_thread_request_exit(LmMessageThread *requester, int status);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);

enum {
    LM_HTTP_RESPONSE_CAPACITY = 16384,
    LM_HTTP_EXPECTED_MESSAGES = 3,
    LM_HTTP_ACTOR_STATUS = 731
};

typedef struct LmHttpResponse {
    char bytes[LM_HTTP_RESPONSE_CAPACITY];
    size_t length;
    unsigned status;
} LmHttpResponse;

typedef struct LmHttpActorState {
    const unsigned char *expected[LM_HTTP_EXPECTED_MESSAGES];
    size_t expected_length[LM_HTTP_EXPECTED_MESSAGES];
    size_t received;
    int errors;
} LmHttpActorState;

static int lm_http_send_all(int socket_fd, const void *bytes, size_t length) {
    const unsigned char *cursor = (const unsigned char *)bytes;
    size_t sent = 0U;

    while (sent < length) {
        ssize_t step = send(socket_fd, cursor + sent, length - sent, 0);

        if (step > 0) {
            sent += (size_t)step;
        } else if (step < 0 && errno == EINTR) {
            continue;
        } else {
            return 1;
        }
    }
    return 0;
}

static int lm_http_parse_status(LmHttpResponse *response) {
    unsigned major = 0U;
    unsigned minor = 0U;
    unsigned status = 0U;

    if (response == 0 || response->length == 0U ||
        sscanf(
            response->bytes,
            "HTTP/%u.%u %u",
            &major,
            &minor,
            &status
        ) != 3 || major != 1U || status < 100U || status > 599U) {
        return 1;
    }
    (void)minor;
    response->status = status;
    return 0;
}

static int lm_http_request(
    unsigned port,
    const char *headers,
    const unsigned char *body,
    size_t body_length,
    LmHttpResponse *response
) {
    struct sockaddr_in address;
    struct timeval timeout;
    int socket_fd = -1;
    int status = 1;

    if (port == 0U || port > 65535U || headers == 0 || response == 0 ||
        (body == 0 && body_length != 0U)) {
        return 1;
    }
    memset(response, 0, sizeof(*response));
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return 1;
    }
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    (void)setsockopt(
        socket_fd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        (socklen_t)sizeof(timeout)
    );
    (void)setsockopt(
        socket_fd,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        (socklen_t)sizeof(timeout)
    );
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1 ||
        connect(
            socket_fd,
            (const struct sockaddr *)&address,
            (socklen_t)sizeof(address)
        ) != 0 || lm_http_send_all(
            socket_fd,
            headers,
            strlen(headers)
        ) != 0 || (body_length != 0U && lm_http_send_all(
            socket_fd,
            body,
            body_length
        ) != 0) || shutdown(socket_fd, SHUT_WR) != 0) {
        goto cleanup;
    }
    while (response->length + 1U < sizeof(response->bytes)) {
        ssize_t step = recv(
            socket_fd,
            response->bytes + response->length,
            sizeof(response->bytes) - response->length - 1U,
            0
        );

        if (step > 0) {
            response->length += (size_t)step;
        } else if (step == 0) {
            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            goto cleanup;
        }
    }
    if (response->length + 1U >= sizeof(response->bytes)) {
        goto cleanup;
    }
    response->bytes[response->length] = '\0';
    status = lm_http_parse_status(response);

cleanup:
    if (socket_fd >= 0) {
        (void)close(socket_fd);
    }
    return status;
}

static int lm_http_expect(
    unsigned port,
    const char *headers,
    const unsigned char *body,
    size_t body_length,
    unsigned expected_status,
    const char *expected_header
) {
    LmHttpResponse response;

    if (lm_http_request(
            port,
            headers,
            body,
            body_length,
            &response
        ) != 0 || response.status != expected_status ||
        (expected_header != 0 &&
         strstr(response.bytes, expected_header) == 0)) {
        return 1;
    }
    return 0;
}

static void lm_http_actor_receive(
    LmMessageThread *thread,
    void *argument
) {
    LmHttpActorState *state = (LmHttpActorState *)argument;
    const char *lmx = 0;
    size_t length = 0U;
    size_t index;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || lmx == 0) {
        if (state != 0) {
            state->errors |= 1;
        }
        lm_message_thread_request_stop(thread, 901);
        return;
    }
    index = state->received;
    if (index >= LM_HTTP_EXPECTED_MESSAGES ||
        length != state->expected_length[index] ||
        (length != 0U && memcmp(
            lmx,
            state->expected[index],
            length
        ) != 0)) {
        state->errors |= 2;
    }
    state->received += 1U;
    if (state->received == LM_HTTP_EXPECTED_MESSAGES) {
        lm_message_thread_request_stop(thread, LM_HTTP_ACTOR_STATUS);
    }
}

int main(void) {
    static const unsigned char expected_zero[] = {'A', 0U, 'B', 0xffU};
    static const unsigned char expected_two[] = {'2'};
    static const unsigned char expected_encoded[] = {'%', 0U, 'H'};
    static const unsigned char body_zero[] = {'A', 0U, 'B', 0xffU};
    static const unsigned char body_two[] = {'2'};
    static const unsigned char body_encoded[] = {'%', 0U, 'H'};
    static const unsigned char body_missing[] = {'x'};
    static const unsigned char body_short[] = {'a', 'b'};
    static const char post_queue_four[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 4\r\n"
        "Connection: close\r\n\r\n";
    static const char post_queue_one[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 1\r\n"
        "Connection: close\r\n\r\n";
    static const char post_encoded[] =
        "POST /lmx/encoded/%2Fvalue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 3\r\n"
        "Connection: close\r\n\r\n";
    static const char post_malformed_route[] =
        "POST /lmx/bad//route HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    static const char post_query[] =
        "POST /lmx/queue?unexpected=1 HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    static const char post_missing_route[] =
        "POST /lmx/missing HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 1\r\n"
        "Connection: close\r\n\r\n";
    static const char get_queue[] =
        "GET /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    static const char post_missing_length[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Connection: close\r\n\r\n";
    static const char post_chunked[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: close\r\n\r\n"
        "1\r\nx\r\n0\r\n\r\n";
    static const char post_too_large[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/lmx\r\n"
        "Content-Length: 9\r\n"
        "Connection: close\r\n\r\n";
    static const char post_wrong_type[] =
        "POST /lmx/queue HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 1\r\n"
        "Connection: close\r\n\r\n";
    LmHttpActorState actor_state = {{0}, {0U}, 0U, 0};
    LmRestLmxHttpServerOptionsV1 options;
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = lm_message_thread_new();
    LmMessageThread *actor = 0;
    LmRestLmxHttpServer *server = 0;
    unsigned port = 0U;
    int root_attached = 0;
    int root_turn_active = 0;
    int actor_started = 0;
    int actor_joined = 0;
    int actor_status = 0;
    int status = 0;

    (void)signal(SIGPIPE, SIG_IGN);
    actor_state.expected[0] = expected_zero;
    actor_state.expected[1] = expected_two;
    actor_state.expected[2] = expected_encoded;
    actor_state.expected_length[0] = sizeof(expected_zero);
    actor_state.expected_length[1] = sizeof(expected_two);
    actor_state.expected_length[2] = sizeof(expected_encoded);
    options.abi_size = sizeof(options);
    options.bind_address = "127.0.0.1";
    options.port = 0U;
    options.base_path = "/lmx";
    options.max_body_bytes = 8U;
    options.worker_count = 4U;
    options.request_timeout_ms = 2000U;

    if (runtime == 0 || root == 0 ||
        strcmp(lm_rest_lmx_http_server_provider_name(), "civetweb") != 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 2U);
    actor = lm_message_thread_new_in(pool);
    if (pool == 0 || actor == 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_start_mailbox(
            actor,
            lm_http_actor_receive,
            &actor_state
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    actor_started = 1;
    if (lm_message_thread_bind_route(actor, "/queue") != 0 ||
        lm_message_thread_bind_route(actor, "/encoded/%2Fvalue") != 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 2;
        goto cleanup;
    }
    root_attached = 1;
    if (lm_rest_lmx_http_server_start(runtime, &options, &server) != 0 ||
        server == 0 || lm_rest_lmx_http_server_port(server, &port) != 0 ||
        port == 0U || port > 65535U) {
        status = 3;
        goto cleanup;
    }
    if (lm_http_expect(
            port,
            post_queue_four,
            body_zero,
            sizeof(body_zero),
            202U,
            0
        ) || lm_http_expect(
            port,
            post_queue_one,
            body_two,
            sizeof(body_two),
            202U,
            0
        ) || lm_http_expect(
            port,
            post_encoded,
            body_encoded,
            sizeof(body_encoded),
            202U,
            0
        )) {
        status = 4;
        goto cleanup;
    }
    if (lm_message_thread_join(actor, &actor_status) != 0) {
        status = 5;
        goto cleanup;
    }
    actor_joined = 1;
    if (actor_status != LM_HTTP_ACTOR_STATUS || actor_state.errors != 0 ||
        actor_state.received != LM_HTTP_EXPECTED_MESSAGES) {
        status = 5;
        goto cleanup;
    }
    if (lm_http_expect(
            port,
            post_malformed_route,
            0,
            0U,
            400U,
            0
        ) || lm_http_expect(port, post_query, 0, 0U, 400U, 0) ||
        lm_http_expect(
            port,
            post_queue_four,
            body_short,
            sizeof(body_short),
            400U,
            0
        ) || lm_http_expect(
            port,
            post_missing_route,
            body_missing,
            sizeof(body_missing),
            404U,
            0
        ) || lm_http_expect(
            port,
            get_queue,
            0,
            0U,
            405U,
            "Allow: POST"
        ) || lm_http_expect(
            port,
            post_missing_length,
            0,
            0U,
            411U,
            0
        ) || lm_http_expect(port, post_chunked, 0, 0U, 411U, 0) ||
        lm_http_expect(port, post_too_large, 0, 0U, 413U, 0) ||
        lm_http_expect(port, post_wrong_type, 0, 0U, 415U, 0)) {
        status = 6;
        goto cleanup;
    }
    if (lm_message_thread_begin_turn(root) != 1) {
        status = 7;
        goto cleanup;
    }
    root_turn_active = 1;
    if (lm_message_thread_request_exit(root, 29) != 0 || lm_http_expect(
            port,
            post_queue_one,
            body_two,
            sizeof(body_two),
            503U,
            0
        )) {
        status = 7;
    }
    if (lm_message_thread_end_turn(root) != 0 && status == 0) {
        status = 7;
    }
    root_turn_active = 0;

cleanup:
    if (server != 0 && (lm_rest_lmx_http_server_stop(&server) != 0 ||
        server != 0) && status == 0) {
        status = 8;
    }
    if (root_turn_active) {
        (void)lm_message_thread_end_turn(root);
    }
    if (root_attached && lm_message_thread_runtime_detach_root(
            runtime,
            root
        ) != 0 && status == 0) {
        status = 9;
    }
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (actor_started && !actor_joined) {
            (void)lm_message_thread_join(actor, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 10;
        }
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 11;
    }
    if (status != 0) {
        fprintf(stderr, "REST/LMX CivetWeb test failed: %d\n", status);
    }
    return status;
}
