// Debug UI, includes VIEW module which does the console screen updating

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Colors for console (standard vga color numbers)
#define RED    "\x1\x4"
#define GRAY   "\x1\x8"
#define NORMAL "\x1\x7"
#define PUR    "\x1\x5"
#define YEL    "\x1\xe"
#define YELLOW "\x1\xe"
#define WHITE  "\x1\xf"
#define CYAN   "\x1\x3"
#define BLUE   "\x1\x9"
#define BROWN  "\x1\x6"
#define GREEN  "\x1\x2"
#define DBLUE  "\x1\x1"

// view.c
typedef struct
{
    int changed;       // bitmask for what to update (use view_changed())
    uint32_t codebase; // address of code window
    uint32_t database; // address of data window
    int datatype;      // type of data window (not supported)
    int showfpu;       // what registers to show (SHOWFPU_*)
    int showhelp;      // show pad help at top of screen
    int shrink;        // maximize console
    int hidestuff;     // if 1 all but console is hidden
    int active;        // active window: 0=console,1=code,2=data

    int consolerow;    // 0=show bottom of console, >0=scroll upward
    int consolecursor; // x-cursor position for console

    int coderows;      // number of rows in code window (read only)
    int datarows;      // number of rows in data window (read only)
    int consrows;      // number of rows in console     (read only)
} View;

extern View view; // in view.c

// register showfpu  modes
#define SHOWFPU_INT       0  // R4300 integer regs
#define SHOWFPU_FPU       1  // R4300 fpu regs as single
#define SHOWFPU_FPUDWORD  2  // R4300 fpu regs as integer

// window numbers
#define WIN_CONS 0
#define WIN_CODE 1
#define WIN_DATA 2

// changed bitmask (for view_changed)
#define VIEW_REGS    0x001
#define VIEW_CODE    0x002
#define VIEW_DATA    0x004
#define VIEW_CONS    0x008
#define VIEW_HELP    0x010
#define VIEW_STAT    0x020
#define VIEW_CLEAR   0x100
#define VIEW_RESIZE  0x200
#define VIEW_ALL     (VIEW_REGS|VIEW_CODE|VIEW_DATA|VIEW_CONS|VIEW_HELP|VIEW_STAT)

void view_open(void);               // initialize view
void view_close(void);              // deinitialize view
void view_changed(int which);       // tell the view to update something
void view_redraw(void);             // update what has been requested
void view_setlast(void);            // set "last" state for register highlight
void view_writeconsole(char* text); // write text to console
void view_status(char* text);       // change statusline text
void view_clear_cmdline();           // Called from cmd.c command to clear the command entry string

void exitnow(void); // exit emu asap
void flushdisplay(void); // redraw debug console
void debugui(void); // main debugui-loop

void breakcommand(char* cmd); // breaks nicely at next retrace, and then executes
void debugui_misckey(int key);      // tell a key has been pressed (keycodes as in console.h)

#ifdef __cplusplus
};
#endif
