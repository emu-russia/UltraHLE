
#pragma once

#define X_FIRSTBLEND X_ZERO
#define X_FIRSTCOMBINE X_WHITE

extern int g_activestateindex;

xt_texture* texture_get(int t);
int x_texture_getinfo(int handle, int* format, int* memformat, int* width, int* height);
