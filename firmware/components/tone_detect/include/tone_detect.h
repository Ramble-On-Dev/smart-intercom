#ifndef SMART_INTERCOM_TONE_DETECT_H
#define SMART_INTERCOM_TONE_DETECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

float goertzel_magnitude_squared(const int16_t *samples, size_t n,
                                 float target_hz, float sample_rate_hz);

typedef struct {
    float target_hz;
    float sample_rate_hz;
    size_t block_size;
    float threshold;
    unsigned debounce_frames;
    unsigned consecutive;
} tone_detector_t;

void td_init(tone_detector_t *d,
             float target_hz,
             float sample_rate_hz,
             size_t block_size,
             float threshold,
             unsigned debounce_frames);

void td_reset(tone_detector_t *d);

bool td_process(tone_detector_t *d, const int16_t *samples);

#ifdef __cplusplus
}
#endif

#endif
