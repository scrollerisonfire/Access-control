idf_component_register(
    SRCS 
    "board.cpp"
    "qr_reader.cpp"
    "nfc_reader.cpp"
    "arduino/arduino_wrapper.cpp"
    "arduino/mfrc522.cpp"
    "arduino/DFRobot_GM60.cpp"
    "_led.cpp"
    INCLUDE_DIRS 
    "."
    ".."
    "../system"
    REQUIRES esp_timer
    REQUIRES driver
)