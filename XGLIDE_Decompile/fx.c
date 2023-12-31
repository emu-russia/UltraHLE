// Set pixel pipeline modes, and initialize the graphics device.

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
	//int result; // eax
	//GrHwConfiguration v1; // [esp+0h] [ebp-94h]
	//int v2; // [esp+4h] [ebp-90h]

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
	if ( g_state[XST].xs < 1024 )
	{
		if ( g_state[XST].xs < 800 )
		{
			if ( g_state[XST].xs < 640 )
			{
				mode = GR_RESOLUTION_512x384;
				if ( g_state[XST].xs < 512 )
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
	grSstWinOpen((FxU32)g_state[XST].hwnd, mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT, g_state[XST].buffers, g_state[XST].vsync);
	grClipWindow(0, 0, g_state[XST].xs, g_state[XST].ys);
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
		g_state[XST].tmus = 1;
	else
		g_state[XST].tmus = hw.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
	x_log("x_open: cartdtype=%i, tmus=%i\n", hw.SSTs[0].type, g_state[XST].tmus);

	grSstSelect(0);
	text_init();
	if ( g_state[XST].xs < 1024 )
	{
		if ( g_state[XST].xs < 800 )
		{
			if ( g_state[XST].xs < 640 )
			{
				mode = GR_RESOLUTION_512x384;
				if ( g_state[XST].xs < 512 )
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
	grSstWinOpen((FxU32)g_state[XST].hwnd, mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT, g_state[XST].buffers, g_state[XST].vsync);
	grClipWindow(0, 0, g_state[XST].xs, g_state[XST].ys);
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
	g_state[XST].xs = xs;
	g_state[XST].ys = ys;
}

void init_bufferswap()
{
	int v0;
	int v1;

	v0 = 0;
	while ( grBufferNumPending() > 3 )
	{
		x_sleep(10);
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
		g_state[XST].view_x0,
		g_state[XST].view_y0,
		g_state[XST].view_x1,
		g_state[XST].view_y1);
	grColorMask(writecolor >= 1 ? FXTRUE : FXFALSE, writecolor >= 1 ? FXTRUE : FXFALSE);
	grDepthMask(writedepth >= 1 ? FXTRUE : FXFALSE);
	grBufferClear(
		(unsigned __int8)(signed __int64)(cr * 255.0) | ((unsigned __int16)(signed __int64)(cg * 255.0) << 8) & 0xFF00 | ((unsigned __int8)(signed __int64)(cb * 255.0) << 16),
		0,
		0xFFFF);
	grDepthMask((g_state[XST].currentmode.mask & 2u) >> 1);
	grColorMask(g_state[XST].currentmode.mask & 1, g_state[XST].currentmode.mask & 1);
}

int init_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	BOOL v7; // ecx
	signed int result; // eax
	int v9; // esi
	unsigned __int16 *v10; // edi
	int v11; // ebx
	unsigned __int16 v12; // ax

	v7 = (fb & X_FB_FRONT) < 1;
	if ( (uint8_t)fb == X_FB_RGB565)
	{
		grLfbReadRegion(v7, x, y, xs, ys, bufrowlen, buffer);
		result = 0;
	}
	else if ( (uint8_t)fb == X_FB_RGBA8888)
	{
		v9 = buffer;
		v10 = (unsigned __int16 *)(buffer + ys * bufrowlen / 2);
		v11 = xs * ys;
		grLfbReadRegion(v7, x, y, xs, ys, bufrowlen / 2, buffer + ys * bufrowlen / 2);
		if (xs * ys > 0 )
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

int init_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	// Not implemented
	return 1;
}

void mode_init()
{
	g_state[XST].currentmode.stwhint = 0;
	grDepthBiasLevel(0);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthMask(FXTRUE);
	grDitherMode(GR_DITHER_4x4);
	grHints(GR_HINT_ALLOW_MIPMAP_DITHER, 1);
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
}

void mode_texturemode(int tmu, int format, int trilin)
{
	if (trilin)
	{
		grTexMipMapMode(tmu, 1, 1);
	}
	else
	{
		grTexMipMapMode(tmu, (format & X_MIPMAP) != 0, 0);
	}
	if (format & X_CLAMP)
		grTexClampMode(tmu, (format & X_CLAMPNOX) < 1, (format & X_CLAMPNOY) < 1);
	else
		grTexClampMode(tmu, 0, 0);
	if (format & X_NOBILIN)
		grTexFilterMode(tmu, 0, 0);
	else
		grTexFilterMode(tmu, 1, 1);
}

void mode_loadtexture(int txtind)
{
	signed int v1; // ebx
	xt_texture* result; // eax
	xt_texture* v3; // esi
	int v4; // edi

	v1 = 1;
	if ( g_state[XST].tmus < 2 )
		v1 = 0;
	result = texture_get(txtind);
	v3 = result;
	if ( result )
	{
		v4 = result->format;
		if ( !(v4 & X_MIPMAP) )
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

void mode_loadmultitexture(int txtind1, int txtind2)
{
	xt_texture* result; // eax
	xt_texture* v3; // ebx
	int v4; // edi
	int v5; // esi

	result = texture_get(txtind1);
	v3 = result;
	if ( result )
	{
		v4 = result->format;
		result = texture_get(txtind2);
		if ( result )
		{
			v5 = result->format;
			fxloadtexture_multi(v3, result);
			mode_texturemode(0, v4, 0);
			mode_texturemode(1, v5, 0);
		}
	}
}

void fixfogtable(GrFog_t* fogtable, int size)
{
	int i; // esi
	unsigned __int8 v3; // dl
	int v4; // ecx

	for ( i = size - 1; i >= 0; --i )
	{
		v3 = *(BYTE *)(i + fogtable);
		v4 = *(unsigned __int8 *)(i + fogtable + 1) - v3;
		if ( v4 > 60 )
			*(BYTE *)(i + fogtable) = v3 + v4 - 60;
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

	if ( lastfogmin == g_state[XST].active.fogmin
		&& lastfogmax == g_state[XST].active.fogmax
		&& g_state[XST].active.fogtype == lastfogtype )
	{
		return fogtable;
	}

	lastfogmin = g_state[XST].active.fogmin;
	lastfogmax = g_state[XST].active.fogmax;
	lastfogtype = g_state[XST].active.fogtype;
	if (g_state[XST].active.fogtype == X_LINEAR || g_state[XST].active.fogtype == X_LINEARADD)
	{
		v5 = g_state[XST].active.fogmax / g_state[XST].znear;
		v6 = g_state[XST].active.fogmin / g_state[XST].znear;
		guFogGenerateLinear(fogtable, v6, v5);
	}
	else if (g_state[XST].active.fogtype == X_EXPONENTIAL)
	{
		v4 = 2.3f / (g_state[XST].active.fogmax / g_state[XST].znear);
		guFogGenerateExp(fogtable, v4);
	}
	fixfogtable(fogtable, GR_FOG_TABLE_SIZE);
	return fogtable;
}

void mode_change()
{
	float a1; // @ecx 
	int v16; // rax
	signed int v17; // edx
	signed int v18; // esi
	signed int v19; // eax
	signed int v20; // ecx
	float v25; // eax

	if ( g_state[XST].changed & 1 )
	{
		if ( g_state[XST].setnew != g_state[XST].geometry)
		{
			if (g_state[XST].geometry & 2 )
			{
				grCullMode(GR_CULL_NEGATIVE);
			}
			else if ( g_state[XST].geometry & 4 )
			{
				grCullMode(GR_CULL_POSITIVE);
			}
			else
			{
				grCullMode(GR_CULL_DISABLE);
			}
			g_state[XST].setnew = g_state[XST].geometry;
		}
	}
	if ( g_state[XST].changed & 8 )
	{
		g_state[XST].active.fogtype = g_state[XST].currentmode.fogtype;
		g_state[XST].active.fogmin = g_state[XST].currentmode.fogmin;
		g_state[XST].active.fogmax = g_state[XST].currentmode.fogmax;
		g_state[XST].active.fogcolor[0] = g_state[XST].currentmode.fogcolor[0];
		g_state[XST].active.fogcolor[1] = g_state[XST].currentmode.fogcolor[1];
		g_state[XST].active.fogcolor[2] = g_state[XST].currentmode.fogcolor[2];
		g_state[XST].active.fogcolor[3] = g_state[XST].currentmode.fogcolor[3];
		if ( g_state[XST].currentmode.fogtype == X_DISABLE)
		{
			grFogMode(GR_FOG_DISABLE);
		}
		else
		{
			grFogTable(generatefogtable());
			grFogColorValue(
				(unsigned __int8)(signed __int64)(g_state[XST].active.fogcolor[0] * 255.0) | 
				((unsigned __int16)(signed __int64)(g_state[XST].active.fogcolor[1] * 255.0) << 8) & 0xFF00 | 
				(((unsigned int)(signed __int64)(g_state[XST].active.fogcolor[2] * 255.0) | 0xFFFFFF00) << 16)
			);
			if ( g_state[XST].active.fogtype == X_LINEARADD)
				grFogMode(GR_FOG_ADD2 | GR_FOG_WITH_TABLE);
			else
				grFogMode(GR_FOG_WITH_TABLE);
		}
	}
	if ( g_state[XST].changed & 4 )
	{
		if ( g_state[XST].active.mask != g_state[XST].currentmode.mask)
		{
			grDepthMask((g_state[XST].currentmode.mask & 2u) >> 1);
			grColorMask(g_state[XST].currentmode.mask & 1, g_state[XST].currentmode.mask & 1);
			g_state[XST].active.mask = g_state[XST].currentmode.mask;
		}
		if ( g_state[XST].active.masktst != g_state[XST].currentmode.masktst)
		{
			if (g_state[XST].currentmode.masktst > X_DISABLE)
			{
				switch (g_state[XST].currentmode.masktst)
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
			else if (g_state[XST].currentmode.masktst == X_DISABLE || g_state[XST].currentmode.masktst == 0 )
			{
				grDepthBufferFunction(7);
			}
			else
			{
$L1409:
				grDepthBufferFunction(3);
			}
			g_state[XST].active.masktst = g_state[XST].currentmode.masktst;
		}
		if ( g_state[XST].currentmode.envc != g_state[XST].active.envc)
		{
			g_state[XST].active.colortext1 = 0xfffffff;
			g_state[XST].active.envc = g_state[XST].currentmode.envc;
		}
		if ( g_state[XST].currentmode.colortext1 != g_state[XST].active.colortext1 )
		{
			g_state[XST].send &= 0xFFFFFFFC;
			switch ( g_state[XST].currentmode.colortext1 & 0xFFFF )
			{
				case X_WHITE:
					grConstantColorValue(0x7FFF7FFF);
					guColorCombineFunction(GR_COLORCOMBINE_CCRGB);
					guAlphaSource(GR_ALPHASOURCE_CC_ALPHA);
					break;
				case X_COLOR:
					guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					g_state[XST].send |= 1u;
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
					if (g_state[XST].geometry & X_DUMPDATA) {
						x_log("envc=%08X\n", g_state[XST].active.envc);
					}
LABEL_47:
					grConstantColorValue(g_state[XST].active.envc);
LABEL_48:
					g_state[XST].send |= 1u;
LABEL_49:
					g_state[XST].send |= 2u;
					break;
				default:
					break;
			}
			switch ( g_state[XST].currentmode.colortext1 >> 16 )
			{
				case X_COLOR:
					guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
					g_state[XST].send |= 1u;
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
					g_state[XST].send |= 1u;
LABEL_56:
					g_state[XST].send |= 2u;
					break;
				default:
					break;
			}
			g_state[XST].active.colortext1 = g_state[XST].currentmode.colortext1;
		}
		if ( g_state[XST].currentmode.text1text2 != g_state[XST].active.text1text2)
		{
			g_state[XST].send &= 0xFFFFFFFB;
			grTexCombine(1, 1, 0, 1, 0, 0, 0);
			if (g_state[XST].currentmode.text1text2 > X_TEXTURE)
			{
				a1 = 0.0;
				switch (g_state[XST].currentmode.text1text2)
				{
					case X_ADD:
						grTexCombine(0, 4, 8, 4, 8, 0, 0);
						g_state[XST].send |= 4u;
						break;
					case X_MUL:
						grTexCombine(0, 3, 1, 3, 1, 0, 0);
						g_state[XST].send |= 4u;
						break;
					case X_DECAL:
						grTexCombine(0, 7, 11, 7, 11, 0, 0);
						g_state[XST].send |= 4u;
						break;
					case X_MULADD:
						grTexCombine(0, 4, 1, 4, 1, 0, 0);
						g_state[XST].send |= 4u;
						break;
					case X_SUB:
						grTexCombine(0, 6, 8, 6, 8, 0, 0);
						g_state[XST].send |= 4u;
						break;
					default:
						break;
				}
			}
			else if (g_state[XST].currentmode.text1text2 >= X_WHITE || !g_state[XST].currentmode.text1text2)
			{
				grTexCombine(0, 1, 0, 1, 0, 0, 0);
				grTexCombine(1, 1, 0, 1, 0, 0, 0);
			}
			g_state[XST].active.text1text2 = g_state[XST].currentmode.text1text2;
		}
		if ( g_state[XST].active.alphatest != g_state[XST].currentmode.alphatest)
		{
			// TODO: Check to see what kind of decompilation is so crooked
			if ( g_state[XST].currentmode.alphatest < 1.0f && g_state[XST].currentmode.alphatest != 0 /*???*/)
			{
				v16 = (int)(g_state[XST].currentmode.alphatest * 256.0);
				// Clamp
				if ( v16 < 0 )
					v16 = 0;
				if ( v16 > 255 )
					v16 = 255;
				grAlphaTestReferenceValue((GrAlpha_t)v16);
				grAlphaTestFunction(GR_CMP_GREATER);
			}
			else
			{
				grAlphaTestFunction(GR_CMP_ALWAYS);
			}
			g_state[XST].active.alphatest = g_state[XST].currentmode.alphatest;
		}
		if ( g_state[XST].active.src != g_state[XST].currentmode.src || g_state[XST].currentmode.dst != g_state[XST].active.dst)
		{
			switch ( g_state[XST].currentmode.src)
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
					v17 = v18;
					break;
				case X_INVOTHERALPHA:
					v18 = 7;
					v17 = v18;
					break;
				default:
					v17 = 4;
					v18 = 4;
					break;
			}
			switch ( g_state[XST].currentmode.dst)
			{
				case X_ONE:
					v20 = 4;
					v19 = v20;
					break;
				case X_OTHER:
					v19 = 2;
					v20 = 1;
					break;
				case X_ALPHA:
					v20 = 3;
					v19 = v20;
					break;
				case X_OTHERALPHA:
					v20 = 1;
					v19 = v20;
					break;
				case X_INVOTHER:
					v19 = 6;
					v20 = 5;
					break;
				case X_INVALPHA:
					v20 = 7;
					v19 = v20;
					break;
				case X_INVOTHERALPHA:
					v20 = 5;
					v19 = v20;
					break;
				default:
					v19 = 0;
					v20 = 0;
					break;
			}
			grAlphaBlendFunction(v17, v19, v18, v20);
			g_state[XST].active.src = g_state[XST].currentmode.src;
			g_state[XST].active.dst = g_state[XST].currentmode.dst;
		}
		if ( g_state[XST].active.dither != g_state[XST].currentmode.dither)
		{
			if (g_state[XST].currentmode.dither == 0 )
				grDitherMode(GR_DITHER_DISABLE);
			else
				grDitherMode(GR_DITHER_4x4);
			g_state[XST].active.dither = g_state[XST].currentmode.dither;
		}
	}
	if ( g_state[XST].send & 4 )
	{
		g_state[XST].currentmode.textures = 2;
	}
	else
	{
		g_state[XST].currentmode.textures = 1;
		if ((g_state[XST].send & 2) == 0)
			g_state[XST].currentmode.textures = 0;
	}
	if ( g_state[XST].setnew & 1 )
		g_state[XST].currentmode.textures = 0;
	if ( g_state[XST].active.textures != g_state[XST].currentmode.textures)
	{
		g_state[XST].active.text1 = 0;
		g_state[XST].active.text2 = 0;
		g_state[XST].changed |= 2u;
		g_state[XST].active.textures = g_state[XST].currentmode.textures;
	}
	if ( g_state[XST].changed & 2 )
	{
		if ( g_state[XST].geometry & 0x10 )
			g_state[XST].currentmode.stwhint |= 2u;
		else
			g_state[XST].currentmode.stwhint &= 0xFFFFFFFD;
		if ( g_state[XST].currentmode.textures == 1 )
		{
			g_state[XST].currentmode.stwhint &= 0xFFFFFFEF;
			if ( g_state[XST].currentmode.text1 != g_state[XST].active.text1)
			{
				mode_loadtexture(g_state[XST].currentmode.text1);
				g_state[XST].active.text1 = g_state[XST].currentmode.text1;
				++g_stats.chg_text;
			}
		}
		else if ( g_state[XST].currentmode.textures == 2 )
		{
			if ( g_state[XST].currentmode.sametex )
				g_state[XST].currentmode.stwhint &= 0xFFFFFFEF;
			else
				g_state[XST].currentmode.stwhint |= 0x10u;
			if ( g_state[XST].currentmode.text1 != g_state[XST].active.text1 || g_state[XST].currentmode.text2 != g_state[XST].active.text2)
			{
				v25 = g_state[XST].currentmode.text2;
				mode_loadmultitexture(g_state[XST].currentmode.text1, g_state[XST].currentmode.text2);
				g_stats.chg_text += 2;
				g_state[XST].active.text1 = g_state[XST].currentmode.text1;
			}
		}
		g_state[XST].active.sametex = g_state[XST].currentmode.sametex;
	}
	if ( g_state[XST].active.stwhint != g_state[XST].currentmode.stwhint)
	{
		grHints(GR_HINT_STWHINT, g_state[XST].currentmode.stwhint);
		g_state[XST].active.stwhint = g_state[XST].currentmode.stwhint;
	}
	g_state[XST].changed = 0;
	++g_stats.chg_mode;
	if ( g_state[XST].geometry & X_DUMPDATA)
	{
		x_log("#modechange:\n");
		x_log("-mask c=%i z=%i zt=%i\n", g_state[XST].currentmode.mask & 1, (g_state[XST].currentmode.mask & 2u) >> 1, (g_state[XST].currentmode.mask & 4u) >> 2);
		x_log("-colortext1 %08X\n", g_state[XST].currentmode.colortext1);
		x_log("-alphatest %04X\n", (int16_t)g_state[XST].currentmode.alphatest);
		x_log("-blend %04X %04X\n", g_state[XST].currentmode.src, g_state[XST].currentmode.dst);
		x_log("-texture %i (%i textures)\n", g_state[XST].currentmode.text1, g_state[XST].active.textures);
	}
}
