// Memory state (saved/loaded)

#pragma once

#define IO_MAX 32

// compiler compiles instructions in groups, groups always end
// at next jump/branch/call instruction's delay slot. In fast
// mode one group is executed at a time, either with cpua or cpuc.
// when compiling, the first opcode of each groups is replaced
// with a special GROUP(x) opcode.††
typedef struct
{
    byte* code;        // ptr to compiled code (NULL=not compiled)
    dword  addr;        // address of group
    dword  opcode;      // original first opcode (which was overwritten)
    word   len;         // length in mips instructions
    char   type;        // GROUP_*
    char   ratio;       // appr. length in x86 bytes is len*ratio/4
} Group; // 16 bytes size (SIZE USED IN INLINE ASM, a_fastgroup in cpua.c!)
#define GROUP_NEW   0   // new group, not yet analyzed
#define GROUP_SLOW  1   // analysed, use slow cpuc
#define GROUP_FAST  2   // analysed, use fast cpua (in this case .code set)
#define GROUP_PATCH 3   // analysed, use slow cpuc (patch)

typedef struct
{
    // VMM page lookup table (4GB lookup = 4MB table)
    byte* lookupr[1048576];  // for reads
    byte* lookupw[1048576];  // for writes

    // memory mapped io (first index from IO_ defines)
    // 32 special pages, 17 used, separate page for READ and WRITE
    // write pages initialized with NULLFILL, read pages with 0
    // use R* and W* macros to access. Note that addresses are
    // DWORD addresses since io's type is DWORD.
    dword  io[IO_MAX][2][1024];

    // stuff for the compiler (cpua.c)
    Group* group;       // table for compiled groups
    int    groupnum;    // used entries
    int    groupmax;    // max size
    byte* code;        // code compiled to this array
    int    codeused;    // used bytes
    int    codemax;     // max size

    // execution profiling (must be enabled in CPUA.H)
    int* groupcnt;

    // Main RDRAM memory (mapped through lookup)
    byte* ram;         // ram data (dynamic alloc at BASE-4096!!)
    int    ramsize;     // size of ram data
    byte* ramalloc;    // ram data (dynamic alloc at BASE-4096!!)
} Mem;

// Magic filler for nullpages and io-write pages for write detection:
// opcode 0x70707070 = PATCH(0x7070) which generates 'execution at null page'
#define NULLFILL   0x70707070

extern Mem mem; // mem.c

// indices to mem.io
// ok to add new ones, but don't change indices to maintain save compatibility
#define IO_NULL    0  // null page (unused memory locations)
#define IO_OS      1  // 03FF0000: special os emulation page
#define IO_SPD     2  // 04000000: SP data memory
#define IO_SPI     3  // 04001000: SP code memory
#define IO_SP      4  // 04040000: SP registers
#define IO_DP      5  // 04100000: DP registers
#define IO_DPSPAN  6  // 04200000: DP span registers
#define IO_MI      7  // 04300000: MIPS interface
#define IO_VI      8  // 04400000: MIPS interface
#define IO_AI      9  // 04500000: AI audio interface
#define IO_PI     10  // 04600000: PI interface
#define IO_RI     11  // 04700000: RI interface
#define IO_SI     12  // 04800000: SI interface
#define IO_MISC1  13  // 18000000: misc ints/control
#define IO_MISC2  14  // 1F400000: misc ints/control
#define IO_RDB    15  // 1F480000: RDB regs
#define IO_PIF    16  // 1FC00000: PIF boot rom and joychannel
#define IO_SP2    17  // 04080000: SP registers 2
// read/write
#define IO_W       0  // CPU writes (hw reads)
#define IO_R       1  // CPU reads (hw writes)

