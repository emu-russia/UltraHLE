#ifndef _X_H_
#define _X_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef EXPORT
#define EXPORT extern
#endif

/****************************************************************************
** misc stuff
*/

#pragma warning(disable:4244)
#pragma warning(disable:4305)

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

#define PI 3.1415926535897932384626

#define X_RGB(r,g,b)    ( (((r)<<16) & 0xff0000) | (((g)<<8) & 0xff00) | (((b)<<0) & 0xff) )
#define X_RGBA(r,g,b,a) ( (((a)<<24) & 0xff000000) | (((r)<<16) & 0xff0000) | (((g)<<8) & 0xff00) | (((b)<<0) & 0xff) )
#define X_B(c)          ( ((c)&0xff) >> 0 )
#define X_G(c)          ( ((c)&0xff00) >> 8 )
#define X_R(c)          ( ((c)&0xff0000) >> 16 )

#define X_DEG(a)      ((a)*(65536.0/360.0))

#define X_RAD(a)      ((a)*(32768.0/PI))

#define MAXTEXTURES 1024

/****************************************************************************
** public structs
*/

/**
 * Input vertex data for the geometry pipe.
 */
// input to geometry pipe

typedef struct
{
    int   flags;
    float r,g,b,a;
    float t1s,t1t,t1w;
    float t2s,t2t,t2w;
} xt_data;

/**
 * Position input to the geometry pipe, stored as a vector or x/y/z triple.
 */
typedef struct
{
    union
    {
        struct
        {
            float x,y,z;
        };
        float v[3];
    };
} xt_pos;

/**
 * Transformed position after geometry processing.
 */
typedef struct
{
    float x,y,z;
    float invz;
    int   clip;
} xt_xfpos;

#define X_CLIPX1   0x00000100
#define X_CLIPX2   0x00000200
#define X_CLIPY1   0x00000400
#define X_CLIPY2   0x00000800
#define X_CLIPZ1   0x00001000
#define X_CLIPZ2   0x00002000

/**
 * Texture coordinate pair.
 */
typedef struct
{
    float s,t;
} xt_tex;

/**
 * Matrix used by the xm_/xv_ routines, with 4x4 elements and a type tag.
 */
// types for matrix/vector (xm_, xv_) routines

typedef struct
{
    float  xax,yax,zax,  x;
    float  xay,yay,zay,  y;
    float  xaz,yaz,zaz,  z;
    float  r1 ,r2 ,r3 , r4;
    int    type;
} xt_matrix;
#define X_MATRIX_IDENT 0
#define X_MATRIX_XFORM 1
#define X_MATRIX_4X4   2

typedef xt_pos xt_vector;

/**
 * Driver statistics counters reported by x_getstats.
 */
typedef struct
{
    // timing
    int   frametime; // us
    // geometry input
    int   in_vx;
    int   in_tri;    // quads count as 2 etc
    // draw output (feedback not included)
    int   out_tri;
    // state changes
    int   chg_mode;  // mode changes
    int   chg_text;  // texture changes
    // texture
    int   text_total;    // bytes in host memory
    int   text_uploaded; // bytes uploaded since last call
    int   text_resident; // bytes resident last frame
    int   text_used;     // bytes used this last frame
    // extra space
    int   RESERVED[16];
} xt_stats;

/****************************************************************************
** xmain.c
**
** - first call x_init() and before shutdown call x_deinit()
** - x_version returns a description text for the driver
** - x_clear can be used to clear the screen
** - x_finish should be called at end of frame (flips buffers etc)
**
** 3DFX:
** - init screen with call to x_open (specify res)
** - deinit screen with call to x_close
** - x_resize and x_select have no effect
**
** OPENGL:
** - init with call to x_open, note returned context handle
** - if window is resized, call x_resize
** - close context with x_close (use right handle!)
** - if multiple contexts, x_select selects the active one
*/

/**
 * Initializes the graphics driver and opens the log file.
 */
EXPORT void    x_init(void);
/**
 * Deinitializes the graphics driver, closing all open contexts and the log file.
 */
