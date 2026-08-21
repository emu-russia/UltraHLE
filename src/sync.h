#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the synchronization timers.
 */
void sync_init(void);
/**
 * Entry point of the synchronization thread.
 */
void sync_thread(void);
/**
 * Called by the RDP when a new graphics frame is complete.
 */
void sync_gfxframedone(void);
/**
 * Handles a video retrace, swapping the framebuffer and synchronizing audio.
 */
void sync_retrace(void);
/**
 * Periodically checks whether a retrace is due and triggers it.
 */
void sync_checkretrace(void);

#ifdef __cplusplus
};
#endif
