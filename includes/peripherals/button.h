#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "../hal/hal-api.h"   // for hal_map_t and SIMULATION_MODE

// ------------------------------------------------------------
// DE10-Standard Push Buttons (KEY[3:0])
// ------------------------------------------------------------
// KEY0 -> Reset / Return to Idle
// KEY1 -> Next / Input / Confirm
// KEY2 -> Start / Pause
// KEY3 -> Exit Simulation
// ------------------------------------------------------------

#define BUTTON_COUNT      4
#define BUTTON_ALL_MASK   0xF   // 4 bits (KEY3–KEY0)

// Optional identifiers for clarity
typedef enum {
    BUTTON_KEY0 = 0,
    BUTTON_KEY1 = 1,
    BUTTON_KEY2 = 2,
    BUTTON_KEY3 = 3
} button_id_t;

// ------------------------------------------------------------
// Button handle
// ------------------------------------------------------------
typedef struct {
    hal_map_t *hal;           // reference to HAL map
    volatile uint32_t *reg;   // mapped virtual register (NULL in simulation)
    int initialized;
} button_handle_t;

// ------------------------------------------------------------
// API
// ------------------------------------------------------------

// Initialize button peripheral
int button_init(button_handle_t *btn, hal_map_t *hal);

// Cleanup button peripheral
int button_cleanup(button_handle_t *btn);

// Read all button states (returns bitmask of KEY3–KEY0)
int button_read_all(const button_handle_t *btn, uint32_t *state);

// Read a specific button (0–3)
int button_read(const button_handle_t *btn, int button_number, int *pressed);

// Wait for a simulated key press (console input) in simulation mode
// Returns pressed key ID (0–3) or -1 for invalid input
int button_get_simulated_key(void);

#endif // BUTTON_H
