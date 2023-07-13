#include "pch.h"

//.data:00000000 _data           segment para public 'DATA' use32
//.data:00000000                 assume cs:_data
//.data:00000000 ; `x_getstats'::`2'::lasttime
//.data:00000000 ?lasttime@?1??x_getstats@@9@9 dd 0      ; DATA XREF: _x_getstats+31↓r
//.data:00000000                                         ; _x_getstats+5A↓w
//.data:00000004 ; `x_getstats'::`2'::nowtime
//.data:00000004 ?nowtime@?1??x_getstats@@9@9 dd 0       ; DATA XREF: _x_getstats+C↓w
//.data:00000004                                         ; _x_getstats+2C↓r ...
//.data:00000008 _identmatrix$S1310 db    0              ; DATA XREF: _x_projmatrix+19↓o
//.data:00000009                 db    0
//.data:0000000A                 db  80h
//.data:0000000B                 db  3Fh ; ?
//.data:0000000C                 db    0
//.data:0000000D                 db    0
//.data:0000000E                 db    0
//.data:0000000F                 db    0
//.data:00000010                 db    0
//.data:00000011                 db    0
//.data:00000012                 db    0
//.data:00000013                 db    0
//.data:00000014                 db    0
//.data:00000015                 db    0
//.data:00000016                 db    0
//.data:00000017                 db    0
//.data:00000018                 db    0
//.data:00000019                 db    0
//.data:0000001A                 db    0
//.data:0000001B                 db    0
//.data:0000001C                 db    0
//.data:0000001D                 db    0
//.data:0000001E                 db  80h
//.data:0000001F                 db  3Fh ; ?
//.data:00000020                 db    0
//.data:00000021                 db    0
//.data:00000022                 db    0
//.data:00000023                 db    0
//.data:00000024                 db    0
//.data:00000025                 db    0
//.data:00000026                 db    0
//.data:00000027                 db    0
//.data:00000028                 db    0
//.data:00000029                 db    0
//.data:0000002A                 db    0
//.data:0000002B                 db    0
//.data:0000002C                 db    0
//.data:0000002D                 db    0
//.data:0000002E                 db    0
//.data:0000002F                 db    0
//.data:00000030                 db    0
//.data:00000031                 db    0
//.data:00000032                 db  80h
//.data:00000033                 db  3Fh ; ?
//.data:00000034                 db    0
//.data:00000035                 db    0
//.data:00000036                 db    0
//.data:00000037                 db    0
//.data:00000038                 db    0
//.data:00000039                 db    0
//.data:0000003A                 db    0
//.data:0000003B                 db    0
//.data:0000003C                 db    0
//.data:0000003D                 db    0
//.data:0000003E                 db    0
//.data:0000003F                 db    0
//.data:00000040                 db    0
//.data:00000041                 db    0
//.data:00000042                 db    0
//.data:00000043                 db    0
//.data:00000044                 db    0
//.data:00000045                 db    0
//.data:00000046                 db  80h
//.data:00000047                 db  3Fh ; ?
//.data:00000075                 align 4
//.data:0000009F                 align 10h
//.data:000000B3                 align 4
//.data:000000B3 _data           ends


void x_init(void)
{
	int v0; // eax

	log_open("wt");
	v0 = x_version();
	x_log("Init: %s\n", v0);
}

void x_deinit(void)
{
	signed int v0; // edi
	DWORD *v1; // esi
	int v2; // eax

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
	v2 = x_version();
	x_log("Deinit: %s\n", v2);
	return log_open(0);
}

