#include "nvs.hpp"

#include <cstring>
#include <cstdio>

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>


#define TAG     "NVS"

#define     MAX_STRING_LENGTH                               (64)

#define     FACTORY_VALUE_HOTEL_CODE                        ((int)1)
#define     FACTORY_VALUE_ROOM_NUMBER                       ((int)1)
#define     FACTORY_VALUE_ACCESS_GROUP                      ((uint64_t)1)
#define     FACTORY_VALUE_DEVICE_NAME                       ((char*)"")
#define     FACTORY_VALUE_DEVICE_TYPE                       ((char*)"")
#define     FACTORY_VALUE_EVENT                             ((char*)"")
#define     FACTORY_VALUE_KEY_AES                           ((char*)"")
#define     FACTORY_VALUE_KEY_UPDATE                        ((char*)"")
// #define     FACTORY_VALUE_STATION_SSID                      ("")
// #define     FACTORY_VALUE_STATION_PASS                      ("")
// TODO: Remove after DEBUG
#define     FACTORY_VALUE_STATION_SSID                      ((char*)"")
#define     FACTORY_VALUE_STATION_PASS                      ((char*)"")

#define     FACTORY_VALUE_STATION_DHCP                      ((int)1)
#define     FACTORY_VALUE_STATION_IP                        ((uint32_t)(192<<0) | (168<<8) | (1<<16) | (19<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_MASK                      ((uint32_t)(255<<0) | (255<<8) | (255<<16) | (0<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_PRI_DNS                   ((uint32_t)(0<<0) | (0<<8) | (0<<16) | (0<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_SEC_DNS                   ((uint32_t)(0<<0) | (0<<8) | (0<<16) | (0<<24))  /* network byte order */
#define     FACTORY_VALUE_STATION_GATEWAY                   ((uint32_t)(10<<0) | (10<<8) | (1<<16) | (15<<24))  /* network byte order */
#define     FACTORY_VALUE_AP_SSID                           ((char*)"SLIM")
#define     FACTORY_VALUE_AP_PASS                           ((char*)"")
#define     FACTORY_VALUE_MQTT                              ((char*)"")  /* left as string in case domain name needs to be used */
#define     FACTORY_VALUE_MQTT_PORT                         ((int)1885)
#define     FACTORY_VALUE_MQTT_USER                         ((char*)"")
#define     FACTORY_VALUE_MQTT_PASS                         ((char*)"")
#define     FACTORY_VALUE_RELAY_TIME                        ((int)3)
#define     FACTORY_VALUE_SOUND                             ((int)1)
#define     FACTORY_VALUE_NTP_SERVER                        ((char*)"")
#define     FACTORY_VALUE_TIME_ZONE                         ((char*)"EET-2EEST,M3.5.0/3,M10.5.0/4")



// Nvs* Nvs::_instance = nullptr;





Nvs::Nvs(std::vector<NvsEntry> *vect, const char * tag) :
        _tag(tag),
        _dataVector(vect)
{
    _bInit = false;
    _hNvsPartition = 0;
}

bool Nvs::setup(const char * sPartitionName, const char * sNamespace)
{
    _sPartitionName = sPartitionName;
    _sNamespace = sNamespace;

    if(_sPartitionName == nullptr || _sNamespace == nullptr 
        || strlen(_sPartitionName) == 0 || strlen(_sNamespace) == 0) {
        ESP_LOGD(_tag, "Partition name and namespace invalid.");
    }

    esp_err_t err = ESP_OK;

    err = nvs_flash_init_partition(_sPartitionName);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased.
        err = nvs_flash_erase_partition(_sPartitionName);
        if(err != ESP_OK)
            return false;

        err = nvs_flash_init_partition(_sPartitionName);
        if(err != ESP_OK)
            return false;
    }

    err = nvs_open_from_partition(_sPartitionName, _sNamespace, NVS_READWRITE, &_hNvsPartition);
    if(err != ESP_OK)
        return false;

    _bInit = true;
    loadData();

    return true;
}


void Nvs::loop(void)
{
    if(!_bInit)
        return;
}


MultiType Nvs::get(int k, NvsType t) {
    int idx = k;
    if(idx < 0 || idx >= _dataVector->size())
        return 0;

    if(_dataVector->at(idx).typ != t)
        return 0;

    return _dataVector->at(idx).data;
}


const char* Nvs::getString(int k) {
    return (std::get<std::string*>(this->get(k, NvsType::kString)))->c_str();
}


int   Nvs::getInt(int k) {
    return (std::get<int>(this->get(k, NvsType::kInt)));
}

uint32_t Nvs::getU32(int k) {
    return (std::get<uint32_t>(this->get(k, NvsType::kIntU32)));
}

uint64_t Nvs::getU64(int k) {
    return (std::get<uint64_t>(this->get(k, NvsType::kIntU64)));
}


double Nvs::getDouble(int k) {
    return (std::get<double>(this->get(k, NvsType::kDouble)));
}



void  Nvs::set(int k, NvsType t, MultiType val) {
    int idx = k;
    if(idx < 0 || idx >= _dataVector->size())
        return;
    if(_dataVector->at(idx).typ != t)
        return;

    if(_dataVector->at(idx).data != val) {
        _dataVector->at(idx).data = val;
        _dataVector->at(idx).modified = true;
    }
}


