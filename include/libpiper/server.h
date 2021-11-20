#pragma once

#include <libpiper/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*piper_request_callback_t)(piper_request *request, int sock, void *user_data);
int piper_server_respond(int sock, piper_response *response);
__attribute__((format(printf, 3, 4))) int piper_server_respond_str(int sock, uint8_t content_type, const char *format, ...);
int piper_server_run(int port, int max_connections, piper_request_callback_t callback, void *user_data);

#ifdef __cplusplus
}
#endif