EXPORT void    x_deinit(void);
/**
 * Returns a description text for the driver.
 * @return Pointer to a static buffer with the description string of the driver version.
 */
EXPORT char   *x_version(void);

/**
 * Checks whether the driver supports the given device and window.
 * @param hdc Handle to the device context.
 * @param hwnd Handle to the window.
 * @return Always 0; the query is not implemented.
 */
EXPORT int     x_query(void *hdc,void *hwnd);
#define X_VOODOO1 0x01
#define X_VOODOO2 0x02
/**
 * Opens a rendering window or fullscreen mode with the given resolution.
 * @param hdc Handle to the device context.
 * @param hwnd Handle to the window.
 * @param width Requested screen width.
 * @param height Requested screen height.
 * @param buffers Number of back buffers requested.
 * @param vsync Non-zero to enable vertical sync.
 * @return Index of the new state on success, -1 on failure.
 */
EXPORT int     x_open(void *hdc,void *hwnd,int width,int height,int buffers,int vsync);
/**
 * Selects the active rendering context.
 * @param which Context handle to select, or 0 to deactivate.
 */
EXPORT void    x_select(int which);
/**
 * Resizes the rendering surface.
 * @param width New surface width.
 * @param height New surface height.
 * @note Library entry point called by the emulator on window resizes.
 */
EXPORT void    x_resize(int width,int height);
/**
 * Closes a rendering context and releases its resources.
 * @param which Context handle to close.
 */
EXPORT void    x_close(int which);
/**
 * Switches between fullscreen and windowed mode.
 * @param fullscreen Non-zero to enter fullscreen mode.
 */
EXPORT void    x_fullscreen(int fullscreen);

/**
 * Clears the color and/or depth buffers.
 * @param writecolor Non-zero to clear the color buffer.
 * @param writedepth Non-zero to clear the depth buffer.
 * @param cr Red clear color component.
 * @param cg Green clear color component.
 * @param cb Blue clear color component.
 */
EXPORT void    x_clear(int writecolor,int writedepth,float cr,float cg,float cb);
/**
 * Reads a rectangle of pixels from a framebuffer.
 * @param fb Framebuffer source (X_FB_RGB565 or X_FB_RGBA8888 format plus X_FB_* buffer flag).
 * @param x Left edge of the rectangle.
 * @param y Top edge of the rectangle.
 * @param xs Width of the rectangle.
 * @param ys Height of the rectangle.
 * @param buffer Destination buffer for the pixel data.
 * @param bufrowlen Row length of the destination buffer in bytes.
 * @return 0 on success, 1 on invalid parameters.
 */
EXPORT int     x_readfb(int fb,int x,int y,int xs,int ys,char *buffer,int bufrowlen);
/**
 * Writes a rectangle of pixels into a framebuffer.
 * @param fb Framebuffer destination (X_FB_RGB565 or X_FB_RGBA8888 format plus X_FB_* buffer flag).
 * @param x Left edge of the rectangle.
 * @param y Top edge of the rectangle.
 * @param xs Width of the rectangle.
 * @param ys Height of the rectangle.
 * @param buffer Source buffer with the pixel data.
 * @param bufrowlen Row length of the source buffer in bytes.
 * @return 0 on success, 1 on invalid parameters.
 */
EXPORT int     x_writefb(int fb,int x,int y,int xs,int ys,char *buffer,int bufrowlen);
#define        X_FB_RGB565   0x11   // 16 bit rgb color
#define        X_FB_RGBA8888 0x12   // 32 bit rgba color
#define        X_FB_FRONT    0x100  // from front buffer
#define        X_FB_BACK     0x200  // from back buffer
/**
 * Finishes the current frame, flipping buffers and resetting the state.
 */
EXPORT void    x_finish(void);

/**
 * Copies the current driver statistics into a caller-provided structure.
 * @param s Destination for the statistics, or NULL to only reset the counters.
 * @param ssize Size of the destination structure in bytes.
 */
