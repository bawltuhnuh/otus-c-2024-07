#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <stdio.h>

void start_server(const char* dir, const char* addr, int port);

void handle_request(int client_fd, const char *root_dir, const char* request);

void send_file(FILE* file, const int fd, const char* path);

void build_response_header(char *buffer, size_t buffer_size, int code, off_t filesize);

#endif // SERVER_H
