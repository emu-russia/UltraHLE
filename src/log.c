#include "ultra.h"

void printtxt(char *txt)
{
    static FILE *logfile;
    char *p;
    static int printedstuff=0;
    static int flushcnt=0;

    if(init.showconsole)
    {
        if(!logfile) logfile=fopen("ultra.log","wt");
        if(!txt)
        {
            if(flushcnt)
            {
                flushcnt=0;
                fflush(logfile);
            }
            return;
        }

        flushcnt++;

        view_writeconsole(txt);

        if(flushcnt>1000)
        {
            // redraw screen every 1000 messages
            flushdisplay();
        }
    }
    else
    {
        if(!txt) return;
    }

    p=txt;
    while(*p)
    {
        if(*p==0x01) p+=2;
        else if(*p==0x02) p+=2;
        else break;
    }

    if(init.showconsole)
    {
        fputs(p,logfile);
    }

    outputhook(p,txt); // for new ui
}

void print(char *txt,...) // generic
{
    static char buf[256];
    va_list argp;

    va_start(argp,txt);

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    vsprintf(buf,txt,argp);
    printtxt(buf);
}

void logd(char *txt,...) // [d]isplay
{
    static FILE *logfile;
    static char buf[256];
    va_list argp;
    char *p;

    if(!st.dumpgfx) return;

    va_start(argp,txt);

    if(!logfile) logfile=fopen("dlist.log","wt");
    if(!txt)
    {
        fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);
    p=buf;
    fputs(p,logfile);
}

void loga(char *txt,...) // [a]udio
{
    static FILE *logfile;
    static char buf[256];
    va_list argp;
    char *p;

    if(!st.dumpsnd) return;

    va_start(argp,txt);

    if(!logfile) logfile=fopen("slist.log","wt");
    if(!txt)
    {
        fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);

    p=buf;
    fputs(p,logfile);
}

void logo(char *txt,...) // [o]s
{
    static char buf[256];
    va_list argp;

    if(!st.dumpos) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

void logh(char *txt,...) // [h]w
{
    static char buf[256];
    va_list argp;

    if(!st.dumphw) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

void logc(char *txt,...) // [c]ompiler
{
    static char buf[256];
    va_list argp;

    if(!st.dumpasm) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

// info logging
void logi(char *txt,...)
{
    static char buf[256];
    va_list argp;

    if(!st.dumpinfo) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);
    vsprintf(buf,txt,argp);
    printtxt(buf);
}

void flushlog(void)
{
    loga(NULL);
    logd(NULL);
    printtxt(NULL);
}

// errors/warnings

void exception(char *txt,...)
{
    static char buf[256];
    va_list argp;

    st2.exception=1;
    va_start(argp,txt);
    sprintf(buf,YEL"exception(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(1)
    {
        print(buf);
        cpu_break();
    }
}

void error(char *txt,...)
{
    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"error(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(st.stoperror)
    {
        print(buf);
        cpu_break();
    }
    else
    {
        print(buf);
    }
}

void warning(char *txt,...)
{
    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"warning(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(st.stopwarning)
    {
        print(buf);
        cpu_break();
    }
    else
    {
        print(buf);
    }
}

