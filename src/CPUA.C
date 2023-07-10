#include <search.h>
#include "ultra.h"
#include "cpua.h"

#define DUMPGO    0  // dump PC as executing (debug) (1=every group, 2=every new group)
#define EXECPROF  0  // profile execution (cmd 'stat3'), slows execution!

// these names are a bit short for global scope, but they were
// originally internal to this file. Then this file grew so large
// it had to be splitted. Well so it goes.
RState r;
CStats cstat;
dword  ip[256];

/**************************************************************************
** Routines for compiling a new group
*/

// select optimization settings
void a_optimizesetup(void)
{
    r.opt_old=0;
    r.opt_directjmp=0;
    r.opt_rejumpgroup=0;
    r.opt_adjacentvm=0;
    r.opt_nospvm=0;
    r.opt_novm=0;
    if(st.optimize==0)
    {
    }
    else if(st.optimize==1)
    {
        r.opt_adjacentvm=1;
        r.opt_slt=1;
    }
    else if(st.optimize==2)
    {
        r.opt_adjacentvm=1;
        r.opt_vmcache=1;
        r.opt_eaxloads=1;
        r.opt_slt=1;
    }
    else if(st.optimize>=3)
    {
        r.opt_adjacentvm=1;
        r.opt_vmcache=1;
        r.opt_eaxloads=1;
        r.opt_slt=1;

        r.opt_nospvm=1;
    }

    if(st.oldcompiler) r.opt_old=1;

    r.opt_domemio=1;
}

// called to compile one opcode and insert the compiled data into
// x86 instruction stream. Calls CPUAOLD.C or CPUANEW.C
int ac_compileop(dword pc)
{
    dword opcode=mem_readop(pc);
    int op;

    op=getop(opcode);

    r.pc=pc;
    ip[IP_PC]=pc;

    cstat.used[op]++;

    if(r.opt_old) return ac_compileopold(pc,opcode,op);
    else          return ac_compileopnew(pc,opcode,op);
}

void ac_sizegroup(Group *g)
{ // determine length of group
    int i,branch;
    int allnops=1;
    dword x,op;
    x=g->opcode;
    for(i=0;i<MAXGROUP;)
    {
        // check if this opcode is a branch/jal
        branch=0;
        if(x!=0) allnops=0;
        op=OP_OP(x);
        if(op>=2 && op<=7) branch=1; // jal, jar, beq, bne, ...
        else if(op>=20 && op<=23) branch=1; // blt, bgt, ...
        else if(op==0 && OP_FUNC(x)>=8 && OP_FUNC(x)<=9) branch=1; // jr, jalr
        else if(op==1) branch=1; // branch likely
        else if(op==17 && OP_RS(x)==8) branch=1; // fpu branch
        else if(op==OP_GROUP || op==OP_PATCH) branch=2;

        if(branch==2)
        {
            // end now
            i+=1;
            break;
        }
        else if(branch)
        {
            // end after the delay slot
            i+=2;
            break;
        }

        // next
        i++;
        x=mem_readop(g->addr+i*4);
    }
/*
    if(g->addr==0x8008c57c)
    {
        i=4;
        print("special groupsize!\n");
    }
    if(g->addr==0x8008c58c)
    {
        i=2;
        print("special groupsize!\n");
    }
    if(g->addr==0x8008c594)
    {
        i=2;
//        r.errors++;
        print("special groupsize!\n");
    }
*/

    g->len=i;
    g->ratio=1;
    logc("compiler: group %08X size %i (op=%i)\n",g->addr,g->len,op);
}

OBEGIN(o_urpo)
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AE0-0x00ED4914]
OEND

float urpo1a;
float urpo2a;
float urpo1b;
float urpo2b;
float urpo_f4;
float urpo_f6;
float urpo_f12;

void urpoprint(void)
{
    /*
    print("urpo ok( f4*f12:%5.2f f6-f4:%5.2f ) \n"
          "    bad( f4*f12:%5.2f f6-f4:%5.2f ) f4:%5.2f f6:%5.2f f12:%5.2f\n",
        urpo1a,urpo2a,
        urpo1b,urpo2b,
        urpo_f4,
        urpo_f6,
        urpo_f12);
    */
}


OBEGIN(o_urpo2)
/*
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fstp dword ptr [urpo_f4]
fld  dword ptr [ebx+0x00ED4B08-0x00ED4914]
fstp dword ptr [urpo_f12]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fstp dword ptr [urpo_f6]

fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fstp dword ptr [urpo1a]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub dword ptr [urpo1a]
fstp dword ptr [urpo2a]

fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub st,st(1)
fstp dword ptr [urpo2b]
fstp dword ptr [urpo1b]
*/

