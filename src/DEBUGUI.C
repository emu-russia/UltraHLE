#include "stdsdk.h"
extern HWND hwndStatus;
extern char szBuffer[];

#define STATLINES 20

int exitflag=0;

void exitnow(void)
{
    exitflag=1;
}

/****************************************************************************
** Debugui console
*/

static State stlast;

int viewopen=0;

char *helptext[]={
"                                                                               ",
" A:A       S:Start  C:L-Trig  I J K L:C-Keys  Shift:Walk   Arrows:Analog Stick ",
" Z:Z-Trig  X:B      V:R-Trig  T F G H:Joypad  Ctrl:Tiptoe  F12:3DFX toggle     ",
"                                                                               ",
NULL};

typedef struct
{
    int  y,sy;
} Win;

// console
#define LINELEN 81
#define LINES   1024
typedef struct
{
    char  linedata[LINES][LINELEN];
    char *line[LINES]; // 0=bottommost
    int   writepos;
} Con;

Win w_help;
Win w_regs;
Win w_data;
Win w_code;
Win w_cons;
Win w_stat;

Con console;

char statusline[128];

View view;

int c_barfg=0x0;
int c_barbg=0x3;

void view_checksize(void)
{
    int h,y;

    h=con_rows();

    if(view.hidestuff || !mem.ram || !cart.data)
    {
        w_help.sy=0;
        w_regs.sy=0;
        w_data.sy=0;
        w_code.sy=0;
        w_stat.sy=0;
        w_cons.sy=h-1;
    }
    else
    {
        if(view.shrink)
        {
            w_regs.sy=0;
            w_data.sy=2;
            w_code.sy=5;
            w_cons.sy=h-w_help.sy-w_regs.sy-w_data.sy-w_code.sy-1;
        }
        else
        {
            if(h<30)
            {
                w_regs.sy=0;
                w_data.sy=2;
                w_code.sy=4;
            }
            else
            {
                w_regs.sy=8;
                w_data.sy=8;
                w_code.sy=16;
            }
        }
        if(view.showhelp)
        {
            w_help.sy=4;
            if(h<30) w_stat.sy=8;
            else w_stat.sy=STATLINES;
            w_code.sy=4;
            w_regs.sy=0;
            w_data.sy=0;
        }
        else
        {
            w_help.sy=0;
            w_stat.sy=0;
        }
    }
    w_cons.sy=h-w_help.sy-w_stat.sy-w_regs.sy-w_data.sy-w_code.sy-1;

    view.datarows=w_data.sy-1;
    view.coderows=w_code.sy-1;
    view.consrows=w_cons.sy-1;

    y=0;
    w_help.y=y; y+=w_help.sy;
    w_stat.y=y; y+=w_stat.sy;
    w_regs.y=y; y+=w_regs.sy;
    w_data.y=y; y+=w_data.sy;
    w_code.y=y; y+=w_code.sy;
    w_cons.y=y; y+=w_cons.sy;
}

void view_changed(int which)
{
    view.changed|=which;
}

