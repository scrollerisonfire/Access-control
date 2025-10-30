#ifndef __NVS_RUNTIME_HPP__
#define __NVS_RUNTIME_HPP__

#include "nvs.hpp"

#include <vector>

namespace Rt {

enum class Key {
    kCounterGuest = 0,
    kCounterStaff,
    kCounterMaster,
    kLastClockUpdate,
    
    kMax
};

}   // namespace Rt


Nvs *NVS_InitRuntime();
Nvs *GetRt();
void NVS_factoryReset();


#endif // __NVS_RUNTIME_HPP__