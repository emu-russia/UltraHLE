
#pragma once

char* init_name();
int init_fullscreen(int a1);
int init_query();
int init_reinit();
int init_init();
void init_deinit();
void init_activate();
void init_resize(int a1, int a2);
int init_bufferswap();
int init_clear(int a1, int a2, float a3, float a4, float a5);
int init_readfb(__int16 a1, int a2, int a3, int a4, int a5, int a6, int a7);
int init_writefb();
int mode_init();
int mode_texturemode(int a1, __int16 a2, int a3);
int mode_loadtexture(int a1);
int mode_loadmultitexture(int a1, int a2);
void fixfogtable(GrFog_t* a1, int a2);
GrFog_t* generatefogtable();
void mode_change();
