// Disassembler

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// text tables for register names
/** Names of the 32 CPU general-purpose registers (indexed by register number). */
extern char* regnames[];    // in disasm.c
/** Names of the 32 MMU control registers of coprocessor 0 (indexed by register number). */
extern char* mmuregnames[]; // in disasm.c

// disassemble a single op. Pos needed for jumps, x is opcode in intel format,
// returned text is in a static buffer, copy away!
/**
 * Disassembles a single CPU instruction.
 * @param pos Address of the instruction, needed for jump targets.
 * @param x Instruction word.
 * @return Disassembled text in a static buffer (copy it before the next call).
 */
char* disasm(uint32_t pos, uint32_t x);

// disassemble a single rsp-op
/**
 * Disassembles a single RSP (Reality Signal Processor) instruction.
 * @param pos Address of the instruction, needed for jump targets.
 * @param x Instruction word.
 * @return Disassembled text in a static buffer (copy it before the next call).
 */
char* disasmrsp(uint32_t pos, uint32_t x);

// unassemble X86 code
/**
 * Disassembles a single x86 instruction.
 * @param opcode Pointer to the opcode bytes.
 * @param codeoff Code offset of the instruction start.
 * @param len Receives the length of the disassembled instruction in bytes.
 * @return Disassembled text in a static buffer (copy it before the next call).
 */
char* disasmx86(uint8_t* opcode, intptr_t codeoff, int* len);

// dump memory range into a file (two ranges; one for code, one for data)
/**
 * Dumps a memory range into a file (two ranges; one for code, one for data).
 * @param filename Name of the output file.
 * @param addr Start address of the code range.
 * @param size Size of the code range in bytes.
 * @param dataaddr Start address of the data range.
 * @param datasize Size of the data range in bytes.
 */
void  disasm_dumpcode(char* filename, uint32_t addr, int size, uint32_t dataaddr, int datasize);

// as above, but use rsp disassembler
/**
 * Dumps a memory range into a file like disasm_dumpcode, but using the RSP disassembler.
 * @param filename Name of the output file.
 * @param addr Start address of the code range.
 * @param size Size of the code range in bytes (0 selects a 4096-byte default).
 * @param dataaddr Start address of the data range.
 * @param datasize Size of the data range in bytes.
 * @param offset RSP address offset used for the disassembly.
 */
void  disasm_dumpucode(char* filename, uint32_t addr, int size, uint32_t dataaddr, int datasize, int offset);

#ifdef __cplusplus
};
#endif
