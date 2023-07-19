// Hardware and Memory mapped io emulation (devices)

#pragma once

void    hw_check(void);
// these in cpu.c temporarily
void    hw_retrace(void);
void    hw_gfxframedone(void);

void    hw_rspcheck(void);

void    hw_memio(void);
int     hw_ismemiorange(dword addr);

void    hw_gfxthread(void);
