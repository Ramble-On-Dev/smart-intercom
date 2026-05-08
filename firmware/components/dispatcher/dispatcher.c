#include "dispatcher.h"

#include <string.h>

void dispatcher_init(dispatcher_t *d,
                     event_queue_t *queue,
                     dispatcher_action_fn on_transition,
                     void *user)
{
    memset(d, 0, sizeof *d);
    d->queue = queue;
    d->on_transition = on_transition;
    d->user = user;
    sm_init(&d->state);
}

void dispatcher_init_tone(dispatcher_t *d,
                          float target_hz,
                          float sample_rate_hz,
                          size_t block_size,
                          float threshold,
                          unsigned debounce_frames)
{
    td_init(&d->tone, target_hz, sample_rate_hz, block_size, threshold, debounce_frames);
}

bool dispatcher_post(dispatcher_t *d, sm_event_t type)
{
    ev_t e = { .type = type, .timestamp_ms = 0 };
    return eq_post(d->queue, &e, 0);
}

void dispatcher_process_audio(dispatcher_t *d, const int16_t *samples)
{
    if (td_process(&d->tone, samples)) {
        ev_t e = { .type = SM_EVT_RING_DETECTED, .timestamp_ms = 0 };
        eq_post(d->queue, &e, 0);
    }
}

bool dispatcher_pump(dispatcher_t *d, uint32_t timeout_ms)
{
    ev_t e;
    if (!eq_receive(d->queue, &e, timeout_ms)) {
        return false;
    }
    sm_state_t prev = sm_current_state(&d->state);
    sm_state_t next = sm_handle_event(&d->state, e.type);
    if (prev != next && d->on_transition) {
        d->on_transition(prev, next, e.type, d->user);
    }
    return true;
}

sm_state_t dispatcher_state(const dispatcher_t *d)
{
    return d->state.current;
}
