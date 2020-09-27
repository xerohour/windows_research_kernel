/**********************************************************************
 *
 * PROGRAM: RES2RC.C
 *
 * PURPOSE: Converts a .RES file to an .RC file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 7, Resource (.RES) File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "ver.h"

/* Declare some global variables */

/* This is a list of known resource types, all handled by the program */
char *res_array[] = { "", "CURSOR", "BITMAP", "ICON", "MENU", "DIALOG",
 "STRINGTABLE", "FONTDIR", "FONT", "ACCELERATORS", "RCDATA", "", 
 "GROUP CURSOR", "", "GROUP ICON" };

int  INDENT=0;        /* records the current number of indented spaces */
WORD StringCount;
WORD VersionUsed = 0; /* records whether ver.h has been included */

/* Define a variable to hold the memory-flags field for the current  */
/* resource.  This is used because for 2 fields (cursor and icon),   */
/* each resource of either type is separated into 2 resources in the */
/* .res file by the rc compiler: data and header.  The correct memory*/
/* flags are set in the first occurrence, but I don't write out the  */
/* resource until I hit the header.  Since we want to write out the  */
/* correct memory flags (e.g., LOADONCALL), we save the flag value in*/
/* this variable, and then use it when we hit the header, instead of */
/* using the header's flag.  It is only saved in the get_mem_flags() */
/* function, since that is the only time we skip a resource header,  */
/* such as for cursors and icons.                                    */
WORD prev_mem_flag=0;

/* define the newline and carriage return characters */
#define NL 10
#define CR 13

/* define constants for different Version Info block types */
#define STRINGBLOCK 1
#define VARBLOCK    2
#define OTHERBLOCK  3

/* define the number of spaces to indent */
#define INDENT_SPACES 3

/* define integer values for some resource types - try to make more elegant */
#define CURSOR_TYPE 1
#define ICON_TYPE   3
#define STRING_TYPE 6

struct MenuHeader {
   WORD wVersion;
   WORD wReserved;
};

struct AccelTableEntry {
   BYTE fFlags;
   WORD wEvent;
   WORD wId;
};

/* structure of icon data in .res file */
typedef struct IconDirectoryEntry {
   BYTE  bWidth;
   BYTE  bHeight;
   BYTE  bColorCount;
   BYTE  bReserved;
   WORD  wPlanes;
   WORD  wBitCount;
   DWORD dwBytesInRes;
   WORD  wImageOffset;
} ICONDIRENTRY;

/* structure of icon data in .ico file */
typedef struct IconResourceEntry {
   BYTE  bWidth;
   BYTE  bHeight;
   BYTE  bColorCount;
   BYTE  bReserved;
   WORD  wPlanes;
   WORD  wBitCount;
   DWORD dwBytesInRes;
   DWORD dwImageOffset;
} ICONRESENTRY;

typedef struct ICONDIR {
   WORD          idReserved;
   WORD          idType;
   WORD          idCount;
} ICONHEADER;

/* structure of cursor data in .res file */
typedef struct CursorDirectoryEntry {
   WORD  wWidth;
   WORD  wHeight;
   WORD  wPlanes;
   WORD  wBitCount;
   DWORD dwBytesInRes;
   WORD  wImageOffset;
} CURSORDIRENTRY;

/* structure of cursor data in .cur file */
typedef struct CursorResourceEntry {
   BYTE  bWidth;
   BYTE  bHeight;
   BYTE  bColorCount;
   BYTE  bReserved;
   WORD  wXHotSpot;
   WORD  wYHotSpot;
   DWORD dwBytesInRes;
   DWORD dwImageOffset;
} CURSORRESENTRY;

typedef struct CURSORDIR {
   WORD cdReserved;
   WORD cdType;
   WORD cdCount;
} CURSORHEADER;

typedef struct DIALOGINFO {
   DWORD lStyle;
   BYTE  bNumberOfItems;
   WORD  x;
   WORD  y;
   WORD  width;
   WORD  height;
} DIALOGHEADER;

typedef struct DIALOGCONTROL {
   WORD  x;
   WORD  y;
   WORD  width;
   WORD  height;
   WORD  id;
   DWORD lStyle;
} CONTROLDATA;