// macros for easy access to the above (CPU write page)
#define WNULL   mem.io[IO_NULL][IO_W]
#define WOS     mem.io[IO_OS][IO_W]
#define WSPD    mem.io[IO_SPD][IO_W]
#define WSPI    mem.io[IO_SPI][IO_W]
#define WSP     mem.io[IO_SP][IO_W]
#define WSP2    mem.io[IO_SP2][IO_W]
#define WDP     mem.io[IO_DP][IO_W]
#define WDPSPAN mem.io[IO_DPSPAN][IO_W]
#define WMI     mem.io[IO_MI][IO_W]
#define WVI     mem.io[IO_VI][IO_W]
#define WAI     mem.io[IO_AI][IO_W]
#define WPI     mem.io[IO_PI][IO_W]
#define WRI     mem.io[IO_RI][IO_W]
#define WSI     mem.io[IO_SI][IO_W]
#define WMISC1  mem.io[IO_MISC1][IO_W]
#define WMISC2  mem.io[IO_MISC2][IO_W]
#define WRDB    mem.io[IO_RDB][IO_W]
#define WPIF    mem.io[IO_PIF][IO_W]

// macros for easy access to the above (CPU read page)
#define RNULL   mem.io[IO_NULL][IO_R]
#define ROS     mem.io[IO_OS][IO_R]
#define RSPD    mem.io[IO_SPD][IO_R]
#define RSPI    mem.io[IO_SPI][IO_R]
#define RSP     mem.io[IO_SP][IO_R]
#define RSP2    mem.io[IO_SP2][IO_R]
#define RDP     mem.io[IO_DP][IO_R]
#define RDPSPAN mem.io[IO_DPSPAN][IO_R]
#define RMI     mem.io[IO_MI][IO_R]
#define RVI     mem.io[IO_VI][IO_R]
#define RAI     mem.io[IO_AI][IO_R]
#define RPI     mem.io[IO_PI][IO_R]
#define RRI     mem.io[IO_RI][IO_R]
#define RSI     mem.io[IO_SI][IO_R]
#define RMISC1  mem.io[IO_MISC1][IO_R]
#define RMISC2  mem.io[IO_MISC2][IO_R]
#define RRDB    mem.io[IO_RDB][IO_R]
#define RPIF    mem.io[IO_PIF][IO_R]

// macros for calculating mem addresses (used internally)
#define mempage(x)  ((dword)( (unsigned)(x             ) >> 12    ) )
#define memoffs(x)  ((dword)( (unsigned)(x & 0xfff     )          ) )
#define memdatar(x) ((dword *)( mem.lookupr[mempage(x)]+x ))
#define memdataw(x) ((dword *)( mem.lookupw[mempage(x)]+x ))

// sim.c
void    mem_init(int ramsize);          // allocates memory and initializes memory system
dword   mem_getphysical(dword virtual); // virtual->physical address (-1=no physical for that address!)

// map page[dst] to external 4K array (external data NOT saved!)
// map page[dst] to physical address src
// map page[dst] to where page[src] points to
void    mem_mapcopy(dword dst, int rw, dword src);
void    mem_mapphysical(dword dst, int rw, dword src);
void    mem_mapexternal(dword dst, int rw, void* data);

// rw parameter values for mapping (WTHENR only works for mapexternal)
#define MAP_W       0  // map page for CPU write
#define MAP_R       1  // map page for CPU read
#define MAP_RW      2  // map page for both
#define MAP_WTHENR  3  // map page for CPU write, and other page at data+4096 to CPU read

void    mem_alloc(dword dst);

int mem_groupat(dword addr);

// routines to access memory. 32-bit accesses are macros and quite fast,
// 8 and 16 bit accesses are routines are slower.
dword   mem_read8(dword addr);
dword   mem_read16(dword addr);
#define mem_read32(addr) (*memdatar(addr))
#define mem_read32p(addr) (*(dword *)( mem.ram+ (addr&(RDRAMSIZE-1)) ) )
void    mem_write8(dword addr, dword data);
void    mem_write16(dword addr, dword data);
#define mem_write32(addr,data) (*memdataw(addr))=(data)

// special routine to read an opcode. This routine replaces
// GROUP(x) opcodes with original opcodes from mem.group table
dword   mem_readop(dword addr);
// read type of group for debugui view module (returns static string)
char* mem_readoptype(dword addr);

// copying a lot of data (addr MUST be dword aligned)
void    mem_readrangeraw(dword addr, int bytes, char* data);
void    mem_writerangeraw(dword addr, int bytes, char* data);


void    mem_save(FILE* f1);
void    mem_load(FILE* f1);
