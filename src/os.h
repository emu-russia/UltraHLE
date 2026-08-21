// os-routines and types

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/**
 * Packed OSTask structure compatible with the Ultra SDK.
 */
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

/**
 * Creates an OS thread with the given id, entry routine, stack and priority.
 * @param m_thread Address of the thread control block in emulated memory.
 * @param id Thread id.
 * @param m_routine Entry point of the thread.
 * @param m_dunno Parameter passed to the thread entry routine.
 * @param m_stack Stack pointer for the thread.
 * @param priority Thread priority.
 */
void osCreateThread(uint32_t m_thread,uint32_t id, uint32_t m_routine,
                    uint32_t m_dunno,uint32_t m_stack,uint32_t priority);

/**
 * Makes the given thread ready to run.
 * @param m_thread Address of the thread control block.
 */
void osStartThread(uint32_t m_thread);
/**
 * Sets the priority of a thread and reschedules.
 * @param m_thread Address of the thread control block.
 * @param pri New thread priority.
 */
void osSetThreadPri(uint32_t m_thread,int pri);

/**
 * Creates an OS message queue at the given address.
 * @param m_queue Address of the queue control block in emulated memory.
 * @param m_mesg Address of the message buffer.
 * @param size Maximum number of messages the queue can hold.
 */
void  osCreateMesgQueue(uint32_t m_queue,uint32_t m_mesg,uint32_t size);
/**
 * Sends a message to an OS queue.
 * @param m_queue Address of the queue.
 * @param m_mesg Message to send.
 * @param block Blocking flag: 1 blocks the caller when the queue is full, 0 and -1 do not block.
 * @return 0 on success, -1 on failure.
 */
uint32_t osSendMesg(uint32_t m_queue,uint32_t m_mesg,int block);
/**
 * Receives a message from an OS queue.
 * @param m_queue Address of the queue.
 * @param mm_mesg Address where the received message is written.
 * @param block Blocking flag: -1 blocks, 0 non-blocking.
 * @return 0 on success, -1 on failure.
 */
uint32_t osRecvMesg(uint32_t m_queue,uint32_t mm_mesg,int block);

/**
 * Returns the current emulated system time as a 64-bit value.
 * @param lo Lower 32 bits of the time.
 * @param hi Upper 32 bits of the time.
 */
void osGetTime(uint32_t *lo,uint32_t *hi);

/**
 * Associates a message with the given OS event.
 * @param ev Event id.
 * @param m_queue Address of the queue that receives the event message.
 * @param mesg Message sent when the event occurs.
 */
void osSetEventMessage(uint32_t ev,uint32_t m_queue,uint32_t mesg);
/**
 * Triggers an OS event, sending its message to the event queue.
 * @param ev Event id.
 */
void os_event(uint32_t ev);
/**
 * Puts a message into the queue with the given id.
 * @param qid Queue id.
 * @param msg Message to put.
 */
void os_stuffqueue(uint32_t qid,uint32_t msg);
/**
 * Checks whether a thread switch is required and performs it.
 * Called from the simulator loop.
 */
void os_switchcheck(void);

/**
 * Maps a range of physical memory into the virtual address space.
 * @param x Unused TLB parameter.
 * @param pagemask Page size mask.
 * @param m_ptr Virtual base address of the mapping.
 * @param a Physical start address.
 * @param b Physical end address.
 * @param c Unused parameter.
 */
void osMapTLB(uint32_t x,uint32_t pagemask, uint32_t m_ptr, uint32_t a,uint32_t b,uint32_t c);

/**
 * Prints information about the emulated OS state.
 */
void os_dumpinfo(void);

/**
 * Saves the OS state to a file.
 * @param f1 File to save to.
 */
void  os_save(FILE *f1);
/**
 * Loads the OS state from a file.
 * @param f1 File to load from.
 */
void  os_load(FILE *f1);

/**
 * Translates a virtual address to its physical address.
 * @param addr Virtual address.
 * @return Physical address, or -1 if not present.
 */
uint32_t osVirtualToPhysical(uint32_t addr);
/**
 * Translates a physical address to a virtual address.
 * @param addr Physical address.
 * @return Corresponding virtual address.
 */
uint32_t osPhysicalToVirtual(uint32_t addr);

/**
 * Starts a PI DMA transfer between the cartridge and RDRAM.
 * @param m_iomsg I/O message.
 * @param priority DMA priority.
 * @param direction Transfer direction (0 = read from the cartridge).
 * @param devaddr Cartridge device address.
 * @param vaddr RDRAM destination address.
 * @param nbytes Number of bytes to transfer.
 * @param m_msgqueue Queue notified when the transfer finishes.
 * @return 0 on success, 1 on error.
 */
int osPiStartDma(uint32_t m_iomsg, int priority, int direction,
                 uint32_t devaddr, uint32_t vaddr, int nbytes, uint32_t m_msgqueue);

/**
 * Starts an EPI DMA transfer described by an I/O message.
 * @param m_pihandle PI handle address.
 * @param m_iomesg Address of the I/O message with the transfer parameters.
 * @param flag Blocking flag.
 * @return 0 on success.
 */
int osEPiStartDma(uint32_t m_pihandle,uint32_t m_iomesg,uint32_t flag);

/**
 * Synchronizes the emulation to the frame.
 */
