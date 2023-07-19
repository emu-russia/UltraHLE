// Texture memory management and texture upload

#include "pch.h"

int g_lasttexture;
xt_texture g_texture[MAXTEXTURES];		// The 0th entry is not used

static xt_memory* mem[GLIDE_NUM_TMU];

t_block* newblock(xt_memory* m)
{
	if (m->tableind >= X_MAX_BLOCKS)
		return 0;
	return &m->table[m->tableind++];
}

DWORD * addbefore(DWORD *b, DWORD *t)
{
	DWORD *result; // eax

	result = b;
	t[1] = b;
	*t = *b;
	*b = t;
	*(DWORD *)(*t + 4) = t;
	return result;
}

int addafter(int b, DWORD *t)
{
	DWORD *v2; // ecx
	int result; // eax

	v2 = *(DWORD **)(b + 4);
	t[1] = v2;
	*t = b;
	*v2 = t;
	result = *t;
	*(DWORD *)(*t + 4) = t;
	return result;
}

DWORD * removeblk(DWORD **b)
{
	DWORD *result; // eax

	*b[1] = *b;
	result = b[1];
	(*b)[1] = result;
	return result;
}

DWORD * memory_clear(DWORD *memory)
{
	// t, base, b
	DWORD *v1; // edi
	DWORD *result; // eax
	signed int v3; // esi
	DWORD *v4; // eax

	v1 = memory + 2;
	result = memory + 6;
	v3 = 8;
	memory[3] = memory + 2;
	*v1 = v1;
	memory[7] = memory + 6;
	*result = result;
	for (memory[11] = 0; memory[1] > v3; result = addbefore(v1, v4) )
	{
		v4 = (DWORD *)newblock((int)memory);
		v4[3] = 0x200000 - 8;
		v4[2] = v3;
		v3 += 0x200000;
	}
	return result;
}

DWORD * memory_alloc(int memory, int size, int* base)
{
	int v3; // ebx
	signed int v4; // edi
	DWORD *result; // eax
	DWORD *v6; // ebp
	int v7; // [esp+10h] [ebp-4h]

	v3 = *(DWORD *)(memory + 12);
	v4 = (size + 7) & 0xFFFFFFF8;
	if (memory - v3 == -8 )
	{
LABEL_4:
		x_log("allocate %6i bytes: failed\n", (size + 7) & 0xFFFFFFF8);
		*base = 0;
		result = 0;
	}
	else
	{
		while ( *(DWORD *)(v3 + 12) < v4 )
		{
			v3 = *(DWORD *)(v3 + 4);
			if (memory - v3 == -8 )
				goto LABEL_4;
		}
		v7 = *(DWORD *)(v3 + 12) - v4;
		*base = *(DWORD *)(v3 + 8);
		if ( v7 >= 1024 )
		{
			v6 = (DWORD *)newblock(memory);
			if ( v6 )
			{
				v6[2] = *(DWORD *)(v3 + 8);
				v6[3] = v4;
				addafter(memory + 24, v6);
				result = v6;
				*(DWORD *)(v3 + 12) = v7;
				*(DWORD *)(v3 + 8) += v4;
			}
			else
			{
				result = 0;
			}
		}
		else
		{
			removeblk((DWORD **)v3);
			addbefore((DWORD *)(memory + 24), (DWORD *)v3);
			result = (DWORD *)v3;
		}
	}
	return result;
}

int memory_free(int memory, DWORD **handle)
{
	removeblk(handle);
	return addafter(memory + 8, handle);
}

xt_memory* memory_create(int min, int max)
{
	xt_memory* mem; // ebx

	x_log("%iMB\n", ((max - min) / 1024 + 512) / 1024);
	mem = (xt_memory *)x_alloc(sizeof(xt_memory));
	if ( !min)
		min = 8;

	mem->min = min;
	mem->max = max;
	mem->free.size = -1;
	mem->used.size = -1;
	mem->table = (t_block *)x_alloc(X_MAX_BLOCKS * sizeof(t_block));	// 0x8000
	mem->tableind = 0;
	memory_clear(mem);
	return mem;
}

void memory_delete(xt_memory* memory)
{
	x_free(memory->table);
	x_free(memory);
}

