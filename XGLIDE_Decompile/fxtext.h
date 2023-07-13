
#pragma once

int newblock(int a1);
DWORD* addbefore(DWORD* a1, DWORD* a2);
int addafter(int a1, DWORD* a2);
DWORD* removeblk(DWORD** a1);
DWORD* memory_clear(DWORD* a1);
DWORD* memory_alloc(int a1, int a2, DWORD* a3);
int memory_free(int a1, DWORD** a2);
signed int* memory_create(signed int a1, signed int a2);
int memory_delete(int a1);
int freetexmem(DWORD* a1);
int makespace();
int clearspace();
int picktmu();
signed int fxloadtexturepart(DWORD* a1, int a2);
signed int fxloadtexture_single(DWORD* a1);
int fxloadtexture_trilin(DWORD* a1);
int fxloadtexture_multi(DWORD* a1, DWORD* a2);
signed int* text_init();
void text_deinit();
int accesstexture(DWORD* a1, int a2, signed int* a3, signed int* a4);
int text_allocdata(int a1);
unsigned int text_loadlevel(DWORD* a1, int a2, unsigned int a3);
int text_freedata(DWORD* a1);
int text_cleartexmem();
int text_opendata(int a1);
int text_closedata(int a1);
int text_frameend();
