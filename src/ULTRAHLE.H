////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, XXX and RealityMan
// ultrahle.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.
#include "version.h"                   // Version Information
#include "main.h"

   // Prototypes

   int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
   LRESULT CALLBACK WindowFunc( HWND, UINT, WPARAM, LPARAM );
   BOOL CALLBACK AboutDialog( HWND, UINT, WPARAM, LPARAM );
   int ControllerProperties( void );
   BOOL APIENTRY Controller1Prop( HWND, UINT, UINT, LONG );

   extern void CreateListView( void );
   extern BOOL UpdateROMList( void );
	extern BOOL LoadImageState( BOOL );
   extern BOOL SaveState( void );

   // Globals

   char szBuffer[ MAX_PATH ];             // Temporary String Buffer
   HANDLE hInst;                          // Global Application Instance
   HWND hwndMain;                         // Handle to the Main App Window
   HWND hwndStatus;                       // Handle to Status Bar

   HANDLE mainthread;                     
   LPDWORD mainthreadid;

	extern HWND hwndList;                  // Handle to Rom List View
	extern HWND hwndDebug;                 // Handle to Debug List View
   extern ROMLIST *romList;               // Pointer to Rom List Information

   extern Init init;

