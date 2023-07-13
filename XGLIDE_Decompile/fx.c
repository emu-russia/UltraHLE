#include "pch.h"

char *init_name()
{
	return "Glide";
}

int init_fullscreen(int fullscreen)
{
	int result; // eax

	if (fullscreen)
		result = grSstControl(GR_CONTROL_ACTIVATE);
	else
		result = grSstControl(GR_CONTROL_DEACTIVATE);
	return result;
}

int init_query()
{
	int result; // eax
	GrHwConfiguration v1; // [esp+0h] [ebp-94h]
	int v2; // [esp+4h] [ebp-90h]

	// Badly decompiled and not used.

	//grGlideInit();
	//if ( grSstQueryHardware(&v1) )
	//	result = ((unsigned int)(v2 - 3) < 1) + 1;
	//else
	//	result = -1;
	//return result;

	return -1;
}

void init_reinit()
{
	GrScreenResolution_t mode;

	x_log("init_reinit: shutdown\n");
	grGlideShutdown();
	x_log("init_reinit: init\n");
	grGlideInit();
	x_log("init_reinit: select\n");
	grSstSelect(0);
	x_log("init_reinit: resolution\n");
	if ( g_state.xs < 1024 )
	{
		if ( g_state.xs < 800 )
		{
			if ( g_state.xs < 640 )
			{
				mode = GR_RESOLUTION_512x384;
				if ( g_state.xs < 512 )
					mode = GR_RESOLUTION_640x480;
			}
			else
			{
				mode = GR_RESOLUTION_640x480;
			}
		}
		else
		{
			mode = GR_RESOLUTION_800x600;
		}
	}
	else
	{
		// 1024x768
		mode = 12;
	}
	grSstWinOpen((FxU32)g_state.hwnd, mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT, g_state.buffers, g_state.vsync);
	grClipWindow(0, 0, g_state.xs, g_state.ys);
	x_log("init_reinit: done\n");
}

int init_init()
{
	GrScreenResolution_t mode;
	GrHwConfiguration hw;

	grGlideInit();
	if ( !grSstQueryHardware(&hw) )
		return -1;
	if (hw.SSTs[0].type != 3 && hw.SSTs[0].type)
		g_state.tmus = 1;
	else
		g_state.tmus = hw.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
	x_log("x_open: cartdtype=%i, tmus=%i\n", hw.SSTs[0].type, g_state.tmus);

	grSstSelect(0);
	text_init();
	if ( g_state.xs < 1024 )
	{
		if ( g_state.xs < 800 )
		{
			if ( g_state.xs < 640 )
			{
				mode = GR_RESOLUTION_512x384;
				if ( g_state.xs < 512 )
					mode = GR_RESOLUTION_640x480;
			}
			else
			{
				mode = GR_RESOLUTION_640x480;
			}
		}
		else
		{
			mode = GR_RESOLUTION_800x600;
		}
	}
	else
	{
		mode = 12;
	}
	grSstWinOpen((FxU32)g_state.hwnd, mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT, g_state.buffers, g_state.vsync);
	grClipWindow(0, 0, g_state.xs, g_state.ys);
	return 0;
}

void init_deinit()
{
	x_log("x_close");
	grGlideShutdown();
}

void init_activate()
{
}

void init_resize(int xs, int ys)
{
	g_state.xs = xs;
	g_state.ys = ys;
}

void init_bufferswap()
{
	signed int v0; // esi
	signed int v1; // eax

	v0 = 0;
	while ( grBufferNumPending() > 3 )
	{
		mysleep(10);
		v1 = v0++;
		if ( v1 > 100 )
		{
			x_log("timeout on bufferswap!\n");
			init_reinit();
			return;
		}
	}
	grBufferSwap(1);
}

