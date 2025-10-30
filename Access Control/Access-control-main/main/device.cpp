#include "device.hpp"

#include <functional>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "board.hpp"
// #include "config.hpp"
#include "wifi_station.hpp"
#include "wifi_ap.hpp"
#include "log_manager.hpp"
#include "qr_reader.hpp"
#include "access_control.hpp"
#include "nfc_reader.hpp"
#include "card_data.hpp"

#include "nvs_config.hpp"
#include "nvs_runtime.hpp"

#define TAG     "DEV"

using namespace std::placeholders;

Device *Device::_instance = nullptr;


Device::Device()
{
    _bInit = false;

    _threadNfc = nullptr;
    _threadQr = nullptr;

    _config = nullptr;
    _runtime = nullptr;
}



bool Device::setup(void)
{
    Board::getInstance().setup();

    if((_config = NVS_InitConfig()) == nullptr) {
        ESP_LOGE(TAG, "Fatal ERROR: Config failed to initialize.");
        return false;
    }
    else {
        ESP_LOGI(TAG, "Config ready.");
    }

    if((_runtime = NVS_InitRuntime()) == nullptr) {
        ESP_LOGE(TAG, "Fatal ERROR: Runtime data failed to initialize.");
        return false;
    }
    else {
        ESP_LOGI(TAG, "Runtime data ready.");
    }


    // Set timezone
    const char *s = _config->getString(INT(Cfg::Key::kTimeZone));
    setenv("TZ", s, 1);
    tzset();

    // LogManager::getInstance().setup();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    _threadNfc = new ThreadManager();
    if(_threadNfc->setup((TaskFunction_t)(&NfcReaderTask), "nfc_task", 10000)) {
        _threadNfc->setCallback(
            std::bind(&Device::cbThreadNfc,this, _1, _2, _3));
    }

    _threadQr = new ThreadManager();
    if(_threadQr->setup((TaskFunction_t)(&QrReaderTask), "qr_task", 10000)) {
        _threadQr->setCallback(
            std::bind(&Device::cbThreadQr,this, _1, _2, _3));
    }


    if(!WifiStation::getInstance().setup()) {
        ESP_LOGE(TAG, "WifiStation failed to initialize.");
    }
    else {
        ESP_LOGI(TAG, "WifiStation ready.");
        WifiStation::getInstance().setCallback(
            std::bind(&Device::cbWiFiStation,this, _1, _2, _3));
    }

    if(!WifiAP::getInstance().setup()) {
        ESP_LOGE(TAG, "WifiAP failed to initialize.");
    }
    else {
        ESP_LOGI(TAG, "WifiAP ready.");
        WifiAP::getInstance().setCallback(
            std::bind(&Device::cbWiFiAccessPoint,this, _1, _2, _3));
    }

    AccessControl::getInstance().setup();
    _bInit = true;
    return true;
}


bool bAP = false;

void Device::loop(void)
{
    if(!_bInit)
        return;

    Board::getInstance().loop();
    WifiStation::getInstance().loop();
    WifiAP::getInstance().loop();

    if(!bAP && MILLIS_WAKE() > 30000) {
        WifiAP::getInstance().startAp();
    }

    AccessControl::getInstance().loop();
}




void Device::cbWiFiAccessPoint(InsideEvent ev, void* pdata, uint16_t len) {
    ESP_LOGW(TAG, "Received event %d from WiFi AP.", 
            static_cast<int>(ev));

    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}


void Device::cbWiFiStation(InsideEvent ev, void* pdata, uint16_t len) {
    ESP_LOGW(TAG, "Received event %d from WiFi Station.", 
            static_cast<int>(ev));
}


void Device::cbAccessControl(InsideEvent ev, void* pdata, uint16_t len) {
    ESP_LOGW(TAG, "Received event %d from AccessControl.", 
            static_cast<int>(ev));
}


void Device::cbThreadNfc(InsideEvent ev, void* pdata, uint16_t len) {
    ESP_LOGW(TAG, "Received event %d from NFC.", 
            static_cast<int>(ev));
}


void Device::cbThreadQr(InsideEvent ev, void* pdata, uint16_t len) {
    ESP_LOGW(TAG, "Received event %d from QR.", 
            static_cast<int>(ev));
}



