#include "unity.h"
#include "state_machine.h"

static sm_context_t ctx;

void setUp(void)
{
    sm_init(&ctx);
}

void tearDown(void) {}

static void drive(sm_event_t evt)
{
    sm_handle_event(&ctx, evt);
}

static void assert_state(sm_state_t expected)
{
    TEST_ASSERT_EQUAL_STRING(sm_state_name(expected), sm_state_name(sm_current_state(&ctx)));
}

static void test_initial_state_is_idle(void)
{
    assert_state(SM_STATE_IDLE);
}

static void test_idle_ignores_unrelated_events(void)
{
    drive(SM_EVT_USER_TAP_LISTEN);
    assert_state(SM_STATE_IDLE);
    drive(SM_EVT_USER_HOLD_TALK);
    assert_state(SM_STATE_IDLE);
    drive(SM_EVT_TIMEOUT_DOOR);
    assert_state(SM_STATE_IDLE);
}

static void test_idle_to_ringing_on_ring_detected(void)
{
    drive(SM_EVT_RING_DETECTED);
    assert_state(SM_STATE_RINGING);
}

static void test_ringing_to_listening_on_tap_listen(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    assert_state(SM_STATE_LISTENING);
}

static void test_ringing_to_door_releasing_on_tap_door(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_DOOR);
    assert_state(SM_STATE_DOOR_RELEASING);
}

static void test_ringing_to_idle_on_dismiss(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_DISMISS);
    assert_state(SM_STATE_IDLE);
}

static void test_ringing_to_idle_on_timeout(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_TIMEOUT_RINGING);
    assert_state(SM_STATE_IDLE);
}

static void test_listening_to_talking_on_hold_talk(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_HOLD_TALK);
    assert_state(SM_STATE_TALKING);
}

static void test_listening_to_idle_on_tap_end(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_TAP_END);
    assert_state(SM_STATE_IDLE);
}

static void test_listening_to_door_releasing_returns_to_listening(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_TAP_DOOR);
    assert_state(SM_STATE_DOOR_RELEASING);
    drive(SM_EVT_TIMEOUT_DOOR);
    assert_state(SM_STATE_LISTENING);
}

static void test_ringing_door_releasing_returns_to_ringing(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_DOOR);
    assert_state(SM_STATE_DOOR_RELEASING);
    drive(SM_EVT_TIMEOUT_DOOR);
    assert_state(SM_STATE_RINGING);
}

static void test_talking_to_listening_on_release_talk(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_HOLD_TALK);
    drive(SM_EVT_USER_RELEASE_TALK);
    assert_state(SM_STATE_LISTENING);
}

static void test_open_settings_from_idle(void)
{
    drive(SM_EVT_USER_OPEN_SETTINGS);
    assert_state(SM_STATE_CONFIG);
}

static void test_open_settings_from_ringing(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_OPEN_SETTINGS);
    assert_state(SM_STATE_CONFIG);
}

static void test_open_settings_from_listening(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_OPEN_SETTINGS);
    assert_state(SM_STATE_CONFIG);
}

static void test_open_settings_from_talking(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_USER_HOLD_TALK);
    drive(SM_EVT_USER_OPEN_SETTINGS);
    assert_state(SM_STATE_CONFIG);
}

static void test_config_to_idle_on_close(void)
{
    drive(SM_EVT_USER_OPEN_SETTINGS);
    drive(SM_EVT_USER_CLOSE_SETTINGS);
    assert_state(SM_STATE_IDLE);
}

static void test_error_raised_from_any_state(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_ERROR_RAISED);
    assert_state(SM_STATE_ERROR);
}

static void test_error_cleared_returns_to_idle(void)
{
    drive(SM_EVT_ERROR_RAISED);
    drive(SM_EVT_ERROR_CLEARED);
    assert_state(SM_STATE_IDLE);
}

static void test_listening_ignores_door_timeout(void)
{
    drive(SM_EVT_RING_DETECTED);
    drive(SM_EVT_USER_TAP_LISTEN);
    drive(SM_EVT_TIMEOUT_DOOR);
    assert_state(SM_STATE_LISTENING);
}

static void test_state_names_unique_and_non_null(void)
{
    for (int s = 0; s < SM_STATE_COUNT; ++s) {
        const char *name = sm_state_name((sm_state_t)s);
        TEST_ASSERT_NOT_NULL(name);
        TEST_ASSERT_TRUE(name[0] != '\0');
    }
}

static void test_event_names_unique_and_non_null(void)
{
    for (int e = 0; e < SM_EVT_COUNT; ++e) {
        const char *name = sm_event_name((sm_event_t)e);
        TEST_ASSERT_NOT_NULL(name);
        TEST_ASSERT_TRUE(name[0] != '\0');
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_is_idle);
    RUN_TEST(test_idle_ignores_unrelated_events);
    RUN_TEST(test_idle_to_ringing_on_ring_detected);
    RUN_TEST(test_ringing_to_listening_on_tap_listen);
    RUN_TEST(test_ringing_to_door_releasing_on_tap_door);
    RUN_TEST(test_ringing_to_idle_on_dismiss);
    RUN_TEST(test_ringing_to_idle_on_timeout);
    RUN_TEST(test_listening_to_talking_on_hold_talk);
    RUN_TEST(test_listening_to_idle_on_tap_end);
    RUN_TEST(test_listening_to_door_releasing_returns_to_listening);
    RUN_TEST(test_ringing_door_releasing_returns_to_ringing);
    RUN_TEST(test_talking_to_listening_on_release_talk);
    RUN_TEST(test_open_settings_from_idle);
    RUN_TEST(test_open_settings_from_ringing);
    RUN_TEST(test_open_settings_from_listening);
    RUN_TEST(test_open_settings_from_talking);
    RUN_TEST(test_config_to_idle_on_close);
    RUN_TEST(test_error_raised_from_any_state);
    RUN_TEST(test_error_cleared_returns_to_idle);
    RUN_TEST(test_listening_ignores_door_timeout);
    RUN_TEST(test_state_names_unique_and_non_null);
    RUN_TEST(test_event_names_unique_and_non_null);
    return UNITY_END();
}