void init_clear(int writecolor, int writedepth, float cr, float cg, float cb)
{
	x_flush();
	grClipWindow(
		g_state.view_x0,
		g_state.view_y0,
		g_state.view_x1,
		g_state.view_y1);
	grColorMask(writecolor >= 1 ? FXTRUE : FXFALSE, writecolor >= 1 ? FXTRUE : FXFALSE);
	grDepthMask(writedepth >= 1 ? FXTRUE : FXFALSE);
	grBufferClear(
		(unsigned __int8)(signed __int64)(cr * 255.0) | ((unsigned __int16)(signed __int64)(cg * 255.0) << 8) & 0xFF00 | ((unsigned __int8)(signed __int64)(cb * 255.0) << 16),
		0,
		0xFFFF);
	grDepthMask((g_state.currentmode.mask & 2u) >> 1);
	grColorMask(g_state.currentmode.mask & 1, g_state.currentmode.mask & 1);
}

int init_readfb(__int16 a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
	BOOL v7; // ecx
	signed int result; // eax
	int v9; // esi
	unsigned __int16 *v10; // edi
	int v11; // ebx
	unsigned __int16 v12; // ax

	v7 = (a1 & 0x100u) < 1;
	if ( (unsigned __int8)a1 == 17 )
	{
		grLfbReadRegion(v7, a2, a3, a4, a5, a7, a6);
		result = 0;
	}
	else if ( (unsigned __int8)a1 == 18 )
	{
		v9 = a6;
		v10 = (unsigned __int16 *)(a6 + a5 * a7 / 2);
		v11 = a4 * a5;
		grLfbReadRegion(v7, a2, a3, a4, a5, a7 / 2, a6 + a5 * a7 / 2);
		if ( a4 * a5 > 0 )
		{
			do
			{
				v12 = *v10;
				++v10;
				v9 += 4;
				*(BYTE *)(v9 - 4) = HIBYTE(v12) & 0xF8;
				--v11;
				*(BYTE *)(v9 - 3) = (v12 >> 3) & 0xFC;
				*(BYTE *)(v9 - 2) = 8 * v12;
				*(BYTE *)(v9 - 1) = -1;
			}
			while ( v11 );
		}
		result = 0;
	}
	else
	{
		result = 1;
	}
	return result;
}

int init_writefb()
{
	// Not implemented
	return 1;
}

void mode_init()
{
	g_state.currentmode.stwhint = 0;
	grDepthBiasLevel(0);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthMask(FXTRUE);
	grDitherMode(GR_DITHER_4x4);
	grHints(GR_HINT_ALLOW_MIPMAP_DITHER, 1);
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
}

void mode_texturemode(int a1, __int16 a2, int a3)
{
	int v3; // esi
	__int16 v4; // di

	if ( a3 )
	{
		v3 = a1;
		grTexMipMapMode(a1, 1, 1);
		v4 = a2;
	}
	else
	{
		v4 = a2;
		v3 = a1;
		grTexMipMapMode(a1, (a2 & 0x200) != 0, 0);
	}
	if ( v4 & 0x100 )
		grTexClampMode(v3, (v4 & 0x1000u) < 1, (v4 & 0x2000u) < 1);
	else
		grTexClampMode(v3, 0, 0);
	if ( v4 & 0x800 )
		grTexFilterMode(v3, 0, 0);
	else
		grTexFilterMode(v3, 1, 1);
}

void mode_loadtexture(int a1)
{
	signed int v1; // ebx
	int result; // eax
	int v3; // esi
	int v4; // edi

	v1 = 1;
	if ( g_state.tmus < 2 )
		v1 = 0;
	result = texture_get(a1);
	v3 = result;
	if ( result )
	{
		v4 = *(DWORD *)(result + 16);
		if ( !(v4 & 0x200) )
			v1 = 0;
		grTexCombine(1, 1, 0, 1, 0, 0, 0);
		if ( v1 )
		{
			if ( fxloadtexture_trilin(v3) )
				grTexCombine(0, 7, 13, 7, 13, 0, 0);
			else
				grTexCombine(0, 7, 5, 7, 5, 0, 0);
			mode_texturemode(0, v4, 1);
			mode_texturemode(1, v4, 1);
		}
		else if ( fxloadtexture_single(v3) )
		{
			grTexCombine(0, 7, 8, 7, 8, 0, 0);
			mode_texturemode(0, v4, 0);
			mode_texturemode(1, v4, 0);
			grTexCombine(1, 1, 0, 1, 0, 0, 0);
		}
		else
		{
			grTexCombine(0, 1, 0, 1, 0, 0, 0);
			mode_texturemode(0, v4, 0);
		}
	}
}

