#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <stdint.h>
#include <stdbool.h>

#include "pinout.h"
#include "tools.hpp"

#include "driver/gpio.h"
#include "esp_sleep.h"

// #include "mfrc522.h" 

#define SOUND_DURATION_ACTIVATE_AP                      (200)
#define SOUND_DURATION_RESET_COUNTERS                   (200)
#define SOUND_DURATION_FACTORY_RESET                    (200)
#define SOUND_DURATION_TIME_UPDATE_START                (100)
#define SOUND_DURATION_TIME_UPDATE_READY                (50)
#define SOUND_DURATION_ERROR                            (200)
#define SOUND_DURATION_UPDATE_KEY_CHANGE                (200)
#define SOUND_DURATION_UPDATE_BROKER_START              (100)
#define SOUND_DURATION_UPDATE_BROKER_END                (150)

#define DEFAULT_SLEEP_TIME_US     (3 * 60 * 1000 * 1000ULL)      // 3 minutes 

class Board {

private:
    Board();
    virtual ~Board() {};
public:
    static Board& getInstance() 
    {    
        if(_instance == nullptr)
            _instance = new Board();
        return *_instance;
    };

public:
    bool setup(void);
    void loop(void);

private:
    void loopBuzzer();
    void loopRelay();
    void loopLed();

public:
    void printWakeupReason();
    void sleep(uint64_t timeToSleep);
    void setClock();

    void startSound(int duration);
    void startErrorSound();
    void startActivateAPSound();
    void startTimeUpdateStartSound();
    void startFactoryResetSound();



    void openDoorForDuration(int duration);
    void forceCloseDoor();

    static void pinMode(int pin, gpio_mode_t pintype);
    

private:
    bool readCard();
    bool checkAccess(uint64_t (*checkOfflineCardAccess)(void), void (*opendoor)(uint32_t), void (*saveEvent)(uint64_t), uint32_t relay_time);




private:
    bool _bInit;
    static Board *_instance;
    esp_sleep_wakeup_cause_t _wakeupReason;


    uint64_t    _buzzerTimeExpire;
    bool        _buzzerActivated;
    uint64_t    _relayTimeExpire;
    bool        _relayActivated;
    bool        _indicateError;
    int         _indicateErrorCounter;
    // mfrc522_dev_t* _rfidDev; 



    uint8_t _cardBlock1[16];
    uint8_t _cardBlock2[16];
    uint8_t _cardBlock4[16];
    uint8_t _cardBlock5[16];
    uint8_t _cardBlock6[16];
    uint8_t _nuidPICC[4];
    uint64_t _accessLateCard;
    uint64_t _accessLateUser;
    bool _toSendLate;
};


#endif // __BOARD_HPP__
