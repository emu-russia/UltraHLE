typedef struct {
	dword	type;
	dword	flags;

	dword   m_ucode_boot;
	dword	ucode_boot_size;

	dword   m_ucode;
	dword	ucode_size;

	dword   m_ucode_data;
	dword	ucode_data_size;

	dword   m_dram_stack;
	dword	dram_stack_size;

	dword   m_output_buff;
	dword   m_output_buff_size;

	dword   m_data_ptr;
	dword	data_size;

	dword   m_yield_data_ptr;
	dword	yield_data_size;
} OSTask_t;

#define OS_EVENT_SW1              0     /* CPU SW1 interrupt */
#define OS_EVENT_SW2              1     /* CPU SW2 interrupt */
#define OS_EVENT_CART             2     /* Cartridge interrupt: used by rmon */
#define OS_EVENT_COUNTER          3     /* Counter int: used by VI/Timer Mgr */
#define OS_EVENT_SP               4     /* SP task done interrupt */
#define OS_EVENT_SI               5     /* SI (controller) interrupt */
#define OS_EVENT_AI               6     /* AI interrupt */
#define OS_EVENT_VI               7     /* VI interrupt: used by VI/Timer Mgr */
#define OS_EVENT_PI               8     /* PI interrupt: used by PI Manager */
#define OS_EVENT_DP               9     /* DP full sync interrupt */
#define OS_EVENT_CPU_BREAK        10    /* CPU breakpoint: used by rmon */
#define OS_EVENT_SP_BREAK         11    /* SP breakpoint:  used by rmon */
#define OS_EVENT_FAULT            12    /* CPU fault event: used by rmon */
#define OS_EVENT_THREADSTATUS     13    /* CPU thread status: used by rmon */
#define OS_EVENT_PRENMI           14    /* Pre NMI interrupt */
#define OS_EVENT_RETRACE          15    /* Retrace (actually not an OS event but a VI event) */

void osCreateThread(dword m_thread,dword id, dword m_routine,
                    dword m_dunno,dword m_stack,dword priority);

void osStartThread(dword m_thread);
void osSetThreadPri(dword m_thread,int pri);

void  osCreateMesgQueue(dword m_queue,dword m_mesg,dword size);
dword osSendMesg(dword m_queue,dword m_mesg,int block);
dword osRecvMesg(dword m_queue,dword mm_mesg,int block);

void osGetTime(dword *lo,dword *hi);

void osSetEventMessage(dword ev,dword m_queue,dword mesg);
void os_event(dword ev);
void os_stuffqueue(dword qid,dword msg);
void os_switchcheck(void);

void osMapTLB(dword x,dword pagemask, dword m_ptr, dword a,dword b,dword c);

void os_dumpinfo(void);

void  os_save(FILE *f1);
void  os_load(FILE *f1);

dword osVirtualToPhysical(dword addr);
dword osPhysicalToVirtual(dword addr);

int osPiStartDma(dword m_iomsg, int priority, int direction,
                 dword devaddr, dword vaddr, int nbytes, dword m_msgqueue);

int osEPiStartDma(dword m_pihandle,dword m_iomesg,dword flag);

void os_framesync(void);
void os_framewait(void);

void osSkip(dword pc,dword a,dword b,dword c,dword d);

int osAiSetFrequency(dword frequency);
int osAiGetLength(void);
int osAiSetNextBuffer(dword m_addr,int bytes);

void osStopTimer(dword m_ostimer);

int osSetTimer(dword m_ostimer,dword count_hi,dword count_lo,
               dword interval_hi,dword interval_lo,
               dword m_queue,dword m_mesg);

int  osContStartReadData(dword m_queue);
void osContGetReadData(dword m_data);
int  osContInit(dword m_queue,dword m_bitpattern, dword m_status);
int  osContStartQuery(dword m_data);
void osContGetQuery(dword m_data);

void os_updatestats(int skip);

int  os_nonidlethread(void);

int  osSpTaskYield(void);
int  osSpTaskYielded(dword m_task);
void osSpTaskStartGo(dword pos);

// game specific hacks (called internally)
void os_taskhacks(int idling);
void os_recvhacks(int queue);

int os_eventqueuefree(dword ev);

int os_finddmasource(dword addr);

void osStopCurrentThread(void);

void os_dumpqueue(int id);
void os_resettimers(void);

void os_clearthreadtime(void);
void os_clearqueues(void);
void os_timers(void);

void os_init(void);

void osMapMem(dword virt,dword phys,int size);
double os_gettimeus(void);

