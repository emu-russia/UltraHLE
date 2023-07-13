#include "pch.h"

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

/// <summary>
/// - x_fastfpu(1) sets the fpu to 32-bit accuracy mode (faster).
/// - x_fastfpu(0) returns the fpu to mode before call to x_fastfpu(1)
/// - the above calls can be nested
/// </summary>
void x_fastfpu(int fast)
{
	static int state = 0;
	static int newcontrol = 0;
	static int originalcontrol = 0;
	static int initdone = 0;

	int v1; // eax
	bool v2; // zf
	unsigned int result; // eax

	if (!initdone) {

		originalcontrol = _controlfp(0, 0);
		// Bugfix
		initdone = 1;
	}

	v1 = state;
	if (fast)
	{
		v1 = state + 1;
	}
	else if ( state > 0 )
	{
		v1 = state - 1;
	}
	state = v1;

	v2 = v1 == 0;
	result = originalcontrol;
	if ( !v2 )
		result = originalcontrol & 0xFFFFFCFF | 0x3F;
	newcontrol = result;

	_controlfp(newcontrol, _MCW_PC);
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
