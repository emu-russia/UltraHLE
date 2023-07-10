#include <windows.h>
#include "ultra.h"

#define CONT_A      0x0080
#define CONT_B      0x0040
#define CONT_G	    0x0020
#define CONT_START  0x0010
#define CONT_UP     0x0008
#define CONT_DOWN   0x0004
#define CONT_LEFT   0x0002
#define CONT_RIGHT  0x0001
#define CONT_L      0x2000
#define CONT_R      0x1000
#define CONT_E      0x0800
#define CONT_D      0x0400
#define CONT_C      0x0200
#define CONT_F      0x0100

#define A_BUTTON	CONT_A
#define B_BUTTON	CONT_B
#define L_TRIG		CONT_L
#define R_TRIG		CONT_R
#define Z_TRIG		CONT_G
#define START       CONT_START
#define U_JPAD		CONT_UP
#define L_JPAD		CONT_LEFT
#define R_JPAD		CONT_RIGHT
#define D_JPAD		CONT_DOWN
#define U_CBUTTONS	CONT_E
#define D_CBUTTONS	CONT_D
#define L_CBUTTONS	CONT_C
#define R_CBUTTONS	CONT_F

typedef struct
{
    word button;
    char stickx;
    char sticky;
} PadStructData;

PadStructData mypad;
word          lastbutton;
int           clearcnt;
int           xnarrow;
int           xcenter;
int           ycenter;
int           joyactive;

int           mouseactive;
int           mousedisablecnt;

int           lastjoybuttons;

int  wire=0;
int  info=0;

#define MM_PADSTRUCTDATA 0x8033afa8

void readjoystick(int *xpos,int *ypos,int *buttons)
{
    JOYINFO joy;
    memset(&joy,0,sizeof(joy));
    joyGetPos(JOYSTICKID1,&joy);
    *xpos=joy.wXpos;
    *ypos=joy.wYpos;
    *buttons=joy.wButtons;
}

void pad_buttons(void)
{
    int a,down;
    down=GetAsyncKeyState('F')<0; a=L_JPAD;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('H')<0; a=R_JPAD;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('T')<0; a=U_JPAD;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('G')<0; a=D_JPAD;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('J')<0; a=L_CBUTTONS; if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('L')<0; a=R_CBUTTONS; if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('I')<0; a=U_CBUTTONS; if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('K')<0; a=D_CBUTTONS; if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('S')<0; a=START;      if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('A')<0; a=A_BUTTON;   if(down) mypad.button|=a; else mypad.button&=~a;
    down=(GetAsyncKeyState('B')<0)|(GetAsyncKeyState('X')<0);
                                  a=B_BUTTON;   if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('Z')<0; a=Z_TRIG;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('C')<0; a=L_TRIG;     if(down) mypad.button|=a; else mypad.button&=~a;
    down=GetAsyncKeyState('V')<0; a=R_TRIG;     if(down) mypad.button|=a; else mypad.button&=~a;
}

