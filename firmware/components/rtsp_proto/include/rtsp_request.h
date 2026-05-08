#ifndef SMART_INTERCOM_RTSP_REQUEST_H
#define SMART_INTERCOM_RTSP_REQUEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTSP_M_OPTIONS,
    RTSP_M_DESCRIBE,
    RTSP_M_SETUP,
    RTSP_M_PLAY,
    RTSP_M_PAUSE,
    RTSP_M_TEARDOWN,
    RTSP_M_GET_PARAMETER,
} rtsp_method_t;

typedef struct {
    rtsp_method_t method;
    const char *uri;
    uint32_t cseq;
    const char *session;
    const char *transport;
    const char *user_agent;
    const char *accept;
    const char *authorization;
} rtsp_request_t;

int rtsp_request_build(const rtsp_request_t *req, char *buf, size_t buflen);

const char *rtsp_method_name(rtsp_method_t m);

#ifdef __cplusplus
}
#endif

#endif
