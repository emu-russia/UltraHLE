# Notes on Ultra OS emulation

## sym.c

Symbol table and OS call detection module. Manages a symbol table (`sym[]`) that maps memory addresses to human-readable names and optional patch codes, and uses CRC-based routine detection to identify OS calls within game code.

### Data structures

- `OSCall` — per-OS-routine entry with 8 data pattern dwords, two CRC values, and metadata (symbol name, patch number, match class, etc.)
- `Sym` — per-symbol entry with address, name text, and patch code
- `oscall[]` — global array of `OSCall` entries, populated from `oscall.h` at startup
- `sym[]` / `symnum` — global symbol table and entry count

### Key functions

| Function | Purpose |
|---|---|
| `sym_clear()` | Reset symbol table and clear `oscall` match states |
| `sym_add()` | Add or replace a symbol at a given address; respects `disablepatches` and game-mode checks (Mario/Zelda) |
| `sym_del()` | Remove a symbol by address |
| `sym_find()` | Look up symbol name for an address; resolves offsets within the symbol's range |
| `sym_load()` | Parse a `.sym` file (format: `ADDRESS name#patch`), detect game title, set flags (`ismario`, `iszelda`) |
| `sym_save()` | Stub — not implemented |
| `sym_dump()` | Print all symbols to console |
| `sym_findoscalls()` | Scan a memory range for known OS calls by matching data patterns and CRC checksums; classifies matches (class 1 = CRC1 match, class 2 = CRC1+CRC2 match) |
| `sym_findfirstos()` | Discover OS calls in the OS range or code base, then resolve patch names |
| `sym_patchnames()` | Auto-assign patch numbers from `ospatch.h` patterns to symbols that lack them |
| `sym_dumposcalls()` | Print found/not-found OS routine list to console |
| `sym_demooscalls()` | Development helper — scans `demo.rom` to generate `oscall.h` |
| `routinecrc2()` | Compute two CRC-like checksums over the first 16 instructions of a routine |

### Patch integration

Symbols with a patch code (`#N` suffix) receive a dummy `OP_PATCH` opcode inserted at their target address via `sym_addpatches()`. When the emulator hits this opcode, `op_patch()` in `patch.c` dispatches to the corresponding stub in `patchtable[]`. `sym_removepatches()` restores the original instructions.

## patch.c

Contains an implementation of Stubs to patch OS/library calls. May also contain patches for game-specific routines (e.g. `p_golden1`).

The patch is quite simple:
- A special dummy opcode (OP_PATCH) is inserted at the beginning of the procedure
- The old opcode is saved
- As soon as the processor enters the patched procedure and executes the OP_PATCH opcode, Stub is called with the corresponding number.

A list of all Stubs is contained in the `patchtable` table.

The implementation of the dummy opcode is in `op_patch`.

Simple patches are implemented directly inside Stubs, more complex OS-calls are redirected to os.c

## os.c

Ultra N64 Operating System emulation core. Implements threads, message queues, events, timers, TLB mapping, PI DMA, AI, controller, and SP task interfaces.

### Data structures (packed for SDK compatibility)

| Struct | Purpose |
|---|---|
| `OSThread` | Thread state: PC, SP, priority, blocking queues, saved `State` (CPU context) |
| `OSQueue` | Message queue: circular buffer with `memaddr`, `size`, `pos`, `num` |
| `OSEvent` | Event handler: target queue ID and message to deliver |
| `OSTimer` | Software timer: count, interval, active flag, target queue/message |
| `OSTask_t` | SP task descriptor (in `os.h`): ucode/boot/yield pointers, data/output buffers |

### Globals

- `thread[MAXTHREAD]` / `threadnum` — thread array and count
- `currentthread` — index of the currently running thread
- `queue[MAXQUEUE]` / `queuenum` — message queue array and count
- `timer[MAXTIMER]` / `timernum` — timer array and count
- `event[MAXEVENTS]` — event handler array
- `CLOCKRATE` — 45637500 Hz (PI bus clock)
- `THREADSLICE` — timeslice per thread = `CYCLES_BURST*2-1`

### Key functions

#### Thread management

| Function | Purpose |
|---|---|
| `osCreateThread()` | Create a thread: set PC, SP (stack-8), return address (0x3F000000), priority×10 |
| `osStartThread()` | Mark thread ready and force a task switch |
| `osStopCurrentThread()` | Deactivate current thread and force reschedule |
| `osSetThreadPri()` | Change thread priority and reschedule |
| `os_switchthread()` | Save `State` to thread, restore new thread's `State`, update `currentthread` |
| `os_findthread()` | Find highest-priority ready thread (respects `trythread`/`avoidthread`) |
| `os_switchcheck()` | Assign timeslice, check idle thread, perform context switch if needed |
| `forcetaskswitch()` | Set `bailout=-1` to trigger task switch in next CPU step |
| `blocktask()` / `unblocktask()` | Mark thread blocked/resumed, trigger switch |

#### Message queues

