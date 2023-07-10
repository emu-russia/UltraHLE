#include "ultra.h"
#include "cpua.h"

/**************************************************************************
** Insert data to compiled stream
*/

void insertnothing(void)
{
    r.inserted=1;
}

void insertbyte(int x)
{
    byte *d;
    r.inserted=1;
    d=mem.code+mem.codeused;
    *d=x;
    mem.codeused+=1;
}

void insertdword(int x)
{
    dword *dd;
    r.inserted=1;
    dd=(dword *)(mem.code+mem.codeused);
    *dd=x;
    mem.codeused+=4;
}

void insertmodrmopcode(int opcode,int reg,int base,int offset)
{
    byte *d,*d0;
    int modrm,mod,offsz=0;
    r.inserted=1;
    d=d0=mem.code+mem.codeused;

    if(r.opt_eaxloads)
    {
        if(opcode==X_MOV+2 && mem.codeused==r.eaxload_codeusedend)
        {
            if(base==r.eaxload_base && offset==r.eaxload_offset)
            {
                if(reg==r.eaxload_reg)
                {
                    // alread loaded
    //                print("loaded at %08X\n",r.pc);
                    return;
                }
                else
                {
                    // just a reg move
    //                print("regmove at %08X\n",r.pc);
                    insertregopcode(X_MOV,reg,r.eaxload_reg);
                    return;
                }
            }
        }
    }

    r.eaxload_reg=reg;
    r.eaxload_base=base;
    r.eaxload_offset=offset;

    if(opcode&0xf0000) *d++=0xf;
    *d++=opcode;
    if((opcode&0xf000)==0x2000) return;
    if((opcode&0xf000)==0x1000) reg=(opcode>>8)&7;

    if(base==REG_EBX && (offset<-128 || offset>127))
    {
        // convert to direct address
        base=REG_NONE;
        offset+=((dword)&st)+STOFFSET;
    }

    if(base==REG_REG)
    {
        base=offset;
        mod=3;
    }
    else if(base==REG_NONE)
    {
        base=REG_EBP;
        mod=0;
        offsz=32;
    }
    else
    {
        if(offset==0)
        {
            mod=0;
        }
        else if(offset>=-128 && offset<128)
        {
            mod=1;
            offsz=8;
        }
        else
        {
            mod=2;
            offsz=32;
        }
    }

    modrm=(reg<<3)+(base)+(mod<<6);

    *d++=modrm;
    if(offsz==8)
    {
        *d++=offset;
    }
    else if(offsz==32)
    {
        *d++=offset; offset>>=8;
        *d++=offset; offset>>=8;
        *d++=offset; offset>>=8;
        *d++=offset; offset>>=8;
    }

    mem.codeused+=d-d0;

    if(opcode==X_MOV)
    {
        r.eaxload_codeusedend=mem.codeused;
    }
}


void insertmemropcode(int opcode,int dst,int base,int offset)
{
    opcode&=~2;
    opcode|=2;
    insertmodrmopcode(opcode,dst,base,offset);
}

void insertmemwopcode(int opcode,int base,int offset,int src)
{
    opcode&=~2;
    insertmodrmopcode(opcode,src,base,offset);
}

void insertregopcode(int opcode,int dst,int src)
{
    if(!(opcode&0xf000)) opcode&=~2;
    if(opcode==X_MOV)
    {
        if(dst==src) return; // null move
    }
    insertmodrmopcode(opcode,src,REG_REG,dst);
}

void insertimmopcode(int opcode,int xrs,int imm)
{
    int a=0;
    if(!(opcode&0xf000))
    {
        exception("cpua: insertimmopcode with X_NONIMM opcode\n");
    }
    if((opcode&0xf000)==0x3000)
    {
        insertbyte(opcode+xrs);
        insertdword(imm);
    }
    else if(opcode==X_IMMADD && imm>=-128 && imm<128)
    {
        opcode=X_IMMADD8;
        insertmodrmopcode(opcode,0,REG_REG,xrs);
        insertbyte(imm);
    }
    else
    {
        insertmodrmopcode(opcode,0,REG_REG,xrs);
        if(opcode&0x800000) insertbyte(imm);
        else insertdword(imm);
    }
}

void insertcall(void *routine)
{
    dword x=(dword)routine;
    dword y=(dword)(mem.code+mem.codeused+5);
    insertbyte(0xE8); // CALL
    insertdword(x-y);
}

