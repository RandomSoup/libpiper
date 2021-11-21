#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <endian.h>
#include <errno.h>

#include <libpiper/client.h>

// Error
static void _send_empty(uint8_t type, piper_response_callback_t callback, void *user_data) {
    static piper_response response;
    response.content_type = type;
    response.content_length = 0;
    callback(&response, user_data);
}

// Handle Redirects
static void _handle_redirects(piper_url url, piper_response *response, piper_response_callback_t callback, void *user_data) {
    // Redirect
    piper_url new_url;
    // Resolve
    if (piper_resolve_url(url, response->content, &new_url) == 0) {
        // Respond
        piper_client_send(new_url, 1, callback, user_data);
        // Free
        free(new_url.path);
        free(new_url.host);
    }
}

// Send Request
void piper_client_send(piper_url url, int handle_redirects, piper_response_callback_t callback, void *user_data) {
    // Open Socket
    int sock = -1;
    {
        // Resolve Host
        struct addrinfo hints = {}, *addrs;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
#define PORT_STR_SIZE 16
        char port_str[PORT_STR_SIZE] = {};
        snprintf(port_str, PORT_STR_SIZE, "%d", url.port);
        if (getaddrinfo(url.host, port_str, &hints, &addrs) != 0) {
            ERR("%s", "DNS Lookup Failed");
            _send_empty(CLIENT_INTERNAL_ERROR, callback, user_data);
            return;
        }

        // Connect
        for (struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next) {
            sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (sock == -1) {
                continue;
            }
            if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0) {
                // Success
                break;
            }
            close(sock);
            sock = -1;
        }

        // Free
        freeaddrinfo(addrs);
    }
    if (sock == -1) {
        ERR("%s", "Failed To Connect");
        _send_empty(CLIENT_INTERNAL_ERROR, callback, user_data);
        return;
    }

    // Allocate Request
    piper_request *request = NULL;
    {
        size_t path_length = strlen(url.path);
        if (path_length > UINT16_MAX) {
            path_length = UINT16_MAX;
        }
        request = malloc(sizeof (piper_request) + path_length);
        if (request == NULL) {
            // Out Of Memory
            _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback, user_data);
            return;
        }
        request->path_length = (uint16_t) path_length;
        memcpy((void *) request->path, (void *) url.path, path_length);
        request->path_length = htole16(request->path_length);
    }

    // Write Request
    if (_safe_write(sock, request, sizeof (piper_request) + request->path_length) != 0) {
        _send_empty(CLIENT_CONNECTION_ERROR, callback, user_data);
        goto free_request;
    }

    // Read Response
    piper_response *response = NULL;
    {
        uint8_t response_content_type;
        if (_safe_read(sock, &response_content_type, sizeof (response_content_type)) != 0) {
            _send_empty(CLIENT_CONNECTION_ERROR, callback, user_data);
            goto free_request;
        }
        uint64_t response_content_size;
        if (_safe_read(sock, &response_content_size, sizeof (response_content_size)) != 0) {
            _send_empty(CLIENT_CONNECTION_ERROR, callback, user_data);
            goto free_request;
        }
        response_content_size = le64toh(response_content_size);
        // Handle Dynamic Content
        if (response_content_size == PIPER_DYNAMIC_CONTENT_LENGTH) {
            // Dynamic Content Size
            response = malloc(sizeof (piper_response));
            if (response == NULL) {
                // Out Of Memory
                _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback, user_data);
                goto free_request;
            }
#define DYNAMIC_CONTENT_CHUNK_SIZE 4096
            size_t size = 0;
            size_t final_content_length = 0;
            size_t real_size = 0;
            // Read Data
            while (1) {
                // Read Byte
                unsigned char x;
                int read_ret = _safe_read(sock, &x, sizeof (x));
                if (read_ret == SAFE_IO_EOF_ERROR) {
                    x = (unsigned char) '\0'; // NULL-Terminator
                } else if (read_ret != 0) {
                    _send_empty(CLIENT_CONNECTION_ERROR, callback, user_data);
                    goto free_response;
                } else {
                    final_content_length++;
                }
                size++;
                // Resize If Needed
                while (size > real_size) {
                    real_size += DYNAMIC_CONTENT_CHUNK_SIZE;
                    piper_response *new_response = realloc(response, sizeof (piper_response) + real_size);
                    if (new_response == NULL) {
                        // Out Of Memory
                        _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback, user_data);
                        goto free_request;
                    }
                }
                // Set Data
                response->content[size - 1] = x;
                // Check If Finished
                if (read_ret == SAFE_IO_EOF_ERROR) {
                    break;
                }
            }
            // Set Metadata
            response->content_length = final_content_length;
        } else {
            // Fixed Content Size
            response = malloc(sizeof (piper_response) + response_content_size + 1 /* NULL-Terminator */);
            if (response == NULL) {
                // Out Of Memory
                _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback, user_data);
                goto free_request;
            }
            response->content_length = response_content_size;
            if (_safe_read(sock, response->content, response->content_length) != 0) {
                _send_empty(CLIENT_CONNECTION_ERROR, callback, user_data);
                goto free_response;
            }
            // Add NULL-Terminator
            response->content[response->content_length] = '\0';
        }
        // Content Type
        response->content_type = response_content_type;
    }

    // Detect Redirect If Needed
    if (handle_redirects && response->content_type == REDIRECT) {
        // Redirect
        _handle_redirects(url, response, callback, user_data);
    } else {
        // Callback
        if (callback != NULL) {
            (*callback)(response, user_data);
        }
    }

    // Free
 free_response:
   free(response);
 free_request:
    free(request);
}
