#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // for usleep()
#include "../includes/hal/hal-api.h"
#include "../includes/peripherals/button.h"
#include "../includes/peripherals/switch.h"
#include "../includes/peripherals/hex-display.h"
#include "../includes/peripherals/soft_timer.h"

// -----------------------------------------------------------------------------
// Application States
// -----------------------------------------------------------------------------
typedef enum {
    STATE_IDLE,
    // Stopwatch Flow
    STATE_STOPWATCH_READY,
    STATE_STOPWATCH_RUNNING,
    STATE_STOPWATCH_PAUSED,
    // Countdown Flow
    STATE_SET_HOURS,
    STATE_SET_MINUTES,
    STATE_SET_SECONDS,
    STATE_COUNTDOWN_READY,
    STATE_COUNTDOWN_RUNNING,
    STATE_COUNTDOWN_PAUSED,
    STATE_COUNTDOWN_FINISHED,
    STATE_EXIT
} app_state_t;

// -----------------------------------------------------------------------------
// Globals / Handles
// -----------------------------------------------------------------------------
hal_map_t               hal;
button_handle_t         btn;
switch_handle_t         sw;
hex_display_handle_t    hex;

// Software timer (pure C, no hardware)
soft_timer_t            timer;

// Preset values for countdown
static int preset_hours     = 0;
static int preset_minutes   = 0;
static int preset_seconds   = 0;

// Last raw key state (for edge detection)
static uint32_t g_prev_keys = 0xF;  // assume all released at start (KEY[3:0] = 1)

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Read Button *edges* (active-low: press = 1->0 transition)
static void read_button_edges(int *k0, int *k1, int *k2, int *k3) {
    uint32_t state = 0;
    if (button_read_all(&btn, &state) != 0) {
        *k0 = *k1 = *k2 = *k3 = 0;
        return;
    }

    // changed bits
    uint32_t changed = g_prev_keys ^ state;

    // active-low press = bit changed AND new bit == 0
    uint32_t pressed_mask = changed & (~state);

    *k0 = (pressed_mask & (1u << 0)) ? 1 : 0;
    *k1 = (pressed_mask & (1u << 1)) ? 1 : 0;
    *k2 = (pressed_mask & (1u << 2)) ? 1 : 0;
    *k3 = (pressed_mask & (1u << 3)) ? 1 : 0;

    g_prev_keys = state;
}

// Format HH:MM:SS as a 6-digit integer for HEX (HHMMSS)
static int format_time_for_hex(int h, int m, int s) {
    if (h < 0) h = 0;
    if (h > 99) h = 99;
    if (m < 0) m = 0;
    if (m > 59) m = 59;
    if (s < 0) s = 0;
    if (s > 59) s = 59;
    return (h * 10000) + (m * 100) + s;
}

