#include "access_control.hpp"

#include <cstring>

#include "esp_log.h"
#include "esp_timer.h" 
#include "esp_system.h"


AccessControl* AccessControl::_instance = nullptr;

AccessControl::AccessControl() {

}


static const char* TAG = "AccessPoint";

AccessControl::AccessControl(const std::string& aesKey, uint16_t hotelCode)
    : aesKey(aesKey), hotelCode(hotelCode) {
    std::memset(nuidPICC, 0, sizeof(nuidPICC));
    std::memset(cardBlock1, 0, sizeof(cardBlock1));
    std::memset(cardBlock2, 0, sizeof(cardBlock2));
    std::memset(cardBlock4, 0, sizeof(cardBlock4));
    std::memset(cardBlock5, 0, sizeof(cardBlock5));
    std::memset(cardBlock6, 0, sizeof(cardBlock6));
}



bool AccessControl::setup(void) {
    _bInit = true;
    return true;
}


void AccessControl::loop(void) {
    if(!_bInit)
        return;
        
}















uint64_t AccessControl::checkOfflineCardAccess() {
    ESP_LOGI(TAG, "Starting offline card access check");

    for (int i = 0; i < 4; i++) {
        nuidPICC[i] = cardBlock1[12 + i]; 
    }
    ESP_LOGI(TAG, "Card UID: %02X %02X %02X %02X", nuidPICC[0], nuidPICC[1], nuidPICC[2], nuidPICC[3]);

    uint8_t decryptedBlock1[16], decryptedBlock2[16], decryptedBlock4[16], decryptedBlock5[16], decryptedBlock6[16];
    decryptBlock(cardBlock1, decryptedBlock1);
    decryptBlock(cardBlock2, decryptedBlock2);
    decryptBlock(cardBlock4, decryptedBlock4);
    decryptBlock(cardBlock5, decryptedBlock5);
    decryptBlock(cardBlock6, decryptedBlock6);

    if (!validateCardUID(nuidPICC, decryptedBlock1)) {
        ESP_LOGE(TAG, "Invalid card UID!");
        return 0;
    }

    uint16_t cardHotel = parseHotelId(decryptedBlock1);
    if (cardHotel != hotelCode) {
        ESP_LOGE(TAG, "Hotel ID mismatch: expected %u, got %u", hotelCode, cardHotel);
        return 0;
    }

    uint64_t groupId = parseGroupId(decryptedBlock5);
    uint32_t userId = parseUserId(decryptedBlock1);

    ESP_LOGI(TAG, "Access granted for user %lu, group %llu", userId, groupId);
    return groupId;
}

bool AccessControl::validateAccessForSpecificRoom(uint16_t roomNumber) {
    uint8_t decryptedBlock1[16];
    decryptBlock(cardBlock1, decryptedBlock1);

    uint16_t cardRoomNumber = parseRoomNumber(decryptedBlock1);
    if (cardRoomNumber == roomNumber) {
        ESP_LOGI(TAG, "Access granted for room %u", roomNumber);
        return true;
    } else {
        ESP_LOGE(TAG, "Access denied for room %u", roomNumber);
        return false;
    }
}

void AccessControl::decryptBlock(const uint8_t* encryptedBlock, uint8_t* decryptedBlock) {
    std::memcpy(decryptedBlock, encryptedBlock, 16);
}

uint64_t AccessControl::parseGroupId(const uint8_t* block) {
    uint64_t groupId = 0;
    for (int i = 0; i < 8; i++) {
        groupId <<= 8;
        groupId |= block[i];
    }
    return groupId;
}

uint32_t AccessControl::parseUserId(const uint8_t* block) {
    uint32_t userId = 0;
    for (int i = 7; i <= 10; i++) {
        userId <<= 8;
        userId |= block[i];
    }
    return userId;
}

uint16_t AccessControl::parseHotelId(const uint8_t* block) {
    uint16_t hotelId = (block[0] << 8) | block[1];
    return hotelId;
}

uint16_t AccessControl::parseRoomNumber(const uint8_t* block) {
    uint16_t roomNumber = (block[2] << 8) | block[3];
    return roomNumber;
}