void insertjump(void *routine)
{
    dword x=(dword)routine;
    dword y=(dword)(mem.code+mem.codeused+5);
    insertbyte(0xE9); // JMP
    insertdword(x-y);
}

void insert(t_asmop o)
{
    byte *d,*s;
    int   i;

    r.inserted=1;

    r.lastjumpto=0;

    s=(byte *)o;
    if(*s!=0x90) s+=5+*(int *)(s+1);
    s++;

    d=mem.code+mem.codeused;

    for(i=0;i<256;i++) // all routines <256 bytes in length
    {
        if(s[0]==0xb8 && s[1]==0xfc && s[2]==0xfd && s[3]==0xfe && s[4]==0xff && s[5]==0xcc)
        { // end marker
            break;
        }
        else if(s[0]==0xb8 && s[1]==0xfb && s[2]==0xfd && s[3]==0xfe && s[4]==0xff && s[5]==0xcc)
        { // end marker (other, no nop in debug cases)
            break;
        }
        else if(s[0]==0xfa && s[1]==0xfb && s[2]==0xfc)
        { // end marker
            dword x;
            x=ip[s[3]];
            s+=4;
            *d++=(x>> 0)&255;
            *d++=(x>> 8)&255;
            *d++=(x>>16)&255;
            *d++=(x>>24)&255;
        }
        else
        {
            *d++=*s++;
        }
    }

    mem.codeused=d-mem.code;
}

/**************************************************************************
** Misc utilities
*/

int getop(dword opcode)
{
    int op;
    op=OP_OP(opcode);
    if(op==0) op=OP_FUNC(opcode)+0x40;
    else if(op==1) op=OP_RT(opcode)+0x80;
    return(op);
}

/**************************************************************************
** Group utilities
*/

Group *ac_creategroup(dword pc)
{ // if group exists, it will be returned
    Group *g;
    dword  op;
    if(mem.groupnum>=mem.groupmax)
    {
        a_clearcodecache();
    }
    g=mem.group+mem.groupnum;
    op=mem_read32(pc);
    if(OP_OP(op)==OP_GROUP)
    {
        // group exists
        return(mem.group+OP_IMM24(op));
    }
    g->addr=pc;
    g->opcode=op;
    mem_write32(pc,GROUP(mem.groupnum));
    g->type=GROUP_NEW;
    g->len=0;
    g->code=NULL;
    mem.groupnum++;
//    logc("compiler: group %08X created\n",g->addr);
    cstat.unexec++;
    return(g);
}

void a_cleardeadgroups(void)
{
    int i,n,cnt=0;
    dword x;
    Group *g;
    static int f=0;

    n=1024;
    if(n>mem.groupnum) n=mem.groupnum;

    for(i=0;i<n;i++)
    {
        g=mem.group+f;
        x=mem_read32(g->addr);
        if(x!=GROUP(f))
        {
            g->type=GROUP_NEW;
            g->code=NULL;
            g->opcode=x;

            if(g->type==GROUP_PATCH) cstat.patch--;
            else if(g->type==GROUP_FAST) cstat.ok--;
            else if(g->type==GROUP_SLOW) cstat.fail--;
            cstat.unexec++;

            cnt++;
        }

        f++;
        if(f>=mem.groupnum) f=0;
    }
//    print("cleardeadgroups: %i cleared\n",cnt);
}

void a_clearcodecache(void)
{
    int    i;
    int    cnt=0;
    int    cnt2=0;
    dword  x;
    Group *g;
    int    clears=cstat.clears+1;

    /*
    if(clears>100)
    {
        exception("Over 100 code clears. Code cache size too small.\n");
    }
    */

    // expensive operation, clear all cache and restore memory
    for(i=1;i<mem.groupnum;i++)
    {
        g=mem.group+i;

        x=mem_read32(g->addr);
        if(x==g->opcode)
        {
            // alread ok
        }
        else if(x==GROUP(i))
        { // yes still unchanged, restore it
//              print("restored %08X: %08X <- %08X\n",g->addr,mem_read32(g->addr),g->opcode);
            mem_write32(g->addr,g->opcode);
            cnt++;
        }
        else
        {
            cnt2++;
//              print("clearcodecache: restore failed g%-5i (addr %08X data %08X orig %08X)\n",i,g->addr,mem_read32(g->addr),g->opcode);
        }

        g->code=NULL;
        g->type=0;
        g->len=0;
        g->addr=0;
    }
    print("clearcodecache: %i groups restored (%i failed)\n",cnt,cnt2);

    // add the first dummy group
    g=mem.group;
    g->code=NULL;
    g->type=0;
    g->len=0;
    g->addr=0;
    mem.groupnum=1;

    // clear codecache
    mem.codeused=0;

    // clear stats
    memset(&cstat,0,sizeof(cstat));
    cstat.clears=clears;
}

