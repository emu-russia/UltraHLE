
#include "ultra.h"

#define FIRSTTAB  10

static int rsp=0;

// Format codes:
// ----------
// %ff = fpu format (word,dword,single,double)
// %fs = fpu source1 (OP_RD)
// %ft = fpu source2 (OP_RT)
// %fd = fpu dest    (OP_SHAMT)
// ----------
// %ve = vec element
// %vs = vec source1 (OP_RD)
// %vt = vec source2 (OP_RT)
// %vd = vec dest    (OP_SHAMT)
// ----------
// %rs = cpu source1 (OP_RS)
// %rt = cpu source2 (OP_RT)
// %rd = cpu dest    (OP_RD)
// ----------
// %cs = cop source1 (OP_RS)
// %ct = cop source2 (OP_RT)
// %cd = cop dest    (OP_RD)
// %cc = cop control (OP_RD)
// ----------
// %im = immediate
// %ir = immediate relative jump
// %ia = immediate absolute jump
// ----------
// %sh = shamt
// %s3 = shamt + 32
// %sc = cache code
// ----------
// %on = cop number
// %oo = cop hex opcode
// ----------specials (handled in main loop)
// %!n = detect nop
// %!m = detect move/clear
// %!b = detect unconditional branch

char *regnames[33]={
"r0","at",
"v0","v1",
"a0","a1","a2","a3",
"t0","t1","t2","t3","t4","t5","t6","t7",
"s0","s1","s2","s3","s4","s5","s6","s7",
"t8","t9","k0","k1","gp","sp","s8","ra",
NULL};

char *mmuregnames[33]={
"$Index",
"$Random",
"$EntryLo0",
"$EntryLo1",
"$Context",
"$PageMask",
"$Wired",
"$R7",
"$BadVAddr",
"$Count",
"$EntryHi",
"$Compare",
"$Status",
"$Cause",
"$EPC",
"$PrID",
"$Config",
"$LLAddr",
"$WatchLo",
"$WatchHi",
"$XContext",
"$R21",
"$R22",
"$R23",
"$R24",
"$R25",
"$ECC",
"$CacheErr",
"$TagLo",
"$TagHi",
"$ErrorEPC",
"$R31",
NULL};

static char *op_main[65]={
// 0
".special",
".regimm",
"j %ia",
"jal %ia",
"beq %rs==%rt -->%ir%!b",
"bne %rs!=%rt -->%ir",
"blez %rs<=0 -->%ir",
"bgtz %rs>0 -->%ir",
// 1
"addi %rt = %rs+%im",
"addiu %rt = %rs+%im ",
"slti %rt = (%rs<%im)",
"sltiu %rt = (%rs<%im)",
"andi %rt = %rs & %im",
"ori %rt = %rs | %im",
"xori %rt = %rs ^ %im",
"lui %rt = %im0000",
// 2
".cop0",
".cop1",
".cop2",
"*",
"beql %rs==%rt -->%ir",
"bnel %rs!=%rt  -->%ir",
"blezl %rs<=0 -->%ir",
"bgtzl %rs>0 -->%ir",
// 3
"Daddi %rt = %rs+%im",
"Daddiu %rt = %rs+%im",
"ldl %rt <- [%rs+%im]",
"ldr %rt <- [%rs+%im]",
"<patch:%im>",
"<group:%im>",
"*",
"*",
// 4
"lb %rt <- [%rs+%im]",
"lh %rt <- [%rs+%im]",
"lwl %rt <- [%rs+%im]",
"lw %rt <- [%rs+%im]",
"lbu %rt <- [%rs+%im]",
"lhu %rt <- [%rs+%im]",
"lwr %rt <- [%rs+%im]",
"lwu %rt <- [%rs+%im]",
// 5
"sb %rt -> [%rs+%im]",
"sh %rt -> [%rs+%im]",
"swl %rt -> [%rs+%im]",
"sw %rt -> [%rs+%im]",
"sdl %rt -> [%rs+%im]",
"sdr %rt -> [%rs+%im]",
"swr %rt -> [%rs+%im]",
"cache %cc [%rs+%im]",
// 6
"ll %rt <- [%rs+%im]",
"lwc1 %ct <- [%rs+%im]",
"vld %ct[%vE] <- [%rs+%iM]",
"*",
"lld %rt <- [%rs+%im]",
"ldc1 %ct <- [%rs+%im]",
"ldc2 %ct <- [%rs+%im]",
"ld %rt <- [%rs+%im]",
// 7
"sc %rt -> [%rs+%im]",
"swc1 %ct -> [%rs+%im]",
"vst %ct[%vE] -> [%rs+%iM]",
"*",
"scd %rt -> [%rs+%im]",
"sdc1 %ct -> [%rs+%im]",
"sdc2 %ct -> [%rs+%im]",
"sd %rt -> [%rs+%im]",
""};

