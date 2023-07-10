#include "ultra.h"

State  st;
State2 st2;
Init   init;

Stats  ss[STATS];
int    ssi;

void cpu_updatestats(int skip)
{
    int i,f,d;
    int totalus;

    st.us_total=timer_usreset(&st2.timer);

    f=st.frames-ss[ssi].frame;
    if(f<1 || f>100) f=1;

    ssi=(ssi+1)%STATS;
    i=ssi;

    ss[i].frame   =st.frames;

    ss[i].ops     =st2.ops/f;
    ss[i].cputime =st.cputime;
    ss[i].slowops =st2.slowops/f;
    ss[i].fastops =st2.fastops/f;

    ss[i].txtbytes=st2.gfx_txtbytes/f;
    ss[i].trisin  =st2.gfx_trisin/f;
    ss[i].vxin    =st2.gfx_vxin/f;
    ss[i].tris    =st2.gfx_tris/f;
    ss[i].memio   =st2.memiocheck/f;
    st2.memiocheck=0;
    ss[i].frametgt=1000000.0/st2.frameus;

    if(!st.memiosp)
    {
        st.us_cpu+=st.us_gfx;
        st.us_cpu+=st.us_audio;
    }

    //print("us total %i gfx %i audio %i cpu %i idle %i\n",st.us_total,st.us_gfx,st.us_audio,st.us_cpu,st.us_idle);

    ss[i].us_total=st.us_total/f;
    ss[i].us_gfx  =st.us_gfx/f;
    ss[i].us_audio=st.us_audio/f;
    ss[i].us_cpu  =(st.us_cpu-st.us_idle-st.us_gfx-st.us_audio)/f;
    ss[i].us_idle =st.us_idle/f;
    ss[i].us_misc =(st.us_total-st.us_cpu)/f;

    // audio
    ss[i].samplehz=(1000000.0*st.samples/st.us_total);
    {
        d=st2.audiobufferedcnt;
        if(d>=0x1000)
        {
            ss[i].samplegap=0x1000000+(d>>16);
        }
        else
        {
            if(!d) d=1;
            d=st2.audiobufferedsum/d;
            ss[i].samplegap=d;
        }
        st2.audiobufferedcnt=0;
        st2.audiobufferedsum=0;
    }
    st2.audioresync=0;
    st.samples=0;

    d=st2.snd_interl; if(!d) d=1;
    ss[i].channels=(float)st2.snd_envmix/d;

    // calc
    totalus=ss[i].us_total;
    ss[i].p_cpu   =100.0*ss[i].us_cpu/totalus;
    ss[i].p_gfx   =100.0*ss[i].us_gfx/totalus;
    ss[i].p_audio =100.0*ss[i].us_audio/totalus;
    ss[i].p_idle  =100.0*ss[i].us_idle/totalus;
    ss[i].p_misc  =100.0*ss[i].us_misc/totalus;

    ss[i].mips    =(float)ss[i].ops/(ss[i].us_cpu+ss[i].us_idle);

    ss[i].fps     =1000000.0/totalus;
    if(ss[i].fps>999) ss[i].fps=999;

    totalus=ss[i].fastops+ss[i].slowops; if(!totalus) totalus=1;
    ss[i].compiled=100.0*ss[i].fastops/totalus;

    if(skip)
    {
        int i1=(i+STATS-1)%STATS;
        ss[i].ops=st.cputime-ss[i1].cputime;
        ss[i].frame=-1;
    }

    st2.gfx_txtbytes=0;
    st2.gfx_trisin=0;
    st2.gfx_vxin=0;
    st2.gfx_tris=0;

    st2.snd_envmix=0;
    st2.snd_interl=0;

    st2.ops=0;
    st2.fastops=0;
    st2.slowops=0;

    st.us_gfx=0;
    st.us_audio=0;
    st.us_cpu=0;
    st.us_idle=0;
    st.us_total=0;
}


/********************************************************************/

void cpu_clearstate2(void)
{
    if(st2.audioon)
    {
        sound_stop();
    }
    if(st2.gfxpending)
    {
        os_event(OS_EVENT_SP);
    }
    if(st2.audiopending)
    {
        os_event(OS_EVENT_SP);
    }
    memset(&st2,0,sizeof(st2));

    st.us_gfx=0;
    st.us_audio=0;
    st.us_cpu=0;
    st.us_idle=0;
    st.us_total=0;

    timer_reset(&st2.timer);
}

