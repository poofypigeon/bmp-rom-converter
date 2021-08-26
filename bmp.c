#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BMP_HEADER_SIZE 14
#define INFO_HEADER_SIZE 124

#define FILE_SIZE_POS 0x2
#define PX_OFFSET_POS 0xA

typedef struct {
    uint32_t size;
    uint32_t px_array_pos;
} BMPHEADER;

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
} BITMAPINFOHEADER;

typedef struct {
    BMPHEADER bmp_header;
    BITMAPINFOHEADER dib_header;
    uint8_t *px_array;
} BMP;

int little_endian_to_int(const uint8_t *ptr, size_t size) {
    int result = 0;
    for (int byte = 0; byte < size; byte++) {
        result += ptr[byte] << (8 * byte);
    }
    return result;
}

size_t read_bytes(uint8_t *dest, size_t n, FILE *fp) {
    memset(dest, '\0', n);
    return fread(dest, 1, n, fp);
}

BMP read_monochrome_bmp(FILE* file) {
    BMP bmp;

    uint8_t bmp_header_buffer[BMP_HEADER_SIZE];
    if (read_bytes(bmp_header_buffer, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE) {
        printf("File could not be read.\n");
        exit(1);
    };

    if (memcmp(bmp_header_buffer, "BM", 2) != 0) {
        printf("File format not recognised.\n");
        exit(1);
    }

    bmp.bmp_header.size            = little_endian_to_int(&bmp_header_buffer[FILE_SIZE_POS], sizeof(int));
    bmp.bmp_header.px_array_pos = little_endian_to_int(&bmp_header_buffer[PX_OFFSET_POS], sizeof(int));

    uint8_t info_header_buffer[INFO_HEADER_SIZE];

    if (read_bytes(info_header_buffer, 4, file) != 4) {
        printf("BITMAPINFOHEADER size could not be read.\n");
        exit(1);
    };
    bmp.dib_header.size = little_endian_to_int(info_header_buffer, sizeof(uint32_t));

    if (bmp.dib_header.size != 40) {
        printf("Only .bmp's with the BITMAPINFOHEADER format are supported.\n");
        exit(1);
    }

    if (read_bytes(&info_header_buffer[4], bmp.dib_header.size - 4, file) != bmp.dib_header.size - 4) {
        printf("BITMAPINFOHEADER could not be read.\n");
        exit(1);
    }

    bmp.dib_header.width             = little_endian_to_int(&info_header_buffer[0x04], sizeof(uint32_t));
    bmp.dib_header.height            = little_endian_to_int(&info_header_buffer[0x08], sizeof(uint32_t));
    bmp.dib_header.planes            = little_endian_to_int(&info_header_buffer[0x0C], sizeof(uint16_t));
    bmp.dib_header.bit_count         = little_endian_to_int(&info_header_buffer[0x0E], sizeof(uint16_t));
    bmp.dib_header.compression       = little_endian_to_int(&info_header_buffer[0x10], sizeof(uint32_t));
    bmp.dib_header.image_size        = little_endian_to_int(&info_header_buffer[0x14], sizeof(uint32_t));
    bmp.dib_header.print_res_x       = little_endian_to_int(&info_header_buffer[0x18], sizeof(uint32_t));
    bmp.dib_header.print_res_y       = little_endian_to_int(&info_header_buffer[0x1C], sizeof(uint32_t));
    bmp.dib_header.colours_used      = little_endian_to_int(&info_header_buffer[0x20], sizeof(uint32_t));
    bmp.dib_header.important_colours = little_endian_to_int(&info_header_buffer[0x24], sizeof(uint32_t));

    if (bmp.dib_header.bit_count != 1) {
        printf("Only .bmp's with a bitdepth of 1 are supported.\n");
        exit(1);
    }

    if (fseek(file, bmp.bmp_header.px_array_pos, SEEK_SET) != 0)
        { perror("Unable to locate image data"); }
    
    bmp.px_array = malloc(bmp.dib_header.image_size);
    if (read_bytes(bmp.px_array, bmp.dib_header.image_size, file) != bmp.dib_header.image_size) {
        printf("Image data could not be read.\n");
        exit(1);
    };

    return bmp;
}

BMP monochrome_raw_to_bmp (uint32_t img_width, FILE* raw_file) {
    fseek(raw_file, 0L, SEEK_END);
    uint32_t file_size = ftell(raw_file);
    rewind(raw_file);

    uint32_t total_px = file_size * 8;
    if (total_px % img_width != 0) {
        printf("%dpx is not a valid width for this image.\n", img_width);
        exit(1);
    }

    uint32_t img_height = total_px / img_width;
    uint32_t row_size = ((img_width + 31) / 32) * 4;

    int px_array_size = img_height * row_size;

    uint8_t *px_array = malloc(px_array_size);
    memset(px_array, 0, px_array_size);

    uint8_t curr_byte = getc(raw_file);

    int in_bit = 0;
    for (int row = 0; row < img_height; row++) {
        int row_byte = 0;

        for (int row_bit = 0; row_bit < img_width; row_bit++) {
            if (row_bit > 0 && row_bit % 8 == 0) row_byte++;
            if (in_bit == 8) {
                curr_byte = getc(raw_file);
                in_bit = 0;
            }
            px_array[row_byte + row * row_size] |= (curr_byte & (0x80 >> in_bit)) << in_bit >> (row_bit % 8);
            in_bit++;
        }
    }

    BMP bmp;
    bmp.bmp_header.px_array_pos = 14 + sizeof(BITMAPINFOHEADER);
    bmp.bmp_header.size = bmp.bmp_header.px_array_pos + (px_array_size);
    bmp.dib_header.width = img_width;
    bmp.dib_header.height = img_height;
    bmp.dib_header.planes = 1;
    bmp.dib_header.bit_count = 1;
    bmp.dib_header.compression = 0;
    bmp.dib_header.image_size = px_array_size;
    bmp.dib_header.print_res_x = 0x0B13;
    bmp.dib_header.print_res_y = 0x0B13;
    bmp.dib_header.colours_used = 0;
    bmp.dib_header.important_colours = 0;

    return bmp;
}

int main() {
    FILE *file = fopen("myfile.dat", "rb");
    monochrome_raw_to_bmp(7, file);
    // BMP bmp = load_monochrome_bmp(file);

    return 0;
}