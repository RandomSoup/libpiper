#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structures

#define PIPER_URL_PREFIX "piper://"
#define PIPER_DEFAULT_PORT 60

typedef struct {
    char *host;
    int port;
    char *path;
} piper_url;

// Network Structures

typedef struct {
    uint16_t path_length; // Little Endian
    char path[];
} __attribute__((packed)) piper_request;

// Content Types Data
#define DEFINE_ALL_PIPER_CONTENT_TYPES() \
    DEFINE_PIPER_CONTENT_TYPE(UTF8_TEXT, 0x00) \
    DEFINE_PIPER_CONTENT_TYPE(UTF8_GEMTEXT, 0x01) \
    DEFINE_PIPER_CONTENT_TYPE(PLAIN_TEXT, 0x02) \
    DEFINE_PIPER_CONTENT_TYPE(RAW_FILE, 0x10) \
    DEFINE_PIPER_CONTENT_TYPE(REDIRECT, 0x20) \
    DEFINE_PIPER_CONTENT_TYPE(SERVER_NOT_FOUND_ERROR, 0x22) \
    DEFINE_PIPER_CONTENT_TYPE(SERVER_INTERNAL_ERROR, 0x23) \
    DEFINE_PIPER_CONTENT_TYPE(SPECIFICATION_VERSION, 0x24) \
    DEFINE_PIPER_CONTENT_TYPE(CLIENT_OUT_OF_MEMORY_ERROR, 0xf0) \
    DEFINE_PIPER_CONTENT_TYPE(CLIENT_CONNECTION_ERROR, 0xf1) \
    DEFINE_PIPER_CONTENT_TYPE(CLIENT_INTERNAL_ERROR, 0xf2)
// Content Types Enum
#define DEFINE_PIPER_CONTENT_TYPE(name, value) name = value,
enum {
    DEFINE_ALL_PIPER_CONTENT_TYPES()
};
#undef DEFINE_PIPER_CONTENT_TYPE
// Hide Internal Macros
#ifndef PIPER_INTERNAL
#undef DEFINE_ALL_PIPER_CONTENT_TYPES
#endif

typedef struct {
    uint8_t content_type;
    uint64_t content_length; // Little Endian
    char content[];
} __attribute__((packed)) piper_response;

// Functions

const char *piper_content_type_to_string(uint8_t content_type);

// NOTE: out->host And out->path Must Be Freed With free()
int piper_parse_url(const char *url, piper_url *out);
int piper_resolve_url(piper_url old_url, const char *path, piper_url *new_url);

// Internal
#ifdef PIPER_INTERNAL

#include <stddef.h>

int _safe_read(int sock, void *buf, size_t len);
int _safe_write(int sock, void *buf, size_t len);

int _starts_with(const char *pre, const char *str);

#ifndef NDEBUG
#define ERR(format, ...) fprintf(stderr, "Piper Error: " format "\n", __VA_ARGS__);
#else
#define ERR(...)
#endif

#endif

#ifdef __cplusplus
}
#endif
