#ifndef TOOLS
#define TOOLS

#include "tools.hpp"
#include "string.h"

#define VOLTAGE_MAX 2047
#define VOLTAGE_MIN 1779


// Converts a byte to a hexadecimal string representation
std::string Tools::byteToHex(uint8_t inp) {
    char hi, lo;
    uint8_t b_hi = inp >> 4, b_lo = inp & 0x0F;
    hi = (b_hi > 9) ? (char)(0x41 + b_hi - 10) : (char)(0x30 + b_hi);
    lo = (b_lo > 9) ? (char)(0x41 + b_lo - 10) : (char)(0x30 + b_lo);
    return std::string("") + hi + lo;
}

// Converts a hexadecimal character to its numeric value
uint8_t Tools::hex2bin(char ch) {
    if (ch > 0x60) ch -= 0x20;
    return (ch < 0x41) ? ch - 0x30 : ch - 0x37;
}

// Converts a decimal character to its numeric value
uint8_t Tools::dec2bin(char ch) {
    uint8_t ret = ch - 0x30;
    return (/*ret < 0 ||*/ ret > 9) ? 0 : ret;
}

// Converts a numeric string to a 64-bit unsigned integer
uint64_t Tools::StringTo64(std::string str) {
    uint64_t num = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if (str.at(i) >= '0' && str.at(i) <= '9') {
            num = num * 10 + dec2bin(str.at(i));
        }
    }
    return num;
}

// Prints a byte array in hexadecimal format
void Tools::printHex(uint8_t *buffer, uint8_t bufferSize) {
    for (uint8_t i = 0; i < bufferSize; i++) {
        printf(" %02X", buffer[i]);
    }
}

// Prints a byte array in decimal format
void Tools::printDec(uint8_t *buffer, uint8_t bufferSize) {
    for (uint8_t i = 0; i < bufferSize; i++) {
        printf(" %02d", buffer[i]);
    }
}

// Converts a Binary-Coded Decimal (BCD) value to a binary value
uint8_t Tools::bcd2bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Prints the contents of a byte array in hexadecimal format with a newline
void Tools::dump_byte_array(uint8_t *buffer, uint16_t bufferSize) {
    for (uint16_t i = 0; i < bufferSize; i++) {
        printf(" %02X", buffer[i]);
    }
    printf("\n");
}

// Converts a hexadecimal string to a byte array
void Tools::convertHexStringToByteArray(uint8_t *buffer, uint16_t len, std::string str) {
    if((len % 2) != 0)
        return;

    for (uint16_t i = 0; i < len/2; i++) {
        buffer[i] = hex2bin(str.at(i * 2)) * 16 + hex2bin(str.at(i * 2 + 1));
    }
}

// Calculates the checksum (CKS) of a hexadecimal command string
uint16_t Tools::calculateCKS(std::string cmd) {
    uint16_t ret = 0;
    for (int i = 0; i < cmd.length(); i += 2) {
        ret += hex2bin(cmd.at(i)) * 16 + hex2bin(cmd.at(i + 1));
    }
    return ret;
}

// Calculates the battery percentage based on Analog to Digital Converter value
uint16_t Tools::calc_battery_percentage(uint16_t adc) {
    int battery = 100 * (adc - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN);
    return (battery < 0) ? 0 : ( ((battery > 100) ? 100 : battery) );
}

// Parses a string into an array of substrings separated by a specified character
uint8_t Tools::parseMessage(std::string *array, int array_count, std::string payload, char separator) {
    uint32_t p_size = payload.length() + 1;
    const char *input = payload.c_str();
    uint8_t j = 0;
    for (uint32_t ii = 0; ii < p_size; ii++) {
        if (input[ii] == separator) {
            j++;
            if (j >= array_count-1) break;
            array[j] = "";
        } else {
            array[j] += input[ii];
        }
    }
    return j;
}

// Compares two byte arrays for equality
bool Tools::compareByteArrays(size_t size, uint8_t *card_code, char *pass) {
    if(memcmp(card_code, pass, size) == 0)
        return true;

    return false;
}

// Checks if a time representation in a byte array is valid
bool Tools::isTimeValid(uint8_t *buffer) {
    // BCD encoding
    return !(buffer[0] < 0x20 || buffer[0] > 0x30 || buffer[1] > 0x99 || buffer[2] > 0x12 ||
             buffer[3] > 0x31 || buffer[4] > 0x24 || buffer[5] > 0x59);
}

// Splits a string into an array of substrings separated by ':'
uint8_t Tools::splitString(std::string *array, int array_count, std::string payload) {
    return parseMessage(array, array_count, payload, ':');
}

#endif
