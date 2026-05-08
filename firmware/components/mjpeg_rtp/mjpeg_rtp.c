#include "mjpeg_rtp.h"

#include <stdlib.h>
#include <string.h>

#define MJPEG_HDR_LEN  8
#define MJPEG_QT_HDR_LEN 4

struct mjpeg_rtp_assembler {
    uint8_t *buf;
    size_t cap;
    size_t scan_len;

    bool in_frame;
    uint32_t expected_offset;
    uint8_t type;
    uint8_t q;
    uint16_t width;
    uint16_t height;
    bool qt_present;
    uint8_t qt_luma[64];
    uint8_t qt_chroma[64];
};

mjpeg_rtp_assembler_t *mjpeg_rtp_create(size_t buf_capacity)
{
    if (buf_capacity == 0) {
        return NULL;
    }
    mjpeg_rtp_assembler_t *a = calloc(1, sizeof *a);
    if (!a) {
        return NULL;
    }
    a->buf = malloc(buf_capacity);
    if (!a->buf) {
        free(a);
        return NULL;
    }
    a->cap = buf_capacity;
    return a;
}

void mjpeg_rtp_destroy(mjpeg_rtp_assembler_t *a)
{
    if (!a) {
        return;
    }
    free(a->buf);
    free(a);
}

void mjpeg_rtp_reset(mjpeg_rtp_assembler_t *a)
{
    a->scan_len = 0;
    a->in_frame = false;
    a->expected_offset = 0;
    a->qt_present = false;
}

static uint32_t rd24be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[2];
}

mjpeg_rtp_status_t mjpeg_rtp_feed(mjpeg_rtp_assembler_t *a,
                                  const uint8_t *payload, size_t len,
                                  bool marker)
{
    if (len < MJPEG_HDR_LEN) {
        mjpeg_rtp_reset(a);
        return MJPEG_RTP_DROPPED;
    }

    uint32_t fragment_offset = rd24be(payload + 1);
    uint8_t type = payload[4];
    uint8_t q = payload[5];
    uint8_t w8 = payload[6];
    uint8_t h8 = payload[7];

    if (type >= 64) {
        mjpeg_rtp_reset(a);
        return MJPEG_RTP_DROPPED;
    }

    size_t pos = MJPEG_HDR_LEN;

    if (fragment_offset == 0) {
        a->scan_len = 0;
        a->in_frame = true;
        a->expected_offset = 0;
        a->type = type;
        a->q = q;
        a->width = (uint16_t)((uint16_t)w8 * 8);
        a->height = (uint16_t)((uint16_t)h8 * 8);
        a->qt_present = false;

        if (q >= 128) {
            if (len < pos + MJPEG_QT_HDR_LEN) {
                mjpeg_rtp_reset(a);
                return MJPEG_RTP_DROPPED;
            }
            uint16_t qt_len = (uint16_t)((uint16_t)payload[pos + 2] << 8 | payload[pos + 3]);
            pos += MJPEG_QT_HDR_LEN;
            if (qt_len != 128) {
                mjpeg_rtp_reset(a);
                return MJPEG_RTP_DROPPED;
            }
            if (len < pos + qt_len) {
                mjpeg_rtp_reset(a);
                return MJPEG_RTP_DROPPED;
            }
            memcpy(a->qt_luma,   payload + pos,      64);
            memcpy(a->qt_chroma, payload + pos + 64, 64);
            a->qt_present = true;
            pos += qt_len;
        }
    } else {
        if (!a->in_frame || fragment_offset != a->expected_offset) {
            mjpeg_rtp_reset(a);
            return MJPEG_RTP_DROPPED;
        }
    }

    size_t scan_chunk = len - pos;
    if (a->scan_len + scan_chunk > a->cap) {
        mjpeg_rtp_reset(a);
        return MJPEG_RTP_DROPPED;
    }

    memcpy(a->buf + a->scan_len, payload + pos, scan_chunk);
    a->scan_len += scan_chunk;
    a->expected_offset = fragment_offset + (uint32_t)scan_chunk;

    if (marker) {
        a->in_frame = false;
        return MJPEG_RTP_FRAME_READY;
    }
    return MJPEG_RTP_OK;
}

const uint8_t *mjpeg_rtp_scan_data(const mjpeg_rtp_assembler_t *a) { return a->buf; }
size_t mjpeg_rtp_scan_size(const mjpeg_rtp_assembler_t *a)         { return a->scan_len; }
uint16_t mjpeg_rtp_width(const mjpeg_rtp_assembler_t *a)           { return a->width; }
uint16_t mjpeg_rtp_height(const mjpeg_rtp_assembler_t *a)          { return a->height; }
uint8_t mjpeg_rtp_type(const mjpeg_rtp_assembler_t *a)             { return a->type; }
uint8_t mjpeg_rtp_q(const mjpeg_rtp_assembler_t *a)                { return a->q; }
bool mjpeg_rtp_qt_present(const mjpeg_rtp_assembler_t *a)          { return a->qt_present; }
const uint8_t *mjpeg_rtp_qt_luma(const mjpeg_rtp_assembler_t *a)   { return a->qt_luma; }
const uint8_t *mjpeg_rtp_qt_chroma(const mjpeg_rtp_assembler_t *a) { return a->qt_chroma; }
