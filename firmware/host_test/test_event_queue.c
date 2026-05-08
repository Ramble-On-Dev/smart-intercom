#include "unity.h"
#include "event_queue.h"

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

static void test_create_with_zero_capacity_returns_null(void)
{
    TEST_ASSERT_NULL(eq_create(0));
}

static void test_create_returns_non_null_for_positive_capacity(void)
{
    event_queue_t *q = eq_create(8);
    TEST_ASSERT_NOT_NULL(q);
    eq_destroy(q);
}

static void test_capacity_returns_requested_value(void)
{
    event_queue_t *q = eq_create(16);
    TEST_ASSERT_EQUAL_UINT(16, eq_capacity(q));
    eq_destroy(q);
}

static void test_size_zero_after_create(void)
{
    event_queue_t *q = eq_create(4);
    TEST_ASSERT_EQUAL_UINT(0, eq_size(q));
    eq_destroy(q);
}

static void test_post_then_receive_yields_same_event(void)
{
    event_queue_t *q = eq_create(4);
    ev_t in = { .type = SM_EVT_RING_DETECTED, .timestamp_ms = 1234 };
    TEST_ASSERT_TRUE(eq_post(q, &in, 0));
    ev_t out = {0};
    TEST_ASSERT_TRUE(eq_receive(q, &out, 0));
    TEST_ASSERT_EQUAL(SM_EVT_RING_DETECTED, out.type);
    TEST_ASSERT_EQUAL_UINT32(1234, out.timestamp_ms);
    eq_destroy(q);
}

static void test_fifo_order_preserved(void)
{
    event_queue_t *q = eq_create(4);
    sm_event_t types[] = {
        SM_EVT_RING_DETECTED,
        SM_EVT_USER_TAP_LISTEN,
        SM_EVT_USER_HOLD_TALK,
    };
    for (int i = 0; i < 3; ++i) {
        ev_t e = { .type = types[i], .timestamp_ms = (uint32_t)i };
        TEST_ASSERT_TRUE(eq_post(q, &e, 0));
    }
    for (int i = 0; i < 3; ++i) {
        ev_t out = {0};
        TEST_ASSERT_TRUE(eq_receive(q, &out, 0));
        TEST_ASSERT_EQUAL(types[i], out.type);
        TEST_ASSERT_EQUAL_UINT32((uint32_t)i, out.timestamp_ms);
    }
    eq_destroy(q);
}

static void test_size_tracks_post_and_receive(void)
{
    event_queue_t *q = eq_create(4);
    ev_t e = { .type = SM_EVT_RING_DETECTED };
    TEST_ASSERT_EQUAL_UINT(0, eq_size(q));
    eq_post(q, &e, 0);
    TEST_ASSERT_EQUAL_UINT(1, eq_size(q));
    eq_post(q, &e, 0);
    TEST_ASSERT_EQUAL_UINT(2, eq_size(q));
    ev_t out;
    eq_receive(q, &out, 0);
    TEST_ASSERT_EQUAL_UINT(1, eq_size(q));
    eq_destroy(q);
}

static void test_post_to_full_queue_returns_false_when_nowait(void)
{
    event_queue_t *q = eq_create(2);
    ev_t e = { .type = SM_EVT_RING_DETECTED };
    TEST_ASSERT_TRUE(eq_post(q, &e, 0));
    TEST_ASSERT_TRUE(eq_post(q, &e, 0));
    TEST_ASSERT_FALSE(eq_post(q, &e, 0));
    eq_destroy(q);
}

static void test_receive_from_empty_returns_false_when_nowait(void)
{
    event_queue_t *q = eq_create(4);
    ev_t out;
    TEST_ASSERT_FALSE(eq_receive(q, &out, 0));
    eq_destroy(q);
}

static void test_receive_with_timeout_returns_false_after_timeout(void)
{
    event_queue_t *q = eq_create(4);
    ev_t out;
    TEST_ASSERT_FALSE(eq_receive(q, &out, 50));
    eq_destroy(q);
}

static void test_post_with_timeout_returns_false_when_full(void)
{
    event_queue_t *q = eq_create(1);
    ev_t e = { .type = SM_EVT_RING_DETECTED };
    TEST_ASSERT_TRUE(eq_post(q, &e, 0));
    TEST_ASSERT_FALSE(eq_post(q, &e, 50));
    eq_destroy(q);
}

static void test_ring_buffer_wraparound_preserves_order(void)
{
    event_queue_t *q = eq_create(2);
    ev_t e1 = { .type = SM_EVT_RING_DETECTED,    .timestamp_ms = 1 };
    ev_t e2 = { .type = SM_EVT_USER_TAP_LISTEN,  .timestamp_ms = 2 };
    ev_t e3 = { .type = SM_EVT_USER_HOLD_TALK,   .timestamp_ms = 3 };

    eq_post(q, &e1, 0);
    eq_post(q, &e2, 0);
    ev_t out;
    eq_receive(q, &out, 0);
    TEST_ASSERT_EQUAL(SM_EVT_RING_DETECTED, out.type);

    eq_post(q, &e3, 0);

    eq_receive(q, &out, 0);
    TEST_ASSERT_EQUAL(SM_EVT_USER_TAP_LISTEN, out.type);
    eq_receive(q, &out, 0);
    TEST_ASSERT_EQUAL(SM_EVT_USER_HOLD_TALK, out.type);
    eq_destroy(q);
}

typedef struct {
    event_queue_t *q;
    sm_event_t expect_type;
    bool received;
    bool type_matched;
} recv_arg_t;

static void *receiver_thread(void *arg)
{
    recv_arg_t *a = arg;
    ev_t out = {0};
    if (eq_receive(a->q, &out, 2000)) {
        a->received = true;
        a->type_matched = (out.type == a->expect_type);
    }
    return NULL;
}

static void test_blocking_receive_unblocks_on_post(void)
{
    event_queue_t *q = eq_create(4);
    recv_arg_t arg = {
        .q = q,
        .expect_type = SM_EVT_USER_TAP_DOOR,
        .received = false,
        .type_matched = false,
    };
    pthread_t tid;
    pthread_create(&tid, NULL, receiver_thread, &arg);
    usleep(50 * 1000);
    ev_t e = { .type = SM_EVT_USER_TAP_DOOR };
    eq_post(q, &e, 0);
    pthread_join(tid, NULL);
    TEST_ASSERT_TRUE(arg.received);
    TEST_ASSERT_TRUE(arg.type_matched);
    eq_destroy(q);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_create_with_zero_capacity_returns_null);
    RUN_TEST(test_create_returns_non_null_for_positive_capacity);
    RUN_TEST(test_capacity_returns_requested_value);
    RUN_TEST(test_size_zero_after_create);
    RUN_TEST(test_post_then_receive_yields_same_event);
    RUN_TEST(test_fifo_order_preserved);
    RUN_TEST(test_size_tracks_post_and_receive);
    RUN_TEST(test_post_to_full_queue_returns_false_when_nowait);
    RUN_TEST(test_receive_from_empty_returns_false_when_nowait);
    RUN_TEST(test_receive_with_timeout_returns_false_after_timeout);
    RUN_TEST(test_post_with_timeout_returns_false_when_full);
    RUN_TEST(test_ring_buffer_wraparound_preserves_order);
    RUN_TEST(test_blocking_receive_unblocks_on_post);
    return UNITY_END();
}
