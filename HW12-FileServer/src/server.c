#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <limits.h>
#include <stdlib.h>

#include "server.h"

#define MAX_EVENTS 10
#define BUFFER_SIZE 8192

typedef struct ConnectionInfo {
    int fd;
    _Bool reading_state;     // true, если ожидается продолжение данных
    char buffer[BUFFER_SIZE];  // Буфер для сохранения полученного запроса
    size_t current_length;      // Текущий размер сохранённых данных
} ConnectionInfo;

void start_server(const char* dir, const char* addr, int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd == -1) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("Ошибка настройки опции SO_REUSEADDR");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &(server_addr.sin_addr));
    server_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(listenfd);
        perror("Ошибка привязки сокета");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, SOMAXCONN) == -1) {
        close(listenfd);
        perror("Ошибка установки режима прослушивания");
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен и слушает %s:%d.\n", addr, port);

    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        close(listenfd);
        perror("Ошибка создания EPOLL экземпляра");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listenfd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) == -1) {
        close(listenfd);
        close(epollfd);
        perror("Ошибка добавления сокета в EPOLL");
        exit(EXIT_FAILURE);
    }

    ConnectionInfo connections[MAX_EVENTS];

    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
                fcntl(connfd, F_SETFL, O_NONBLOCK);

                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;

                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);

                ConnectionInfo new_conn = { .fd = connfd, .reading_state = 1 };
                connections[connfd] = new_conn;
            } else {
                int connfd = events[i].data.fd;
                ConnectionInfo *info = &connections[connfd];

                if (info->reading_state) {
                    char buffer[BUFFER_SIZE];
                    ssize_t bytes_read = recv(connfd, buffer, BUFFER_SIZE - 1, MSG_DONTWAIT);

                    if (bytes_read == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        } else {
                            perror("Ошибка при получении данных");
                            close(connfd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, NULL);
                            continue;
                        }
                    } else if (bytes_read == 0) {
                        close(connfd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, NULL);
                        continue;
                    }

                    strncat(info->buffer, buffer, bytes_read);
                    info->current_length += bytes_read;
                    info->buffer[info->current_length] = '\0';

                    if (strstr(info->buffer, "\r\n\r\n")) {
                        handle_request(connfd, dir, info->buffer);
                        info->reading_state = 0;
                    }
                }
            }
        }
    }
}

void handle_request(int client_fd, const char *root_dir, const char* request) {
    printf("Handling request\n");
    char method[16], uri[256];
    sscanf(request, "%s %s ", method, uri);

    if (strcmp(method, "GET") != 0) {
        write(client_fd, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 76);
        close(client_fd);
        return;
    }

    char full_path[PATH_MAX + 1];
    snprintf(full_path, PATH_MAX, "%s%s", root_dir, uri);

    struct stat st;
    if (stat(full_path, &st) == -1) {
        build_response_header(request, BUFFER_SIZE, 404, 0);
        strcat(request, "\r\nFile not found.");
        write(client_fd, request, strlen(request));
        close(client_fd);
        return;
    }

    FILE *file = fopen(full_path, "rb");

    if (!(S_ISREG(st.st_mode)) || !file) {
        build_response_header(request, BUFFER_SIZE, 403, 0);
        strcat(request, "\r\nAccess denied.");
        write(client_fd, request, strlen(request));
        close(client_fd);
        return;
    }

    send_file(file, client_fd, full_path);
    close(client_fd);
}

void send_file(FILE* file, const int fd, const char* path) {

    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    struct stat st;
    fstat(fileno(file), &st);

    char header_buf[BUFFER_SIZE];
    build_response_header(header_buf, BUFFER_SIZE, 200, st.st_size);
    write(fd, header_buf, strlen(header_buf));

    char buf[BUFFER_SIZE];
    size_t nread;
    while ((nread = fread(buf, 1, sizeof(buf), file)) > 0) {
        write(fd, buf, nread);
    }

    fclose(file);
}

void build_response_header(char *buffer, size_t buffer_size, int code, off_t filesize) {
    switch(code) {
        case 200:
            sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n",
                   filesize);
            break;
        case 404:
            strcpy(buffer, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            break;
        case 403:
            strcpy(buffer, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            break;
        default:
            strcpy(buffer, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
    }
}
