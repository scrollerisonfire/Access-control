// general include file for this module
#include "board.hpp"

// ESP-IDF system include files
#include "esp_system.h"
#include "esp_sleep.h"

// C system files
#include "time.h"

// other include files
#include "esp_log.h"




#define TAG "BRD"

#define DEFAULT_SLEEP_TIME 600000//We need to change this value 
#define SOUND_DURATION_ACTIVATE_AP (200)
#define SOUND_DURATION_RESET_COUNTERS (200)
#define SOUND_DURATION_FACTORY_RESET (200)
#define SOUND_DURATION_TIME_UPDATE_START (100)
#define SOUND_DURATION_TIME_UPDATE_READY (50)

#define SOUND_DURATION_UPDATE_KEY_CHANGE (200)
#define SOUND_DURATION_UPDATE_BROKER_START (100) 
#define SOUND_DURATION_UPDATE_BROKER_END (150)
#define SOUND_ERROR_COUNT (5)
#define SOUND_DURATION_ERROR_TICK (50)



#define HIGH  (1)
#define LOW   (0)




Board* Board::_instance = nullptr;


/*
    General - print_wakeup_reason(), isTimeToSleep(), [restart() - not restart, sleep for 3 min.], 

    GPIO - initGPIO()
    Time - [setClock() - there is a setTime() function!], [setTime() -- see also setClock(), also time_clock.setTime]

    Buzzer - errorSound(), setSound(), soundLoop(), 
    Relay - open_relay(), closedoor(), relayLoop(), 
    LEDs - lightDimmer(), setGreenBreathing(), setPurpleBreathing(), setRedBreathing(), setYellowBreathing(), setBlueBreathing()

    

    Setup()
        - check wakeup reason - write into a variable
        - set GPIO >> initGPIO()

    Loop()
        - loopRelay()
        - loopBuzzer()      // old soundLoop()
        - loopLed()

    printWakeupReason()
    sleep(DEFAULT_SLEEP_TIME)
    setClock()

    #define SOUND_DURATION_xxxx (100)
    #define SOUND_DURATION_xxxx (150)
    #define SOUND_DURATION_xxxx (200)

    #define SOUND_DURATION_ACTIVATE_AP (200)
    #define SOUND_DURATION_RESET_COUNTERS (200)
    #define SOUND_DURATION_FACTORY_RESET (200)

    startSound(int duration)        // old setSound()       --> use `_buzzerTimeExpire` variable
    {
        GPIO_buzzer.ON()
        _buzzerTimeExpire = millis() + duration;    
    }

    startErrorSound()

    openDoorForDuration(int duiration)     // open_relay()
    forceCloseDoor()        // close before the interval is elapsed

    // lightDimmer(), setGreenBreathing(), setPurpleBreathing(), setRedBreathing(), setYellowBreathing(), setBlueBreathing()

    loopRelay()
    loopBuzzer()
    {
        if(_buzzerTimeExpire < millis())
            GPIO_buzzer.OFF()
    }


*/

Board::Board() {
    _bInit = false;
    // _rfidDev = nullptr;
    _accessLateCard = 0;
    _accessLateUser = 0;
    _toSendLate = false;
    _buzzerTimeExpire = 0;
    _buzzerActivated = false;
    _relayTimeExpire = 0;
    _relayActivated = false;
    _indicateError = false;

    for (size_t i = 0; i < 16; i++) {
        _cardBlock1[i] = 0;
        _cardBlock2[i] = 0;
        _cardBlock4[i] = 0;
        _cardBlock5[i] = 0;
        _cardBlock6[i] = 0;
    }
}

