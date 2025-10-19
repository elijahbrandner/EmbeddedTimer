#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

// DE10 Standard has 10 switches (SW0-SW9)
#define SWITCH_COUNT 10

// Switch masks
#define SWITCH_ALL_MASK 0x3FF  // 10 bits (0-9)

// Switch handle structure
typedef struct {
    void *reg_addr;
    int initialized;
} switch_handle_t;

// Function declarations
int switch_init(switch_handle_t *sw);
int switch_cleanup(switch_handle_t *sw);
int switch_read_all(switch_handle_t *sw, uint32_t *switch_state);
int switch_read(switch_handle_t *sw, int switch_number, int *state);

#endif // SWITCH_H