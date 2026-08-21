#include "ultra.h"

Mem mem; // mem.c

void mem_init(int ramsize)
{
    int   i,a;

    mem.ramsize=ramsize;
    if(mem.ram)
    {
        free(mem.ramalloc);
        mem.ram=NULL;
        mem.ramalloc=NULL;
    }
    // have 4K extra around mem to avoid access violations when
    // asm optimization code accesses a bit outside memory
    mem.ramalloc=(uint8_t *)malloc(mem.ramsize+16384);
    if(mem.ramalloc)
    {
        memset(mem.ramalloc,0,mem.ramsize+16384);
    }
    else
    {
        print("fatal error could not allocate mem.ramsize\n");
        flushdisplay();
        exit(3);
    }
    mem.ram=(uint8_t *)( ((uintptr_t)mem.ramalloc+8192) & ~(uintptr_t)4095 );

    // init io pages
    for(i=0;i<IO_MAX;i++)
    {
        a=0; //NULLFILL;
        memset(mem.io[i][0],a,4096); // write page
        a=0;
        memset(mem.io[i][1],a,4096); // read page
    }

    // setup NULL read page specially:
    // full of ZERO, except a single NULLFILL at center to stop exec
    memset(RNULL,0,4096);
    RNULL[512]=NULLFILL;

    // clear memlookup to null memory
    for(i=0;i<1048576;i++)
    {
        mem_mapexternal(i*4096,MAP_WTHENR,mem.io[IO_NULL]);
    }

    // map rdram into virtual memory
    for(i=0;i<mem.ramsize;i+=4096)
    {
        mem_mapphysical(i,MAP_RW,i);
    }

    // map iopages
    mem_mapexternal(0x03FF0000,MAP_WTHENR,mem.io[IO_OS]);
    mem_mapexternal(0x04000000,MAP_WTHENR,mem.io[IO_SPD]);
    mem_mapexternal(0x04001000,MAP_WTHENR,mem.io[IO_SPI]);
    mem_mapexternal(0x04040000,MAP_WTHENR,mem.io[IO_SP]);
    mem_mapexternal(0x04080000,MAP_WTHENR,mem.io[IO_SP2]);
    mem_mapexternal(0x04100000,MAP_WTHENR,mem.io[IO_DP]);
    mem_mapexternal(0x04200000,MAP_WTHENR,mem.io[IO_DPSPAN]);
    mem_mapexternal(0x04300000,MAP_WTHENR,mem.io[IO_MI]);
    mem_mapexternal(0x04400000,MAP_WTHENR,mem.io[IO_VI]);
    mem_mapexternal(0x04500000,MAP_WTHENR,mem.io[IO_AI]);
    mem_mapexternal(0x04600000,MAP_WTHENR,mem.io[IO_PI]);
    mem_mapexternal(0x04700000,MAP_WTHENR,mem.io[IO_RI]);
    mem_mapexternal(0x04800000,MAP_WTHENR,mem.io[IO_SI]);
    mem_mapexternal(0x18000000,MAP_WTHENR,mem.io[IO_MISC1]);
    mem_mapexternal(0x1F400000,MAP_WTHENR,mem.io[IO_MISC2]);
    mem_mapexternal(0x1F480000,MAP_WTHENR,mem.io[IO_RDB]);
    mem_mapexternal(0x1FC00000,MAP_WTHENR,mem.io[IO_PIF]);

    // duplicate physical memory from 00000000..1f000000 to
    // 80000000, A0000000, C0000000, E0000000
    for(i=0;i<0x20000000;i+=4096)
    {
        mem_mapcopy(i+0x80000000,MAP_RW,i);
        mem_mapcopy(i+0xa0000000,MAP_RW,i);
        mem_mapcopy(i+0xc0000000,MAP_RW,i);
        mem_mapcopy(i+0xe0000000,MAP_RW,i);
    }

    // allocate codecaches
    mem.codemax=COMPILER_CODEMAX;
    mem.code=malloc(mem.codemax);
    if(!mem.code)
    {
        print("fatal error could not allocate mem.code\n");
        flushdisplay();
        exit(3);
    }

    // allocate group table
    mem.groupmax=COMPILER_GROUPMAX;

    if(mem.group)
    {
        free(mem.group);
        mem.group=NULL;
    }
    mem.group=malloc(sizeof(Group)*mem.groupmax);
    if (!mem.group) {
        print("fatal error could not allocate mem.group\n");
        flushdisplay();
        exit(3);
    }
    memset(mem.group,0,sizeof(Group)*mem.groupmax);

    if(mem.groupcnt)
    {
        free(mem.groupcnt);
        mem.groupcnt=NULL;
    }
    mem.groupcnt=malloc(sizeof(int)*mem.groupmax);
    if (!mem.groupcnt) {
        print("fatal error could not allocate mem.groupcnt\n");
        flushdisplay();
        exit(3);
    }
    memset(mem.groupcnt,0,sizeof(int)*mem.groupmax);

    // set a single NULLFILL to first page
    mem_write32(0x1f0,NULLFILL);

    // init default values to hardware regs
    hw_init();
}

