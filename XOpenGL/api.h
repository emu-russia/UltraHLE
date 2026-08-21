
#pragma once

#define X_FIRSTBLEND X_ZERO
#define X_FIRSTCOMBINE X_WHITE

#define X_MAXSTATES 2			// Number of available states. The 0th state is not used

/** Array of driver states; entry 0 is unused. */
extern xt_state g_state[X_MAXSTATES];
/** Global driver statistics counters. */
extern xt_stats g_stats;

/** Index of the currently active state. */
extern int g_activestateindex;
/** Pointer to the currently active state. */
extern xt_state* g_activestate;

//#define XST g_activestateindex
#define XST 1			// For the Glide version, only a single state (1) is always used

/**
 * Resolves a texture handle to its xt_texture entry.
 * @param t Texture handle to resolve.
 * @return Pointer to the texture, or NULL if the handle is invalid or undefined.
 */
xt_texture* texture_get(int t);
/**
 * Retrieves information about a texture.
 * @param handle Texture handle to query.
 * @param format Output for the texture format, may be NULL.
 * @param memformat Output for the memory format, may be NULL.
 * @param width Output for the texture width, may be NULL.
 * @param height Output for the texture height, may be NULL.
 * @return 0 on success, 1 if the handle is invalid.
 */
int x_texture_getinfo(int handle, int* format, int* memformat, int* width, int* height);