static char *op_special[65]={
// 0
"sll %rd = %rt<<%sh%!n",
"*",
"srl %rd = %rt>>%sh",
"sra %rd = %rt>>%sh",
"sllv %rd = %rt<<%rs",
"*",
"srlv %rd = %rt>>%rs",
"srav %rd = %rt>>%rs",
// 1
"jr %rs",
"jalr %rs",
"*",
"*",
"syscall",
"break",
"*patch*",
"sync",
// 2
"mfhi %rd",
"mthi %rs",
"mflo %rd",
"mtlo %rs",
"Dsllv %rd = %rt<<%rs",
"*",
"Dsrlv %rd = %rt>>%rs",
"Dsrav %rd = %rt>>%rs",
// 3
"mult %rs * %rt",
"multu %rs * %rt",
"div %rs / %rt",
"divu %rs / %rt",
"Dmult %rs * %rt",
"Dmultu %rs * %rt",
"Ddiv %rs / %rt",
"Ddivu %rs / %rt",
// 4
"add %rd = %rs+%rt",
"addu %rd = %rs+%rt",
"sub %rd = %rs-%rt",
"subu %rd = %rs-%rt",
"and %rd = %rs & %rt",
"or %rd = %rs | %rt%!m",
"xor %rd = %rs ^ %rt",
"nor %rd = %rs !| %rt",
// 5
"*",
"*",
"slt %rd = (%rs<%rt)",
"sltu %rd = (%rs<%rt)",
"Dadd %rd = %rs+%rt",
"Daddu %rd = %rs+%rt",
"Dsub %rd = %rs-%rt",
"Dsubu %rd = %rs-%rt",
// 6
"tge %rs> = %rt",
"tgeu %rs> = %rt",
"tlt %rs<%rt",
"tlt %rs<%rt",
"teq %rs==%rt",
"*",
"tne %rs!=%rt",
"*",
// 7
"Dsll %rd = %rt<<%sh",
"*",
"Dsrl %rd = %rt>>%sh",
"Dsra %rd = %rt>>%sh",
"Dsll32 %rd = %rt<<%s3",
"*",
"Dsrl32 %rd = %rt>>%s3",
"Dsra32 %rd = %rt>>%s3",
""};

char *op_regimm[33]={
// 0
"bltz %rs<0 -->%ir",
"bgez %rs>=0 -->%ir",
"bltzl %rs<0 -->%ir",
"bgezl %rs>=0 -->%ir",
"*",
"*",
"*",
"*",
// 1
"tgei %rs> = %im",
"tgeiu %rs> = %im",
"tlti %rs<%im",
"tltiu %rs<%im",
"teqi %rs==%im",
"*",
"tnei %rs!=%im",
"*",
// 2
"bltzal %rs -->%ir",
"bgezal %rs -->%ir",
"bltzall %rs -->%ir",
"bgezall %rs -->%ir",
"*",
"*",
"*",
"*",
// 3
"*",
"*",
"*",
"*",
"*",
"*",
"*",
"*",
""};

static char *op_coprs[33]={
// 0
"mfc%on %rt <- %fs [%vE]",
"Dmfc%on %rt <- %fs",
"cfc%on %rt <- %cc",
"*cop3",
"mtc%on %rt -> %fs [%vE]",
"Dmtc%on %rt -> %fs",
"ctc%on %rt -> %cc",
"*cop7",
// 1
".bc",
"*cop9",
"*copA",
"*copB",
"*copC",
"*copD",
"*copE",
"*copF",
// 2
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
// 3
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
".cop%on %oo",
""};

