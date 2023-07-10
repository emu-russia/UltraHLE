#include "ultra.h"

#define DUMP64 0 // dump 64 bit arithmetic

#define GETREGS \
        rs=&st.g[OP_RS(opcode)].d; \
        rt=&st.g[OP_RT(opcode)].d; \
        rd=&st.g[OP_RD(opcode)].d;

#define GETREGSIMM \
        rs=&st.g[OP_RS(opcode)].d; \
        rd=&st.g[OP_RT(opcode)].d; \
        imm[0]=SIGNEXT16(OP_IMM(opcode)); \
        rt=imm;

#define GETREGSIMM64 \
        rs=&st.g[OP_RS(opcode)].d; \
        rd=&st.g[OP_RT(opcode)].d; \
        imm[0]=SIGNEXT16(OP_IMM(opcode)); \
        if(imm[0]&0x80000000) imm[1]=-1; else imm[1]=0; \
        rt=imm;

#define GETREGSBR \
        rs=&st.g[OP_RS(opcode)].d; \
        rt=&st.g[OP_RT(opcode)].d;

#define GETREGSIMMUNS \
        rs=&st.g[OP_RS(opcode)].d; \
        rd=&st.g[OP_RT(opcode)].d; \
        imm[0]=OP_IMM(opcode); \
        rt=imm;

static void op_64bitexpand(void)
{
    if(!st.expanded64bit)
    {
        if(DUMP64)
        {
            print("expandargs\n");
        }
        // expand input argument regs
        if(A0.d2[0]&0x80000000) A0.d2[1]=-1; else A0.d2[1]=0;
        if(A1.d2[0]&0x80000000) A1.d2[1]=-1; else A1.d2[1]=0;
        if(A2.d2[0]&0x80000000) A2.d2[1]=-1; else A2.d2[1]=0;
        if(A3.d2[0]&0x80000000) A3.d2[1]=-1; else A3.d2[1]=0;
    }
    st.expanded64bit=1;
}

static void op_dmultu(int reg1,int reg2)
{
    qreg a,b;
    op_64bitexpand();
    a=st.g[reg1];
    b=st.g[reg2];
    st.mlo.q=(unsigned __int64)a.q*(unsigned __int64)b.q;
    st.mhi.q=0;
    if(DUMP64)
    {
        print("(%08X) ",st.pc);
        print("dmult  %08X%08X*%08X%08X = %08X%08X %08X%08X\n",
            a.d2[1],a.d2[0],b.d2[1],b.d2[0],
            st.mhi.d2[1],st.mhi.d2[0],
            st.mlo.d2[1],st.mlo.d2[0]);
    }
}

static void op_ddivu(int reg1,int reg2)
{
    qreg a,b;
    op_64bitexpand();
    a=st.g[reg1];
    b=st.g[reg2];
    if(!b.q)
    {
        st.mlo.q=0;
        st.mhi.q=0;
    }
    else
    {
        st.mlo.q=(unsigned __int64)a.q/(unsigned __int64)b.q;
        st.mhi.q=(unsigned __int64)a.q%(unsigned __int64)b.q;
    }
    if(DUMP64)
    {
        print("(%08X) ",st.pc);
        print("ddivu  %08X%08X/%08X%08X = %08X%08X , %08X%08X\n",
            a.d2[1],a.d2[0],b.d2[1],b.d2[0],
            st.mhi.d2[1],st.mhi.d2[0],
            st.mlo.d2[1],st.mlo.d2[0]);
    }
}

static void op_ddiv(int reg1,int reg2)
{
    qreg a,b;
    op_64bitexpand();
    a=st.g[reg1];
    b=st.g[reg2];
    if(!b.q)
    {
        st.mlo.q=0;
        st.mhi.q=0;
    }
    else
    {
        st.mlo.q=(__int64)a.q/(__int64)b.q;
        st.mhi.q=(__int64)a.q%(__int64)b.q;
    }
    if(DUMP64)
    {
        print("(%08X) ",st.pc);
        print("ddiv   %08X%08X/%08X%08X = %08X%08X , %08X%08X\n",
            a.d2[1],a.d2[0],
            b.d2[1],b.d2[0],
            st.mhi.d2[1],st.mhi.d2[0],
            st.mlo.d2[1],st.mlo.d2[0]);
    }
}

