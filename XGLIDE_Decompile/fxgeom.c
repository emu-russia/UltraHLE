// This module deals with vertex transformation (3D->2D), since Glide does not know how to do Z. It also deals with Clipping and partitioning of edge polygons. This is supposed to be handled by the graphics API, but alas....

#include "pch.h"

static float identmatrix[4 * 4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

static int flip;
static int allxformed;
static int state;
static int mode;		// x_begin type

static int16_t debugcount = 0;
static int posarrayallocsize = 0;
static int posarraysize = 0;
static xt_xfpos* posarray;			// Item size: 20 bytes

static int vertices;
static int vertices_base;
static int vertices_lastnonrel;

static int corners_base;
static int corners;

static int clipnewvx;
static int clipor;
static int clipbuf1[0x10];		// TODO: not sure of the size
static int clipbuf2[0x10];		// TODO: not sure of the size
static int* clipin;
static int* clipout;

static GrVertex grvx[0x200];		// Item size: 60 bytes
static float corner[0x200 * 5];
static xt_pos pos[0x200];
static xt_tex tex[0x200];
static xt_tex tex2[0x200];
static xt_tex texp[0x200];
static xt_xfpos xfpos[0x200];		// Item size: 20 bytes
static uint8_t xformed[0x200];			// Contains an indication that the vertex has been transformed (?)

static int splitbuf[0x10];		// TODO: not sure of the size

void geom_init()
{
}

void x_cameramatrix(xt_matrix* a1)
{
	int v2; // zf  - bool
	BYTE *v3; // edi
	signed int v4; // ecx
	BYTE *v5; // esi

	v2 = a1 == 0;
	if ( !a1 )
		goto LABEL_10;
	v3 = (BYTE *)identmatrix;
	v4 = 64;
	v5 = (BYTE *)a1;
	do
	{
		if ( !v4 )
			break;
		v2 = *v5++ == *v3++;
		--v4;
	}
	while ( v2 );

	if ( v2 )
	{
LABEL_10:
		g_state[XST].campresent = 0;
	}
	else
	{
		g_state[XST].campresent = 1;
		memcpy(g_state[XST].camxform, a1, 0x40u);
	}
}

void x_getmatrix(xt_matrix* matrix)
{
	memcpy(matrix, g_state[XST].matrix, 0x40u);
}

void dumpmatrix(float *a1, int a2)
{
	signed int v2; // esi
	float *v3; // ebx
	signed int v4; // edi
	long double v5; // fst7

	v2 = 4;
	x_log("%s matrix:\n", a2);
	v3 = a1;
	do
	{
		v4 = 4;
		x_log(" {");
		do
		{
			v5 = *v3;
			++v3;
			x_log("%13.5f ", (double)v5);
			--v4;
		}
		while ( v4 );
		x_log("}\n");
		--v2;
	}
	while ( v2 );
}

void x_matrix(xt_matrix* matrix)
{
	char *v1; // edi
	float *v2; // edx
	float *v3; // ebx
	signed int v4; // esi
	float *v5; // ecx
	float *v6; // eax
	long double v7; // fst7
	signed int v8; // ebp
	long double v9; // fst6
	float v10; // ST28_4
	float *v11; // edi
	float *v12; // edx
	float *v13; // ebx
	signed int v14; // esi
	float *v15; // ebp
	float *v16; // ecx
	long double v17; // fst7
	signed int v18; // eax
	long double v19; // fst6
	float v20; // ST28_4
	float *v21; // [esp+24h] [ebp-44h]
	char v22; // [esp+28h] [ebp-40h]

	if (matrix)
	{
		g_state[XST].matrixnull = 0;
	}
	else
	{
		matrix = identmatrix;
		g_state[XST].matrixnull = 1;
	}
	if ( g_state[XST].projchanged )
	{
		recalc_projection();
		g_state[XST].projchanged = 0;
	}
	if ( g_state[XST].projnull && g_state[XST].matrixnull)
	{
		g_state[XST].usexformmode = 3;
	}
	else
	{
		g_state[XST].usexformmode = g_state[XST].xformmode;
		x_flush();
		memcpy(g_state[XST].matrix, matrix, 0x40u);
		if ( g_state[XST].xformmode != 1 && g_state[XST].campresent)
		{
			v1 = &v22;
			v2 = (float *)g_state[XST].camxform;
			do
			{
				v3 = (float *)matrix;
				v4 = 4;
				do
				{
					v5 = v2;
					v6 = v3;
					v7 = 0.0;
					v8 = 4;
					do
					{
						v9 = *v6 * *v5;
						++v5;
						v6 += 4;
						--v8;
						v7 = v7 + v9;
					}
					while ( v8 );
					v10 = v7;
					v1 += 4;
					++v3;
					--v4;
					*((float *)v1 - 1) = v10;
				}
				while ( v4 );
				v2 += 4;
			}
			while ( v2 < (float *)&g_state[XST].camxform[4*4]);
			v21 = (float *)&v22;
		}
		else
		{
			v21 = (float *)g_state[XST].matrix;
		}
		v11 = (float *)g_state[XST].xform;
		v12 = (float *)g_state[XST].projxform;
		do
		{
			v13 = v21;
			v14 = 4;
			do
			{
				v15 = v12;
				v16 = v13;
				v17 = 0.0;
				v18 = 4;
				do
				{
					v19 = *v16 * *v15;
					++v15;
					v16 += 4;
					--v18;
					v17 = v17 + v19;
				}
				while ( v18 );
				v20 = v17;
				++v11;
				++v13;
				--v14;
				*(v11 - 1) = v20;
			}
			while ( v14 );
			v12 += 4;
		}
		while ( v12 < (float *)&g_state[XST].projxform[4*4]);

		if ( g_state[XST].geometry & X_DUMPDATA)
		{
			dumpmatrix(v21, "Modelview");
			dumpmatrix(g_state[XST].projxform, "Projection");
			dumpmatrix(g_state[XST].xform, "Combined");
			x_log("Xformmode: %i Znear: %f Zfar: %f\n", g_state[XST].xformmode, g_state[XST].znear, g_state[XST].zfar);
		}
	}
}

int recalc_projection()
{
	long double v1; // fst7
	long double v2; // fst6
	float v6; // eax
	int result; // eax
	long double v8; // fst7
	long double v9; // fst7
	long double v10; // fst6
	long double v11; // fst5
	long double v12; // fst4

	v1 = (g_state[XST].view_x1 - g_state[XST].view_x0 + 1.0f) * 0.5f;
	g_state[XST].projxmul = v1;
	v2 = (g_state[XST].view_y1 - g_state[XST].view_y0 + 1.0f) * 0.5f;
	g_state[XST].projymul = v2;
	g_state[XST].projxadd = v1 + g_state[XST].view_x0 + 0.2f;
	g_state[XST].projyadd = v2 + g_state[XST].view_y0 + 0.2f;
	// TODO: Check. Hard place for decompiler.
	grClipWindow(g_state[XST].view_x0, g_state[XST].view_y0, g_state[XST].view_x1, g_state[XST].view_y1);
	v6 = g_state[XST].xformmode;
	if ( v6 == 0 )
	{
		memset(g_state[XST].projxform, 0, 0x40u);
		v8 = g_state[XST].znear / g_state[XST].xmax;
		g_state[XST].projxform[227 - 217] = 1.0f;
		g_state[XST].projxform[232 - 217] = 1.0f;
		g_state[XST].projxform[217 - 217] = v8;
		g_state[XST].projxform[222 - 217] = g_state[XST].znear / g_state[XST].ymax;
		result = projrecalced();
	}
	else
	{
		if ( v6 == 1 )
		{
			v9 = 2.0 / (g_state[XST].xmax - g_state[XST].xmin);
			v10 = 2.0 / (g_state[XST].ymax - g_state[XST].ymin);
			v11 = (g_state[XST].xmin + g_state[XST].xmax) * 0.5f;
			v12 = (g_state[XST].ymax + g_state[XST].ymin) * 0.5f;
			memset(g_state[XST].projxform, 0, 0x40u);
			g_state[XST].projxform[227 - 217] = 1.0f;
			g_state[XST].projxform[217 - 217] = v9;
			g_state[XST].projxform[222 - 217] = v10;
			g_state[XST].projxform[228 - 217] = 0.0f;
			g_state[XST].projxform[232 - 217] = 0.0f;
			g_state[XST].projxform[220 - 217] = -(v11 * v9);
			g_state[XST].projxform[224 - 217] = -(v10 * v12);
		}
		result = projrecalced();
	}
	return result;
}

void x_begin(int a1)
{
	int result; // eax

	if ( g_state[XST].changed || g_state[XST].projchanged )
		x_flush();
	mode = a1;
	result = 0;
	vertices_base = vertices;
	corners_base = corners;
	state = 0;
	flip = 0;
	return result;
}

void x_end()
{
	int v0; // eax

	if (mode == X_POLYGON)
	{
		v0 = corners - corners_base - 1;
		if ( v0 >= 3 )
		{
			corner[corners_base] = v0;
		}
		else
		{
			corners = corners_base;
			vertices = vertices_base;
		}
	}
	mode = 0;
	if (vertices > 127) {
		x_flush();
	}
}

void vertexdata(xt_data* a1)
{
	int v1; // eax
	float *v2; // ecx
	int v3; // eax
	long double v4; // fst7
	int v5; // eax
	int result; // eax
	int v7; // ecx
	int v8; // eax
	int v9; // ecx
	int v10; // ecx
	int v11; // eax
	int v12; // ecx
	int v13; // edx
	int v14; // eax
	int v15; // ecx
	int v16; // ecx
	int v17; // eax
	int v18; // edx
	int v19; // eax
	int v20; // ecx
	int v21; // ecx
	int v22; // eax
	int v23; // ecx
	int v24; // ecx
	int v25; // ecx
	int v26; // ecx
	int v27; // eax
	int v28; // ecx
	int v29; // ecx
	int v30; // eax
	int v31; // ecx
	int v32; // ecx
	int v33; // eax
	int v34; // ecx
	float v35; // [esp+0h] [ebp-8h]
	float v36; // [esp+4h] [ebp-4h]

	v35 = g_state[XST].texturexmul;
	v36 = g_state[XST].textureymul;
	if ( mode == 0 )
		x_fatal("vertex without begin");
	if ( g_state[XST].send & 1 )
	{
		v2 = a1;
		grvx[vertices].r = a1->r * 256.0;
		grvx[vertices].g = a1->g * 256.0;
		grvx[vertices].b = a1->b * 256.0;
		grvx[vertices].a = a1->a * 256.0;
	}
	else
	{
		v2 = a1;
	}
	if ( g_state[XST].send & 2 )
	{
		if ( g_state[XST].setnew & 0x10 )
		{
			texp[vertices].s = *((DWORD *)v2 + 7);
			tex[vertices].s = v2[5] * v2[7] * v35;
			v4 = v2[6] * v2[7];
		}
		else
		{
			tex[vertices].s = v2[5] * v35;
			v4 = v2[6];
		}
		tex[vertices].t = v4 * v36;
	}
	if ( g_state[XST].send & 4 )
	{
		tex2[vertices].s = v2[8] * v35;
		tex2[vertices].t = v2[9] * v36;
	}
	switch (mode)
	{
		case X_POINTS:
			corner[corners] = 1;
			corner[corners + 1] = vertices;
			corners += 2;
			break;
		case X_TRIANGLES:
			if (state >= 2 )
			{
				v8 = corners++;
				corner[v8] = 3;
				v9 = corners++;
				corner[v9] = vertices - 2;
				state = 0;
				corner[corners++] = vertices - 1;
				v10 = corners++;
				corner[v10] = vertices;
			}
			else
			{
				++state;
			}
			break;
		case X_TRISTRIP:
		case X_QUADSTRIP:
			if (state >= 2 )
			{
				v11 = corners++;
				corner[v11] = 3;
				if (flip)
				{
					v12 = corners++;
					v13 = vertices;
					corner[v12] = vertices - 1;
					v14 = corners++;
					corner[v14] = vertices - 2;
					corner[corners] = v13;
				}
				else
				{
					v15 = corners++;
					corner[v15] = vertices - 2;
					v16 = corners++;
					corner[v16] = vertices - 1;
					corner[corners] = vertices;
				}
				flip ^= 1u;
				++corners;
			}
			else
			{
				++state;
			}
			break;
		case X_TRIFAN:
			if (state >= 2 )
			{
				v17 = corners;
				v18 = vertices_base;
				++corners;
				corner[v17] = 3;
				v19 = corners++;
				corner[v19] = v18;
				v20 = corners++;
				corner[v20] = vertices - 1;
				v21 = corners++;
				corner[v21] = vertices;
			}
			else
			{
				++state;
			}
			break;
		case X_QUADS:
			if (state >= 3 )
			{
				v22 = corners++;
				corner[v22] = 4;
				v23 = corners++;
				corner[v23] = vertices - 3;
				v24 = corners++;
				corner[v24] = vertices - 2;
				v25 = corners++;
				state = 0;
				corner[v25] = vertices - 1;
				v26 = corners++;
				corner[v26] = vertices;
			}
			else
			{
				++state;
			}
			break;
		case X_POLYLINE:
			if (state >= 1 )
			{
				v27 = corners++;
				corner[v27] = 2;
				v28 = corners++;
				corner[v28] = vertices - 1;
				v29 = corners++;
				corner[v29] = vertices;
			}
			else
			{
				++state;
			}
			break;
		case X_LINES:
			if (state >= 1 )
			{
				v30 = corners++;
				corner[v30] = 2;
				v31 = corners++;
				state = 0;
				corner[v31] = vertices - 1;
				v32 = corners++;
				corner[v32] = vertices;
			}
			else
			{
				++state;
			}
			break;
		case X_POLYGON:
			if ( !state)
			{
				v33 = corners;
				++state;
				++corners;
				corner[v33] = 1;
			}
			v34 = corners++;
			corner[v34] = vertices;
			result = corners_base + 64;
			if ( corners_base + 64 < corners )
				x_fatal("xgeom: too large X_POLYGON!\n");
			break;
		default:
			break;
	}
	if ( ++vertices >= 191 && (!state || vertices >= 256) )
		x_flush();
}

void xform(int a1, xt_pos* a2, int a3, char *a4)
{
	int v4; // ecx
	int v5; // edx
	float *v6; // esi
	char *v7; // edi
	signed int v8; // eax
	long double v9; // fst7
	int v10; // eax
	int v11; // edx
	float *v12; // esi
	char *v13; // edi
	signed int v14; // eax
	signed int v15; // ebx
	float v16; // et1
	float v18; // ST14_4
	unsigned __int8 v19; // c0
	unsigned __int8 v20; // c3
	int v21; // eax
	float v22; // et1
	float v24; // ST14_4
	unsigned __int8 v25; // c0
	unsigned __int8 v26; // c3
	int v27; // ecx
	int v28; // edx
	float *v29; // esi
	BYTE *v30; // edi
	BYTE *v31; // eax
	char v33; // c3
	signed int v34; // eax
	int v35; // ecx
	int v36; // ebx
	int v37; // edx
	float *v38; // esi
	char *v39; // edi
	int v40; // eax
	signed int v41; // ebp
	float v42; // et1
	float v44; // ST14_4
	unsigned __int8 v45; // c0
	unsigned __int8 v46; // c3
	int v47; // eax
	signed int v48; // ebx
	float v49; // et1
	float v51; // ST14_4
	unsigned __int8 v52; // c0
	unsigned __int8 v53; // c3
	float v54; // [esp+10h] [ebp-8h]
	float v55; // [esp+10h] [ebp-8h]
	float v56; // [esp+10h] [ebp-8h]
	float v57; // [esp+10h] [ebp-8h]

	switch ( g_state[XST].usexformmode)
	{
		case 0:
			v4 = a3;
			if ( g_state[XST].setnew & 8 )
			{
				if ( a3 <= 0 )
					return;
				v5 = a1;
				v6 = a2;
				v7 = a4;
				while ( 1 )
				{
					v8 = 0;
					if ( v7 )
						v8 = *v7++;
					if ( !v8 )
						break;
					if ( v8 > 16 )
					{
						v10 = v5 + 20 * (16 - v8);
						*(float *)v5 = *v6 + *(float *)v10;
						*(float *)(v5 + 4) = v6[1] + *(float *)(v10 + 4);
						*(DWORD *)(v5 + 8) = *(DWORD *)(v10 + 8);
						*(DWORD *)(v5 + 12) = *(DWORD *)(v10 + 12);
						goto LABEL_11;
					}
LABEL_12:
					v6 += 3;
					v5 += 20;
					if ( !--v4 )
						return;
				}
				*(float *)v5 = *v6 * g_state[XST].xform[185 - 185]
										 + v6[1] * g_state[XST].xform[186 - 185]
										 + v6[2] * g_state[XST].xform[187 - 185]
										 + g_state[XST].xform[188 - 185];
				*(float *)(v5 + 4) = *v6 * g_state[XST].xform[189 - 185]
													 + v6[1] * g_state[XST].xform[190 - 185]
													 + v6[2] * g_state[XST].xform[191 - 185]
													 + g_state[XST].xform[192 - 185];
				v9 = *v6 * g_state[XST].xform[193 - 185]
					 + v6[1] * g_state[XST].xform[194 - 185]
					 + v6[2] * g_state[XST].xform[195 - 185]
					 + g_state[XST].xform[196 - 185];
				*(float *)(v5 + 8) = v9;
				*(float *)(v5 + 12) = 1.0 / v9;
LABEL_11:
				*(DWORD *)(v5 + 16) = 0;
				goto LABEL_12;
			}
			if ( a3 > 0 )
			{
				v11 = a1;
				v12 = a2;
				v13 = a4;
				while ( 1 )
				{
					v14 = 0;
					if ( v13 )
						v14 = *v13++;
					if ( !v14 )
						break;
					if ( v14 > 16 )
					{
						v21 = 20 * (16 - v14);
						*(float *)v11 = *v12 + *(float *)(v21 + v11);
						*(float *)(v11 + 4) = v12[1] + *(float *)(v21 + v11 + 4);
						v55 = *(float *)(v21 + v11 + 8);
						*(float *)(v11 + 8) = v55;
						*(DWORD *)(v11 + 12) = *(DWORD *)(v21 + v11 + 12);
						v15 = 0;
						v22 = *(float *)v11;
						if ( !(v25 | v26) )
							v15 = 256;
						if ( *(float *)v11 > (long double)v55 )
							v15 |= 0x200u;
						v24 = -v55;
						if ( *(float *)(v11 + 4) < (long double)v24 )
							v15 |= 0x400u;
						if ( *(float *)(v11 + 4) > (long double)v55 )
							v15 |= 0x800u;
						if ( g_state[XST].znear > (long double)v55 )
							v15 |= 0x1000u;
						if ( g_state[XST].zfar < (long double)v55 )
							v15 |= 0x2000u;
						goto LABEL_46;
					}
LABEL_47:
					v12 += 3;
					v11 += 20;
					if ( !--v4 )
						return;
				}
				v15 = 0;
				*(float *)v11 = *v12 * g_state[XST].xform[185 - 185]
											+ v12[1] * g_state[XST].xform[186 - 185]
											+ v12[2] * g_state[XST].xform[187 - 185]
											+ g_state[XST].xform[188 - 185];
				*(float *)(v11 + 4) = *v12 * g_state[XST].xform[189 - 185]
														+ v12[1] * g_state[XST].xform[190 - 185]
														+ v12[2] * g_state[XST].xform[191 - 185]
														+ g_state[XST].xform[192 - 185];
				v54 = *v12 * g_state[XST].xform[193 - 185]
						+ v12[1] * g_state[XST].xform[194 - 185]
						+ v12[2] * g_state[XST].xform[195 - 185]
						+ g_state[XST].xform[196 - 185];
				*(float *)(v11 + 8) = v54;
				v16 = *(float *)v11;
				if ( !(v19 | v20) )
					v15 = 256;
				if ( *(float *)v11 > (long double)v54 )
					v15 |= 0x200u;
				v18 = -v54;
				if ( *(float *)(v11 + 4) < (long double)v18 )
					v15 |= 0x400u;
				if ( *(float *)(v11 + 4) > (long double)v54 )
					v15 |= 0x800u;
				if ( g_state[XST].znear > (long double)v54 )
					v15 |= 0x1000u;
				if ( g_state[XST].zfar < (long double)v54 )
					v15 |= 0x2000u;
				if ( !v15 )
					*(float *)(v11 + 12) = 1.0 / v54;
LABEL_46:
				*(DWORD *)(v11 + 16) = v15;
				goto LABEL_47;
			}
			return;
		case 1:
			v27 = a3;
			if ( a3 > 0 )
			{
				v28 = a1;
				v29 = a2;
				v30 = a4;
				do
				{
					if ( !v30 || (v31 = v30, ++v30, !*v31) )
					{
						*(float *)v28 = *v29 * g_state[XST].xform[185 - 185]
													+ v29[1] * g_state[XST].xform[186 - 185]
													+ v29[2] * g_state[XST].xform[187 - 185]
													+ g_state[XST].xform[188 - 185];
						*(float *)(v28 + 4) = *v29 * g_state[XST].xform[189 - 185]
																+ v29[1] * g_state[XST].xform[190 - 185]
																+ v29[2] * g_state[XST].xform[191 - 185]
																+ g_state[XST].xform[192 - 185];
						*(float *)(v28 + 8) = *v29 * g_state[XST].xform[193 - 185]
																+ v29[1] * g_state[XST].xform[194 - 185]
																+ v29[2] * g_state[XST].xform[195 - 185]
																+ g_state[XST].xform[196 - 185];
						if ( v33 )
							*(DWORD *)(v28 + 12) = g_state[XST].invznear;
						else
							*(float *)(v28 + 12) = 1.0 / *(float *)(v28 + 8);
						v34 = 0;
						if ( *(DWORD *)v28 > -1.0f)
							v34 = 256;
						if ( *(DWORD *)v28 > 1.0f )
							v34 |= 0x200u;
						if ( *(DWORD *)(v28 + 4) > -1.0f)
							v34 |= 0x400u;
						if ( *(DWORD *)(v28 + 4) > 1.0f)
							v34 |= 0x800u;
						*(DWORD *)(v28 + 16) = v34;
					}
					v29 += 3;
					v28 += 20;
					--v27;
				}
				while ( v27 );
			}
			return;
		case 2:
			v35 = a3;
			if ( a3 <= 0 )
			{
				v37 = a1;
				v38 = a2;
				goto LABEL_91;
			}
			v36 = a3;
			v37 = a1;
			v38 = a2;
			v39 = a4;
			do
			{
				v40 = 0;
				if ( v39 )
					v40 = *v39++;
				if ( !v40 )
				{
					v41 = 0;
					*(float *)v37 = *v38 * g_state[XST].xform[185 - 185]
												+ v38[1] * g_state[XST].xform[186 - 185]
												+ v38[2] * g_state[XST].xform[187 - 185]
												+ g_state[XST].xform[188 - 185];
					*(float *)(v37 + 4) = *v38 * g_state[XST].xform[189 - 185]
															+ v38[1] * g_state[XST].xform[190 - 185]
															+ v38[2] * g_state[XST].xform[191 - 185]
															+ g_state[XST].xform[192 - 185];
					v56 = *v38 * g_state[XST].xform[197 - 185]
							+ v38[1] * g_state[XST].xform[198 - 185]
							+ v38[2] * g_state[XST].xform[199 - 185]
							+ g_state[XST].xform[200 - 185];
					*(float *)(v37 + 8) = v56;
					v42 = *(float *)v37;
					if ( !(v45 | v46) )
						v41 = 256;
					if ( *(float *)v37 > (long double)v56 )
						v41 |= 0x200u;
					v44 = -v56;
					if ( *(float *)(v37 + 4) < (long double)v44 )
						v41 |= 0x400u;
					if ( *(float *)(v37 + 4) > (long double)v56 )
						v41 |= 0x800u;
					if ( g_state[XST].znear > (long double)v56 )
						v41 |= 0x1000u;
					if ( g_state[XST].zfar < (long double)v56 )
						v41 |= 0x2000u;
					if ( !v41 )
						*(float *)(v37 + 12) = 1.0 / v56;
					*(DWORD *)(v37 + 16) = v41;
				}
				v38 += 3;
				v37 += 20;
				--v36;
			}
			while ( v36 );
			goto LABEL_92;
		case 3:
			v37 = a1;
			v38 = a2;
			v35 = a3;
LABEL_91:
			v39 = a4;
LABEL_92:
			if ( v35 > 0 )
			{
				do
				{
					v47 = 0;
					if ( v39 )
						v47 = *v39++;
					if ( !v47 )
					{
						*(float *)v37 = *v38;
						*(float *)(v37 + 4) = v38[1];
						v48 = 0;
						v57 = v38[2];
						*(float *)(v37 + 8) = v57;
						v49 = *(float *)v37;
						if ( !(v52 | v53) )
							v48 = 256;
						if ( *(float *)v37 > (long double)v57 )
							v48 |= 0x200u;
						v51 = -v57;
						if ( *(float *)(v37 + 4) < (long double)v51 )
							v48 |= 0x400u;
						if ( *(float *)(v37 + 4) > (long double)v57 )
							v48 |= 0x800u;
						if ( g_state[XST].znear > (long double)v57 )
							v48 |= 0x1000u;
						if ( g_state[XST].zfar < (long double)v57 )
							v48 |= 0x2000u;
						if ( !v48 )
							*(float *)(v37 + 12) = 1.0 / v57;
						*(DWORD *)(v37 + 16) = v48;
					}
					v38 += 3;
					v37 += 20;
					--v35;
				}
				while ( v35 );
			}
			return;
		default:
			return;
	}
}

int setuprvx(int a1, int a2)
{
	float *v2; // esi
	float *v3; // ebx
	float *v4; // edi
	float *v5; // ebp
	int *result; // eax
	int v7; // ecx
	long double v8; // fst4
	int v9; // edx
	long double v10; // fst7
	int v11; // ecx
	long double v12; // fst4
	int v13; // ecx
	long double v14; // fst4
	int v15; // edx
	float v16; // [esp+30h] [ebp-20h]
	float v17; // [esp+30h] [ebp-20h]
	float v18; // [esp+30h] [ebp-20h]
	float v19; // [esp+34h] [ebp-1Ch]
	float v20; // [esp+38h] [ebp-18h]
	float v21; // [esp+3Ch] [ebp-14h]
	float v22; // [esp+40h] [ebp-10h]
	float v23; // [esp+44h] [ebp-Ch]
	float *v24; // [esp+48h] [ebp-8h]
	int v25; // [esp+4Ch] [ebp-4h]

	v21 = g_state[XST].projxmul;
	v22 = g_state[XST].projymul;
	v19 = g_state[XST].projxadd;
	v20 = g_state[XST].projyadd;
	v23 = g_state[XST].znear * g_state[XST].zdecal;
	v2 = &xfpos[a1];
	v3 = &grvx[a1];
	v4 = &tex[a1];
	v5 = &tex2[a1];
	result = &texp[a1];
	v24 = &texp[a1];
	if ( g_state[XST].xformmode == 1 )
	{
		v7 = a2;
		if ( a2 > 0 )
		{
			result = debugcount;
			do
			{
				*v3 = *v2 * v21 + v19;
				v3[1] = v2[1] * v22 + v20;
				v3[8] = v2[3] * v23;
				v8 = *v3 + 786432.0;
				*v3 = v8;
				*v3 = v8 - 786432.0;
				v3[1] = v3[1] + 786432.0;
				v3[1] = v3[1] - 786432.0;
				v9 = *((DWORD *)v3 + 8);
				v16 = v3[8];
				if ( g_state[XST].send & 2 )
				{
					v3[9] = *v4 * v16;
					v3[10] = v4[1] * v16;
					*((DWORD *)v3 + 11) = v9;
				}
				if ( g_state[XST].send & 4 )
				{
					v3[12] = *v5 * v16;
					v3[13] = v5[1] * v16;
				}
				v5 += 2;
				v4 += 2;
				v2 += 5;
				v3 += 15;
				--v7;
			}
			while ( v7 );
		}
		return result;
	}
	if ( g_state[XST].xformmode == 2 )
	{
		if ( a2 <= 0 )
			return result;
		v25 = a2;
		while ( 1 )
		{
			if ( !*((DWORD *)v2 + 4) )
			{
				*v3 = v2[3] * *v2 * v21 + v19;
				v3[1] = v2[1] * v2[3] * v22 + v20;
				v3[8] = v2[3] * v23;
				v10 = *v3 + 786432.0;
				*v3 = v10;
				*v3 = v10 - 786432.0;
				v3[1] = v3[1] + 786432.0;
				v3[1] = v3[1] - 786432.0;
				result = (int *)*((DWORD *)v3 + 8);
				v17 = v3[8];
				if ( g_state[XST].setnew & 0x10 )
				{
					if ( g_state[XST].send & 2 )
					{
						v3[9] = *v4 * v17;
						v3[10] = v4[1] * v17;
						v3[11] = *v24 * v17;
					}
					if ( !(g_state[XST].send & 4) )
						goto LABEL_23;
				}
				else
				{
					if ( g_state[XST].send & 2 )
					{
						v3[9] = *v4 * v17;
						v3[10] = v4[1] * v17;
						*((DWORD *)v3 + 11) = result;
					}
					if ( !(g_state[XST].send & 4) )
						goto LABEL_23;
				}
				v3[12] = *v5 * v17;
				v3[13] = v5[1] * v17;
			}
LABEL_23:
			if ( g_state[XST].geometry & X_DUMPDATA)
			{
				x_log(
					"#x_vx[ %13.5f %13.5f %13.5f ]\n",
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20)],
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20) + 1],
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20) + 2]);
				if ( *((DWORD *)v2 + 4) )
					x_log("#clip[ %13.5f %13.5f %13.5f clip %08X ]\n", v2[0], v2[1], v2[2], *((DWORD*)v2 + 4));
				else
					x_log("#clip[ %13.5f %13.5f %13.5f   w:%13.5f ]\n", v2[0], v2[1], v2[2], (double)(1.0 / v2[3]));
				if ( !*((DWORD *)v2 + 4) )
					x_log("#scrn[ %13.5f %13.5f %13.5f oow:%13.5f ]\n", v3[0], v3[1], v3[2], v3[8]);
				x_log("#\n");
			}
			v5 += 2;
			v4 += 2;
			v2 += 5;
			v3 += 15;
			v24 += 2;
			if ( !--v25 )
				return result;
		}
	}
	if ( g_state[XST].currentmode.zbias || g_state[XST].setnew & 0x10 )
	{
		v13 = a2;
		if ( a2 <= 0 )
			return result;
		result = debugcount;
		while ( 1 )
		{
			if ( !*((DWORD *)v2 + 4) )
			{
				*v3 = v2[3] * *v2 * v21 + v19;
				v3[1] = v2[1] * v2[3] * v22 + v20;
				v3[8] = v2[3] * v23;
				v14 = *v3 + 786432.0;
				*v3 = v14;
				*v3 = v14 - 786432.0;
				v3[1] = v3[1] + 786432.0;
				v3[1] = v3[1] - 786432.0;
				v15 = *((DWORD *)v3 + 8);
				v18 = v3[8];
				if ( g_state[XST].setnew & 0x10 )
				{
					if ( g_state[XST].send & 2 )
					{
						v3[9] = *v4 * v18;
						v3[10] = v4[1] * v18;
						v3[11] = *v24 * v18;
					}
					if ( !(g_state[XST].send & 4) )
						goto LABEL_55;
				}
				else
				{
					if ( g_state[XST].send & 2 )
					{
						v3[9] = *v4 * v18;
						v3[10] = v4[1] * v18;
						*((DWORD *)v3 + 11) = v15;
					}
					if ( !(g_state[XST].send & 4) )
						goto LABEL_55;
				}
				v3[12] = *v5 * v18;
				v3[13] = v5[1] * v18;
			}
LABEL_55:
			v5 += 2;
			v4 += 2;
			v2 += 5;
			v3 += 15;
			v24 += 2;
			if ( !--v13 )
				return result;
		}
	}
	v11 = a2;
	if ( a2 > 0 )
	{
		result = 0;
		do
		{
			if ( !*((DWORD *)v2 + 4) )
			{
				*v3 = v2[3] * *v2 * v21 + v19;
				v3[1] = v2[1] * v2[3] * v22 + v20;
				v3[8] = v2[3] * v23;
				v12 = *v3 + 786432.0;
				*v3 = v12;
				*v3 = v12 - 786432.0;
				v3[1] = v3[1] + 786432.0;
				v3[1] = v3[1] - 786432.0;
				if ( g_state[XST].send & 2 )
				{
					v3[9] = *v4 * v3[8];
					v3[10] = v4[1] * v3[8];
				}
				if ( g_state[XST].send & 4 )
				{
					v3[12] = *v5 * v3[8];
					v3[13] = v5[1] * v3[8];
				}
			}
			v5 += 2;
			v4 += 2;
			v2 += 5;
			v3 += 15;
			--v11;
		}
		while ( v11 );
	}
	return result;
}