void freetexmem(DWORD *txt)
{
	signed int v1; // esi
	int v2; // ebx

	v1 = 4;
	v2 = (int)(txt + 18);
	do
	{
		if ( *(DWORD *)v2 )
			memory_free(mem[*(DWORD *)(v2 + 32)], *(DWORD ***)(v2 + 48));
		*(DWORD *)(v2 + 16) = 0;
		v2 += 4;
		--v1;
		*(DWORD *)(v2 - 4) = 0;
		*(DWORD *)(v2 + 44) = 0;
	}
	while ( v1 );

	txt[26] = 0;
	txt[27] = 1;
	txt[28] = 0;
	txt[29] = 1;
}

void makespace()
{
	int v0; // esi
	signed int v1; // ebx
	DWORD *v2; // edi
	int v3; // eax

	v0 = 0;
	v1 = 1;
	if ( g_lasttexture >= 1 )
	{
		v2 = (DWORD *)(&g_texture[1]);
		do
		{
			if ( *v2 )
			{
				v3 = v2[21] + v2[20] + v2[19] + v2[18];
				if ( v3 )
				{
					if ( v2[11] < g_state[XST].frame - 1 )
					{
						v0 += v3;
						freetexmem(v2);
					}
				}
			}
			v2 += 38;
			++v1;
		}
		while ( v1 <= g_lasttexture );
	}
	x_log("texture: makespace %i..%i freed %i bytes\n", 1, g_lasttexture, v0);
}

void clearspace()
{
	signed int v0; // esi
	DWORD *v1; // edi

	v0 = 1;
	memory_clear(mem[0]);
	memory_clear(mem[1]);
	if ( g_lasttexture >= 1 )
	{
		v1 = (DWORD *)(&g_texture[1]);
		do
		{
			if ( *v1 && v1[21] + v1[20] + v1[19] + v1[18] )
			{
				v1[18] = 0;
				v1[19] = 0;
				v1[20] = 0;
				v1[21] = 0;
				freetexmem(v1);
			}
			v1 += 38;
			++v0;
		}
		while ( v0 <= g_lasttexture );
	}
	x_log("texture: clearspace %i..%i freed all memory\n", 1, g_lasttexture);
}

int picktmu()
{
	static int tmupick = 0;
	int result;

	if ( g_state[XST].tmus < 2 )
		return 0;
	result = tmupick ^ 1;
	tmupick ^= 1u;
	return result;
}

int fxloadtexturepart(xt_texture *txt, int texturepart)
{
	int v2; // ebx
	FxU32 v3; // ebp
	int v7; // [esp+10h] [ebp-Ch]
	int v8; // [esp+14h] [ebp-8h]
	DWORD *v9; // [esp+18h] [ebp-4h]

	txt->lastframeused = g_state[XST].frame;
	v2 = txt->tmu[texturepart];
	switch (texturepart)
	{
		case 0:
			v3 = GR_MIPMAPLEVELMASK_BOTH;
			break;
		case 1:
			v3 = GR_MIPMAPLEVELMASK_BOTH;
			break;
		case 2:
			v3 = GR_MIPMAPLEVELMASK_EVEN;
			break;
		case 3:
			v3 = GR_MIPMAPLEVELMASK_ODD;
			break;
		default:
			// TODO: uninitialized local variable 'v8' used
			v3 = 0;// v8;
			break;
	}
	if ( !txt->size[texturepart] )
	{
		v7 = grTexTextureMemRequired(v3, &txt->ti);
		v9 = memory_alloc(mem[v2], v7, &v8);
		if ( !v7 )
			x_fatal("texture: zero size for handle %i (part=%i,mask=%i)\n", txt->handle, texturepart, v3);
		if ( !v9 )
			return -1;
		txt->size[texturepart] = v7;
		txt->xblock[texturepart] = v9;
		txt->base[texturepart] = v8;
		txt->reload = 1;
	}
	if (txt->reload)
	{
		grTexDownloadMipMap(v2, txt->base[texturepart], v3, &txt->ti);
		txt->reload = 0;
		g_stats.text_uploaded += txt->size[texturepart];
	}
	grTexSource(v2, txt->base[texturepart], v3, &txt->ti);
	txt->usedsize[texturepart] = txt->size[texturepart];
	return 0;
}

