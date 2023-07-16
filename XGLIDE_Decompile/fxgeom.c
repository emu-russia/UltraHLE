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
static int clipbuf1[0x100];		// TODO: not sure of the size
static int clipbuf2[0x100];		// TODO: not sure of the size
static int* clipin;
static int* clipout;

static GrVertex grvx[0x200];		// Item size: 60 bytes
static int corner[0x200 * 5];
static xt_pos pos[0x200];
static xt_tex tex[0x200];
static xt_tex tex2[0x200];
static xt_tex texp[0x200];
static xt_xfpos xfpos[0x200];		// Item size: 20 bytes
static uint8_t xformed[0x200];			// Contains an indication that the vertex has been transformed (?)

static int splitbuf[0x100];		// TODO: not sure of the size

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

void dumpmatrix(float *f, char* name)
{
	signed int v2; // esi
	float *v3; // ebx
	signed int v4; // edi
	long double v5; // fst7

	v2 = 4;
	x_log("%s matrix:\n", name);
	v3 = f;
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
		g_state[XST].usexformmode = XFORM_MODE_NONE;
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

void recalc_projection()
{
	long double v1; // fst7
	long double v2; // fst6
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

	if (g_state[XST].xformmode == XFORM_MODE_FRUSTUM)
	{
		memset(g_state[XST].projxform, 0, 0x40u);
		g_state[XST].projxform[227 - 217] = 1.0f;
		g_state[XST].projxform[232 - 217] = 1.0f;
		g_state[XST].projxform[217 - 217] = g_state[XST].znear / g_state[XST].xmax;;
		g_state[XST].projxform[222 - 217] = g_state[XST].znear / g_state[XST].ymax;
		projrecalced();
	}
	else
	{
		if (g_state[XST].xformmode == XFORM_MODE_ORTHO)
		{
			v9 = 2.0f / (g_state[XST].xmax - g_state[XST].xmin);
			v10 = 2.0f / (g_state[XST].ymax - g_state[XST].ymin);
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
		projrecalced();
	}
}

void x_begin(int type)
{
	int result; // eax

	if ( g_state[XST].changed || g_state[XST].projchanged )
		x_flush();
	mode = type;
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

void vertexdata(xt_data* data)
{
	float *v2; // ecx
	long double v4; // fst7
	int result; // eax
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
		v2 = data;
		grvx[vertices].r = data->r * 256.0;
		grvx[vertices].g = data->g * 256.0;
		grvx[vertices].b = data->b * 256.0;
		grvx[vertices].a = data->a * 256.0;
	}
	else
	{
		v2 = data;
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

/// <summary>
/// It looks scary, but in reality, macro programming or inline programming is most likely applied here.
/// Works in 1 of 4 modes (XFORM_MODE). First it performs matrix multiplication, then it checks if clipping 
/// should be done and sets `clip` flags.
/// </summary>
/// <param name="dst"></param>
/// <param name="src"></param>
/// <param name="count"></param>
/// <param name="mask"></param>
void xform(xt_xfpos* dst, xt_pos* src, int count, char* mask)
{
	int v4; // ecx
	xt_xfpos* v5; // edx
	float *v6; // esi
	char *v7; // edi
	signed int v8; // eax
	long double v9; // fst7
	xt_xfpos* v10; // eax
	xt_xfpos* v11; // edx
	float *v12; // esi
	char *v13; // edi
	signed int v14; // eax
	signed int v15; // ebx
	float v18; // ST14_4
	int v21; // eax
	float v24; // ST14_4
	int v27; // ecx
	xt_xfpos* v28; // edx
	float *v29; // esi
	BYTE *v30; // edi
	BYTE *v31; // eax
	// TODO: uninitialized local variable 'v33' used
	char v33 = 0; // c3
	signed int v34; // eax
	int v35; // ecx
	int v36; // ebx
	xt_xfpos* v37; // edx
	float *v38; // esi
	char *v39; // edi
	int v40; // eax
	signed int v41; // ebp
	float v44; // ST14_4
	int v47; // eax
	signed int v48; // ebx
	float v51; // ST14_4
	float v54; // [esp+10h] [ebp-8h]
	float v55; // [esp+10h] [ebp-8h]
	float v56; // [esp+10h] [ebp-8h]
	float v57; // [esp+10h] [ebp-8h]

	switch ( g_state[XST].usexformmode)
	{
		case XFORM_MODE_FRUSTUM:
			v4 = count;
			if ( g_state[XST].setnew & 8 )
			{
				if (count <= 0 )
					return;
				v5 = dst;
				v6 = src;
				v7 = mask;
				while ( 1 )
				{
					v8 = 0;
					if ( v7 )
						v8 = *v7++;
					if ( !v8 )
						break;
					if ( v8 > 16 )
					{
						v10 = v5 + (16 - v8);
						v5->x = *v6 + v10->x;
						v5->y = v6[1] + v10->y;
						v5->z = v10->z;
						v5->invz = v10->invz;
						goto LABEL_11;
					}
LABEL_12:
					v6 += 3;
					v5 ++;
					if ( !--v4 )
						return;
				}
				v5->x = *v6 * g_state[XST].xform[185 - 185]
										 + v6[1] * g_state[XST].xform[186 - 185]
										 + v6[2] * g_state[XST].xform[187 - 185]
										 + g_state[XST].xform[188 - 185];
				v5->y = *v6 * g_state[XST].xform[189 - 185]
										+ v6[1] * g_state[XST].xform[190 - 185]
										+ v6[2] * g_state[XST].xform[191 - 185]
										+ g_state[XST].xform[192 - 185];
				v9 = *v6 * g_state[XST].xform[193 - 185]
					 + v6[1] * g_state[XST].xform[194 - 185]
					 + v6[2] * g_state[XST].xform[195 - 185]
					 + g_state[XST].xform[196 - 185];
				v5->z = v9;
				v5->invz = 1.0f / v9;		// TODO: div by 0
LABEL_11:
				v5->clip = 0;
				goto LABEL_12;
			}
			if (count > 0 )
			{
				v11 = dst;
				v12 = src;
				v13 = mask;
				while ( 1 )
				{
					v14 = 0;
					if ( v13 )
						v14 = *v13++;
					if ( !v14 )
						break;
					if ( v14 > 16 )
					{
						v21 = (16 - v14);
						v11->x = *v12 + v11[v21].x;
						v11->y = v12[1] + v11[v21].y;
						v55 = v11[v21].z;
						v11->z = v55;
						v11->invz = v11[v21].invz;
						v15 = 0;
						v24 = -v55;
						if (v11->x < v24)
							v15 = X_CLIPX1;
						if ( v11->x > v55 )
							v15 |= X_CLIPX2;
						if ( v11->y < v24 )
							v15 |= X_CLIPY1;
						if ( v11->y > v55 )
							v15 |= X_CLIPY2;
						if ( g_state[XST].znear > v55 )
							v15 |= X_CLIPZ1;
						if ( g_state[XST].zfar < v55 )
							v15 |= X_CLIPZ2;
						goto LABEL_46;
					}
LABEL_47:
					v12 += 3;
					v11 ++;
					if ( !--v4 )
						return;
				}
				v15 = 0;
				v11->x = *v12 * g_state[XST].xform[185 - 185]
											+ v12[1] * g_state[XST].xform[186 - 185]
											+ v12[2] * g_state[XST].xform[187 - 185]
											+ g_state[XST].xform[188 - 185];
				v11->y = *v12 * g_state[XST].xform[189 - 185]
											+ v12[1] * g_state[XST].xform[190 - 185]
											+ v12[2] * g_state[XST].xform[191 - 185]
											+ g_state[XST].xform[192 - 185];
				v54 = *v12 * g_state[XST].xform[193 - 185]
						+ v12[1] * g_state[XST].xform[194 - 185]
						+ v12[2] * g_state[XST].xform[195 - 185]
						+ g_state[XST].xform[196 - 185];
				v11->z = v54;
				v18 = -v54;
				if (v11->x < v18)
					v15 = X_CLIPX1;
				if ( v11->x > v54 )
					v15 |= X_CLIPX2;
				if ( v11->y < v18 )
					v15 |= X_CLIPY1;
				if ( v11->y > v54 )
					v15 |= X_CLIPY2;
				if ( g_state[XST].znear > v54 )
					v15 |= X_CLIPZ1;
				if ( g_state[XST].zfar < v54 )
					v15 |= X_CLIPZ2;
				if ( !v15 )
					v11->invz = 1.0f / v54;			// TODO: div by 0
LABEL_46:
				v11->clip = v15;
				goto LABEL_47;
			}
			return;
		case XFORM_MODE_ORTHO:
			v27 = count;
			if (count > 0 )
			{
				v28 = dst;
				v29 = src;
				v30 = mask;
				do
				{
					if ( !v30 || (v31 = v30, ++v30, !*v31) )
					{
						v28->x = *v29 * g_state[XST].xform[185 - 185]
													+ v29[1] * g_state[XST].xform[186 - 185]
													+ v29[2] * g_state[XST].xform[187 - 185]
													+ g_state[XST].xform[188 - 185];
						v28->y = *v29 * g_state[XST].xform[189 - 185]
													+ v29[1] * g_state[XST].xform[190 - 185]
													+ v29[2] * g_state[XST].xform[191 - 185]
													+ g_state[XST].xform[192 - 185];
						v28->z = *v29 * g_state[XST].xform[193 - 185]
													+ v29[1] * g_state[XST].xform[194 - 185]
													+ v29[2] * g_state[XST].xform[195 - 185]
													+ g_state[XST].xform[196 - 185];
						// TODO: uninitialized local variable 'v33' used
						if ( v33 )
							v28->invz = g_state[XST].invznear;
						else
							v28->invz = 1.0f / v28->z;		// TODO: div by 0
						v34 = 0;
						if ( v28->x > -1.0f)
							v34 = X_CLIPX1;
						if ( v28->x > 1.0f )
							v34 |= X_CLIPX2;
						if ( v28->y > -1.0f)
							v34 |= X_CLIPY1;
						if ( v28->y > 1.0f)
							v34 |= X_CLIPY2;
						v28->clip = v34;
					}
					v29 += 3;
					v28 ++;
					--v27;
				}
				while ( v27 );
			}
			return;
		case XFORM_MODE_PROJECT:
			v35 = count;
			if (count <= 0 )
			{
				v37 = dst;
				v38 = src;
				goto LABEL_91;
			}
			v36 = count;
			v37 = dst;
			v38 = src;
			v39 = mask;
			do
			{
				v40 = 0;
				if ( v39 )
					v40 = *v39++;
				if ( !v40 )
				{
					v41 = 0;
					v37->x = *v38 * g_state[XST].xform[185 - 185]
												+ v38[1] * g_state[XST].xform[186 - 185]
												+ v38[2] * g_state[XST].xform[187 - 185]
												+ g_state[XST].xform[188 - 185];
					v37->y = *v38 * g_state[XST].xform[189 - 185]
												+ v38[1] * g_state[XST].xform[190 - 185]
												+ v38[2] * g_state[XST].xform[191 - 185]
												+ g_state[XST].xform[192 - 185];
					v56 = *v38 * g_state[XST].xform[197 - 185]
							+ v38[1] * g_state[XST].xform[198 - 185]
							+ v38[2] * g_state[XST].xform[199 - 185]
							+ g_state[XST].xform[200 - 185];
					v37->z = v56;
					v44 = -v56;
					if (v37->x < v44)
						v41 = X_CLIPX1;
					if ( v37->x > v56 )
						v41 |= X_CLIPX2;
					if ( v37->y < v44 )
						v41 |= X_CLIPY1;
					if ( v37->y > v56 )
						v41 |= X_CLIPY2;
					if ( g_state[XST].znear > v56 )
						v41 |= X_CLIPZ1;
					if ( g_state[XST].zfar < v56 )
						v41 |= X_CLIPZ2;
					if ( !v41 )
						v37->invz = 1.0f / v56;			// TODO: div by 0
					v37->clip = v41;
				}
				v38 += 3;
				v37 ++;
				--v36;
			}
			while ( v36 );
			goto LABEL_92;
		case XFORM_MODE_NONE:
			v37 = dst;
			v38 = src;
			v35 = count;
LABEL_91:
			v39 = mask;
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
						v37->x = *v38;
						v37->y = v38[1];
						v48 = 0;
						v57 = v38[2];
						v37->z = v57;
						v51 = -v57;
						if (v37->x < v51)
							v48 = X_CLIPX1;
						if ( v37->x > v57 )
							v48 |= X_CLIPX2;
						if ( v37->y < v51 )
							v48 |= X_CLIPY1;
						if ( v37->y > v57 )
							v48 |= X_CLIPY2;
						if ( g_state[XST].znear > v57 )
							v48 |= X_CLIPZ1;
						if ( g_state[XST].zfar < v57 )
							v48 |= X_CLIPZ2;
						if ( !v48 )
							v37->invz = 1.0f / v57;			// TODO: div by 0
						v37->clip = v48;
					}
					v38 += 3;
					v37 ++;
					--v35;
				}
				while ( v35 );
			}
			return;
		default:
			return;
	}
}

int setuprvx(int first, int count)
{
	xt_xfpos* v2; // esi
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
	v2 = &xfpos[first];
	v3 = &grvx[first];
	v4 = &tex[first];
	v5 = &tex2[first];
	result = &texp[first];
	v24 = &texp[first];
	if ( g_state[XST].xformmode == XFORM_MODE_ORTHO)
	{
		v7 = count;
		if (count > 0 )
		{
			result = debugcount;
			do
			{
				v3[0] = v2->x * v21 + v19;
				v3[1] = v2->y * v22 + v20;
				v3[8] = v2->invz * v23;
				v8 = v3[0] + 786432.0;
				v3[0] = v8;
				v3[0] = v8 - 786432.0;
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
				v2 ++;
				v3 += 15;
				--v7;
			}
			while ( v7 );
		}
		return result;
	}
	if ( g_state[XST].xformmode == XFORM_MODE_PROJECT)
	{
		if (count <= 0 )
			return result;
		v25 = count;
		while ( 1 )
		{
			if ( !v2->clip )
			{
				v3[0] = v2->invz * v2->x * v21 + v19;
				v3[1] = v2->y * v2->invz * v22 + v20;
				v3[8] = v2->invz * v23;
				v10 = v3[0] + 786432.0;
				v3[0] = v10;
				v3[0] = v10 - 786432.0;
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
				x_log("#x_vx[ %13.5f %13.5f %13.5f ]\n", pos[first].x, pos[first].y, pos[first].z);
				if ( v2->clip )
					x_log("#clip[ %13.5f %13.5f %13.5f clip %08X ]\n", v2->x, v2->y, v2->z, v2->clip);
				else
					x_log("#clip[ %13.5f %13.5f %13.5f   w:%13.5f ]\n", v2->x, v2->y, v2->z, (double)(1.0f / v2->invz));
				if ( !v2->clip )
					x_log("#scrn[ %13.5f %13.5f %13.5f oow:%13.5f ]\n", v3[0], v3[1], v3[2], v3[8]);
				x_log("#\n");
			}
			v5 += 2;
			v4 += 2;
			v2 ++;
			v3 += 15;
			v24 += 2;
			if ( !--v25 )
				return result;
		}
	}
	if ( g_state[XST].currentmode.zbias || g_state[XST].setnew & 0x10 )
	{
		v13 = count;
		if (count <= 0 )
			return result;
		result = debugcount;
		while ( 1 )
		{
			if ( !v2->clip )
			{
				*v3 = v2->invz * v2->x * v21 + v19;
				v3[1] = v2->y * v2->invz * v22 + v20;
				v3[8] = v2->invz * v23;
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
	v11 = count;
	if (count > 0 )
	{
		result = 0;
		do
		{
			if ( !*((DWORD *)v2 + 4) )
			{
				*v3 = v2->invz * v2->x * v21 + v19;
				v3[1] = v2->y * v2->invz * v22 + v20;
				v3[8] = v2->invz * v23;
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

void x_vx(xt_pos* p, xt_data* d)
{
	vertices_lastnonrel = vertices;
	pos[vertices].x = p->x;
	pos[vertices].y = p->y;
	pos[vertices].z = p->z;
	allxformed = 0;
	++g_stats.in_vx;
	xformed[vertices] = 0;
	vertexdata(d);
}

void x_vxa(int arrayindex, xt_data* d)
{
	if (arrayindex < 0 || arrayindex >= posarraysize)
		x_fatal("invalid vertex for x_vxa");
	vertices_lastnonrel = vertices;
	memcpy(&xfpos[vertices], &posarray[arrayindex], sizeof(xt_xfpos));
	++g_stats.in_vx;
	xformed[vertices] = 1;
	vertexdata(d);
}

void x_vxrel(xt_pos* p, xt_data* d)
{
	xt_pos *v2;
	v2 = &pos[vertices];
	v2->x = p->x;
	v2->y = p->y;
	v2->z = p->z;
	allxformed = 0;
	xformed[vertices] = vertices - vertices_lastnonrel + 16;
	vertexdata(d);
}

void x_vxarray(xt_pos* src, int count, char* mask)
{
	int v3; // eax

	if (src && count)
	{
		x_begin(0);
		if (posarrayallocsize < count)
		{
			v3 = count + 256;
			posarrayallocsize = count + 256;
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
		posarraysize = count;
		x_fastfpu(1);
		xform(posarray, src, count, mask);
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

int doclipvertex(int bit, int vi, int vo)
{
	int v3; // edx
	int v4; // esi
	signed int v12; // edx
	float v14; // ST14_4
	int v17; // edx
	int v18; // eax
	float v22; // [esp+10h] [ebp-10h]
	float v23; // [esp+10h] [ebp-10h]
	float v24; // [esp+14h] [ebp-Ch]
	float v25; // [esp+18h] [ebp-8h]
	int v26; // [esp+1Ch] [ebp-4h]

	if (bit > X_CLIPX2)
	{
		if (bit > X_CLIPY2)
		{
			if (bit == X_CLIPZ1)
			{
				v3 = vi;
				v4 = vo;
				v22 = xfpos[vi].z - g_state[XST].znear;
				v24 = xfpos[vi].z - xfpos[vo].z;
			}
			else
			{
				if (bit != X_CLIPZ2)
					goto LABEL_4;
				v3 = vi;
				v4 = vo;
				v22 = g_state[XST].zfar - xfpos[vi].z;
				v24 = xfpos[vo].z - xfpos[vi].z;
			}
		}
		else if (bit == X_CLIPY2)
		{
			v3 = vi;
			v4 = vo;
			v22 = xfpos[vi].z - xfpos[vi].y;
			v24 = xfpos[vo].y - xfpos[vi].y - (xfpos[vo].z - xfpos[vi].z);
		}
		else
		{
			if (bit != X_CLIPY1)
				goto LABEL_4;
			v3 = vi;
			v4 = vo;
			v22 = xfpos[vi].y + xfpos[vi].z;
			v24 = -(xfpos[vo].z - xfpos[vi].z + xfpos[vo].y - xfpos[vi].y);
		}
	}
	else if (bit == X_CLIPX2)
	{
		v3 = vi;
		v4 = vo;
		v22 = xfpos[vi].z - xfpos[vi].x;
		v24 = xfpos[vo].x - xfpos[vi].x - (xfpos[vo].z - xfpos[vi].z);
	}
	else
	{
		if (bit != X_CLIPX1)
		{
LABEL_4:
			v3 = vi;
			v4 = vo;
			goto LABEL_5;
		}
		v3 = vi;
		v4 = vo;
		v22 = xfpos[vi].z + xfpos[vi].x;
		v24 = -(xfpos[vo].z - xfpos[vi].z + xfpos[vo].x - xfpos[vi].x);
	}

LABEL_5:
	v26 = clipnewvx;
	v23 = v22 / v24;
	xfpos[clipnewvx].x = (xfpos[v4].x - xfpos[v3].x) * v23 + xfpos[v3].x;
	xfpos[clipnewvx].y = (xfpos[v4].y - xfpos[v3].y) * v23 + xfpos[v3].y;
	v25 = (xfpos[v4].z - xfpos[v3].z) * v23 + xfpos[v3].z;
	xfpos[clipnewvx].z = v25;
	if (g_state[XST].send & 1)
	{
		grvx[clipnewvx].r = (grvx[v4].r - grvx[v3].r) * v23 + grvx[v3].r;
		grvx[clipnewvx].g = (grvx[v4].g - grvx[v3].g) * v23 + grvx[v3].g;
		grvx[clipnewvx].b = (grvx[v4].b - grvx[v3].b) * v23 + grvx[v3].b;
		grvx[clipnewvx].a = (grvx[v4].a - grvx[v3].a) * v23 + grvx[v3].a;
	}
	if ( g_state[XST].send & 2 )
	{
		tex[clipnewvx].s = (tex[v4].s - tex[v3].s) * v23 + tex[v3].s;
		tex[clipnewvx].t = (tex[v4].t - tex[v3].t) * v23 + tex[v3].t;
		if (g_state[XST].setnew & 0x10)
			texp[clipnewvx].s = (texp[v4].s - texp[v3].s) * v23 + texp[v3].s;
	}
	if ( g_state[XST].send & 4 )
	{
		tex2[clipnewvx].s = (tex2[v4].s - tex2[v3].s) * v23 + tex2[v3].s;
		tex2[clipnewvx].t = (tex2[v4].t - tex2[v3].t) * v23 + tex2[v3].t;
	}
	v12 = 0;
	v14 = -v25;
	if (xfpos[clipnewvx].x < v14)
		v12 = X_CLIPX1;
	if ( xfpos[clipnewvx].x > v25 )
		v12 |= X_CLIPX2;
	if (xfpos[clipnewvx].y < v14 )
		v12 |= X_CLIPY1;
	if (xfpos[clipnewvx].y > v25 )
		v12 |= X_CLIPY2;
	if ( g_state[XST].znear > v25 )
		v12 |= X_CLIPZ1;
	if ( g_state[XST].zfar < v25 )
		v12 |= X_CLIPZ2;
	v17 = v12 & ~bit;
	v18 = v17 | clipor;
	xfpos[clipnewvx].clip = v17;
	clipor = v18;
	++clipnewvx;
	return v26;
}

int doclip(int bit)
{
	int *v1; // esi
	int v2; // ecx
	int v3; // ebx
	int *v4; // ebp
	int result; // eax

	v1 = clipout;
	v2 = clipin[0];
	v3 = clipin[1];
	v4 = clipin[2];
	if ( *clipin != -1 && v3 != -1 )
	{
		while (bit & xfpos[v3].clip )
		{
			if ( !(bit & xfpos[v2].clip) )
			{
				*v1 = doclipvertex(bit, v2, v3);
LABEL_9:
				++v1;
			}
			v2 = v3;
			v3 = *v4;
			++v4;
			if ( v3 == -1 )
				goto LABEL_11;
		}
		if (bit & xfpos[v2].clip )
		{
			++v1;
			*(v1 - 1) = doclipvertex(bit, v3, v2);
		}
		*v1 = v3;
		goto LABEL_9;
	}
LABEL_11:
	v1[0] = clipout[0];
	v1[1] = -1;
	// swap
	result = clipout;
	clipout = clipin;
	clipin = result;
	return result;
}

int clipfinish(int *vx)
{
	int v1; // edx
	xt_xfpos* v2; // ecx
	int v3; // eax
	int result; // eax
	int *v6; // ecx

	v1 = -1;
	if (vertices < clipnewvx )
	{
		v2 = &xfpos[vertices];
		v3 = clipnewvx - vertices;
		while ( 1 )
		{
			v1 &= v2->clip;
			--v3;
			v2->clip = 0;
			// Update clipped invz
			v2->invz = 1.0f / v2->z;		// TODO: Potential division by 0
			if ( !v3 )
				break;
			v2 ++;
		}
	}
	if ( v1 )
		return 0;
	setuprvx(vertices, clipnewvx - vertices);
	v6 = vx;
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

int clippoly(int clor, int vxn, int *v, int *out)
{
	int *v4; // edi
	int v5; // ecx
	int *v6; // edx
	int v7; // esi
	int *v9; // ST00_4

	v4 = clipbuf1;
	clipor = clor;
	clipnewvx = vertices;
	v5 = vxn;
	clipin = clipbuf1;
	clipout = clipbuf2;
	if (vxn > 0 )
	{
		v6 = v;
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
	v4[0] = v[0];
	v4[1] = -1;
	if ( clipor & X_CLIPZ1)
		doclip(X_CLIPZ1);
	if ( clipor & X_CLIPZ2)
		doclip(X_CLIPZ2);
	if ( clipor & X_CLIPX1)
		doclip(X_CLIPX1);
	if ( clipor & X_CLIPX2)
		doclip(X_CLIPX2);
	if ( clipor & X_CLIPY1)
		doclip(X_CLIPY1);
	if ( clipor & X_CLIPY2)
		doclip(X_CLIPY2);
	if ( clipin[0] == -1)
		return 0;
	v9 = &clipin[1];
	*out = &clipin[1];
	return clipfinish(v9);
}

int docliplineend(int v1, int v2)
{
	int result; // eax

	result = v1;
	if ( xfpos[result].clip & X_CLIPZ1)
		result = doclipvertex(X_CLIPZ1, v2, result);
	if (xfpos[result].clip & X_CLIPZ2)
		result = doclipvertex(X_CLIPZ2, v2, result);
	if (xfpos[result].clip & X_CLIPX1)
		result = doclipvertex(X_CLIPX1, v2, result);
	if (xfpos[result].clip & X_CLIPX2)
		result = doclipvertex(X_CLIPX2, v2, result);
	if (xfpos[result].clip & X_CLIPY1)
		result = doclipvertex(X_CLIPY1, v2, result);
	if (xfpos[result].clip & X_CLIPY2)
		result = doclipvertex(X_CLIPY2, v2, result);
	return result;
}

int clipline(int v1, int v2, int *out)
{
	int v3; // eax
	int v4; // esi
	int v5; // eax

	clipnewvx = vertices;
	v3 = docliplineend(v1, v2);
	v4 = v3;
	v5 = docliplineend(v2, v3);
	clipbuf1[0] = v4;
	clipbuf1[1] = v5;
	clipbuf1[2] = -1;
	*out = clipbuf1;
	return clipfinish(clipbuf1);
}

int splitpoly(int a1, int a2, DWORD *a3)
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
		if ( grvx[*(DWORD *)(a2 + 4 * v3)].y > grvx[*(DWORD *)(a2 + 4 * i)].y )
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
		if ( grvx[*(DWORD *)(a2 + 4 * v15)].y <= grvx[*v9].y )
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
		memmove(&xfpos[v2], &xfpos[vertices - v1], v1 * sizeof(xt_xfpos));
		memmove(&grvx[v2], &grvx[vertices - v1], v1 * sizeof(GrVertex));
		memmove(&tex[v2], &tex[vertices - v1], v1 * sizeof(xt_tex));
		memmove(&texp[v2], &texp[vertices - v1], v1 * sizeof(xt_tex));
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

int flush_drawfx()
{
	int *v0; // edi
	unsigned int v1; // edx
	int v2; // ecx
	int result; // eax
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
	v0 = (int *)(corner + 1);
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
				v7 = xfpos[v6].clip;
				v5 |= v7;
				result &= v7;
				--v2;
			}
			while ( v2 );
		}
		if ( v5 )
		{
			if ( !result && g_state[XST].xformmode != XFORM_MODE_ORTHO)
			{
				if ( i == 2 )
				{
					result = clipline(v0[0], v0[1], &v16);
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
					// TODO: The call parameters were badly decompiled
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
