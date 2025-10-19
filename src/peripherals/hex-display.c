#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
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
//?     GLOBALS
//?------------------------------------------------------------------------
static hal_map_t hex03_map;
static hal_map_t hex45_map;

static volatile uint32_t *hex03_ptr = NULL;
static volatile uint32_t *hex45_ptr = NULL;

//?------------------------------------------------------------------------
//?     INIT & CLOSE
//?------------------------------------------------------------------------
int init_hex0_hex3(void) {
    if (hal_open(&hex03_map) != 0) {
        return -1;
    }
    hex03_ptr = (volatile uint32_t *)hal_get_virtual_addr(&hex03_map, HEX3_HEX0_BASE);
    if (hex03_ptr == NULL) {
        hal_close(&hex03_map);
        return -1;
    }
    return 0;
}

int init_hex4_hex5(void) {
    if (hal_open(&hex45_map) != 0) {
        return -1;
    }
    hex45_ptr = (volatile uint32_t *)hal_get_virtual_addr(&hex45_map, HEX5_HEX4_BASE);
    if (hex45_ptr == NULL) {
        hal_close(&hex45_map);
        return -1;
    }
    return 0;
}

int close_hex0_hex3(void) {
    hex03_ptr = NULL;
    return hal_close(&hex03_map);
}

int close_hex4_hex5(void) {
    hex45_ptr = NULL;
    return hal_close(&hex45_map);
}

//?------------------------------------------------------------------------
//?     WRITE FUNCTIONS
//?------------------------------------------------------------------------
//* Write a digit (0–F) to a given HEX display
int hex_display_write(int display, int value) {
    if (value < 0 || value > 15) return -1;  // invalid digit
    uint8_t seg_code = seg_table[value];

    switch (display) {
        case 0: *hex03_ptr = (*hex03_ptr & 0xFFFFFF00) | seg_code; break;
        case 1: *hex03_ptr = (*hex03_ptr & 0xFFFF00FF) | (seg_code << 8); break;
        case 2: *hex03_ptr = (*hex03_ptr & 0xFF00FFFF) | (seg_code << 16); break;
        case 3: *hex03_ptr = (*hex03_ptr & 0x00FFFFFF) | (seg_code << 24); break;
        case 4: *hex45_ptr = (*hex45_ptr & 0xFFFFFF00) | seg_code; break;
        case 5: *hex45_ptr = (*hex45_ptr & 0xFFFF00FF) | (seg_code << 8); break;
        default: return -1;
    }
    return 0;
}

int hex_display_clear(int display) {
    return hex_display_write(display, 0x10); //* 0x10 = blank (not in seg_table)
}

void hex_display_clear_all(void) {
    if (hex03_ptr) *hex03_ptr = 0;
    if (hex45_ptr) *hex45_ptr = 0;
}