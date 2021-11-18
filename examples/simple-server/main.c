#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpiper/server.h>

// Internal Error
static void internal_error(int sock) {
    piper_response response;
    response.content_type = SERVER_INTERNAL_ERROR;
    response.content_length = 0;
    piper_server_respond(sock, &response);
}

// Callback
static int callback(piper_request *request, int sock) {
    // Log
    printf("New Connection: %s\n", request->path);
    // Choose Behavior
    if (strcmp("/redirect", request->path) == 0) {
        // Redirect At "/redirect"
        if (piper_server_respond_str(sock, REDIRECT, "/hello") != 0) {
            internal_error(sock);
        }
    } else if (strcmp("/shutdown", request->path) == 0) {
        // Shutdown Server
        if (piper_server_respond_str(sock, UTF8_TEXT, "Goodbye.") != 0) {
            internal_error(sock);
        }
        return 1;
    } else {
        // Everything Else
        if (piper_server_respond_str(sock, UTF8_TEXT, "Hello From \"%s\"!", request->path) != 0) {
            internal_error(sock);
        }
    }
    // Return
    return 0;
}

// Main
int main() {
    printf("Starting...\n");
    // Run
    if (piper_server_run(60, 10, callback) != 0) {
        fprintf(stderr, "Failed To Start Server\n");
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
