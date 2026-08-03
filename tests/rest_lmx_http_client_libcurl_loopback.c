#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;

const char *lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);
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
LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
int lm_message_thread_send_lmx(
    LmMessageThread *sender,
    const char *endpoint,
    const char *route,
    const char *lmx,
    size_t length
);

static size_t lm_rest_lmx_find_bytes(
    const unsigned char *data,
    size_t length,
    const char *needle
) {
    size_t needle_length = strlen(needle);
    size_t index;

    if (needle_length == 0U || needle_length > length) {
        return SIZE_MAX;
    }
    for (index = 0U; index <= length - needle_length; index += 1U) {
        if (memcmp(data + index, needle, needle_length) == 0) {
            return index;
        }
    }
    return SIZE_MAX;
}

static int lm_rest_lmx_send_all(
    int socket_fd,
    const char *data,
    size_t length
) {
    size_t sent = 0U;

    while (sent < length) {
        ssize_t result = send(socket_fd, data + sent, length - sent, 0);

        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            return 1;
        }
        sent += (size_t)result;
    }
    return 0;
}

static int lm_rest_lmx_loopback_serve(int listener) {
    static const char response[] =
        "HTTP/1.1 204 No Content\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";
    static const unsigned char expected_body[] = {'A', '\0', 'B'};
    unsigned char request[16384];
    int connection;
    size_t received = 0U;
    size_t header_marker = SIZE_MAX;
    size_t body_offset;

    alarm(10U);
    do {
        connection = accept(listener, 0, 0);
    } while (connection < 0 && errno == EINTR);
    if (connection < 0) {
        return 1;
    }
    while (received < sizeof(request)) {
        ssize_t result = recv(
            connection,
            request + received,
            sizeof(request) - received,
            0
        );

        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            break;
        }
        received += (size_t)result;
        header_marker = lm_rest_lmx_find_bytes(
            request,
            received,
            "\r\n\r\n"
        );
        if (header_marker != SIZE_MAX && received >= header_marker + 7U) {
            break;
        }
    }
    body_offset = header_marker == SIZE_MAX ? SIZE_MAX : header_marker + 4U;
    if (body_offset == SIZE_MAX || received != body_offset + 3U ||
        lm_rest_lmx_find_bytes(
            request,
            header_marker,
            "POST /base/a/%2e%2e/message HTTP/1.1\r\n"
        ) != 0U ||
        lm_rest_lmx_find_bytes(
            request,
            body_offset,
            "\r\nContent-Type: application/lmx\r\n"
        ) == SIZE_MAX ||
        lm_rest_lmx_find_bytes(
            request,
            body_offset,
            "\r\nAccept: application/lmx\r\n"
        ) == SIZE_MAX ||
        lm_rest_lmx_find_bytes(
            request,
            body_offset,
            "\r\nContent-Length: 3\r\n"
        ) == SIZE_MAX ||
        memcmp(request + body_offset, expected_body, sizeof(expected_body)) !=
            0) {
        fprintf(
            stderr,
            "loopback request mismatch: received=%zu header=%zu body=%zu\n",
            received,
            header_marker,
            body_offset
        );
        if (header_marker != SIZE_MAX) {
            (void)fwrite(request, 1U, body_offset, stderr);
            (void)fputc('\n', stderr);
        }
        (void)close(connection);
        return 2;
    }
    if (lm_rest_lmx_send_all(connection, response, sizeof(response) - 1U) !=
        0) {
        (void)close(connection);
        return 3;
    }
    (void)close(connection);
    return 0;
}

static int lm_rest_lmx_loopback_client(unsigned port) {
    static const char body[] = {'A', '\0', 'B'};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    char endpoint[128];
    int endpoint_length;
    int attached = 0;
    int status = 0;

    endpoint_length = snprintf(
        endpoint,
        sizeof(endpoint),
        "http://127.0.0.1:%u/base/",
        port
    );
    if (runtime == 0 || root == 0 || endpoint_length < 0 ||
        (size_t)endpoint_length >= sizeof(endpoint) ||
        strcmp(lm_rest_lmx_http_client_provider_name(), "libcurl") != 0 ||
        lm_rest_lmx_http_client_install_default(runtime) != 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            endpoint,
            "/a/%2e%2e/message",
            body,
            sizeof(body)
        ) != 0 || lm_message_thread_end_turn(root) != 0) {
        status = 2;
    }

cleanup:
    if (attached && lm_message_thread_runtime_detach_root(runtime, root) != 0 &&
        status == 0) {
        status = 3;
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 4;
    }
    return status;
}

int main(void) {
    struct sockaddr_in address;
    socklen_t address_length = (socklen_t)sizeof(address);
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    pid_t server;
    int client_status;
    int child_status = 0;

    if (listener < 0) {
        return 1;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(UINT32_C(0x7f000001));
    address.sin_port = htons(0U);
    if (bind(listener, (const struct sockaddr *)&address, sizeof(address)) !=
            0 || listen(listener, 1) != 0 || getsockname(
                listener,
                (struct sockaddr *)&address,
                &address_length
            ) != 0) {
        (void)close(listener);
        return 2;
    }

    server = fork();
    if (server < 0) {
        (void)close(listener);
        return 3;
    }
    if (server == 0) {
        int server_status = lm_rest_lmx_loopback_serve(listener);

        (void)close(listener);
        _exit(server_status);
    }

    (void)close(listener);
    client_status = lm_rest_lmx_loopback_client(
        (unsigned)ntohs(address.sin_port)
    );
    if (client_status != 0) {
        (void)kill(server, SIGTERM);
    }
    while (waitpid(server, &child_status, 0) < 0) {
        if (errno != EINTR) {
            child_status = -1;
            break;
        }
    }
    if (client_status != 0 || !WIFEXITED(child_status) ||
        WEXITSTATUS(child_status) != 0) {
        fprintf(
            stderr,
            "REST/LMX libcurl loopback failed: client=%d child=%d\n",
            client_status,
            child_status
        );
        return 4;
    }
    return 0;
}