void x_vx(xt_pos* a1, xt_data* a2)
{
	float *v2; // edx
	int v3; // edx

	vertices_lastnonrel = vertices;
	v2 = &pos[vertices];
	v2[0] = a1->v[0];
	v2[1] = a1->v[1];
	v2[2] = a1->v[2];
	v3 = vertices;
	allxformed = 0;
	++g_stats.in_vx;
	xformed[v3] = 0;
	vertexdata(a2);
}

void x_vxa(int a1, xt_data* a2)
{
	int v2; // ecx

	if ( a1 < 0 || a1 >= posarraysize)
		x_fatal("invalid vertex for x_vxa");
	vertices_lastnonrel = vertices;
	memcpy(&xfpos[vertices], &posarray[a1], 0x14u);
	v2 = vertices;
	++g_stats.in_vx;
	xformed[v2] = 1;
	vertexdata(a2);
}

void x_vxrel(float *a1, float *a2)
{
	float *v2; // edx
	int v3; // eax
	int v4; // ecx

	v2 = &pos[vertices];
	*v2 = *a1;
	v2[1] = a1[1];
	v3 = vertices;
	v4 = *((DWORD *)a1 + 2);
	allxformed = 0;
	*((DWORD *)v2 + 2) = v4;
	xformed[v3] = vertices - vertices_lastnonrel + 16;
	vertexdata(a2);
}