EXPORT void    x_getstats(xt_stats *s,int ssize);

/* Utilities (xmain.c) - mostly used internally
**
** - x_fastfpu(1) sets the fpu to 32-bit accuracy mode (faster).
** - x_fastfpu(0) returns the fpu to mode before call to x_fastfpu(1)
** - the above calls can be nested
**
** - x_timems and x_timeus can be used for profiling etc. They return
**   times from the Pentium Performance Counters in milliseconds and
**   microseconds (respectively). Only use for timing intervals.
**   These do take a a few thousand cycles, so don't time small loops.
**
** - x_log is a simple way to output to the file 'x.log'
**   Used for errors and for debugging
*/

/**
 * Prints a fatal error message to x.log, closes the active context, and terminates the driver.
 * @param txt Format string for the message.
 * @param ... Arguments for the format string.
 */
EXPORT void    x_fatal(char *txt,...);
/**
 * Appends a formatted message to the file x.log.
 * @param txt Format string for the message.
 * @param ... Arguments for the format string.
 */
EXPORT void    x_log(char *txt,...);
/**
 * Allocates a cleared block of memory.
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory.
 */
EXPORT void   *x_alloc(int size); // clears memory
/**
 * Allocates an uncleared block of memory.
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory.
 */
EXPORT void   *x_allocfast(int size); // no clearing
/**
 * Resizes an allocated block of memory without clearing it.
 * @param p Pointer to the block to resize, or NULL to allocate a new block.
 * @param newsize New size in bytes.
 * @return Pointer to the resized memory.
 * @note Non-positive sizes are treated as 1.
 */
EXPORT void   *x_realloc(void *p,int newsize); // no clearing
/**
 * Frees a block of memory allocated by x_alloc/x_allocfast/x_realloc.
 * @param blk Pointer to the block to free.
 */
EXPORT void    x_free(void *blk);
/**
 * Sets the FPU to 32-bit accuracy mode when fast is non-zero, restoring the previous mode otherwise.
 * @param fast Non-zero for fast 32-bit FPU mode.
 */
EXPORT void    x_fastfpu(int fast);

/**
 * Resets the timing counters used by x_timems and x_timeus.
 */
EXPORT void    x_timereset(void);
/**
 * Returns the time in milliseconds since the last x_timereset.
 * @return Elapsed time in milliseconds.
 */
EXPORT int     x_timems(void);
/**
 * Returns the time in microseconds since the last x_timereset.
 * @return Elapsed time in microseconds.
 */
EXPORT int     x_timeus(void);
/**
 * Sleeps for the given number of milliseconds.
 * @param ms Number of milliseconds to sleep.
 */
EXPORT void    x_sleep(int ms);

/****************************************************************************
** xmode.c
**
** All nongeometry settings are controlled with modes. A mode describes
** the full state of the 3D-card. All modes have to be coded into the
** engine, so just pick a free mode number and add the code to xmode.c
**
** This means every mode has to be done separately for every driver,
** but also makes it easy to optimize the modes separately to use the
** features of each card. Also, a mode usually requires a certain amount
** of textures and data (color, alpha, coordinates) to work correctly.
**
** There is a separate mode call for no textures, 1 textures or 2 textures.
** The point here is that the engine wants to know ALL the textures at
** the same time to properly setup multiple tmus. If only one texture
** changes from the last call, only that is respecified to the hardware.
**
*/

/**
 * Resets all rendering modes to their default settings.
 */
// this sets default settings
EXPORT void    x_reset(void);

/**
 * Forces geometry processing on or off regardless of the current mode.
 * @param forceon Non-zero to force geometry processing on.
 * @param forceoff Non-zero to force geometry processing off.
 */
// flags
EXPORT void    x_forcegeometry(int forceon,int forceoff);
/**
 * Sets the geometry processing flags, applying the forced on and off masks.
 * @param flags Bitmask of X_* geometry flags.
 */
