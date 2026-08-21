// os-routines and types

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

// This structure must be packed in order to make the formats compatible with the Ultra SDK
typedef struct {
	uint32_t	type;
	uint32_t	flags;

	uint32_t   m_ucode_boot;
	uint32_t	ucode_boot_size;

	uint32_t   m_ucode;
	uint32_t	ucode_size;

	uint32_t   m_ucode_data;
	uint32_t	ucode_data_size;

	uint32_t   m_dram_stack;
	uint32_t	dram_stack_size;

	uint32_t   m_output_buff;
	uint32_t   m_output_buff_size;

	uint32_t   m_data_ptr;
	uint32_t	data_size;

	uint32_t   m_yield_data_ptr;
	uint32_t	yield_data_size;
} OSTask_t;

#pragma pack(pop)

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

void osCreateThread(uint32_t m_thread,uint32_t id, uint32_t m_routine,
                    uint32_t m_dunno,uint32_t m_stack,uint32_t priority);

void osStartThread(uint32_t m_thread);
void osSetThreadPri(uint32_t m_thread,int pri);

void  osCreateMesgQueue(uint32_t m_queue,uint32_t m_mesg,uint32_t size);
uint32_t osSendMesg(uint32_t m_queue,uint32_t m_mesg,int block);
uint32_t osRecvMesg(uint32_t m_queue,uint32_t mm_mesg,int block);

void osGetTime(uint32_t *lo,uint32_t *hi);

void osSetEventMessage(uint32_t ev,uint32_t m_queue,uint32_t mesg);
void os_event(uint32_t ev);
void os_stuffqueue(uint32_t qid,uint32_t msg);
void os_switchcheck(void);

void osMapTLB(uint32_t x,uint32_t pagemask, uint32_t m_ptr, uint32_t a,uint32_t b,uint32_t c);

void os_dumpinfo(void);

void  os_save(FILE *f1);
void  os_load(FILE *f1);

uint32_t osVirtualToPhysical(uint32_t addr);
uint32_t osPhysicalToVirtual(uint32_t addr);

int osPiStartDma(uint32_t m_iomsg, int priority, int direction,
                 uint32_t devaddr, uint32_t vaddr, int nbytes, uint32_t m_msgqueue);

int osEPiStartDma(uint32_t m_pihandle,uint32_t m_iomesg,uint32_t flag);

void os_framesync(void);
void os_framewait(void);

void osSkip(uint32_t pc,uint32_t a,uint32_t b,uint32_t c,uint32_t d);

int osAiSetFrequency(uint32_t frequency);
int osAiGetLength(void);
int osAiSetNextBuffer(uint32_t m_addr,int bytes);

void osStopTimer(uint32_t m_ostimer);

int osSetTimer(uint32_t m_ostimer,uint32_t count_hi,uint32_t count_lo,
               uint32_t interval_hi,uint32_t interval_lo,
               uint32_t m_queue,uint32_t m_mesg);

int  osContStartReadData(uint32_t m_queue);
void osContGetReadData(uint32_t m_data);
int  osContInit(uint32_t m_queue,uint32_t m_bitpattern, uint32_t m_status);
int  osContStartQuery(uint32_t m_data);
void osContGetQuery(uint32_t m_data);

void os_updatestats(int skip);

int  os_nonidlethread(void);

int  osSpTaskYield(void);
int  osSpTaskYielded(uint32_t m_task);
void osSpTaskStartGo(uint32_t pos);

// game specific hacks (called internally)
void os_taskhacks(int idling);

int os_eventqueuefree(uint32_t ev);

int os_finddmasource(uint32_t addr);

void osStopCurrentThread(void);

void os_dumpqueue(int id);
void os_resettimers(void);

void os_clearthreadtime(void);
void os_clearqueues(void);
void os_timers(void);

void os_init(void);

void osMapMem(uint32_t virt,uint32_t phys,int size);
double os_gettimeus(void);

#ifdef __cplusplus
};
#endif
