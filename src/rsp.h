/**
 * \file rsp.h
 * RSP (Reality Signal Processor) emulation declarations.
 */

// Everything about RSP emulation is here

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define DMEM_ADDRESS 0xa4000000
#define IMEM_ADDRESS 0xa4001000

#define DMEM_SIZE 0x1000		// bytes
#define IMEM_SIZE 0x1000		// bytes

// RSP state
/** RSP state: the instruction and data memories. */
typedef struct _SPState
{

	uint8_t imem[IMEM_SIZE];
	uint8_t dmem[DMEM_SIZE];

} SPState;

/** Global RSP state. */
extern SPState sp;

/**
 * Initializes the RSP: clears and maps DMEM and IMEM.
 * @return 0 on success.
 */
int rsp_init();

#ifdef __cplusplus
};
#endif