// map page[dst] to external 4K array (external data NOT saved!)
// also used by mapphysical internally
void mem_mapexternal(uint32_t dst,int rw,void *data)
{
    uint8_t *b;
    b=(uint8_t *)( (uintptr_t)data - dst );
    switch(rw)
    {
    case MAP_W:
        mem.lookupw[mempage(dst)]=b;
        break;
    case MAP_R:
        mem.lookupr[mempage(dst)]=b;
        break;
    case MAP_RW:
        mem.lookupw[mempage(dst)]=b;
        mem.lookupr[mempage(dst)]=b;
        break;
    case MAP_WTHENR:
        mem.lookupw[mempage(dst)]=b;
        mem.lookupr[mempage(dst)]=b+4096;
        break;
    }
}

// map page[dst] to where page[src] points to
void mem_mapcopy(uint32_t dst,int rw,uint32_t src)
{
    uint8_t *br;
    uint8_t *bw;
    br=(uint8_t *)((uintptr_t)mem.lookupr[mempage(src)]-dst+src);
    bw=(uint8_t *)((uintptr_t)mem.lookupw[mempage(src)]-dst+src);
    switch(rw)
    {
    case MAP_W:
        mem.lookupw[mempage(dst)]=bw;
        break;
    case MAP_R:
        mem.lookupr[mempage(dst)]=br;
        break;
    case MAP_RW:
        mem.lookupr[mempage(dst)]=br;
        mem.lookupw[mempage(dst)]=bw;
        break;
    }
}

// map page[dst] to physical address src
void mem_mapphysical(uint32_t dst,int rw,uint32_t src)
{
    mem_mapexternal(dst,rw,mem.ram+src);
}

// physical is based on write mapping
uint32_t mem_getphysical(uint32_t virtual_addr)
{
    uint8_t *raw=(uint8_t *)memdatar(virtual_addr);
    if(raw>=mem.ram && raw<mem.ram+mem.ramsize)
    {
        return(raw-mem.ram);
    }
    else return(-1);
}

/**
 * Computes the physical offset of a lookup table entry.
 * @param x Lookup table (mem.lookupr or mem.lookupw).
 * @param i Page index.
 * @return Physical offset, or 0xffffffff if outside RDRAM.
 */
uint32_t lookupvalue(uint8_t **x,int i)
{
    uintptr_t a;
    a=(uintptr_t)(i*4096)+(uintptr_t)x[i]-(uintptr_t)mem.ram;
    if(a>(uintptr_t)mem.ramsize) return(0xffffffff);
    return((uint32_t)a);
}

/**
 * Saves a lookup table to a file with simple run-length compression.
 * @param x Lookup table to save.
 * @param cnt Number of entries in the table.
 * @param f1 File to save to.
 */
void savelookup(uint8_t **x,int cnt,FILE *f1)
{
    int i,j,a;
    uintptr_t x0,x1;
    for(i=0;i<cnt;)
    {
        x0=lookupvalue(x,i);
        x1=lookupvalue(x,i+1);
        if(x0==x1)
        {
            // scan for same values
            for(j=i+2;j<cnt;j++) if(lookupvalue(x,j)!=x0) break;

            putc(2,f1);
            a=j-i;
            fwrite(&a,1,4,f1);
            fwrite(&x0,1,4,f1);
            i=j;
        }
        else if(x0+4096==x1)
        {
            // scan for continuous values
            for(j=i+2;j<cnt;j++) if(lookupvalue(x,j)!=x0+(j-i)*4096) break;

            putc(3,f1);
            a=j-i;
            fwrite(&a,1,4,f1);
            fwrite(&x0,1,4,f1);
            i=j;
        }
        else
        {
            // single value
            putc(1,f1);
            fwrite(&x0,1,4,f1);
            i++;
        }
    }
}

/**
 * Loads a lookup table from a file.
 * @param x Lookup table to fill.
 * @param cnt Number of entries in the table.
 * @param f1 File to load from.
 */