void op_shift64(dword opcode,int type,int amount)
{
    dword *s,*d;
    dword t[2];

    if(amount==-1)
    {
        // from reg
        s=st.g[OP_RT(opcode)].d2;
        d=st.g[OP_RD(opcode)].d2;
        amount=st.g[OP_RS(opcode)].d;
    }
    else
    {
        s=st.g[OP_RT(opcode)].d2;
        d=st.g[OP_RD(opcode)].d2;
    }

    op_64bitexpand();
    t[0]=s[0];
    t[1]=s[1];
    switch(type)
    {
    case 0: // left
        _asm
        {
            mov   ecx,amount
            mov   eax,t[0]
            mov   edx,t[4]
            cmp   ecx,32
            jb    g1
            mov   edx,eax
            xor   eax,eax
            and   ecx,31
            jz    g2
        g1: shld  edx,eax,cl
            shl   eax,cl
        g4: mov   t[0],eax
            mov   t[4],edx
        }
        break;
    case 1: // right
        _asm
        {
            mov   ecx,amount
            mov   eax,t[0]
            mov   edx,t[4]
            cmp   ecx,32
            jb    g2
            mov   eax,edx
            xor   edx,edx
            and   ecx,31
            jz    g5
        g2: shrd  eax,edx,cl
            shr   edx,cl
        g5: mov   t[0],eax
            mov   t[4],edx
        }
        break;
    case 2: // right arithmetic
        _asm
        {
            mov   ecx,amount
            mov   eax,t[0]
            mov   edx,t[4]
            cmp   ecx,32
            jb    g3
            mov   eax,edx
            xor   edx,edx
            and   ecx,31
            jz    g6
        g3: shrd  eax,edx,cl
            sar   edx,cl
        g6: mov   t[0],eax
            mov   t[4],edx
        }
        break;
    }
    if(DUMP64)
    {
        print("(%08X) ",st.pc);
        print("shift%i %08X%08X,%i = %08X%08X (s=%i d=%i)\n",
            type,s[1],s[0],amount,t[1],t[0],
            OP_RT(opcode),OP_RD(opcode));
    }
    d[0]=t[0];
    d[1]=t[1];
}

void addi64(dword *d,dword *a,dword *b)
{
    op_64bitexpand();
    _asm
    {
        mov ebx,a
        mov ecx,b
        mov eax,[ebx+0]
        mov edx,[ebx+4]
        add eax,[ecx+0]
        adc edx,[ecx+4]
        mov ebx,d
        mov [ebx+0],eax
        mov [ebx+4],edx
    }
    if(DUMP64)
    {
        print("(%08X) ",st.pc);
        print("addi   %08X%08X+%08X%08X = %08X%08X\n",
            a[1],a[0],b[1],b[0],d[1],d[0]);
    }
}

//----

__inline dword op_memaddr(dword opcode)
{
    dword a;
    a = OP_IMM(opcode);
    a = SIGNEXT16(a);
    a+= st.g[OP_RS(opcode)].d;
    return(a);
}

static void op_readmem(dword opcode,int bytes)
{
    int a,x,*d;
    a=op_memaddr(opcode);
    if(bytes>0x10)
    { // fpu
        d =&st.f[OP_RT(opcode)].d;
        bytes-=0x10;
    }
    else
    {
        d =&st.g[OP_RT(opcode)].d;
    }

    cpu_notify_readmem(a,bytes);

    switch(bytes)
    {
    case -1:
        x=mem_read8(a);
        d[0]=SIGNEXT8(x);
        break;
    case 1:
        x=mem_read8(a);
        d[0]=x;
        break;
    case -2:
        x=mem_read16(a);
        d[0]=SIGNEXT16(x);
        break;
    case 2:
        x=mem_read16(a);
        d[0]=x;
        break;
    case -4:
    case 4:
        x=mem_read32(a);
        d[0]=x;
        break;
    case 8:
    case -8:
        d[1]=mem_read32(a);
        d[0]=mem_read32(a+4);
        break;
    }
}

static void op_writemem(dword opcode,int bytes)
{
    int a,*d;
    a=op_memaddr(opcode);
    if(bytes>0x10)
    { // fpu
        d =&st.f[OP_RT(opcode)].d;
        bytes-=0x10;
    }
    else
    {
        d =&st.g[OP_RT(opcode)].d;
    }

    switch(bytes)
    {
    case 1:
        mem_write8(a,d[0]);
        break;
    case 2:
        mem_write16(a,d[0]);
        break;
    case 4:
        mem_write32(a,d[0]);
        break;
    case 8:
        mem_write32(a,d[1]);
        mem_write32(a+4,d[0]);
        break;
    }

    cpu_notify_writemem(a,bytes);
}

