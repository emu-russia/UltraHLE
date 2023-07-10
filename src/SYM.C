// NOTE: when freeing symbols memory is lost, should fix at some point :)

#include "ultra.h"

//#define SHOWCRC

#define MAXOSCALL 256

#define SYMSIZE 1024

#define ADDRMASK 0x3fffffff

//#define NOPATCHES

//#define NOSPPATCH

#include "osignore.h"
#include "ospatch.h"

dword oscall[256][16]={
#include "oscall.h"
{0}};

// fields:
// 0-7=first 8 dwords (0=do not care, only check upper 16 bits)
//   8=crc-routine
//   9=crc-calls
//  10=(temp flag)
//  11=(symbol)
//  12=found at
//  13=found class: 0=not, 1=crc1ok, 2=crc1&2ok
//  14=patch number
//  15=symbol name ptr

typedef struct
{
    dword  addr;
    char  *text;
    int    patch;
    dword  original;
} Sym;

Sym   sym[SYMSIZE];
int   symnum;

static int cmp(Sym *a,Sym *b)
{
    return(b->addr-a->addr);
}

void sortsymbols(void)
{
//    qsort(sym,symnum,sizeof(Sym),cmp);
}

int findsym(int addr)
{
    int i,nearesti=0;
    dword nearest=0;
    for(i=0;i<symnum;i++)
    {
        if(addr==sym[i].addr)
        {
            return(i);
        }
        if(sym[i].addr>nearest && sym[i].addr<addr)
        {
            nearest=sym[i].addr;
            nearesti=i;
        }
    }
    return(nearesti);
}

void sym_clear(void)
{
    int i;

    // who cares about memory loss :)
    symnum=0;

    // default null symbol
    sym_add(0,"(null)",0);

    // clear ospatches
    for(i=0;oscall[i][15];i++)
    {
        oscall[i][12]=0;
        oscall[i][13]=0;
    }
}

int sym_add(int addr,char *text0,int patch)
{
    Sym  *s;
    char *text;
    int   i;

    addr&=ADDRMASK;

    // check disables
    if(st.memiosp)
    {
        // don't patch osSP routines in this mode
        if(!memcmp(text0,"osSp",4) ||
           !memcmp(text0,"__osSp",6))
        {
            patch=0;
        }
    }
    #ifdef NOPATCHES
    patch=0;
    #endif

    for(i=0;disablepatches[i];i++)
    {
        if(!memicmp(disablepatches[i],text0,strlen(disablepatches[i])))
        {
            patch=0;
            break;
        }
    }

    // dup text
    text=malloc(strlen(text0)+1);
    strcpy(text,text0);

    // remove # if no patch
    if(!patch)
    {
        for(i=strlen(text);i>=0;i--) if(text[i]=='#')
        {
            text[i+0]='%';
            text[i+1]='D';
            text[i+2]=0;
            break;
        }
    }

    // find
    s=sym+findsym(addr);
    if(s->addr!=addr)
    { // create new
        s=sym+symnum++;
        if(symnum>=SYMSIZE-16)
        {
            print("LOW ON SYMBOL SPACE!\n");
        }
        if(symnum>=SYMSIZE-1)
        {
            print("OUT OF SYMBOLS");
            exit(3);
        }
    }
    // set fields
    if(s->text) free(s->text);
    s->addr=addr;
    s->text=text;
    s->patch=patch;

    sortsymbols();

    return(s-sym);
}

void sym_del(int addr)
{
    Sym  *s;

    // find
    s=sym+findsym(addr);
    if(s->addr==addr)
    {
        memmove(s,s+1,SYMSIZE-1-(s-sym));
        symnum--;
    }
}

char *sym_find(int addr)
{
    static char buf[256];
    Sym  *s;
    int   offset;

    addr&=ADDRMASK;

    // find
    s=sym+findsym(addr);
    offset=addr-s->addr;

    if(offset<0 || offset>99999)
    {
        int a;
        a=mem_read32(addr);
        if(OP_OP(a)==19)
        {
            sprintf(buf,"?<patch:%i>",OP_IMM(a));
            return(buf);
        }
        else return("?");
    }

    if(offset==0) strcpy(buf,s->text);
    else sprintf(buf,"?%i+%s",offset,s->text);
    return(buf);
}

void sym_dump(void)
{
    int i;
    for(i=0;i<symnum;i++)
    {
        print("sym %i: %08X %s (%i)\n",i,sym[i].addr,sym[i].text,sym[i].patch);
    }
    print("total %i symbols.\n",symnum);
}

