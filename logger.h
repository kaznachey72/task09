#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    LEVEL_ERROR,
    LEVEL_WARNING,
    LEVEL_INFO,
    LEVEL_DEBUG,
} LEVEL;

bool log_init_fpath(const char *path, bool is_append);
bool log_init_fstream(FILE *fstream);
void log_release();

void log_set_verbose(LEVEL level);
void log_printf(LEVEL level, const char *format, ...);

#endif // LOGGER_H
