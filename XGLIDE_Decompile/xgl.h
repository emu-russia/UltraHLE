#ifndef _XGL_H_
#define _XGL_H_

#include <GL/gl.h>
//#include <GL/glext.h>

#ifdef __cplusplus
extern "C" {
#endif

// OpenGL-specific extensions and function pointers
typedef void (APIENTRY *PFNGLACTIVETEXTUREARBPROC)(GLenum);
typedef void (APIENTRY *PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2FARBPROC)(GLenum, GLfloat, GLfloat);

// Global OpenGL state
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;

// Initialize OpenGL subsystem
int xgl_init(void* hdc, void* hwnd, int width, int height);
void xgl_deinit(void);
void xgl_resize(int width, int height);
void xgl_select_context(void);
void xgl_clear(int writecolor, int writedepth, float cr, float cg, float cb);
int xgl_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);
int xgl_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen);
void xgl_bufferswap(void);

// Texture handling for OpenGL
int xgl_createtexture(int handle, int format, int width, int height);
int xgl_loadtexturelevel(int handle, int level, char* data);
void xgl_freetexture(int handle);
void xgl_cleartexmem(void);
int xgl_gettextureinfo(int handle, int* format, int* memformat, int* width, int* height);
unsigned char* xgl_opentexturedata(int handle);
void xgl_closetexturedata(int handle);

// Mode setting for OpenGL
void xgl_mode_init(void);
void xgl_mode_change(void);
void xgl_mode_texturemode(int tmu, int format, int trilin);
void xgl_mode_loadtexture(int txtind);
void xgl_mode_loadmultitexture(int txtind1, int txtind2);

// Geometry handling for OpenGL
void xgl_geom_init(void);
void xgl_vertexdata(xt_data* data);
void xgl_xform(xt_xfpos* dst, xt_pos* src, int count, char* mask);
void xgl_flush(void);
void xgl_begin(int type);
void xgl_end(void);
void xgl_vx(xt_pos* pos, xt_data* data);
void xgl_vxa(int arrayindex, xt_data* data);
void xgl_vxarray(xt_pos* pos, int size, char* mask);

// Utility functions
void xgl_log(char* txt, ...);
void xgl_fatal(char* txt, ...);
void* xgl_alloc(int size);
void* xgl_allocfast(int size);
void* xgl_realloc(void* p, int newsize);
void xgl_free(void* blk);
void xgl_fastfpu(int fast);

// Timer functions
void xgl_timereset(void);
int xgl_timems(void);
int xgl_timeus(void);
void xgl_sleep(int ms);

extern GLuint g_opengl_texture_ids[MAXTEXTURES];

#ifdef __cplusplus
}
#endif

#endif // _XGL_H_