void view_regs(void)
{
    char regname[16];
    int x,y,r;
    int vnow,vlast,vnowhi;
    int printq,printdec;
    qword vnowq;
    char *vname;
    int c_reg=0x03;
    int c_num=0x07;
    int c_chg=0x0f;
    int c_bg =0x01;

    con_gotoxy(0,w_regs.y+0);
    con_attr2(c_reg,c_bg);

    for(y=0;y<8;y++)
    {
        con_print("  ");
        for(x=0;x<5;x++)
        {
            printq=printdec=0;
            if(x<4)
            {
                r=y+x*8;
                if(!view.showfpu)
                {
                    vname=regnames[r];
                    vnow=st.g[r].d;
                    vlast=stlast.g[r].d;
                    vnowhi=st.g[r].d2[1];
                }
                else
                {
                    vname=regname;
                    sprintf(regname,"%02i",r);
                    vnow=st.f[r].d;
                    vlast=stlast.f[r].d;
                }
            }
            else
            {
                switch(y)
                {
                case 0:  vname=" pc";
                         vnow=st.pc;
                         vlast=stlast.pc;
                         break;
                case 1:  vname="mlo";
                         vnow=st.mlo.d;
                         vlast=stlast.mlo.d;
                         break;
                case 2:  vname="mhi";
                         vnow=st.mhi.d;
                         vlast=stlast.mhi.d;
                         break;
                case 3:  vname="thr";
                         vnow=st.thread;
                         vlast=stlast.thread;
                         printdec=1;
                         break;
                case 4:  vname="fra";
                         vnow=st.frames;
                         vlast=stlast.frames;
                         printdec=1;
                         break;
                case 5:  vname="mio";
                         vnow=st.memiodetected;
                         vlast=0;
                         break;
                case 6:  vname="thT";
                         vnowq=st.threadtime;
                         vnow=vlast=0;
                         printq=1;
                         break;
                case 7:  vname="cpT";
                         vnowq=st.cputime;
                         vnow=vlast=0;
                         printq=1;
                         break;
                default: vname=NULL;
                         break;
                }
            }

            if(vname)
            {
                con_attr(c_num);
                con_printf("%s:",vname);
                if(x==0)
                {
                    if(vnowhi==0) con_printf("%08X:",vnowhi);
                    else          con_printf("........:");
                }
                if(vnow!=vlast) con_attr(c_chg);
                if(printq)
                {
                    dword lo,hi;
                    lo=vnowq&0xffffffff;
                    hi=(vnowq>>32)&0xffff;
                    con_printf("%04X%08X  ",hi,lo);
                }
                else if(printdec)
                {
                    con_printf("%-8i  ",vnow);
                }
                else if(vnow==0)
                {
                    if(view.showfpu==1) con_printf("  ........  ",vnow);
                    else con_printf("........  ",vnow);
                }
                else if(view.showfpu && x<4)
                {
                    float f;
                    f=*(float *)&vnow;
                    if(view.showfpu==2) con_printf("%08X  ",vnow);
                    else con_printf("%+10.4f  ",f);
                }
                else
                {
                    con_printf("%08X  ",vnow);
                }
            }
        }
        con_tabto(' ',256);
        con_print("\n");
    }
}

void view_help(void)
{
    int c_txt  =0x02;
    int c_bg   =0x00;
    int i=0,y;
    y=w_help.y;

    con_attr2(c_txt,c_bg);
    while(y<w_help.y+w_help.sy )
    {
        con_gotoxy(0,y++);
        if(helptext[i])
        {
            con_print(helptext[i]);
            i++;
        }
        else break;
        con_tabto(' ',256);
    }
}

