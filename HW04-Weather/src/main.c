#include <stdio.h>
#include <stdlib.h>

#include "weather.h"

void print_usage() {
    printf("Usage: weather <location>");
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        print_usage();
        return 0;
    }

    print_weather(argv[1]);

    return 0;
}
