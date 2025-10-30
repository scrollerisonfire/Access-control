#include "thread.hpp"

#include <functional>
#include <cstring>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define TAG     "THR"

ThreadManager::ThreadManager() {
    _bInit = false;
    _fnEventCallback = nullptr;
    _fnTask = nullptr;
    _bRunning = false;
    _taskTag = "task";
    _taskStackSize = 0;
    _timeKeepAlive = 0;
    memset(&_threadSettings, 0, sizeof(_threadSettings));
}



bool ThreadManager::setup(TaskFunction_t fnTask, const char *taskTag, 
        int taskStackSize, void *args, uint32_t timeKeepAlive, bool start) {
    if(_bInit)
        return _bInit;

    _fnTask = fnTask;
    _taskTag = taskTag;
    _taskStackSize = taskStackSize;
    _timeKeepAlive = timeKeepAlive;

    _threadSettings.pArgs = args;
    _threadSettings.timeKeepAlive_ms = _timeKeepAlive;

    if(start) {
        threadStart();
    }

    _bInit = true;
    return _bInit;
}



void ThreadManager::loop(void) {
    if(!_bInit)
        return;
    
    ThreadMessage msg;
    memset(&msg, 0, sizeof(ThreadMessage));

    if(xQueueReceive(_threadSettings.queueFromThread, (void*)&msg, 
        pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS))) {

        _tsLastResponse = MILLIS_WAKE();

        switch(msg.command) {
            case THREAD_COMMAND_KEEP_ALIVE: {
                ESP_LOGI(TAG, "Thread [%s] is running fine.", _taskTag);
            } break;

            default: {
                // application commands here
                uint16_t len = msg.dataLength;
                if(len > 0) {
                    if(xSemaphoreTake(_threadSettings.smphBufferFromThread, 
                        pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) == pdTRUE) {

                        execCallback(InsideEvent::kThreadData, 
                            _threadSettings.bufferFromThread, len);
                        xSemaphoreGive(_threadSettings.smphBufferFromThread);
                    }
                }
                else {
                    execCallback(InsideEvent::kThreadData, nullptr, 0);
                }
            } break;
        }

    }

    if(_tsLastResponse + THREAD_KEEP_ALIVE_TIMEOUT_MS < MILLIS_WAKE()) {
        // thread not responding -> restart
        ESP_LOGE(TAG, "Thread [%s] not responding. Restart...", _taskTag);
        threadStop();
        threadStart();
    }
}


bool ThreadManager::pushCommand(uint32_t command, 
        uint8_t* data, uint16_t length) {

    bool success = true;

    if(data != nullptr && length != 0) {
        if(xSemaphoreTake(_threadSettings.smphBufferToThread, 
            pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) == pdTRUE) {

            memset(_threadSettings.bufferToThread, 0, 
                sizeof(_threadSettings.bufferToThread));
            memcpy(_threadSettings.bufferToThread, data, length);
        
            xSemaphoreGive(_threadSettings.smphBufferToThread);
        }
        else {
            success = false;
        }
    }

    if(success) {
        ThreadMessage msg;
        memset(&msg, 0, sizeof(ThreadMessage));
        msg.command = command;
        msg.dataLength = (data != nullptr && length != 0) ? length : 0;
        if(xQueueSend(_threadSettings.queueToThread, (void*)&msg, 
            pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) != pdTRUE) {
            success = false;
        }
    }
    return success;
}



void ThreadManager::threadStop(void) {
    if(!_bRunning)
        return;

    vTaskDelete(_taskHandle);

    vQueueDelete(_threadSettings.queueToThread);
    vQueueDelete(_threadSettings.queueFromThread);
    vSemaphoreDelete(_threadSettings.smphBufferToThread);
    vSemaphoreDelete(_threadSettings.smphBufferFromThread);
    memset(&_threadSettings.bufferFromThread, 0, 
            sizeof(_threadSettings.bufferFromThread));
    memset(&_threadSettings.bufferToThread, 0, 
            sizeof(_threadSettings.bufferToThread));
    _threadSettings.queueToThread = nullptr;
    _threadSettings.queueFromThread = nullptr;
    _threadSettings.smphBufferToThread = nullptr;
    _threadSettings.smphBufferFromThread = nullptr;

    _state = ThreadManager::ThreadState::kStopped;
    _taskHandle = nullptr;
    _bRunning = false;
}


