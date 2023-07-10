
/***********************************************************************
** Header file for MIPS->X86 compiler (CPUA*.C)
*/

// TODO:
// - optimized code for BYTE/SHORT loads/stores
// + convert LIKELY branch to normal with target-4, if target-4
//   has same instruction as delay slot
// - alloc SP to register whenever it's used as an index
// - delay fpu stores
//   - simple: delay store until flush or next fpu operation starts
//     FLD   s1
//     FADD  s2
//     ...
//     FLD   s3
//     FADD  s4
//     FXCHG st(0),st(1)
//     FSTP  st(0)
// + cleanup, reorganize to
//   * CPUA.H     - header for internal constants
//   * CPUA.C     - high level compiler and execution
//   * CPUAUTIL.C - x86 code insert (ip[], insertop, regs... )
//   * CPUANEW.C  - opcode compiler
//   * CPUAOLD.C  - old opcode compiler
// + compares to
//     SETXY EAX
//     mov   eax,[groupbase-8+EAX*4)
//     -
//     Just before code there is:
//       mov eax,GROUPPC
//       mov [JUMP1GROUP],JUMP2GROUP
//       -align32
//       -code at align+8
// + direct shifts
//     C1 = shift ev,ib
// + ZELDA sometimes gets stuck (village, run forward to water)
// + fpu moves LWC1/SWC1 and MTC1/MFC1 to regs
// + branches
// + branches to regs
// + SLT to regs: SETNG,SETGTE,... (1clock on p2)


/***********************************************************************
** Register usage in the compiled code:
**
**  EAX temp
**  EBX pointer to st+STOFFSET, used for accessing mips regs etc
**  ECX temp
**  EDX used for holding mips registers (when using register allocation)
**  ESI used for holding mips registers (when using register allocation)
**  EDI used for holding mips registers (when using register allocation)
**  EBP used for holding mips registers (when using register allocation)
**  ESP stack pointer (as usually)
**
** Registers on entry to a group
**  EBX pointer to st+STOFFSET
**  other registers undefined
**
** Register on exit from a group (group should set)
**  if next group to execute is known:
**    ECX -(number of instructions executed in this group)
**    EAX ptr to next group (in mem.group table)
**  if group is not known but PC is:
**    ECX -(number of instructions executed in this group)
**    EAX 0
**    EDX new PC (will be written to st.pc by main loop)
*/

/***********************************************************************
** Important defines
*/

#define MAXGROUP 95   // maximum group size in mips instructions

#define VMCACHESIZE 4

/***********************************************************************
** Global compile state struct for the compiler.
** Only used when compiling.
*/

typedef struct
{
    dword  pc;
    dword  x86code;    // offset to mem.code
    char   r[3];       // regs, -1=not present, r[0] is dest, r[1..2]=src
    char   memop;
    int    RESERVED;
} MOp;

typedef struct
{
    // parameters set when compile starts
    Group *g; // group we are compiling
    int    len;
    dword  pc0;

    // parameters for active instruction
    dword  pc;
    int    inserted;
    int    lastjumpto;

    // errors
    int    errors;
    int    fpuused;

    // optimization settings
    int    opt_old;         // use the old compiler (no regs allocation)

    int    opt_rejumpgroup; // jump back to group start

    // for old only
    int    opt_directjmp;   // direct jump to next group

    // for new only
    int    opt_adjacentvm;  // do not do another vm for nearby [reg+1] [reg+2] if reg is the same
    int    opt_vmcache;     // cache vm lookups to memory
    int    opt_nospvm;      // no vm lookup for sp
    int    opt_novm;        // no vm lookup for any loads
    int    opt_slt;         // slt,branch to single branch
    int    opt_eaxloads;    // eliminate unnecessary loads (all regs, not just eax actually)
    // the above works be checking if a MOV reg,[mem] is preceded by
    // MOV [mem],reg2 where mem is same. In this case a MOV reg,reg2
    // is done (or nothing if regs are the same).

    int    opt_domemio;

    // eaxload
    int    eaxload_reg;
    int    eaxload_base;
    int    eaxload_offset;
    int    eaxload_codeusedend;

    // for slt optimization
    int    slt_imm;
    int    slt_rs;
    int    slt_rt;
    int    slt_branch; // 1=do slt branch

    // other stuff
    dword  lastma;
    int    lastmareg;
    int    lastmaoff;

    struct
    {
        int    reg;
        dword  off;
    } vmcache[VMCACHESIZE];
    int vmcachei;

    int    mark;

    MOp    op[MAXGROUP*2];

    int    OBS_regsused;
} RState;