void sym_patchnames(void)
{
    int i,j;
    char *p;

    for(j=0;ospatch[j];j++)
    {
        p=strchr(ospatch[j],' ');
        if(!p)
        {
            print("ERROR in ospatch.h (%s)\n",ospatch[j]);
            return;
        }
        else
        {
            if(p[-1]=='*')
            {
                ospatchwild[j]=1;
                ospatchlen[j]=p-ospatch[j]-1;
            }
            else
            {
                ospatchwild[j]=0;
                ospatchlen[j]=p-ospatch[j];
            }
        }
    }

    for(i=0;i<symnum;i++)
    {
        p=strchr(sym[i].text,'%');
        if(p) continue; // patch disabled
        p=strchr(sym[i].text,'#');
        if(!p)
        { // no patch yet, add one from ospatch[] if applicable
            for(j=0;ospatch[j];j++)
            {
                if(strlen(sym[i].text)<ospatchlen[j]) continue;
                if(memicmp(sym[i].text,ospatch[j],ospatchlen[j])) continue;
                if(!ospatchwild[j] && sym[i].text[ospatchlen[j]]>32) continue;
                // match
                {
                    char buf[256];
//                    print("- symbol patch %s [%s,%i] => ",sym[i].text,ospatch[j],ospatchlen[j]);
                    sprintf(buf,"%s %s",sym[i].text,ospatch[j]+ospatchlen[j]+1);
                    p=strchr(ospatch[j],'#');
                    if(!p) p="0";
                    sym_add(sym[i].addr,buf,atoi(p+1));
//                    print("%s\n",sym[i].text);
                }
                break;
            }
        }
    }
}

void sym_load(char *file)
{
    FILE *f1;
    int findoscalls=1;

    sym_clear();

    cart.simplegfx=1; // default to 1 unless cart named

    f1=fopen(file,"rt");
    if(!f1)
    {
        *cart.symname=0;
    }
    else
    {
        while(!feof(f1))
        {
            char  buf[256];
            dword addr;
            char *text=buf+9;
            char *patch=text;
            int   patchcode;

            *buf=0;
            fgets(buf,255,f1);
            buf[strlen(buf)-1]=0;

            while(*patch && *patch!='#') patch++;
            if(*patch) patchcode=atoi(patch+1);
            else patchcode=0;

            addr=0;
            sscanf(buf,"%X",&addr);
            addr&=ADDRMASK;

            if(addr>0 && buf)
            {
                sym_add(addr,text,patchcode);
            }
        }
        fclose(f1);
    }

    if(!*cart.symname) print("No symbols. ");
    else print("Loaded %i symbols from %s. ",symnum,file);

    findoscalls=1;
    cart.simplegfx=0;
    if(!memcmp(cart.title,"SUPER MARIO",11)) cart.ismario=1;
    if(!memcmp(cart.title,"THE LEGEND ",11)) cart.iszelda=1;
    print("Mode %i%i. ",cart.ismario,cart.iszelda);

    if(cart.simplegfx)
    {
        showwire=0;
        showinfo=1;
        printf("debuggfxmode ");
    }
    print("\n");
}

void sym_findfirstos(void)
{
    if(cart.osrangestart && cart.osrangeend)
    {
        sym_findoscalls(cart.osrangestart,cart.osrangeend-cart.osrangestart,0);
    }
    else
    {
        sym_findoscalls(cart.codebase,cart.codesize,0);
    }

    sortsymbols();

    sym_patchnames();
}

void sym_save(char *file)
{
}