void ThreadManager::threadStart(void) {
    if(_bRunning)
        return;

    _threadSettings.queueToThread = 
        xQueueCreate(THREAD_QUEUE_LENGTH, THREAD_QUEUE_DATA_SIZE);
    _threadSettings.queueFromThread = 
        xQueueCreate(THREAD_QUEUE_LENGTH, THREAD_QUEUE_DATA_SIZE);
    vSemaphoreCreateBinary(_threadSettings.smphBufferToThread);
    vSemaphoreCreateBinary(_threadSettings.smphBufferFromThread);

    // ESP_LOGW(TAG, "TaskFunction_t = 0x%08lX, Tag = [%s], Stack = %d, Settings: 0x%08lX",
    //     (uint32_t)(_fnTask), _taskTag, _taskStackSize, (uint32_t)(&_threadSettings));

    xTaskCreatePinnedToCore(
            (TaskFunction_t)(_fnTask),
            _taskTag,
            _taskStackSize,
            (void*) &_threadSettings,
            5, &_taskHandle, 0);
    _bRunning = true;
    _state = ThreadManager::ThreadState::kRunning;
}

void ThreadManager::threadSuspend(void) {
    if(!_bRunning)
        return;

    vTaskSuspend(_taskHandle);
    _state = ThreadManager::ThreadState::kSuspend;
}


void ThreadManager::threadResume(void) {
    if(!_bRunning)
        return;

    vTaskResume(_taskHandle);
    _state = ThreadManager::ThreadState::kRunning;
}


void ThreadManager::setCallback(InsideCb fnCallback) {
    _fnEventCallback = fnCallback;
}


ThreadManager::ThreadState ThreadManager::getThreadState() {
    return _state;
}




void ThreadManager::execCallback(InsideEvent event, void* pdata, uint16_t len) {
    if(_fnEventCallback != nullptr) {
        _fnEventCallback(event, pdata, len);
    }
}




ThreadFunction::ThreadFunction(ThreadSettings *pThreadSettings) {
    _pThreadSettings = pThreadSettings;
    _tsLastKeepAlive = 0;
}

void ThreadFunction::loop() {
    if(_pThreadSettings == nullptr)
        return;

    if(_tsLastKeepAlive + _pThreadSettings->timeKeepAlive_ms < MILLIS_WAKE()) {
        pushMessage(THREAD_COMMAND_KEEP_ALIVE, 0, false);
        _tsLastKeepAlive = MILLIS_WAKE();
    }
}

bool ThreadFunction::peekCommand(uint32_t *pcommand, uint16_t *plen, uint8_t **pdata) {
    if(_pThreadSettings == nullptr)
        return false;

    if(pcommand == nullptr || plen == nullptr || pdata == nullptr) {
        ESP_LOGE(TAG, "ThreadFunction::peekCommand() - invalid arguments.");
        return false;
    }

    ThreadMessage msg;
    memset(&msg, 0, sizeof(ThreadMessage));

    if(xQueueReceive(_pThreadSettings->queueToThread, (void*)&msg, 
        pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS))) {
    
        *pcommand = msg.command;
        *plen = msg.dataLength;
        if(msg.dataLength > 0) {
            if(xSemaphoreTake(_pThreadSettings->smphBufferToThread, 
                    pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) == pdTRUE) {

                *pdata = _pThreadSettings->bufferToThread;
            }
            else {
                *pcommand = 0;
                *plen = 0;
                *pdata = nullptr;
                return false;
            }
        }
        else {
            *pdata = nullptr;
        }
        return true;        
    }
    return false;   // no new messages
}


void ThreadFunction::releaseInputBuffer(void) {
    if(_pThreadSettings == nullptr)
        return;

    xSemaphoreGive(_pThreadSettings->smphBufferToThread);
}

uint8_t * ThreadFunction::getSendBuffer() {
    if(_pThreadSettings == nullptr)
        return nullptr;

    if(xSemaphoreTake(_pThreadSettings->smphBufferFromThread, 
            pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) == pdTRUE) {
        return _pThreadSettings->bufferFromThread;
    }

    return nullptr;
}


