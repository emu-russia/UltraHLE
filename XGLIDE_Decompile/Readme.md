# Decompiling the XGLIDE.LIB library

The decompilation is done in IDA, using HexRays. After that, additional manual processing for buildability is performed.

Overall status: Everything works well. The source code can still be cleaned up a bit for beauty.

Progress:

![progress](progress.png)

## Decompilation status

|Module/Function|Status|Notes|
|---|---|---|
|**api.c**|||
|x_init|Ready| |
|x_deinit|Ready| |
|x_version|Ready| |
|x_open|Ready| |
|x_resize|Ready| |
|x_select|Ready| |
|x_close|Ready| |
|x_getstats|Ready|:warning: Uses internal static variables that are not cleared between restarts of the graphics subsystem.|
|x_query|Ready|empty. It was most likely intended to use `init_query`|
|x_clear|Ready| |
|x_readfb|Ready| |
|x_writefb|Ready| |
|x_finish|Ready| |
|x_frustum|Ready| |
|x_projmatrix|Ready|xt_matrix is taken as a pointer to the internal representation of 4x4 matrices (without the xt_matrix::type property). If xt_matrix changes, it could break everything.|
|projrecalced|Ready|empty|
|x_ortho|Ready| |
|x_viewport|Ready| |
|x_projection|Ready| |
|x_zrange|Ready| |
|x_zdecal|Ready| |
|texture_get|Ready| |
|x_createtexture|Ready| |
|x_gettextureinfo|Ready| |
|x_loadtexturelevel|Ready| |
|x_freetexture|Ready| |
|x_texture_getinfo|Ready|Copy-paste of `x_gettextureinfo`|
|x_cleartexmem|Ready|Calls `text_cleartexmem`|
|x_opentexturedata|Ready| |
|x_closetexturedata|Ready| |
|x_forcegeometry|Ready| |
|x_geometry|Ready| |
|x_mask|Ready| |
|x_dither|Ready| |
|x_blend|Ready| |
|x_alphatest|Ready|Check range check|
|x_combine|Ready| |
|x_procombine|Ready| |
|x_envcolor|Ready|envc buggy (decompile bug?)|
|x_combine2|Ready| |
|x_procombine2|Ready| |
|x_texture|Ready| |
|x_texture2|Ready| |
|x_reset|Ready|Check x_dither(1). Probably meant X_ENABLE?|
|x_fog|Ready| |
|x_fullscreen|Ready|Calls `init_fullscreen`|
|**fx.c**|||
|init_name|Ready|return "Glide"|
|init_fullscreen|Ready| |
|init_query|Ready|Badly decompiled and not used|
|init_reinit|Ready| |
|init_init|Ready| |
|init_deinit|Ready| |
|init_activate|Ready|empty|
|init_resize|Ready| |
|init_bufferswap|Ready| |
|init_clear|Ready|:construction: Might be worth a little cleanup.|
|init_readfb|Ready|:construction: Might be worth a little cleanup.|
|init_writefb|Ready|empty (not implemented)|
|mode_init|Ready| |
|mode_texturemode|Ready| |
|mode_loadtexture|Ready| |
|mode_loadmultitexture|Ready| |
|fixfogtable|Ready| |
|generatefogtable|Ready| |
|mode_change|Ready (almost)|Pretty much done, but need to check a couple places that probably didn't decompile well (marked TODO).|
|**fxgeom.c**|||
|geom_init|Ready|empty|
|x_cameramatrix|Ready| |
|x_getmatrix|Ready| |
|dumpmatrix|Ready| |
|x_matrix| | |
|recalc_projection|Ready| |
|x_begin|Ready| |
|x_end|Ready| |
|vertexdata|Ready| |
|xform|Ready| |
|setuprvx| | |
|x_vx|Ready| |
|x_vxa|Ready| |
|x_vxrel|Ready| |
|x_vxarray|Ready| |
|clear|Ready| |
|doclipvertex|Ready| |
|doclip|Ready| |
|clipfinish|Ready| |
|clippoly|Ready| |
|docliplineend|Ready| |
|clipline|Ready| |
|splitpoly| | |
|flush_reordertables|Ready| |
|flush_drawfx| |Check grDrawPlanarPolygon|
|x_flush|Ready| |
|**fxtext.c**|||
|newblock|Ready| |
|addbefore| |TODO: Change to use t_block and xt_memory to make it pretty|
|addafter| |TODO: Change to use t_block and xt_memory to make it pretty|
|removeblk| |TODO: Change to use t_block and xt_memory to make it pretty|
|memory_clear| |TODO: Change to use xt_memory|
|memory_alloc| |TODO: Change to use xt_memory|
|memory_free| |TODO: Change to use xt_memory|
|memory_create| |TODO: Change to use xt_memory|
|memory_delete| |TODO: Change to use xt_memory|
|freetexmem| | |
|makespace| | |
|clearspace| | |
|picktmu|Ready| |
|fxloadtexturepart|Ready| |
|fxloadtexture_single|Ready| |
|fxloadtexture_trilin|Ready| |
|fxloadtexture_multi|Ready| |
|text_init| | |
|text_deinit| | |
|accesstexture|Ready| |
|text_allocdata|Ready|TODO: Why are float constants used here?|
|text_loadlevel| | |
|text_freedata|Ready| |
|text_cleartexmem|Ready| |
|text_opendata|Ready| |
|text_closedata|Ready|Fixed typo here (should be text2)|
|text_frameend| | |
|**main.c**|||
|zerobase|Ready|empty|
|mysleep|Ready|Uses Win32 Sleep|
|DllMain|Ready|not used|
|**util.c**|||
|log_open|Ready| |
|x_log|Ready| |
|breakpoint|Ready|Uses __debugbreak|
|x_fatal|Ready|Does a repeated log_open to flush the contents into x.log|
|x_allocfast|Ready|malloc|
|x_alloc|Ready|malloc+memset|
|x_realloc|Ready| |
|x_free|Ready|free|
|x_fastfpu|Not sure|I'm not too sure if the decompilation is done correctly, but there are no plans to use this call. Modern compilers/Windows do not require it.|
|x_timereset|Ready|Uses Win32 QueryPerformanceCounter|
|x_timeus|Ready| |
|x_timems|Ready| |