EXPORT void    x_geometry(int flags);
#define X_WIRE       0x00000001 // for debugging, not fast
#define X_CULLFRONT  0x00000002 // NOTE: culling done with glide
#define X_CULLBACK   0x00000004 // so no geometry savings yet
#define X_NOCLIP     0x00000008
#define X_PROJTEXT   0x00000010 // projective texture 1
#define X_DUMPDATA   0x00000020 // debugdump (for full projection only)

// These control rendering settings:
// - all return 0 if mode supported, 1 otherwise
// - in practice all modes we use WILL be supported, somehow :)
// - combine2 and texture2 always return 1 if only one tmu present

/**
 * Sets the color and depth write masks and the depth test mode.
 * @param colormask X_ENABLE or X_DISABLE for color writes.
 * @param depthmask X_ENABLE or X_DISABLE for depth writes.
 * @param depthtest X_ENABLE, X_DISABLE or one of the X_TEST* comparison modes.
 * @return Zero if the mode is supported, one otherwise.
 */
EXPORT int     x_mask(int colormask,int depthmask,int depthtest);
#define X_ENABLE      0x1001
#define X_DISABLE     0x1002
// these also allowed for depthtest
#define X_TESTEQ      0x10d1
#define X_TESTNE      0x10d2
#define X_TESTGE      0x10d3
#define X_TESTLE      0x10d4
#define X_TESTGT      0x10d5
#define X_TESTLT      0x10d6

/**
 * Sets the dithering mode.
 * @param type X_ENABLE or X_DISABLE.
 * @return Zero if the mode is supported, one otherwise.
 */
EXPORT int     x_dither(int type); // ENABLE / DISABLE

/**
 * Sets the fog mode and parameters.
 * @param type X_LINEAR, X_EXPONENTIAL or X_DISABLE.
 * @param min Fog start distance for linear fog.
 * @param max Fog end distance for linear fog, 90 percent density distance for exponential fog.
 * @param r Fog color red component.
 * @param g Fog color green component.
 * @param b Fog color blue component.
 * @return Zero if the mode is supported, one otherwise.
 */
// Fog settings:
// for X_LINEAR   min/max = eye-z-distances for 0% and 100% of fog
// for X_EXPONENTIAL  max = distance with 90% fog density, min ignored
EXPORT int     x_fog(int type,float min,float max,float r,float g,float b);
//      X_DISABLE
#define X_LINEAR      0x1f01
#define X_EXPONENTIAL 0x1f02
#define X_LINEARADD   0x1f03 // second pass add mode

/**
 * Sets the depth buffer z-range mapping.
 * @param znear Z value mapped to the near depth.
 * @param zfar Z value mapped to the far depth.
 * @return Always 0.
 */
EXPORT int     x_zrange(float znear,float zfar);
/**
 * Sets the depth decal factor used for polygon offsetting.
 * @param factor Decal factor.
 * @return Always 0.
 */
EXPORT int     x_zdecal(float factor);

/**
 * Sets the source and destination blend factors.
 * @param src Source blend factor (X_* blend constant).
 * @param dst Destination blend factor (X_* blend constant).
 * @return Zero if the mode is supported, one otherwise.
 */
EXPORT int     x_blend(int src,int dst);
#define X_ZERO          0x1201
#define X_ONE           0x1202
#define X_OTHER         0x1203
#define X_ALPHA         0x1204
#define X_OTHERALPHA    0x1205
#define X_INVOTHER      0x1206
#define X_INVALPHA      0x1207
#define X_INVOTHERALPHA 0x1208
#define X_LASTBLEND     0x1208

/**
 * Sets the alpha test limit; values >= 1.0 disable the test.
 * @param limit Alpha reference value.
 * @return 0 on success, 1 if the limit is out of range.
 */
EXPORT int     x_alphatest(float limit); // >=1.0 disabled

/**
 * Sets the single-texture combine mode.
 * @param colortext1 Color combine mode (X_* combine constant).
 * @return Zero if the mode is supported, one otherwise.
 */