static void op_rwmemrl(dword opcode,int write,int right)
{
    dword x,y,a,s,m;
    a=op_memaddr(opcode);
    s=a&3;
    a&=~3;
    if(write)
    {
        x=mem_read32(a);
        y=st.g[OP_RT(opcode)].d;
        if(right)
        {
            m=0x00ffffff >> (s*8);
            y<<=(3-s)*8;
            x&=m;
            x|=y;
        }
        else
        {
            m=0xffffff00 << ((3-s)*8);
            y>>=s*8;
            x&=m;
            x|=y;
        }
        mem_write32(a,x);
    }
    else
    {
        x=st.g[OP_RT(opcode)].d;
        y=mem_read32(a);
        if(right)
        {
            m=0xffffff00 << (s*8);
            y>>=(3-s)*8;
            x&=m;
            x|=y;
        }
        else
        {
            m=0x00ffffff >> ((3-s)*8);
            y<<=s*8;
            x&=m;
            x|=y;
        }
        st.g[OP_RT(opcode)].d=x;
    }
}
static void op_mult(int a,int b)
{
    int lo,hi;
    _asm
    {
        mov  eax,a
        mov  edx,b
        imul edx
        mov  lo,eax
        mov  hi,edx
    }
    st.mlo.d=lo;
    st.mhi.d=hi;
}

static void op_multu(int a,int b)
{
    int lo,hi;
    _asm
    {
        mov  eax,a
        mov  edx,b
        mul  edx
        mov  lo,eax
        mov  hi,edx
    }
    st.mlo.d=lo;
    st.mhi.d=hi;
}

static void op_div(int a,int b)
{
    int lo,hi;
    if(!b)
    {
        error("divide by zero");
        st.mlo.d=0;
        st.mhi.d=0;
        return;
    }
    _asm
    {
        mov  eax,a
        cdq
        mov  ecx,b
        idiv ecx
        mov  lo,eax
        mov  hi,edx
    }
    st.mlo.d=lo;
    st.mhi.d=hi;
}

static void op_divu(int a,int b)
{
    int lo,hi;
    if(!b)
    {
        error("divide by zero");
        st.mlo.d=0;
        st.mhi.d=0;
        return;
    }
    _asm
    {
        mov  eax,a
        xor  edx,edx
        mov  ecx,b
        div  ecx
        mov  lo,eax
        mov  hi,edx
    }
    st.mlo.d=lo;
    st.mhi.d=hi;
}

static void op_jump(dword opcode,int link,int reg)
{
    int to;

    st.branchtype=BRANCH_NORMAL;

    if(reg==-1)
    {
        to=((OP_TAR(opcode)<<2)&0x0fffffff)
          |(st.pc&0xf0000000);
    }
    else
    {
        to=st.g[reg].d;
        if(reg==31)
        {
            st.branchtype=BRANCH_RET;
            st.expanded64bit=0;
        }
    }

    if(link)
    { // link
        st.branchtype=BRANCH_CALL;
        st.g[31].d=st.pc+8;
    }

    st.branchdelay=2;
    st.branchto=to;
}

static void op_branch(dword opcode,int doit,int likely,int link)
{
    if(doit)
    {
        int imm=SIGNEXT16(OP_IMM(opcode));
        if(link)
        { // link
            st.g[31].d=st.pc+8;
            st.branchtype=BRANCH_CALL;
        }
        else st.branchtype=BRANCH_NORMAL;
        imm<<=2;
        imm+=st.pc+4;
        st.branchdelay=2;
        st.branchto=imm;
    }
    else
    {
        if(likely)
        {
            st.pc+=4; // likely, skip delay slot instruction
        }
    }
}

double readdouble(int reg)
{
    dword x[2];
    x[0]=st.f[reg+0].d;
    x[1]=st.f[reg+1].d;
    return(*(double *)x);
}

void writedouble(int reg,double value)
{
    dword x[2];
    *(double *)x=value;
    st.f[reg+0].d=x[0];
    st.f[reg+1].d=x[1];
}

