#include "qr_reader.hpp"

#include "pinout.h"

#include "esp_log.h"
#include "driver/uart.h"

#include "thread.hpp"

#include "structures.hpp"

#include "arduino/DFRobot_GM60.hpp"

#define TAG     "QRC"


#define QR_LIGHT_TIMEOUT_MS    (20000) // 20 seconds

// private functions
void _setBlueBreathing(DFRobot_GM60_UART *pQrSensor);
void _setYellowBreathing(DFRobot_GM60_UART *pQrSensor);
void _setRedBreathing(DFRobot_GM60_UART *pQrSensor);
void _setGreenBreathing(DFRobot_GM60_UART *pQrSensor);
void _setPurpleBreathing(DFRobot_GM60_UART *pQrSensor);
void _lightDimmer(DFRobot_GM60_UART *pQrSensor);


// private variables
static uint64_t _tsLightTimer_ms;


void QrReaderTask(void *args) {
    ThreadFunction fn((ThreadSettings*)args);
    uint32_t command = 0;
    uint16_t dataLength = 0;
    uint8_t* dataBuffer = nullptr;
    uint32_t taskDelay_ms = 50;
    uint64_t tsInitBegin, tsInitEnd;
    uint64_t tsLoopBegin, tsLoopEnd;

    /**  Start the object. 
     * Check the time - if too large (close to keep alive period), this 
     * has to be implemented with a state machine in the loop in order 
     * to send kleep alive messages and process incoming commands. */
    tsInitBegin = MILLIS_WAKE();

    ESP_LOGW(TAG, "GM60 initialization start...");
    DFRobot_GM60_UART qrSensor; 
    uint64_t tsDetectBegin, tsDetectEnd;
    CardData  cardData;
    memset(&cardData, 0x00, sizeof(cardData));

    qrSensor.begin(Serial2);
    vTaskDelay(pdMS_TO_TICKS(50));

    qrSensor.encode(qrSensor.eUTF8);
    qrSensor.setupCode(true, true);
    qrSensor.setIdentify(qrSensor.eForbidAllBarcode);
    qrSensor.setIdentifyQR();
    qrSensor.rawCommand(0x0014, 50);

    vTaskDelay(pdMS_TO_TICKS(50));
    qrSensor.cycleLEDs(100);
    qrSensor.rawCommand(0x0015, 0x63);
    _setBlueBreathing(&qrSensor);
    ESP_LOGW(TAG, "GM60 ready.");

    tsInitEnd = MILLIS_WAKE();
    fn.checkExecutionTime(tsInitEnd-tsInitBegin, __FILE__, __LINE__);

    while(true) {
        /* Execution time control */
        tsLoopBegin = MILLIS_WAKE();
        /* Zero-fill the values for recevied command */
        command = 0;
        dataLength = 0;
        dataBuffer = nullptr;

        /* Allow thread to send KeepAlive messages */
        fn.loop();

        /* Check if new command is received and parse it */
        if(fn.peekCommand(&command, &dataLength, &dataBuffer)) {
            /* execute the command */
            switch(command) {
                case THREAD_COMMAND_QR_BREATH_BLUE: {
                    _setBlueBreathing(&qrSensor);
                } break;

                case THREAD_COMMAND_QR_BREATH_YELLOW: {
                    _setYellowBreathing(&qrSensor);
                } break;

                case THREAD_COMMAND_QR_BREATH_RED: {
                    _setRedBreathing(&qrSensor);
                } break;

                case THREAD_COMMAND_QR_BREATH_GREEN: {
                    _setGreenBreathing(&qrSensor);
                } break;

                case THREAD_COMMAND_QR_BREATH_PURPLE: {
                    _setPurpleBreathing(&qrSensor);
                } break;

                case THREAD_COMMAND_QR_LIGHT_DIMMER: {
                    _lightDimmer(&qrSensor);
                } break;

                default: {
                } break;
            }

            /* release the semaphore */
            if(dataBuffer != nullptr) {
                fn.releaseInputBuffer();
            }
        }

        /* Loop the object, use dynamic waiting time based on current state */
        /* Using <do{}while(0)> structure to allow safe break-s in the code. */
        do {
            tsDetectBegin = MILLIS_WAKE();
            std::string qr = qrSensor.detection();
            tsDetectEnd = MILLIS_WAKE();
            if ((qr.compare("null")==0) || qr.length() < 20)
            {
                break;
            }
            ESP_LOGI(TAG, "Code detected: %s", qr.c_str());
            cardData.lenUID = 4;
            cardData.mUID[0] = 1;
            cardData.mUID[1] = 1;
            cardData.mUID[2] = 1;
            cardData.mUID[3] = 1;

            std::string msg[4] = {"", "", "", ""};
            int chunks = Tools::parseMessage(msg, 4, qr, ',');

            Tools::convertHexStringToByteArray(cardData.cardBlocks[0], 16, msg[0]);
            Tools::convertHexStringToByteArray(cardData.cardBlocks[1], 16, msg[1]);
            Tools::convertHexStringToByteArray(cardData.cardBlocks[3], 16, msg[2]);
            Tools::convertHexStringToByteArray(cardData.cardBlocks[4], 16, msg[3]);

            Tools::dump_byte_array(cardData.cardBlocks[0], 16);
            Tools::dump_byte_array(cardData.cardBlocks[1], 16);
            Tools::dump_byte_array(cardData.cardBlocks[3], 16);
            Tools::dump_byte_array(cardData.cardBlocks[4], 16);

            /* Send data */
            uint8_t * buff = fn.getSendBuffer();
            if(buff != nullptr) {
                memcpy(buff, &cardData, sizeof(cardData));
                fn.pushMessage(THREAD_COMMAND_QR_CARD, sizeof(cardData), true);
            }

            memset(&cardData, 0x00, sizeof(cardData));
        } while(0);
        
        /* Execution time control */
        tsLoopEnd = MILLIS_WAKE();
        fn.checkExecutionTime(tsLoopEnd-tsLoopBegin, __FILE__, __LINE__);

        /* Release the execution to OS */
        vTaskDelay(pdMS_TO_TICKS(taskDelay_ms));
    }
}


