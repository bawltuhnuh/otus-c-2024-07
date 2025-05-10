#include <stdio.h>
#include <stdlib.h>

#include "convertor.h"

void print_usage() {
    printf("Usage: font_convertor <font> <message>");
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    print_message(argv[1], argv[2]);

    return 0;
}
