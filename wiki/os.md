# Notes on Ultra OS emulation

## sym.c

TBD.

## patch.c

Contains an implementation of Stubs to patch OS/library calls. May also contain patches for game-specific routines (e.g. `p_golden1`).

The patch is quite simple:
- A special dummy opcode (OP_PATCH) is inserted at the beginning of the procedure
- The old opcode is saved
- As soon as the processor enters the patched procedure and executes the OP_PATCH opcode, Stub is called with the corresponding number.

A list of all Stubs is contained in the `patchtable` table.

The implementation of the dummy opcode is in `op_patch`.

Simple patches are implemented directly inside Stubs, more complex OS-calls are redirected to os.c

## os.c

TBD.

## Important calls

A patch with a number >= 10 is considered important.

List:
- osGetCount #48
- osInvalICache #38
- osMapTLB #24
- osCreateMesgQueue #13
- osCreateThread #10
- osGetTime #19
- osJamMesg #15
- osPhysicalToVirtual #29
- osRecvMesg #14
- osSendMesg #15
- osSetEventMesg #16
- osSetThreadPri #18
- osSetTimer #33
- osStartThread #11
- osStopThread #28
- osStopTimer #34
- osVirtualToPhysical #21
- osAiGetLength #31
- osAiSetFrequency #32
- osAiSetNextBuffer #30
- osSpTaskLoad #39
- osSpTaskStartGo #22
- osSpTaskYield #46
- osSpTaskYielded #47
- osViGetCurrentFramebuffer #20
- osViGetNextFramebuffer #20
- osCreateViManager #55
- osViSetEvent #17
- osViSwapBuffer #23
- osContStartQuery #42
- osContGetQuery #43
- osContStartReadData #26
- osContGetReadData #27
- osContInit #25
- osEPiStartDma #37
- osPiStartDma #12
