idf_component_register(
    SRCS "dispatcher.c"
    INCLUDE_DIRS "include"
    REQUIRES state_machine event_queue tone_detect
)