| Function | Purpose |
|---|---|
| `osCreateMesgQueue()` | Initialize queue: find free slot (reuses dead queues), set size, write to memory |
| `osSendMesg()` | Enqueue message; if full and blocking, block sender thread |
| `osRecvMesg()` | Dequeue message; if empty and blocking, block receiver thread |
| `os_queuetomem()` | Sync queue state (id, num, size) back to guest memory |
| `os_queuecheck()` | Validate queue integrity via magic value `0xFCFC1234` |
| `os_clearqueues()` | Drain all queues and unblock senders |

#### Events

| Function | Purpose |
|---|---|
| `osSetEventMessage()` | Associate an event with a queue + message |
| `os_event()` | Deliver the event message to the target queue (with queue-full protection) |

#### Timers

| Function | Purpose |
|---|---|
| `osSetTimer()` | Configure timer: convert cycles to microseconds, set count/interval, activate |
| `osStopTimer()` | Deactivate a timer |
| `os_timers()` | Per-frame timer tick: decrement counts, fire messages when expired, reschedule |

#### TLB / memory mapping

| Function | Purpose |
|---|---|
| `osMapTLB()` | Map physical range `[a..b]` to virtual `pagebase` at given page size (4 KB .. 16 MB) |
| `osMapMem()` | Map physical-to-virtual in 4 KB pages |
| `osVirtualToPhysical()` | Translate virtual address to physical via `mem_getphysical()` |
| `osPhysicalToVirtual()` | Translate physical to virtual (KSEG0: `addr \| 0x80000000`) |

#### PI (Peripheral Interface) DMA

| Function | Purpose |
|---|---|
| `osPiStartDma()` | DMA from cart to DRAM: handle alignment, byte-swapping, address validation, send completion message |
| `osEPiStartDma()` | Higher-level PI DMA wrapper; decodes `m_iomesg` struct (type, dram-addr, cart-addr, count), dispatches to `osPiStartDma` |

#### AI (Audio Interface)

| Function | Purpose |
|---|---|
| `osAiSetFrequency()` | Set audio sample rate; detects PAL timing (26800 Hz) |
| `osAiSetNextBuffer()` | Forward to `slist_nextbuffer()` (audio sample list) |
| `osAiGetLength()` | Forward to `slist_getlength()` |

#### Controller

| Function | Purpose |
|---|---|
| `osContInit()` | Initialize controller: mark controller 1 present in status buffer |
| `osContStartQuery()` / `osContStartReadData()` | Start query/read, send completion message |
| `osContGetQuery()` | Return controller type info (calls `osContInit`) |
| `osContGetReadData()` | Forward to `pad_writedata()` |

#### SP (Signal Processor) tasks

| Function | Purpose |
|---|---|
| `osSpTaskStartGo()` | Load `OSTask_t`, classify as gfx (type 1), audio (type 2), or compressed (type 4/Zelda), set pending flags |
| `osSpTaskYield()` / `osSpTaskYielded()` | Yield support for SP tasks |
| `osSpTaskLoad()` | Set `sptaskload=1` flag before `osSpTaskStartGo` |

#### Time

| Function | Purpose |
|---|---|
| `osGetTime()` | Return elapsed time as 64-bit value derived from frame count × clock/60 |
| `os_gettimeus()` | Return elapsed time in microseconds |

#### Save/Load state

| Function | Purpose |
|---|---|
| `os_save()` | Write OS state header (`OS`×8), thread/queue/timer/event counts and data |
| `os_load()` | Restore OS state from save file |

#### Internal / debug

| Function | Purpose |
|---|---|
| `os_init()` | Reset all OS subsystems to initial state |
| `os_dumpinfo()` | Print events, queues, timers, threads, idle %, frametime, CPU time, SP pending counts |
| `os_dumpqueue()` | Print messages in a specific queue |
| `os_taskhacks()` | Detect infinite loops when idle thread would run |
| `os_resettimers()` / `os_clearthreadtime()` | Reset timers / per-thread CPU time counters |

## Important calls

A patch with a number >= 10 is considered important.

List:
- osGetCount #48
- osInvalICache #38
- osMapTLB #24
- osCreateMesgQueue #13
- osCreateThread #10
- osGetTime #19
- osJamMesg #15
- osPhysicalToVirtual #29
- osRecvMesg #14
- osSendMesg #15
- osSetEventMesg #16
- osSetThreadPri #18
- osSetTimer #33
- osStartThread #11
- osStopThread #28
- osStopTimer #34
- osVirtualToPhysical #21
- osAiGetLength #31
- osAiSetFrequency #32
- osAiSetNextBuffer #30
- osSpTaskLoad #39
- osSpTaskStartGo #22
- osSpTaskYield #46
- osSpTaskYielded #47
- osViGetCurrentFramebuffer #20
- osViGetNextFramebuffer #20
- osCreateViManager #55
- osViSetEvent #17
- osViSwapBuffer #23
- osContStartQuery #42
- osContGetQuery #43
- osContStartReadData #26
- osContGetReadData #27
- osContInit #25
- osEPiStartDma #37
- osPiStartDma #12
