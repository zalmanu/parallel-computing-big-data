#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#define IMG_SIZE 54

#pragma pack(push, 2)
typedef struct {
    char signature;
    int size;
    int reserved;
    int data;
    int header_size;
    int width;
    int height;
    short planes;
    short pixel_bits;
    int compression_method;
    int img_size;
    int x_res;
    int y_res;
    int colors_num;
    int base_color_count;
} img_header;
#pragma pop

unsigned char* read_image(char file_name[], img_header* header) {
    FILE* fin;
    unsigned char* data;

    if(!(fin = fopen(file_name, "rb"))) {
        printf("Cannot open %s", file_name);
        exit(1);
    }

    fread(header, IMG_SIZE, 1, fin);
    data = (char*) malloc(header->img_size);
    fseek(fin, header->data, SEEK_SET);
    fread(data, header->img_size, 1, fin);

    fclose(fin);

    return data;

}

int get_padded_width(img_header* header) {
    int total_width = header->width * 3;

    if (total_width % 4) {
        total_width += (4 - (total_width % 4));
    }

    return total_width;

}
void extract_rgb(img_header* header, unsigned char* data, unsigned char* red, unsigned char* green, unsigned char* blue) {
    int pos = 0;
    int total_width = get_padded_width(header);

    for(int row = 0; row < header->height; row++) {
        for(int col = 0; col < header->width * 3; col += 3, pos++)  {
            red[pos] = data[row * total_width + col];
            green[pos] = data[row * total_width + col + 1];
            blue[pos] = data[row * total_width + col + 2];
        }
    }
}

int apply_bound(int value, int min, int max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }

    return value;
}


void blur(img_header* header, unsigned char* data, unsigned char* red, unsigned char* green, unsigned char* blue, int radius) {
    int pos;
    double sigma = radius * radius;
    double red_total, green_total, blue_total, weight_total;

    for(int row = 0; row < header->height; row++) {
        for(int col = 0; col < header->width; col++) {

            red_total = 0;
            green_total = 0;
            blue_total = 0;
            weight_total = 0;

            for(int kernel_row = row-radius; kernel_row <= row + radius; kernel_row++) {
                for(int kernel_col = col-radius; kernel_col <= col + radius; kernel_col++) {
                    int bounded_row = apply_bound(kernel_row, 0, header->height-1);
                    int bounded_col = apply_bound(kernel_col, 0, header->width-1);

                    pos = bounded_row * header->width + bounded_col;

                    double squared_dist = (kernel_row-row)^2 + (kernel_col-col)^2;
                    double weight = exp(-squared_dist / (2*sigma)) / (M_PI * 2 * sigma);

                    red_total += red[pos] * weight;
                    green_total += green[pos] * weight;
                    blue_total += blue[pos] * weight;
                    weight_total += weight;
                }
            }

            pos = row * header->width + col;
            red[pos] = round(red_total / weight_total);
            green[pos] = round(green_total / weight_total);
            blue[pos] = round(blue_total / weight_total);
        }
    }
}

void build_result(img_header* header, unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* out_data) {
    int pos = 0;
    int total_width = get_padded_width(header);

    for(int row = 0; row < header->height; row++) {
        for(int col = 0; col < header->width*3; col += 3, pos++) {
            out_data[row * total_width + col] = red[pos];
            out_data[row * total_width + col + 1] = green[pos];
            out_data[row * total_width + col + 2] = blue[pos];
        }
    }
}

void write_image(img_header* header, unsigned char* data) {
    FILE* fout;

    fout = fopen("result.bmp", "wb");
    fwrite(header, IMG_SIZE, 1, fout);
    fseek(fout, header->data, SEEK_SET);
    fwrite(data, header->img_size, 1, fout);

    fclose(fout);
}

int main(int argc, char* argv[]){

    char* file_name = argv[1];
    int radius = atoi(argv[2]);

    img_header* header = (img_header*) malloc(IMG_SIZE);
    unsigned char* img_data = read_image(file_name, header);

    unsigned char* red = (unsigned char*) malloc(header->width * header->height);
    unsigned char* green = (unsigned char*) malloc(header->width * header->height);
    unsigned char* blue = (unsigned char*) malloc(header->width * header->height);

    extract_rgb(header, img_data, red, green, blue);
    blur(header, img_data, red, green, blue, radius);
    build_result(header, red, green, blue, img_data);
    write_image(header, img_data);

    free(red);
    free(green);
    free(blue);
    free(header);
    free(img_data);

    return 0;
}