// Display current preset value (used in SET_* and READY states)
static void show_preset_on_hex(void) {
    int value = format_time_for_hex(preset_hours, preset_minutes, preset_seconds);
    hex_display_write(&hex, value);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(void) {

    int countdown_finished_printed = 0;
    int countdown_paused_printed = 0;


    printf("[SYSTEM] Timer Application - Software Mode\n");

    // Open HAL / map LW bridge FIRST
    if (hal_open(&hal) != 0) {
        printf("[ERROR] HAL Initialization failed.\n");
        return -1;
    }

    // Initialize peripherals
    if (button_init(&btn, &hal) != 0) {
        printf("[ERROR] Button init failed.\n");
        hal_close(&hal);
        return -1;
    }

    if (switch_init(&sw, &hal) != 0) {
        printf("[ERROR] Switch init failed.\n");
        button_cleanup(&btn);
        hal_close(&hal);
        return -1;
    }

    if (hex_display_init(&hex, &hal) != 0) {
        printf("[ERROR] HEX init failed.\n");
        switch_cleanup(&sw);
        button_cleanup(&btn);
        hal_close(&hal);
        return -1;
    }

    // Now that HAL + peripherals are initialized, clear HEX cleanly
    hex_display_clear_all(&hex);

    // Initialize prev button state from hardware
    uint32_t initial_buttons = 0;
    button_read_all(&btn, &initial_buttons);
    g_prev_keys = initial_buttons;

    // Initialize software timer
    soft_timer_init(&timer);

    app_state_t current_state = STATE_IDLE;
    int running = 1;

    printf("[SYSTEM] Use SW9 to select mode (0=Countdown, 1=Stopwatch).\n");
    printf("[SYSTEM] KEY0=Reset/Idle, KEY1=Confirm/Next, KEY2=Start/Pause, KEY3=Exit.\n");

    while (running) {
        int key0 = 0, key1 = 0, key2 = 0, key3 = 0;
        read_button_edges(&key0, &key1, &key2, &key3);

        // Global exit
        if (key3) {
            current_state = STATE_EXIT;
        }

        switch (current_state) {
        // -----------------------------------------------------------------------------------
        case STATE_IDLE: {
            // Show 00:00:00 and mode hint
            preset_hours = preset_minutes = preset_seconds = 0;
            show_preset_on_hex();

            // Decide mode based on SW9 when user presses KEY1 or KEY2
            if (key0) {
                // Already idle; nothing to do
            } else if (key1 || key2) {
                int mode = switch_read_mode(&sw); // 0 = countdown, 1 = stopwatch
                if (mode == SWITCH_MODE_COUNTDOWN) {
                    printf("[MODE] Countdown Selected (SW9 = 0).\n");
                    current_state = STATE_SET_HOURS;
                } else {
                    printf("[MODE] Stopwatch selected (SW9 = 1).\n");
                    soft_timer_reset(&timer);
                    preset_hours = preset_minutes = preset_seconds = 0;
                    current_state = STATE_STOPWATCH_READY;
                }
            }
            break;
        }

        // -----------------------------------------------------------------------------
        // COUNTDOWN input flow (H -> M -> S)
        // -----------------------------------------------------------------------------
        case STATE_SET_HOURS: {
            // Read numeric value from SW[6:0], clamped 0-99
            int value = switch_read_input_value(&sw);
            if (value > 99) value = 99;
            preset_hours = value;
            show_preset_on_hex();

            if (key0) {
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key1) {
                printf("[COUNTDOWN] Hours set to %d.\n", preset_hours);
                current_state = STATE_SET_MINUTES;
            }
            break;
        }

        case STATE_SET_MINUTES: {
            int value = switch_read_input_value(&sw);
            if (value > 59) value = 59;
            preset_minutes = value;
            show_preset_on_hex();

            if (key0) {
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key1) {
                printf("[COUNTDOWN] Minutes set to %d.\n", preset_minutes);
                current_state = STATE_SET_SECONDS;
            }
            break;
        }

        case STATE_SET_SECONDS: {
            int value = switch_read_input_value(&sw);
            if (value > 59) value = 59;
            preset_seconds = value;
            show_preset_on_hex();

            if (key0) {
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key1) {
                printf("[COUNTDOWN] Seconds set to %d.\n", preset_seconds);
                current_state = STATE_COUNTDOWN_READY;
            }
            break;
        }

        case STATE_COUNTDOWN_READY: {
            // show full preset time and wait for key2 (Start) or reset
            show_preset_on_hex();

            if (key0) {
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key2 || key1) {
                printf("[COUNTDOWN] Starting countdown...\n");

                soft_timer_set(&timer, preset_hours, preset_minutes, preset_seconds);
                soft_timer_start(&timer);

                current_state = STATE_COUNTDOWN_RUNNING;
            }
            break;
        }

        case STATE_COUNTDOWN_RUNNING: {

            int ms_counter = 0;

            while (current_state == STATE_COUNTDOWN_RUNNING) {

                int ik0=0, ik1=0, ik2=0, ik3=0;
                read_button_edges(&ik0, &ik1, &ik2, &ik3);

                if (ik0) {
                    printf("[COUNTDOWN] Reset.\n");
                    soft_timer_reset(&timer);
                    preset_hours = preset_minutes = preset_seconds = 0;
                    current_state = STATE_IDLE;
                    break;
                }


                // PAUSE
                if (ik2) {
                    soft_timer_pause(&timer);
                    current_state = STATE_COUNTDOWN_PAUSED;
                    break;
                }

                // Tick once per second
                if (ms_counter >= 1000) {
                    soft_timer_tick_down(&timer);
                    ms_counter = 0;
                }

                // Finished?
                if (soft_timer_is_finished(&timer)) {
                    // Force display to show 00:00:00
                    hex_display_write(&hex, 0);
                    current_state = STATE_COUNTDOWN_FINISHED;
                    break;
                }

                // Update HEX every loop
                hex_display_write(&hex,
                    format_time_for_hex(timer.hours, timer.minutes, timer.seconds));

                usleep(10000); // 10 ms
                ms_counter += 10;
            }

            break;
        }


        case STATE_COUNTDOWN_PAUSED: {

            if (!countdown_paused_printed) {
                printf("[COUNTDOWN] Paused.\n");
                countdown_paused_printed = 1;
            }

            if (key0) {
                printf("[COUNTDOWN] Reset to idle from paused.\n");
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                countdown_paused_printed = 0;
                current_state = STATE_IDLE;
            } 
            else if (key2 || key1) {
                printf("[COUNTDOWN] Resuming.\n");
                soft_timer_start(&timer);
                countdown_paused_printed = 0;
                current_state = STATE_COUNTDOWN_RUNNING;
            }

            break;
        }


        case STATE_COUNTDOWN_FINISHED: {

            if (!countdown_finished_printed) {
                printf("[COUNTDOWN] Finished.\n");
                hex_display_write(&hex, 0);    // extra safety
                countdown_finished_printed = 1;
            }

            if (key0 || key1) {
                printf("[COUNTDOWN] Reset.\n");
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                countdown_finished_printed = 0;
                current_state = STATE_IDLE;
            }

            break;
        }



        // -----------------------------------------------------------------------------
        // STOPWATCH FLOW
        // -----------------------------------------------------------------------------
        case STATE_STOPWATCH_READY: {
            // Stopwatch always starts from 00:00:00
            preset_hours = preset_minutes = preset_seconds = 0;
            show_preset_on_hex();

            if (key0) {
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key2 || key1) {
                printf("[STOPWATCH] Starting.\n");

                soft_timer_reset(&timer);
                soft_timer_start(&timer);

                current_state = STATE_STOPWATCH_RUNNING;
            }
            break;
        }

        case STATE_STOPWATCH_RUNNING: {

            int ms_counter = 0;

            while (current_state == STATE_STOPWATCH_RUNNING) {

                int ik0=0, ik1=0, ik2=0, ik3=0;
                read_button_edges(&ik0, &ik1, &ik2, &ik3);

                // RESET
                if (ik0) {
                    printf("[STOPWATCH] Reset.\n");
                    soft_timer_reset(&timer);
                    current_state = STATE_IDLE;
                    break;
                }

                // PAUSE
                if (ik2) {
                    printf("[STOPWATCH] Paused.\n");
                    soft_timer_pause(&timer);
                    current_state = STATE_STOPWATCH_PAUSED;
                    break;
                }

                // Tick once per 1000ms
                if (ms_counter >= 1000) {
                    soft_timer_tick_up(&timer);
                    ms_counter = 0;
                }

                // Always update HEX display
                hex_display_write(&hex,
                    format_time_for_hex(timer.hours, timer.minutes, timer.seconds));

                usleep(10000);  // 10ms
                ms_counter += 10;
            }

            break;
        }


        case STATE_STOPWATCH_PAUSED: {
            // Time is frozen in software; user can resume or reset
            if (key0) {
                printf("[STOPWATCH] Reset to idle from paused.\n");
                soft_timer_reset(&timer);
                preset_hours = preset_minutes = preset_seconds = 0;
                current_state = STATE_IDLE;
            } else if (key2 || key1) {
                printf("[STOPWATCH] Resuming.\n");
                soft_timer_start(&timer);
                current_state = STATE_STOPWATCH_RUNNING;
            }
            break;
        }

        // -----------------------------------------------------------------------------
        case STATE_EXIT:
            running = 0;
            break;
        } // end switch

        // Small delay so we dont busy-spin the CPU
        usleep(10000); // 10 ms
    }

    printf("[SYSTEM] Exiting application.\n");

    // Cleanup
    hex_display_clear_all(&hex);
    hex_display_cleanup(&hex);
    switch_cleanup(&sw);
    button_cleanup(&btn);
    hal_close(&hal);

    return 0;
}