/***********************************************************************
** Routines for X86 register allocation
*/

XReg reg[8];

static int  reglastused;

// List of regs used for allocation in reg_oldest()
static int useregs[]={REG_ESI,REG_EDI,REG_EBP,REG_EDX,-1};
//static int useregs[]={REG_ESI,-1};

//--

int reg_oldest(void)
{
    int i,j;
    dword best,besti;
    besti=0;
    best=0xffffffff;
    for(j=0;useregs[j]>=0;j++)
    {
        i=useregs[j];
        if(reg[i].lastused<best)
        {
            besti=i;
            best=reg[i].lastused;
        }
    }
    i=besti;
    return(i);
}

int reg_alloc(int name,int x86)
{
    int i;

    reg_free(x86);

    // make sure no other regs have this same name
    for(i=0;i<8;i++)
    {
        if(reg[i].name==name)
        {
            reg[i].name=0;
            reg[i].changed=0;
            reg[i].lastused=0;
        }
    }

    reg[x86].name=name;
    reg_rd(x86);

    return(x86);
}

int reg_allocnew(int name)
{
    int i,x;

    if(!name)
    {
        exception("cpuautil: reg_allocnew with r0 at %08X\n",r.pc);
        return(REG_NONE);
    }
    else if(name==0x1f)
    {
        // always allocate RA to EDX if EDX is not reserved to a TEMP.
        // this is because RA is often used to return very soon after,
        // in which case it would have to be copied to EDX
        if((reg[REG_EDX].name&XNAME_TYPE)!=XNAME_TEMP)
        {
            i=REG_EDX;
        }
        else
        {
            i=reg_oldest();
        }
    }
    else
    {
        i=reg_oldest();
    }

    x=reg_alloc(name,i);

    if(x<0 || x>7 || x==REG_ESP) exception("cpuautil: reg_alloc (name=%i, x=%i, pc=%08X)",name,x,r.pc);

    return(x);
}

//--

void reg_load(int x86)
{
    int type=reg[x86].name&XNAME_TYPE;
    if(type==XNAME_MIPS)
    {
        // load data to the reg
        insertmemropcode(X_MOV,x86,REG_EBX,STADDR(st.g[reg[x86].name&XNAME_INDEX]));
    }
    else exception("cpuanew: reg_save unrecognized register type");
}

void reg_save(int x86)
{
    int type=reg[x86].name&XNAME_TYPE;
    if(x86==REG_EAX || x86==REG_ESP)
    {
        exception("cpuautil: reg_save %i at %08X!\n",x86,r.pc);
    }
    if(type==XNAME_MIPS)
    {
        // save data to the reg
        insertmemwopcode(X_MOV,REG_EBX,STADDR(st.g[reg[x86].name&XNAME_INDEX]),x86);
    }
    else if(type==XNAME_TEMP)
    {
        // temps not saved
    }
    else exception("cpuanew: reg_save unrecognized register type");
    reg[x86].changed=0;
}

//--

int reg_find(int name)
{
    int i;
    if(!name) return(REG_NONE);
    for(i=0;i<8;i++)
    {
        if(reg[i].name==name) return(i);
    }
    return(REG_NONE);
}

void reg_rename(int x86,int name)
{
    int i;
    for(i=0;i<8;i++)
    {
        if(reg[i].name==name)
        {
            reg[x86].name=0;
            reg[x86].changed=0;
            reg[x86].lastused=0;
        }
    }
    reg_alloc(name,x86);
}

void reg_wr(int x86)
{
    reg[x86].changed=1;
    reg[x86].lastused=reglastused++;
}

void reg_rd(int x86)
{
    reg[x86].lastused=reglastused++;
}

//--

void reg_free(int x86)
{
    if(reg[x86].changed)
    {
        reg_save(x86);
    }
    reg[x86].name=0;
    reg[x86].changed=0;
    reg[x86].lastused=0;
}

void reg_freeall(void)
{
    int i;
    for(i=0;i<8;i++) reg_free(i);
}

