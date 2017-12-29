#include "rcGlobal.h"

void RCGlobal::rc_cpy(void* destination, const void* start, uint8_t size) {
    uint8_t* finalArray = reinterpret_cast<uint8_t*>(destination);
    const uint8_t* startArray = reinterpret_cast<const uint8_t*>(start);

    for(uint8_t i=0; i<size; i++) {
        finalArray[i] = startArray[i];
    }
}