void cpu_save(FILE *f1)
{
    int part1=(char *)&st._boundary_ -(char *)&st.bailout;
    int part3=sizeof(st);
    st.magic=MAGIC1;
    fwrite(&part1,1,4,f1);
    fwrite(&part3,1,4,f1);
    fwrite(&st,1,part1,f1);
    fwrite(&st,1,part3,f1);
    putc(0xfc,f1);
}

void cpu_load(FILE *f1)
{
    int part1=(char *)&st._boundary_ -(char *)&st.bailout;
    int part3=sizeof(st);
    int p1,p3;

    memset(&st,0,sizeof(st));
    fread(&p1,1,4,f1);
    fread(&p3,1,4,f1);
    if(p1==part1) fread(&st,1,part1,f1); else fseek(f1,p1,SEEK_CUR);
    if(p3==part3) fread(&st,1,part3,f1); else fseek(f1,p3,SEEK_CUR);
    if(p1!=part1 || p3!=part3)
    {
        exception("cpu_load file incompatible (mem:%i/%i,file:%i/%i)",
            part1,part3,p1,p3);
    }
    if(st.magic!=MAGIC1)
    {
        // set some fields in old states that should be nonzero
        if(!st.optimize)
        {
            if(cart.iszelda || cart.ismario) st.optimize=3;
            else st.optimize=1;
        }
        st.dmatransfers=10000;
    }
    if(getc(f1)!=0xfc)
    {
        exception("cpu_load fatal error!");
    }
}

void cpu_init(void)
{
    int i;

    memset(&st,0,sizeof(st));
    memset(&st2,0,sizeof(st2));

    st.nextswitch=0x40000000;

    RA.d=0x3ff0000; // return address os segment

    mem.groupnum=0;
    a_clearcodecache();
    for(i=0;i<STATS;i++) ss[i].frame=-2;

    st.graphicsenable=1;
    st.soundenable=0;
}

void cpu_break(void)
{
    st.bailout=BAILOUTNOW; // out of cpua/cpuc
    st.breakout=1; // out of cpu.c
    // the changing of bailout will falsify cputime a little,
    // but it's not accurate anyway, and breaks do not even
    // happen when things are working ok
}

void cpu_nicebreak(void)
{
    if(!st.executing) return;
    st.nicebreak=1; // out of cpu.c
}

void cpu_goto(dword pc)
{
    st.pc=pc;
    st.branchdelay=0;
    view.codebase=pc;
}

void cpu_keys(int dopad)
{
    int key,stopnow;

    key=1;
    stopnow=0;
    while(key)
    {
        key=con_readkey_noblock();
        if(key==27)
        {
            st.pause=0;
            stopnow=1;
        }
        else if(key && dopad) pad_misckey(key);
    }

    if(stopnow==1)
    {
        exception("ESC pressed");
    }
}

void cpu_checkui(int fast)
{
    static qword  lastupdate;
    static int    lastvidframe=-9;
    static int    laststatframe=-99;
    int    update;

    // check keyboard (1=handle pads too)
    cpu_keys(1);

    // check whether to update screen or not
    update=0;
    if(st.frames && !lastvidframe)
    {
        // first frame came! always update then
        update=1;
    }
    else if(!st.frames)
    {
        // update every CYCLES_PRINTSTART
        if(st.cputime<lastupdate || st.cputime>lastupdate+CYCLES_PRINTSTART) update=1;
    }
    else if(!fast)
    {
        // update every frame
        if(st.frames!=lastvidframe) update=1;
        // update every CYCLES_PRINTSLOW
        if(st.cputime<lastupdate || st.cputime>lastupdate+CYCLES_PRINTSLOW) update=1;
    }
    else
    {
        // update every X frames
        if(st.frames<lastvidframe || st.frames>lastvidframe+st.statfreq) update=1;
        // update every CYCLES_PRINTFAST
        if(st.cputime<lastupdate || st.cputime>lastupdate+CYCLES_PRINTFAST) update=1;
    }

    if(update)
    {
        if(st.cputime>500000+lastupdate && lastvidframe==st.frames)
        {
            cpu_updatestats(1);
        }
        else
        {
            cpu_updatestats(0);
        }

        st.statfreq*=2;
        if(st.statfreq>50) st.statfreq=50;

        lastvidframe=st.frames;
        lastupdate=st.cputime;

        flushdisplay();
    }
}

void cpu_dumptrace(int isret,dword addr)
{
    static char line[30]="------------------------------";

    if(st.callnest<0) st.callnest=0;
    if(st.callnest>30) st.callnest=30;

    if(st.dumptrace)
    {
        if(isret)
        {
            print("\x01\x03T%02i-%s",st.thread,line+30-st.callnest);
            print("ret to %08X\n",addr);
        }
        else
        {
            print("\x01\x03T%02i-%s",st.thread,line+30-st.callnest);
            print("call %08X -> %08X %s\n",st.pc,addr,sym_find(addr));
        }
    }
}

