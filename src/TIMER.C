#include <windows.h>
#include "ultra.h"

static LARGE_INTEGER  perf_freq;
static int            initdone;

void timer_reset(Timer *t)
{
    if(!initdone)
    {
        if(!QueryPerformanceFrequency(&perf_freq))
        {
            exception("This computer does not support Pentium Performance Counter\n");
        }
        initdone=1;
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&t->perf_zero);
}

int  timer_us(Timer *t)
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return((int)(1000000*(pc.QuadPart-((LARGE_INTEGER *)&t->perf_zero)->QuadPart)/perf_freq.QuadPart));
}

int  timer_ms(Timer *t)
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return((int)(1000*(pc.QuadPart-((LARGE_INTEGER *)&t->perf_zero)->QuadPart)/perf_freq.QuadPart));
}

int  timer_usreset(Timer *t)
{
    LARGE_INTEGER pc;
    int r;
    QueryPerformanceCounter(&pc);
    r=(int)(1000000*(pc.QuadPart-((LARGE_INTEGER *)&t->perf_zero)->QuadPart)/perf_freq.QuadPart);
    *(LARGE_INTEGER *)&t->perf_zero=pc;
    return(r);
}

int  timer_msreset(Timer *t)
{
    LARGE_INTEGER pc;
    int r;
    QueryPerformanceCounter(&pc);
    r=(int)(1000*(pc.QuadPart-((LARGE_INTEGER *)&t->perf_zero)->QuadPart)/perf_freq.QuadPart);
    *(LARGE_INTEGER *)&t->perf_zero=pc;
    return(r);
}