extern RState r; // in cpua.c

// values orred to r.error
#define ERROR_MMU    0x000001
#define ERROR_FPU    0x000020
#define ERROR_OP     0x000300
#define ERROR_PATCH  0x004000
#define ERROR_BRANCH 0x050000
#define ERROR_FLUSH  0x0F0000
#define ERROR_INTER  0x100000
#define ERROR_TEST   0x00aaaa



/***********************************************************************
** Compiler statistics (use cmd 'stats' to see these in the debugui)
*/

typedef struct
{
    // compiler stats
    int    in;     // bytes of MIPS code compiled
    int    out;    // bytes of X86 code generated
    int    ok;
    int    fail;
    int    unexec;
    int    patch;
    int    clears;
    int    inregs;
    int    inregsused;
    int    innorm;
    int    infpu;
    int    inma;
    int    inmasimple;
    // instruction frequencies
    int    used[256];
} CStats;

extern CStats cstat; // in cpua.c

/***********************************************************************
** IP-Table (used for inserting immediate values to code)
**
** When insert() routine inserts code, it replaces 32-bit immediates
** and memory offsets of form A(IP_xxx) with the value in ip[IP_xxx]
**
** The names for the ip[] locations are standardized with the defines
** below, although the meanings are not fixed in the code.
**
** Fields in the global state struct (st) should be accessed with
**   [ebx+STADDR(st.field)]
** which when using the ip table is
**   [ebx+A(IP_PX)]      and ip[IP_PX]=STADDR(st.field)
**
** Also some direct STADDR offsets have been defined below
** for direct usage in inline asm (a_fastexec uses them).
*/

extern dword ip[256];

#define IP_PC        0x00 // pc value
#define IP_D         0x01 // destination reg STADDR
#define IP_RS        0x02 // source1 reg STADDR
#define IP_RT        0x03 // source2 reg STADDR
#define IP_IMM       0x04 // immediate
#define IP_IMMS      0x05 // immediate sign extended
#define IP_FD        0x06 // fpu destination
#define IP_FS1       0x07 // fpu source 1
#define IP_FS2       0x08 // fpu source 2
#define IP_RETGROUP  0x09 // return group
#define IP_RETLEN    0x0a // value returned in ECX
#define IP_P1        0x0b // misc parameter 1
#define IP_P2        0x0c // misc parameter 2
#define IP_P3        0x0d // misc parameter 3
#define IP_P4        0x0e // misc parameter 4

// reference to ip-table in inline asim
#define A(x)         (((x)<<24)|0xfcfbfa)

// this offset allows access to PC and all 32-bit regs
// inside a 8-bit signed offset
#define STOFFSET     0x84 // ebx=&st+STOFFSET

// direct offset values for a few fields
#define STBAIL      -0x84
#define STPC        -0x80
#define STR0        -0x7C
#define STEXP64BIT   (292-STOFFSET)
#define STMEMIODET   (296-STOFFSET)

// macro to calculate offset for any filed (not usable directly in inline asm)
#define STADDR(x)   ((dword)&(x) - (dword)&(st) - STOFFSET)
#define STADDR0(x)  ((dword)&(x))

/***********************************************************************
** Macros for generating insertable inline routines
**
** Only these routines can be inserted with insert() and only
** these can use the ip-table. Here is an example:
**
** OBEGIN(o_example)
**   mov eax,1
**   add eax,A(IP_P1)
** OEND
**
** the above can be inserted to the compiled code with insert(o_example)
**
** routines with RBEGIN,REND are those that can be called with insertcall,
** remember RET at end and saving regs
*/

typedef void (*t_asmop)(void);

#define OBEGIN(x) __declspec(naked) static void x(void) { _asm nop _asm {
#define OEND      } _asm mov eax,0xfffefdfc _asm int 3 }

#define RBEGIN(x) __declspec(naked) static void x(void) { _asm {
#define REND      } }

#define PUBLICRBEGIN(x) __declspec(naked) void x(void) { _asm {
#define PUBLICREND      } }

