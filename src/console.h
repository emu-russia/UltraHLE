// Win32 Console code is not exported externally, but is entirely dependent on the console implementation of debugui.
// This is done so that debugui can be ported to other engines, like imgui.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the console window and internal console state.
 */
void con_init(void);
/**
 * Initializes console state without a real console, using a dummy 80x50 size.
 */
void con_initdummy(void);
/**
 * Detaches from the console window.
 */
void con_deinit(void);

/**
 * Checks the console buffer size for changes and updates the stored dimensions.
 * @return 1 if the console was resized, 0 otherwise.
 */
int  con_resized(void);
/**
 * Returns the number of console rows.
 * @return Number of rows.
 */
int  con_rows(void);
/**
 * Returns the number of console columns.
 * @return Number of columns.
 */
int  con_cols(void);

/**
 * Clears the console screen and resets the cursor to the top-left corner.
 */
void con_clear(void);
/**
 * Moves the text cursor to the given column and row.
 * @param x Column position.
 * @param y Row position.
 */
void con_gotoxy(int x,int y);
/**
 * Sets the cursor position and size.
 * @param x Column position.
 * @param y Row position.
 * @param size Cursor size as a percentage (1..100).
 */
void con_cursorxy(int x,int y,int size); // size=1..100 (%)
/**
 * Sets the foreground attribute of the console text, keeping the background.
 * @param fg Foreground color attribute.
 */
void con_attr(int fg);
/**
 * Sets the foreground and background attributes of the console text.
 * @param fg Foreground color attribute.
 * @param bg Background color attribute.
 */
void con_attr2(int fg,int bg);
/**
 * Outputs a single character to the console through the output buffer.
 * @param ch Character to print.
 */
void con_printchar(int ch);
/**
 * Prints a text string to the console, honoring attribute and newline control codes.
 * @param text String to print.
 */
void con_print(char *text);
/**
 * Formats and prints a text string to the console.
 * @param text Format string.
 * @param ... Additional arguments for the format string.
 */
void con_printf(char *text,...);
/**
 * Prints the given character until the cursor reaches column x.
 * @param ch Character to print.
 * @param x Target column.
 */
void con_tabto(int ch,int x);

/**
 * Reads the mouse position relative to the console center and the button state.
 * @param xp Receives the relative X position.
 * @param yp Receives the relative Y position.
 * @param bp Receives the button state bitmask.
 */
void con_readmouserelative(int *xp,int *yp,int *bp);

/**
 * Reads the next key event, blocking until a key is pressed.
 * @return Key code.
 */
int con_readkey(void);
/**
 * Reads a key event without blocking.
 * @return Key code, or 0 if no key is available.
 */
int con_readkey_noblock(void);

#define KEY_ESC     27
#define KEY_ENTER   13
#define KEY_BKSPACE 8
#define KEY_DEL     0x1f0

#define KEY_F1      0x101
#define KEY_F2      0x102
#define KEY_F3      0x103
#define KEY_F4      0x104
#define KEY_F5      0x105
#define KEY_F6      0x106
#define KEY_F7      0x107
#define KEY_F8      0x108
#define KEY_F9      0x109
#define KEY_F10     0x10A
#define KEY_F11     0x10B
#define KEY_F12     0x10C

#define KEY_LEFT    0x120
#define KEY_RIGHT   0x121
#define KEY_UP      0x122
#define KEY_DOWN    0x123
#define KEY_PGUP    0x124
#define KEY_PGDN    0x125
#define KEY_HOME    0x126
#define KEY_END     0x127

#define KEY_RELEASE 0x1000

#ifdef __cplusplus
};
#endif
