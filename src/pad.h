// Pad emulation

#pragma once

void  pad_key(int key);          // tell a key has been pressed (keycodes as in console.h)
void  pad_misckey(int key);      // tell a key has been pressed (keycodes as in console.h)
void  pad_frame(void);           // call this every frame (pad centering and stuff)
dword pad_getdata(int pad);
void  pad_writedata(dword addr); // write pad state to a memory location
void  pad_drawframe(void);

void  pad_enablejoy(int enable);
