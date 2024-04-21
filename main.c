#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "helpers.h"

enum ENDIAN{
    LITTLE,
    BIG,
};
enum TYPE{
    FOLDER,
    SINGLE,
};



bool getendianness(int argc, char *argv[], char *files[1024], int *filelen, char **output) {
    int opt;
    bool endian = false; // 0 is little endian
    bool set = false;
    bool setout = false;
    int newarg = 0;
    while ((opt = getopt(argc, argv, ":lbho:")) != -1){
        switch (opt) {
            case 'l':
                if (set){
                    if (endian){
                        fprintf(stderr, "Cannot use -l and -b.\n");
                        exit(1);
                    }
                    fprintf(stderr, "Cannot use -l multiple times.\n");
                    exit(1);
                }
                set = true;
                break;
            case 'b':
                if (set){
                    if (!endian){
                        fprintf(stderr, "Cannot use -l and -b.\n");
                        exit(1);
                    }
                    fprintf(stderr, "Cannot use -b multiple times.\n");
                    exit(1);
                }
                endian = true; // Big endian
                set = true;
                break;
            case 'h':
                fprintf(stdout, "Usage: %s [-h] -o output [-b|-l] file [files...]\n", argv[0]);
                exit(0);
            case 'o':
                if (setout){
                    fprintf(stderr, "Cannot have multiple outputs\n");
                    exit(1);
                }
                *output = optarg;
                setout = true;
                break;
            case '?':
            default:
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                exit(1);
        }
    }
    if (!setout){
        fprintf(stderr, "Must have an output (-o)\n");
        exit(1);
    }
    // printarray(argv,argc);
    for (; optind < argc && newarg < 1025; optind++,newarg++) {
        // printf("CurrIdx - %i: %s",optind,argv[optind]);
        files[newarg] = argv[optind];
        // printf("Non-option argument: %s\n", argv[optind]);
    }
    *filelen = newarg;
    // printf("Endianness is %s\n", endian?"big":"little");
    return endian;
}

bool worthcompressing(void*data, size_t datalen, size_t* putsize){
    unsigned char prevbit = 0;
    int max_num = (int)pow(2,MAX_BITS)-1;
    int newcount = 0;
    int accuratecount;
    bool worth;
    int bitrow = -1;
    // fwrite(data, 1, datalen+1, stdout);
    // Calculate the new size
    unsigned char *byte_data = (unsigned char *)data;

    for (size_t i = 0; i < datalen; i++) {
        unsigned char byte = byte_data[i];
        printf("Byte %zu: %02X -- ", i+1, byte);
        for (unsigned char j = 0;j < 8;j++){
            unsigned char bit = CHECK_BIT(byte, j)?1:0;
            printf("%s",bit?"1":"0");
            if (bitrow == -1){
                prevbit = bit;
                bitrow = 1;
                continue;
            }
            if (bitrow == max_num){
                bitrow = 0;
                newcount++;
                continue;
            }else{
                if (bit != prevbit){
                    bitrow = 1;
                    prevbit = bit;
                    newcount++;
                    continue;
                }
                bitrow++;
                continue;
            }
        }
        printf("\n");

    }
    accuratecount = (int)ceilf((((float)newcount)/2)+1);
    worth = accuratecount < datalen;
    printf("\n\nNew Size: %i\nOld Size: %i\nWorth it: %s\n", accuratecount, (int)datalen, worth?"Yes":"No");
    *putsize = accuratecount;
    return worth;
}

void write_to_file(unsigned char* data, size_t datalen, const char* filename) {
    FILE *file = fopen(filename, "wb"); // Open the file in binary write mode
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    size_t bytes_written = fwrite(data, sizeof(unsigned char), datalen, file);
    if (bytes_written != datalen) {
        fprintf(stderr, "Error writing to file\n");
    } else {
        printf("Data successfully written to file\n");
    }

    fclose(file); // Close the file
}

void compress(void* data, size_t datalen, char* output, bool bigendian){
    size_t size;
    if (!worthcompressing(data, datalen, &size)) return;
    unsigned char *compdata = (unsigned char *)malloc(size);
    unsigned char prevbit = 0;
    size_t newidx = 1;
    int max_num = (int)pow(2,MAX_BITS)-1;
    unsigned int bitrow;
    bool startout = true;
    bool first = true;
    compdata[0] = 0b00000000; // Config Data
    printf("Output to: %s\n", output);
    // fwrite(data, 1, datalen+1, stdout);
    // Calculate the new size
    unsigned char *byte_data = (unsigned char *)data;

    for (size_t i = 0; i < datalen; i++) {
        unsigned char byte = byte_data[i];
        printf("Byte %zu: %02X -- ", i+1, byte);
        for (unsigned char j = 0;j < 8;j++){
            unsigned char bit = CHECK_BIT(byte, j)?1:0;
            printf("%s",bit?"1":"0");
            if (startout){
                prevbit = bit;
                bitrow = 1;
                startout = false;
                continue;
            }
            if (bitrow == max_num){
                if (!first){
                    first = true;
                    compdata[newidx] = compdata[newidx] | 0b1110 | prevbit;
                    newidx++;
                    bitrow = 1;
                    prevbit = bit;
                    continue;
                }
                compdata[newidx] = (0b1110 | prevbit) << 4;
                first = false;
                bitrow = 1;
                prevbit = bit;
                continue;
            }else{
                if (bit != prevbit){
                    if (!first){
                        first = true;
                        compdata[newidx] = compdata[newidx] | (bitrow << 1) | prevbit;
                        newidx++;
                        prevbit = bit;
                        bitrow = 1;
                        continue;
                    }
                    compdata[newidx] = ((bitrow << 1) | prevbit) << 4;
                    prevbit = bit;
                    first = false;
                    bitrow = 1;
                    continue;
                }
                bitrow++;
                continue;
            }
        }
        printf("\n");

    }
    if (!first){
        compdata[newidx] &= 0b11110000;
        compdata[0] |= 0b00000001; // Add padding to the config
    }

    write_to_file(compdata,datalen, output);
    free(compdata);
}


void compressone(char* file, char* output, bool bigendian){
    char* buffer;
    size_t bytes_read;
    void* toret;
    FILE* thefile = fopen(file, "rb");
    if (thefile == NULL) exit(2);
    if (fseek(thefile, 0, SEEK_END)) exit(3);
    size_t len;
    if ((len = ftell(thefile)) == -1) exit(4);
    rewind(thefile);
    printf("Length of file: %zu\n", len);

    buffer = (char *)malloc(len * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(thefile);
        exit(5);
    }
    bytes_read = fread(buffer, 1, len, thefile);
    if (bytes_read != len) {
        perror("Error reading file");
        fclose(thefile);
        free(buffer);
        exit(6);
    }
    fclose(thefile);

    compress(buffer, len, output, bigendian);

    free(buffer);

}


int main(int argc, char *argv[]) {
    char *files[1024];
    int filelen;
    char* output;
    bool endian = getendianness(argc, argv, files, &filelen, &output);
    printarray(files, filelen);
    if (filelen == 1){
        compressone(files[0], output, endian);
    }

}
