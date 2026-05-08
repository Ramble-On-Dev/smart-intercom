#include "rtsp_request.h"

#include <stdarg.h>
#include <stdio.h>

const char *rtsp_method_name(rtsp_method_t m)
{
    switch (m) {
        case RTSP_M_OPTIONS:       return "OPTIONS";
        case RTSP_M_DESCRIBE:      return "DESCRIBE";
        case RTSP_M_SETUP:         return "SETUP";
        case RTSP_M_PLAY:          return "PLAY";
        case RTSP_M_PAUSE:         return "PAUSE";
        case RTSP_M_TEARDOWN:      return "TEARDOWN";
        case RTSP_M_GET_PARAMETER: return "GET_PARAMETER";
    }
    return "UNKNOWN";
}

static int append(char *buf, size_t buflen, size_t *off, const char *fmt, ...)
{
    if (*off >= buflen) {
        return -1;
    }
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf + *off, buflen - *off, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= buflen - *off) {
        return -1;
    }
    *off += (size_t)n;
    return 0;
}

int rtsp_request_build(const rtsp_request_t *req, char *buf, size_t buflen)
{
    size_t off = 0;
    if (append(buf, buflen, &off, "%s %s RTSP/1.0\r\n",
               rtsp_method_name(req->method), req->uri) < 0) {
        return -1;
    }
    if (append(buf, buflen, &off, "CSeq: %u\r\n", req->cseq) < 0) {
        return -1;
    }
    if (req->user_agent &&
        append(buf, buflen, &off, "User-Agent: %s\r\n", req->user_agent) < 0) {
        return -1;
    }
    if (req->accept &&
        append(buf, buflen, &off, "Accept: %s\r\n", req->accept) < 0) {
        return -1;
    }
    if (req->session &&
        append(buf, buflen, &off, "Session: %s\r\n", req->session) < 0) {
        return -1;
    }
    if (req->transport &&
        append(buf, buflen, &off, "Transport: %s\r\n", req->transport) < 0) {
        return -1;
    }
    if (req->method == RTSP_M_PLAY &&
        append(buf, buflen, &off, "Range: npt=0.000-\r\n") < 0) {
        return -1;
    }
    if (req->authorization &&
        append(buf, buflen, &off, "Authorization: %s\r\n", req->authorization) < 0) {
        return -1;
    }
    if (append(buf, buflen, &off, "\r\n") < 0) {
        return -1;
    }
    return (int)off;
}