void loadlookup(uint8_t **x,int cnt,FILE *f1)
{
    int i,a,n;
    for(i=0;i<cnt;)
    {
//        print("i=%08X ",i);
        a=getc(f1);
        switch(a)
        {
        case 1:
            fread(&a,1,4,f1);
//            print("- %08X\n",a);
            if(i>cnt) break;
            if(a!=0xffffffff) x[i]=(uint8_t *)((uintptr_t)a+(uintptr_t)mem.ram-(uintptr_t)i*4096);
            i++;
            break;
        case 2:
            fread(&n,1,4,f1);
            fread(&a,1,4,f1);
//            print("- %08X * %i\n",a,n);
            if(i+n>cnt) break;
            while(n-->0)
            {
                if(a!=0xffffffff) x[i]=(uint8_t *)((uintptr_t)a+(uintptr_t)mem.ram-(uintptr_t)i*4096);
                i++;
            }
            break;
        case 3:
            fread(&n,1,4,f1);
            fread(&a,1,4,f1);
//            print("- %08X * %i (incr)\n",a,n);
            if(i+n>cnt) break;
            while(n-->0)
            {
                if(a!=0xffffffff) x[i]=(uint8_t *)((uintptr_t)a+(uintptr_t)mem.ram-(uintptr_t)i*4096);
                a+=4096;
                i++;
            }
            break;
        default:
            exception("mem_load fatal error!");
            return;
        }
    }
    print(NULL);
}

void mem_save(FILE *f1)
{
    int i,j;
    int ramsize2;
    uint32_t *ram,x;
    // save lookups (simple compression)
    #if 0
    int rw,n,m;
    uint8_t **lookup;
    fwrite(&mem.ramsize,1,4,f1);
    // old
    for(rw=0;rw<2;rw++)
    {
        if(rw) lookup=mem.lookupw;
        else lookup=mem.lookupr;
        for(i=0;i<1048576;)
        {
            m=(i*4096+lookup[i])-(uintptr_t)mem.ram;
            if(m<0 || m>=mem.ramsize) m=0xffffffff;
            for(j=1;j<30000 && i+j<1048576;j++)
            {
                m=(i*4096+lookup[i])-(uintptr_t)mem.ram;
                n=lookup[i+j]-mem.ram;
                if(n<0 || n>=mem.ramsize) n=0xffffffff;
                if(m!=n) break;
            }
            putc(j&255,f1);
            putc(j>>8,f1);

            fwrite(&m,1,4,f1);
            i+=j;
        }
    }
    #else
    i=0;
    fwrite(&i,1,4,f1);
    fwrite(&mem.ramsize,1,4,f1);
    savelookup(mem.lookupw,1048576,f1);
    savelookup(mem.lookupr,1048576,f1);
    // guard byte, simple verification uncompress succeeded
    putc(0xfc,f1);
    #endif
    // save memory (simple runlength compression)
    j=0;
    ram=(uint32_t *)mem.ram;
    ramsize2=mem.ramsize>>2;
    for(i=0;i<ramsize2;)
    {
        // scan same data
        x=ram[i];
        for(j=0;j<30000 && i+j<ramsize2;j++)
        {
            if(ram[i+j]!=x) break;
        }

        putc(j&255,f1);
        putc(j>>8,f1);
        fwrite(&x,1,4,f1);
        i+=j;

        // scan different data
        for(j=0;j<30000 && i+j<ramsize2;j++)
        {
            if(ram[i+j]==ram[i+j-1]) break;
        }

        putc(j&255,f1);
        putc(j>>8,f1);
        fwrite(ram+i,4,j,f1);
        i+=j;
    }
    // guard byte, simple verification uncompress succeeded
    putc(0xfc,f1);
}

void mem_load(FILE *f1)
{
    int i,n,j,m,rw;
    int ramsize2;
    int new=0;
    uint8_t **lookup;
    uint32_t *ram,x;
    fread(&mem.ramsize,1,4,f1);
    if(!mem.ramsize)
    {
        // new lookup
        new=1;
        fread(&mem.ramsize,1,4,f1);
        loadlookup(mem.lookupw,1048576,f1);
        loadlookup(mem.lookupr,1048576,f1);
        if(getc(f1)!=0xfc)
        {
            exception("mem_load fatal error (vm)!");
            return;
        }
    }
    else
    {
        // old lookup
        for(rw=0;rw<2;rw++)
        {
            if(rw) lookup=mem.lookupw;
            else lookup=mem.lookupr;
            for(i=0;i<1048576;)
            {
                n=getc(f1);
                n|=getc(f1)<<8;
                if(n<0)
                {
                    exception("mem_load fatal error (eof)!\n");
                    return;
                }
                fread(&m,1,4,f1);
                /*
                if(i>=0x7f000000/4096 && i<=0x7fffffff/4096)
                {
                    // no write
                }
                else
                */
                {
                    if(m!=0xffffffff)
                    {
                        for(j=0;j<n;j++) lookup[i+j]=m+mem.ram;
                    }
                }
                i+=n;
            }
        }
    }

    ramsize2=mem.ramsize>>2;
    ram=(uint32_t *)mem.ram;
    for(i=0;i<ramsize2;)
    {
        n=getc(f1);
        n|=getc(f1)<<8;
        if(n<0)
        {
            exception("mem_load fatal error (eof1-%06X)!",i);
            return;
        }
        fread(&x,1,4,f1);
        while(n-->0) ram[i++]=x;

        n=getc(f1);
        n|=getc(f1)<<8;
        if(n<0)
        {
            exception("mem_load fatal error (eof2-%06X)!",i);
            return;
        }
        fread(ram+i,n,4,f1);
        i+=n;
    }

    if(getc(f1)!=0xfc)
    {
        exception("mem_load fatal error (data)!");
    }
}

