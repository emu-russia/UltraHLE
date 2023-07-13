#include "pch.h"

char *init_name()
{
	return "Glide";
}

int init_fullscreen(int a1)
{
	int result; // eax

	if ( a1 )
		result = grSstControl(1);
	else
		result = grSstControl(2);
	return result;
}

int init_query()
{
	int result; // eax
	char v1; // [esp+0h] [ebp-94h]
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
	signed int v0; // ecx

	x_log("init_reinit: shutdown\n");
	grGlideShutdown();
	x_log("init_reinit: init\n");
	grGlideInit();
	x_log("init_reinit: select\n");
	grSstSelect(0);
	x_log("init_reinit: resolution\n");
	if ( g_state[163] < 1024 )
	{
		if ( g_state[163] < 800 )
		{
			if ( g_state[163] < 640 )
			{
				v0 = 3;
				if ( g_state[163] < 512 )
					v0 = 7;
			}
			else
			{
				v0 = 7;
			}
		}
		else
		{
			v0 = 8;
		}
	}
	else
	{
		v0 = 12;
	}
	grSstWinOpen(g_state[162], v0, 0, 1, 1, g_state[165], g_state[166]);
	grClipWindow(0, 0, g_state[163], g_state[164]);
	x_log("init_reinit: done\n");
}

signed int init_init()
{
	int v1; // ST18_4
	signed int v2; // ecx
	char v3; // [esp+0h] [ebp-94h]
	int v4; // [esp+4h] [ebp-90h]
	int v5; // [esp+10h] [ebp-84h]

	grGlideInit();
	if ( !grSstQueryHardware(&v3) )
		return -1;
	if ( v4 != 3 && v4 )
		g_state[160] = 1;
	else
		g_state[160] = v5;
	v1 = g_state[160];
	x_log("x_open: cartdtype=%i, tmus=%i\n");
	grSstSelect(0);
	text_init();
	if ( g_state[163] < 1024 )
	{
		if ( g_state[163] < 800 )
		{
			if ( g_state[163] < 640 )
			{
				v2 = 3;
				if ( g_state[163] < 512 )
					v2 = 7;
			}
			else
			{
				v2 = 7;
			}
		}
		else
		{
			v2 = 8;
		}
	}
	else
	{
		v2 = 12;
	}
	grSstWinOpen(g_state[162], v2, 0, 1, 1, g_state[165], g_state[166]);
	grClipWindow(0, 0, g_state[163], g_state[164]);
	return 0;
}

int init_deinit()
{
	x_log("x_close");
	return grGlideShutdown();
}

void init_activate()
{
}

int init_resize(int a1, int a2)
{
	int result; // eax

	result = a1;
	g_state[163] = a1;
	g_state[164] = a2;
	return result;
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
	return grBufferSwap(1);
}

int init_clear(int a1, int a2, float a3, float a4, float a5)
{
	int v5; // ecx

	x_flush(v5, (unsigned __int64)(signed __int64)(a3 * 255.0) >> 32);
	grClipWindow(
		(signed __int64)g_state[248],
		(signed __int64)g_state[250],
		(signed __int64)g_state[249],
		(signed __int64)g_state[251]);
	grColorMask((unsigned int)a1 >= 1, (unsigned int)a1 >= 1);
	grDepthMask((unsigned int)a2 >= 1);
	grBufferClear(
		(unsigned __int8)(signed __int64)(a3 * 255.0) | ((unsigned __int16)(signed __int64)(a4 * 255.0) << 8) & 0xFF00 | ((unsigned __int8)(signed __int64)(a5 * 255.0) << 16),
		0,
		0xFFFF);
	grDepthMask((LODWORD(g_state[291]) & 2u) >> 1);
	return grColorMask(LODWORD(g_state[291]) & 1, LODWORD(g_state[291]) & 1);
}

signed int init_readfb(__int16 a1, int a2, int a3, int a4, int a5, int a6, int a7)
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

signed int init_writefb()
{
	return 1;
}

int mode_init()
{
	g_state[307] = 0.0;
	grDepthBiasLevel(0);
	grDepthBufferMode(2);
	grDepthMask(1);
	grDitherMode(2);
	grHints(3, 1);
	grTexFilterMode(0, 1, 1);
	return grTexFilterMode(1, 1, 1);
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
		result = grTexFilterMode(v3, 0, 0);
	else
		result = grTexFilterMode(v3, 1, 1);
	return result;
}

