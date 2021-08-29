#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define betoh16(n) reverse_bytes16__(n)
    #define htobe16(n) reverse_bytes16__(n)
    #define betoh32(n) reverse_bytes32__(n)
    #define htobe32(n) reverse_bytes32__(n)
    #define betoh64(n) reverse_bytes64__(n)
    #define htobe64(n) reverse_bytes64__(n)

    #define letoh16(n) (n)
    #define htole16(n) (n)
    #define letoh32(n) (n)
    #define htole32(n) (n)
    #define letoh64(n) (n)
    #define htole64(n) (n)
#else
    #define letoh16(n) reverse_bytes16__(n)
    #define htole16(n) reverse_bytes16__(n)
    #define letoh32(n) reverse_bytes32__(n)
    #define htole32(n) reverse_bytes32__(n)
    #define letoh64(n) reverse_bytes64__(n)
    #define htole64(n) reverse_bytes64__(n)
    
    #define betoh16(n) (n)
    #define htobe16(n) (n)
    #define betoh32(n) (n)
    #define htobe32(n) (n)
    #define betoh64(n) (n)
    #define htobe64(n) (n)
#endif

uint16_t reverse_bytes16__(uint16_t n);
uint32_t reverse_bytes32__(uint32_t n);
uint64_t reverse_bytes64__(uint64_t n);



#endif // ENDIAN_H