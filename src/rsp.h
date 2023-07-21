// Everything about RSP emulation is here

#pragma once

#define DMEM_ADDRESS 0xa4000000
#define IMEM_ADDRESS 0xa4001000

#define DMEM_SIZE 0x1000		// bytes
#define IMEM_SIZE 0x1000		// bytes

// RSP state
typedef struct _SPState
{

	uint8_t* imem;
	uint8_t* dmem;

} SPState;

extern SPState sp;

int rsp_init();