void pad_misckey(int key)
{ // called from exec
    int down,a;

    if(key&KEY_RELEASE) down=0; else down=1;
    key&=0xfff;
    if(key>32 && key<256) key=toupper(key);

//    print("key %04X down %i\n",key,down);

    switch(key)
    {
        case KEY_HOME:  xnarrow=down; xcenter=-6*down;  ycenter=+12*down; joyactive=0; break;
        case KEY_PGUP:  xnarrow=down; xcenter=+6*down;  ycenter=+12*down; joyactive=0; break;
        case KEY_PGDN:  xnarrow=down; xcenter=+6*down;  ycenter=-12*down; joyactive=0; break;
        case KEY_END:   xnarrow=down; xcenter=-6*down;  ycenter=-12*down; joyactive=0; break;
        case KEY_LEFT:  xnarrow=0;    xcenter=-16*down;                   joyactive=0; break;
        case KEY_RIGHT: xnarrow=0;    xcenter=+16*down;                   joyactive=0; break;
        case KEY_UP:    xnarrow=0;                      ycenter=+16*down; joyactive=0; break;
        case KEY_DOWN:  xnarrow=0;                      ycenter=-16*down; joyactive=0; break;

        case 'F':       a=L_JPAD;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'H':       a=R_JPAD;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'T':       a=U_JPAD;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'G':       a=D_JPAD;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'J':       a=L_CBUTTONS; mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'L':       a=R_CBUTTONS; mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'I':       a=U_CBUTTONS; mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'K':       a=D_CBUTTONS; mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'S':       a=START;      mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'A':       a=A_BUTTON;   mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'X':
        case 'B':       a=B_BUTTON;   mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'Z':       a=Z_TRIG;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'C':       a=L_TRIG;     mypad.button|=a; if(!down) mypad.button^=a; break;
        case 'V':       a=R_TRIG;     mypad.button|=a; if(!down) mypad.button^=a; break;
    }

    switch(key)
    {
        case 'W': if(down)
                  {
                      showwire^=1;
                  } break;
        case 'Q': if(down)
                  {
                      showinfo^=1;
                  } break;
        case 'E': if(down)
                  {
                      showtest++;
                      if(showtest>3) showtest=0;
                  } break;
        case 'R': if(down)
                  {
                      showtest2++;
                      if(showtest2>7) showtest2=0;
                  } break;
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
                  if(down)
                  {
                      rdp_togglefullscreen();
                  } break;
        case KEY_F6:
                  if(st.pause) st.pause=0;
                  if(down)
                  {
                      breakcommand("save;go");
                  }
                  break;
        case KEY_F8:
                  if(down)
                  {
                      st.pause^=1;
                      if(st.pause)
                      {
                          if(st2.audioon)
                          {
                              st2.audioon=0;
                              sound_stop();
                          }
                      }
                  }
                  break;
        case KEY_F9:
                  if(st.pause) st.pause=0;
                  if(down)
                  {
                      breakcommand("load;go");
                  }
                  if(st.pause) st.pause=0;
                  break;
        case KEY_F3:
        case KEY_F4:
                  if(down)
                  {
                      if(key==KEY_F3)
                      {
                          joyactive^=1;
                          if(joyactive) mouseactive=0;
                      }
                      else
                      {
                          mouseactive^=1;
                          if(mouseactive) joyactive=0;
                      }
                      mousedisablecnt=0;
                      print("Joystick (F5) %s, Mouse (F6) %s\n",
                        joyactive?"enabled ":"disabled",
                        mouseactive?"enabled ":"disabled");
                      flushdisplay();
                  } break;
    }

}

void pad_joy(void)
{
    int x,y,b,bc;

    readjoystick(&x,&y,&b);
    if(b && !joyactive)
    {
        joyactive=1;
    }

    bc=lastjoybuttons^b;
    if(joyactive)
    {
        int a;
        if(bc&1)
        {
            a=Z_TRIG;   mypad.button|=a; if(!(b&1)) mypad.button^=a;
        }
        if(bc&2)
        {
            a=A_BUTTON; mypad.button|=a; if(!(b&2)) mypad.button^=a;
        }
        if(bc&4)
        {
            a=R_TRIG;   mypad.button|=a; if(!(b&4)) mypad.button^=a;
        }
        if(bc&8)
        {
            a=B_BUTTON; mypad.button|=a; if(!(b&8)) mypad.button^=a;
        }

        if(x<16384) x=8192;
        if(x>28672 && x<36864) x=32768;
        if(x>49152) x=49152;

        if(y<16384) y=8192;
        if(y>28672 && y<36864) y=32768;
        if(y>49152) y=49152;

        x=(x-32768)*80/16384;
        y=(y-32768)*80/16384;

        mypad.stickx=x;
        mypad.sticky=-y;
    }
    lastjoybuttons=b;
}

#define AVERAGE 2

