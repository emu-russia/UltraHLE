
#pragma once

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
void mode_init();
void mode_texturemode(int a1, __int16 a2, int a3);
void mode_loadtexture(int a1);
void mode_loadmultitexture(int a1, int a2);
void fixfogtable(GrFog_t* a1, int a2);
GrFog_t* generatefogtable();
void mode_change();
