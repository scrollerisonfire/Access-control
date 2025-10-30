#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__

#include <stdint.h>
#include <stdbool.h>
#include "tools.hpp"
#include "thread.hpp"
#include "nvs_config.hpp"


class Device {

private:
    Device();
    virtual ~Device() {};
public:
    static Device& getInstance() 
    {    
        if(_instance == nullptr)
            _instance = new Device();
        return *_instance;
    };

public:
    bool setup(void);
    void loop(void);

    void wifiStaHandler(InsideEvent event, void* pdata);
    void wifiApHandler(InsideEvent event, void* pdata);

private:
    // private member methods

    void cbWiFiAccessPoint(InsideEvent ev, void* pdata, uint16_t len);
    void cbWiFiStation(InsideEvent ev, void* pdata, uint16_t len);
    void cbAccessControl(InsideEvent ev, void* pdata, uint16_t len);
    void cbThreadNfc(InsideEvent ev, void* pdata, uint16_t len);
    void cbThreadQr(InsideEvent ev, void* pdata, uint16_t len);



private:
    // private data mebers
    bool    _bInit;
    static Device *_instance;

    Nvs             *_config;
    Nvs             *_runtime;

    ThreadManager   *_threadNfc;
    ThreadManager   *_threadQr;



};


#endif // __DEVICE_HPP__