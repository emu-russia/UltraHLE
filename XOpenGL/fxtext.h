
#pragma once

// Texture management for the OpenGL backend.
//
// The original Glide library stored textures in the fixed TMU memory of the
// Voodoo card and used grTexDownloadMipMap/grTexSource to upload/select them.
// OpenGL manages texture memory itself, so this backend:
//
//   - keeps the host-side texture data (RGBA8888, all mip levels) exactly like
//     the original, including the x_opentexturedata() dynamic path,
//   - allocates one GL texture object per x_createtexture() handle,
//   - uploads the data with glTexImage2D() when the texture is first used.
//
// The TMU memory allocator (xt_memory/t_block) of the Glide backend is not
// needed and has been dropped.

#define X_MAX_BLOCKS 2048				// kept for xt_memory source compatibility

#define X_TEXPARTS 4

/// <summary>
/// Texture descriptor (OpenGL variant)
/// </summary>
typedef struct _xt_texture
{
	int state;				// Active number of the state (g_state). 0: texture is not used.
	int handle;				// X handle, == index into g_texture[]
	int width;				// pixels
	int height;				// pixels
	int format;				// X_* format + flags (X_CLAMP, X_MIPMAP, ...)
	int memformat;			// X_* mem format (always X_RGBA8888 here)
	int bytes;				// host data size (all levels)
	int levels;				// 1 - no mipmap
	int levelsloaded;		// bitmask of uploaded levels
	float xmul;				// texture width in vertex-texcoord units
	float ymul;				// texture height in vertex-texcoord units
	int lastframeused;		// frame number of last use (statistics)
	int reload;				// set when host data changed and must be re-uploaded
	void* data;				// host-side RGBA8888 data for all levels
	unsigned int gltexture;	// OpenGL texture object name (0 = not created yet)

	// statistics (kept for x_getstats)
	int size[X_TEXPARTS];		// resident bytes per part (0 = not on GPU)
	int usedsize[X_TEXPARTS];	// used bytes per part
} xt_texture;

/** Highest used X texture handle (index into g_texture). */
extern int g_lasttexture;
/** Texture descriptors indexed by X handle; entry 0 is unused. */
extern xt_texture g_texture[MAXTEXTURES];		// The 0th entry is not used

// Texture management (fxtext.c)
/**
 * Initializes texture management.
 */
void text_init();
/**
 * Releases all GL texture objects.
 */
void text_deinit();
/**
 * Returns the texel offset and size of a mip level inside the packed host buffer.
 * @param txt Texture descriptor.
 * @param level Mip level index.
 * @param xsize Receives the level width, or NULL.
 * @param ysize Receives the level height, or NULL.
 * @return Texel offset of the level in the packed buffer.
 */
int accesstexture(xt_texture* txt, int level, int* xsize, int* ysize);
/**
 * Allocates the host-side data buffer for all mip levels.
 * @param txt Texture descriptor.
 */
void text_allocdata(xt_texture* txt);
/**
 * Copies one mip level of RGBA8888 data into the host buffer.
 * @param txt Texture descriptor.
 * @param level Mip level index.
 * @param data Source pixel data.
 * @return Number of texels copied.
 */
int text_loadlevel(xt_texture* txt, int level, void* data);
/**
 * Frees the host data and GL texture object of a texture.
 * @param txt Texture descriptor.
 */
void text_freedata(xt_texture* txt);
/**
 * Deletes all GL texture objects, keeping the host data.
 */
void text_cleartexmem();
/**
 * Returns a pointer to the host-side texture data.
 * @param txt Texture descriptor.
 * @return Pointer to the texture data.
 */
void* text_opendata(xt_texture* txt);
/**
 * Marks the texture for re-upload and unbinds it from the active slots.
 * @param txt Texture descriptor.
 */
void text_closedata(xt_texture* txt);
/**
 * Updates the texture memory statistics at the end of a frame.
 * @return Always 0.
 */
int text_frameend();

/**
 * Uploads the texture data to an OpenGL texture object.
 * @param txt Texture descriptor.
 * @return 0 on success, -1 if the texture is not in use.
 */
// Upload a texture to OpenGL (called from fx.c when a texture becomes active).
// Returns 0 on success.
int fxuploadtexture(xt_texture* txt);
