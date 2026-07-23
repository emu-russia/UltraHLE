# Console commands

## Main commands

### help
Displays the full list of available console commands and keyboard shortcuts.

### x, exit
Exits the emulator.

### reset
Resets/restarts the currently loaded ROM.

### save [<file>]
Saves the emulator state to a file (~4MB). If no filename is provided, saves to `ultra.sav`. Automatically renames the previous `ultra.sav` to `ultrabak.sav`.

### rom <file>
Loads a new ROM file.

### load [<file>]
Loads the emulator state from a save file. If no filename is provided, loads from `ultra.sav`. Re-enables graphics after loading.

### go
Starts execution using the JIT compiler (fast mode).

### sgo
Starts execution using the C interpreter (slow mode, useful for debugging).

## Step commands

### skip <count>
Skips forward by `<count>` instructions by directly modifying the program counter.

### goto <addr>
Sets the program counter (PC) to the specified address.

### s [count]
Steps `<count>` instructions using the C emulator simulator (default: 1).

### f [count]
Steps `<count>` instructions using the JIT compiler simulator (default: 1).

### n
Executes until the next instruction (steps over calls).

### nr
Executes until the next return (stops at the end of the current routine).

### nc
Executes until the next function call.

### t <addr>
Executes until the PC reaches the specified address (execute to breakpoint).

### mw <addr>
Sets a memory write breakpoint at `<addr>`. Executes until memory at that address is written.

### mr <addr>
Sets a memory read breakpoint at `<addr>`. Executes until memory at that address is read.

### ma <addr>
Sets a memory access breakpoint at `<addr>`. Executes until memory at that address is accessed (read or write).

### mc <addr> [value]
Sets a memory change breakpoint at `<addr>`. Executes until memory at that address changes (optionally compares against `<value>`).

### nt
Executes until the next thread switch.

### nf
Executes until the next frame.

### nm <queue>
Executes until the next message operation in the specified SPI queue.

### mw <addr>
Execute until memory at `<addr>` is written.

### mr <addr>
Execute until memory at `<addr>` is read.

### ma <addr>
Execute until memory at `<addr>` is accessed.

### mc <addr>
Execute until memory at `<addr>` is changed.

## Dump commands

### info [0|1]
Toggles or sets generic info dumping to console (0=off, 1=on).

### os [0|1]
Toggles or sets OS call info dumping to `ultra.log` (0=off, 1=on).

### hw [0|1]
Toggles or sets hardware info dumping to `ultra.log` (0=off, 1=on).

### ops [0|1]
Toggles or sets full RSP operation trace info dumping to console (0=off, 1=on).

### asm [0|1]
Toggles or sets JIT compiler assembly info dumping (0=off, 1=on).

### gfx [0|1]
Toggles or sets graphics/dlist info dumping to `dlist.log` (0=off, 1=on).

### snd [0|1]
Toggles or sets sound/slist info dumping to `slist.log` (0=off, 1=on).

### wav [0|1]
Toggles or sets WAV audio dumping (0=off, 1=on).

### trace [0|1]
Toggles or sets execution trace dumping to console (0=off, 1=on).

### all [0|1]
Toggles all dump flags on (1) or off (0) at once.

### stop [0|1|2]
Sets stopping behavior: 0=exceptions only, 1=exceptions+errors (default), 2=exceptions+errors+warnings.

## Exam commands

### .
Disassembles code at the current program counter.

### u <addr>
Disassembles code at the specified address.

### d <addr>
Displays data at the specified address in the data viewer.

### e <addr> <value>
Writes a 32-bit DWORD `<value>` to memory at `<addr>`.

### eb <addr> <value>
Writes an 8-bit BYTE `<value>` to memory at `<addr>`.

### ss <string>
Searches RDRAM for the specified ASCII string and prints all matching addresses.

### ssmario <string>
Searches RDRAM for Mario 64-specific patterns (used for finding character model data).