static char *op_coprt[33]={
// 0
"bc%onf -->%ir",
"bc%ont -->%ir",
"bc%onfl -->%ir",
"bc%ontl -->%ir",
"*cop04",
"*cop05",
"*cop06",
"*cop07",
// 1
"*cop08",
"*cop09",
"*cop0A",
"*cop0B",
"*cop0C",
"*cop0D",
"*cop0E",
"*cop0F",
// 2
"*cop10",
"*cop11",
"*cop12",
"*cop13",
"*cop14",
"*cop15",
"*cop16",
"*cop17",
// 3
"*cop18",
"*cop19",
"*cop1A",
"*cop1B",
"*cop1C",
"*cop1D",
"*cop1E",
"*cop1F",
""};

static char *op_fpufmt[33]={ // fmt<-RS
"*","*","*","*","*","*","*","*",
"*","*","*","*","*","*","*","*",
".s",".d","*","*",".w",".l","*","*",
"*","*","*","*","*","*","*","*",
""};

// fd=%fd
// fs=%fs
// ft=%ft
static char *op_fpu[65]={ // <-FUNC
// 0
"fadd%ff %fd = %fs+%ft",
"fsub%ff %fd = %fs-%ft",
"fmul%ff %fd = %fs*%ft",
"fdiv%ff %fd = %fs/%ft",
"fsqrt%ff %fd = sqrt(%fs)",
"fabs%ff %fd = abs(%fs)",
"fmov%ff %fd = %fs",
"fneg%ff %fd = -%fs",
// 1
"froun.l%ff %fd = rnd(%fs)",
"ftrun.l%ff %fd = rnd(%fs)",
"fceil.l%ff %fd = rnd(%fs)",
"ffloo.l%ff %fd = rnd(%fs)",
"froun.w%ff %fd = rnd(%fs)",
"ftrun.w%ff %fd = rnd(%fs)",
"fceil.w%ff %fd = rnd(%fs)",
"ffloo.w%ff %fd = rnd(%fs)",
// 2
"*","*","*","*","*","*","*","*",
// 3
"*","*","*","*","*","*","*","*",
// 4
"cvt.s%ff %fd = (s)%fs",
"cvt.d%ff %fd = (d)%fs",
"*",
"*",
"cvt.w%ff %fd = (w)%fs",
"cvt.l%ff %fd = (l)%fs",
"*",
"*",
// 5
"*","*","*","*","*","*","*","*",
// 6
"c.f%ff %fs ? %ft",
"c.un%ff %fs ? %ft",
"c.eq%ff %fs ? %ft",
"c.ueq%ff %fs ? %ft",
"c.olt%ff %fs ? %ft",
"c.ult%ff %fs ? %ft",
"c.ole%ff %fs ? %ft",
"c.ule%ff %fs ? %ft",
// 7
"c.sf%ff %fs ? %ft",
"c.ngel%ff %fs ? %ft",
"c.seq%ff %fs ? %ft",
"c.ngl%ff %fs ? %ft",
"c.lt%ff %fs ? %ft",
"c.nge%ff %fs ? %ft",
"c.le%ff %fs ? %ft",
"c.ngt%ff %fs ? %ft",
""};

