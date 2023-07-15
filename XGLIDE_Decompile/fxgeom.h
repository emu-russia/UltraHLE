
#pragma once

#define XFORM_MODE_FRUSTUM 0
#define XFORM_MODE_ORTHO 1
#define XFORM_MODE_PROJECT 2
#define XFORM_MODE_NONE 3

void geom_init();
void dumpmatrix(float* f, char* name);
void recalc_projection();
void vertexdata(xt_data* data);
void xform(xt_xfpos* dst, xt_pos* src, int count, char* mask);
int setuprvx(int a1, int a2);
void x_vxrel(xt_pos* a1, xt_data* a2);
void clear();
int doclipvertex(signed int a1, int a2, int a3);
int doclip(signed int a1);
int clipfinish(DWORD* a1);
int clippoly(int a1, int a2, int* a3, DWORD* a4);
int docliplineend(int a1, int a2);
int clipline(int a1, int a2, DWORD* a3);
int splitpoly(int a1, int a2, DWORD* a3);
void flush_reordertables();
int flush_drawfx();
