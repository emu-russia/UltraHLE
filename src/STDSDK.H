////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// stdsdk.h

#ifndef _STDSDK_H_
#define _STDSDK_H_

#include <windows.h>                   // Master Include File for Windows Apps
#include <commctrl.h>                  // Interface for the Common Controls  
#include "resource.h"                  // Application Resource Definitions
#include "ultra.h"                     // UltraHLE Core Emu File

   // Typedefs

   typedef struct _RomList
   {
      char  name[MAX_PATH];                // ROM Name
      char  alttitle[MAX_PATH];             // Alternate ROM Name
      char  country;                       // Country Code
      char  filename[MAX_PATH];            // ROM File Name
      char  comment[MAX_PATH];             // Comments on ROM Functionality
      DWORD dwFileSize;                    // ROM File Size
   } ROMLIST;

#endif // _STDSDK_H_