static char *op_rpc[65]={ // <-FUNC
// 0
"vmulf %vd  = %vs * %vt[%ve]",
"vmulu %vd  = %vs * %vt[%ve]",
"vrndp %vd  = %vs ? %vt[%ve] (dct-positive)",
"vmulq %vd  = %vs * %vt[%ve]",
"vmudl %vd  = ( acc = %vs * %vt[%ve]     )",
"vmudm %vd  = ( acc = %vs * %vt[%ve] >>16)",
"vmudn %vd  = ( acc = %vs * %vt[%ve]     )>>16",
"vmudh %vd  = ( acc = %vs * %vt[%ve] >>16)>>16",
// 1
"vmacf %vd += %vs * %vt[%ve]",
"vmacu %vd += %vs * %vt[%ve]",
"vrndn %vd  = %vs ? %vt[%ve] (dct-negative)",
"vmacq %vd += %vs * %vt[%ve]",
"vmadl %vd  = ( acc+= %vs * %vt[%ve]     )",
"vmadm %vd  = ( acc+= %vs * %vt[%ve] >>16)",
"vmadn %vd  = ( acc+= %vs * %vt[%ve]     )>>16",
"vmadh %vd  = ( acc+= %vs * %vt[%ve] >>16)>>16",
// 2
"vadd %vd  = %vs + %vt[%ve]",
"vsub %vd  = %vs - %vt[%ve]",
"vsut %vd  = %vs -? %vt[%ve]",
"vabs %vd  = %vs ? %vt[%ve]",
"vaddc %vd  = %vs + %vt[%ve] + C",
"vsubc %vd  = %vs - %vt[%ve] + C",
"vaddb %vd  = %vs + %vt[%ve] (byte)",
"vsubb %vd  = %vs - %vt[%ve] (byte)",
// 3
"vaccb %vd  = %vs +? %vt[%ve]",
"vsucb %vd  = %vs -? %vt[%ve]",
"vsad %vd  = %vs ? %vt[%ve]",
"vsac %vd  = %vs ? %vt[%ve]",
"vsum %vd  = %vs ? %vt[%ve]",
"vsaw %vd  = %vs ? %vt[%ve]",
"*36",
"*37",
// 4
"vlt %vd  = %vs <  %vt[%ve]",
"veq %vd  = %vs == %vt[%ve]",
"vne %vd  = %vs != %vt[%ve]",
"vge %vd  = %vs >= %vt[%ve]",
"vcl %vd  = %vs ? %vt[%ve]",
"vch %vd  = %vs ? %vt[%ve]",
"vcr %vd  = %vs ? %vt[%ve]",
"vmgr %vd  = %vs ? %vt[%ve]",
// 5
"vand %vd  = %vs and %vt[%ve]",
"vnand %vd  = %vs nand %vt[%ve]",
"vor %vd  = %vs or %vt[%ve]",
"vnor %vd  = %vs nor %vt[%ve]",
"vxor %vd  = %vs xor %vt[%ve]",
"vnxor %vd  = %vs nxor %vt[%ve]",
"*56",
"*57",
// 6
"*60","*61","*62","*63","*64","*65","*66","*67",
// 7
"vextt %vd  = %vs ? %vt[%ve] 5551",
"vextq %vd  = %vs ? %vt[%ve] 4444",
"vextn %vd  = %vs ? %vt[%ve] 4444se",
"vinst %vd  = %vs ? %vt[%ve] 5551",
"vinsq %vd  = %vs ? %vt[%ve] 4444",
"vinsn %vd  = %vs ? %vt[%ve] 4444se",
"*76",
"*77",
""};

char *copreg(dword x,int reg)
{
    int cop=OP_OP(x)&3;
    static char buf[32];
    if(reg>=32)
    {
        sprintf(buf,"Ctrl%02i",reg-32);
        return(buf);
    }
    if(rsp)
    {
        if(cop==0)
        {
            sprintf(buf,"DMA%02i",reg);
            return(buf);
        }
        else if(cop==2)
        {
            sprintf(buf,"vec%02i",reg);
            return(buf);
        }
        else
        {
            sprintf(buf,"GR%02i",reg);
            return(buf);
        }
    }
    else
    {
        if(cop==0)
        {
            return(mmuregnames[reg]);
        }
        else if(cop==1)
        {
            sprintf(buf,"FP%02i",reg);
            return(buf);
        }
        else
        {
            sprintf(buf,"GR%02i",reg);
            return(buf);
        }
    }
}