EXPORT int     x_combine(int colortext1); // single texture
/**
 * Sets the dual-texture combine mode.
 * @param colortext1 Color combine mode for the first texture.
 * @param text1text2 Combine mode between texture 1 and texture 2.
 * @param sametex Non-zero to use the same texture for both stages.
 * @return 0 on success, 1 if unsupported or invalid.
 */
EXPORT int     x_combine2(int colortext1,int text1text2,int sametex); // dual texture
#define X_WHITE         0x1301 // result is white
#define X_COLOR         0x1302 // just the gouraud rgba
#define X_TEXTURE       0x1303 // just the texture
#define X_ADD           0x1304 // gouraud + texture
#define X_MUL           0x1305 // gouraud * texture
#define X_DECAL         0x1306 // blend(gouraud,texture,texturealpha)
#define X_MULADD        0x1307 // gouraud * texture + gouraud [temp!]
#define X_TEXTURE_IA    0x1308 // texture, alpha=iterated
#define X_MUL_TA        0x1309 // gouraud * texture
#define X_MUL_IA        0x130a // gouraud * texture
#define X_TEXTUREBLEND  0x130b // blend(grouraud,texture,texturealpha)
#define X_TEXTUREENVA   0x130c // blend(envcolor,gouraud,texturealpha)
#define X_TEXTUREENVC   0x130d // blend(envcolor,gouraud,texturecolor)
#define X_SUB           0x130e // gouraud - texture
#define X_TEXTUREENVCR  0x130f // blend(gouraud,envcolor,texturecolor)
#define X_LASTCOMBINE   0x130f

/**
 * Sets the environment (constant) color.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @param a Alpha component.
 * @return Always 0.
 */
EXPORT int     x_envcolor(float r,float g,float b,float a);

/**
 * Sets the programmable combine modes for RGB and alpha.
 * @param rgb RGB combine mode.
 * @param alpha Alpha combine mode.
 * @return Always 0.
 */
EXPORT int     x_procombine(int rgb,int alpha);
/**
 * Sets the programmable dual-texture combine modes for RGB and alpha.
 * @param rgb RGB combine mode.
 * @param alpha Alpha combine mode.
 * @param text1text2 Combine mode between texture 1 and texture 2.
 * @param sametex Non-zero to use the same texture for both stages.
 * @return 0 on success, 1 if unsupported or invalid.
 */
EXPORT int     x_procombine2(int rgb,int alpha,int text1text2,int sametex);

/**
 * Selects the single texture used for rendering.
 * @param text1handle Handle of the texture to use.
 * @return 0 on success, 1 on invalid handle.
 */
EXPORT int     x_texture(int text1handle); // single texture
/**
 * Selects the two textures used for rendering.
 * @param text1handle Handle of the first texture.
 * @param text2handle Handle of the second texture.
 * @return 0 on success, 1 if unsupported or invalid.
 */
EXPORT int     x_texture2(int text1handle,int text2handle); // dual texture

/****************************************************************************
** xtext.c
**
** Texture routines. Textures are selected with the x_mode routines
** when selecting rendering mode, but they are loaded with these
** routines.
**
** Currently all texture data is given as 32 bit RGBA and is
** internally converted to a better format. You can specify the
** suggested format (to control what components get most accuracy)
**
** Texture filtering is specified by the mode.
**
** - routines for doing fast texture anim/recalc will be added later.
** - freetexture doesn't work yet.
*/

// suggested formats for data once loaded to 3dfx
#define X_RGBA5551 0
#define X_RGBA4444 1
#define X_RGBA8888 2 // not supported in 3dfx
#define X_RGB565   3
#define X_I8       4
#define X_IA88     5
#define X_Z16      6
#define X_Z32      7
#define X_FORMATMASK 0xff
// these can be orred to format
#define X_CLAMP    0x100 // clamp this texture (default is loop)
#define X_MIPMAP   0x200 // mipmap texture (you have to provide ALL levels)
#define X_DYNAMIC  0x400 // allows opentexturedata & closetexturedata
#define X_NOBILIN  0x800 // pointsample
#define X_CLAMPNOX 0x1000
#define X_CLAMPNOY 0x2000

