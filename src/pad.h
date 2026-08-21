// Pad emulation

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Per-frame pad processing hook.
 */
void  pad_frame(void);           // call this every frame (pad centering and stuff)
/**
 * Returns the pad state for a controller as a 32-bit value.
 * @param pad Controller index.
 * @return Pad state in the emulated byte order.
 */
uint32_t pad_getdata(int pad);
/**
 * Writes the pad state of controller 0 to an emulated memory address.
 * @param addr Destination address in emulated memory.
 */
void  pad_writedata(uint32_t addr); // write pad state to a memory location
/**
 * Updates the pad state once per frame using the active input device.
 */
void  pad_drawframe(void);

/**
 * Enables or disables joystick input.
 * @param enable Non-zero to enable joystick input.
 */
void  pad_enablejoy(int enable);

// A temporary solution for now. The controller emulation code is for some reason heavily tied to console.c, we need to untangle this doshirak.

/** Non-zero when joystick input is active. */
extern int joyactive;
/** Non-zero when mouse input is active. */
extern int mouseactive;
/** Frame counter used to detect F6 key presses. */
extern int mousedisablecnt;

#ifdef __cplusplus
};
#endif