#if 0
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AE0-0x00ED4914]
#else
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub st,st(1)
fstp dword ptr [ebx+0x00ED4AE0-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AD8-0x00ED4914]
#endif

/*
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fst  dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub st,st(1)
fstp dword ptr [ebx+0x00ED4AE0-0x00ED4914]
fstp st(0)
*/
/*
fld  dword ptr [ebx+0x00ED4AE8-0x00ED4914]
fmul dword ptr [ebx+0x00ED4B08-0x00ED4914]
fstp dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AD8-0x00ED4914]
fld  dword ptr [ebx+0x00ED4AF0-0x00ED4914]
fsub st,st(1)
fstp dword ptr [ebx+0x00ED4AE0-0x00ED4914]
fstp st(0)
*/

OEND

void ac_compilegroup(Group *g) // main
{
    int   i,l;
    dword lastcodepos;
    dword alignedcodepos;

    // init r to zero
    memset(&r,0,sizeof(r));

    // set r fields used by global parameters
    r.g=g;
    r.pc0=g->addr;

    // select optimization settings
    a_optimizesetup();

    // determine size of the group
    ac_sizegroup(g);
    r.len=g->len;

    // is this a patch group (os routine)
    if(OP_OP(g->opcode)==OP_PATCH)
    {
        cstat.patch++;
        logc("compiler: group %08X patched\n",g->addr,r.errors);
        // mark as uncompilable
        g->type=GROUP_PATCH;
        g->code=NULL;
        return;
    }

    // store last codepos for restoring it if compile fails
    lastcodepos=mem.codeused;

    r.mark=0;

    if(r.opt_old) ac_compilestartold();
    else          ac_compilestartnew();
if(0 && r.pc0==0x8008c594)
{
    insert(o_urpo2);
}
else
{
    alignedcodepos=mem.codeused;
    for(i=0;i<g->len;)
    {
        l=ac_compileop(g->addr+i*4);
        if(!l)
        {
            r.errors|=ERROR_INTER;
            break;
        }
        i+=l;
        if(mem.codeused>=mem.codemax-1000)
        {
            // out of mem, clear code cache
            a_clearcodecache();
            break;
        }
    }
}
    if(r.opt_old) ac_compileendold();
    else          ac_compileendnew();

    if(!r.errors && r.mark)
    {
        exception("compiler-test-mark");
    }

    // check for errors
    if(r.errors)
    {
        cstat.fail++;
        logc("compiler: group %08X failed (error %08X)\n",g->addr,r.errors);
//print("compiler: group %08X failed (error %08X)\n",g->addr,r.errors);
        // mark as uncompilable
        g->type=GROUP_SLOW;
        g->code=NULL;
        // free used code memory
        mem.codeused=lastcodepos;
    }
    else
    {
        cstat.ok++;
        cstat.in+=4*g->len;
        cstat.out+=mem.codeused-alignedcodepos;
        logc("compiler: group %08X succeeded\n",g->addr,r.errors);
        // mark as compiled;
        g->type=GROUP_FAST;
        if(g->len==0) g->ratio=1;
        else g->ratio=(mem.codeused-alignedcodepos)*4/g->len;

if(r.fpuused) g->ratio=3;
else          g->ratio=2;
        //g->code has been set earlier

        if(mem.codeused<lastcodepos)
        {
            // cache was cleared at some point due to overflow.
            // must compile again, mark as uncompiled
            g->type=GROUP_NEW;
            g->code=NULL;
        }
    }

    cstat.unexec--;
}

// public routine for compiling a group for testing purposes
void a_compilegroupat(dword x)
{
    Group *g;
    g=ac_creategroup(x);
    ac_compilegroup(g);
}

/**************************************************************************
** Statistics
*/