void view_stat(void)
{
    int c_txt2 =0x0f;
    int c_txt  =0x07;
    int c_warn =0x0c;
    int c_bg   =0x01;
    int i,y,yi,x,ns;

    y=w_stat.y;
    con_gotoxy(0,y++);
    con_attr2(c_barfg,c_barbg);
    con_tabto('Ä',5);
    con_printf("F1");
    con_tabto('Ä',16);
    con_printf("StatsÄÄÄNewÄ>OldÄÄÄ(values for a single frame)ÄÄÄ");
    con_tabto('Ä',256);

    yi=0;
    while(y<w_stat.y+w_stat.sy )
    {
        con_gotoxy(0,y++);
        for(x=0;x<8;x++)
        {
            if(x==1) con_attr2(c_txt2,c_bg);
            else con_attr2(c_txt,c_bg);
            i=(ssi-(x-1)+STATS); i=((unsigned)i)%STATS;
            if(x>0)
            {

                if(ss[i].frame<-1) continue;
                if(ss[i].frame==-1) ns=1; else ns=0;
            }
            switch(yi)
            {
            case 0:
                if(!x) con_print("    frame:");
                else
                {
                    if(!ss[i].frame) con_printf("    start");
                    else if(ns)        con_printf("   nosync");
                    else               con_printf("%9i",ss[i].frame);
                }
                break;
            case 1:
                if(!x) con_print("      cpu:");
                else   con_printf("%8.1f%%",ss[i].p_cpu);
                break;
            case 2:
                if(!x) con_print("      gfx:");
                else   con_printf("%8.1f%%",ss[i].p_gfx);
                break;
            case 3:
                if(!x) con_print("    audio:");
                else   con_printf("%8.1f%%",ss[i].p_audio);
                break;
            case 4:
                if(!x) con_print("     misc:");
                else   con_printf("%8.1f%%",ss[i].p_misc);
                break;
            case 5:
                if(!x) con_print("     idle:");
                else   con_printf("%8.1f%%",ss[i].p_idle);
                break;
            case 6:
                if(!x) con_print("    total:");
                else   con_printf("%7.1fms",(float)ss[i].us_total/1000.0);
                break;
            case 7:
                if(!x) con_print("  cpu/ops:");
                else   con_printf("%8iK",ss[i].ops/1000);
                break;
            case 8:
                if(!x) con_print(" cpu/MIPS:");
                else   con_printf("%9.1f",ss[i].mips);
                break;
            case 9:
                if(!x) con_print(" cpu/fast:");
                else   con_printf("%8.2f%%",ss[i].compiled);
                break;
            case 10:
                if(!x) con_print(" memiochk:");
                else   con_printf("%9i",ss[i].memio);
                break;
            case 11:
                if(!x) con_print(" sync-tgt:");
                else   con_printf("%9.1f",ss[i].frametgt);
                break;
            case 12:
                if(!x) con_print("  snd/khz:");
                else   con_printf("%9.1f",0.001*ss[i].samplehz);
                break;
            case 13:
                if(!x) con_print(" snd/sync:");
                else
                {
                    if(ss[i].samplegap>=0x1000000)
                    {
                        con_attr(c_warn);
                        con_printf("    gap%2i",ss[i].samplegap&0xffff);
                    }
                    else
                    {
                       if(ss[i].samplegap<-9999 || ss[i].samplegap>9999)
                            con_attr(c_warn);
                       con_printf("%+9i",ss[i].samplegap);
                    }
                }
                break;
            case 14:
                if(!x) con_print(" snd/chan:");
                else   con_printf("%9.1f",ss[i].channels);
                break;
            case 15:
                if(!x) con_print(" gfx/tris:");
                else   con_printf("%9i",ss[i].trisin);
                break;
            case 16:
                if(!x) con_print("  gfx/vis:");
                else   con_printf("%9i",ss[i].tris);
                break;
            case 17:
                if(!x) con_print(" gfx/text:");
                else   con_printf("%7ikb",ss[i].txtbytes/1024);
                break;
            case 18:
                if(!x) con_print("  gfx/fps:");
                else   con_printf("%9.1f",ss[i].fps);
                break;
            }
            con_tabto(' ',10*(x+1));
        }
        con_tabto(' ',256);
        yi++;
    }

    {
        char szBuffer[256]; // added a local szbuffer, multithreading!!
        // GH - Update Status Bar Stats
        if( st.frames )
        {
          sprintf( szBuffer, "FRAME: %8u", (int)st.frames );
          SendMessage( hwndStatus, SB_SETTEXT, 1, (LPARAM)szBuffer );
        }
        else
        {
          sprintf( szBuffer, "CYCLE: %8u", (int)st.cputime );
          SendMessage( hwndStatus, SB_SETTEXT, 1, (LPARAM)szBuffer );
        }

        i=ssi; i=((unsigned)i)%STATS;
        if( ss[i].fps )
        {
          sprintf( szBuffer, "CPU: %5.1f%%", ss[i].p_cpu+ss[i].p_misc );
          SendMessage( hwndStatus, SB_SETTEXT, 2, (LPARAM)szBuffer );
          sprintf( szBuffer, "GFX: %5.1f%%", ss[i].p_gfx );
          SendMessage( hwndStatus, SB_SETTEXT, 3, (LPARAM)szBuffer );
          sprintf( szBuffer, "SND: %5.1f%%", ss[2].p_audio );
          SendMessage( hwndStatus, SB_SETTEXT, 4, (LPARAM)szBuffer );
          sprintf( szBuffer, "IDLE: %5.1f%%", ss[i].p_idle );
          SendMessage( hwndStatus, SB_SETTEXT, 5, (LPARAM)szBuffer );
          sprintf( szBuffer, "FPS: %5.1f", ss[i].fps );
          SendMessage( hwndStatus, SB_SETTEXT, 6, (LPARAM)szBuffer );
        }
    }
}

