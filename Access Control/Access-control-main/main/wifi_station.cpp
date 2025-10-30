#include "wifi_station.hpp"

#include "nvs_config.hpp"

#include "esp_log.h"

#define TAG "WST"

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#define WIFI_CONNECTION_TIMEOUT_MS          (5*60*1000ULL)
#define WIFI_CONNECTION_CLEANUP_TIME_MS     (10*1000ULL)
#define WIFI_CONNECTION_NTP_TIMEOUT_MS      (3*60*1000ULL)


WifiStation* WifiStation::_instance = nullptr;



WifiStation::WifiStation() {
    _bInit = false;
    _tsDisconnected = 0;
    _tsConnectionAttempt = 0;
    _state = State::kInit;
    _stateOld = _state;
    _bTimeReceived = false;
    _fnEventCallback = nullptr;
}


bool WifiStation::setup(void) {

    _bInit = true;
    return true;
}


void WifiStation::loop(void) {
    if(!_bInit)
        return;

    if(_stateOld != _state) {
        ESP_LOGI(TAG, "State changed to: %d", static_cast<int>(_state));
        _stateOld = _state;
    }

    switch(_state) {
        case State::kInit: {
            esp_netif_create_default_wifi_sta();
            wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
            esp_wifi_init(&wifi_initiation); 
            esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
            esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
            _state = State::kConfig;
        } break;

        case State::kConfig: {
            wifi_config_t wifi_configuration = { };
            const char *s;
            s = GetCfg()->getString(INT(Cfg::Key::kStationSSID));
            memcpy(wifi_configuration.sta.ssid,  s, strlen(s) + 1);
            s = GetCfg()->getString(INT(Cfg::Key::kStationPass));
            memcpy(wifi_configuration.sta.password, s, strlen(s) + 1);
            ESP_LOGI(TAG, "WiFi Station attempt connection to SSID <%s> with Pass <%s>",
                    wifi_configuration.sta.ssid, wifi_configuration.sta.password);
            esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);
            esp_wifi_set_mode(WIFI_MODE_APSTA);
            esp_wifi_start();
            esp_wifi_connect();
            _tsConnectionAttempt = MILLIS_WAKE();
            _state = State::kWaiting;
        } break;

        case State::kWaiting: {
            if(_tsConnectionAttempt != 0 &&
                _tsConnectionAttempt + WIFI_CONNECTION_TIMEOUT_MS 
                        < MILLIS_WAKE()) {
                esp_wifi_disconnect();
                esp_wifi_stop();
                _state = State::kDisconnected;
            }
        } break;

        case State::kNtpRequest: {
            _tsConnectionAttempt = 0;
            esp_sntp_stop();
            esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
            const char *s = GetCfg()->getString(INT(Cfg::Key::kNtpServer));
            ESP_LOGI(TAG, "Request to NTP server at: %s", s);
            esp_sntp_setservername(0, s);
            esp_sntp_init();
            _tsNtpRequest = MILLIS_WAKE();
            _state = State::kNtpWaiting;
        } break;

        case State::kNtpWaiting: {
            sntp_sync_status_t err = sntp_get_sync_status();

            if(err == SNTP_SYNC_STATUS_COMPLETED) {
                time_t timestamp = 0;
                struct tm timeinfo;
                time(&timestamp);
                localtime_r(&timestamp, &timeinfo);
                char strftime_buf[64];
                strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
                ESP_LOGI(TAG, "NTP Ready. Current time: %s", strftime_buf);
                execCallback(InsideEvent::kTimeChanged, nullptr, 0);
                _tsNtpRequest = 0;
                _bTimeReceived = true;
                _state = State::kReady;
            }

            if(_tsNtpRequest != 0 && 
                _tsNtpRequest + WIFI_CONNECTION_NTP_TIMEOUT_MS 
                        < MILLIS_WAKE()) {
                ESP_LOGW(TAG, "NTP server timeout. Skip.");
                _tsNtpRequest = 0;
                _state = State::kReady;
            }
        } break;

        case State::kReady: {
            // do nothing
        } break;

        case State::kDisconnected: {
            if(_tsDisconnected != 0 &&
                _tsDisconnected + WIFI_CONNECTION_CLEANUP_TIME_MS 
                        < MILLIS_WAKE()) {
                _state = State::kConfig;
            }
        } break;
    }
}


void WifiStation::onWifiEvent(int32_t event_id) {
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WiFi connecting WIFI_EVENT_STA_START ...");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WiFi connected WIFI_EVENT_STA_CONNECTED ...");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "WiFi lost connection WIFI_EVENT_STA_DISCONNECTED");
        _tsDisconnected = MILLIS_WAKE();
        _state = State::kDisconnected;
        break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "WiFi got IP");
        execCallback(InsideEvent::kWiFiStationConnected, nullptr, 0);
        _state = State::kNtpRequest;
        break;
    default:
        break;
    }
}


void WifiStation::execCallback(InsideEvent event, void* pdata, uint16_t len) {
    if(_fnEventCallback != nullptr) {
        _fnEventCallback(event, pdata, len);
    }
}


void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, 
    int32_t event_id, void *event_data)
{
    WifiStation::getInstance().onWifiEvent(event_id);
}