void a_stats(void)
{
    float ratio,ratio2;
    int a,c,c2;

    print("MIPS->X86 compiler statistics: (optimize=%i)\n",st.optimize);

    if(!cstat.out) ratio=0.0f;
    else ratio=(float)cstat.out/cstat.in;

    print( "%8i bytes input MIPS code\n",cstat.in);
    print( "%8i bytes output X86 code\n",cstat.out);
    print( "%8i bytes used in buffer\n",mem.codeused);
//    print( "%8i code in code cache\n",mem.codeused);
    print("%8.2f code expansion ratio\n",ratio);
    print( "%8i codecache clears\n",cstat.clears);
    print( "%8i instruction groups\n",mem.groupnum);
    a=mem.groupnum; if(!a) a=1;
    print( "%8i - average length\n",cstat.in/a);
    print( "%8i - succesfully compiled\n",cstat.ok);
    print( "%8i - patched\n",cstat.patch);
    print( "%8i - not yet executed\n",cstat.unexec);
    print( "%8i - failed compile (slow)\n",cstat.fail);

    c=cstat.in/4; if(!c) c=1;
    print( "%8i compiled ops total\n",c);
    ratio=100.0*cstat.infpu/c;
    print( "%7.2f%% - use floating point compiler\n",ratio);
    ratio=100.0*cstat.innorm/c;
    print( "%7.2f%% - use nonregister compiler\n",ratio);
    ratio=100.0*cstat.inregs/c;
    c2=cstat.inregs; if(!c2) c2=1;
    ratio2=100.0*cstat.inregsused/c2;
    print( "%7.2f%% - use register compiler (of which %.2f%% use regs)\n",ratio,ratio2);
    ratio=100.0*cstat.inma/c;
    c2=cstat.inma; if(!c2) c2=1;
    ratio2=100.0*cstat.inmasimple/c2;
    print( "%7.2f%% - use memory accesses (of which %.2f%% simplified)\n",ratio,ratio2);
}

void a_stats2(void)
{
    int i,j,i1,i1c;
    int bak[256];
    print("Compiled opcodes:\n");
    memcpy(bak,cstat.used,256*4);
    for(j=0;j<256;j++)
    {
        // find least common
        i1=-1;
        i1c=0x7fffffff;
        for(i=0;i<256;i++) if(cstat.used[i])
        {
            if(cstat.used[i]<i1c)
            {
                i1=i;
                i1c=cstat.used[i];
            }
        }
        if(i1>=0)
        {
            print("-op %02X+%-2i : %i\n",i1&0xc0,i1&0x3f,i1c);
            cstat.used[i1]=0;
        }
    }
    memcpy(cstat.used,bak,256*4);
}

typedef struct
{
    int  group;
    int  cnt;
    int  bytecnt;
    int  RESERVED;
} GCnt;

int gcntcmp(const void *a0,const void *b0)
{
    GCnt *a=(GCnt *)a0;
    GCnt *b=(GCnt *)b0;
//    return(b->bytecnt - a->bytecnt);
    return(b->cnt - a->cnt);
}


#if EXECPROF
void a_stats3(void)
{
    GCnt *gcnt;
    int   i;
    gcnt=malloc(mem.groupmax*sizeof(GCnt));

    for(i=0;i<mem.groupnum;i++)
    {
        gcnt[i].group=i;
        gcnt[i].cnt=mem.groupcnt[i];
        gcnt[i].bytecnt=mem.groupcnt[i]*(mem.group[i].len*mem.group[i].ratio/4);
    }

    qsort(gcnt,mem.groupnum,sizeof(GCnt),gcntcmp);

    print("Group execution profile:\n"
          "##  group-pc   len     count  x86bytes\n");
    for(i=19;i>=0;i--)
    {
        print("%2i: %08X  %4i  %8i  %8i\n",
            i+1,
            mem.group[gcnt[i].group].addr,
            mem.group[gcnt[i].group].len,
            gcnt[i].cnt,
            gcnt[i].bytecnt);
    }

    free(gcnt);
}
#else
void a_stats3(void)
{
    print("Execution profiling support not compiled (see cpua.c)\n");
}
#endif

/**************************************************************************
** Executing a compiled group
*/

static int fpucw;
static int originalfpucw=0;
static int fastgroupinitdone=0;

void a_fastgroupinit(void)
{
    fastgroupinitdone=1;
    fpucw=63; // mask exceptions
    if(0)
    {
        fpucw|=(0<<8);  // 32 bit accuracy
        fpucw|=(3<<10); // chop rounding
    }
    else
    {
        fpucw|=(1<<8);  // 64 bit accuracy
        fpucw|=(3<<10); // chop rounding
    }
    _asm fstcw [originalfpucw]
}

PUBLICRBEGIN(fastexec_loopjrra)
        mov    esi,[ebx+STMEMIODET]
        xor    eax,eax
        test   esi,esi
        mov    [ebx+STEXP64BIT],eax
        jnz    memio
        jmp    fastexec_loop
memio:  pushad
        call   hw_memio
        popad
        jmp    fastexec_loop
PUBLICREND

//--main loop for executing groups. Normally every MIPS jump/call
//--returns here for dispatching. However, optimized code might
//--do the jump/call internally without returning (but it still
//--checks the bailout, so if that goes negative execution returns).
PUBLICRBEGIN(fastexec_loop)
        // EDX=group.code
        // returns with either (len=number of instructions executed)
        //    EAX=next group  ECX=-len
        // or EAX=0           ECX=-len  EDX=st.pc
