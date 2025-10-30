#include "nvs_runtime.hpp"
#include "structures.hpp"

#include "esp_log.h"

#define TAG "RT"

namespace Rt {


#define CONFIG_PARTITION_NAME   ("nvs_log")
#define CONFIG_NAMESPACE        ("runtime")

#define     MAX_STRING_LENGTH                               (64)

#define     FACTORY_VALUE_COUNTER_GUEST                     ((uint32_t)0)
#define     FACTORY_VALUE_COUNTER_STAFF                     ((uint32_t)0)
#define     FACTORY_VALUE_COUNTER_MASTER                    ((uint32_t)0)
#define     FACTORY_VALUE_LAST_CLOCK_UPDATE                 ((uint64_t)0)


std::vector<NvsEntry> RT_dataVector = {
    NvsEntry{.key=INT(Rt::Key::kCounterGuest), .nvsKey="C_GUEST", .typ=NvsType::kIntU32, .data=(FACTORY_VALUE_COUNTER_GUEST), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Rt::Key::kCounterStaff), .nvsKey="C_STAFF", .typ=NvsType::kIntU32, .data=(FACTORY_VALUE_COUNTER_STAFF), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Rt::Key::kCounterMaster), .nvsKey="C_MASTR", .typ=NvsType::kIntU32, .data=(FACTORY_VALUE_COUNTER_MASTER), .loaded=false, .modified=true},
    NvsEntry{.key=INT(Rt::Key::kLastClockUpdate), .nvsKey="LUPDATE", .typ=NvsType::kIntU64, .data=(FACTORY_VALUE_LAST_CLOCK_UPDATE), .loaded=false, .modified=true},
};

} // namespace Rt

Nvs *RT_config = nullptr;

Nvs * NVS_InitRuntime() {
    if(RT_config != nullptr) {
        ESP_LOGE(TAG, "Fatal error: Reinitialization of Runtime Data.");
        return nullptr;
    }

    RT_config = new Nvs(&Rt::RT_dataVector, TAG);
    if(!RT_config->setup(CONFIG_PARTITION_NAME, CONFIG_NAMESPACE)) {
        delete RT_config;
        RT_config = nullptr;
        return nullptr;
    }

   return RT_config;
}

Nvs *GetRt() {
    if(RT_config == nullptr) {
        ESP_LOGE(TAG, "Fatal error: Runtime Data not initialized.");
        return nullptr;
    }
    return RT_config;
}

void NVS_factoryReset() {
    RT_config->set(INT(Rt::Key::kCounterGuest), NvsType::kIntU32, FACTORY_VALUE_COUNTER_GUEST);
    RT_config->set(INT(Rt::Key::kCounterStaff), NvsType::kIntU32, FACTORY_VALUE_COUNTER_STAFF);
    RT_config->set(INT(Rt::Key::kCounterMaster), NvsType::kIntU32, FACTORY_VALUE_COUNTER_MASTER);
    RT_config->set(INT(Rt::Key::kLastClockUpdate), NvsType::kIntU64, FACTORY_VALUE_LAST_CLOCK_UPDATE);
}