#include "ultra.h"

//#define DYNAMICPRIORITY

#define CLOCKRATE 45637500

#define TIMERS

#define MAXTHREAD 16
#define MAXEVENTS 16
#define MAXTIMER  16
#define MAXQUEUE  128

#define MAXQUEUESIZE 256

#define THREADSLICE     (CYCLES_BURST*2-1) // timeslice for a thread (uncless block comes along)

typedef struct
{
    dword  memaddr;
    dword  id;
    int    realpriority;
    dword  flags;
    dword  active; // 1 for any created threads
    dword  ready;  // 1=ok to execute (0=waiting)
    State  st;
    // blocking queues
    dword  sendblock;
    dword  recvblock;
    int    priority;
    dword  RESERVED[15];
} OSThread;

typedef struct
{
    dword  memaddr;
    int    size; // total size of queue
    int    pos;  // position in queue
    int    num;  // messages in queue
    int    totalmsgs;
    int    fullerrors;
    dword  RESERVED[15];
    dword  data[MAXQUEUESIZE];
} OSQueue;

typedef struct
{
    int    queueid; // 0=no queue
    dword  mesg;
    int    count;
    dword  RESERVED[16];
} OSEvent;

typedef struct
{
    dword  memaddr;
    int    active;
    int    timerid; // 0=unused
    dword  m_queue;
    dword  m_mesg;
    int    queueid;
    int    count; // us
    int    interval; // us
    dword  RESERVED[16];
} OSTimer;

OSThread thread[MAXTHREAD];
int    currentthread=0;
int    threadnum=1;

OSQueue  queue[MAXQUEUE];
int    queuenum=1; // 0 unused

OSTimer  timer[MAXTIMER];
int    timernum=1; // 0 unused

OSEvent  event[MAXEVENTS];

void os_init(void)
{
    currentthread=0;
    threadnum=1;
    queuenum=1;
    timernum=1;
    memset(thread,0,sizeof(thread));
    memset(queue,0,sizeof(queue));
    memset(timer,0,sizeof(timer));
    memset(event,0,sizeof(event));
}

void forcetaskswitch(int totask,int nottotask)
{
    st.bailout=-1; // asap to cpu.c to do a taskswitch
    st.nextswitch=0;
    st.trythread=totask;
    st.avoidthread=nottotask;
    st.checkswitch=1;
}

void blocktask(int i)
{
    // back up PC to retry call on return to task
    st.pc-=4;
    st.branchdelay=0; // no branchout yet

    thread[i].ready=0;
    forcetaskswitch(0,0);
}

void unblocktask(int i)
{
    thread[i].ready=1;
    forcetaskswitch(i,0);
}

void os_resettimers(void)
{
    timer_reset(&st2.ostimer);
}

void os_timers(void)
{
#ifdef TIMERS
    int i,t;
    double tim,delta;
    static double lastt=999999999.0;
    tim=os_gettimeus();
    delta=tim-lastt;
    lastt=tim;
    if(delta<0 || delta>1000000.0)
    {
        // error in delta
        return;
    }
    t=(int)delta;

    for(i=0;i<MAXTIMER;i++)
    {
        if(!timer[i].active) continue;
        timer[i].count-=t*4;
        if(timer[i].count<0)
        {
            if(!timer[i].interval) timer[i].active=0;
            else
            {
                timer[i].count+=timer[i].interval;
                if(timer[i].count<=0) timer[i].count=timer[i].interval;
            }
            // send message
            logo(BLUE"OSTimer %i triggered (interval %i) -> queue %i\n",
                i,timer[i].interval,timer[i].queueid);
            osSendMesg(timer[i].m_queue,timer[i].m_mesg,-1);
        }
    }
#endif
}

void os_switchthread(dword id)
{
    int copysize=(&st._boundary_-&st.bailout)*4;

    logo(BLUE"SwitchOSThread %i:%08X -> ",st.thread,st.pc);

    memcpy(&thread[currentthread].st,&st,sizeof(State));
    currentthread=id;
    memcpy(&st,&thread[currentthread].st,copysize);

    logo("%i:%08X (cputime %i)\n",id,st.pc,(int)st.cputime);
}

int  os_findthread(void)
{
    int i,t,tp,next;
    t=0;
    tp=-1;
    if(st.trythread) next=st.trythread-1;
    else next=st.thread;
    for(i=0;i<MAXTHREAD*2;i++)
    {
        next=(next+1)&(MAXTHREAD-1);
        if(thread[next].ready)
        {
            if(next==st.avoidthread && i<MAXTHREAD) continue;
            if(thread[next].priority>tp)
            {
                t=next;
                tp=thread[next].priority;
            }
        }
    }
    return(t);
}

// always called from sim_step
void os_switchcheck(void)
{
    int t;

    // assign timeslice
    st.nextswitch=st.cputime+THREADSLICE;
    // find highest priority ready thread
    t=os_findthread();
    // kick some events
    if(thread[t].priority==0)
    {
        os_taskhacks(1);
        t=os_findthread();
    }
    else
    {
        os_taskhacks(0);
    }
    // switch if thread changed
    if(t!=st.thread)
    {
        if(t==0)
        {
            exception("At %08X, no runnable threads!\n",st.pc);
            return; // no runnable threads, continue as usual
        }
        os_switchthread(t);
    }
    // if no priority, minimize slice
    if(thread[t].priority==0) st.nextswitch=st.cputime;
}

