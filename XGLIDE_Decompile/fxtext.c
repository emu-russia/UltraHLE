#include "pch.h"

int g_lasttexture;

int newblock(int a1)
{
	signed int v1; // edx

	v1 = *(DWORD *)(a1 + 44);
	if ( v1 >= 2048 )
		return 0;
	*(DWORD *)(a1 + 44) = v1 + 1;
	return 16 * v1 + *(DWORD *)(a1 + 40);
}

DWORD * addbefore(DWORD *a1, DWORD *a2)
{
	DWORD *result; // eax

	result = a1;
	a2[1] = a1;
	*a2 = *a1;
	*a1 = a2;
	*(DWORD *)(*a2 + 4) = a2;
	return result;
}

int addafter(int a1, DWORD *a2)
{
	DWORD *v2; // ecx
	int result; // eax

	v2 = *(DWORD **)(a1 + 4);
	a2[1] = v2;
	*a2 = a1;
	*v2 = a2;
	result = *a2;
	*(DWORD *)(*a2 + 4) = a2;
	return result;
}

DWORD * removeblk(DWORD **a1)
{
	DWORD *result; // eax

	*a1[1] = *a1;
	result = a1[1];
	(*a1)[1] = result;
	return result;
}

DWORD * memory_clear(DWORD *a1)
{
	DWORD *v1; // edi
	DWORD *result; // eax
	signed int v3; // esi
	DWORD *v4; // eax

	v1 = a1 + 2;
	result = a1 + 6;
	v3 = 8;
	a1[3] = a1 + 2;
	*v1 = v1;
	a1[7] = a1 + 6;
	*result = result;
	for ( a1[11] = 0; a1[1] > v3; result = addbefore(v1, v4) )
	{
		v4 = (DWORD *)newblock((int)a1);
		v4[3] = 2097144;
		v4[2] = v3;
		v3 += 0x200000;
	}
	return result;
}