void routinecrc2(dword addr,int barrier,dword *xcrc1,dword *xcrc2)
{
    dword crc,crc1=0,crc2=0;
    dword x1,x2;
    int i,in,errorsaid=0;
    int dump=0;

//    if(addr>=0x002004b0 && addr<=0x002004ff) dump=1;

    if(!barrier) crc1=*xcrc1;

    in=16;

    x1=mem_read32(addr);
    if(!(x1&0xffffff))
    {
        *xcrc1=-1;
        *xcrc2=-1;
        return;
    }

    for(i=0;i<in;i++)
    {
        x1=mem_read32(addr+i*4);
        if(x1==0x03e00008)
        { // JR ret
            in=i+2;
            if(in>16) in=16;
        }
    }

    for(i=0;i<in;i++)
    {
        x1=mem_read32(addr+i*4);
        crc=i;
        if(OP_OP(x1)==3 || OP_OP(x1)==2)
        { // JAL or J
            crc+=OP_OP(x1);
        }
        else if(OP_OP(x1)==15)
        { // LUI
            if(OP_IMM(x1)>=0xa400 && OP_IMM(x1)<=0xafff) crc2+=x1; // must be right
            else crc+=OP_OP(x1);
        }
        else if(OP_OP(x1)==16)
        { // COP0
            crc2^=x1; // these must be totally correct
            crc=0;
        }
        else if(OP_OP(x1)==17)
        { // COP1
            crc2^=x1; // these must be totally correct
            crc=0;
        }
        else
        { // default instr, just check upper 16 bits
            if(barrier)
            { // generating
                x2=mem_read32(addr+i*4+barrier);
                if(OP_OP(x1)>=4 && OP_OP(x1)<=15)
                { // immediates
                    crc1|=(1<<i);
                    crc^=x1>>16;
                }
                else if(x1!=x2)
                {
                    if((x1^x2)>>16)
                    {
                        if(!errorsaid)
                        {
                            //print("32-bit routine difference at %08X\n",addr+i*4);
                            errorsaid=1;
                            crc1|=-1;
                        }
                    }
                    crc1|=(1<<i);
                    crc^=x1>>16;
                }
                else
                {
                    crc^=x1;
                }
            }
            else
            { // testing
                if(crc1&(1<<i))
                {
                    crc^=x1>>16;
                }
                else
                {
                    crc^=x1;
                }
            }
        }

        x1=crc;
        if(in<4)
        {
            x1^=(x1>>16);
            x1^=(x1>>8);
            x1=(x1&255)<<(i*8);
        }
        else if(in<8)
        {
            x1^=(x1>>16);
            x1^=(x1>>8);
            x1^=(x1>>4);
            x1=(x1&15)<<(i*4);
        }
        else
        {
            x1^=(x1>>16);
            x1^=(x1>>8);
            x1^=(x1>>4);
            x1^=(x1>>2);
            x1=(x1&3)<<(i*2);
        }
        crc2^=x1;

        if(dump)
        {
            print("* %08X crc %08X -> %08X crc2 %08X\n",addr+i*4,crc,x1,crc2);
        }
    }
    if(barrier) *xcrc1=crc1;
    *xcrc2=crc2;
}

void sym_demooscalls(void) // for demo.rom
{
    int i,j,k,l,n,jm;
    dword x,y,a;
    char *p;
    FILE *f1;
    dword base1;
    dword base2;

    sym_removepatches();

    // find barrier
    for(i=0x200000;i<=0x300000;i+=4)
    {
        if(mem_read32(i)==0x3c3c3c3c && mem_read32(i+4)==0x2d2d2d2d)
        {
            break;
        }
    }
    i+=16;
    for(;i<=0x300000;i+=4)
    {
        if(mem_read32(i)!=0) break;
    }

    base1=0x200000;
    base2=i;
    if(base2>=0x300000)
    {
        base2=base1+7;
    }

    print("Scanning os calls: base1=%08X base2=%08X\n",base1,base2);

    print("Generating OSCALL.lst\n");
    f1=fopen("oscall.lst","wt");
    for(i=0;i<symnum;i++)
    {
//        if(sym[i].text[0]=='o' && sym[i].text[1]=='s' && sym[i].text[2]>='A')
        if(sym[i].addr<0x20e000)
        {
            fprintf(f1,"%s:\n",sym[i].text);
            for(j=0;j<1024;j++)
            {
                a=sym[i].addr+j*4;
                fprintf(f1,"%08X: ",a);

                x=mem_read32(a);
                y=mem_read32(a+base2-base1);
                p=disasm(a,x);
                fprintf(f1,"<%08X><%08X> %s ",x,y,p);

                fprintf(f1,"\n");

                x=mem_read32(a-4);
                if(x==0x03E00008) break;
            }
            fprintf(f1,"\n");
        }
    }
    fclose(f1);

    print("Calculating routine crcs\n");
    memset(oscall,0,MAXOSCALL*16*4);
    n=0;
    for(i=0;i<symnum;i++)
    {
//        if(sym[i].text[0]=='o' && sym[i].text[1]=='s' && sym[i].text[2]>='A')
        if(sym[i].addr<0x20e000)
        {
            jm=8;
            for(j=0;j<jm;j++)
            {
                x=mem_read32(sym[i].addr+j*4);
                if(x==0x03e00008)
                {
                    jm=j+2;
                    if(jm>8) jm=8;
                }
                if(OP_OP(x)==3 || OP_OP(x)==2) x=0;
                fprintf(f1,"0x%08X,",x);
                oscall[n][j]=x;
            }
            for(;j<8;j++)
            {
                oscall[n][j]=0;
            }

            routinecrc2(sym[i].addr,base2-base1,&x,&y);

            oscall[n][8]=x;
            oscall[n][9]=y;
            oscall[n][10]=0;
            oscall[n][11]=0;
            oscall[n][12]=0;
            oscall[n][13]=1;
            oscall[n][14]=sym[i].patch;
            oscall[n][15]=(dword)sym[i].text;

            if(x==-1)
            {
                print("ossym %3i: %08X %-32s CRC-error, data? (skipped)\n",i,sym[i].addr,sym[i].text,x,y);
            }
            else
            {
                n++;
                print("ossym %3i: %08X %-32s crc:%08X%08X patch:%i\n",i,sym[i].addr,sym[i].text,x,y,sym[i].patch);
            }
        }
    }

    print("Generating OSCALL.H (%i entries).\n",n);
    f1=fopen("oscall.h","wt");
//    fprintf(f1,"dword oscall[%i][16]={\n",MAXOSCALL);
    for(i=0;i<n;i++)
    {
        if(strstr((char *)oscall[i][15],"__ll") || strstr((char *)oscall[i][15],"__ull")) k=4;
        else k=1;
        for(l=0;l<k;l++)
        {
            fprintf(f1,"{");
            for(j=0;j<10;j++)
            {
                fprintf(f1,"0x%08X,",oscall[i][j]);
            }
            for(;j<14;j++)
            {
                fprintf(f1,"%i,",oscall[i][j]);
            }
            fprintf(f1,"%3i,(dword)\"",oscall[i][14]);
            if(l) fprintf(f1,"A");
            fprintf(f1,"%s\"},\n",oscall[i][15]);
        }
    }
//    fprintf(f1,"{0}};\n");
    fclose(f1);

    print("Checking results.\n");
    sym_findoscalls(cart.codebase,base2-base1,0);
}