/**
 * Creates a texture with the given format and size.
 * @param format Texture format (X_RGBA5551, X_RGBA4444, X_RGBA8888, X_RGB565, X_I8, X_IA88, X_Z16 or X_Z32, optionally orred with X_* flags).
 * @param width Texture width in pixels.
 * @param height Texture height in pixels.
 * @return Handle to the new texture, or -1 on failure.
 */
EXPORT int     x_createtexture(int format,int width,int height);
/**
 * Loads one mipmap level of a texture.
 * @param handle Handle of the texture.
 * @param level Mipmap level to load.
 * @param data Pointer to the level's pixel data.
 * @return Non-zero on success (1 without mipmaps, 2 with mipmaps), zero on failure.
 */
EXPORT int     x_loadtexturelevel(int handle,int level,char *data);
/**
 * Frees a texture.
 * @param handle Handle of the texture to free.
 */
EXPORT void    x_freetexture(int handle);
/**
 * Unloads all textures from the accelerator.
 */
EXPORT void    x_cleartexmem(void); // unloads all textures from accelerator
/**
 * Returns information about a texture.
 * @param handle Handle of the texture.
 * @param format Receives the requested format.
 * @param memformat Receives the format the data is stored in.
 * @param width Receives the texture width.
 * @param height Receives the texture height.
 * @return 0 on success, 1 if the handle is invalid.
 */
EXPORT int     x_gettextureinfo(int handle,int *format,int *memformat,int *width,int *height);
/**
 * Opens a texture for direct data access.
 * @param handle Handle of the texture.
 * @return Pointer to the texture data, or NULL if the handle is invalid or the texture is not dynamic.
 */
EXPORT uchar  *x_opentexturedata(int handle); // check memformat!
/**
 * Closes a texture opened with x_opentexturedata.
 * @param handle Handle of the texture.
 */
EXPORT void    x_closetexturedata(int handle);

/****************************************************************************
** xgeom.c
**
** NOTE: x_matrix is not a high speed thing right now (a few brute force
** 4x4 matrix multiplies each time called). For a large number of
** independent objects a better system has to be designed.
*/

/**
 * Sets the viewport in pixel coordinates.
 * @param x0 Left edge of the viewport.
 * @param y0 Top edge of the viewport.
 * @param x1 Right edge of the viewport.
 * @param y1 Bottom edge of the viewport.
 */
// projection setup
EXPORT void    x_viewport(float x0,float y0,float x1,float y1);
/**
 * Sets a perspective projection frustum.
 * @param xmin Left clipping plane.
 * @param xmax Right clipping plane.
 * @param ymin Bottom clipping plane.
 * @param ymax Top clipping plane.
 * @param znear Near clipping distance.
 * @param zfar Far clipping distance.
 */
EXPORT void    x_frustum(float xmin,float xmax,float ymin,float ymax,float znear,float zfar);
/**
 * Sets an orthographic projection.
 * @param xmin Left edge.
 * @param ymin Bottom edge.
 * @param xmax Right edge.
 * @param ymax Top edge.
 * @param znear Near clipping distance.
 * @param zfar Far clipping distance.
 */
EXPORT void    x_ortho(float xmin,float ymin,float xmax,float ymax,float znear,float zfar);
/**
 * Sets the projection matrix directly.
 * @param matrix Matrix to use, or NULL for identity.
 */
EXPORT void    x_projmatrix(xt_matrix *matrix); // NULL=ident
/**
 * Sets the projection with a field of view; fov<1 means ortho, fov>1 means perspective.
 * @param fov Field of view, or ortho scale when below 1.
 * @param znear Near clipping distance.
 * @param zfar Far clipping distance.
 */
// simple version of x_frustum/x_ortho. fov<1 means ortho, fov>1 means perspective
EXPORT void    x_projection(float fov,float znear,float zfar);

/**
 * Sets the camera (view) matrix.
 * @param matrix Matrix to use, or NULL for identity.
 */
