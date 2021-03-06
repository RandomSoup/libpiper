#include <stdio.h>
#include <stdlib.h>

#include <libpiper/client.h>

// Callback
static void callback(piper_response *response, __attribute__((unused)) void *user_data) {
    printf("Content Type: %s (%#02x)\n", piper_content_type_to_string(response->content_type), response->content_type);
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
        piper_client_send(url, 1, callback, NULL);
        free(url.host);
        free(url.path);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
