#ifndef SMART_INTERCOM_RTP_PARSER_H
#define SMART_INTERCOM_RTP_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t version;
    bool padding;
    bool extension;
    uint8_t cc;
    bool marker;
    uint8_t payload_type;
    uint16_t sequence;
    uint32_t timestamp;
    uint32_t ssrc;
    const uint8_t *payload;
    size_t payload_len;
} rtp_packet_t;

bool rtp_packet_parse(const uint8_t *buf, size_t len, rtp_packet_t *out);

#ifdef __cplusplus
}
#endif

#endif
