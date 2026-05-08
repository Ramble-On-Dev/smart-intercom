#include "rtp_parser.h"

#include <string.h>

static uint16_t rd16be(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint32_t rd32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
}

bool rtp_packet_parse(const uint8_t *buf, size_t len, rtp_packet_t *out)
{
    memset(out, 0, sizeof *out);
    if (len < 12) {
        return false;
    }

    out->version       = (uint8_t)((buf[0] >> 6) & 0x03);
    out->padding       = (bool)((buf[0] >> 5) & 0x01);
    out->extension     = (bool)((buf[0] >> 4) & 0x01);
    out->cc            = (uint8_t)(buf[0] & 0x0F);
    out->marker        = (bool)((buf[1] >> 7) & 0x01);
    out->payload_type  = (uint8_t)(buf[1] & 0x7F);
    out->sequence      = rd16be(buf + 2);
    out->timestamp     = rd32be(buf + 4);
    out->ssrc          = rd32be(buf + 8);

    if (out->version != 2) {
        return false;
    }

    size_t header_len = 12u + (size_t)out->cc * 4u;
    if (len < header_len) {
        return false;
    }

    if (out->extension) {
        if (len < header_len + 4) {
            return false;
        }
        uint16_t ext_words = rd16be(buf + header_len + 2);
        size_t ext_total = 4u + (size_t)ext_words * 4u;
        if (len < header_len + ext_total) {
            return false;
        }
        header_len += ext_total;
    }

    size_t payload_len = len - header_len;

    if (out->padding) {
        if (payload_len == 0) {
            return false;
        }
        uint8_t pad = buf[len - 1];
        if (pad == 0 || (size_t)pad > payload_len) {
            return false;
        }
        payload_len -= pad;
    }

    out->payload = buf + header_len;
    out->payload_len = payload_len;
    return true;
}
