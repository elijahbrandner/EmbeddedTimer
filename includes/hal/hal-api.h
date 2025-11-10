#ifndef HAL_API_H
#define HAL_API_H

#include <stddef.h> // for size_t
#include <stdint.h> // for uint32_t

// --------------------------------------------------------------
// HAL CONFIGURATION
// --------------------------------------------------------------
// Toggle Simulation mode:
// 1 = simulation (no /dev/mem, only console I/O)
// 0 = real hardware (uses mmap via /dev/mem)
#define SIMULATION_MODE 1

// --------------------------------------------------------------
// HAL STRUCTURE
// --------------------------------------------------------------

typedef struct {
    int fd;                 // File descriptor for /dev/mem
    void *virtual_base;     // Base virtual address (mmap pointer)
    unsigned int span;      // Memory span for the mapping
} hal_map_t;

// --------------------------------------------------------------
// FUNCTION PROTOTYPES
// --------------------------------------------------------------

// Open and map /dev/mem to access physical FPGA addresses
// Returns 0 on success, -1 on failure
int hal_open(hal_map_t *map);

// Close mapping and cleanup
int hal_close(hal_map_t *map);

// Get virutual address of register at given offset
void* hal_get_virtual_addr(hal_map_t *map, unsigned int offset);

// Initialize all mapped peripherals (stub for now)
int hal_init(void);

// Cleanup all mapped peripherals (sub for now)
int hal_cleanup(void);

#endif // HAL_API_H
