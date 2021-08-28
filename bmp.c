#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <machine/endian.h>

#define BMP_HEADER_SIZE 14
#define MAX_DIB_HEADER_SIZE 124

typedef struct {
    uint32_t size;
    uint32_t px_array_pos;
} bmp_header_t;

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
    bmp_header_t
 bmp_header;
    dib_header_t dib_header;
    uint8_t *px_array;
} BMP;

BMP read_monochrome_bmp(FILE* file) {
    BMP bmp;

    uint8_t bmp_header_buffer[BMP_HEADER_SIZE];
    if (fread(bmp_header_buffer, 1, BMP_HEADER_SIZE, file) != 0) {
        printf("File could not be read.\n");
        exit(1);
    };

    if (memcmp(bmp_header_buffer, "BM", 2) != 0) {
        printf("File format not recognised.\n");
        exit(1);
    }
    
    bmp.bmp_header.size         = *(uint32_t*) &bmp_header_buffer[0x2];
    bmp.bmp_header.px_array_pos = *(uint32_t*) &bmp_header_buffer[0xA];

    uint8_t info_header_buffer[MAX_DIB_HEADER_SIZE];

    if (fread(info_header_buffer, 1, 4, file) != 4) {
        printf("dib_header_t size could not be read.\n");
        exit(1);
    };
    bmp.dib_header.size = *(uint32_t*) info_header_buffer;

    if (bmp.dib_header.size != 40) {
        printf("Only .bmp's with the dib_header_t format are supported.\n");
        exit(1);
    }

    if (fread(&info_header_buffer[4], 1, bmp.dib_header.size - 4, file) != bmp.dib_header.size - 4) {
        printf("dib_header_t could not be read.\n");
        exit(1);
    }

    bmp.dib_header.width             = *(uint32_t*) &info_header_buffer[0x04];
    bmp.dib_header.height            = *(uint32_t*) &info_header_buffer[0x08];
    bmp.dib_header.planes            = *(uint16_t*) &info_header_buffer[0x0C];
    bmp.dib_header.bit_count         = *(uint16_t*) &info_header_buffer[0x0B];
    bmp.dib_header.compression       = *(uint32_t*) &info_header_buffer[0x10];
    bmp.dib_header.image_size        = *(uint32_t*) &info_header_buffer[0x14];
    bmp.dib_header.print_res_x       = *(uint32_t*) &info_header_buffer[0x18];
    bmp.dib_header.print_res_y       = *(uint32_t*) &info_header_buffer[0x1C];
    bmp.dib_header.colours_used      = *(uint32_t*) &info_header_buffer[0x20];
    bmp.dib_header.important_colours = *(uint32_t*) &info_header_buffer[0x24];

    if (bmp.dib_header.bit_count != 1) {
        printf("Only .bmp's with a bitdepth of 1 are supported.\n");
        exit(1);
    }

    if (fseek(file, bmp.bmp_header.px_array_pos, SEEK_SET) != 0)
        { perror("Unable to locate image data"); }
    
    bmp.px_array = malloc(bmp.dib_header.image_size);
    
    if (fread(bmp.px_array, 1, bmp.dib_header.image_size, file) != bmp.dib_header.image_size) {
        printf("Image data could not be read.\n");
        exit(1);
    };

    return bmp;
}

BMP monochrome_raw_to_bmp (uint32_t img_width, FILE* infile) {
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
    int in_bit = 0;
    for (int row = img_height - 1; row >= 0 ; row--) {
        int row_byte = 0;

        for (int row_bit = 0; row_bit < img_width; row_bit++) {
            if (row_bit > 0 && row_bit % 8 == 0) row_byte++;
            if (in_bit == 8) {
                curr_byte = getc(infile);
                in_bit = 0;
            }
            px_array[row_byte + row * row_size] |= (curr_byte & (0x80 >> in_bit)) << in_bit >> (row_bit % 8);
            in_bit++;
        }
    }

    BMP bmp;
    bmp.bmp_header.px_array_pos = 14 + sizeof(dib_header_t) + 8;
    bmp.bmp_header.size = bmp.bmp_header.px_array_pos + (px_array_size);

    bmp.dib_header.size = 0x28;
    bmp.dib_header.width = img_width;
    bmp.dib_header.height = img_height;
    bmp.dib_header.planes = 1;
    bmp.dib_header.bit_count = 1;
    bmp.dib_header.compression = 0;
    bmp.dib_header.image_size = px_array_size;
    bmp.dib_header.print_res_x = 0x0B13; // 72dpi
    bmp.dib_header.print_res_y = 0x0B13; // 72dpi
    bmp.dib_header.colours_used = 0;
    bmp.dib_header.important_colours = 0;

    bmp.px_array = px_array;

    return bmp;
}

void write_bmp(BMP bmp, FILE* outfile) {
    fwrite("BM", 1, 2, outfile);
    fwrite(&bmp.bmp_header.size, 1, sizeof(uint32_t), outfile);
    fwrite("\0\0\0\0", 1, 4, outfile); // Application Specific
    fwrite(&bmp.bmp_header.px_array_pos, 1, sizeof(uint32_t), outfile);

    fwrite(&bmp.dib_header, 1, sizeof(dib_header_t), outfile);
    fwrite("\xFF\xFF\xFF\0\0\0\0\0", 1, 8, outfile); // Weird nescessary bit masks
    fwrite(bmp.px_array, 1, bmp.dib_header.image_size, outfile);
}

int main() {
    FILE *infile = fopen("test.dat", "rb");
    BMP bmp = monochrome_raw_to_bmp(8, infile);

    FILE *outfile = fopen("img.bmp", "wb");
    write_bmp(bmp, outfile);
    // BMP bmp = load_monochrome_bmp(file);

    free(bmp.px_array);

    return 0;
}