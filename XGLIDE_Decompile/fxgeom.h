
#pragma once

void geom_init();
BYTE* x_cameramatrix(BYTE* a1);
void x_getmatrix(void* a1);
int dumpmatrix(float* a1, int a2);
void x_matrix(void* a1);
int recalc_projection(int a1);
int x_begin(int a1);
void x_end();
int vertexdata(float* a1);
void xform(int a1, float* a2, int a3, char* a4);
int* setuprvx(int a1, int a2);
int x_vx(float* a1, float* a2);
int x_vxa(int a1, float* a2);
int x_vxrel(float* a1, float* a2);
void x_vxarray(float* a1, int a2, char* a3);
int clear();
int doclipvertex(signed int a1, int a2, int a3);
int doclip(signed int a1);
signed int clipfinish(DWORD* a1);
signed int clippoly(int a1, int a2, int* a3, DWORD* a4);
int docliplineend(int a1, int a2);
signed int clipline(int a1, int a2, DWORD* a3);
signed int splitpoly(int a1, int a2, DWORD* a3);
void* flush_reordertables();
signed int flush_drawfx();
