idf_component_register(
    SRCS 
    "nvs.cpp"
    "nvs_config.cpp"
    "nvs_runtime.cpp"
    "log_manager.cpp"
    "tools.cpp"
    "thread.cpp"
    INCLUDE_DIRS 
    "."
    ".."
    REQUIRES driver
    REQUIRES esp_timer
    REQUIRES nvs_flash
    REQUIRES freertos
)