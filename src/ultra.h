/****************************************************************************
** UltraHLE (temp.name) - Ultra64 High Level Emulator
** Copyright (C) 1998 Epsilon & RealityMan
*/
#ifndef _ULTRA_H_
#define _ULTRA_H_

/*

Main header file. This is also the main documentation file :)

All emulation module interfaces and types are combined to this.
There are some other header files, used only in a few modules.
Probably all ui stuff should get its own header(s) and keep
this as the emulator core interface.

Actually, the UI should at first only use:
- command(...) in cmd.c for doing things, and
- outputhook(...) in main.c for reading the results.
If it get's more complex it might need direct access to stuff
in this header though.

List of modules and their descriptions:

Debug user interface:
- MAIN.C     main
- DEBUGUI.C  debugui text window updates and keyboard handling
- DISASM.C   disassembler for code window and code dumps
- CONSOLE.C  module for using windows consoles (also CONSOLE.H)

Debugui commands:
s CMD.C      main command file

Cpu emulation:
s CPU.C      main interface file, calls the other two and components
g CPUC.C     c-emulator (slow) with better compatibility
s CPUA.C     asm-compiler (fast) with less compatibility
s MEM.C      memory emulation (virtual memory and related stuff too)

Emulation components:
g HW.C       hardware emulation (interrupts, devices)
- PAD.C      pad emulation
s OS.C       os-emulation (also OS*.H)
s DLIST.C    display lists (calls RDP)
s RDP.C      RDP emulation (also RDP.H)
s SLIST.C    sound lists

Miscellaneous:
- LOG.C      logging/print routines used by other modules
- BOOT.C     boot/initialization
- CART.C     cart loading and flipping
s SYM.C      symbol table and os-routine search
s PATCH.C    forwarding of patched routines to os

I've marked modules with 's' and 'g' for my ideas of main responsibility
to avoid us modifying much the same modules. The debugui probably remains
my responsibility, but you'll add your own ui which mostly replaces it.
I guess you should rewrite main.c but keep the rest separate. First look
over the source and planned the ui/hw integration. Then tell me what you
are going to change, and at that point we'll "sync" the sources and agree
on what not to change.

--

Here's some misc info in random order, I was planning on writing more but
then I though you'll probably want the source earlier with less docs than
the other way around :) Just do an email with a lot of questions and we'll
sort things out that way. If I try to write a manual for this thing, it'll
take ages.

The PATCH and OS modules should go away once hardware emulation is
working properly. The os-routine search probably should be kept for
creating symbols to ease debugging, but not for patching. For this reason
I haven't cleaned them up and they are probably most confusing.

All windows io (console output / keyboard input) is done in console.c.
The keycodes used are custom defined in console.h. Only keys really
used are defined and mapped from virtual key codes.

The X-Engine is the Glide interface I use (XGLIDE.LIB & X.H). I cannot
distribute it in source form, and in theory not even in library/header
form, but here I have to make a practical exception to save time. It
will be removed as we convert to OGL, but I didn't have time for that
yet. But anyway it's only used in RDP.C so you can pretty much ignore it.

Memory system uses separate lookups for reading and writing. This means
you can map different pages for processor to read and write. For example
the NULL page is write protected by mapping a different read and write
page. COP0/MMU should use the same mapping mechanism to maintain
compatibility, it just needs to generate mem_map* calls based on COP0
instructions.

When you need to access the main memory, it's best to use mem_read/write
routines. There are also mem_readrange/mem_writerange for copying ranges,
but they are less used/tested. Note that mem_read reads the PATCHED
instructions when compiler is enabled. If you want to read unmodified
memory, use mem_readop(). It won't always read data correctly though
(if data looks like the reserved patch instruction!).

When doing hw.c, just access the hardware memory directly using the RPI,
WPI, etc macros (RPI=pi regs for cpu to read, WPI=regs for cpu to write).
For DMA transfers, copy the code from
osPiStartDMA, it's tested and handles the special nonaligned length/base
cases properly (although not optimally). Since the HW-pages are separate
for read/write, you can check for register writes like this:
  if(WPI[2]!=NULLFILL) { dmaxfer(); WPI[2]=NULLFILL; }
See hw_checkoften() for an example of PIDMA emulation (not tested/not
compilete, just an example). Also note that I created a new logh(...)
call just for you for easy logging of hw-related messages (so they
can be enabled/disabled separately with command 'hw').

Execution system: The code is executed in 'bursts' controlled by cpu.c.
The st.bailout value is set to burst size, and then either cpuc or cpua
is called. They do their work and should decrement bailout on every
instruction. When bailout becomes negative they return to cpu.c. The
slow emu returns on next instruction, but the fast emu compiles
instruction groups, and returns at end of group.

The new compiler is a bit faster but more importantly more compatible.
Since it now detects self-modifying code (or uncompressed code) it
now works on Wave and Zelda without the older versions DMA hacks.
There might still be bugs in it (but there could also be bugs in
sgo). So usually a good idea to just be brave and try 'go' first,
it has a good chance of working, and shows results or bugs a lot
faster than 'sgo'. Still, call/ops tracing won't work with 'go',
but os-logging does and it tells a lot already without flooding
too much.

Use exception/error/warning (in log.c) to break execution anytime. They
set bailout to 0 to exit to cpu.c and breakout to 1 to force cpu.c stop.
So they print the message immediately, and breakout as soon as possible.

NOTE! The default is now that only exceptions stop, errors/warnings are
only printed. The command 'stop' can be used to adjust ('stop 0' stops
only on exceptiosn, 'stop 1' also on errors, 'stop 2' also on warnings).

The RESERVED slots in different global structs that are saved. The idea
is that if you need to add just a few variables, you can take the space
from RESERVED arrays and thus maintain binary compatibility in the
structure, so that loading old states still works.

Cpu time is also tracked with bailout (by seeing how much it decreased)
so breakpoints add errors to cputime. Also the compiler doesn't count
instructions exactly. In practice this doesn't matter since no n64 code
is cycle-accurate. The main function of bailout is to make sure we can
guarantee periodic execution of hw.c and checking of keyboard (esc) to
avoid infinite loops.

Use st.cputime for timing interrupts and stuff, although timers should
probably be synced to realtime. Also, since we skip idletime on the cpu,
cputime actually goes too slowly (it should increase by the number of
ops executed in 1/25th sec, not by number of ops really used in the frame).
Doesn't seem to be a problem, but might be in the future

At start of sym.c there is a 'disablepatches' array. Uncomment os-call
names in it to disable patching for those calls. This makes it easy
to replace calls one by one with their real nonemulated versions. The
oscalls.h and ospatches.h files are autogenerated files for the os-call
detection, better not modify them.

Greatly enhanced breakpoint support in sgo. You can define up to 16
breakpoints with different conditions. No ui to define them yet though.
The 'n' command does use 3 breakpoints though to step nicely over
routines while still doing the jumps right (F7=n now).

To create more complex breakpoints for testing weird problesm more easily,
you can patch code in cpu.c. See the cpu_notify_* routines, which contain
the current breakpoint code.

The disassembly window now shows symbolic call names (don't remember
if they were in the version you got) and the symboling name of PC
in the code-window bar. It also displays JUMP/CALL/RET in bright white
texts at right end of the line after BNE calls to quickly show if the
CPU will jump or not. The dark symbols after dword opcode tell about
the compiler patches. 'a5' means here starts a 5 instruction group that
is compiled (a=asm). 'c3' means a 3 instr. group that is executeed in c
(some instruction in it not supported by asm). 'n' means a group not
yet analyzed, 'p' means a group with an os-patch.

The version.h is related to a simple backup system I have, and is updated
when I take a numbered backup of the code. The last part of the version
number is that number.

More notes intermixed with the definitions, read on.

*/

