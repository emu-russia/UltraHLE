#include "pch.h"

//.data:00000000 _debugcount$S1205 dd 0
//.data:00000004 _posarrayallocsize$S1222 dd 0           ; DATA XREF: _x_vxarray+24↓r
//.data:00000004                                         ; _x_vxarray+38↓w ...
//.data:00000008 _posarraysize$S1223 dd 0                ; DATA XREF: _x_vxa+A↓r
//.data:00000008                                         ; _x_vxarray+7B↓w ...
//.data:0000000C                 align 10h
//.data:00000010 _identmatrix$S1242 db    0              ; DATA XREF: _x_cameramatrix+A↓o
//.data:00000010                                         ; _x_matrix+E↓o
//.data:00000011                 db    0
//.data:00000012                 db  80h
//.data:00000013                 db  3Fh ; ?
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
//.data:0000001E                 db    0
//.data:0000001F                 db    0
//.data:00000020                 db    0
//.data:00000021                 db    0
//.data:00000022                 db    0
//.data:00000023                 db    0
//.data:00000024                 db    0
//.data:00000025                 db    0
//.data:00000026                 db  80h
//.data:00000027                 db  3Fh ; ?
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
//.data:00000032                 db    0
//.data:00000033                 db    0
//.data:00000034                 db    0
//.data:00000035                 db    0
//.data:00000036                 db    0
//.data:00000037                 db    0
//.data:00000038                 db    0
//.data:00000039                 db    0
//.data:0000003A                 db  80h
//.data:0000003B                 db  3Fh ; ?
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
//.data:00000046                 db    0
//.data:00000047                 db    0
//.data:00000048                 db    0
//.data:00000049                 db    0
//.data:0000004A                 db    0
//.data:0000004B                 db    0
//.data:0000004C                 db    0
//.data:0000004D                 db    0
//.data:0000004E                 db  80h
//.data:0000004F                 db  3Fh ; ?
//.data:0000005C $SG1260         dd offset loc_2076+5    ; DATA XREF: _dumpmatrix:loc_2BA↓o
//.data:00000068 $SG1265         dd offset loc_A78+5     ; DATA XREF: _dumpmatrix+4B↓o
//.data:00000190 $SG1494         dd offset loc_A22+1     ; DATA XREF: _setuprvx:loc_15FB↓o
//.data:000001AD                 align 10h
//.data:00000219 _data           ends


void geom_init()
{
}

BYTE * x_cameramatrix(BYTE *a1)
{
	BYTE *result; // eax
	bool v2; // zf
	BYTE *v3; // edi
	signed int v4; // ecx
	BYTE *v5; // esi

	result = a1;
	v2 = a1 == 0;
	if ( !a1 )
		goto LABEL_10;
	v3 = &identmatrix_S1242;
	v4 = 64;
	v5 = a1;
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
		g_state[233] = 0;
	}
	else
	{
		g_state[233] = 1;
		memcpy(&g_state[201], a1, 0x40u);
	}
	return result;
}


void x_getmatrix(void *a1)
{
	memcpy(a1, &g_state[169], 0x40u);
}

int dumpmatrix(float *a1, int a2)
{
	signed int v2; // esi
	float *v3; // ebx
	signed int v4; // edi
	long double v5; // fst7
	int result; // eax

	v2 = 4;
	x_log("%s matrix:\n", a2);
	v3 = a1;
	do
	{
		v4 = 4;
		x_log((const char *)&_SG1260);
		do
		{
			v5 = *v3;
			++v3;
			x_log("%13.5f ", (double)v5);
			--v4;
		}
		while ( v4 );
		result = x_log((const char *)&_SG1265);
		--v2;
	}
	while ( v2 );
	return result;
}

void x_matrix(void *a1)
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

	if ( a1 )
	{
		g_state[235] = 0;
	}
	else
	{
		a1 = &identmatrix_S1242;
		g_state[235] = 1;
	}
	if ( g_state[234] )
	{
		recalc_projection();
		g_state[234] = 0;
	}
	if ( g_state[236] && g_state[235] )
	{
		g_state[247] = 3;
	}
	else
	{
		g_state[247] = g_state[246];
		x_flush();
		memcpy(&g_state[169], a1, 0x40u);
		if ( g_state[246] != 1 && g_state[233] )
		{
			v1 = &v22;
			v2 = (float *)&g_state[201];
			do
			{
				v3 = (float *)a1;
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
			while ( v2 < (float *)&g_state[217] );
			v21 = (float *)&v22;
		}
		else
		{
			v21 = (float *)&g_state[169];
		}
		v11 = (float *)&g_state[185];
		v12 = (float *)&g_state[217];
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
		while ( v12 < (float *)&g_state[233] );
		if ( g_state[317] & 0x20 )
		{
			dumpmatrix(v21, "Modelview");
			dumpmatrix((float *)&g_state[217], "Projection");
			dumpmatrix((float *)&g_state[185], "Combined");
			x_log("Xformmode: %i Znear: %f Zfar: %f\n", g_state[246], (_QWORD)g_state[241], (_QWORD)g_state[242]);
		}
	}
}

