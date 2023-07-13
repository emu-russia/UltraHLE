#include "pch.h"

xt_state g_state;
xt_stats g_stats;

void zerobase()
{
}

void mysleep(DWORD dwMilliseconds)
{
	Sleep(dwMilliseconds);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}
