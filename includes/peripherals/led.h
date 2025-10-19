#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef struct {
    void *reg_addr;   
    int initialized;
} led_handle_t;

int led_init(led_handle_t *led);
int led_cleanup(led_handle_t *led);
int led_set(led_handle_t *led, uint32_t pattern);
int led_get(led_handle_t *led, uint32_t *pattern);
int led_turn_on(led_handle_t *led, int led_number);

#define LED_COUNT       10
#define LED_ALL_ON      0x3FF  /* All 10 LEDs on: 1111111111 binary */
#define LED_ALL_OFF     0x000  /* All LEDs off */

#endif // LED_H


