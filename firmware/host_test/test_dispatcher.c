#include "unity.h"
#include "dispatcher.h"

#include <math.h>
#include <string.h>

#define BS       200
#define FS       8000.0f
#define F_TARGET 1000.0f
#define AMP      10000.0f
#define PI_F     3.14159265358979323846f

typedef struct {
    int count;
    sm_state_t last_prev;
    sm_state_t last_next;
    sm_event_t last_evt;
    void *last_user;
} cb_capture_t;

static event_queue_t *g_q;
static dispatcher_t g_d;
static cb_capture_t g_cb;

static void on_transition(sm_state_t prev, sm_state_t next, sm_event_t evt, void *user)
{
    cb_capture_t *c = user;
    c->count++;
    c->last_prev = prev;
    c->last_next = next;
    c->last_evt = evt;
    c->last_user = user;
}

void setUp(void)
{
    g_q = eq_create(16);
    memset(&g_cb, 0, sizeof g_cb);
    dispatcher_init(&g_d, g_q, on_transition, &g_cb);
}

void tearDown(void)
{
    eq_destroy(g_q);
    g_q = NULL;
}

static void fill_sine(int16_t *buf, size_t n, float freq, float fs, float amp)
{
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / fs;
        buf[i] = (int16_t)(amp * sinf(2.0f * PI_F * freq * t));
    }
}

static void test_initial_state_is_idle(void)
{
    TEST_ASSERT_EQUAL(SM_STATE_IDLE, dispatcher_state(&g_d));
}

static void test_pump_empty_queue_returns_false(void)
{
    TEST_ASSERT_FALSE(dispatcher_pump(&g_d, 0));
}

static void test_ring_event_drives_idle_to_ringing(void)
{
    TEST_ASSERT_TRUE(dispatcher_post(&g_d, SM_EVT_RING_DETECTED));
    TEST_ASSERT_TRUE(dispatcher_pump(&g_d, 0));
    TEST_ASSERT_EQUAL(SM_STATE_RINGING, dispatcher_state(&g_d));
}

static void test_callback_invoked_on_transition(void)
{
    dispatcher_post(&g_d, SM_EVT_RING_DETECTED);
    dispatcher_pump(&g_d, 0);
    TEST_ASSERT_EQUAL(1, g_cb.count);
    TEST_ASSERT_EQUAL(SM_STATE_IDLE, g_cb.last_prev);
    TEST_ASSERT_EQUAL(SM_STATE_RINGING, g_cb.last_next);
    TEST_ASSERT_EQUAL(SM_EVT_RING_DETECTED, g_cb.last_evt);
    TEST_ASSERT_EQUAL_PTR(&g_cb, g_cb.last_user);
}

static void test_callback_not_invoked_when_no_state_change(void)
{
    dispatcher_post(&g_d, SM_EVT_USER_TAP_LISTEN);
    dispatcher_pump(&g_d, 0);
    TEST_ASSERT_EQUAL(0, g_cb.count);
}

static void test_full_call_flow(void)
{
    dispatcher_post(&g_d, SM_EVT_RING_DETECTED);
    dispatcher_post(&g_d, SM_EVT_USER_TAP_LISTEN);
    dispatcher_post(&g_d, SM_EVT_USER_HOLD_TALK);
    dispatcher_post(&g_d, SM_EVT_USER_RELEASE_TALK);
    while (dispatcher_pump(&g_d, 0)) {}
    TEST_ASSERT_EQUAL(SM_STATE_LISTENING, dispatcher_state(&g_d));
    TEST_ASSERT_EQUAL(4, g_cb.count);
}

static void test_audio_above_threshold_posts_ring_and_drives_state(void)
{
    int16_t buf[BS];
    fill_sine(buf, BS, F_TARGET, FS, AMP);
    dispatcher_init_tone(&g_d, F_TARGET, FS, BS, 1e8f, 4);
    for (int i = 0; i < 4; ++i) {
        dispatcher_process_audio(&g_d, buf);
    }
    TEST_ASSERT_EQUAL_UINT(1, eq_size(g_q));
    TEST_ASSERT_TRUE(dispatcher_pump(&g_d, 0));
    TEST_ASSERT_EQUAL(SM_STATE_RINGING, dispatcher_state(&g_d));
}

static void test_audio_silence_does_not_post(void)
{
    int16_t buf[BS] = {0};
    dispatcher_init_tone(&g_d, F_TARGET, FS, BS, 1e8f, 4);
    for (int i = 0; i < 20; ++i) {
        dispatcher_process_audio(&g_d, buf);
    }
    TEST_ASSERT_EQUAL_UINT(0, eq_size(g_q));
}

static void test_pump_returns_false_after_draining(void)
{
    dispatcher_post(&g_d, SM_EVT_RING_DETECTED);
    dispatcher_post(&g_d, SM_EVT_USER_TAP_DISMISS);
    TEST_ASSERT_TRUE(dispatcher_pump(&g_d, 0));
    TEST_ASSERT_TRUE(dispatcher_pump(&g_d, 0));
    TEST_ASSERT_FALSE(dispatcher_pump(&g_d, 0));
}

static void test_dispatcher_works_without_callback(void)
{
    dispatcher_t local;
    dispatcher_init(&local, g_q, NULL, NULL);
    dispatcher_post(&local, SM_EVT_RING_DETECTED);
    TEST_ASSERT_TRUE(dispatcher_pump(&local, 0));
    TEST_ASSERT_EQUAL(SM_STATE_RINGING, dispatcher_state(&local));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_is_idle);
    RUN_TEST(test_pump_empty_queue_returns_false);
    RUN_TEST(test_ring_event_drives_idle_to_ringing);
    RUN_TEST(test_callback_invoked_on_transition);
    RUN_TEST(test_callback_not_invoked_when_no_state_change);
    RUN_TEST(test_full_call_flow);
    RUN_TEST(test_audio_above_threshold_posts_ring_and_drives_state);
    RUN_TEST(test_audio_silence_does_not_post);
    RUN_TEST(test_pump_returns_false_after_draining);
    RUN_TEST(test_dispatcher_works_without_callback);
    return UNITY_END();
}
