////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// loadsave.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

/**
 * Opens a file dialog and loads either a ROM image or a saved state, depending on the flag.
 * @param loadType TRUE to load a ROM image, FALSE to load a saved state.
 * @return TRUE on success, FALSE if no file was selected.
 */
BOOL LoadImageState( BOOL );
/**
 * Saves the current emulator state to a file chosen via the Save As dialog.
 * @return TRUE on success, FALSE if no file was selected.
 */
BOOL SaveState( void );
/**
 * Entry point of the main application window.
 * @return Window result code.
 */
extern int mainwindow( int );

// Globals

/** Structure used by the Open and Save common dialogs. */
OPENFILENAME OpenFileName;          // Structure for Open Common Dialog
/** Path of the ROM image file. */
char romfilename[ MAX_PATH ];

extern char szBuffer[ MAX_PATH ];   // Temporary String Buffer
extern HWND hwndMain;               // Handle to the Main App Window
extern HANDLE hInst;                // Global Application Instance
   
extern HANDLE mainthread;
extern LPDWORD mainthreadid;

#ifdef __cplusplus
};
#endif