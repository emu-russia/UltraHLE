// Set pixel pipeline modes, and initialize the graphics device.
// OpenGL backend for the X library (replaces the Glide backend of XGLIDE).
//
// The fixed-function OpenGL pipeline is used (compatibility profile), which
// matches the capabilities the original Voodoo-era library expected:
//
//   - multitexture        (GL_ARB_multitexture)     - dual texture passes
//   - env combiners       (GL_ARB_texture_env_combine) - X combine modes
//   - blend func separate (GL_EXT_blend_func_separate) - Glide blend factors
//   - fog coordinate      (GL_EXT_fog_coord)       - Glide style fog

#include "pch.h"

char *init_name()
{
	return "OpenGL";
}

// ---------------------------------------------------------------------------
// WGL context
// ---------------------------------------------------------------------------

static HDC  gl_hdc = NULL;
static HGLRC gl_hrc = NULL;
static HWND gl_hwnd = NULL;
static volatile LONG gl_resized = 0;	// window size changed (set by x_resize)

// Hide/show the child windows of the window we render into (the emulator's
// ROM list, status bar, debug output). They are hidden while the display is
// open so they do not show through the game image, and restored afterwards.
static void gl_showhide_children(int show)
{
	HWND parent = (HWND)g_state[XST].hwnd;
	HWND child;
	if (!parent)
		return;
	child = GetWindow(parent, GW_CHILD);
	while (child)
	{
		ShowWindow(child, show ? SW_SHOW : SW_HIDE);
		child = GetWindow(child, GW_HWNDNEXT);
	}
}

// Compute the letterboxed viewport that shows the game image (xs x ys)
// inside the current window size, preserving the aspect ratio.
void gl_viewport_geom(int* lx, int* ly, int* vw, int* vh)
{
	RECT rc;
	int winw, winh;
	float aspect;

	if (gl_hwnd)
		GetClientRect(gl_hwnd, &rc);
	else
	{
		rc.left = 0;
		rc.top = 0;
		rc.right = g_state[XST].xs;
		rc.bottom = g_state[XST].ys;
	}
	winw = rc.right - rc.left;
	winh = rc.bottom - rc.top;
	if (winw < 1) winw = 1;
	if (winh < 1) winh = 1;

	if (g_state[XST].ys <= 0)
		aspect = 4.0f / 3.0f;
	else
		aspect = (float)g_state[XST].xs / (float)g_state[XST].ys;

	if ((float)winw / (float)winh > aspect)
	{
		*vh = winh;
		*vw = (int)(winh * aspect + 0.5f);
	}
	else
	{
		*vw = winw;
		*vh = (int)(winw / aspect + 0.5f);
	}
	*lx = (winw - *vw) / 2;
	*ly = (winh - *vh) / 2;
}

int init_fullscreen(int fullscreen)
{
	// OpenGL renders into the window the emulator gives us; nothing to do.
	return 0;
}

int init_query()
{
	return 0;
}

