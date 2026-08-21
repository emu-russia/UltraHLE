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

/**
 * Clears the symbol table and the os-call search state.
 */
void  sym_clear(void);       // clear symbol table
/**
 * Dumps all symbols and their patch codes to the console.
 */
void  sym_dump(void);        // dump table to console
/**
 * Loads symbols from a file, clearing any previous table and setting cart mode flags.
 * @param file Path of the symbol file.
 */
void  sym_load(char* file);  // load from file
/**
 * Saves the symbol table to a file (not implemented).
 * @param file Path of the file to save to.
 */
void  sym_save(char* file);  // save to file (NOT IMPLEMENTED)
/**
 * Adds or replaces the symbol for an address.
 * @param addr MIPS address of the symbol.
 * @param text Symbol name.
 * @param patch Patch code; 0 for no patch.
 * @return Index of the symbol in the table.
 */
int   sym_add(int addr, char* text, int patch); // add/replace a symbol
/**
 * Deletes the symbol at an address, if it exists.
 * @param addr MIPS address of the symbol to delete.
 */
void  sym_del(int addr);     // delete a symbol
/**
 * Finds the symbol name for an address.
 * @param addr MIPS address to look up.
 * @return Name of the nearest symbol, or "?" if none applies.
 */
char* sym_find(int addr);    // find symbol name for an address
/**
 * Finds the os-calls in the loaded cart and patches the found routines.
 */
void  sym_findfirstos(void);

/**
 * Writes patch opcodes into memory for all patched symbols.
 */
// patching related routines
void  sym_addpatches(void);  // put patch opcodes into memory for patched symbols
/**
 * Removes all applied patches from memory, restoring the original contents.
 */
void  sym_removepatches(void);  // put patch opcodes into memory for patched symbols
/**
 * Searches memory for known os-call routines and adds symbols for them.
 * @param base Start address of the range to search.
 * @param bytes Size of the range to search.
 * @param cont Non-zero to continue a previous search without resetting results.
 */
void  sym_findoscalls(uint32_t base, uint32_t bytes, int cont); // search memory for os-calls and add symbols for them
/**
 * Prints the os-call search results, found and missing, to the console.
 */
void  sym_dumposcalls(void); // print list of found os routines to console
/**
 * Builds the os-call list from the demo ROM, which must be loaded, and writes oscall.h.
 */
void  sym_demooscalls(void); // create list of oscalls from DEMO.ROM (which must be loaded)

/**
 * Records the name of an os routine found at the given address.
 * @param addr Address of the os routine.
 * @param name Name to record.
 */
void  symfind_saveroutine(uint32_t addr, char* name);
/**
 * Returns the name of the os routine matching the given address.
 * @param addr Address of the routine to look up.
 * @return Name of the matching routine.
 */
char* symfind_matchroutine(uint32_t addr);

#ifdef __cplusplus
};
#endif
