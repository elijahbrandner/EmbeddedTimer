#include <stdio.h>
#include <stdint.h>
#include "../../includes/hal/hal-api.h"
#include "../../includes/peripherals/hex-display.h"
#include "../../lib/address_map_arm.h"

//?------------------------------------------------------------------------
//?     CONSTANTS
//?------------------------------------------------------------------------
static const uint8_t seg_table[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71  // F
};

//?------------------------------------------------------------------------
//?     Initialization / Cleanup 
//?------------------------------------------------------------------------
int hex_display_init(hex_display_handle_t *hex, hal_map_t *hal) {
    if (!hex) return -1;
    hex->hal = hal;

#if SIMULATION_MODE
    hex->hex03_reg = NULL;
    hex->hex45_reg = NULL;
    hex->initialized = 1;
    printf("[HEX] Simulation mode - initialized (no MMIO). \n");
    return 0;
#else
    if (!hal) return -1;
    hex->hex03_reg = (volatile uint32_t *)hal_get_virtual_addr(hal, HEX3_HEX0_BASE);
    hex->hex45_reg = (volatile uint32_t *)hal_get_virtual_addr(hal, HEX5_HEX4_BASE);

    if (!hex->hex03_reg || !hex->hex45_reg) {
        hex->initialized = 0;
        return -1;
    }
    hex->initialized = 1;
    return 0;
#endif
}

int hex_display_cleanup(hex_display_handle_t *hex) {
    if (!hex) return -1;
    hex->hal = NULL;
    hex->hex03_reg = NULL;
    hex->hex45_reg = NULL;
    hex->initialized = 0;
}

//?------------------------------------------------------------------------
// Write / Clear
//?------------------------------------------------------------------------

int hex_display_write (const hex_display_handle_t *hex, int value) {
    if (!hex || !hex->initialized) return -1;

#if SIMULATION_MODE
    printf("[HEX] Simulated display output: %06d\n", value);
    return 0;
#else
    // Split the value into digits and update HEX0-HEX5
    int digits[6];
    for (int i = 0; i < 6; i++) {
        digits[i] = value % 10;
        value /= 10;
    }

    // Write to HEX0-HEX3
    uint32_t word03 = 0;
    for (int i = 0; i < 4; i++) {
        word03 |= (seg_table[digits[i]] << (8 * i));
    }
    *(hex->hex45_reg) = word45;
    return 0;
#endif
}

int hex_display_clear_digit(const hex_display_handle_t *hex, int digit) {
    if (!hex || !hex->initialized) return -1;

#if SIMULATION_MODE
    printf("[HEX] Clear digit %d (simulated) \n", digit);
    return 0;
#else
    // Active-low, so 0xFF means "off"
    if (digit < 0 || digit > 5) return -1;

    if (digit <= 3) {
        uint32_t word = *(hex->hex03_reg);
        word &= ~(0xFF << (8 * digit));
        word |= (0xFF << (8 * digit));
        *(hex->hex03_reg) = word;
    } else {
        uint32_t word = *(hex->hex45_reg);
        word &= ~(0xFF << (8 * (digit -4)));
        word |= (0xFF << (8 * (digit -4)));
        *(hex->hex45_reg) = word;
    }
    return 0;
#endif
}

int hex_display_clear_all(const hex_display_handle_t *hex) {
    if (!hex || !hex->initialized) return -1;

#if SIMULATION_MODE
    printf("[HEX] Clear all digits (simulated)\n");
    return 0;
#else  
    *(hex->hex03_reg) = 0xFFFFFFFF;
    *(hex->hex45_reg) = 0xFFFFFFFF;
    return 0;
#endif
}