#ifndef RELEASE
#define RELEASE 0
#endif

/****************************************************************************
** Important constants
*/

// how often to:
#define CYCLES_BURST        5000      // break burst (=burst length)
#define CYCLES_CHECKOFTEN   5000      // check common hw
#define CYCLES_CHECKSELDOM  50000     // check not so common hw
#define CYCLES_CHECKUI      200000    // check user interface
#define CYCLES_RETRACE      25000     // min cycles between retraces

// how often to update console:
#define CYCLES_PRINTSTART   2000000   // before first gfx frame (startup)
#define CYCLES_PRINTSLOW    5000000   // in slow (cpus) mode
#define CYCLES_PRINTFAST    50000000  // in fast (cpua) mode

// counts for cmd() when it does different execs
#define CYCLES_STEP         100000000 // max cycles to execute for 'n' 'nr' etc step commands
#define CYCLES_INFINITE     1000000000000000 // a big enough number so 'go' won't stop too soon

// main memory size
//#define RDRAMSIZE           0x800000   //    =8MB
#define RDRAMSIZE           0x400000   //    =4MB

// compiled code storage
// These parameters allow for about 1MB of compiled MIPS code,
// distributed into up to 65536 groups (allowing 4 ops/group)
#define COMPILER_CODEMAX    0x500000   //    =5MB
#define COMPILER_GROUPMAX   0x10000    // *16=1MB

