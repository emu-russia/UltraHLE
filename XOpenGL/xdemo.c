/*
** xdemo.c - demonstration / test program for the XOpenGL library.
**
** Opens a window, initializes the X library (OpenGL backend) and renders:
**   - a textured 3D cube (perspective projection, z-buffer, gouraud shading)
**   - a textured 2D overlay (ortho path used by the emulator for 2D)
**   - alpha blending and different combine modes
**
** This exercises the same X API calls the emulator (rdp.c) uses.
*/

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "x.h"

/* the library does not define these (the consumer does, see src/dlist.c) */
xt_data g_data;
xt_pos g_pos;

#define WIN_W 640
#define WIN_H 480

static HWND hwnd;
static int g_context = -1;

/* ------------------------------------------------------------------ */
/* window                                                              */
/* ------------------------------------------------------------------ */

static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wp == VK_ESCAPE)
			DestroyWindow(h);
		return 0;
	}
	return DefWindowProc(h, msg, wp, lp);
}

static int create_window(HINSTANCE inst)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "XDemoGL";
	if (!RegisterClass(&wc))
		return -1;
	hwnd = CreateWindow("XDemoGL", "XOpenGL demo", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIN_W, WIN_H, NULL, NULL, inst, NULL);
	if (!hwnd)
		return -1;
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	return 0;
}

/* ------------------------------------------------------------------ */
/* texture helpers                                                     */
/* ------------------------------------------------------------------ */

/* fill an RGBA8888 buffer with a checkerboard */
static void make_checker(unsigned char* buf, int w, int h, int square)
{
	int x, y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			int c = ((x / square) + (y / square)) & 1;
			int i = (y * w + x) * 4;
			buf[i + 0] = c ? 0x00 : 0xff;   /* R */
			buf[i + 1] = c ? 0xff : 0x00;   /* G */
			buf[i + 2] = c ? 0x00 : 0x80;   /* B */
			buf[i + 3] = 0xff;              /* A */
		}
	}
}

/* fill an RGBA8888 buffer with a smooth gradient (tests filtering) */
static void make_gradient(unsigned char* buf, int w, int h)
{
	int x, y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			int i = (y * w + x) * 4;
			buf[i + 0] = (unsigned char)(x * 255 / w);
			buf[i + 1] = (unsigned char)(y * 255 / h);
			buf[i + 2] = (unsigned char)((x + y) * 255 / (w + h));
			buf[i + 3] = 0xff;
		}
	}
}

/* ------------------------------------------------------------------ */
/* cube                                                                */
/* ------------------------------------------------------------------ */

static float rot = 0.0f;

