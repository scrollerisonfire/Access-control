#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"

class Led {

public:
    Led();
    virtual ~Led() {};  //test

    void setup(void);
    void loop(void);

    void lightDimmer(void);
    void setGreenBreathing(void);
    void setPurpleBreathing(void);
    void setRedBreathing(void);
    void setYellowBreathing(void);
    void setBlueBreathing(void);

private:
    void setBreathing(uint16_t lampAddress, uint8_t command);

private:
    bool _bState; 
};

#endif // __LED_H__
