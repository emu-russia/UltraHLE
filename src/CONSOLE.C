#include <windows.h>
#include "console.h"

#define MAXSTRING 1024

struct
{
    // handle
    HANDLE hconsole;
    HANDLE hconsoleinput;
    // console size
    int    sx,sy;
    int    basey;
    // cursor position
    int    cx,cy;
    // current attribute
    int    attr;
    // mouse center
    int    mcx,mcy;
    int    mbutton;
    // output buffer
    int       cix,ciy;
    int       cicount;
    CHAR_INFO ci[MAXSTRING];
} con;

void ResizeConsole(HANDLE hConsole, SHORT xSize, SHORT ySize)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
  BOOL bSuccess;   SMALL_RECT srWindowRect; /* hold the new console size */
  COORD coordScreen;    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
  /* get the largest size we can size the console window to */
  coordScreen = GetLargestConsoleWindowSize(hConsole);
  /* define the new console window size and scroll position */
  srWindowRect.Right = (SHORT) (min(xSize, coordScreen.X) - 1);
  srWindowRect.Bottom = (SHORT) (min(ySize, coordScreen.Y) - 1);
  srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
  /* define the new console buffer size */   coordScreen.X = xSize;
  coordScreen.Y = ySize;
  /* if the current buffer is larger than what we want, resize the */
  /* console window first, then the buffer */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)     {
    bSuccess = SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
    bSuccess = SetConsoleScreenBufferSize(hConsole, coordScreen);
  }
  /* if the current buffer is smaller than what we want, resize the */
  /* buffer first, then the console window */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)     {
    bSuccess = SetConsoleScreenBufferSize(hConsole, coordScreen);
    bSuccess = SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
  }
  /* if the current buffer *is* the size we want, don't do anything! */
  return;
}

void con_opennew(void)
{
    FreeConsole();
    AllocConsole();
    // use a 80x60 buffer
    ResizeConsole(GetStdHandle(STD_OUTPUT_HANDLE),80,60);
}

void con_init(void)
{
    memset(&con,0,sizeof(con));

    con_opennew();

    con.hconsole=GetStdHandle(STD_OUTPUT_HANDLE);
    con.hconsoleinput=GetStdHandle(STD_INPUT_HANDLE);
    con_resized();
    con.sy--; // force another resize
    con.attr=0x0f;
//    SetConsoleMode(con.hconsoleinput,ENABLE_MOUSE_INPUT);

    con.mcx=GetSystemMetrics(SM_CXSCREEN)/2;
    con.mcy=GetSystemMetrics(SM_CYSCREEN)/2;
}

void con_initdummy(void)
{
    memset(&con,0,sizeof(con));
    con.sx=80;
    con.sy=50;
}

void con_readmouserelative(int *xp,int *yp,int *buttons)
{
    POINT p;
    int x,y,b;

    GetCursorPos(&p);
    SetCursorPos(con.mcx,con.mcy);
    x=p.x-con.mcx;
    y=p.y-con.mcy;

    b =(GetAsyncKeyState(VK_LBUTTON)<0)?1:0;
    b+=(GetAsyncKeyState(VK_RBUTTON)<0)?2:0;
    b+=(GetAsyncKeyState(VK_MBUTTON)<0)?4:0;

    *buttons=b;
    *xp=x;
    *yp=y;

//    print("%6i,%6i,%6i\n",*xp,*yp,*buttons);
//    flushdisplay();
}

void con_deinit(void)
{
    FreeConsole();
}

int  con_cols(void)
{
    return(con.sx);
}

int  con_rows(void)
{
    return(con.sy);
}

int  con_resized(void)
{
    int resized=0,x,y;
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(con.hconsole,&info);

    x=info.srWindow.Right-info.srWindow.Left+1;
    y=info.srWindow.Bottom-info.srWindow.Top+1;

    con.basey=info.srWindow.Top;

    if(x!=con.sx)
    {
        resized=1;
        con.sx=x;
    }
    if(y!=con.sy)
    {
        resized=1;
        con.sy=y;
    }

    return(resized);
}

void con_clear(void)
{
    COORD zero;
    int  x;
    zero.X=0;
    zero.Y=0;
    FillConsoleOutputCharacter(
        con.hconsole,
        ' ',
        65535,
        zero,
        &x);
    FillConsoleOutputAttribute(
        con.hconsole,
        0x7,
        65535,
        zero,
        &x);
    con.cx=con.cy=0;
}

void con_gotoxy(int x,int y)
{
    if(!con.hconsole) return;
    con_printchar(-1);
    con.cx=x;
    con.cy=y;
}

