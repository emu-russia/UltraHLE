#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * High-resolution timer based on the Windows performance counter.
 */
typedef struct
{
    /** Last performance counter reading used as the timer reference point. */
    int perf_zero[2];
} Timer;

/**
 * Resets the timer, recording the current performance counter value. Initializes the performance counter frequency on first use.
 * @param t Timer to reset.
 */
void timer_reset(Timer *t);
/**
 * Returns the time elapsed since the timer was reset, in microseconds.
 * @param t Timer to query.
 * @return Elapsed time in microseconds.
 */
int  timer_us(Timer *t);
/**
 * Returns the time elapsed since the timer was reset, in milliseconds.
 * @param t Timer to query.
 * @return Elapsed time in milliseconds.
 */
int  timer_ms(Timer *t);
/**
 * Returns the time elapsed since the timer was reset, in microseconds, and resets the timer.
 * @param t Timer to query and reset.
 * @return Elapsed time in microseconds.
 */
int  timer_usreset(Timer *t);
/**
 * Returns the time elapsed since the timer was reset, in milliseconds, and resets the timer.
 * @param t Timer to query and reset.
 * @return Elapsed time in milliseconds.
 */
int  timer_msreset(Timer *t);

#ifdef __cplusplus
};
#endif
