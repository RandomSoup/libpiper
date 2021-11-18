#pragma once

#include <libpiper/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*piper_response_callback_t)(piper_response *response);
void piper_client_send(piper_url url, int handle_redirects, piper_response_callback_t callback);

#ifdef __cplusplus
}
#endif
