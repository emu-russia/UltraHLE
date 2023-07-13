#include "pch.h"

static float identmatrix[4 * 4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

int g_activestateindex;

void x_init(void)
{
	log_open("wt");
	x_log("Init: %s\n", x_version());
}

void x_deinit(void)
{
	signed int v0; // edi
	DWORD *v1; // esi

	v0 = 1;
	v1 = &g_state[159];
	do
	{
		if ( *v1 )
			x_close(v0);
		v1 += 159;
		++v0;
	}
	while ( v1 < &g_state[318] );
	x_log("Deinit: %s\n", x_version());
	return log_open(0);
}

char *x_version()
{
	static char version[64];
	sprintf(version, "%s Build %i (%s)", init_name(), g_version, g_datetime);
	return version;
}

signed int __cdecl x_open(int a1, int a2, int a3, int a4, int a5, int a6)
{
	DWORD *v6; // eax
	signed int v7; // esi
	signed int result; // eax
	int v9; // edi

	v6 = &g_state[159];
	v7 = 1;
	do
	{
		if ( !*v6 )
			break;
		v6 += 159;
		++v7;
	}
	while ( v6 < &g_state[318] );
	if ( v7 >= 2 )
		return -1;
	memset(&g_state[159 * v7], 0, 0x27Cu);
	g_state[159 * v7] = 1;
	g_activestateindex = v7;
	g_activestate = &g_state[159 * v7];
	g_state[161] = a1;
	g_state[162] = a2;
	g_state[163] = a3;
	g_state[164] = a4;
	g_state[165] = a5;
	g_state[166] = a6;
	v9 = init_init();
	mode_init();
	geom_init();
	g_state[234] = 0;
	g_state[290] = 0;
	x_projection(90.0, 1063675494, 65535.0);
	result = -1;
	if ( !v9 )
		result = v7;
	return result;
}

void x_resize(int width, int height)
{
	init_resize(width, height);
}

void x_select(int a1)
{
	if ( a1 > 0 )
	{
		g_activestateindex = a1;
		g_activestate = &g_state[159 * a1];
		init_activate();
	}
	else
	{
		g_activestateindex = -1;
		g_activestate = 0;
	}
}

void x_close(int a1)
{
	DWORD *result; // eax

	result = (DWORD *)a1;
	if ( a1 < 2 )
	{
		result = &g_state[159 * a1];
		if ( *result )
		{
			*result = 0;
			init_deinit();
			text_deinit();
			memset(&g_state[159 * g_activestateindex], 0, 0x27Cu);
			x_select(0);
		}
	}
}

int __cdecl x_getstats(DWORD *a1, unsigned int a2)
{
	static int nowtime = 0;
	static int lasttime = 0;

	bool v2; // zf
	int result; // eax

	nowtime = x_timeus();
	if ( a1 )
	{
		if ( a2 == 104 )
		{
			memcpy(a1, g_stats, 0x68u);
			v2 = nowtime == lasttime;
			*a1 = nowtime - lasttime;
			if ( v2 )
				*a1 = 1;
		}
		else
		{
			memset(a1, 0, a2);
		}
	}
	`x_getstats'::`2'::lasttime = `x_getstats'::`2'::nowtime;
	result = 0;
	g_stats[1] = 0;
	g_stats[2] = 0;
	g_stats[3] = 0;
	g_stats[4] = 0;
	g_stats[5] = 0;
	g_stats[7] = 0;
	return result;
}

int x_query()
{
	return 0;
}

int __cdecl x_clear(int a1, int a2, int a3, int a4, int a5)
{
	return init_clear(a1, a2, a3, a4, a5);
}

int __cdecl x_readfb(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
	int v8; // esi
	int v9; // edx

	if ( (unsigned __int8)a1 == 17 )
	{
		v8 = a4;
		v9 = a7;
		if ( 2 * a4 > a7 )
			return 1;
	}
	else
	{
		if ( (unsigned __int8)a1 != 18 )
			return 1;
		v8 = a4;
		v9 = a7;
		if ( 4 * a4 > a7 )
			return 1;
	}
	return init_readfb(a1, a2, a3, v8, a5, a6, v9);
}

int __cdecl x_writefb(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
	int v8; // esi
	int v9; // edx

	if ( (unsigned __int8)a1 == 17 )
	{
		v8 = a4;
		v9 = a7;
		if ( 2 * a4 > a7 )
			return 1;
	}
	else
	{
		if ( (unsigned __int8)a1 != 18 )
			return 1;
		v8 = a4;
		v9 = a7;
		if ( 4 * a4 > a7 )
			return 1;
	}
	return init_writefb(a1, a2, a3, v8, a5, a6, v9);
}

void x_finish(void)
{
	x_flush();
	init_bufferswap();
	text_frameend();
	x_reset();
	++g_state[167];
}

int __cdecl x_frustum(int a1, int a2, int a3, int a4, int a5, float a6)
{
	int result; // eax

	x_flush();
	g_state[237] = a1;
	result = a4;
	g_state[238] = a2;
	g_state[239] = a3;
	g_state[241] = a5;
	g_state[246] = 0;
	g_state[240] = a4;
	g_state[234] = 1;
	*(float *)&g_state[242] = a6;
	*(float *)&g_state[244] = 1.0 / a6;
	*(float *)&g_state[245] = 1.0 / *(float *)&a5;
	return result;
}

void x_projmatrix(xt_matrix* a1)
{
	int result; // eax
	const void *v2; // esi

	x_flush();
	v2 = a1;
	if ( a1 )
	{
		g_state[236] = 0;
	}
	else
	{
		g_state[236] = 1;
		v2 = &identmatrix;
	}
	memcpy(&g_state[217], v2, 0x40u);
	g_state[246] = 2;
	g_state[234] = 1;
}

void projrecalced()
{
}

void x_ortho(float xmin, float ymin, float xmax, float ymax, float znear, float zfar)
{
	x_flush();
	g_state[237] = xmin;
	g_state[238] = ymin;
	g_state[239] = xmax;
	g_state[241] = znear;
	g_state[240] = ymax;
	g_state[242] = zfar;
	g_state[246] = 1;
	g_state[234] = 1;
	g_state[244] = 1.0 / zfar;
	g_state[245] = 1.0 / znear;
}

void x_viewport(float x0, float y0, float x1, float y1)
{
	int v5; // ST00_4
	double v6; // st7

	x_flush();
	g_state[248] = x0;
	v5 = g_state[164] - 1;
	g_state[249] = x1;
	v6 = (double)v5;
	g_state[234] = 1;
	g_state[250] = v6 - y1;
	g_state[251] = v6 - y0;
}

int __cdecl x_projection(float a1, int a2, float a3)
{
	int v3; // ST08_4
	int v4; // ST04_4
	int result; // eax
	long double v6; // st7
	int v7; // ST0C_4
	int v8; // ST08_4
	int v9; // ST04_4
	int v10; // ST00_4

	x_flush();
	if ( SLODWORD(a1) >= 1065353216 )
	{
		v6 = tan(a1 * 0.0174532925199433 * 0.5) * *(float *)&a2;
		*(float *)&v7 = 0.75 * v6;
		*(float *)&v8 = -(0.75 * v6);
		*(float *)&v9 = v6;
		*(float *)&v10 = -v6;
		result = x_frustum(v10, v9, v8, v7, a2, a3);
	}
	else
	{
		*(float *)&v3 = (double)(signed int)g_state[164];
		*(float *)&v4 = (double)(signed int)g_state[163];
		x_ortho(0, v4, v3, 0, a2, a3);
	}
	return result;
}

int x_zrange(float znear, float zfar)
{
	x_flush();
	g_state[290] = 1;
	g_state[234] = 1;
	g_state[241] = znear;
	g_state[242] = zfar;
	return 0;
}

int x_zdecal(float factor)
{
	x_flush();
	g_state[243] = factor;
	return 0;
}

DWORD *__cdecl texture_get(signed int a1)
{
	DWORD *result; // eax

	if ( a1 <= 0 || a1 > 1024 )
	{
		g_state[168] |= 0x20000000u;
		result = 0;
	}
	else
	{
		result = (DWORD *)((char *)&g_texture + 152 * a1);
		if ( !*result )
		{
			x_log("undefined texture xhandle %i selected\n");
			result = 0;
			g_state[168] |= 0x20000000u;
		}
	}
	return result;
}

signed int __cdecl x_createtexture(int a1, signed int a2, signed int a3)
{
	DWORD *v3; // eax
	signed int v4; // esi
	signed int result; // eax
	DWORD *v6; // ebx
	signed int v7; // edx
	signed int v8; // ecx
	int v9; // eax

	v3 = &g_texture[38];
	v4 = 1;
	do
	{
		if ( !*v3 )
			break;
		v3 += 38;
		++v4;
	}
	while ( v3 < &g_texture[38912] );
	if ( v4 == 1024 )
	{
		x_log("too many textures\n");
		result = -1;
		g_state[168] |= 0x10000000u;
	}
	else
	{
		if ( g_lasttexture < v4 )
			g_lasttexture = v4;
		g_texture[38 * v4] = g_activestateindex;
		v6 = texture_get(v4);
		if ( v6 )
		{
			memset(v6, 0, 0x98u);
			v7 = a2;
			v8 = a3;
			v6[1] = v4;
			*v6 = g_activestateindex;
			v6[2] = a2;
			v6[3] = a3;
			v6[4] = a1;
			if ( a1 & 0x200 )
			{
				v9 = 0;
				while ( v7 > 1 || v8 > 1 )
				{
					v7 >>= 1;
					++v9;
					v8 >>= 1;
				}
				v6[7] = v9;
			}
			else
			{
				v6[7] = 1;
			}
			text_allocdata(v6);
			g_stats[6] += v6[6];
			result = v4;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}

//int x_gettextureinfo(int handle, int* format, int* memformat, int* width, int* height)
int x_gettextureinfo(int a1, int* a2, int* a3, int* a4, int* a5)
{
	DWORD *v5; // eax

	v5 = texture_get(a1);
	if ( !v5 )
		return 1;
	if ( a2 )
		*a2 = v5[4];
	if ( a3 )
		*a3 = v5[5];
	if ( a4 )
		*a4 = v5[2];
	if ( a5 )
		*a5 = v5[3];
	return 0;
}

int __cdecl x_loadtexturelevel(signed int a1, signed int a2, int a3)
{
	DWORD *v3; // eax
	DWORD *v4; // esi
	int v6; // edx

	v3 = texture_get(a1);
	v4 = v3;
	if ( !v3 )
		return 0;
	if ( v3[7] < a2 + 1 )
		v3[7] = a2 + 1;
	if ( a2 > 31 )
		return 0;
	v6 = v3[8];
	if ( !(v6 & (1 << a2)) )
		v3[8] = (1 << a2) | v6;
	text_loadlevel(v3, a2, a3);
	return 2 - ((v4[4] & 0x200u) < 1);
}

DWORD *__cdecl x_freetexture(signed int a1)
{
	DWORD *result; // eax

	result = texture_get(a1);
	if ( result )
		result = (DWORD *)text_freedata(result);
	return result;
}

signed int x_texture_getinfo(signed int a1, DWORD *a2, DWORD *a3, DWORD *a4, DWORD *a5)
{
	DWORD *v5; // eax

	v5 = texture_get(a1);
	if ( !v5 )
		return 1;
	if ( a2 )
		*a2 = v5[4];
	if ( a3 )
		*a3 = v5[5];
	if ( a4 )
		*a4 = v5[2];
	if ( a5 )
		*a5 = v5[3];
	return 0;
}

void x_cleartexmem(void)
{
	return text_cleartexmem();
}

uchar* x_opentexturedata(int a1)
{
	DWORD *v1; // eax
	int result; // eax

	v1 = texture_get(a1);
	if ( v1 && *((BYTE *)v1 + 17) & 4 )
		result = text_opendata(v1);
	else
		result = 0;
	return result;
}

void x_closetexturedata(int a1)
{
	DWORD *result; // eax

	result = texture_get(a1);
	if ( result )
	{
		if ( *((BYTE *)result + 17) & 4 )
			result = (DWORD *)text_closedata(result);
	}
	return result;
}

void x_forcegeometry(int forceon, int forceoff)
{
	g_state[259] = forceon;
	g_state[260] = forceoff;
	g_state[290] |= 1u;
}

void x_geometry(int flags)
{
	int v1; // ecx
	int result; // eax

	v1 = g_state[259];
	result = ~g_state[260];
	g_state[290] |= 1u;
	g_state[317] = result & (flags | v1);
}

int x_mask(int colormask, int depthmask, int depthtest)
{
	signed int v3; // ecx

	v3 = 0;
	if (colormask == 4097 )
	{
		v3 = 1;
	}
	else if (colormask != 4098 )
	{
		return 1;
	}
	if (depthmask == 4097 )
	{
		v3 |= 2u;
	}
	else if (depthmask != 4098 )
	{
		return 1;
	}
	if ( !depthtest || depthtest == 1 )
		return 1;
	g_state[292] = depthtest;
	g_state[291] = v3;
	g_state[290] |= 4u;
	return 0;
}

int x_dither(int type)
{
	if (type == X_ENABLE)
	{
		g_state[303] = 1;
LABEL_5:
		g_state[290] |= 4u;
		return 0;
	}
	if (type == X_DISABLE)
	{
		g_state[303] = 0;
		goto LABEL_5;
	}
	return 1;
}

int x_blend(int src, int dst)
{
	if (src < 4609 || src > 4616 )
		return 1;
	if (dst < 4609 || dst > 4616 )
		return 1;
	g_state.src = src;
	g_state.dst = dst;
	g_state[290] |= 4u;
	return 0;
}

int x_alphatest(float limit)
{
	if ( g_state[317] & 1 )
		limit = 1.0f;
	if ( limit < 0.0f || limit > 1.0f)
		return 1;
	g_state[290] |= 4u;
	g_state.alphatest = limit;
	return 0;
}

int x_combine(int colortext1)
{
	if (colortext1 < 4865 || colortext1 > 4879 )
		return 1;
	g_state[294] = 0;
	g_state[290] |= 4u;
	g_state[293] = colortext1;
	return 0;
}

int x_procombine(int rgb, int alpha)
{
	g_state[294] = 0;
	g_state[290] |= 4u;
	g_state[293] = rgb | (alpha << 16);
	return 0;
}

int x_envcolor(float r, float g, float b, float a)
{
	*(float *)&g_state[313] = r;
	*(float *)&g_state[314] = g;
	*(float *)&g_state[315] = b;
	*(float *)&g_state[316] = a;
	g_state[290] |= 4u;
	g_state[312] = (unsigned __int8)(signed __int64)(r * 255.0) | ((unsigned __int8)(signed __int64)(b * 255.0) << 16) | ((((unsigned int)(signed __int64)(a * 255.0) << 16) | (unsigned __int8)(signed __int64)(g * 255.0)) << 8);
	return 0;
}

int x_combine2(int colortext1, int text1text2, int sametex)
{
	if ( g_state[160] < 2 )
		return 1;
	if (colortext1 < X_FIRSTCOMBINE || colortext1 > X_LASTCOMBINE)
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state[293] = colortext1;
	g_state.sametex = sametex;
	g_state[294] = text1text2;
	g_state[290] |= 4u;
	return 0;
}

int x_procombine2(int rgb, int alpha, int text1text2, int sametex)
{
	if ( g_state[160] < 2 )
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state.sametex = sametex;
	g_state[293] = rgb | (alpha << 16);
	g_state[294] = text1text2;
	g_state[290] |= 4u;
	return 0;
}

int x_texture(int text1handle)
{
	if (text1handle <= 0 )
		return 1;
	g_state[311] = 0;
	g_state[290] |= 2u;
	g_state[310] = text1handle;
	return 0;
}

int x_texture2(int text1handle, int text2handle)
{
	if ( g_state[160] < 2 )
		return 1;
	if (text1handle <= 0 || text2handle <= 0 )
		return 1;
	g_state[310] = text1handle;
	g_state[311] = text2handle;
	g_state[290] |= 2u;
	return 0;
}

void x_reset(void)
{
	x_geometry(4);
	x_mask(4097, 4097, 4097);
	x_dither(1);
	x_blend(4610, 4609);
	x_alphatest(1.0f);
	x_combine(4866);
	x_zdecal(1.0f);
}

int x_fog(int type, float min, float max, float r, float g, float b)
{
	g_state[296] = type;
	g_state[297] = min;
	g_state[298] = max;
	g_state[299] = r;
	g_state[300] = g;
	g_state[301] = b;
	g_state[302] = 1.0f;
	g_state[290] |= 8u;
	return 0;
}

void x_fullscreen(int fullscreen)
{
	init_fullscreen(fullscreen);
}
