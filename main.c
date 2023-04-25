#include "app_control.h"
#include "logger.h"
#include "server.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


const char *LOG_FILE = "calc_file_size.log";
bool IS_DEMONIZE = false;

void show_usage(const char *app)
{
    printf("Использование: %s [ПАРАМЕТР]\n", app);
    printf("ПАРАМЕТРЫ:\n");
    printf("   -h  отображение справки\n");
    printf("   -d  запуск в режиме демона\n");
}

void parse_argv(const char *app, const char *arg)
{
    if (strncmp(arg, "-h", 2) == 0) {
        show_usage(app);
        exit(EXIT_SUCCESS);
    }
    else if (strncmp(arg, "-d", 2) == 0) {
        IS_DEMONIZE = true;
    }
    else {
        show_usage(app);
        exit(EXIT_FAILURE);
    }
}

void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            log_printf(LEVEL_INFO, "signal: завершение работы");
            srv_stop();            
            break;

        case SIGHUP:
            log_printf(LEVEL_INFO, "signal: обновление конфигурации");
            srv_update_config();
            break;
    }
}

void on_app_exit()
{
    log_printf(LEVEL_DEBUG, "exit: завершение работы");
    log_release();
    app_clear();
}

int main(int argc, char *argv[])
{
    if (argc > 2) {
        show_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc == 2) {
        parse_argv(argv[0], argv[1]);
    }


    if (IS_DEMONIZE) {
        demonize();
        log_init_fpath(LOG_FILE, true);
    }
    else {
        log_init_fstream(stdout);
    }

    log_set_verbose(LEVEL_DEBUG);
    atexit(on_app_exit);

    if (!is_single()) {
        log_printf(LEVEL_WARNING, "run app: другая копия приложения уже запущена (%s)", strerror(errno));
        log_release();
        _exit(EXIT_FAILURE); // заверешить без обработки 
    }
    
    signal(SIGINT,  signal_handler);
    signal(SIGHUP,  signal_handler);
    signal(SIGTERM, signal_handler);
    
    srv_start();

    return EXIT_SUCCESS;
}