/*
        mov    esi,[mem.ram]
        mov    edi,[esi+0x33b218]
        cmp    edi,78h
        je     cont
        mov    esi,-99999999
        mov    [ebx+STBAIL],esi
        ret
cont:
*/
        test   eax,eax
        jz     nogroup

        //--a jump to a new known group
        // EAX=group, ECX=-len
        // decrement bailout, load codeptr for next group
        add    ecx,[ebx+STBAIL]
        mov    edx,[eax] // Group.code
        mov    [ebx+STBAIL],ecx
#if DUMPGO==1 | EXECPROF
jmp finish2
#endif
        jl     finish2   // bailout, exit
        test   edx,edx
        jz     finish2   // no code, exit
        jmp    edx       // go!

        //--a jump to register / ret (no destination group specified)
    nogroup:
        // EAX=0, ECX=-len, EDX=st.pc
        // decrement bailout, load codeptr for next group
        add    ecx,[ebx+STBAIL]
        mov    [ebx+STPC],edx // store pc
        mov    [ebx+STBAIL],ecx
#if DUMPGO==1 | EXECPROF
jmp finish
#endif
        jl     finish
        // read memory at st.pc
        mov    eax,edx
        shr    edx,12
        add    eax,[OFFSET mem.lookupr+edx*4]
        mov    eax,[eax]   // mem_read32(st.pc)
        // calc EDX=OP_OP(mem), EAX=sizeof(Group)*OP_IMM(mem)
        mov    edx,eax
        and    eax,0xffff  // &0xffff
        shr    edx,26
        shl    eax,4       // *sizeof(Group)
        cmp    edx,OP_GROUP
        jne    finish3     // no group at destination
        // convert EAX to group ptr, and go to execute group
        add    eax,[mem.group]
        mov    edx,[eax+0] // Group.code
        test   edx,edx
        jz     finish3     // no code, exit
        jmp    edx         // go!

    finish2:
        mov    edx,[eax+4] // Group.addr
    finish:
        // EDX=ending pc
        // set pc
        mov    [ebx+STPC],edx // st.pc=addr
    finish3:
        ret
PUBLICREND

void a_fastgroup(Group *g)
{
    dword stptr=(dword)&st;
    stptr+=STOFFSET;
    // will execute multiple groups as long as bailout is positibe
    // and groups remain compiled
    _asm
    {
        // save regs
        push   ebp
        // set fpu rounding mode
        fldcw  [fpucw]
        // load current group we will execute, and first codeptr
        mov    eax,[g]
        // load constant regs for fastexec
        mov    ebx,[stptr] // can't use 'OFFSET st' since the inline asm thinks st is a fpu reg :(
        mov    edx,[eax] // Group.code
        mov    esi,OFFSET mem.lookupr
        mov    edi,OFFSET mem.lookupw
        xor    ecx,ecx

        push   OFFSET rethere
        jmp    edx // will return to fastexec_loop
    rethere:

        // restore regs
        fldcw  [originalfpucw]
        pop    ebp
    }
}

