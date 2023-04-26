#include "config.h"
#include "logger.h"
#include "ini.h"

#include <stdlib.h>
#include <string.h>


const char *CONF_FILE = "calc_file_size.ini";


static bool exists()
{
    FILE *f = fopen(CONF_FILE, "r");
    bool is_exist = false;
    if (f) {
        is_exist = true;
        fclose(f);
    }
    return is_exist;
}

static void create_default()
{
    FILE *f = fopen(CONF_FILE, "w");
    if (!f) {
        log_printf(LEVEL_ERROR, "ini file: ошибка записи конфигурационного файла (%s)", CONF_FILE);
        exit(EXIT_FAILURE);
    }
    
    fprintf(f, "[global]\nfile_path = main.c\n");
    fclose(f);
}

static int handler(void *rec, const char *section, const char *name, const char *value)
{
    config_t *cfg = (config_t*)rec;

    #define MATCH(s, n) (strcmp(section, s) == 0) && (strcmp(name, n) == 0)

    if (MATCH("global", "file_path")) {
        cfg->fpath = strdup(value);
    } 
    else {
        return 0;
    }
    return 1;
}

void get_config(config_t *cfg)
{
    if (!exists()) {
        create_default();
    }

    config_clear(cfg);
    if (ini_parse(CONF_FILE, handler, cfg) < 0) {
        log_printf(LEVEL_ERROR, "ini file: ошибка чтения конфигурационного файла (%s)", CONF_FILE);
        exit(EXIT_FAILURE);
    }
}

void config_clear(config_t *cfg)
{
    free(cfg->fpath);
}
