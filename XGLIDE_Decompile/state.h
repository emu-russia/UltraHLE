
#pragma once

/// <summary>
/// Contains pixel output and texturing modes. What in modern video cards is essentially realized with the use of pixel shaders.
/// </summary>
typedef struct _xt_rendmode
{
	int mask;		// 291
	int masktst;	// 292
	int colortext1;		// 293
	int text1text2;		// 294
	int zbias;		// 295
	int fogtype;	// 296
	float fogmin;	// 297
	float fogmax;	// 298
	float fogcolor[4];	// 299(r), 300(g), 301(b), 302(a)
	int dither;		// 303
	int src;		// g_state[304]
	int dst;		// g_state[305]
	int sametex;	// g_state[306]
	FxU32 stwhint;		// 307
	float alphatest;	// g_state[308]
	int textures;		// 309
	int text1;			// 310
	int text2;			// 311
	uint32_t envc;		// 312 - combined env color
	float env[4];		// 313(r), 314(g), 315(b), 316(a)
} xt_rendmode;

typedef struct _xt_state		// 159 dwords
{
	int used;		// 159  1: This state is the current state in use
	int tmus;		// g_state[160]
	void* hdc;		// 161
	void* hwnd;		// 162
	int xs;			// g_state[163]
	int ys;			// g_state[164]
	int buffers;	// 165
	int vsync;		// 166
	int frame;		// 167
	uint32_t error;		// 168  Contains a mask of various errors

	// This block contains what is implemented in modern video cards using vertex shaders

#pragma region "Xform"

	float matrix[4 * 4];		// Modelview: 169, 170, 171, 172,   173, 174, 175, 176,   177, 178, 179, 180,   181, 182, 183, 184
	float xform[4 * 4];			// Combined: 185, 186, 187, 188,   189, 190, 191, 192,   193, 194, 195, 196,    197, 198, 199, 200
	float camxform[4 * 4];		// x_cameramatrix: 201, 202, 203, 204,  205, 206, 207, 208,    209, 210, 211, 212,   213, 214, 215, 216
	float projxform[4 * 4];		// Projection: 217, 218, 219, 220,   221, 222, 223, 224,   225, 226, 227, 228,   229, 230, 231, 232
	int campresent;		// 233

	int projchanged;	// 234
	int matrixnull;		// 235
	int projnull;		// 236
	float xmin;			// 237
	float xmax;			// 238
	float ymin;			// 239
	float ymax;			// 240
	float znear;		// g_state[241]
	float zfar;			// g_state[242]
	float zdecal;		// g_state[243]
	float invzfar;		// 244
	float invznear;		// 245
	int xformmode;		// 246
	int usexformmode;	// 247
	int view_x0;		// 248
	int view_x1;		// 249
	int view_y0;		// 250
	int view_y1;		// 251

	float projxmul;		// 252
	float projxadd;		// 253
	float projymul;		// 254
	float projyadd;		// 255
	float texturexmul;	// 256
	float textureymul;	// 257
	
#pragma endregion "Xform"

	uint32_t send;		// 258  - wtf? ("s"omething end?)
	int geometryon;		// 259
	int geometryoff;	// 260

	xt_rendmode active;		// [263 ... 288]   Actual set modes, corresponding to what was set in Glide API. The values get here from `currentmode`.

	int setnew;		// 289  -- new geometry
	int changed;	// 290   -- mode changed

	xt_rendmode currentmode;	// [291 ... 316]  Used by X api calls to set new values for different modes. But they are not set immediately yet.

	int geometry;		// 317

} xt_state;

// Error mask for g_state.error

#define X_ERROR_TOO_MANY_TEXTURES 0x10000000	// Too many textures (see x_createtexture)
#define X_ERROR_GET_TEXTURE 0x20000000			// Failed to get texture pointer in `texture_get`
