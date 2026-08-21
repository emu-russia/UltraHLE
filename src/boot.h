/**
 * \file boot.h
 * Declarations for cartridge loading and emulator reset.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Loads a cartridge image and resets the emulator.
 * @param cartname Path of the ROM file to load, or NULL for a dummy cartridge.
 * @param nomemmap If non-zero, the ROM is loaded without file mapping.
 */
void boot(char* cartname, int nomemmap);

/** Resets the emulator and boots the currently loaded cartridge. */
void reset(void);

#ifdef __cplusplus
};
#endif
