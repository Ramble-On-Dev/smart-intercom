#ifndef SMART_INTERCOM_JPEG_SYNTH_H
#define SMART_INTERCOM_JPEG_SYNTH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t type;
    uint8_t q;
    bool qt_present;
    const uint8_t *qt_luma;
    const uint8_t *qt_chroma;
    const uint8_t *scan;
    size_t scan_len;
} jpeg_synth_input_t;

int jpeg_synth_build(const jpeg_synth_input_t *in, uint8_t *out, size_t out_cap);

void jpeg_default_quant_tables(uint8_t q, uint8_t out_luma[64], uint8_t out_chroma[64]);

#ifdef __cplusplus
}
#endif

#endif
