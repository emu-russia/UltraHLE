
#include "ultra.h"
#include "cpua.h"

static void ac_regsma(dword opcode,int type,int size,int fpu);

// globals used temporarily in asm:
static dword   asm_vmcache[VMCACHESIZE];
static dword   asm_groupto;
static dword   asm_temp1;
static dword   asm_temp2;
static dword   asm_temp3;
static dword   asm_temp4;

#include "noc.h"

int isnoc(dword addr)
{
    int i;
    for(i=1;noc[i];i++)
    {
        if(addr==noc[i]) return(1);
    }
    return(0);
}

void vmcache_add(int reg,int off)
{
    r.vmcachei=(r.vmcachei+1)&(VMCACHESIZE-1);
    insertmemwopcode(X_MOV,REG_NONE,(dword)(asm_vmcache+r.vmcachei),REG_ECX);
    r.vmcache[r.vmcachei].reg=reg;
    r.vmcache[r.vmcachei].off=off;
}

int vmcache_find(int reg,int off)
{
    int i;

    for(i=0;i<VMCACHESIZE;i++)
    {
        if(r.vmcache[i].reg==reg &&
           abs(r.vmcache[i].off-off)<120) return(i);
    }
    return(-1);
}

int vmcache_regreused(int reg)
{
    int i;
    i=(r.pc-r.pc0)/4+1;
    for(;i<r.len;i++)
    {
        if(r.op[i].r[0]==reg) return(0);
        if(r.op[i].memop && r.op[i].r[2]==reg) return(1);
    }
    return(0);
}

void vmcache_clear(int reg)
{
    int i;
    if(reg==-1)
    {
        r.vmcachei=0;
        r.lastmareg=-1;
        r.lastma=0xffffffff;
    }
    for(i=0;i<VMCACHESIZE;i++)
    {
        if(r.vmcache[i].reg==reg || reg<0)
        {
//            if(reg!=-1) print("vmclear %2i at %08X\n",reg,r.pc);
            r.vmcache[i].reg=-1;
            r.vmcache[i].off=0;
        }
    }
}

void regwrite(int xreg)
{
    reg_wr(xreg);
    if(r.opt_vmcache)
    {
//        print("vmcacheclear x%i (%i) at %08X\n",xreg,reg[xreg].name,r.pc);
        vmcache_clear(reg[xreg].name);
    }
    if(reg[xreg].name==r.lastmareg)
    {
        r.lastma=0xffffffff;
        r.lastmareg=-1;
    }
}

//------------------------ misc codes
//ONLY USE EAX

OBEGIN(o_int3)
    int 3
OEND

OBEGIN(o_nop)
    nop
OEND

OBEGIN(o_break)
    mov  eax,A(IP_P2)
    mov  [ebx+A(IP_P1)],eax  // will set st.breakout=2
OEND

OBEGIN(o_clear64bit)
    xor  eax,eax
    mov  [ebx+A(IP_P1)],eax
OEND

//------------------------ mul & div
//ONLY USE EAX,ECX

OBEGIN(o_mult)
    mov  ecx,edx
    mov  eax,[ebx+A(IP_RS)]
    imul dword ptr [ebx+A(IP_RT)]
    mov  [ebx+A(IP_P1)],eax
    mov  [ebx+A(IP_P2)],edx
    mov  edx,ecx
OEND

OBEGIN(o_multu)
    mov  ecx,edx
    mov  eax,[ebx+A(IP_RS)]
    mul  dword ptr [ebx+A(IP_RT)]
    mov  [ebx+A(IP_P1)],eax
    mov  [ebx+A(IP_P2)],edx
    mov  edx,ecx
OEND

OBEGIN(o_div1)
    mov  eax,[ebx+A(IP_RS)]
    mov  ecx,dword ptr [ebx+A(IP_RT)]
OEND
RBEGIN(or_div)
    test ecx,ecx
    jz   dz
    mov  [asm_temp1],edx
    cdq
    idiv ecx
    mov  ecx,edx
    mov  edx,[asm_temp1]
    ret
dz: xor  eax,eax
    xor  ecx,ecx
    ret
REND
RBEGIN(or_divu)
    test ecx,ecx
    jz   dz
    mov  [asm_temp1],edx
    xor  edx,edx
    div  ecx
    mov  ecx,edx
    mov  edx,[asm_temp1]
    ret
dz: xor  eax,eax
    xor  ecx,ecx
    ret
REND
OBEGIN(o_div2)
    mov  [ebx+A(IP_P1)],eax
    mov  [ebx+A(IP_P2)],ecx
OEND

//------------------------ memory lwlrsdfas argh opcodes

OBEGIN(o_lwl)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
    mov  [asm_temp3],ecx
    //
    mov  eax,ecx
    and  ecx,3
    sub  eax,ecx
    shl  ecx,3
    // y<<=s*8;
    mov  edx,[eax]
    shl  edx,cl
    // m=0x00ffffff >> ((3-s)*8);
    xor  ecx,3*8
    mov  esi,0x00ffffff
    shr  esi,cl
    //
    and  esi,[ebx+A(IP_RT)]
    or   esi,edx
    mov  [ebx+A(IP_RT)],esi
    //
    mov  ecx,[asm_temp3]
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_lwr)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
    mov  [asm_temp3],ecx
    //
    mov  eax,ecx
    and  ecx,3
    sub  eax,ecx
    shl  ecx,3
    // m=0xffffff00 << (s*8);
    mov  esi,0xffffff00
    shl  esi,cl
    // y>>=(3-s)*8;
    xor  ecx,3*8
    mov  edx,[eax]
    shr  edx,cl
    //
    and  esi,[ebx+A(IP_RT)]
    or   esi,edx
    mov  [ebx+A(IP_RT)],esi
    //
    mov  ecx,[asm_temp3]
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_swl)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
    mov  [asm_temp3],ecx
    //
    mov  eax,ecx
    and  ecx,3
    sub  eax,ecx
    shl  ecx,3
    // y>>=s*8;
    mov  edx,[ebx+A(IP_RT)]
    shr  edx,cl
    // m=0xffffff00 << ((3-s)*8);
    xor  ecx,3*8
    mov  esi,0xffffff00
    shl  esi,cl
    //
    and  esi,[eax]
    or   esi,edx
    mov  [eax],esi
    //
    mov  ecx,[asm_temp2]
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_swr)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
    mov  [asm_temp3],ecx
    //
    mov  eax,ecx
    and  ecx,3
    sub  eax,ecx
    shl  ecx,3
    // m=0x00ffffff >> (s*8);
    mov  esi,0x00ffffff
    shr  esi,cl
    // y<<=(3-s)*8;
    xor  ecx,3*8
    mov  edx,[ebx+A(IP_RT)]
    shl  edx,cl
    //
    and  esi,[eax]
    or   esi,edx
    mov  [eax],esi
    //
    mov  ecx,[asm_temp2]
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

/***********************************************************************
** Full register (interger/fp) flush
*/

static void ac_flushregs(int allbut1,int allbut2)
{
    freg_saveall();
    if(allbut1 || allbut2) reg_freeallbut(allbut1,allbut2);
    else reg_freeall();
}

/***********************************************************************
** Basic jump operations (which end the group)
*/

static void ac_jumptogroup(int x86reg)
{
    // move group to eax
    insertregopcode(X_MOV,REG_EAX,x86reg);
    // load group length
    insertimmopcode(X_IMMMOV,REG_ECX,-r.g->len);
    insertjump(fastexec_loop); // insertbyte(X_RET);
    r.inserted=99;
}

OBEGIN(o_memiocheck)
    test esi,esi
    jnz  l1
    jmp  ac_jumptogroup // jump is replaced
l1:
OEND

//#define OLDSTYLE

