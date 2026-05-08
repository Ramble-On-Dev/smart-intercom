#include "tone_detect.h"

#include <math.h>
#include <string.h>

#define TONE_PI_F 3.14159265358979323846f

float goertzel_magnitude_squared(const int16_t *samples, size_t n,
                                 float target_hz, float sample_rate_hz)
{
    float k = 0.5f + ((float)n * target_hz) / sample_rate_hz;
    float omega = (2.0f * TONE_PI_F * k) / (float)n;
    float coeff = 2.0f * cosf(omega);

    float q1 = 0.0f;
    float q2 = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        float q0 = coeff * q1 - q2 + (float)samples[i];
        q2 = q1;
        q1 = q0;
    }
    return q1 * q1 + q2 * q2 - q1 * q2 * coeff;
}

void td_init(tone_detector_t *d,
             float target_hz,
             float sample_rate_hz,
             size_t block_size,
             float threshold,
             unsigned debounce_frames)
{
    memset(d, 0, sizeof *d);
    d->target_hz = target_hz;
    d->sample_rate_hz = sample_rate_hz;
    d->block_size = block_size;
    d->threshold = threshold;
    d->debounce_frames = debounce_frames;
}

void td_reset(tone_detector_t *d)
{
    d->consecutive = 0;
}

bool td_process(tone_detector_t *d, const int16_t *samples)
{
    float mag2 = goertzel_magnitude_squared(samples, d->block_size,
                                            d->target_hz, d->sample_rate_hz);
    if (mag2 > d->threshold) {
        d->consecutive++;
        if (d->consecutive >= d->debounce_frames) {
            d->consecutive = 0;
            return true;
        }
    } else {
        d->consecutive = 0;
    }
    return false;
}