void osCreateThread(dword m_thread,dword id, dword m_routine,
                    dword m_param,dword m_stack,dword priority)
{
    int a,i;
    logo(BLUE"osCreateThread(%08X/%i,...) stack:%08X code:%08X pri:%i\n",
        m_thread,id,m_stack,m_routine,priority);

    if(id<=0 || id>=MAXTHREAD || thread[id].active)
    {
        a=id;
        for(i=1;i<MAXTHREAD;i++)
        {
            if(!thread[i].active)
            {
                id=i;
                break;
            }
        }
        logo(BLUE"OSThread number collision, converting %i -> %i\n",a,id);
        if(id==a) exception("Out of threads!");
    }
    if(id>=MAXTHREAD)
    {
        exception("thread id too large");
        return;
    }

    if(id>=threadnum) threadnum=id+1;

    mem_write32(m_thread,id);
    memset(thread+id,0,sizeof(OSThread));
    thread[id].memaddr=m_thread;
    thread[id].id=id;
    thread[id].priority=priority*10; // use 10x priorities for more control
    thread[id].realpriority=priority*10;
    thread[id].active=1;
    thread[id].ready=0;
    thread[id].flags=0;
    thread[id].st.thread=id;
    thread[id].st.nextswitch=256;
    thread[id].st.pc     =m_routine; // PC
    thread[id].st.g[ 4].d=m_param;
    thread[id].st.g[29].d=m_stack-8; // SP

    thread[id].st.g[31].d=0x3ff0000; // return address os segment

    ROS[0]=PATCH(28); // osStopCurrentThread routine there
    WOS[0]=PATCH(28); // osStopCurrentThread routine there
}

void osStopCurrentThread(void)
{
    logo(BLUE"osStopCurrentThread()\n");
    thread[currentthread].active=0;
    thread[currentthread].ready=0;
    forcetaskswitch(0,0);
}

void osSetThreadPri(dword m_thread,int pri)
{
    int id;
    if(!m_thread) id=st.thread;
    else id=mem_read32(m_thread);
    logo(BLUE"osSetThreadPriority(%08X=%i,%i)\n",m_thread,id,pri);
    if(id>=MAXTHREAD)
    {
        exception("thread id too large");
        return;
    }

    thread[id].priority=pri*10; // use 10x priorities

    // reschedule
    forcetaskswitch(id,0);
}

void osStartThread(dword m_thread)
{
    dword id=mem_read32(m_thread);
    logo(BLUE"osStartThread(%08X/%i)\n",m_thread,id);
    if(id>=MAXTHREAD)
    {
        exception("thread id too large");
        return;
    }

    thread[id].ready=1;

    // reschedule
    forcetaskswitch(id,0);
}

/********************************************************/

void os_queuetomem(int id)
{
    int max,valid;
    max=queue[id].size;
    valid=queue[id].num;
    mem_write32(queue[id].memaddr+0,id);
    mem_write32(queue[id].memaddr+4,0xfcfc1234);
    mem_write32(queue[id].memaddr+8,valid);
    mem_write32(queue[id].memaddr+16,max);
}

int os_queuecheck(int id)
{
    if(!id || id>=queuenum) return(1);
    if(mem_read32(queue[id].memaddr+0)!=id ||
       mem_read32(queue[id].memaddr+4)!=0xfcfc1234) return(1);
    return(0);
}

void  osCreateMesgQueue(dword m_queue,dword m_mesg,dword size)
{
    int id,i;

    // find if queue already exits
    for(i=1;i<queuenum;i++)
    {
        if(m_queue==queue[i].memaddr)
        {
            id=i;
            break;
        }
    }
    if(i==queuenum)
    { // not found
        // find first corrupted queue
        for(i=1;i<queuenum;i++)
        {
            if(os_queuecheck(i))
            { // this queue has died (was in stack?)
                id=i;
                logo("queue %i dead (recovered)\n",i);
                break;
            }
        }
        if(i==queuenum)
        {
            // no dead queues found, use next unused one
            id=queuenum++;
        }
    }

    if(!id || id>=MAXQUEUE)
    {
        exception("queue id too large");
        return;
    }
    if(size>MAXQUEUESIZE)
    {
        exception("queue size too large");
        return;
    }
    logo(BLUE"CreateMesgOSQueue %08X (%i)\n",m_queue,id);
    queue[id].memaddr=m_queue;
    queue[id].size=size;
    queue[id].num=0;
    queue[id].pos=0;
    os_queuetomem(id);
}

