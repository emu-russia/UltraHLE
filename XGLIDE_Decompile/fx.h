
#pragma once

char* init_name();
int init_fullscreen(int a1);
int init_query();
int init_reinit();
signed int init_init();
void init_deinit();
void init_activate();
void init_resize(int a1, int a2);
int init_bufferswap();
int init_clear(int a1, int a2, float a3, float a4, float a5);
signed int init_readfb(__int16 a1, int a2, int a3, int a4, int a5, int a6, int a7);
signed int init_writefb();
int mode_init();
int mode_texturemode(int a1, __int16 a2, int a3);
int mode_loadtexture(int a1);
int mode_loadmultitexture(int a1, int a2);
void fixfogtable(int a1, int a2);
void* generatefogtable();
void mode_change();