int fxloadtexture_single(xt_texture* txt)
{
	int v1; // edi
	int v2; // ebx

	v1 = 0;
	if (g_state[XST].tmus > 1 && txt->size[0] <= 0)
	{
		if (txt->size[1] <= 0)
			v1 = picktmu();
		else
			v1 = 1;
	}
	v2 = 0;
	txt->tmu[v1] = v1;
	g_state[XST].texturexmul = txt->xmul;
	g_state[XST].textureymul = txt->ymul;
	do
	{
		if (fxloadtexturepart(txt, v1) >= 0)
			break;
		if (!v2)
			makespace();
		if (v2 == 1)
			clearspace();
		if (v2 == 2)
			x_fatal("loadtexture: unable to load texture (single/part %i)!\n", v1);
		++v2;
	} while (v2 < 3);
	return v1;
}

int fxloadtexture_trilin(xt_texture* txt)
{
	int v1; // eax
	int v2; // esi

	if ( !txt->size[2] && !txt->size[3])
	{
		v1 = picktmu();
		txt->tmu[2] = v1;
		txt->tmu[3] = v1 ^ 1;
	}
	v2 = 0;
	g_state[XST].texturexmul = txt->xmul;
	g_state[XST].textureymul = txt->ymul;
	do
	{
		if ( fxloadtexturepart(txt, 2) >= 0 && fxloadtexturepart(txt, 3) >= 0 )
			break;
		if ( !v2 )
			makespace();
		if ( v2 == 1 )
			clearspace();
		if ( v2 == 2 )
			x_fatal("loadtexture: unable to load texture (trilin)!\n", 0);
		++v2;
	}
	while ( v2 < 3 );
	return txt->tmu[2];
}

int fxloadtexture_multi(xt_texture* txt1, xt_texture* txt2)
{
	int v2; // ebx

	v2 = 0;
	g_state[XST].texturexmul = txt1->xmul;
	g_state[XST].textureymul = txt1->ymul;
	do
	{
		if ( fxloadtexturepart(txt1, 0) >= 0 && fxloadtexturepart(txt2, 1) >= 0 )
			break;
		if ( !v2 )
			makespace();
		if ( v2 == 1 )
			clearspace();
		if ( v2 == 2 )
			x_fatal("loadtexture: unable to load texture (multi)!\n");
		++v2;
	}
	while ( v2 < 3 );
	return 0;
}

void text_init()
{
	signed int v0; // ST04_4
	int v1; // eax
	xt_memory *result; // eax
	signed int v3; // ST04_4
	int v4; // eax

	x_log("Texture memory TMU0: ");
	v1 = grTexMinAddress(GR_TMU0);
	v0 = grTexMaxAddress(GR_TMU0);
	mem[0] = memory_create(v1 + 256, v0);
	x_log("Texture memory TMU1: ");
	if ( g_state[XST].tmus >= 2 )
	{
		v4 = grTexMinAddress(GR_TMU1);
		v3 = grTexMaxAddress(GR_TMU1);
		result = memory_create(v4 + 256, v3);
	}
	else
	{
		result = memory_create(0, 0);
	}
	mem[1] = result;
}

void text_deinit()
{
	if ( mem[0] )
		memory_delete(mem[0]);
	if ( mem[1])
		memory_delete(mem[1]);
	mem[0] = 0;
	mem[1] = 0;
}

int accesstexture(xt_texture *txt, int level, int *xsize, int *ysize)
{
	signed int v4; // ebp
	signed int v5; // ebx
	int v6[2];

	if (txt->levels < level)
		x_fatal("access texture illegal level");
	v4 = 0;
	v5 = 0;
	v6[0] = v6[1] = 0;
	if (level >= 0 )
	{
		do
		{
			v4 = txt->width >> (v6[1] & 0xf);
			if ( !v4 )
				v4 = 1;
			v5 = txt->height >> (v6[1] & 0xf);
			if ( !v5 )
				v5 = 1;
			if ( v6[0] != level)
				v6[1] = v4 * v5 + v6[1];
			++v6[0];
		}
		while ( v6[0] <= level);
	}
	if (txt->levels == level)
	{
		v4 = 0;
		v5 = 0;
	}
	if (xsize)
		*xsize = v4;
	if (ysize)
		*ysize = v5;
	return v6[1];
}

