// Cart state - Info on cart (not saved/loaded)

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    byte* data;
    int    size;
    int    mapped;
    dword  mapfilehandle;
    dword  maphandle;
    int    fileflip; // original flipstate in file

    // cart name and info
    char   cartname[256]; // filename
    char   symname[256];  // symfilename
    char   title[32];     // title from rom header (0x20)

    // bootloader info
    dword  codebase;
    dword  codesize;

    // optional hints for os detection
    dword  osrangestart;
    dword  osrangeend;

    // modeflags
    int    simplegfx;
    int    isdocalls;
    int    ismario;  // debug
    int    iszelda;
    int    iswaverace;
    int    isfzero;
    int    framesync;
    int    bootloader;
    int    RESERVED[8];

    // gfx and sound modes
    int    dlist_diddlyvx;
    int    dlist_zelda;
    int    dlist_wavevx;
    int    dlist_geyevx;
    int    slist_type; // 0=mario, 1=zelda, 2=banjo

    // notes on what has happened for this cart
    int    first_rcp;  // 0=not yet happened
    int    first_pad;
} Cart;

extern Cart  cart; // cart.c

// NOTES:
// - opening a cart *should* free the previous cart, not tested though
// - '!cart' means open cart with memory mapping (if possible)
// - '*cart' means cart is DEMO.ROM and signals special os-call detection mode

void cart_open(char* fname, int memmap); // open a cart

// these are currently only used internally by cart_open:
void cart_dummy(void);        // create a dummy cart (used when loading fails)
void cart_free(void);         // free cart
int cart_map(char* fname);    // map cart into memory (fast!)
int  cart_load(char* fname);  // load cart into memory (slow)
int  cart_save(char* fname);  // save cart image (not used)
int  cart_check(char* fname); // check byte order (returns flipcode)
void cart_flip(int flip);     // flip cart (based on flipcode)
void cart_flipheader(char* header);

#ifdef __cplusplus
};
#endif