char *breakname[]={"PC","MEM","MEMW","MEMR","MEMDATA","-","-","-",
"NEXTRET","NEXTCALL","NEXTTHREAD","NEXTFRAME","MSG","-","-","-"};

void cpu_clearbp(void)
{
    st.breakpoints=0;
}

void cpu_addbp(int type,dword addr,dword data)
{
    Break *bp;
    if(st.breakpoints==16)
    {
        exception("too many breakpoints");
        return;
    }
    bp=st.breakpoint+st.breakpoints++;
    bp->type=type;
    bp->addr=addr;
    bp->data=data;
}

void cpu_onebp(int type,dword addr,dword data)
{
    cpu_clearbp();
    cpu_addbp(type,addr,data);
}

void cpu_notify_readmem(dword addr,int bytes)
{
    if(memdatar(addr)==RNULL)
    {
        warning("null page %08X read",addr);
    }
    if(st.breakpoints) cpu_checkmembreak(addr,bytes,0);
}

void cpu_notify_writemem(dword addr,int bytes)
{
    /*
    if((addr&0xffff0000)==0xa4600000)
    {
        exception("-> %08X\n",addr);
    }
    */
    if(memdataw(addr)==WNULL)
    {
        warning("null page %08X write",addr);
    }
    if(st.breakpoints) cpu_checkmembreak(addr,bytes,1);
}

void cpu_notify_branch(dword addr,int type)
{
    int isret=(type==BRANCH_RET || type==BRANCH_PATCHRET);
    cpu_checkbranchbreak(addr,type);
    if(type==BRANCH_CALL) st.callnest++;
    if(type!=BRANCH_NORMAL && st.dumptrace)
    {
        cpu_dumptrace(isret,addr);
    }
    if(isret) st.callnest--;
}

void cpu_notify_pc(dword addr)
{
    int i;

    if(st.dumpops)
    {
        dword x;
        x=mem_read32(addr);
        print(GRAY"/%08X: <%08X>  %s\n",addr,x,disasm(addr,x));
    }

    // check pc breakpoints
    for(i=0;i<st.breakpoints;i++)
    {
        if(st.breakpoint[i].type==BREAK_PC)
        {
            if(addr==st.breakpoint[i].addr) cpu_breakpoint(i);
        }
    }
}

void cpu_notify_msg(int queue,dword qaddr,int issend)
{
    int i;

    // check pc breakpoints
    for(i=0;i<st.breakpoints;i++)
    {
        if(st.breakpoint[i].type==BREAK_MSG)
        {
            if(queue==st.breakpoint[i].data) cpu_breakpoint(i);
        }
    }
}

void cpu_breakpoint(int i)
{
    // breakpoint triggered. Some count etc checking could be done here
    // to skip breakpoint. Nothing now.

    if(st.quietbreak)
    {
        cpu_break();
    }
    else
    {
        exception("breakpoint %i (%s)",i,breakname[st.breakpoint[i].type]);
        flushdisplay();
    }
}

void cpu_initbreak(void)
{
    int i;
    // init data values for comparing later on
    for(i=0;i<st.breakpoints;i++)
    {
        switch(st.breakpoint[i].type)
        {
        case BREAK_NEXTTHREAD:
            st.breakpoint[i].data=st.thread;
            break;
        case BREAK_NEXTFRAME:
            st.breakpoint[i].data=st.frames;
            break;
        case BREAK_NEXTRET:
            st.breakpoint[i].data=0;
            break;
        }
    }
}

void cpu_checkeventbreak(void)
{
    int i;
    // check global event breakpoints
    for(i=0;i<st.breakpoints;i++)
    {
        switch(st.breakpoint[i].type)
        {
        case BREAK_NEXTTHREAD:
            if(st.thread!=st.breakpoint[i].data) cpu_breakpoint(i);
            break;
        case BREAK_NEXTFRAME:
            if(st.frames!=st.breakpoint[i].data) cpu_breakpoint(i);
            break;
        }
    }
}