int restorepatch(Sym *sym)
{
    int address,patch;
    dword old;

    address=sym->addr;
    patch=sym->patch;

    if(address&3) print("Illegal patch address %08X\n",address);

    old=mem_read32(address);
    if(OP_OP(old)==OP_PATCH)
    {
        mem_write32(address,sym->original);
        return(1);
    }
    return(0);
}

void putpatch(Sym *sym)
{
    int address,patch;
    int x;
    dword old;

    address=sym->addr;
    patch=sym->patch;

    if(address&3) print("Illegal patch address %08X\n",address);

    old=mem_read32(address);
    x=PATCH(patch);

    if(OP_OP(old)!=OP_PATCH) sym->original=old;

    //print("Patch %08X old %08X new %08X\n",address,old,x);

    mem_write32(address,x);
}

void sym_addpatches(void)
{
    int i;
    st.patches=0;
    st.firstpatch=0xffffffff;
    st.lastpatch=0;
    for(i=0;i<symnum;i++)
    {
        if(sym[i].patch)
        {
            st.patches++;
            putpatch(sym+i);
            if(sym[i].addr<st.firstpatch) st.firstpatch=sym[i].addr;
            if(sym[i].addr>st.lastpatch)  st.lastpatch=sym[i].addr;
        }
    }
    if(st.firstpatch==0xffffffff) st.firstpatch=0;
    print("Adding memory patches, range %08X..%08X\n",st.firstpatch,st.lastpatch);
}

void sym_removepatches(void)
{
    int i,cnt=0;
    for(i=0;i<symnum;i++)
    {
        if(sym[i].patch)
        {
            cnt+=restorepatch(sym+i);
        }
    }
    if(st.firstpatch==0xffffffff) st.firstpatch=0;
    print("Removed %i memory patches\n",cnt);
}

void sym_dumposcalls(void)
{
    int cnt,j,l,total;

    for(j=0;oscall[j][15];j++)
    {
    }
    total=j;

    cnt=0;
    for(j=0;j<total;j++)
    {
        if(oscall[j][12])
        {
            print("- %-32s found at %08X (patch %2i) class %i\n",
                sym[oscall[j][11]].text,
                oscall[j][12],oscall[j][14],oscall[j][13]);
            cnt++;
            if(cart.isdocalls)
            { // check against mario sym file
                for(l=0;l<symnum;l++)
                {
                    if(sym[l].addr==oscall[j][12] && sym[l].patch!=oscall[j][14])
                    {
                        print("INVALID PATCH should be %i\n",sym[l].patch);
                    }
                }
            }
        }
    }
    for(j=0;j<total;j++)
    {
        if(!oscall[j][12])
        {
            if(!memcmp((char *)oscall[j][15],"A__",3)) continue;
            if(oscall[j][11])
            {
                print("- %-32s not found (maybe at %08X)\n",
                    oscall[j][15],oscall[j][11]);
            }
            else
            {
                print("- %-32s not found\n",
                    oscall[j][15]);
            }
        }
    }
}