bool AccessControl::validateCardUID(const uint8_t* cardUID, const uint8_t* block) {
    for (int i = 0; i < 4; i++) {
        if (cardUID[i] != block[12 + i]) {
            return false;
        }
    }
    return true;
}

bool AccessControl::validateGroup(const uint8_t* block) {
    for (int i = 0; i < 8; i++) {
        if (block[i] != block[i + 8]) {
            return false;
        }
    }
    return true;
}

bool AccessControl::compareTime() {
    uint16_t endYear = (cardBlock2[0] >> 4) * 10 + (cardBlock2[0] & 0x0F);
    endYear = endYear * 100 + ((cardBlock2[1] >> 4) * 10 + (cardBlock2[1] & 0x0F));

    timeMeasure = esp_timer_get_time() / 1000; 
    
    // if (time_clock.getYear() > endYear) {
    //     ESP_LOGE(TAG, "Expired year!");
    //     errorLate = "Expired year!";
    //     return false;
    // } else if (time_clock.getYear() < endYear) {
    //     return true;
    // }

    // if (time_clock.getMonth() > ((cardBlock2[2] >> 4) * 10 + (cardBlock2[2] & 0x0F))) {
    //     ESP_LOGE(TAG, "Expired month!");
    //     errorLate = "Expired month!";
    //     return false;
    // } else if (time_clock.getMonth() < ((cardBlock2[2] >> 4) * 10 + (cardBlock2[2] & 0x0F))) {
    //     return true;
    // }

    // if (time_clock.getDay() > ((cardBlock2[3] >> 4) * 10 + (cardBlock2[3] & 0x0F))) {
    //     ESP_LOGE(TAG, "Expired day!");
    //     errorLate = "Expired day!";
    //     return false;
    // } else if (time_clock.getDay() < ((cardBlock2[3] >> 4) * 10 + (cardBlock2[3] & 0x0F))) {
    //     return true;
    // }

    // if (time_clock.getHour() > ((cardBlock2[4] >> 4) * 10 + (cardBlock2[4] & 0x0F))) {
    //     ESP_LOGE(TAG, "Expired hour!");
    //     errorLate = "Expired hour!";
    //     return false;
    // } else if (time_clock.getHour() < ((cardBlock2[4] >> 4) * 10 + (cardBlock2[4] & 0x0F))) {
    //     return true;
    // }

    // if (time_clock.getMinute() > ((cardBlock2[5] >> 4) * 10 + (cardBlock2[5] & 0x0F))) {
    //     ESP_LOGE(TAG, "Expired minutes!");
    //     errorLate = "Expired minutes!";
    //     return false;
    // }

    return true;
}

void AccessControl::qrLoop() {
    timeMeasure = esp_timer_get_time() / 1000; 
    //std::string qr = qrsensor.detection(); 
    int measure = (esp_timer_get_time() / 1000) - timeMeasure;  

    // if (qr == "null" || qr.length() < 20) {
    //     return;  
    // }

    // ESP_LOGI(TAG, "QR Code: %s", qr.c_str());  

    nuidPICC[0] = 1;
    nuidPICC[1] = 1;
    nuidPICC[2] = 1;
    nuidPICC[3] = 1;

    std::memset(cardBlock1, 0, sizeof(cardBlock1));
    std::memset(cardBlock2, 0, sizeof(cardBlock2));
    std::memset(cardBlock4, 0, sizeof(cardBlock4));
    std::memset(cardBlock5, 0, sizeof(cardBlock5));
    std::memset(cardBlock6, 0, sizeof(cardBlock6));

    std::string msg[4] = {"", "", "", ""};
    //int chunks = tools.parseMessage(msg, qr, ',');
    
    // tools.convertHexStringToByteArray(cardBlock1, 16, msg[0]);
    // tools.convertHexStringToByteArray(cardBlock2, 16, msg[1]);
    // tools.convertHexStringToByteArray(cardBlock4, 16, msg[2]);
    // tools.convertHexStringToByteArray(cardBlock5, 16, msg[3]);

    // tools.dump_byte_array(cardBlock1, 16);
    // tools.dump_byte_array(cardBlock2, 16);
    // tools.dump_byte_array(cardBlock4, 16);
    // tools.dump_byte_array(cardBlock5, 16);

}
