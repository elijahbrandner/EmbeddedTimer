#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H
#include <stdint.h>
#include "../hal/hal-api.h"

// ------------------------------------------------------------
// DE10-Standard HEX Displays
// ------------------------------------------------------------
// HEX0–HEX5 are driven in two physical groups on the board:
//   Group A: HEX0–HEX3 (BASE = HEX3_HEX0_BASE)
//   Group B: HEX4–HEX5 (BASE = HEX5_HEX4_BASE)
// Each display shows a single 7-segment digit (0–9, A–F).
// ------------------------------------------------------------

// Constants
#define HEX_DISPLAY_COUNT 6
#define HEX_DISPLAY_MASK  0x7F   // 7 segments (not counting decimal point)

// ------------------------------------------------------------
// Handle
// ------------------------------------------------------------
typedef struct {
    hal_map_t *hal;
    volatile uint32_t *hex03_reg;  // Base address for HEX0–HEX3
    volatile uint32_t *hex45_reg;  // Base address for HEX4–HEX5
    int initialized;
} hex_display_handle_t;

// ------------------------------------------------------------
// API
// ------------------------------------------------------------

// Initialize both HEX display groups
int hex_display_init(hex_display_handle_t *hex, hal_map_t *hal);

// Cleanup (currently a no-op)
int hex_display_cleanup(hex_display_handle_t *hex);

// Write a value (0–9 or 0–99) across the HEX displays
//   e.g., 42 -> "000042"
int hex_display_write(const hex_display_handle_t *hex, int value);

// Clear a specific HEX digit (0–5)
int hex_display_clear_digit(const hex_display_handle_t *hex, int digit);

// Clear all six HEX displays
int hex_display_clear_all(const hex_display_handle_t *hex);

#endif // HEX_DISPLAY_H