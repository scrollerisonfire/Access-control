idf_component_register(
    SRCS 
    "main.cpp"
    "device.cpp"
    "wifi_station.cpp"
    "wifi_ap.cpp"
    "card_data.cpp"
    "access_control.cpp"
    "_wificonnect.cpp"
    "_access_point.cpp"

    INCLUDE_DIRS 
    "." 
    "../components/system"
    "../components/drivers"

    EMBED_TXTFILES
    "../webserver/index.html"
    "../webserver/style.css"
    "../webserver/update.html"

    REQUIRES driver
    REQUIRES esp_wifi
    REQUIRES nvs_flash
    REQUIRES esp_timer
    REQUIRES mqtt
    REQUIRES esp_https_server

)