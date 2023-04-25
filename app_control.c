#include "app_control.h"
#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

const char *LOCK_FILE = "/tmp/calc_file_size.pid";

void demonize()
{
    //--- созадть 1-го потомка, с родителем "init" (reparenting) ---------------

    pid_t pid = fork();
    if (pid < 0) {
        log_printf(LEVEL_ERROR, "demonize: ошибка создания первого потомка (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        exit(EXIT_SUCCESS); // родитель
    }
    

    //--- сознать новоую сиссию, и стать ее лидиром ----------------------------

    if (setsid() < 0) {
        log_printf(LEVEL_ERROR, "demonize: ошибка создания новой сессии (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //--- закрыть старые, и привязать новые файловые дескрипторы ---------------

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        log_printf(LEVEL_ERROR, "demonize: ошибка получение максимального дескриптора (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (size_t i=0; i<rl.rlim_max; ++i) {
        close(i);
    }

    // присоединить файловые дескрипторы stdin, stdout и stderr к /dev/null.
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        log_printf(LEVEL_ERROR, "demonize: ошибка установки файловых дескрипторов для stdin, stdout и stderr (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    

    //--- созадть 2-го потомка, перестанет быть лидером сессии -----------------
    pid = fork();
    if (pid < 0) {
        log_printf(LEVEL_ERROR, "demonize: ошибка создания второго потомка (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        exit(EXIT_SUCCESS); // потомок 1
    }
}

bool is_single()
{
    int fd = open(LOCK_FILE, O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
        log_printf(LEVEL_ERROR, "single app: ошибка открытия lock-файла (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rc = flock(fd, LOCK_EX|LOCK_NB);
    if (rc < 0) {
        if (EWOULDBLOCK == errno) {
            close(fd);
            return false;
        }
        else {
            log_printf(LEVEL_ERROR, "single app: ошибка блокировки lock-файла (%s)", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid = getpid();
    const char *template = "%d";
    size_t len = snprintf(NULL, 0, template, pid) + 1;
    char *buf = (char*)malloc(len);
    if (!buf) {
        log_printf(LEVEL_ERROR, "single app: ошибка формирования буфера (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    snprintf(buf, len, template, pid);
    write(fd, buf, len);
    // дескриптор освободится после завершения приложения, вместе со снятием блокировки
    free(buf);
    return true;
}

void app_clear()
{
    unlink(LOCK_FILE);
}
