void con_init(void);
void con_initdummy(void);
void con_deinit(void);

void con_sleep(int ms);

int  con_resized(void);
int  con_rows(void);
int  con_cols(void);

void con_clear(void);
void con_gotoxy(int x,int y);
void con_cursorxy(int x,int y,int size); // size=1..100 (%)
void con_attr(int fg);
void con_attr2(int fg,int bg);
void con_printchar(int ch);
void con_print(char *text);
void con_printf(char *text,...);
void con_tabto(int ch,int x);

void con_readmouserelative(int *xp,int *yp,int *bp);

int con_readkey(void);
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