/***********************************************************************
** Routines for inserting data to the compiled instruction stream
**
** All routines set r.inserted=1, it can be cleared by the compiler
** and checked later to see if anything has been inserted.
**
** Data is added to the array mem.code at the position mem.codeused
** The position is incremented accordingly.
**
** The X86 opcode inserts use registers REG_* and opcode names are X_*
**
** The opcodes have a special format: 0xSFTCAA
** if F!=0 the 0F prefix is inserted
** if T=1  modrm reg field comes from C
** if T=2  no modrm inserted at all
** if T=3  reg is added to opcode (only for immediate opcodes)
** if T=A  not a real opcode (changes before it gets to inserts)
** if S=8  then 8 bit immediate
**
** These are all in CPUAUTIL.C
*/

#define REG_EAX      0
#define REG_ECX      1
#define REG_EDX      2
#define REG_EBX      3
#define REG_ESP      4
#define REG_EBP      5
#define REG_ESI      6
#define REG_EDI      7
#define REG_NONE     8
#define REG_REG      9

// branch compare styles (numbers have no direct meaning on x86)
#define CMP_ALWAYS 0x000
#define CMP_FPU    0x100
#define CMP_INVERT 0x01
#define CMP_EQ     0x10
#define CMP_NE     0x11
#define CMP_LT     0x20
#define CMP_GE     0x21
#define CMP_GT     0x30
#define CMP_LE     0x31
#define CMP_LTUNS  0x40
#define CMP_GEUNS  0x41
#define CMP_GTUNS  0x50
#define CMP_LEUNS  0x51

// memory load/store type
#define M_WR    0
#define M_RD    1
#define M_RDEXT 2

// jump types of ac_jump
#define J_LINK     1
#define J_TOREG    2

// branch types for ac_branch
#define BR_LIKELY  1
#define BR_CMPZERO 2
#define BR_CMPFPU  4

#define X_INT3       0x00CC
#define X_NOP        0x0090
#define X_RET        0x00C3

#define X_MOV        0x0089
#define X_ADD        0x0001
#define X_SUB        0x0029
#define X_AND        0x0021
#define X_OR         0x0009
#define X_XOR        0x0031
#define X_CMP        0x0039
#define X_TEST       0x0085

#define X_SHLCL      0x14D3
#define X_SHRCL      0x15D3
#define X_SARCL      0x17D3

#define X_SHL01      0x14D1
#define X_SHR01      0x15D1
#define X_SAR01      0x17D1

#define X_SHLIMM     0x8014C1
#define X_SHRIMM     0x8015C1
#define X_SARIMM     0x8017C1

#define X_FLD4       0x10d9
#define X_FLD8       0x10dd
#define X_FST4       0x12d9
#define X_FST8       0x12dd
#define X_FSTP4      0x13d9
#define X_FSTP8      0x13dd

#define X_FADD4      0x10d8
#define X_FADD8      0x10dc
#define X_FSUB4      0x14d8
#define X_FSUB8      0x14dc
#define X_FSUBR4     0x15d8
#define X_FSUBR8     0x15dc
#define X_FMUL4      0x11d8
#define X_FMUL8      0x11dc
#define X_FDIV4      0x16d8
#define X_FDIV8      0x16dc
#define X_FCOMP4     0x13d8
#define X_FCOMP8     0x13dc
#define X_FCOM4      0x12d8
#define X_FCOM8      0x12dc

#define X_IMMMOV     0x30b8 // T=3, reg adds opcode
#define X_IMMADD     0x1081
#define X_IMMADD8    0x1083
#define X_IMMSUB     0x1581
#define X_IMMAND     0x1481
#define X_IMMOR      0x1181
#define X_IMMXOR     0x1681
#define X_IMMCMP     0x1781

#define X_NOR        0xA001
#define X_LUIMOV     0xA002 // not a real opcode

void insertnothing(void); // inserts nothing (but sets r.inserted=1)
void insertbyte(int x);   // inserts a single byte
void insertdword(int x);  // inserts a dword (32-bit)

// insert a routine (with ip[] table data replacements)
void insert(t_asmop o);

// generic MODRM inserter. Only supports a single base register
// and offset, no index register. If possible use the other more
// specific insert routines below this.
void insertmodrmopcode(int opcode,int reg,int base,int offset);