void x_vxarray(xt_pos* a1, int a2, char* a3)
{
	int v3; // eax

	if ( a1 && a2 )
	{
		x_begin(0);
		if (posarrayallocsize < a2 )
		{
			v3 = a2 + 256;
			posarrayallocsize = a2 + 256;
			if ( posarray )
			{
				x_free(posarray);
				v3 = posarrayallocsize;
			}
			posarrayallocsize = v3;
			posarray = x_allocfast(20 * v3);
			if ( !posarray )
				x_fatal("out of memory");
		}
		posarraysize = a2;
		x_fastfpu(1);
		xform(posarray, a1, a2, a3);
		x_fastfpu(0);
	}
	else
	{
		posarraysize = 0;
	}
}

void clear()
{
	allxformed = 1;
	state = 0;
	vertices = 0;
	corners = 0;
}

int doclipvertex(signed int a1, int a2, int a3)
{
	int v3; // edx
	int v4; // esi
	int v5; // edi
	int v6; // zf  - bool
	int v7; // ebx
	int v8; // ebx
	int v9; // esi
	int v10; // ebx
	float v11; // et1
	signed int v12; // edx
	float v14; // ST14_4
	unsigned __int8 v15; // c0
	unsigned __int8 v16; // c3
	int v17; // edx
	int v18; // eax
	float *v20; // eax
	float *v21; // eax
	float v22; // [esp+10h] [ebp-10h]
	float v23; // [esp+10h] [ebp-10h]
	float v24; // [esp+14h] [ebp-Ch]
	float v25; // [esp+18h] [ebp-8h]
	int v26; // [esp+1Ch] [ebp-4h]

	if ( a1 > 512 )
	{
		if ( a1 > 2048 )
		{
			if ( a1 == 4096 )
			{
				v3 = a2;
				v4 = a3;
				v20 = &flt_11454[5 * a2];
				v22 = *v20 - g_state[XST].znear;
				v24 = *v20 - flt_11454[5 * a3];
			}
			else
			{
				if ( a1 != 0x2000 )
					goto LABEL_4;
				v3 = a2;
				v4 = a3;
				v21 = &flt_11454[5 * a2];
				v22 = g_state[XST].zfar - *v21;
				v24 = flt_11454[5 * a3] - *v21;
			}
		}
		else if ( a1 == 2048 )
		{
			v3 = a2;
			v4 = a3;
			v22 = flt_11454[5 * a2] - flt_11450[5 * a2];
			v24 = flt_11450[5 * a3] - flt_11450[5 * a2] - (flt_11454[5 * a3] - flt_11454[5 * a2]);
		}
		else
		{
			if ( a1 != 1024 )
				goto LABEL_4;
			v3 = a2;
			v4 = a3;
			v22 = flt_11450[5 * a2] + flt_11454[5 * a2];
			v24 = -(flt_11454[5 * a3] - flt_11454[5 * a2] + flt_11450[5 * a3] - flt_11450[5 * a2]);
		}
	}
	else if ( a1 == 512 )
	{
		v3 = a2;
		v4 = a3;
		v22 = flt_11454[5 * a2] - xfpos_S1212[5 * a2];
		v24 = xfpos_S1212[5 * a3] - xfpos_S1212[5 * a2] - (flt_11454[5 * a3] - flt_11454[5 * a2]);
	}
	else
	{
		if ( a1 != 256 )
		{
LABEL_4:
			v3 = a2;
			v4 = a3;
			goto LABEL_5;
		}
		v3 = a2;
		v4 = a3;
		v22 = flt_11454[5 * a2] + xfpos_S1212[5 * a2];
		v24 = -(flt_11454[5 * a3] - flt_11454[5 * a2] + xfpos_S1212[5 * a3] - xfpos_S1212[5 * a2]);
	}
LABEL_5:
	v26 = clipnewvx;
	v5 = 5 * clipnewvx;
	v23 = v22 / v24;
	xfpos_S1212[v5] = (xfpos_S1212[5 * v4] - xfpos_S1212[5 * v3]) * v23 + xfpos_S1212[5 * v3];
	flt_11450[v5] = (flt_11450[5 * v4] - flt_11450[5 * v3]) * v23 + flt_11450[5 * v3];
	v6 = (g_state[XST].send & 1) == 0;
	v25 = (flt_11454[5 * v4] - flt_11454[5 * v3]) * v23 + flt_11454[5 * v3];
	flt_11454[v5] = v25;
	if ( !v6 )
	{
		v7 = 15 * clipnewvx;
		flt_2A30[v7] = (flt_2A30[15 * v4] - flt_2A30[15 * v3]) * v23 + flt_2A30[15 * v3];
		flt_2A34[v7] = (flt_2A34[15 * v4] - flt_2A34[15 * v3]) * v23 + flt_2A34[15 * v3];
		flt_2A38[v7] = (flt_2A38[15 * v4] - flt_2A38[15 * v3]) * v23 + flt_2A38[15 * v3];
		flt_2A40[v7] = (flt_2A40[15 * v4] - flt_2A40[15 * v3]) * v23 + flt_2A40[15 * v3];
	}
	if ( g_state[XST].send & 2 )
	{
		v8 = 2 * clipnewvx;
		v6 = (g_state[XST].setnew & 0x10) == 0;
		tex[clipnewvx].s = (tex[v4].s - tex[v3].s) * v23 + tex[v3].s;
		tex[clipnewvx].t = (tex[v4].t - tex[v3].t) * v23 + tex[v3].t;
		if ( !v6 )
			texp[clipnewvx].s = (texp[v4].s - texp[v3].s) * v23
											+ texp[v3].s;
	}
	if ( g_state[XST].send & 4 )
	{
		tex2[clipnewvx].s = (tex2[v4].s - tex2[v3].s) * v23 + tex2[v3].s;
		tex2[clipnewvx].t = (tex2[v4].t - tex2[v3].t) * v23 + tex2[v3].t;
	}
	v11 = xfpos_S1212[v5];
	v12 = 0;
	if ( !(v15 | v16) )
		v12 = 256;
	if ( xfpos_S1212[v5] > (long double)v25 )
		v12 |= 0x200u;
	v14 = -v25;
	if ( flt_11450[v5] < (long double)v14 )
		v12 |= 0x400u;
	if ( flt_11450[v5] > (long double)v25 )
		v12 |= 0x800u;
	if ( g_state[XST].znear > (long double)v25 )
		v12 |= 0x1000u;
	if ( g_state[XST].zfar < (long double)v25 )
		v12 |= 0x2000u;
	++clipnewvx;
	v17 = ~a1 & v12;
	v18 = v17 | clipor;
	dword_1145C[v5] = v17;
	clipor = v18;
	return v26;
}

