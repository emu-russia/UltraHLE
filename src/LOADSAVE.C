////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// loadsave.c

#include "loadsave.h"
#include "main.h" // ST:

	// Load Image or State Procedure
	// loadType Settings : TRUE  = Load Image
	//		        			  FALSE = Load State

   BOOL LoadImageState( BOOL loadType )
   {
      TCHAR szFile[ MAX_PATH ] = "\0"; // Filename

		// Load Image Filter

		LPCTSTR szFilterI = "N64 Rom Images (bin,mov,n64,rom,u64,v64)\0"
				              "*.bin;*.mov;*.n64;*.rom;*.u64;*.v64\0"
					           "All Files (*.*)\0"
						        "*.*\0\0";

		// Load State Filter

   	LPCTSTR szFilterS = "Saved State Files (*.sav)\0*.sav\0"
								  "All Files (*.*)\0*.*\0\0";

		// Open Filename Flags

      DWORD ofnFlags = OFN_FILEMUSTEXIST |
                       OFN_HIDEREADONLY  |
                       OFN_PATHMUSTEXIST;

        LPCTSTR startdir=init.rootpath;

      // ST: set starting paths from inifile
      if(loadType==TRUE)
      {
         if(*init.rompath) startdir=init.rompath;
      }
      else
      {
         if(*init.savepath) startdir=init.savepath;
      }

      // Fill in the OPENFILENAME Structure

		if( loadType )							// Load Image
		{
         OpenFileName.lpstrFilter = szFilterI;
		   OpenFileName.lpstrTitle  = "Open";
			OpenFileName.lpstrDefExt = "*.rom";
		}
		else										// Load State
		{
         OpenFileName.lpstrFilter = szFilterS;
			OpenFileName.lpstrTitle  = "Load State";
		   OpenFileName.lpstrDefExt = "*.*";
		}


      OpenFileName.lStructSize       = sizeof( OPENFILENAME );
      OpenFileName.hwndOwner         = hwndMain;
      OpenFileName.hInstance         = hInst;
      OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
      OpenFileName.nMaxCustFilter    = 0;
      OpenFileName.nFilterIndex      = 1L;
      OpenFileName.lpstrFile         = szFile;
      OpenFileName.nMaxFile          = sizeof( szFile );
      OpenFileName.lpstrFileTitle    = NULL;
      OpenFileName.nMaxFileTitle     = 0;
      OpenFileName.lpstrInitialDir   = startdir;
      OpenFileName.nFileOffset       = 0;
      OpenFileName.nFileExtension    = 0;
      OpenFileName.Flags             = ofnFlags;
      OpenFileName.lCustData         = 0;
      OpenFileName.lpfnHook          = NULL;
      OpenFileName.lpTemplateName    = NULL;

     // ST: Stop execution BEFORE opening file dialog
     main_stop();
     main_hide3dfx();

      // Call the Common Dialog Function

      if( GetOpenFileName( &OpenFileName ) )
      {

         if( loadType ) // Load Image
         {
             SetCursor(LoadCursor(NULL,IDC_WAIT));
             main_command("rom %s",OpenFileName.lpstrFile);
             SetCursor(LoadCursor(NULL,IDC_ARROW));
         }
         else // Load State
         {
            SetCursor(LoadCursor(NULL,IDC_WAIT));
            main_command("load %s",OpenFileName.lpstrFile);
            SetCursor(LoadCursor(NULL,IDC_ARROW));
         }

         if(main_commanderrors()>1)
         {
            MessageBox(NULL,"Error loading","UltraHLE",MB_OK);
         }
         else
         {
            main_command("go");
         }
      }
      else
         return( FALSE );				   // No File was Selected

      // ROM is Now Ready to Execute

      return( TRUE );
   }


   // Save State

   BOOL SaveState( void )
   {
      DWORD ssFlags = OFN_PATHMUSTEXIST;
   	LPCTSTR szFilterS = "State Files (*.sav)\0*.sav\0"
								  "All Files (*.*)\0*.*\0\0";
      TCHAR szFile[ MAX_PATH ] = "\0"; // Filename

      LPCTSTR startdir=init.rootpath;
      if(*init.savepath) startdir=init.savepath;

      // Fill in the OPENFILENAME Structure
      
      OpenFileName.lStructSize       = sizeof( OPENFILENAME );
      OpenFileName.hwndOwner         = hwndMain;
      OpenFileName.hInstance         = hInst;
      OpenFileName.lpstrFilter       = szFilterS;
      OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
      OpenFileName.nMaxCustFilter    = 0;
      OpenFileName.nFilterIndex      = 1L;
      OpenFileName.lpstrFile         = szFile;
      OpenFileName.nMaxFile          = sizeof( szFile );
      OpenFileName.lpstrFileTitle    = NULL;
      OpenFileName.nMaxFileTitle     = 0;
      OpenFileName.lpstrInitialDir   = startdir;
      OpenFileName.lpstrTitle        = "Save State As...";
      OpenFileName.Flags             = ssFlags;
		OpenFileName.lpstrDefExt       = "*.rom";
      OpenFileName.nFileOffset       = 0;
      OpenFileName.nFileExtension    = 0;
      OpenFileName.lCustData         = 0;
      OpenFileName.lpfnHook          = NULL;
      OpenFileName.lpTemplateName    = NULL;

     // ST: Stop execution BEFORE opening file dialog
     main_stop();
     main_hide3dfx();

      // Call the Common Dialog Function

      if( GetSaveFileName( &OpenFileName ) )
      {
            // ST: Changed how this works, now using main_command
            // if executing right now, stop it
            main_stop();

            SetCursor(LoadCursor(NULL,IDC_WAIT));
            main_command("save %s",OpenFileName.lpstrFile);
            SetCursor(LoadCursor(NULL,IDC_ARROW));

         if(main_commanderrors()>1)
         {
            MessageBox(NULL,"Error Saving State","UltraHLE",MB_OK);
         }
         else
         {
            main_command("go");
         }
      }
      else
         return( FALSE );				   // No File was Selected

      return( TRUE );
   }
