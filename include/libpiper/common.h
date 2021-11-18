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

#define UTF8_TEXT 0x00
#define UTF8_GEMTEXT 0x01
#define PLAIN_TEXT 0x02
#define RAW_FILE 0x10
#define REDIRECT 0x20
#define SERVER_NOT_FOUND_ERROR 0x22
#define SERVER_INTERNAL_ERROR 0x23
#define SPECIFICATION_VERSION 0x24
#define CLIENT_OUT_OF_MEMORY_ERROR 0xf0
#define CLIENT_CONNECTION_ERROR 0xf1
#define CLIENT_INTERNAL_ERROR 0xf2

typedef struct {
    uint8_t content_type;
    uint64_t content_length; // Little Endian
    char content[];
} __attribute__((packed)) piper_response;

// Functions

// NOTE: out->host And out->path Must Be Freed With free()
int piper_parse_url(const char *url, piper_url *out);

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
