#include "event_queue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <stdlib.h>

struct event_queue {
    QueueHandle_t handle;
    size_t cap;
};

event_queue_t *eq_create(size_t capacity)
{
    if (capacity == 0) {
        return NULL;
    }
    event_queue_t *q = calloc(1, sizeof *q);
    if (!q) {
        return NULL;
    }
    q->handle = xQueueCreate(capacity, sizeof(ev_t));
    if (!q->handle) {
        free(q);
        return NULL;
    }
    q->cap = capacity;
    return q;
}

void eq_destroy(event_queue_t *q)
{
    if (!q) {
        return;
    }
    if (q->handle) {
        vQueueDelete(q->handle);
    }
    free(q);
}

static TickType_t ms_to_ticks(uint32_t ms)
{
    if (ms == EQ_WAIT_FOREVER) {
        return portMAX_DELAY;
    }
    return pdMS_TO_TICKS(ms);
}

bool eq_post(event_queue_t *q, const ev_t *evt, uint32_t timeout_ms)
{
    return xQueueSend(q->handle, evt, ms_to_ticks(timeout_ms)) == pdPASS;
}

bool eq_receive(event_queue_t *q, ev_t *out, uint32_t timeout_ms)
{
    return xQueueReceive(q->handle, out, ms_to_ticks(timeout_ms)) == pdPASS;
}

size_t eq_size(event_queue_t *q)
{
    return uxQueueMessagesWaiting(q->handle);
}

size_t eq_capacity(event_queue_t *q)
{
    return q->cap;
}
