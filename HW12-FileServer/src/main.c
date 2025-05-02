#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "server.h"

void print_usage() {
    printf("Usage: file_server <path> <adress:port>");
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        print_usage();
        return 1;
    }

    const char *dir = argv[1];
    char *address_and_port = strdup(argv[2]);
    char *address = strtok(address_and_port, ":");
    char *port_str = strtok(NULL, "");
    int port = atoi(port_str);

    printf("%s, %s, %d\n", dir, address, port);

    start_server(dir, address, port);

    return 0;
}
