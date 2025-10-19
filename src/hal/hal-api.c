#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../../lib/address_map_arm.h"
#include "../../includes/hal/hal-api.h"

int hal_open(hal_map_t *map) {
    if (!map) return -1;

    map->fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (map->fd == -1) {
        perror("ERROR: could not open /dev/mem");
        return -1;
    }

    map->virtual_base = mmap(NULL, LW_BRIDGE_SPAN, PROT_READ | PROT_WRITE, 
                            MAP_SHARED, map->fd, LW_BRIDGE_BASE);
    if (map->virtual_base == MAP_FAILED) {
        perror("ERROR: mmap() failed");
        close(map->fd);
        return -1;
    }

    map->span = LW_BRIDGE_SPAN;
    return 0;
}

int hal_close(hal_map_t *map) {
    if (!map) return -1;

    if (munmap(map->virtual_base, map->span) != 0) {
        perror("ERROR: munmap() failed");
        return -1;
    }

    close(map->fd);
    map->fd = -1;
    map->virtual_base = NULL;
    map->span = 0;
    return 0;
}

void* hal_get_virtual_addr(hal_map_t *map, unsigned int offset) {
    if (!map || !map->virtual_base) return NULL;
    
    return (void*)((char*)map->virtual_base + offset);
}