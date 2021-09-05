#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    uint32_t print_res_x;
    uint32_t print_res_y;
    uint32_t colours_used;
    uint32_t important_colours;
} dib_header_t;

typedef struct {
    uint32_t size;
    uint32_t px_array_pos;
    dib_header_t dib_header;
    uint8_t *px_array;
} bmp_t;

#endif // BMP_H