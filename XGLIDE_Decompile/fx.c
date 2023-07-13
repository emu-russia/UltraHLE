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

	grGlideInit();
	if ( grSstQueryHardware(&v1) )
		result = ((unsigned int)(v2 - 3) < 1) + 1;
	else
		result = -1;
	return result;
}

int init_reinit()
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
	grSstWinOpen(g_state.hwnd, mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT, g_state.buffers, g_state.vsync);
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

int init_bufferswap()
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
			return init_reinit();
		}
	}
	grBufferSwap(1);
}

int init_clear(int a1, int a2, float a3, float a4, float a5)
{
	int v5; // ecx

	x_flush(v5, (unsigned __int64)(signed __int64)(a3 * 255.0) >> 32);
	grClipWindow(
		g_state.view_x0,
		g_state.view_y0,
		g_state.view_x1,
		g_state.view_y1);
	grColorMask((unsigned int)a1 >= 1, (unsigned int)a1 >= 1);
	grDepthMask((unsigned int)a2 >= 1);
	grBufferClear(
		(unsigned __int8)(signed __int64)(a3 * 255.0) | ((unsigned __int16)(signed __int64)(a4 * 255.0) << 8) & 0xFF00 | ((unsigned __int8)(signed __int64)(a5 * 255.0) << 16),
		0,
		0xFFFF);
	grDepthMask((g_state.mask & 2u) >> 1);
	grColorMask(g_state.mask & 1, g_state.mask & 1);
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

int mode_init()
{
	g_state.stwhint = 0;
	grDepthBiasLevel(0);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthMask(FXTRUE);
	grDitherMode(GR_DITHER_4x4);
	grHints(GR_HINT_ALLOW_MIPMAP_DITHER, 1);
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
}

int mode_texturemode(int a1, __int16 a2, int a3)
{
	int v3; // esi
	__int16 v4; // di
	int result; // eax

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
	return result;
}

int mode_loadtexture(int a1)
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
			result = mode_texturemode(1, v4, 1);
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
			result = mode_texturemode(0, v4, 0);
		}
	}
	return result;
}

int mode_loadmultitexture(int a1, int a2)
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
			result = mode_texturemode(1, v5, 0);
		}
	}
	return result;
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
	double v1; // st7
	float v2; // ecx
	float v3; // edx
	float v4; // ST08_4
	float v5; // ST08_4
	float v6; // ST04_4

	static int lastfogtype;
	static float lastfogmin;
	static float lastfogmax;
	static GrFog_t fogtable[GR_FOG_TABLE_SIZE];

	if ( lastfogmin == g_state[269]
		&& lastfogmax == g_state[270]
		&& (double)SLODWORD(g_state[268]) == lastfogtype )
	{
		return fogtable;
	}
	v1 = (double)SLODWORD(g_state[268]);
	v2 = g_state[270];
	v3 = g_state[268];
	lastfogmin = g_state[269];
	lastfogmax = v2;
	lastfogtype = v1;
	if ( LODWORD(v3) == X_LINEAR || LODWORD(v3) == X_LINEARADD)
	{
		v5 = g_state[270] / g_state[241];
		v6 = g_state[269] / g_state[241];
		guFogGenerateLinear(fogtable, LODWORD(v6), LODWORD(v5));
	}
	else if ( LODWORD(v3) == X_EXPONENTIAL)
	{
		v4 = 2.3f / (g_state[270] / g_state[241]);
		guFogGenerateExp(fogtable, LODWORD(v4));
	}
	fixfogtable(fogtable, GR_FOG_TABLE_SIZE);
	return fogtable;
}