void sym_findoscalls(dword base,dword bytes,int cont)
{
    int i,j,k,cl,end,x0,x,y,found,class;
    int cnt=0,total=0,cnt2=0,total2=0;

    base|=0x80000000;
    end=base+bytes;

    print("Searching for os-calls in range %08X..%08X\n",
        base,base+bytes-1);
    flushdisplay();
    if(bytes<=0) return;

    for(j=0;oscall[j][15];j++)
    {
        oscall[j][10]=0;
        if(!cont)
        {
            oscall[j][11]=0;
            oscall[j][12]=0;
            oscall[j][13]=0;
        }
        else
        {
            if(oscall[j][13]) oscall[j][13]+=100;
        }
    }
    total=j;

    for(i=end-4;i>=base;i-=4)
    {
        x0=mem_read32(i);
        if(!(i&(256*1024-1)) && bytes>1024*1024)
        {
            print("at %08X\n",i);
            flushdisplay();
        }
        if(!(x0&0xffffff)) continue;
        cnt=0;
        for(j=0;j<total;j++)
        {
            if( ((x0^oscall[j][0])>>16) && oscall[j][0] ) continue;

            if(oscall[j][12]) continue; // routine already found

            for(k=1;k<8;k++)
            {
                x=mem_read32(i+k*4);
                y=oscall[j][k];
                if( ((x^y)>>16) && y) break;
            }
            if(k!=8) continue;

#ifdef SHOWCRC
            print("# maybe %08X is %s\n",i,oscall[j][15]);
#endif
            if(!oscall[j][11]) oscall[j][11]=i; // maybestore
            oscall[j][10]=1;
            cnt++;
        }
        if(cnt>0)
        { // some matches found
            // check we don't already have a symbol for this
            if(*sym_find(i)!='?') continue;
            // complete match?
            found=class=0;
            cnt=0;
            for(j=0;j<total;j++)
            {
                if(oscall[j][10])
                {
                    x=oscall[j][8];
                    routinecrc2(i,0,&x,&y);

                    cl=0;
                    if(oscall[j][9]==y) cl=16;
                    else
                    {
                        y^=oscall[j][9];
                        for(k=0;k<32;k+=2)
                        {
                            if(!(y&(3<<k))) cl++;
                        }
                    }

                    if(cl>class)
                    {
                        class=cl;
#ifdef SHOWCRC
                        print("# crc %08X class %i, OK for %s\n",y,class,oscall[j][15]);
#endif
                        if(class==16) cnt++;
                        found=j;
                        oscall[j][10]=2;
                    }
                    else
                    {
#ifdef SHOWCRC
                        print("# crc %08X class %i, FAILED for %s\n",y,cl,oscall[j][15]);
#endif
                    }
                }
            }
            // so did we find it?
            if(class>8)
            {
                if(cnt>1)
                { // find first matching routine not yet mapped
                    print("- multiple at %08X: ",i);
                    for(j=0;j<total;j++)
                    {
                        if(oscall[j][10]==2)
                        {
                            print("%s ",oscall[j][15]);
                        }
                    }
                    print("\n");
                    for(j=0;j<total;j++)
                    {
                        if(oscall[j][10]==2 && !oscall[j][13])
                        {
                            break;
                        }
                    }
                }
                else j=found;
                if(j<total)
                {
                    oscall[j][13]=class;
                    oscall[j][12]=i;
                    oscall[j][11]=sym_add(i,(char *)oscall[j][15],oscall[j][14]);
                }
                else
                {
                    print("- extra %-32s at %08X ignored\n",oscall[j][15],oscall[j][14]);
                }
            }
            // clear
            for(j=0;j<total;j++) oscall[j][10]=0;
        }
    }

    sym_patchnames();

    // remove maybes that were assigned with certainty
    cnt=0;
    for(i=0;i<total;i++)
    {
        if(oscall[i][14]>=10) total2++;
        if(oscall[i][12])
        {
            // patch found
            if(oscall[i][14]>=10) cnt2++;
            cnt++;
            continue;
        }
        if(!oscall[i][11]) continue;

        for(j=0;j<total;j++)
        {
            if(oscall[i][11]==oscall[j][12])
            {
                oscall[i][11]=0;
            }
        }
    }

    print("OS-routine search: %i/%i found, "YELLOW"%i/%i"GREEN" important.",
        cnt,total,cnt2,total2);
    if(cont) print("(continued)");
    print("\n");
    flushdisplay();
//    sym_dump();
}