dword osSendMesg(dword m_queue,dword m_mesg,int block)
{
    dword id=mem_read32(m_queue);
    OSQueue *q=queue+id;
    int i;
    if(!id || id>=queuenum)
    {
        if(!id)
        {
            logo("queue at %08X uninitialized, skipping SendMesg\n",m_queue);
        }
        else
        {
            exception("queue id %i at %08X too large",id,m_queue);
        }
        return(0);
    }
    if(os_queuecheck(id))
    {
        logo(BROWN"{{ %08X -> queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",m_mesg,id,q->num,st.thread,RA.d);
        warning("sendmesg to dead queue %i (block=%i)",id,block);
        mem_write32(m_queue,0);
        return(0);
    }

    cpu_notify_msg(id,m_queue,1);

    if(q->num==q->size)
    { // full!
        q->fullerrors++;
        if(q->fullerrors<50)
        {
            logo("queue %i full!\n",id);
        }
        if(block==-1)
        {
            if(cart.ismario) return(-1); // who cares, mario works
//            exception("queue %i full on OS messge send",id);
            return(-1);
        }
        if(!block) return(-1);
        // force a taskswitch
        if(thread[st.thread].sendblock!=id)
        {
            logo("thread %i blocked on send on queue %i\n",st.thread,id);
        }
        thread[st.thread].sendblock=id;
        blocktask(st.thread);
        return(0);
    }

    logo(BROWN"{{ %08X -> queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",m_mesg,id,q->num,st.thread,RA.d);
//    if(!st.dumpos) logh(BROWN"{{ %08X -> queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",m_mesg,id,q->num,st.thread,RA.d);
    q->totalmsgs++;

    // put message
    q->data[q->pos]=m_mesg;
    q->num++;
    // move pointer
    q->pos++; if(q->pos>=q->size) q->pos=0;

    // check if tasks should be unlocked now
    for(i=0;i<MAXTHREAD;i++)
    {
        if(thread[i].recvblock==id)
        {
            thread[i].recvblock=0;
            logo("thread %i unblocked on receive\n",i);
            unblocktask(i);
        }
    }

    os_queuetomem(id);
    return(0);
}

dword osRecvMesg(dword m_queue,dword mm_mesg,int block)
{
    dword id=mem_read32(m_queue);
    OSQueue *q=queue+id;
    int i,m_mesg,pos;

    st.checkswitch=1;

    os_recvhacks(id);

    cpu_notify_msg(id,m_queue,0);

/*
    if(id==2)
    { // serialQ hack
        // no matter what we return
        return(0);
    }
*/
    if(!id || id>=queuenum)
    {
        if(!id)
        {
            logo("queue at %08X uninitialized, skipping RecvMesg\n",m_queue);
        }
        else
        {
            exception("queue id %i at %08X too large",id,m_queue);
        }
        if(!block) return(-1);
        else return(0);
    }
//    if(!block) exception("nonblocking RecvMesg! CHECK RETURN\n");

    if(os_queuecheck(id))
    {
        logo(BROWN"{{ xxxxxxxx <- queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",id,q->num,st.thread,RA.d);
        warning("recvmesg from dead queue %i",id);
        mem_write32(m_queue,0);
        if(!block) return(-1);
        else return(0);
    }

    if(q->num==0)
    { // empty!
        if(!block)
        {
            // force a taskswitch soon anyhow since this task won't generate any messages
//            forcetaskswitch(0,st.thread);
            return(-1);
        }
        // force a taskswitch, after which sendmesg is re-executed
        if(thread[st.thread].recvblock!=id)
        {
            logo("thread %i blocked on receive on queue %i\n",st.thread,id);
        }
        thread[st.thread].recvblock=id;
        blocktask(st.thread);
        return(0);
    }

    // calc pos
    pos=q->pos-q->num;
    if(pos<0) pos+=q->size;
    // get message
    if(mm_mesg) // could be NULL in which case no write
    {
        m_mesg=q->data[pos];
        mem_write32(mm_mesg,m_mesg);
    }
    else m_mesg=0xffffffff; // ignored
    q->num--;

    logo(BROWN"{{ %08X <- queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",m_mesg,id,q->num,st.thread,RA.d);
//    if(!st.dumpos) logh(BROWN"{{ %08X <- queue %2i (num=%2i, thr=%2i, ra=%08X) }}\n",m_mesg,id,q->num,st.thread,RA.d);

    // check if tasks should be unlocked now
    for(i=0;i<MAXTHREAD;i++)
    {
        if(thread[i].sendblock==id)
        {
            thread[i].sendblock=0;
            logo("thread %i unblocked on send\n",i);
            // force a taskswitch next
            unblocktask(i);
            break;
        }
    }

    os_queuetomem(id);

    return(0);
}

void os_stuffqueue(dword id,dword msg)
{
    if(!id || id>=queuenum)
    {
        exception("queue id too large");
        return;
    }
    osSendMesg(queue[id].memaddr,msg,0);
}

/********************************************************/

void osSetEventMessage(dword ev,dword m_queue,dword mesg)
{
    int queueid=mem_read32(m_queue);
    if(queueid>=queuenum)
    {
        exception("queue id too large");
        return;
    }

    logo(BLUE"SetEventMessage event %i queue %08X mesg %X\n",
            ev,m_queue,mesg);

    event[ev].queueid=queueid;
    event[ev].mesg=mesg;
}

void os_event(dword ev)
{
    int qid,limit;
    if(ev>=MAXEVENTS)
    {
        exception("event too large");
        return;
    }
    qid=event[ev].queueid;
    if(qid)
    {
        limit=queue[qid].size*3/4;
        if(ev==15) limit=limit*3/4;
        if(!limit) limit=1;
        if(queue[qid].num>limit)
        {
            warning("destination queue %i full on os_event %i (limit %i)",qid,ev,limit);
            return; // don't fill queues completely
        }
//        logh("--event %i--\n",ev);
        osSendMesg(queue[qid].memaddr,event[ev].mesg,0);
    }
}

