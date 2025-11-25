#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

typedef struct {
    int hours;
    int minutes;
    int seconds;
    int running;   // 1 = running, 0 = paused
} soft_timer_t;

void soft_timer_init(soft_timer_t *t);
void soft_timer_set(soft_timer_t *t, int h, int m, int s);
void soft_timer_start(soft_timer_t *t);
void soft_timer_pause(soft_timer_t *t);
void soft_timer_reset(soft_timer_t *t);

// Count up 1 second (stopwatch)
void soft_timer_tick_up(soft_timer_t *t);

// Count down 1 second (countdown)
void soft_timer_tick_down(soft_timer_t *t);

int soft_timer_is_finished(const soft_timer_t *t);

#endif