void mode_change()
{
	float a1; // @ecx 
	float v1; // eax
	float v2; // ecx
	float v3; // edx
	float v4; // eax
	float v5; // ecx
	float v6; // edx
	float v7; // eax
	void *v8; // eax
	signed __int64 v9; // rax
	int v10; // ecx
	float v11; // eax
	float v12; // eax
	float v13; // ST18_4
	signed int v14; // eax
	double v15; // ST24_8
	signed __int64 v16; // rax
	signed int v17; // edx
	signed int v18; // esi
	signed int v19; // eax
	signed int v20; // ecx
	float v21; // ecx
	float v22; // eax
	bool v23; // zf
	float v24; // ecx
	float v25; // eax
	float v26; // ecx
	float result; // eax
	unsigned int v28; // ST18_4
	unsigned int v29; // ST14_4
	int v30; // ST10_4
	float v31; // ST18_4
	double v32; // st7
	float v33; // ecx
	int v34; // ST14_4
	float v35; // ST18_4
	float v36; // ST14_4

	if ( g_state.changed & 1 )
	{
		v1 = g_state.geometry;
		if ( LODWORD(g_state[289]) != LODWORD(v1) )
		{
			if ( LOBYTE(v1) & 2 )
			{
				grCullMode(1);
			}
			else if ( g_state.geometry & 4 )
			{
				grCullMode(2);
			}
			else
			{
				grCullMode(0);
			}
			g_state[289] = g_state.geometry;
		}
	}
	if ( g_state.changed & 8 )
	{
		v2 = g_state[297];
		v3 = g_state[298];
		g_state[268] = g_state[296];
		v4 = g_state[299];
		g_state[269] = v2;
		v5 = g_state[300];
		g_state[270] = v3;
		v6 = g_state[301];
		g_state[271] = v4;
		v7 = g_state[302];
		g_state[272] = v5;
		g_state[273] = v6;
		g_state[274] = v7;
		if ( LODWORD(g_state[296]) == 4098 )
		{
			grFogMode(GR_FOG_DISABLE);
		}
		else
		{
			v8 = generatefogtable();
			grFogTable(v8);
			v9 = (signed __int64)(g_state[273] * 255.0);
			grFogColorValue(
				v10,
				HIDWORD(v9),
				(unsigned __int8)(signed __int64)(g_state[271] * 255.0) | ((unsigned __int16)(signed __int64)(g_state[272] * 255.0) << 8) & 0xFF00 | (((unsigned int)v9 | 0xFFFFFF00) << 16));
			if ( LODWORD(g_state[268]) == 7939 )
				grFogMode(GR_FOG_ADD2 | GR_FOG_WITH_TABLE);
			else
				grFogMode(GR_FOG_WITH_TABLE);
		}
	}
	if ( g_state.changed & 4 )
	{
		v11 = g_state[291];
		if ( LODWORD(g_state[263]) != LODWORD(v11) )
		{
			grDepthMask((LOBYTE(v11) & 2u) >> 1);
			grColorMask(LODWORD(g_state[291]) & 1, LODWORD(g_state[291]) & 1);
			a1 = g_state[291];
			g_state[263] = a1;
		}
		v12 = g_state[292];
		if ( LODWORD(g_state[264]) != LODWORD(v12) )
		{
			if ( SLODWORD(v12) > 4098 )
			{
				switch ( LODWORD(v12) )
				{
					case 0x10D1:
						grDepthBufferFunction(2);
						break;
					case 0x10D2:
						grDepthBufferFunction(5);
						break;
					case 0x10D5:
						grDepthBufferFunction(4);
						break;
					case 0x10D6:
						grDepthBufferFunction(1);
						break;
					default:
						goto $L1409;
				}
			}
			else if ( LODWORD(v12) == 4098 || v12 == 0.0 )
			{
				grDepthBufferFunction(7);
			}
			else
			{
$L1409:
				grDepthBufferFunction(3);
			}
			g_state[264] = g_state[292];
		}
		if ( LODWORD(g_state[312]) != LODWORD(g_state[284]) )
		{
			g_state[265] = -6.8056469e38/*NaN*/;
			g_state[284] = g_state[312];
		}
		if ( g_state.colortext1 != LODWORD(g_state[265]) )
		{
			g_state.send &= 0xFFFFFFFC;
			switch ( g_state.colortext1 & 0xFFFF )
			{
				case X_WHITE:
					grConstantColorValue(2147450879);
					guColorCombineFunction(1);
					guAlphaSource(0);
					break;
				case X_COLOR:
					guColorCombineFunction(2);
					guAlphaSource(1);
					g_state.send |= 1u;
					break;
				case X_TEXTURE:
				case X_DECAL:
					guColorCombineFunction(4);
					guAlphaSource(2);
					goto LABEL_49;
				case X_ADD:
					guColorCombineFunction(11);
					guAlphaSource(2);
					goto LABEL_48;
				case X_MUL:
					guColorCombineFunction(6);
					guAlphaSource(3);
					goto LABEL_48;
				case X_TEXTURE_IA:
					guColorCombineFunction(4);
					guAlphaSource(1);
					goto LABEL_48;
				case X_MUL_TA:
					guColorCombineFunction(6);
					guAlphaSource(2);
					goto LABEL_48;
				case X_MUL_IA:
					guColorCombineFunction(6);
					guAlphaSource(1);
					goto LABEL_48;
				case X_TEXTUREBLEND:
					grColorCombine(7, 4, 0, 1, 0);
					guAlphaSource(1);
					goto LABEL_48;
				case X_TEXTUREENVA:
					grColorCombine(7, 4, 1, 0, 0);
					guAlphaSource(1);
					goto LABEL_47;
				case X_TEXTUREENVC:
					grColorCombine(7, 5, 1, 0, 0);
					guAlphaSource(1);
					goto LABEL_47;
				case X_SUB:
					grColorCombine(6, 8, 0, 1, 0);
					guAlphaSource(2);
					goto LABEL_48;
				case X_TEXTUREENVCR:
					grColorCombine(7, 5, 0, 2, 0);
					guAlphaSource(1);
					v13 = g_state[284];
					x_log("envc=%08X\n");
LABEL_47:
					grConstantColorValue(LODWORD(g_state[284]));
LABEL_48:
					g_state.send |= 1u;
LABEL_49:
					g_state.send |= 2u;
					break;
				default:
					break;
			}
			switch ( g_state.colortext1 >> 16 )
			{
				case X_COLOR:
					guAlphaSource(1);
					g_state.send |= 1u;
					break;
				case X_TEXTURE:
				case X_DECAL:
					guAlphaSource(2);
					goto LABEL_56;
				case X_ADD:
					guAlphaSource(2);
					goto LABEL_55;
				case X_MUL:
					guAlphaSource(3);
LABEL_55:
					g_state.send |= 1u;
LABEL_56:
					g_state.send |= 2u;
					break;
				default:
					break;
			}
			g_state[265] = g_state.colortext1;
		}
		if ( g_state.text1text2 != LODWORD(g_state[266]) )
		{
			g_state.send &= 0xFFFFFFFB;
			grTexCombine(1, 1, 0, 1, 0, 0, 0);
			v14 = g_state.text1text2;
			if ( v14 > X_TEXTURE)
			{
				a1 = 0.0;
				switch ( v14 )
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
			else if ( v14 >= X_WHITE || !v14 )
			{
				grTexCombine(0, 1, 0, 1, 0, 0, 0);
				grTexCombine(1, 1, 0, 1, 0, 0, 0);
			}
			g_state[266] = g_state.text1text2;
		}
		if ( g_state[280] != g_state.alphatest)
		{
			if ( SLODWORD(g_state[308]) < 1065353216 && (v15 = g_state[308], LODWORD(v15) | HIDWORD(v15) & 0x7FFFFFFF) )
			{
				v16 = (signed __int64)(g_state[308] * 256.0);
				if ( (signed int)v16 < 0 )
					LODWORD(v16) = 0;
				if ( (signed int)v16 > 255 )
					LODWORD(v16) = 255;
				grAlphaTestReferenceValue(LODWORD(a1), HIDWORD(v16), v16);
				grAlphaTestFunction(GR_CMP_GREATER);
			}
			else
			{
				grAlphaTestFunction(GR_CMP_ALWAYS);
			}
			g_state[280] = g_state.alphatest;
		}
		if ( LODWORD(g_state[276]) != g_state.src || g_state.dst != LODWORD(g_state[277]) )
		{
			switch ( g_state.src)
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
			switch ( g_state.dst)
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
			v21 = g_state.dst;
			g_state[276] = g_state.src;
			g_state[277] = v21;
		}
		v22 = g_state[303];
		if ( LODWORD(g_state[275]) != LODWORD(v22) )
		{
			if ( v22 == 0.0 )
				grDitherMode(GR_DITHER_DISABLE);
			else
				grDitherMode(GR_DITHER_4x4);
			g_state[275] = g_state[303];
		}
	}
	if ( g_state.send & 4 )
	{
		LODWORD(g_state[309]) = 2;
	}
	else
	{
		v23 = (g_state.send & 2) == 0;
		LODWORD(g_state[309]) = 1;
		if ( v23 )
			g_state[309] = 0;
	}
	if ( LOBYTE(g_state[289]) & 1 )
		g_state[309] = 0;
	if ( LODWORD(g_state[281]) != LODWORD(g_state[309]) )
	{
		v24 = g_state[309];
		g_state[282] = 0.0;
		g_state[283] = 0.0;
		g_state.changed |= 2u;
		g_state[281] = v24;
	}
	if ( g_state.changed & 2 )
	{
		if ( g_state.geometry & 0x10 )
			g_state.stwhint |= 2u;
		else
			g_state.stwhint &= 0xFFFFFFFD;
		if ( LODWORD(g_state[309]) == 1 )
		{
			g_state.stwhint &= 0xFFFFFFEF;
			if ( LODWORD(g_state[310]) != LODWORD(g_state[282]) )
			{
				mode_loadtexture(LODWORD(g_state[310]));
				g_state[282] = g_state[310];
				++g_stats.chg_text;
			}
		}
		else if ( LODWORD(g_state[309]) == 2 )
		{
			if ( LODWORD(g_state.sametex) )
				g_state.stwhint &= 0xFFFFFFEF;
			else
				g_state.stwhint |= 0x10u;
			if ( LODWORD(g_state[310]) != LODWORD(g_state[282]) || LODWORD(g_state[311]) != LODWORD(g_state[283]) )
			{
				v25 = g_state[311];
				mode_loadmultitexture(*(_QWORD *)&g_state[310], *(_QWORD *)&g_state[310] >> 32);
				v26 = g_state[310];
				g_stats.chg_text += 2;
				g_state[282] = v26;
			}
		}
		g_state[278] = g_state.sametex;
	}
	result = g_state.stwhint;
	if ( LODWORD(g_state[279]) != LODWORD(result) )
	{
		grHints(GR_HINT_STWHINT, g_state.stwhint);
		result = g_state.stwhint;
		g_state[279] = result;
	}
	g_state.changed = 0;
	++g_stats.chg_mode;
	if ( g_state.geometry & X_DUMPDATA)
	{
		x_log("#modechange:\n");
		x_log("-mask c=%i z=%i zt=%i\n", (g_state.mask & 4u) >> 2, (g_state.mask & 2u) >> 1, g_state.mask & 1);
		x_log("-colortext1 %08X\n", g_state.colortext1);
		x_log("-alphatest %04X\n", (int16_t)g_state.alphatest);
		x_log("-blend %04X %04X\n", g_state.dst, g_state.src);
		v35 = g_state[281];
		v36 = g_state[310];
		x_log("-texture %i (%i textures)\n");
	}
}