int os_eventqueuefree(dword ev)
{
    if(ev>=MAXEVENTS)
    {
        exception("event too large");
        return(0);
    }
    if(!event[ev].queueid) return(1);
    if(!queue[event[ev].queueid].num) return(1);
    else return(0);
}

void os_clearthreadtime(void)
{
    int i;
    for(i=0;i<MAXTHREAD;i++)
    {
        thread[i].st.threadtime=0;
    }
}

void os_dumpinfo(void) // osinfo
{
    int i;
    print("Captured events:\n");
    for(i=0;i<MAXEVENTS;i++) if(event[i].queueid)
    {
        print("- event %2i = mesg %08X -> queue %2i\n",
            i,event[i].mesg,event[i].queueid);
    }
    print("Message OSQueues:\n");
    for(i=1;i<queuenum;i++)
    {
        print("- queue %2i at %08X size %3i used %3i total %5i\n",
            i,queue[i].memaddr,queue[i].size,queue[i].num,queue[i].totalmsgs);
    }
    print("OSTimers:\n");
    for(i=1;i<timernum;i++)
    {
        if(!timer[i].active)
        {
            print("- timer %2i at %08X inactive (mesg %08X -> queue %08X)\n",
                    i,timer[i].memaddr,timer[i].m_mesg,timer[i].m_queue);
        }
        else
        {
            print("- timer %2i at %08X count %08ius interval %08ius mesg %08X -> queue %08X\n",
                    i,timer[i].memaddr,timer[i].count,timer[i].interval,timer[i].m_mesg,timer[i].m_queue);
        }
    }
    print("OSThreads:\n");
    for(i=1;i<MAXTHREAD;i++) if(thread[i].active)
    {
        print("- thread %2i ip %08X pri %4i realpri %4i cnt %08iK: ",
            i,thread[i].st.pc,thread[i].priority,thread[i].realpriority,
            (int)(thread[i].st.threadtime/1024));
        if(thread[i].sendblock) print("send(q%i) ",thread[i].sendblock);
        if(thread[i].recvblock) print("recv(q%i) ",thread[i].recvblock);
        if(thread[i].ready) print("ready ");
        if(i==st.thread) print("running");
        print("\n");
    }

    {
        qword  tmp;
        int    x;
        double f;
        __int64 lli;
        tmp=thread[1].st.cputime;
        if(!tmp) tmp=1;
        tmp=100*thread[1].st.threadtime/tmp;
        x=(int)tmp;
        print("Idlethread: %2i%%\n",x);
        f=(double)st.frames/60.0;
        print("Frametime : %4.2f sec (%i frames)\n",f,st.frames);
        lli=st.cputime;
        f=(double)lli/93750000.0;
        print("Cputime   : %4.2f sec\n",f);
        print("SP Tasks  : gfx=%i audio=%i\n",st2.gfxpending,st2.audiopending);
    }
}

double os_gettimeus(void)
{
    qint q;
    double d;
    q=(qint)st.retraces*(CLOCKRATE/60);  // total frames
    q+=(st.cputime-st2.retracetime); // within frame
    d=q*(1000000.0/CLOCKRATE);
    return(d);
}

void osGetTime(dword *lo,dword *hi)
{
    qreg x;
    double d;
    x.q=(qint)st.retraces*(CLOCKRATE/60);  // total frames
    x.q+=(st.cputime-st2.retracetime); // within frame
    if(0)
    {
        d=(double)(qint)x.q / CLOCKRATE;
        print("retraces %i, time %.4f sec\n",st.retraces,d);
    }
    *lo=x.d2[0];
    *hi=x.d2[1];
}

/**************/

void osMapMem(dword virt,dword phys,int size)
{
    dword srcmin=phys;
    dword srcmax=phys+size;
    dword srcsize=srcmax-srcmin;
    dword pagebase=virt;
    int i;

    logo(BLUE"- map %08X..%08X (size %08X)",srcmin,srcmax,srcsize);
    logo(BLUE" to  %08X..%08X\n",pagebase,pagebase+srcsize);

    for(i=0;i<srcsize;i+=4096)
    {
        mem_mapphysical(pagebase+i,MAP_RW,srcmin+i);
    }
}

void osMapTLB(dword x,dword pagemask, dword m_ptr, dword a,dword b,dword c)
{
    dword pagesize,pagebase;
    dword srcmin,srcmax,srcsize;
    dword i;

    logo(BLUE"MapTLB(%X,mask:%X,mem:%X,%X,%X,%X) [called at %08X]\n",
        x,pagemask,m_ptr,a,b,c,st.pc);

    switch(pagemask)
    {
        case 0x1ffe000: pagesize=4*4*4*4*4*4*4096; break; // 16M
        case 0x07fe000: pagesize=4*4*4*4*4*4096; break; // 4MB
        case 0x01fe000: pagesize=4*4*4*4*4096; break;
        case 0x007e000: pagesize=4*4*4*4096; break;
        case 0x001e000: pagesize=4*4*4096; break;
        case 0x0006000: pagesize=4*4096; break;
        default:
        case 0x0000000: pagesize=4096; break;
    }
    pagebase=m_ptr;
    srcmin=a;
    srcmax=b;
    srcsize=b-a+pagesize;

    logo(BLUE"- map %08X..%08X (size %08X)",srcmin,srcmax,srcsize);
    logo(BLUE" to  %08X..%08X\n",pagebase,pagebase+srcsize);

    for(i=0;i<srcsize;i+=4096)
    {
        mem_mapphysical(pagebase+i,MAP_RW,srcmin+i);
    }
}

