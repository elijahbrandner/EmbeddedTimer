#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../../lib/address_map_arm.h"
#include "../../includes/hal/hal-api.h"

// --------------------------------------------------------------
// HAL IMPLEMENTATION
// --------------------------------------------------------------

// Open and initialize the hardware abstraction layer
int hal_open(hal_map_t *map) {
    if (!map) return -1;


#if SIMULATION_MODE
    printf("[HAL] Simulation mode active - skipping /dev/mem mapping. \n");
    map->fd = -1;
    map->virtual_base = NULL;
    map->span = 0;
    return 0;

#else 
    map->fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (map->fd == -1) {
        perror("[HAL] ERROR: could not open /dev/mem");
        return -1;
    }

    map->virtual_base = mmap(
        NULL, 
        LW_BRIDGE_SPAN,
        PROT_READ | PROT_WRITE, 
        MAP_SHARED,
        map->fd,
        LW_BRIDGE_BASE
    );

    if (map->virtual_base == MAP_FAILED) {
        perror("[HAL] ERROR: mmap() failed");
        close(map->fd);
        return -1;
    }

    map->span = LW_BRIDGE_SPAN;
    printf("[HAL] Hardware mapping successful (base: 0x%X)\n", LW_BRIDGE_BASE);
    return 0;
#endif
}

int hal_close(hal_map_t *map) {
    if (!map) return -1;

#if SIMULATION_MODE
    printf("[HAL] Simulation mode - no mmap to close.\n");
    map->fd = -1;
    map->virtual_base = NULL;
    map->span = 0;
    return 0;
#else
    if (munmap(map->virtual_base, map->span) != 0) {
        perror("[HAL] ERROR: munmap() failed");
        return -1;
    }

    close(map->fd);
    map->fd = -1;
    map->virtual_base = NULL;
    map->span = 0;
    printf("[HAL] Hardware unmapped successfully. \n");
    return 0;
#endif
}

void* hal_get_virtual_addr(hal_map_t *map, unsigned int offset) {
#if SIMULATION_MODE
    // Simulation: just return NULL to indicate there's no mapped memory
    (void)map;
    (void)offset;
    return NULL;
#else 
    if (!map || !map->virtual_base) return NULL;
    return (void*)((uint8_t)map->virtual_base + offset);
#endif
}

// Optional stubs (for clarity and expansion)
int hal_init(void) {
#if SIMULATION_MODE
    printf("[HAL] Simulation init stub executed. \n");
#endif
    return 0;
}

int hal_cleanup(void) {
#if SIMULATION_MODE
    printf("[HAL] Simulation cleanup stub executed. \n");
#endif
    return 0;
}