#ifndef SMART_INTERCOM_STATE_MACHINE_H
#define SMART_INTERCOM_STATE_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SM_STATE_IDLE = 0,
    SM_STATE_RINGING,
    SM_STATE_LISTENING,
    SM_STATE_TALKING,
    SM_STATE_DOOR_RELEASING,
    SM_STATE_CONFIG,
    SM_STATE_ERROR,
    SM_STATE_COUNT
} sm_state_t;

typedef enum {
    SM_EVT_RING_DETECTED = 0,
    SM_EVT_USER_TAP_LISTEN,
    SM_EVT_USER_TAP_DOOR,
    SM_EVT_USER_TAP_DISMISS,
    SM_EVT_USER_TAP_END,
    SM_EVT_USER_HOLD_TALK,
    SM_EVT_USER_RELEASE_TALK,
    SM_EVT_USER_OPEN_SETTINGS,
    SM_EVT_USER_CLOSE_SETTINGS,
    SM_EVT_TIMEOUT_RINGING,
    SM_EVT_TIMEOUT_DOOR,
    SM_EVT_ERROR_RAISED,
    SM_EVT_ERROR_CLEARED,
    SM_EVT_COUNT
} sm_event_t;

typedef struct {
    sm_state_t current;
    sm_state_t prior_before_door;
} sm_context_t;

void sm_init(sm_context_t *ctx);
sm_state_t sm_current_state(const sm_context_t *ctx);
sm_state_t sm_handle_event(sm_context_t *ctx, sm_event_t evt);

const char *sm_state_name(sm_state_t state);
const char *sm_event_name(sm_event_t evt);

#ifdef __cplusplus
}
#endif

#endif
