// Symbol table handling, also used for patch addresses

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
NOTES:
- One address can only have one symbol (adding to same address replaces last)
- When changing/freeing symbols memory is lost (lazy :)
- The symbol search (sym_find) is a linear (slow) search
- The patch system has a lot of problems, it works temporarily but
  we'll get rid of it when os emulation is no longer needed
- symbol names with #<number> mean patch(number)

The patch system works like this:
- a symbol can have a patch code
- sym_addpatches writes a special PATCH(patch) opcode to all
  memory locations specfied by patched symbols
- when this opcode is executed by cpu.c it calls patch.c which
  executes that particular patch
- the patches are design to be used at start of os-routines to replace them

The sym_findoscalls and related routines are quite a hack right now.
There are various minor problems, especially if searching os-routines
multiple times. I'm not going to fix these since patches are going away.

Problems:
- memory is different (application might detect)
- when code is loaded/uncompressed again it might overwrite the code
  that had patches. So patches become inactive.
- if patches are reapplied and not all of the patched routines are
  really in memory, some other memory could be overwritten!
- usually the os stays resident all the time (although it might not
  be loaded at once). So in practice patches seem to work.
*/

void  sym_clear(void);       // clear symbol table
void  sym_dump(void);        // dump table to console
void  sym_load(char* file);  // load from file
void  sym_save(char* file);  // save to file (NOT IMPLEMENTED)
int   sym_add(int addr, char* text, int patch); // add/replace a symbol
void  sym_del(int addr);     // delete a symbol
char* sym_find(int addr);    // find symbol name for an address
void  sym_findfirstos(void);

// patching related routines
void  sym_addpatches(void);  // put patch opcodes into memory for patched symbols
void  sym_removepatches(void);  // put patch opcodes into memory for patched symbols
void  sym_findoscalls(dword base, dword bytes, int cont); // search memory for os-calls and add symbols for them
void  sym_dumposcalls(void); // print list of found os routines to console
void  sym_demooscalls(void); // create list of oscalls from DEMO.ROM (which must be loaded)

void  symfind_saveroutine(dword addr, char* name);
char* symfind_matchroutine(dword addr);

#ifdef __cplusplus
};
#endif
