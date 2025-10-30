#ifndef __QR_READER_HPP__
#define __QR_READER_HPP__

#include "esp_log.h"
#include "driver/uart.h"  

#include "thread.hpp"

#include "arduino/DFRobot_GM60.hpp"


#define THREAD_COMMAND_QR_SECTION           (2000)
#define THREAD_COMMAND_QR_CARD              (1 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_BREATH_BLUE       (2 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_BREATH_YELLOW     (3 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_BREATH_RED        (4 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_BREATH_GREEN      (5 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_BREATH_PURPLE     (6 + THREAD_COMMAND_QR_SECTION)
#define THREAD_COMMAND_QR_LIGHT_DIMMER      (7 + THREAD_COMMAND_QR_SECTION)



void QrReaderTask(void *args);


void QR_setBlueBreathing(ThreadManager *th);
void QR_setYellowBreathing(ThreadManager *th);
void QR_setRedBreathing(ThreadManager *th);
void QR_setGreenBreathing(ThreadManager *th);
void QR_setPurpleBreathing(ThreadManager *th);
void QR_lightDimmer(ThreadManager *th);




#endif // __QR_READER_HPP__