int doclip(signed int a1)
{
	int *v1; // esi
	int v2; // ecx
	int v3; // ebx
	int *v4; // ebp
	int result; // eax

	v1 = (int *)clipout;
	v2 = *clipin;
	v3 = *(DWORD *)(clipin + 4);
	v4 = (int *)(clipin + 8);
	if ( *clipin != -1 && v3 != -1 )
	{
		while ( a1 & dword_1145C[5 * v3] )
		{
			if ( !(a1 & dword_1145C[5 * v2]) )
			{
				*v1 = doclipvertex(a1, v2, v3);
LABEL_9:
				++v1;
			}
			v2 = v3;
			v3 = *v4;
			++v4;
			if ( v3 == -1 )
				goto LABEL_11;
		}
		if ( a1 & dword_1145C[5 * v2] )
		{
			++v1;
			*(v1 - 1) = doclipvertex(a1, v3, v2);
		}
		*v1 = v3;
		goto LABEL_9;
	}
LABEL_11:
	*v1 = *clipout;
	v1[1] = -1;
	result = clipout;
	clipout = clipin;
	clipin = result;
	return result;
}

signed int clipfinish(DWORD *a1)
{
	signed int v1; // edx
	int *v2; // ecx
	int v3; // eax
	long double v4; // fst6
	signed int result; // eax
	DWORD *v6; // ecx

	v1 = -1;
	if (vertices < clipnewvx )
	{
		v2 = &dword_1145C[5 * vertices];
		v3 = clipnewvx - vertices;
		while ( 1 )
		{
			v1 &= *v2;
			v2 += 5;
			--v3;
			*(v2 - 5) = 0;
			v4 = 1.0 / *((float *)v2 - 7);
			if ( !v3 )
				break;
			*((float *)v2 - 6) = v4;
		}
		*((float *)v2 - 6) = v4;
	}
	if ( v1 )
		return 0;
	setuprvx(vertices, clipnewvx - vertices);
	v6 = a1;
	result = 0;
	do
	{
		if ( *v6 == -1 )
			break;
		++v6;
		++result;
	}
	while ( result < 256 );
	return result;
}

