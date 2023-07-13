#include "pch.h"

//.data:00000000 _data           segment dword public 'DATA' use32
//.data:00000000                 assume cs:_data
//.data:00000008 ; `x_fastfpu'::`2'::state
//.data:00000008 ?state@?1??x_fastfpu@@9@9 dd 0          ; DATA XREF: _x_fastfpu+17↓r
//.data:00000008                                         ; _x_fastfpu:loc_28E↓w
//.data:0000006C _data           ends

FILE* logfile;
char* logfilename = "x.log";
uint32_t initdone;
LARGE_INTEGER perf_freq;
LARGE_INTEGER perf_counter;

FILE * log_open(char *Mode)
{
	FILE *result; // eax

	if (logfile)
		fclose(logfile);
	result = (FILE *)Mode;
	if ( Mode )
	{
		result = fopen(logfilename, Mode);
		logfile = result;
	}
	return result;
}

void x_log(char* Format, ...)
{
	static char buf[0x100];
	FILE *result; // eax
	va_list va; // [esp+8h] [ebp+8h]

	va_start(va, Format);
	vsprintf(buf, Format, va);
	result = (FILE *)fputs(buf, logfile);
	if ( *Format != 35 )
		result = log_open("at");
}

void breakpoint()
{
	__debugbreak();
}

void x_fatal(char* Format, ...)
{
	static char buf[0x100];
	va_list va; // [esp+8h] [ebp+8h]

	va_start(va, Format);
	vsprintf(buf, Format, va);
	fprintf(logfile, "fatal: ");
	fprintf(logfile, buf);
	log_open("at");
	if ( g_activestateindex )
		x_close(g_activestateindex);
	exit(4);
}

void* x_allocfast(int Size)
{
	void *result; // eax

	result = malloc(Size);
	if ( !result )
		x_fatal("out of memory allocating %i bytes", Size);
	return result;
}

void* x_alloc(int Size)
{
	void *result; // eax

	result = x_allocfast(Size);
	memset(result, 0, Size);
	return result;
}

void* x_realloc(void* Memory, int NewSize)
{
	size_t v2; // edi
	void *result; // eax

	v2 = NewSize;
	if ( !Memory )
		return x_alloc(NewSize);
	if ( (signed int)NewSize <= 0 )
		v2 = 1;
	result = realloc(Memory, v2);
	if ( !result )
		x_fatal("out of memory allocating %i bytes", v2);
	return result;
}

void x_free(void *Memory)
{
	free(Memory);
}

void x_fastfpu(int a1)
{
	int v1; // eax
	bool v2; // zf
	unsigned int result; // eax

	v1 = `x_fastfpu'::`2'::state;
	if ( a1 )
	{
		v1 = `x_fastfpu'::`2'::state + 1;
	}
	else if ( `x_fastfpu'::`2'::state > 0 )
	{
		v1 = `x_fastfpu'::`2'::state - 1;
	}
	`x_fastfpu'::`2'::state = v1;
	v2 = v1 == 0;
	result = `x_fastfpu'::`2'::originalcontrol;
	if ( !v2 )
		result = `x_fastfpu'::`2'::originalcontrol & 0xFFFFFCFF | 0x3F;
	`x_fastfpu'::`2'::newcontrol = result;
	return result;
}

void x_timereset(void)
{
	BOOL result;

	if ( !QueryPerformanceFrequency(&perf_freq) )
		x_fatal("This computer does not support Pentium Performance Counter\n");
	result = QueryPerformanceCounter(&perf_counter);
	initdone = 1;
}

int x_timeus(void)
{
	LARGE_INTEGER PerformanceCount;

	if ( !initdone)
		x_timereset();
	QueryPerformanceCounter(&PerformanceCount);
	return 1000000 * (PerformanceCount.QuadPart - perf_counter.QuadPart) / perf_freq.QuadPart;
}

int x_timems(void)
{
	LARGE_INTEGER PerformanceCount;

	if ( !initdone)
		x_timereset();
	QueryPerformanceCounter(&PerformanceCount);
	return 1000 * (PerformanceCount.QuadPart - perf_counter.QuadPart) / perf_freq.QuadPart;
}

//.bss:000003C8 _bss            segment para public 'BSS' use32
//.bss:000003C8                 assume cs:_bss
//.bss:000003C8                 ;org 3C8h
//.bss:000003C8                 assume es:nothing, ss:nothing, ds:_data, fs:nothing, gs:nothing
//.bss:000005E0 ; `x_fastfpu'::`2'::newcontrol
//.bss:000005E0 ?newcontrol@?1??x_fastfpu@@9@9 dd ?     ; DATA XREF: _x_fastfpu+3C↑w
//.bss:000005E0                                         ; _x_fastfpu+41↑r ...
//.bss:000005E4 ; `x_fastfpu'::`2'::originalcontrol
//.bss:000005E4 ?originalcontrol@?1??x_fastfpu@@9@9 dd ?
//.bss:000005E4                                         ; DATA XREF: _x_fastfpu+9↑w
//.bss:000005E4                                         ; _x_fastfpu+2D↑r
//.bss:000005E8 ; `x_fastfpu'::`2'::initdone
//.bss:000005E8 ?initdone@?1??x_fastfpu@@9@9 dd ?       ; DATA XREF: _x_fastfpu↑r
//.bss:000005E8 _bss            ends
