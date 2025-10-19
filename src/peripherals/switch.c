#include "../../includes/peripherals/switch.h"
#include "../../includes/hal/hal-api.h"
#include <stdio.h>

#include "../../lib/address_map_arm.h"

// Global HAL mapping - shared across switch operations
static hal_map_t hal_map;
static int hal_initialized = 0;

int switch_init(switch_handle_t *sw) {
    if (!sw) return -1;
    
    // Initialize HAL if not already done
    if (!hal_initialized) {
        if (hal_open(&hal_map) != 0) {
            fprintf(stderr, "Failed to initialize HAL for switches\n");
            return -1;
        }
        hal_initialized = 1;
    }
    
    // Get virtual address for switch register (offset from LW bridge base)
    sw->reg_addr = hal_get_virtual_addr(&hal_map, SW_BASE);
    if (!sw->reg_addr) {
        fprintf(stderr, "Failed to get switch register address\n");
        return -1;
    }
    
    sw->initialized = 1;
    
    printf("Switch peripheral initialized successfully at virtual address %p\n", sw->reg_addr);
    return 0;
}

int switch_cleanup(switch_handle_t *sw) {
    if (!sw || !sw->initialized) return -1;
    
    sw->reg_addr = NULL;
    sw->initialized = 0;
    
    // Close HAL if this was the last switch user
    if (hal_initialized) {
        if (hal_close(&hal_map) != 0) {
            fprintf(stderr, "Failed to cleanup HAL\n");
            return -1;
        }
        hal_initialized = 0;
    }
    
    printf("Switch peripheral cleaned up successfully\n");
    return 0;
}

int switch_read_all(switch_handle_t *sw, uint32_t *switch_state) {
    if (!sw || !sw->initialized || !sw->reg_addr || !switch_state) return -1;
    
    // Read from switch register
    *switch_state = *(volatile uint32_t *)sw->reg_addr;
    
    // Mask to ensure only valid switch bits (10 switches)
    *switch_state &= SWITCH_ALL_MASK;
    
    return 0;
}

int switch_read(switch_handle_t *sw, int switch_number, int *state) {
    if (!sw || !sw->initialized || switch_number < 0 || switch_number >= SWITCH_COUNT || !state) {
        return -1;
    }
    
    // Get all switch states
    uint32_t all_switches;
    if (switch_read_all(sw, &all_switches) != 0) return -1;
    
    // Extract the specific switch state (1 if on, 0 if off)
    *state = (all_switches >> switch_number) & 1;
    
    return 0;
}