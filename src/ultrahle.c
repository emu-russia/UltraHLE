////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// ultrahle.c

#include "ultrahle.h"

   // ST: emulator thread; basically calls main.c which returns
   // if debugui is ever exited with 'x'.

   int emuthread(int value)
   {
       main_thread();
       // exit main program too if debugui is exited
       SendMessage(hwndMain,WM_CLOSE,0,0);
       // end thread
       return(0);
   }

   // Main Application Code Starts Here

   int WINAPI WinMain( HINSTANCE hThisInst,  // Handle to this Program Instance
                       HINSTANCE hPrevInst,  // Previous Instance (Obsolete)
                       LPSTR lpszArgs,       // Passed Arguments
                       int nWinMode )        // Windows Display Mode
   {
      WNDCLASSEX wclex;                // Pointer to Windows Class
      HWND       hwnd;                 // Window Handle
      MSG        msg;                  // Windows Message Pointer
      HACCEL     hAccel;               // Accelerator Resource

      // Store the Global Application Instance Handle

      hInst = hThisInst;

	  // ST: main_startup reads program parameters and sets up paths.
      // This may show a message box with command line help. MUST be
      // called before main window is created, since this sets up
      // paths, that are used by listview. Also opens console window.
      main_startup();

      // Define the Primary Windows Class

      wclex.hInstance     = hInst;
      wclex.lpszClassName = APPNAME;
      wclex.lpfnWndProc   = WindowFunc;
      wclex.style         = 0;
      wclex.cbSize        = sizeof( WNDCLASSEX );
      wclex.hIcon         = LoadIcon( hThisInst, MAKEINTRESOURCE(IDI_N64LOGO) );
      wclex.hIconSm       = LoadIcon( hThisInst, MAKEINTRESOURCE(IDI_N64LOGO) );
      wclex.hCursor       = LoadCursor( NULL, IDC_ARROW );
      wclex.lpszMenuName  = MAKEINTRESOURCE( IDR_MAINMENU );
      wclex.cbClsExtra    = 0;
      wclex.cbWndExtra    = 0;
      wclex.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );

      // Register the Window Class

      if( !RegisterClassEx( &wclex ) )
         return( 0 );

      // Create the Main Application Window

      wsprintf( szBuffer, "%s - %s v%i.%i.%i",
                APPNAME, TITLE, MAJORREV, MINORREV, PATCHLVL );

      hwnd = CreateWindow( APPNAME,
                           szBuffer,
                           WS_OVERLAPPEDWINDOW,
                           50, 50, 600, 450,
                           HWND_DESKTOP,
                           NULL,
                           hInst,
                           NULL );

      // Initialise the Common Controls Library (Mainly for Open File Dialog)

      InitCommonControls();

      // Load the Accelerator Key Table

      hAccel = LoadAccelerators( hInst, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );

      // Display and Update the Application Window
      
      ShowWindow( hwnd, nWinMode );
      UpdateWindow( hwnd );

      // ST: main_thread is the main emulator thread and will run in
      // the background. By default it loads an empty 'dummy' cart
      // and does nothing. Use main_command() to communicate with it
      {
          int mainthreadid;
          mainthread=CreateThread(NULL,0,
                     (LPTHREAD_START_ROUTINE)emuthread,
                     NULL,0,&mainthreadid );
      }
      
      // Windows Message Processing Loop

      while( GetMessage( &msg, NULL, 0, 0 ) )
      {
         if( !TranslateAccelerator( hwnd, hAccel, &msg ) )
         {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
         }
      }

      // Exit the Application

      return( msg.wParam );
   }

   // Application Window Message Processing

   LRESULT CALLBACK WindowFunc( HWND hwnd,      // Handle to Application Window
                                UINT message,   // Windows Message
                                WPARAM wParam,  // Message Parameter
                                LPARAM lParam ) // Message Parameter
   {
      static int aWidths[7];
      RECT rcl;                        // Window Size
      HDWP hdwp;					   // Deferred Window Position Structure
      HMENU hMenu, hSubMenu;           // Handle to the Menu and Sub Menus
      NMLISTVIEW *pNm;                 // List View Notifications

      switch( message )
      {
         // Do Application Initialisation

         case WM_CREATE:

            // Set Main Window Handle

            hwndMain = hwnd;

            // Create the Status Bar

            hwndStatus = CreateWindowEx( 0L,
                                         STATUSCLASSNAME,
                                         "",
                                         WS_CHILD | WS_BORDER |
                                         WS_VISIBLE | SBS_SIZEGRIP,
                                         0, 0, 0, 0,
                                         hwndMain,
                                         (HMENU)ID_STATUSBAR,
                                         hInst,
                                         NULL );
            if( hwndStatus == NULL )
               MessageBox( NULL, 
                           "Status Bar not Created",
                           "Error",
                           MB_ICONEXCLAMATION | MB_OK );

            // Create the ROM List View

            SetCursor(LoadCursor(NULL,IDC_WAIT));
            CreateListView();
			   UpdateROMList();
            SetCursor(LoadCursor(NULL,IDC_ARROW));

            // Default GFX Mode
            
            init.gfxwid=640;
            init.gfxhig=480;

            break;

         // Handle Application Defined Windows Messages

         case WM_COMMAND:

            switch( LOWORD( wParam ) )
            {
					// Open and Load ROM Image

					case IDM_FILE_OPEN:

						LoadImageState( TRUE );

						break;

					// Open and Load Saved State

					case IDM_FILE_LOADSTATE:

						LoadImageState( FALSE );

						break;

					// Save Emulator State

					case IDM_FILE_SAVESTATE:

                  SaveState();

						break;

					// Quickload 'ultra.sav'

					case IDM_FILE_QUICKLOAD:

                        // S: Need to call Quickload Command

                  SetCursor(LoadCursor(NULL,IDC_WAIT));
                  main_command("load");
                  SetCursor(LoadCursor(NULL,IDC_ARROW));

						break;

					// Quicksave 'ultra.save'

					case IDM_FILE_QUICKSAVE:

                        // S: Need to call Quicksave Command

                   SetCursor(LoadCursor(NULL,IDC_WAIT));
                   main_command("save");
                   SetCursor(LoadCursor(NULL,IDC_ARROW));

						break;

               case IDM_FILE_EXIT:

                  SendMessage( hwndMain, WM_DESTROY, 0, 0L );

                  break;

					// Start Emulation

					case IDM_EMULATION_START:

                        main_start();
                        UpdateWindow(hwndStatus);
                        SetFocus( hwndStatus );

						break;

					// Stop Emulation

					case IDM_EMULATION_STOP:

                        main_stop();
                        main_hide3dfx();
                        UpdateWindow(hwndStatus);

						break;

					// Pause Emulation

					case IDM_EMULATION_PAUSE:
                        if(main_executing())
                        {
                            main_stop();
                            // NOTE: difference to stop is that
                            // hide3dfx is not called
                        }
                        else
                        {
                            main_start();
                            SetFocus( hwndStatus );
                        }

						break;

					// Reset Emulator

					case IDM_EMULATION_RESET:
                        main_command("reset");
						break;

					// Unlock 'Stuck' Emulation

					case IDM_EMULATION_UNLOCK:
                        main_command("event all");
						break;

					// Enable/Disable Sound Emulation

					case IDM_OPTIONS_ENABLESOUND:

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );

						if( GetMenuState( hSubMenu, IDM_OPTIONS_ENABLESOUND,
												MF_BYCOMMAND ) && MF_CHECKED )
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_ENABLESOUND,
												MF_BYCOMMAND | MF_UNCHECKED );

                            // S: Need to Call Disable Sound Command
                            main_command("sound 0");
						}
						else
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_ENABLESOUND,
												MF_BYCOMMAND | MF_CHECKED );

                            // S: Need to Call Enable Sound Command
                            main_command("sound 1");
						}

						break;

					// Enable/Disable Grpahics Emulation

					case IDM_OPTIONS_ENABLEGRAPHICS:

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );

						if( GetMenuState( hSubMenu, IDM_OPTIONS_ENABLEGRAPHICS,
												MF_BYCOMMAND ) && MF_CHECKED )
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_ENABLEGRAPHICS,
												MF_BYCOMMAND | MF_UNCHECKED );

                            // S: Need to Call Disable Graphics Command
                            main_command("graphics 0");
                            if(main_executing())
                            {
                                main_hide3dfx(); // this will stop execution
                                main_start();
                                SetFocus( hwndStatus );
                            }
                            else
                            {
                                main_hide3dfx();
                            }
						}
						else
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_ENABLEGRAPHICS,
												MF_BYCOMMAND | MF_CHECKED );

                            // S: Need to Call Enable Graphics Command
                            main_command("graphics 1");
						}

						break;

					// Set Screen Resolution to 512 x 384

					case IDM_SCREENRES_512x384:

                        // S: Need to check that emu is not running first as
						// resolution can not be changed during execution
                        // ST: fixed that, now it can be changed :)

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_512x384,
											MF_BYCOMMAND | MF_CHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_640x480,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_800x600,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_1024x768,
											MF_BYCOMMAND | MF_UNCHECKED );

                        // S: Need to Set Resolution to 512 x 384.
                        main_command("resolution 512");

						break;

					// Set Screen Resolution to 640 x 480

					case IDM_SCREENRES_640x480:

                        // S: Need to check that emu is not running first as
						// resolution can not be changed during execution
                        // ST: fixed that, now it can be changed :)

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_512x384,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_640x480,
											MF_BYCOMMAND | MF_CHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_800x600,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_1024x768,
											MF_BYCOMMAND | MF_UNCHECKED );

                        // S: Need to Set Resolution to 640 x 480
                        main_command("resolution 640");

						break;

					// Set Screen Resolution to 800 x 600

					case IDM_SCREENRES_800x600:

                        // S: Need to check that emu is not running first as
						// resolution can not be changed during execution
                        // ST: fixed that, now it can be changed :)

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_512x384,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_640x480,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_800x600,
											MF_BYCOMMAND | MF_CHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_1024x768,
											MF_BYCOMMAND | MF_UNCHECKED );

                        // S: Need to Set Resolution to 800 x 600
                        main_command("resolution 800");

						break;

					// Set Screen Resolution to 1024 x 768

					case IDM_SCREENRES_1024x768:

                        // S: Need to check that emu is not running first as
						// resolution can not be changed during execution
                        // ST: fixed that, now it can be changed :)

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_512x384,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_640x480,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_800x600,
											MF_BYCOMMAND | MF_UNCHECKED );
   					CheckMenuItem( hSubMenu, IDM_SCREENRES_1024x768,
											MF_BYCOMMAND | MF_CHECKED );

                        // S: Need to Set Resolution to 1024 x 768
                        main_command("resolution 1024");

						break;

					// Enable/Disable Wireframe Display

					case IDM_OPTIONS_WIREFRAME:

						// Note: Wireframe Display is OFF by Default.

                  hMenu = GetMenu( hwndMain );
                  hSubMenu = GetSubMenu( hMenu, 2 );

						if( GetMenuState( hSubMenu, IDM_OPTIONS_WIREFRAME,
												MF_BYCOMMAND ) && MF_CHECKED )
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_WIREFRAME,
												MF_BYCOMMAND | MF_UNCHECKED );

                            // S: Need to Disable Wireframe Display
                            main_command("wireframe 0");
						}
						else
						{
							CheckMenuItem( hSubMenu, IDM_OPTIONS_WIREFRAME,
												MF_BYCOMMAND | MF_CHECKED );

                            // S: Need to Enable Wireframe Display
                            main_command("wireframe 1");
						}

						break;

					// Take a Screenshot

					case IDM_OPTIONS_SCREENSHOT:

                        // S: Call Screenshot Command
                        main_command("screen");

						break;

					// Flip Between Windows and 3DFX Display

					case IDM_OPTIONS_FLIP3DFX:

                        // S: Call Flip 3DFX Command

                  rdp_togglefullscreen();

						break;

               //  Configure Controllers

               case IDM_CONTROLLER_CONFIGURE:

                  ControllerProperties();

                  break;

					// Help Topics

					case IDM_HELP_HELPTOPICS:

						// GH: To Be Completed

						break;

					// About the Emulator

					case IDM_HELP_ABOUT:

                  DialogBox( hInst, MAKEINTRESOURCE( IDD_ABOUT ),
                             hwndMain, AboutDialog );
						break;

               // Ignore any other Application Defined Messages

               default:

                  break;
            }

            break;

         // Handle Window Resizing

         case WM_SIZE:

            // Get Window Size Parameters

            GetClientRect( hwndMain, &rcl );

            hdwp = BeginDeferWindowPos( 3 );

            // Resize Status Bar

            DeferWindowPos( hdwp, hwndStatus, NULL, 0, 0,
	                         rcl.right - rcl.left, 20,
                            SWP_NOZORDER | SWP_NOMOVE );

            // Resize ROM List View

            DeferWindowPos( hdwp, hwndList, NULL, 0, 0,
                            rcl.right - rcl.left,
                            rcl.bottom - rcl.top - 99,
                            SWP_NOZORDER );

            // Resize Debug List View

            DeferWindowPos( hdwp, hwndDebug, NULL,
                            0,
                            rcl.bottom - rcl.top - 100,
                            rcl.right - rcl.left,
                            80,
                            SWP_NOZORDER );

            EndDeferWindowPos( hdwp );

            // Resize Debug Output Column

            ListView_SetColumnWidth( hwndDebug, 0, rcl.right );

            // Set Status Bar Section Widths Accordingly

            aWidths[0] = rcl.right - 500;
            aWidths[1] = rcl.right - 391;
            aWidths[2] = rcl.right - 316;
				aWidths[3] = rcl.right - 241;
            aWidths[4] = rcl.right - 166;
            aWidths[5] = rcl.right - 91;
            aWidths[6] = rcl.right - 16;
            
            SendMessage( hwndStatus, SB_SETPARTS, 7, (LPARAM)aWidths );

            SendMessage( hwndStatus, SB_SETTEXT, 1, (LPARAM)"CYCLE" );
            SendMessage( hwndStatus, SB_SETTEXT, 2, (LPARAM)"CPU" );
				SendMessage( hwndStatus, SB_SETTEXT, 3, (LPARAM)"GFX" );
            SendMessage( hwndStatus, SB_SETTEXT, 4, (LPARAM)"SND" );
            SendMessage( hwndStatus, SB_SETTEXT, 5, (LPARAM)"IDLE" );
            SendMessage( hwndStatus, SB_SETTEXT, 6, (LPARAM)"FPS" );

            break;

         // Terminate the Application

         case WM_DESTROY:                 

            // Do whatever tidying up is needed
            main_command("x");

            PostQuitMessage( 0 );

            break;

         // Handle Notification Messages

         case WM_NOTIFY:

            if( wParam != ID_LISTVIEW )
               break;

            pNm = (NMLISTVIEW *)lParam;

            switch( pNm->hdr.code )
            {
               case NM_DBLCLK:

                  ListView_GetItemText( hwndList,
                                        pNm->iItem,
                                        3,
                                        szBuffer,
                                        MAX_PATH );

                  SetCursor(LoadCursor(NULL,IDC_WAIT));
                  main_command("rom %s",szBuffer);
                  SetCursor(LoadCursor(NULL,IDC_ARROW));

                  if(main_commanderrors()>1)
                  {
                     MessageBox(NULL,"Error loading","UltraHLE",MB_OK);
                  }
                  else
                  {
                     main_start();
                     SetFocus( hwndStatus );
                  }

                  break;

               default:

                  break;
            }

            break;

         case WM_ACTIVATE:

            // This will switch the screen accordingly in BOTH losing and
            // setting the focus.

            //rdp_togglefullscreen();

            // ST: rdp_togglefullscreen doesn't work on banshee. This
            // is a bit more complicated, but should work on all cards.

            if(init.showconsole)
            {
                // ST: the console is considered a separate window and
                // application by windows. ARGH. So I disabled this
                // when using console (you can still use F12 to toggle
                // gfx there and disable keys in that way).
                break;
            }

            if(LOWORD(wParam)==WA_INACTIVE)
            {
                main_hide3dfx();
            }
            else
            {
                main_show3dfx();
            }

            break;

         // Handle any Unhandle Windows Messages

         default:

            return( DefWindowProc( hwnd, message, wParam, lParam ) );

            break;
      }

      // Return from Main Window Message Handling Procedure

      return( 0L );
   }

   // About UltraHLE Dialog

   BOOL CALLBACK AboutDialog( HWND hdwnd, UINT message,
                              WPARAM wParam, LPARAM lParam )
   {
      switch( message )
      {
         case WM_COMMAND:

            switch( LOWORD( wParam ) )
            {
               case IDOK:

                  // Close the Dialog Window

                  EndDialog( hdwnd, 0 );
                  return( TRUE );

                  break;
            }

            break;

         case WM_INITDIALOG:

            ShowWindow( hdwnd, SW_SHOW );

            break;
      }

      return( FALSE );
   }

   // Display Controller Configuration Properties

   int ControllerProperties( void )
   {
      PROPSHEETPAGE   psp[1];
      PROPSHEETHEADER psh;

      // Define the Property Sheet Page

      psp[0].dwSize      = sizeof( PROPSHEETPAGE );
      psp[0].dwFlags     = PSP_USETITLE;
      psp[0].hInstance   = hInst;
      psp[0].pszTemplate = MAKEINTRESOURCE( IDD_CONTROLLER1 );
      psp[0].pszIcon     = NULL;
      psp[0].pfnDlgProc  = (DLGPROC)Controller1Prop;
      psp[0].pszTitle    = "Controller 1";
      psp[0].lParam      = 0;

      // Define Property Sheet Header

      psh.dwSize     = sizeof( PROPSHEETHEADER );
      psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USEICONID;
      psh.hwndParent = hwndMain;
      psh.hInstance  = hInst;
      psh.pszIcon    = MAKEINTRESOURCE( IDI_N64CART );
      psh.pszCaption = (LPSTR)"Controller Configuration";
      psh.nPages     = 1;
      psh.nStartPage = 0;
      psh.ppsp       = (LPCPROPSHEETPAGE) &psp;

      return( PropertySheet( &psh ) );
   }

   // Controller 1 Configuration Tabbed Dialog

   BOOL APIENTRY Controller1Prop( HWND hDlg, UINT message, 
                                  UINT wParam, LONG lParam )
   {
      static PROPSHEETPAGE *ps;        // Pointer to Property Sheet Information

      switch( message )
      {
         case WM_INITDIALOG:

            // Save the PROPSHEETPAGE Information

            ps = (PROPSHEETPAGE *)lParam;
            return( TRUE );
            break;

         case WM_NOTIFY:

            switch( ((NMHDR FAR *)lParam)->code )
            {
               case PSN_SETACTIVE:

                  // Initialise the Information Fields

                  SetDlgItemText( hDlg, IDC_C1_START, "S" );
                  SetDlgItemText( hDlg, IDC_C1_A, "A" );
                  SetDlgItemText( hDlg, IDC_C1_B, "X" );
                  SetDlgItemText( hDlg, IDC_C1_Z, "Z" );
                  SetDlgItemText( hDlg, IDC_C1_L, "C" );
                  SetDlgItemText( hDlg, IDC_C1_R, "V" );
                  SetDlgItemText( hDlg, IDC_C1_CU, "I" );
                  SetDlgItemText( hDlg, IDC_C1_CD, "K" );
                  SetDlgItemText( hDlg, IDC_C1_CL, "J" );
                  SetDlgItemText( hDlg, IDC_C1_CR, "L" );
                  SetDlgItemText( hDlg, IDC_C1_DU, "T" );
                  SetDlgItemText( hDlg, IDC_C1_DD, "G" );
                  SetDlgItemText( hDlg, IDC_C1_DL, "F" );
                  SetDlgItemText( hDlg, IDC_C1_DR, "H" );
                  SetDlgItemText( hDlg, IDC_C1_AU, "Up" );
                  SetDlgItemText( hDlg, IDC_C1_AD, "Down" );
                  SetDlgItemText( hDlg, IDC_C1_AL, "Left" );
                  SetDlgItemText( hDlg, IDC_C1_AR, "Right" );
                  SetDlgItemText( hDlg, IDC_C1_SLOWER, "Shift" );
                  SetDlgItemText( hDlg, IDC_C1_MUCHSLOWER, "Ctrl" );

                  SetDlgItemText( hDlg, IDC_C1_STICK, "Keyboard" );
                  SendDlgItemMessage( hDlg, IDC_C1_STICK, CB_ADDSTRING, 0, 
                                      (LPARAM)"Keyboard" ); 
                  SendDlgItemMessage( hDlg, IDC_C1_STICK, CB_ADDSTRING, 0,
                                      (LPARAM)"Joystick" );
                  break;

               case PSN_APPLY:

                  GetDlgItemText( hDlg, IDC_C1_STICK, szBuffer, MAX_PATH );

                  if( !strncmp( szBuffer, "Joy", 3 ) )
                  {
                     pad_enablejoy(1);
                  }
                  else
                  {
                     pad_enablejoy(0);
                  }

                  SetWindowLong( hDlg, DWL_MSGRESULT, TRUE );

                  break;

               case PSN_KILLACTIVE:

                  SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
                  return( 1 );
                  break;

               case PSN_RESET:

                  SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
                  break;

               default:

                  break;
            }

            break;
      }

      return( FALSE );
   }
