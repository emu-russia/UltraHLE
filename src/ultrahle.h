/**
 * \file ultrahle.h
 * Declarations for the main Win32 application module (ultrahle.c).
 */

////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, XXX and RealityMan
// ultrahle.h

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.
#include "version.h"                   // Version Information
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

/**
 * Application entry point: registers the window class, creates the main window and runs the message loop.
 * @param hThisInst Handle to the current program instance.
 * @param hPrevInst Handle to the previous instance (obsolete).
 * @param lpszArgs Command line arguments.
 * @param nWinMode Initial window display mode.
 * @return The wParam of the last message processed.
 */
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
/**
 * Window procedure of the main application window.
 * @param hwnd Window handle.
 * @param message Window message.
 * @param wParam Message parameter.
 * @param lParam Message parameter.
 * @return Message result.
 */
LRESULT CALLBACK WindowFunc( HWND, UINT, WPARAM, LPARAM );
/**
 * Dialog procedure of the About box.
 * @param hdwnd Dialog window handle.
 * @param message Dialog message.
 * @param wParam Message parameter.
 * @param lParam Message parameter.
 * @return Message result.
 */
LRESULT CALLBACK AboutDialog( HWND, UINT, WPARAM, LPARAM );
/**
 * Displays the controller configuration property sheet.
 * @return The result of the property sheet.
 */
int ControllerProperties( void );
/**
 * Dialog procedure of the controller 1 configuration tab.
 * @param hDlg Dialog window handle.
 * @param message Dialog message.
 * @param wParam Message parameter.
 * @param lParam Message parameter.
 * @return Message result.
 */
LRESULT APIENTRY Controller1Prop( HWND, UINT, WPARAM, LPARAM );

extern void CreateListView( void );
extern BOOL UpdateROMList( void );
extern BOOL LoadImageState( BOOL );
extern BOOL SaveState( void );

// Globals

/** Temporary string buffer. */
char szBuffer[ MAX_PATH ];             // Temporary String Buffer
/** Global application instance handle. */
HANDLE hInst;                          // Global Application Instance
/** Handle to the main application window. */
HWND hwndMain;                         // Handle to the Main App Window
/** Handle to the status bar window. */
HWND hwndStatus;                       // Handle to Status Bar

/** Handle of the emulator thread. */
HANDLE mainthread;                     
/** Pointer to the emulator thread identifier. */
LPDWORD mainthreadid;

/** Handle to the ROM list view. */
extern HWND hwndList;                  // Handle to Rom List View
/** Handle to the debug list view. */
extern HWND hwndDebug;                 // Handle to Debug List View
/** Pointer to the ROM list information. */
extern ROMLIST *romList;               // Pointer to Rom List Information

extern Init init;

#ifdef __cplusplus
};
#endif
