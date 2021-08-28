#include "endian.h"

uint16_t reverse_bytes16(uint16_t n) {
    uint8_t *np = (uint8_t*) &n;
    return  ((uint16_t) np[0] << 8)
        |   ((uint16_t) np[1]);
}

uint32_t reverse_bytes32(uint32_t n) {
    uint8_t *np = (uint8_t*) &n;
    return  ((uint32_t) np[0] << 24)
        |   ((uint32_t) np[1] << 16)
        |   ((uint32_t) np[2] << 8)
        |   ((uint32_t) np[3]);
}

uint64_t reverse_bytes64(uint64_t n) {
    uint8_t *np = (uint8_t*) &n;
    return  ((uint32_t) np[0] << 56)
        |   ((uint32_t) np[1] << 48)
        |   ((uint32_t) np[2] << 40)
        |   ((uint32_t) np[3] << 32)
        |   ((uint32_t) np[4] << 24)
        |   ((uint32_t) np[5] << 16)
        |   ((uint32_t) np[6] << 8)
        |   ((uint32_t) np[7]);
}