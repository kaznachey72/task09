#include "logger.h"
#include <execinfo.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

static FILE *fs = NULL;
static LEVEL cur_level = LEVEL_ERROR;
static bool was_fstream_opened = false; 
static pthread_mutex_t mutex;

bool log_init_fpath(const char *path, bool is_append)
{
    FILE *fstream = fopen(path, is_append ? "a" : "w");

    bool result = log_init_fstream(fstream);
    if (result) {
        was_fstream_opened = true;
    }
    
    return result;
}

bool log_init_fstream(FILE *fstream) 
{
    if (!fstream) {
        return false; 
    }

    if (fs && was_fstream_opened) {
        fclose(fs);
    }
    
    pthread_mutex_init(&mutex, NULL);
    was_fstream_opened = false;
    fs = fstream;

    return true;
}

void log_release() 
{
    if (fs) {
        fflush(fs);
        if (was_fstream_opened) {
            fclose(fs);
        } 
        fs = NULL;
    }
    pthread_mutex_destroy(&mutex);
}

void log_set_verbose(LEVEL level) 
{
    cur_level = level; 
}


static const char *get_level_name(LEVEL level)
{
    switch (level) {
        case LEVEL_ERROR:   return "ERROR";
        case LEVEL_WARNING: return "WARN ";
        case LEVEL_INFO:    return "INFO ";
        case LEVEL_DEBUG:   return "DEBUG";
        default:            return "UnDEF";
    }
}

static void print_prefix(LEVEL level)
{
    char dt_str[20] = {0};
    time_t dt = time(NULL);
    struct tm *now = localtime(&dt);
    strftime(dt_str, sizeof(dt_str), "%Y-%m-%d %T", now);
    fprintf(fs, "%s [ %s ]: ", dt_str, get_level_name(level));
}

static void print_bt()
{
    const int skip_sym = 2;
    const int max_size = 22;

    void *bt[max_size];
    int bt_size = backtrace(bt, max_size);
    char **bt_syms = backtrace_symbols(bt, bt_size);

    fprintf(fs, "--- BACKTRACE (max = %d) ----------------------\n", (max_size - skip_sym));
    for (int i=skip_sym; i!=bt_size; ++i) {
        fprintf(fs, " [%2d] %s\n", (i-skip_sym+1), bt_syms[i]);
    }
    fprintf(fs, "-----------------------------------------------\n");

    free(bt_syms);
}

void log_printf(LEVEL level, const char *format, ...) 
{
    if ((level <= cur_level) && fs) {
        pthread_mutex_lock(&mutex);
        
        va_list args;
        va_start(args, format);
        {
            print_prefix(level);
            vfprintf(fs, format, args);
            fprintf(fs, "\n");
        }
        va_end(args);
        
        if (level == LEVEL_ERROR) {
            print_bt();
        }

        fflush(fs);
        pthread_mutex_unlock(&mutex);
    }
}