void pad_drawframe(void)
{
    static float xhistory[AVERAGE];
    static float yhistory[AVERAGE];
    static int   hi=0;
    static int lastx,lasty;
    int   i,sx,sy;
    int   maxspd;
    float x,y,l;

    lastbutton=mypad.button;

    if(mouseactive)
    {
        int xi,yi,b;
        static int lastb;
        static int stopcnt=0;
        float speed=10.0;

        sx=mypad.stickx;
        sy=mypad.sticky;

        con_readmouserelative(&xi,&yi,&b);

        yi=-yi;
        xhistory[hi]=xi;
        yhistory[hi]=yi;
        hi=(hi+1)%AVERAGE;

        if(b&1)
        {
            sx+=xi/2;
            sy+=yi/2;
            stopcnt=99;
        }
        else
        {
            if(stopcnt>0)
            {
                sx/=2;
                sy/=2;
                stopcnt=0;
            }
            else
            {
                xi=yi=0;
                for(i=0;i<AVERAGE;i++)
                {
                    xi+=xhistory[i];
                    yi+=yhistory[i];
                }
                sx=xi*4/AVERAGE;
                sy=yi*4/AVERAGE;
            }
        }

        /*
        if(GetAsyncKeyState(VK_CONTROL)<0) b|=8;
        else b&=~8;
        */

        if((b^lastb)&2)
        {
            if(b&2) mypad.button|=A_BUTTON; else mypad.button&=~A_BUTTON;
        }
        if((b^lastb)&4)
        {
            if(b&4) mypad.button|=B_BUTTON; else mypad.button&=~B_BUTTON;
        }
        if((b^lastb)&8)
        {
            if(b&8) mypad.button|=Z_TRIG; else mypad.button&=~Z_TRIG;
        }
        lastb=b;

        if(mousedisablecnt==1 && GetAsyncKeyState(VK_F6)<0) mouseactive=0;
        if(GetAsyncKeyState(VK_F6)==0) mousedisablecnt=1;

        if(sx> 80) sx= 80;
        if(sx<-80) sx=-80;
        if(sy> 80) sy= 80;
        if(sy<-80) sy=-80;

        mypad.stickx=sx;
        mypad.sticky=sy;
    }
    else if(joyactive)
    {
        pad_joy();
    }
    else
    { // keyboard delay
        if(!xcenter) mypad.stickx/=2;
        else if(xcenter<0 && mypad.stickx>0) mypad.stickx=xcenter*3;
        else if(xcenter>0 && mypad.stickx<0) mypad.stickx=xcenter*3;
        else mypad.stickx+=xcenter;

        if(!ycenter) mypad.sticky/=2;
        else if(ycenter<0 && mypad.sticky>0) mypad.sticky=ycenter*3;
        else if(ycenter>0 && mypad.sticky<0) mypad.sticky=ycenter*3;
        else mypad.sticky+=ycenter;

        maxspd=115;
        if(GetAsyncKeyState(VK_SHIFT)<0) maxspd=50;
        if(GetAsyncKeyState(VK_CONTROL)<0) maxspd=30;

        x=mypad.stickx;
        y=mypad.sticky;
        l=sqrt(x*x+y*y);
        if(l>maxspd)
        {
            x*=maxspd/l;
            y*=maxspd/l;
        }
        if(xnarrow)
        {
            if(x> 40) x= 40;
            if(x<-40) x=-40;
        }

        mypad.stickx=(x+lastx)/2;
        mypad.sticky=(y+lasty)/2;
        lastx=x;
        lasty=y;
    }

    if(mypad.stickx> 80) mypad.stickx= 80;
    if(mypad.stickx<-80) mypad.stickx=-80;
    if(mypad.sticky> 80) mypad.sticky= 80;
    if(mypad.sticky<-80) mypad.sticky=-80;

    clearcnt=4; // clear button in 4 frames
}

void pad_frame(void)
{
}

void pad_writedata(dword addr)
{
    dword state;
    state=*(dword *)&mypad;
//    print("pad %08X\n",state);
    st.padstate=state;
    mem_write32(addr,FLIP32(state));
}