void os_save(FILE *f1)
{
    int i,n;

    for(i=0;i<8;i++)
    {
        putc('O',f1);
        putc('S',f1);
    }

    fwrite(&currentthread,1,4,f1);
    fwrite(&queuenum,1,4,f1);
    fwrite(&threadnum,1,4,f1);
    fwrite(&timernum,1,4,f1);
    n=0;
    fwrite(&n,1,4,f1);
    fwrite(&n,1,4,f1);
    fwrite(&n,1,4,f1);
    fwrite(&n,1,4,f1);

    n=MAXEVENTS;
    fwrite(&n,1,4,f1);
    for(i=0;i<n;i++) fwrite(event+i,1,sizeof(OSEvent),f1);

    n=threadnum;
    fwrite(&n,1,4,f1);
    for(i=0;i<n;i++) fwrite(thread+i,1,sizeof(OSThread),f1);

    n=timernum;
    fwrite(&n,1,4,f1);
    for(i=0;i<n;i++) fwrite(timer+i,1,sizeof(OSTimer),f1);

    n=queuenum;
    fwrite(&n,1,4,f1);
    for(i=0;i<n;i++) fwrite(queue+i,1,sizeof(OSQueue),f1);

    putc(0xfc,f1);
}

void os_load(FILE *f1)
{
    int i,n;

    for(i=0;i<8;i++)
    {
        getc(f1);
        getc(f1);
    }

    currentthread=0;
    fread(&currentthread,1,4,f1);
    fread(&queuenum,1,4,f1);
    fread(&threadnum,1,4,f1);
    fread(&timernum,1,4,f1);
    fread(&n,1,4,f1);
    fread(&n,1,4,f1);
    fread(&n,1,4,f1);
    fread(&n,1,4,f1);

    memset(thread,0,sizeof(OSThread)*MAXTHREAD);
    memset(queue ,0,sizeof(OSQueue )*MAXQUEUE);
    memset(event ,0,sizeof(OSEvent )*MAXEVENTS);
    memset(timer ,0,sizeof(OSTimer )*MAXTIMER);

    fread(&n,1,4,f1);
    for(i=0;i<n;i++) fread(event+i,1,sizeof(OSEvent),f1);
    fread(&n,1,4,f1);
    for(i=0;i<n;i++) fread(thread+i,1,sizeof(OSThread),f1);
    fread(&n,1,4,f1);
    for(i=0;i<n;i++) fread(timer+i,1,sizeof(OSTimer),f1);
    fread(&n,1,4,f1);
    for(i=0;i<n;i++) fread(queue+i,1,sizeof(OSQueue),f1);

    if(getc(f1)!=0xfc)
    {
        exception("os_load fatal error!\n");
    }
}

dword osVirtualToPhysical(dword addr)
{
    dword x;
    x=mem_getphysical(addr);
    if(RA.d==0x80043300)
    {
        logh("<%08X @ ra%08X>\n",
            x,RA.d);
        st.memiodetected=1;
    }
    logo(BLUE"osVirtualToPhysical(%08X)=%08X",addr,x);
    if(x==-1) logo(" (NOT PRESENT)\n");
    else logo("\n");
    return(x);
}

dword osPhysicalToVirtual(dword addr)
{
    dword x;
    x=addr|0x80000000;
    logo(BLUE"osPhysicalToVirtual(%08X)=%08X",addr,x);
    return(x);
}

int os_finddmasource(dword addr)
{
    int i;
    for(i=0;i<DMAHISTORYSIZE;i++)
    {
        if(addr>=st2.dmahistory[i].addr &&
           addr< st2.dmahistory[i].addr+st2.dmahistory[i].size)
        {
            return(i);
        }
    }
    return(-1);
}

