#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endian.h>

#include <libpiper/server.h>

// Send Response
int piper_server_respond(int sock, piper_response *response) {
    if (_safe_write(sock, &response->content_type, sizeof (response->content_type)) != 0) {
        // Failed
        return 1;
    }
    uint64_t fixed_content_length = htole64(response->content_length);
    if (_safe_write(sock, &fixed_content_length, sizeof (fixed_content_length)) != 0) {
        // Failed
        return 1;
    }
    if (_safe_write(sock, (void *) response->content, response->content_length) != 0) {
        // Failed
        return 1;
    }
    // Success
    return 0;
}
int piper_server_respond_str(int sock, uint8_t content_type, const char *format, ...) {
    // Store Return Value
    int ret = 0;

    // Allocate Response String
    char *response_str = NULL;
    {
        va_list args;
        va_start(args, format);
        int vasprintf_ret = vasprintf(&response_str, format, args);
        va_end(args);
        if (vasprintf_ret == -1) {
            return 1;
        }
    }

    // Allocate Response
    size_t response_str_length = strlen(response_str);
    piper_response *response = malloc(sizeof (piper_response) + response_str_length);
    if (response == NULL) {
        // Out Of Memory
        ret = 1;
        goto free_response_str;
    }

    // Setup Response
    response->content_type = content_type;
    response->content_length = response_str_length;
    memcpy((void *) response->content, (void *) response_str, response_str_length);

    // Respond
    ret = piper_server_respond(sock, response);

    // Free
    free(response);
free_response_str:
    free(response_str);
    // Return
    return ret;
}

// Run Server
int piper_server_run(int port, int max_connections, piper_request_callback_t callback, void *user_data) {
    // Store Return Value
    int ret = 0;

    // Open Socket
    int server_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == -1) {
        // Failed
        ERR("%s", "Failed To Create Socket");
        return 1;
    }

    // Configure Socket
    {
        int enable = 1;
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) != 0) {
            // Failed
            ERR("%s", "Failed To Configure Socket");
            ret = 1;
            goto close_server_socket;
        }
    }

    // Bind Socket
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof (server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    server_addr.sin6_addr = in6addr_any;
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof (server_addr)) != 0) {
        // Failed
        ERR("%s", "Failed To Bind");
        ret = 1;
        goto close_server_socket;
    }

    // Listen
    if (listen(server_sock, max_connections) != 0) {
        // Failed
        ERR("%s", "Failed To Listen");
        ret = 1;
        goto close_server_socket;
    }

    // Loop
    struct sockaddr_in6 client_addr;
    memset(&client_addr, 0, sizeof (client_addr));
    socklen_t client_addr_length = sizeof (client_addr);
    int client_sock;
    while ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_length)) >= 0) {
        // Read URI Length
        uint16_t path_length;
        if (_safe_read(client_sock, &path_length, sizeof (path_length)) != 0) {
            // Bad Connection
            goto close_client_socket;
        }
        path_length = le16toh(path_length);

        // Allocate Request
        piper_request *request = malloc(sizeof (piper_request) + path_length + 1 /* NULL-Terminator */ + 1 /* Extra Preceding '/' If Needed */);
        if (request == NULL) {
            // Out Of Memory
            goto free_request;
        }
        request->path_length = path_length;

        // Read URI
        if (_safe_read(client_sock, (void *) request->path, request->path_length) != 0) {
            // Bad Connection
            goto free_request;
        }
        request->path[request->path_length] = '\0';
        // Add Extra Preceding '/' If Needed
        if (request->path[0] != '/') {
            uint8_t full_path_length = request->path_length + 1 /* NULL-Terminator */;
            for (uint8_t j = 0; j < full_path_length; j++) {
                uint8_t i = full_path_length - j - 1;
                request->path[i + 1] = request->path[i];
            }
            request->path[0] = '/';
            request->path_length++;
        }

        // Callback
        int callback_ret = (*callback)(request, client_sock, user_data);

        // Cleanup
 free_request:
        free(request);
 close_client_socket:
        close(client_sock);

        // Exit If Needed
        if (callback_ret != 0) {
            break;
        }
    }

    // Cleanup
 close_server_socket:
    close(server_sock);

    // Return
    return ret;
}
