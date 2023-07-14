
#pragma once

#define X_FIRSTBLEND X_ZERO
#define X_FIRSTCOMBINE X_WHITE

#define X_MAXSTATES 2			// Number of available states. The 0th state is not used

extern xt_state g_state[X_MAXSTATES];
extern xt_stats g_stats;

extern int g_activestateindex;
extern xt_state* g_activestate;

//#define XST g_activestateindex
#define XST 1			// For the Glide version, only a single state (1) is always used

xt_texture* texture_get(int t);
int x_texture_getinfo(int handle, int* format, int* memformat, int* width, int* height);