void cpu_checkmembreak(dword addr,int bytes,int iswrite)
{
    int i;
    // check memory breakpoint. Bytes is ignored currently.
    for(i=0;i<st.breakpoints;i++)
    {
        switch(st.breakpoint[i].type)
        {
        case BREAK_MEMDATA:
            if(mem_read32(st.breakpoint[i].addr)!=st.breakpoint[i].data) cpu_breakpoint(i);
            break;
        case BREAK_MEM:
            if(addr>=st.breakpoint[i].addr && addr<=st.breakpoint[i].addr+7) cpu_breakpoint(i);
            break;
        case BREAK_MEMR:
            if(!iswrite && addr==st.breakpoint[i].addr) cpu_breakpoint(i);
            break;
        case BREAK_MEMW:
if(mem_read32(st.breakpoint[i].addr)!=0xc6fa0000) break;
            if(iswrite && addr==st.breakpoint[i].addr) cpu_breakpoint(i);
            break;
        }
    }
}

void cpu_checkbranchbreak(dword addr,int type)
{
    int i;
    // check memory breakpoint. Bytes is ignored currently.
    for(i=0;i<st.breakpoints;i++)
    {
        switch(st.breakpoint[i].type)
        {
        case BREAK_NEXTCALL:
            if(type==BRANCH_CALL) cpu_breakpoint(i);
            break;
        case BREAK_NEXTRET:
            if(type==BRANCH_CALL)
            {
                st.breakpoint[i].data++;
            }
            else if(type==BRANCH_RET)
            {
                if(--(int)st.breakpoint[i].data<0)
                {
                    cpu_breakpoint(i);
                }
            }
            break;
        case BREAK_FWBRANCH:
            if(type==BRANCH_NORMAL && addr>st.pc+4) cpu_breakpoint(i);
            break;
        }
    }
}

void cpu_exec(qword ops0,int fast)
{
    int   hwcheckcnt=CYCLES_BURST;
    int   uicheckcnt=CYCLES_CHECKUI;
    int   breakpointnum;
    int   num,starttime;
    qint  ops=ops0;
    qint  tmpq;

    st.executing=1;

    view_status("executing");
    view_setlast(); // for proper register highlighitng

    st.exectime=st.cputime;

    // clear breakpoints temporarily if fast
    if(fast==2) st.oldcompiler=1;
    else st.oldcompiler=0;
    if(fast)
    {
        breakpointnum=st.breakpoints; // stores
        st.breakpoints=0;
    }
    st.breakout=0;

    cpu_clearstate2();

    cpu_initbreak();

    os_resettimers();

    st.statfreq=2;
    st.nicebreak=0;

    hw_memio();

    // no task changes immediately (if not on top of a PATCH opcode)
    if(OP_OP(mem_readop(st.pc))!=OP_PATCH)
    {
        st.nextswitch=st.cputime+CYCLES_BURST;
    }

    while(ops>0)
    {
        if(st.pause)
        {
            cpu_checkui(0);
            continue;
        }

        if(st.thread==1 || hwcheckcnt<0)
        {
            sync_checkretrace();
            if(st.breakout) break;
        }

        // emulate hardware
        if(hwcheckcnt<0 && !st.memiodetected)
        {
            hwcheckcnt=CYCLES_CHECKOFTEN;
            hw_check();
            os_timers();
            if(!st.memiosp) hw_rspcheck();
        }

        // check keyboard presses, print status info
        if(uicheckcnt<0)
        {
            cpu_checkui(fast);
            uicheckcnt=CYCLES_CHECKUI;
        }

        // thread switch?
        if(st.cputime>st.nextswitch) os_switchcheck();

        // check event breakpoints
        if(st.breakpoints) cpu_checkeventbreak();

        // execute a burst
        {
            starttime=timer_us(&st2.timer);

            if(ops<CYCLES_BURST) num=ops; else num=CYCLES_BURST;

            st.bailout=num;
            if(fast) a_exec();
            else c_exec();

            if(st.bailout<=BAILOUTNOW) num=1;
            else num=num-st.bailout; // how many were really executed

            starttime=timer_us(&st2.timer)-starttime;
            st.us_cpu+=starttime;
            if(st.thread==1)
            {
                st.us_idle+=starttime;
            }
        }

        // increment counters
        ops          -=num;
        st.cputime   +=num;
        st.threadtime+=num;
        hwcheckcnt   -=num;
        uicheckcnt   -=num;

        // exception happened?
        if(st.breakout)
        {
            if(st.breakout!=1) exception("opcode break encountered at %08X",st.breakout);
            break;
        }
    }

print("break!\n");
    cpu_clearstate2();

    st.quietbreak=0;
    st.breakout=0;

    // restore breakpoints
    if(fast)
    {
        st.breakpoints=breakpointnum;
    }

    tmpq=st.cputime-st.exectime;
    if(tmpq>1000)
    {
        print("executed %iK instructions\n",(int)(tmpq/1000));
    }

    st.executing=0;

    view_status("ready");
    flushdisplay();
}

