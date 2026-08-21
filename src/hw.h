// Hardware and Memory mapped io emulation (devices)

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes memory-mapped IO registers that must not start at zero.
 */
void    hw_init(void);

/**
 * Schedules and runs the periodic hardware checks.
 */
void    hw_check(void);
/**
 * Handles the vertical retrace event.
 */
// these in cpu.c temporarily
void    hw_retrace(void);
/**
 * Handles the graphics-frame-done event.
 */
void    hw_gfxframedone(void);

/**
 * Checks pending RSP tasks when direct SP execution is disabled.
 */
void    hw_rspcheck(void);

/**
 * Detects and handles pending memory-mapped IO accesses.
 */
void    hw_memio(void);
/**
 * Checks whether a memory page should be reported to hw_memio.
 * @param addr 64K page address to check.
 * @return 1 if the page contains handled IO registers, 0 otherwise.
 */
int     hw_ismemiorange(uint32_t addr);

/**
 * Runs the graphics task in a separate thread.
 */
void    hw_gfxthread(void);

/**
 * Selects which controller is reported on SI pad reads.
 * @param pad Controller index.
 */
void    hw_selectpad(int pad);

#ifdef __cplusplus
};
#endif
