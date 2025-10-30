#ifndef __ACCESS_CONTROL_HPP__
#define __ACCESS_CONTROL_HPP__

#include <cstdint>
#include <string>
#include <vector>

class AccessControl {

private:
    AccessControl();
    virtual ~AccessControl() {};
public:
    static AccessControl& getInstance() 
    {    
        if(_instance == nullptr)
            _instance = new AccessControl();
        return *_instance;
    };



public:
    // TODO: Refactor constructor!
    AccessControl(const std::string& aesKey, uint16_t hotelCode);

public:
    bool setup(void);
    void loop(void);


    uint64_t checkOfflineCardAccess();
    bool compareTime();
    bool validateAccessForSpecificRoom(uint16_t roomNumber);

    void qrLoop();

private:
    bool _bInit;
    static AccessControl *_instance;


    void decryptBlock(const uint8_t* encryptedBlock, uint8_t* decryptedBlock);
    uint64_t parseGroupId(const uint8_t* block);
    uint32_t parseUserId(const uint8_t* block);
    uint16_t parseHotelId(const uint8_t* block);
    uint16_t parseRoomNumber(const uint8_t* block);
    bool validateCardUID(const uint8_t* cardUID, const uint8_t* block);
    bool validateGroup(const uint8_t* block);

    std::string aesKey;
    uint16_t hotelCode;
    uint8_t nuidPICC[4];
    uint8_t cardBlock1[16], cardBlock2[16], cardBlock4[16], cardBlock5[16], cardBlock6[16];
    uint32_t timeMeasure;
    std::string errorLate;

    uint8_t card_block1[16], card_block2[16], card_block4[16], card_block5[16], card_block6[16];
};

#endif // __ACCESS_CONTROL_HPP__
