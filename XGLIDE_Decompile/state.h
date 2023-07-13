
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
//error
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
//view_x0
//view_x1
//view_y0
//view_y1
//projxmul
//projxadd
//projymul
//projyadd
//texturexmul
//textureymul
//send
//geometryon
//geometryoff
//currentmode
//active
//setnew
	int changed;	// 290   -- mode changed
	int mask;		// 291
	int masktst;	// 292
//colortext1
//text1text2
//zbias
	int fogtype;	// 296
	float fogmin;	// 297
	float fogmax;	// 298
	float fogcolor[4];	// 299(r), 300(g), 301(b), 302(a)
	int dither;		// 303
	int src;		// g_state[304]
	int dst;		// g_state[305]
	int sametex;	// g_state[306]
//stwhint
	float alphatest;	// g_state[308]
//textures
//text1
//text2
//envc
//env
//geometry

	//int x_envcolor(float r, float g, float b, float a)
	//	*(float*)&g_state[313] = r;
	//	*(float*)&g_state[314] = g;
	//	*(float*)&g_state[315] = b;
	//	*(float*)&g_state[316] = a;

} xt_state;