void mode_loadmultitexture(int a1, int a2)
{
	int result; // eax
	int v3; // ebx
	int v4; // edi
	int v5; // esi

	result = texture_get(a1);
	v3 = result;
	if ( result )
	{
		v4 = *(DWORD *)(result + 16);
		result = texture_get(a2);
		if ( result )
		{
			v5 = *(DWORD *)(result + 16);
			fxloadtexture_multi(v3, result);
			mode_texturemode(0, v4, 0);
			mode_texturemode(1, v5, 0);
		}
	}
}

void fixfogtable(GrFog_t* a1, int a2)
{
	int i; // esi
	unsigned __int8 v3; // dl
	int v4; // ecx

	for ( i = a2 - 1; i >= 0; --i )
	{
		v3 = *(BYTE *)(i + a1);
		v4 = *(unsigned __int8 *)(i + a1 + 1) - v3;
		if ( v4 > 60 )
			*(BYTE *)(i + a1) = v3 + v4 - 60;
	}
}

GrFog_t* generatefogtable()
{
	float v4; // ST08_4
	float v5; // ST08_4
	float v6; // ST04_4

	static int lastfogtype;
	static float lastfogmin;
	static float lastfogmax;
	static GrFog_t fogtable[GR_FOG_TABLE_SIZE];

	if ( lastfogmin == g_state.active.fogmin
		&& lastfogmax == g_state.active.fogmax
		&& g_state.active.fogtype == lastfogtype )
	{
		return fogtable;
	}

	lastfogmin = g_state.active.fogmin;
	lastfogmax = g_state.active.fogmax;
	lastfogtype = g_state.active.fogtype;
	if (g_state.active.fogtype == X_LINEAR || g_state.active.fogtype == X_LINEARADD)
	{
		v5 = g_state.active.fogmax / g_state.znear;
		v6 = g_state.active.fogmin / g_state.znear;
		guFogGenerateLinear(fogtable, v6, v5);
	}
	else if (g_state.active.fogtype == X_EXPONENTIAL)
	{
		v4 = 2.3f / (g_state.active.fogmax / g_state.znear);
		guFogGenerateExp(fogtable, v4);
	}
	fixfogtable(fogtable, GR_FOG_TABLE_SIZE);
	return fogtable;
}