void reg_freeallbut(int name1,int name2)
{
    int i;
    for(i=0;i<8;i++)
    {
        if(reg[i].name==name1 || reg[i].name==name2) continue;
        reg_free(i);
    }
}

/***********************************************************************
** Routines for X86 FPU register allocation
*/

XReg  fpreg[16];
static int  fpregnum;

static int  fpreglastused;

int freg_find(int name)
{
    int i,j;
    if(fpregnum>8)
    {
        exception("cpuautil: fpregnum %i > 8\n",fpregnum);
    }

    j=REG_NONE;
    for(i=0;i<fpregnum;i++)
    {
        if(fpreg[i].name==name && fpreg[i].size>0)
        {
            if(j!=REG_NONE) exception("cpuautil: duplicate fpreg\n");
            j=i;
        }
    }

    return(j);
}

// these really load/save/change things

// this creates a new stack top and loads given reg to it
void freg_push(int name,int size)
{
    int i;

    if(fpregnum>=7)
    {
        freg_save(fpregnum-1);
    }

    i=freg_find(name);
    if(i!=REG_NONE)
    {
        // FLD st(i)
        insertbyte(0xd9);
        insertbyte(0xc0+i);
        freg_pushtop(0,0); // will be overritten
    }
    else
    {
        freg_load(name,size);
    }
}

void freg_xchg(int i)
{
    XReg tmp;

    if(!i) return;

    tmp=fpreg[i];
    fpreg[i]=fpreg[0];
    fpreg[0]=tmp;

    // FXCHG st(i)
    insertbyte(0xd9);
    insertbyte(0xc8+i);
}

void freg_load(int name,int size)
{
    if(fpregnum>=7)
    {
        freg_save(fpregnum-1);
    }

    freg_pushtop(name,size);

    // load it
    if(fpreg[0].size==4)
    {
        insertmodrmopcode(X_FLD4,0,REG_EBX,STADDR(st.f[name]));
    }
    else if(fpreg[0].size==8)
    {
        insertmodrmopcode(X_FLD8,0,REG_EBX,STADDR(st.f[name]));
    }
    else exception("cpuautil: illegal freg_load\n");
}

void freg_save(int i)
{
    int name;
    freg_xchg(i);

    name=fpreg[0].name;

    // save it
    if(fpreg[0].size==0)
    {
        // fstp st(0)
        insertbyte(0xDD);
        insertbyte(0xD8);
    }
    else if(fpreg[0].size==4)
    {
        insertmodrmopcode(X_FSTP4,0,REG_EBX,STADDR(st.f[name]));
    }
    else if(fpreg[0].size==8)
    {
        insertmodrmopcode(X_FSTP8,0,REG_EBX,STADDR(st.f[name]));
    }
    else exception("cpuautil: illegal freg_save\n");

    freg_poptop();
}

// these just rename/reogranize stack, loading done by caller

void freg_renametop(int name,int size)
{
    int i;
    fpreg[0].name=name;
    fpreg[0].size=size;

    if(!size) return;

    // delete other copies of this reg
    for(i=1;i<fpregnum;i++)
    {
        if(fpreg[i].name==name)
        {
            freg_delete(i);
        }
    }
}

void freg_pushtop(int name,int size)
{
    int i;

    // push the stack
    for(i=fpregnum;i>=1;i--)
    {
        fpreg[i]=fpreg[i-1];
    }
    fpregnum++;

    freg_renametop(name,size);
}

void freg_poptop(void)
{
    int i;

    // pop the stack
    for(i=0;i<fpregnum-1;i++)
    {
        fpreg[i]=fpreg[i+1];
    }
    fpregnum--;

    fpreg[fpregnum].size=0;
    fpreg[fpregnum].name=0;
}

void freg_delete(int i)
{
    // just mark as nosave
    fpreg[i].size=0;
    fpreg[i].name=0;
}

void freg_saveall(void)
{
    while(fpregnum>0)
    {
        freg_save(0);
    }
}

void reg_init(void)
{
    memset(reg,0,sizeof(reg));
}

void freg_init(void)
{
    memset(fpreg,0,sizeof(reg));
    fpregnum=0;
}

void freg_dump(void)
{
    int i;
    print("FP-Stack: ");
    for(i=0;i<fpregnum;i++)
    {
        print("%2i.%i ",fpreg[i].name,fpreg[i].size);
    }
    print("\n");
}