void view_data(void)
{
    int c_cur  =0x03;
    int c_txt  =0x07;
    int c_bg   =0x00;
    int y,i;
    int b;
    unsigned char buf[16];

    view.database&=~3;

    y=w_data.y;
    con_gotoxy(0,y++);
    con_attr2(c_barfg,c_barbg);
    if(view.active==WIN_DATA) con_print("\x1f\x1f\x1f\x1f");
    con_tabto('Ä',5);
    con_printf("F2");
    con_tabto('Ä',16);
    con_printf("Data: %08X",view.database);
    con_tabto('Ä',256);

    if(!mem.ram) return;

    con_attr2(c_txt,c_bg);
    b=view.database;
    while(y<w_data.y+w_data.sy)
    {
        con_gotoxy(0,y++);
        con_printf("%08X:  ",b);

        for(i=0;i<16;i++) buf[i]=mem_read8(b+i);

        switch(view.datatype)
        {
        case 4:
            for(i=0;i<16;i++)
            {
                con_printf("%02X",buf[i]);
                if((i&3)==3) con_print(" ");
            }

            for(i=0;i<16;i++)
            {
                if(buf[i]<32) con_printchar('.');
                else          con_printchar(buf[i]);
                if((i&3)==3) con_print(" ");
            }
            b+=16;
            break;
        case 1:
            for(i=0;i<16;i++)
            {
                con_printf("%02X ",buf[i]);
                if((i&7)==7) con_print(" ");
            }

            for(i=0;i<16;i++)
            {
                if(buf[i]<32) con_printchar('.');
                else          con_printchar(buf[i]);
            }

            b+=16;
            break;
        }

        con_tabto(' ',256);
    }
}

void view_code(void)
{
    int c_cur  =0x03;
    int c_hex  =0x03;
    int c_type =0x08;
    int c_txt  =0x07;
    int c_txt2 =0x02;
    int c_bg   =0x00;
    int c_bg2  =0x01;
    int y,b;
    char *optype;
    dword opcode;

    view.codebase&=~3;

    y=w_code.y;
    con_gotoxy(0,y++);
    con_attr2(c_barfg,c_barbg);
    if(view.active==WIN_CODE) con_print("\x1f\x1f\x1f\x1f");
    con_tabto('Ä',5);
    con_printf("F3");
    con_tabto('Ä',16);
    con_printf("Code: %08X ÄÄÄ PC: %s",view.codebase,sym_find(st.pc));
    con_tabto('Ä',256);

    if(!mem.ram) return;

    b=view.codebase;
    while(y<w_code.y+w_code.sy)
    {
        con_gotoxy(0,y++);
        if(st.pc==b) con_attr2(c_txt,c_bg2);
        else         con_attr2(c_txt,c_bg);

        con_printf("%08X:  ",b);

        opcode=mem_readop(b);
        optype=mem_readoptype(b);

        con_attr(c_hex);
        con_printf("%08X ",opcode);
        con_attr(c_type);
        con_printf("%-4s ",optype);

        if(opcode==0x03e00008) con_attr(c_txt2); // jr
        else if(OP_OP(opcode)>=1 && OP_OP(opcode)<=7) con_attr(c_txt2); // jal or other branch
        else if(OP_OP(opcode)>=20 && OP_OP(opcode)<=23) con_attr(c_txt2); // jal or other branch
        else con_attr(c_txt);
        con_print(disasm(b,opcode));

        if(st.pc==b && st.branchdelay>0)
        {
            con_tabto(' ',74);
            if(st.branchtype==1)      con_printf(WHITE"Call ");
            else if(st.branchtype==2) con_printf(WHITE"Ret  ");
            else
            {
                if(st.branchto<st.pc) con_printf(WHITE"Jump\x18");
                else                  con_printf(WHITE"Jump\x19");
            }
        }

        con_tabto(' ',256);
        b+=4;
    }
}

void view_cons(void)
{
    int c_cur  =0x03;
    int c_txt  =0x02;
    int c_bg   =0x00;
    int y,i;

    y=w_cons.y;
    con_gotoxy(0,y++);
    con_attr2(c_barfg,c_barbg);
    if(view.active==WIN_CONS) con_print("\x1f\x1f\x1f\x1f");
    con_tabto('Ä',5);
    con_printf("F4");
    con_tabto('Ä',256);

    i=view.consolerow+w_cons.sy-2;
    while(y<w_cons.y+w_cons.sy)
    {
        if(!i) con_cursorxy(view.consolecursor,y,25);
        con_gotoxy(0,y++);
        con_attr2(c_txt,c_bg);
        con_print(console.line[i--]);
        con_tabto(' ',256);
    }

    if(view.consolerow>0) con_cursorxy(0,0,0);
}