void format(char **d0,char *p,dword x,dword pos)
{
    int a=-1,b;
    char *d=*d0;
    static int vecsize; // set by %vE, read by %iM
    int vecelem;
    char *vecstr;

    *d=0;
    if(p[1]=='f' || p[1]=='v')
    { // fpu & vec
        if(p[2]=='f')
        { // format
            a=OP_RS(x);
            strcpy(d,op_fpufmt[a]);
        }
        else if(p[2]=='e' || p[2]=='E')
        {
            vecsize=1;
            if(p[2]=='e')
            {
                a=OP_RS(x);
                if((a&0x18)==0x18)
                {
                    vecstr=" ";
                    vecsize=2;
                    vecelem=a&7;
                }
                else if((a&0x1C)==0x14)
                {
                    vecstr="h";
                    vecsize=4;
                    vecelem=a&3;
                }
                else if((a&0x1E)==0x12)
                {
                    vecstr="q";
                    vecsize=8;
                    vecelem=a&1;
                }
                else if(a==0x10)
                {
                    vecstr="a";
                    vecsize=16;
                    vecelem=0;
                }
                else
                {
                    vecstr="?";
                    vecsize=16;
                    vecelem=0;
                }
                sprintf(d,"%i%s",
                    vecelem,vecstr);
                /*
                sprintf(d+strlen(d),"/%i%i%i%i%i",
                    vecelem,vecstr,
                    a&16?1:0,
                    a&8?1:0,
                    a&4?1:0,
                    a&2?1:0,
                    a&1?1:0);
                */
            }
            else
            {
                a=(OP_IMM(x)>>8);
                vecelem=a&7;
                b=a>>3;
                if(b>=4)
                {
                    vecstr="a";
                    vecsize=16;
                }
                else if(b==1)
                {
                    vecstr=" ";
                    vecsize=2;
                }
                else if(b==2 || b==3)
                {
                    vecstr="q";
                    vecsize=8;
                }
                else
                {
                    vecstr="?";
                    vecsize=1;
                }
                sprintf(d,"%i%s.%i",vecelem,vecstr,b);
                /*
                sprintf(d+strlen(d),"%i/%i%i%i%i%i",
                    a&128?1:0,
                    a&64?1:0,
                    a&32?1:0,
                    a&16?1:0,
                    a&8?1:0,
                    a&4?1:0,
                    a&2?1:0,
                    a&1?1:0);
                */
            }
        }
        else if(p[2]=='s')
        {
            sprintf(d,copreg(x,OP_RD(x)));
        }
        else if(p[2]=='t')
        {
            sprintf(d,copreg(x,OP_RT(x)));
        }
        else if(p[2]=='d')
        {
            sprintf(d,copreg(x,OP_SHAMT(x)));
        }
    }
    else if(p[1]=='c')
    { // cop
        if(p[2]=='s')
        {
            sprintf(d,copreg(x,OP_RS(x)));
        }
        else if(p[2]=='t')
        {
            sprintf(d,copreg(x,OP_RT(x)));
        }
        else if(p[2]=='d')
        {
            sprintf(d,copreg(x,OP_RD(x)));
        }
        else if(p[1]=='c')
        {
            sprintf(d,copreg(x,OP_RD(x)+32));
        }
    }
    else if(p[1]=='r')
    { // cpu
        if(p[2]=='s')
        { // %rs=OP_RS
            a=OP_RS(x);
        }
        else if(p[2]=='t')
        { // %rt=OP_RT
            a=OP_RT(x);
        }
        else if(p[2]=='d')
        { // %rd=OP_RD
            a=OP_RD(x);
        }
        strcpy(d,regnames[a]);
    }
    else if(p[1]=='i')
    { // %im=OP_IMM
        if(p[2]=='m')
        {
            a=OP_IMM(x);
            sprintf(d,"%04X",a);
        }
        else if(p[2]=='M')
        {
            a=OP_IMM(x)&63;
            sprintf(d,"%02X",a*vecsize);
        }
        else if(p[2]=='N')
        {
            a=OP_IMM(x)>>8;
            sprintf(d,"%02X",a);
        }
        else if(p[2]=='r')
        { // jump relative
            a=SIGNEXT16(OP_IMM(x))*4+pos+4;
            sprintf(d,"%08X",a);
        }
        else if(p[2]=='a')
        { // jump absolute also add symbol
            a=OP_TAR(x)*4 | ((pos+4)&0xf0000000);
            sprintf(d,"%08X ",a);
            strcat(d,sym_find(a));
        }
    }
    else if(p[1]=='s')
    { // %sh=OP_SHAMT , %s3=OP_SHAMT + 32
        a=OP_SHAMT(x);
        if(p[2]=='c')
        {
            static char *caches[]={"pr_inst","pr_data","sn_inst","sn_data"};
            a=OP_RT(x);
            sprintf(d,"%s op_%i",
                caches[a&3],
                a>>2);
        }
        else if(p[2]=='3')
        {
            sprintf(d,"%i",a+32);
        }
        else
        {
            sprintf(d,"%i",a);
        }
    }
    else if(p[1]=='o')
    { // %ox coprocessor
        if(p[2]=='n')
        { // %on
            sprintf(d,"%i",OP_OP(x)&3);
        }
        else if(p[2]=='o')
        { // %oo
            sprintf(d,"<%08X>",x&0x1ffffff);
        }
    }

    *d0=d;
}

