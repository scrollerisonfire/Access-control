#include "nvs_config.hpp"
#include "structures.hpp"

#include "esp_log.h"

#define TAG "CFG"

namespace Cfg {


#define CONFIG_PARTITION_NAME   ("nvs")
#define CONFIG_NAMESPACE        ("config")

#define     MAX_STRING_LENGTH                               (64)

#define     FACTORY_VALUE_HOTEL_CODE                        ((int)1)
#define     FACTORY_VALUE_ROOM_NUMBER                       ((int)1)
#define     FACTORY_VALUE_ACCESS_GROUP                      ((uint64_t)1)
#define     FACTORY_VALUE_DEVICE_NAME                       ((char*)"name")
#define     FACTORY_VALUE_DEVICE_TYPE                       ((char*)"")
#define     FACTORY_VALUE_EVENT                             ((char*)"access")
//#define     FACTORY_VALUE_KEY_AES                           ((char*)"xX17y#y%qhvMjt&9")
//#define     FACTORY_VALUE_KEY_UPDATE                        ((char*)"t3o6z7fryx3lrt1c")
// #define     FACTORY_VALUE_STATION_SSID                      ("")
// #define     FACTORY_VALUE_STATION_PASS                      ("4ETj5dPy")
// TODO: Remove after DEBUG
#define     FACTORY_VALUE_STATION_SSID                      ((char*)"")
#define     FACTORY_VALUE_STATION_PASS                      ((char*)"")

#define     FACTORY_VALUE_STATION_DHCP                      ((int)1)
#define     FACTORY_VALUE_STATION_IP                        ((uint32_t)(192<<0) | (168<<8) | (1<<16) | (19<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_MASK                      ((uint32_t)(255<<0) | (255<<8) | (255<<16) | (0<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_PRI_DNS                   ((uint32_t)(8<<0) | (8<<8) | (8<<16) | (8<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_SEC_DNS                   ((uint32_t)(8<<0) | (8<<8) | (4<<16) | (4<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_GATEWAY                   ((uint32_t)(10<<0) | (10<<8) | (1<<16) | (15<<24))  /* network byte order */
#define     FACTORY_VALUE_AP_SSID                           ((char*)"")
#define     FACTORY_VALUE_AP_PASS                           ((char*)"")
#define     FACTORY_VALUE_MQTT                              ((char*)"10.10.1.15")  /* left as string in case domain name needs to be used */
#define     FACTORY_VALUE_MQTT_PORT                         ((int)1885)
#define     FACTORY_VALUE_MQTT_USER                         ((char*)"")
#define     FACTORY_VALUE_MQTT_PASS                         ((char*)"")
#define     FACTORY_VALUE_RELAY_TIME                        ((int)3)
#define     FACTORY_VALUE_SOUND                             ((int)1)
#define     FACTORY_VALUE_NTP_SERVER                        ((char*)"")
#define     FACTORY_VALUE_TIME_ZONE                         ((char*)"EET-2EEST,M3.5.0/3,M10.5.0/4")


std::vector<NvsEntry> CFG_dataVector = {
    NvsEntry{.key=INT(Cfg::Key::kHotelCode), .nvsKey="HTLCODE", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_HOTEL_CODE), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kRoomNumber), .nvsKey="ROOM", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_ROOM_NUMBER), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kAccessGroup), .nvsKey="GROUP", .typ=NvsType::kIntU64, .data=((uint64_t)FACTORY_VALUE_ACCESS_GROUP), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kDeviceName), .nvsKey="NAME", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_DEVICE_NAME)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kDeviceType), .nvsKey="TYPE", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_DEVICE_TYPE)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kEvent), .nvsKey="EVENT", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_EVENT)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kKeyAES), .nvsKey="AESKEY", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_KEY_AES)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kKeyUpdate), .nvsKey="UP_CODE", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_KEY_UPDATE)), .loaded=false, .modified=true},

    NvsEntry{.key=INT(Cfg::Key::kStationSSID), .nvsKey="STASSID", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_STATION_SSID)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationPass), .nvsKey="STAPASS", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_STATION_PASS)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationDHCP), .nvsKey="STADHCP", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_STATION_DHCP), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationIPv4addr), .nvsKey="IP_ADDR", .typ=NvsType::kIntU32, .data=((uint32_t)FACTORY_VALUE_STATION_IP), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationIPv4mask), .nvsKey="IP_MASK", .typ=NvsType::kIntU32, .data=((uint32_t)FACTORY_VALUE_STATION_MASK), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationPrimaryDns), .nvsKey="PRI_DNS", .typ=NvsType::kIntU32, .data=((uint32_t)FACTORY_VALUE_STATION_PRI_DNS), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationSecondaryDns), .nvsKey="SEC_DNS", .typ=NvsType::kIntU32, .data=((uint32_t)FACTORY_VALUE_STATION_SEC_DNS), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kStationGateway), .nvsKey="GATEWAY", .typ=NvsType::kIntU32, .data=((uint32_t)FACTORY_VALUE_STATION_GATEWAY), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kAccessPointSSID), .nvsKey="APSSID", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_AP_SSID)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kAccessPointPass), .nvsKey="APPASS", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_AP_PASS)), .loaded=false, .modified=true},

    NvsEntry{.key=INT(Cfg::Key::kMqttServer), .nvsKey="MQTT", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_MQTT)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kMqttPort), .nvsKey="MQ_PORT", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_MQTT_PORT), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kMqttUser), .nvsKey="MQ_USER", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_MQTT_USER)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kMqttUserPass), .nvsKey="MQ_PASS", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_MQTT_PASS)), .loaded=false, .modified=true},

    NvsEntry{.key=INT(Cfg::Key::kRelayTime), .nvsKey="R_TIME", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_RELAY_TIME), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kSound), .nvsKey="SOUND", .typ=NvsType::kInt, .data=((int)FACTORY_VALUE_SOUND), .loaded=false, .modified=true},

    NvsEntry{.key=INT(Cfg::Key::kNtpServer), .nvsKey="NTPADDR", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_NTP_SERVER)), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Cfg::Key::kTimeZone), .nvsKey="TIMZONE", .typ=NvsType::kString, .data=(new std::string(FACTORY_VALUE_TIME_ZONE)), .loaded=false, .modified=true},
};




} // namespace Cfg