bool Board::setup(void) {
    _wakeupReason = esp_sleep_get_wakeup_cause();

    ESP_LOGI(TAG, "Board init.");
    _bInit = true;
    // connectWiFi();
    
    pinMode(BUZZ_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZ_PIN, LOW);
    _indicateError = false;

    pinMode(REL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(REL_PIN, LOW);
    _relayActivated = false;

    setClock();  
    return true;
}

void Board::loop(void) {
    if (!_bInit) return;

    loopRelay();  
    loopBuzzer();  
    loopLed();    

}

void Board::setClock() {
    time_t now;
    struct tm timeinfo;
    // if (!getLocalTime(&timeinfo)) {
    //     ESP_LOGE(TAG, "Failed to obtain time");
    // } else {
    //     time(&now); 
    //     struct timeval tv = {now, 0};
    //     settimeofday(&tv, NULL);
    // }
}

void Board::printWakeupReason() {
    switch (_wakeupReason) {
        case ESP_SLEEP_WAKEUP_EXT0: ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO"); break;
        case ESP_SLEEP_WAKEUP_EXT1: ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL"); break;
        case ESP_SLEEP_WAKEUP_TIMER: ESP_LOGI(TAG, "Wakeup caused by timer"); break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD: ESP_LOGI(TAG, "Wakeup caused by touchpad"); break;
        case ESP_SLEEP_WAKEUP_ULP: ESP_LOGI(TAG, "Wakeup caused by ULP program"); break;
        default: ESP_LOGI(TAG, "Wakeup was not caused by deep sleep");
    }
}


void Board::openDoorForDuration(int duration) {
    _relayActivated = true;
    gpio_set_level(REL_PIN, HIGH);  
    _relayTimeExpire = MILLIS_WAKE() + duration;
}


void Board::forceCloseDoor() {
    _relayActivated = false;
    gpio_set_level(REL_PIN, LOW);  
    _relayTimeExpire = 0;
}

void Board::loopRelay() {
    if (_relayTimeExpire != 0 && _relayTimeExpire < MILLIS_WAKE()) {
        if (_relayActivated) {
            forceCloseDoor();
        }
    }
}


void Board::loopLed() {
#warning "Not implemented!"    
}

void Board::startSound(int duration) {
    gpio_set_level(BUZZ_PIN, HIGH);  
    _buzzerTimeExpire = MILLIS_WAKE() + duration; 
    _buzzerActivated = true;
}

void Board::startErrorSound() {
    // startSound(SOUND_DURATION_ERROR); 
    _indicateError = true;
    _indicateErrorCounter = SOUND_ERROR_COUNT - 1;
    _buzzerTimeExpire = MILLIS_WAKE() + SOUND_DURATION_ERROR_TICK;
    _buzzerActivated = true;
    // TODO: Add Red LED here - true
    gpio_set_level(BUZZ_PIN, HIGH);
}

void Board::startActivateAPSound() {
    startSound(SOUND_DURATION_ACTIVATE_AP); 
}

void Board::startTimeUpdateStartSound() {
    startSound(SOUND_DURATION_TIME_UPDATE_START); 
}

void Board::startFactoryResetSound() {
    startSound(SOUND_DURATION_FACTORY_RESET);  
}

void Board::loopBuzzer() {
    if (_buzzerTimeExpire != 0 && _buzzerTimeExpire < MILLIS_WAKE()) {

        if(_indicateError) {
            if(_indicateErrorCounter > 0) {
                if(_buzzerActivated) {
                    _buzzerActivated = false;
                    gpio_set_level(BUZZ_PIN, LOW);  
                    // TODO: Add Red LED here - false
                    _buzzerTimeExpire = MILLIS_WAKE() + SOUND_DURATION_ERROR_TICK;
                }
                else {
                    _buzzerActivated = true;
                    gpio_set_level(BUZZ_PIN, HIGH);
                    // TODO: Add Red LED here - true  
                    _buzzerTimeExpire = MILLIS_WAKE() + SOUND_DURATION_ERROR_TICK;
                    _indicateErrorCounter--;
                }
            }
            else {
                gpio_set_level(BUZZ_PIN, LOW);  
                _buzzerTimeExpire = 0;
                _buzzerActivated = false;
                _indicateErrorCounter = 0;
                _indicateError = false;
            }
        }
        else {
            gpio_set_level(BUZZ_PIN, LOW);  
            _buzzerTimeExpire = 0;
            _buzzerActivated = false;
            _indicateErrorCounter = 0;
        }
    }
}


bool Board::checkAccess(uint64_t (*checkOfflineCardAccess)(void), void (*opendoor)(uint32_t), void (*saveEvent)(uint64_t), uint32_t relay_time) {
#warning "Move to AccessControl"   
return false; 
}

bool Board::readCard() {

#warning "Move to CardData"    

    for (size_t i = 0; i < 16; i++) {
        _cardBlock1[i] = 0;
        _cardBlock2[i] = 0;
        _cardBlock4[i] = 0;
        _cardBlock5[i] = 0;
        _cardBlock6[i] = 0;
    }

    // if (!mfrc522_is_new_card_present(_rfidDev)) {
    //     mfrc522_halt_a(_rfidDev);
    //     mfrc522_stop_crypto1(_rfidDev);
    //     return false;
    // }

    // if (!mfrc522_read_card_serial(_rfidDev)) {
    //     ESP_LOGI(TAG, "No UID");
    //     mfrc522_halt_a(_rfidDev);
    //     mfrc522_stop_crypto1(_rfidDev);
    //     return false;
    // }

    // mfrc522_picc_type_t piccType = mfrc522_get_type(_rfidDev, _rfidDev->uid.sak);
    // ESP_LOGI(TAG, "Card type: %s", mfrc522_picc_get_type_name(piccType));

    // if (piccType != MFRC522_PICC_TYPE_MIFARE_1K && piccType != MFRC522_PICC_TYPE_MIFARE_4K) {
    //     ESP_LOGI(TAG, "Unsupported card type");
    //     mfrc522_halt_a(_rfidDev);
    //     mfrc522_stop_crypto1(_rfidDev);
    //     return false;
    // }

    // for (uint8_t i = 0; i < 4; i++) {
    //     _nuidPICC[i] = _rfidDev->uid.uid_byte[i];
    // }

    // readBlock(_rfidDev, 1, _cardBlock1);
    // readBlock(_rfidDev, 2, _cardBlock2);
    // readBlock(_rfidDev, 4, _cardBlock4);
    // readBlock(_rfidDev, 5, _cardBlock5);
    // readBlock(_rfidDev, 6, _cardBlock6);

    // mfrc522_halt_a(_rfidDev);
    // mfrc522_stop_crypto1(_rfidDev);

    ESP_LOGI(TAG, "Card detected");
    return true;
}

// void connectWiFi() {
//     esp_err_t ret;

//     ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     esp_netif_t *wifi_sta = esp_netif_create_default_wifi_sta();
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

//     wifi_config_t wifi_config = {};
//     strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
//     strcpy((char*)wifi_config.sta.password, WIFI_PASS);
//     wifi_config.sta.bssid_set = false;

//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

//     ESP_LOGI(TAG, "Connecting to Wi-Fi...");
//     ESP_ERROR_CHECK(esp_wifi_start());

//     TickType_t start_time = xTaskGetTickCount();
//     while (esp_wifi_connect() == ESP_ERR_WIFI_NOT_CONNECTED && (xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(WIFI_TIMEOUT_MS)) {
//         ESP_LOGI(TAG, ".");
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }

//     wifi_ap_record_t ap_info;
//     if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
//         ESP_LOGI(TAG, "Wi-Fi Connected");

//         uint8_t mac[6];
//         ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
//         snprintf(MAC_char, sizeof(MAC_char), "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//         ESP_LOGI(TAG, "Device MAC: %s", MAC_char);

//         snprintf(clientName, sizeof(clientName), "ESPSlim_%s", MAC_char);
//         ESP_LOGI(TAG, "Set MQTT client name: %s", clientName);

//         esp_netif_ip_info_t ip_info;
//         ESP_ERROR_CHECK(esp_netif_get_ip_info(wifi_sta, &ip_info));
//         ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&ip_info.ip));
//     } else {
//         ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
//     }
// }

// static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
//     if (event_id == WIFI_EVENT_STA_CONNECTED) {
//         ESP_LOGI(TAG, "Connected to Wi-Fi");
//     } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         ESP_LOGI(TAG, "Disconnected from Wi-Fi");
//     }
// }


void Board::pinMode(int pin, gpio_mode_t pintype) {

	gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;          // disable interrupt
	io_conf.mode = pintype;                         // set as output/input mode
	io_conf.pin_bit_mask = (1 << pin);              // bit mask of the pin
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;   // disable pull-down mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;       // disable pull-up mode
	gpio_config(&io_conf);                          
}


void sleep(uint64_t timeToSleep) {
#warning "Not implemented!"
}

