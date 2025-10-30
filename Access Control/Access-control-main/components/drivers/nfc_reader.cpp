#include "nfc_reader.hpp"

#include "pinout.h"
#include "tools.hpp"
#include "thread.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"  

#include <cstring>

#include "arduino/mfrc522.hpp"

#include "structures.hpp"

#define TAG     "NFC"


// private
void _readBlock(MFRC522* pDriver, MFRC522::MIFARE_Key *pMifareKey, 
        byte block, byte *buffer);



void NfcReaderTask(void *args) {
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
    ESP_LOGW(TAG, "MFRC522 initialization start...");
    MFRC522::MIFARE_Key mifareKey;
    CardData  cardData;
    memset(&mifareKey, 0xFF, sizeof(mifareKey));  /* default for KeyA */
    memset(&cardData, 0x00, sizeof(cardData));

    SPI.begin(SPI_CS_PIN,RFID_RST_PIN,SPI_MOSI_PIN,SPI_MISO_PIN,SPI_SCK_PIN);
    MFRC522 mfrc522(SPI_CS_PIN, RFID_RST_PIN);
    mfrc522.PCD_Init();
    mfrc522.PCD_SoftPowerUp();
    ESP_LOGW(TAG, "MFRC522 ready.");
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

            /* release the semaphore */
            if(dataBuffer != nullptr) {
                fn.releaseInputBuffer();
            }
        }

        /* Loop the object, use dynamic waiting time based on current state */
        /* Using <do{}while(0)> structure to allow safe break-s in the code. */
        do {
            if (!mfrc522.PICC_IsNewCardPresent()) {
                mfrc522.PICC_HaltA();//state activated = state halted
                mfrc522.PCD_StopCrypto1();//stop algorithym for cripting
                break;
            }
            if (!mfrc522.PICC_ReadCardSerial()) {
                ESP_LOGI(TAG, "No UID");
                mfrc522.PICC_HaltA();// Прекратяваме комуникацията с картата
                mfrc522.PCD_StopCrypto1();//Спираме криптирането на четеца
                break;
            }
            else {
                cardData.lenUID = mfrc522.uid.size;
                memcpy(&cardData.mUID, &mfrc522.uid.uidByte, 
                    MIN(sizeof(MFRC522::Uid), mfrc522.uid.size));
                ESP_LOGW(TAG, "Card found:");
                esp_log_buffer_hex(TAG, mfrc522.uid.uidByte, mfrc522.uid.size);
                ESP_LOGW(TAG, "===================");
            }
            MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
            ESP_LOGI(TAG, "Type: %s", mfrc522.PICC_GetTypeName(piccType));
            if (piccType != MFRC522::PICC_TYPE_MIFARE_1K 
                    && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
            {
                ESP_LOGW(TAG,"This reader only works only"
                                " with MIFARE Classic cards 1kB and 4kB.");
                mfrc522.PICC_HaltA();
                mfrc522.PCD_StopCrypto1();
                memset(&cardData, 0x00, sizeof(cardData));
                break;
            }
            _readBlock(&mfrc522, &mifareKey, 1, cardData.cardBlocks[0]);
            _readBlock(&mfrc522, &mifareKey, 2, cardData.cardBlocks[1]);
            _readBlock(&mfrc522, &mifareKey, 4, cardData.cardBlocks[3]);
            _readBlock(&mfrc522, &mifareKey, 5, cardData.cardBlocks[4]);
            _readBlock(&mfrc522, &mifareKey, 6, cardData.cardBlocks[5]);

            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();

            /* Send data */
            uint8_t * buff = fn.getSendBuffer();
            if(buff != nullptr) {
                memcpy(buff, &cardData, sizeof(cardData));
                fn.pushMessage(THREAD_COMMAND_NFC_CARD, sizeof(cardData), true);
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



void _readBlock(MFRC522* pDriver, MFRC522::MIFARE_Key *pMifareKey, 
        byte block, byte *buffer)
{
  byte len = 18;
  MFRC522::StatusCode status;

  status = pDriver->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, pMifareKey, &(pDriver->uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(pDriver->GetStatusCodeName(status));
    return;
  }

  status = pDriver->MIFARE_Read(block, buffer, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(pDriver->GetStatusCodeName(status));
    return;
  }

  esp_log_buffer_hex(TAG, buffer, len);
}


