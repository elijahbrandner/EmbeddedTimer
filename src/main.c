#include <stdio.h>
#include <unistd.h> // for sleep()
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include "../includes/hal/hal-api.h"
#include "../includes/peripherals/button.h"
#include "../includes/peripherals/switch.h"
#include "../includes/peripherals/hex-display.h"


//? Application States
typedef enum {
    STATE_IDLE,
    //? Stopwatch Flow
    STATE_STOPWATCH_READY,
    STATE_STOPWATCH_RUNNING,
    STATE_STOPWATCH_PAUSED,
    //? Countdown Flow
    STATE_SET_HOURS,
    STATE_SET_MINUTES,
    STATE_SET_SECONDS,
    STATE_COUNTDOWN_READY,
    STATE_COUNTDOWN_RUNNING,
    STATE_COUNTDOWN_PAUSED, 
    STATE_COUNTDOWN_FINISHED,
    STATE_EXIT
} app_state_t;

//? Globals
int hours = 0, minutes = 0, seconds = 0;

// ----- Helpers ----

//? Non-blocking input check
int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

void print_key_directions() {
    printf("\n=== KEY DIRECTIONS ===\n");
    printf("KEY0 (0) - Return to Idle\n");
    printf("KEY1 (1) - Input / Next\n");
    printf("KEY2 (2) - Start / Pause\n");
    printf("KEY3 (3) - EXIT SIMULATION\n");
    printf("======================\n");
}

//? Read simulated key input from user KEY0-KEY3 (Prints once per call)
int get_key_input() {
    char buffer[16];
    printf("Enter Key (0-3)");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        switch (buffer[0]) {
            case '0': return 0; // Key0
            case '1': return 1; // Key1
            case '2': return 2; // Key2
            case '3': return 3; // Key3
            default: return -1; // Invalid input
        }
    }
    return -1; // No input
}

//? Helper: Print Time formatted
void print_time() {
    printf("Current Time: %02d:%02d:%02d\n", hours, minutes, seconds);
}

//? Helper: Countdown tick
void countdown_tick(int *hours, int *minutes, int *seconds) {
    sleep(1); // wait one second
    if (*seconds > 0) {
        (*seconds)--;
    } else if (*minutes > 0) {
            (*minutes)--;
            *seconds = 59;
    } else if (*hours > 0) {
            *minutes = 59;
            *seconds = 59;
            (*hours)--;
        }
    }

//? Helper: Stopwatch tick
void countup_tick(int *hours, int *minutes, int *seconds) {
    sleep(1); // wait one second
    (*seconds)++;
    if (*seconds >= 60) {
        *seconds = 0;
        (*minutes)++;
        if (*minutes >= 60) {
            *minutes = 0;
            (*hours)++;
        }
    }
}

void get_time_input_confirmed(const char *label, int *value, int min, int max, app_state_t next_state, app_state_t *current_state) {
    printf("\n[%s] Enter %s (%d-%d): ", label, label, min, max);
    scanf("%d", value);
    getchar(); // Consume newline

    if (*value < min) *value = min;
    if (*value > max) *value = max;

    printf("You entered %02d %s.\n", *value, label);

    print_key_directions();
    int key = get_key_input();
    if (key == 1) {
        *current_state = next_state;
    } else if (key == 0) {
        *current_state = STATE_IDLE;
    } else if (key == 3) {
        *current_state = STATE_EXIT;
    } else {
        printf("Invalid key. Try again. \n");
    }
}

// HAL and Peripherals
hal_map_t hal;
button_handle_t btn;
switch_handle_t sw;
hex_display_handle_t hex;

