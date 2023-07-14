#include "pch.h"

static float identmatrix[4 * 4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

int g_activestateindex;
xt_state* g_activestate;

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

int x_open(void* hdc, void* hwnd, int width, int height, int buffers, int vsync)
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

	g_state.hdc = hdc;
	g_state.hwnd = hwnd;
	g_state.xs = width;
	g_state.ys = height;
	g_state.buffers = buffers;
	g_state.vsync = vsync;
	v9 = init_init();
	mode_init();
	geom_init();
	g_state.projchanged = 0;
	g_state.changed = 0;
	x_projection(90.0, 0.9f, 65535.0);
	result = -1;
	if ( !v9 )
		result = v7;
	return result;
}

void x_resize(int width, int height)
{
	init_resize(width, height);
}

void x_select(int which)
{
	if (which > 0 )
	{
		g_activestateindex = which;
		g_activestate = &g_state[159 * which];
		init_activate();
	}
	else
	{
		g_activestateindex = -1;
		g_activestate = 0;
	}
}

void x_close(int which)
{
	DWORD *result; // eax

	result = (DWORD *)which;
	if (which < 2 )
	{
		result = &g_state[159 * which];
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

void x_getstats(xt_stats* s, int ssize)
{
	static int nowtime = 0;
	static int lasttime = 0;

	bool v2; // zf

	nowtime = x_timeus();
	if ( s )
	{
		if ( ssize == sizeof(xt_stats))
		{
			memcpy(s, &g_stats, sizeof(xt_stats));
			v2 = nowtime == lasttime;
			s->frametime = nowtime - lasttime;
			if ( v2 )
				s->frametime = 1;
		}
		else
		{
			memset(s, 0, ssize);
		}
	}
	lasttime = nowtime;
	g_stats.in_vx = 0;
	g_stats.in_tri = 0;
	g_stats.out_tri = 0;
	g_stats.chg_mode = 0;
	g_stats.chg_text = 0;
	g_stats.text_uploaded = 0;
}

int x_query(void* hdc, void* hwnd)
{
	// It was most likely intended to use `init_query`
	return 0;
}

void x_clear(int writecolor, int writedepth, float cr, float cg, float cb)
{
	init_clear(writecolor, writedepth, cr, cg, cb);
}

int x_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	if ( fb == X_FB_RGB565)
	{
		if ( 2 * xs > bufrowlen)
			return 1;
	}
	else
	{
		if ( fb != X_FB_RGBA8888)
			return 1;
		if ( 4 * xs > bufrowlen)
			return 1;
	}
	return init_readfb(fb, x, y, xs, ys, buffer, bufrowlen);
}

int x_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	if (fb == X_FB_RGB565)
	{
		if ( 2 * xs > bufrowlen)
			return 1;
	}
	else
	{
		if (fb != X_FB_RGBA8888)
			return 1;
		if ( 4 * xs > bufrowlen)
			return 1;
	}
	return init_writefb(fb, x, y, xs, ys, buffer, bufrowlen);
}

void x_finish(void)
{
	x_flush();
	init_bufferswap();
	text_frameend();
	x_reset();
	++g_state.frame;
}

void x_frustum(float xmin, float xmax, float ymin, float ymax, float znear, float zfar)
{
	x_flush();
	g_state.xmin = xmin;
	g_state.xmax = xmax;
	g_state.ymin = ymin;
	g_state.ymax = ymax;
	g_state.znear = znear;
	g_state.zfar = zfar;
	g_state.xformmode = 0;
	g_state.projchanged = 1;
	g_state.invzfar = 1.0 / zfar;
	g_state.invznear = 1.0 / znear;
}

void x_projmatrix(xt_matrix* a1)
{
	const void *v2; // esi

	x_flush();
	v2 = a1;
	if ( a1 )
	{
		g_state.projnull = 0;
	}
	else
	{
		g_state.projnull = 1;
		v2 = &identmatrix;
	}
	memcpy(g_state.projxform, v2, 0x40u);
	g_state.xformmode = 2;
	g_state.projchanged = 1;
}

void projrecalced()
{
}

void x_ortho(float xmin, float ymin, float xmax, float ymax, float znear, float zfar)
{
	x_flush();
	g_state.xmin = xmin;
	g_state.ymin = ymin;
	g_state.xmax = xmax;
	g_state.ymax = ymax;
	g_state.znear = znear;
	g_state.zfar = zfar;
	g_state.xformmode = 1;
	g_state.projchanged = 1;
	g_state.invzfar = 1.0f / zfar;
	g_state.invznear = 1.0f / znear;
}

void x_viewport(float x0, float y0, float x1, float y1)
{
	int v5; // ST00_4
	double v6; // st7

	x_flush();
	g_state.view_x0 = x0;
	v5 = g_state.ys - 1;
	g_state.view_x1 = x1;
	v6 = (double)v5;
	g_state.projchanged = 1;
	g_state.view_y0 = v6 - y1;
	g_state.view_y1 = v6 - y0;
}

