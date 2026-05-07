#include "state_machine.h"

void sm_init(sm_context_t *ctx)
{
    ctx->current = SM_STATE_IDLE;
    ctx->prior_before_door = SM_STATE_IDLE;
}

sm_state_t sm_current_state(const sm_context_t *ctx)
{
    return ctx->current;
}

static sm_state_t next_for_ringing(sm_event_t evt, sm_state_t *prior_before_door)
{
    switch (evt) {
        case SM_EVT_USER_TAP_LISTEN:  return SM_STATE_LISTENING;
        case SM_EVT_USER_TAP_DOOR:
            *prior_before_door = SM_STATE_RINGING;
            return SM_STATE_DOOR_RELEASING;
        case SM_EVT_USER_TAP_DISMISS: return SM_STATE_IDLE;
        case SM_EVT_TIMEOUT_RINGING:  return SM_STATE_IDLE;
        default:                      return SM_STATE_RINGING;
    }
}

static sm_state_t next_for_listening(sm_event_t evt, sm_state_t *prior_before_door)
{
    switch (evt) {
        case SM_EVT_USER_HOLD_TALK: return SM_STATE_TALKING;
        case SM_EVT_USER_TAP_END:   return SM_STATE_IDLE;
        case SM_EVT_USER_TAP_DOOR:
            *prior_before_door = SM_STATE_LISTENING;
            return SM_STATE_DOOR_RELEASING;
        default:                    return SM_STATE_LISTENING;
    }
}

sm_state_t sm_handle_event(sm_context_t *ctx, sm_event_t evt)
{
    if (evt == SM_EVT_ERROR_RAISED) {
        ctx->current = SM_STATE_ERROR;
        return ctx->current;
    }

    if (evt == SM_EVT_USER_OPEN_SETTINGS && ctx->current != SM_STATE_CONFIG) {
        ctx->current = SM_STATE_CONFIG;
        return ctx->current;
    }

    sm_state_t next = ctx->current;

    switch (ctx->current) {
        case SM_STATE_IDLE:
            if (evt == SM_EVT_RING_DETECTED) next = SM_STATE_RINGING;
            break;

        case SM_STATE_RINGING:
            next = next_for_ringing(evt, &ctx->prior_before_door);
            break;

        case SM_STATE_LISTENING:
            next = next_for_listening(evt, &ctx->prior_before_door);
            break;

        case SM_STATE_TALKING:
            if (evt == SM_EVT_USER_RELEASE_TALK) next = SM_STATE_LISTENING;
            break;

        case SM_STATE_DOOR_RELEASING:
            if (evt == SM_EVT_TIMEOUT_DOOR) next = ctx->prior_before_door;
            break;

        case SM_STATE_CONFIG:
            if (evt == SM_EVT_USER_CLOSE_SETTINGS) next = SM_STATE_IDLE;
            break;

        case SM_STATE_ERROR:
            if (evt == SM_EVT_ERROR_CLEARED) next = SM_STATE_IDLE;
            break;

        default:
            break;
    }

    ctx->current = next;
    return ctx->current;
}

const char *sm_state_name(sm_state_t state)
{
    switch (state) {
        case SM_STATE_IDLE:           return "IDLE";
        case SM_STATE_RINGING:        return "RINGING";
        case SM_STATE_LISTENING:      return "LISTENING";
        case SM_STATE_TALKING:        return "TALKING";
        case SM_STATE_DOOR_RELEASING: return "DOOR_RELEASING";
        case SM_STATE_CONFIG:         return "CONFIG";
        case SM_STATE_ERROR:          return "ERROR";
        case SM_STATE_COUNT:          return "INVALID";
    }
    return "UNKNOWN";
}

const char *sm_event_name(sm_event_t evt)
{
    switch (evt) {
        case SM_EVT_RING_DETECTED:       return "RING_DETECTED";
        case SM_EVT_USER_TAP_LISTEN:     return "USER_TAP_LISTEN";
        case SM_EVT_USER_TAP_DOOR:       return "USER_TAP_DOOR";
        case SM_EVT_USER_TAP_DISMISS:    return "USER_TAP_DISMISS";
        case SM_EVT_USER_TAP_END:        return "USER_TAP_END";
        case SM_EVT_USER_HOLD_TALK:      return "USER_HOLD_TALK";
        case SM_EVT_USER_RELEASE_TALK:   return "USER_RELEASE_TALK";
        case SM_EVT_USER_OPEN_SETTINGS:  return "USER_OPEN_SETTINGS";
        case SM_EVT_USER_CLOSE_SETTINGS: return "USER_CLOSE_SETTINGS";
        case SM_EVT_TIMEOUT_RINGING:     return "TIMEOUT_RINGING";
        case SM_EVT_TIMEOUT_DOOR:        return "TIMEOUT_DOOR";
        case SM_EVT_ERROR_RAISED:        return "ERROR_RAISED";
        case SM_EVT_ERROR_CLEARED:       return "ERROR_CLEARED";
        case SM_EVT_COUNT:               return "INVALID";
    }
    return "UNKNOWN";
}