static void ac_jumptoreg(int x86reg,int isret)
{
#if OLDSTYLE
    // move PC to edx
    insertregopcode(X_MOV,REG_EDX,x86reg);
    if(isret && r.opt_domemio)
    {
        insertmemropcode(X_MOV,REG_ESI,REG_EBX,STADDR(st.memiodetected));
    }
    // load group length
    insertimmopcode(X_IMMMOV,REG_ECX,-r.g->len);
    // clear eax
    insertregopcode(X_XOR,REG_EAX,REG_EAX);
    if(isret)
    {
        // clear 64bit variable in st (all reg jumps, mainly JR RA clear it)
        insertmemwopcode(X_MOV,REG_EBX,STADDR(st.expanded64bit),REG_EAX);
        if(r.opt_domemio)
        {
            // check memio (returns if not)
            insert(o_memiocheck);
            // replace jump
            mem.codeused-=5;
            insertjump(fastexec_loop); // insertbyte(X_RET);
            // insert CALL or_memiocheck, RET
            insertbyte(0x60); // pushad
            insertcall(hw_memio);
            insertbyte(0x61); // popad
        }
        insertjump(fastexec_loop); // insertbyte(X_RET);
    }
    else
    {
        // insert RET
        insertjump(fastexec_loop); // insertbyte(X_RET);
    }
#else
    // move PC to edx
    insertregopcode(X_MOV,REG_EDX,x86reg);
    // load group length
    insertimmopcode(X_IMMMOV,REG_ECX,-r.g->len);
    // clear eax
    if(isret)
    {
        insertjump(fastexec_loopjrra);
    }
    else
    {
        // insert RET
        insertregopcode(X_XOR,REG_EAX,REG_EAX);
        insertjump(fastexec_loop); // insertbyte(X_RET);
    }
#endif
    r.inserted=99;
}

static void ac_jumpto(dword addr)
{
    dword ng;
    ng=(dword)ac_creategroup(addr);
    insertimmopcode(X_IMMMOV,REG_EAX,ng);
    ac_jumptogroup(REG_EAX);
}

/***********************************************************************
** Compilers for JUMPS and BRANCHES
*/

static void ac_jump(dword opcode,int options)
{
    int   rs,imm,xrs;

    rs=OP_RS(opcode);
    imm=((OP_TAR(opcode)<<2)&0x0fffffff)|(r.pc&0xf0000000);

    // execute delay slot
    ac_compileop(r.pc+4);

    // find where source reg is stored
    if(options&J_TOREG)
    {
        xrs=reg_find(rs);
        // must do this before freeall, and remember that
        // the value stays there even after freeall
    }

    // flush registers
    ac_flushregs(0,0);

    // link
    if(options&J_LINK)
    {
        insertimmopcode(X_IMMMOV,REG_EAX,r.pc+4); // delay slot already added 4
        insertmemwopcode(X_MOV,REG_EBX,STADDR(RA.d),REG_EAX);
    }

    // jump
    if(options&J_TOREG)
    {
        if(xrs==REG_NONE)
        {
            xrs=REG_EDX;
            insertmemropcode(X_MOV,xrs,REG_EBX,STADDR(st.g[rs]));
        }
        ac_jumptoreg(xrs,rs==0x1f);
    }
    else
    {
        ac_jumpto(imm);
    }
}

// generic 2 register arithmetic op
void ac_branch(dword opcode, int cmpop, int flags)
{
    int rs,rt;
    int xrs,xrt,xrd,xcm;
    int imm,slt_flip=0;
    int nodelayslot=0;
    dword truepc,falsepc;
    dword jumptrue,jumpfalse;
    static XReg regbak[8];

    imm=SIGNEXT16(OP_IMM(opcode));
    if((flags&BR_LIKELY) && cmpop==CMP_ALWAYS)
    {
        flags&=~BR_LIKELY;
        // remove likely flag if it's a forced jump
        // (likely slot will be executed anyway)
    }

    // convert likely to normal jump if delay slot found
    // at target-4
    if(flags&BR_LIKELY)
    {
        if(mem_readop(r.pc+4)==mem_readop(r.pc+4+imm*4-4))
        {
            imm--;
            nodelayslot=1;
            flags&=~BR_LIKELY;
        }
    }

    #if 0
    if(flags&BR_CMPFPU)
    {
        /*
        if(isnoc(r.pc+4+imm*4))
        {
            r.errors++;
            print("at %08X cmpop %04X flags %04X true %08X\n",r.pc,cmpop,flags,r.pc+4+imm*4);
        }
        */
        if(r.pc==0x8008c5a4) r.errors++;
    }
    #endif

    truepc   =r.pc+4+imm*4;
    falsepc  =r.pc+8;
    jumptrue =(dword)ac_creategroup(truepc);
    jumpfalse=(dword)ac_creategroup(falsepc);
//    print("\n#cmp true %08X (%08X) false %08X (%08X)\n",truepc,jumptrue,falsepc,jumpfalse);

    if(flags&BR_LIKELY)
    {
        dword delay,drs,drt;
        int op;

        // flush regs not used by delay slot
        delay=mem_readop(r.pc+4);

        op=getop(delay);
        drs=OP_RS(delay);
        if(op>=32 && op<=63) drt=0; // mem access
        else drt=OP_RT(delay);

        ac_flushregs(drs,drt);

        // store state of what's not saved
        memcpy(&regbak,&reg,sizeof(reg));
    }

    if(flags&BR_CMPFPU)
    {
        if(!(flags&BR_LIKELY))
        {
            // alloc EDX register for compare result
            xrd=reg_alloc(XNAME_TEMP+1,REG_EDX);
            // - clear edx
            insertregopcode(X_XOR,xrd,xrd);
        }

        // test fpu flag
        insertmemropcode(X_MOV,REG_EAX,REG_EBX,STADDR(st.fputrue));
        insertregopcode(X_TEST,REG_EAX,REG_EAX);
    }
    else if(cmpop!=CMP_ALWAYS)
    {
        if(!(flags&BR_LIKELY))
        {
            // alloc EDX register for compare result
            xrd=reg_alloc(XNAME_TEMP+1,REG_EDX);
            // - clear edx
            insertregopcode(X_XOR,xrd,xrd);
        }

        rs=OP_RS(opcode);
        rt=OP_RT(opcode);
        if(flags&BR_CMPZERO) rt=0;

        xcm=REG_EAX; // register we use for comparing

        if(r.slt_branch)
        {
            rs=r.slt_rs;
            rt=r.slt_rt;
            if(flags&BR_LIKELY)
            {
                r.errors++;
                exception("invalid slt optimization (likely branch)\n");
            }
            else if(cmpop==CMP_NE)
            {
                if(r.slt_branch==2) cmpop=CMP_LTUNS;
                else                cmpop=CMP_LT;
                slt_flip=0;
            }
            else if(cmpop==CMP_EQ)
            {
                if(r.slt_branch==2) cmpop=CMP_LTUNS;
                else                cmpop=CMP_LT;
                slt_flip=1;
            }
            else
            {
                exception("invalid slt optimization\n");
                r.errors++;
            }
        }

        xrs=reg_find(rs);
        if(rt!=-1) xrt=reg_find(rt);
        else xrt=REG_NONE;

        // perform compare:
        // - load xrs to EAX
        // - cmp to xcm
        // - setXX al
        if(rt==0 && xrs!=REG_NONE)
        {
            // direct test of register
            insertregopcode(X_TEST,xrs,xrs);
        }
        else
        {
            if(xrs!=REG_NONE && rt==-1)
            {
                insertimmopcode(X_IMMCMP,xrs,r.slt_imm);
            }
            else if(xrs!=REG_NONE && xrt!=REG_NONE)
            {
                insertregopcode(X_CMP,xrs,xrt);
            }
            else
            {
                if(rs==0)
                {
                    insertregopcode(X_XOR,xcm,xcm);
                }
                else if(xrs==REG_NONE)
                {
                    // rs not in reg, load from mem
                    insertmemropcode(X_MOV,xcm,REG_EBX,STADDR(st.g[rs]));
                }
                else
                {
                    // rs is in reg, just a reg move
                    insertregopcode(X_MOV,xcm,xrs);
                }
                if(rt==-1)
                {
                    insertimmopcode(X_IMMCMP,xcm,r.slt_imm);
                }
                else if(rt==0)
                {
                    insertregopcode(X_TEST,xcm,xcm);
                }
                else if(xrt==REG_NONE)
                {
                    // rs not in reg, load from mem
                    insertmemropcode(X_CMP,xcm,REG_EBX,STADDR(st.g[rt]));
                }
                else
                {
                    // rs is in reg, just a reg move
                    insertregopcode(X_CMP,xcm,xrt);
                }
            }
        }
    }

    if(flags&BR_LIKELY)
    {
        byte *fixup;
        int   offset;

        // - jxx OVER
        switch(cmpop^CMP_INVERT)
        {
        case CMP_EQ: insertbyte(0x74); break;
        case CMP_NE: insertbyte(0x75); break;
        case CMP_LT: insertbyte(0x7C); break;
        case CMP_LE: insertbyte(0x7E); break;
        case CMP_GT: insertbyte(0x7F); break;
        case CMP_GE: insertbyte(0x7D); break;
        case CMP_LTUNS: insertbyte(0x72); break;
        case CMP_LEUNS: insertbyte(0x76); break;
        case CMP_GTUNS: insertbyte(0x77); break;
        case CMP_GEUNS: insertbyte(0x73); break;
        default: exception("cpuanew: invalid ac_branch type %04X %04X\n",cmpop,flags);
        }
        fixup=mem.code+mem.codeused;
        insertbyte(0x00);

        //---- branch true (likely)

        // execute delay slot
        ac_compileop(r.pc+4);
        // flush registers
        ac_flushregs(0,0);

        // do the jump
        insertimmopcode(X_IMMMOV,REG_EAX,jumptrue);
        ac_jumptogroup(REG_EAX);

        //---- branch false

        // set jxx offset (max 127 bytes!)
        offset=(dword)(mem.code+mem.codeused)-((dword)fixup+1);
        *fixup=offset;
        if(offset>127)
        {
            // too long, too bad
            r.errors|=ERROR_BRANCH;
        }

        // flush registers (based on regbak taken before the
        // delay slot was executed)
        memcpy(&reg,&regbak,sizeof(reg));
        ac_flushregs(0,0);

        // do the jump
        insertimmopcode(X_IMMMOV,REG_EAX,jumpfalse);
        ac_jumptogroup(REG_EAX);
    }
    else
    {
        if(cmpop!=CMP_ALWAYS)
        {
            // - setxx DL
            insertbyte(0x0F);
            switch(cmpop)
            {
            case CMP_EQ: insertbyte(0x94); break;
            case CMP_NE: insertbyte(0x95); break;
            case CMP_LT: insertbyte(0x9C); break;
            case CMP_LE: insertbyte(0x9E); break;
            case CMP_GT: insertbyte(0x9F); break;
            case CMP_GE: insertbyte(0x9D); break;
            case CMP_LTUNS: insertbyte(0x92); break;
            case CMP_LEUNS: insertbyte(0x96); break;
            case CMP_GTUNS: insertbyte(0x97); break;
            case CMP_GEUNS: insertbyte(0x93); break;
            default: exception("cpuanew: invalid ac_branch type %04X %04X\n",cmpop,flags);
            }
            insertbyte(0xC2);
        }

        if(r.slt_branch)
        {
            insertmemwopcode(X_MOV,REG_EBX,STADDR(st.g[1]),REG_EDX);
            if(slt_flip)
            {
                insertimmopcode(X_IMMXOR,REG_EDX,1);
            }
        }

        if(!nodelayslot)
        {
            // execute delay slot (EDX should stay reserved since its XNAME_TEMP)
            ac_compileop(r.pc+4);
        }

        // flush registers (will not destroy EDX)
        ac_flushregs(XNAME_TEMP+1,0);

        // load EAX with destination address
        if(cmpop==CMP_ALWAYS)
        {
            insertimmopcode(X_IMMMOV,REG_EAX,jumptrue);
        }
        else
        {
            dword *d=(dword *)(r.g->code-8);

            if(reg[REG_EDX].name!=XNAME_TEMP+1)
            {
                exception("cpuanew: branch EDX corrupted\n");
            }

            // - mov EAX,[g->code-8+EDX*4]
            insertbyte(0x8B);
            insertbyte(0x04);
            insertbyte(0x95);
            insertdword((dword)d);

            // write the true/false addresses
            d[0]=jumpfalse;
            d[1]=jumptrue;
        }

        // jump there
        ac_jumptogroup(REG_EAX);
    }
}

