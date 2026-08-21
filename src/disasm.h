// Disassembler

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// text tables for register names
extern char* regnames[];    // in disasm.c
extern char* mmuregnames[]; // in disasm.c

// disassemble a single op. Pos needed for jumps, x is opcode in intel format,
// returned text is in a static buffer, copy away!
char* disasm(uint32_t pos, uint32_t x);

// disassemble a single rsp-op
char* disasmrsp(uint32_t pos, uint32_t x);

// unassemble X86 code
char* disasmx86(uint8_t* opcode, int codeoff, int* len);

// dump memory range into a file (two ranges; one for code, one for data)
void  disasm_dumpcode(char* filename, uint32_t addr, int size, uint32_t dataaddr, int datasize);

// as above, but use rsp disassembler
void  disasm_dumpucode(char* filename, uint32_t addr, int size, uint32_t dataaddr, int datasize, int offset);

#ifdef __cplusplus
};
#endif