int text_allocdata(int txt)
{
	signed int v1; // ebx
	int v2; // ebp
	unsigned int v3; // esi
	int v4; // eax
	signed int v5; // ebx
	DWORD *v6; // esi
	int v7; // eax
	int v8; // eax
	int v9; // eax
	int v10; // eax
	int v11; // eax
	DWORD *v12; // ecx
	signed int v13; // eax
	int result; // eax

	v1 = *(DWORD *)(txt + 8);
	if ( v1 < *(DWORD *)(txt + 12) )
	{
		v2 = *(DWORD *)(txt + 8);
		v3 = 0;
		v1 = *(DWORD *)(txt + 12);
	}
	else
	{
		v2 = *(DWORD *)(txt + 12);
		v3 = 1;
	}
	if ( v1 > 8 )
	{
		switch ( v1 )
		{
			case 16:
				*(DWORD *)(txt + 56) = 4;
				*(DWORD *)(txt + 28) = 5;
				break;
			case 32:
				*(DWORD *)(txt + 56) = 3;
				*(DWORD *)(txt + 28) = 6;
				break;
			case 64:
				*(DWORD *)(txt + 56) = 2;
				*(DWORD *)(txt + 28) = 7;
				break;
			case 128:
				*(DWORD *)(txt + 56) = 1;
				*(DWORD *)(txt + 28) = 8;
				break;
			case 256:
				*(DWORD *)(txt + 56) = 0;
				*(DWORD *)(txt + 28) = 9;
				break;
			default:
				x_fatal("Illegal texture size %ix%i", v1, v2);
				break;
		}
	}
	else
	{
		switch ( v1 )
		{
			case 8:
				*(DWORD *)(txt + 56) = 5;
				*(DWORD *)(txt + 28) = 4;
				break;
			case 1:
				*(DWORD *)(txt + 56) = 8;
				*(DWORD *)(txt + 28) = 1;
				break;
			case 2:
				*(DWORD *)(txt + 56) = 7;
				*(DWORD *)(txt + 28) = 2;
				break;
			case 4:
				*(DWORD *)(txt + 56) = 6;
				*(DWORD *)(txt + 28) = 3;
				break;
			default:
				x_fatal("Illegal texture size %ix%i", v1, v2);
				break;
		}
	}
	if ( !(*(BYTE *)(txt + 17) & 2) )
		*(DWORD *)(txt + 28) = 1;
	switch ( v1 / v2 )
	{
		case 1:
			*(DWORD *)(txt + 60) = 3;
			break;
		case 2:
			v4 = v3 < 1 ? 4 : 2;
			*(DWORD*)(txt + 60) = v4;
			break;
		case 4:
			v4 = v3 < 1 ? 5 : 1;
			*(DWORD*)(txt + 60) = v4;
			break;
		case 8:
			v4 = v3 < 1 ? 6 : 0;
			*(DWORD*)(txt + 60) = v4;
			break;
		default:
			x_fatal("Illegal texture aspect %i/%i", v1, v2);
			break;
	}
	switch ( *(DWORD *)(txt + 60) )
	{
		case 0:
			*(DWORD *)(txt + 36) = 256.0f;
			*(DWORD *)(txt + 40) = 32.0f;
			break;
		case 1:
			*(DWORD *)(txt + 36) = 256.0f;
			*(DWORD *)(txt + 40) = 64.0f;
			break;
		case 2:
			*(DWORD *)(txt + 36) = 256.0f;
			*(DWORD *)(txt + 40) = 128.0f;
			break;
		case 3:
			*(DWORD *)(txt + 36) = 256.0f;
			*(DWORD *)(txt + 40) = 256.0f;
			break;
		case 4:
			*(DWORD *)(txt + 36) = 128.0f;
			*(DWORD*)(txt + 40) = 256.0f;
			break;
		case 5:
			*(DWORD *)(txt + 36) = 64.0f;
			*(DWORD*)(txt + 40) = 256.0f;
			break;
		case 6:
			*(DWORD *)(txt + 36) = 32.0f;
			*(DWORD *)(txt + 40) = 256.0f;
			break;
		default:
			break;
	}
	v5 = v1 >> (*(unsigned int *)(txt + 28) - 1);
	if ( v5 > 8 )
	{
		switch ( v5 )
		{
			case 16:
				*(DWORD *)(txt + 52) = 4;
				break;
			case 32:
				*(DWORD *)(txt + 52) = 3;
				break;
			case 64:
				*(DWORD *)(txt + 52) = 2;
				break;
			case 128:
				*(DWORD *)(txt + 52) = 1;
				break;
			case 256:
				*(DWORD *)(txt + 52) = 0;
				break;
			default:
				x_fatal("Illegal texture small size");
				break;
		}
	}
	else
	{
		switch ( v5 )
		{
			case 8:
				*(DWORD *)(txt + 52) = 5;
				break;
			case 1:
				*(DWORD *)(txt + 52) = 8;
				break;
			case 2:
				*(DWORD *)(txt + 52) = 7;
				break;
			case 4:
				*(DWORD *)(txt + 52) = 6;
				break;
			default:
				x_fatal("Illegal texture small size");
				break;
		}
	}
	switch ( *(DWORD *)(txt + 16) & X_FORMATMASK)
	{
		case X_RGBA5551:
			v6 = (DWORD *)(txt + 24);
			v8 = accesstexture((DWORD *)txt, *(DWORD *)(txt + 28), 0, 0);
			*(DWORD *)(txt + 20) = 0;
			*(DWORD *)(txt + 64) = 11;
			*(DWORD *)(txt + 24) = 2 * v8;
			break;
		case X_RGB565:
			v6 = (DWORD *)(txt + 24);
			v9 = accesstexture((DWORD *)txt, *(DWORD *)(txt + 28), 0, 0);
			*(DWORD *)(txt + 20) = 3;
			*(DWORD *)(txt + 64) = 10;
			*(DWORD *)(txt + 24) = 2 * v9;
			break;
		case X_I8:
			v6 = (DWORD *)(txt + 24);
			v10 = accesstexture((DWORD *)txt, *(DWORD *)(txt + 28), 0, 0);
			*(DWORD *)(txt + 20) = 4;
			*(DWORD *)(txt + 64) = 3;
			*(DWORD *)(txt + 24) = 2 * v10;
			break;
		default:
			v6 = (DWORD *)(txt + 24);
			v7 = accesstexture((DWORD *)txt, *(DWORD *)(txt + 28), 0, 0);
			*(DWORD *)(txt + 20) = 1;
			*(DWORD *)(txt + 64) = 12;
			*(DWORD *)(txt + 24) = 2 * v7;
			break;
	}
	v11 = x_alloc(*v6);
	v12 = (DWORD *)(txt + 72);
	*(DWORD *)(txt + 68) = v11;
	v13 = 4;
	do
	{
		*v12 = 0;
		++v12;
		--v13;
		v12[11] = 0;
	}
	while ( v13 );
	result = 0;
	*(DWORD *)(txt + 104) = 0;
	*(DWORD *)(txt + 108) = 1;
	*(DWORD *)(txt + 112) = 0;
	*(DWORD *)(txt + 116) = 1;
	return result;
}

