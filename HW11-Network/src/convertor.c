#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "convertor.h"

#define TELEHACK "telehack.com"
#define TELEHACK_PORT "23"
#define BUF_SIZE 2048

void read_msg(int sock_fd, int print) {
    char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int n, idx;
    while (1) {
        n = read(sock_fd, buffer, BUF_SIZE);
        idx = strlen(buffer) - 1;
        if (print) {
            printf("%s", buffer);
        }
        if (buffer[idx] == '.') {
            break;
        }
    }
    if (print) {
        printf("\n");
    }
}

void fetch(int sock_fd, const char* font_name, const char* text) {
    read_msg(sock_fd, 0);
    char msg[BUF_SIZE];
    snprintf(msg, BUF_SIZE, "figlet /%s %s\r\n", font_name, text);
    write(sock_fd, msg, strlen(msg));
    read_msg(sock_fd, 1);
}

void print_message(const char* font, const char* message) {
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(TELEHACK, TELEHACK_PORT, &hints, &res);

    if (status < 0) {
        printf("Failed to resolve name %s\n", TELEHACK);
        return;
    }

    int sock_fd;
    for (p = res; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd > 0 && connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(sock_fd);
    }

    if (p == NULL || sock_fd == -1) {
        printf("Failed to connect to %s\n", TELEHACK);
        freeaddrinfo(res);
        return;
    }
    fetch(sock_fd, font, message);

    close(sock_fd);
    freeaddrinfo(res);

}
