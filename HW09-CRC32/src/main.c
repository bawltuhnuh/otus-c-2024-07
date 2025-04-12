#include <stdio.h>
#include <stdlib.h>
#include "crc32.h"

void print_usage() {
    printf("Usage: crc <path>");
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        print_usage();
        exit(1);
    }

    uint32_t result = get_crc32(argv[1]);
    if (result == 0) {
        exit(1);
    }
    printf("CRC32: %x\n", result);
    return 0;
}
