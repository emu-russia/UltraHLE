#include "ultra.h"

/***********************************************************/

dword spmem(int x)
{
    int d;
    d=mem_read32(SP.d+x);
    return(d);
}

void printmem(dword addr)
{
    char buf[256];
    int i,b=addr;
    for(i=0;i<255;i++)
    {
        buf[i]=mem_read8(b+i);
        if(buf[i]==0) break;
        if(buf[i]<32) buf[i]='.';
    }
    buf[i]=0;
    logi("Print(%08X) \"%s\"\n",b,buf);
}

void p_starttimer(void)
{
    logi("\x01\x09""start_timer(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
    printmem(A0.d);
}

void p_restarttimer(void)
{
    logi("\x01\x09""restart_timer(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
    printmem(A0.d);
}

void p_dunno1timer(void)
{
    logi("\x01\x09""dunno1_timer(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
    printmem(A0.d);
}

void p_dunno2timer(void)
{
    logi("\x01\x09""dunno2_timer(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
    printmem(A0.d);
}

void p_gettimernum(void)
{
    logi("\x01\x09""getnum_timer(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
    printmem(A0.d);
}

void p_osCont(void)
{
    logi("\x01\x09""osController at %08X\n",st.pc);
}

void p_osGetTime(void)
{
    dword lo,hi;
    osGetTime(&lo,&hi);
    V0.d=hi;
    V1.d=lo;
}

void p_osGetCount(void)
{
    dword lo,hi;
    osGetTime(&lo,&hi);
    V0.d=lo;
}

void p_osCreateMesgQueue(void)
{
    osCreateMesgQueue(A0.d,A1.d,A2.d);
}

void p_readdma(void)
{
    print("readdma?(%X,%X,%X,%X,%X)\n",
        A0.d,A1.d,A2.d,A3.d,spmem(0x10));
    st.branchdelay=0; // do the real routine too
}

void p_readdma2(void)
{
    logi("ReadDma(%X,%X,%X) patched to osPiStartDMA\n",A0.d,A1.d,A2.d);
    osPiStartDma(0,0,0,A0.d,A1.d,A2.d,0);
}

void p_osPiStartDma(void)
{
    V0.d=osPiStartDma(A0.d,A1.d,A2.d,A3.d,
                      spmem(0x10),spmem(0x14),spmem(0x18));
}

void p_osEPiStartDma(void)
{
    V0.d=osEPiStartDma(A0.d,A1.d,A2.d);
}

void p_osSetEventMessage(void)
{
    osSetEventMessage(A0.d,A1.d,A2.d);
}

void p_osViSetEvent(void)
{
    // A2.d == 1 ?
    print("osViSetEvent: %X %X %X\n",A0.d,A1.d,A2.d);
    osSetEventMessage(OS_EVENT_RETRACE,A0.d,A1.d);
}

void p_osViSwapBuffer(void)
{
    st.fb_next=A0.d;
    logo("osViSwapBuffer(%08X)\n",A0.d);
    rdp_swap();
}

void p_osViGetCurrentFrameBuffer(void)
{
    V0.d=st.fb_current;
    logo("osViGetCurrentFrameBuffer=%08X (next=%08X) [from %08X]\n",V0.d,st.fb_next,RA.d);
}

void p_osAiSetNextBuffer(void)
{
    V0.d=osAiSetNextBuffer(A0.d,A1.d);
}

void p_osAiSetFrequency(void)
{
    V0.d=osAiSetFrequency(A0.d);
}

void p_osAiGetLength(void)
{
    V0.d=osAiGetLength();
}

void p_osSendMesg(void)
{
    V0.d=osSendMesg(A0.d,A1.d,A2.d);
}

void p_osRecvMesg(void)
{
    V0.d=osRecvMesg(A0.d,A1.d,A2.d);
}

void p_osCreateThread(void)
{
    osCreateThread(A0.d,A1.d,A2.d,A3.d,spmem(0x10),spmem(0x14));
}

void p_osStartThread(void)
{
    osStartThread(A0.d);
}

void p_osStopCurrentThread(void)
{
    osStopCurrentThread();
}

void p_osSetThreadPri(void)
{
    osSetThreadPri(A0.d,A1.d);
}

void p_osMapTLB(void)
{
    osMapTLB(A0.d,A1.d,A2.d,A3.d,spmem(0x10),spmem(0x14));
}

void p_osSetTimer(void)
{
    // note A1.d unused! (register align issue on 64 bit params?)
    V0.d=osSetTimer(A0.d,A2.d,A3.d,spmem(0x10),spmem(0x14),spmem(0x18),spmem(0x1c));
}

void p_osStopTimer(void)
{
    osStopTimer(A0.d);
}

void p_sinf(void)
{
    float f;
    f=st.f[12].f;
    st.f[0].f=sin(f);
    logi("\x01\x09""sinf(%e)=%e\n",
        f,st.f[0].f);
}

void p_cosf(void)
{
    float f;
    f=st.f[12].f;
    st.f[0].f=cos(f);
    logi("\x01\x09""cosf(%e)=%e\n",
        f,st.f[0].f);
}

void p_osVirtualToPhysical(void)
{
    V0.d=osVirtualToPhysical(A0.d);
//    logi("\x01\x09""osVirtualToPhysical(%08X)=%08X\n",A0.d,V0.d);
}

void p_osPhysicalToVirtual(void)
{
    V0.d=osPhysicalToVirtual(A0.d);
}

void p_osContInit(void)
{
    V0.d=osContInit(A0.d,A1.d,A2.d);
}

void p_osContStartReadData(void)
{
    V0.d=osContStartReadData(A0.d);
}

void p_osContStartQuery(void)
{
    V0.d=osContStartQuery(A0.d);
}

void p_osContGetReadData(void)
{
    osContGetReadData(A0.d);
}

void p_osContGetQuery(void)
{
    osContGetQuery(A0.d);
}

void p_osInvalICache(void)
{
    logo("- osInvalICache(%X,%X)\n",A0.d,A1.d);
//    asm_clearrange(A0.d,A1.d);
}

void p_osSpTaskStartGo(void)
{
    if(!st2.sptaskload)
    {
        osSpTaskStartGo(A0.d);
        st2.sptaskload=0;
    }
}

void p_osSpTaskYield(void)
{
    V0.d=osSpTaskYield();
}

void p_osSpTaskYielded(void)
{
    V0.d=osSpTaskYielded(A0.d);
}

void p_osSpTaskLoad(void)
{
    osSpTaskStartGo(A0.d);
    st2.sptaskload=1;
}

void p_zeldacont(void)
{
    os_event(OS_EVENT_SI);
}

void p_zeldagrabscreen(void)
{
}

void p_createvimanager(void)
{
//    print("vi manager: %X %X %X\n",A0.d,A1.d,A2.d);
}

void p_skip(void)
{
//    logi("\x01\x09""- skip(%X,%X,%X,%X)\n",A0.d,A1.d,A2.d,A3.d);
}

void p_osskip(void)
{
    logo("- osskip(%X,%X,%X,%X) ",A0.d,A1.d,A2.d,A3.d);
    logo("%s (ra=%08X)\n",sym_find(st.pc),RA.d);
}

void p_osskip0(void)
{
    p_osskip();
    V0.d=0;
}

void p_long2double(void)
{
    // A0:A1 -> FP00
    qreg a;
    double d;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    d=(qint)a.q;
    st.f[0].d=*((dword *)&d+0);
    st.f[1].d=*((dword *)&d+1);
//    print("from %08X long2double %08X%08X -> %f\n",RA.d,A0.d,A1.d,d);
}

void p_double2long(void)
{
    // FP12 -> A0:A1
    qreg a;
    double d;
    d=*(double *)&st.f[12].f;
    d+=rand()*1e8;
    a.q=(qint)d;
    V1.d=a.d2[0];
    V0.d=a.d2[1];
//    print("from %08X double2long %f -> %08X%08X\n",RA.d,d,A0.d,A1.d,d);
}

void p_long2single(void)
{
    // A0:A1 -> FP00
    qreg a;
    float f;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    f=(qint)a.q;
    st.f[0].f=f;
//    print("from %08X long2single %08X%08X -> %f\n",RA.d,A0.d,A1.d,f);
}

void p_single2long(void)
{
    // FP12 -> A0:A1
    qreg a;
    float f;
    f=st.f[12].f;
    a.q=(qint)f;
    V1.d=a.d2[0];
    V0.d=a.d2[1];
//    print("from %08X single2long %f -> %08X%08X\n",RA.d,f,A0.d,A1.d,f);
}

/*
7000A450:  lui        a0 = 80020000
7000A454:  ld         a0 <- [a0+4460]
7000A458:  lui        at = 80020000
7000A45C:  Dsll32     a2 = a0<<63
7000A460:  Dsll       a1 = a0<<31
7000A464:  Dsrl       a2 = a2>>31
7000A468:  Dsrl32     a1 = a1>>32
7000A46C:  Dsll32     a0 = a0<<44
7000A470:  or         a2 = a2 | a1
7000A474:  Dsrl32     a0 = a0>>32
7000A478:  xor        a2 = a2+a0
7000A47C:  Dsrl       a0 = a2>>20
7000A480:  andi       a0 = a0 & 0FFF
7000A484:  xor        a0 = a0+a2
7000A488:  Dsll32     v0 = a0<<32
7000A48C:  sd         a0 -> [at+4460]
7000A490:  jr         ra
7000A494:  Dsra32     v0 = v0>>32
*/
void p_golden1(void) // random bit?
{
#if 0
    qreg x;
    qword a0,a1,a2,v0;
    x.d2[1]=mem_read32(0x80024460);
    x.d2[0]=mem_read32(0x80024464);

//    print("golden1 from %08X: %08X%08X ->",RA.d,x.d2[1],x.d2[0]);
    a0=x.q;
    a2=a0<<63; // a2=sign
    a1=a0<<31; // a1=lower 32
    a2=a2>>31; // 00000000???????? or FFFFFFFF????????
    a1=a1>>32; // a1=lower 32 sign extended
    a0=a0<<44; // a0=value << 32 << 12
    a2=a2|a1;  // lower 32 forced to negative if total negative?
    a0=a0>>32; // a0=value << 32 << 12 >> 32=bits 32..(32+12) sign extended
    a2=a2^a0;
    a0=a2>>20;
    a0&=0xfff;
    a0=a0^a2;
    v0=a0<<32;
    x.q=a0;

//    print("%08X%08X\n",x.d2[1],x.d2[0]);
    mem_write32(0x80024460,x.d2[1]);
    mem_write32(0x80024464,x.d2[0]);
    V0.d=(qint)v0>>32;
#else
    dword x;
    x=rand();
    x<<=16;
    x|=rand();
    x=(x<<8)|(x>>24);
    V0.d=x;
#endif
}

void p_dmultu(void)
{
    qreg a,b,r;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    b.d2[0]=A3.d;
    b.d2[1]=A2.d;
    r.q=(unsigned __int64)a.q*(unsigned __int64)b.q;
    V1.d=r.d2[0];
    V0.d=r.d2[1];
//    logi("dop: %08X%08X=%08X%08X*%08X%08X\n",A0.d,A1.d,A2.d,A3.d,V0.d,V1.d);
}

void p_ddivu(void)
{
    qreg a,b,r;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    b.d2[0]=A3.d;
    b.d2[1]=A2.d;
    r.q=(unsigned __int64)a.q/(unsigned __int64)b.q;
    V1.d=r.d2[0];
    V0.d=r.d2[1];
//    logi("dop: %08X%08X=%08X%08X/%08X%08X (uns)\n",A0.d,A1.d,A2.d,A3.d,V0.d,V1.d);
}

void p_ddiv(void)
{
    qreg a,b,r;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    b.d2[0]=A3.d;
    b.d2[1]=A2.d;
    r.q=(__int64)a.q/(__int64)b.q;
    V1.d=r.d2[0];
    V0.d=r.d2[1];
//    logi("dop: %08X%08X=%08X%08X/%08X%08X (sig)\n",A0.d,A1.d,A2.d,A3.d,V0.d,V1.d);
}

void p_drem(void) // _ll_rem and _ull_rem seem to be the same!
{
    qreg a,b,r;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    b.d2[0]=A3.d;
    b.d2[1]=A2.d;
    r.q=(unsigned __int64)a.q%(unsigned __int64)b.q;
    V1.d=r.d2[0];
    V0.d=r.d2[1];
//    logi("dop: %08X%08X=%08X%08X rem %08X%08X\n",A0.d,A1.d,A2.d,A3.d,V0.d,V1.d);
}

void p_dmod(void)
{
    qreg a,b,r;
    a.d2[0]=A1.d;
    a.d2[1]=A0.d;
    b.d2[0]=A3.d;
    b.d2[1]=A2.d;
    r.q=(__int64)a.q%(__int64)b.q;
    V1.d=r.d2[0];
    V0.d=r.d2[1];
//    logi("dop: %08X%08X=%08X%08X mod %08X%08X\n",A0.d,A1.d,A2.d,A3.d,V0.d,V1.d);
}

void p_dabort(void)
{
    exception("unimplemented dword routine");
}

void p_banjojalr(void)
{
    inifile_patches(-2);

    st.branchtype=BRANCH_NORMAL;
    st.branchdelay=1;
    st.branchto=st.pc+4;
}

void p_memcpy(void)
{
    int s,d,c,a;
    d=A0.d;
    s=A1.d;
    c=A2.d;
    c>>=2;
    while(c-->0)
    {
        a=mem_read32(s);
        s+=4;
        mem_write32(d,a);
        d+=4;
    }
}

typedef void (*t_patchroutine)(void);
t_patchroutine patchtable[]={
// 0
NULL,
p_skip,
p_osskip,
p_ddivu,
p_dmultu,
// 5
p_drem,
p_ddiv,
p_drem,
p_osskip0,
p_dabort,
// 10
p_osCreateThread,
p_osStartThread,
p_osPiStartDma,
p_osCreateMesgQueue,
p_osRecvMesg,
// 15
p_osSendMesg, // also JamMesg
p_osSetEventMessage,
p_osViSetEvent,
p_osSetThreadPri,
p_osGetTime,
// 20
p_osViGetCurrentFrameBuffer,
p_osVirtualToPhysical,
p_osSpTaskStartGo,
p_osViSwapBuffer,
p_osMapTLB,
// 25
p_osContInit,
p_osContStartReadData,
p_osContGetReadData,
p_osStopCurrentThread,
p_osPhysicalToVirtual,
// 30
p_osAiSetNextBuffer,
p_osAiGetLength,
p_osAiSetFrequency,
p_osSetTimer,
p_osStopTimer,
// 35
p_sinf,
p_cosf,
p_osEPiStartDma,
p_osInvalICache,
p_osSpTaskLoad,
// 40
p_readdma,
p_readdma2,
p_osContStartQuery,
p_osContGetQuery,
p_zeldacont,
// 45
p_zeldagrabscreen,
p_osSpTaskYield,
p_osSpTaskYielded,
p_osGetCount,
p_banjojalr,
// 50
p_long2double,
p_long2single,
p_double2long,
p_single2long,
p_golden1,
// 55
p_createvimanager,
p_memcpy,
NULL};

void op_patch(int patch)
{
    // setup return
    st.branchtype=BRANCH_PATCHRET;
    st.branchdelay=1;
    st.branchto=st.g[31].d;

    if(patch<=2 && !st.dumpinfo) return;

    // execute patched routine
    if(!patchtable[patch] || patch>=sizeof(patchtable)/4)
    {
        if((patch&0xffff)==(NULLFILL&0xffff))
        {
            st.branchdelay=0;
            exception("Execution at null page (st.pc=%08X).",st.pc);
        }
        else exception("Unimplemented patch %i!",patch);
        return;
    }
    patchtable[patch]();
}