// inserts a mem opcode:   OP  dst,[base+offset]
// optimized: offset=0 or base=REG_NONE
void insertmemropcode(int opcode,int dst,int base,int offset);

// inserts a mem write:    OP  [base+offset],src
// optimized: offset=0 or base=REG_NONE
void insertmemwopcode(int opcode,int base,int offset,int src);

// inserts a reg opcode:   OP  dst,src
// ignores: MOV reg,reg
// for some opcodes (like NOT) src is ignored
void insertregopcode(int opcode,int dst,int src);

// inserts an immediate:   OP  dst,imm
// optimizes: ADD reg,32bit to ADD reg,8bit if immediate small enough
void insertimmopcode(int opcode,int dst,int imm);

// inserts a call:         CALL routine
// note that the routine must be naked and note the reg usage etc
void insertcall(void *routine);
void insertjump(void *routine);

/***********************************************************************
** Misc utils
*/

// returns an internally used opcode number from a mips opcode
int  getop(dword opcode);

/***********************************************************************
** Main compile-an-opcode entrypoint
*/

// group compiler main routine
void   ac_compilegroup(Group *g);

// call this
int    ac_compileop(dword pc);
// it calls one of these based on r.opt_old parameter
int    ac_compileopold(dword pc,dword opcode,int op);
int    ac_compileopnew(dword pc,dword opcode,int op);

// start/end routines for new/old compilers
void   ac_compilestartnew(void);
void   ac_compilestartold(void);
void   ac_compileendnew(void);
void   ac_compileendold(void);

// routine for creating a new group Tthe group is allocated an entry
// in mem.group table, but it is not compiled before it is executed.
Group *ac_creategroup(dword pc);

/***********************************************************************
** Routines for X86 register allocation
*/

// Allocating x86 registers (caller must load the reg!)
int  reg_alloc(int name,int x86);  // alloc a specific x86 register for 'name'
int  reg_allocnew(int name);       // alloc a new register for 'name' (even if it already  has one)
int  reg_oldest(void);             // find oldest (by lastused) reg for overwriting

// Helpers for loading/saving MIPS regs (reg must be allocated first)
void reg_save(int x86);            // load value to an x86 register (only works for mips regs)
void reg_load(int x86);            // save value from an x86 register (only works for mips regs)

// Using loaded registers
int  reg_find(int name);           // where 'name' is loaded, returns x86 reg (REG_NONE=nowhere)
void reg_wr(int x86);              // mark x86 register changed and increment lastused
void reg_rd(int x86);              // increment lastused
void reg_rename(int x86,int name); // increment lastused

// Freeing registers for other usage (reg saved using reg_save)
void reg_free(int x86);            // make x86 register available
void reg_freeall(void);            // make all x86 register unused (saves it)
void reg_freeallbut(int name,int name2);

// Floating point regs
void freg_push(int name,int size);
void freg_xchg(int i); // with st(0)
void freg_save(int i);
void freg_delete(int i);
void freg_load(int name,int size);
void freg_saveall();
int  freg_find(int name);          // where 'name' is loaded, returns x86 reg (REG_NONE=nowhere)
void freg_renametop(int name,int size);
void freg_pushtop(int name,int size);
void freg_poptop(void);

void reg_init(void);
void freg_init(void);
void freg_dump(void);

// Register name is 8bit index + one of the constants below
// NOTE: mips reg 0 cannot be loaded to an x86 reg
#define XNAME_EMPTY   0x000
#define XNAME_MIPS    0x000
#define XNAME_MIPSVM  0x100
#define XNAME_TEMP    0x200
#define XNAME_FPUD    0x400
// temps are really quite important. They are never overridden with
// reg_allocnew. The routine that allocs them should free them soon.
// They are right now used only at exit sequences.

// masks for accessing INDEX and TYPE in name
#define XNAME_INDEX   0x0ff
#define XNAME_TYPE    0xf00

// Tables for keeping track what data is in all x86 regs
typedef struct
{
    int    name;      // type (XNAME_*)
    int    size;      // fpu size, 4/8
    int    changed;   // changed, must flush  (0=data unchanged since loaded)
    int    lastused;
} XReg;

extern XReg reg[8];

extern XReg fpreg[16];

extern void fastexec_loop(void);
extern void fastexec_loopjrra(void);

