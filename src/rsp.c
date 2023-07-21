#include "ultra.h"

SPState sp;

int rsp_init()
{
	// Reallocate DMEM/IMEM

	if (sp.dmem) {
		free(sp.dmem);
		sp.dmem = NULL;
	}

	if (sp.imem) {
		free(sp.imem);
		sp.imem = NULL;
	}

	sp.dmem = malloc(DMEM_SIZE);
	if (!sp.dmem)
		return -1;

	sp.imem = malloc(IMEM_SIZE);
	if (!sp.imem) {
		free(sp.dmem);
		sp.dmem = NULL;
		return -1;
	}

	memset(sp.dmem, 0, DMEM_SIZE);
	memset(sp.imem, 0, IMEM_SIZE);

	// Map DMEM/IMEM

	mem_mapexternal(DMEM_ADDRESS, MAP_RW, sp.dmem);
	mem_mapexternal(IMEM_ADDRESS, MAP_RW, sp.imem);

	return 0;
}