## Additional symbolic information

IDA does not fully understand the format of old .LIB files, so some of the information is not retracted.

In particular, if you open a .LIB file in a HEX editor, you can find there the names of parameters for calls, as well as the names of fields of private structures.

## xt_state

Large structure with all execution context. It also contains 2 substructures `xt_rendmode` (new modes and current modes).

:warning: There are actually 2 instancies (g_state\[2\]), and the current one is selected by the `x_select` method (0 - unused). But actually `g_state[1]` is used everywhere.

## xt_stats

A `xt_stats` structure with global library statistics. Single instance - `g_stats`. Used in `x_getstats`.

|n|field|
|---|---|
|0|frametime |
|1|in_vx |
|2|in_tri |
|3|out_tri |
|4|chg_mode |
|5|chg_text |
|6|text_total |
|7|text_uploaded |
|8|text_resident |
|9|text_used|

## GrVertex (fxgeom.c)

Used in fxgeom.c, `grvx` array.

```
/*
** GrVertex
** If these are changed the C & assembly language trisetup routines MUST
** be changed, for they will no longer work.
*/
typedef struct
{
  float x, y, z;                /* X, Y, and Z of scrn space -- Z is ignored */       0, 1, 2
  float r, g, b;                /* R, G, B, ([0..255.0]) */ 						3, 4, 5
  float ooz;                    /* 65535/Z (used for Z-buffering) */ 				6
  float a;                      /* Alpha [0..255.0] */ 								7
  float oow;                    /* 1/W (used for W-buffering, texturing) */ 		8
  //GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
//TMU0
  float  sow;                   /* s texture ordinate (s over w) */ 				9
  float  tow;                   /* t texture ordinate (t over w) */     			10
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */   		11
//TMU1
  float  sow;                   /* s texture ordinate (s over w) */ 				12
  float  tow;                   /* t texture ordinate (t over w) */     			13
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */   		14     = 15 dword total
} GrVertex;

typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;
```

## t_block

Texture block.

```
typedef struct _t_block   // 16 bytes
{
  struct _t_block* prev;
  struct _t_block* next;
  int base;
  int size;
} t_block;
```

## xt_memory

Texture memory map.

```
typedef struct _xt_memory // 48 bytes
{
  int min;    // 0
  int max;    // 1
  t_block free; // 2..5
  t_block used;   // 6..9
  void* table;  // 10
  int tableind; // 11  - table index
} xt_memory;
```

## xt_texture

Texture descriptor.

```
#define X_TEXPARTS 4
typedef struct _xt_texture    // 152 bytes (38 dwords)
{
  int state;        // 0  - Active number of the state (g_state). 0: texture is not used.
  int handle;       // 1
  int width;        // 2
  int height;       // 3
  int format;       // 4
  int memformat;      // 5
  int bytes;        // 6
  int levels;       // 7  (1 - no mipmap)
  int levelsloaded;   // 8
  int xmul;       // 9
  int ymul;       // 10
  int lastframeused;    // 11
  int reload;       // 12
  GrTexInfo ti;     // 13, 14, 15, 16, 17[data]
  int size[X_TEXPARTS];   // 18  19  20  21 
  int base[X_TEXPARTS];   // 22  23  24  25
  int tmu[X_TEXPARTS];    // 26  27  28  29
  int xblock[X_TEXPARTS];   // 30  31  32  33
  int usedsize[X_TEXPARTS]; // 34  35  36  37
} xt_texture;
```

## Glide API Calls

List:
- grSstControl
- grSstQueryHardware
- grGlideShutdown
- grGlideInit
- grSstSelect
- grSstWinOpen
- grClipWindow
- grBufferNumPending
- grBufferSwap
- grColorMask
- grDepthMask
- grBufferClear
- grLfbReadRegion
- grDepthBiasLevel
- grDepthBufferMode
- grDitherMode
- grHints
- grTexFilterMode
- grTexMipMapMode
- grTexClampMode
- grTexCombine
- guFogGenerateLinear
- guFogGenerateExp
- grCullMode
- grFogMode
- grFogTable
- grFogColorValue
- grDepthBufferFunction
- grConstantColorValue
- guColorCombineFunction
- guAlphaSource
- grColorCombine
- grAlphaTestReferenceValue
- grAlphaTestFunction
- grAlphaBlendFunction
- grDrawLine
- grDrawTriangle
- grDrawPoint
- grDrawPlanarPolygon
- grTexTextureMemRequired
- grTexDownloadMipMap
- grTexSource
- grTexMinAddress
- grTexMaxAddress