DWORD * memory_alloc(int a1, int a2, DWORD *a3)
{
	int v3; // ebx
	signed int v4; // edi
	DWORD *result; // eax
	DWORD *v6; // ebp
	int v7; // [esp+10h] [ebp-4h]

	v3 = *(DWORD *)(a1 + 12);
	v4 = (a2 + 7) & 0xFFFFFFF8;
	if ( a1 - v3 == -8 )
	{
LABEL_4:
		x_log("allocate %6i bytes: failed\n", (a2 + 7) & 0xFFFFFFF8);
		*a3 = 0;
		result = 0;
	}
	else
	{
		while ( *(DWORD *)(v3 + 12) < v4 )
		{
			v3 = *(DWORD *)(v3 + 4);
			if ( a1 - v3 == -8 )
				goto LABEL_4;
		}
		v7 = *(DWORD *)(v3 + 12) - v4;
		*a3 = *(DWORD *)(v3 + 8);
		if ( v7 >= 1024 )
		{
			v6 = (DWORD *)newblock(a1);
			if ( v6 )
			{
				v6[2] = *(DWORD *)(v3 + 8);
				v6[3] = v4;
				addafter(a1 + 24, v6);
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
			addbefore((DWORD *)(a1 + 24), (DWORD *)v3);
			result = (DWORD *)v3;
		}
	}
	return result;
}

int memory_free(int a1, DWORD **a2)
{
	removeblk(a2);
	return addafter(a1 + 8, a2);
}

signed int * memory_create(signed int a1, signed int a2)
{
	signed int v2; // esi
	signed int *v3; // ebx

	v2 = a1;
	x_log("%iMB\n", ((a2 - a1) / 1024 + 512) / 1024);
	v3 = (signed int *)x_alloc(48);
	if ( !a1 )
		v2 = 8;
	*v3 = v2;
	v3[1] = a2;
	v3[5] = -1;
	v3[9] = -1;
	v3[10] = x_alloc(0x8000);
	v3[11] = 0;
	memory_clear(v3);
	return v3;
}

void memory_delete(uint8_t* memory)
{
	x_free(*(DWORD *)(memory + 40));
	x_free(memory);
}

int freetexmem(DWORD *a1)
{
	signed int v1; // esi
	int v2; // ebx
	int result; // eax

	v1 = 4;
	v2 = (int)(a1 + 18);
	do
	{
		if ( *(DWORD *)v2 )
			memory_free(mem_S1205[*(DWORD *)(v2 + 32)], *(DWORD ***)(v2 + 48));
		*(DWORD *)(v2 + 16) = 0;
		v2 += 4;
		--v1;
		*(DWORD *)(v2 - 4) = 0;
		*(DWORD *)(v2 + 44) = 0;
	}
	while ( v1 );
	result = 0;
	a1[26] = 0;
	a1[27] = 1;
	a1[28] = 0;
	a1[29] = 1;
	return result;
}

int makespace()
{
	int v0; // esi
	signed int v1; // ebx
	DWORD *v2; // edi
	int v3; // eax

	v0 = 0;
	v1 = 1;
	if ( g_lasttexture >= 1 )
	{
		v2 = (DWORD *)((char *)&g_texture + 152);
		do
		{
			if ( *v2 )
			{
				v3 = v2[21] + v2[20] + v2[19] + v2[18];
				if ( v3 )
				{
					if ( v2[11] < g_state.frame - 1 )
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

int clearspace()
{
	signed int v0; // esi
	DWORD *v1; // edi

	v0 = 1;
	memory_clear((DWORD *)mem_S1205[0]);
	memory_clear((DWORD *)dword_131C);
	if ( g_lasttexture >= 1 )
	{
		v1 = (DWORD *)((char *)&g_texture + 152);
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

	if ( g_state.tmus < 2 )
		return 0;
	result = tmupick ^ 1;
	tmupick ^= 1u;
	return result;
}

signed int fxloadtexturepart(DWORD *a1, int a2)
{
	int v2; // ebx
	signed int v3; // ebp
	signed int result; // eax
	DWORD *v5; // ecx
	int v6; // edx
	int v7; // [esp+10h] [ebp-Ch]
	int v8; // [esp+14h] [ebp-8h]
	DWORD *v9; // [esp+18h] [ebp-4h]

	a1[11] = g_state.frame;
	v2 = a1[a2 + 26];
	switch ( a2 )
	{
		case 0:
			v3 = 3;
			break;
		case 1:
			v3 = 3;
			break;
		case 2:
			v3 = 1;
			break;
		case 3:
			v3 = 2;
			break;
		default:
			v3 = v8;
			break;
	}
	if ( !a1[a2 + 18] )
	{
		v7 = grTexTextureMemRequired(v3, a1 + 13);
		v9 = memory_alloc(mem_S1205[v2], v7, &v8);
		if ( !v7 )
			x_fatal("texture: zero size for handle %i (part=%i,mask=%i)\n", a1[1], a2, v3);
		if ( !v9 )
			return -1;
		v5 = v9;
		v6 = v8;
		a1[a2 + 18] = v7;
		a1[a2 + 30] = v5;
		a1[a2 + 22] = v6;
		a1[12] = 1;
	}
	if ( a1[12] )
	{
		grTexDownloadMipMap(v2, a1[a2 + 22], v3, a1 + 13);
		a1[12] = 0;
		g_stats.text_uploaded += a1[a2 + 18];
	}
	grTexSource(v2, a1[a2 + 22], v3, a1 + 13);
	result = 0;
	a1[a2 + 34] = a1[a2 + 18];
	return result;
}

signed int fxloadtexture_single(DWORD *a1)
{
	signed int v1; // edi
	signed int v2; // ebx

	v1 = 0;
	if ( g_state.tmus > 1 && a1[18] <= 0 )
	{
		if ( a1[19] <= 0 )
			v1 = picktmu();
		else
			v1 = 1;
	}
	v2 = 0;
	a1[v1 + 26] = v1;
	g_state[256] = a1[9];
	g_state[257] = a1[10];
	do
	{
		if ( fxloadtexturepart(a1, v1) >= 0 )
			break;
		if ( !v2 )
			makespace();
		if ( v2 == 1 )
			clearspace();
		if ( v2 == 2 )
			x_fatal("loadtexture: unable to load texture (single/part %i)!\n", v1);
		++v2;
	}
	while ( v2 < 3 );
	return v1;
}

int fxloadtexture_trilin(DWORD *a1)
{
	int v1; // eax
	signed int v2; // esi

	if ( !a1[20] && !a1[21] )
	{
		v1 = picktmu();
		a1[28] = v1;
		a1[29] = v1 ^ 1;
	}
	v2 = 0;
	g_state[256] = a1[9];
	g_state[257] = a1[10];
	do
	{
		if ( fxloadtexturepart(a1, 2) >= 0 && fxloadtexturepart(a1, 3) >= 0 )
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
	return a1[28];
}

int fxloadtexture_multi(DWORD *a1, DWORD *a2)
{
	signed int v2; // ebx

	v2 = 0;
	g_state[256] = a1[9];
	g_state[257] = a1[10];
	do
	{
		if ( fxloadtexturepart(a1, 0) >= 0 && fxloadtexturepart(a2, 1) >= 0 )
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

signed int *text_init()
{
	signed int v0; // ST04_4
	int v1; // eax
	signed int *result; // eax
	signed int v3; // ST04_4
	int v4; // eax

	x_log("Texture memory TMU0: ");
	v0 = grTexMaxAddress(GR_TMU0);
	v1 = grTexMinAddress(GR_TMU0);
	mem_S1205[0] = (int)memory_create(v1 + 256, v0);
	x_log("Texture memory TMU1: ");
	if ( g_state.tmus >= 2 )
	{
		v3 = grTexMaxAddress(GR_TMU1);
		v4 = grTexMinAddress(GR_TMU1);
		result = memory_create(v4 + 256, v3);
	}
	else
	{
		result = memory_create(0, 0);
	}
	dword_131C = (int)result;
	return result;
}

void text_deinit()
{
	if ( mem_S1205[0] )
		memory_delete(mem_S1205[0]);
	if ( dword_131C )
		memory_delete(dword_131C);
	mem_S1205[0] = 0;
	dword_131C = 0;
}

int accesstexture(DWORD *a1, int a2, signed int *a3, signed int *a4)
{
	signed int v4; // ebp
	signed int v5; // ebx
	__int64 v6; // rax

	if ( a1[7] < a2 )
		x_fatal("access texture illegal level");
	v4 = 0;
	v5 = 0;
	v6 = 0i64;
	if ( a2 >= 0 )
	{
		do
		{
			v4 = a1[2] >> SBYTE4(v6);
			if ( !v4 )
				v4 = 1;
			v5 = a1[3] >> SBYTE4(v6);
			if ( !v5 )
				v5 = 1;
			if ( HIDWORD(v6) != a2 )
				LODWORD(v6) = v4 * v5 + v6;
			++HIDWORD(v6);
		}
		while ( SHIDWORD(v6) <= a2 );
	}
	if ( a1[7] == a2 )
	{
		v4 = 0;
		v5 = 0;
	}
	if ( a3 )
		*a3 = v4;
	if ( a4 )
		*a4 = v5;
	return v6;
}

int text_allocdata(int a1)
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

	v1 = *(DWORD *)(a1 + 8);
	if ( v1 < *(DWORD *)(a1 + 12) )
	{
		v2 = *(DWORD *)(a1 + 8);
		v3 = 0;
		v1 = *(DWORD *)(a1 + 12);
	}
	else
	{
		v2 = *(DWORD *)(a1 + 12);
		v3 = 1;
	}
	if ( v1 > 8 )
	{
		switch ( v1 )
		{
			case 16:
				*(DWORD *)(a1 + 56) = 4;
				*(DWORD *)(a1 + 28) = 5;
				break;
			case 32:
				*(DWORD *)(a1 + 56) = 3;
				*(DWORD *)(a1 + 28) = 6;
				break;
			case 64:
				*(DWORD *)(a1 + 56) = 2;
				*(DWORD *)(a1 + 28) = 7;
				break;
			case 128:
				*(DWORD *)(a1 + 56) = 1;
				*(DWORD *)(a1 + 28) = 8;
				break;
			case 256:
				*(DWORD *)(a1 + 56) = 0;
				*(DWORD *)(a1 + 28) = 9;
				break;
			default:
				goto $L1425;
		}
	}
	else
	{
		switch ( v1 )
		{
			case 8:
				*(DWORD *)(a1 + 56) = 5;
				*(DWORD *)(a1 + 28) = 4;
				break;
			case 1:
				*(DWORD *)(a1 + 56) = 8;
				*(DWORD *)(a1 + 28) = 1;
				break;
			case 2:
				*(DWORD *)(a1 + 56) = 7;
				*(DWORD *)(a1 + 28) = 2;
				break;
			case 4:
				*(DWORD *)(a1 + 56) = 6;
				*(DWORD *)(a1 + 28) = 3;
				break;
			default:
$L1425:
				x_fatal("Illegal texture size %ix%i", v1, v2);
				break;
		}
	}
	if ( !(*(BYTE *)(a1 + 17) & 2) )
		*(DWORD *)(a1 + 28) = 1;
	switch ( v1 / v2 )
	{
		case 1:
			*(DWORD *)(a1 + 60) = 3;
			break;
		case 2:
			v4 = v3 < 1 ? 4 : 2;
			goto LABEL_28;
		case 4:
			v4 = v3 < 1 ? 5 : 1;
			goto LABEL_28;
		case 8:
			v4 = v3 < 1 ? 6 : 0;
LABEL_28:
			*(DWORD *)(a1 + 60) = v4;
			break;
		default:
			x_fatal("Illegal texture aspect %i/%i", v1, v2);
			break;
	}
	switch ( *(DWORD *)(a1 + 60) )
	{
		case 0:
			*(DWORD *)(a1 + 36) = 1132462080;
			*(DWORD *)(a1 + 40) = 1107296256;
			break;
		case 1:
			*(DWORD *)(a1 + 36) = 1132462080;
			*(DWORD *)(a1 + 40) = 1115684864;
			break;
		case 2:
			*(DWORD *)(a1 + 36) = 1132462080;
			*(DWORD *)(a1 + 40) = 1124073472;
			break;
		case 3:
			*(DWORD *)(a1 + 36) = 1132462080;
			*(DWORD *)(a1 + 40) = 1132462080;
			break;
		case 4:
			*(DWORD *)(a1 + 36) = 1124073472;
			goto LABEL_37;
		case 5:
			*(DWORD *)(a1 + 36) = 1115684864;
			goto LABEL_37;
		case 6:
			*(DWORD *)(a1 + 36) = 1107296256;
LABEL_37:
			*(DWORD *)(a1 + 40) = 1132462080;
			break;
		default:
			break;
	}
	v5 = v1 >> (*(unsigned int *)(a1 + 28) - 1);
	if ( v5 > 8 )
	{
		switch ( v5 )
		{
			case 16:
				*(DWORD *)(a1 + 52) = 4;
				break;
			case 32:
				*(DWORD *)(a1 + 52) = 3;
				break;
			case 64:
				*(DWORD *)(a1 + 52) = 2;
				break;
			case 128:
				*(DWORD *)(a1 + 52) = 1;
				break;
			case 256:
				*(DWORD *)(a1 + 52) = 0;
				break;
			default:
				goto $L1462;
		}
	}
	else
	{
		switch ( v5 )
		{
			case 8:
				*(DWORD *)(a1 + 52) = 5;
				break;
			case 1:
				*(DWORD *)(a1 + 52) = 8;
				break;
			case 2:
				*(DWORD *)(a1 + 52) = 7;
				break;
			case 4:
				*(DWORD *)(a1 + 52) = 6;
				break;
			default:
$L1462:
				x_fatal("Illegal texture small size");
				break;
		}
	}
	switch ( *(DWORD *)(a1 + 16) & 0xFF )
	{
		case 0:
			v6 = (DWORD *)(a1 + 24);
			v8 = accesstexture((DWORD *)a1, *(DWORD *)(a1 + 28), 0, 0);
			*(DWORD *)(a1 + 20) = 0;
			*(DWORD *)(a1 + 64) = 11;
			*(DWORD *)(a1 + 24) = 2 * v8;
			break;
		case 3:
			v6 = (DWORD *)(a1 + 24);
			v9 = accesstexture((DWORD *)a1, *(DWORD *)(a1 + 28), 0, 0);
			*(DWORD *)(a1 + 20) = 3;
			*(DWORD *)(a1 + 64) = 10;
			*(DWORD *)(a1 + 24) = 2 * v9;
			break;
		case 4:
			v6 = (DWORD *)(a1 + 24);
			v10 = accesstexture((DWORD *)a1, *(DWORD *)(a1 + 28), 0, 0);
			*(DWORD *)(a1 + 20) = 4;
			*(DWORD *)(a1 + 64) = 3;
			*(DWORD *)(a1 + 24) = 2 * v10;
			break;
		default:
			v6 = (DWORD *)(a1 + 24);
			v7 = accesstexture((DWORD *)a1, *(DWORD *)(a1 + 28), 0, 0);
			*(DWORD *)(a1 + 20) = 1;
			*(DWORD *)(a1 + 64) = 12;
			*(DWORD *)(a1 + 24) = 2 * v7;
			break;
	}
	v11 = x_alloc(*v6);
	v12 = (DWORD *)(a1 + 72);
	*(DWORD *)(a1 + 68) = v11;
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
	*(DWORD *)(a1 + 104) = 0;
	*(DWORD *)(a1 + 108) = 1;
	*(DWORD *)(a1 + 112) = 0;
	*(DWORD *)(a1 + 116) = 1;
	return result;
}

unsigned int text_loadlevel(DWORD *a1, int a2, unsigned int a3)
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

	result = a1[16];
	switch ( result )
	{
		case 3u:
			v4 = accesstexture(a1, a2, &v20, &v19) + a1[17];
			result = v20 * v19;
			v5 = a3;
			v6 = a3 + 4 * v20 * v19;
			if ( v6 > a3 )
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
		case 0xAu:
			v7 = accesstexture(a1, a2, &v20, &v19);
			v8 = a3;
			result = a1[17] + 2 * v7;
			v9 = a3 + 4 * v20 * v19;
			if ( v9 > a3 )
			{
				do
				{
					v10 = *(BYTE *)(v8 + 1);
					v8 += 4;
					result += 2;
					*(WORD *)(result - 2) = ((char)(*(BYTE *)(v8 - 4) >> 3) << 11) | (*(BYTE *)(v8 - 2) >> 3) & 0x1F | 32 * (v10 >> 2);
				}
				while ( v9 > v8 );
			}
			break;
		case 0xBu:
			v11 = accesstexture(a1, a2, &v20, &v19);
			v12 = a3;
			v13 = a1[17] + 2 * v11;
			result = a3 + 4 * v20 * v19;
			if ( result > a3 )
			{
				do
				{
					v14 = *(BYTE *)(v12 + 3);
					v12 += 4;
					v13 += 2;
					*(WORD *)(v13 - 2) = (*(BYTE *)(v12 - 2) >> 3) & 0x1F | (((*(BYTE *)(v12 - 4) >> 3) & 0x1F) << 10) | 32 * ((*(BYTE *)(v12 - 3) >> 3) & 0x1F | ((char)(v14 >> 7) << 10));
				}
				while ( result > v12 );
			}
			break;
		case 0xCu:
			v15 = accesstexture(a1, a2, &v20, &v19);
			v16 = a3;
			v17 = a1[17] + 2 * v15;
			result = a3 + 4 * v20 * v19;
			if ( result > a3 )
			{
				do
				{
					v18 = *(BYTE *)(v16 + 3);
					v16 += 4;
					v17 += 2;
					*(WORD *)(v17 - 2) = (((*(BYTE *)(v16 - 4) >> 4) & 0xF) << 8) | (*(BYTE *)(v16 - 2) >> 4) & 0xF | 16 * ((*(BYTE *)(v16 - 3) >> 4) & 0xF | ((char)(v18 >> 4) << 8));
				}
				while ( result > v16 );
			}
			break;
		default:
			break;
	}
	a1[12] = 1;
	return result;
}

void text_freedata(DWORD *a1)
{
	freetexmem(a1);
	if ( a1[17] )
		x_free(a1[17]);
	memset(a1, 0, 0x98u);
}

int text_cleartexmem()
{
	return clearspace();
}

int text_opendata(int a1)
{
	return *(DWORD *)(a1 + 68);
}

void text_closedata(int a1)
{
	int result; // eax

	*(DWORD *)(a1 + 48) = 1;
	if ( *(DWORD *)(a1 + 4) == g_state[282] )
		g_state[282] = 0;
	result = g_state[283];
	if ( *(DWORD *)(a1 + 4) == result )
		g_state[282] = 0;
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
		v2 = (DWORD *)((char *)&g_texture + 152);
		do
		{
			if ( *v2 )
			{
				v3 = v2 + 34;
				v4 = 4;
				do
				{
					g_stats.text_resident += *(v3 - 16);
					result = g_state.frame - 1;
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
