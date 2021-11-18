#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <libpiper/common.h>

// Socket Utility
int _safe_read(int sock, void *buf, size_t len) {
    // Read
    size_t to_read = len;
    while (to_read > 0) {
        ssize_t x = read(sock, (void *) (((unsigned char *) buf) + (len - to_read)), to_read);
        if (x == -1 && errno != EINTR) {
            return 1;
        }
        to_read -= x;
    }
    return 0;
}
int _safe_write(int sock, void *buf, size_t len) {
    // Write
    size_t to_write = len;
    while (to_write > 0) {
        ssize_t x = write(sock, (void *) (((unsigned char *) buf) + (len - to_write)), to_write);
        if (x == -1 && errno != EINTR) {
            return 1;
        }
        to_write -= x;
    }
    return 0;
}
int _starts_with(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

// Parse URL
int piper_parse_url(const char *url, piper_url *out) {
    // Store Return Value
    int ret = 0;

    // Strip piper://
    char *url_stripped = (char *) url;
    if (_starts_with(PIPER_URL_PREFIX, url_stripped)) {
        url_stripped += strlen(PIPER_URL_PREFIX);
    }

    // IPv6
    int is_ipv6 = 0;
    {
        if (strlen(url_stripped) > 0 && url_stripped[0] == '[') {
            // Is IPv6
            is_ipv6 = 1;
            url_stripped++;
        }
    }

    // Copy
    url_stripped = strdup(url_stripped);
    if (url_stripped == NULL) {
        return 1;
    }

    // Locate Path
    char *path = NULL;
    {
        size_t url_stripped_length = strlen(url_stripped);
        for (size_t i = 0; i < url_stripped_length; i++) {
            if (url_stripped[i] == '/') {
                url_stripped[i] = '\0';
                path = &url_stripped[i + 1];
                break;
            }
        }
    }

    // Locate Port
    char *port = NULL;
    {
        size_t url_stripped_length = strlen(url_stripped);
        if (is_ipv6) {
            for (size_t j = 0; j < url_stripped_length; j++) {
                size_t i = url_stripped_length - j - 1;
                if (is_ipv6 && url_stripped[i] == ']') {
                    url_stripped[i] = '\0';
                    break;
                }
            }
        }
        for (size_t j = 0; j < url_stripped_length; j++) {
            size_t i = url_stripped_length - j - 1;
            if (is_ipv6 && url_stripped[i] == '\0') {
                break;
            } else if (url_stripped[i] == ':') {
                url_stripped[i] = '\0';
                if ((i + 1) < url_stripped_length) {
                    port = &url_stripped[i + 1];
                }
                break;
            }
        }
    }

    // Return
    (*out).host = strdup(url_stripped);
    if ((*out).host == NULL) {
        ret = 1;
        goto free_url_stripped;
    }
    (*out).port = port != NULL ? atoi(port) : PIPER_DEFAULT_PORT;
    if (path != NULL) {
        if (asprintf(&(*out).path, "/%s", path) == -1) {
            (*out).path = NULL;
        }
    } else {
        (*out).path = strdup("");
    }
    if ((*out).path == NULL) {
        free((*out).host);
        ret = 1;
        goto free_url_stripped;
    }

    // Free
 free_url_stripped:
    free(url_stripped);

    // Return
    return ret;
}