/****************************************************************************
** Disable some warnings
*/

#pragma warning(disable:4244)  // conversion, possible loss of data
#pragma warning(disable:4018)  // signed/unsigned mismatch

/****************************************************************************
** Headers
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>

/****************************************************************************
** Basic types
*/

typedef          __int64    qint;
typedef unsigned __int64    qword;
typedef unsigned int        dword;
typedef unsigned short      word;
typedef unsigned char       byte;

// quadword register
typedef union
{
    qword q;
    dword d;
    dword d2[2];
} qreg;

// floating point register (single)
//   Double registers are stored in two consecutive singles as in R4300 using
// casts. The register order is luckily same as for X86 double ordering!
typedef union
{
    int   d;
    float f;
} freg;

/****************************************************************************
** os-routines and types
*/

#include "os.h"

#include "timer.h"

#include "console.h"

/****************************************************************************
** Macros for handling opcodes and bits in general
*/

#define SIGNEXT16(a) ((((int)(a))<<16)>>16)
#define SIGNEXT8(a)  ((((int)(a))<<24)>>24)
#define FLIP32(x)    (((x)>>24)&0x000000ff \
                     |((x)>>8 )&0x0000ff00 \
                     |((x)<<8 )&0x00ff0000 \
                     |((x)<<24)&0xff000000)
#define FLIP32B(x)   (((x)>>16)&0x0000ff00 \
                     |((x)>>16)&0x000000ff \
                     |((x)<<16)&0xff000000 \
                     |((x)<<16)&0x00ff0000)

#define FIELD(x,b,w) ( (x>>b) & ((1<<w)-1) )

#define OP_OP(a)     ((a>>26)&0x3f)
#define OP_RS(a)     ((a>>21)&0x1f)
#define OP_RT(a)     ((a>>16)&0x1f)
#define OP_RD(a)     ((a>>11)&0x1f)
#define OP_SHAMT(a)  ((a>>6 )&0x1f)
#define OP_FUNC(a)   ((a>>0 )&0x3f)
#define OP_IMM(a)    ((a>>0 )&0xffff)
#define OP_TAR(a)    ((a>>0 )&0x3ffffff)
#define OP_IMM24(a)  ((a>>0 )&0x0ffffff)

#define OP_FD(a)     OP_SHAMT(a);
#define OP_FS(a)     OP_RD(a);
#define OP_FT(a)     OP_RT(a);

// Special patch opcode (reserved in real processor)
#define OP_PATCH     28
#define OP_GROUP     29
#define PATCH(x)     ((OP_PATCH<<26)+(x))
#define GROUP(x)     ((OP_GROUP<<26)+(x))

/****************************************************************************
** Main emulator state (saved/loaded)
** Also macros for accessing registers by name
*/

// breakpoint
typedef struct
{
    int    type;
    dword  addr;
    dword  data;
    int    RESERVED;
} Break;

#define BREAK_PC         0x00    // break if pc==addr
#define BREAK_MEM        0x01    // break if mem[addr] accessed
#define BREAK_MEMW       0x02    // break if mem[addr] written
#define BREAK_MEMR       0x03    // break if mem[addr] read
#define BREAK_MEMDATA    0x04    // break if mem[addr]!=data
#define BREAK_FWBRANCH   0x05    // break if branch forward
// these break at next event after cpu_exec call.
#define BREAK_NEXTRET    0x08    // routine return
#define BREAK_NEXTCALL   0x09    // routine call
#define BREAK_NEXTTHREAD 0x0a    // threadchange
#define BREAK_NEXTFRAME  0x0b    // framesync
#define BREAK_MSG        0x0c    // break if send/recv on queue data
// or this to disable breakpoint
#define BREAK_DISABLE    0x1000

extern char *breakname[]; // table of above defines in text (cpu.c)

// The spare RESERVED arrays are meant for maintaining save compatibility.
// If you need to add new variables to state, put them at the end and
// decrease RESERVED count by 1. In this way the format of struct remains
// compatible and loading old savefiles still works (the values are 0
// in old files). When there are a lot of additions things are reorganized
// and RESERVED resized back, losing compatibility at that point.

// The state is split into two parts from the variable _boundary_
// The first part is thread specific and is switched by os at threadswitch.
// The latter parts are same for all threads.

