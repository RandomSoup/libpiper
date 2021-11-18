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
static void _send_empty(uint8_t type, piper_response_callback_t callback) {
    static piper_response response;
    response.content_type = type;
    response.content_length = 0;
    callback(&response);
}

// Handle Redirects
static void _handle_redirects(piper_url url, piper_response *response, piper_response_callback_t callback) {
    // Redirect
    piper_url new_url;
    // Resolve
    if (piper_resolve_relative_url(url, response->content, &new_url) == 0) {
        // Respond
        piper_client_send(new_url, 1, callback);
        // Free
        free(new_url.path);
        free(new_url.host);
    }
}

// Send Request
void piper_client_send(piper_url url, int handle_redirects, piper_response_callback_t callback) {
    // Open Socket
    int sock = -1;
    {
        // Resolve Host
        struct addrinfo hints = {}, *addrs;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        char port_str[16] = {};
        sprintf(port_str, "%d", url.port);
        if (getaddrinfo(url.host, port_str, &hints, &addrs) != 0) {
            ERR("%s", "DNS Lookup Failed");
            _send_empty(CLIENT_INTERNAL_ERROR, callback);
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
        _send_empty(CLIENT_INTERNAL_ERROR, callback);
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
            _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback);
            return;
        }
        request->path_length = (uint16_t) path_length;
        memcpy((void *) request->path, (void *) url.path, path_length);
        request->path_length = htole16(request->path_length);
    }

    // Write Request
    if (_safe_write(sock, request, sizeof (piper_request) + request->path_length) != 0) {
        _send_empty(CLIENT_CONNECTION_ERROR, callback);
        goto free_request;
    }

    // Read Response
    piper_response *response = NULL;
    {
        uint8_t response_content_type;
        if (_safe_read(sock, &response_content_type, sizeof (response_content_type)) != 0) {
            _send_empty(CLIENT_CONNECTION_ERROR, callback);
            goto free_request;
        }
        uint64_t response_content_size;
        if (_safe_read(sock, &response_content_size, sizeof (response_content_size)) != 0) {
            _send_empty(CLIENT_CONNECTION_ERROR, callback);
            goto free_request;
        }
        response_content_size = le64toh(response_content_size);
        response = malloc(sizeof (piper_response) + response_content_size + 1);
        if (response == NULL) {
            // Out Of Memory
            _send_empty(CLIENT_OUT_OF_MEMORY_ERROR, callback);
            goto free_request;
        }
        response->content_type = response_content_type;
        response->content_length = response_content_size;
        if (_safe_read(sock, response->content, response->content_length) != 0) {
            _send_empty(CLIENT_CONNECTION_ERROR, callback);
            goto free_response;
        }
        response->content[response->content_length] = '\0';
    }

    // Detect Redirect If Needed
    if (handle_redirects && response->content_type == REDIRECT) {
        // Redirect
        _handle_redirects(url, response, callback);
    } else {
        // Callback
        (*callback)(response);
    }

    // Free
 free_response:
   free(response);
 free_request:
    free(request);
}
