#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// execute a patched routine (called by cpu.c)
void op_patch(int patch); // patch.c

#ifdef __cplusplus
};
#endif
