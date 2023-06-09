#include "server.h"
#include "config.h"
#include "logger.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>


const char *SOCK_FILE = "/tmp/calc_file_size.sock";
const int CONNECTION_LENGTH = 10;
bool IS_LOOP_RUNNING = true;
config_t cfg;

typedef struct {
    int client_sd;
    char *fpath;
} arg_t;


static off_t fsize(const char *fpath)
{
    int fd = open(fpath, O_RDONLY);
    if (fd < 0) {
        log_printf(LEVEL_ERROR, "file size: ошибка доступа к файлу (%s)", strerror(errno));
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        log_printf(LEVEL_ERROR, "file size: ошибка получения статистики файла (%s)", strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return st.st_size;
}

static void *client_handler(void *arg_)
{
    arg_t *arg = (arg_t*)arg_;

    const char *template = "размер файла '%s': %ld байт\n";
    size_t len = snprintf(NULL, 0, template, arg->fpath, fsize(arg->fpath)) + 1;
    char *response = (char*)malloc(len);
    if (!response) {
        log_printf(LEVEL_ERROR, "client handler: ошибка формирования ответа (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    snprintf(response, len, template, arg->fpath, fsize(arg->fpath));

    write(arg->client_sd, response, len);
    free(response);
    close(arg->client_sd);
    free(arg->fpath);
    free(arg);

    return NULL;
}

static void accept_loop(int server_sd)
{
    struct pollfd pollfds[1];
    pollfds[0].fd = server_sd;
    pollfds[0].events = POLLIN;

    while (IS_LOOP_RUNNING) {
        pollfds[0].revents = 0;
        if (poll(pollfds, 1, -1) < 0 && errno != EINTR) {
            log_printf(LEVEL_ERROR, "accept loop: ошибка получения события (%s)", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (pollfds[0].revents & POLLIN) {
            int client_sd = accept(server_sd, NULL, NULL);
            if (client_sd < 0) {
                close(server_sd);
                log_printf(LEVEL_ERROR, "accept loop: ошибка подключения клиента (%s)", strerror(errno));
                exit(EXIT_FAILURE);
            }

            pthread_t tid;
            arg_t *arg = malloc(sizeof(arg_t));
            arg->client_sd = client_sd;
            arg->fpath = strdup(cfg.fpath);
            int result = pthread_create(&tid, NULL, &client_handler, arg);
            if (result) {
                close(client_sd);
                close(server_sd);
                free(arg);
                log_printf(LEVEL_ERROR, "accept loop: ошибка запуска потока обработки клиента (%s)", strerror(errno));
                exit(EXIT_FAILURE);
            }
            pthread_detach(tid);
        }
    }
}


void srv_start()
{
    int sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd < 0) {
        log_printf(LEVEL_ERROR, "socket: ошибка создания сокета (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    unlink(SOCK_FILE);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_FILE, sizeof(addr.sun_path) - 1);
    int result = bind(sd, (struct sockaddr*)&addr, sizeof(addr));
    if (result < 0) {
        close(sd);
        log_printf(LEVEL_ERROR, "socket: ошибка привязки адреса (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    result = listen(sd, CONNECTION_LENGTH);
    if (result < 0) {
        close(sd);
        log_printf(LEVEL_ERROR, "socket: ошибка установки очереди (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    srv_update_config();
    accept_loop(sd);

    close(sd);
    unlink(SOCK_FILE);
    config_clear(&cfg);
}

void srv_stop()
{
    IS_LOOP_RUNNING = false;
}

void srv_update_config()
{
    get_config(&cfg);
}

