#include "ultra.h"

#define LOGH if(st.dumphw) logh

void hw_checkoften(void)
{
    // Frequently used hardware checks here. Probably things like
    //   interrupts, dma, pads
    // to avoid the cpu having to wait for them too long. This
    // routine is called every CYCLE_CHECKOFTEN cycles (was 5000)

    // PI registers
    // 0: DRAM address
    // 1: Cart address
    // 2: read DRAM length
    // 3: write DRAM length
}

void hw_checkseldom(void)
{
    // Seldom used hardware checks here, to avoid spending too much
    // time on checking them. This routine is called every
    // CYCLE_CHECKSELDOM cycles (was 50000)

    // SI registers
    // 0: DRAM address
    // 1: read 64B
    // 2: write 64B
    // 3: status

    /*
    if(WPI[2]!=NULLFILL)
    {
        logh("hw: SI Dma Transfer (dram:%08X rd:%X wr:%X st:%X)\n",
            WPI[0],WPI[1],WPI[2],WPI[3]);
        WPI[2]=NULLFILL;
        os_event(5);
    }
    */

    // SP registers
    // 0: DW/IW address
    // 1: DRAM address
    // 2: read (dram->mem)
    // 3: write (mem->dram)
    // 4: status/signals
    // 5: DMA full bit
    // 6: DMA busy bit
    // 7: SP semaphore

    // VI registers
    // 0: status/control
    // 1: origin
    // 2: width
    // 3: vertical interrupt
    // 4: current scanline
    //..

    { // make the current scanline change (Zelda Waits for it at boot)
        RVI[10]+=50;
        if(RVI[10]>480)
        {
            RVI[10]&=1;
            RVI[10]^=1;
        }
    }
}

void hw_check(void)
{
    static qword lastcheck=0;
    static qword nextcheck1=0;
    static qword nextcheck2=0;

    if(st.cputime<lastcheck)
    {
        // loaded a state with a smaller cputime than we had
        nextcheck1=0;
        nextcheck2=0;
    }
    lastcheck=st.cputime;

    if(st.cputime>nextcheck1)
    {
        hw_checkoften();
        nextcheck1=st.cputime+CYCLES_CHECKOFTEN;
    }

    if(st.cputime>nextcheck2)
    {
        hw_checkseldom();
        nextcheck2=st.cputime+CYCLES_CHECKSELDOM;
    }
}

/********************************************************************
** PI emulation
*/

void hw_pi_dma(void)
{
    if(WPI[3])
    {
        logh("hw: PI Dma Transfer (dram:%08X cart:%08X rd:%X wr:%X st:%X)\n",
            WPI[0],WPI[1],WPI[2],WPI[3],WPI[4]);
        osPiStartDma(0,0,0,WPI[1],WPI[0],WPI[3]+1,0);
        WPI[3]=0;
    }
}

/********************************************************************
** SP emulation
*/

static dword    sptaskpos;
static OSTask_t sptask;
static int      spstatus;
static int      sploaded;
static int      spgfxexecuting; // 0=not, 1=yup, 2=done

// NOT USED RIGHT NOW
void hw_gfxthread(void)
{
    if(!st.gfxthread)
    {
        con_sleep(100);
        return;
    }

    if(spgfxexecuting==1)
    {
        //print("gfxthread-start (%i)\n",timer_ms(&st2.timer));
        dlist_execute(&st2.gfxtask);
        //print("gfxthread-end   (%i)\n",timer_ms(&st2.timer));
        spgfxexecuting=2;
    }

    con_sleep(1); // always sleep a little bit
}

void hw_sp_endgfx(void)
{
    os_event(OS_EVENT_SP);
    if(st2.gfxdpsyncpending)
    {
        sync_gfxframedone();
        os_event(OS_EVENT_DP);
    }
    st2.gfxdpsyncpending=0;
    spgfxexecuting=0;
}

void hw_sp_startgfx(void)
{
    if(st.graphicsenable<=0)
    {
        hw_sp_endgfx();
        return;
    }

    if(!st.gfxthread)
    {
        // run in the same thread
        dlist_execute(&st2.gfxtask);
        hw_sp_endgfx();
    }
    else
    {
        // run in a separate thread
        spgfxexecuting=1;
    }
}