typedef struct
{
    // NOTE: bailout and pc used with direct offsets in inline assembly!!
    // instructions left before bailing out to cpu.c
    int    bailout;     // if <0 please return to cpu.c
    // cpu state
    dword  pc;          // program counter
    qreg   g[32];       // general registers
    qreg   mhi;         // MHI mul/div result reg
    qreg   mlo;         // MLO mul/div result reg
    // branching state (cpuc.c only, cpua.c never breaks when branchdelay>0)
    int    branchdelay; // when goes from 1->0 branch to branchto
    dword  branchto;    // address to branch to
    int    branchtype;  // BRANCH_*
    int    expanded64bit;
    int    memiodetected;
    dword  RESERVED1[2];
    // mmu state
    qreg   mmu[32];     // mmu registers
    dword  RESERVED2[4];
    // fpu state
    freg   f[32];       // cop1 (fpu) registers
    qword  fputmp;      // temporary used by cpua.c in generated code
    int    fputrue;     // fpu compare result true/false
    dword  RESERVED3[3];
    // thread state (os.c)
    qword  threadtime;  // instructions executed
    int    thread;      // active thread
    int    callnest;    // nesting in calls (for tracing)
    int    RESERVED4[16];
    // above fields change with thread
    int    _boundary_;

    // if set to 1 cpu.c stops executing
    int    breakout;

    // timekeeping
    qword  cputime;       // instructions executed
    qword  synctime;      // instructions executed when last framesync done
    qword  exectime;      // instructions executed when last cpu_exec call started
    int    frames;        // frames drawn (used for some timings)

    // os-emulation and threading
    int    trythread;   // prefer switch to this thread next
    qword  nextswitch;  // cputime for next task switch
    int    doframesync;
    int    avoidthread;
    int    hackcount;

    // patch tracking
    dword  firstpatch;
    dword  lastpatch;
    dword  patches;

    // breakpoints (do not work for cpua.c)
    // pretty versatile, but ui doesn't have a good interface to these yet
    Break  breakpoint[16];
    int    breakpoints;
    int    quietbreak; // set to 1 if you don't want printing at trigger (cleared when breakpoint happens)

    // dumping enables and stopping
    int    dumpinfo;
    int    dumpgfx;
    int    dumpsnd;
    int    dumptrace;
    int    dumpos;
    int    dumpops;
    int    dumphw;
    int    dumpasm;
    int    stoperror;
    int    stopwarning;
    int    dumpwav;
    int    RESERVED5[12];

    int    timing; // 0=ntsc, 1=pal
    int    dmatransfers;

    // statistics for last frame
    int    retraces;
    int    OBS_frame_tris;     // triangles
    int    OBS_frame_ops;      // cpu ops during frame
    int    OBS_frame_slowtime; // cycles in fast mode
    int    OBS_frame_fasttime; // cycles in slow mode
    int    OBS_frame_ms;       // real frametime in ms

    // fast execution stats
    int    ops_fast;
    int    ops_slow;
    int    OBS_compiledin;     // bytes of MIPS code compiled
    int    OBS_compiledout;    // bytes of X86 code generated
    int    OBS_compiledok;
    int    OBS_compiledfail;
    int    OBS_compilednew;
    int    OBS_compiledpatch;
    int    OBS_compiledclears;
    int    OBS_compiledinregs;
    int    OBS_compiledinnorm;
    int    OBS_compiledinfpu;

    // framebuffer state
    dword  fb_current;
    dword  fb_next;
    dword  fb_nextcurrent;

    // misc
    int    framesync;      // framerate (fps) syncronization target speed (hz)
    dword  padstate;       // pad state (for showing in debugui)
    int    OBS_idlepercentage; // percentage of idle thread execution (os.c)

    // dma tracking for automatic codecache clears (tmp)
    int    OBS_dmacount;
    int    OBS_lastdmacount;
    int    OBS_codecleared;

    // accumulating
    int    us_gfx;
    int    us_audio;
    int    us_cpu;
    int    us_idle;
    int    us_total;
    int    OBS_us_misc;

    int    samples;
    int    executing;
    int    soundenable;
    int    samplesskipped;
    int    graphicsenable;

    int    statfreq;

    int    soundslowsgfx;  // 1=slow gfx down if losing sound sync

    int    optimize;

    int    gfxthread;

    int    checkswitch;

    int    memiosp;  // emulate sp with memio (must be set at boot-time)

    int    audiorate;

    int    nicebreak;
    int    pause;

    int    started;
    int    keyboarddisable;

    int    RESERVED6[40];
    int    magic;  // set to MAGIC1 in newver versions, 0 in old versions
} State;

