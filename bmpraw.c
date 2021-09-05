#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <argp.h>

#include "endian.h"

#define BMP_HEADER_SIZE 14
#define DIB_HEADER_SIZE 40

#define ptr_letoh16(n) letoh16(*(uint16_t*) &n)
#define ptr_letoh32(n) letoh32(*(uint32_t*) &n)

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

bmp_t read_monochrome_bmp(FILE* file) {
    bmp_t bmp;

    uint8_t bmp_header_buffer[BMP_HEADER_SIZE];
    if (fread(bmp_header_buffer, 1, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE) {
        printf("File could not be read.\n");
        exit(1);
    };

    if (memcmp(bmp_header_buffer, "BM", 2) != 0) {
        printf("File format not recognised.\n");
        exit(1);
    }
    
    bmp.size         = ptr_letoh32(bmp_header_buffer[0x02]);
    bmp.px_array_pos = ptr_letoh32(bmp_header_buffer[0x0A]);

    uint8_t info_header_buffer[DIB_HEADER_SIZE];

    if (fread(info_header_buffer, 1, 4, file) != 4) {
        printf("dib_header_t size could not be read.\n");
        exit(1);
    };
    bmp.dib_header.size = ptr_letoh32(info_header_buffer[0x00]);

    if (bmp.dib_header.size != 40) {
        printf("Only .bmp's with the dib_header_t format are supported.\n");
        exit(1);
    }

    if (fread(&info_header_buffer[0x04], 1, bmp.dib_header.size - 4, file) != bmp.dib_header.size - 0x04) {
        printf("dib_header_t could not be read.\n");
        exit(1);
    }

    bmp.dib_header.width             = ptr_letoh32(info_header_buffer[0x04]);
    bmp.dib_header.height            = ptr_letoh32(info_header_buffer[0x08]);
    bmp.dib_header.planes            = ptr_letoh16(info_header_buffer[0x0C]);
    bmp.dib_header.bit_count         = ptr_letoh16(info_header_buffer[0x0E]);
    bmp.dib_header.compression       = ptr_letoh32(info_header_buffer[0x10]);
    bmp.dib_header.image_size        = ptr_letoh32(info_header_buffer[0x14]);
    bmp.dib_header.print_res_x       = ptr_letoh32(info_header_buffer[0x18]);
    bmp.dib_header.print_res_y       = ptr_letoh32(info_header_buffer[0x1C]);
    bmp.dib_header.colours_used      = ptr_letoh32(info_header_buffer[0x20]);
    bmp.dib_header.important_colours = ptr_letoh32(info_header_buffer[0x24]);

    if (bmp.dib_header.bit_count != 1) {
        printf("Only .bmp's with a bitdepth of 1 are supported.\n");
        exit(1);
    }

    if (fseek(file, bmp.px_array_pos, SEEK_SET) != 0)
        { perror("Unable to locate image data"); }
    
    bmp.px_array = malloc(bmp.dib_header.image_size);
    
    if (fread(bmp.px_array, 1, bmp.dib_header.image_size, file) != bmp.dib_header.image_size) {
        printf("Image data could not be read.\n");
        exit(1);
    };

    return bmp;
}

bmp_t monochrome_raw_to_bmp (uint32_t img_width, FILE* infile) {
    // Get the full size of the input file
    fseek(infile, 0L, SEEK_END);
    size_t file_size = ftell(infile);
    rewind(infile);

    // Ensure that the full pixel count is divisable by the specified image width
    if ((file_size * 8) % img_width != 0) {
        printf("%dpx is not a valid width for this image.\n", img_width);
        exit(1);
    }

    uint32_t img_height = (file_size * 8) / img_width;
    size_t row_size = ((img_width + 31) / 32) * 4; // equation for row width with padding in bytes

    size_t px_array_size = img_height * row_size;

    uint8_t *px_array = malloc(px_array_size);
    memset(px_array, 0, px_array_size);

    uint8_t curr_byte = getc(infile);

    // Construct pixel array including padding
    uint8_t raw_bit = 0;
    for (int row = img_height - 1; row >= 0 ; row--) {
        int row_byte = 0;
        for (int row_pos = 0; row_pos < img_width; row_pos++) {
            if (row_pos > 0 && row_pos % 8 == 0) row_byte++;
            if (raw_bit == 8) {
                curr_byte = getc(infile);
                raw_bit = 0;
            }
            px_array[row_byte + row * row_size] |= (curr_byte & (0x80 >> raw_bit)) << raw_bit >> (row_pos % 8);
            raw_bit++;
        }
    }

    bmp_t bmp;
    bmp.px_array_pos = 14 + sizeof(dib_header_t) + 8;
    bmp.size = bmp.px_array_pos + (px_array_size);

    bmp.dib_header.size              = DIB_HEADER_SIZE;
    bmp.dib_header.width             = img_width;
    bmp.dib_header.height            = img_height;
    bmp.dib_header.planes            = 0x01;
    bmp.dib_header.bit_count         = 0x01;
    bmp.dib_header.compression       = 0x00;
    bmp.dib_header.image_size        = px_array_size;
    bmp.dib_header.print_res_x       = 0x0B13; // 72dpi
    bmp.dib_header.print_res_y       = 0x0B13; // 72dpi
    bmp.dib_header.colours_used      = 0x00;
    bmp.dib_header.important_colours = 0x00;

    bmp.px_array = px_array;

    return bmp;
}

dib_header_t le_dib_header(dib_header_t header) {
    header.size              = htole32(header.size);
    header.width             = htole32(header.width);
    header.height            = htole32(header.height);
    header.planes            = htole16(header.planes);
    header.bit_count         = htole16(header.bit_count);
    header.compression       = htole16(header.compression);
    header.image_size        = htole32(header.image_size);
    header.print_res_x       = htole32(header.print_res_x);
    header.print_res_y       = htole32(header.print_res_y);
    header.colours_used      = htole32(header.colours_used);
    header.important_colours = htole32(header.important_colours);
    return header;
}

int write_bmp(bmp_t bmp, FILE* outfile) {
    if (fwrite("BM", 1, 2, outfile) != 2) 
        { return 1; }
    if (fwrite(&bmp.size, 1, sizeof(uint32_t), outfile) != sizeof(uint32_t)) 
        { return 1; }
    if (fwrite("\0\0\0\0", 1, 4, outfile) != 4) // Application specific empty bytes
        { return 1; }
    if (fwrite(&bmp.px_array_pos, 1, sizeof(uint32_t), outfile) != sizeof(uint32_t))
        { return 1; }
    dib_header_t dib_header_little_endian = le_dib_header(bmp.dib_header);
    if (fwrite(&dib_header_little_endian, 1, sizeof(dib_header_t), outfile) != sizeof(dib_header_t))
        { return 1; }
    if (fwrite("\xFF\xFF\xFF\0\0\0\0\0", 1, 8, outfile) != 8) // Weird nescessary bit masks
        { return 1; }
    if (fwrite(bmp.px_array, 1, bmp.dib_header.image_size, outfile) != bmp.dib_header.image_size)
        { return 1; }
    return 0;
}

size_t bmp_to_monochrome_raw (uint8_t **dest, bmp_t bmp) {
    int total_bits = (bmp.dib_header.height * bmp.dib_header.width);
    size_t size = (total_bits % 8 == 0) ? total_bits / 8 : total_bits / 8 + 1;

    *dest = malloc(size);

    uint32_t dest_byte = 0;  // index of dest_byte pos
    uint8_t dest_bit  = 0;   // current bit offset of next byte

    int row_size = bmp.dib_header.image_size / bmp.dib_header.height;

    for (int row = bmp.dib_header.height - 1; row >= 0; row--) {
        int row_byte = 0;
        for (int row_pos = 0; row_pos < bmp.dib_header.width; row_pos++) {
            if (row_pos > 0 && row_pos % 8 == 0) row_byte++;
            int bmp_bit = row_pos % 8;
            // retreive bit at bmp_bit location from the bmp pixel array, then shift it to dest_bit location
            (*dest)[dest_byte] |= (bmp.px_array[row_byte + row * row_size] & (0x80 >> bmp_bit)) << bmp_bit >> dest_bit;
            dest_bit++;
            if (row_pos == 0) {
                printf("%d\n", dest_byte);
            }
            if (dest_bit == 8) {
                dest_byte++;
                dest_bit = 0;
            }
        }
    }

    printf("%d\n", dest_byte);

    return size;
}

int main() {
    FILE *infile = fopen("img.bmp", "rb");
    bmp_t bmp = read_monochrome_bmp(infile);
    fclose(infile);

    FILE *outfile = fopen("out.dat", "wb");
    // write_bmp(bmp, outfile);
    uint8_t *data;
    size_t outfile_size = bmp_to_monochrome_raw(&data, bmp);
    fwrite(data, 1, outfile_size, outfile);
    
    free(data);
    free(bmp.px_array);
    fclose(outfile);

    return 0;
}