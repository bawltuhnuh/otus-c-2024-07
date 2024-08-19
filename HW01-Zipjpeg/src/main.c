#include <stdio.h>

#include "zip.h"


void print_usage() {
    printf("Usage: zipjpeg <path>");
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        print_usage();
    }

    list_zip(argv[1]);

    return 0;
}