static void op_scc( dword opcode )
{
#define  MF           0  // Move From Coprocessor (COPx Sub OpCode)
#define  MT           4  // Move to Coprocessor (COPx Sub OpCode)
//   print( "at %08X cop0 (0x%08X) Rs (0x%02X) Func (0x%02X)\n",
//          st.pc, opcode, OP_RS(opcode), OP_FUNC(opcode));
   switch( OP_RS(opcode) )
   {
      case MF:

//         print( "at %08X read mmu %s\n",st.pc,mmuregnames[OP_RD(opcode)]);
         st.g[OP_RT(opcode)].d = st.mmu[OP_RD(opcode)].d;

         break;

      case MT:

         // If write to the Compare Register, Clear the Timer Interrupt
         // in the Cause Register

         if( OP_RD(opcode) == 11 )
            st.mmu[13].d &= ~0x00008000;


         // If write to the Cause Register, mask out all bits except
         // IP0 & IP1

         if( OP_RD(opcode) == 13 )
         {
            st.mmu[13].d = st.g[OP_RT(opcode)].d & 0x00000300;
            return;
         }

         st.mmu[OP_RD(opcode)].d = st.g[OP_RT(opcode)].d;
         logh("TLB regwrite at %08X: %s=%08X\n",
            st.pc,
            mmuregnames[OP_RD(opcode)],
            st.g[OP_RT(opcode)].d);

         // Later Implementation:
         // If Status Register written to and that write included any
         // interrupt bits then check for CPU Interrupts.

         break;

      case 16:
         switch(OP_FUNC(opcode))
         {
          case 2: // TLBWI - write index
          case 6: // TLBWR - write random
             {
                 int mask,size;
                 int virt[2],phys[2];

                 mask=st.mmu[5].d;
                 virt[0]=st.mmu[10].d&~255;
                 phys[0]=st.mmu[2].d&~63;
                 phys[1]=st.mmu[3].d&~63;
                 size=mask/2+4096;
                 virt[1]=virt[0]+size;

                 logh("TLB at %08X: ind:%i lo0:%08X lo1:%08X hi:%08X mask:%08X\n",
                    st.pc,
                    st.mmu[0].d,
                    st.mmu[2].d,
                    st.mmu[3].d,
                    st.mmu[10].d,
                    st.mmu[5].d);
                 logh("TLB %08X,%08X = %08X,%08X size %08X\n",
                    virt[0],virt[1],phys[0],phys[1],size);

                 osMapMem(virt[0],phys[0],size);
                 osMapMem(virt[1],phys[1],size);

                 /*
                 if(st.dmatransfers<0x7fff0000)
                 {
                     st.dmatransfers++;
                     inifile_patches(st.dmatransfers);
                 }
                 */
             } break;
         }
         break;

      default:

         st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
         error( "RM - Unimplemented COP0-MMU Opcode" );
         print( "RM - Opcode (0x%08X) Rs (0x%02X) Func (0x%02X)\n", opcode, OP_RS(opcode), OP_FUNC(opcode));

         break;
   }
}

