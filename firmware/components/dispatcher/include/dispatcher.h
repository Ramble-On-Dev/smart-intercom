#ifndef SMART_INTERCOM_DISPATCHER_H
#define SMART_INTERCOM_DISPATCHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "event_queue.h"
#include "state_machine.h"
#include "tone_detect.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*dispatcher_action_fn)(sm_state_t prev,
                                     sm_state_t next,
                                     sm_event_t evt,
                                     void *user);

typedef struct {
    event_queue_t *queue;
    sm_context_t state;
    tone_detector_t tone;
    dispatcher_action_fn on_transition;
    void *user;
} dispatcher_t;

void dispatcher_init(dispatcher_t *d,
                     event_queue_t *queue,
                     dispatcher_action_fn on_transition,
                     void *user);

void dispatcher_init_tone(dispatcher_t *d,
                          float target_hz,
                          float sample_rate_hz,
                          size_t block_size,
                          float threshold,
                          unsigned debounce_frames);

bool dispatcher_post(dispatcher_t *d, sm_event_t type);

void dispatcher_process_audio(dispatcher_t *d, const int16_t *samples);

bool dispatcher_pump(dispatcher_t *d, uint32_t timeout_ms);

sm_state_t dispatcher_state(const dispatcher_t *d);

#ifdef __cplusplus
}
#endif

#endif
