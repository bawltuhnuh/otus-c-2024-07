#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#define NAME "size.socket"

int server_fd = 0;

int is_file_exists(const char* file_path) {
    return access(file_path, F_OK) == 0;
}

void rstrip(char *s) {
    size_t size;
    char *end;

    size = strlen(s);

    if (!size) {
        return;
    }

    end = s + size - 1;
    while (end >= s && isspace(*end)) {
        *end = '\0';
        end--;
    }
}

int get_file_size(const char* file_path) {
    struct stat st;
    stat(file_path, &st);
    return st.st_size;
}

void get_file_size_as_string(const char* file_path, char* message) {
    int size = get_file_size(file_path);
    sprintf(message, "Size of %s: %d\n", file_path, size);
}

void get_file_not_exists_message(const char* file_path, char* message) {
    sprintf(message, "File %s not exists\n", file_path);
}

char* get_responce(const char* file_path) {
    char* message = (char*) malloc(1024);
    if (!is_file_exists(file_path)) {
        get_file_not_exists_message(file_path, message);
    } else {
        get_file_size_as_string(file_path, message);
    }
    return message;
}

void create_server() {
    int sock;
    struct sockaddr_un server;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, NAME);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) {
        perror("binding stream socket");
        exit(1);
    }
    printf("Socket has name %s\n", server.sun_path);
    server_fd = sock;
}

void close_server() {
    close(server_fd);
    unlink(NAME);
}

void start_server(const char* file_path) {
    int msgsock, rval;
    char buf[1024];

    rstrip(file_path);

    create_server();

    listen(server_fd, 5);
    for (;;) {
        msgsock = accept(server_fd, 0, 0);
        if (msgsock == -1) {
            perror("accept");
            break;
        } else do {
            bzero(buf, sizeof(buf));
            if ((rval = read(msgsock, buf, 1024)) < 0)
                perror("reading stream message");
            else if (rval == 0)
                printf("Ending connection\n");
            else {
                char* responce = get_responce(file_path);
                send(msgsock, responce, strlen(responce), 0);
                free(responce);
            }
        } while (rval > 0);
        close(msgsock);
    }
    close_server();
}

#endif // SERVER_H
