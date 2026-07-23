
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
int setuprvx(int first, int count);
void x_vxrel(xt_pos* p, xt_data* d);
void clear();
int doclipvertex(int bit, int vi, int vo);
void doclip(int bit);
int clipfinish(int* vx);
int clippoly(int clor, int vxn, int* v, int* out);
int docliplineend(int v1, int v2);
int clipline(int v1, int v2, int* out);
int splitpoly(int a1, int a2, DWORD* a3);
void flush_reordertables();
int flush_drawfx();

extern int allxformed;
extern int state;
extern int mode;
extern int vertices;
extern int vertices_base;
extern int corners_base;
extern int corners;
extern int corner[0x200 * 5];
extern GrVertex grvx[0x200];
extern xt_pos pos[0x200];
extern xt_tex tex[0x200];
extern xt_tex tex2[0x200];
extern xt_tex texp[0x200];
extern xt_xfpos xfpos[0x200];
extern uint8_t xformed[0x200];