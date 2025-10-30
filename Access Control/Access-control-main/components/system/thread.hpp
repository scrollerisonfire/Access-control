#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


#include "tools.hpp"


#define THREAD_KEEP_ALIVE_TIME_MS           (1000)
#define THREAD_KEEP_ALIVE_TIMEOUT_MS        (2*THREAD_KEEP_ALIVE_TIME_MS)
#define THREAD_SEMAPHORE_TIMEOUT_MS         (200)            
#define THREAD_EXCHANGE_BUFFER_SIZE         (2048)
#define THREAD_QUEUE_DATA_SIZE              (16)
#define THREAD_QUEUE_LENGTH                 (4)


#define THREAD_COMMAND_KEEP_ALIVE           (0x80000001)


#pragma pack(1)
struct ThreadSettings {
    QueueHandle_t   queueToThread;
    QueueHandle_t   queueFromThread;
    QueueHandle_t   smphBufferToThread;
    QueueHandle_t   smphBufferFromThread;
    uint32_t        timeKeepAlive_ms;
    void *          pArgs;
    uint8_t         bufferToThread[THREAD_EXCHANGE_BUFFER_SIZE];
    uint8_t         bufferFromThread[THREAD_EXCHANGE_BUFFER_SIZE];
};
#pragma pack (0)

#pragma pack (1)
struct ThreadMessage {
    uint32_t command;
    uint16_t dataLength;
    uint8_t  buffer[10];
};
#pragma pack (0)



class ThreadManager {
public:

    enum class ThreadState {
        kInit,
        kRunning,
        kSuspend,
        kStopped
    };

public:
    ThreadManager();
    virtual ~ThreadManager() {};

public:
    bool setup(TaskFunction_t fnTask, const char *taskTag, 
            int taskStackSize, void *args = NULL, 
            uint32_t timeKeepAlive = THREAD_KEEP_ALIVE_TIME_MS, 
            bool start = true);
    void loop(void);

    bool pushCommand(uint32_t command, 
            uint8_t* data = nullptr, uint16_t length = 0);

    ThreadState getThreadState();
    void threadStart(void);
    void threadStop(void);
    void threadSuspend(void);
    void threadResume(void);

    void setCallback(InsideCb fnCallback);

private:
    void execCallback(InsideEvent event, void* pdata, uint16_t len);


private:
    bool            _bInit;
    bool            _bRunning;
    TaskFunction_t  _fnTask;
    const char*     _taskTag;
    int             _taskStackSize;
    uint32_t        _timeKeepAlive;

    TaskHandle_t    _taskHandle;
    ThreadState     _state;
    uint64_t        _tsLastResponse;
    ThreadSettings  _threadSettings;

    InsideCb        _fnEventCallback;
};



class ThreadFunction {
public:
    ThreadFunction(ThreadSettings *pThreadSettings);
    virtual ~ThreadFunction() {};

public:
    void loop();

    bool peekCommand(uint32_t *pcommand, uint16_t *plen, uint8_t **pdata);
    void releaseInputBuffer(void);

    uint8_t *getSendBuffer();
    bool pushMessage(uint32_t command, uint16_t len, bool releaseBuffer);

    void checkExecutionTime(uint64_t timePassed_us, 
            const char* filename, size_t fileline);

private:

private:
    ThreadSettings  *_pThreadSettings;
    uint64_t        _tsLastKeepAlive;
};






#endif // __THREAD_HPP__