// public function definitions

void QR_setBlueBreathing(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_BREATH_BLUE);
}


void QR_setYellowBreathing(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_BREATH_YELLOW);
}


void QR_setRedBreathing(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_BREATH_RED);
}


void QR_setGreenBreathing(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_BREATH_GREEN);
}


void QR_setPurpleBreathing(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_BREATH_PURPLE);
}


void QR_lightDimmer(ThreadManager *th) {
    th->pushCommand(THREAD_COMMAND_QR_LIGHT_DIMMER);
}






// private function definitions

void _setBlueBreathing(DFRobot_GM60_UART *pQrSensor)
{
  pQrSensor->rawCommand(0x001C, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));

  pQrSensor->rawCommand(0x001B, 0b10011111); // Lamp 1 prompt white, green breathing
}


void _setYellowBreathing(DFRobot_GM60_UART *pQrSensor)
{
  pQrSensor->rawCommand(0x001C, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));

  pQrSensor->rawCommand(0x001B, 0b11101111); // Lamp 1 prompt white, green breathing
}

void _setRedBreathing(DFRobot_GM60_UART *pQrSensor)
{
  pQrSensor->rawCommand(0x001C, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));

  pQrSensor->rawCommand(0x001B, 0b11001111); // Lamp 1 prompt white, green breathing
}


void _setGreenBreathing(DFRobot_GM60_UART *pQrSensor)
{
  pQrSensor->rawCommand(0x001C, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));

  pQrSensor->rawCommand(0x001B, 0b10101111); // Lamp 1 prompt white, green breathing
}

void _setPurpleBreathing(DFRobot_GM60_UART *pQrSensor)
{
  pQrSensor->rawCommand(0x001C, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01000000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001B, 0b11011111);
}

void _lightDimmer(DFRobot_GM60_UART *pQrSensor)
{
  if (_tsLightTimer_ms > MILLIS_WAKE())
    return;
  
  _tsLightTimer_ms = MILLIS_WAKE() + QR_LIGHT_TIMEOUT_MS;

  pQrSensor->rawCommand(0x001C, 0b01100000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001D, 0b01100000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));
  pQrSensor->rawCommand(0x001E, 0b01100000); // disable lamp 2,3 breathing
  vTaskDelay(pdMS_TO_TICKS(10));

  pQrSensor->rawCommand(0x001B, 0b00101111); // Lamp 1 prompt green, blue breathing
  vTaskDelay(pdMS_TO_TICKS(10));
}