/***********************************************************************
** Compilers for FPU
*/

//------------------------ fpu helpers (only EAX used)

static float floatconst05=-0.4999f;

OBEGIN(o_cvt_w2s)
    fild  dword ptr [ebx+A(IP_FS1)]
    fstp  dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_w2d)
    fild  dword ptr [ebx+A(IP_FS1)]
    fstp  qword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_s2w)
    fld   dword ptr [ebx+A(IP_FS1)]
//    fadd  dword ptr floatconst05
    fistp dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_s2wopt)
    fistp dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_s2d)
    fld   dword ptr [ebx+A(IP_FS1)]
    fstp  qword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_d2w)
    fld   qword ptr [ebx+A(IP_FS1)]
//    fadd  dword ptr floatconst05
    fistp dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_cvt_d2s)
    fld   qword ptr [ebx+A(IP_FS1)]
    fstp  dword ptr [ebx+A(IP_FD)]
OEND

// routines to load/store fpu temps

OBEGIN(o_fpu_ls)
    fld  dword ptr [ebx+A(IP_FS1)]
    fld  dword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu_ls1)
    fld  dword ptr [ebx+A(IP_FS1)]
OEND

OBEGIN(o_fpu_ss)
    fstp dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_fpu_ld)
    fld  qword ptr [ebx+A(IP_FS1)]
    fld  qword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu_ld1)
    fld  qword ptr [ebx+A(IP_FS1)]
OEND

OBEGIN(o_fpu_sd)
    fstp qword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_fpu_cmp)
    fstsw ax
    and eax,A(IP_P1)
    mov [ebx+A(IP_P2)],eax
OEND

OBEGIN(o_fpu4_cmp)
    fld    dword ptr [ebx+A(IP_FS1)]
    fcomp  dword ptr [ebx+A(IP_FS2)]
    fstsw ax
    and eax,A(IP_P1)
    mov [ebx+A(IP_P2)],eax
OEND

OBEGIN(o_fpu8_cmp)
    fld    qword ptr [ebx+A(IP_FS1)]
    fcomp  qword ptr [ebx+A(IP_FS2)]
    fstsw ax
    and eax,A(IP_P1)
    mov [ebx+A(IP_P2)],eax
OEND

// 387 status bits
#define FPU_ZERO      (1<<14)
#define FPU_UNORDEPUR (1<<10)
#define FPU_LESS      (1<<8)

#define REGMOVE_MLO -1
#define REGMOVE_MHI -2
#define REGMOVE_ZERO -3
static void ac_regmove(int fpureg,int intreg,int tofpu)
{
    int xrd,xrs;
    if(tofpu)
    {
        // intreg->fpureg
        if(intreg==0)
        {
            xrs=REG_EAX;
            insertregopcode(X_XOR,xrs,xrs);
        }
        else
        {
            xrs=reg_find(intreg);
            if(xrs==REG_NONE)
            {
                xrs=REG_EAX;
                insertmemropcode(X_MOV,xrs,REG_EBX,STADDR(st.g[intreg]));
            }
        }
        if(fpureg==REGMOVE_MLO)      insertmemwopcode(X_MOV,REG_EBX,STADDR(st.mlo),xrs);
        else if(fpureg==REGMOVE_MHI) insertmemwopcode(X_MOV,REG_EBX,STADDR(st.mhi),xrs);
        else                         insertmemwopcode(X_MOV,REG_EBX,STADDR(st.f[fpureg]),xrs);
    }
    else
    {
        // fpureg->intreg
        xrd=reg_allocnew(intreg);
        if(fpureg==REGMOVE_MLO)       insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.mlo));
        else if(fpureg==REGMOVE_MHI)  insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.mhi));
        else if(fpureg==REGMOVE_ZERO) insertregopcode(X_XOR,xrd,xrd);
        else                          insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.f[fpureg]));
        regwrite(xrd);
        // ecx address cache changed
        if(r.lastmareg==intreg)
        {
            r.lastma=0xffffffff;
            r.lastmareg=-1;
        }
    }
}