void x_projection(float fov, float znear, float zfar)
{
	double v6; // st7

	x_flush();
	if ( fov >= 1.0 )
	{
		v6 = tan(fov * (PI/180.0f) * 0.5f) * zfar;
		x_frustum(-v6, v6, -(0.75f * v6), 0.75f * v6, znear, zfar);
	}
	else
	{
		x_ortho(0, (float)g_state.xs, (float)g_state.ys, 0, znear, zfar);
	}
}

int x_zrange(float znear, float zfar)
{
	x_flush();
	g_state.changed = 1;
	g_state.projchanged = 1;
	g_state.znear = znear;
	g_state.zfar = zfar;
	return 0;
}

int x_zdecal(float factor)
{
	x_flush();
	g_state.zdecal = factor;
	return 0;
}

xt_texture* texture_get(int t)
{
	xt_texture* txt;

	// TODO: There seems to be an error here, 1024 != X_MAX_TEXTURES
	if ( t <= 0 || t > 1024 )
	{
		g_state.error |= X_ERROR_GET_TEXTURE;
		txt = 0;
	}
	else
	{
		txt = &g_texture[t];
		if (txt->state == 0)
		{
			x_log("undefined texture xhandle %i selected\n");
			txt = 0;
			g_state.error |= X_ERROR_GET_TEXTURE;
		}
	}
	return txt;
}