int osPiStartDma(dword m_iomsg, int priority, int direction,
                 dword devaddr, dword vaddr, int nbytes, dword m_msgqueue)
{
    dword orgdevaddr=devaddr;
    int save[2],saved,i;

    devaddr&=64*1024*1024-1;
    vaddr&=0xfffffff;

    i=(st2.dmahistorycnt++)&255;
    st2.dmahistorycnt=i;
    st2.dmahistory[i].cart=devaddr;
    st2.dmahistory[i].addr=vaddr;
    st2.dmahistory[i].size=nbytes;
//    print("#dma %08X->%08X (%04X)\n",devaddr,vaddr,nbytes);

    #if 0
    if(!st.codecleared)
    {
        int limit;
        /*
        limit=2*(st.lastdmacount+1);
        if(limit>10) limit=10;
        */
        limit=100;
        st.dmacount++;
        if(cart.iszelda && vaddr<0xa000) limit=-1;
        if(st.dmacount>limit && !cart.ismario)
        {
            print("Over %i DMAs, clearing code cache\n",limit);
            a_clearcache();
            st.codecleared=1;
        }
    }
    #endif

    logo(BLUE"osPiStartDMA %08X (org %08X) -> %08X (%i bytes)\n",
        devaddr,orgdevaddr,vaddr,nbytes);
    if(0 && !st.dumpos && st.dumpinfo)
    {
        logi(BLUE"StartDMA %08X -> %08X (%i bytes)\n",
            devaddr,vaddr,nbytes);
    }
/*
if(devaddr>=0x8CBAA0 && devaddr<=0x944605)
{
    exception("zelda-text");
}
*/
    if(orgdevaddr&0x0c000000)
    {
        exception("DmaCopy cartaddr %08X, changed to %08X, transfer done.\n",
            orgdevaddr,devaddr);
    }

    if(devaddr>cart.size || vaddr>8*1024*1024 || nbytes>cart.size-devaddr)
    {
        os_event(OS_EVENT_PI);
        exception("DmaCopy illegal ranges");
        return(1);
    }

    if(direction!=0)
    {
        exception("DmaCopy write/other");
        return(1);
    }

    if(nbytes<0 || nbytes>16*1024*1024)
    {
        exception("DmaCopy overflow");
        return(1);
    }

    if(nbytes&1)
    {
        //warning("DmaCopy nonaligned size");
        nbytes++;
    }

    if(vaddr&3)
    {
        exception("DmaCopy nonaligned destination");
        return(1);
    }

    if(devaddr&1)
    {
        exception("DmaCopy nonaligned source");
        return(1);
    }

    if(nbytes&2)
    {
        saved=1;
        save[0]=mem_read8(vaddr+nbytes+0);
        save[1]=mem_read8(vaddr+nbytes+1);
        nbytes+=2;
    }
    else saved=0;

    if(devaddr&2)
    {
        int i;
        dword a,x;
        //print(WHITE"Shortaligned source (%08X->%08X)",devaddr,vaddr);
        for(i=0;i<nbytes;i+=4)
        {
            a=devaddr+i;
            x =cart.data[(a+0)^3]<<24;
            x|=cart.data[(a+1)^3]<<16;
            x|=cart.data[(a+2)^3]<<8;
            x|=cart.data[(a+3)^3]<<0;
            mem_write32(vaddr+i,x);
        }
    }
    else
    {
        mem_writerangeraw(vaddr,nbytes,cart.data+devaddr);
    }

    if(saved)
    {
        mem_write8(vaddr+nbytes+0,save[0]);
        mem_write8(vaddr+nbytes+1,save[1]);
        nbytes+=2;
    }

    if(m_msgqueue) osSendMesg(m_msgqueue,0,-1);
    os_event(OS_EVENT_PI);

    if(st.dmatransfers<0x7fff0000)
    {
        st.dmatransfers++;
        inifile_patches(st.dmatransfers);
    }

    return(0);
}

int osEPiStartDma(dword m_pihandle,dword m_iomesg,dword flag)
{
    dword type,dram,dev,count,queue,handle;
    // flag=block?
    //
    // OS_MESG_TYPE_BASE	(10)
    // OS_MESG_TYPE_LOOPBACK	(OS_MESG_TYPE_BASE+0)
    // OS_MESG_TYPE_DMAREAD	(OS_MESG_TYPE_BASE+1)
    // OS_MESG_TYPE_DMAWRITE	(OS_MESG_TYPE_BASE+2)
    // OS_MESG_TYPE_VRETRACE	(OS_MESG_TYPE_BASE+3)
    // OS_MESG_TYPE_COUNTER	(OS_MESG_TYPE_BASE+4)
    // OS_MESG_TYPE_EDMAREAD	(OS_MESG_TYPE_BASE+5)
    // OS_MESG_TYPE_EDMAWRITE	(OS_MESG_TYPE_BASE+6)
    //
    //  m_iomesg points to:
    //  0: type
    //  2: priority
    //  3: return status
    //  4: return queue (notify when done)
    //  8: dram-addr
    // 12: cart-addr
    // 16: count
    // 20: pihandle
    type  =mem_read32(m_iomesg+0);
    queue =mem_read32(m_iomesg+4);
    dram  =mem_read32(m_iomesg+8);
    dev   =mem_read32(m_iomesg+12);
    count =mem_read32(m_iomesg+16);
    handle=mem_read32(m_iomesg+20);
    logo(BLUE"osEPiStartDMA d:%X t:%X h:%08X %08X->%08X,%04X =>\n",flag,type,handle,dev,dram,count);

    if(0)
    {
        int i;
        print("osEPiStartDMA ");
        for(i=0;i<32;i+=4) print("%08X ",mem_read32(m_iomesg+i));
        print("\n");
    }

    if(cart.iszelda && (dev&0xff000000)==0x08000000)
    {
        logo(BLUE"- zelda:ignored.\n");
        /*
        print("Zelda SaveGame access (%i,%08X->%08X,%04X). Ignored.\n",
            flag,dev,dram,count);
        */
        if(queue) osSendMesg(queue,0,-1);
        os_event(OS_EVENT_PI);
        return(0);
    }
    if(1) //type==11)
    {
        osPiStartDma(0,0,0,dev,dram,count,queue);
    }
    else
    {
        logo(BLUE"UNKNOWN TYPE (dram=%08X cart=%08X count=%08X)\n",
            dram,cart,count);
        exception("unsupported EPiStartDMA message\n");
    }
    return(0);
}