void ac_fpulwsw(dword opcode,int op)
{
    int rt=OP_RT(opcode),f;
    switch(op)
    {
    /*
    case 53: // LDC1
        ac_regsma(opcode,M_RD,0,0);
        insertmodrmopcode(X_FLD8,0,REG_ECX,0);
        freg_pushtop(OP_RT(opcode),8);
        break;
    case 61: // SDC1
        freg_push(OP_RT(opcode),8);
        ac_regsma(opcode,M_RD,0,0);
        insertmodrmopcode(X_FSTP8,0,REG_ECX,0);
        freg_poptop();
        break;
    case 49: // LWC1
        ac_regsma(opcode,M_RD,0,0);
        insertmodrmopcode(X_FLD4,0,REG_ECX,0);
        freg_pushtop(OP_RT(opcode),4);
        print("pushtop %i\n",OP_RT(opcode));
        break;
    case 57: // SWC1
        freg_push(OP_RT(opcode),4);
        ac_regsma(opcode,M_RD,0,0);
        insertmodrmopcode(X_FSTP4,0,REG_ECX,0);
        freg_poptop();
        break;
    */
    case 53: // LDC1
        ac_regsma(opcode,M_RD,4,2);
        ac_regsma(opcode,M_RD,4,3);
        f=freg_find(rt);   if(f!=REG_NONE) freg_delete(f);
        f=freg_find(rt+1); if(f!=REG_NONE) freg_delete(f);
        break;
    case 61: // SDC1
        f=freg_find(rt);   if(f!=REG_NONE) freg_save(f);
        f=freg_find(rt+1); if(f!=REG_NONE) freg_save(f);
        ac_regsma(opcode,M_WR,4,2);
        ac_regsma(opcode,M_WR,4,3);
        break;
    case 49: // LWC1
        ac_regsma(opcode,M_RD,4,1);
        rt&=~1;
        f=freg_find(rt);   if(f!=REG_NONE) freg_delete(f);
        rt|=1;
        f=freg_find(rt);   if(f!=REG_NONE) freg_delete(f);
        break;
    case 57: // SWC1
        rt&=~1;
        f=freg_find(rt);   if(f!=REG_NONE) freg_save(f);
        rt|=1;
        f=freg_find(rt);   if(f!=REG_NONE) freg_save(f);
        ac_regsma(opcode,M_WR,4,1);
        break;
    default:
        exception("cpuanew: invalid fpulwsw op\n");
        break;
    }
}

static int ac_fpu(dword opcode)
{
    int fmt=OP_RS(opcode);
    int op=OP_FUNC(opcode);
    int ret=1; // return 2 if delay slot also parsed
    int i;

freg_saveall(); //required to get rid of accuarcy problems :(

    r.fpuused=1;

    if(fmt<8)
    { // Move ops
        switch(fmt)
        {
        case 0: // MFC1
            i=freg_find(OP_RD(opcode));
            if(i!=REG_NONE) freg_save(i);
            ac_regmove(OP_RD(opcode),OP_RT(opcode),0);
            break;
        case 4: // MTC1
            i=freg_find(OP_RD(opcode));
            if(i!=REG_NONE) freg_delete(i);
            ac_regmove(OP_RD(opcode),OP_RT(opcode),1);
            break;
        case 2: // CFC0
            ac_regmove(REGMOVE_ZERO,OP_RT(opcode),0);
            break;
        case 6: // CTC0
            // ignore
            r.inserted=1;
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Move %i at %08X\n",fmt,r.pc);
            r.errors|=ERROR_FPU;
            break;
        }
    }
    else if(fmt==8)
    { // BC ops (branch)
        int rt=OP_RT(opcode);
        int ontrue=rt&1;
        int likely=((rt&2)>>1) ? BR_LIKELY : 0;

        if(ontrue)
        {
            ac_branch(opcode,CMP_NE,likely|BR_CMPFPU);
        }
        else
        {
            ac_branch(opcode,CMP_EQ,likely|BR_CMPFPU);
        }

        ret=2;
    }
    else if(op>=8 && op<=47)
    { // converts
        int ok=0;

        ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
        ip[IP_FD ]=STADDR(st.f[OP_SHAMT(opcode)]);

        i=freg_find(OP_RD(opcode));
        if(0 && i!=REG_NONE && op==13 && fmt==16)
        {
            //†† doesn't work
            ok=1;
            freg_xchg(i);
            insert(o_cvt_s2wopt);
            freg_poptop();
        }
        else
        {
            if(i!=REG_NONE) freg_save(i);
            i=freg_find(OP_SHAMT(opcode));
            if(i!=REG_NONE) freg_delete(i);

            switch(op)
            {
                case 32: // CVT.S [used]
                    if(fmt==20)      insert(o_cvt_w2s);
                    else if(fmt==17) insert(o_cvt_d2s);
                    ok=1;
                    break;
                case 33: // CVT.D [used]
                    if(fmt==20)      insert(o_cvt_w2d);
                    else if(fmt==16) insert(o_cvt_s2d);
                    ok=1;
                    break;
                case 36: // CVT.W
                    // rounding might be wrong, but it's still close
                    if(fmt==17)      insert(o_cvt_d2w);
                    else if(fmt==16) insert(o_cvt_s2w);
                    ok=1;
                    break;
                case 12: // ROUND.W
                    // use same code as CVT for now (rounding might be wrong)
                    if(fmt==17)      insert(o_cvt_d2w);
                    else if(fmt==16) insert(o_cvt_s2w);
                    ok=1;
                    break;
                case 13: // TRUN.W [used] TRUNC.W
                    // use same code as CVT for now (rounding might be wrong)
                    if(fmt==17)      insert(o_cvt_d2w);
                    else if(fmt==16) insert(o_cvt_s2w);
                    ok=1;
                    break;
                case 14: // CEIL.W
                    break;
                case 15: // FLOOR.W
                    break;
            }

            i=freg_find(OP_SHAMT(opcode));
            if(i!=REG_NONE) freg_delete(i);
        }

        if(!ok)
        {
            insert(o_int3);
            logi("compiler: unimplemented FPU-Convert %i at %08X\n",op,r.pc);
            r.errors|=ERROR_FPU;
        }
    }
    else if(op>=48)
    { // compares
        int rt;

        switch(op)
        {
        case 48: // C.F
        case 49: // C.UN
        case 56: // C.SF
print("strange FPU cmp at %08X\n",r.pc);
        case 57: // C.NGLE
            ip[IP_P1]=0*FPU_ZERO+0*FPU_LESS; // bits we are interested in
            break;
        case 51: // C.UEQ
        case 58: // C.SEQ
        case 59: // C.NGL
print("strange FPU cmp at %08X\n",r.pc);
        case 50: // C.EQ   [actually used]
            ip[IP_P1]=1*FPU_ZERO+0*FPU_LESS; // bits we are interested in
            break;
        case 52: // C.OLT
        case 53: // C.ULT
        case 61: // C.NGE
print("strange FPU cmp at %08X\n",r.pc);
        case 60: // C.LT   [actually used]
            ip[IP_P1]=0*FPU_ZERO+1*FPU_LESS; // bits we are interested in
            break;
        case 54: // C.OLE
        case 55: // C.ULE
        case 63: // C.NGT
print("strange FPU cmp at %08X\n",r.pc);
        case 62: // C.LE   [actually used]
            ip[IP_P1]=1*FPU_ZERO+1*FPU_LESS; // bits we are interested in
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Op %i at %08X\n",op,r.pc);
            r.errors|=ERROR_FPU;
            break;
        }
        ip[IP_P2]=STADDR(st.fputrue);

        i=freg_find(OP_RD(opcode));
        if(i==0)
        {
            rt=freg_find(OP_RT(opcode));
            if(rt!=REG_NONE)                         
            {
                // FCOM st,st(i)
                insertbyte(0xD8);
                insertbyte(0xD0+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FCOM4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else if(fmt==17) insertmodrmopcode(X_FCOM8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
        }
        else
        {
            if(fmt==16)
            {
                freg_push(OP_RD(opcode),4);
            }
            else if(fmt==17)
            {
                freg_push(OP_RD(opcode),8);
            }
            else
            {
                r.errors|=ERROR_FPU;
            }

            rt=freg_find(OP_RT(opcode));
            if(rt!=REG_NONE)
            {
                // FCOMP st,st(i)
                insertbyte(0xD8);
                insertbyte(0xD8+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FCOMP4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else if(fmt==17) insertmodrmopcode(X_FCOMP8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));

            freg_poptop();
        }
        insert(o_fpu_cmp);
    }
    else
    { // generic ops & compares
        int rt;
        ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
        ip[IP_FS2]=STADDR(st.f[OP_RT(opcode)]);
        ip[IP_FD] =STADDR(st.f[OP_SHAMT(opcode)]);
        ip[IP_P1] =STADDR(st.fputmp);

        if(fmt==16)
        { // single
            freg_push(OP_RD(opcode),4);
        }
        else if(fmt==17)
        { // double
            freg_push(OP_RD(opcode),8);
        }
        else
        {
            op=255; // force error (only word ops are the converts)
        }
        r.inserted=0;

        rt=freg_find(OP_RT(opcode));
        switch(op)
        {
        case 0: // FADD
            if(rt!=REG_NONE)
            {
                insertbyte(0xD8);
                insertbyte(0xC0+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FADD4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else             insertmodrmopcode(X_FADD8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            break;
        case 1: // FSUB
            if(rt!=REG_NONE)
            {
                insertbyte(0xD8);
                insertbyte(0xE0+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FSUB4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else             insertmodrmopcode(X_FSUB8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            break;
        case 2: // FMUL
            if(rt!=REG_NONE)
            {
                insertbyte(0xD8);
                insertbyte(0xC8+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FMUL4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else             insertmodrmopcode(X_FMUL8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            break;
        case 3: // FDIV
            if(rt!=REG_NONE)
            {
                insertbyte(0xD8);
                insertbyte(0xF0+rt);
            }
            else if(fmt==16) insertmodrmopcode(X_FDIV4,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            else             insertmodrmopcode(X_FDIV8,0,REG_EBX,STADDR(st.f[OP_RT(opcode)]));
            break;
        case 4: // SQRT
            insertbyte(0xD9);
            insertbyte(0xFA);
            break;
        case 5: // ABS
            insertbyte(0xD9);
            insertbyte(0xE1);
            break;
        case 6: // MOV
            // just the rename at end will do
            r.inserted=1;
            break;
        case 7: // NEG
            insertbyte(0xD9);
            insertbyte(0xE0);
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Op %i at %08X\n",op,r.pc);
            r.errors|=ERROR_FPU;
            break;
        }

        if(r.inserted)
        {
            // doesn't really save, just marks for saving
            if(fmt==16)
            { // single
                freg_renametop(OP_SHAMT(opcode),4);
            }
            else if(fmt==17)
            { // double
                freg_renametop(OP_SHAMT(opcode),8);
            }
        }
    }

    if(!r.inserted)
    {
        insert(o_int3);
        logi("compiler: FPU TBD op %i at %08X\n",op,r.pc);
        r.errors|=ERROR_FPU;
        r.inserted=1;
    }
    return(ret);
}

/***********************************************************************
** Compilers for REGS ops
*/

//-------------------------- helpers for regsma()

OBEGIN(o_eaxecxlookupr)
{
    mov  ecx,eax
    shr  eax,12
    add  ecx,[OFFSET mem.lookupr+eax*4]
}
OEND

OBEGIN(o_eaxecxlookupw)
{
    mov  ecx,eax
    shr  eax,12
    add  ecx,[OFFSET mem.lookupw+eax*4]
}
OEND

OBEGIN(o_lhu1)
{
    xor  ecx,2
    movzx eax,word ptr [ecx]
}
OEND

OBEGIN(o_lh1)
{
    xor  ecx,2
    movsx eax,word ptr [ecx]
}
OEND

OBEGIN(o_sh1)
{
    xor  ecx,2
    mov  [ecx],ax
}
OEND

OBEGIN(o_lbu1)
{
    xor  ecx,3
    movzx eax,byte ptr [ecx]
}
OEND

OBEGIN(o_lb1)
{
    xor  ecx,3
    movsx eax,byte ptr [ecx]
}
OEND

OBEGIN(o_sb1)
{
    xor  ecx,3
    mov  [ecx],al
}
OEND

// fpu parameter
// 1=load FPU reg
// 2=load FPU double reg 0 at offset 4
// 3=load FPU double reg 1 at offset 0
// size=0 means just calc address to ECX
static void ac_regsma(dword opcode,int type,int size,int fpu)
{
    int rs,rd;
    int xrs,xrd;
    int imm=SIGNEXT16(OP_IMM(opcode));
    int addecx=0,i;
    int vmcachehit=1;

//    insert(o_int3);

    cstat.inma++;

    rs=OP_RS(opcode);
    rd=OP_RT(opcode);

    // ecx address cache changed
    if(r.lastmareg==rd && size) r.lastma=0xffffffff;

    xrs=reg_find(rs);
    if(xrs!=REG_NONE) reg_rd(xrs); // protect by increasing lastused

    if(fpu)
    {
        if(fpu==2)
        {
            imm+=4;
            rd+=0;
        }
        if(fpu==3)
        {
            imm+=0;
            rd+=1;
        }
        xrd=REG_EAX;
    }
    else if(size>=0x10)
    {
        xrd=reg_find(rd);
        if(xrd!=REG_NONE) reg_free(xrd);
        xrd=REG_EAX;
    }
    else if(size!=0)
    {
        if(type==M_WR)
        {
            xrd=reg_find(rd);
            if(xrd==REG_NONE) xrd=REG_EAX;
        }
        else
        {
            xrd=reg_allocnew(rd);
        }
    }

    if(r.opt_adjacentvm &&
       rs==r.lastmareg &&
       r.lastma!=0xffffffff &&
       r.lastma!=0 &&
       abs(imm-r.lastmaoff)<120)
    {
//        print("adjacentvm %08X last %08X\n",r.pc,r.lastma);
        cstat.inmasimple++;
        addecx=imm-r.lastmaoff;
    }
    else if(r.opt_novm || (r.opt_nospvm && rs==0x1d))
    {
        r.lastmareg=-1;
        r.lastma=0xffffffff;
        // sp and fast sp mode on
        cstat.inmasimple++;
        // perform virtual memory lookup (->ECX)
        if(xrs==REG_NONE)
        {
            // rs not in reg, load from mem
            insertmemropcode(X_MOV,REG_ECX,REG_EBX,STADDR(st.g[rs]));
        }
        else
        {
            // rs is in reg, just a reg move
            insertregopcode(X_MOV,REG_ECX,xrs);
        }
        // mask to ramsize
        insertimmopcode(X_IMMAND,REG_ECX,mem.ramsize-1);
        // add immediate and lookup base
        insertimmopcode(X_IMMADD,REG_ECX,imm+(dword)mem.ram);
    }
    else if(r.opt_vmcache && (i=vmcache_find(rs,imm))>=0)
    {
        cstat.inmasimple++;
        insertmemropcode(X_MOV,REG_ECX,REG_NONE,(dword)(asm_vmcache+i));
        addecx=imm-r.vmcache[i].off;
//        print("vmcache %i at %08X reg r%02i+%-8i (last r%02i+%-8i) addecx=%i\n",
//            i,r.pc,rs,imm,r.vmcache[i].reg,r.vmcache[i].off,addecx);
    }
    else
    {
        vmcachehit=0;

        // perform virtual memory lookup (->ECX)
        if(xrs==REG_NONE)
        {
            // rs not in reg, load from mem
            insertmemropcode(X_MOV,REG_EAX,REG_EBX,STADDR(st.g[rs]));
        }
        else
        {
            // rs is in reg, just a reg move
            insertregopcode(X_MOV,REG_EAX,xrs);
            // TBD: convert to LEA
        }
        // add immediate (if present)
        if(imm)
        {
            insertimmopcode(X_IMMADD,REG_EAX,imm);
        }

        // perform virtual memory lookup
        if(type==M_WR) insert(o_eaxecxlookupw);
        else           insert(o_eaxecxlookupr);
    }

    if(size==4 || size==0)
    {
        // only dword accesses (or size=0 generic address calc)
        // leave ECX correctly
        r.lastma=r.pc;
        r.lastmareg=rs;
        r.lastmaoff=imm-addecx;
    }
    else
    {
        // lastma stuff corrupted
        r.lastmareg=-1;
        r.lastma=0xffffffff;
    }

    if(r.opt_vmcache && !vmcachehit && vmcache_regreused(rs))
    {
        vmcache_add(rs,imm);
    }

    if(size==0)
    {
        if(addecx)
        {
            insertimmopcode(X_IMMADD,REG_ECX,addecx);
            r.lastmaoff+=addecx;
        }
        // just calc address to ECX
        return;
    }
    else if(size>=0x10)
    {
        if(addecx)
        {
            insertimmopcode(X_IMMADD,REG_ECX,addecx);
        }
        ip[IP_RT]=STADDR(st.g[rd]);
        switch(size)
        {
        case 0x10: insert(o_lwl); break;
        case 0x11: insert(o_lwr); break;
        case 0x12: insert(o_swl); break;
        case 0x13: insert(o_swr); break;
        default: exception("cpuanew: illegal lwl-etc size\n"); break;
        }
    }
    else if(size==4)
    {
        // dword accesses have faster code and sign extend is ignored
        if(type==M_WR)
        {
            if(fpu)
            {
                insertmemropcode(X_MOV,REG_EAX,REG_EBX,STADDR(st.f[rd]));
            }
            else
            {
                if(rd==0)
                {
                    insertregopcode(X_XOR,REG_EAX,REG_EAX);
                }
                else if(xrd==REG_EAX)
                {
                    // rd is temp, load
                    insertmemropcode(X_MOV,REG_EAX,REG_EBX,STADDR(st.g[rd]));
                }
            }
            // store
            insertmemwopcode(X_MOV,REG_ECX,addecx,xrd);
        }
        else
        {
            // load
            insertmemropcode(X_MOV,xrd,REG_ECX,addecx);
            if(fpu)
            {
                insertmemwopcode(X_MOV,REG_EBX,STADDR(st.f[rd]),xrd);
            }
        }
    }
    else
    {
        // other sizes always go through eax
        if(addecx)
        {
            insertimmopcode(X_IMMADD,REG_ECX,addecx);
        }
        if(type==M_WR)
        {
            if(rd==0)
            {
                insertregopcode(X_XOR,REG_EAX,REG_EAX);
            }
            else if(xrd==REG_EAX)
            {
                // rd not in reg, load from mem
                insertmemropcode(X_MOV,REG_EAX,REG_EBX,STADDR(st.g[rd]));
            }
            else
            {
                // rd is in reg, just a reg move
                insertregopcode(X_MOV,REG_EAX,xrd);
            }
        }
        if(size==2)
        {
            if(type==M_WR) insert(o_sh1);
            else if(type==M_RDEXT) insert(o_lh1);
            else insert(o_lhu1);
        }
        else if(size==1)
        {
            if(type==M_WR) insert(o_sb1);
            else if(type==M_RDEXT) insert(o_lb1);
            else insert(o_lbu1);
        }
        if(type!=M_WR)
        {
            // move result to rd reg
            insertregopcode(X_MOV,xrd,REG_EAX);
        }
    }

    if(xrd!=REG_EAX)
    {
        if(type!=M_WR)
        {
            regwrite(xrd);
        }
        else
        {
            reg_rd(xrd);
        }
    }
}

// shift ops
static void ac_regssh(dword opcode,int x86op,int fromreg)
{
    int rs,rt,rd;
    int xrs,xrt,xrd;

    rt=OP_RT(opcode);
    rd=OP_RD(opcode);

    // ecx address cache changed
    if(r.lastmareg==rd)
    {
        r.lastmareg=-1;
        r.lastma=0xffffffff;
    }

    xrt=reg_find(rt);
    if(xrt!=REG_NONE) reg_rd(xrt); // protect by increasing lastused

    // load shift count
    if(fromreg)
    {
        // register shifts leave ECX corrupted
        r.lastmareg=-1;
        r.lastma=0xffffffff;

        rs=OP_RS(opcode);
        xrs=reg_find(rs);
        if(xrs==REG_NONE)
        {
            // no reg, do with a memory access
            insertmemropcode(X_MOV,REG_ECX,REG_EBX,STADDR(st.g[rs]));
        }
        else
        {
            // we have a reg, do a regop
            insertregopcode(X_MOV,REG_ECX,xrs);
        }
    }

    xrd=reg_allocnew(rd);

    // load value to be shifted to xrd
    if(xrt==REG_NONE)
    {
        // rt not in reg, load from mem
        insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.g[rt]));
    }
    else
    {
        // rt is in reg, just a reg move
        insertregopcode(X_MOV,xrd,xrt);
    }

    // shift
    if(fromreg)
    {
        insertregopcode(x86op,xrd,REG_ECX);
    }
    else
    {
        insertimmopcode(x86op,xrd,OP_SHAMT(opcode));
    }

    // mark changed, will be flushed later
    regwrite(xrd);
}

// slti ops (returns ops compiled)
static int ac_regsslt(dword opcode,int fromreg,int unsignedcmp)
{
    int rs,rt,rd;
    int xrs,xrt,xrd;
    int imm;

    if(!fromreg)
    {
        rs=OP_RS(opcode);
        rt=-1;
        rd=OP_RT(opcode);
        imm=SIGNEXT16(OP_IMM(opcode));
    }
    else
    {
        rs=OP_RS(opcode);
        rt=OP_RT(opcode);
        rd=OP_RD(opcode);
        imm=0;
    }

    r.slt_branch=0;
    if(r.opt_slt && rd==1)
    { // rd=1=AT
        dword nextop=mem_readop(r.pc+4);
        if(OP_RS(nextop)==1 && OP_RT(nextop)==0 && r.pc!=r.pc0+4*r.len-4)
        { // cmp at,r0
            r.slt_imm=imm;
            r.slt_rs=rs;
            r.slt_rt=rt;
            if(OP_OP(nextop)==5 || OP_OP(nextop)==4)
            { // SLT + BNE/BEQ
                if(unsignedcmp) r.slt_branch=2;
                else            r.slt_branch=1;
//                print("slt at %08X, rs=%2i rt=%2i imm=%4i b=%i \n",r.pc,rs,rt,imm,r.slt_branch);
                return(1);
            }
        }
    }

    // clear eax
    insertregopcode(X_XOR,REG_EAX,REG_EAX);

    xrs=reg_find(rs);
    if(xrs!=REG_NONE)
    {
        reg_rd(xrs); // protect by increasing lastused
    }
    else
    {
        // load to ECX
        xrs=REG_ECX;
        insertmemropcode(X_MOV,xrs,REG_EBX,STADDR(st.g[rs]));
    }

    if(rt==-1)
    {
        // compare to immediate
        insertimmopcode(X_IMMCMP,xrs,imm);
    }
    else
    {
        xrt=reg_find(rt);
        if(xrt!=REG_NONE)
        {
            reg_rd(xrt); // protect by increasing lastused
            insertregopcode(X_CMP,xrs,xrt);
        }
        else
        {
            insertmemropcode(X_CMP,xrs,REG_EBX,STADDR(st.g[rt]));
        }
    }

    // ecx address cache changed
    r.lastmareg=-1;
    r.lastma=0xffffffff;

    // alloc register for result
    xrd=reg_allocnew(rd);
    // load the value
    insertbyte(0xF);
    if(unsignedcmp) insertbyte(0x92); // SETB al
    else            insertbyte(0x9C); // SETL al
    insertbyte(0xC0);

    // mov data to xrd
    insertregopcode(X_MOV,xrd,REG_EAX);

    regwrite(xrd);

    return(1);
}

// generic 3 register arithmetic op
static void ac_regs3(dword opcode,int x86op)
{
    int rs,rt,rd,a;
    int xrs,xrt,xrd;
    int nor=0;

    if(x86op==X_NOR)
    {
        x86op=X_XOR;
        nor=1;
    }

    rs=OP_RS(opcode);
    rt=OP_RT(opcode);
    rd=OP_RD(opcode);

    // ecx address cache changed
    if(r.lastmareg==rd)
    {
        r.lastma=0xffffffff;
        r.lastmareg=-1;
    }

    xrs=reg_find(rs);
    if(xrs!=REG_NONE) reg_rd(xrs); // protect by increasing lastused

    xrt=reg_find(rt);
    if(xrt!=REG_NONE) reg_rd(xrt); // protect by increasing lastused

    xrd=reg_allocnew(rd);

    if(rs==0 && rt==0 && (x86op==X_ADD || x86op==X_OR))
    {
        // this is a CLEAR REG op
        insertregopcode(X_XOR,xrd,xrd);
    }
    else
    {
        if(xrt==xrd && xrt!=REG_NONE && xrt!=REG_EAX)
        {
            // both in regs && x=y-x (x would be corrupted by x=y, can't have that)
            if(x86op==X_SUB)
            {
                // convert x=y-x to x=-x + y
                x86op=X_ADD;
                // insert NEG
                insertbyte(0xF7);
                insertbyte(3*64+3*8+xrd);
            }
            // swap operands
            a=xrs; xrs=xrt; xrt=a;
            a=rs; rs=rt; rt=a;
        }

        if(rs==0)
        {
            insertregopcode(X_XOR,xrd,xrd);
        }
        else if(xrs==REG_NONE)
        {
            // rs not in reg, load from mem
            insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.g[rs]));
        }
        else
        {
            // rs is in reg, just a reg move
            insertregopcode(X_MOV,xrd,xrs);
        }

        if(rt==0 && (x86op==X_ADD || x86op==X_OR))
        {
            // done already (zero as a second parameter does nothing)
        }
        else
        {
            if(xrt==REG_NONE)
            {
                // no reg, do with a memory access
                insertmemropcode(x86op,xrd,REG_EBX,STADDR(st.g[rt]));
            }
            else
            {
                // we have a reg, do a regop
                insertregopcode(x86op,xrd,xrt);
            }
            if(nor)
            {
                // NOT xrd
                insertbyte(0xF7);
                insertbyte(3*64+2*8+xrd);
            }
        }
    }

    // mark changed, will be flushed later
    regwrite(xrd);
}

// generic 2 register 1 immediate arithmetic op
static void ac_regs2(dword opcode,int x86op,int signextend)
{
    int rs,rd;
    int xrs,xrd;
    int luimov=0;
    int imm=OP_IMM(opcode);

    if(signextend) imm=SIGNEXT16(imm);

    if(x86op==X_LUIMOV)
    {
        luimov=1;
        x86op=X_IMMMOV;
        imm<<=16;
    }

    rs=OP_RS(opcode);
    rd=OP_RT(opcode);

    // ecx address cache changed
    if(r.lastmareg==rd)
    {
        r.lastma=0xffffffff;
        r.lastmareg=-1;
    }

    xrs=reg_find(rs);
    if(xrs!=REG_NONE) reg_rd(xrs); // protect by increasing lastused

    xrd=reg_allocnew(rd);

    if(rs==0 && (x86op==X_IMMADD || x86op==X_IMMOR))
    {
        // just convert to load
        x86op=X_IMMMOV;
    }

    if(x86op!=X_IMMMOV)
    {
        if(xrs==REG_NONE)
        {
            // rs not in reg, load from mem
            insertmemropcode(X_MOV,xrd,REG_EBX,STADDR(st.g[rs]));
        }
        else
        {
            // rs is in reg, just a reg move
            insertregopcode(X_MOV,xrd,xrs);
        }
    }

    insertimmopcode(x86op,xrd,imm);

    if(luimov)
    {
        if(hw_ismemiorange(imm))
        {
            insertmemwopcode(X_MOV,REG_EBX,STADDR(st.memiodetected),xrd);
        }
    }
    // mark changed, will be flushed later
    regwrite(xrd);
}

static void preparemuldiv(dword opcode)
{
    int xrs,xrt,rs,rt;
    rs=OP_RS(opcode);
    rt=OP_RT(opcode);
    xrs=reg_find(rs);
    xrt=reg_find(rt);
    if(xrs!=REG_NONE) reg_save(xrs);
    if(xrt!=REG_NONE) reg_save(xrt);
    ip[IP_RS]=STADDR(st.g[rs]);
    ip[IP_RT]=STADDR(st.g[rt]);
    ip[IP_P1]=STADDR(st.mlo);
    ip[IP_P2]=STADDR(st.mhi);
    {
        r.lastma=0xffffffff;
        r.lastmareg=-1;
    }
}

int ac_compileopnew(dword pc,dword opcode,int op)
{
    int ret=1; // return 2 if delay slot also parsed

    if(op!=4 && op!=5) r.slt_branch=0;

    if(!opcode)
    {
        // nop
    }
    else switch(op)
    {
    case 53: // LDC1
    case 61: // SDC1
    case 49: // LWC1
    case 57: // SWC1
        cstat.infpu++;
        ac_fpulwsw(opcode,op);
        ret=1;
        break;
    case 17:
        cstat.infpu++;
        ret=ac_fpu(opcode);
        break;
    //------------------------------stores
    case 32: // LB
        ac_regsma(opcode,M_RDEXT,1,0);
        break;
    case 36: // LBU
        ac_regsma(opcode,M_RD,1,0);
        break;
    case 33: // LH
        ac_regsma(opcode,M_RDEXT,2,0);
        break;
    case 37: // LHU
        ac_regsma(opcode,M_RD,2,0);
        break;
    case 35: // LW
    case 39: // LWU
        ac_regsma(opcode,M_RD,4,0);
        break;
    //------------------------------stores
    case 40: // SB
        ac_regsma(opcode,M_WR,1,0);
        break;
    case 41: // SH
        ac_regsma(opcode,M_WR,2,0);
        break;
    case 43: // SW
        ac_regsma(opcode,M_WR,4,0);
        break;
    /*
    case 53: // LDC1
        cstat.infpu++;
        ac_regsma(opcode,M_RD,4,2);
        ac_regsma(opcode,M_RD,4,3);
        break;
    case 61: // SDC1
        cstat.infpu++;
        ac_regsma(opcode,M_WR,4,2);
        ac_regsma(opcode,M_WR,4,3);
        break;
    case 49: // LWC1
        cstat.infpu++;
        ac_regsma(opcode,M_RD,4,1);
        break;
    case 57: // SWC1
        cstat.infpu++;
        ac_regsma(opcode,M_WR,4,1);
        break;
    */
    //--------------argh move/stores
    case 34: // LWL
        ac_regsma(opcode,M_RD,0x10,0);
        break;
    case 38: // LWR
        ac_regsma(opcode,M_RD,0x11,0);
        break;
    case 42: // SWL
        ac_regsma(opcode,M_WR,0x12,0);
        break;
    case 46: // SWR
        ac_regsma(opcode,M_WR,0x13,0);
        break;
    //--------------
    case 8: // ADDI
    case 9: // ADDIU
        ac_regs2(opcode,X_IMMADD,1);
        break;
    case 12: // ANDI
        ac_regs2(opcode,X_IMMAND,0);
        break;
    case 13: // ORI
        ac_regs2(opcode,X_IMMOR,0);
        break;
    case 14: // XORI
        ac_regs2(opcode,X_IMMXOR,0);
        break;
    case 15: // LUI
        ac_regs2(opcode,X_LUIMOV,0);
        break;
    //--------------
    case 0x40+0: // SLL
        ac_regssh(opcode,X_SHLIMM,0);
        break;
    case 0x40+2: // SRL
        ac_regssh(opcode,X_SHRIMM,0);
        break;
    case 0x40+3: // SRA
        ac_regssh(opcode,X_SARIMM,0);
        break;
    case 0x40+4: // SLLV
        ac_regssh(opcode,X_SHLCL,1);
        break;
    case 0x40+6: // SRLV
        ac_regssh(opcode,X_SHRCL,1);
        break;
    case 0x40+7: // SRAV
        ac_regssh(opcode,X_SARCL,1);
        break;
    //--------------
    case 10: // SLTI
        ret=ac_regsslt(opcode,0,0);
        break;
    case 11: // SLTIU
        ret=ac_regsslt(opcode,0,1);
        break;
    case 0x40+42: // SLT
        ret=ac_regsslt(opcode,1,0);
        break;
    case 0x40+43: // SLTU
        ret=ac_regsslt(opcode,1,1);
        break;
    //--------------
    case 0x40+32: // ADD
    case 0x40+33: // ADDU
        ac_regs3(opcode,X_ADD);
        break;
    case 0x40+34: // SUB
    case 0x40+35: // SUBU
        ac_regs3(opcode,X_SUB);
        break;
    case 0x40+36: // AND
        ac_regs3(opcode,X_AND);
        break;
    case 0x40+37: // OR
        ac_regs3(opcode,X_OR);
        break;
    case 0x40+38: // XOR
        ac_regs3(opcode,X_XOR);
        break;
    case 0x40+39: // NOR
        ac_regs3(opcode,X_NOR);
        break;
    //--------------
    case 0x40+16: // MFHI
        ac_regmove(REGMOVE_MHI,OP_RD(opcode),0);
        break;
    case 0x40+17: // MTHI
        ac_regmove(REGMOVE_MHI,OP_RS(opcode),1);
        break;
    case 0x40+18: // MFLO
        ac_regmove(REGMOVE_MLO,OP_RD(opcode),0);
        break;
    case 0x40+19: // MTLO
        ac_regmove(REGMOVE_MLO,OP_RS(opcode),1);
        break;
    case 0x40+24: // MULT
        preparemuldiv(opcode);
        insert(o_mult);
        break;
    case 0x40+25: // MULTU
        preparemuldiv(opcode);
        insert(o_multu);
        break;
    case 0x40+26: // DIV
        preparemuldiv(opcode);
        insert(o_div1);
        insertcall(or_div);
        insert(o_div2);
        break;
    case 0x40+27: // DIVU
        preparemuldiv(opcode);
        insert(o_div1);
        insertcall(or_divu);
        insert(o_div2);
        break;
    //--------------
    case 2: // J
        ac_jump(opcode,0);
        ret=2;
        break;
    case 3: // JAL
        ac_jump(opcode,J_LINK);
        ret=2;
        break;
    case 0x40+8: // JR
        ac_jump(opcode,J_TOREG);
        ret=2;
        break;
    case 0x40+9: // JALR
        ac_jump(opcode,J_LINK|J_TOREG);
        ret=2;
        break;
    //--------------
    case 4: // BEQ
        if(OP_RS(opcode)==0 && OP_RT(opcode)==0)
             ac_branch(opcode,CMP_ALWAYS,0);
        else ac_branch(opcode,CMP_EQ,0);
        ret=2;
        break;
    case 5: // BNEQ
        ac_branch(opcode,CMP_NE,0);
        ret=2;
        break;
    case 6: // BLEZ
        ac_branch(opcode,CMP_LE,BR_CMPZERO);
        ret=2;
        break;
    case 7: // BGTZ
        ac_branch(opcode,CMP_GT,BR_CMPZERO);
        ret=2;
        break;
    case 20: // BEQL
        ac_branch(opcode,CMP_EQ,BR_LIKELY);
        ret=2;
        break;
    case 21: // BNEQL BNEL
        ac_branch(opcode,CMP_NE,BR_LIKELY);
        ret=2;
        break;
    case 22: // BLEZL
        ac_branch(opcode,CMP_LE,BR_CMPZERO|BR_LIKELY);
        ret=2;
        break;
    case 23: // BGTZL
        ac_branch(opcode,CMP_GT,BR_CMPZERO|BR_LIKELY);
        ret=2;
        break;
    case 0x80+0: // BLTZ
        ac_branch(opcode,CMP_LT,BR_CMPZERO);
        ret=2;
        break;
    case 0x80+1: // BGEZ
        ac_branch(opcode,CMP_GE,BR_CMPZERO);
        ret=2;
        break;
    case 0x80+2: // BLTZL
        ac_branch(opcode,CMP_LT,BR_CMPZERO|BR_LIKELY);
        ret=2;
        break;
    case 0x80+3: // BGEZL
        ac_branch(opcode,CMP_GE,BR_CMPZERO|BR_LIKELY);
        ret=2;
        break;
     //--------------
    default:
        r.errors|=ERROR_OP;
        return(0); // not compiled here
    }
    cstat.inregs++;
    return(ret);
}

static void a_analyzeops(void)
{
    int i,op;
    dword opcode,a;
    for(i=0;i<r.len;i++)
    {
        a=i*4+r.pc0;
        r.op[i].pc=a;
        opcode=mem_readop(a);
        op=getop(opcode);
        r.op[i].r[0]=-1;
        r.op[i].r[1]=-1;
        r.op[i].r[2]=-1;
        r.op[i].memop=0;

        if(op>=32 && op<=39)
        { // Load mem
            r.op[i].memop=1;
            r.op[i].r[0]=OP_RT(opcode);
            r.op[i].r[2]=OP_RS(opcode);
        }
        else if(op>=40 && op<=47)
        { // Store mem
            r.op[i].memop=2;
            r.op[i].r[1]=OP_RT(opcode);
            r.op[i].r[2]=OP_RS(opcode);
        }
        else if(op>=8 && op<=15)
        {
            r.op[i].r[0]=OP_RT(opcode);
            r.op[i].r[1]=OP_RS(opcode);
        }
        else if(op>=0x40 && op<=0x47)
        { // shifts
            r.op[i].r[0]=OP_RD(opcode);
            r.op[i].r[1]=OP_RT(opcode);
            r.op[i].r[2]=OP_RS(opcode);
        }
        else if(op>=0x40+32 && op<=0x40+43)
        {
            r.op[i].r[0]=OP_RD(opcode);
            r.op[i].r[1]=OP_RT(opcode);
            r.op[i].r[2]=OP_RS(opcode);
        }

        if(!r.op[i].r[0]) r.op[i].r[0]=-1;
        if(!r.op[i].r[1]) r.op[i].r[1]=-1;
        if(!r.op[i].r[2]) r.op[i].r[2]=-1;

        /*
        print("group %08X/%i: %02X regs %02i %02i %02i\n",
            r.pc0,i,op,
            r.op[i].r[0],
            r.op[i].r[1],
            r.op[i].r[2]);
        */
    }
}

static void ac_compilealign(void)
{
    // align to 16 byte boundary -15
    while((mem.codeused+15)&0x0f) insertbyte(X_INT3); // int 3
    // These are never executed but when looking at the code
    // in an X86 debugger you see what group comes from where
    //
    // before group goes: (total 3+3*4=15 bytes)
    // MOV EAX, groupc
    // MOV [tmp1],tmp2
    insertbyte(0xb8);
    insertdword(st.pc);
    insertbyte(0xc7);
    insertbyte(0x05);
    insertdword(0x11111111);
    insertdword(0x22222222);
}

dword urpoenv[32];
dword *urpox=&st.breakout;

OBEGIN(o_urpo2_checkfpustack)
fnstenv ds:urpoenv
fwait
mov     eax,ds:[urpoenv+8]
and     eax,0xffff
cmp     eax,0xffff
je      ok
mov     eax,-1
mov     ds:[ebx+0],eax
mov     edx,[urpox]
mov     eax,5
mov     ds:[edx],eax
ok:
OEND

void ac_compilestartnew(void)
{
    Group *g=r.g;
    a_analyzeops();

    // clear regs
    reg_init();
    freg_init();

    // align to a 16 byte boundary and insert
    // a MOV eax,PC for debuggers to see
    ac_compilealign();
    // set codebase to group
    g->code=mem.code+mem.codeused;

    vmcache_clear(-1);
//    insertbyte(0xCC);

//    insert(o_urpo2_checkfpustack);
}

void ac_compileendnew(void)
{
    Group *g=r.g;

    if(r.inserted!=99)
    {
        // flush registers
        ac_flushregs(0,0);

        // insert end jump
        ac_jumpto(g->addr+g->len*4);
    }

    insertbyte(X_INT3); // should never get here
}

