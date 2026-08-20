
#pragma once

// Initialization / display management (fx.c, OpenGL backend)
char* init_name();
int init_fullscreen(int fullscreen);
int init_query();
void init_reinit();
int init_init();
void init_deinit();
void init_activate();
void init_resize(int xs, int ys);
void init_bufferswap();
void init_clear(int writecolor, int writedepth, float cr, float cg, float cb);
int init_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);
int init_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);

// Pixel pipeline modes (fx.c, OpenGL backend)
void mode_init();
void mode_texturemode(int tmu, int format, int trilin);
void mode_loadtexture(int txtind);
void mode_loadmultitexture(int txtind1, int txtind2);
void mode_change();