signed int clippoly(int a1, int a2, int *a3, DWORD *a4)
{
	int *v4; // edi
	int v5; // ecx
	int *v6; // edx
	int v7; // esi
	DWORD *v9; // ST00_4

	v4 = clipbuf1;
	clipor = a1;
	clipnewvx = vertices;
	v5 = a2;
	clipin = clipbuf1;
	clipout = clipbuf2;
	if ( a2 > 0 )
	{
		v6 = a3;
		do
		{
			v7 = *v6;
			++v4;
			++v6;
			--v5;
			*(v4 - 1) = v7;
		}
		while ( v5 );
	}
	*v4 = *a3;
	v4[1] = -1;
	if ( *((BYTE *)&clipor + 1) & 0x10 )
		doclip(4096);
	if ( *((BYTE *)&clipor + 1) & 0x20 )
		doclip(0x2000);
	if ( *((BYTE *)&clipor + 1) & 1 )
		doclip(256);
	if ( *((BYTE *)&clipor + 1) & 2 )
		doclip(512);
	if ( *((BYTE *)&clipor + 1) & 4 )
		doclip(1024);
	if ( *((BYTE *)&clipor + 1) & 8 )
		doclip(2048);
	if ( *clipin == -1 )
		return 0;
	v9 = (DWORD *)(clipin + 4);
	*a4 = clipin + 4;
	return clipfinish(v9);
}