Nvs *CFG_config = nullptr;

Nvs * NVS_InitConfig() {
    if(CFG_config != nullptr) {
        ESP_LOGE(TAG, "Fatal error: Reinitialization of Config.");
        return nullptr;
    }

    CFG_config = new Nvs(&Cfg::CFG_dataVector, TAG);
    if(!CFG_config->setup(CONFIG_PARTITION_NAME, CONFIG_NAMESPACE)) {
        delete CFG_config;
        CFG_config = nullptr;
        return nullptr;
    }

   return CFG_config;
}

Nvs *GetCfg() {
    if(CFG_config == nullptr) {
        ESP_LOGE(TAG, "Fatal error: Config not initialized.");
        return nullptr;
    }

    return CFG_config;
}

void CFG_factoryReset() {
    if(CFG_config == nullptr) {
        ESP_LOGE(TAG, "Fatal error: Config not initialized.");
        return;
    }

    CFG_config->set(INT(Cfg::Key::kHotelCode), NvsType::kInt, FACTORY_VALUE_HOTEL_CODE);
    CFG_config->set(INT(Cfg::Key::kRoomNumber), NvsType::kInt, FACTORY_VALUE_ROOM_NUMBER);
    CFG_config->set(INT(Cfg::Key::kAccessGroup), NvsType::kIntU64, FACTORY_VALUE_ACCESS_GROUP);

    CFG_config->setString(INT(Cfg::Key::kDeviceName), FACTORY_VALUE_DEVICE_NAME);
    CFG_config->setString(INT(Cfg::Key::kDeviceType), FACTORY_VALUE_DEVICE_TYPE);
    CFG_config->setString(INT(Cfg::Key::kEvent), FACTORY_VALUE_EVENT);
    CFG_config->setString(INT(Cfg::Key::kKeyAES), FACTORY_VALUE_KEY_AES);
    CFG_config->setString(INT(Cfg::Key::kKeyUpdate), FACTORY_VALUE_KEY_UPDATE);

    CFG_config->setString(INT(Cfg::Key::kStationSSID), FACTORY_VALUE_STATION_SSID);
    CFG_config->setString(INT(Cfg::Key::kStationPass), FACTORY_VALUE_STATION_PASS);
    CFG_config->set(INT(Cfg::Key::kStationDHCP), NvsType::kInt, FACTORY_VALUE_STATION_DHCP);
    CFG_config->set(INT(Cfg::Key::kStationIPv4addr), NvsType::kIntU32, FACTORY_VALUE_STATION_IP);
    CFG_config->set(INT(Cfg::Key::kStationIPv4mask), NvsType::kIntU32, FACTORY_VALUE_STATION_MASK);
    CFG_config->set(INT(Cfg::Key::kStationPrimaryDns), NvsType::kIntU32, FACTORY_VALUE_STATION_PRI_DNS);
    CFG_config->set(INT(Cfg::Key::kStationSecondaryDns), NvsType::kIntU32, FACTORY_VALUE_STATION_SEC_DNS);
    CFG_config->set(INT(Cfg::Key::kStationGateway), NvsType::kIntU32, FACTORY_VALUE_STATION_GATEWAY);
    CFG_config->setString(INT(Cfg::Key::kAccessPointSSID), FACTORY_VALUE_AP_SSID);
    CFG_config->setString(INT(Cfg::Key::kAccessPointPass), FACTORY_VALUE_AP_PASS);

    CFG_config->setString(INT(Cfg::Key::kMqttServer), FACTORY_VALUE_MQTT);
    CFG_config->set(INT(Cfg::Key::kMqttPort), NvsType::kInt, FACTORY_VALUE_MQTT_PORT);
    CFG_config->setString(INT(Cfg::Key::kMqttUser), FACTORY_VALUE_MQTT_USER);
    CFG_config->setString(INT(Cfg::Key::kMqttUserPass), FACTORY_VALUE_MQTT_PASS);

    CFG_config->set(INT(Cfg::Key::kRelayTime), NvsType::kInt, FACTORY_VALUE_RELAY_TIME);
    CFG_config->set(INT(Cfg::Key::kSound), NvsType::kInt, FACTORY_VALUE_SOUND);

    CFG_config->setString(INT(Cfg::Key::kNtpServer), FACTORY_VALUE_NTP_SERVER);
    CFG_config->setString(INT(Cfg::Key::kTimeZone), FACTORY_VALUE_TIME_ZONE);
}