void view_redraw(void)
{
    if(!view.changed)
    {
        return;
    }
    if(con_resized() || (view.changed&VIEW_RESIZE))
    {
        view_checksize();
    }
    if(view.changed&VIEW_CLEAR)
    {
        con_clear();
    }
    if(view.changed&VIEW_HELP)
    {
        if(w_help.sy) view_help();
    }
    if(view.changed&VIEW_STAT)
    {
        if(w_stat.sy) view_stat();
    }
    if(view.changed&VIEW_REGS)
    {
        if(w_regs.sy) view_regs();
    }
    if(view.changed&VIEW_DATA)
    {
        if(w_data.sy) view_data();
    }
    if(view.changed&VIEW_CODE)
    {
        if(w_code.sy) view_code();
    }
    if(view.changed&VIEW_CONS)
    {
        view_cons();
    }

    con_gotoxy(0,con_rows()-1);
    con_attr2(0,3);
    con_print(statusline);

    con_gotoxy(0,0); // will flush output buffers
    view.changed=0;
}

void view_open(void)
{
    int i;

    view.changed=-1;
    view.codebase=0x80246000;
    view.database=0x10000000;
    view.datatype=1;
    view.showhelp=0;
    view.active=0;

    if(init.showconsole && !RELEASE)
    {
        con_init();
        SetConsoleTitle( "UltraHLE Debug Console" );
    }
    else
    {
        con_initdummy();
    }

    memset(&console,0,sizeof(console));
    for(i=0;i<LINES;i++)
    {
        console.line[i]=console.linedata[i];
    }

    viewopen=1;
}

void view_close(void)
{
    viewopen=0;

    con_clear();
    con_deinit();
}

void view_status(char *text)
{
    int len=strlen(text);
    if(len>78) len=78;
    memset(statusline,' ',80);
    statusline[80]=0;
    memcpy(statusline+1,text,len);

    con_gotoxy(0,con_rows()-1);
    con_attr2(0,3);
    con_print(statusline);
}

void view_writeconsole(char *text)
{
    if(!viewopen) return;

    while(*text)
    {
        if(*text==0x1 || *text==0x2)
        {
            console.line[0][console.writepos++]=*text++;
            console.line[0][console.writepos++]=*text;
        }
        else if(*text=='\r')
        {
            console.line[0][console.writepos]=0;
            console.writepos=0;
        }
        else if(*text=='\n')
        {
            char *p;
            console.line[0][console.writepos]=0;
            console.writepos=0;
            // scroll
            p=console.line[LINES-1];
            memmove(console.line+1,console.line+0,sizeof(char *)*(LINES-1));
            console.line[0]=p;
            memset(console.line[0],0,LINELEN);
        }
        else if(console.writepos<LINELEN-1)
        {
            console.line[0][console.writepos++]=*text;
        }
        text++;
        if(console.writepos>80) console.writepos=80;
    }
}

void view_setlast(void)
{
    memcpy(&stlast,&st,sizeof(st));
}


/****************************************************************************
** Keyboard handling
*/

static char editline[256];
static int  editpos;

#define HISTORY 64

static char lastcmd[256];
static char breakcmd[256];
static char tmpcmd[256];
static char history[HISTORY][256];
static int  historypos;
static int  scrolling;
static void conkey(int a);

void docommand(char *editline)
{
    int i;
    if(!*editline) return;
    if(*editline=='+') strcpy(editline,lastcmd);
    else strcpy(lastcmd,editline);
    // put to history (unless same as last)
    if(strcmp(editline,history[0]))
    {
        for(i=HISTORY-1;i>0;i--)
        {
            strcpy(history[i],history[i-1]);
        }
        strcpy(history[0],editline);
    }
    historypos=-1;
    command(editline);
    conkey(0);
    view_changed(VIEW_ALL);
}

void flushdisplay(void)
{
    // update code window pos
    if(st.pc<view.codebase+4 || st.pc>view.codebase+view.coderows*4-8)
    {
        view.codebase=st.pc-4*(view.coderows/3);
        view_changed(VIEW_CODE);
    }

    flushlog();
    view_changed(VIEW_ALL);
    view_redraw();
}

void datakey(int a)
{
    switch(a)
    {
    case KEY_UP:
        view.database-=0x10;
        break;
    case KEY_DOWN:
        view.database+=0x10;
        break;
    case KEY_PGUP:
        view.database-=0x10*view.datarows;
        break;
    case KEY_PGDN:
        view.database+=0x10*view.datarows;
        break;
    }
    view_changed(VIEW_DATA);
}