char *disasmmain(dword pos,dword x)
{
    static char buf[256];
    char *p,*d;
    int cop,a;

    p=op_main[ OP_OP(x) ];
    if(p[0]=='.')
    {
        if(p[1]=='s')      p=op_special[ OP_FUNC(x) ];
        else if(p[1]=='r') p=op_regimm[ OP_RT(x) ];
        else if(p[1]=='c')
        {
            cop=OP_OP(x)&3;

            p=op_coprs[ OP_RS(x) ];
            if(p[0]=='.' && p[1]=='b')
            {
                p=op_coprt[ OP_RT(x) ];
            }
            else if(p[0]=='.' && p[1]=='c')
            { // cop (maybe fpu)
                if(cop==1 && !rsp)
                {
                    p=op_fpu[ OP_FUNC(x) ];
                }
                else if(cop==2 && rsp)
                {
                    p=op_rpc[ OP_FUNC(x) ];
                }
                else p++;
            }
        }
    }

    a=strlen(p)-3;
    if(p[a]=='%' && p[a+1]=='!')
    { // specials
        if(p[a+2]=='n')
        { // nop
            if(OP_SHAMT(x)==0)
            {
                p="nop";
            }
        }
        else if(p[a+2]=='m')
        { // move/clear
            if(OP_RS(x)==0 && OP_RT(x)==0)
            {
                p="clear %rd = 0";
            }
            else if(OP_RT(x)==0)
            {
                p="move %rd = %rs";
            }
        }
        else if(p[a+2]=='b')
        { // move/clear
            if(OP_RS(x)==0 && OP_RT(x)==0)
            {
                p="b --> %ir";
            }
        }
    }

    d=buf;
    while(*p)
    {
        if(*p==' ' && d<buf+FIRSTTAB)
        {
            while(d<buf+FIRSTTAB) *d++=' ';
        }
        else if(*p=='%')
        {
            format(&d,p,x,pos);
            d+=strlen(d);
            p+=3;
        }
        else *d++=*p++;
    }
    *d++=0;

    return(buf);
}

char *disasmrsp(dword pos,dword x)
{
    rsp=1;
    return(disasmmain(pos,x));
}

char *disasm(dword pos,dword x)
{
    rsp=0;
    return(disasmmain(pos,x));
}

void disasm_dumpucode(char *filename,dword addr,int size,dword dataaddr,int datasize,int rspoffset)
{
    int i;
    dword x;
    char *p;
    FILE *f1;
    f1=fopen(filename,"wb");
    if(!f1) return;
    if(!size)
    {
        size=4096;
        rspoffset=4096;
    }
    fprintf(f1,"\n");
    fprintf(f1,"RSP microcode at %08X (size %i):\n",addr,size);
    for(i=0;i<size;i+=4)
    {
        x=mem_read32(addr+i);
        p=disasmrsp(rspoffset+i,x);
        fprintf(f1,"%04X: <%08X>  %s\n",rspoffset+i,x,p);
    }
    fprintf(f1,"\n");
    fprintf(f1,"RSP data at %08X (size %i):",dataaddr,datasize);
    for(i=0;i<datasize;i+=4)
    {
        x=mem_read32(dataaddr+i);
        if(!(i&15)) fprintf(f1,"\n%04X:",i);
        fprintf(f1," %08X",x);
    }
    fprintf(f1,"\n");
    fclose(f1);
}

void disasm_dumpcode(char *filename,dword addr,int size,dword dataaddr,int datasize)
{
    int i;
    dword x;
    char *p;
    FILE *f1;
    f1=fopen(filename,"wb");
    if(!f1) return;
    fprintf(f1,"\n");
    fprintf(f1,"CPU code at %08X (size %i):\n",addr,size);
    for(i=0;i<size;i+=4)
    {
        x=mem_read32(addr+i);
        p=disasm(addr+i,x);
        fprintf(f1,"%04X: <%08X>  %s\n",addr+i,x,p);
    }
    fprintf(f1,"\n");
    fprintf(f1,"CPU data at %08X (size %i):",dataaddr,datasize);
    for(i=0;i<datasize;i+=4)
    {
        x=mem_read32(dataaddr+i);
        if(!(i&15)) fprintf(f1,"\n%04X:",dataaddr+i);
        fprintf(f1," %08X",x);
    }
    fprintf(f1,"\n");
    fclose(f1);
}