int text_loadlevel(xt_texture* txt, int level, unsigned int data)
{
	unsigned int result; // eax
	int v4; // esi
	unsigned int v5; // edx
	unsigned int v6; // ebx
	int v7; // eax
	unsigned int v8; // edx
	unsigned int v9; // esi
	unsigned __int8 v10; // cl
	int v11; // eax
	unsigned int v12; // edx
	int v13; // ecx
	char v14; // bl
	int v15; // eax
	unsigned int v16; // edx
	int v17; // ecx
	char v18; // bl
	int v19; // [esp+Ch] [ebp-8h]
	int v20; // [esp+10h] [ebp-4h]

	result = txt->ti.format;
	switch ( result )
	{
		case GR_TEXFMT_INTENSITY_8:
			v4 = accesstexture(txt, level, &v20, &v19) + (uint8_t*)txt->ti.data;
			result = v20 * v19;
			v5 = data;
			v6 = data + 4 * v20 * v19;
			if ( v6 > data)
			{
				do
				{
					++v4;
					v5 += 4;
					result = *(char *)(v5 - 4);
					*(BYTE *)(v4 - 1) = (signed int)(result + *(char *)(v5 - 2) + 2 * *(char *)(v5 - 3)) >> 2;
				}
				while ( v6 > v5 );
			}
			break;
		case GR_TEXFMT_RGB_565:
			v7 = accesstexture(txt, level, &v20, &v19);
			v8 = data;
			result = (uint8_t*)txt->ti.data + 2 * v7;
			v9 = data + 4 * v20 * v19;
			if ( v9 > data)
			{
				do
				{
					v10 = *(BYTE *)(v8 + 1);
					v8 += 4;
					result += 2;
					*(WORD *)(result - 2) = ((char)(*(BYTE *)(v8 - 4) >> 3) << 11) | 
						(*(BYTE *)(v8 - 2) >> 3) & 0x1F | 
						32 * (v10 >> 2);
				}
				while ( v9 > v8 );
			}
			break;
		case GR_TEXFMT_ARGB_1555:
			v11 = accesstexture(txt, level, &v20, &v19);
			v12 = data;
			v13 = (uint8_t*)txt->ti.data + 2 * v11;
			result = data + 4 * v20 * v19;
			if ( result > data)
			{
				do
				{
					v14 = *(BYTE *)(v12 + 3);
					v12 += 4;
					v13 += 2;
					*(WORD *)(v13 - 2) = (*(BYTE *)(v12 - 2) >> 3) & 0x1F | 
						(((*(BYTE *)(v12 - 4) >> 3) & 0x1F) << 10) | 
						32 * ((*(BYTE *)(v12 - 3) >> 3) & 0x1F | 
						((char)(v14 >> 7) << 10));
				}
				while ( result > v12 );
			}
			break;
		case GR_TEXFMT_ARGB_4444:
			v15 = accesstexture(txt, level, &v20, &v19);
			v16 = data;
			v17 = (uint8_t*)txt->ti.data + 2 * v15;
			result = data + 4 * v20 * v19;
			if ( result > data)
			{
				do
				{
					v18 = *(BYTE *)(v16 + 3);
					v16 += 4;
					v17 += 2;
					*(WORD *)(v17 - 2) = (((*(BYTE *)(v16 - 4) >> 4) & 0xF) << 8) | 
						(*(BYTE *)(v16 - 2) >> 4) & 0xF | 
						16 * ((*(BYTE *)(v16 - 3) >> 4) & 0xF | 
						((char)(v18 >> 4) << 8));
				}
				while ( result > v16 );
			}
			break;
		default:
			break;
	}
	txt->reload = 1;
	return result;
}

