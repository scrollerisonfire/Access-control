#ifndef __WIFI_STATION_H__
#define __WIFI_STATION_H__

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "time.h"

#include "tools.hpp"


class WifiStation {
private:
    WifiStation();
    virtual ~WifiStation() {};
public:
    static WifiStation& getInstance() 
    {    
        if(_instance == nullptr)
            _instance = new WifiStation();
        return *_instance;
    };

public:
    enum class State {
        kInit = 0,
        kConfig,
        kWaiting,
        kNtpRequest,
        kNtpWaiting,
        kReady,
        kDisconnected
    };

public:
    bool setup(void);
    void loop(void);
    bool isConnected() { 
        return ((_state == State::kReady) 
                || (_state == State::kNtpRequest)
                || (_state == State::kNtpWaiting)); 
    };

    void onWifiEvent(int32_t event_id);
    void setCallback(InsideCb cb) { _fnEventCallback = cb; };

private:
    void execCallback(InsideEvent event, void* pdata, uint16_t len);


private:
    bool _bInit;
    static WifiStation *_instance;
    State   _state;
    State   _stateOld;
    bool    _bTimeReceived;
    InsideCb    _fnEventCallback;

    uint64_t    _tsDisconnected;
    uint64_t    _tsConnectionAttempt;
    uint64_t    _tsNtpRequest;

};



#endif // __WIFI_STATION_H__