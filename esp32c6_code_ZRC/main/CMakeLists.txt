idf_component_register(SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer wifi_component mqtt_component)