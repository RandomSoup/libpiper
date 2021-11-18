#include <stdio.h>
#include <stdlib.h>

#include <libpiper/client.h>

// Callback
static void callback(piper_response *response) {
    printf("Content Type: %#02x\n", response->content_type);
    switch (response->content_type) {
        case UTF8_TEXT:
        case UTF8_GEMTEXT:
        case PLAIN_TEXT: {
            printf("Content: %s\n", response->content);
            break;
        }
    }
}

// Main
int main(int argc, char *argv[]) {
    // Check Arguments
    if (argc != 2) {
        fprintf(stderr, "USAGE: simple-client <URL>\n");
        return EXIT_FAILURE;
    }

    // Send Request
    piper_url url;
    if (piper_parse_url(argv[1], &url) == 0) {
        printf("Host: \"%s\", Port: %i, Path: \"%s\"\n", url.host, url.port, url.path);
        piper_client_send(url, 1, callback);
        free(url.host);
        free(url.path);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
