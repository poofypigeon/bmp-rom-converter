#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define betoh16(n) reverse_bytes16(n)
    #define htobe16(n) reverse_bytes16(n)
    #define betoh32(n) reverse_bytes32(n)
    #define htobe32(n) reverse_bytes32(n)
    #define betoh64(n) reverse_bytes64(n)
    #define htobe64(n) reverse_bytes64(n)

    uint16_t letoh16(uint16_t n) { return n; }
    uint16_t htole16(uint16_t n) { return n; }
    uint32_t letoh32(uint32_t n) { return n; }
    uint32_t htole32(uint32_t n) { return n; }
    uint32_t letoh64(uint64_t n) { return n; }
    uint32_t htole64(uint64_t n) { return n; }
#else
    #define letoh16(n) reverse_bytes16(n)
    #define htole16(n) reverse_bytes16(n)
    #define letoh32(n) reverse_bytes32(n)
    #define htole32(n) reverse_bytes32(n)
    #define letoh64(n) reverse_bytes64(n)
    #define htole64(n) reverse_bytes64(n)
    
    uint16_t betoh16(uint16_t n) { return n; }
    uint16_t htobe16(uint16_t n) { return n; }
    uint32_t betoh32(uint32_t n) { return n; }
    uint32_t htobe32(uint32_t n) { return n; }
    uint32_t betoh64(uint64_t n) { return n; }
    uint32_t htobe64(uint64_t n) { return n; }
#endif

uint16_t reverse_bytes16(uint16_t n);
uint32_t reverse_bytes32(uint32_t n);
uint64_t reverse_bytes64(uint64_t n);

#endif // ENDIAN_H