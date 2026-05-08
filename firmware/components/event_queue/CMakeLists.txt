idf_component_register(
    SRCS "freertos/event_queue_freertos.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos state_machine
)
