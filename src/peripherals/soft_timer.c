#include "../../includes/peripherals/soft_timer.h"

void soft_timer_init(soft_timer_t *t) {
    t->hours = 0;
    t->minutes = 0;
    t->seconds = 0;
    t->running = 0;
}

void soft_timer_set(soft_timer_t *t, int h, int m, int s) {
    t->hours = h;
    t->minutes = m;
    t->seconds = s;
}

void soft_timer_start(soft_timer_t *t) {
    t->running = 1;
}

void soft_timer_pause(soft_timer_t *t) {
    t->running = 0;
}

void soft_timer_reset(soft_timer_t *t) {
    t->hours = 0;
    t->minutes = 0;
    t->seconds = 0;
    t->running = 0;
}

void soft_timer_tick_up(soft_timer_t *t) {
    if (!t->running) return;

    t->seconds++;
    if (t->seconds >= 60) {
        t->seconds = 0;
        t->minutes++;
        if (t->minutes >= 60) {
            t->minutes = 0;
            t->hours++;
            if (t->hours > 99) t->hours = 99; // clamp
        }
    }
}

void soft_timer_tick_down(soft_timer_t *t) {
    if (!t->running) return;
    if (t->hours == 0 && t->minutes == 0 && t->seconds == 0) return;

    t->seconds--;
    if (t->seconds < 0) {
        t->seconds = 59;
        t->minutes--;
        if (t->minutes < 0) {
            t->minutes = 59;
            t->hours--;
            if (t->hours < 0) t->hours = 0;
        }
    }
}

int soft_timer_is_finished(const soft_timer_t *t) {
    return (t->hours == 0 && t->minutes == 0 && t->seconds == 0);
}
