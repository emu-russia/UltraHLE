
#pragma once

typedef struct
{

//xt_rendmode
//used
//tmus
//hdc
//hwnd
//xs
//ys
//buffers
//vsync
//frame
//error
//matrix
//xform
//camxform
//projxform
//campresent
//projchanged
//matrixnull
//projnull
//xmin
//xmax
//ymin
//ymax
	float znear;		// g_state[241]
	float zfar;			// g_state[242]
	float zdecal;		// g_state[243]
//invzfar
//invznear
//xformmode
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
//changed
//mask
//masktst
//colortext1
//text1text2
//zbias
//fogtype
//fogmin
//fogmax
//fogcolor
//dither
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
