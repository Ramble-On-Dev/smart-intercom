#ifndef SMART_INTERCOM_RTSP_RESPONSE_H
#define SMART_INTERCOM_RTSP_RESPONSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTSP_RESP_REASON_MAX        64
#define RTSP_RESP_SESSION_MAX       64
#define RTSP_RESP_TRANSPORT_MAX     256
#define RTSP_RESP_CONTENT_TYPE_MAX  64
#define RTSP_RESP_AUTH_MAX          512

typedef struct {
    int status_code;
    char reason[RTSP_RESP_REASON_MAX];
    uint32_t cseq;
    char session[RTSP_RESP_SESSION_MAX];
    uint32_t session_timeout;
    char transport[RTSP_RESP_TRANSPORT_MAX];
    char content_type[RTSP_RESP_CONTENT_TYPE_MAX];
    char www_authenticate[RTSP_RESP_AUTH_MAX];
    size_t content_length;
    const char *body;
    size_t body_len;
} rtsp_response_t;

bool rtsp_response_parse(const char *buf, size_t len, rtsp_response_t *out);

#ifdef __cplusplus
}
#endif

#endif