static void op_fpu(dword opcode)
{
    int fmt=OP_RS(opcode);
    int op=OP_FUNC(opcode);

    // 0x00-0x3f = basic ops
    // 0x40-0x7F = BC ops
    // 0x80-0x8F = BC ops
    if(fmt<8)
    { // Move ops
        int rt,fs;
        rt=OP_RT(opcode);
        fs=OP_RD(opcode);
        switch(fmt)
        {
        case 0: // MFC1
            st.g[rt].d=st.f[fs].d;
            break;
        case 1: // DMFC1
            st.g[rt].d2[1]=st.f[fs+0].d;
            st.g[rt].d2[0]=st.f[fs+1].d;
            break;
        case 4: // MTC1
            st.f[fs].d=st.g[rt].d;
            break;
        case 5: // DMTC1
            st.f[fs+0].d=st.g[rt].d2[1];
            st.f[fs+1].d=st.g[rt].d2[0];
            break;
        case 2: // CFC1
            st.g[rt].d=0; // no exceptions
            break;
        case 6: // CTC1
            // ignore
            break;
        default:
            st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
            error("unimplemented FPU-Move opcode");
            break;
        }
    }
    else if(fmt==8)
    { // BC ops (branch)
        int rt=OP_RT(opcode);
        int ontrue=rt&1;
        int likely=(rt&2)>>1;
        int flag;

        if(ontrue) flag=st.fputrue;
        else       flag=!st.fputrue;

        op_branch(opcode,flag,likely,0);
    }
    else
    { // generic ops
        double  r,a,b;
        int     storer=1;
        if(fmt==16)
        { // single
            a=(double)st.f[OP_RD(opcode)].f;
            b=(double)st.f[OP_RT(opcode)].f;
        }
        else if(fmt==17)
        { // double
            a=readdouble(OP_RD(opcode));
            b=readdouble(OP_RT(opcode));
        }
        else
        {
            if(op!=33 && op!=32) op=255; // force error
        }
        switch(op)
        {
        case 0: // ADD
            r=a+b;
            break;
        case 1: // SUB
            r=a-b;
            break;
        case 2: // MUL
            r=a*b;
            break;
        case 3: // DIV
            r=a/b;
            break;
        case 4: // SQRT
            r=sqrt(a);
            break;
        case 5: // ABS [not used]
            r=fabs(a);
            break;
        case 7: // NEG
            r=-a;
            break;
        case 48: // C.F
        case 49: // C.UN
        case 56: // C.SF
        case 57: // C.NGLE
            storer=0;
            st.fputrue=0;
            break;
        case 50: // C.EQ   [actually used]
        case 51: // C.UEQ
        case 58: // C.SEQ
        case 59: // C.NGL
            storer=0;
            st.fputrue= (a==b);
            break;
        case 60: // C.LT   [actually used]
        case 52: // C.OLT
        case 53: // C.ULT
        case 61: // C.NGE
            storer=0;
            st.fputrue= (a<b);
            break;
        case 62: // C.LE   [actually used]
        case 54: // C.OLE
        case 55: // C.ULE
        case 63: // C.NGT
            storer=0;
            st.fputrue= (a<=b);
            break;
        // convert & move
        case 6: // MOV
            storer=0;
            if(fmt==17 || fmt==21)
            { // qword
                st.f[OP_SHAMT(opcode)+0].d=st.f[OP_RD(opcode)+0].d;
                st.f[OP_SHAMT(opcode)+1].d=st.f[OP_RD(opcode)+1].d;
            }
            else
            { // dword
                st.f[OP_SHAMT(opcode)].d=st.f[OP_RD(opcode)].d;
            }
            break;
        case 12: // ROUND.W [not used]
            storer=0;
            st.f[OP_SHAMT(opcode)].d=(int)(a);
            break;
        case 9: // TRUN.L [used in goldeneye]
            {
                qreg v;
                storer=0;
                v.q=(qint)a;
                st.f[OP_SHAMT(opcode)+0].d=v.d2[1];
                st.f[OP_SHAMT(opcode)+1].d=v.d2[0];
            }
            break;
        case 13: // TRUN.W [used]
            storer=0;
            st.f[OP_SHAMT(opcode)].d=(int)(a);
            break;
        case 14: // CEIL.W [not used]
            storer=0;
            st.f[OP_SHAMT(opcode)].d=ceil(a);
            break;
        case 15: // FLOOR.W [not used]
            storer=0;
            st.f[OP_SHAMT(opcode)].d=floor(a);
            break;
        case 32: // CVT.S [used]
            storer=0;
            if(fmt==20)      st.f[OP_SHAMT(opcode)].f=st.f[OP_RD(opcode)].d;
            else if(fmt==21)
            {
                st.f[OP_SHAMT(opcode)].f=*(qint *)&st.f[OP_RD(opcode)].d;
            }
            else if(fmt==17) st.f[OP_SHAMT(opcode)].f=readdouble(OP_RD(opcode));
            else
            {
                 st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
                 error("unimplemented FPU-CVT-opcode");
            }
            break;
        case 33: // CVT.D [used]
            storer=0;
            if(fmt==20)      writedouble(OP_SHAMT(opcode),st.f[OP_RD(opcode)].d);
            else if(fmt==21)
            {
                // [used in goldeneye]
                qreg x;
                double d;
                x.q=*(qint *)&st.f[OP_RD(opcode)].d;
                d=(qint)x.q;
                writedouble(OP_SHAMT(opcode),d);
            }
            else if(fmt==16) writedouble(OP_SHAMT(opcode),st.f[OP_RD(opcode)].f);
            else
            {
                st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
                error("unimplemented FPU-CVT-opcode");
            }
            break;
        case 36: // CVT.W
            storer=0;
            if(fmt==17)      st.f[OP_SHAMT(opcode)].d=a;
            else if(fmt==16) st.f[OP_SHAMT(opcode)].d=(int)a;
            else
            {
                st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
                error("unimplemented FPU-CVT-opcode");
            }
            break;
        default:
            storer=0;
            st2.cpuerrorcnt++; if(st2.cpuerrorcnt>100) break;
            error("unimplemented FPU-opcode");
            break;
        }
        if(storer)
        {
            if(fmt==16)
            { // single
                st.f[OP_SHAMT(opcode)].f=(float)r;
            }
            else if(fmt==17)
            { // double
                writedouble(OP_SHAMT(opcode),r);
            }
        }
    }

}

