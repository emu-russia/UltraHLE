// Hardware and Memory mapped io emulation (devices)

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void    hw_init(void);

void    hw_check(void);
// these in cpu.c temporarily
void    hw_retrace(void);
void    hw_gfxframedone(void);

void    hw_rspcheck(void);

void    hw_memio(void);
int     hw_ismemiorange(dword addr);

void    hw_gfxthread(void);

void    hw_selectpad(int pad);

#ifdef __cplusplus
};
#endif