int osContInit(dword m_queue,dword m_bitpattern, dword m_status)
{
    int i;

    logo(BLUE"osContInit(%08X,%08X,%08X)\n",m_queue,m_bitpattern,m_status);
    //print(BLUE"osContInit(%08X,%08X,%08X)\n",m_queue,m_bitpattern,m_status);

    for(i=0;i<6;i++)
    {
        mem_write32(m_status+i*4,0);
    }
    // controller 1 present
    mem_write32(m_status+0,0x00010000);
    if(m_bitpattern) mem_write8(m_bitpattern,1<<0);
    if(0)
    {
        // controller 2-4 present
        mem_write32(m_status+4,0x00010000);
        mem_write32(m_status+8,0x00010000);
        mem_write32(m_status+12,0x00010000);
        if(m_bitpattern) mem_write8(m_bitpattern,15<<0);
    }

    return(0);
}

int osContStartReadData(dword m_queue)
{
    logo(BLUE"osContStartReadData\n");
    //print(BLUE"osContStartReadData\n");
    osSendMesg(m_queue,0,-1);
    return(0);
}

void osContGetReadData(dword m_data)
{
    logo(BLUE"osContGetReadData(%08X)\n",m_data);
    //print(BLUE"osContGetReadData(%08X)\n",m_data);
    pad_writedata(m_data);
}

int osContStartQuery(dword m_queue)
{
    static int cnt;
    logo(BLUE"osContStartQuery\n");
    //print(BLUE"osContStartQuery\n");
    osSendMesg(m_queue,0,-1);
    /*
    if(cart.iszelda)
    {
        if(mem_read32(0x800a26c0)==0x54b80003)
        {
            print(WHITE"hacks/zelda: controller hack\n");
            mem_write32(0x800a26c0,0);
            mem_write32(0x800a26c4,0);
        }
    }
    */
    return(0);
}

void osContGetQuery(dword m_data)
{
    logo(BLUE"osContGetQuery(%08X)\n",m_data);
    //print(BLUE"osContGetQuery(%08X)\n",m_data);
    osContInit(0,0,m_data);
}

int osSpTaskYield(void)
{
    logo(BLUE"osSpTaskYield(%08X) (ra=%08X)\n",A0.d,RA.d);
    return(0);
}

int osSpTaskYielded(dword m_task)
{
    logo(BLUE"osSpTaskYielded(%08X) (ra=%08X)\n",A0.d,RA.d);
    return(1); // OS_TASK_YIELDED;
}

void osSpTaskStartGo(dword pos)
{
    OSTask_t task;

    mem_readrangeraw(pos,sizeof(task),(char *)&task);
    logo(BLUE"osSpTaskStartGo %08X (type=%04X,flag=%04X,ucode=%08X/%04X, data=%08X/%04X, yield=%08X/%04X) ",
        pos,task.type,task.flags,
            task.m_ucode,task.ucode_size,
            task.m_data_ptr,task.data_size,
            task.m_yield_data_ptr,task.yield_data_size);
    //logi("RSP Task %04X (ra=%08X, thread=%i)\n",task.type,RA.d,st.thread);
    logo("[pending gfx=%i audio=%i finish=%i]\n",st2.gfxpending,st2.audiopending,st2.gfxfinishpending);

    if(task.type==1)
    {
        if(st2.gfxpending) error("Another gfx task while previous pending!");
        memcpy(&st2.gfxtask,&task,sizeof(OSTask_t));
        st2.gfxpending=1;
    }
    else if(task.type==2)
    {
        if(st2.audiopending) error("Another audio task while previous pending!");
        memcpy(&st2.audiotask,&task,sizeof(OSTask_t));
        st2.audiopending=1;
    }
    else if(task.type==4 && cart.iszelda)
    {
        zlist_uncompress(&task);
        os_event(OS_EVENT_SP); // sp task done
    }
    else
    {
        error("Unknown SP task type %04X\n",task.type);
        os_event(OS_EVENT_SP); // sp task done
    }
}

int osAiSetNextBuffer(dword m_addr,int bytes)
{
    return(slist_nextbuffer(m_addr,bytes));
}

int osAiGetLength(void)
{
    return(slist_getlength());
}

int osAiSetFrequency(dword frequency)
{
    int org=frequency;
    if(frequency==26800)
    {
        print("Timing: PAL (26800 hz audio requested)\n");
        //frequency=32000; // mkart fix
        st.timing=1;
    }
    logo(BLUE"osAiSetFrequency %i hz (%i)\n",frequency,org);
    st.audiorate=frequency;
    return(frequency);
}

/***/