static void op_main(dword opcode)
{
    int op,flag;
    int *rs,*rt,*rd,imm[2];

    op=OP_OP(opcode);
    if(op==0) op=OP_FUNC(opcode)+0x40;
    else if(op==1) op=OP_RT(opcode)+0x80;

    switch(op)
    {
    //------------------------------ignored coprosessor stuff
    case 16: // cop0
        op_scc( opcode );
        break;
    case 18: // cop2
        error("unimplemented COP2 opcode");
        break;
    //------------------------------special ops!
    case OP_PATCH: // *PATCH*
        {
            int p=OP_IMM(opcode);
            op_patch(p);
            if(p<10 || p>=50) st.bailout-=4;
            else st.bailout-=150; // approximate every os routine takes 100 cycles
        }
        break;
    case OP_GROUP: // *GROUP*
        error("reserved group-opcode encountered in cpuc");
        break;
    //------------------------------loads
    case 32: // LB
        op_readmem(opcode,-1);
        break;
    case 36: // LBU
        op_readmem(opcode,1);
        break;
    case 33: // LH
        op_readmem(opcode,-2);
        break;
    case 37: // LHU
        op_readmem(opcode,2);
        break;
    case 35: // LW
    case 39: // LWU
        op_readmem(opcode,4);
        break;
    case 34: // LWL
        op_rwmemrl(opcode,0,0);
        break;
    case 38: // LWR
        op_rwmemrl(opcode,0,1);
        break;
//    case 26: // LDL
//    case 27: // LDR
    case 55: // LD
//        logi("doubleword load at (%08X)\n",st.pc);
        st.expanded64bit=1;
        op_readmem(opcode,8);
        break;
    //------------------------------stores
    case 40: // SB
        op_writemem(opcode,1);
        break;
    case 41: // SH
        op_writemem(opcode,2);
        break;
    case 43: // SW
        op_writemem(opcode,4);
        break;
    case 42: // SWL
        op_rwmemrl(opcode,1,0);
        break;
    case 46: // SWR
        op_rwmemrl(opcode,1,1);
        break;
//    case 44: // SDL
//    case 45: // SDR
    case 63: // SD
//        logi("doubleword store at (%08X)\n",st.pc);
        op_writemem(opcode,8);
        break;
    //------------------------------arithmetic
    case 8: // ADDI
    case 9: // ADDIU
        GETREGSIMM;
        *rd=*rs + *rt;
        break;
    case 24: // DADDI
    case 25: // DADDIU
        GETREGSIMM64;
        addi64(rd,rs,rt);
        break;
    case 10: // SLTI
        GETREGSIMM;
        *rd=((int)*rs < (int)*rt);
        break;
    case 11: // SLTIU
        GETREGSIMM;
        *rd=((unsigned)*rs < (unsigned)*rt);
        break;
    case 12: // ANDI
        GETREGSIMMUNS;
        *rd=*rs & *rt;
        break;
    case 13: // ORI
        GETREGSIMMUNS;
        *rd=*rs | *rt;
        break;
    case 14: // XORI
        GETREGSIMMUNS;
        *rd=*rs ^ *rt;
        break;
    case 15: // LUI
        GETREGSIMM;
//        print("at %08X lui %04X\n",st.pc,(*rt)&0xffff);
        if((*rt&0x1F00)==0x0400)
        {
            if(hw_ismemiorange(*rt<<16)) st.memiodetected=(*rt<<16);
        }
        *rd=*rt << 16;
        break;
    case 0x40+0: // SLL
        GETREGS;
        *rd=*rt << OP_SHAMT(opcode);
        break;
    case 0x40+2: // SRL
        GETREGS;
        *rd=(unsigned)*rt >> OP_SHAMT(opcode);
        break;
    case 0x40+3: // SRA
        GETREGS;
        *rd=(int)*rt >> OP_SHAMT(opcode);
        break;
    case 0x40+4: // SLLV
        GETREGS;
        *rd=*rt << *rs;
        break;
    case 0x40+6: // SRLV
        GETREGS;
        *rd=(unsigned)*rt >> *rs;
        break;
    case 0x40+7: // SRAV
        GETREGS;
        *rd=(int)*rt >> *rs;
        break;
    case 0x40+20: // DSLLV
        op_shift64(opcode,0,-1);
        break;
    case 0x40+22: // DSRLV
        op_shift64(opcode,1,-1);
        break;
    case 0x40+23: // DSRAV
        op_shift64(opcode,2,-1);
        break;
    case 0x40+12: // SYSCALL
        exception("opcode syscall");
        break;
    case 0x40+13: // BREAK
        exception("opcode break");
        break;
    case 0x40+16: // MFHI
        GETREGS;
        rd[0]=st.mhi.d2[0];
        rd[1]=st.mhi.d2[1];
        break;
    case 0x40+17: // MTHI
        GETREGS;
        st.mhi.d2[0]=rs[0];
        st.mhi.d2[1]=rs[1];
        break;
    case 0x40+18: // MFLO
        GETREGS;
        rd[0]=st.mlo.d2[0];
        rd[1]=st.mlo.d2[1];
        break;
    case 0x40+19: // MTLO
        GETREGS;
        st.mlo.d2[0]=rs[0];
        st.mlo.d2[1]=rs[1];
        break;
    case 0x40+24: // MULT
        GETREGS;
        op_mult(*rs,*rt);
        break;
    case 0x40+25: // MULTU
        GETREGS;
        op_multu(*rs,*rt);
        break;
    case 0x40+26: // DIV
        GETREGS;
        op_div(*rs,*rt);
        break;
    case 0x40+27: // DIVU
        GETREGS;
        op_divu(*rs,*rt);
        break;
    case 0x40+28: // DMULT
        op_dmultu(OP_RS(opcode),OP_RT(opcode));
        break;
    case 0x40+29: // DMULTU
        op_dmultu(OP_RS(opcode),OP_RT(opcode));
        break;
    case 0x40+30: // DDIV
        op_ddiv(OP_RS(opcode),OP_RT(opcode));
        break;
    case 0x40+31: // DDIVU
        op_ddivu(OP_RS(opcode),OP_RT(opcode));
        break;
    case 0x40+32: // ADD
    case 0x40+33: // ADDU
        GETREGS;
        *rd=*rs + *rt;
        break;
    case 0x40+34: // SUB
    case 0x40+35: // SUBU
        GETREGS;
        *rd=*rs - *rt;
        break;
    case 0x40+36: // AND
        GETREGS;
        *rd=*rs & *rt;
        rd[1]=rs[1] & rt[1]; // do 64bit too (goldeneye needs)
        break;
    case 0x40+37: // OR
        GETREGS;
        *rd=*rs | *rt;
        rd[1]=rs[1] | rt[1]; // do 64bit too (goldeneye needs)
        break;
    case 0x40+38: // XOR
        GETREGS;
        *rd=*rs ^ *rt;
        rd[1]=rs[1] ^ rt[1]; // do 64bit too (goldeneye needs)
        break;
    case 0x40+39: // NOR
        GETREGS;
        *rd=~(*rs | *rt);
        rd[1]=~(rs[1] | rt[1]); // do 64bit too (goldeneye needs)
        break;
    case 0x40+42: // SLT
        GETREGS;
        *rd=((int)*rs < (int)*rt);
        break;
    case 0x40+43: // SLTU
        GETREGS;
        *rd=((unsigned)*rs < (unsigned)*rt);
        break;
    case 0x40+44: // DADD
    case 0x40+45: // DADDU
        GETREGS;
        addi64(rd,rs,rt);
        break;
    case 0x40+56: // DSLL
        op_shift64(opcode,0,OP_SHAMT(opcode)+0);
        break;
    case 0x40+58: // DSLR
        op_shift64(opcode,1,OP_SHAMT(opcode)+0);
        break;
    case 0x40+59: // DSLA
        op_shift64(opcode,2,OP_SHAMT(opcode)+0);
        break;
    case 0x40+60: // DSLL32
        op_shift64(opcode,0,OP_SHAMT(opcode)+32);
        break;
    case 0x40+62: // DSLR32
        op_shift64(opcode,1,OP_SHAMT(opcode)+32);
        break;
    case 0x40+63: // DSLA32
        op_shift64(opcode,2,OP_SHAMT(opcode)+32);
        break;
    //------------------------------jump and branch
    case 2: // J
        op_jump(opcode,0,-1);
        break;
    case 3: // JAL
        op_jump(opcode,1,-1);
        break;
    case 4: // BEQ
        GETREGSBR;
        flag=*rs==*rt;
        op_branch(opcode,flag,0,0);
        break;
    case 5: // BNEQ
        GETREGSBR;
        flag=*rs!=*rt;
        op_branch(opcode,flag,0,0);
        break;
    case 6: // BLEZ
        GETREGSBR;
        flag=(int)*rs<=0;
        op_branch(opcode,flag,0,0);
        break;
    case 7: // BGTZ
        GETREGSBR;
        flag=(int)*rs>0;
        op_branch(opcode,flag,0,0);
        break;
    case 20: // BEQL
        GETREGSBR;
        flag=(*rs==*rt);
        op_branch(opcode,flag,1,0);
        break;
    case 21: // BNEL BNEQL
        GETREGSBR;
        flag=(*rs!=*rt);
        op_branch(opcode,flag,1,0);
        break;
    case 22: // BLEZL
        GETREGSBR;
        flag=(int)*rs<=0;
        op_branch(opcode,flag,1,0);
        break;
    case 23: // BGTZL
        GETREGSBR;
        flag=(int)*rs>0;
        op_branch(opcode,flag,1,0);
        break;
    case 0x40+8: // JR
        op_jump(opcode,0,OP_RS(opcode));
        break;
    case 0x40+9: // JALR
        op_jump(opcode,1,OP_RS(opcode));
        break;
    case 0x80+0: // BLTZ
        GETREGSBR;
        flag=(int)*rs<0;
        op_branch(opcode,flag,0,0);
        break;
    case 0x80+1: // BGEZ
        GETREGSBR;
        flag=(int)*rs>=0;
        op_branch(opcode,flag,0,0);
        break;
    case 0x80+2: // BLTZL
        GETREGSBR;
        flag=(int)*rs<0;
        op_branch(opcode,flag,1,0);
        break;
    case 0x80+3: // BGEZL
        GETREGSBR;
        flag=(int)*rs>=0;
        op_branch(opcode,flag,1,0);
        break;
    case 0x80+16: // BLTZAL
        GETREGSBR;
        flag=(int)*rs<0;
        op_branch(opcode,flag,0,1);
        break;
    case 0x80+17: // BGEZAL
        GETREGSBR;
        flag=(int)*rs>=0;
        op_branch(opcode,flag,0,1);
        break;
    case 0x80+18: // BLTZALL
        GETREGSBR;
        flag=(int)*rs<0;
        op_branch(opcode,flag,1,1);
        break;
    case 0x80+19: // BGEZALL
        GETREGSBR;
        flag=(int)*rs>=0;
        op_branch(opcode,flag,1,1);
        break;
    case 47: // CACHE
        break;
    //------------------------------fpu
    case 17:
        op_fpu(opcode);
        break;
    case 53: // LDC1
        op_readmem(opcode,0x18);
        break;
    case 61: // SDC1
        op_writemem(opcode,0x18);
        break;
    case 49: // LWC1
        op_readmem(opcode,0x14);
        break;
    case 57: // SWC1
        op_writemem(opcode,0x14);
        break;
    //--------------
    default:
        error("unimplemented CPU opcode");
    }
}

void c_execop(dword opcode)
{
    op_main(opcode);

    st.pc+=4;

    if(st.branchdelay>0)
    {
        if(!--st.branchdelay)
        {
            if(st.branchtype==BRANCH_RET && st.memiodetected)
            {
                hw_memio();
                st.memiodetected=0;
            }
            cpu_notify_branch(st.branchto,st.branchtype);
            st.pc=st.branchto;
        }
    }
}

void c_exec(void)
{
    while(st.bailout>0)
    {
        c_execop(mem_readop(st.pc));
        st.bailout--;
        cpu_notify_pc(st.pc);
    }
}

