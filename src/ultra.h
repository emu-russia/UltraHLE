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
#include <ctype.h>

/****************************************************************************
** Basic types
*/

typedef          __int64    qint;
typedef unsigned __int64    qword;
typedef unsigned int        dword;
typedef unsigned short      word;
typedef unsigned char       byte;

#include "os.h"
#include "timer.h"
#include "console.h"
#include "cpu.h"
#include "mem.h"
#include "hw.h"
#include "cart.h"
#include "disasm.h"
#include "log.h"
#include "sym.h"
#include "pad.h"
#include "debugui.h"
#include "sync.h"
#include "patch.h"
#include "rdp.h"
#include "dlist.h"
#include "slist.h"
#include "zlist.h"
#include "sound.h" // directsound
#include "inifile.h"
#include "boot.h"
#include "main.h"
#include "x.h"

#endif  _ULTRA_H_