static void gl_setup_view(void)
{
	int lx, ly, vw, vh;

	gl_viewport_geom(&lx, &ly, &vw, &vh);
	glViewport(lx, ly, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Depth is the oow value (see fxgeom.c): nearer = larger oow. With
	// near=-1, far=1 the ortho matrix maps z to z_ndc = -z, so a fragment
	// at oow lands at window depth (1-oow)/2 - nearer = smaller depth,
	// which is the standard GL_LESS convention (see mode_change()).
	glOrtho(0, g_state[XST].xs, 0, g_state[XST].ys, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init_reinit()
{
	x_log("init_reinit: shutdown\n");
	init_deinit();
	x_log("init_reinit: init\n");
	init_init();
}

int init_init()
{
	PIXELFORMATDESCRIPTOR pfd;
	int pf;
	int ok = 0;
	HWND parent = (HWND)g_state[XST].hwnd;

	if (!parent)
		return -1;

	gl_hwnd = parent;
	gl_hdc = GetDC(gl_hwnd);
	if (!gl_hdc)
	{
		x_log("x_open: GetDC failed\n");
		return -1;
	}

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pf = ChoosePixelFormat(gl_hdc, &pfd);
	if (!pf)
	{
		// Try a more conservative pixel format
		pfd.cColorBits = 16;
		pfd.cDepthBits = 16;
		pf = ChoosePixelFormat(gl_hdc, &pfd);
	}
	if (!pf)
	{
		x_log("x_open: ChoosePixelFormat failed\n");
		goto fail;
	}
	if (!SetPixelFormat(gl_hdc, pf, &pfd))
	{
		x_log("x_open: SetPixelFormat failed\n");
		goto fail;
	}

	gl_hrc = wglCreateContext(gl_hdc);
	if (!gl_hrc)
	{
		x_log("x_open: wglCreateContext failed\n");
		goto fail;
	}
	if (!wglMakeCurrent(gl_hdc, gl_hrc))
	{
		x_log("x_open: wglMakeCurrent failed\n");
		goto fail;
	}

	if (glcompat_load() != 0)
	{
		x_log("x_open: some OpenGL extensions unavailable\n");
	}

	// VSync
	if (g_state[XST].vsync)
	{
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
			(PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(1);
	}

	// Capabilities: multitexture present?
	g_state[XST].tmus = (glActiveTextureARB && glMultiTexCoord4fARB) ? 2 : 1;
	x_log("x_open: OpenGL %s (%s) tmus=%i\n",
		glGetString(GL_VERSION), glGetString(GL_RENDERER), g_state[XST].tmus);

	// The emulator's UI children (ROM list, status bar, debug output) are
	// hidden while the display is open so they do not paint over the image.
	gl_showhide_children(0);

	gl_setup_view();

	// Default GL state
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_SCISSOR_TEST);

	// Texture env defaults
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (glActiveTextureARB)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glMatrixMode(GL_MODELVIEW);
	}

	text_init();
	mode_init();
	ok = 1;

fail:
	if (!ok)
	{
		if (gl_hrc) { wglDeleteContext(gl_hrc); gl_hrc = NULL; }
		if (gl_hdc) { ReleaseDC(gl_hwnd, gl_hdc); gl_hdc = NULL; }
		return -1;
	}
	return 0;
}

void init_deinit()
{
	x_log("x_close");
	if (gl_hrc)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(gl_hrc);
		gl_hrc = NULL;
	}
	if (gl_hdc)
	{
		ReleaseDC(gl_hwnd, gl_hdc);
		gl_hdc = NULL;
	}
	gl_hwnd = NULL;
	// restore the emulator's UI children
	gl_showhide_children(1);
	InterlockedExchange(&gl_resized, 0);
}

void init_activate()
{
}

void init_resize(int xs, int ys)
{
	// The emulator calls this when the main window is resized: xs/ys is the
	// new client size. The game resolution (g_state[XST].xs/ys) is kept and
	// the image is letterboxed into the new window size. The viewport is
	// re-applied on the emulation thread on the next frame (see init_clear).
	if (gl_hrc)
		InterlockedExchange(&gl_resized, 1);
}

void init_bufferswap()
{
	if (gl_hdc)
		SwapBuffers(gl_hdc);
}

void init_clear(int writecolor, int writedepth, float cr, float cg, float cb)
{
	GLbitfield mask = 0;
	int lx, ly, vw, vh;
	float sx, sy;
	int cx0, cy0, cx1, cy1;

	x_flush();

	// The window may have been resized by the user; re-apply the viewport
	// (this runs on the emulation thread, which owns the GL context).
	if (gl_resized)
	{
		InterlockedExchange(&gl_resized, 0);
		if (gl_hrc)
			gl_setup_view();
	}

	// Map the game-space viewport rectangle through the letterboxed viewport
	// into window pixels (y up) for the scissor test.
	gl_viewport_geom(&lx, &ly, &vw, &vh);
	sx = (float)vw / (float)g_state[XST].xs;
	sy = (float)vh / (float)g_state[XST].ys;

	cx0 = (int)g_state[XST].view_x0;
	cy0 = (int)g_state[XST].view_y0;
	cx1 = (int)g_state[XST].view_x1;
	cy1 = (int)g_state[XST].view_y1;
	if (cx0 < 0) cx0 = 0;
	if (cy0 < 0) cy0 = 0;
	if (cx1 >= g_state[XST].xs) cx1 = g_state[XST].xs - 1;
	if (cy1 >= g_state[XST].ys) cy1 = g_state[XST].ys - 1;

	glColorMask(writecolor >= 1 ? GL_TRUE : GL_FALSE,
		writecolor >= 1 ? GL_TRUE : GL_FALSE,
		writecolor >= 1 ? GL_TRUE : GL_FALSE,
		writecolor >= 1 ? GL_TRUE : GL_FALSE);
	glDepthMask(writedepth >= 1 ? GL_TRUE : GL_FALSE);

	glClearColor(cr, cg, cb, 1.0f);
	glClearDepth(1.0);

	// Clear the WHOLE window (including the letterbox bars around the
	// letterboxed viewport), then limit the actual drawing to the game
	// viewport with the scissor test.
	glDisable(GL_SCISSOR_TEST);
	if (writecolor) mask |= GL_COLOR_BUFFER_BIT;
	if (writedepth) mask |= GL_DEPTH_BUFFER_BIT;
	glClear(mask);

	// re-enable the scissor for the game area draws (game y is top-down,
	// GL scissor y is bottom-up)
	glScissor(lx + (int)(cx0 * sx),
		ly + (int)((g_state[XST].ys - 1 - cy1) * sy),
		(int)((cx1 - cx0 + 1) * sx) + 1,
		(int)((cy1 - cy0 + 1) * sy) + 1);
	glEnable(GL_SCISSOR_TEST);

	// restore mask state
	glDepthMask((g_state[XST].currentmode.mask & 2u) ? GL_TRUE : GL_FALSE);
	glColorMask(g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
		g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
		g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
		g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE);
}

// Map a game-space rectangle to window pixels (through the letterboxed
// viewport). x/y are the game coordinates (y from the top), xs/ys the size.
// wy = the GL row of the FIRST game row (y); the rows below it are at
// wy - k*sy (GL y grows upwards).
static void gl_map_game_rect(int x, int y, int xs, int ys, int* wx, int* wy, int* ww, int* wh)
{
	int lx, ly, vw, vh;
	float sx, sy;
	gl_viewport_geom(&lx, &ly, &vw, &vh);
	sx = (float)vw / (float)g_state[XST].xs;
	sy = (float)vh / (float)g_state[XST].ys;
	*wx = lx + (int)(x * sx);
	*wy = ly + (int)((g_state[XST].ys - 1 - y) * sy);	// GL y is bottom-up
	*ww = (int)(xs * sx) + 1;
	*wh = (int)(ys * sy) + 1;
}

static void readfb_rgb565(int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	unsigned char* row;
	int i, j;
	int wx, wy, ww, wh;

	gl_map_game_rect(x, y, xs, ys, &wx, &wy, &ww, &wh);

	row = (unsigned char*)x_allocfast(ww * 4);
	for (j = 0; j < ys; j++)
	{
		unsigned short* dst = (unsigned short*)(buffer + j * bufrowlen);
		// one game row = one band of window rows; read the middle one
		int gy = wy - (int)((j + 0.5f) * (float)wh / (float)ys);
		glReadPixels(wx, gy, ww, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
		for (i = 0; i < xs; i++)
		{
			unsigned short v;
			int pi = (int)((i + 0.5f) * (float)ww / (float)xs) * 4;
			v = (unsigned short)(((row[pi + 0] >> 3) << 11) |
				((row[pi + 1] >> 2) << 5) |
				(row[pi + 2] >> 3));
			dst[i] = v;
		}
	}
	x_free(row);
}

int init_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	GLenum srcbuf;

	if (!gl_hrc)
		return 1;

	srcbuf = (fb & X_FB_FRONT) ? GL_FRONT : GL_BACK;
	glReadBuffer(srcbuf);

	if ((uint8_t)fb == X_FB_RGB565)
	{
		readfb_rgb565(x, y, xs, ys, buffer, bufrowlen);
		return 0;
	}
	else if ((uint8_t)fb == X_FB_RGBA8888)
	{
		unsigned char* row;
		int i, j;
		int wx, wy, ww, wh;
		// glReadPixels has y=0 at the bottom, the emulator wants y=0 at the top
		gl_map_game_rect(x, y, xs, ys, &wx, &wy, &ww, &wh);
		row = (unsigned char*)x_allocfast(ww * 4);
		for (j = 0; j < ys; j++)
		{
			int gy = wy - (int)((j + 0.5f) * (float)wh / (float)ys);
			glReadPixels(wx, gy, ww, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
			// resample the (possibly scaled) window row into the game-size row
			{
				unsigned char* src = row;
				unsigned char* dst = (unsigned char*)buffer + j * bufrowlen;
				if ((float)ww / (float)xs > 1.01f || (float)ww / (float)xs < 0.99f)
				{
					// scaled: sample the texel centres
					for (i = 0; i < xs; i++)
					{
						int pi = (int)((i + 0.5f) * (float)ww / (float)xs) * 4;
						dst[i * 4 + 0] = src[pi + 0];
						dst[i * 4 + 1] = src[pi + 1];
						dst[i * 4 + 2] = src[pi + 2];
						dst[i * 4 + 3] = src[pi + 3];
					}
				}
				else
				{
					memcpy(dst, src, xs * 4);
				}
			}
		}
		x_free(row);
		return 0;
	}
	return 1;
}

int init_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
	// Not implemented (same as the Glide backend)
	return 1;
}

// ---------------------------------------------------------------------------
// Modes
// ---------------------------------------------------------------------------

void mode_init()
{
	g_state[XST].currentmode.stwhint = 0;
	g_state[XST].active.mask = 0;
	g_state[XST].active.masktst = 0;
	g_state[XST].active.colortext1 = 0;
	g_state[XST].active.text1text2 = 0;
	g_state[XST].active.alphatest = 0;
	g_state[XST].active.src = 0;
	g_state[XST].active.dst = 0;
	g_state[XST].active.dither = 0;
	g_state[XST].active.textures = 0;
	g_state[XST].active.fogtype = 0;
	g_state[XST].active.envc = 0;
	g_state[XST].send = 0;
	g_state[XST].setnew = 0;
}

// GL texture unit selection
static void select_unit(int tmu)
{
	if (glActiveTextureARB)
		glActiveTextureARB((tmu == 0) ? GL_TEXTURE0_ARB : GL_TEXTURE1_ARB);
}

void mode_texturemode(int tmu, int format, int trilin)
{
	GLint minfilter, magfilter, wraps, wrapt;

	if (trilin || (format & X_MIPMAP))
	{
		minfilter = GL_LINEAR_MIPMAP_LINEAR;
		magfilter = GL_LINEAR;
	}
	else
	{
		minfilter = (format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR;
		magfilter = (format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR;
	}

	if (format & X_CLAMP)
	{
		wraps = (format & X_CLAMPNOX) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
		wrapt = (format & X_CLAMPNOY) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	}
	else
	{
		wraps = GL_REPEAT;
		wrapt = GL_REPEAT;
	}

	select_unit(tmu);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt);
	if (glActiveTextureARB)
		select_unit(0);
}

// Set the texture matrix of unit `tmu` to normalize the library's texture
// coordinate space ([0..xmul]) into OpenGL's [0..1].
static void set_texmatrix(int tmu, float xmul, float ymul)
{
	select_unit(tmu);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	if (xmul != 0.0f && ymul != 0.0f)
		glScalef(1.0f / xmul, 1.0f / ymul, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	if (glActiveTextureARB)
		select_unit(0);
}

void mode_loadtexture(int txtind)
{
	xt_texture* txt;
	int fmt;

	txt = texture_get(txtind);
	if (!txt)
		return;
	fmt = txt->format;

	if (fxuploadtexture(txt) != 0)
		return;

	// Texture 0
	select_unit(0);
	glBindTexture(GL_TEXTURE_2D, txt->gltexture);
	glEnable(GL_TEXTURE_2D);
	set_texmatrix(0, txt->xmul, txt->ymul);
	if (glActiveTextureARB)
	{
		// keep unit 1 consistent (used by the trilinear/mipmap path)
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	mode_texturemode(0, fmt, (fmt & X_MIPMAP) ? 1 : 0);

	g_state[XST].texturexmul = txt->xmul;
	g_state[XST].textureymul = txt->ymul;
}

void mode_loadmultitexture(int txtind1, int txtind2)
{
	xt_texture* txt1;
	xt_texture* txt2;

	if (!glActiveTextureARB)
	{
		// no multitexture: fall back to a single texture
		mode_loadtexture(txtind1);
		return;
	}

	txt1 = texture_get(txtind1);
	txt2 = texture_get(txtind2);
	if (!txt1 || !txt2)
		return;

	if (fxuploadtexture(txt1) != 0 || fxuploadtexture(txt2) != 0)
		return;

	// Texture 0 <- txtind1
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, txt1->gltexture);
	glEnable(GL_TEXTURE_2D);
	set_texmatrix(0, txt1->xmul, txt1->ymul);
	mode_texturemode(0, txt1->format, 0);

	// Texture 1 <- txtind2
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, txt2->gltexture);
	glEnable(GL_TEXTURE_2D);
	set_texmatrix(1, txt1->xmul, txt1->ymul);
	mode_texturemode(1, txt2->format, 0);

	glActiveTextureARB(GL_TEXTURE0_ARB);

	// The vertex pipeline computes BOTH texture coordinate pairs with the
	// first texture's scale (same as the original Glide backend), so the
	// second unit uses the same texture matrix.
	g_state[XST].texturexmul = txt1->xmul;
	g_state[XST].textureymul = txt1->ymul;
}

// ---------------------------------------------------------------------------
// Combiners (X combine modes -> GL_ARB_texture_env_combine)
// ---------------------------------------------------------------------------

// Configure the env combiner of texture unit `tmu`.
//   stage 0: sources are PRIMARY (iterated vertex color), TEXTURE, CONSTANT
//   stage 1: sources are PREVIOUS (previous unit output), TEXTURE, CONSTANT
//   rgbmode / alphamode: X combine modes for the color and alpha channels.
//   If alphamode is 0, the alpha follows the rgb mode (plain x_combine).
static void set_combine(int tmu, int rgbmode, int alphamode, int stage)
{
	GLenum srcA = (stage == 0) ? GL_PRIMARY_COLOR : GL_PREVIOUS;
	GLenum fnRGB, fnA;
	GLenum s0, s1, s2;
	GLenum as0;

	if (alphamode == 0)
		alphamode = rgbmode;

	switch (rgbmode)
	{
		case X_TEXTURE:			// color = texture
		case X_DECAL:			// Glide: DECAL_TEXTURE = just the texture
		case X_TEXTURE_IA:		// texture, alpha handled below
			fnRGB = GL_REPLACE; s0 = GL_TEXTURE; s1 = srcA; s2 = GL_TEXTURE;
			break;
		case X_MUL:				// gouraud * texture
			fnRGB = GL_MODULATE; s0 = srcA; s1 = GL_TEXTURE; s2 = srcA;
			break;
		case X_ADD:				// gouraud + texture
			fnRGB = GL_ADD; s0 = srcA; s1 = GL_TEXTURE; s2 = srcA;
			break;
		case X_TEXTUREBLEND:	// Glide: blend(texture, iterated, texturealpha)
			// C = TEXTURE*TA + ITERATED*(1-TA)  (Glide grColorCombine(7,4,0,1,0))
			fnRGB = GL_INTERPOLATE; s0 = GL_TEXTURE; s1 = srcA; s2 = GL_TEXTURE;
			break;
		case X_TEXTUREENVA:		// Glide: blend(iterated, envcolor, texturealpha)
			// C = ITERATED*TA + ENV*(1-TA)  (Glide grColorCombine(7,4,1,0,0))
			fnRGB = GL_INTERPOLATE; s0 = srcA; s1 = GL_CONSTANT; s2 = GL_TEXTURE;
			break;
		case X_TEXTUREENVC:		// Glide: blend(iterated, envcolor, factor) - approx: texturealpha
			// C = ITERATED*F + ENV*(1-F)  (Glide grColorCombine(7,5,1,0,0))
			fnRGB = GL_INTERPOLATE; s0 = srcA; s1 = GL_CONSTANT; s2 = GL_TEXTURE;
			break;
		case X_TEXTUREENVCR:	// Glide: blend(envcolor, iterated, factor) - approx: texturealpha
			// C = ENV*F + ITERATED*(1-F)  (Glide grColorCombine(7,5,0,2,0))
			fnRGB = GL_INTERPOLATE; s0 = GL_CONSTANT; s1 = srcA; s2 = GL_TEXTURE;
			break;
		case X_SUB:				// Glide: texture - iterated  (grColorCombine(6,8,0,1,0))
			fnRGB = GL_SUBTRACT; s0 = GL_TEXTURE; s1 = srcA; s2 = srcA;
			break;
		case X_MUL_TA:			// gouraud * texture
		case X_MUL_IA:
		case X_MULADD:			// gouraud * texture + gouraud - not available, approx.
			fnRGB = GL_MODULATE; s0 = srcA; s1 = GL_TEXTURE; s2 = srcA;
			break;
		default:				// X_WHITE, X_COLOR and anything else: pass through
			fnRGB = GL_REPLACE; s0 = srcA; s1 = GL_TEXTURE; s2 = srcA;
			break;
	}

	switch (alphamode)
	{
		case X_TEXTURE:			// alpha = texture
			fnA = GL_REPLACE; as0 = GL_TEXTURE;
			break;
		case X_COLOR:			// alpha = iterated
		case X_TEXTURE_IA:
		case X_TEXTUREBLEND:
		case X_TEXTUREENVA:
		case X_TEXTUREENVC:
		case X_TEXTUREENVCR:
		case X_MUL_IA:
			fnA = GL_REPLACE; as0 = srcA;
			break;
		case X_ADD:				// alpha = texture
		case X_DECAL:
		case X_SUB:
		case X_MUL_TA:
			fnA = GL_REPLACE; as0 = GL_TEXTURE;
			break;
		case X_MUL:				// alpha = iterated * texture  (Glide TEXTURE_ALPHA_TIMES_ITERATED_ALPHA)
			fnA = GL_MODULATE; as0 = srcA;
			break;
		case X_MULADD:
		default:
			fnA = GL_MODULATE; as0 = srcA;
			break;
	}

	select_unit(tmu);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, fnRGB);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, s0);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, s1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, s2);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, fnA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, as0);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, (alphamode == X_MUL) ? GL_TEXTURE : as0);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, as0);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
	if (glActiveTextureARB)
		select_unit(0);
}