int main(void) {

    if (hal_open(&hal) != 0) {
        printf(" HAL Initialization failed.\n");
        return -1;
    }
    button_init(&btn, &hal);
    switch_init(&sw, &hal);
    hex_display_init(&hex, &hal);

    printf("[SYSTEM] Simulation mode active - peripherals initialized.\n\n");

    #if SIMULATION_MODE
    int fake_val = switch_read_input_value(&sw);
    int mode = switch_read_mode(&sw);
    printf("[TEST] Simulated Switch Input Value: %d | Mode: %s\n", 
            fake_val, (mode == SWITCH_MODE_STOPWATCH) ? "STOPWATCH" : "COUNTDOWN");

    int key = button_get_simulated_key();
    printf("[TEST] Simulated Button Pressed KEY%d\n", key);

    hex_display_write(&hex, 123456);
    sleep(1);
    hex_display_clear_all(&hex);
    #endif

    app_state_t current_state = STATE_IDLE;
    int running = 1;

    printf("Embedded App Milestone 2 (Skeleton/Structure\n\n===TIMER SIMULATION===\n");

    while (running) {
        switch (current_state) {
            case STATE_IDLE:
            printf("Select mode:\n1. Countdown Timer\n2. Stopwatch\n3. Exit\n Enter Choice: ");
            int choice;
            scanf("%d", &choice);
                if (choice == 1) {
                    printf("Time: 00:00:00");
                    hours = minutes = seconds = 0;
                    current_state = STATE_SET_HOURS;
                } else if (choice == 2) {
                    hours = minutes = seconds = 0;
                    current_state = STATE_STOPWATCH_READY;
                } else if (choice == 3 ) {
                    current_state = STATE_EXIT;
                } else {
                    printf("Invalid Choice. Try Again. \n");
                }
                break;
        
            case STATE_SET_HOURS:
                get_time_input_confirmed("Hours", &hours, 0, 99, STATE_SET_MINUTES, &current_state);
                break;
            
            case STATE_SET_MINUTES:
                get_time_input_confirmed("Minutes", &minutes, 0, 59, STATE_SET_SECONDS, &current_state);
                break;

            case STATE_SET_SECONDS:
                get_time_input_confirmed("Seconds", &seconds, 0, 59, STATE_COUNTDOWN_READY, &current_state);
                break;

            case STATE_COUNTDOWN_READY:
                printf("\n[STATE_READY] Timer staged: ");
                print_time();
                print_key_directions();
                {
                int input = get_key_input();
                if (input == 2 || input == 1) current_state = STATE_COUNTDOWN_RUNNING;
                else if (input == 0) current_state = STATE_IDLE;
                else if (input == 3) current_state = STATE_EXIT;
                }
                break;
            
            case STATE_COUNTDOWN_RUNNING:
                printf("\n[STATE_COUNTDOWN_RUNNING] Countdown started...\n");
                
                while (current_state == STATE_COUNTDOWN_RUNNING) {
                    countdown_tick(&hours, &minutes, &seconds);

                    printf("\rCountdown: %02d:%02d:%02d", hours, minutes, seconds);
                    fflush(stdout);

                    if (hours == 0 && minutes == 0 && seconds == 0){
                        printf("\nTimer Finished!\n");
                        current_state = STATE_COUNTDOWN_FINISHED;
                        break;
                    }

                    //check if user pressed a key
                    if (kbhit()) {
                        int input = get_key_input();
                        if (input == 2) current_state = STATE_COUNTDOWN_PAUSED;
                        else if (input == 0) current_state = STATE_IDLE;
                        else if (input == 3) current_state = STATE_EXIT;
                    }
                }
                break;

            case STATE_COUNTDOWN_PAUSED:
                printf("\n[STATE_COUNTDOWN_PAUSED] Timer paused: ");
                print_time();
                print_key_directions();
                {
                    int input = get_key_input();
                    if (input == 1 || input == 2) current_state = STATE_COUNTDOWN_RUNNING;
                    else if (input == 0) current_state = STATE_IDLE;
                    else if (input == 3) current_state = STATE_EXIT;
                }
                break;

            case STATE_COUNTDOWN_FINISHED:
                printf("\n[STATE_COUNTDOWN_FINISHED] Countdown complete!\n");
                print_time();
                print_key_directions();
                {
                    int input = get_key_input();
                    if (input == 0 || input == 1) current_state = STATE_IDLE;
                    else if (input == 3) current_state = STATE_EXIT;
                }
                break;
            
            case STATE_STOPWATCH_READY:
                printf("\n[STATE_STOPWATCH_READY] Stopwatch Ready.\n");
                print_key_directions();
                {
                    int input = get_key_input();
                    if (input == 1 || input == 2) current_state = STATE_STOPWATCH_RUNNING;
                    else if (input == 0) current_state = STATE_IDLE;
                    else if (input == 3) current_state = STATE_EXIT;
                }
                break;

            case STATE_STOPWATCH_RUNNING:
                printf("\n [STATE_STOPWATCH_RUNNING] Stopwatch running...\n");
                //loop until paused/quit
                while (current_state == STATE_STOPWATCH_RUNNING) {
                    // Check for input
                    if (kbhit()) {
                        int input = get_key_input();
                        if (input == 2) current_state = STATE_STOPWATCH_PAUSED;
                        else if (input == 0) current_state = STATE_IDLE;
                        else if (input == 3) current_state = STATE_EXIT;
                    }
                    countup_tick(&hours, &minutes, &seconds);
                    printf("\rStopwatch: %02d:%02d:%02d", hours, minutes, seconds);
                    fflush(stdout);
                }   
                break;

            case STATE_STOPWATCH_PAUSED:
                printf("\n[STATE_STOPWATCH_PAUSED] Stopwatch Paused at ");
                print_time();
                print_key_directions();

                {
                    int input = get_key_input();
                    if (input == 2) current_state = STATE_STOPWATCH_RUNNING;
                    else if (input == 0) current_state = STATE_IDLE;
                    else if (input == 3) current_state = STATE_EXIT;
                }
                break;

            case STATE_EXIT: 
                running = 0;
                break;
            }
    } 
    
    printf("\nExiting Simulation...\n");
    button_cleanup(&btn);
    switch_cleanup(&sw);
    hex_display_cleanup(&hex);
    hal_close(&hal);
    return 0;
}