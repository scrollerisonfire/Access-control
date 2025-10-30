#ifndef __STRUCTURES_HPP__
#define __STRUCTURES_HPP__

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <variant>
#include <array>


#define INT(x_)      (static_cast<int>(x_))


typedef std::variant<int, uint32_t, uint64_t, double, std::string*> MultiType;

enum class NvsType {
    kInt,
    kIntU32,
    kIntU64,
    kDouble,
    kString
};



#pragma pack(1)
struct CardData {
    uint8_t     lenUID;
    uint8_t     mUID[10];
    uint8_t     cardBlocks[6][20];
};
#pragma pack(0)

#endif // __STRUCTURES_HPP__