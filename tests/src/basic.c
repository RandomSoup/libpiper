#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <libpiper/client.h>
#include <libpiper/server.h>

// Check If Port Is Open
static int is_port_open(int port) {
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(sock != -1);
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    int ret = connect(sock, (struct sockaddr *) &addr, sizeof (addr));
    close(sock);
    return ret == -1;
}

// Target URL
#define TARGET_PORT 8080
#define TARGET_PATH "/test"
#define SHUTDOWN_REDIRECT_PATH "/shutdown-redirect"
#define SHUTDOWN_PATH "/shutdown"

// Server Callback
static int server_callback(piper_request *request, int sock, __attribute__((unused)) void *user_data) {
    // Handle
    if (strcmp(SHUTDOWN_REDIRECT_PATH, request->path) == 0) {
        assert(piper_server_respond_str(sock, REDIRECT, "%s", SHUTDOWN_PATH) == 0);
    } else {
        assert(piper_server_respond_str(sock, UTF8_TEXT, "Requested URI: %s", request->path) == 0);
    }
    // Return
    return strcmp(SHUTDOWN_PATH, request->path) == 0;
}
static void *server_thread(__attribute__((unused)) void *data) {
    // Start Server
    assert(piper_server_run(TARGET_PORT, 10, server_callback, NULL) == 0);
    return NULL;
}

// Client Callbacks
static void client_callback_after_shutdown(piper_response *response, __attribute__((unused)) void *user_data) {
    assert(response->content_type == UTF8_TEXT);
    assert(response->content_length == strlen(response->content));
    assert(strcmp(response->content, "Requested URI: " SHUTDOWN_PATH) == 0);
}
static void client_callback(piper_response *response, __attribute__((unused)) void *user_data) {
    assert(response->content_type == UTF8_TEXT);
    assert(response->content_length == strlen(response->content));
    assert(strcmp(response->content, "Requested URI: " TARGET_PATH) == 0);
    // Shutdown Server
    piper_url url = {
        .host = "127.0.0.1",
        .port = TARGET_PORT,
        .path = SHUTDOWN_REDIRECT_PATH
    };
    piper_client_send(url, 1, client_callback_after_shutdown, NULL);
}

// Test
int main() {
    // Start Server
    assert(is_port_open(TARGET_PORT));
    pthread_t server_thread_obj;
    pthread_create(&server_thread_obj, NULL, server_thread, NULL);

    // Wait For Server
    while (1) {
        if (!is_port_open(TARGET_PORT)) {
            break;
        }
    }

    // Send Request
    piper_url url = {
        .host = "127.0.0.1",
        .port = TARGET_PORT,
        .path = TARGET_PATH
    };
    piper_client_send(url, 0, client_callback, NULL);

    // Join
    pthread_join(server_thread_obj, NULL);

    // Return
    return EXIT_SUCCESS;
}
