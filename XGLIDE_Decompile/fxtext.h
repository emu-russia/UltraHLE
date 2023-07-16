
#pragma once

#define X_MAX_BLOCKS 2048				// Maximum number of blocks that can be addressed by xt_memory

#pragma pack(push, 4)

/// <summary>
/// Texture block
/// </summary>
typedef struct _t_block		// 16 bytes
{
	struct _t_block* prev;
	struct _t_block* next;
	int base;
	int size;
} t_block;

/// <summary>
/// Texture memory map
/// </summary>
typedef struct _xt_memory	// 48 bytes
{
	int min;		// 0
	int max;		// 1
	t_block free;	// 2..5
	t_block used;   // 6..9
	void* table;	// 10
	int tableind;	// 11  - table index
} xt_memory;

#define X_TEXPARTS 4

/// <summary>
/// Texture descriptor
/// </summary>
typedef struct _xt_texture		// 152 bytes (38 dwords)
{
	int state;				// 0  - Active number of the state (g_state). 0: texture is not used.
	int handle;				// 1
	int width;				// 2
	int height;				// 3
	int format;				// 4
	int memformat;			// 5
	int bytes;				// 6
	int levels;				// 7  (1 - no mipmap)
	int levelsloaded;		// 8   mask
	int xmul;				// 9
	int ymul;				// 10
	int lastframeused;		// 11
	int reload;				// 12
	GrTexInfo ti;			// 13, 14, 15, 16, 17[data]
	int	size[X_TEXPARTS];		// 18  19  20  21 
	FxU32 base[X_TEXPARTS];		// 22  23  24  25
	int	tmu[X_TEXPARTS];		// 26  27  28  29
	int xblock[X_TEXPARTS];		// 30  31  32  33      TODO: Make like t_block
	int	usedsize[X_TEXPARTS];	// 34  35  36  37
} xt_texture;

#pragma pack(pop)

extern int g_lasttexture;
extern xt_texture g_texture[MAXTEXTURES];

int newblock(int m);
DWORD* addbefore(DWORD* b, DWORD* t);
int addafter(int b, DWORD* t);
DWORD* removeblk(DWORD** b);
DWORD* memory_clear(DWORD* memory);
DWORD* memory_alloc(int memory, int size, int* base);
int memory_free(int memory, DWORD** handle);
xt_memory* memory_create(int min, int max);
void memory_delete(xt_memory* memory);
void freetexmem(DWORD* txt);
void makespace();
void clearspace();
int picktmu();
int fxloadtexturepart(xt_texture* txt, int texturepart);
int fxloadtexture_single(xt_texture* txt);
int fxloadtexture_trilin(xt_texture* txt);
int fxloadtexture_multi(xt_texture* txt1, xt_texture* txt2);
void text_init();
void text_deinit();
int accesstexture(xt_texture* txt, int level, int* xsize, int* ysize);
int text_allocdata(int txt);
int text_loadlevel(xt_texture* txt, int level, unsigned int data);
void text_freedata(xt_texture* txt);
void text_cleartexmem();
void* text_opendata(xt_texture* txt);
void text_closedata(xt_texture* txt);
int text_frameend();