int docliplineend(int a1, int a2)
{
	int result; // eax
	int v3; // esi

	result = a1;
	if ( dword_1145C[5 * a1] & 0x1000 )
	{
		v3 = a2;
		result = doclipvertex(4096, a2, a1);
	}
	else
	{
		v3 = a2;
	}
	if ( dword_1145C[5 * result] & 0x2000 )
		result = doclipvertex(0x2000, v3, result);
	if ( dword_1145C[5 * result] & 0x100 )
		result = doclipvertex(256, v3, result);
	if ( dword_1145C[5 * result] & 0x200 )
		result = doclipvertex(512, v3, result);
	if ( dword_1145C[5 * result] & 0x400 )
		result = doclipvertex(1024, v3, result);
	if ( dword_1145C[5 * result] & 0x800 )
		result = doclipvertex(2048, v3, result);
	return result;
}

signed int clipline(int a1, int a2, DWORD *a3)
{
	int v3; // eax
	int v4; // esi
	int v5; // eax

	clipnewvx = vertices;
	v3 = docliplineend(a1, a2);
	v4 = v3;
	v5 = docliplineend(a2, v3);
	clipbuf1[0] = v4;
	clipbuf1[1] = v5;
	clipbuf1[2] = -1;
	*a3 = clipbuf1;
	return clipfinish(clipbuf1);
}