#define MAGIC1  0xfcabcd02

#define DMAHISTORYSIZE 256

// nonsaved state, initialized to zero every time execution starts
typedef struct
{
    // audio
    int      audioon;      // playing with directsound
    int      audioadded;
    int      audiorequest; // please start playing audio (from slist.c)
    int      audiostatus;  // 0=rate ok, 1=too much, -1=too little, -2=gap!
    int      audiobuffered;
    int      audioresync;
    int      audiobufferedsum;
    int      soundlists;
    int      sync_soundadd;
    int      sync_soundused;
    int      audiobufferedcnt;

    // frame timing (based on audio)
    int      frameus;
    qword    retracetime; // cputime at last retrace

    // os timers
    Timer    ostimer;

    // snd
    int      snd_interl;
    int      snd_envmix;

    // gfx
    int      gfx_trisin;   // read from dlist
    int      gfx_vxin;     // read from dlist
    int      gfx_tris;     // drawn
    int      gfx_txtbytes; // txt loaded

    // gfx thread
    int      gfxthread_execute;
    int      gfxthread_executing;
    int      RESERVED[4];
    int      exception;

    int      sptaskload;
    int      usleft;
    int      memiocheck;

    // cpu
    int      ops;
    int      slowops;  // only set by cpua.c
    int      fastops;  // only set by cpua.c

    // generic us timer (reset at every stat)
    Timer    timer;

    // buffered SP tasks
    int      audiopending;
    OSTask_t audiotask;
    int      gfxpending;
    OSTask_t gfxtask;
    int      gfxfinishpending;
    int      gfxdpsyncpending;

    qword    lastretracecputime;

    int      pendingretraces;

    // last dma transfers (for debugging)
    struct
    {
        dword  cart;
        dword  addr;
        int    size;
        int    RESERVED;
    } dmahistory[DMAHISTORYSIZE];
    int dmahistorycnt;

    int cpuerrorcnt;
} State2;

typedef struct
{
    int    frame;  // for which frame these stats are
    // cpu info
    int    ops;    // ops this frame
    qword  cputime;
    int    fastops;
    int    slowops;
    // gfx info
    int    trisin;
    int    vxin;
    int    tris;
    int    txtbytes;
    // audio info
    float  samplehz;  // samples/sec generated
    int    samplegap; // percentage missing
    float  channels;
    // times for operation categories in microseconds
    int    us_gfx;
    int    us_audio;
    int    us_cpu;
    int    us_idle;
    int    us_misc;
    int    us_total;
    // calculated: percentages
    float  p_cpu;
    float  p_gfx;
    float  p_audio;
    float  p_idle;
    float  p_misc;
    // calculated: other
    float  mips;
    float  compiled;
    float  fps;
    // misc
    int    memio;
    float  frametgt;
    int    RESERVED[7];
} Stats;

#define MAXFILE 1024
// Init settings from command line/startup (not loaded)
typedef struct
{
    int    gfxwid,gfxhig;     // screen resolution

    int    nomemmap;          // do not use memory mapping for roms
    int    novoodoo2;         // no voodoo2 features
    int    shutdownglide;     // full glide shutdown on F12 (Banshee)
    int    showconsole;

    char   rootpath[MAXFILE]; // exetable path (ends in \)
    char   savepath[MAXFILE]; // default path for saves
    char   rompath[MAXFILE];  // default path for rom search

    int    oldkeyb;

    int    viewportwid;
    int    viewporthig;
} Init;

#define BAILOUTNOW  -100000

// active state
extern State  st;     // in cpu.c
extern State2 st2;    // in cpu.c
extern Init   init;   // in cpu.c

#define STATS 16
#define STATUPDATE 30 // every 30 frames
extern Stats ss[STATS];
extern int   ssi; // index to current stat

