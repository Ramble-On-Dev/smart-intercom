#ifndef SMART_INTERCOM_RTSP_URL_H
#define SMART_INTERCOM_RTSP_URL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTSP_URL_SCHEME_MAX 8
#define RTSP_URL_USER_MAX   64
#define RTSP_URL_PASS_MAX   64
#define RTSP_URL_HOST_MAX   128
#define RTSP_URL_PATH_MAX   256

typedef struct {
    char scheme[RTSP_URL_SCHEME_MAX];
    char user[RTSP_URL_USER_MAX];
    char password[RTSP_URL_PASS_MAX];
    char host[RTSP_URL_HOST_MAX];
    uint16_t port;
    char path[RTSP_URL_PATH_MAX];
} rtsp_url_t;

bool rtsp_url_parse(const char *url, rtsp_url_t *out);

#ifdef __cplusplus
}
#endif

#endif
