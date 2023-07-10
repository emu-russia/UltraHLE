////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// listview.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.
#include "version.h"                   // Version Information

	// Prototypes

   void CreateListView( void );
   BOOL UpdateROMList( void );
   void AddDebugLine( char *, int );

	// Globals

   HWND hwndList;                         // Handle to Rom List View
   HWND hwndDebug;                        // Handle to Debug List View
   ROMLIST romList;                       // Pointer to Rom List Information
   
   extern char szBuffer[];                // Temporary String Buffer
   extern HANDLE hInst;                   // Global Application Instance
   extern HWND hwndMain;                  // Handle to the Main App Window
