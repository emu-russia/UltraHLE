#include "ultra.h"

SPState sp;

int rsp_init()
{
	// Clear DMEM/IMEM

	memset(sp.dmem, 0, DMEM_SIZE);
	memset(sp.imem, 0, IMEM_SIZE);

	// Map DMEM/IMEM

	mem_mapexternal(DMEM_ADDRESS, MAP_RW, sp.dmem);
	mem_mapexternal(IMEM_ADDRESS, MAP_RW, sp.imem);

	return 0;
}