void mode_change()
{
	float a1; // @ecx 
	signed __int64 v16; // rax
	signed int v17; // edx
	signed int v18; // esi
	signed int v19; // eax
	signed int v20; // ecx
	bool v23; // zf
	float v25; // eax
	float result; // eax

	if ( g_state.changed & 1 )
	{
		if ( g_state.setnew != g_state.geometry)
		{
			if (g_state.geometry & 2 )
			{
				grCullMode(GR_CULL_NEGATIVE);
			}
			else if ( g_state.geometry & 4 )
			{
				grCullMode(GR_CULL_POSITIVE);
			}
			else
			{
				grCullMode(GR_CULL_DISABLE);
			}
			g_state.setnew = g_state.geometry;
		}
	}
	if ( g_state.changed & 8 )
	{
		g_state.active.fogtype = g_state.currentmode.fogtype;
		g_state.active.fogmin = g_state.currentmode.fogmin;
		g_state.active.fogmax = g_state.currentmode.fogmax;
		g_state.active.fogcolor[0] = g_state.currentmode.fogcolor[0];
		g_state.active.fogcolor[1] = g_state.currentmode.fogcolor[1];
		g_state.active.fogcolor[2] = g_state.currentmode.fogcolor[2];
		g_state.active.fogcolor[3] = g_state.currentmode.fogcolor[3];
		if ( g_state.currentmode.fogtype == X_DISABLE)
		{
			grFogMode(GR_FOG_DISABLE);
		}
		else
		{
			grFogTable(generatefogtable());
			grFogColorValue(
				(unsigned __int8)(signed __int64)(g_state.active.fogcolor[0] * 255.0) | 
				((unsigned __int16)(signed __int64)(g_state.active.fogcolor[1] * 255.0) << 8) & 0xFF00 | 
				(((unsigned int)(signed __int64)(g_state.active.fogcolor[2] * 255.0) | 0xFFFFFF00) << 16)
			);
			if ( g_state.active.fogtype == X_LINEARADD)
				grFogMode(GR_FOG_ADD2 | GR_FOG_WITH_TABLE);
			else
				grFogMode(GR_FOG_WITH_TABLE);
		}
	}
	if ( g_state.changed & 4 )
	{
		if ( g_state.active.mask != g_state.currentmode.mask)
		{
			grDepthMask((g_state.currentmode.mask & 2u) >> 1);
			grColorMask(g_state.currentmode.mask & 1, g_state.currentmode.mask & 1);
			g_state.active.mask = g_state.currentmode.mask;
		}
		if ( g_state.active.masktst != g_state.currentmode.masktst)
		{
			if (g_state.currentmode.masktst > X_DISABLE)
			{
				switch (g_state.currentmode.masktst)
				{
					case X_TESTEQ:
						grDepthBufferFunction(2);
						break;
					case X_TESTNE:
						grDepthBufferFunction(5);
						break;
					case X_TESTGT:
						grDepthBufferFunction(4);
						break;
					case X_TESTLT:
						grDepthBufferFunction(1);
						break;
					default:
						goto $L1409;
				}
			}
			else if (g_state.currentmode.masktst == X_DISABLE || g_state.currentmode.masktst == 0 )
			{
				grDepthBufferFunction(7);
			}
			else
			{
$L1409:
				grDepthBufferFunction(3);
			}
			g_state.active.masktst = g_state.currentmode.masktst;
		}
		if ( g_state.currentmode.envc != g_state.active.envc)
		{
			g_state.active.colortext1 = -6.8056469e38/*NaN*/;
			g_state.active.envc = g_state.currentmode.envc;
		}
		if ( g_state.currentmode.colortext1 != g_state.active.colortext1 )
		{
			g_state.send &= 0xFFFFFFFC;
			switch ( g_state.currentmode.colortext1 & 0xFFFF )
			{
				case X_WHITE:
					grConstantColorValue(0x7FFF7FFF);
					guColorCombineFunction(GR_COLORCOMBINE_CCRGB);
					guAlphaSource(0);
					break;
				case X_COLOR:
					guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
					guAlphaSource(1);
					g_state.send |= 1u;
					break;
				case X_TEXTURE:
				case X_DECAL:
					guColorCombineFunction(GR_COLORCOMBINE_DECAL_TEXTURE);
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_49;
				case X_ADD:
					guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_ADD_ITRGB);
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_48;
				case X_MUL:
					guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA);
					goto LABEL_48;
				case X_TEXTURE_IA:
					guColorCombineFunction(GR_COLORCOMBINE_DECAL_TEXTURE);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					goto LABEL_48;
				case X_MUL_TA:
					guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_48;
				case X_MUL_IA:
					guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					goto LABEL_48;
				case X_TEXTUREBLEND:
					grColorCombine(7, 4, 0, 1, 0);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					goto LABEL_48;
				case X_TEXTUREENVA:
					grColorCombine(7, 4, 1, 0, 0);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					goto LABEL_47;
				case X_TEXTUREENVC:
					grColorCombine(7, 5, 1, 0, 0);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					goto LABEL_47;
				case X_SUB:
					grColorCombine(6, 8, 0, 1, 0);
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_48;
				case X_TEXTUREENVCR:
					grColorCombine(7, 5, 0, 2, 0);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					x_log("envc=%08X\n", g_state.active.envc);
LABEL_47:
					grConstantColorValue(g_state.active.envc);
LABEL_48:
					g_state.send |= 1u;
LABEL_49:
					g_state.send |= 2u;
					break;
				default:
					break;
			}
			switch ( g_state.currentmode.colortext1 >> 16 )
			{
				case X_COLOR:
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					g_state.send |= 1u;
					break;
				case X_TEXTURE:
				case X_DECAL:
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_56;
				case X_ADD:
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);
					goto LABEL_55;
				case X_MUL:
					guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA);