#define R0  st.g[0x00]
#define AT  st.g[0x01]
#define V0  st.g[0x02]
#define V1  st.g[0x03]
#define A0  st.g[0x04]
#define A1  st.g[0x05]
#define A2  st.g[0x06]
#define A3  st.g[0x07]
#define T0  st.g[0x08]
#define T1  st.g[0x09]
#define T2  st.g[0x0A]
#define T3  st.g[0x0B]
#define T4  st.g[0x0C]
#define T5  st.g[0x0D]
#define T6  st.g[0x0E]
#define T7  st.g[0x0F]
#define S0  st.g[0x10]
#define S1  st.g[0x11]
#define S2  st.g[0x12]
#define S3  st.g[0x13]
#define S4  st.g[0x14]
#define S5  st.g[0x15]
#define S6  st.g[0x16]
#define S7  st.g[0x17]
#define T8  st.g[0x18]
#define T9  st.g[0x19]
#define K0  st.g[0x1A]
#define K1  st.g[0x1B]
#define GP  st.g[0x1C]
#define SP  st.g[0x1D]
#define S8  st.g[0x1E]
#define RA  st.g[0x1F]

// branchtypes
#define BRANCH_NORMAL   0
#define BRANCH_CALL     1
#define BRANCH_RET      2
#define BRANCH_PATCHRET 3

/****************************************************************************
** Cpu emulation high level control (cpu.c)
*/

void cpu_save(FILE *f1);    // save state to a file
void cpu_load(FILE *f1);    // load state from a file
void cpu_init(void);        // clear all regs (entire st struct)
void cpu_break(void);       // stop execution at next possible point and return to console
void cpu_nicebreak(void);   // stop execution at next idle thread
void cpu_goto(dword pc);    // change pc

// main 'execute' routine, parameters are number of ops to execute and
// a flag for requesting fast (compiled) execution. Breakpoints and
// traces etc don't work in fast mode. Mode can be changed at every
// cpu_exec call if so desired. Singlestep with ops=1.
void cpu_exec(qword ops,int fast);

// routines to set breakpoints (see definition of Break above)
void cpu_clearbp(void); // clear all breakpoints
void cpu_addbp(int type,dword addr,dword data); // add a breakpoint
void cpu_onebp(int type,dword addr,dword data); // clear breakpoints, and add this one

// these used internally by cpu.c for calling the slow emulator (cpuc.c)
void c_exec(void); // execute st.bailout instructions
void c_execop(dword opcode); // execute one instruction, no checks

// these used internally by cpu.c for calling the fast compiler (cpua.c)
// the fast compiler calls the slow emulator (with c_execone) for parts
// it cannot compile
void a_clearcodecache(void); // clear compiled code cache
void a_cleardeadgroups(void);
void a_stats(void); // print stats to console
void a_stats2(void); // print stats to console
void a_stats3(void); // print stats to console
void a_exec(void); // execute approximately st.bailout instructions, compile as needed
void a_compilegroupat(dword x);

// these are used internally for breakpoints
void cpu_breakpoint(int i); // breakpoint triggered (count etc checking could be here)
void cpu_initbreak(void);   // do breakpoint init for event breakpoints before exec
void cpu_checkeventbreak(void);  // check event breakpoints
void cpu_checkmembreak(dword addr,int bytes,int iswrite); // check a memory breakpoint
void cpu_checkbranchbreak(dword addr,int type); // check call/ret/branch breakpoint

// these are used internally
void cpu_keys(int dopad);   // check runtime keys (used internally)
void cpu_checkui(int fast); // check/update user interface

// these used by cpua.c to notify cpu.c of things. Cpu.c does call
// tracing and breakpoints based on these calls. Addresses virtual.
// this slows cpuc down but make enhancing breakpoints/traces more
// easy and modular. And cpuc it's not used often anyway.
void cpu_notify_readmem(dword addr,int bytes);  // read of address
void cpu_notify_writemem(dword addr,int bytes); // write of address
void cpu_notify_branch(dword addr,int type);    // branch to address, type=BRANCH_*
void cpu_notify_pc(dword addr);                 // execution at address
void cpu_notify_msg(int queue,dword qaddr,int issend); // execution at address

void cpu_updatestats(int skip);

/****************************************************************************
** Memory state (saved/loaded)
*/

#include "mem.h"
#include "hw.h"
#include "cart.h"
#include "disasm.h"
#include "log.h"
#include "sym.h"
#include "pad.h"
#include "debugui.h"
#include "sync.h"

/****************************************************************************
** Graphics, Sound and OS emulation
*/

#include "patch.h"
#include "rdp.h"
#include "dlist.h"
#include "slist.h"
#include "zlist.h"
#include "sound.h" // directsound

void *main_gethwnd(void);

void outputhook(char *txt,char *full);

#include "inifile.h"

void boot(char *cartname,int nomemmap);

void reset(void);

void fixpath(char *path,int striplastname);

/****************************************************************************
*/

#include "x.h"

#endif  _ULTRA_H_