void  Nvs::setString(int k, char* cstr) {
    int idx = k;
    if(idx < 0 || idx >= _dataVector->size())
        return;
    if(_dataVector->at(idx).typ != NvsType::kString)
        return;

    if(strlen(cstr) >= MAX_STRING_LENGTH)
        cstr[MAX_STRING_LENGTH] = '\0';

    if((std::get<std::string*>(_dataVector->at(idx).data))->compare(cstr) != 0) {
        *(std::get<std::string*>(_dataVector->at(idx).data)) = std::string(cstr);
        _dataVector->at(idx).modified = true;
    }
}

void  Nvs::setString(int k, std::string &s) {
    int idx = k;
    if(idx < 0 || idx >= _dataVector->size())
        return;
    if(_dataVector->at(idx).typ != NvsType::kString)
        return;

    if(s.length() >= MAX_STRING_LENGTH)
        s.erase(MAX_STRING_LENGTH-1, std::string::npos);

    if((std::get<std::string*>(_dataVector->at(idx).data))->compare(s) != 0) {
        *(std::get<std::string*>(_dataVector->at(idx).data)) = s;
        _dataVector->at(idx).modified = true;
    }
}


void Nvs::loadData() {
    if(!_bInit)
        return;

    bool failed = false;
    esp_err_t err = ESP_OK;

    for(NvsEntry& item : *_dataVector) {
        switch(item.typ) {
            case NvsType::kInt: {
                int32_t val = 0;
                err = nvs_get_i32(_hNvsPartition, item.nvsKey, &val);
                if(err != ESP_OK) { failed = true; break; }
                if(err == ESP_ERR_NVS_NOT_FOUND) break;     // just skip this one
                set(item.key, NvsType::kInt, (int)val);
                item.loaded = true;
                item.modified = false;
            } break;
            case NvsType::kIntU32: {
                uint32_t val = 0;
                err = nvs_get_u32(_hNvsPartition, item.nvsKey, &val);
                if(err != ESP_OK) { failed = true; break; }
                if(err == ESP_ERR_NVS_NOT_FOUND) break;     // just skip this one
                set(item.key, NvsType::kIntU32, val);
                item.loaded = true;
                item.modified = false;
            } break;
            case NvsType::kIntU64: {
                uint64_t val = 0;
                err = nvs_get_u64(_hNvsPartition, item.nvsKey, &val);
                if(err != ESP_OK) { failed = true; break; }
                if(err == ESP_ERR_NVS_NOT_FOUND) break;     // just skip this one
                set(item.key, NvsType::kIntU64, val);
                item.loaded = true;
                item.modified = false;
            } break;
            case NvsType::kDouble: {
                uint64_t val64 = 0;
                double val_lf = 0;
                err = nvs_get_u64(_hNvsPartition, item.nvsKey, &val64);
                if(err != ESP_OK) { failed = true; break; }
                if(err == ESP_ERR_NVS_NOT_FOUND) break;     // just skip this one
                memcpy(&val_lf, &val64, sizeof(val64));
                set(item.key, NvsType::kDouble, val_lf);
                item.loaded = true;
                item.modified = false;
            } break;
            case NvsType::kString: {
                char buff[MAX_STRING_LENGTH];
                size_t slen = MAX_STRING_LENGTH;
                err = nvs_get_str(_hNvsPartition, item.nvsKey, buff, &slen);
                if(err != ESP_OK) { failed = true; break; }
                if(err == ESP_ERR_NVS_NOT_FOUND) break;     // just skip this one
                buff[slen] = '\0';
                setString(item.key, (char*)buff);
                item.loaded = true;
                item.modified = false;
            } break;
        }

        if(failed)
            break;
    }

}

void Nvs::saveData() {
    if(!_bInit)
        return;

    bool failed = false;
    esp_err_t err = ESP_OK;
    int iItems = 0;
    int iUpdated = 0;

    for(NvsEntry& item : *_dataVector) {
        iItems++;
        if(!item.modified)
            continue;

        switch(item.typ) {
            case NvsType::kInt: {
                err = nvs_set_i32(_hNvsPartition, item.nvsKey, 
                        (int32_t)(std::get<int>(item.data)));
                if(err != ESP_OK) { failed = true; break; }
                item.modified = false;
            } break;
            case NvsType::kIntU32: {
                err = nvs_set_u32(_hNvsPartition, item.nvsKey, 
                        (std::get<uint32_t>(item.data)));
                if(err != ESP_OK) { failed = true; break; }
                item.modified = false;
            } break;
            case NvsType::kIntU64: {
                err = nvs_set_u64(_hNvsPartition, item.nvsKey, 
                        (std::get<uint64_t>(item.data)));
                if(err != ESP_OK) { failed = true; break; }
                item.modified = false;
            } break;
            case NvsType::kDouble: {
                uint64_t val64 = 0;
                double val_lf = (std::get<double>(item.data));
                memcpy(&val64, &val_lf, sizeof(val64));
                err = nvs_set_u64(_hNvsPartition, item.nvsKey, 
                        val64);
                if(err != ESP_OK) { failed = true; break; }
                item.modified = false;
            } break;
            case NvsType::kString: {
                const char *s = (std::get<std::string*>(item.data))->c_str();
                err = nvs_set_str(_hNvsPartition, item.nvsKey, s);
                if(err != ESP_OK) { failed = true; break; }
                item.modified = false;
            } break;
        }

        if(failed)
            break;
        iUpdated++;
    }

    if(iItems == _dataVector->size()) {
        ESP_LOGI(_tag, "NVS [%s] saveData(): OK. Items processed = %d, Items updated = %d", _sNamespace, iItems, iUpdated);
    }
    else {
        ESP_LOGE(_tag, "NVS [%s] saveData(): Failed. Items processed = %d, Items updated = %d", _sNamespace, iItems, iUpdated);
    }
}


