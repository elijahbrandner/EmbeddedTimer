#include <stdio.h>
#include <stdint.h>
#include "../../includes/peripherals/button.h"
#include "../../includes/hal/hal-api.h"
#include "../../lib/address_map_arm.h"

// -----------------------------------------------------------------------------
// Internal helper (simulation mode only)
// -----------------------------------------------------------------------------
#if SIMULATION_MODE
int button_get_simulated_key(void) {
    char buffer[16];
    printf("[BUTTON] Simulation — Enter Key (0–3): ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        switch (buffer[0]) {
            case '0': return 0; // KEY0
            case '1': return 1; // KEY1
            case '2': return 2; // KEY2
            case '3': return 3; // KEY3
            default:  return -1; // Invalid
        }
    }
    return -1;
}
#endif

// -----------------------------------------------------------------------------
// API IMPLEMENTATION
// -----------------------------------------------------------------------------
int button_init(button_handle_t *btn, hal_map_t *hal) {
    if (!btn) return -1;
    btn->hal = hal;

#if SIMULATION_MODE
    btn->reg = NULL;
    btn->initialized = 1;
    printf("[BUTTON] Simulation mode — initialized (no MMIO).\n");
    return 0;
#else
    if (!hal) return -1;
    btn->reg = (volatile uint32_t *)hal_get_virtual_addr(hal, KEY_BASE);
    if (!btn->reg) {
        btn->initialized = 0;
        return -1;
    }
    btn->initialized = 1;
    return 0;
#endif
}

int button_cleanup(button_handle_t *btn) {
    if (!btn) return -1;
    btn->reg = NULL;
    btn->initialized = 0;
    btn->hal = NULL;
    return 0;
}

int button_read_all(const button_handle_t *btn, uint32_t *state) {
    if (!btn || !state || !btn->initialized) return -1;

#if SIMULATION_MODE
    int key = button_get_simulated_key();
    if (key >= 0 && key < BUTTON_COUNT)
        *state = (1u << key);
    else
        *state = 0;
    return 0;
#else
    *state = (*(btn->reg)) & BUTTON_ALL_MASK;
    return 0;
#endif
}

int button_read(const button_handle_t *btn, int button_number, int *pressed) {
    if (!btn || !pressed || !btn->initialized) return -1;
    if (button_number < 0 || button_number >= BUTTON_COUNT) return -1;

    uint32_t all = 0;
    if (button_read_all(btn, &all) != 0) return -1;

    *pressed = (int)((all >> button_number) & 0x1u);
    return 0;
}