int osSetTimer(dword m_ostimer,dword count_hi,dword count_lo,
               dword interval_hi,dword interval_lo,
               dword m_queue,dword m_mesg)
{
    dword id=mem_read32(m_ostimer);
    dword check=mem_read32(m_ostimer+4);
    int count,interval,i;
    qreg q;

    if(check!=0xfcfc1155)
    {
        id=-1;
    }

    if(id<0 || id>=MAXTIMER)
    {
        for(i=0;i<MAXTIMER;i++)
        {
            if(timer[i].memaddr==m_ostimer)
            {
                id=i;
                break;
            }
        }
        if(i==MAXTIMER)
        {
            id=timernum++;
            if(!id || id>=MAXQUEUE)
            {
                exception("timer id too large");
                return(0);
            }
        }
        mem_write32(m_ostimer,id);
        mem_write32(m_ostimer+4,0xfcfc1155);
    }

    q.d2[0]=count_lo;
    q.d2[1]=count_hi;
    if(!q.q) count=0;
    else
    {
        count=1000000*q.q/(CLOCKRATE);
        if(!count) count=1;
    }

    q.d2[0]=interval_lo;
    q.d2[1]=interval_hi;
    if(!q.q) interval=0;
    else
    {
        interval=1000000*q.q/(CLOCKRATE);
        if(!interval) interval=1;
    }

    logo(BLUE"osSetTimer(%08X,...) id=%i count=%ius interval=%ius msg=%i -> %08X\n",
        m_ostimer,id,count,interval,m_mesg,m_queue);
//print(BLUE"from %08X osSetTimer(%08X,...) id=%i count=%ius (%08X%08X) msg=%i -> %08X\n",
//        RA.d,m_ostimer,id,count,count_hi,count_lo,interval,m_mesg,m_queue);

    timer[id].memaddr=m_ostimer;
    timer[id].count=count;
    timer[id].interval=interval;
    timer[id].m_mesg=m_mesg;
    timer[id].m_queue=m_queue;
    timer[id].queueid=mem_read32(m_queue);
    if(count || interval) timer[id].active=1;
    else timer[id].active=0;

    return(0);
}

void osStopTimer(dword m_ostimer)
{
    dword id=mem_read32(m_ostimer);
    logo(BLUE"osStopTimer(%08X,...)\n",m_ostimer);
    if(!id)
    {
        warning("uninitialized timer");
        return;
    }
    if(!id || id>=MAXQUEUE)
    {
        exception("timer id too large");
        return;
    }
    timer[id].active=0;
}

void os_clearqueues(void)
{
    int i,id;
    for(id=0;id<MAXQUEUE;id++)
    {
        queue[id].num=0;
        // check if tasks should be unlocked now
        for(i=0;i<MAXTHREAD;i++)
        {
            if(thread[i].sendblock==id)
            {
                thread[i].sendblock=0;
                logo("thread %i unblocked on send\n",i);
                // force a taskswitch next
                unblocktask(i);
            }
        }
    }
}

int os_nonidlethread(void)
{
    return(threadnum<2 || thread[st.thread].priority!=0);
}

/**************************************************************************/

void os_recvhacks(int queue)
{
    #if 0
    int i;
    if(cart.ismario)
    {
        if(RA.d==0x80246a68+8)
        {
            st.hackcount++;
            if(st.hackcount==10)
            {
                print(WHITE"hacks/mario: init complete (just a note)\n");
                flushdisplay();
            }
        }
//        mem_write32(0x80226b74,0x220);
//        mem_write32(0x80226b78,0x200);
    }
    if(cart.iszelda)
    {
        if(RA.d==0x80000e50+8)
        {
            st.hackcount++;
            if(st.hackcount==2)
            {
                print("dmatransfers=%i\n",st.dmatransfers);
                print(WHITE"hacks/zelda: init complete, rescanning os routines\n");
                sym_findoscalls(0x800c8000,0x800d8000-0x800c8000,1);
                sym_addpatches();
            }
            if(st.hackcount==3)
            {
                print(WHITE"hacks/zelda: language=1 (English) [also Japanese available!]\n");
                mem_write8(0x8011b9d9,1);
                /*
                print(WHITE"hacks/zelda: audio speed adjustment\n");
                mem_write16(0x80127e7e,0x220);
                mem_write16(0x80127e80,0x250);
                mem_write16(0x80127e82,0x210);
                */
            }
            for(i=0;i<MAXTHREAD;i++)
            {
                if(thread[i].priority>1000)
                {
                    thread[i].priority=1;
                    print(WHITE"hacks/zelda: stopping os thread %i\n",i);
                }
            }
        }
//        mem_write16(0x80127e80,0x230*2); // max audiomix
    }
    #endif
}

void os_taskhacks(int idling)
{ // hacks, called when we would switch to idle thread
    int shownow=0;

    // check if we are looping (not a 100% sure check)
    if(1)
    {
        static int   lastpc;
        static int   lastthread;
        static int   loopcnt;
        static int   idleloopcnt;

        if(st.thread==lastthread && abs(st.pc-lastpc)<16)
        {
            loopcnt++;
            if(loopcnt>10000)
            {
                print("waiting in a loop; Emulation seems stuck.\n");
                loopcnt=0;
            }
        }
        else
        {
            idleloopcnt=0;
            loopcnt=0;
        }

        lastpc=st.pc;
        lastthread=st.thread;
    }
}

void os_dumpqueue(int id)
{
    OSQueue *q=&queue[id];
    int i,pos;
    print("Queue%02i(%2i/%2i): ",id,q->num,q->size);
    for(i=0;i<q->num;i++)
    {
        // calc pos
        pos=q->pos-q->num+i;
        if(pos<0) pos+=q->size;
        print("%X ",q->data[pos]);
    }
    print("\n");
}

