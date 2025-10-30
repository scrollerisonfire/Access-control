#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"


#include "device.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "nvs_config.hpp"
#include "nvs_runtime.hpp"


#define TAG     "MAIN"

#ifdef __cplusplus
extern "C"{
#endif

void app_main(void) {

    if(!Device::getInstance().setup()) {
        while(true) {
            // block on a fatal error
            vTaskDelay(1);    
        }
    }

    


    while(true) {
        Device::getInstance().loop();
        vTaskDelay(1);
    }
}



/*
void app_main_test_cfg(void) {

    if(!Device::getInstance().setup()) {
        while(true) {
            // block on a fatal error
            vTaskDelay(1);    
        }
    }

    int r = GetCfg()->getInt(INT(Cfg::Key::kRoomNumber));
    ESP_LOGI(TAG, "Check data: kRoomNumber = %d", r);
    GetCfg()->set(INT(Cfg::Key::kRoomNumber), NvsType::kInt, r+1);
    ESP_LOGI(TAG, "Check data: kRoomNumber = %d", GetCfg()->getInt(INT(Cfg::Key::kRoomNumber)));

    std::string s = GetCfg()->getString(INT(Cfg::Key::kEvent));
    ESP_LOGI(TAG, "Check data: kEvent = %s, length = %d", s.c_str(), s.length());
    s += "A";
    GetCfg()->setString(INT(Cfg::Key::kEvent), s);
    ESP_LOGI(TAG, "Check data: kEvent = %s", GetCfg()->getString(INT(Cfg::Key::kEvent)));

    uint64_t tStart = 0;
    uint64_t tEnd = 0;
    tStart = esp_timer_get_time();
    GetCfg()->saveData();
    tEnd = esp_timer_get_time();

    ESP_LOGW(TAG, "Saving config took %llu microseconds.", (tEnd-tStart));


    ESP_LOGI(TAG, "Check data: kHotelCode = %d", GetCfg()->getInt(INT(Cfg::Key::kHotelCode)));
    ESP_LOGI(TAG, "Check data: kRoomNumber = %d", GetCfg()->getInt(INT(Cfg::Key::kRoomNumber)));
    ESP_LOGI(TAG, "Check data: kAccessGroup = %llu", GetCfg()->getU64(INT(Cfg::Key::kAccessGroup)));
    ESP_LOGI(TAG, "Check data: kDeviceName = %s", GetCfg()->getString(INT(Cfg::Key::kDeviceName)));
    ESP_LOGI(TAG, "Check data: kEvent = %s", GetCfg()->getString(INT(Cfg::Key::kEvent)));
    ESP_LOGI(TAG, "Check data: kKeyUpdate = %s", GetCfg()->getString(INT(Cfg::Key::kKeyUpdate)));
    GetCfg()->setString(INT(Cfg::Key::kKeyUpdate), "0123456789ABCDEF0123456789ABCDEF");
    ESP_LOGW(TAG, "Check data: kKeyUpdate = %s", GetCfg()->getString(INT(Cfg::Key::kKeyUpdate)));
    GetCfg()->saveData();


    uint32_t guest = GetRt()->getU32(INT(Rt::Key::kCounterGuest));
    ESP_LOGI(TAG, "Check data: kCounterGuest = %lu", guest);
    guest++;
    GetRt()->set(INT(Rt::Key::kCounterGuest), NvsType::kIntU32, guest);
    ESP_LOGI(TAG, "Check data: kCounterGuest = %lu", guest);
    GetRt()->saveData();


    while(true) {
        Device::getInstance().loop();
        vTaskDelay(1);
    }
}
*/



#ifdef __cplusplus
} // extern "C"
#endif




