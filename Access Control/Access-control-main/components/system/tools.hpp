#ifndef __TOOLS_HPP__
#define __TOOLS_HPP__

#include <string>
#include <functional>

#include "esp_timer.h"


#define MILLIS_WAKE() ((uint64_t) (esp_timer_get_time() / 1000ULL))


enum class InsideEvent {
    kConfigReady = 0,
    kWiFiStationConnected,
    kTimeChanged,
    kThreadData,
    kReboot,
};


typedef std::function<void(InsideEvent ev, void* pdata, uint16_t len)> InsideCb;


class Tools {
public:
    Tools() {};
    virtual ~Tools() {};

public:
    static uint8_t dec2bin(char);
    static std::string byteToHex(uint8_t);
    static uint8_t hex2bin(char);
    static uint64_t StringTo64(std::string);
    static void printHex(uint8_t *buffer, uint8_t bufferSize);
    static void printDec(uint8_t *buffer, uint8_t bufferSize);
    static void dump_byte_array(uint8_t *buffer, uint16_t bufferSize);
    static void convertHexStringToByteArray(uint8_t *buffer, uint16_t len, std::string cmd);
    static uint16_t calculateCKS(std::string command);
    static uint16_t calc_battery_percentage(uint16_t adc);
    static uint8_t parseMessage(std::string *array, int array_count, std::string message, char separator);
    static uint8_t bcd2bin(uint8_t bcd);
    static bool compareByteArrays(size_t size, uint8_t *card_code, char *pass);
    static bool isTimeValid(uint8_t *buffer);
    static uint8_t splitString(std::string *array, int array_count, std::string payload);
};

#endif // __TOOLS_HPP__