LABEL_55:
					g_state.send |= 1u;
LABEL_56:
					g_state.send |= 2u;
					break;
				default:
					break;
			}
			g_state.active.colortext1 = g_state.currentmode.colortext1;
		}
		if ( g_state.currentmode.text1text2 != g_state.active.text1text2)
		{
			g_state.send &= 0xFFFFFFFB;
			grTexCombine(1, 1, 0, 1, 0, 0, 0);
			if (g_state.currentmode.text1text2 > X_TEXTURE)
			{
				a1 = 0.0;
				switch (g_state.currentmode.text1text2)
				{
					case X_ADD:
						grTexCombine(0, 4, 8, 4, 8, 0, 0);
						goto LABEL_70;
					case X_MUL:
						grTexCombine(0, 3, 1, 3, 1, 0, 0);
						goto LABEL_70;
					case X_DECAL:
						grTexCombine(0, 7, 11, 7, 11, 0, 0);
						goto LABEL_70;
					case X_MULADD:
						grTexCombine(0, 4, 1, 4, 1, 0, 0);
						goto LABEL_70;
					case X_SUB:
						grTexCombine(0, 6, 8, 6, 8, 0, 0);
LABEL_70:
						g_state.send |= 4u;
						break;
					default:
						break;
				}
			}
			else if (g_state.currentmode.text1text2 >= X_WHITE || !g_state.currentmode.text1text2)
			{
				grTexCombine(0, 1, 0, 1, 0, 0, 0);
				grTexCombine(1, 1, 0, 1, 0, 0, 0);
			}
			g_state.active.text1text2 = g_state.currentmode.text1text2;
		}
		if ( g_state.active.alphatest != g_state.currentmode.alphatest)
		{
			if ( g_state.currentmode.alphatest < 1065353216 && g_state.currentmode.alphatest != 0 /*???*/)
			{
				v16 = (signed __int64)(g_state.currentmode.alphatest * 256.0);
				// Clamp
				if ( (signed int)v16 < 0 )
					v16 = 0;
				if ( (signed int)v16 > 255 )
					v16 = 255;
				grAlphaTestReferenceValue(v16);
				grAlphaTestFunction(GR_CMP_GREATER);
			}
			else
			{
				grAlphaTestFunction(GR_CMP_ALWAYS);
			}
			g_state.active.alphatest = g_state.currentmode.alphatest;
		}
		if ( g_state.active.src != g_state.currentmode.src || g_state.currentmode.dst != g_state.active.dst)
		{
			switch ( g_state.currentmode.src)
			{
				case X_ZERO:
					v17 = 0;
					v18 = 0;
					break;
				case X_OTHER:
					v17 = 2;
					v18 = 3;
					break;
				case X_ALPHA:
					v17 = 1;
					v18 = 1;
					break;
				case X_OTHERALPHA:
					v17 = 3;
					v18 = 3;
					break;
				case X_INVOTHER:
					v17 = 6;
					v18 = 7;
					break;
				case X_INVALPHA:
					v18 = 5;
					goto LABEL_93;
				case X_INVOTHERALPHA:
					v18 = 7;
LABEL_93:
					v17 = v18;
					break;
				default:
					v17 = 4;
					v18 = 4;
					break;
			}
			switch ( g_state.currentmode.dst)
			{
				case X_ONE:
					v20 = 4;
					goto LABEL_103;
				case X_OTHER:
					v19 = 2;
					v20 = 1;
					break;
				case X_ALPHA:
					v20 = 3;
					goto LABEL_103;
				case X_OTHERALPHA:
					v20 = 1;
					goto LABEL_103;
				case X_INVOTHER:
					v19 = 6;
					v20 = 5;
					break;
				case X_INVALPHA:
					v20 = 7;
					goto LABEL_103;
				case X_INVOTHERALPHA:
					v20 = 5;
LABEL_103:
					v19 = v20;
					break;
				default:
					v19 = 0;
					v20 = 0;
					break;
			}
			grAlphaBlendFunction(v17, v19, v18, v20);
			g_state.active.src = g_state.currentmode.src;
			g_state.active.dst = g_state.currentmode.dst;
		}
		if ( g_state.active.dither != g_state.currentmode.dither)
		{
			if (g_state.currentmode.dither == 0 )
				grDitherMode(GR_DITHER_DISABLE);
			else
				grDitherMode(GR_DITHER_4x4);
			g_state.active.dither = g_state.currentmode.dither;
		}
	}
	if ( g_state.send & 4 )
	{
		g_state.currentmode.textures = 2;
	}
	else
	{
		v23 = (g_state.send & 2) == 0;
		g_state.currentmode.textures = 1;
		if ( v23 )
			g_state.currentmode.textures = 0;
	}
	if ( g_state.setnew & 1 )
		g_state.currentmode.textures = 0;
	if ( g_state.active.textures != g_state.currentmode.textures)
	{
		g_state.active.text1 = 0;
		g_state.active.text2 = 0;
		g_state.changed |= 2u;
		g_state.active.textures = g_state.currentmode.textures;
	}
	if ( g_state.changed & 2 )
	{
		if ( g_state.geometry & 0x10 )
			g_state.currentmode.stwhint |= 2u;
		else
			g_state.currentmode.stwhint &= 0xFFFFFFFD;
		if ( g_state.currentmode.textures == 1 )
		{
			g_state.currentmode.stwhint &= 0xFFFFFFEF;
			if ( g_state.currentmode.text1 != g_state.active.text1)
			{
				mode_loadtexture(g_state.currentmode.text1);
				g_state.active.text1 = g_state.currentmode.text1;
				++g_stats.chg_text;
			}
		}
		else if ( g_state.currentmode.textures == 2 )
		{
			if ( LODWORD(g_state.currentmode.sametex) )
				g_state.currentmode.stwhint &= 0xFFFFFFEF;
			else
				g_state.currentmode.stwhint |= 0x10u;
			if ( g_state.currentmode.text1 != g_state.active.text1 || g_state.currentmode.text2 != g_state.active.text2)
			{
				v25 = g_state.currentmode.text2;
				mode_loadmultitexture(g_state.currentmode.text1, g_state.currentmode.text2);
				g_stats.chg_text += 2;
				g_state.active.text1 = g_state.currentmode.text1;
			}
		}
		g_state.active.sametex = g_state.currentmode.sametex;
	}
	result = g_state.currentmode.stwhint;
	if ( g_state.active.stwhint != LODWORD(result) )
	{
		grHints(GR_HINT_STWHINT, g_state.currentmode.stwhint);
		result = g_state.currentmode.stwhint;
		g_state.active.stwhint = result;
	}
	g_state.changed = 0;
	++g_stats.chg_mode;
	if ( g_state.geometry & X_DUMPDATA)
	{
		x_log("#modechange:\n");
		x_log("-mask c=%i z=%i zt=%i\n", g_state.currentmode.mask & 1, (g_state.currentmode.mask & 2u) >> 1, (g_state.currentmode.mask & 4u) >> 2);
		x_log("-colortext1 %08X\n", g_state.currentmode.colortext1);
		x_log("-alphatest %04X\n", (int16_t)g_state.currentmode.alphatest);
		x_log("-blend %04X %04X\n", g_state.currentmode.src, g_state.currentmode.dst);
		x_log("-texture %i (%i textures)\n", g_state.currentmode.text1, g_state.active.textures);
	}
}
