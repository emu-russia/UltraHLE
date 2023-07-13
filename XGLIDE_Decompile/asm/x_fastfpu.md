# x_fastfpu

Now there is no point in controlling the FPU control word in any way, because the compiler optimizes everything on SSE.

The code contains an error: the uninitialized internal variable initdone is always equal to 0, but it is never set. It should be set right after `fstcw originalcontrol`.

And in general, the code looks very strange.

```
.text:00000268 _x_fastfpu      proc near
.text:00000268
.text:00000268 arg_0           = dword ptr  4
.text:00000268
.text:00000268                 cmp     ds:?initdone@?1??x_fastfpu@@9@9, 0 ; `x_fastfpu'::`2'::initdone
.text:0000026F                 jnz     short loc_27A
.text:00000271                 db      3Eh
.text:00000271                 fstcw   word ptr ?originalcontrol@?1??x_fastfpu@@9@9 ; `x_fastfpu'::`2'::originalcontrol
.text:00000279                 wait
.text:0000027A
.text:0000027A loc_27A:                                ; CODE XREF: _x_fastfpu+7↑j
.text:0000027A                 cmp     [esp+arg_0], 0
.text:0000027F                 mov     eax, ?state@?1??x_fastfpu@@9@9 ; `x_fastfpu'::`2'::state
.text:00000284                 jz      short loc_289
.text:00000286                 inc     eax
.text:00000287                 jmp     short loc_28E
.text:00000289 ; ---------------------------------------------------------------------------
.text:00000289
.text:00000289 loc_289:                                ; CODE XREF: _x_fastfpu+1C↑j
.text:00000289                 test    eax, eax
.text:0000028B                 jle     short loc_28E
.text:0000028D                 dec     eax
.text:0000028E
.text:0000028E loc_28E:                                ; CODE XREF: _x_fastfpu+1F↑j
.text:0000028E                                         ; _x_fastfpu+23↑j
.text:0000028E                 mov     ?state@?1??x_fastfpu@@9@9, eax ; `x_fastfpu'::`2'::state
.text:00000293                 test    eax, eax
.text:00000295                 mov     eax, ds:?originalcontrol@?1??x_fastfpu@@9@9 ; `x_fastfpu'::`2'::originalcontrol
.text:0000029A                 jz      short loc_2B2
.text:0000029C                 and     eax, 0FFFFFCFFh
.text:000002A1                 or      eax, 3Fh
.text:000002A4                 mov     ds:?newcontrol@?1??x_fastfpu@@9@9, eax ; `x_fastfpu'::`2'::newcontrol
.text:000002A9                 db      3Eh
.text:000002A9                 fldcw   word ptr ?newcontrol@?1??x_fastfpu@@9@9 ; `x_fastfpu'::`2'::newcontrol
.text:000002B0                 wait
.text:000002B1                 retn
.text:000002B2 ; ---------------------------------------------------------------------------
.text:000002B2
.text:000002B2 loc_2B2:                                ; CODE XREF: _x_fastfpu+32↑j
.text:000002B2                 mov     ds:?newcontrol@?1??x_fastfpu@@9@9, eax ; `x_fastfpu'::`2'::newcontrol
.text:000002B7                 db      3Eh
.text:000002B7                 fldcw   word ptr ?newcontrol@?1??x_fastfpu@@9@9 ; `x_fastfpu'::`2'::newcontrol
.text:000002BE                 wait
.text:000002BF                 retn
.text:000002BF _x_fastfpu      endp
```