int mode_loadtexture(int a1)
{
	signed int v1; // ebx
	int result; // eax
	int v3; // esi
	int v4; // edi

	v1 = 1;
	if ( SLODWORD(g_state[160]) < 2 )
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
			result = grTexCombine(1, 1, 0, 1, 0, 0, 0);
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

void fixfogtable(int a1, int a2)
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

void *generatefogtable()
{
	double v1; // st7
	float v2; // ecx
	float v3; // edx
	float v4; // ST08_4
	float v5; // ST08_4
	float v6; // ST04_4

	if ( `generatefogtable'::`2'::lastfogmin == g_state[269]
		&& `generatefogtable'::`2'::lastfogmax == g_state[270]
		&& (double)SLODWORD(g_state[268]) == `generatefogtable'::`2'::lastfogtype )
	{
		return &`generatefogtable'::`2'::fogtable;
	}
	v1 = (double)SLODWORD(g_state[268]);
	v2 = g_state[270];
	v3 = g_state[268];
	`generatefogtable'::`2'::lastfogmin = g_state[269];
	`generatefogtable'::`2'::lastfogmax = v2;
	`generatefogtable'::`2'::lastfogtype = v1;
	if ( LODWORD(v3) == 7937 || LODWORD(v3) == 7939 )
	{
		v5 = g_state[270] / g_state[241];
		v6 = g_state[269] / g_state[241];
		guFogGenerateLinear(&`generatefogtable'::`2'::fogtable, LODWORD(v6), LODWORD(v5));
	}
	else if ( LODWORD(v3) == 7938 )
	{
		v4 = 2.3 / (g_state[270] / g_state[241]);
		guFogGenerateExp(&`generatefogtable'::`2'::fogtable, LODWORD(v4));
	}
	fixfogtable((int)&`generatefogtable'::`2'::fogtable, 64);
	return &`generatefogtable'::`2'::fogtable;
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

	if ( LOBYTE(g_state[290]) & 1 )
	{
		v1 = g_state[317];
		if ( LODWORD(g_state[289]) != LODWORD(v1) )
		{
			if ( LOBYTE(v1) & 2 )
			{
				grCullMode(1);
			}
			else if ( LOBYTE(g_state[317]) & 4 )
			{
				grCullMode(2);
			}
			else
			{
				grCullMode(0);
			}
			g_state[289] = g_state[317];
		}
	}
	if ( LOBYTE(g_state[290]) & 8 )
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
			grFogMode(0);
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
				grFogMode(514);
			else
				grFogMode(2);
		}
	}
	if ( LOBYTE(g_state[290]) & 4 )
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
		if ( LODWORD(g_state[293]) != LODWORD(g_state[265]) )
		{
			LODWORD(g_state[258]) &= 0xFFFFFFFC;
			switch ( LODWORD(g_state[293]) & 0xFFFF )
			{
				case 0x1301:
					grConstantColorValue(2147450879);
					guColorCombineFunction(1);
					guAlphaSource(0);
					break;
				case 0x1302:
					guColorCombineFunction(2);
					guAlphaSource(1);
					LODWORD(g_state[258]) |= 1u;
					break;
				case 0x1303:
				case 0x1306:
					guColorCombineFunction(4);
					guAlphaSource(2);
					goto LABEL_49;
				case 0x1304:
					guColorCombineFunction(11);
					guAlphaSource(2);
					goto LABEL_48;
				case 0x1305:
					guColorCombineFunction(6);
					guAlphaSource(3);
					goto LABEL_48;
				case 0x1308:
					guColorCombineFunction(4);
					guAlphaSource(1);
					goto LABEL_48;
				case 0x1309:
					guColorCombineFunction(6);
					guAlphaSource(2);
					goto LABEL_48;
				case 0x130A:
					guColorCombineFunction(6);
					guAlphaSource(1);
					goto LABEL_48;
				case 0x130B:
					grColorCombine(7, 4, 0, 1, 0);
					guAlphaSource(1);
					goto LABEL_48;
				case 0x130C:
					grColorCombine(7, 4, 1, 0, 0);
					guAlphaSource(1);
					goto LABEL_47;
				case 0x130D:
					grColorCombine(7, 5, 1, 0, 0);
					guAlphaSource(1);
					goto LABEL_47;
				case 0x130E:
					grColorCombine(6, 8, 0, 1, 0);
					guAlphaSource(2);
					goto LABEL_48;
				case 0x130F:
					grColorCombine(7, 5, 0, 2, 0);
					guAlphaSource(1);
					v13 = g_state[284];
					x_log("envc=%08X\n");
LABEL_47:
					grConstantColorValue(LODWORD(g_state[284]));
LABEL_48:
					LODWORD(g_state[258]) |= 1u;
LABEL_49:
					LODWORD(g_state[258]) |= 2u;
					break;
				default:
					break;
			}
			switch ( SLODWORD(g_state[293]) >> 16 )
			{
				case 0x1302:
					guAlphaSource(1);
					LODWORD(g_state[258]) |= 1u;
					break;
				case 0x1303:
				case 0x1306:
					guAlphaSource(2);
					goto LABEL_56;
				case 0x1304:
					guAlphaSource(2);
					goto LABEL_55;
				case 0x1305:
					guAlphaSource(3);
LABEL_55:
					LODWORD(g_state[258]) |= 1u;
LABEL_56:
					LODWORD(g_state[258]) |= 2u;
					break;
				default:
					break;
			}
			g_state[265] = g_state[293];
		}
		if ( LODWORD(g_state[294]) != LODWORD(g_state[266]) )
		{
			LODWORD(g_state[258]) &= 0xFFFFFFFB;
			grTexCombine(1, 1, 0, 1, 0, 0, 0);
			v14 = LODWORD(g_state[294]);
			if ( v14 > 4867 )
			{
				a1 = 0.0;
				switch ( v14 )
				{
					case 4868:
						grTexCombine(0, 4, 8, 4, 8, 0, 0);
						goto LABEL_70;
					case 4869:
						grTexCombine(0, 3, 1, 3, 1, 0, 0);
						goto LABEL_70;
					case 4870:
						grTexCombine(0, 7, 11, 7, 11, 0, 0);
						goto LABEL_70;
					case 4871:
						grTexCombine(0, 4, 1, 4, 1, 0, 0);
						goto LABEL_70;
					case 4878:
						grTexCombine(0, 6, 8, 6, 8, 0, 0);
LABEL_70:
						LODWORD(g_state[258]) |= 4u;
						break;
					default:
						break;
				}
			}
			else if ( v14 >= 4865 || !v14 )
			{
				grTexCombine(0, 1, 0, 1, 0, 0, 0);
				grTexCombine(1, 1, 0, 1, 0, 0, 0);
			}
			g_state[266] = g_state[294];
		}
		if ( g_state[280] != g_state[308] )
		{
			if ( SLODWORD(g_state[308]) < 1065353216 && (v15 = g_state[308], LODWORD(v15) | HIDWORD(v15) & 0x7FFFFFFF) )
			{
				v16 = (signed __int64)(g_state[308] * 256.0);
				if ( (signed int)v16 < 0 )
					LODWORD(v16) = 0;
				if ( (signed int)v16 > 255 )
					LODWORD(v16) = 255;
				grAlphaTestReferenceValue(LODWORD(a1), HIDWORD(v16), v16);
				grAlphaTestFunction(4);
			}
			else
			{
				grAlphaTestFunction(7);
			}
			g_state[280] = g_state[308];
		}
		if ( LODWORD(g_state[276]) != LODWORD(g_state[304]) || LODWORD(g_state[305]) != LODWORD(g_state[277]) )
		{
			switch ( LODWORD(g_state[304]) )
			{
				case 0x1201:
					v17 = 0;
					v18 = 0;
					break;
				case 0x1203:
					v17 = 2;
					v18 = 3;
					break;
				case 0x1204:
					v17 = 1;
					v18 = 1;
					break;
				case 0x1205:
					v17 = 3;
					v18 = 3;
					break;
				case 0x1206:
					v17 = 6;
					v18 = 7;
					break;
				case 0x1207:
					v18 = 5;
					goto LABEL_93;
				case 0x1208:
					v18 = 7;
LABEL_93:
					v17 = v18;
					break;
				default:
					v17 = 4;
					v18 = 4;
					break;
			}
			switch ( LODWORD(g_state[305]) )
			{
				case 0x1202:
					v20 = 4;
					goto LABEL_103;
				case 0x1203:
					v19 = 2;
					v20 = 1;
					break;
				case 0x1204:
					v20 = 3;
					goto LABEL_103;
				case 0x1205:
					v20 = 1;
					goto LABEL_103;
				case 0x1206:
					v19 = 6;
					v20 = 5;
					break;
				case 0x1207:
					v20 = 7;
					goto LABEL_103;
				case 0x1208:
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
			v21 = g_state[305];
			g_state[276] = g_state[304];
			g_state[277] = v21;
		}
		v22 = g_state[303];
		if ( LODWORD(g_state[275]) != LODWORD(v22) )
		{
			if ( v22 == 0.0 )
				grDitherMode(0);
			else
				grDitherMode(2);
			g_state[275] = g_state[303];
		}
	}
	if ( LOBYTE(g_state[258]) & 4 )
	{
		LODWORD(g_state[309]) = 2;
	}
	else
	{
		v23 = (LOBYTE(g_state[258]) & 2) == 0;
		LODWORD(g_state[309]) = 1;
		if ( v23 )
			g_state[309] = 0.0;
	}
	if ( LOBYTE(g_state[289]) & 1 )
		g_state[309] = 0.0;
	if ( LODWORD(g_state[281]) != LODWORD(g_state[309]) )
	{
		v24 = g_state[309];
		g_state[282] = 0.0;
		g_state[283] = 0.0;
		LODWORD(g_state[290]) |= 2u;
		g_state[281] = v24;
	}
	if ( LOBYTE(g_state[290]) & 2 )
	{
		if ( LOBYTE(g_state[317]) & 0x10 )
			LODWORD(g_state[307]) |= 2u;
		else
			LODWORD(g_state[307]) &= 0xFFFFFFFD;
		if ( LODWORD(g_state[309]) == 1 )
		{
			LODWORD(g_state[307]) &= 0xFFFFFFEF;
			if ( LODWORD(g_state[310]) != LODWORD(g_state[282]) )
			{
				mode_loadtexture(LODWORD(g_state[310]));
				g_state[282] = g_state[310];
				++g_stats[5];
			}
		}
		else if ( LODWORD(g_state[309]) == 2 )
		{
			if ( LODWORD(g_state[306]) )
				LODWORD(g_state[307]) &= 0xFFFFFFEF;
			else
				LODWORD(g_state[307]) |= 0x10u;
			if ( LODWORD(g_state[310]) != LODWORD(g_state[282]) || LODWORD(g_state[311]) != LODWORD(g_state[283]) )
			{
				v25 = g_state[311];
				mode_loadmultitexture(*(_QWORD *)&g_state[310], *(_QWORD *)&g_state[310] >> 32);
				v26 = g_state[310];
				g_stats[5] += 2;
				g_state[282] = v26;
			}
		}
		g_state[278] = g_state[306];
	}
	result = g_state[307];
	if ( LODWORD(g_state[279]) != LODWORD(result) )
	{
		grHints(0, LODWORD(g_state[307]));
		result = g_state[307];
		g_state[279] = result;
	}
	g_state[290] = 0.0;
	++g_stats[4];
	if ( LOBYTE(g_state[317]) & 0x20 )
	{
		x_log("#modechange:\n");
		v28 = (LODWORD(g_state[291]) & 4u) >> 2;
		v29 = (LODWORD(g_state[291]) & 2u) >> 1;
		v30 = LODWORD(g_state[291]) & 1;
		x_log("-mask c=%i z=%i zt=%i\n");
		v31 = g_state[293];
		x_log("-colortext1 %08X\n");
		v32 = g_state[308];
		x_log("-alphatest %04X\n");
		v33 = g_state[305];
		v34 = *(_QWORD *)&g_state[304];
		x_log("-blend %04X %04X\n");
		v35 = g_state[281];
		v36 = g_state[310];
		x_log("-texture %i (%i textures)\n");
	}
}


//.data:00001150 _data           segment dword public 'DATA' use32
//.data:00001150                 assume cs:_data
//.data:00001150                 ;org 1150h
//.data:0000120C $SG1379         db 'fog: type %i, %.4f..%.4f (znear=%f)',0Ah,0
//.data:00001231                 align 4
//.data:00001234 $SG1383         db '%3i: %3i',0Ah,0
//.data:0000123E                 align 10h
//.data:000012CB _data           ends
//
//
//.rdata:000012CC _rdata          segment para public 'DATA' use32
//.rdata:000012CC                 assume cs:_rdata
//.rdata:000012CC                 ;org 12CCh
//.rdata:000012CC $T1525          dq 255.0                ; DATA XREF: _init_clear+4↑r
//.rdata:000012CC                                         ; _init_clear+14↑r ...
//.rdata:000012D4 $T1539          dq 2.3                  ; DATA XREF: _generatefogtable+99↑r
//.rdata:000012DC $T1547          db    0
//.rdata:000012DD                 db    0
//.rdata:000012DE                 db    0
//.rdata:000012DF                 db    0
//.rdata:000012E0                 db    0
//.rdata:000012E1                 db    0
//.rdata:000012E2                 db 0F0h
//.rdata:000012E3                 db  3Fh ; ?
//.rdata:000012E4 $T1548          db    0
//.rdata:000012E5                 db    0
//.rdata:000012E6                 db    0
//.rdata:000012E7                 db    0
//.rdata:000012E8                 db    0
//.rdata:000012E9                 db    0
//.rdata:000012EA                 db    0
//.rdata:000012EB                 db    0
//.rdata:000012EC $T1549          dd 256.0                ; DATA XREF: _mode_change+4D8↑r
//.rdata:000012EC _rdata          ends
//
//
//
//.bss:000012F0 _bss            segment para public 'BSS' use32
//.bss:000012F0                 assume cs:_bss
//.bss:000012F0                 ;org 12F0h
//.bss:000012F0                 assume es:nothing, ss:nothing, ds:_data, fs:nothing, gs:nothing
//.bss:000012F0 ; `generatefogtable'::`2'::fogtable
//.bss:000012F0 ?fogtable@?1??generatefogtable@@9@9 db    ? ;
//.bss:000012F0                                         ; DATA XREF: _generatefogtable+3C↑o
//.bss:000012F0                                         ; _generatefogtable+94↑o ...
//.bss:000012F1                 db    ? ;
//.bss:000012F2                 db    ? ;
//.bss:000012F3                 db    ? ;
//.bss:000012F4                 db    ? ;
//.bss:000012F5                 db    ? ;
//.bss:000012F6                 db    ? ;
//.bss:000012F7                 db    ? ;
//.bss:000012F8                 db    ? ;
//.bss:000012F9                 db    ? ;
//.bss:000012FA                 db    ? ;
//.bss:000012FB                 db    ? ;
//.bss:000012FC                 db    ? ;
//.bss:000012FD                 db    ? ;
//.bss:000012FE                 db    ? ;
//.bss:000012FF                 db    ? ;
//.bss:00001300                 db    ? ;
//.bss:00001301                 db    ? ;
//.bss:00001302                 db    ? ;
//.bss:00001303                 db    ? ;
//.bss:00001304                 db    ? ;
//.bss:00001305                 db    ? ;
//.bss:00001306                 db    ? ;
//.bss:00001307                 db    ? ;
//.bss:00001308                 db    ? ;
//.bss:00001309                 db    ? ;
//.bss:0000130A                 db    ? ;
//.bss:0000130B                 db    ? ;
//.bss:0000130C                 db    ? ;
//.bss:0000130D                 db    ? ;
//.bss:0000130E                 db    ? ;
//.bss:0000130F                 db    ? ;
//.bss:00001310                 db    ? ;
//.bss:00001311                 db    ? ;
//.bss:00001312                 db    ? ;
//.bss:00001313                 db    ? ;
//.bss:00001314                 db    ? ;
//.bss:00001315                 db    ? ;
//.bss:00001316                 db    ? ;
//.bss:00001317                 db    ? ;
//.bss:00001318                 db    ? ;
//.bss:00001319                 db    ? ;
//.bss:0000131A                 db    ? ;
//.bss:0000131B                 db    ? ;
//.bss:0000131C                 db    ? ;
//.bss:0000131D                 db    ? ;
//.bss:0000131E                 db    ? ;
//.bss:0000131F                 db    ? ;
//.bss:00001320                 db    ? ;
//.bss:00001321                 db    ? ;
//.bss:00001322                 db    ? ;
//.bss:00001323                 db    ? ;
//.bss:00001324                 db    ? ;
//.bss:00001325                 db    ? ;
//.bss:00001326                 db    ? ;
//.bss:00001327                 db    ? ;
//.bss:00001328                 db    ? ;
//.bss:00001329                 db    ? ;
//.bss:0000132A                 db    ? ;
//.bss:0000132B                 db    ? ;
//.bss:0000132C                 db    ? ;
//.bss:0000132D                 db    ? ;
//.bss:0000132E                 db    ? ;
//.bss:0000132F                 db    ? ;
//.bss:00001330 ; `generatefogtable'::`2'::lastfogtype
//.bss:00001330 ?lastfogtype@?1??generatefogtable@@9@9 dd ?
//.bss:00001330                                         ; DATA XREF: _generatefogtable+2F↑r
//.bss:00001330                                         ; _generatefogtable+6D↑w
//.bss:00001334 ; `generatefogtable'::`2'::lastfogmax
//.bss:00001334 ?lastfogmax@?1??generatefogtable@@9@9 dd ?
//.bss:00001334                                         ; DATA XREF: _generatefogtable+16↑r
//.bss:00001334                                         ; _generatefogtable+61↑w
//.bss:00001338 ; `generatefogtable'::`2'::lastfogmin
//.bss:00001338 ?lastfogmin@?1??generatefogtable@@9@9 dd ?
//.bss:00001338                                         ; DATA XREF: _generatefogtable↑r
//.bss:00001338                                         ; _generatefogtable+5C↑w
//.bss:00001338 _bss            ends