void codekey(int a)
{
    switch(a)
    {
    case KEY_UP:
        view.codebase-=4;
        break;
    case KEY_DOWN:
        view.codebase+=4;
        break;
    case KEY_PGUP:
        view.codebase-=4*view.coderows;
        break;
    case KEY_PGDN:
        view.codebase+=4*view.coderows;
        break;
    }
    view_changed(VIEW_CODE);
}

void conkey(int a)
{
    int editlen=strlen(editline);
    int resetrow=1,x;

    switch(a)
    {
    case KEY_PGUP:
        resetrow=0;
        view.consolerow+=view.consrows-1;
        scrolling=1;
        break;
    case KEY_PGDN:
        resetrow=0;
        if(view.consolerow==0) scrolling=0;
        else scrolling=1;
        view.consolerow-=view.consrows-1;
        break;
    case KEY_UP:
        if(scrolling)
        {
            resetrow=0;
            view.consolerow+=1;
        }
        else
        {
            x=historypos+1;
            if(x<HISTORY && *history[x])
            {
                historypos=x;
                strcpy(editline,history[historypos]);
                editpos=strlen(editline);
            }
        }
        break;
    case KEY_DOWN:
        if(scrolling)
        {
            resetrow=0;
            view.consolerow-=1;
        }
        else
        {
            x=historypos-1;
            if(x>0 && *history[x])
            {
                historypos=x;
                strcpy(editline,history[historypos]);
                editpos=strlen(editline);
            }
        }
        break;
    case KEY_LEFT:
        editpos--;
        if(editpos<0) editpos=0;
        break;
    case KEY_RIGHT:
        editpos++;
        if(editpos>editlen) editpos=editlen;
        break;
    case KEY_HOME:
        editpos=0;
        break;
    case KEY_END:
        editpos=editlen;
        break;
    case KEY_BKSPACE:
        if(editpos==0) break;
        editpos--;
    case KEY_DEL:
        if(editline[editpos])
            memmove(editline+editpos,editline+editpos+1,256-1-editpos);
        break;
    case KEY_ENTER:
        view_writeconsole("\x01\x17");
        print(": %s\n",editline);
        // execute
        docommand(editline);
        // flow on
    case KEY_ESC:
        // clear
        scrolling=0;
        *editline=0;
        editpos=0;
        break;
    default:
        if(a>=32 && a<256)
        {
            memmove(editline+editpos+1,editline+editpos,256-1-editpos);
            editline[editpos++]=a;
        }
        else resetrow=0;
        break;
    }

    if(scrolling) view_status("scrolling (esc to stop)");
    else view_status("ready (pgup to scroll console)");
    if(editpos>77) editpos=77;

    if(resetrow) view.consolerow=0;
    if(view.consolerow<0) view.consolerow=0;
    else if(view.consolerow>1024-50) view.consolerow=1024-50;

    view.consolecursor=editpos+2;
    view_writeconsole("\x01\x17: ");
    view_writeconsole(editline);
    view_writeconsole("\r");

    view_changed(VIEW_CONS);
}

void debugui_key(int a)
{
    int flag=1;
    if(a>=KEY_F1 && a<=KEY_F12) command_fkey(a);
    else
    {
        flag=0;
        if(a>=32 && a<256 && view.active!=WIN_CONS)
        {
            view.active=WIN_CONS;
            flag=1;
        }
        if(view.active==WIN_DATA) datakey(a);
        else if(view.active==WIN_CODE) codekey(a);
        else if(view.active==WIN_CONS) conkey(a);
    }

    if(flag) view_changed(VIEW_ALL);
}

// break execution and then execute
void breakcommand(char *cmd)
{
    if(*breakcmd) return; // ignore command, still processing last
    strcpy(breakcmd,cmd);
    cpu_nicebreak();
}

/****************************************************************************
** Main loop
*/

void debugui(void)
{
    int key;
    conkey(0);
    while(!exitflag)
    {
        view_redraw();
        if(*breakcmd)
        {
            print(NORMAL":: %s\n",breakcmd);
            strcpy(tmpcmd,breakcmd);
            *breakcmd=0;
            command(tmpcmd);
            flushdisplay();
        }
        else
        {
            key=con_readkey_noblock();
            if(key)
            {
                debugui_key(key);
            }
            else
            {
                con_sleep(10);
            }
        }
        con_sleep(0);
    }
}

