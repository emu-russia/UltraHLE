
#pragma once

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
//matrixnull
//projnull
	float xmin;			// 237
	float xmax;			// 238
	float ymin;			// 239
	float ymax;			// 240
	float znear;		// g_state[241]
	float zfar;			// g_state[242]
	float zdecal;		// g_state[243]
//invzfar
//invznear
	int xformmode;		// 246
//usexformmode
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
//currentmode
//active
//setnew
	int changed;	// 290   -- mode changed
	int mask;		// 291
	int masktst;	// 292
	int colortext1;		// 293
	int text1text2;		// 294
//zbias
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
//textures
	int text1;			// 310
	int text2;			// 311
	uint32_t envc;		// 312 - combined env color (buggy)
	float env[4];		// 313(r), 314(g), 315(b), 316(a)
	int geometry;		// 317

} xt_state;
