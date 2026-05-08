#include "rtsp_response.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const char *find_crlf(const char *p, const char *end)
{
    while (p + 1 < end) {
        if (p[0] == '\r' && p[1] == '\n') {
            return p;
        }
        ++p;
    }
    return NULL;
}

static const char *find_crlfcrlf(const char *p, const char *end)
{
    while (p + 3 < end) {
        if (p[0] == '\r' && p[1] == '\n' && p[2] == '\r' && p[3] == '\n') {
            return p;
        }
        ++p;
    }
    return NULL;
}

static void copy_bounded(char *dst, size_t cap, const char *src, size_t len)
{
    if (cap == 0) {
        return;
    }
    size_t n = len < cap - 1 ? len : cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static bool header_matches(const char *name, size_t name_len, const char *expected)
{
    return name_len == strlen(expected) && strncasecmp(name, expected, name_len) == 0;
}

static uint32_t parse_uint32(const char *src, size_t len)
{
    char buf[16] = {0};
    size_t n = len < sizeof buf - 1 ? len : sizeof buf - 1;
    memcpy(buf, src, n);
    return (uint32_t)strtoul(buf, NULL, 10);
}

static size_t parse_size(const char *src, size_t len)
{
    char buf[24] = {0};
    size_t n = len < sizeof buf - 1 ? len : sizeof buf - 1;
    memcpy(buf, src, n);
    return (size_t)strtoul(buf, NULL, 10);
}

static void parse_session_value(rtsp_response_t *out, const char *val, size_t val_len)
{
    const char *semi = memchr(val, ';', val_len);
    size_t id_len = semi ? (size_t)(semi - val) : val_len;
    while (id_len > 0 && (val[id_len - 1] == ' ' || val[id_len - 1] == '\t')) {
        --id_len;
    }
    copy_bounded(out->session, sizeof out->session, val, id_len);

    if (semi) {
        const char *param_end = val + val_len;
        const char *to = semi + 1;
        while (to + 7 < param_end) {
            if (strncasecmp(to, "timeout=", 8) == 0) {
                out->session_timeout = parse_uint32(to + 8, (size_t)(param_end - (to + 8)));
                return;
            }
            ++to;
        }
    }
}

static bool parse_status_line(const char *buf, size_t len, rtsp_response_t *out)
{
    if (len < 12) {
        return false;
    }
    if (memcmp(buf, "RTSP/1.0 ", 9) != 0) {
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (buf[9 + i] < '0' || buf[9 + i] > '9') {
            return false;
        }
    }
    out->status_code = (buf[9] - '0') * 100 + (buf[10] - '0') * 10 + (buf[11] - '0');

    if (len <= 12) {
        return true;
    }
    if (buf[12] != ' ') {
        return false;
    }
    size_t reason_len = len - 13;
    copy_bounded(out->reason, sizeof out->reason, buf + 13, reason_len);
    return true;
}

bool rtsp_response_parse(const char *buf, size_t len, rtsp_response_t *out)
{
    memset(out, 0, sizeof *out);

    const char *end = buf + len;
    const char *hdr_end = find_crlfcrlf(buf, end);
    if (!hdr_end) {
        return false;
    }

    const char *status_eol = find_crlf(buf, hdr_end);
    if (!status_eol) {
        return false;
    }
    if (!parse_status_line(buf, (size_t)(status_eol - buf), out)) {
        return false;
    }

    const char *cur = status_eol + 2;
    while (cur < hdr_end) {
        const char *line_eol = find_crlf(cur, hdr_end + 2);
        if (!line_eol || line_eol > hdr_end) {
            break;
        }
        const char *colon = memchr(cur, ':', (size_t)(line_eol - cur));
        if (colon) {
            size_t name_len = (size_t)(colon - cur);
            const char *val = colon + 1;
            while (val < line_eol && (*val == ' ' || *val == '\t')) {
                ++val;
            }
            const char *val_end = line_eol;
            while (val_end > val && (val_end[-1] == ' ' || val_end[-1] == '\t')) {
                --val_end;
            }
            size_t val_len = (size_t)(val_end - val);

            if (header_matches(cur, name_len, "CSeq")) {
                out->cseq = parse_uint32(val, val_len);
            } else if (header_matches(cur, name_len, "Session")) {
                parse_session_value(out, val, val_len);
            } else if (header_matches(cur, name_len, "Transport")) {
                copy_bounded(out->transport, sizeof out->transport, val, val_len);
            } else if (header_matches(cur, name_len, "Content-Type")) {
                copy_bounded(out->content_type, sizeof out->content_type, val, val_len);
            } else if (header_matches(cur, name_len, "Content-Length")) {
                out->content_length = parse_size(val, val_len);
            } else if (header_matches(cur, name_len, "WWW-Authenticate")) {
                copy_bounded(out->www_authenticate, sizeof out->www_authenticate, val, val_len);
            }
        }
        cur = line_eol + 2;
    }

    const char *body_start = hdr_end + 4;
    size_t available = (size_t)(end - body_start);
    if (out->content_length > 0) {
        if (available < out->content_length) {
            return false;
        }
        out->body = body_start;
        out->body_len = out->content_length;
    }

    return true;
}