// transform setup
EXPORT void    x_cameramatrix(xt_matrix *matrix); // NULL=ident
/**
 * Sets the world transform matrix and recomputes the combined transform.
 * @param matrix Matrix to use, or NULL for identity.
 */
EXPORT void    x_matrix(xt_matrix *matrix); // NULL=ident
/**
 * Returns the current world transform matrix.
 * @param matrix Destination for the current matrix.
 */
EXPORT void    x_getmatrix(xt_matrix *matrix);

// vertex uploading
#define X_POINTS     1
#define X_TRIANGLES  2
#define X_TRISTRIP   3
#define X_TRIFAN     4
#define X_QUADS      5
#define X_POLYLINE   6
#define X_LINES      7
#define X_QUADSTRIP  8  // for now converted to tristrip
#define X_POLYGON    9  // [MAX 64 vertices!!]
/**
 * Starts a primitive of the given type.
 * @param type Primitive type (X_POINTS, X_TRIANGLES, X_TRISTRIP, X_TRIFAN, X_QUADS, X_POLYLINE, X_LINES, X_QUADSTRIP or X_POLYGON).
 */
EXPORT void    x_begin(int type);
/**
 * Ends the current primitive and finalizes its corner list.
 */
EXPORT void    x_end(void);
/**
 * Flushes buffered geometry and pending state changes to the hardware.
 */
EXPORT void    x_flush(void);
/**
 * Sends one vertex at an absolute position with its data.
 * @param pos Vertex position.
 * @param data Vertex data (color, texture coordinates).
 */
EXPORT void    x_vx(xt_pos *pos,xt_data *data);
/**
 * Sends one pre-transformed vertex using a position from an array.
 * @param arrayindex Index of the vertex position in the array.
 * @param data Vertex data (color, texture coordinates).
 */
EXPORT void    x_vxa(int arrayindex,xt_data *data);
/**
 * Transforms an array of vertices into the reusable vertex array.
 * @param pos Array of vertex positions.
 * @param size Number of vertices in the array.
 * @param mask Per-vertex transform flag: zero requests a full transform, non-zero marks already-transformed or relative vertices; NULL to transform all.
 */
EXPORT void    x_vxarray(xt_pos *pos,int size,char *mask);
// these set fields in the global variables and call x_vx*
#define x_vxcolor(zr,zg,zb)      g_data.r=(zr), g_data.g=(zg), g_data.b=(zb), g_data.a=1.0f
#define x_vxcolor4(zr,zg,zb,za)  g_data.r=(zr), g_data.g=(zg), g_data.b=(zb), g_data.a=(za)
#define x_vxalpha(za)            g_data.a=(za)
#define x_vxtex(zs,zt)           g_data.t1s=(zs), g_data.t1t=(zt)
#define x_vxtexv(st)             g_data.t1s=((float *)(st))[0], g_data.t1t=((float *)(st))[1]
#define x_vxtexp(zs,zt,zw)       g_data.t1s=(zs), g_data.t1t=(zt), g_data.t1w=(zw)
#define x_vxtex2(zs,zt)          g_data.t2s=(zs), g_data.t2t=(zt)
#define x_vxtex2v(st)            g_data.t2s=((float *)(st))[0], g_data.t2t=((float *)(st))[1]
#define x_vxpos(zx,zy,zz)        g_pos.x=(zx), g_pos.y=(zy), g_pos.z=(zz), x_vx(&g_pos,&g_data)
#define x_vxposv(xyz)            x_vx((xyz),&g_data)
#define x_vxposa(ind)            x_vxa((ind),&g_data)

/****************************************************************************
** Public structures that should be in client data segment (for xv_ macros).
** Currently defined in xmain.c.
*/

/**
 * Current vertex data used by the x_vx* macros.
 */
extern xt_data  g_data;
/**
 * Current vertex position used by the x_vx* macros.
 */
extern xt_pos   g_pos;

#ifdef  __cplusplus
}
#endif

#endif//_X_H_

