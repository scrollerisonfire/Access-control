#include "_led.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "esp_timer.h"

// #include "wificonnect.h"

#define LED_PIN 15
#define LED_COUNT 6
#define LIGHT_TIMEOUT 5000

Led::Led() : _bState(false) {}

static uint32_t light_timer = 0;

void Led::lightDimmer() {
    if (light_timer > esp_timer_get_time() / 1000) 
        return;

    light_timer = (esp_timer_get_time() / 1000) + LIGHT_TIMEOUT;

    // qrsensor.rawCommand(0x001C, 0b01100000);
    // vTaskDelay(pdMS_TO_TICKS(10));
    // qrsensor.rawCommand(0x001D, 0b01100000);
    // vTaskDelay(pdMS_TO_TICKS(10));
    // qrsensor.rawCommand(0x001E, 0b01100000);
    // vTaskDelay(pdMS_TO_TICKS(10));

    // qrsensor.rawCommand(0x001B, 0b00101111);
    // vTaskDelay(pdMS_TO_TICKS(10));
}

void Led::setBreathing(uint16_t lampAddress, uint8_t command) {
    // qrsensor.rawCommand(0x001C, 0b01000000);
    // vTaskDelay(pdMS_TO_TICKS(10));
    // qrsensor.rawCommand(0x001D, 0b01000000);
    // vTaskDelay(pdMS_TO_TICKS(10));
    // qrsensor.rawCommand(0x001E, 0b01000000);
    // vTaskDelay(pdMS_TO_TICKS(10));

    // qrsensor.rawCommand(lampAddress, command);
    // vTaskDelay(pdMS_TO_TICKS(10));
}

void Led::setGreenBreathing() {
    setBreathing(0x001B, 0b10101111);
}

void Led::setPurpleBreathing() {
    setBreathing(0x001B, 0b11011111);
}

void Led::setRedBreathing() {
    setBreathing(0x001B, 0b11001111);
}

void Led::setYellowBreathing() {
    setBreathing(0x001B, 0b11101111);
}

void Led::setBlueBreathing() {
    setBreathing(0x001B, 0b10011111);
}

void Led::setup(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LED_PIN,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,    
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void Led::loop(void) {
    static int duty = 0;
    static bool increasing = true;

    if (increasing) {
        duty += 100;
        if (duty >= 8191) {
            increasing = false;
        }
    } else {
        duty -= 100;
        if (duty <= 0) {
            increasing = true;
        }
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

Led led;
