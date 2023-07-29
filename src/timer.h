#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int perf_zero[2];
} Timer;

void timer_reset(Timer *t);
int  timer_us(Timer *t);
int  timer_ms(Timer *t);
int  timer_usreset(Timer *t);
int  timer_msreset(Timer *t);

#ifdef __cplusplus
};
#endif
