
#pragma once

void geom_init();
void dumpmatrix(float* a1, int a2);
int recalc_projection();
void vertexdata(xt_data* a1);
void xform(int a1, xt_pos* a2, int a3, char* a4);
int setuprvx(int a1, int a2);
void x_vxrel(float* a1, float* a2);
void clear();
int doclipvertex(signed int a1, int a2, int a3);
int doclip(signed int a1);
int clipfinish(DWORD* a1);
int clippoly(int a1, int a2, int* a3, DWORD* a4);
int docliplineend(int a1, int a2);
signed int clipline(int a1, int a2, DWORD* a3);
signed int splitpoly(int a1, int a2, DWORD* a3);
void flush_reordertables();
signed int flush_drawfx();
