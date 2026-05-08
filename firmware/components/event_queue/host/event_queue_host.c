#include "event_queue.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct event_queue {
    ev_t *buf;
    size_t cap;
    size_t head;
    size_t tail;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
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
    q->buf = calloc(capacity, sizeof *q->buf);
    if (!q->buf) {
        free(q);
        return NULL;
    }
    q->cap = capacity;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    return q;
}

void eq_destroy(event_queue_t *q)
{
    if (!q) {
        return;
    }
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    free(q->buf);
    free(q);
}

static void abs_time_after_ms(struct timespec *ts, uint32_t ms)
{
    clock_gettime(CLOCK_REALTIME, ts);
    uint64_t total_ns = (uint64_t)ts->tv_nsec + (uint64_t)ms * 1000000ULL;
    ts->tv_sec += (time_t)(total_ns / 1000000000ULL);
    ts->tv_nsec = (long)(total_ns % 1000000000ULL);
}

bool eq_post(event_queue_t *q, const ev_t *evt, uint32_t timeout_ms)
{
    pthread_mutex_lock(&q->lock);
    while (q->count == q->cap) {
        if (timeout_ms == 0) {
            pthread_mutex_unlock(&q->lock);
            return false;
        }
        if (timeout_ms == EQ_WAIT_FOREVER) {
            pthread_cond_wait(&q->not_full, &q->lock);
        } else {
            struct timespec ts;
            abs_time_after_ms(&ts, timeout_ms);
            int rc = pthread_cond_timedwait(&q->not_full, &q->lock, &ts);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&q->lock);
                return false;
            }
        }
    }
    q->buf[q->tail] = *evt;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
    return true;
}

bool eq_receive(event_queue_t *q, ev_t *out, uint32_t timeout_ms)
{
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        if (timeout_ms == 0) {
            pthread_mutex_unlock(&q->lock);
            return false;
        }
        if (timeout_ms == EQ_WAIT_FOREVER) {
            pthread_cond_wait(&q->not_empty, &q->lock);
        } else {
            struct timespec ts;
            abs_time_after_ms(&ts, timeout_ms);
            int rc = pthread_cond_timedwait(&q->not_empty, &q->lock, &ts);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&q->lock);
                return false;
            }
        }
    }
    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return true;
}

size_t eq_size(event_queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    size_t n = q->count;
    pthread_mutex_unlock(&q->lock);
    return n;
}

size_t eq_capacity(event_queue_t *q)
{
    return q->cap;
}