// Map a Glide blend factor code (from the original decompiled mode_change)
// to an OpenGL blend factor.
static GLenum blendfactor(int glidefactor)
{
	switch (glidefactor)
	{
		case 0:  return GL_ZERO;						// GR_BLEND_ZERO
		case 1:  return GL_SRC_ALPHA;					// GR_BLEND_SRC_ALPHA
		case 2:  return GL_DST_COLOR;					// GR_BLEND_DST_COLOR
		case 3:  return GL_DST_ALPHA;					// GR_BLEND_DST_ALPHA
		case 4:  return GL_ONE;							// GR_BLEND_ONE
		case 5:  return GL_ONE_MINUS_SRC_ALPHA;			// GR_BLEND_ONE_MINUS_SRC_ALPHA
		case 6:  return GL_ONE_MINUS_DST_COLOR;			// GR_BLEND_ONE_MINUS_DST_COLOR
		default: return GL_ONE_MINUS_DST_ALPHA;			// GR_BLEND_ONE_MINUS_DST_ALPHA
	}
}

void mode_change()
{
	if (g_state[XST].changed & 1)
	{
		if (g_state[XST].setnew != g_state[XST].geometry)
		{
			// X_CULLFRONT -> GR_CULL_NEGATIVE (culls clockwise = GL back)
			// X_CULLBACK -> GR_CULL_POSITIVE (culls counter-clockwise = GL front)
			if (g_state[XST].geometry & 2)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}
			else if (g_state[XST].geometry & 4)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
			g_state[XST].setnew = g_state[XST].geometry;
		}
	}
	if (g_state[XST].changed & 8)
	{
		g_state[XST].active.fogtype = g_state[XST].currentmode.fogtype;
		g_state[XST].active.fogmin = g_state[XST].currentmode.fogmin;
		g_state[XST].active.fogmax = g_state[XST].currentmode.fogmax;
		memcpy(g_state[XST].active.fogcolor, g_state[XST].currentmode.fogcolor, sizeof(float) * 4);

		if (g_state[XST].currentmode.fogtype == X_DISABLE)
		{
			glDisable(GL_FOG);
		}
		else
		{
			// Fog is applied per-vertex with glFogCoord (see draw_vertex in
			// fxgeom.c): fogcoord = w = 1/oow. GL_LINEAR/GL_EXP then produce
			// exactly the fog curve the X API documents (min = 0% fog,
			// max = 100% / 90% fog). The original Glide backend used a fog
			// table indexed by w; the linear curve is the OpenGL equivalent.
			float znear = g_state[XST].znear;
			GLfloat fogcolor[4];
			fogcolor[0] = g_state[XST].active.fogcolor[0];
			fogcolor[1] = g_state[XST].active.fogcolor[1];
			fogcolor[2] = g_state[XST].active.fogcolor[2];
			fogcolor[3] = 1.0f;
			glEnable(GL_FOG);
			// Fog is applied per-vertex through glFogCoord (see draw_vertex
			// in fxgeom.c): fogcoord = w = 1/oow.
			if (glFogCoordfEXT)
				glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
			glFogfv(GL_FOG_COLOR, fogcolor);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			if (znear != 0.0f)
			{
				glFogf(GL_FOG_START, g_state[XST].active.fogmin / znear);
				glFogf(GL_FOG_END, g_state[XST].active.fogmax / znear);
			}
			else
			{
				glFogf(GL_FOG_START, g_state[XST].active.fogmin);
				glFogf(GL_FOG_END, g_state[XST].active.fogmax);
			}
			if (g_state[XST].active.fogtype == X_EXPONENTIAL)
			{
				// 90% fog at fogmax: density = 2.3 / (fogmax/znear)
				glFogi(GL_FOG_MODE, GL_EXP);
				glFogf(GL_FOG_DENSITY, (g_state[XST].active.fogmax / znear) != 0.0f
					? 2.3f / (g_state[XST].active.fogmax / znear) : 1.0f);
			}
			// X_LINEARADD (additive fog) is approximated with linear fog.
		}
	}
	if (g_state[XST].changed & 4)
	{
		if (g_state[XST].active.mask != g_state[XST].currentmode.mask)
		{
			glDepthMask((g_state[XST].currentmode.mask & 2u) ? GL_TRUE : GL_FALSE);
			glColorMask(g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
				g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
				g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE,
				g_state[XST].currentmode.mask & 1 ? GL_TRUE : GL_FALSE);
			g_state[XST].active.mask = g_state[XST].currentmode.mask;
		}
		if (g_state[XST].active.masktst != g_state[XST].currentmode.masktst)
		{
			// Depth is the oow value (see fxgeom.c); with the ortho setup in
			// gl_setup_view the window depth = (1-oow)/2, so nearer fragments
			// have smaller depth.
			//
			// GL_LEQUAL: the N64 RDP renders lots of coplanar geometry
			// (adjacent floor/wall tiles etc.); a strict GL_LESS test would
			// reject the equal-depth fragments and leave holes. GL_LEQUAL
			// matches the RDP z-buffer behavior (nearer or equal passes).
			switch (g_state[XST].currentmode.masktst)
			{
				case X_ENABLE:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LEQUAL);
					break;
				case X_DISABLE:
					glDisable(GL_DEPTH_TEST);
					break;
				case X_TESTEQ:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_EQUAL);
					break;
				case X_TESTNE:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_NOTEQUAL);
					break;
				case X_TESTGE:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_GEQUAL);
					break;
				case X_TESTLE:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LEQUAL);
					break;
				case X_TESTGT:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_GREATER);
					break;
				case X_TESTLT:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LESS);
					break;
				default:
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LEQUAL);
					break;
			}
			g_state[XST].active.masktst = g_state[XST].currentmode.masktst;
		}
		if (g_state[XST].currentmode.envc != g_state[XST].active.envc)
		{
			g_state[XST].active.colortext1 = 0xfffffff;
			g_state[XST].active.envc = g_state[XST].currentmode.envc;
			memcpy(g_state[XST].active.env, g_state[XST].currentmode.env, sizeof(float) * 4);
			// env color for the combiners (GL_CONSTANT) - both texture units
			// have their own texture-env color.
			{
				GLfloat env[4];
				env[0] = g_state[XST].currentmode.env[0];
				env[1] = g_state[XST].currentmode.env[1];
				env[2] = g_state[XST].currentmode.env[2];
				env[3] = g_state[XST].currentmode.env[3];
				select_unit(0);
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env);
				if (glActiveTextureARB)
				{
					glActiveTextureARB(GL_TEXTURE1_ARB);
					glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env);
					glActiveTextureARB(GL_TEXTURE0_ARB);
				}
			}
		}
		if (g_state[XST].currentmode.colortext1 != g_state[XST].active.colortext1)
		{
			g_state[XST].send &= 0xFFFFFFFC;
			switch (g_state[XST].currentmode.colortext1 & 0xFFFF)
			{
				case X_WHITE:
					// constant white
					break;
				case X_COLOR:
					g_state[XST].send |= 1u;
					break;
				case X_TEXTURE:
				case X_DECAL:
					g_state[XST].send |= 2u;
					break;
				case X_ADD:
				case X_MUL:
				case X_MUL_TA:
				case X_MUL_IA:
				case X_TEXTUREBLEND:
				case X_TEXTUREENVA:
				case X_TEXTUREENVC:
				case X_TEXTUREENVCR:
				case X_SUB:
				case X_MULADD:
					g_state[XST].send |= 3u;
					break;
				case X_TEXTURE_IA:
					// Glide: DECAL_TEXTURE + ITERATED_ALPHA -> needs both
					// the vertex color (for alpha) and the texture.
					g_state[XST].send |= 3u;
					break;
				default:
					break;
			}
			// x_procombine(x, y) packs the alpha combine in the high 16 bits
			{
				int amode = (g_state[XST].currentmode.colortext1 >> 16) & 0xFFFF;
				switch (amode)
				{
					case X_COLOR:
						g_state[XST].send |= 1u;
						break;
					case X_TEXTURE:
					case X_DECAL:
						g_state[XST].send |= 2u;
						break;
					case X_ADD:
					case X_MUL:
					case X_MUL_TA:
					case X_MUL_IA:
					case X_TEXTUREBLEND:
					case X_TEXTUREENVA:
					case X_TEXTUREENVC:
					case X_TEXTUREENVCR:
					case X_SUB:
					case X_MULADD:
						g_state[XST].send |= 3u;
						break;
					default:
						break;
				}
				set_combine(0, g_state[XST].currentmode.colortext1 & 0xFFFF, amode, 0);
			}
			g_state[XST].active.colortext1 = g_state[XST].currentmode.colortext1;
		}
		if (g_state[XST].currentmode.text1text2 != g_state[XST].active.text1text2)
		{
			g_state[XST].send &= 0xFFFFFFFB;
			if (g_state[XST].currentmode.text1text2 > X_TEXTURE)
			{
				switch (g_state[XST].currentmode.text1text2)
				{
					case X_ADD:
					case X_MUL:
					case X_DECAL:
					case X_MULADD:
					case X_SUB:
						g_state[XST].send |= 4u;
						break;
					default:
						break;
				}
			}
			set_combine(1, g_state[XST].currentmode.text1text2, 0, 1);
			g_state[XST].active.text1text2 = g_state[XST].currentmode.text1text2;
		}
		if (g_state[XST].active.alphatest != g_state[XST].currentmode.alphatest)
		{
			if (g_state[XST].currentmode.alphatest < 1.0f && g_state[XST].currentmode.alphatest > 0.0f)
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, g_state[XST].currentmode.alphatest);
			}
			else
			{
				glDisable(GL_ALPHA_TEST);
			}
			g_state[XST].active.alphatest = g_state[XST].currentmode.alphatest;
		}
		if (g_state[XST].active.src != g_state[XST].currentmode.src ||
			g_state[XST].active.dst != g_state[XST].currentmode.dst)
		{
			// Decode the blend factors exactly like the original Glide
			// backend did (the constants are Glide GrAlphaBlendMode_t codes).
			int v17, v18, v19, v20;
			switch (g_state[XST].currentmode.src)
			{
				case X_ZERO:  v17 = 0; v18 = 0; break;
				case X_OTHER: v17 = 2; v18 = 3; break;
				case X_ALPHA: v17 = 1; v18 = 1; break;
				case X_OTHERALPHA: v17 = 3; v18 = 3; break;
				case X_INVOTHER: v17 = 6; v18 = 7; break;
				case X_INVALPHA: v17 = 5; v18 = 5; break;
				case X_INVOTHERALPHA: v17 = 7; v18 = 7; break;
				default: v17 = 4; v18 = 4; break;	// X_ONE
			}
			switch (g_state[XST].currentmode.dst)
			{
				case X_ONE: v19 = 4; v20 = 4; break;
				case X_OTHER: v19 = 2; v20 = 1; break;
				case X_ALPHA: v19 = 3; v20 = 3; break;
				case X_OTHERALPHA: v19 = 1; v20 = 1; break;
				case X_INVOTHER: v19 = 6; v20 = 5; break;
				case X_INVALPHA: v19 = 7; v20 = 7; break;
				case X_INVOTHERALPHA: v19 = 5; v20 = 5; break;
				default: v19 = 0; v20 = 0; break;	// X_ZERO
			}
			if (g_state[XST].currentmode.src == X_ZERO && g_state[XST].currentmode.dst == X_ONE)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				glEnable(GL_BLEND);
				if (glBlendFuncSeparateEXT)
					glBlendFuncSeparateEXT(blendfactor(v17), blendfactor(v19), blendfactor(v18), blendfactor(v20));
				else
					glBlendFunc(blendfactor(v17), blendfactor(v19));
			}
			g_state[XST].active.src = g_state[XST].currentmode.src;
			g_state[XST].active.dst = g_state[XST].currentmode.dst;
		}
		if (g_state[XST].active.dither != g_state[XST].currentmode.dither)
		{
			if (g_state[XST].currentmode.dither)
				glEnable(GL_DITHER);
			else
				glDisable(GL_DITHER);
			g_state[XST].active.dither = g_state[XST].currentmode.dither;
		}
	}
	if (g_state[XST].send & 4)
	{
		g_state[XST].currentmode.textures = 2;
	}
	else
	{
		g_state[XST].currentmode.textures = 1;
		if ((g_state[XST].send & 2) == 0)
			g_state[XST].currentmode.textures = 0;
	}
	if (g_state[XST].setnew & 1)
		g_state[XST].currentmode.textures = 0;
	if (g_state[XST].active.textures != g_state[XST].currentmode.textures)
	{
		g_state[XST].active.text1 = 0;
		g_state[XST].active.text2 = 0;
		g_state[XST].changed |= 2u;
		g_state[XST].active.textures = g_state[XST].currentmode.textures;
	}
	if (g_state[XST].changed & 2)
	{
		if (g_state[XST].currentmode.textures == 0)
		{
			// no textures: disable texturing entirely
			select_unit(0);
			glDisable(GL_TEXTURE_2D);
			if (glActiveTextureARB)
			{
				glActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0_ARB);
			}
		}
		else if (g_state[XST].currentmode.textures == 1)
		{
			if (g_state[XST].currentmode.text1 != g_state[XST].active.text1)
			{
				mode_loadtexture(g_state[XST].currentmode.text1);
				g_state[XST].active.text1 = g_state[XST].currentmode.text1;
				++g_stats.chg_text;
			}
		}
		else if (g_state[XST].currentmode.textures == 2)
		{
			if (g_state[XST].currentmode.text1 != g_state[XST].active.text1 ||
				g_state[XST].currentmode.text2 != g_state[XST].active.text2)
			{
				mode_loadmultitexture(g_state[XST].currentmode.text1, g_state[XST].currentmode.text2);
				g_stats.chg_text += 2;
				g_state[XST].active.text1 = g_state[XST].currentmode.text1;
			}
		}
		g_state[XST].active.sametex = g_state[XST].currentmode.sametex;
	}
	g_state[XST].changed = 0;
	++g_stats.chg_mode;
	if (g_state[XST].geometry & X_DUMPDATA)
	{
		x_log("#modechange:\n");
		x_log("-mask c=%i z=%i zt=%i\n", g_state[XST].currentmode.mask & 1, (g_state[XST].currentmode.mask & 2u) >> 1, (g_state[XST].currentmode.mask & 4u) >> 2);
		x_log("-colortext1 %08X\n", g_state[XST].currentmode.colortext1);
		x_log("-alphatest %04X\n", (int16_t)g_state[XST].currentmode.alphatest);
		x_log("-blend %04X %04X\n", g_state[XST].currentmode.src, g_state[XST].currentmode.dst);
		x_log("-texture %i (%i textures)\n", g_state[XST].currentmode.text1, g_state[XST].active.textures);
	}
}