void text_freedata(xt_texture *txt)
{
	freetexmem(txt);
	if (txt->ti.data )
		x_free(txt->ti.data);
	memset(txt, 0, sizeof(xt_texture));
}

void text_cleartexmem()
{
	clearspace();
}

void* text_opendata(xt_texture* txt)
{
	return txt->ti.data;
}

void text_closedata(xt_texture* txt)
{
	txt->reload = 1;
	if ( txt->handle == g_state[XST].active.text1)
		g_state[XST].active.text1 = 0;
	if ( txt->handle == g_state[XST].active.text2)
		g_state[XST].active.text2 = 0;
}

int text_frameend()
{
	int result; // eax
	signed int v1; // edi
	DWORD *v2; // ecx
	int *v3; // esi
	signed int v4; // edx

	result = 0;
	g_stats.text_resident = 0;
	v1 = 1;
	g_stats.text_used = 0;
	if ( g_lasttexture >= 1 )
	{
		v2 = (DWORD *)(&g_texture[1]);
		do
		{
			if ( *v2 )
			{
				v3 = v2 + 34;
				v4 = 4;
				do
				{
					g_stats.text_resident += *(v3 - 16);
					result = g_state[XST].frame - 1;
					if ( result <= v2[11] )
					{
						result = *v3;
						g_stats.text_used += *v3;
					}
					*v3 = 0;
					++v3;
					--v4;
				}
				while ( v4 );
			}
			v2 += 38;
			++v1;
		}
		while ( g_lasttexture >= v1 );
	}
	return result;
}