void os_framesync(void);
/**
 * Waits for the current frame to complete.
 */
void os_framewait(void);

/**
 * Skips the instruction stream from the given program counter.
 * @param pc Program counter to skip from.
 * @param a Skip parameter.
 * @param b Skip parameter.
 * @param c Skip parameter.
 * @param d Skip parameter.
 */
void osSkip(uint32_t pc,uint32_t a,uint32_t b,uint32_t c,uint32_t d);

/**
 * Sets the audio interface frequency.
 * @param frequency Requested frequency in Hz.
 * @return The frequency that was set.
 */
int osAiSetFrequency(uint32_t frequency);
/**
 * Returns the length of the current AI audio buffer.
 * @return Length of the current audio buffer.
 */
int osAiGetLength(void);
/**
 * Sets the next AI audio buffer.
 * @param m_addr Address of the audio buffer.
 * @param bytes Size of the buffer in bytes.
 * @return 0 on success, -1 if the audio system is busy.
 */
int osAiSetNextBuffer(uint32_t m_addr,int bytes);

/**
 * Stops the given OS timer.
 * @param m_ostimer Address of the timer control block.
 */
void osStopTimer(uint32_t m_ostimer);

/**
 * Sets an OS timer with the given count and interval.
 * @param m_ostimer Address of the timer control block.
 * @param count_hi High 32 bits of the initial count.
 * @param count_lo Low 32 bits of the initial count.
 * @param interval_hi High 32 bits of the repeat interval.
 * @param interval_lo Low 32 bits of the repeat interval.
 * @param m_queue Queue notified when the timer expires.
 * @param m_mesg Message sent when the timer expires.
 * @return 0 on success.
 */
int osSetTimer(uint32_t m_ostimer,uint32_t count_hi,uint32_t count_lo,
               uint32_t interval_hi,uint32_t interval_lo,
               uint32_t m_queue,uint32_t m_mesg);

/**
 * Starts reading controller data.
 * @param m_queue Queue notified when the read finishes.
 * @return 0 on success.
 */
int  osContStartReadData(uint32_t m_queue);
/**
 * Gets the controller data from the last read.
 * @param m_data Address where the controller data is written.
 */
void osContGetReadData(uint32_t m_data);
/**
 * Initializes the emulated controllers.
 * @param m_queue Queue for controller messages.
 * @param m_bitpattern Address where the bit pattern of the present controllers is written.
 * @param m_status Address where the controller status is written.
 * @return 0 on success.
 */
int  osContInit(uint32_t m_queue,uint32_t m_bitpattern, uint32_t m_status);
/**
 * Starts querying the controllers.
 * @param m_data Queue notified when the query finishes.
 * @return 0 on success.
 */
int  osContStartQuery(uint32_t m_data);
/**
 * Gets the result of the last controller query.
 * @param m_data Address where the query result is written.
 */
void osContGetQuery(uint32_t m_data);

/**
 * Updates the emulation statistics.
 * @param skip Skip flag for the update.
 */
void os_updatestats(int skip);

/**
 * Checks whether the current thread is a non-idle thread.
 * @return 1 if the current thread has a non-zero priority or no other threads exist, 0 otherwise.
 */
int  os_nonidlethread(void);

/**
 * Yields the current SP task.
 * @return 0 on success.
 */
int  osSpTaskYield(void);
/**
 * Checks whether the SP task has been yielded.
 * @param m_task Address of the SP task.
 * @return 1 if the task was yielded.
 */
int  osSpTaskYielded(uint32_t m_task);
/**
 * Starts the SP task stored at the given address.
 * @param pos Address of the SP task structure.
 */
void osSpTaskStartGo(uint32_t pos);

/**
 * Applies game specific hacks when the idle thread is about to run.
 * @param idling 1 if switching to the idle thread.
 */
// game specific hacks (called internally)
void os_taskhacks(int idling);

/**
 * Checks whether the queue of the given event is empty.
 * @param ev Event id.
 * @return 1 if the event queue is free, 0 otherwise.
 */
int os_eventqueuefree(uint32_t ev);

/**
 * Finds the DMA history entry that covers the given address.
 * @param addr Address to look up.
 * @return Index of the DMA history entry, or -1 if not found.
 */
int os_finddmasource(uint32_t addr);

/**
 * Stops the currently running thread.
 */
void osStopCurrentThread(void);

/**
 * Prints the contents of the queue with the given id.
 * @param id Queue id.
 */
void os_dumpqueue(int id);
/**
 * Resets the OS timer state.
 */
void os_resettimers(void);

/**
 * Clears the recorded execution time of all threads.
 */
void os_clearthreadtime(void);
/**
 * Clears all message queues and unblocks threads waiting on them.
 */
void os_clearqueues(void);
/**
 * Updates the OS timers and sends messages for expired ones.
 */
void os_timers(void);

/**
 * Initializes the OS emulation state.
 */
void os_init(void);

/**
 * Maps a range of physical memory into the virtual address space.
 * @param virt Virtual base address.
 * @param phys Physical base address.
 * @param size Size of the mapping in bytes.
 */
void osMapMem(uint32_t virt,uint32_t phys,int size);
/**
 * Returns the emulated system time in microseconds.
 * @return Current system time in microseconds.
 */
double os_gettimeus(void);

#ifdef __cplusplus
};
#endif
