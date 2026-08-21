/**
 * \file listview.h
 * Declarations for the ROM list and debug output list views.
 */

////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// listview.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.
#include "version.h"                   // Version Information

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

/** Creates the ROM list and debug output list view windows. */
void CreateListView( void );
/**
 * Scans the ROM directory and fills the ROM list view with the found images.
 * @return TRUE on success.
 */
BOOL UpdateROMList( void );
/**
 * Adds one line of text to the debug list view.
 * @param szBuf Text to add, or NULL to clear the list.
 * @param lineno Line number (ignored; an internal counter is used).
 */
void AddDebugLine( char *, int );

// Globals

HWND hwndList;                         // Handle to Rom List View
HWND hwndDebug;                        // Handle to Debug List View
/** Global ROM list information. */
ROMLIST romList;                       // Pointer to Rom List Information
   
extern char szBuffer[];                // Temporary String Buffer
extern HANDLE hInst;                   // Global Application Instance
extern HWND hwndMain;                  // Handle to the Main App Window

#ifdef __cplusplus
};
#endif