void hw_sp_yield(void)
{
    if(!spgfxexecuting)
    {
        LOGH("\n");
        warning("hw: spYield with no executing task");
        // hack
//        os_event(OS_EVENT_SP);
    }
    else
    {
        // this is a YIELD request
        if(spgfxexecuting)
        {
            spstatus|=(1<<8);  // still executing, raise signal 1
        }
        // sp task done
        os_event(OS_EVENT_SP);
    }
    // remove signal 0
    spstatus&=~(1<<7);
    // mark that task has halted
    spstatus|=1;
    RSP[4]=spstatus;
}

void hw_sp_start(void)
{
    logh("hw-sp: task %08X (type=%04X,flag=%04X,ucode=%08X/%04X,data=%08X/%04X,boot=%08X/%04X,yield=%08X/%04X)\n",
            sptaskpos,
            sptask.type,sptask.flags,
            sptask.m_ucode,sptask.ucode_size,
            sptask.m_data_ptr,sptask.data_size,
            sptask.m_ucode_boot,sptask.ucode_boot_size,
            sptask.m_yield_data_ptr,sptask.yield_data_size);

    if(sptask.type==1)
    {
        // GFX task
        if(sptask.flags&1)
        {
            // ignore yield continues, we have all the
            // info from the first exec. Keep spexecuting=1
            // since we are still processing
            return;
        }
        else
        {
            memcpy(&st2.gfxtask,&sptask,sizeof(OSTask_t));
            hw_sp_startgfx();
        }
    }
    else
    {
        // NON-GFX task
        if(sptask.type==2)
        {
            memcpy(&st2.audiotask,&sptask,sizeof(OSTask_t));
            if(st.soundenable) slist_execute(&st2.audiotask);
            os_event(OS_EVENT_SP);
        }
        else if(sptask.type==4 && cart.iszelda)
        {
            zlist_uncompress(&sptask);
            os_event(OS_EVENT_SP); // sp task done
        }
        else
        {
            error("Unknown SP task type %04X\n",sptask.type);
            os_event(OS_EVENT_SP); // sp task done
        }
    }
    sploaded=0;
}

void hw_sp_check(void)
{
    if(spgfxexecuting==2)
    {
        // gfx task ended
        hw_sp_endgfx();
        spstatus|=1;
        RSP[4]=spstatus;
    }
}

void hw_sp_statuswrite(void)
{
    dword status;
    int   go;

    status=WSP[4];
    WSP[4]=0;

    if(status && status!=0x70707070)
    {
        go=0;
        LOGH("hw-sp: command %08X ",status,spstatus);

        if(status&(1<<0))
        { // clear halt, start exec
            LOGH("go! ");
            if(sploaded) go=1;
            else
            {
                LOGH("\n");
                warning("hw: spGo without loaded task");
                os_event(OS_EVENT_SP);
            }
        }
        if(status&(1<<1))
        { // set halt, stop exec
            warning("hw: spHalt\n");
        }
        if(status&(1<<2)) spstatus&=~(1<<1);
        /*
        if(status&(1<<3)) ; // clear intr
        if(status&(1<<4)) ; // set intr
        if(status&(1<<5)) ; // clear sstop
        if(status&(1<<6)) ; // set sstep
        if(status&(1<<7)) ; // clear intr-on-break
        if(status&(1<<8)) ; // set intr-on-break
        */
        if(status&(1<< 9))
        {
            spstatus&=~(1<<7);  // clear signal 0
        }
        if(status&(1<<10))
        {
            spstatus|= (1<<7);  // set   signal 0
            LOGH("yield! ");
            hw_sp_yield();
        }
        if(status&(1<<11)) spstatus&=~(1<<8);  // clear signal 1
        if(status&(1<<12)) spstatus|= (1<<8);  // set   signal 1
        if(status&(1<<13)) spstatus&=~(1<<9);  // clear signal 2
        if(status&(1<<14)) spstatus|= (1<<9);  // set   signal 2
        LOGH(" status=%08X\n",spstatus);

        if(go)
        {
            hw_sp_start();
        }
    }

    if(spgfxexecuting) spstatus&=~1;
    else               spstatus|=1;
    RSP[4]=spstatus;
}

void hw_sp_dmawrite(void)
{
    dword from,to,cnt;

    to  =WSP[0];
    from=WSP[1];
    cnt =WSP[2];

    if(!cart.first_rcp)
    {
        print("note: first rcp access\n");
        cart.first_rcp=1;
    }

    LOGH("hw-sp: dma %08X->%08X,%-4X ",from,to,cnt);

    if((to&0xffff)==0x0FC0)
    {
        // detected load if OSTask structure
        sptaskpos=from;
        mem_readrangeraw(from,sizeof(sptask),(char *)&sptask);
        sploaded=1;
        LOGH(" Taskload (type=%i, flags=%i) ",sptask.type,sptask.flags);
    }
    LOGH("\n");

    WSP[0]=0;
    WSP[1]=0;
    WSP[2]=0;
}