void a_fastgroup_old(Group *g)
{
    dword stptr=(dword)&st;
    stptr+=STOFFSET;
    // will execute multiple groups as long as bailout is positibe
    // and groups remain compiled
    _asm
    {
        // save regs
        push   ebp
        // set fpu rounding mode
        fldcw  [fpucw]
        // load current group we will execute, and first codeptr
        mov    eax,[g]
        // load constant regs for fastexec
        mov    ebx,[stptr] // can't use 'OFFSET st' since the inline asm thinks st is a fpu reg :(
        mov    esi,OFFSET mem.lookupr
        mov    edi,OFFSET mem.lookupw
        mov    edx,[eax+0] // Group.code (first group always compiled)

        //--main loop for executing groups. Normally every MIPS jump/call
        //--returns here for dispatching. However, optimized code might
        //--do the jump/call internally without returning (but it still
        //--checks the bailout, so if that goes negative execution returns).
    again:
        // EDX=group.code
        call   edx
        // returns with either (len=number of instructions executed)
        //    EAX=next group  ECX=-len
        // or EAX=0           ECX=-len  EDX=st.pc
        test   eax,eax
        jz     nogroup

        //--a jump to a new known group
    //havegroup:
        // EAX=group, ECX=-len
        // decrement bailout, load codeptr for next group
        add    ecx,[ebx+STBAIL]
        mov    edx,[eax] // Group.code
        mov    [ebx+STBAIL],ecx
        // bailout?
#if DUMPGO==1 | EXECPROF
jmp finish2
#endif
        jl     finish2   // bailout, exit
        test   edx,edx
        jnz    again     // we have code, go
        jmp    finish2   // no code, exit

        //--a jump to register / ret (no destination group specified)
    nogroup:
        // EAX=0, ECX=-len, EDX=st.pc
        // decrement bailout, load codeptr for next group
        add    ecx,[ebx+STBAIL]
        mov    [ebx+STPC],edx // store pc
        mov    [ebx+STBAIL],ecx
        // bailout?
#if DUMPGO==1 | EXECPROF
jmp finish
#endif
        jl     finish
        // read memory at st.pc
        mov    eax,edx
        shr    edx,12
        add    eax,[OFFSET mem.lookupr+edx*4]
        mov    eax,[eax]   // mem_read32(st.pc)
        // calc EDX=OP_OP(mem), EAX=sizeof(Group)*OP_IMM(mem)
        mov    edx,eax
        and    eax,0xffff  // &0xffff
        shr    edx,26
        shl    eax,4       // *sizeof(Group)
        cmp    edx,OP_GROUP
        jne    finish3     // no group at destination
        // convert EAX to group ptr, and go to execute group
        add    eax,[mem.group]
        mov    edx,[eax+0] // Group.code
        test   edx,edx
        jnz    again       // we have code
        jmp    finish3     // no code, exit

    finish2:
        mov    edx,[eax+4] // Group.addr
    finish:
        // EDX=ending pc
        // set pc
        mov    [ebx+STPC],edx // st.pc=addr
    finish3:
        // restore regs
        fldcw  [originalfpucw]
        pop    ebp
    }
}

/**************************************************************************
** Executing a noncompiled group
*/

void a_slowgroup(Group *g)
{ // execute with cpuc
    dword pcmin,pcmax;

    pcmin=g->addr;
    pcmax=g->addr+g->len*4;
    while(st.pc>=pcmin && st.pc<pcmax && st.bailout>0)
    {
        c_execop(mem_readop(st.pc));
        st.bailout--;
    }
    // avoid stopping at middle of group (if the group is a loop)
    if(st.bailout<=0) while(st.pc!=pcmin && st.pc<pcmax && st.bailout>-MAXGROUP)
    {
        c_execop(mem_readop(st.pc));
        st.bailout--;
    }
    // never stop when branchdelay in progress
    if(st.branchdelay)
    {
        c_execop(mem_readop(st.pc));
        st.bailout--;
    }
}

/**************************************************************************
** Executing in fast mode
*/

void a_exec(void)
{
    int    startbail=st.bailout;
    int    slowcnt=0;
    dword  opcode;
    Group *g;

    if(!fastgroupinitdone) a_fastgroupinit();

    while(st.bailout>0)
    {
        opcode=mem_read32(st.pc);
        if(OP_OP(opcode)==OP_GROUP)
        { // group found
            g=mem.group+OP_IMM24(opcode);
#if EXECPROF
            mem.groupcnt[OP_IMM24(opcode)]++;
#endif
            if(g->type==GROUP_NEW)
            {
                ac_compilegroup(g);
                if(st.bailout<0) break;
            }
            if(g->code)
            {
#if DUMPGO
print("fast-go %08X (%i)\n",st.pc,g->ratio&1);
#endif
                // NOTE: fastgroup may actually execute multiple
                // groups, as long as bailout remains positive
                // and Group.code is present for all groups.
                if(st.oldcompiler) a_fastgroup_old(g);
                else a_fastgroup(g);
            }
            else
            {
#if DUMPGO
print("slow-go %08X\n",st.pc);
#endif
                a_slowgroup(g);
                slowcnt+=g->len;
            }
        }
        else
        { // no group, create one, restart op
            g=ac_creategroup(st.pc);
            opcode=mem_read32(st.pc);
            if(opcode==0x7000001c)
            {
                osStopCurrentThread();
                break;
            }
            if(OP_OP(opcode)!=OP_GROUP || st.pc==0x70707070 || st.pc==0 || opcode==0x70707070)
            {
                exception("Execution at invalid address.");
                break;
            }
        }
    }

    if(st.bailout<-1000000)
    {
        st.bailout=0;
        st.breakout=1;
    }

    st2.fastops+=startbail-st.bailout-slowcnt;
    st2.slowops+=slowcnt;

    if(st.bailout>BAILOUTNOW && (st.bailout<-2*30000 || st.bailout>startbail))
    {
        exception("compiler error: bailout is %i (was %i on entry)\n",st.bailout,startbail);
        st.bailout=0;
    }
}

