#ifndef HAL_API_H
#define HAL_API_H

#include <stddef.h>

typedef struct {
    int fd;
    void *virtual_base;
    unsigned int span;
} hal_map_t;

int hal_open(hal_map_t *map);
int hal_close(hal_map_t *map);
void* hal_get_virtual_addr(hal_map_t *map, unsigned int offset);

#endif // HAL_API_H
