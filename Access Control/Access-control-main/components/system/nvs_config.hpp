#ifndef __NVS_CONFIG_HPP__
#define __NVS_CONFIG_HPP__

#include "nvs.hpp"

#include <vector>

namespace Cfg {

enum class Key {
    kHotelCode = 0,
    kRoomNumber,
    kAccessGroup,
    kDeviceName,
    kDeviceType,
    kEvent,
    kKeyAES,
    kKeyUpdate,
    kStationSSID,
    kStationPass,
    kStationDHCP,
    kStationIPv4addr,
    kStationIPv4mask,
    kStationPrimaryDns,
    kStationSecondaryDns,
    kStationGateway,
    kAccessPointSSID,
    kAccessPointPass,
    kMqttServer,
    kMqttPort,
    kMqttUser,
    kMqttUserPass,
    kRelayTime,
    kSound,
    kNtpServer,
    kTimeZone,
    
    kMax
};

}   // namespace Cfg


Nvs *NVS_InitConfig();
Nvs *GetCfg();
void CFG_factoryReset();


#endif // __NVS_CONFIG_HPP__