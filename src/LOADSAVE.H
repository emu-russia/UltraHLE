////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// loadsave.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.

   // Prototypes

	BOOL LoadImageState( BOOL );
   BOOL SaveState( void );
   extern int mainwindow( int );

   // Globals

   OPENFILENAME OpenFileName;          // Structure for Open Common Dialog
   char romfilename[ MAX_PATH ];

   extern char szBuffer[ MAX_PATH ];   // Temporary String Buffer
	extern HWND hwndMain;               // Handle to the Main App Window
	extern HANDLE hInst;                // Global Application Instance
   
   extern HANDLE mainthread;
   extern LPDWORD mainthreadid;



