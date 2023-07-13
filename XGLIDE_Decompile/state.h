
#pragma once

typedef struct
{
	int mask;		// 291
	int masktst;	// 292
	int colortext1;		// 293
	int text1text2;		// 294
	int zbias;		// 295  (unused?)
	int fogtype;	// 296
	float fogmin;	// 297
	float fogmax;	// 298
	float fogcolor[4];	// 299(r), 300(g), 301(b), 302(a)
	int dither;		// 303
	int src;		// g_state[304]
	int dst;		// g_state[305]
	int sametex;	// g_state[306]
	FxU32 stwhint;		// 307
	float alphatest;	// g_state[308]
	int textures;		// 309
	int text1;			// 310
	int text2;			// 311
	uint32_t envc;		// 312 - combined env color (buggy)
	float env[4];		// 313(r), 314(g), 315(b), 316(a)
} xt_mode;

typedef struct
{

//xt_rendmode
//used
	int tmus;		// g_state[160]
	void* hdc;		// 161
	void* hwnd;		// 162
	int xs;			// g_state[163]
	int ys;			// g_state[164]
	int buffers;	// 165
	int vsync;		// 166
	int frame;		// 167
	uint32_t error;		// 168
//matrix
//xform
//camxform
//projxform
//campresent
	int projchanged;	// 234
	int matrixnull;		// 235
	int projnull;		// 236
	float xmin;			// 237
	float xmax;			// 238
	float ymin;			// 239
	float ymax;			// 240
	float znear;		// g_state[241]
	float zfar;			// g_state[242]
	float zdecal;		// g_state[243]
	float invzfar;		// 244
	float invznear;		// 245
	int xformmode;		// 246
	int usexformmode;	// 247
	int view_x0;		// 248
	int view_x1;		// 249
	int view_y0;		// 250
	int view_y1;		// 251

//projxmul
//projxadd
//projymul
//projyadd
//texturexmul
//textureymul
	
	uint32_t send;		//  258  - wtf? ("s"omething end?)
	int geometryon;		// 259
	int geometryoff;	// 260

	xt_mode active;		// [263 ... 288]
	// 263 -- mask
	// 264 -- masktst
	// 265 -- colortext1
	// 266 -- text1text2
	// 267 -- zbias
	// 268 -- fogtype
	// 269 -- fogmin
	// 270 -- fogmax
	// 271 -- fogcolor[0]
	// 272 -- fogcolor[1]
	// 273 -- fogcolor[2]
	// 274 -- fogcolor[3]
	// 275 -- dither
	// 276 -- src
	// 277 -- dst
	// 278 -- sametex
	// 279 -- stwhint
	// 280 -- alphatest
	// 281 -- textures
	// 282 -- text1
	// 283 -- text2
	// 284 -- envc
	// 285 -- env[4]

	int setnew;		// 289  -- new geometry
	int changed;	// 290   -- mode changed

	xt_mode currentmode;	// [291 ... 316]

	int geometry;		// 317

} xt_state;