/*

    Config tests


    // int r = Config::getInstance().getInt(Config::Key::kRoomNumber);
    // ESP_LOGI(TAG, "Check data: kRoomNumber = %d", r);
    // Config::getInstance().set(Config::Key::kRoomNumber, NvsType::kInt, r+1);
    // ESP_LOGI(TAG, "Check data: kRoomNumber = %d", Config::getInstance().getInt(Config::Key::kRoomNumber));

    // std::string s = Config::getInstance().getString(Config::Key::kEvent);
    // ESP_LOGI(TAG, "Check data: kEvent = %s, length = %d", s.c_str(), s.length());
    // s += "A";
    // Config::getInstance().setString(Config::Key::kEvent, s);
    // ESP_LOGI(TAG, "Check data: kEvent = %s", Config::getInstance().getString(Config::Key::kEvent));

    // uint64_t tStart = 0;
    // uint64_t tEnd = 0;
    // tStart = esp_timer_get_time();
    // Config::getInstance().saveConfig();
    // tEnd = esp_timer_get_time();

    // ESP_LOGW(TAG, "Saving config took %llu microseconds.", (tEnd-tStart));


    // ESP_LOGI(TAG, "Check data: kHotelCode = %d", Config::getInstance().getInt(Config::Key::kHotelCode));
    // ESP_LOGI(TAG, "Check data: kRoomNumber = %d", Config::getInstance().getInt(Config::Key::kRoomNumber));
    // ESP_LOGI(TAG, "Check data: kAccessGroup = %llu", Config::getInstance().getU64(Config::Key::kAccessGroup));
    // ESP_LOGI(TAG, "Check data: kDeviceName = %s", Config::getInstance().getString(Config::Key::kDeviceName));
    // ESP_LOGI(TAG, "Check data: kEvent = %s", Config::getInstance().getString(Config::Key::kEvent));
    // ESP_LOGI(TAG, "Check data: kKeyUpdate = %s", Config::getInstance().getString(Config::Key::kKeyUpdate));
    // Config::getInstance().setString(Config::Key::kKeyUpdate, "0123456789ABCDEF0123456789ABCDEF");
    // ESP_LOGW(TAG, "Check data: kKeyUpdate = %s", Config::getInstance().getString(Config::Key::kKeyUpdate));




*/



















// #include "_wificonnect.h"




// #define RC522_SPI_BUS_GPIO_MISO    (GPIO_NUM_22)
// #define RC522_SPI_BUS_GPIO_MOSI    (GPIO_NUM_23)
// #define RC522_SPI_BUS_GPIO_SCLK    (GPIO_NUM_21)
// #define RC522_SCANNER_GPIO_CS      (GPIO_NUM_20) 
// #define RC522_SCANNER_GPIO_RST     (GPIO_NUM_14)




// #ifdef __cplusplus
// extern "C"{
// #endif

// bool request(RFID *rfid) {
//     unsigned char reqMode = PICC_REQALL;  
//     unsigned char tagType[MI_MAX_LEN];

//     unsigned char status = rfid->MFRC522Request(reqMode, tagType);  
//     printf("Result %d\n", status);

//     if (status == MI_OK) {
//         return true;  
//     } else {
//         return false; 
//     }
// }





// void app_main(void)
// {
//     //Initialize NVS
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//       ESP_ERROR_CHECK(nvs_flash_erase());
//       ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
//     wifi_init_softap();
//     ESP_LOGI(TAG, "before delay");
//     vTaskDelay(pdMS_TO_TICKS (10000));
//     ESP_LOGI(TAG, "after delay");

//     while (1)
//     {
//         vTaskDelay(pdMS_TO_TICKS (1000));
//     }




//     Board::getInstance().setup();
//     Board::getInstance().openDoorForDuration(3000);
//     //board.startTimeUpdateStartSound();
//     while (1)
//     {
//         Board::getInstance().loop();
//         vTaskDelay(pdMS_TO_TICKS (5));
//     }
    
    //board.startSound();
    //board.loopBuzzer();
    // wifi_connection();


    // printf("Wi-Fi initialized successfully\n");
    // RFID rfid(RC522_SCANNER_GPIO_CS, RC522_SCANNER_GPIO_RST, RC522_SPI_BUS_GPIO_MOSI,RC522_SPI_BUS_GPIO_MISO, RC522_SPI_BUS_GPIO_SCLK);

    // rfid.init();  
    
    // printf("RFID Reader initialized successfully\n");

    // while (true) {
    //     if (request(&rfid)) {  
    //         printf("Card detected!\n");
 
    //         if (rfid.readCardSerial()) {  
    //             printf("Card UID: ");
    //             for (int i = 0; i < 4; i++) {
    //                 printf("%02X ", rfid.serNum[i]);
    //             }
    //             printf("\n");
    //         } else {
    //             printf("Error reading card UID\n");
    //         }
    //         sleep(2);
    //     } else 
        
    //     {
    //         printf("No card detected.\n");
    //         sleep(1);
    //     }
    // }
// }



// #ifdef __cplusplus
// } // extern "C"
// #endif