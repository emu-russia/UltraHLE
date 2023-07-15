#include "pch.h"

static float identmatrix[4 * 4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

int g_activestateindex;
xt_state* g_activestate;

xt_state g_state[X_MAXSTATES];		// The 0th entry is not used
xt_stats g_stats;

void x_init(void)
{
	log_open("wt");
	x_log("Init: %s\n", x_version());
}

void x_deinit(void)
{
	signed int v0; // edi
	xt_state *v1; // esi

	v0 = 1;
	v1 = &g_state[1];
	do
	{
		if ( v1->used )
			x_close(v0);
		v1++;
		++v0;
	}
	while ( v1 < &g_state[X_MAXSTATES] );
	x_log("Deinit: %s\n", x_version());
	log_open(0);
}

char *x_version()
{
	static char version[64];
	sprintf(version, "%s Build %i (%s)", init_name(), g_version, g_datetime);
	return version;
}

int x_open(void* hdc, void* hwnd, int width, int height, int buffers, int vsync)
{
	xt_state *v6; // eax
	signed int v7; // esi
	signed int result; // eax
	int v9; // edi

	v6 = &g_state[1];
	v7 = 1;
	do
	{
		if ( v6->used == 0 )
			break;
		v6++;
		++v7;
	}
	while ( v6 < &g_state[X_MAXSTATES] );
	if ( v7 >= X_MAXSTATES)
		return -1;
	memset(&g_state[v7], 0, sizeof(xt_state));
	g_state[v7].used = 1;
	g_activestateindex = v7;
	g_activestate = &g_state[v7];

	g_state[XST].hdc = hdc;
	g_state[XST].hwnd = hwnd;
	g_state[XST].xs = width;
	g_state[XST].ys = height;
	g_state[XST].buffers = buffers;
	g_state[XST].vsync = vsync;
	v9 = init_init();
	mode_init();
	geom_init();
	g_state[XST].projchanged = 0;
	g_state[XST].changed = 0;
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
		g_activestate = &g_state[which];
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
	xt_state *result; // eax

	if (which < X_MAXSTATES)
	{
		result = &g_state[which];
		if ( result->used )
		{
			result->used = 0;
			init_deinit();
			text_deinit();
			memset(&g_state[g_activestateindex], 0, sizeof(xt_state));
			x_select(0);
		}
	}
}

void x_getstats(xt_stats* s, int ssize)
{
	static int nowtime = 0;
	static int lasttime = 0;

	nowtime = x_timeus();
	if ( s )
	{
		if ( ssize == sizeof(xt_stats))
		{
			memcpy(s, &g_stats, sizeof(xt_stats));
			s->frametime = nowtime - lasttime;
			if (nowtime == lasttime)
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
	if ((uint8_t)fb == X_FB_RGB565)
	{
		if ( 2 * xs > bufrowlen)
			return 1;
	}
	else
	{
		if ((uint8_t)fb != X_FB_RGBA8888)
			return 1;
		if ( 4 * xs > bufrowlen)
			return 1;
	}
	return init_readfb(fb, x, y, xs, ys, buffer, bufrowlen);
}

int x_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	if ((uint8_t)fb == X_FB_RGB565)
	{
		if ( 2 * xs > bufrowlen)
			return 1;
	}
	else
	{
		if ((uint8_t)fb != X_FB_RGBA8888)
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
	++g_state[XST].frame;
}

void x_frustum(float xmin, float xmax, float ymin, float ymax, float znear, float zfar)
{
	x_flush();
	g_state[XST].xmin = xmin;
	g_state[XST].xmax = xmax;
	g_state[XST].ymin = ymin;
	g_state[XST].ymax = ymax;
	g_state[XST].znear = znear;
	g_state[XST].zfar = zfar;
	g_state[XST].xformmode = XFORM_MODE_FRUSTUM;
	g_state[XST].projchanged = 1;
	g_state[XST].invzfar = 1.0f / zfar;
	g_state[XST].invznear = 1.0f / znear;
}

void x_projmatrix(xt_matrix* m)
{
	float *v2;

	x_flush();
	// Not a very proper type-casting, but it'll do.
	v2 = (float *)m;
	if ( m )
	{
		g_state[XST].projnull = 0;
	}
	else
	{
		g_state[XST].projnull = 1;
		v2 = identmatrix;
	}
	memcpy(g_state[XST].projxform, v2, 0x40u);
	g_state[XST].xformmode = XFORM_MODE_PROJECT;
	g_state[XST].projchanged = 1;
}

void projrecalced()
{
	// Not used. Called from recalc_projection to notify that projection matrices are recalculated.
}

void x_ortho(float xmin, float ymin, float xmax, float ymax, float znear, float zfar)
{
	x_flush();
	g_state[XST].xmin = xmin;
	g_state[XST].ymin = ymin;
	g_state[XST].xmax = xmax;
	g_state[XST].ymax = ymax;
	g_state[XST].znear = znear;
	g_state[XST].zfar = zfar;
	g_state[XST].xformmode = XFORM_MODE_ORTHO;
	g_state[XST].projchanged = 1;
	g_state[XST].invzfar = 1.0f / zfar;
	g_state[XST].invznear = 1.0f / znear;
}

void x_viewport(float x0, float y0, float x1, float y1)
{
	int v5; // ST00_4
	double v6; // st7

	x_flush();
	g_state[XST].view_x0 = x0;
	v5 = g_state[XST].ys - 1;
	g_state[XST].view_x1 = x1;
	v6 = (double)v5;
	g_state[XST].projchanged = 1;
	g_state[XST].view_y0 = v6 - y1;
	g_state[XST].view_y1 = v6 - y0;
}

void x_projection(float fov, float znear, float zfar)
{
	double v6; // st7

	x_flush();
	if ( fov >= 1.0f )
	{
		v6 = tan(fov * (PI/180.0f) * 0.5f) * zfar;
		x_frustum(-v6, v6, -(0.75f * v6), 0.75f * v6, znear, zfar);
	}
	else
	{
		x_ortho(0, (float)g_state[XST].xs, (float)g_state[XST].ys, 0.0f, znear, zfar);
	}
}

int x_zrange(float znear, float zfar)
{
	x_flush();
	g_state[XST].changed = 1;
	g_state[XST].projchanged = 1;
	g_state[XST].znear = znear;
	g_state[XST].zfar = zfar;
	return 0;
}

int x_zdecal(float factor)
{
	x_flush();
	g_state[XST].zdecal = factor;
	return 0;
}

xt_texture* texture_get(int t)
{
	xt_texture* txt;

	if ( t <= 0 || t > X_MAX_TEXTURES)
	{
		g_state[XST].error |= X_ERROR_GET_TEXTURE;
		txt = 0;
	}
	else
	{
		txt = &g_texture[t];
		if (txt->state == 0)
		{
			x_log("undefined texture xhandle %i selected\n");
			txt = 0;
			g_state[XST].error |= X_ERROR_GET_TEXTURE;
		}
	}
	return txt;
}

int x_createtexture(int format, int width, int height)
{
	xt_texture *v3; // eax
	signed int v4; // esi
	signed int result; // eax
	xt_texture *v6; // ebx
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

	if ( v4 == X_MAX_TEXTURES)
	{
		x_log("too many textures\n");
		result = -1;
		g_state[XST].error |= X_ERROR_TOO_MANY_TEXTURES;
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
			v6->handle = v4;
			v6->state = g_activestateindex;
			v6->width = width;
			v6->height = height;
			v6->format = format;
			if (format & X_MIPMAP)
			{
				v9 = 0;
				while ( v7 > 1 || v8 > 1 )
				{
					v7 >>= 1;
					++v9;
					v8 >>= 1;
				}
				v6->levels = v9;
			}
			else
			{
				v6->levels = 1;
			}
			text_allocdata(v6);
			g_stats.text_total += v6->bytes;
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
	xt_texture* v3; // eax
	xt_texture* v4; // esi
	int v6; // edx

	v3 = texture_get(handle);
	v4 = v3;
	if ( !v3 )
		return 0;
	if ( v3->levels < level + 1 )
		v3->levels = level + 1;
	if (level > 31 )
		return 0;
	v6 = v3->levelsloaded;
	if ( !(v6 & (1 << level)) )
		v3->levelsloaded = (1 << level) | v6;
	text_loadlevel(v3, level, data);
	return 2 - ((v4->format & X_MIPMAP) < 1);
}

void x_freetexture(int handle)
{
	xt_texture* result; // eax

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

uchar* x_opentexturedata(int handle)
{
	xt_texture *v1; // eax
	int result; // eax

	v1 = texture_get(handle);
	if ( v1 && (v1->format & X_DYNAMIC))
		result = text_opendata(v1);
	else
		result = 0;
	return result;
}

void x_closetexturedata(int handle)
{
	xt_texture *result; // eax

	result = texture_get(handle);
	if ( result )
	{
		if ( result->format & X_DYNAMIC )
			text_closedata(result);
	}
}

void x_forcegeometry(int forceon, int forceoff)
{
	g_state[XST].geometryon = forceon;
	g_state[XST].geometryoff = forceoff;
	g_state[XST].changed |= 1u;
}

void x_geometry(int flags)
{
	g_state[XST].geometry = (flags | g_state[XST].geometryon) & ~g_state[XST].geometryoff;
	g_state[XST].changed |= 1u;
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
	g_state[XST].currentmode.masktst = depthtest;
	g_state[XST].currentmode.mask = v3;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_dither(int type)
{
	if (type == X_ENABLE)
	{
		g_state[XST].currentmode.dither = 1;
LABEL_5:
		g_state[XST].changed |= 4u;
		return 0;
	}
	if (type == X_DISABLE)
	{
		g_state[XST].currentmode.dither = 0;
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
	g_state[XST].currentmode.src = src;
	g_state[XST].currentmode.dst = dst;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_alphatest(float limit)
{
	if ( g_state[XST].geometry & 1 )
		limit = 1.0f;
	if ( limit < 0.0f || limit > 1.0f)
		return 1;
	g_state[XST].currentmode.alphatest = limit;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_combine(int colortext1)
{
	if (colortext1 < X_FIRSTCOMBINE || colortext1 > X_LASTCOMBINE)
		return 1;
	g_state[XST].currentmode.text1text2 = 0;
	g_state[XST].currentmode.colortext1 = colortext1;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_procombine(int rgb, int alpha)
{
	g_state[XST].currentmode.text1text2 = 0;
	g_state[XST].currentmode.colortext1 = rgb | (alpha << 16);
	g_state[XST].changed |= 4u;
	return 0;
}

int x_envcolor(float r, float g, float b, float a)
{
	g_state[XST].currentmode.env[0] = r;
	g_state[XST].currentmode.env[1] = g;
	g_state[XST].currentmode.env[2] = b;
	g_state[XST].currentmode.env[3] = a;
	// TODO: Buggy (decompile bug?)
	g_state[XST].currentmode.envc =
		(unsigned __int8)(signed __int64)(r * 255.0) | 
		((unsigned __int8)(signed __int64)(b * 255.0) << 16) | 
		((((unsigned int)(signed __int64)(a * 255.0) << 16) | 
		(unsigned __int8)(signed __int64)(g * 255.0)) << 8);
	g_state[XST].changed |= 4u;
	return 0;
}

int x_combine2(int colortext1, int text1text2, int sametex)
{
	if ( g_state[XST].tmus < 2 )
		return 1;
	if (colortext1 < X_FIRSTCOMBINE || colortext1 > X_LASTCOMBINE)
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state[XST].currentmode.colortext1 = colortext1;
	g_state[XST].currentmode.sametex = sametex;
	g_state[XST].currentmode.text1text2 = text1text2;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_procombine2(int rgb, int alpha, int text1text2, int sametex)
{
	if ( g_state[XST].tmus < 2 )
		return 1;
	if (text1text2 < X_FIRSTCOMBINE || text1text2 > X_LASTCOMBINE)
		return 1;
	g_state[XST].currentmode.sametex = sametex;
	g_state[XST].currentmode.colortext1 = rgb | (alpha << 16);
	g_state[XST].currentmode.text1text2 = text1text2;
	g_state[XST].changed |= 4u;
	return 0;
}

int x_texture(int text1handle)
{
	if (text1handle <= 0 )
		return 1;
	g_state[XST].currentmode.text1 = text1handle;
	g_state[XST].currentmode.text2 = 0;
	g_state[XST].changed |= 2u;
	return 0;
}

int x_texture2(int text1handle, int text2handle)
{
	if ( g_state[XST].tmus < 2 )
		return 1;
	if (text1handle <= 0 || text2handle <= 0 )
		return 1;
	g_state[XST].currentmode.text1 = text1handle;
	g_state[XST].currentmode.text2 = text2handle;
	g_state[XST].changed |= 2u;
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
	g_state[XST].currentmode.fogtype = type;
	g_state[XST].currentmode.fogmin = min;
	g_state[XST].currentmode.fogmax = max;
	g_state[XST].currentmode.fogcolor[0] = r;
	g_state[XST].currentmode.fogcolor[1] = g;
	g_state[XST].currentmode.fogcolor[2] = b;
	g_state[XST].currentmode.fogcolor[3] = 1.0f;
	g_state[XST].changed |= 8u;
	return 0;
}

void x_fullscreen(int fullscreen)
{
	init_fullscreen(fullscreen);
}
