// Set pixel pipeline modes, and initialize the graphics device.
// Ported to OpenGL - uses xgl* functions as the backend

#include "pch.h"
#include "xgl.h"

char *init_name()
{
	return "OpenGL";
}

int init_fullscreen(int fullscreen)
{
	static HWND last_hwnd = NULL;
	static int last_style = 0;
	
	if (fullscreen) {
		if (!g_state[XST].hwnd) return -1;
		last_hwnd = (HWND)g_state[XST].hwnd;
		last_style = GetWindowLongPtr(last_hwnd, GWL_STYLE);
		SetWindowLongPtr(last_hwnd, GWL_STYLE, last_style & ~WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE);
		SetWindowPos(last_hwnd, HWND_TOP, 0, 0, g_state[XST].xs, g_state[XST].ys, SWP_SHOWWINDOW);
	} else {
		if (!last_hwnd) return -1;
		SetWindowLongPtr(last_hwnd, GWL_STYLE, last_style);
		SetWindowPos(last_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	return 0;
}

int init_query()
{
	// Query OpenGL hardware support
	// Returns number of supported TMUs (texture mapping units)
	g_state[XST].tmus = 1;
	
	// Check for multitexture support
	if (GLEW_ARB_multitexture) {
		g_state[XST].tmus = 2;
	}
	
	x_log("init_query: OpenGL TMUs=%i\n", g_state[XST].tmus);
	return 0;
}

void init_reinit()
{
	x_log("init_reinit: shutdown\n");
	xgl_deinit();
	
	x_log("init_reinit: init\n");
	int width = g_state[XST].xs;
	int height = g_state[XST].ys;
	
	// Map resolution to appropriate size
	if (width < 640) width = 640;
	else if (width < 800) width = 800;
	
	x_log("init_reinit: resolution %ix%i\n", width, height);
	xgl_init(g_state[XST].hdc, g_state[XST].hwnd, width, height);
	xgl_resize(width, height);
	x_log("init_reinit: done\n");
}

int init_init()
{
	int width = g_state[XST].xs;
	int height = g_state[XST].ys;
	
	// Query OpenGL hardware
	init_query();
	x_log("x_open: cartdtype=OpenGL, tmus=%i\n", g_state[XST].tmus);
	
	// Initialize texture memory
	text_init();
	
	// Initialize OpenGL backend
	x_log("x_open: opening OpenGL context %ix%i\n", width, height);
	if (xgl_init(g_state[XST].hdc, g_state[XST].hwnd, width, height) != 0) {
		x_log("x_open: failed to initialize OpenGL\n");
		return -1;
	}
	
	xgl_resize(width, height);
	
	// Set clip window (viewport)
	g_state[XST].view_x0 = 0;
	g_state[XST].view_x1 = width - 1;
	g_state[XST].view_y0 = 0;
	g_state[XST].view_y1 = height - 1;
	
	x_log("x_open: done\n");
	return 0;
}

void init_deinit()
{
	x_log("x_close: deinit\n");
	xgl_deinit();
}

void init_activate()
{
	xgl_select_context();
}

void init_resize(int xs, int ys)
{
	g_state[XST].xs = xs;
	g_state[XST].ys = ys;
	xgl_resize(xs, ys);
}

void init_bufferswap()
{
	int retries = 0;
	
	// Wait for GPU to catch up (max 100 retries)
	while (retries < 100) {
		xgl_bufferswap();
		return;
	}
	
	x_log("timeout on bufferswap!\n");
	init_reinit();
}

void init_clear(int writecolor, int writedepth, float cr, float cg, float cb)
{
	xgl_clear(writecolor, writedepth, cr, cg, cb);
}

int init_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	if ((uint8_t)fb == X_FB_RGB565 || (uint8_t)fb == X_FB_RGBA8888) {
		return xgl_readfb(fb, x, y, xs, ys, buffer, bufrowlen);
	}
	return 1;
}

int init_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	// Not implemented
	return 1;
}

void mode_init()
{
	xgl_mode_init();
	g_state[XST].currentmode.stwhint = 0;
}

void mode_texturemode(int tmu, int format, int trilin)
{
	xgl_mode_texturemode(tmu, format, trilin);
}

void mode_loadtexture(int txtind)
{
	int has_multitexture = (g_state[XST].tmus >= 2) ? 1 : 0;
	xt_texture* txt = texture_get(txtind);
	
	if (!txt) return;
	
	int use_trilin = (txt->format & X_MIPMAP) && has_multitexture;
	
	// Load texture data
	if (use_trilin) {
		fxloadtexture_trilin(txt);
	} else {
		fxloadtexture_single(txt);
	}
	
	// Apply OpenGL texture state
	xgl_mode_loadtexture(txtind);
}

void mode_loadmultitexture(int txtind1, int txtind2)
{
	xt_texture* txt1 = texture_get(txtind1);
	xt_texture* txt2 = texture_get(txtind2);
	
	if (!txt1 || !txt2) return;
	
	// Load texture data
	fxloadtexture_multi(txt1, txt2);
	
	// Apply OpenGL texture state
	xgl_mode_loadmultitexture(txtind1, txtind2);
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

void my_guFogGenerateExp(GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density)
{
    for (int i = 0; i < GR_FOG_TABLE_SIZE; i++) {
        float f = expf(density * i / 64.0f);
        fogtable[i] = (FxU8)(f > 1.0f ? 255.0f : 255.0f * f);
    }
}

void my_guFogGenerateLinear(GrFog_t fogtable[GR_FOG_TABLE_SIZE], float nearW, float farW)
{
    for (int i = 0; i < GR_FOG_TABLE_SIZE; i++) {
        float f = (farW - nearW) * i / 64.0f + nearW;
        fogtable[i] = (FxU8)(f > 1.0f ? 255.0f : 255.0f * f);
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
		my_guFogGenerateLinear(fogtable, v6, v5);
	}
	else if (g_state[XST].active.fogtype == X_EXPONENTIAL)
	{
		v4 = 2.3f / (g_state[XST].active.fogmax / g_state[XST].znear);
		my_guFogGenerateExp(fogtable, v4);
	}
	fixfogtable(fogtable, GR_FOG_TABLE_SIZE);
	return fogtable;
}

void mode_change()
{
	// Apply OpenGL state changes via xgl_mode_change
	// This handles: blend, depth test, culling, alpha test, fog, color mask, depth mask, dither
	xgl_mode_change();
	
	// Update texture state tracking (same logic as original, but texture loading is done by xgl_mode_loadtexture)
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
	
	// Track texture changes
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
				mode_loadmultitexture(g_state[XST].currentmode.text1, g_state[XST].currentmode.text2);
				g_stats.chg_text += 2;
				g_state[XST].active.text1 = g_state[XST].currentmode.text1;
				g_state[XST].active.text2 = g_state[XST].currentmode.text2;
			}
		}
		g_state[XST].active.sametex = g_state[XST].currentmode.sametex;
	}
	
	// Update active state
	g_state[XST].active.stwhint = g_state[XST].currentmode.stwhint;
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