void con_cursorxy(int x,int y,int size)
{
    COORD c;
    CONSOLE_CURSOR_INFO ci;
    if(!con.hconsole) return;
    c.X=x;
    c.Y=y+con.basey;
    if(size<1 || size>100)
    {
        ci.bVisible=FALSE;
        ci.dwSize=50;
    }
    else
    {
        ci.bVisible=TRUE;
        ci.dwSize=size;
    }
    SetConsoleCursorInfo(con.hconsole,&ci);
    SetConsoleCursorPosition(con.hconsole,c);
}

void con_attr2(int fg,int bg)
{
    con.attr=(bg<<4)|fg;
}

void con_attr(int fg)
{
    con.attr&=0xfff0;
    con.attr|=fg;
}

void con_printchar(int ch)
{
    if(!con.hconsole) return;
    if(con.cx!=con.cix+con.cicount || con.cy!=con.ciy || ch==-1)
    {
        if(con.cicount)
        {
            COORD  cisize,cibase;
            SMALL_RECT region;
            cisize.X=con.cicount;
            cisize.Y=1;
            cibase.X=0;
            cibase.Y=0;
            region.Left=con.cix;
            region.Top=con.ciy+con.basey;
            region.Right=con.cix+con.cicount-1;
            region.Bottom=con.ciy+con.basey;
            WriteConsoleOutput(
                con.hconsole,
                con.ci,
                cisize,
                cibase,
                &region);
        }

        con.cicount=0;
        con.cix=con.cx;
        con.ciy=con.cy;

        if(ch==-1) return;
    }

    con.ci[con.cicount].Char.AsciiChar=ch;
    con.ci[con.cicount].Attributes=con.attr;
    con.cicount++;
    con.cx++;
}

void con_print(char *text)
{
    static CHAR_INFO ci[MAXSTRING];
    int    i=0;

    if(!con.hconsole) return;
    while(*text)
    {
        if(*text==0x01)
        { // foreground set
            con.attr&=0xfff0;
            con.attr|=text[1]&15;
            text+=2;
        }
        else if(*text==0x02)
        { // background set
            con.attr&=0xff0f;
            con.attr|=(text[1]&15)<<4;
            text+=2;
        }
        else if(*text=='\n')
        {
            con_gotoxy(0,con.cy+1);
            text++;
        }
        else
        {
            con_printchar(*text++);
        }
    }
}

void con_tabto(int ch,int x)
{
    if(!con.hconsole) return;
    while(con.cx<con.sx && con.cx<x)
    {
        con_printchar(ch);
    }
}

void con_printf(char *txt,...)
{
    static char buf[MAXSTRING];
    va_list argp;
    va_start(argp,txt);
    vsprintf(buf,txt,argp);
    con_print(buf);
}

int readevent(int dopeek)
{
    INPUT_RECORD inp;
    DWORD num;
    int ascii,vkey,down,res;

    if(!con.hconsole) return(0);

    if(dopeek)
    {
        PeekConsoleInput(con.hconsoleinput,&inp,1,&num);
        if(!num) return(0);
    }
    ReadConsoleInput(con.hconsoleinput,&inp,1,&num);

    if(inp.EventType==MOUSE_EVENT)
    {
        con.mbutton=inp.Event.MouseEvent.dwButtonState;
    }
    else if(inp.EventType==KEY_EVENT)
    {
        ascii=inp.Event.KeyEvent.uChar.AsciiChar;
        vkey=inp.Event.KeyEvent.wVirtualKeyCode;
        down=inp.Event.KeyEvent.bKeyDown;
        res=0;

        if(vkey>='A' && vkey<='Z' && (ascii<'A' || ascii>'z'))
        {
            res=vkey;
        }
        else if(vkey>=VK_F1 && vkey<=VK_F12)
        {
            res=(KEY_F1+vkey-VK_F1);
        }
        else switch(vkey)
        {
        case VK_DELETE: res=(KEY_DEL);    break;
        case VK_UP:     res=(KEY_UP);     break;
        case VK_DOWN:   res=(KEY_DOWN);   break;
        case VK_LEFT:   res=(KEY_LEFT);   break;
        case VK_RIGHT:  res=(KEY_RIGHT);  break;
        case VK_PRIOR:  res=(KEY_PGUP);   break;
        case VK_NEXT:   res=(KEY_PGDN);   break;
        case VK_HOME:   res=(KEY_HOME);   break;
        case VK_END:    res=(KEY_END);    break;
        default:        res=(ascii);      break;
        }

        if(!down) res|=KEY_RELEASE;

        return(res);
    }

    return(0);
}

int con_readkey(void)
{
    if(!con.hconsole) return(0);

    for(;;)
    {
        int key=readevent(0);
        if(key) return(key);
    }
}

int con_readkey_noblock(void)
{
    if(!con.hconsole) return(0);
    return(readevent(1));
}

void con_sleep(int ms)
{
    Sleep(ms);
}