/********************************************************************
** Old RSP execution (when directsp=0)
*/

void hw_rspcheck(void)
{
    // this is only used when osSp calls are done directly
    if(st2.audiopending)
    {
        if(st2.gfxdpsyncpending)
        {
            os_event(OS_EVENT_DP); // dp full sync interrupt
            st2.gfxdpsyncpending=0;
        }
        if(st.soundenable)
        {
            slist_execute(&st2.audiotask);
        }
        os_event(OS_EVENT_SP);
        st2.audiopending=0;
    }

    if(st2.gfxpending && !st2.gfxdpsyncpending && !st2.gfxfinishpending && !st2.gfxthread_execute)
    {
        if(st.graphicsenable>0)
        {
            dlist_execute(&st2.gfxtask);
        }

        st2.gfxdpsyncpending=1;
        st2.gfxfinishpending=1;

        st2.gfxpending=0;

        if(st2.gfxfinishpending)
        {
            os_event(OS_EVENT_SP);
            sync_gfxframedone();
            st2.gfxfinishpending=0;
        }

        if(st2.gfxdpsyncpending)
        {
            os_event(OS_EVENT_DP); // dp full sync interrupt
            st2.gfxdpsyncpending=0;
        }
    }
}

/********************************************************************
** SI (pads)
*/

static int selectpad=0;

void hw_selectpad(int pad)
{
    selectpad=pad;
}


void hw_si_pads(int write)
{
    dword base=WSI[0];
    int i;

    logh("hw-si: dma %08X (write=%i)\n",base,write);
    if(!cart.first_pad)
    {
        print("note: first pad access\n");
        cart.first_pad=1;
    }

    if(write)
    {
        for(i=0;i<16;i++)
        {
            RPIF[0x1f0+i]=mem_read32(base+i*4);
        }
    }
    else
    {
        // 2 dwords for each controller:
        // 000000ss bbbbxxyy
        // ss=status? !&00C0 or error
        // bbbb=buttons
        // xx=stick-x
        // yy=stick-y

        // construct reply
        memset(RPIF+0x1f0,0,16*4);
        RPIF[0x1f0+1+2*selectpad]=pad_getdata(0);

        // copy it
        for(i=0;i<16;i++)
        {
            mem_write32(base+i*4,RPIF[0x1f0+i]);
        }
    }

    /*
    for(i=0;i<16;i++)
    {
        print("%08X ",RPIF[0x1f0+i]);
        if((i&3)==3) print("\n");
    }
    */

    WSI[1]=NULLFILL;
    WSI[4]=NULLFILL;

    RSI[0]=WSI[0];
    RSI[6]=0; // not busy

    os_event(OS_EVENT_SI);
}

/********************************************************************
** Memory IO Detected entrypoint
**
** a LUI with x40xxxxx was loaded recently, memory IO may have happened.
** This may be called from a_exec, so st.pc is not current.
*/

void hw_init(void)
{
    // init reg data we don't want to be 0 (default init value)
    WSI[1]=NULLFILL;
    WSI[4]=NULLFILL;
}

void hw_memio(void)
{
    st.memiodetected=0;
    st2.memiocheck++;

//    print("hw: memio (pc=%08X,memio=%08X,spstat=%08X)\n",st.pc,st.memiodetected,RSP[4]);

    hw_sp_check(); // detectes thread finishes
    if(WSP[4]) hw_sp_statuswrite();
    if(WSP[2]) hw_sp_dmawrite();

    if(WPI[3]) hw_pi_dma();

    if(WSI[1]!=NULLFILL) hw_si_pads(0); // write
    if(WSI[4]!=NULLFILL) hw_si_pads(1); // read
}

// return 1 if the 64K memory page address given should be alerted
// to hw_memio when LUI loads it. This is called from cpuc.c and
// from cpua.c when compiling. It is not called when executing compiled
// code, so speed is not that important here.
int hw_ismemiorange(dword addr)
{
    addr&=0x1fff0000;
    if(addr==0x04040000) return(1); // SP registers are handled
    if(addr==0x04600000) return(1); // PI registers are handled
    if(addr==0x04800000) return(1); // PI registers are handled
    return(0);
}