int recalc_projection(int a1)
{
	long double v1; // fst7
	long double v2; // fst6
	signed __int64 v3; // rax
	signed __int64 v4; // rax
	signed __int64 v5; // rax
	float v6; // eax
	int result; // eax
	long double v8; // fst7
	long double v9; // fst7
	long double v10; // fst6
	long double v11; // fst5
	long double v12; // fst4

	v1 = (g_state[249] - g_state[248] + 1.0) * 0.5;
	g_state[252] = v1;
	v2 = (g_state[251] - g_state[250] + 1.0) * 0.5;
	g_state[254] = v2;
	g_state[253] = v1 + g_state[248] + 0.2;
	g_state[255] = v2 + g_state[250] + 0.2;
	v3 = (signed __int64)g_state[251];
	v4 = (signed __int64)g_state[249];
	v5 = (signed __int64)g_state[250];
	grClipWindow(a1, (unsigned __int64)(signed __int64)g_state[248] >> 32, (signed __int64)g_state[248]);
	v6 = g_state[246];
	if ( v6 == 0.0 )
	{
		memset(&g_state[217], 0, 0x40u);
		v8 = g_state[241] / g_state[238];
		g_state[227] = 1.0;
		g_state[232] = 1.0;
		g_state[217] = v8;
		g_state[222] = g_state[241] / g_state[240];
		result = projrecalced();
	}
	else
	{
		if ( LODWORD(v6) == 1 )
		{
			v9 = 2.0 / (g_state[238] - g_state[237]);
			v10 = 2.0 / (g_state[240] - g_state[239]);
			v11 = (g_state[237] + g_state[238]) * 0.5;
			v12 = (g_state[240] + g_state[239]) * 0.5;
			memset(&g_state[217], 0, 0x40u);
			g_state[227] = 1.0;
			g_state[217] = v9;
			g_state[222] = v10;
			g_state[228] = 0.0;
			g_state[232] = 0.0;
			g_state[220] = -(v11 * v9);
			g_state[224] = -(v10 * v12);
		}
		result = projrecalced();
	}
	return result;
}

int x_begin(int a1)
{
	int result; // eax

	if ( g_state[290] || g_state[234] )
		x_flush();
	mode_S1225 = a1;
	result = 0;
	vertices_base_S1217 = vertices_S1216;
	corners_base_S1221 = corners_S1220;
	state_S1226 = 0;
	flip_S1227 = 0;
	return result;
}

void x_end()
{
	int v0; // eax

	if ( mode_S1225 == 9 )
	{
		v0 = corners_S1220 - corners_base_S1221 - 1;
		if ( v0 >= 3 )
		{
			corner_S1219[corners_base_S1221] = v0;
		}
		else
		{
			corners_S1220 = corners_base_S1221;
			vertices_S1216 = vertices_base_S1217;
		}
	}
	mode_S1225 = 0;
	if ( vertices_S1216 > 127 )
		x_flush();
}

