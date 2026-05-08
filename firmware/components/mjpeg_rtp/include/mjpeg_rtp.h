#ifndef SMART_INTERCOM_MJPEG_RTP_H
#define SMART_INTERCOM_MJPEG_RTP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MJPEG_RTP_OK,
    MJPEG_RTP_FRAME_READY,
    MJPEG_RTP_DROPPED,
} mjpeg_rtp_status_t;

typedef struct mjpeg_rtp_assembler mjpeg_rtp_assembler_t;

mjpeg_rtp_assembler_t *mjpeg_rtp_create(size_t buf_capacity);
void mjpeg_rtp_destroy(mjpeg_rtp_assembler_t *a);
void mjpeg_rtp_reset(mjpeg_rtp_assembler_t *a);

mjpeg_rtp_status_t mjpeg_rtp_feed(mjpeg_rtp_assembler_t *a,
                                  const uint8_t *payload, size_t len,
                                  bool marker);

const uint8_t *mjpeg_rtp_scan_data(const mjpeg_rtp_assembler_t *a);
size_t mjpeg_rtp_scan_size(const mjpeg_rtp_assembler_t *a);
uint16_t mjpeg_rtp_width(const mjpeg_rtp_assembler_t *a);
uint16_t mjpeg_rtp_height(const mjpeg_rtp_assembler_t *a);
uint8_t mjpeg_rtp_type(const mjpeg_rtp_assembler_t *a);
uint8_t mjpeg_rtp_q(const mjpeg_rtp_assembler_t *a);
bool mjpeg_rtp_qt_present(const mjpeg_rtp_assembler_t *a);
const uint8_t *mjpeg_rtp_qt_luma(const mjpeg_rtp_assembler_t *a);
const uint8_t *mjpeg_rtp_qt_chroma(const mjpeg_rtp_assembler_t *a);

#ifdef __cplusplus
}
#endif

#endif
