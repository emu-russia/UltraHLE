// Cart state - Info on cart (not saved/loaded)

#pragma once

#include <windows.h> // HANDLE (for mapfilehandle/maphandle)

#ifdef __cplusplus
extern "C" {
#endif

/** Cartridge state, info on the cart (not saved/loaded). */
typedef struct
{
    uint8_t* data;
    int    size;
    int    mapped;
    /** Handle of the mapped ROM file. */
    HANDLE mapfilehandle;
    /** Handle of the file mapping. */
    HANDLE maphandle;
    int    fileflip; // original flipstate in file

    // cart name and info
    char   cartname[256]; // filename
    char   symname[256];  // symfilename
    char   title[32];     // title from rom header (0x20)

    // bootloader info
    /** Bootloader code base address. */
    uint32_t  codebase;
    /** Bootloader code size in bytes. */
    uint32_t  codesize;

    // optional hints for os detection
    /** Start of the OS-call detection address range. */
    uint32_t  osrangestart;
    /** End of the OS-call detection address range. */
    uint32_t  osrangeend;

    // modeflags
    int    simplegfx;
    /** Non-zero in DEMO.ROM special os-call detection mode. */
    int    isdocalls;
    int    ismario;  // debug
    int    iszelda;
    int    iswaverace;
    int    isfzero;
    /** Non-zero to enable frame synchronization. */
    int    framesync;
    /** Non-zero when a bootloader is present. */
    int    bootloader;
    /** Reserved for future flags. */
    int    RESERVED[8];

    // gfx and sound modes
    int    dlist_diddlyvx;
    int    dlist_zelda;
    int    dlist_wavevx;
    int    dlist_geyevx;
    int    slist_type; // 0=mario, 1=zelda, 2=banjo

    // notes on what has happened for this cart
    int    first_rcp;  // 0=not yet happened
    /** Non-zero after the first pad access. */
    int    first_pad;
} Cart;

/** Global cartridge state. */
extern Cart  cart; // cart.c

// NOTES:
// - opening a cart *should* free the previous cart, not tested though
// - '!cart' means open cart with memory mapping (if possible)
// - '*cart' means cart is DEMO.ROM and signals special os-call detection mode

/**
 * Opens a cart file, loading or mapping it into memory and preparing the cart state.
 * @param fname Path of the ROM file (may be prefixed by '*' or '!').
 * @param memmap Non-zero to memory-map the file when possible.
 */
void cart_open(char* fname, int memmap); // open a cart

// these are currently only used internally by cart_open:
/**
 * Creates a dummy cart used when loading fails.
 */
void cart_dummy(void);        // create a dummy cart (used when loading fails)
/**
 * Frees the cart image and mapping resources.
 */
void cart_free(void);         // free cart
/**
 * Maps a cart file into memory.
 * @param fname Path of the ROM file.
 * @return 0 on success, 1 on failure.
 */
int cart_map(char* fname);    // map cart into memory (fast!)
/**
 * Loads a cart file into memory.
 * @param fname Path of the ROM file.
 * @return 0 on success, 1 if the file could not be opened, -1 on allocation failure.
 */
int  cart_load(char* fname);  // load cart into memory (slow)
/**
 * Saves the cart image to a file.
 * @param fname Destination path.
 * @return 0 on success, 1 if the file could not be opened.
 */
int  cart_save(char* fname);  // save cart image (not used)
/**
 * Checks a ROM file and returns its byte order flip code.
 * @param fname Path of the ROM file.
 * @return Flip code (0x0123/0x1032/0x2301/0x3210), -1 if unopenable, -2 if unrecognized.
 */
int  cart_check(char* fname); // check byte order (returns flipcode)
/**
 * Flips the byte order of the cart image.
 * @param flip Flip code from cart_check.
 */
void cart_flip(int flip);     // flip cart (based on flipcode)
/**
 * Flips the byte order of a 64-byte ROM header.
 * @param header Pointer to the header to flip.
 */
void cart_flipheader(char* header);

#ifdef __cplusplus
};
#endif
