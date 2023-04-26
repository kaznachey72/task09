#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char *fpath;
} config_t;

void get_config(config_t *cfg);
void config_clear(config_t *cfg);

#endif // CONFIG_H