static void draw_cube(void)
{
	/* cube vertices in the emulator convention: pos = (NDC*x_w, NDC*y_w, w) */
	static const float v[8][3] = {
		{ -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
		{ -1, -1, 1 },  { 1, -1, 1 },  { 1, 1, 1 },  { -1, 1, 1 },
	};
	/* 6 faces (quads) */
	static const int faces[6][4] = {
		{ 0, 1, 2, 3 }, { 1, 5, 6, 2 }, { 5, 4, 7, 6 },
		{ 4, 0, 3, 7 }, { 3, 2, 6, 7 }, { 4, 5, 1, 0 },
	};
	static const float facecol[6][3] = {
		{ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
		{ 1, 1, 0 }, { 1, 0, 1 }, { 0, 1, 1 },
	};
	const float scale = 0.28f;   /* NDC half-size */
	const float wbase = 5.0f;    /* depth */
	int f, j;
	float s = sinf(rot), c = cosf(rot);
	float xv[8][3];

	/* rotate the cube around Y */
	for (j = 0; j < 8; j++)
	{
		float wx = v[j][0] * c - v[j][2] * s;
		float wz = v[j][0] * s + v[j][2] * c;
		float w = wbase + v[j][2];     /* depth (w) */
		xv[j][0] = wx * scale * w;     /* clip space: ndc * w */
		xv[j][1] = v[j][1] * scale * w;
		xv[j][2] = w;
	}

	x_begin(X_QUADS);
	for (f = 0; f < 6; f++)
	{
		for (j = 0; j < 4; j++)
		{
			int idx = faces[f][j];
			float tu = (j == 0 || j == 3) ? 0.0f : 1.0f;
			float tv = (j == 0 || j == 1) ? 0.0f : 1.0f;
			x_vxcolor(facecol[f][0], facecol[f][1], facecol[f][2]);
			x_vxtex(tu, tv);
			x_vxpos(xv[idx][0], xv[idx][1], xv[idx][2]);
		}
	}
	x_end();
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int tex_checker, tex_grad;
	unsigned char* buf;
	MSG msg;
	int frame = 0;

	if (create_window(inst) != 0)
		return 1;

	x_init();
	x_log("XDemo: %s\n", x_version());

	g_context = x_open(NULL, hwnd, WIN_W, WIN_H, 2, 1);
	if (g_context < 0)
	{
		x_log("x_open failed\n");
		return 1;
	}
	x_log("context %i\n", g_context);

	/* default rendering state, same as the emulator */
	x_projection(90.0f, 0.9f, 32768.0f);
	x_viewport(0, 0, (float)WIN_W - 1, (float)WIN_H - 1);
	x_projmatrix(NULL);
	x_matrix(NULL);
	x_reset();
	x_geometry(0);
	x_flush();

	/* textures */
	buf = (unsigned char*)x_alloc(256 * 256 * 4);
	make_checker(buf, 256, 256, 32);
	tex_checker = x_createtexture(X_RGBA5551 | X_CLAMP, 256, 256);
	x_loadtexturelevel(tex_checker, 0, (char*)buf);
	make_gradient(buf, 128, 128);
	tex_grad = x_createtexture(X_RGBA8888 | X_CLAMP, 128, 128);
	x_loadtexturelevel(tex_grad, 0, (char*)buf);
	x_free(buf);

	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;

		x_clear(1, 1, 0.02f, 0.02f, 0.06f);

		/* ---- 3D: textured cube (z-buffered) ---- */
		x_viewport(0, 0, (float)WIN_W - 1, (float)WIN_H - 1);
		x_projmatrix(NULL);
		x_matrix(NULL);
		x_reset();
		x_geometry(0);
		x_mask(X_ENABLE, X_ENABLE, X_ENABLE);
		x_zdecal(1.0f);
		x_blend(X_ONE, X_ZERO);
		x_alphatest(1.0f);
		x_combine(X_MUL);
		x_texture(tex_checker);
		x_fog(X_DISABLE, 0, 0, 1, 1, 1);
		rot += 0.01f;
		draw_cube();
		x_flush();

		/* ---- 2D overlay: alpha-blended textured quad (top-right corner) ---- */
		x_viewport(0, 0, (float)WIN_W - 1, (float)WIN_H - 1);
		x_projmatrix(NULL);
		x_matrix(NULL);
		x_mask(X_ENABLE, X_DISABLE, X_DISABLE);
		x_blend(X_ALPHA, X_INVOTHERALPHA);
		x_alphatest(0.05f);
		x_combine(X_TEXTURE);
		x_texture(tex_grad);
		x_begin(X_QUADS);
		x_vxtex(0, 0); x_vxpos(0.15f, 0.55f, 1.0f);
		x_vxtex(1, 0); x_vxpos(0.55f, 0.55f, 1.0f);
		x_vxtex(1, 1); x_vxpos(0.55f, 0.85f, 1.0f);
		x_vxtex(0, 1); x_vxpos(0.15f, 0.85f, 1.0f);
		x_end();
		x_flush();

		x_finish();
		frame++;
		if ((frame & 63) == 0)
			x_log("frame %i\n", frame);
	}

	x_deinit();
	return 0;
}
