// Texture management and texture upload (OpenGL backend)
//
// The original Glide library stored textures in the fixed TMU memory of the
// Voodoo card. OpenGL manages texture memory itself, so this backend stores
// the texture data on the host side (RGBA8888) and uploads it to a GL texture
// object when the texture first becomes active.

#include "pch.h"

int g_lasttexture;
xt_texture g_texture[MAXTEXTURES];		// The 0th entry is not used

void text_init()
{
	x_log("Texture memory: OpenGL managed\n");
}

void text_deinit()
{
	// Release all GL texture objects
	if (g_lasttexture >= 1)
	{
		for (int i = 1; i <= g_lasttexture; i++)
		{
			if (g_texture[i].state && g_texture[i].gltexture)
			{
				glDeleteTextures(1, &g_texture[i].gltexture);
				g_texture[i].gltexture = 0;
				g_texture[i].size[0] = 0;
				g_texture[i].reload = 1;
			}
		}
	}
}

/// <summary>
/// Returns the texel offset of <paramref name="level"/> inside the packed
/// host-side buffer and the size of that level.
/// </summary>
int accesstexture(xt_texture* txt, int level, int* xsize, int* ysize)
{
	int w, h, off;
	int i;

	if (level < 0 || level >= txt->levels)
	{
		x_fatal("access texture illegal level");
		return 0;
	}

	w = txt->width;
	h = txt->height;
	off = 0;
	for (i = 0; i < level; i++)
	{
		off += w * h;
		if (w > 1) w >>= 1;
		if (h > 1) h >>= 1;
	}
	if (xsize) *xsize = w;
	if (ysize) *ysize = h;
	return off;
}

void text_allocdata(xt_texture* txt)
{
	int w, h, m, levels, i, cw, ch;

	w = txt->width;
	h = txt->height;
	m = (w > h) ? w : h;

	// Number of mipmap levels (1 = no mipmaps)
	levels = 1;
	while (m > 1)
	{
		m >>= 1;
		levels++;
	}
	if (!(txt->format & X_MIPMAP))
		levels = 1;

	txt->levels = levels;
	txt->memformat = X_RGBA8888;
	txt->xmul = (float)w;
	txt->ymul = (float)h;

	// Host-side RGBA8888 buffer containing all levels, largest first
	txt->bytes = 0;
	cw = w;
	ch = h;
	for (i = 0; i < levels; i++)
	{
		txt->bytes += cw * ch * 4;
		if (cw > 1) cw >>= 1;
		if (ch > 1) ch >>= 1;
	}

	if (txt->data)
		x_free(txt->data);
	txt->data = x_alloc(txt->bytes);

	txt->reload = 1;
	txt->gltexture = 0;
	for (i = 0; i < X_TEXPARTS; i++)
	{
		txt->size[i] = 0;
		txt->usedsize[i] = 0;
	}
}

int text_loadlevel(xt_texture* txt, int level, void* data)
{
	int xsize, ysize, off, need;

	if (level > 31)
		return 0;

	off = accesstexture(txt, level, &xsize, &ysize);
	need = off * 4 + xsize * ysize * 4;
	if (need > txt->bytes)
	{
		// levels were grown after text_allocdata (x_loadtexturelevel can do that)
		txt->data = x_realloc(txt->data, need);
		txt->bytes = need;
	}

	// Data comes in as 32-bit RGBA (little endian: R,G,B,A bytes)
	memcpy((char*)txt->data + off * 4, data, xsize * ysize * 4);

	txt->reload = 1;
	return xsize * ysize;
}

void text_freedata(xt_texture* txt)
{
	if (txt->gltexture)
	{
		glDeleteTextures(1, &txt->gltexture);
		txt->gltexture = 0;
	}
	if (txt->data)
		x_free(txt->data);
	memset(txt, 0, sizeof(xt_texture));
}

// Delete all GL texture objects (unload from the accelerator), keep host data.
void text_cleartexmem()
{
	if (g_lasttexture >= 1)
	{
		for (int i = 1; i <= g_lasttexture; i++)
		{
			if (g_texture[i].state && g_texture[i].gltexture)
			{
				glDeleteTextures(1, &g_texture[i].gltexture);
				g_texture[i].gltexture = 0;
				g_texture[i].size[0] = 0;
				g_texture[i].reload = 1;
			}
		}
	}
	x_log("texture: clearspace 1..%i freed all memory\n", g_lasttexture);
}

void* text_opendata(xt_texture* txt)
{
	return txt->data;
}

void text_closedata(xt_texture* txt)
{
	txt->reload = 1;
	if (txt->handle == g_state[XST].active.text1)
		g_state[XST].active.text1 = 0;
	if (txt->handle == g_state[XST].active.text2)
		g_state[XST].active.text2 = 0;
}

int text_frameend()
{
	int result = 0;

	g_stats.text_resident = 0;
	g_stats.text_used = 0;
	for (int i = 1; i <= g_lasttexture; i++)
	{
		if (g_texture[i].state)
		{
			g_stats.text_resident += g_texture[i].size[0];
			if (g_state[XST].frame - 1 <= g_texture[i].lastframeused)
				g_stats.text_used += g_texture[i].usedsize[0];
			g_texture[i].usedsize[0] = 0;
		}
	}
	return result;
}

// ---------------------------------------------------------------------------
// OpenGL upload
// ---------------------------------------------------------------------------

// Upload one mip level.
//
// The emulator provides the texture with row 0 = top of the image and maps
// the vertex texture coordinate t=0 to the top of the screen (the N64
// convention). OpenGL samples texture coordinate v=0 from the first row of
// the data, so the rows are uploaded as-is - no vertical flip is needed.
static void upload_level(xt_texture* txt, int level)
{
	int w, h, off;
	const unsigned char* src;
	int rowbytes;

	off = accesstexture(txt, level, &w, &h);
	if (!w || !h)
		return;

	src = (const unsigned char*)txt->data + off * 4;
	rowbytes = w * 4;

	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, src);
}

/// <summary>
/// Make sure <paramref name="txt"/> is uploaded to OpenGL. Called when the
/// texture is selected (mode_loadtexture / mode_loadmultitexture).
/// </summary>
int fxuploadtexture(xt_texture* txt)
{
	unsigned int tex;
	int level, alllevels;

	if (txt->state == 0)
		return -1;

	tex = txt->gltexture;
	if (!tex)
	{
		glGenTextures(1, &tex);
		txt->gltexture = tex;
	}

	glBindTexture(GL_TEXTURE_2D, tex);

	if (txt->reload)
	{
		// Upload all levels that have data
		for (level = 0; level < txt->levels; level++)
		{
			if (txt->levelsloaded & (1 << level))
				upload_level(txt, level);
		}
		txt->reload = 0;

		// Filtering / clamping
		alllevels = (1 << txt->levels) - 1;
		if ((txt->levelsloaded & alllevels) == alllevels && txt->levels > 1)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (txt->format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (txt->format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR);
		}

		if (txt->format & X_CLAMP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (txt->format & X_CLAMPNOX) ? GL_CLAMP_TO_EDGE : GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (txt->format & X_CLAMPNOY) ? GL_CLAMP_TO_EDGE : GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		txt->lastframeused = g_state[XST].frame;
		txt->size[0] = txt->bytes;
		txt->usedsize[0] = txt->bytes;
		g_stats.text_uploaded += txt->bytes;
	}
	else
	{
		// already uploaded this frame? just mark used
		txt->lastframeused = g_state[XST].frame;
		txt->usedsize[0] = txt->bytes;
	}

	return 0;
}
