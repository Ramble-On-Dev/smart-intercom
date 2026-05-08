#include "rtsp_url.h"

#include <stdlib.h>
#include <string.h>

#define RTSP_DEFAULT_PORT 554

static bool copy_bounded(char *dst, size_t cap, const char *src, size_t len)
{
    if (len >= cap) {
        return false;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return true;
}

bool rtsp_url_parse(const char *url, rtsp_url_t *out)
{
    memset(out, 0, sizeof *out);

    const char *scheme_end = strstr(url, "://");
    if (!scheme_end || scheme_end == url) {
        return false;
    }
    if (!copy_bounded(out->scheme, sizeof out->scheme, url, scheme_end - url)) {
        return false;
    }

    const char *p = scheme_end + 3;

    const char *path_start = strchr(p, '/');
    const char *authority_end = path_start ? path_start : p + strlen(p);

    const char *at = NULL;
    for (const char *cur = p; cur < authority_end; ++cur) {
        if (*cur == '@') {
            at = cur;
            break;
        }
    }

    if (at) {
        const char *colon = NULL;
        for (const char *cur = p; cur < at; ++cur) {
            if (*cur == ':') {
                colon = cur;
                break;
            }
        }
        if (colon) {
            if (!copy_bounded(out->user, sizeof out->user, p, colon - p)) {
                return false;
            }
            if (!copy_bounded(out->password, sizeof out->password,
                              colon + 1, at - colon - 1)) {
                return false;
            }
        } else {
            if (!copy_bounded(out->user, sizeof out->user, p, at - p)) {
                return false;
            }
        }
        p = at + 1;
    }

    const char *port_colon = NULL;
    for (const char *cur = p; cur < authority_end; ++cur) {
        if (*cur == ':') {
            port_colon = cur;
            break;
        }
    }
    const char *host_end = port_colon ? port_colon : authority_end;
    size_t host_len = host_end - p;
    if (host_len == 0) {
        return false;
    }
    if (!copy_bounded(out->host, sizeof out->host, p, host_len)) {
        return false;
    }

    if (port_colon) {
        size_t port_len = authority_end - port_colon - 1;
        if (port_len == 0 || port_len > 5) {
            return false;
        }
        char portbuf[8] = {0};
        memcpy(portbuf, port_colon + 1, port_len);
        for (size_t i = 0; i < port_len; ++i) {
            if (portbuf[i] < '0' || portbuf[i] > '9') {
                return false;
            }
        }
        long port = strtol(portbuf, NULL, 10);
        if (port < 1 || port > 65535) {
            return false;
        }
        out->port = (uint16_t)port;
    } else {
        out->port = RTSP_DEFAULT_PORT;
    }

    if (path_start) {
        size_t path_len = strlen(path_start);
        if (!copy_bounded(out->path, sizeof out->path, path_start, path_len)) {
            return false;
        }
    } else {
        out->path[0] = '/';
        out->path[1] = '\0';
    }

    return true;
}