int vertexdata(float *a1)
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

	v35 = *(float *)&g_state[256];
	v36 = *(float *)&g_state[257];
	if ( !mode_S1225 )
		x_fatal("vertex without begin");
	if ( g_state[258] & 1 )
	{
		v1 = 15 * vertices_S1216;
		v2 = a1;
		flt_2A30[v1] = a1[1] * 256.0;
		flt_2A34[v1] = a1[2] * 256.0;
		flt_2A38[v1] = a1[3] * 256.0;
		flt_2A40[v1] = a1[4] * 256.0;
	}
	else
	{
		v2 = a1;
	}
	if ( g_state[258] & 2 )
	{
		if ( g_state[289] & 0x10 )
		{
			v3 = 2 * vertices_S1216;
			texp_S1211[v3] = *((_DWORD *)v2 + 7);
			tex_S1209[v3] = v2[5] * v2[7] * v35;
			v4 = v2[6] * v2[7];
		}
		else
		{
			v3 = 2 * vertices_S1216;
			tex_S1209[2 * vertices_S1216] = v2[5] * v35;
			v4 = v2[6];
		}
		flt_E230[v3] = v4 * v36;
	}
	if ( g_state[258] & 4 )
	{
		v5 = 2 * vertices_S1216;
		tex2_S1210[v5] = v2[8] * v35;
		flt_F440[v5] = v2[9] * v36;
	}
	result = mode_S1225 - 1;
	switch ( mode_S1225 )
	{
		case 1:
			v7 = vertices_S1216;
			result = 4 * corners_S1220;
			corners_S1220 += 2;
			*(int *)((char *)corner_S1219 + result) = 1;
			*(int *)((char *)&dword_A228 + result) = v7;
			break;
		case 2:
			if ( state_S1226 >= 2 )
			{
				v8 = corners_S1220++;
				corner_S1219[v8] = 3;
				v9 = corners_S1220++;
				corner_S1219[v9] = vertices_S1216 - 2;
				state_S1226 = 0;
				corner_S1219[corners_S1220++] = vertices_S1216 - 1;
				result = vertices_S1216;
				v10 = corners_S1220++;
				corner_S1219[v10] = vertices_S1216;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 3:
		case 8:
			if ( state_S1226 >= 2 )
			{
				v11 = corners_S1220++;
				corner_S1219[v11] = 3;
				if ( flip_S1227 )
				{
					v12 = corners_S1220++;
					v13 = vertices_S1216;
					corner_S1219[v12] = vertices_S1216 - 1;
					v14 = corners_S1220++;
					corner_S1219[v14] = vertices_S1216 - 2;
					result = corners_S1220;
					corner_S1219[corners_S1220] = v13;
				}
				else
				{
					v15 = corners_S1220++;
					corner_S1219[v15] = vertices_S1216 - 2;
					v16 = corners_S1220++;
					corner_S1219[v16] = vertices_S1216 - 1;
					result = vertices_S1216;
					corner_S1219[corners_S1220] = vertices_S1216;
				}
				flip_S1227 ^= 1u;
				++corners_S1220;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 4:
			if ( state_S1226 >= 2 )
			{
				v17 = corners_S1220;
				v18 = vertices_base_S1217;
				++corners_S1220;
				corner_S1219[v17] = 3;
				v19 = corners_S1220++;
				corner_S1219[v19] = v18;
				v20 = corners_S1220++;
				corner_S1219[v20] = vertices_S1216 - 1;
				result = vertices_S1216;
				v21 = corners_S1220++;
				corner_S1219[v21] = vertices_S1216;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 5:
			if ( state_S1226 >= 3 )
			{
				v22 = corners_S1220++;
				corner_S1219[v22] = 4;
				v23 = corners_S1220++;
				corner_S1219[v23] = vertices_S1216 - 3;
				v24 = corners_S1220++;
				corner_S1219[v24] = vertices_S1216 - 2;
				v25 = corners_S1220++;
				state_S1226 = 0;
				corner_S1219[v25] = vertices_S1216 - 1;
				result = vertices_S1216;
				v26 = corners_S1220++;
				corner_S1219[v26] = vertices_S1216;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 6:
			if ( state_S1226 >= 1 )
			{
				v27 = corners_S1220++;
				corner_S1219[v27] = 2;
				v28 = corners_S1220++;
				corner_S1219[v28] = vertices_S1216 - 1;
				result = vertices_S1216;
				v29 = corners_S1220++;
				corner_S1219[v29] = vertices_S1216;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 7:
			if ( state_S1226 >= 1 )
			{
				v30 = corners_S1220++;
				corner_S1219[v30] = 2;
				v31 = corners_S1220++;
				state_S1226 = 0;
				corner_S1219[v31] = vertices_S1216 - 1;
				result = vertices_S1216;
				v32 = corners_S1220++;
				corner_S1219[v32] = vertices_S1216;
			}
			else
			{
				++state_S1226;
			}
			break;
		case 9:
			if ( !state_S1226 )
			{
				v33 = corners_S1220;
				++state_S1226;
				++corners_S1220;
				corner_S1219[v33] = 1;
			}
			v34 = corners_S1220++;
			corner_S1219[v34] = vertices_S1216;
			result = corners_base_S1221 + 64;
			if ( corners_base_S1221 + 64 < corners_S1220 )
				x_fatal("xgeom: too large X_POLYGON!\n");
			break;
		default:
			break;
	}
	if ( ++vertices_S1216 >= 191 && (!state_S1226 || vertices_S1216 >= 256) )
		result = x_flush();
	return result;
}

void xform(int a1, float *a2, int a3, char *a4)
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

	switch ( g_state[247] )
	{
		case 0:
			v4 = a3;
			if ( g_state[289] & 8 )
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
				*(float *)v5 = *v6 * *(float *)&g_state[185]
										 + v6[1] * *(float *)&g_state[186]
										 + v6[2] * *(float *)&g_state[187]
										 + *(float *)&g_state[188];
				*(float *)(v5 + 4) = *v6 * *(float *)&g_state[189]
													 + v6[1] * *(float *)&g_state[190]
													 + v6[2] * *(float *)&g_state[191]
													 + *(float *)&g_state[192];
				v9 = *v6 * *(float *)&g_state[193]
					 + v6[1] * *(float *)&g_state[194]
					 + v6[2] * *(float *)&g_state[195]
					 + *(float *)&g_state[196];
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
						*(DWORD *)(v11 + 12) = *(_DWORD *)(v21 + v11 + 12);
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
						if ( *(float *)&g_state[241] > (long double)v55 )
							v15 |= 0x1000u;
						if ( *(float *)&g_state[242] < (long double)v55 )
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
				*(float *)v11 = *v12 * *(float *)&g_state[185]
											+ v12[1] * *(float *)&g_state[186]
											+ v12[2] * *(float *)&g_state[187]
											+ *(float *)&g_state[188];
				*(float *)(v11 + 4) = *v12 * *(float *)&g_state[189]
														+ v12[1] * *(float *)&g_state[190]
														+ v12[2] * *(float *)&g_state[191]
														+ *(float *)&g_state[192];
				v54 = *v12 * *(float *)&g_state[193]
						+ v12[1] * *(float *)&g_state[194]
						+ v12[2] * *(float *)&g_state[195]
						+ *(float *)&g_state[196];
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
				if ( *(float *)&g_state[241] > (long double)v54 )
					v15 |= 0x1000u;
				if ( *(float *)&g_state[242] < (long double)v54 )
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
						*(float *)v28 = *v29 * *(float *)&g_state[185]
													+ v29[1] * *(float *)&g_state[186]
													+ v29[2] * *(float *)&g_state[187]
													+ *(float *)&g_state[188];
						*(float *)(v28 + 4) = *v29 * *(float *)&g_state[189]
																+ v29[1] * *(float *)&g_state[190]
																+ v29[2] * *(float *)&g_state[191]
																+ *(float *)&g_state[192];
						*(float *)(v28 + 8) = *v29 * *(float *)&g_state[193]
																+ v29[1] * *(float *)&g_state[194]
																+ v29[2] * *(float *)&g_state[195]
																+ *(float *)&g_state[196];
						if ( v33 )
							*(DWORD *)(v28 + 12) = g_state[245];
						else
							*(float *)(v28 + 12) = 1.0 / *(float *)(v28 + 8);
						v34 = 0;
						if ( *(DWORD *)v28 > 0xBF800000 )
							v34 = 256;
						if ( *(DWORD *)v28 > 1065353216 )
							v34 |= 0x200u;
						if ( *(DWORD *)(v28 + 4) > 0xBF800000 )
							v34 |= 0x400u;
						if ( *(DWORD *)(v28 + 4) > 1065353216 )
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
					*(float *)v37 = *v38 * *(float *)&g_state[185]
												+ v38[1] * *(float *)&g_state[186]
												+ v38[2] * *(float *)&g_state[187]
												+ *(float *)&g_state[188];
					*(float *)(v37 + 4) = *v38 * *(float *)&g_state[189]
															+ v38[1] * *(float *)&g_state[190]
															+ v38[2] * *(float *)&g_state[191]
															+ *(float *)&g_state[192];
					v56 = *v38 * *(float *)&g_state[197]
							+ v38[1] * *(float *)&g_state[198]
							+ v38[2] * *(float *)&g_state[199]
							+ *(float *)&g_state[200];
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
					if ( *(float *)&g_state[241] > (long double)v56 )
						v41 |= 0x1000u;
					if ( *(float *)&g_state[242] < (long double)v56 )
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
						if ( *(float *)&g_state[241] > (long double)v57 )
							v48 |= 0x1000u;
						if ( *(float *)&g_state[242] < (long double)v57 )
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

int * setuprvx(int a1, int a2)
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

	v21 = *(float *)&g_state[252];
	v22 = *(float *)&g_state[254];
	v19 = *(float *)&g_state[253];
	v20 = *(float *)&g_state[255];
	v23 = *(float *)&g_state[241] * *(float *)&g_state[243];
	v2 = &xfpos_S1212[5 * a1];
	v3 = (float *)((char *)&grvx_S1213 + 60 * a1);
	v4 = &tex_S1209[2 * a1];
	v5 = &tex2_S1210[2 * a1];
	result = &texp_S1211[2 * a1];
	v24 = (float *)&texp_S1211[2 * a1];
	if ( g_state[246] == 1 )
	{
		v7 = a2;
		if ( a2 > 0 )
		{
			result = (int *)(&debugcount_S1205 + 2);
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
				if ( g_state[258] & 2 )
				{
					v3[9] = *v4 * v16;
					v3[10] = v4[1] * v16;
					*((DWORD *)v3 + 11) = v9;
				}
				if ( g_state[258] & 4 )
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
	if ( g_state[246] == 2 )
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
				if ( g_state[289] & 0x10 )
				{
					if ( g_state[258] & 2 )
					{
						v3[9] = *v4 * v17;
						v3[10] = v4[1] * v17;
						v3[11] = *v24 * v17;
					}
					if ( !(g_state[258] & 4) )
						goto LABEL_23;
				}
				else
				{
					if ( g_state[258] & 2 )
					{
						v3[9] = *v4 * v17;
						v3[10] = v4[1] * v17;
						*((DWORD *)v3 + 11) = result;
					}
					if ( !(g_state[258] & 4) )
						goto LABEL_23;
				}
				v3[12] = *v5 * v17;
				v3[13] = v5[1] * v17;
			}
LABEL_23:
			if ( g_state[317] & 0x20 )
			{
				x_log(
					"#x_vx[ %13.5f %13.5f %13.5f ]\n",
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20)],
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20) + 1],
					pos_S1208[3 * (((char *)v2 - (char *)xfpos_S1212) / 20) + 2]);
				if ( *((DWORD *)v2 + 4) )
					x_log("#clip[ %13.5f %13.5f %13.5f clip %08X ]\n", *v2, v2[1], v2[2], *((DWORD*)v2 + 4));
				else
					x_log("#clip[ %13.5f %13.5f %13.5f   w:%13.5f ]\n", *v2, v2[1], v2[2], (double)(1.0 / v2[3]));
				if ( !*((DWORD *)v2 + 4) )
					x_log("#scrn[ %13.5f %13.5f %13.5f oow:%13.5f ]\n", *v3, v3[1], v3[2], v3[8]);
				result = (int *)x_log((const char *)&_SG1494);
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
	if ( g_state[295] || g_state[289] & 0x10 )
	{
		v13 = a2;
		if ( a2 <= 0 )
			return result;
		result = (int *)(&debugcount_S1205 + 2);
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
				if ( g_state[289] & 0x10 )
				{
					if ( g_state[258] & 2 )
					{
						v3[9] = *v4 * v18;
						v3[10] = v4[1] * v18;
						v3[11] = *v24 * v18;
					}
					if ( !(g_state[258] & 4) )
						goto LABEL_55;
				}
				else
				{
					if ( g_state[258] & 2 )
					{
						v3[9] = *v4 * v18;
						v3[10] = v4[1] * v18;
						*((DWORD *)v3 + 11) = v15;
					}
					if ( !(g_state[258] & 4) )
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
				if ( g_state[258] & 2 )
				{
					v3[9] = *v4 * v3[8];
					v3[10] = v4[1] * v3[8];
				}
				if ( g_state[258] & 4 )
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

int x_vx(float *a1, float *a2)
{
	float *v2; // edx
	int v3; // edx

	vertices_lastnonrel_S1218 = vertices_S1216;
	v2 = &pos_S1208[3 * vertices_S1216];
	*v2 = *a1;
	v2[1] = a1[1];
	v2[2] = a1[2];
	v3 = vertices_S1216;
	allxformed_S1215 = 0;
	++g_stats[1];
	xformed_S1214[v3] = 0;
	return vertexdata(a2);
}

int x_vxa(int a1, float *a2)
{
	int v2; // ecx

	if ( a1 < 0 || a1 >= posarraysize_S1223 )
		x_fatal("invalid vertex for x_vxa");
	vertices_lastnonrel_S1218 = vertices_S1216;
	memcpy(&xfpos_S1212[5 * vertices_S1216], (const void *)(posarray_S1224 + 20 * a1), 0x14u);
	v2 = vertices_S1216;
	++g_stats[1];
	xformed_S1214[v2] = 1;
	return vertexdata(a2);
}

int x_vxrel(float *a1, float *a2)
{
	float *v2; // edx
	int v3; // eax
	int v4; // ecx

	v2 = &pos_S1208[3 * vertices_S1216];
	*v2 = *a1;
	v2[1] = a1[1];
	v3 = vertices_S1216;
	v4 = *((DWORD *)a1 + 2);
	allxformed_S1215 = 0;
	*((DWORD *)v2 + 2) = v4;
	xformed_S1214[v3] = vertices_S1216 - vertices_lastnonrel_S1218 + 16;
	return vertexdata(a2);
}

void x_vxarray(float *a1, int a2, char *a3)
{
	int v3; // eax

	if ( a1 && a2 )
	{
		x_begin(0);
		if ( posarrayallocsize_S1222 < a2 )
		{
			v3 = a2 + 256;
			posarrayallocsize_S1222 = a2 + 256;
			if ( posarray_S1224 )
			{
				x_free(posarray_S1224);
				v3 = posarrayallocsize_S1222;
			}
			posarrayallocsize_S1222 = v3;
			posarray_S1224 = x_allocfast(20 * v3);
			if ( !posarray_S1224 )
				x_fatal("out of memory");
		}
		posarraysize_S1223 = a2;
		x_fastfpu(1);
		xform(posarray_S1224, a1, a2, a3);
		x_fastfpu(0);
	}
	else
	{
		posarraysize_S1223 = 0;
	}
}

int clear()
{
	int result; // eax

	allxformed_S1215 = 1;
	result = 0;
	state_S1226 = 0;
	vertices_S1216 = 0;
	corners_S1220 = 0;
	return result;
}

int doclipvertex(signed int a1, int a2, int a3)
{
	int v3; // edx
	int v4; // esi
	int v5; // edi
	bool v6; // zf
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
				v22 = *v20 - *(float *)&g_state[241];
				v24 = *v20 - flt_11454[5 * a3];
			}
			else
			{
				if ( a1 != 0x2000 )
					goto LABEL_4;
				v3 = a2;
				v4 = a3;
				v21 = &flt_11454[5 * a2];
				v22 = *(float *)&g_state[242] - *v21;
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
	v6 = (g_state[258] & 1) == 0;
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
	if ( g_state[258] & 2 )
	{
		v8 = 2 * clipnewvx;
		v6 = (g_state[289] & 0x10) == 0;
		tex_S1209[v8] = (tex_S1209[2 * v4] - tex_S1209[2 * v3]) * v23 + tex_S1209[2 * v3];
		flt_E230[v8] = (flt_E230[2 * v4] - flt_E230[2 * v3]) * v23 + flt_E230[2 * v3];
		if ( !v6 )
			*(float *)&texp_S1211[v8] = (*(float *)&texp_S1211[2 * v4] - *(float *)&texp_S1211[2 * v3]) * v23
																+ *(float *)&texp_S1211[2 * v3];
	}
	if ( g_state[258] & 4 )
	{
		v9 = 2 * v4;
		v10 = 2 * clipnewvx;
		tex2_S1210[v10] = (tex2_S1210[v9] - tex2_S1210[2 * v3]) * v23 + tex2_S1210[2 * v3];
		flt_F440[v10] = (flt_F440[v9] - flt_F440[2 * v3]) * v23 + flt_F440[2 * v3];
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
	if ( *(float *)&g_state[241] > (long double)v25 )
		v12 |= 0x1000u;
	if ( *(float *)&g_state[242] < (long double)v25 )
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
	if ( vertices_S1216 < clipnewvx )
	{
		v2 = &dword_1145C[5 * vertices_S1216];
		v3 = clipnewvx - vertices_S1216;
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
	setuprvx(vertices_S1216, clipnewvx - vertices_S1216);
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

	v4 = (int *)&clipbuf1;
	clipor = a1;
	clipnewvx = vertices_S1216;
	v5 = a2;
	clipin = &clipbuf1;
	clipout = &clipbuf2;
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
	if ( *((_BYTE *)&clipor + 1) & 0x10 )
		doclip(4096);
	if ( *((_BYTE *)&clipor + 1) & 0x20 )
		doclip(0x2000);
	if ( *((_BYTE *)&clipor + 1) & 1 )
		doclip(256);
	if ( *((_BYTE *)&clipor + 1) & 2 )
		doclip(512);
	if ( *((_BYTE *)&clipor + 1) & 4 )
		doclip(1024);
	if ( *((_BYTE *)&clipor + 1) & 8 )
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

	clipnewvx = vertices_S1216;
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

void *flush_reordertables()
{
	int v0; // ecx
	signed int v1; // ebx
	signed int v2; // esi
	float v3; // ebp
	int *v4; // eax
	int v5; // edx
	int v6; // ebp
	void *result; // eax
	signed int v8; // ebx
	signed int v9; // [esp+10h] [ebp-4h]

	v0 = mode_S1225;
	if ( !mode_S1225 )
		return (void *)clear();
	switch ( mode_S1225 )
	{
		case 1:
			v1 = 0;
			break;
		case 2:
			v1 = 2;
			break;
		case 3:
			v1 = 2;
			break;
		case 4:
			v1 = 2;
			break;
		case 5:
			v1 = 3;
			break;
		case 6:
			v1 = 1;
			break;
		case 9:
			x_fatal("xgeom: too large X_POLYGON!\n");
			v1 = v9;
			v0 = mode_S1225;
			break;
		default:
			v1 = 0;
			break;
	}
	v2 = 0;
	if ( v0 == 4 )
	{
		memcpy(&grvx_S1213, (char *)&grvx_S1213 + 60 * vertices_base_S1217, 0x3Cu);
		memcpy(xfpos_S1212, &xfpos_S1212[5 * vertices_base_S1217], 0x14u);
		v3 = flt_E230[2 * vertices_base_S1217];
		tex_S1209[0] = tex_S1209[2 * vertices_base_S1217];
		v4 = &texp_S1211[2 * vertices_base_S1217];
		v2 = 1;
		flt_E230[0] = v3;
		v5 = *v4;
		v6 = v4[1];
		vertices_base_S1217 = 0;
		texp_S1211[0] = v5;
		dword_10448 = v6;
	}
	if ( v1 <= vertices_S1216 )
	{
		memmove(&xfpos_S1212[5 * v2], &xfpos_S1212[5 * (vertices_S1216 - v1)], 20 * v1);
		memmove((char *)&grvx_S1213 + 60 * v2, (char *)&grvx_S1213 + 60 * (vertices_S1216 - v1), 60 * v1);
		memmove(&tex_S1209[2 * v2], &tex_S1209[2 * (vertices_S1216 - v1)], 8 * v1);
		result = memmove(&texp_S1211[2 * v2], &texp_S1211[2 * (vertices_S1216 - v1)], 8 * v1);
	}
	else
	{
		x_fatal("internal error flushing vertex buffer");
	}
	v8 = v2 + v1;
	if ( v8 > 0 )
	{
		result = (void *)16843009;
		memset(xformed_S1214, 1u, v8);
	}
	vertices_S1216 = v8;
	allxformed_S1215 = 1;
	corners_S1220 = 0;
	return result;
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

	if ( !allxformed_S1215 )
		xform((int)xfpos_S1212, pos_S1208, vertices_S1216, xformed_S1214);
	v0 = &dword_A228;
	setuprvx(0, vertices_S1216);
	result = corners_S1220;
	corner_S1219[corners_S1220] = 0;
	for ( i = corner_S1219[0]; i; v0 = v15 + 1 )
	{
		if ( i < 3 )
			++g_stats[2];
		else
			g_stats[2] += i - 2;
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
			if ( !result && g_state[246] != 1 )
			{
				if ( i == 2 )
				{
					result = clipline(*v0, v0[1], &v16);
					if ( result >= 2 )
					{
						v9 = v16;
						++g_stats[3];
						result = grDrawLine((char *)&grvx_S1213 + 60 * *v16, (char *)&grvx_S1213 + 60 * v9[1]);
					}
				}
				else
				{
					result = clippoly(v5, i, v0, &v16);
					v10 = result;
					if ( result >= 3 )
					{
						result -= 2;
						g_stats[3] += v10 - 2;
						if ( g_state[289] & 1 )
						{
							v2 = v10 - 1;
							for ( j = 0; v10 > j; ++j )
							{
								result = grDrawLine((char *)&grvx_S1213 + 60 * v16[v2], (char *)&grvx_S1213 + 60 * v16[j]);
								v2 = j;
							}
						}
						else
						{
							result = splitpoly(v10, (int)v16, &splitbuf);
							if ( result > 0 )
							{
								v12 = (int *)((char *)&splitbuf + 8);
								v1 = (result + 2) % 3u;
								v13 = (result + 2) / 3u;
								do
								{
									v14 = *v12;
									v12 += 3;
									result = grDrawTriangle(
														 12 * *(v12 - 5),
														 v1,
														 (char *)&grvx_S1213 + 60 * *(v12 - 5),
														 (char *)&grvx_S1213 + 60 * *(v12 - 4),
														 (char *)&grvx_S1213 + 60 * v14);
									--v13;
								}
								while ( v13 );
							}
						}
					}
				}
			}
		}
		else if ( g_state[289] & 1 && i > 2 )
		{
			v2 = i - 1;
			result = i - 2;
			v8 = 0;
			g_stats[3] += i - 2;
			if ( i > 0 )
			{
				do
				{
					result = grDrawLine((char *)&grvx_S1213 + 60 * v0[v2], (char *)&grvx_S1213 + 60 * v0[v8]);
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
					++g_stats[3];
					result = grDrawPoint(12 * *v0, v1, (char *)&grvx_S1213 + 60 * *v0);
					break;
				case 2:
					++g_stats[3];
					result = grDrawLine((char *)&grvx_S1213 + 60 * *v0, (char *)&grvx_S1213 + 60 * v0[1]);
					break;
				case 3:
					++g_stats[3];
					result = grDrawTriangle(
										 12 * *v0,
										 v1,
										 (char *)&grvx_S1213 + 60 * *v0,
										 (char *)&grvx_S1213 + 60 * v0[1],
										 (char *)&grvx_S1213 + 60 * v0[2]);
					break;
				default:
					g_stats[3] += i - 2;
					result = grDrawPlanarPolygon(v2, v1, i, v0, &grvx_S1213);
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
	int v0; // eax

	v0 = corners_S1220;
	if ( g_state[317] & 0x20 )
	{
		x_log("#flush %i %i %i\n", g_state[234], g_state[290], corners_S1220);
		v0 = corners_S1220;
	}
	if ( v0 )
	{
		x_fastfpu(1);
		flush_drawfx();
		flush_reordertables();
		x_fastfpu(0);
	}
	if ( g_state[290] )
		mode_change();
	if ( g_state[234] )
	{
		g_state[235] = 1;
		x_matrix(0);
	}
}


//.rdata:000029DC _rdata          segment para public 'DATA' use32
//.rdata:000029DC                 assume cs:_rdata
//.rdata:000029DC                 ;org 29DCh
//.rdata:000029DC $T1773          db    0
//.rdata:000029DD                 db    0
//.rdata:000029DE                 db    0
//.rdata:000029DF                 db    0
//.rdata:000029E0 $T1775          dd 1.0                  ; DATA XREF: _recalc_projection+D↑r
//.rdata:000029E0                                         ; _recalc_projection+2B↑r ...
//.rdata:000029E4 $T1776          dd 0.5                  ; DATA XREF: _recalc_projection+13↑r
//.rdata:000029E4                                         ; _recalc_projection+31↑r
//.rdata:000029E8 $T1777          dd 0.2                  ; DATA XREF: _recalc_projection+45↑r
//.rdata:000029E8                                         ; _recalc_projection+57↑r
//.rdata:000029EC $T1779          dq 2.0                  ; DATA XREF: _recalc_projection+101↑r
//.rdata:000029EC                                         ; _recalc_projection+113↑r
//.rdata:000029F4 $T1780          dq 0.5                  ; DATA XREF: _recalc_projection+131↑r
//.rdata:000029F4                                         ; _recalc_projection+143↑r
//.rdata:000029FC $T1783          dd 256.0                ; DATA XREF: _vertexdata+4A↑r
//.rdata:000029FC                                         ; _vertexdata+59↑r ...
//.rdata:00002A00                 db    0
//.rdata:00002A01                 db    0
//.rdata:00002A02                 db    0
//.rdata:00002A03                 db    0
//.rdata:00002A04 $T1802          dq 0.0                  ; DATA XREF: _xform+3C9↑r
//.rdata:00002A0C $T1803          db    0
//.rdata:00002A0D                 db    0
//.rdata:00002A0E                 db  80h
//.rdata:00002A0F                 db 0BFh
//.rdata:00002A10 $T1816          dd 786432.0             ; DATA XREF: _setuprvx+90↑r
//.rdata:00002A10                                         ; _setuprvx+193↑r ...
//.rdata:00002A14 $T1817          db    0
//.rdata:00002A15                 db    0
//.rdata:00002A16                 db    0
//.rdata:00002A17                 db    0
//.rdata:00002A18                 db    0
//.rdata:00002A19                 db    0
//.rdata:00002A1A                 db 0F0h
//.rdata:00002A1B                 db  3Fh ; ?
//.rdata:00002A1B _rdata          ends





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
//.bss:0000CA24 _allxformed$S1215 dd ?                  ; DATA XREF: _x_vx+35↑w
//.bss:0000CA24                                         ; _x_vxrel+26↑w ...
//.bss:0000CA28 _state$S1226    dd ?                    ; DATA XREF: _x_begin+3A↑w
//.bss:0000CA28                                         ; _vertexdata:$L1335↑r ...
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
//.bss:0000F22C _mode$S1225     dd ?                    ; DATA XREF: _x_begin+27↑w
//.bss:0000F22C                                         ; _x_end↑r ...
//.bss:0000F230                 db    ? ;
//.bss:0000F231                 db    ? ;
//.bss:0000F232                 db    ? ;
//.bss:0000F233                 db    ? ;
//.bss:0000F234 _xformed$S1214  db ?                    ; DATA XREF: _x_vx+41↑w
//.bss:0000F234                                         ; _x_vxa+57↑w ...
//.bss:0000F235                 db    ? ;
//.bss:0000F236                 db    ? ;
//...
//.bss:0000F432                 db    ? ;
//.bss:0000F433                 db    ? ;
//.bss:0000F434 _vertices_base$S1217 dd ?               ; DATA XREF: _x_begin+2E↑w
//.bss:0000F434                                         ; _x_end+1C↑r ...
//.bss:0000F438 _vertices_lastnonrel$S1218 dd ?         ; DATA XREF: _x_vx+9↑w
//.bss:0000F438                                         ; _x_vxa+2C↑w ...
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
//.bss:0001043C _vertices$S1216 dd ?                    ; DATA XREF: _x_begin+1B↑r
//.bss:0001043C                                         ; _x_end+27↑w ...
//.bss:00010440                 db    ? ;
//.bss:00010441                 db    ? ;
//.bss:00010442                 db    ? ;
//.bss:00010443                 db    ? ;
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
//.bss:00011444 _corners_base$S1221 dd ?                ; DATA XREF: _x_begin+34↑w
//.bss:00011444                                         ; _x_end+E↑r ...
//.bss:00011448                 db    ? ;
//.bss:00011449                 db    ? ;
//.bss:0001144A                 db    ? ;
//.bss:0001144B                 db    ? ;
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
//.bss:00013C50 _flip$S1227     dd ?                    ; DATA XREF: _x_begin+3F↑w
//.bss:00013C50                                         ; _vertexdata+1FB↑r ...
//.bss:00013C50 _bss            ends
