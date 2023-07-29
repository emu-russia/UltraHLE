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
typedef struct _SPState
{

	uint8_t imem[IMEM_SIZE];
	uint8_t dmem[DMEM_SIZE];

} SPState;

extern SPState sp;

int rsp_init();

#ifdef __cplusplus
};
#endif
