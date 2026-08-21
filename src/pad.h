// Pad emulation

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void  pad_frame(void);           // call this every frame (pad centering and stuff)
uint32_t pad_getdata(int pad);
void  pad_writedata(uint32_t addr); // write pad state to a memory location
void  pad_drawframe(void);

void  pad_enablejoy(int enable);

// A temporary solution for now. The controller emulation code is for some reason heavily tied to console.c, we need to untangle this doshirak.

extern int joyactive;
extern int mouseactive;
extern int mousedisablecnt;

#ifdef __cplusplus
};
#endif
