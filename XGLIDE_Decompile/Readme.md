# Decompiling the XGLIDE.LIB library


## g_stats

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
