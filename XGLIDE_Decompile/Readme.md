# Decompiling the XGLIDE.LIB library

The decomposition is done in IDA, using HexRays. After that, additional manual processing for buildability is performed.

## Decompilation status

|Module/Function|Status|Notes|
|---|---|---|
|**api.c**|||
|x_init| | |
|x_deinit| | |
|x_version| | |
|x_open| | |
|x_resize| | |
|x_select| | |
|x_close| | |
|x_getstats| | |
|x_query| | |
|x_clear| | |
|x_readfb| | |
|x_writefb| | |
|x_finish| | |
|x_frustum| | |
|x_projmatrix| | |
|projrecalced| | |
|x_ortho| | |
|x_viewport| | |
|x_projection| | |
|x_zrange| | |
|x_zdecal| | |
|texture_get| | |
|x_createtexture| | |
|x_gettextureinfo| | |
|x_loadtexturelevel| | |
|x_freetexture| | |
|x_texture_getinfo| | |
|x_cleartexmem| | |
|x_opentexturedata| | |
|x_closetexturedata| | |
|x_forcegeometry| | |
|x_geometry| | |
|x_mask| | |
|x_dither| | |
|x_blend| | |
|x_alphatest| | |
|x_combine| | |
|x_procombine| | |
|x_envcolor| | |
|x_combine2| | |
|x_procombine2| | |
|x_texture| | |
|x_texture2| | |
|x_reset| | |
|x_fog| | |
|x_fullscreen| | |
|**fx.c**|||
|init_name| | |
|init_fullscreen| | |
|init_query| | |
|init_reinit| | |
|init_init| | |
|init_deinit| | |
|init_activate| | |
|init_resize| | |
|init_bufferswap| | |
|init_clear| | |
|init_readfb| | |
|init_writefb| | |
|mode_init| | |
|mode_texturemode| | |
|mode_loadtexture| | |
|mode_loadmultitexture| | |
|fixfogtable| | |
|generatefogtable| | |
|mode_change| | |
|**fxgeom.c**|||
|geom_init| | |
|x_cameramatrix| | |
|x_getmatrix| | |
|dumpmatrix| | |
|x_matrix| | |
|recalc_projection| | |
|x_begin| | |
|x_end| | |
|vertexdata| | |
|xform| | |
|setuprvx| | |
|x_vx| | |
|x_vxa| | |
|x_vxrel| | |
|x_vxarray| | |
|clear| | |
|doclipvertex| | |
|doclip| | |
|clipfinish| | |
|clippoly| | |
|docliplineend| | |
|clipline| | |
|splitpoly| | |
|flush_reordertables| | |
|flush_drawfx| | |
|x_flush| | |
|**fxtext.c**|||
|newblock| | |
|addbefore| | |
|addafter| | |
|removeblk| | |
|memory_clear| | |
|memory_alloc| | |
|memory_free| | |
|memory_create| | |
|memory_delete| | |
|freetexmem| | |
|makespace| | |
|clearspace| | |
|picktmu| | |
|fxloadtexturepart| | |
|fxloadtexture_single| | |
|fxloadtexture_trilin| | |
|fxloadtexture_multi| | |
|text_init| | |
|text_deinit| | |
|accesstexture| | |
|text_allocdata| | |
|text_loadlevel| | |
|text_freedata| | |
|text_cleartexmem| | |
|text_opendata| | |
|text_closedata| | |
|text_frameend| | |
|**main.c**|||
|zerobase| | |
|mysleep| | |
|DllMain| | |
|**util.c**|||
|log_open| | |
|x_log| | |
|breakpoint| | |
|x_fatal| | |
|x_allocfast| | |
|x_alloc| | |
|x_realloc| | |
|x_free| | |
|x_fastfpu| | |
|x_timereset| | |
|x_timeus| | |
|x_timems| | |

## Additional symbolic information

IDA does not fully understand the format of old .LIB files, so some of the information is not retracted.

In particular, if you open a .LIB file in a HEX editor, you can find there the names of parameters for calls, as well as the names of fields of private structures.

## g_state

Large structure with all execution context. It also contains 2 substructures xt_mode (new modes and current modes).

:warning: There are actually 2 structures (g_state is a pointer to the current state), and the current one is selected by the `x_select` method. But this is not yet fully decompiled.

## g_stats

A `xt_stats` structure with global library statistics. Used in `x_getstats`.

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