### snap
Takes a snapshot of the current RDRAM memory for later comparison.

### snaphw <old_val> <new_val>
Finds halfwords in RDRAM that changed from `<old_val>` to `<new_val>` since the last `snap`.

### fill <addr> <count> <byte>
Fills `<count>` bytes at `<addr>` with the specified `<byte>` value.

### filltst <addr>
Fills 32 consecutive halfwords at `<addr>` with test pattern values (0x30-0x4F).

### ssr <string>
Searches RDRAM for a string using relative character matching (finds strings with same character offsets regardless of base value).

### regs
Toggles the display of integer/FPU registers (also bound to F6 key).

### setreg <reg> <value>
Sets integer register `<reg>` (0-31) to the specified `<value>`.

## Sym commands

### saveoscalls
Saves detected OS call addresses to the symbol file.

### findos
Rescans memory for Nintendo OS routines and reloads symbol data.

### listos
Lists all currently detected OS call routines.

### sym
Dumps all loaded symbols to the console.

### addos
(Disabled in current build) Adds an OS routine entry with a custom name at the given address.

## Misc commands

### pad <0-3>
Selects which controller (0-3) to use.

### keys [0|1]
Toggles keyboard input (0=disabled, 1=enabled).

### hide3dfx
Hides the 3DFX graphics output (useful when running without a 3DFX Voodoo card). Closes the RDP display.

### show3dfx
Restores 3DFX graphics output.

### swap
Forces a framesync (renders a single frame immediately).

### sound [0|1]
Toggles sound output (0=disabled, 1=enabled).

### pimem
Prints the contents of the PI (Processor Interface) registers.

### soundsync [0|1]
Toggles sound synchronization (when enabled, audio sync slows down graphics to match; 0=disabled, 1=enabled).

### gfxthread [0|1]
Toggles running graphics in a separate thread (0=disabled, 1=enabled).

### graphics [0|1]
Toggles graphics output (0=disabled, 1=enabled).

### zeldajap
Patches Zelda to use Japanese language (works only with Zelda carts).

### zeldaeng
Patches Zelda to use English language (works only with Zelda carts).

### camera <x> <y> <z>
Manually moves the camera position (DLIST hack for debugging graphics).

### resolution <width>
Changes the emulator's display resolution. Supported widths: 320-2048 (default: 640). Height is auto-calculated as 480 * width / 640.

### wireframe <0|1>
Sets graphics wireframe rendering mode (0=off, 1=on).

### boot
Resets the RSP microcode by copying cart data to DMEM/IMEM and jumping to the microcode entry point.

### memory
Displays a visual map of RDRAM usage (C=code, d=data, e=nearly empty, .=empty).

### group [addr]
Disassembles the X86 compiled group at `<addr>` (or current PC if no address given). Shows expansion ratio and compiled x86 instructions.

### compile [addr]
Compiles RSP code at `<addr>` (or current PC) to x86 and disassembles it.

### disasm <file> <base> <count>
Disassembles memory from `<base>` for `<count>` bytes and writes to `<file>`.

### disasmrsp <file> <base> <count>
Disassembles RSP microcode from `<base>` for `<count>` instructions and writes to `<file>`.

### savemem <file> <base> <count>
Saves memory from `<base>` for `<count>` bytes to `<file>`.

### stat
Displays JIT compiler statistics.

### stat2
Lists used RSP opcodes in the compiler.

### stat3
Displays the CPU execution profile (must be compiled with profiling enabled in cpua.c).

### screen [file]
Takes a screenshot. If no filename is provided, saves to `1.tga`, `2.tga`, etc.

### osinfo
Lists OS tasks, queues, and events.

### clearosinfo
Clears OS thread timing information.

### emptyq
Clears all OS message queues.

### event <num|all|sp>
Sends an OS event. Use `all` to send all events, `sp` for SP event, or a specific event number (0-9, 15).

### send <queue> <data>
Sends a data value to the specified OS message queue.
