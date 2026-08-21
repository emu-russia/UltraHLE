/**
 * \file patch.h
 * Declares the dispatcher for patched OS routines.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Executes the patched routine with the given patch index.
 * @param patch Patch index into the patch table.
 */
// execute a patched routine (called by cpu.c)
void op_patch(int patch); // patch.c

#ifdef __cplusplus
};
#endif
