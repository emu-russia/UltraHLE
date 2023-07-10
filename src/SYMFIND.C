#include "ultra.h"

#define OPS     16
#define CALLS   8
#define MAXDESC 512

// magic xors to avoid direct MIPS code in the executable
#define XORMAGIC1  0x1a2b3cff
#define XORMAGIC2  0x0000ffff

// routine description
typedef struct
{
    // first 16 operators
    char  *name;
    // basic info
    dword  addr;              // unknown when searching of course
    dword  length;            // length in ops
    // simple compare: X=(MEM^DATA)&MASK, if 0 then ok
    dword  opdata[OPS];       // data                            [XOR MAGIC1 in h]
    dword  opmask[OPS];       // mask (which bits are important) [XOR MAGIC2 in h]
    // additional
    dword  calls;             // number of JALs
    dword  calllength[CALLS]; // lengths of first 8 calls
} Desc;

Desc  desc[MAXDESC];

void symfind_init(void)
{
    int i,j;
    // unxor the magic words
    for(i=0;i<MAXDESC;i++)
    {
        for(j=0;j<OPS;j++)
        {
            desc[i].opdata[j]^=XORMAGIC1;
            desc[i].opmask[j]^=XORMAGIC2;
        }
    }
}

int d_length(dword addr)
{
    int i;
    dword x;
    for(i=0;i<256;i++)
    {
        x=mem_read32(addr+i*4);
        if(x==0x03e00008)
        { // JR RA
            return(i+2);
        }
    }
    return(i);
}

void d_readops(Desc *d)
{
    int i,imm,n,c;
    dword x,data,mask;
    n=d->length;
    if(n>OPS) n=OPS;
    for(i=0;i<n;i++)
    {
        x=mem_read32(d->addr+i*4);
        data=x; mask=0;

        c=OP_OP(x);
        imm=OP_IMM(x);
        if(c==3 || c==2)
        { // JAL or J
            mask|=0x3ffffff;
        }
        else if(c==15)
        { // LUI
            if(imm>=0xa400 && imm<=0xafff) { /* leave mask*/ }
            else mask|=0xffff;
        }
        else if(c==16 || c==17)
        { // COP0/COP1
            // leave mask
        }
        else if(c==9 && OP_RS(x)==0x1d && OP_RT(x)==0x1d)
        { // addiu sp=..
            // leave mask
        }
        else if(c>=4 && c<=15)
        { // immediates
            mask|=0xffff;
        }
        else if(c>=32 && c<=46)
        { // load/store
            if(OP_RS(x)==0x1d)
            { // stack address, keep immediate
                // leave mask
            }
            else mask|=0xffff;
        }
        else
        {
            // leave mask
        }

        mask=~mask;
        data&=mask;
        d->opdata[i]=data;
        d->opmask[i]=mask;
    }
}

void d_readcalls(Desc *d)
{
    int i,n,c;
    dword x;
    n=d->length;
    d->calls=0;
    for(i=0;i<n;i++)
    {
        x=mem_read32(d->addr+i*4);

        c=OP_OP(x);
        if(c==3)
        { // JAL
            if(d->calls<CALLS)
            {
                x=((OP_TAR(x)<<2)&0x0fffffff)|((d->addr)&0xf0000000);
                d->calllength[d->calls]=d_length(x);
            }
            d->calls++;
        }
    }
}

void d_dump(Desc *d)
{
    int i;
    print("Routine: %s\n",d->name);
    print("address: %08X\n",d->addr);
    print(" length: %i\n",d->length);

    print("  calls: %i ",d->calls);
    if(d->calls>0)
    {
        print("(");
        for(i=0;i<d->calls;i++) print("%4i",d->calllength[i]);
        print(" )\n");
    }
    else print("\n");

    print("   data: ");
    for(i=0;i<7;i++) print("%08X ",d->opdata[i]);
    print("...\n");

    print("   mask: ");
    for(i=0;i<7;i++) print("%08X ",d->opmask[i]);
    print("...\n");
}

void symfind_saveroutine(dword addr,char *name)
{
    Desc d0;
    Desc *d=&d0;

    d->addr=addr;
    d->name=malloc(strlen(name)+1);
    strcpy(d->name,name);
    d->length=d_length(d->addr);
    d_readops(d);
    d_readcalls(d);
    d_dump(d);
}

char *symfind_matchroutine(dword addr)
{
    return(NULL);
}