bool ThreadFunction::pushMessage(uint32_t command, uint16_t len, 
            bool releaseBuffer) {
    if(_pThreadSettings == nullptr)
        return false;
    
    bool success = true;
    ThreadMessage msg;
    memset(&msg, 0, sizeof(ThreadMessage));
    msg.command = command;
    msg.dataLength = len;
    if(xQueueSend(_pThreadSettings->queueFromThread, (void*)&msg, 
        pdMS_TO_TICKS(THREAD_SEMAPHORE_TIMEOUT_MS)) != pdTRUE) {
        success = false;
    }

    if(releaseBuffer) {
        xSemaphoreGive(_pThreadSettings->smphBufferFromThread);
    }

    return success;
}



void ThreadFunction::checkExecutionTime(uint64_t timePassed_us,
        const char* filename, size_t fileline) {
    if(timePassed_us > _pThreadSettings->timeKeepAlive_ms * 1000 / 2) {
        ESP_LOGE(TAG, "Execution time is critical in thread [%s : %d].\n"
            "Consider refactoring!", filename, fileline);
    }
}



/* ====================== EXAMPLE USAGE CODE ====================== */


// // some private function declaration
// void _privateFunction(void);

// // task function
// void MyTask(void *args) {
//     ThreadFunction fn((ThreadSettings*)args);
//     uint32_t command = 0;
//     uint16_t dataLength = 0;
//     uint8_t* dataBuffer = nullptr;
//     uint32_t taskDelay_ms = 10;
//     uint64_t tsInitBegin, tsInitEnd;
//     uint64_t tsLoopBegin, tsLoopEnd;

//     /**  Start the object. 
//      * Check the time - if too large (close to keep alive period), this 
//      * has to be implemented with a state machine in the loop in order 
//      * to send kleep alive messages and process incoming commands. */
//     tsInitBegin = MILLIS_WAKE();
//     ESP_LOGW(TAG, "<Your Task Name> initialization start...");
//     /* declare task variables here */
//     /* init peripheral and other objects here */
//     ESP_LOGW(TAG, "<Your Task Name> initialization ready.");
//     tsInitEnd = MILLIS_WAKE();
//     fn.checkExecutionTime(tsInitEnd-tsInitBegin, __FILE__, __LINE__);

//     while(true) {
//         /* Execution time control */
//         tsLoopBegin = MILLIS_WAKE();
//         /* Zero-fill the values for recevied command */
//         command = 0;
//         dataLength = 0;
//         dataBuffer = nullptr;
//        /* Allow thread to send KeepAlive messages */
//         fn.loop();
//         /* Check if new command is received and parse it */
//         if(fn.peekCommand(&command, &dataLength, &dataBuffer)) {
//             /* execute the command */

//             /* release the semaphore */
//             if(dataBuffer != nullptr) {
//                 fn.releaseInputBuffer();
//             }
//         }

//         /* Loop the object, use dynamic waiting time based on current state */
//         /* Using <do{}while(0)> structure to allow safe break-s in the code. */
//         do {
//             /* job-specific code here */
//             if(false /* something happened */) {
//                 break;  /* this will leave job-specific section for this 
//                 iteration but will still execute system-related stuff below  */
//             }
//             /* job-specific code here */

//             /* Send data */
//             uint8_t * buff = fn.getSendBuffer();
//             if(buff != nullptr) {
//                 // memcpy(buff, &cardData, sizeof(cardData));
//                 // fn.pushMessage(THREAD_COMMAND_NFC_CARD, sizeof(cardData), true);
//             }
//             // memset(&cardData, 0x00, sizeof(cardData));
//         } while(0);
        
//         /* Execution time control */
//         tsLoopEnd = MILLIS_WAKE();
//         fn.checkExecutionTime(tsLoopEnd-tsLoopBegin, __FILE__, __LINE__);

//         /* Release the execution to OS */
//         vTaskDelay(pdMS_TO_TICKS(taskDelay_ms));
//     }
// }



// // some private function definition
// void _privateFunction(void)
// {
//     /* job-specific code here */
// }


/* ====================== End-Of: EXAMPLE USAGE CODE ====================== */