signed int splitpoly(int a1, int a2, DWORD *a3)
{
	signed int v3; // edx
	signed int i; // edi
	int v5; // esi
	int v6; // edx
	int v7; // edi
	int v8; // ebx
	DWORD *v9; // ebp
	int v10; // edx
	DWORD *v11; // eax
	int v12; // edx
	int v13; // ebp
	int v15; // [esp+10h] [ebp-Ch]
	DWORD *v16; // [esp+14h] [ebp-8h]
	signed int v17; // [esp+18h] [ebp-4h]

	v3 = 1;
	for ( i = 0; a1 > v3; ++v3 )
	{
		if ( flt_2A28[15 * *(DWORD *)(a2 + 4 * v3)] > (long double)flt_2A28[15 * *(DWORD *)(a2 + 4 * i)] )
			i = v3;
	}
	v5 = i - 1;
	if ( i - 1 < 0 )
		v5 += a1;
	v15 = i + 1;
	if ( a1 <= i + 1 )
		v15 -= a1;
	v17 = 3;
	*a3 = *(DWORD *)(a2 + 4 * v5);
	v6 = (int)(a3 + 3);
	*(DWORD *)(v6 - 8) = *(DWORD *)(a2 + 4 * i);
	for ( *(DWORD *)(v6 - 4) = *(DWORD *)(a2 + 4 * v15); v15 != v5; v17 += 3 )
	{
		v7 = v5 - 1;
		if ( v5 - 1 < 0 )
			v7 += a1;
		v8 = v15 + 1;
		if ( a1 <= v15 + 1 )
			v8 -= a1;
		v16 = (DWORD *)(a2 + 4 * v15);
		v9 = (DWORD *)(a2 + 4 * v5);
		if ( flt_2A28[15 * *(DWORD *)(a2 + 4 * v15)] <= (long double)flt_2A28[15 * *v9] )
		{
			v12 = v6 + 8;
			v15 = v8;
			*(DWORD *)(v12 - 8) = *(DWORD *)(a2 + 4 * v8);
			v6 = v12 + 4;
			v11 = v16;
			*(DWORD *)(v6 - 8) = *v9;
			v13 = *v16;
		}
		else
		{
			v10 = v6 + 8;
			v5 = v7;
			*(DWORD *)(v10 - 8) = *(DWORD *)(a2 + 4 * v7);
			v6 = v10 + 4;
			v11 = (DWORD *)(a2 + 4 * v15);
			*(DWORD *)(v6 - 8) = *v9;
		}
		*(DWORD *)(v6 - 4) = *v11;
	}
	return v17;
}

void flush_reordertables()
{
	signed int v1; // ebx
	signed int v2; // esi
	float v3; // ebp
	xt_tex* v4; // eax
	float v5; // edx
	float v6; // ebp
	signed int v8; // ebx

	if (mode == 0) {
		clear();
		return;
	}

	switch (mode)
	{
		case X_POINTS:
			v1 = 0;
			break;
		case X_TRIANGLES:
			v1 = 2;
			break;
		case X_TRISTRIP:
			v1 = 2;
			break;
		case X_TRIFAN:
			v1 = 2;
			break;
		case X_QUADS:
			v1 = 3;
			break;
		case X_POLYLINE:
			v1 = 1;
			break;
		case X_POLYGON:
			x_fatal("xgeom: too large X_POLYGON!\n");
			v1 = 0;
			break;
		default:
			v1 = 0;
			break;
	}
	v2 = 0;
	if (mode == X_TRIFAN)
	{
		memcpy(grvx, &grvx[vertices_base], 0x3Cu);
		memcpy(xfpos, &xfpos[vertices_base], 0x14u);
		tex[0].s = tex[vertices_base].s;
		v4 = &texp[vertices_base];
		v2 = 1;
		tex[0].t = tex[vertices_base].t;
		v5 = v4->s;
		v6 = v4->t;
		vertices_base = 0;
		texp[0].s = v5;
		texp[0].t = v6;
	}
	if ( v1 <= vertices)
	{
		memmove(&xfpos[v2], &xfpos[vertices - v1], 20 * v1);
		memmove(&grvx[v2], &grvx[vertices - v1], 60 * v1);
		memmove(&tex[v2], &tex[vertices - v1], 8 * v1);
		memmove(&texp[v2], &texp[vertices - v1], 8 * v1);
	}
	else
	{
		x_fatal("internal error flushing vertex buffer");
	}
	v8 = v2 + v1;
	if ( v8 > 0 )
	{
		memset(xformed, 1u, v8);
	}
	vertices = v8;
	allxformed = 1;
	corners = 0;
}

signed int flush_drawfx()
{
	int *v0; // edi
	unsigned int v1; // edx
	int v2; // ecx
	signed int result; // eax
	int i; // esi
	int v5; // ebx
	int v6; // ebp
	int v7; // ebp
	int v8; // ebx
	DWORD *v9; // eax
	int v10; // ebx
	int j; // ebp
	int *v12; // ebx
	unsigned int v13; // ebp
	int v14; // eax
	int *v15; // edi
	DWORD *v16; // [esp+10h] [ebp-4h]

	if ( !allxformed)
		xform(xfpos, pos, vertices, xformed);
	v0 = &dword_A228;
	setuprvx(0, vertices);
	result = corners;
	corner[corners] = 0;
	for ( i = corner[0]; i; v0 = v15 + 1 )
	{
		if ( i < 3 )
			++g_stats.in_tri;
		else
			g_stats.in_tri += i - 2;
		v5 = 0;
		result = -1;
		if ( i > 0 )
		{
			v1 = (unsigned int)v0;
			v2 = i;
			do
			{
				v6 = *(DWORD *)v1;
				v1 += 4;
				v7 = dword_1145C[5 * v6];
				v5 |= v7;
				result &= v7;
				--v2;
			}
			while ( v2 );
		}
		if ( v5 )
		{
			if ( !result && g_state[XST].xformmode != 1 )
			{
				if ( i == 2 )
				{
					result = clipline(*v0, v0[1], &v16);
					if ( result >= 2 )
					{
						v9 = v16;
						++g_stats.out_tri;
						grDrawLine(&grvx[*v16], &grvx[v9[1]]);
					}
				}
				else
				{
					result = clippoly(v5, i, v0, &v16);
					v10 = result;
					if ( result >= 3 )
					{
						result -= 2;
						g_stats.out_tri += v10 - 2;
						if ( g_state[XST].setnew & 1 )
						{
							v2 = v10 - 1;
							for ( j = 0; v10 > j; ++j )
							{
								grDrawLine(&grvx[v16[v2]], &grvx[v16[j]]);
								v2 = j;
							}
						}
						else
						{
							result = splitpoly(v10, (int)v16, splitbuf);
							if ( result > 0 )
							{
								v12 = (int *)((char *)splitbuf + 8);
								v1 = (result + 2) % 3u;
								v13 = (result + 2) / 3u;
								do
								{
									v14 = *v12;
									v12 += 3;
									grDrawTriangle( &grvx[*(v12 - 5)],
													&grvx[*(v12 - 4)],
													&grvx[v14] );
									--v13;
								}
								while ( v13 );
							}
						}
					}
				}
			}
		}
		else if ( g_state[XST].setnew & 1 && i > 2 )
		{
			v2 = i - 1;
			result = i - 2;
			v8 = 0;
			g_stats.out_tri += i - 2;
			if ( i > 0 )
			{
				do
				{
					grDrawLine(&grvx[v0[v2]], &grvx[v0[v8]]);
					v2 = v8++;
				}
				while ( i > v8 );
			}
		}
		else
		{
			switch ( i )
			{
				case 1:
					++g_stats.out_tri;
					grDrawPoint(&grvx[v0[0]]);
					break;
				case 2:
					++g_stats.out_tri;
					grDrawLine(&grvx[v0[0]], &grvx[v0[1]]);
					break;
				case 3:
					++g_stats.out_tri;
					grDrawTriangle( &grvx[v0[0]],
									&grvx[v0[1]],
									&grvx[v0[2]] );
					break;
				default:
					g_stats.out_tri += i - 2;
					grDrawPlanarPolygon(v2, v1, i, v0, grvx);
					break;
			}
		}
		v15 = &v0[i];
		i = *v15;
	}
	return result;
}

