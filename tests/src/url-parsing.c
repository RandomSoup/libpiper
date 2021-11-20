#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <libpiper/common.h>

// Test
static void reset_url(piper_url *url) {
    free(url->host);
    free(url->path);
}
int main() {
    piper_url url;
    // Test Basic IPv4
    assert(piper_parse_url("127.0.0.1", &url) == 0);
    assert(strcmp(url.host, "127.0.0.1") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic IPv4 (With Path)
    assert(piper_parse_url("127.0.0.1/test", &url) == 0);
    assert(strcmp(url.host, "127.0.0.1") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Test Basic IPv4 (With Port)
    assert(piper_parse_url("127.0.0.1:20", &url) == 0);
    assert(strcmp(url.host, "127.0.0.1") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic IPv4 (With Path And Port)
    assert(piper_parse_url("127.0.0.1:20/test", &url) == 0);
    assert(strcmp(url.host, "127.0.0.1") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Test Basic IPv4 (With Protocol Prefix)
    assert(piper_parse_url("piper://127.0.0.1", &url) == 0);
    assert(strcmp(url.host, "127.0.0.1") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic Domain
    assert(piper_parse_url("localhost", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic Domain (With Path)
    assert(piper_parse_url("localhost/test", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Test Basic Domain (With Port)
    assert(piper_parse_url("localhost:20", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic Domain (With Path And Port)
    assert(piper_parse_url("localhost:20/test", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Test Basic IPv6
    assert(piper_parse_url("[::1]", &url) == 0);
    assert(strcmp(url.host, "::1") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic IPv6 (With Path)
    assert(piper_parse_url("[::1]/test", &url) == 0);
    assert(strcmp(url.host, "::1") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Test Basic IPv6 (With Port)
    assert(piper_parse_url("[::1]:20", &url) == 0);
    assert(strcmp(url.host, "::1") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "") == 0);
    reset_url(&url);
    // Test Basic IPv6 (With Path And Port)
    assert(piper_parse_url("[::1]:20/test", &url) == 0);
    assert(strcmp(url.host, "::1") == 0);
    assert(url.port == 20);
    assert(strcmp(url.path, "/test") == 0);
    reset_url(&url);
    // Return
    return EXIT_SUCCESS;
}
