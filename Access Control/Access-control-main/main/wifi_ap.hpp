#ifndef __WIFI_AP_H__
#define __WIFI_AP_H__

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "time.h"
#include <esp_http_server.h>


#include "tools.hpp"


class WifiAP {
private:
    WifiAP();
    virtual ~WifiAP() {};
public:
    static WifiAP& getInstance() 
    {    
        if(_instance == nullptr)
            _instance = new WifiAP();
        return *_instance;
    };

public:
    enum class State {
        kInit = 0,
        kConfig,
        // kWaiting,
        // kNtpRequest,
        // kNtpWaiting,
        kReady,
        // kDisconnected
    };

public:
    bool setup(void);
    void loop(void);
    void startAp();
    // bool isConnected() { 
    //     return ((_state == State::kReady) 
    //             || (_state == State::kNtpRequest)
    //             || (_state == State::kNtpWaiting)); 
    // };

    // void onWifiEvent(int32_t event_id);
    void setCallback(InsideCb cb) { _fnEventCallback = cb; };


    esp_err_t httpGetIndex(httpd_req_t *req);
    esp_err_t httpGetJson(httpd_req_t *req);
    esp_err_t httpGetStylesheet(httpd_req_t *req);
    esp_err_t httpGetExit(httpd_req_t *req);
    esp_err_t httpPostConfig(httpd_req_t *req);
    esp_err_t httpPostUpdate(httpd_req_t *req);


private:
    void execCallback(InsideEvent event, void* pdata, uint16_t len);


private:
    bool _bInit;
    static WifiAP *_instance;
    State   _state;
    State   _stateOld;
    InsideCb    _fnEventCallback;

    uint64_t    _tsDisconnected;
    uint64_t    _tsConnectionAttempt;
    uint64_t    _tsNtpRequest;

};



#endif // __WIFI_AP_H__