#ifndef SMART_INTERCOM_EVENT_QUEUE_H
#define SMART_INTERCOM_EVENT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "state_machine.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EQ_WAIT_FOREVER ((uint32_t)-1)

typedef struct {
    sm_event_t type;
    uint32_t timestamp_ms;
} ev_t;

typedef struct event_queue event_queue_t;

event_queue_t *eq_create(size_t capacity);
void eq_destroy(event_queue_t *q);

bool eq_post(event_queue_t *q, const ev_t *evt, uint32_t timeout_ms);
bool eq_receive(event_queue_t *q, ev_t *out, uint32_t timeout_ms);

size_t eq_size(event_queue_t *q);
size_t eq_capacity(event_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif
