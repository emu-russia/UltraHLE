#include "ultra.h"
#include "cpua.h"

// globals used temporarily in asm:
static dword   asm_groupto;
static dword   asm_temp1;
static dword   asm_temp2;
static dword   asm_temp3;
static dword   asm_temp4;

//------------------------ return codes (these ONLY may return to a_fastexec)
//ALL REG LIMITS

OBEGIN(o_gotoreg)
    mov  edx,[ebx+A(IP_RS)]
    mov  ecx,A(IP_RETLEN)
    xor  eax,eax
    ret  // EAX=0, ECX=this group length, EDX=new pc
OEND

OBEGIN(o_gotoreg_ret)
    mov  edx,[ebx+A(IP_RS)]
    mov  esi,[ebx+A(IP_P2)]
    xor  eax,eax
    mov  ecx,A(IP_RETLEN)
    mov  [ebx+A(IP_P1)],eax
    test esi,esi
    jnz  l1
    ret  // EAX=0, ECX=this group length, EDX=new pc
l1:
OEND

OBEGIN(o_goto_bailoutcheck)
    mov  eax,[ebx+A(IP_P1)]
    add  eax,A(IP_RETLEN)
    mov  [ebx+A(IP_P1)],eax
OEND

OBEGIN(o_gotogroupto)
    mov  eax,[asm_groupto]
    mov  ecx,A(IP_RETLEN)
    ret  // EAX=new group, ECX=this group length
OEND

OBEGIN(o_gotogroupeax)
    mov  ecx,A(IP_RETLEN)
    ret  // EAX=new group, ECX=this group length
OEND

OBEGIN(o_gotogroup_retalways)
    mov  eax,A(IP_RETGROUP)
    mov  ecx,A(IP_RETLEN)
    ret  // EAX=new group, ECX=this group length
OEND

// this fast version includes code from a_fastgroup.
OBEGIN(o_gotogroup)
    // decrement bailout, load codeptr for next group
    mov  eax,A(IP_RETGROUP)
    mov  ecx,[ebx+A(IP_P1)]
    mov  edx,[eax+0]
    add  ecx,A(IP_RETLEN)
    mov  [ebx+A(IP_P1)],ecx
    // still not bailout?
    jl   ex
    // do we have code?
    test edx,edx
    jz   ex
    // execute
    jmp  edx
ex: xor  ecx,ecx // already subtracted
    ret  // EAX=new group, ECX=this group length
OEND

//------------------------ misc codes
//ONLY USE EAX

OBEGIN(o_int3)
    int 3
OEND

OBEGIN(o_nop)
    nop
OEND

OBEGIN(o_ebxinfo)
    nop
    mov  ebx,A(IP_P1)
OEND

OBEGIN(o_break)
    mov  eax,A(IP_P2)
    mov  [ebx+A(IP_P1)],eax  // will set st.breakout=2
OEND

OBEGIN(o_clear64bit)
    xor  eax,eax
    mov  [ebx+A(IP_P1)],eax
OEND

OBEGIN(o_regmove) // from P1 -> P2
    mov  eax,[ebx+A(IP_P1)]
    mov  [ebx+A(IP_P2)],eax
OEND

OBEGIN(o_link)
    mov  eax,A(IP_P1)
    mov  [ebx+A(IP_P2)],eax
OEND

//------------------------ immediate DIMM=RS op IMM
//ONLY USE EAX

OBEGIN(o_addi)
    mov  eax,[ebx+A(IP_RS)]
    add  eax,A(IP_IMMS)
    mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_andi)
    mov  eax,[ebx+A(IP_RS)]
    and  eax,A(IP_IMM)
    mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_ori)
    mov  eax,[ebx+A(IP_RS)]
    or   eax,A(IP_IMM)
    mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_xori)
    mov  eax,[ebx+A(IP_RS)]
    xor  eax,A(IP_IMM)
    mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_addiself)
    mov  eax,A(IP_IMMS)
    add  dword ptr [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_oriself)
    mov  eax,A(IP_IMM)
    or   dword ptr [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_orizero)
    mov  eax,A(IP_IMM)
    mov  dword ptr [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_addizero)
    mov  eax,A(IP_IMMS)
    mov  dword ptr [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_lui)
    mov  eax,A(IP_IMM)
    mov  dword ptr [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_luimemio)
    mov  dword ptr [ebx+A(IP_P1)],eax
OEND

//------------------------ set less than
//ONLY USE EAX

OBEGIN(o_slti)
    mov  eax,[ebx+A(IP_RS)]
    cmp  eax,A(IP_IMMS)
    mov  eax,1
    jl   ov
    xor  eax,eax
ov: mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_slt)
    mov  eax,[ebx+A(IP_RS)]
    cmp  eax,[ebx+A(IP_RT)]
    mov  eax,1
    jl   ov
    xor  eax,eax
ov: mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_sltiu)
    mov  eax,[ebx+A(IP_RS)]
    cmp  eax,A(IP_IMM)
    mov  eax,1
    jb   ov
    xor  eax,eax
ov: mov  [ebx+A(IP_RT)],eax
OEND

OBEGIN(o_sltu)
    mov  eax,[ebx+A(IP_RS)]
    cmp  eax,[ebx+A(IP_RT)]
    mov  eax,1
    jb   ov
    xor  eax,eax
ov: mov  [ebx+A(IP_D)],eax
OEND

//------------------------ shift D=S1 op S2 or D=S1 op IMM
//ONLY USE EAX,ECX

OBEGIN(o_sll)
    mov  ecx,A(IP_IMM)
    mov  eax,[ebx+A(IP_RT)]
    shl  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_srl)
    mov  ecx,A(IP_IMM)
    mov  eax,[ebx+A(IP_RT)]
    shr  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_sra)
    mov  ecx,A(IP_IMM)
    mov  eax,[ebx+A(IP_RT)]
    sar  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_sllv)
    mov  cl,[ebx+A(IP_RS)]
    mov  eax,[ebx+A(IP_RT)]
    shl  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_srlv)
    mov  cl,[ebx+A(IP_RS)]
    mov  eax,[ebx+A(IP_RT)]
    shr  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_srav)
    mov  cl,[ebx+A(IP_RS)]
    mov  eax,[ebx+A(IP_RT)]
    sar  eax,cl
    mov  [ebx+A(IP_D)],eax
OEND

//------------------------ register D=S1 op S2
//ONLY USE EAX

OBEGIN(o_add)
    mov  eax,[ebx+A(IP_RS)]
    add  eax,[ebx+A(IP_RT)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_sub)
    mov  eax,[ebx+A(IP_RS)]
    sub  eax,[ebx+A(IP_RT)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_and)
    mov  eax,[ebx+A(IP_RS)]
    and  eax,[ebx+A(IP_RT)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_or)
    mov  eax,[ebx+A(IP_RS)]
    or   eax,[ebx+A(IP_RT)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_orzero)
    mov  eax,[ebx+A(IP_RS)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_xor)
    mov  eax,[ebx+A(IP_RS)]
    xor  eax,[ebx+A(IP_RT)]
    mov  [ebx+A(IP_D)],eax
OEND

OBEGIN(o_nor)
    mov  eax,[ebx+A(IP_RS)]
    or   eax,[ebx+A(IP_RT)]
    not  eax
    mov  [ebx+A(IP_D)],eax
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

//------------------------ compares

OBEGIN(o_comparepush)
    push eax
OEND

OBEGIN(o_comparepop)
    pop  eax
    test eax,eax
OEND

OBEGIN(o_compareint)
    mov  eax,[ebx+A(IP_RS)]
    sub  eax,[ebx+A(IP_RT)]
OEND

OBEGIN(o_compareint0)
    mov  eax,[ebx+A(IP_RS)]
    test eax,eax
OEND

OBEGIN(o_comparefpu)
    mov  eax,[ebx+A(IP_P1)]
    test eax,eax
OEND

OBEGIN(o_lt)
    jl   $+11
OEND
OBEGIN(o_le)
    jle  $+11
OEND
OBEGIN(o_eq)
    je   $+11
OEND
OBEGIN(o_ne)
    jne  $+11
OEND
OBEGIN(o_ge)
    jge  $+11
OEND
OBEGIN(o_gt)
    jg   $+11
OEND

OBEGIN(o_comparegroupto1)
    mov  eax,A(IP_P1)
OEND
// insert jump (o_lt etc) here
OBEGIN(o_comparegroupto2)
    mov  eax,A(IP_P2)
OEND
// jump target
OBEGIN(o_comparegroupto3)
    mov  [asm_groupto],eax
OEND

//------------------------ fpu
//DO NOT USE INTEGER REGS EXCEPT EAX

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

// actual ops
// st(1)=FS1
// st(0)=FS2

OBEGIN(o_fpu_add)
    fadd
OEND

OBEGIN(o_fpu_sub)
    fsub
OEND

OBEGIN(o_fpu_mul)
    fmul
OEND

OBEGIN(o_fpu_div)
    fdiv
OEND

OBEGIN(o_fpu_sqrt)
    fsqrt
OEND

OBEGIN(o_fpu_abs)
    fabs
OEND

OBEGIN(o_fpu_neg)
    fchs
OEND

OBEGIN(o_fpu8_add)
    fadd qword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu8_sub)
    fsub qword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu8_mul)
    fmul qword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu8_div)
    fdiv qword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu4_add)
    fadd dword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu4_sub)
    fsub dword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu4_mul)
    fmul dword ptr [ebx+A(IP_FS2)]
OEND

OBEGIN(o_fpu4_div)
    fdiv dword ptr [ebx+A(IP_FS2)]
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

OBEGIN(o_special_sin)
    fld  dword ptr [ebx+A(IP_FS1)]
    fsin
    fstp dword ptr [ebx+A(IP_FD)]
OEND

OBEGIN(o_special_cos)
    fld  dword ptr [ebx+A(IP_FS1)]
    fcos
    fstp dword ptr [ebx+A(IP_FD)]
OEND

//------------------------ memory lwlrsdfas argh opcodes

OBEGIN(o_lwl)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
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
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_lwr)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
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
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_swl)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
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
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

OBEGIN(o_swr)
    mov  [asm_temp1],esi
    mov  [asm_temp2],edx
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
    mov  edx,[asm_temp2]
    mov  esi,[asm_temp1]
OEND

//------------------------ memory vm lookup
// input virtual address EAX, output physical address ECX

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

OBEGIN(o_eaxecxlookupsp)
{
    and  ecx,A(IP_P1)
    add  ecx,A(IP_P2)
}
OEND

OBEGIN(o_startmemrnoimm)
{
    mov  ecx,[ebx+A(IP_RS)]
    mov  eax,ecx
    shr  eax,12
    add  ecx,[OFFSET mem.lookupr+eax*4]
}
OEND

OBEGIN(o_startmemwnoimm)
{
    mov  ecx,[ebx+A(IP_RS)]
    mov  eax,ecx
    shr  eax,12
    add  ecx,[OFFSET mem.lookupw+eax*4]
}
OEND

OBEGIN(o_startmemr)
{
    mov  ecx,[ebx+A(IP_RS)]
    add  ecx,A(IP_IMMS)
    mov  eax,ecx
    shr  eax,12
    add  ecx,[OFFSET mem.lookupr+eax*4]
}
OEND

OBEGIN(o_startmemw)
{
    mov  ecx,[ebx+A(IP_RS)]
    add  ecx,A(IP_IMMS)
    mov  eax,ecx
    shr  eax,12
    add  ecx,[OFFSET mem.lookupw+eax*4]
}
OEND

//------------------------ memory load/store (old)

OBEGIN(o_lw)
{
    mov  eax,[ecx]
    mov  [ebx+A(IP_RT)],eax
}
OEND

OBEGIN(o_sw)
{
    mov  eax,[ebx+A(IP_RT)]
    mov  [ecx],eax
}
OEND

OBEGIN(o_lhu)
{
    xor  ecx,2
    movzx eax,word ptr [ecx]
    mov  [ebx+A(IP_RT)],eax
}
OEND

OBEGIN(o_lh)
{
    xor  ecx,2
    movsx eax,word ptr [ecx]
    mov  [ebx+A(IP_RT)],eax
}
OEND

OBEGIN(o_sh)
{
    xor  ecx,2
    mov  eax,[ebx+A(IP_RT)]
    mov  [ecx],ax
}
OEND

OBEGIN(o_lbu)
{
    xor  ecx,3
    movzx eax,byte ptr [ecx]
    mov  [ebx+A(IP_RT)],eax
}
OEND

OBEGIN(o_lb)
{
    xor  ecx,3
    movsx eax,byte ptr [ecx]
    mov  [ebx+A(IP_RT)],eax
}
OEND

OBEGIN(o_sb)
{
    xor  ecx,3
    mov  eax,[ebx+A(IP_RT)]
    mov  [ecx],al
}
OEND

// versions that don't load/store eax

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

//------------------------ branch and jump compiling

static void ac_jumptoreg(int ip_rs)
{
    // jump to register, most commonly JR RA
    ip[IP_RETLEN]=-r.g->len;
    ip[IP_RS]=ip_rs;
    if(ip_rs==STADDR(RA.d))
    {
        ip[IP_P1]=STADDR(st.expanded64bit);
        ip[IP_P2]=STADDR(st.memiodetected);
        insert(o_gotoreg_ret);
        // insert CALL or_memiocheck, RET
        insertbyte(0x60); // pushad
        insertcall(hw_memio);
        insertbyte(0x61); // popad
        insertbyte(X_RET);
    }
    else insert(o_gotoreg);
    r.lastjumpto=1;
//    if(ip_rs!=STADDR(RA.d)) exception("stop3");
}

static void ac_jumptogroupto(void)
{
    // jump to group st.groupto
    ip[IP_RETLEN]=-r.g->len;
    insert(o_gotogroupto);
    r.lastjumpto=1;
}

static void ac_jumptogroupeax(void)
{
    // jump to group st.groupto
    ip[IP_RETLEN]=-r.g->len;
    insert(o_gotogroupeax);
    r.lastjumpto=1;
}

static void ac_jumpto(dword to)
{
    Group *g;

    g=ac_creategroup(to);
    ip[IP_RETGROUP]=(dword)g;
    ip[IP_RETLEN]=-r.g->len;

    if(r.opt_rejumpgroup && to==r.g->addr)
    {
        // faster jump back to group start (small loop)
        ip[IP_P1]=STADDR(st.bailout);
        insert(o_goto_bailoutcheck);
        // jump back to group start (if not bailout by last check)
        insertbyte(0x0F); //
        insertbyte(0x8F); // JG long
        insertdword((dword)r.g->code-(mem.codeused+4));
        // if bailouted, normal jump
        insert(o_gotogroup_retalways);
    }
    else
    {
        // normal jump (to another group)
        if(r.opt_directjmp)
        {
            ip[IP_P1]=STADDR(st.bailout);
            insert(o_gotogroup);
        }
        else insert(o_gotogroup_retalways);
    }
    r.lastjumpto=1;
}

static void ac_compilebranch(dword pc,dword opcode,int type,int likely)
{
    dword delayopcode;
    dword jumpto;
    int   fixuppos;

    jumpto=pc+4+SIGNEXT16(OP_IMM(opcode))*4;

    if(type==CMP_ALWAYS)
    {
        if(likely) exception("compiler: likely+ALWAYS not allowed\n");

        // delay slot
        ac_compileop(pc+4);

        // jump
        ac_jumpto(jumpto);
        return;
    }

    // load regs compare
    if(type&CMP_FPU)
    {
        ip[IP_P1]=STADDR(st.fputrue);
        type&=~CMP_FPU;
        insert(o_comparefpu);
    }
    else
    {
        if(ip[IP_RT]==STADDR(R0.d)) insert(o_compareint0);
        else insert(o_compareint);
    }

    if(likely)
    {
        fixuppos=mem.codeused+2;
        switch(type^1) // jump on negated type
        {
        case CMP_EQ: insert(o_eq); break;
        case CMP_NE: insert(o_ne); break;
        case CMP_LT: insert(o_lt); break;
        case CMP_LE: insert(o_le); break;
        case CMP_GT: insert(o_gt); break;
        case CMP_GE: insert(o_ge); break;
        }

        // delay slot
        ac_compileop(pc+4);

        // jump to TRUE target
        ac_jumpto(jumpto);

        // do a fixup to the previous jump
        *(dword *)(mem.code+fixuppos)=mem.codeused-(fixuppos+4);

        // jump to FALSE target
        ac_jumpto(pc+8);
    }
    else
    {
        ip[IP_P1]=(dword)ac_creategroup(pc+8);
        ip[IP_P2]=(dword)ac_creategroup(jumpto);

        delayopcode=mem_readop(pc+4);

        if(delayopcode==0) //††
        {
            insert(o_comparegroupto1);

            fixuppos=mem.codeused+2;

            switch(type^1) // jump on type
            {
            case CMP_EQ: insert(o_eq); break;
            case CMP_NE: insert(o_ne); break;
            case CMP_LT: insert(o_lt); break;
            case CMP_LE: insert(o_le); break;
            case CMP_GT: insert(o_gt); break;
            case CMP_GE: insert(o_ge); break;
            }

            insert(o_comparegroupto2);

            ac_jumptogroupeax();
        }
        else
        {
            insert(o_comparegroupto1);

            fixuppos=mem.codeused+2;

            switch(type^1) // jump on type
            {
            case CMP_EQ: insert(o_eq); break;
            case CMP_NE: insert(o_ne); break;
            case CMP_LT: insert(o_lt); break;
            case CMP_LE: insert(o_le); break;
            case CMP_GT: insert(o_gt); break;
            case CMP_GE: insert(o_ge); break;
            }

            insert(o_comparegroupto2);

            // do a fixup to the previous jump
            *(dword *)(mem.code+fixuppos)=mem.codeused-(fixuppos+4);

            insert(o_comparegroupto3);

            // delay slot
            ac_compileop(pc+4);

            ac_jumptogroupto();
        }
    }
}

static void ac_compilejump(dword pc,dword opcode,int usereg,int link)
{
    dword ip_rs;
    dword ip_imm;

    ip_rs=ip[IP_RS]; // save since we'll do a compileop which corrupts this
    ip_imm=((OP_TAR(opcode)<<2)&0x0fffffff)|(pc&0xf0000000);

    // operation is always RS cmp RT

    // code structure:
    // - execute delay slot
    // - perform link (if link)
    // - load reg/immediate to EAX (usereg/useimm)
    // - jumpreturn

    ac_compileop(pc+4);

    if(link)
    {
        ip[IP_P1]=pc+8;
        ip[IP_P2]=STADDR(RA.d);
        insert(o_link);
    }

    if(usereg)
    {
        ac_jumptoreg(ip_rs);
    }
    else
    {
        ac_jumpto(ip_imm);
    }
}

// 387 status bits
#define FPU_ZERO      (1<<14)
#define FPU_UNORDEPUR (1<<10)
#define FPU_LESS      (1<<8)

static int ac_compilefpu(dword pc,dword opcode)
{
    int fmt=OP_RS(opcode);
    int op=OP_FUNC(opcode);
    int ret=1; // return 2 if delay slot also parsed

    if(fmt<8)
    { // Move ops
        switch(fmt)
        {
        case 0: // MFC1
            ip[IP_P1]=STADDR(st.f[OP_RD(opcode)]);
            ip[IP_P2]=STADDR(st.g[OP_RT(opcode)]);
            insert(o_regmove);
            break;
        case 4: // MTC1
            ip[IP_P1]=STADDR(st.g[OP_RT(opcode)]);
            ip[IP_P2]=STADDR(st.f[OP_RD(opcode)]);
            insert(o_regmove);
            break;
        case 2:
        case 6:
            // ignore CFC0 CTC0
            r.inserted=1;
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Move %i at %08X\n",fmt,pc);
            r.errors|=ERROR_FPU;
            break;
        }
    }
    else if(fmt==8)
    { // BC ops (branch)
        int rt=OP_RT(opcode);
        int ontrue=rt&1;
        int likely=(rt&2)>>1;

        if(ontrue)
        {
            ac_compilebranch(pc,opcode,CMP_NE|CMP_FPU,likely);
        }
        else
        {
            ac_compilebranch(pc,opcode,CMP_EQ|CMP_FPU,likely);
        }

        ret=2;
    }
    else if(op>=8 && op<=47)
    { // converts
        int ok=0;
        switch(op)
        {
            case 32: // CVT.S [used]
                ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
                ip[IP_FD ]=STADDR(st.f[OP_SHAMT(opcode)]);
                if(fmt==20)      insert(o_cvt_w2s);
                else if(fmt==17) insert(o_cvt_d2s);
                ok=1;
                break;
            case 33: // CVT.D [used]
                ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
                ip[IP_FD ]=STADDR(st.f[OP_SHAMT(opcode)]);
                if(fmt==20)      insert(o_cvt_w2d);
                else if(fmt==16) insert(o_cvt_s2d);
                ok=1;
                break;
            case 36: // CVT.W
                // rounding might be wrong, but it's still close
                ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
                ip[IP_FD ]=STADDR(st.f[OP_SHAMT(opcode)]);
                if(fmt==17)      insert(o_cvt_d2w);
                else if(fmt==16) insert(o_cvt_s2w);
                ok=1;
                break;
            case 12: // ROUND.W
                break;
            case 13: // TRUN.W [used]
                // use same code as CVT for now (rounding might be wrong)
                ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
                ip[IP_FD ]=STADDR(st.f[OP_SHAMT(opcode)]);
                if(fmt==17)      insert(o_cvt_d2w);
                else if(fmt==16) insert(o_cvt_s2w);
                ok=1;
                break;
            case 14: // CEIL.W
                break;
            case 15: // FLOOR.W
                break;
        }
        if(!ok)
        {
            insert(o_int3);
            logi("compiler: unimplemented FPU-Convert %i at %08X\n",op,pc);
            r.errors|=ERROR_FPU;
        }
    }
    else if(op>=48)
    { // compares
        ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
        ip[IP_FS2]=STADDR(st.f[OP_RT(opcode)]);
        ip[IP_FD] =STADDR(st.f[OP_SHAMT(opcode)]);
        ip[IP_P1] =STADDR(st.fputmp);

        if(fmt!=16 && fmt!=17)
        {
            op=255; // force error (only word ops are the converts)
        }
        r.inserted=0;

        switch(op)
        {
        case 48: // C.F
        case 49: // C.UN
        case 56: // C.SF
        case 57: // C.NGLE
            ip[IP_P1]=0*FPU_ZERO+0*FPU_LESS; // bits we are interested in
            break;
        case 50: // C.EQ   [actually used]
        case 51: // C.UEQ
        case 58: // C.SEQ
        case 59: // C.NGL
            ip[IP_P1]=1*FPU_ZERO+0*FPU_LESS; // bits we are interested in
            break;
        case 60: // C.LT   [actually used]
        case 52: // C.OLT
        case 53: // C.ULT
        case 61: // C.NGE
            ip[IP_P1]=0*FPU_ZERO+1*FPU_LESS; // bits we are interested in
            break;
        case 62: // C.LE   [actually used]
        case 54: // C.OLE
        case 55: // C.ULE
        case 63: // C.NGT
            ip[IP_P1]=1*FPU_ZERO+1*FPU_LESS; // bits we are interested in
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Op %i at %08X\n",op,pc);
            r.errors|=ERROR_FPU;
            break;
        }
        ip[IP_P2]=STADDR(st.fputrue);
        if(fmt==16) insert(o_fpu4_cmp);
        else        insert(o_fpu8_cmp);
    }
    else
    { // generic ops & compares
        ip[IP_FS1]=STADDR(st.f[OP_RD(opcode)]);
        ip[IP_FS2]=STADDR(st.f[OP_RT(opcode)]);
        ip[IP_FD] =STADDR(st.f[OP_SHAMT(opcode)]);
        ip[IP_P1] =STADDR(st.fputmp);

        if(fmt==16)
        { // single
            insert(o_fpu_ls1);
        }
        else if(fmt==17)
        { // double
            insert(o_fpu_ld1);
        }
        else
        {
            op=255; // force error (only word ops are the converts)
        }
        r.inserted=0;

        switch(op)
        {
        case 0: // ADD
            if(fmt==16) insert(o_fpu4_add);
            else        insert(o_fpu8_add);
            break;
        case 1: // SUB
            if(fmt==16) insert(o_fpu4_sub);
            else        insert(o_fpu8_sub);
            break;
        case 2: // MUL
            if(fmt==16) insert(o_fpu4_mul);
            else        insert(o_fpu8_mul);
            break;
        case 3: // DIV
            if(fmt==16) insert(o_fpu4_div);
            else        insert(o_fpu8_div);
            break;
        case 4: // SQRT
            insert(o_fpu_sqrt);
            break;
        case 5: // ABS
            insert(o_fpu_abs);
            break;
        case 6: // MOV
            // just the store at end will do
            r.inserted=1;
            break;
        case 7: // NEG
            insert(o_fpu_neg);
            break;
        default:
            insert(o_int3);
            logi("compiler: unimplemented FPU-Op %i at %08X\n",op,pc);
            r.errors|=ERROR_FPU;
            break;
        }
        if(r.inserted)
        {
            if(fmt==16)
            { // single
                insert(o_fpu_ss);
            }
            else if(fmt==17)
            { // double
                insert(o_fpu_sd);
            }
        }
    }

    if(!r.inserted)
    {
        insert(o_int3);
        logi("compiler: FPU TBD op %i at %08X\n",op,pc);
        r.errors|=ERROR_FPU;
        r.inserted=1;
    }

    return(ret);
}

static void ac_startmem(int write)
{
    cstat.inma++;
    if(write)
    {
        if(ip[IP_IMM]) insert(o_startmemw);
        else           insert(o_startmemwnoimm);
    }
    else
    {
        if(ip[IP_IMM]) insert(o_startmemr);
        else           insert(o_startmemrnoimm);
    }
}

int ac_compileopold(dword pc,dword opcode,int op)
{
    int ret=1; // return 2 if delay slot also parsed

    cstat.innorm++;

    if(opcode==0)
    { // nop
        r.inserted=1;
        return(1);
    }

    r.inserted=0;

    if(op!=17)
    { // setup generic stuff (if not fpu op)
        ip[IP_D ]=STADDR(st.g[OP_RD(opcode)]);
        ip[IP_RS]=STADDR(st.g[OP_RS(opcode)]);
        ip[IP_RT]=STADDR(st.g[OP_RT(opcode)]);
        ip[IP_IMM]=OP_IMM(opcode);
        ip[IP_IMMS]=SIGNEXT16(OP_IMM(opcode));
    }

    {
        switch(op)
        {
        case 16: // COP0/MMU
            r.errors|=ERROR_MMU;
            break;
        case OP_PATCH: // PATCH
            r.errors|=ERROR_PATCH;
            break;
        //------------------------------loads
        case 32: // LB
            ac_startmem(0);
            insert(o_lb);
            break;
        case 36: // LBU
            ac_startmem(0);
            insert(o_lbu);
            break;
        case 33: // LH
            ac_startmem(0);
            insert(o_lh);
            break;
        case 37: // LHU
            ac_startmem(0);
            insert(o_lhu);
            break;
        case 35: // LW
        case 39: // LWU
            ac_startmem(0);
            insert(o_lw);
            break;
        case 34: // LWL
            ac_startmem(0);
            insert(o_lwl);
            break;
        case 38: // LWR
            ac_startmem(0);
            insert(o_lwr);
            break;
        //------------------------------stores
        case 40: // SB
            ac_startmem(1);
            insert(o_sb);
            break;
        case 41: // SH
            ac_startmem(1);
            insert(o_sh);
            break;
        case 43: // SW
            ac_startmem(1);
            insert(o_sw);
            break;
        case 42: // SWL
            ac_startmem(1);
            insert(o_swl);
            break;
        case 46: // SWR
            ac_startmem(1);
            insert(o_swr);
            break;
        //------------------------------arithmetic
        case 8: // ADDI
        case 9: // ADDIU
            if(OP_RT(opcode)==0) insert(o_addizero);
            else if(OP_RT(opcode)==OP_RS(opcode)) insert(o_addiself);
            else insert(o_addi);
            break;
        case 10: // SLTI
            insert(o_slti);
            break;
        case 11: // SLTIU
            insert(o_sltiu);
            break;
        case 12: // ANDI
            insert(o_andi);
            break;
        case 13: // ORI
            if(OP_RT(opcode)==0) insert(o_orizero);
            else if(OP_RT(opcode)==OP_RS(opcode)) insert(o_oriself);
            else insert(o_ori);
            break;
        case 14: // XORI
            insert(o_xori);
            break;
        case 15: // LUI
            ip[IP_IMM]<<=16;
            insert(o_lui);
            if(hw_ismemiorange(ip[IP_IMM]))
            {
                ip[IP_P1]=STADDR(st.memiodetected);
                insert(o_luimemio);
            }
            break;
        //------------------------------arithmetic register
        case 0x40+0: // SLL
            ip[IP_IMM]=OP_SHAMT(opcode);
            insert(o_sll);
            break;
        case 0x40+2: // SRL
            ip[IP_IMM]=OP_SHAMT(opcode);
            insert(o_srl);
            break;
        case 0x40+3: // SRA
            ip[IP_IMM]=OP_SHAMT(opcode);
            insert(o_sra);
            break;
        case 0x40+4: // SLLV
            insert(o_sllv);
            break;
        case 0x40+6: // SRLV
            insert(o_srlv);
            break;
        case 0x40+7: // SRAV
            insert(o_srav);
            break;
        case 0x40+13: // BREAK
            ip[IP_P2]=r.pc;
            ip[IP_P1]=STADDR(st.breakout);
            insert(o_break);
            ac_jumpto(pc+4);
            break;
        case 0x40+16: // MFHI
            ip[IP_P1]=STADDR(st.mhi);
            ip[IP_P2]=ip[IP_D];
            insert(o_regmove);
            break;
        case 0x40+17: // MTHI
            ip[IP_P1]=ip[IP_RS];
            ip[IP_P2]=STADDR(st.mhi);
            insert(o_regmove);
            break;
        case 0x40+18: // MFLO
            ip[IP_P1]=STADDR(st.mlo);
            ip[IP_P2]=ip[IP_D];
            insert(o_regmove);
            break;
        case 0x40+19: // MTLO
            ip[IP_P1]=ip[IP_RS];
            ip[IP_P2]=STADDR(st.mlo);
            insert(o_regmove);
            break;
        case 0x40+24: // MULT
            ip[IP_P1]=STADDR(st.mlo);
            ip[IP_P2]=STADDR(st.mhi);
            insert(o_mult);
            break;
        case 0x40+25: // MULTU
            ip[IP_P1]=STADDR(st.mlo);
            ip[IP_P2]=STADDR(st.mhi);
            insert(o_multu);
            break;
        case 0x40+26: // DIV
            ip[IP_P1]=STADDR(st.mlo);
            ip[IP_P2]=STADDR(st.mhi);
            insert(o_div1);
            insertcall(or_div);
            insert(o_div2);
            break;
        case 0x40+27: // DIVU
            ip[IP_P1]=STADDR(st.mlo);
            ip[IP_P2]=STADDR(st.mhi);
            insert(o_div1);
            insertcall(or_divu);
            insert(o_div2);
            break;
        case 0x40+32: // ADD
        case 0x40+33: // ADDU
            insert(o_add);
            break;
        case 0x40+34: // SUB
        case 0x40+35: // SUBU
            insert(o_sub);
            break;
        case 0x40+36: // AND
            insert(o_and);
            break;
        case 0x40+37: // OR
            if(OP_RT(opcode)==0) insert(o_orzero);
            else insert(o_or);
            break;
        case 0x40+38: // XOR
            insert(o_xor);
            break;
        case 0x40+39: // NOR
            insert(o_nor);
            break;
        case 0x40+42: // SLT
            insert(o_slt);
            break;
        case 0x40+43: // SLTU
            insert(o_sltu);
            break;
        //------------------------------jump and branch
        case 2: // J
            ac_compilejump(pc,opcode,0,0);
            ret=2; break;
        case 3: // JAL
            ac_compilejump(pc,opcode,0,1);
            ret=2; break;
        case 0x40+8: // JR
            ac_compilejump(pc,opcode,1,0);
            ret=2; break;
        case 0x40+9: // JALR
            ac_compilejump(pc,opcode,1,1);
            ret=2; break;
        //------------------------------jump and branch (assumed inside routine)
        case 4: // BEQ
            if(OP_RS(opcode)==0 && OP_RT(opcode)==0)
                 ac_compilebranch(pc,opcode,CMP_ALWAYS,0);
            else ac_compilebranch(pc,opcode,CMP_EQ,0);
            ret=2; break;
        case 5: // BNEQ
            ac_compilebranch(pc,opcode,CMP_NE,0);
            ret=2; break;
        case 6: // BLEZ
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_LE,0);
            ret=2; break;
        case 7: // BGTZ
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_GT,0);
            ret=2; break;
        case 20: // BEQL
            ac_compilebranch(pc,opcode,CMP_EQ,1);
            ret=2; break;
        case 21: // BNEQL BNEL
            ac_compilebranch(pc,opcode,CMP_NE,1);
            ret=2; break;
        case 22: // BLEZL
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_LE,1);
            ret=2; break;
        case 23: // BGTZL
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_GT,1);
            ret=2; break;
        case 0x80+0: // BLTZ
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_LT,0);
            ret=2; break;
        case 0x80+1: // BGEZ
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_GE,0);
            ret=2; break;
        case 0x80+2: // BLTZL
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_LT,1);
            ret=2; break;
        case 0x80+3: // BGEZL
            ip[IP_RT]=STADDR(R0.d);
            ac_compilebranch(pc,opcode,CMP_GE,1);
            ret=2; break;
        //------------------------------fpu
        case 17:
            cstat.innorm--;
            cstat.infpu++;
            if(1)
            {
                ret=ac_compilefpu(pc,opcode);
            }
            else
            {
                insert(o_int3);
                logi("compiler: unimplemented FPU-op at %08X\n",pc);
                r.errors|=ERROR_FPU;
            }
            break;
        case 53: // LDC1
            cstat.innorm--;
            cstat.infpu++;
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)+1]);
            ac_startmem(0);
            insert(o_lw);
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)+0]);
            ip[IP_IMMS]+=4;
            ac_startmem(0);
            insert(o_lw);
            break;
        case 61: // SDC1
            cstat.innorm--;
            cstat.infpu++;
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)+1]);
            ac_startmem(1);
            insert(o_sw);
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)+0]);
            ip[IP_IMMS]+=4;
            ac_startmem(1);
            insert(o_sw);
            break;
        case 49: // LWC1
            cstat.innorm--;
            cstat.infpu++;
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)]);
            ac_startmem(0);
            insert(o_lw);
            break;
        case 57: // SWC1
            cstat.innorm--;
            cstat.infpu++;
            ip[IP_RT]=STADDR(st.f[OP_RT(opcode)]);
            ac_startmem(1);
            insert(o_sw);
            break;
        //--------------
        default:
            insert(o_int3);
            logi("compiler: unimplemented op %i at %08X\n",op,pc);
            r.errors|=ERROR_OP;
            break;
        }
    }

    if(!r.inserted)
    {
        insert(o_int3);
        logi("compiler: TBD op %i at %08X\n",op,pc);
        r.errors|=ERROR_OP;
    }

    return(ret);
}

static void ac_compilealign(void)
{
    // align to 16 byte boundary -5
    while((mem.codeused+5)&0xf) insertbyte(X_INT3); // int 3
    // insert a MOV EAX,pc before the group
    // This is never executed but when looking at the code
    // in an X86 debugger you see what group comes from where
    insertbyte(0xb8);
    insertdword(st.pc);
}

void ac_compilestartold(void)
{
    Group *g=r.g;

    // align to a 16 byte boundary and insert
    // a MOV eax,PC for debuggers to see
    ac_compilealign();
    // set codebase to group
    g->code=mem.code+mem.codeused;

//    insertbyte(0xCC);
}

void ac_compileendold(void)
{
    Group *g=r.g;

    if(!r.lastjumpto)
    {
        ac_jumpto(g->addr+g->len*4);
    }

    insertbyte(X_INT3); // should never get here
}