char *x_version()
{
	static char version[64];
	int v0; // eax

	v0 = init_name(g_version, g_datetime);
	sprintf(version, "%s Build %i (%s)", v0);
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
	int result; // eax

	result = a1;
	if ( a1 > 0 )
	{
		g_activestateindex = a1;
		g_activestate = &g_state[159 * a1];
		result = init_activate();
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
	bool v2; // zf
	int result; // eax

	`x_getstats'::`2'::nowtime = x_timeus();
	if ( a1 )
	{
		if ( a2 == 104 )
		{
			memcpy(a1, g_stats, 0x68u);
			v2 = `x_getstats'::`2'::nowtime == `x_getstats'::`2'::lasttime;
			*a1 = `x_getstats'::`2'::nowtime - `x_getstats'::`2'::lasttime;
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
		v2 = &identmatrix_S1310;
	}
	memcpy(&g_state[217], v2, 0x40u);
	g_state[246] = 2;
	g_state[234] = 1;
}

void projrecalced()
{
}

//void x_ortho(float xmin, float ymin, float xmax, float ymax, float znear, float zfar)
void x_ortho(float a1, float a2, float a3, float a4, float a5, float a6)
{
	x_flush();
	g_state[237] = a1;
	g_state[238] = a2;
	g_state[239] = a3;
	g_state[241] = a5;
	g_state[240] = a4;
	*(float *)&g_state[242] = a6;
	g_state[246] = 1;
	g_state[234] = 1;
	*(float *)&g_state[244] = 1.0 / a6;
	*(float *)&g_state[245] = 1.0 / a5;
}

int __cdecl x_viewport(int a1, float a2, int a3, float a4)
{
	int result; // eax
	int v5; // ST00_4
	double v6; // st7

	x_flush();
	g_state[248] = a1;
	result = g_state[164] - 1;
	v5 = g_state[164] - 1;
	g_state[249] = a3;
	v6 = (double)v5;
	g_state[234] = 1;
	*(float *)&g_state[250] = v6 - a4;
	*(float *)&g_state[251] = v6 - a2;
	return result;
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

int __cdecl x_forcegeometry(int a1, int a2)
{
	int result; // eax

	result = a1;
	g_state[259] = a1;
	g_state[260] = a2;
	g_state[290] |= 1u;
	return result;
}

int __cdecl x_geometry(int a1)
{
	int v1; // ecx
	int result; // eax

	v1 = g_state[259];
	result = ~g_state[260];
	g_state[290] |= 1u;
	g_state[317] = result & (a1 | v1);
	return result;
}

signed int __cdecl x_mask(int a1, int a2, int a3)
{
	signed int v3; // ecx

	v3 = 0;
	if ( a1 == 4097 )
	{
		v3 = 1;
	}
	else if ( a1 != 4098 )
	{
		return 1;
	}
	if ( a2 == 4097 )
	{
		v3 |= 2u;
	}
	else if ( a2 != 4098 )
	{
		return 1;
	}
	if ( !a3 || a3 == 1 )
		return 1;
	g_state[292] = a3;
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
	g_state[304] = src;
	g_state[305] = dst;
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
	g_state[308] = limit;
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

int __cdecl x_envcolor(float a1, float a2, float a3, float a4)
{
	int result; // eax

	*(float *)&g_state[313] = a1;
	*(float *)&g_state[314] = a2;
	*(float *)&g_state[315] = a3;
	*(float *)&g_state[316] = a4;
	g_state[290] |= 4u;
	result = 0;
	g_state[312] = (unsigned __int8)(signed __int64)(a1 * 255.0) | ((unsigned __int8)(signed __int64)(a3 * 255.0) << 16) | ((((unsigned int)(signed __int64)(a4 * 255.0) << 16) | (unsigned __int8)(signed __int64)(a2 * 255.0)) << 8);
	return result;
}

int x_combine2(int colortext1, int text1text2, int sametex)
{
	signed int result; // eax

	if ( g_state[160] < 2 )
		return 1;
	if (colortext1 < 4865 || colortext1 > 4879 )
		return 1;
	if (text1text2 < 4865 || text1text2 > 4879 )
		return 1;
	g_state[293] = colortext1;
	g_state[306] = sametex;
	g_state[294] = text1text2;
	result = 0;
	g_state[290] |= 4u;
	return result;
}

//int x_procombine2(int rgb, int alpha, int text1text2, int sametex)
int x_procombine2(int a1, int a2, int a3, int a4)
{
	signed int result; // eax

	if ( g_state[160] < 2 )
		return 1;
	if ( a3 < 4865 || a3 > 4879 )
		return 1;
	g_state[306] = a4;
	result = 0;
	g_state[293] = a1 | (a2 << 16);
	g_state[294] = a3;
	g_state[290] |= 4u;
	return result;
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
	x_alphatest(1065353216);
	x_combine(4866);
	x_zdecal(1065353216);
}

//int x_fog(int type, float min, float max, float r, float g, float b)
int x_fog(int a1, float a2, float a3, float a4, float a5, float a6)
{
	int result; // eax

	g_state[296] = a1;
	g_state[297] = a2;
	g_state[298] = a3;
	g_state[299] = a4;
	result = 0;
	g_state[300] = a5;
	g_state[302] = 1.0f;
	g_state[290] |= 8u;
	g_state[301] = a6;
	return result;
}

void x_fullscreen(int fullscreen)
{
	init_fullscreen(fullscreen);
}



//.rdata:00000F54 _rdata          segment para public 'DATA' use32
//.rdata:00000F54                 assume cs:_rdata
//.rdata:00000F54                 ;org 0F54h
//.rdata:00000F54 $T1520          db    0
//.rdata:00000F55                 db 0FFh
//.rdata:00000F56                 db  7Fh ; 
//.rdata:00000F57                 db  47h ; G
//.rdata:00000F58 $T1521          db  66h ; f
//.rdata:00000F59                 db  66h ; f
//.rdata:00000F5A                 db  66h ; f
//.rdata:00000F5B                 db  3Fh ; ?
//.rdata:00000F5C $T1522          db    0
//.rdata:00000F5D                 db    0
//.rdata:00000F5E                 db 0B4h
//.rdata:00000F5F                 db  42h ; B
//.rdata:00000F60 $T1526          dd 1.0                  ; DATA XREF: _x_frustum+2E↑r
//.rdata:00000F60                                         ; _x_frustum+67↑r ...
//.rdata:00000F64 $T1527          db    0
//.rdata:00000F65                 db    0
//.rdata:00000F66                 db    0
//.rdata:00000F67                 db    0
//.rdata:00000F68                 db    0
//.rdata:00000F69                 db    0
//.rdata:00000F6A                 db 0F0h
//.rdata:00000F6B                 db  3Fh ; ?
//.rdata:00000F6C $T1528          db    0
//.rdata:00000F6D                 db    0
//.rdata:00000F6E                 db    0
//.rdata:00000F6F                 db    0
//.rdata:00000F70                 db    0
//.rdata:00000F71                 db    0
//.rdata:00000F72                 db    0
//.rdata:00000F73                 db    0
//.rdata:00000F74 $T1529          dq 0.0174532925199433   ; DATA XREF: _x_projection+4F↑r
//.rdata:00000F7C $T1530          dd 0.5                  ; DATA XREF: _x_projection+55↑r
//.rdata:00000F80                 db    0
//.rdata:00000F81                 db    0
//.rdata:00000F82                 db    0
//.rdata:00000F83                 db    0
//.rdata:00000F84 $T1531          dq 0.75                 ; DATA XREF: _x_projection+73↑r
//.rdata:00000F8C $T1536          dq 255.0                ; DATA XREF: _x_envcolor+11↑r
//.rdata:00000F8C                                         ; _x_envcolor+3A↑r ...
//.rdata:00000F8C _rdata          ends