// memory access

uint32_t mem_read8(uint32_t addr)
{
    uint32_t a=addr&~3;
    uint32_t x=*memdatar(a);
    uint32_t b=((uint8_t *)&x)[3-(addr&3)];
    return(b);
}

uint32_t mem_read16(uint32_t addr)
{
    uint32_t a=addr&~3;
    uint32_t b;
    if(addr&1) error("mem_read16: address align");
    else
    {
        b=mem_read8(addr+0)<<8;
        b|=mem_read8(addr+1);
    }
    return(b);
}

uint32_t mem_readop(uint32_t addr)
{
    uint32_t x;
    x=mem_read32(addr);
    if(OP_OP(x)==OP_GROUP)
    { // group opcode
        x=OP_IMM24(x);
        if(x>=mem.groupnum) x=GROUP(0);
        else x=mem.group[x].opcode;
    }
    return(x);
}

char *mem_readoptype(uint32_t addr)
{
    static char buf[40];
    uint32_t x,t,l;
    x=mem_read32(addr);
    if(OP_OP(x)!=29)
    {
        strcpy(buf,"    ");
    }
    else
    {
        x=OP_IMM24(x);
        if(x>=mem.groupnum) t=l=0;
        else
        {
            t=mem.group[x].type;
            l=mem.group[x].len;
        }
        if(l>99) l=99;
        switch(t)
        {
        case GROUP_NEW:
            sprintf(buf,"n   ");
            break;
        case GROUP_FAST:
            sprintf(buf,"a%-2i ",l);
            break;
        case GROUP_SLOW:
            sprintf(buf,"c%-2i ",l);
            break;
        case GROUP_PATCH:
            sprintf(buf,"p   ");
            break;
        default:
            sprintf(buf,"?   ");
            break;
        }
    }
    return(buf);
}

int mem_groupat(uint32_t addr)
{
    int gi;
    gi=mem_read32(addr)&0xffff;
    if(gi<mem.groupnum && mem.group[gi].addr==addr)
    {
        return(gi);
    }
    return(-1);
}

void mem_write8(uint32_t addr,uint32_t data)
{
    uint32_t a=addr&~3;
    uint32_t x=*memdataw(a);
    ((uint8_t *)&x)[3-(addr&3)]=data;
    *memdataw(a)=x;
}

void mem_write16(uint32_t addr,uint32_t data)
{
    uint32_t a=addr&~3;
    if(addr&1) error("mem_read16: address align");
    else
    {
        mem_write8(addr+0,data>>8);
        mem_write8(addr+1,data&255);
    }
}

// copying a lot of data (addr MUST be dword aligned)

void  mem_readrangeraw(uint32_t addr,int bytes,char *data)
{
    uint32_t *m;
    int    num,i;

    if((addr&3) || (bytes&3)) exception("mem_readrangeraw: align error (%i bytes at %08X)",bytes,addr);

    while(bytes>0)
    {
        //print("memcpy %08X read %04X\n",addr,bytes);
        m=memdatar(addr);
        num=0x1000-(addr&0xfff);
        if(num>bytes) num=bytes;

        for(i=0;i<num;i++)
        {
        }
        memcpy(data,m,num);

        addr+=num;
        bytes-=num;
        data+=num;
    }
}

void  mem_writerangeraw(uint32_t addr,int bytes,char *data)
{
    uint32_t *m;
    int    num;

    if((addr&3) || (bytes&3)) exception("mem_writerangeraw: align error (%i bytes at %08X)",bytes,addr);

    while(bytes>0)
    {
        //print("memcpy %08X bytes %04X\n",addr,bytes);

        m=memdataw(addr);
        num=0x1000-(addr&0xfff);
        if(num>bytes) num=bytes;
        memcpy(m,data,num);

        addr+=num;
        bytes-=num;
        data+=num;
    }
}

