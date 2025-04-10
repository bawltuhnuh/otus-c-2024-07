#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>

#include "server.h"

#define NO_DAEMON_ARG "--no-daemonize"

#define CONFIG_PATH "/etc/ipc.conf"

#define BUF_SIZE 1024

static void clean_exit(int signo) {
    close_server();
    exit(0);
}

char* get_file_path() {
    FILE* file_ptr = fopen(CONFIG_PATH, "r");
    if (file_ptr == NULL) {
        printf("Failed to open configuration file %s!\n", CONFIG_PATH);
        return NULL;
    }
    char* file_content = (char*)malloc(BUF_SIZE);
    if (file_content == NULL) {
        printf("Failed to malloc!\n");
        return NULL;
    }
    if (fgets(file_content, BUF_SIZE, file_ptr) == NULL){
        printf("Failed to read file!\n");
        fclose(file_ptr);
        free(file_content);
        return NULL;
    }
    fclose(file_ptr);
    return file_content;
}

void print_usage() {
    printf("Usage: ipc [--no-daemonize]");
}

void daemonize()
{
    int                 i, fd0, fd1, fd2;
    pid_t               pid;
    struct rlimit       rl;
    struct sigaction    sa;


    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("невозможно получить максимальный номер дескриптора");
    }

    if ((pid = fork()) < 0) {
        perror("ошибка вызова функции fork");
    } else if (pid != 0) {
        exit(0);
    }

    setsid();

    if ((pid = fork()) < 0) {
        syslog(LOG_CRIT, "ошибка вызова функции fork");
    } else if (pid != 0) {
        exit(0);
    }

    if (chdir("/") < 0) {
        syslog(LOG_CRIT, "невозможно сделать текущим рабочим каталогом /");
    }

    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_CRIT, "ошибочные файловые дескрипторы %d %d %d",
               fd0, fd1, fd2);
    }
}

int main(int argc, char** argv)
{
    if (argc > 2) {
        print_usage();
        exit(1);
    } else if (argc == 2 && strcmp(argv[1], NO_DAEMON_ARG) != 0) {
        printf("Unrecognised arg: %s\n", argv[1]);
        print_usage();
        exit(1);
    }
    char* file_path = get_file_path();
    if (file_path == NULL) {
        exit(1);
    }

    if (argc < 2) {
        daemonize();
    }

    signal(SIGINT, clean_exit);
    signal(SIGTERM, clean_exit);

    start_server(file_path);

    free(file_path);

    return 0;
}