int x_createtexture(int format, int width, int height)
{
	xt_texture *v3; // eax
	signed int v4; // esi
	signed int result; // eax
	DWORD *v6; // ebx
	signed int v7; // edx
	signed int v8; // ecx
	int v9; // eax

	// Find the first free texture

	v3 = &g_texture[1];
	v4 = 1;
	do
	{
		if ( v3->state == 0)
			break;
		v3++;		// next texture
		++v4;
	}
	while ( v3 < &g_texture[X_MAX_TEXTURES] );

	if ( v4 == 1024 )
	{
		x_log("too many textures\n");
		result = -1;
		g_state.error |= X_ERROR_TOO_MANY_TEXTURES;
	}
	else
	{
		if ( g_lasttexture < v4 )
			g_lasttexture = v4;
		g_texture[v4].state = g_activestateindex;
		v6 = texture_get(v4);
		if ( v6 )
		{
			memset(v6, 0, sizeof(xt_texture));
			v7 = width;
			v8 = height;
			v6[1] = v4;
			v6[0] = g_activestateindex;
			v6[2] = width;
			v6[3] = height;
			v6[4] = format;
			if (format & X_MIPMAP)
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
			g_stats.text_total += v6[6];
			result = v4;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}

int x_gettextureinfo(int handle, int* format, int* memformat, int* width, int* height)
{
	xt_texture* tex;

	tex = texture_get(handle);
	if ( !tex)
		return 1;
	if (format)
		*format = tex->format;
	if (memformat)
		*memformat = tex->memformat;
	if (width)
		*width = tex->width;
	if (height)
		*height = tex->height;
	return 0;
}

int x_loadtexturelevel(int handle, int level, char* data)
{
	DWORD *v3; // eax
	DWORD *v4; // esi
	int v6; // edx

	v3 = texture_get(handle);
	v4 = v3;
	if ( !v3 )
		return 0;
	if ( v3[7] < level + 1 )
		v3[7] = level + 1;
	if (level > 31 )
		return 0;
	v6 = v3[8];
	if ( !(v6 & (1 << level)) )
		v3[8] = (1 << level) | v6;
	text_loadlevel(v3, level, data);
	return 2 - ((v4[4] & 0x200u) < 1);
}

void x_freetexture(int handle)
{
	DWORD *result; // eax

	result = texture_get(handle);
	if ( result )
		text_freedata(result);
}

int x_texture_getinfo(int handle, int* format, int* memformat, int* width, int* height)
{
	xt_texture* tex;

	tex = texture_get(handle);
	if (!tex)
		return 1;
	if (format)
		*format = tex->format;
	if (memformat)
		*memformat = tex->memformat;
	if (width)
		*width = tex->width;
	if (height)
		*height = tex->height;
	return 0;
}

void x_cleartexmem(void)
{
	text_cleartexmem();
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
			text_closedata(result);
	}
}

void x_forcegeometry(int forceon, int forceoff)
{
	g_state.geometryon = forceon;
	g_state.geometryoff = forceoff;
	g_state.changed |= 1u;
}

void x_geometry(int flags)
{
	g_state.geometry = (flags | g_state.geometryon) & ~g_state.geometryoff;
	g_state.changed |= 1u;
}

int x_mask(int colormask, int depthmask, int depthtest)
{
	signed int v3; // ecx

	v3 = 0;
	if (colormask == X_ENABLE)
	{
		v3 = 1;
	}
	else if (colormask != X_DISABLE)
	{
		return 1;
	}
	if (depthmask == X_ENABLE)
	{
		v3 |= 2u;
	}
	else if (depthmask != X_DISABLE)
	{
		return 1;
	}
	if ( !depthtest || depthtest == 1 )
		return 1;
	g_state.currentmode.masktst = depthtest;
	g_state.currentmode.mask = v3;
	g_state.changed |= 4u;
	return 0;
}

int x_dither(int type)
{
	if (type == X_ENABLE)
	{
		g_state.currentmode.dither = 1;
LABEL_5:
		g_state.changed |= 4u;
		return 0;
	}
	if (type == X_DISABLE)
	{
		g_state.currentmode.dither = 0;
		goto LABEL_5;
	}
	return 1;
}

int x_blend(int src, int dst)
{
	if (src < X_FIRSTBLEND || src > X_LASTBLEND)
		return 1;
	if (dst < X_FIRSTBLEND || dst > X_LASTBLEND)
		return 1;
	g_state.currentmode.src = src;
	g_state.currentmode.dst = dst;
	g_state.changed |= 4u;
	return 0;
}

int x_alphatest(float limit)
{
	if ( g_state.geometry & 1 )
		limit = 1.0f;
	if ( limit < 0.0f || limit > 1.0f)
		return 1;
	g_state.currentmode.alphatest = limit;
	g_state.changed |= 4u;
	return 0;
}

int x_combine(int colortext1)
{
	if (colortext1 < X_FIRSTCOMBINE || colortext1 > X_LASTCOMBINE)
		return 1;
	g_state.currentmode.text1text2 = 0;
	g_state.currentmode.colortext1 = colortext1;
	g_state.changed |= 4u;
	return 0;
}

int x_procombine(int rgb, int alpha)
{
	g_state.currentmode.text1text2 = 0;
	g_state.currentmode.colortext1 = rgb | (alpha << 16);
	g_state.changed |= 4u;
	return 0;
}

int x_envcolor(float r, float g, float b, float a)
{
	g_state.currentmode.env[0] = r;
	g_state.currentmode.env[1] = g;
	g_state.currentmode.env[2] = b;
	g_state.currentmode.env[3] = a;
	// Buggy (decompile bug?)
	g_state.currentmode.envc = 
		(unsigned __int8)(signed __int64)(r * 255.0) | 
		((unsigned __int8)(signed __int64)(b * 255.0) << 16) | 
		((((unsigned int)(signed __int64)(a * 255.0) << 16) | 
		(unsigned __int8)(signed __int64)(g * 255.0)) << 8);
	g_state.changed |= 4u;
	return 0;
}

int x_combine2(int colortext1, int text1text2, int sametex)
{
	if ( g_state.tmus < 2 )
		return 1;
	if (colortext1 < X_FIRSTCOMBINE || colortext1 > X_LASTCOMBINE)
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state.currentmode.colortext1 = colortext1;
	g_state.currentmode.sametex = sametex;
	g_state.currentmode.text1text2 = text1text2;
	g_state.changed |= 4u;
	return 0;
}

int x_procombine2(int rgb, int alpha, int text1text2, int sametex)
{
	if ( g_state.tmus < 2 )
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state.currentmode.sametex = sametex;
	g_state.currentmode.colortext1 = rgb | (alpha << 16);
	g_state.currentmode.text1text2 = text1text2;
	g_state.changed |= 4u;
	return 0;
}

int x_texture(int text1handle)
{
	if (text1handle <= 0 )
		return 1;
	g_state.currentmode.text1 = text1handle;
	g_state.currentmode.text2 = 0;
	g_state.changed |= 2u;
	return 0;
}

int x_texture2(int text1handle, int text2handle)
{
	if ( g_state.tmus < 2 )
		return 1;
	if (text1handle <= 0 || text2handle <= 0 )
		return 1;
	g_state.currentmode.text1 = text1handle;
	g_state.currentmode.text2 = text2handle;
	g_state.changed |= 2u;
	return 0;
}

void x_reset(void)
{
	x_geometry(X_CULLBACK);
	x_mask(X_ENABLE, X_ENABLE, X_ENABLE);
	// TODO: Probably meant X_ENABLE?
	x_dither(1);
	x_blend(X_ONE, X_ZERO);
	x_alphatest(1.0f);
	x_combine(X_COLOR);
	x_zdecal(1.0f);
}

int x_fog(int type, float min, float max, float r, float g, float b)
{
	g_state.currentmode.fogtype = type;
	g_state.currentmode.fogmin = min;
	g_state.currentmode.fogmax = max;
	g_state.currentmode.fogcolor[0] = r;
	g_state.currentmode.fogcolor[1] = g;
	g_state.currentmode.fogcolor[2] = b;
	g_state.currentmode.fogcolor[3] = 1.0f;
	g_state.changed |= 8u;
	return 0;
}

void x_fullscreen(int fullscreen)
{
	init_fullscreen(fullscreen);
}
