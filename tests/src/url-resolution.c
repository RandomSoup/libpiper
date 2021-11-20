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
    // Test Out-Of-Site Absolute
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/test";
    assert(piper_resolve_url(url, "piper://ip6-localhost/hello", &url) == 0);
    assert(strcmp(url.host, "ip6-localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/hello") == 0);
    reset_url(&url);
    // Test In-Site Absolute
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/test";
    assert(piper_resolve_url(url, "/hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #1)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/test";
    assert(piper_resolve_url(url, "hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #2)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/test/";
    assert(piper_resolve_url(url, "../hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/test/../hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #3)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/a/b";
    assert(piper_resolve_url(url, "hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/a/hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #4)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/a/b/";
    assert(piper_resolve_url(url, "hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/a/b/hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #5)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "";
    assert(piper_resolve_url(url, "hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/hello") == 0);
    reset_url(&url);
    // Test In-Site Relative (Variation #6)
    url.host = "localhost";
    url.port = PIPER_DEFAULT_PORT;
    url.path = "/";
    assert(piper_resolve_url(url, "hello", &url) == 0);
    assert(strcmp(url.host, "localhost") == 0);
    assert(url.port == PIPER_DEFAULT_PORT);
    assert(strcmp(url.path, "/hello") == 0);
    reset_url(&url);
    // Return
    return EXIT_SUCCESS;
}