void x_flush(void)
{
	if ( g_state[XST].geometry & X_DUMPDATA)
	{
		x_log("#flush %i %i %i\n", g_state[XST].projchanged, g_state[XST].changed, corners);
	}
	if (corners)
	{
		x_fastfpu(1);
		flush_drawfx();
		flush_reordertables();
		x_fastfpu(0);
	}
	if ( g_state[XST].changed )
		mode_change();
	if ( g_state[XST].projchanged )
	{
		g_state[XST].matrixnull = 1;
		x_matrix(0);
	}
}



//.bss:00002A1C _bss            segment para public 'BSS' use32
//.bss:00002A1C                 assume cs:_bss
//.bss:00002A1C                 ;org 2A1Ch
//.bss:00002A1C                 assume es:nothing, ss:nothing, ds:_data, fs:nothing, gs:nothing
//.bss:00002A1C _corners$S1220  dd ?                    ; DATA XREF: _x_begin+21↑r
//.bss:00002A1C                                         ; _x_end+9↑r ...
//.bss:00002A20                 db    ? ;
//.bss:00002A21                 db    ? ;
//.bss:00002A22                 db    ? ;
//.bss:00002A23                 db    ? ;
//.bss:00002A24 _grvx$S1213     db    ? ;               ; DATA XREF: _setuprvx+52↑o
//.bss:00002A24                                         ; _flush_reordertables+7A↑o ...
//.bss:00002A25                 db    ? ;
//.bss:00002A26                 db    ? ;
//.bss:00002A27                 db    ? ;
//.bss:00002A28 flt_2A28        dd ?                    ; DATA XREF: _splitpoly+27↑r
//.bss:00002A28                                         ; _splitpoly+31↑r ...
//.bss:00002A2C                 align 10h
//.bss:00002A30 flt_2A30        dd ?                    ; DATA XREF: _vertexdata+50↑w
//.bss:00002A30                                         ; _doclipvertex+E3↑r ...
//.bss:00002A34 flt_2A34        dd ?                    ; DATA XREF: _vertexdata+5F↑w
//.bss:00002A34                                         ; _doclipvertex+111↑r ...
//.bss:00002A38 flt_2A38        dd ?                    ; DATA XREF: _vertexdata+6E↑w
//.bss:00002A38                                         ; _doclipvertex+12D↑r ...
//.bss:00002A3C                 align 10h
//.bss:00002A40 flt_2A40        dd ?                    ; DATA XREF: _vertexdata+7D↑w
//.bss:00002A40                                         ; _doclipvertex+149↑r ...
//.bss:00002A44                 db    ? ;
//.bss:00002A45                 db    ? ;
//.bss:00002A46                 db    ? ;
//...
//.bss:0000A222                 db    ? ;
//.bss:0000A223                 db    ? ;


//.bss:0000A224 _corner$S1219   dd ?                    ; DATA XREF: _x_end:loc_6DA↑w
//.bss:0000A224                                         ; _vertexdata+137↑w ...
//.bss:0000A228 dword_A228      dd ?                    ; DATA XREF: _vertexdata+141↑w
//.bss:0000A228                                         ; _flush_drawfx+35↑o
//.bss:0000A22C                 db    ? ;
//...
//.bss:0000CA21                 db    ? ;
//.bss:0000CA22                 db    ? ;
//.bss:0000CA23                 db    ? ;

//.bss:0000CA2C _pos$S1208      db    ? ;               ; DATA XREF: _setuprvx+263↑o
//.bss:0000CA2C                                         ; _x_vx+13↑o ...
//.bss:0000CA2D                 db    ? ;
//.bss:0000CA2E                 db    ? ;
//...
//.bss:0000E229                 db    ? ;
//.bss:0000E22A                 db    ? ;
//.bss:0000E22B                 db    ? ;
//.bss:0000E22C _tex$S1209      dd ?                    ; DATA XREF: _vertexdata+B6↑w
//.bss:0000E22C                                         ; _vertexdata+CE↑w ...
//.bss:0000E230 flt_E230        dd ?                    ; DATA XREF: _vertexdata+DB↑w
//.bss:0000E230                                         ; _doclipvertex+1AA↑r ...
//.bss:0000E234                 db    ? ;
//...
//.bss:0000F22A                 db    ? ;
//.bss:0000F22B                 db    ? ;
//.bss:0000F234 _xformed$S1214  db ?                    ; DATA XREF: _x_vx+41↑w
//.bss:0000F234                                         ; _x_vxa+57↑w ...
//.bss:0000F235                 db    ? ;
//.bss:0000F236                 db    ? ;
//...
//.bss:0000F432                 db    ? ;
//.bss:0000F433                 db    ? ;

//.bss:0000F43C _tex2$S1210     dd ?                    ; DATA XREF: _vertexdata+F9↑w
//.bss:0000F43C                                         ; _setuprvx+60↑o ...
//.bss:0000F440 flt_F440        dd ?                    ; DATA XREF: _vertexdata+106↑w
//.bss:0000F440                                         ; _doclipvertex+21C↑r ...
//.bss:0000F444                 db    ? ;
//.bss:0000F445                 db    ? ;
//.bss:0000F446                 db    ? ;
//...
//.bss:0001043A                 db    ? ;
//.bss:0001043B                 db    ? ;

//.bss:00010444 _texp$S1211     dd ?                    ; DATA XREF: _vertexdata+A6↑w
//.bss:00010444                                         ; _setuprvx+6E↑o ...
//.bss:00010448 dword_10448     dd ?                    ; DATA XREF: _flush_reordertables+EF↑w
//.bss:0001044C                 db    ? ;
//.bss:0001044D                 db    ? ;
//.bss:0001044E                 db    ? ;
//.bss:0001044F                 db    ? ;
//...
//.bss:00011441                 db    ? ;
//.bss:00011442                 db    ? ;
//.bss:00011443                 db    ? ;
//.bss:0001144C _xfpos$S1212    dd ?                    ; DATA XREF: _setuprvx+4B↑o
//.bss:0001144C                                         ; _setuprvx+24F↑o ...
//.bss:00011450 flt_11450       dd ?                    ; DATA XREF: _doclipvertex+7D↑r
//.bss:00011450                                         ; _doclipvertex+83↑r ...
//.bss:00011454 flt_11454       dd ?                    ; DATA XREF: _doclipvertex+99↑r
//.bss:00011454                                         ; _doclipvertex+9F↑r ...
//.bss:00011458                 db    ? ;
//.bss:00011459                 db    ? ;
//.bss:0001145A                 db    ? ;
//.bss:0001145B                 db    ? ;
//.bss:0001145C dword_1145C     dd ?                    ; DATA XREF: _doclipvertex+2DA↑w
//.bss:0001145C                                         ; _doclip+2F↑r ...
//.bss:00011460                 db    ? ;
//.bss:00011461                 db    ? ;
//...
//.bss:00013C49                 db    ? ;
//.bss:00013C4A                 db    ? ;
//.bss:00013C4B                 db    ? ;
//.bss:00013C4C _posarray$S1224 dd ?                    ; DATA XREF: _x_vxa+37↑r
//.bss:00013C4C                                         ; _x_vxarray+32↑r ...
//.bss:00013C50 _bss            ends
